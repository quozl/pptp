/* pptp_callmgr.h ... Call manager for PPTP connections.
 *                    Handles TCP port 1723 protocol.
 *                    C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_callmgr.h,v 1.2 2003/02/15 04:32:50 quozl Exp $
 */

#define PPTP_SOCKET_PREFIX "/var/run/pptp/"
#define PPTP_CALLMGR_BINARY "./pptp_callmgr"

void name_unixsock(struct sockaddr_un *where,
		   struct in_addr inetaddr,
		   struct in_addr localbind);
