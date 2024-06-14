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



int wait_recv(char *buff, long size, int sockfd) { 
  int totalReceived = 0;
  if (sockfd > 0) {
    int received = 0;
    while(size > 0) {
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

    return totalReceived;
  }
  return -1;
}



int recv_rel(int sock, char *buffer, size_t dim, bool size_rcv, struct sockaddr_in *address, socklen_t *addr_length) {
  int k;
  if(!size_rcv) {
    k = recvfrom(sock, buffer, dim, 0, (struct sockaddr *)address, addr_length);
    if (k < 0) {
      error("Error in recvfrom");
    }
  } else {
    k = wait_recv(buffer, dim, sock);
  }
  return k;
}



int bytes_read_funct(char **data, bool is_file) {
  int bytes_read;
  if (is_file) {
    bytes_read = fread(buff, 1, MAXLINE, file);
  } else {
    bytes_read = strlen(*data);
    if (bytes_read > MAXLINE) {
      bytes_read = MAXLINE;
    }
    memcpy(buff, *data, bytes_read);
    *data += bytes_read;
  }
  return bytes_read;
}



void send_rel(int fd, struct sockaddr_in send_addr, bool send_size, bool is_file, char *data) {
  
  int bytes_read;

  if (send_size) {
    uint32_t size; 
    if(is_file) {
      size = htonl(size_file);
    } else {
      size = htonl(bytes_read);
    }
    char size_str[11];
    size_str[0] = '\0';
    snprintf(size_str, sizeof(size_str), "%u", size);
    if (sendto(fd, size_str, sizeof(size_str), 0, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
      error("Error in sendto");
  }
  
  while ((bytes_read = bytes_read_funct(&data, is_file)) > 0) {
    if (sendto(fd, buff, bytes_read, 0, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
      error("Error in sendto");
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
    error("Error in bind");
  }
  printf("Socket binded\n");
  fflush(stdout);
}



void ls(char *list_command){

  sprintf(list_command, "ls %s", FILENAME_PATH);
     
  FILE *pipe = popen(list_command, "r");
  if (pipe == NULL) {
    error("Error in opening the pipe");
  }
  // Leggi l'output del comando e salvalo in una stringa
  size_t bytes_read = fread(list_command, 1, MAXLINE, pipe);
  if (bytes_read == 0) {
    error("Error in reading the output of the command");
  }
  list_command[bytes_read] = '\0';

  // Chiudi la pipe
  pclose(pipe);

}



void* handle_user(void* arg) {
  // parsa arg
  struct sockaddr_in client_addr = *((struct sockaddr_in *) arg);

  // ottiene il thread id
  pthread_t tid = pthread_self();

  // crea una nuova socket, ne fa la bind a un nuovo numero di porta e lo manda al client
	int new_socket;
	struct sockaddr_in new_addr;

	if ((new_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		error("Error socket");
	}

  // trova il nuovo numero di porta
	int i = 0;
  uint32_t new_port;
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
  printf("[%lu] New port number: %u\n", tid, new_port);
  fflush(stdout);

  // invio della nuova porta al client
  char new_port_str[11];
  new_port_str[0] = '\0';
  snprintf(new_port_str, sizeof(new_port_str), "%u", new_port);
  send_rel(sockfd, client_addr, false, false, new_port_str);

  // richiesta di connessione ricevuta, invio stringa con comandi
  char welcome_msg[] = "Welcome! Choose one of the following options:\n\t1. LIST all avaiable files\n\t2. GET a file\n\t3. PUT a file in the server\n\t4. Quit.\nType the corresponding number:";
  send_rel(new_socket, client_addr, false, false, welcome_msg);
  printf("[%lu] Welcome message sent\n", tid);
  
  while (1) {
    char option[2];
    recv_rel(new_socket, option, sizeof(option), false, NULL, NULL);
    int option_int = atoi(option);

    if (option_int == 1) { // LIST
      char list_command[MAXLINE];
      list_command[0] = '\0';
      ls(list_command);
      send_rel(new_socket, client_addr, false, false, list_command);
    }

    else if (option_int == 2) { // GET
      // list
      char list_command[MAXLINE];
      list_command[0] = '\0';
      ls(list_command);
      send_rel(new_socket, client_addr, false, false, list_command);
      // nome del file
      char name[MAXLINE];
      int lent = sizeof(client_addr);
      int j = recv_rel(new_socket, name, MAXLINE, false, NULL, NULL);
      name[j] = '\0';
      // invio della dimensione del file e del file
      printf("[%lu] Sending file\n", tid);
      fflush(stdout);
      char *FILENAME = file_path(FILENAME_PATH, name);
      file = fopen(FILENAME, "rb");
      if (file == NULL) {
        error("Error in file opening");
      }
      size_file = file_size(FILENAME);
      send_rel(new_socket, client_addr, true, true, NULL);
      fclose(file);
      printf("[%lu] Sending complete\n", tid);
      fflush(stdout);
    }
    
    else if (option_int == 3) { // PUT
      // ricezione del nome del file da salvare
      char name_to_save[MAXLINE];
      int k = recv_rel(new_socket, name_to_save, MAXLINE, false, NULL, NULL);
      name_to_save[k] = '\0';
      // ricezione della grandezza del file da salvare
      int size_rcv;
      char size_rcv_str[11];
      size_rcv_str[0] = '\0';
      recv_rel(new_socket, size_rcv_str, sizeof(size_rcv_str), false, NULL, NULL);
      size_rcv = strtol(size_rcv_str, NULL, 10);
      size_t size_next = ntohl(size_rcv);
      char recvline[size_next + 1];
      // ricezione del file
      int n = recv_rel(new_socket, recvline, size_next, true, NULL, NULL);
      recvline[n] = '\0';        // aggiunge il carattere di terminazione
      char *path_to_save = file_path(FILENAME_PATH, name_to_save);
      FILE *file_to_save = fopen(path_to_save, "wb"); // Apertura del file in modalità binaria
      if (file_to_save == NULL) {
        error("Error in destination file opening");
      }
      // Scrittura dei dati nel file
      if (fwrite(recvline, 1, n, file_to_save) != n) {
        error("Error in file writing");
      }
      fclose(file_to_save); // Chiusura del file
      printf("File saved successfully in: %s\n\n", path_to_save);
      fflush(stdout);

    }
    
    else if (option_int == 4) break;

  }
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

    recv_rel(sockfd, buff, MAXLINE, false, &addr, &len); 
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