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
    size_t total_length = 1 + 1 + packet.remaining_length;
    if (packet.payload != NULL) {
        total_length += strlen((char*)packet.payload);
    }

    // Crear un buffer para enviar el paquete completo
    uint8_t* buffer = malloc(total_length);
    if (buffer == NULL) {
        perror("Error al asignar memoria");
        return -1;
    }

    // Copiar el encabezado fijo y la longitud restante al buffer
    buffer[0] = packet.fixed_header;
    buffer[1] = packet.remaining_length;

    // Copiar el encabezado variable al buffer
    memcpy(buffer + 2, packet.variable_header, packet.remaining_length);

    // Copiar el payload al buffer (si existe)
    if (packet.payload != NULL) {
        memcpy(buffer + 2, packet.payload, packet.remaining_length);
    }

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

    //MQTT_Packet packet = create_connect_packet(0, "01");
    // send_packet_connect(sockfd, packet);
    // MQTT_Packet packet = create_subscribe_packet(encodeMessageToUTF8("EAFIT/Sede/Poblado/Bloque/33/Salon/301/humedad"));
    // Send the packet to the server
    // send_packet_subscribe(sockfd, packet);
    //MQTT_Packet packet = create_publish_packet(encodeMessageToUTF8("America/Educacion/Colombia/Antioquia/AreaMetropolitana/Universidades/Pregrado/EAFIT/Sede/Poblado/Bloque/18/Aula/304/Microcontroladores/Sensores/Clima/Humedad"), encodeMessageToUTF8("20%"));
    //send_packet_to_server(sockfd, packet);

    const char *topics[] = {
        "a/b",
        "EAFIT/Poblado/Bloque/18",
        "Nacho/Minas/Bloque/1",
        "UdeA/Principal/Bloque/5",
        "America/Educacion/Colombia/Antioquia/AreaMetropolitana/Universidades/Pregrado/EAFIT/Sede/Poblado/Bloque/19/Aula/416/Microcontroladores/Sensores/Clima/Temperatura",
        NULL
    };

    MQTT_Packet packet = create_subscribe_packet(topics);
    print_mqtt_packet(packet);
    send_mqtt_packet(sockfd, packet);
    size_t total_size = sizeof(MQTT_Packet);

    // Tamaño de los datos a los que apuntan los punteros
    total_size += strlen((char*)packet.variable_header);
    total_size += strlen((char*)packet.payload);

    printf("Tamaño total de MQTT_Packet: %zu\n", total_size);
    // Close the socket
    close(sockfd);
  
    free_packet(&packet);
  
    return 0;
}
