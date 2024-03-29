#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>

#define MAX 80 
#define SA struct sockaddr 

// Function designed for chat between client and server. 
void *func(void *arg) { 
    int connfd = *((int*)arg);
	char buff[MAX]; 
	int n; 
	// Infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// Read the message from client and copy it to buffer 
		read(connfd, buff, sizeof(buff)); 
		// Print buffer which contains the client contents 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// Copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(connfd, buff, sizeof(buff)); 

		// If message contains "Exit" then server exit and chat ended. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	} 
	close(connfd); // Close the connection with this client
	return NULL;
} 

int main(int argc, char *argv[]) { 
	int sockfd, connfd, len; 
	char ip[MAX], log_path[MAX];
    int port;
	struct sockaddr_in servaddr, cli;

	if (argc == 4) {
		strcpy(ip, argv[1]);
		port = atoi(argv[2]);
		strcpy(log_path, argv[3]);
	}
	else
		return 1;

	printf("IP: %s\n", ip);
	printf("PORT %d\n", port);
	printf("PATH: %s\n", log_path);

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// Assign IP and PORT
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(ip); 
	servaddr.sin_port = htons(port); 

	// Bind socket to IP and PORT
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Listen for incoming connections
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 

	len = sizeof(cli); 

	// Loop to accept incoming connections
	while (1) {
		// Accept the data packet from client
		connfd = accept(sockfd, (SA*)&cli, &len); 
		if (connfd < 0) { 
			printf("server accept failed...\n"); 
			exit(0); 
		} 
		else
			printf("server accept the client...\n"); 
		
		pthread_t tid; // Thread identifier
		pthread_create(&tid, NULL, func, &connfd); // Create a thread to handle this connection
	}

	close(sockfd); // Close the main socket
	return 0;
}