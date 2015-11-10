/**
*     Sourisoft's client header
*
*     @author Kevin Hascoët
*
    
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

std::string getResponse(std::string buffer,int sock);
bool my_popen (const std::string& cmd,std::string &out );