/* pptp.c ... client shell to launch call managers, data handlers, and
 *            the pppd from the command line.
 *            C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp.c,v 1.2 2000/12/23 08:32:15 scott Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <pty.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/wait.h>
#include "pptp_callmgr.h"
#include "pptp_gre.h"
#include "version.h"
#include "inststr.h"
#include "util.h"
#include "pty.h"

#ifndef PPPD_BINARY
#define PPPD_BINARY "pppd"
#endif

struct in_addr get_ip_address(char *name);
int open_callmgr(struct in_addr inetaddr, int argc,char **argv,char **envp);
void launch_callmgr(struct in_addr inetaddr, int argc,char **argv,char **envp);
void get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id);
void launch_pppd(char *ttydev, int argc, char **argv);

void usage(char *progname) {
  fprintf(stderr,
	  "%s\n"
	 "Usage:\n"
	 " %s hostname [pppd options]\n", version, progname);
  exit(1);
}

static int signaled = 0;

void do_nothing(int sig) { 
    /* do nothing signal handler. Better than SIG_IGN. */
    signaled = 1;
}

sigjmp_buf env;
void sighandler(int sig) {
  siglongjmp(env, 1);
}

int main(int argc, char **argv, char **envp) {
  struct in_addr inetaddr;
  int callmgr_sock;
  char ttydev[TTYMAX];
  int pty_fd, tty_fd, rc;
  pid_t parent_pid, child_pid;
  u_int16_t call_id, peer_call_id;
  
  if (argc < 2)
    usage(argv[0]);

  /* Step 1: Get IP address for the hostname in argv[1] */
  inetaddr = get_ip_address(argv[1]);

  /* Step 2: Open connection to call manager
   *         (Launch call manager if necessary.)
   */
  callmgr_sock = open_callmgr(inetaddr, argc,argv,envp);

  /* Step 3: Find an open pty/tty pair. */
  /* pty_fd = getpseudotty(ttydev, ptydev); */
  rc = openpty (&pty_fd, &tty_fd, ttydev, NULL, NULL);
  if (rc < 0) { close(callmgr_sock); fatal("Could not find free pty."); }
  
  /* Step 4: fork and wait. */
  signal(SIGUSR1, do_nothing); /* don't die */
  parent_pid = getpid();
  switch (child_pid = fork()) {
  case -1:
    fatal("Could not fork pppd process");

  case 0: /* I'm the child! */
    close (tty_fd);
    signal(SIGUSR1, SIG_DFL);
    child_pid = getpid();
    break;
  default: /* parent */
    close (pty_fd);
    /*
     * There is still a very small race condition here.  If a signal
     * occurs after signaled is checked but before pause is called,
     * things will hang.
     */
    if (!signaled) {
	pause(); /* wait for the signal */
    }
    launch_pppd(ttydev, argc-2, argv+2); /* launch pppd */
    perror("Error");
    fatal("Could not launch pppd");
  }

  /* Step 5: Exchange PIDs, get call ID */
  get_call_id(callmgr_sock, parent_pid, child_pid, 
	      &call_id, &peer_call_id);

  /* Step 5b: Send signal to wake up pppd task */
  kill(parent_pid, SIGUSR1);
  sleep(2);

  if (sigsetjmp(env, 1)!=0) goto shutdown;
  signal(SIGINT,  sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGKILL, sighandler);
  
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "pptp: GRE-to-PPP gateway on %s", ttydev);
    inststr(argc,argv,envp, buf);
  }

  /* Step 6: Do GRE copy until close. */
  pptp_gre_copy(call_id, peer_call_id, pty_fd, inetaddr);

shutdown:
  /* on close, kill all. */
  kill(parent_pid, SIGTERM);
  close(pty_fd);
  close(callmgr_sock);
  exit(0);
}

struct in_addr get_ip_address(char *name) {
  struct in_addr retval;
  struct hostent *host = gethostbyname(name);
  if (host==NULL) {
    if (h_errno == HOST_NOT_FOUND)
      fatal("gethostbyname: HOST NOT FOUND");
    else if (h_errno == NO_ADDRESS)
      fatal("gethostbyname: NO IP ADDRESS");
    else
      fatal("gethostbyname: name server error");
  }
  
  if (host->h_addrtype != AF_INET)
    fatal("Host has non-internet address");
  
  memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
  return retval;
}

int open_callmgr(struct in_addr inetaddr, int argc, char **argv, char **envp) {
  /* Try to open unix domain socket to call manager. */
  struct sockaddr_un where;
  const int NUM_TRIES = 3;
  int i, fd;
  pid_t pid;
  int status;

  /* Open socket */
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    fatal("Could not create unix domain socket: %s", strerror(errno));
  }

  /* Make address */
  where.sun_family = AF_UNIX;
  snprintf(where.sun_path, sizeof(where.sun_path), 
	   PPTP_SOCKET_PREFIX "%s", inet_ntoa(inetaddr));

  for (i=0; i<NUM_TRIES; i++) {
    if (connect(fd, (struct sockaddr *) &where, sizeof(where)) < 0) {
      /* couldn't connect.  We'll have to launch this guy. */

      unlink (where.sun_path);	

      /* fork and launch call manager process */
      switch (pid=fork()) {
      case -1: /* failure */
	  fatal("fork() to launch call manager failed.");
      case 0: /* child */
	  {
	      close (fd);
	      launch_callmgr(inetaddr, argc,argv,envp);
	  }
      default: /* parent */
	  waitpid(pid, &status, 0);
	  if (status!=0)
	      fatal("Call manager exited with error %d", status);
	  break;
      }
      sleep(1);
    }
    else return fd;
  }
  close(fd);
  fatal("Could not launch call manager after %d tries.", i);
  return -1;   /* make gcc happy */
}

void launch_callmgr(struct in_addr inetaddr, int argc,char**argv,char**envp) {
      int callmgr_main(int argc, char**argv, char**envp);
      char *my_argv[2] = { argv[0], inet_ntoa(inetaddr) };
      char buf[128];
      snprintf(buf, sizeof(buf), "pptp: call manager for %s", my_argv[1]);
      inststr(argc,argv,envp,buf);
      exit(callmgr_main(2, my_argv, envp));
      /*
      const char *callmgr = PPTP_CALLMGR_BINARY;
      execlp(callmgr, callmgr, inet_ntoa(inetaddr), NULL);
      fatal("execlp() of call manager [%s] failed: %s", 
	  callmgr, strerror(errno));
      */
}

/* XXX need better error checking XXX */
void get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id) {
  u_int16_t m_call_id, m_peer_call_id;
  /* write pid's to socket */
  /* don't bother with network byte order, because pid's are meaningless
   * outside the local host.
   */
  write(sock, &gre, sizeof(gre));
  write(sock, &pppd, sizeof(pppd));
  read(sock,  &m_call_id, sizeof(m_call_id));
  read(sock,  &m_peer_call_id, sizeof(m_peer_call_id));
  /* XXX FIX ME ... DO ERROR CHECKING & TIME-OUTS XXX */
  *call_id = m_call_id;
  *peer_call_id = m_peer_call_id;
}

void launch_pppd(char *ttydev, int argc, char **argv) {
  char *new_argv[argc+4]; /* XXX if not using GCC, hard code a limit here. */
  int i;

  new_argv[0] = PPPD_BINARY;
  new_argv[1] = ttydev;
  new_argv[2] = "38400";
  for (i=0; i<argc; i++)
    new_argv[i+3] = argv[i];
  new_argv[i+3] = NULL;
  execvp(new_argv[0], new_argv);
}

/*************** COMPILE call manager into same binary *********/
#define main       callmgr_main
#define sighandler callmgr_sighandler
#define do_nothing callmgr_do_nothing
#include "pptp_callmgr.c"
