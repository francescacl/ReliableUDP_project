#include "server.h"


void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char **argv) {
  int sockfd;
  socklen_t len=sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  char buff[MAXLINE];
  time_t ticks;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
    error("errore in socket");
  }

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); /* il server accetta pacchetti su una qualunque delle sue interfacce di rete */
  addr.sin_port = htons(SERV_PORT); /* numero di porta del server */

  /* assegna l'indirizzo al socket */
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    error("errore in bind");
  }

  while (1) {
    
    if ( (recvfrom(sockfd, buff, MAXLINE, 0, (struct sockaddr *)&addr, &len)) < 0) {
      error("errore in recvfrom");
    }

    ticks = time(NULL); /* legge l'orario usando la chiamata di sistema time */
    /* scrive in buff l'orario nel formato ottenuto da ctime; snprintf impedisce l'overflow del buffer. */
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); /* ctime trasforma la data e lora da binario in ASCII. \r\n per carriage return e line feed*/
    /* scrive sul socket il contenuto di buff */
    if (sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      error("errore in sendto");
    }
  }
  exit(0);
}
