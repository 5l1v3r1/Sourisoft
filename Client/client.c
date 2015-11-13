/**
*     Sourisoft's client     
*
*     @author Kevin Hascoët
*
      g++ client.cpp -o client -lssh -Wall
*
*/
#define LIBSSH_STATIC 1

#include <libssh/libssh.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h>

#include "ConnectAuth.h"

int send_command_and_receive_result(ssh_session session,char* command);
int shell_session(ssh_session session);
int interactive_shell_session(ssh_channel channel);

int main(int argc, char const *argv[])
{
	ssh_session my_ssh_session;
	my_ssh_session = ssh_new();
  int verbosity = SSH_LOG_NOLOG;
	if (my_ssh_session == NULL)
		return -1;

 
	my_ssh_session = connect_ssh("127.0.0.1","atlas",verbosity);
  if(my_ssh_session == NULL)
  {
    printf("Myssh NULL error conenct_ssh\n");
    return -1;
  }
  printf("Connection Terminée lancement du shell");
	/* Le code du client */
	send_command_and_receive_result(my_ssh_session,"ls");
  //shell_session(my_ssh_session);

	
	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);
	return 0;
}

int send_command_and_receive_result(ssh_session session,char* command)
{
  ssh_channel channel;
  int rc;
  char buffer[256];
  int nbytes;
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
  rc = ssh_channel_request_exec(channel, command);
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (write(1, buffer, nbytes) != (unsigned int) nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }
    
  if (nbytes < 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}


int shell_session(ssh_session session)
{
  ssh_channel channel;
  int rc;
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
  interactive_shell_session(channel);

  ssh_channel_close(channel);
  ssh_channel_send_eof(channel);
  ssh_channel_free(channel);
  return SSH_OK;
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int interactive_shell_session(ssh_channel channel)
{
  int rc;
  char buffer[256];
  int nbytes,nwritten;
  rc = ssh_channel_request_pty(channel);
  if (rc != SSH_OK) return rc;
  rc = ssh_channel_change_pty_size(channel, 80, 24);
  if (rc != SSH_OK) return rc;
  rc = ssh_channel_request_shell(channel);
  if (rc != SSH_OK) return rc;
  while (ssh_channel_is_open(channel) &&
         !ssh_channel_is_eof(channel))
  {
    nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
    if (nbytes < 0) return SSH_ERROR;
    if (nbytes > 0) 
    {
      nwritten = write(1, buffer, nbytes);
      if (nwritten != nbytes) return SSH_ERROR;
    }
    if (!kbhit())
    {
      usleep(50000L); // 0.05 second
      continue;
    }
    nbytes = read(0, buffer, sizeof(buffer));
    if (nbytes < 0) return SSH_ERROR;
    if (nbytes > 0)
    {
      nwritten = ssh_channel_write(channel, buffer, nbytes);
      if (nwritten != nbytes) return SSH_ERROR;
    }
  }

  return rc;
}