#ifndef NETWORK_H
#define NETWORK_H

#define BUFFERSIZE 256

#define ERROR -1
#define SUCCESS 0

int connect_to_server( const char* const );
int connect_from_client( void );

#endif
