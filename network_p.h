#ifndef NETWORK_P_H
#define NETWORK_P_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.h"

#define SEND 0
#define RECV 1

#define PORT 15246

void sent_to_client( int );
void recv_from_server( int );

int receipt_confirmation( int, int );

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

#endif
