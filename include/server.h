#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <time.h>

#define SERV_PORT	5193
#define BACKLOG		10
#define MAXLINE		1024

void error(const char *msg);

#endif // SERVER_H