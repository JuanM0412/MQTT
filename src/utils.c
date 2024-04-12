#include <time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>

#include "../include/utils.h"

void logger(char *message, char *serverIP, char *clientIP) {
    time_t now = time(NULL);
    char timeStr[50];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(log_file, "%s - %s - ClientIP: %s - Query: %s - ResponseIP: %s\n", timeStr, message, clientIP, serverIP, message);
}