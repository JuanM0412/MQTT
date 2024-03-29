#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/encode.h"
#include "../include/decode.h"
#include "../include/packet.h"

#define MAX 80
#define CONFIG_FILE "config.txt"
#define SA struct sockaddr

void send_packet_to_server(int sockfd, MQTT_Packet packet) {
    printf("Fixed Header: %u\n", packet.fixed_header);
    printf("Remaining Length: %u\n", packet.remaining_length);
    printf("Variable Header: ");
    for (int i = 0; i < packet.remaining_length; i++) {
        printf("%02X ", packet.variable_header[i]);
    }
    printf("\n");
    
    printf("Payload: ");
    for (int i = 0; i < packet.remaining_length; i++) {
        printf("%02X ", packet.payload[i]);
    }
    printf("\n");
    // Calculate total size of the packet
    size_t total_size = sizeof(packet.fixed_header) + sizeof(packet.remaining_length) +
                        packet.remaining_length + sizeof(packet.payload);

    // Serialize structure data into a byte buffer
    unsigned char buffer[total_size];
    size_t offset = 0;

    // Copy structure fields into buffer
    memcpy(buffer + offset, &packet.fixed_header, sizeof(packet.fixed_header));
    offset += sizeof(packet.fixed_header);

    memcpy(buffer + offset, &packet.remaining_length, sizeof(packet.remaining_length));
    offset += sizeof(packet.remaining_length);

    memcpy(buffer + offset, packet.variable_header, packet.remaining_length);
    offset += packet.remaining_length;

    memcpy(buffer + offset, packet.payload, sizeof(packet.payload));

    // Send the buffer through the socket
    write(sockfd, buffer, total_size);
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // Read IP and port from configuration file
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

    // Socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
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
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    MQTT_Packet publish_packet = create_publish_packet(encodeMessageToUTF8("EAFIT/Sede/Poblado/Bloque/33/Salon/301/humedad"), encodeMessageToUTF8("33%"));
    // Send the packet to the server
    send_packet_to_server(sockfd, publish_packet);

    // Close the socket
    close(sockfd);

    return 0;
}