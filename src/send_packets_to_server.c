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

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    // Liberar memoria
    free(buffer);
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