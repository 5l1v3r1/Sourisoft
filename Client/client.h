#ifndef _CLIENT_H_
#define _CLIENT_H_


int authenticate_password(ssh_session session);

int send_command_and_receive_result(ssh_session session,char* command);

int shell_session(ssh_session session);

int kbhit();

int interactive_shell_session(ssh_channel channel);

void free_channel(ssh_channel channel);

void free_session(ssh_session session);


void error(ssh_session session);

#endif