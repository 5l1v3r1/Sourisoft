/* This is a sample implementation of a libssh based SSH server */
/*
Copyright 2014 Audrius Butkevicius

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action.
*/

#include <libssh/callbacks.h>
#include <poll.h>
#include <libssh/server.h>

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif
#include <fcntl.h>
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#include <signal.h>
#include <stdlib.h>
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdio.h>

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define RSAPATH "/home/atlas/.ssh/rsa_souris"
#define DSAPATH "/home/atlas/.ssh/dsa_souris"
#endif
#endif

#define USER "sourisoft"
#define PASS "softsouris"
#define BUF_SIZE 1048576
#define SESSION_END (SSH_CLOSED | SSH_CLOSED_ERROR)


struct session_data_struct {
    /* Pointer to the channel the session will allocate. */
    ssh_channel channel;
    int auth_attempts;
    int authenticated;
};


static void set_default_keys(ssh_bind sshbind,
                             int rsa_already_set,
                             int dsa_already_set,
                             int ecdsa_already_set) {
    if (!rsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
                             RSAPATH );
    }
    if (!dsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY,
                             DSAPATH);
    }
 /*   if (!ecdsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY,
                             KEYS_FOLDER "ssh_host_ecdsa_key");
    }
    */ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR,
                                 "0");

            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, "2222");
}

#ifdef HAVE_ARGP_H
const char *argp_program_version = "libssh server example "
SSH_STRINGIFY(LIBSSH_VERSION);
const char *argp_program_bug_address = "<libssh@libssh.org>";

/* Program documentation. */
static char doc[] = "libssh -- a Secure Shell protocol implementation";

/* A description of the arguments we accept. */
static char args_doc[] = "BINDADDR";

/* The options we understand. */
static struct argp_option options[] = {
    {
        .name  = "port",
        .key   = 'p',
        .arg   = "PORT",
        .flags = 0,
        .doc   = "Set the port to bind.",
        .group = 0
    },
    {
        .name  = "hostkey",
        .key   = 'k',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set a host key.  Can be used multiple times.  "
                 "Implies no default keys.",
        .group = 0
    },
    {
        .name  = "dsakey",
        .key   = 'd',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the dsa key.",
        .group = 0
    },
    {
        .name  = "rsakey",
        .key   = 'r',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the rsa key.",
        .group = 0
    },
    {
        .name  = "ecdsakey",
        .key   = 'e',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the ecdsa key.",
        .group = 0
    },
    {
        .name  = "no-default-keys",
        .key   = 'n',
        .arg   = NULL,
        .flags = 0,
        .doc   = "Do not set default key locations.",
        .group = 0
    },
    {
        .name  = "verbose",
        .key   = 'v',
        .arg   = NULL,
        .flags = 0,
        .doc   = "Get verbose output.",
        .group = 0
    },
    {NULL, 0, NULL, 0, NULL, 0}
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
     * know is a pointer to our arguments structure. */
    ssh_bind sshbind = state->input;
    static int no_default_keys = 0;
    static int rsa_already_set = 0, dsa_already_set = 0, ecdsa_already_set = 0;

    switch (key) {
        case 'n':
            no_default_keys = 1;
            break;
        case 'p':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, arg);
            break;
        case 'd':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, arg);
            dsa_already_set = 1;
            break;
        case 'k':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, arg);
            /* We can't track the types of keys being added with this
               option, so let's ensure we keep the keys we're adding
               by just not setting the default keys */
            no_default_keys = 1;
            break;
        case 'r':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, arg);
            rsa_already_set = 1;
            break;
        case 'e':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY, arg);
            ecdsa_already_set = 1;
            break;
        case 'v':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR,
                                 "3");
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
                /* Too many arguments. */
                argp_usage (state);
            }
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, arg);
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
                /* Not enough arguments. */
                argp_usage (state);
            }

            if (!no_default_keys) {
                set_default_keys(sshbind,
                                 rsa_already_set,
                                 dsa_already_set,
                                 ecdsa_already_set);
            }

            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};
#endif /* HAVE_ARGP_H */


static int auth_password(ssh_session session, const char *user,
                         const char *pass, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    (void) session;

    if (strcmp(user, USER) == 0 && strcmp(pass, PASS) == 0) {
        sdata->authenticated = 1;
        return SSH_AUTH_SUCCESS;
    }

    sdata->auth_attempts++;
    return SSH_AUTH_DENIED;
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

static ssh_channel channel_open(ssh_session session, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    sdata->channel = ssh_channel_new(session);
    return sdata->channel;
}

int shell_session(ssh_session session)
{

  ssh_channel channel;
  int rc;
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;

printf("GET CHANNEL OK\n");
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    fprintf(stderr,"error channel open session\n");
    ssh_channel_free(channel);
    return rc;
  }

printf("GET SESSION OK\n");
rc=  interactive_shell_session(channel);
if(rc==SSH_ERROR)
{
  fprintf(stderr,"error interactive shelll\n");
  return rc;
}
printf("GET SHELL OK\n");
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
  if (rc != SSH_OK) 
  {
    fprintf(stderr,"erreur channel request pty\n");
    return rc;
  }
  rc = ssh_channel_change_pty_size(channel, 80, 24);
    if (rc != SSH_OK) 
  {
    fprintf(stderr,"erreur channel change size\n");
    return rc;
  }
  rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK) 
  {
    fprintf(stderr,"erreur channel request shell\n");
    return rc;
  }
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

void handle_session(event,session)
{

    /* Our struct holding information about the session. */
    struct session_data_struct sdata = {
        .channel = NULL,
        .auth_attempts = 0,
        .authenticated = 0
    };
    
     struct ssh_server_callbacks_struct server_cb = {
        .userdata = &sdata,
        .auth_password_function = auth_password,
        .channel_open_request_session_function = channel_open,
    };

       ssh_callbacks_init(&server_cb);

    ssh_set_server_callbacks(session, &server_cb);


       if (ssh_handle_key_exchange(session) != SSH_OK) {
            fprintf(stderr, "%s\n", ssh_get_error(session));
            return;
        }



    ssh_set_auth_methods(session, SSH_AUTH_METHOD_PASSWORD);
    ssh_event_add_session(event, session);

    int n = 0;
    while (sdata.authenticated == 0) {
        /* If the user has used up all attempts, or if he hasn't been able to
         * authenticate in 10 seconds (n * 100ms), disconnect. */
        if (sdata.auth_attempts >= 3 || n >= 100) {
            return;
        }

        if (ssh_event_dopoll(event, 100) == SSH_ERROR) {
            fprintf(stderr, "%s\n", ssh_get_error(session));
            return;
        }
        n++;
      }

}



int main(int argc, char **argv) {
    ssh_bind sshbind;
    ssh_session session;
         struct sigaction sa;
         ssh_event event;
    char buffer[1024];

    int continuer=1;
    char choix;
    int rc;
    ssh_init();
    sshbind = ssh_bind_new();

#ifdef HAVE_ARGP_H
    argp_parse(&argp, argc, argv, 0, 0, sshbind);
#else
    (void) argc;
    (void) argv;

    set_default_keys(sshbind, 0, 0, 0);
#endif /* HAVE_ARGP_H */

    printf("listen....\n");

    if(ssh_bind_listen(sshbind) < 0) {
        fprintf(stderr, "%s\n", ssh_get_error(sshbind));
        return 1;
    }   

    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Failed to allocate session\n");
    }



    /* Blocks until there is a new incoming connection. */
    if(ssh_bind_accept(sshbind, session) != SSH_ERROR) {
        printf("Connection accepted\n");
        switch(fork()) {
            case 0:
              /* Remove the SIGCHLD handler inherited from parent. */
              sa.sa_handler = SIG_DFL;
              sigaction(SIGCHLD, &sa, NULL);
              /* Remove socket binding, which allows us to restart the
               * parent process, without terminating existing sessions. */
              ssh_bind_free(sshbind);
             event = ssh_event_new();
              if(event != NULL)
              {
                printf("NEW EVENT\n");
                handle_session(event,session);
                printf("HANDLE SESSION DONE\n");
                ssh_event_free(event);
              }      
               shell_session(session);
          
            default:
              wait(NULL);
        }
    } else {
        fprintf(stderr, "%s\n", ssh_get_error(sshbind));
    }
    /* Since the session has been passed to a child fork, do some cleaning
     * up at the parent process. */
    ssh_disconnect(session);
    ssh_free(session);

    ssh_bind_free(sshbind);
    ssh_finalize();
    return 0;
}

