#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERV_PORT	5193
#define BACKLOG		10
#define MAXLINE		1024
#define DEST_PATH   "files/client/"

// Functions
void error(const char *msg);
size_t fileSize(char *filename);
char* filePath(char *fpath, char *fname);
int wait_recv(char *buff, long size, int sockfd);
void check_args(int argc);
void create_conn(char *ip_address, uint16_t port);
int send_rel();
int recv_rel(); 

// Variables
int sockfd, n;
struct sockaddr_in servaddr;
FILE *file;
char buff_in;

#endif // CLIENT_H