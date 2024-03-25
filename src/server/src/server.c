#include "server.h"


void error(const char *msg) {
  perror(msg);
  exit(1);
}


size_t fileSize(char *filename) {
/* Ritorna la grandezza del file in byte */
  FILE *file = fopen(filename, "rb");
  size_t size = -1;

  if (file != NULL) {
    if (fseek(file, 0, SEEK_END) == 0) { // Sposta l'indicatore di posizione alla fine del file
      size = ftell(file); // La posizione corrente corrisponde alla dimensione del file
    }
  fclose(file);
  }

  return size;
}


char* filePath(char *fpath, char *fname) {
/* Ritorna il path del file concatenato al nome del file */
  size_t len1 = strlen(fpath);
  size_t len2 = strlen(fname);

  char *path = (char *)malloc((len1 + len2 + 1) * sizeof(char));
  strcpy(path, fpath);
  strcat(path, fname);

  return path;
}


int recv_rel() {

  return recvfrom(sockfd, buff, MAXLINE, 0, (struct sockaddr *)&addr, &len);
}


int send_rel() {

  return sendto(sockfd, buff, bytes_read, 0, (struct sockaddr *)&addr, sizeof(addr));
}


void create_conn() {

  len=sizeof(struct sockaddr_in);

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea la socket */
    error("Error in socket");
  }
  printf("Socket created\n");
  fflush(stdout);

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); /* il server accetta pacchetti su una qualunque delle sue interfacce di rete */
  addr.sin_port = htons(SERV_PORT); /* numero di porta del server */

  /* assegna l'indirizzo al socket */
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    error("errore in bind");
  }
  printf("Socket binded\n");
  fflush(stdout);
}


int main(int argc, char **argv) {

  create_conn();

  while (1) {
    
    printf("Waiting for request...\n");
    fflush(stdout);
    
    if (recv_rel() < 0) {
      error("Error in recvfrom");
    }
    printf("Request received\n");
    fflush(stdout);

    //

    printf("Sending image\n");
    fflush(stdout);

    char *name = "Oscar.jpg";
    //char *name = "Poe.txt";
    char *FILENAME = filePath(FILENAME_PATH, name);
    file = fopen(FILENAME, "rb");

    if (file == NULL) {
      error("Error in file opening");
    }

    size_t size = fileSize(FILENAME);

    while ((bytes_read = fread(buff, 1, MAXLINE, file)) > 0) {
      if (send_rel() < 0)
        error("error in sendto");
    }

    fclose(file);

    printf("Sending complete\n\n");
    fflush(stdout);
  
  }

  exit(0);
}
