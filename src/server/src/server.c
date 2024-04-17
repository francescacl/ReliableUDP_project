#include "server.h"


void error(const char *msg) {
  perror(msg);
  exit(1);
}


size_t file_size(char *filename) {
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


char* file_path(char *fpath, char *fname) {
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


void send_rel(int fd, struct sockaddr_in send_addr) {

  // inserire dimensione nel buffer
  int size = htonl(size_file);
  
  if (sendto(fd, &size, sizeof(int), 0, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
    error("error in sendto");

  while ((bytes_read = fread(buff, 1, MAXLINE, file)) > 0) {
    if (sendto(fd, buff, bytes_read, 0, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
      error("error in sendto");
  }
}


void create_conn() {

  len = sizeof(struct sockaddr_in);

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


void* handle_user(void* arg) {
  // parse arg
  struct sockaddr_in client_addr = *((struct sockaddr_in *) arg);

  // get thread id
  pthread_t tid = pthread_self();

  // crea una nuova socket, ne fa la bind a un nuovo numero di porta e lo manda al client
	int new_socket;
	struct sockaddr_in new_addr;

	if ((new_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		error("Error socket");
	}

  // trova il nuovo numero di porta
	int i = 0;
  unsigned int new_port;
	while (1) {
		new_port = SERV_PORT + i;
		
		// bind della nuova socket col nuovo numero di porta
		memset((void *)&new_addr, 0, sizeof(new_addr));
		new_addr.sin_family = AF_INET;
		new_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		new_addr.sin_port = htons(new_port);
		if (bind(new_socket, (struct sockaddr *)&new_addr, sizeof(new_addr)) < 0) {
			if (errno == EADDRINUSE) {
				i++; // riprova incrementando i
			}
			else if (errno != EINTR) {
				error("Error bind");
			}
		}
		else {
			break;
		}
	}

  // stampa il nuovo numero di porta
  printf("[%lu] New port number: %d\n", tid, new_port);
  fflush(stdout);

  // invio della nuova porta al client
  if (sendto(sockfd, &new_port, sizeof(new_port), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    error("Error in sendto");
  }
  printf("[%lu] Port number sent\n", tid);
  fflush(stdout);

  // richiesta di connessione ricevuta, invio stringa con comandi
  char welcome_msg[] = "Benvenuto! Scegliere una delle seguenti opzioni:\n\t1. Oscar.jpg,\n\t2. Poe.txt.\nDigitare il numero corrispondente:";
  if (sendto(new_socket, welcome_msg, sizeof(welcome_msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    error("Error sending welcome message");
  }
  printf("[%lu] Welcome message sent\n", tid);
  char option[2];
  int k = recvfrom(new_socket, option, sizeof(*option), 0, NULL, NULL);
  if (k < 0) {
    error("Error in recvfrom");
  }
  option[1] = '\0';
  int optionInt = atoi(option);
  char *name;
  switch (optionInt) {
    case 1:
      name = "Oscar.jpg";
      break;
    case 2:
      name = "Poe.txt";
      break;
    default:
      error("Error in switch-case");
  }
  printf("[%lu] Caso %d: %s\n", tid, optionInt, name);
  fflush(stdout);
  
  printf("[%lu] Sending file\n", tid);
  fflush(stdout);
  char *FILENAME = file_path(FILENAME_PATH, name);
  file = fopen(FILENAME, "rb");
  if (file == NULL) {
    error("Error in file opening");
  }
  size_file = file_size(FILENAME);
  send_rel(new_socket, client_addr);

  fclose(file);
  printf("[%lu] Sending complete\n", tid);
  fflush(stdout);

  // Termina correttamente il thread
  printf("Thread terminated\n\n");
  fflush(stdout);
  pthread_exit(NULL);

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
  
    // copia l'indirizzo per passarlo al thread
    struct sockaddr_in client_addr = addr;

    // Avvio di un nuovo thread per gestire la connessione
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_user, &client_addr) != 0) {
      error("Error creating thread");
    }
    pthread_detach(thread_id);
  }
  
  exit(0);
}