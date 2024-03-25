#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Packets/packet.c"

#define MAX 80
#define CONFIG_FILE "config.txt" // Configuration file name
#define SA struct sockaddr

// Function to send a CONNECT packet to the server
void send_connect_packet(int sockfd) {
    MQTT_Packet connect_packet = create_connect_packet(0, "client");
    
    write(sockfd, &connect_packet, sizeof(connect_packet));

    free_packet(&connect_packet);
}

void send_publish_packet(int sockfd) {
    MQTT_Packet publish_packet = create_publish_packet("test/01", "sending");
    
    // Serializar el paquete antes de enviarlo
    char buffer[sizeof(MQTT_Packet)];
    memcpy(buffer, &publish_packet, sizeof(MQTT_Packet));

    // Enviar el paquete serializado
    write(sockfd, buffer, sizeof(buffer));

    free_packet(&publish_packet);
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr;
    char ip[MAX];
    int port;
    FILE *config_file;

    // Read IP and port from the configuration file
    config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    if (fscanf(config_file, "%s%d", ip, &port) != 2) {
        perror("Error reading config file");
        exit(EXIT_FAILURE);
    }
    fclose(config_file);

    // Socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // Connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    // Function for chat
    send_publish_packet(sockfd);

    // Close the socket
    close(sockfd);

    return 0;
}
