#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"



int main( int argc, char** argv ) {

    if ( argc != 2 ) {
        fprintf( stderr, "Usege: %s [IP]\n",argv[0]);
        exit(1);
    }

    if ( strlen(argv[1]) + 1 >= BUFFERSIZE ) {
        fprintf( stderr, "error: IPaddress length\n");
        exit(1);
    }
    while(1)
    {
        connect_to_server( argv[1] );   
        sleep(30);
    }

    return 0;
}


