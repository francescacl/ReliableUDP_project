#include "client.h"


void error(const char *msg) {
  perror(msg);
  exit(1);
}


size_t file_size(char *filename) {
/* Ritorna la grandezza del file in byte */
  FILE *file = fopen(filename, "rb");
  size_t fsize = -1;

  if (file != NULL) {
    if (fseek(file, 0, SEEK_END) == 0) { // Sposta l'indicatore di posizione alla fine del file
      fsize = ftell(file); // La posizione corrente corrisponde alla dimensione del file
    }
  fclose(file);
  }

  return fsize;
}


char* file_path(char *fpath, char *fname) {
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


void create_conn(char *ip_address, uint16_t port) {

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
    error("Error in socket");
  }
  
  memset(&servaddr, 0, sizeof(servaddr));      /* azzera servaddr */
  servaddr.sin_family = AF_INET;       /* assegna il tipo di indirizzo */
  servaddr.sin_port = port;  /* assegna la porta del server */
  /* assegna l'indirizzo del server prendendolo dalla riga di comando. L'indirizzo è una stringa da convertire in intero secondo network byte order. */
  if (inet_pton(AF_INET, ip_address, &servaddr.sin_addr) <= 0) { /* inet_pton (p=presentation) vale anche per indirizzi IPv6 */
    fprintf(stderr, "Error in inet_pton for %s", ip_address);
    exit(1);
  }

  return;
}


int send_rel(int sock_fd, struct sockaddr_in addr) {

  /* Invia al server il pacchetto di richiesta*/
  if (sendto(sock_fd, NULL, 0, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    error("Error in sendto");
  }
  return 0;

}


int recv_rel() {
  return 0;
}


int main(int argc, char *argv[]) {

  check_args(argc);

  ////////// New connection //////////

  create_conn(argv[1], htons(SERV_PORT));
  printf("Connection with main established\n");
  fflush(stdout);

  send_rel(sockfd, servaddr);
  printf("Request sent\n");
  fflush(stdout);

  // receive an unsigned int from socket
  int received;
  unsigned int new_port;
  received = recvfrom(sockfd, &new_port, sizeof(unsigned int), 0, NULL, NULL);
  if (received < 0) {
    error("Error in recvfrom");
  }
  printf("Received: %d\n", new_port);
  fflush(stdout);

  servaddr.sin_port = new_port;
  printf("Connection with new port established\n");
  fflush(stdout);

  ////////////////////////////////////
  //client_actions();
  ///////// Welcome message //////////
  
  char welcome[MAXLINE];
  socklen_t servaddr_len = sizeof(servaddr);
  n = recvfrom(sockfd, welcome, MAXLINE, 0, (struct sockaddr *)&servaddr, &servaddr_len);
  if (n < 0) {
    error("Error in recvfrom");
  }
  welcome[n] = '\0';
  
  char w[12*sizeof(char)];
  strncpy(w, welcome, 11);
  w[12] = '\0';
  
  char choice_action[n-11];
  strncpy(choice_action, welcome + 11, n-11);
  choice_action[n-11] = '\0';
  printf("%s", w);
  fflush(stdout);
  printf("%s\n", choice_action);
  fflush(stdout);

  while(1) {
    __fpurge(stdin);
    buff_in = getchar();
    if (buff_in != '1' && buff_in != '2') {
      printf("Il numero inserito non è valido. Riprovare:\n");
      fflush(stdout);
      continue;
    }
    break;
  }
  if (sendto(sockfd, &buff_in, sizeof(buff_in), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      error("Error sending welcome message");
  }

  // Ricezione del nome del file dal server
  // Ricezione dei dati dal server
  int size_rcv;
  
  if (recvfrom(sockfd, &size_rcv, sizeof(int), 0, (struct sockaddr *)&servaddr, &servaddr_len) < 0) {
    error("Error in recvfrom");
  }
  size_t size_next = ntohl(size_rcv);
  char recvline[size_next + 1];
  n = wait_recv(recvline, size_next, sockfd);

  if (n < 0) {
      error("Error in recvfrom");
  } else {
    recvline[n] = '\0';        // aggiunge il carattere di terminazione
   
    char *name;
    switch (buff_in) {
      case '1':
        name = "Oscar.jpg";
        break;
      case '2':
        name = "Poe.txt";
        break;
      default:
        error("Error in switch-case");
    }

    char *path = file_path(DEST_PATH, name);
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