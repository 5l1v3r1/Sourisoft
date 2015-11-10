/**
*     server used for testing the client while developmnent
*     
*
*     @author Kevin Hascoët
*
      g++ serveur.cpp -o serveur -lssh -Wall
*
*/

#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <iostream>
#include <string>


int show_remote_processes(ssh_session session);

int main(int argc, char const *argv[])
{
	ssh_session my_ssh_session;
  ssh_bind sshbind;
	int verbosity = SSH_LOG_PROTOCOL;
	int port = 22;
  int r;

	my_ssh_session = ssh_new();
  sshbind = ssh_bind_new();
	if (my_ssh_session == NULL)
		return -1;

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(my_ssh_session,  SSH_OPTIONS_PORT, &port);
	
  
  ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, "/etc/ssh/rsa");
  ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, "2222");


  if(ssh_bind_listen(sshbind)<0){
          std::cerr<<"Error listening to socket: "<<ssh_get_error(sshbind)<<std::endl;
          return 1;
      }
  r=ssh_bind_accept(sshbind,my_ssh_session);
  if(r==SSH_ERROR){
    std::cerr<<"error accepting a connection : "<<ssh_get_error(sshbind)<<std::endl;
    return 1;
  }


	ssh_free(my_ssh_session);

	return 0;
}


int show_remote_processes(ssh_session session)
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
  rc = ssh_channel_request_exec(channel, "ps aux");
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

