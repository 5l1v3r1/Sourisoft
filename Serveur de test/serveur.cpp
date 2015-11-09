/**
*     server used for testing the client while developmnent
*     
*
*     @author Kevin Hascoët
*
      gcc serveur.cpp -o serveur -Wall
*
*/

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define PORT 4444

#define BUFSIZE 256


using namespace std;

int main(int argc,char *argv[])
{
    int sockRVfd,sockCLfd;
    string buffer;
    struct sockaddr_in addressRV;
    socklen_t addressRVlen;
    struct sockaddr_in addressClient;
    socklen_t addressClientlen;
    char buf[BUFSIZE];
    char name[BUFSIZE];
    // Create the rendez vous socket
    if((sockRVfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        exit (EXIT_FAILURE);
    }

    if(gethostname(name,BUFSIZE))
    {
        perror("gethostname");
        exit (EXIT_FAILURE);
    }
    cout<<"Je m'execute sur "<<name<<"."<<endl;

    // Prepare the local address
    addressRV.sin_family = AF_INET;
    addressRV.sin_port = htons(PORT);
    addressRV.sin_addr.s_addr = htonl(INADDR_ANY);

    addressRVlen = sizeof(addressRV);

    // bind socket to local address
    if( bind(sockRVfd , (struct sockaddr*)&addressRV,addressRVlen) == -1 )
    {
        perror("bind");
        exit (EXIT_FAILURE);
    }

    // start listening
    if(listen(sockRVfd,10)==-1)
    {
        perror("listen");
        exit (EXIT_FAILURE);
    }

    // Waiting for client
    cout<<"Attente d'un client..."<<endl;
    addressClientlen = sizeof(addressClient);
    sockCLfd = accept(sockRVfd,(struct sockaddr*)&addressClient,&addressClientlen);
    if(sockCLfd == -1)
    {
        perror("accept");
        exit (EXIT_FAILURE);
    }
   struct sockaddr_storage addr;
   socklen_t len;
   char ipstr[INET6_ADDRSTRLEN];

   len = sizeof addr;
   getpeername(sockCLfd, (struct sockaddr*)&addr, &len);

   // deal with both IPv4 and IPv6:
   if (addr.ss_family == AF_INET) {
       struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    
       inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
   } else { // AF_INET6
       struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
       inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
   }


    // A client just arrives he is connected with sockCLfd0
    cout<<"Un client vient de se connecter."<<endl;
    cout<<"Son IP est : "<<ipstr<<endl;
    // This is a server for test so it can treat only one client
    close(sockRVfd);
/*
    if(read(sockCLfd,buf,BUFSIZE)<0)
    {
        perror("read");
        exit (EXIT_FAILURE);
    }
*/
    int val=0;
  //  cout<<"Le client s'appelle "<<buf<<endl;
    while(true)
    {
        cout<<"Entrez la commande a executer"<<endl;
        cin>>buffer;
        if(buffer=="q")
        {
            break;
        }
        buffer = "sh:"+buffer;
        val = buffer.size();
        if(send(sockCLfd,buffer.c_str(),val,0)!=val)
        {
            perror("write");
            exit (EXIT_FAILURE);
        }
        cout<<"Command sent...\nwaiting for response..."<<endl;
    
        if((val=recv(sockCLfd,buf,BUFSIZE,0))<0)
        {
            perror("read");
            exit (EXIT_FAILURE);
        }
        buf[val]='\0';
        cout<<"Reponse:\n"<<buf<<endl;

    }
    close(sockCLfd);
    cout<<"Server stopped"<<endl;

}
