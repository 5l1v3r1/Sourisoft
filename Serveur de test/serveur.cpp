#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

#define PORT 4444



using namespace std;

int main(int argc,char *argv[])
{
    int sockRVfd,sockCLfd;
    string buffer;
    struct sockaddr_in addressRV;
    socklen_t addressRVlen;
    struct sockaddr_in addressClient;
    socklen_t addressClientlen;
    char buf[256];
   char name[256];
    // Create the rendez vous socket
    if((sockRVfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return -1;
    }

    if(gethostname(name,256))
    {
        perror("gethostname");
        return -1;
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
        return -1;
    }

    // start listening
    if(listen(sockRVfd,10)==-1)
    {
        perror("listen");
        return -1;
    }

    // Waiting for client
    cout<<"Attente d'un client..."<<endl;
    addressClientlen = sizeof(addressClient);
    sockCLfd = accept(sockRVfd,(struct sockaddr*)&addressClient,&addressClientlen);
    if(sockCLfd == -1)
    {
        perror("accept");
        return -1;
    }

    // A client just arrives he is connected with sockCLfd
    cout<<"Un client vient de se connecter."<<endl;
    // This is a server for test so it can treat only one client
    close(sockRVfd);

    if(read(sockCLfd,buf,256)<0)
    {
        perror("read");
        return -1;
    }

    int val=0;
    cout<<"Le client s'appelle "<<buf<<endl;
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
        if(write(sockCLfd,buffer.c_str(),val)!=val)
        {
            perror("write");
            return -1;
        }
        cout<<"Command sent...\nwaiting for response..."<<endl;
    
        if(read(sockCLfd,buf,256)<0)
        {
            perror("read");
            return -1;
        }
        cout<<"Reponse:\n"<<buf<<endl;

    }
    close(sockCLfd);
    cout<<"Server stopped"<<endl;

}
