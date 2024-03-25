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

#define SERV_PORT	   5193
#define BACKLOG		   10
#define MAXLINE		   2048
#define FILENAME_PATH "files/input/"

// Functions
void error(const char *msg);
size_t fileSize(char *filename);
char* filePath(char *fpath, char *fname);
int recv_rel();
int send_rel();
void create_conn();

// Variables
int sockfd;
socklen_t len;
struct sockaddr_in addr;
char buff[MAXLINE];
FILE *file;
size_t bytes_read;

#endif // SERVER_H