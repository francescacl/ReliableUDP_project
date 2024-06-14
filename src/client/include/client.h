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
#include <stdbool.h>

#define SERV_PORT	5193
#define BACKLOG		10
#define MAXLINE		1024
#define DEST_PATH   "files/client/"
#define MAXVECT 512

// Funzioni
void error(const char *msg);
size_t fileSize(char *filename);
char* filePath(char *fpath, char *fname);
int wait_recv(char *buff, long size, int sockfd);
void check_args(int argc);
void create_conn(char *ip_address, uint16_t port);
int send_rel(int sock_fd, struct sockaddr_in addr, char* str, bool is_file);
int recv_rel(int sock, char *buffer, size_t dim, bool size_rcv, struct sockaddr_in *address, socklen_t *addr_length); 

// Variabili
int sockfd, n;
struct sockaddr_in servaddr;
FILE *file;
char buff_in[2];
int buff_in_int;
char list_str[MAXLINE];
FILE *file_to_save;
size_t size_file_to_save;
char buff[MAXLINE];

#endif // CLIENT_H