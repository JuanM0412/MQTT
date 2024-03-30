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
#define CONFIG_FILE "../config.txt"
#define SA struct sockaddr

void send_packet_connect(int sockfd, MQTT_Packet packet) {
    printf("Fixed Header: %u\n", packet.fixed_header);
    printf("Remaining Length: %u\n", packet.remaining_length);
    
    // Calcular el tamaño total del paquete
    size_t total_size = 1 + sizeof(packet.remaining_length) +
                        packet.remaining_length;

    // Crear un búfer para almacenar el paquete MQTT
    unsigned char buffer[total_size];
    size_t offset = 0;

    // Copiar el encabezado fijo en el búfer
    buffer[offset++] = packet.fixed_header;

    // Copiar la longitud restante en el búfer
    buffer[offset++] = packet.remaining_length;

    // Copiar el encabezado variable en el búfer
    memcpy(buffer + offset, packet.variable_header, packet.remaining_length);
    offset += packet.remaining_length;

    // Enviar el paquete a través del socket
    ssize_t bytes_sent = send(sockfd, buffer, total_size, 0);
    if (bytes_sent < 0) {
        perror("Error sending packet to server");
        exit(EXIT_FAILURE);
    } else if (bytes_sent != total_size) {
        fprintf(stderr, "Incomplete packet sent to server\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Packet sent successfully\n");
    }
}

void send_packet_subscribe(int sockfd, MQTT_Packet packet) {
    printf("Fixed Header: %u\n", packet.fixed_header);
    printf("Remaining Length: %u\n", packet.remaining_length);

    // Calculate total size of the packet
    size_t total_size = sizeof(packet.fixed_header) + sizeof(packet.remaining_length) +
                        packet.remaining_length + sizeof(packet.payload);

    free_packet(&packet);
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

void send_packet_to_server(int sockfd, MQTT_Packet packet) {
    printf("Fixed Header: %u\n", packet.fixed_header);
    printf("Remaining Length: %u\n", packet.remaining_length);
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

    while(1){
        ;;
    }
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
    } else
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
    } else
        printf("connected to the server..\n");

    MQTT_Packet packet = create_subscribe_packet(encodeMessageToUTF8("EAFIT/Sede/Poblado/Bloque/33/Salon/301/humedad"));
    // Send the packet to the server
    send_packet_subscribe(sockfd, packet);

    // Close the socket
    close(sockfd);

    return 0;
}
