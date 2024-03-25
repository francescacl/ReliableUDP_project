#include "client.h"


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


int wait_recv(char *buff, long size, int sockfd) { 
  int totalReceived = 0;
  
  if (sockfd > 0) {
    int received = 0;
    do {
      errno = 0;
      if ((received = recvfrom(sockfd, buff + totalReceived, size, 0, NULL, NULL)) < 0) {
        if (errno == EINTR || errno == EAGAIN) {
          continue;
        }
        fprintf(stderr, "Error udp wait_recv: %d\n", errno);
        return -1;
      }
      totalReceived += received;
      size -= received;
    }
    while (size > 0);

    return totalReceived;
  }
  return -1;
}

void check_args(int argc) {
  if (argc != 2) { /* controlla numero degli argomenti */
    printf("argc: %d\n",argc);
    fprintf(stderr, "utilizzo: daytime_clientUDP <indirizzo IP server>\n");
    exit(1);
  }
  return;
}

void create_conn(char *ip_address) {

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
    error("Error in socket");
  }
  
  memset(&servaddr, 0, sizeof(servaddr));      /* azzera servaddr */
  servaddr.sin_family = AF_INET;       /* assegna il tipo di indirizzo */
  servaddr.sin_port = htons(SERV_PORT);  /* assegna la porta del server */
  /* assegna l'indirizzo del server prendendolo dalla riga di comando. L'indirizzo è una stringa da convertire in intero secondo network byte order. */
  if (inet_pton(AF_INET, ip_address, &servaddr.sin_addr) <= 0) { /* inet_pton (p=presentation) vale anche per indirizzi IPv6 */
    fprintf(stderr, "Error in inet_pton for %s", ip_address);
    exit(1);
  }

  return;

}

int send_rel() {

  /* Invia al server il pacchetto di richiesta*/
  if (sendto(sockfd, NULL, 0, 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    error("Error in sendto");
  }
  return 0;

}

int recv_rel() {
  return 0;

}


int main(int argc, char *argv[]) {

  // TODO
  const long size = 174647;
  //const long size = 182;
  char recvline[size + 1];

  check_args(argc);

  create_conn(argv[1]);
  printf("Connection established\n");
  fflush(stdout);

  send_rel();
  printf("Request sent\n");
  fflush(stdout);

  
  /*-------------------*/

  // Ricezione del nome del file dal server
    // Ricezione dei dati dal server
    n = wait_recv(recvline, size, sockfd);

    if (n < 0) {
        error("Error in recvfrom");
    } else {
      recvline[n] = '\0';        // aggiunge il carattere di terminazione

      char *name = "Oscar.jpg";
      //char *name = "Poe.txt";
      char *path = filePath(DEST_PATH, name);
      FILE *file = fopen(path, "wb"); // Apertura del file in modalità binaria
      if (file == NULL) {
        error("Error in destination file opening");
      }
      // Scrittura dei dati nel file
      if (fwrite(recvline, 1, n, file) != n) {
        error("Error in file writing");
      }
      fclose(file); // Chiusura del file
      printf("File saved successfully in: %s\n", path);
    }

    close(sockfd); // Chiusura del socket

  exit(0);
}