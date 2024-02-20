#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERV_PORT	5193
#define BACKLOG		10
#define MAXLINE		1024
#define DEST_PATH "files/output/"

void error(const char *msg);
size_t fileSize(char *filename);
char* filePath(char *fpath, char *fname);
int wait_recv(char *buff, long size, int sockfd);

#endif // CLIENT_H