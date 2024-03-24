#include "encode/Encode.c"
#include "encode/Decode.c"
#include <stdio.h>
#include <stdlib.h>

int main() {
    const char *cadena = "ñato";
    unsigned char *buffer = encodeMessageToUTF8(cadena);

    printf("UTF-8 encode: ");
    for (int i = 0; buffer[i] != '\0'; ++i) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
    
    free(buffer);

    return 0;
}
