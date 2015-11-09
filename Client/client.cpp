/**
*     Sourisoft's client
*
*     @author Kevin Hascoët
*
      gcc client.cpp -o client -Wall
*
*/
	
// if we're on windows
#if defined (WIN32)
	#include <windows.h>
 
    #include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
   typedef int socklen_t;

// else, if we're on Linux
#elif defined (linux)

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>

#endif

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define PORT 4444
#define BUFSIZE 256
#define IP "127.0.0.1"

using namespace std;

int main(int argc, char const *argv[])
{

    // if we're on windows
    #if defined (WIN32)
        WSADATA WSAData;
        WSAStartup(MAKEWORD(2,2), &WSAData);
    #endif


	int sock,len,val;
	char buffer[BUFSIZE];
	struct sockaddr_in server_address;
	bool isConnected = true;
	while(true)
	{
	// Create the socket 
	if((sock= socket(AF_INET,SOCK_STREAM,0))== -1)
	{
		perror("socket");
		exit (EXIT_FAILURE);
	}

	// configuration of the connection
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr= inet_addr(IP);
	
		cout<<"Try to connect..."<<endl;
		// Try to connect 
		while(connect(sock, (struct sockaddr*)&server_address,sizeof(server_address)) == -1)
		{
			perror("connect");
			// if we're on windows
			#if defined (WIN32)
				Sleep(20000);
			// else, if we're on Linux
			#elif defined (linux)
				sleep(2);
			#endif
		}
		cout<<"Connecté au serveur !"<<endl;
		isConnected=true;
	    while(isConnected)
	    {
	    	#if defined (WIN32)
				Sleep(500);
			// else, if we're on Linux
			#elif defined (linux)
				usleep(500000);
			#endif
	    	if((val=recv(sock,buffer,BUFSIZE,0))<0)
	        {
	        	cout<<"RIEN RECU"<<endl;
	            perror("recv");
	        }
	        buffer[val]='\0';
	        cout<<"recu :"<<buffer<<endl;
	        if(val>0)
	        {
		        string result = "I'M ROOT";
		        len = result.size();
		        if(send(sock,result.c_str(),len,0)!=len)
		        {
		            perror("send");
		        }
	    	}else if(val<=0)
	    	{
	    		close(sock);
	    		isConnected=false;
	    	}
	    }
	}
	return 0;
}