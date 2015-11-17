/**
*     Sourisoft's client 
*
*		Connection and authentification functions header
*
*     @author Kevin Hascoët
*
*
*
*/

#ifndef _ConnectAuth_
#define _ConnectAuth_


#define PORT 2222
#define LIBSSH_STATIC 1
#define PASSWORD "*****"

#include <libssh/libssh.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ssh_session connect_ssh(const char *host, const char *user,int verbosity);
int verify_knownhost(ssh_session session);
int authenticate_console(ssh_session session);

#endif
