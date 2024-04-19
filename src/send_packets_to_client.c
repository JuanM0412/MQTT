#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "../include/send_packets_to_client.h"

void send_publish_to_client(int connfd, MQTT_Packet packet){
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
    write(connfd, buffer, total_size);

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

void send_connack_to_client(int connfd, MQTT_Packet packet) {
    unsigned char buffer[4];
    size_t offset = 0;

    buffer[0] = packet.fixed_header;
    buffer[1] = packet.remaining_length;
    buffer[2] = packet.variable_header[0];
    buffer[3] = packet.variable_header[1];

    write(connfd, buffer, 4);
}

void send_suback_to_client(int connfd, MQTT_Packet packet) {
    size_t total_size = sizeof(packet.fixed_header) + packet.remaining_length + 1;
    printf("total_size -> %zu\n", total_size);

    unsigned char buffer[total_size];
    printf("Buffer");
    size_t offset = 0;

    // Copy structure fields into buffer
    memcpy(buffer + offset, &packet.fixed_header, sizeof(packet.fixed_header));
    offset += 1;
    printf("Offset (1) -> %zu\n", offset);

    memcpy(buffer + offset, &packet.remaining_length, sizeof(packet.remaining_length));
    offset += 1;
    printf("Offset (2) -> %zu\n", offset);

    memcpy(buffer + offset, packet.variable_header, 2);
    offset += 2;

    memcpy(buffer + offset, packet.payload, packet.remaining_length - 2);

    // Send the buffer through the socket
    write(connfd, buffer, total_size);

    printf("Contenido del buffer:\n");
    for (size_t i = 0; i < total_size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}