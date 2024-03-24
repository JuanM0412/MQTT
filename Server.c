#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include <pthread.h> // For using threads

#define MAX 80 
#define CONFIG_FILE "config.txt" // Configuration file name
#define SA struct sockaddr 

// Function designed for the chat between client and server
void *func(void *arg) { 
    int connfd = *((int*)arg);
    char buff[MAX]; 
    int n; 
    // Infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 

        // Read the message from client and copy it to buffer 
        read(connfd, buff, sizeof(buff)); 

        // Check if the received packet is a CONNECT packet
        if (buff[0] == 0x10) {
            printf("Received CONNECT packet from client\n");
            // Here you can add logic to handle the CONNECT packet, if needed
        }

        // Print the buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        // Copy server message to the buffer 
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
    close(connfd); // Close this connection with client
    return NULL;
} 

// Main function 
int main() { 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 

    // Read IP and port from the configuration file
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char ip[MAX];
    int port;
    if (fscanf(config_file, "%s%d", ip, &port) != 2) {
        perror("Error reading config file");
        exit(EXIT_FAILURE);
    }
    fclose(config_file);

    // Create the socket
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

    // Bind the socket with the server address 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 

    // Listen to connections
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 

    len = sizeof(cli); 

    // Infinite loop to accept incoming connections
    while (1) {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("server accept failed...\n"); 
            exit(0); 
        } 
        else
            printf("server accept the client...\n"); 
        
        pthread_t tid; // Thread ID
        pthread_create(&tid, NULL, func, &connfd); // Create a thread to handle this connection
    }

    close(sockfd); // Close the main socket
    return 0;
}
