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
#include <arpa/inet.h>
#include "encode/Encode.c"
#include "encode/Decode.c"
#include "Packets/packet.c"
#include "tree/Tree.c"

#define MAX 80 
#define CONFIG_FILE "config.txt" // Configuration file name
#define SA struct sockaddr 

// Función para recibir y procesar un paquete MQTT
void handle_mqtt_packet(char *buffer) {
    MQTT_Packet received_packet;
    memcpy(&received_packet, buffer, sizeof(MQTT_Packet));

    // Acceder al tópico y al mensaje
    printf("Topic: %s\n", received_packet.variable_header);
    printf("Message: %s\n", received_packet.payload);
}

// Función para el hilo de cliente
void *func(void *arg) {
    int connfd = *((int*)arg);
    char buff[sizeof(MQTT_Packet)]; 
    int n; 

    for (;;) {
        bzero(buff, sizeof(buff));
        read(connfd, buff, sizeof(buff));

        // Procesar el paquete MQTT recibido
        handle_mqtt_packet(buff);

        // Envía una respuesta al cliente si es necesario
        // write(connfd, response, sizeof(response));

        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }

    close(connfd);
    return NULL;
}

// Main function 
int main() { 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 

    // Read IP and port from the configuration file
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL)
    {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char ip[MAX];
    int port;
    if (fscanf(config_file, "%s%d", ip, &port) != 2)
    {
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

	close(sockfd); // Cerrar el socket principal
	return 0;
}