#ifndef _SIMPLE_SERVER_H
#define _SIMPLE_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int open_listenfd(int);
#endif
