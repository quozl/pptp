/* pptp_gre.h -- encapsulate PPP in PPTP-GRE.
 *               Handle the IP Protocol 47 portion of PPTP.
 *               C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_gre.h,v 1.1 2000/12/23 08:19:51 scott Exp $
 */

/*void pptp_gre_copy(u_int16_t call_id, u_int16_t peer_call_id,
		   char *pty, char *inetaddr);*/
void pptp_gre_copy(u_int16_t call_id, u_int16_t peer_call_id,
		   int pty_fd, struct in_addr inetaddr);
