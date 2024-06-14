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

#include <pthread.h>

#include <time.h>
#include <stdbool.h>

#define SERV_PORT	   5193
#define BACKLOG		   10
#define MAXLINE		   2048
#define FILENAME_PATH "files/server/"

// Funzioni
void error(const char *msg);
size_t fileSize(char *filename);
char* filePath(char *fpath, char *fname);
int recv_rel(int sock, char *buffer, size_t dim, bool size_rcv, struct sockaddr_in *address, socklen_t *addr_length);
void send_rel(int fd, struct sockaddr_in send_addr, bool send_size, bool is_file, char *data);
void create_conn();

// Variabili
int sockfd;
socklen_t len;
struct sockaddr_in addr;
char buff[MAXLINE];
FILE *file;
size_t bytes_read;
size_t size_file;
FILE *file_to_save;

#endif // SERVER_H