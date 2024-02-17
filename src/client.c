#include "client.h"


void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[ ]) {

  fflush(stdout);
  int   sockfd, n;
  char  recvline[MAXLINE + 1];
  struct    sockaddr_in   servaddr;

  if (argc != 2) { /* controlla numero degli argomenti */
    fprintf(stderr, "utilizzo: daytime_clientUDP <indirizzo IP server>\n");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
    error("errore in socket");
  }

  
  //memset((void *)&servaddr, 0, sizeof(servaddr));      /* azzera servaddr */
  memset(&servaddr, 0, sizeof(servaddr));      /* azzera servaddr */
  servaddr.sin_family = AF_INET;       /* assegna il tipo di indirizzo */
  servaddr.sin_port = htons(SERV_PORT);  /* assegna la porta del server */
  /* assegna l'indirizzo del server prendendolo dalla riga di comando. L'indirizzo è una stringa da convertire in intero secondo network byte order. */
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
                /* inet_pton (p=presentation) vale anche per indirizzi IPv6 */
    fprintf(stderr, "errore in inet_pton per %s", argv[1]);
    exit(1);
  }

  /* Invia al server il pacchetto di richiesta*/
  if (sendto(sockfd, NULL, 0, 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    error("errore in sendto");
  }

  /* Legge dal socket il pacchetto di risposta */
  n = recvfrom(sockfd, recvline, MAXLINE, 0 , NULL, NULL);
  if (n < 0) {
    error("errore in recvfrom");
  }
  if (n > 0) {
    recvline[n] = 0;        /* aggiunge il carattere di terminazione */
    if (fputs(recvline, stdout) == EOF)   {  /* stampa recvline sullo stdout */
      fprintf(stderr, "errore in fputs");
      exit(1);
    }
    fflush(stdout);
  }
  exit(0);
}