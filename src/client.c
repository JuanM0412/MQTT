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

void send_publish_to_server(int sockfd, MQTT_Packet packet) {
    printf("Fixed Header: %u\n", packet.fixed_header);
    printf("Remaining Length: %u\n", packet.remaining_length);
    
    // Calculate total size of the packet
    size_t total_size = sizeof(packet.fixed_header) + packet.remaining_length + 1;
    printf("Total size %zu\n", total_size);
    
    size_t topic_length;

    // Serialize structure data into a byte buffer
    unsigned char buffer[total_size];
    size_t offset = 0;

    // Copy structure fields into buffer
    memcpy(buffer + offset, &packet.fixed_header, sizeof(packet.fixed_header));
    offset += 1;
    printf("Offset (1) -> %zu\n", offset);

    memcpy(buffer + offset, &packet.remaining_length, sizeof(packet.remaining_length));
    offset += 1;
    printf("Offset (2) -> %zu\n", offset);

    topic_length = (packet.variable_header[0] << 8) | packet.variable_header[1];
    printf("Topic len -> %zu\n", topic_length);
    memcpy(buffer + offset, &packet.variable_header[0], 1);
    offset += 1;
    printf("Offset (3) -> %zu\n", offset);
    memcpy(buffer + offset, &packet.variable_header[1], 1);
    offset += 1;
    memcpy(buffer + offset, packet.variable_header + 2, topic_length);
    printf("Offset (4) -> %zu\n", offset);
    offset += topic_length;
    printf("Offset (5) -> %zu\n", offset);

    memcpy(buffer + offset, &packet.variable_header[topic_length + 2], 2);
    offset += 2;
    printf("Offset (6) -> %zu\n", offset);

    for (int i = 0; i < packet.remaining_length - (topic_length + 4); i++) {
        buffer[offset + i] = packet.payload[i];
        printf("buffer[offset + i] = %02x ", buffer[offset + i]);
        printf("payload[i] = %02x ", packet.payload[i]);
    }

    // Send the buffer through the socket
    write(sockfd, buffer, total_size);

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    while(1){
        ;;
    }
}

void print_mqtt_packet(MQTT_Packet packet) {
    printf("Handle... \n");
    printf("Fixed Header: 0x%02X\n", packet.fixed_header);
    printf("Remaining Length: %d\n", packet.remaining_length);
    printf("Variable Header: 0x%02X 0x%02X\n", packet.variable_header[0], packet.variable_header[1]);
    printf("Payload:\n");
    
    for (int i = 0; i < packet.remaining_length; ) {
        // Leer longitud del tema
        int topic_length = (packet.payload[i] << 8) | packet.payload[i + 1];
        printf("Topic Length: %d\n", topic_length);

        // Leer el tema
        printf("Topic: ");
        char *topic = malloc(topic_length);
        for (int j = 0; j < topic_length; j++) {
            topic[j] = packet.payload[i + 2 + j];
        }
        printf("Topic: %s\n", topic);
        printf("\n");

        // Mover al siguiente tema
        i += 2 + topic_length;
    }
}

int send_mqtt_packet(int sockfd, MQTT_Packet packet) {
    // Calcular la longitud total del paquete
    size_t total_length = 2 + packet.remaining_length;

    // Crear un buffer para enviar el paquete completo
    char* buffer = malloc(total_length);
    if (buffer == NULL) {
        perror("Error al asignar memoria");
        return -1;
    }

    // Copiar el encabezado fijo y la longitud restante al buffer
    buffer[0] = packet.fixed_header;
    buffer[1] = packet.remaining_length;

    // Copiar el encabezado variable al buffer
    memcpy(buffer + 2, &packet.variable_header[0], 1);
    memcpy(buffer + 3, &packet.variable_header[1], 1);

    // Copiar el payload al buffer (si existe)
    memcpy(buffer + 4, packet.payload, packet.remaining_length - 2);

    // Enviar el paquete completo a través del socket
    ssize_t bytes_sent = send(sockfd, buffer, total_length, 0);
    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    if (bytes_sent < 0) {
        perror("Error al enviar el paquete");
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}

void print_connect_packet(const MQTT_Packet *packet) {
    printf("Fixed Header: 0x%02X\n", packet->fixed_header);
    printf("Remaining Length: %d\n", packet->remaining_length);

    printf("Variable Header: ");
    for (int i = 0; i < packet->remaining_length - 2; i++) { // Resta 2 para los bytes del packet_id
        printf("%c ", packet->variable_header[i]);
    }
    printf("\n");

    printf("Payload: ");
    for (int i = 0; i < packet->remaining_length - 2; i++) { // Resta 2 para los bytes del packet_id
        printf("%c ", packet->payload[i]);
    }
    printf("\n");
}

void send_connect_to_server(int sockfd, MQTT_Packet packet) {
    // Construir el buffer a enviar
    size_t total_length = 2 + packet.remaining_length;
    char* buffer = malloc(total_length);
    buffer[0] = packet.fixed_header;
    buffer[1] = packet.remaining_length;
    memcpy(&buffer[2], packet.variable_header, 10);
    memcpy(&buffer[12], packet.payload, packet.remaining_length - 10);

    // Enviar el paquete al servidor
    write(sockfd, buffer, total_length);

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    // Liberar memoria
    free(buffer);
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

    MQTT_Packet packet = create_connect_packet(1, encodeMessageToUTF8("J01"), encodeMessageToUTF8("Juan123"), encodeMessageToUTF8("12345678"));
    print_connect_packet(&packet);
    // send_connect_to_server(sockfd, packet);
    // MQTT_Packet packet = create_publish_packet(encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Temperatura"), encodeMessageToUTF8("29"));
    // send_publish_to_server(sockfd, packet);

    const char *topics[] = {
        encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Humedad"),
        encodeMessageToUTF8("EAFIT/Bloque/18/Salon/301/Temperatura"),
        NULL
    };

    // MQTT_Packet packet = create_subscribe_packet(topics);
    // print_mqtt_packet(packet);
    // send_mqtt_packet(sockfd, packet);
    // printf("ENVIADO");

    while (1) {
        printf("receive_packet_from_server\n");
        MQTT_Packet received_packet;
        unsigned char buffer[MAX];

        ssize_t bytes_received = read(sockfd, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            exit(EXIT_FAILURE);
        }

        size_t offset = 0;
        size_t payload_length = 0;

        received_packet.fixed_header = buffer[offset++];
        received_packet.remaining_length = buffer[offset++];
        
        if (received_packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH) {
            printf("MQTT_FIXED_HEADER_PUBLISH\n");
            size_t topic_length = (buffer[offset++] << 8) | buffer[offset++];
            received_packet.variable_header = malloc(4 + topic_length);
            printf("Malloc var\n");

            received_packet.variable_header[0] = topic_length >> 8;
            received_packet.variable_header[1] = topic_length & 0xFF;
            
            memcpy(&received_packet.variable_header[2], buffer + offset, topic_length);
            offset += topic_length;
            printf("Var(1)\n");

            received_packet.variable_header[topic_length + 2] = (buffer[offset++] >> 8) & 0xFF;
            received_packet.variable_header[topic_length + 3] = buffer[offset++] & 0xFF;

            printf("Malloc var(2)\n");
            payload_length = received_packet.remaining_length - (topic_length + 4);
            received_packet.payload = malloc(payload_length);
            printf("Pay(1)\n");
            memcpy(received_packet.payload, buffer + offset, payload_length);
            printf("Pay(2)\n");

            printf("Variable Header: ");
            for (int i = 0; i < topic_length + 4; i++) { // Resta 2 para los bytes del received_packet_id
                printf("%c ", received_packet.variable_header[i]);
            }
            printf("\n");

            printf("Payload: ");
            for (int i = 0; i < payload_length; i++) {
                printf("%c ", received_packet.payload[i]);
            }
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_CONNACK) {
            received_packet.variable_header = malloc(2);
            received_packet.variable_header[0] = buffer[offset++];
            received_packet.variable_header[1] = buffer[offset];

            if (received_packet.variable_header[1] == 0x00) {
                printf("Connected to MQTT broker");
            }
        } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBACK) {
            received_packet.variable_header = malloc(2);
            payload_length = received_packet.remaining_length - 2;

            received_packet.variable_header[0] = buffer[offset++];
            printf("received_packet.variable_header[0]: %02X\n", received_packet.variable_header[0]);
            received_packet.variable_header[1] = buffer[offset++];
            printf("received_packet.variable_header[1]: %02X\n", received_packet.variable_header[1]);

            received_packet.payload = malloc(payload_length);
            memcpy(received_packet.payload, buffer + offset, payload_length);
            printf("Payload: ");
            for (int i = 0; i < payload_length; i++) {
                printf("%02X ", received_packet.payload[i]);
            }
        }

        free_packet(&received_packet);
    }

    // Close the socket
    close(sockfd);
  
    free_packet(&packet);
  
    return 0;
}
