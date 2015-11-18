
#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>

#include "client.h"

int main() {

    ssh_session session;
    ssh_channel channel;
    int rc, port = 2222;
    char buffer[1024];
    char choix;
    unsigned int nbytes;

    printf("Session...\n");
    session = ssh_new();
    if (session == NULL) exit(-1);

    int i=1;
    ssh_options_set(session, SSH_OPTIONS_HOST, "127.0.0.1");
    ssh_options_set(session, SSH_OPTIONS_SSH1,&i);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_USER, "atlas");
    ssh_options_set(session, SSH_OPTIONS_HOSTKEYS,"ssh-rsa");

    printf("Connecting...\n");
    rc = ssh_connect(session);
    if (rc != SSH_OK) error(session);


    printf("Autentication...\n");
    authenticate_password(session);

    int continuer=1;
    while(continuer)
    {
        printf("Tapez 's' pour avoir un shell ou 'c' pour envoyer une commande 'q' pour quitter :");
        scanf("%c",&choix);
        if(choix=='s')
        {
             shell_session(session);
        }else if(choix=='c')
        {
          printf("Entrez la commande a envoyer :");
          scanf("%s",buffer);
          rc = send_command_and_receive_result(session,buffer);
          if(rc!= SSH_OK){
            fprintf(stderr,"erreur commande\n");
          }
        }else if(choix=='q')
        {
          continuer=0;
        }
    }
    free_session(session);

    return 0;
}

int authenticate_password(ssh_session session)
{
  char *password="EffyKurt";
  int rc;
  rc = ssh_userauth_password(session, NULL, password);
  if (rc == SSH_AUTH_ERROR)
  {
     fprintf(stderr, "Authentication failed: %s\n",
       ssh_get_error(session));
     return SSH_AUTH_ERROR;
  }
  return rc;
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
printf("GET CHANNEL OK\n");
  if (channel == NULL)
    return SSH_ERROR;
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    fprintf(stderr,"error channel open session\n");
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



void free_channel(ssh_channel channel) {
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void free_session(ssh_session session) {
    ssh_disconnect(session);
    ssh_free(session);
}

void error(ssh_session session) {
    fprintf(stderr, "Error: %s\n", ssh_get_error(session));
    free_session(session);
    exit(-1);
}
