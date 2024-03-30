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
#include "../include/encode.h"
#include "../include/decode.h"
#include "../include/packet.h"
#include "../include/tree.h"

#define MAX 180
#define SA struct sockaddr

// Función para recibir y procesar un paquete MQTT
void handle_mqtt_packet(char *buffer) {
    MQTT_Packet received_packet;
    memcpy(&received_packet, buffer, sizeof(MQTT_Packet));

    // Acceder al tópico y al mensaje
    printf("Topic: %s\n", received_packet.variable_header);
    printf("Message: %s\n", received_packet.payload);
}

// Function designed for chat between client and server
void *func(void *arg) { 
    int connfd = *((int*)arg);
    
    // Receive packet from client
    MQTT_Packet received_packet = receive_packet_from_client(connfd);

    printf("Variable Header: ");
    for (int i = 0; i < received_packet.remaining_length; i++) {
        printf("%02X ", received_packet.variable_header[i]);
    }
    printf("\n");

    printf("Payload: ");
    for (int i = 0; i < received_packet.remaining_length; i++) {
        printf("%02X ", received_packet.payload[i]);
    }
    printf("\n");

    // Concatenate variable header and payload into a single buffer
    size_t total_length = received_packet.remaining_length + strlen(received_packet.payload);
    unsigned char *serialized_packet = malloc(total_length);
    memcpy(serialized_packet, received_packet.variable_header, received_packet.remaining_length);
    memcpy(serialized_packet + received_packet.remaining_length, received_packet.payload, strlen(received_packet.payload));

    // Print serialized packet
    printf("Serialized packet: ");
    for (int i = 0; i < total_length; ++i) {
        printf("%02X ", serialized_packet[i]);
    }
    printf("\n");

    // Decode topic encoded in UTF-8
    unsigned int topic_length = (received_packet.variable_header[0] << 8) | received_packet.variable_header[1];
    char *decoded_topic = decodeUTF8(serialized_packet + 2); // Ignore variable header
    printf("Decoded topic: %s\n", decoded_topic);

    // Decode message encoded in UTF-8 (ignoring variable header)
    char *decoded_message = decodeUTF8(serialized_packet + received_packet.remaining_length); // Ignore variable header
    printf("Decoded message: %s\n", decoded_message);

    // Concatenate topic and message using strcat
    char *topic_message = malloc(strlen(decoded_topic) + strlen(decoded_message) + 2); // +2 for space and null terminator
    strcpy(topic_message, decoded_topic);
    strcat(topic_message, " ");
    strcat(topic_message, decoded_message);

    // Free memory
    free(received_packet.variable_header);
    free(received_packet.payload);

    // Close connection with this client
    close(connfd);
    return NULL;
}

// Main function 
int main(int argc, char *argv[]) { 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
    char ip[MAX], log_path[MAX];
    int port;

    if (argc == 4) {
		strcpy(ip, argv[1]);
		port = atoi(argv[2]);
		strcpy(log_path, argv[3]);
	}
	else
		return 1;

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
        printf("Thread ID: %lu\n", (unsigned long)tid);
        pthread_create(&tid, NULL, func, &connfd); // Create a thread to handle this connection
    }

	close(sockfd); // Cerrar el socket principal
	return 0;
}
