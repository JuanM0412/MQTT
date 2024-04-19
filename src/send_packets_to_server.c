#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/send_packets_to_server.h"

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

    // Liberar memoria
    free(buffer);

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

void send_publish_to_server(int sockfd, MQTT_Packet packet) {
    // Calculate total size of the packet
    size_t total_size = sizeof(packet.fixed_header) + packet.remaining_length + 1;
    
    size_t topic_length;

    // Serialize structure data into a byte buffer
    unsigned char buffer[total_size];
    size_t offset = 0;

    // Copy structure fields into buffer
    memcpy(buffer + offset, &packet.fixed_header, sizeof(packet.fixed_header));
    offset += 1;

    memcpy(buffer + offset, &packet.remaining_length, sizeof(packet.remaining_length));
    offset += 1;

    topic_length = (packet.variable_header[0] << 8) | packet.variable_header[1];
    memcpy(buffer + offset, &packet.variable_header[0], 1);
    offset += 1;
    memcpy(buffer + offset, &packet.variable_header[1], 1);
    offset += 1;
    memcpy(buffer + offset, packet.variable_header + 2, topic_length);
    offset += topic_length;

    memcpy(buffer + offset, &packet.variable_header[topic_length + 2], 2);
    offset += 2;

    for (int i = 0; i < packet.remaining_length - (topic_length + 4); i++) {
        buffer[offset + i] = packet.payload[i];
    }

    // Send the buffer through the socket
    write(sockfd, buffer, total_size);
}

int send_subscribe_to_server(int sockfd, MQTT_Packet packet) {
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

    free(buffer);
    return 0;
}