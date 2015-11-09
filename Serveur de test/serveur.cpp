#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#define PORT 4444



using namespace std;

int main(int argc,char *argv[])
{
    int sockRVfd,sockCLfd;
    string buffer,nameH;
    struct sockaddr_in addressRV;
    socklen_t addressRVlen;
    struct sockaddr_in addressClient;
    socklen_t addressClientlen;

    // Create the rendez vous socket
    if((sockRVfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return -1;
    }

    if(gethostname(const_cast<char*>(nameH.c_str()),256))
    {
        perror("gethostname");
        return -1;
    }
    cout<<"Je m'execute sur "<<nameH<<"."<<endl;

    // Prepare the local address
    addressRV.sin_family = AF_INET;
    addressRV.sin_port = PORT;
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

    if(read(sockCLfd,const_cast<char*>(buffer.c_str()),256)<0)
    {
        perror("read");
        return -1;
    }

    int val=0;
    cout<<"Le client s'appelle "<<buffer<<endl;
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
        if(write(sockCLfd,buffer.c_str()  ,val)!=val)
        {
            perror("write");
            return -1;
        }
        cout<<"Command sent...\nwaiting for response..."<<endl;
    
        if(read(sockCLfd,const_cast<char*>(buffer.c_str()),256)<0)
        {
            perror("read");
            return -1;
        }
        cout<<"Reponse:\n"<<buffer<<endl;

    }
    close(sockCLfd);
    cout<<"Server stopped"<<endl;

}
