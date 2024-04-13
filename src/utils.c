#include <time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"

void logger_server(char *message, int socket) {
    time_t now = time(NULL);
    char timeStr[50];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(log_file, "%s - Query: %s - ServerIP: %s - Socket: %d\n", timeStr, message, serverIP, socket);
    fflush(log_file);
}

void logger_client(char *message, int socket) {
    time_t now = time(NULL);
    char timeStr[50];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(log_file, "%s - Query: %s - ServerIP: %s - Socket: %d\n", timeStr, message, serverIP, socket);
    fflush(log_file);
}