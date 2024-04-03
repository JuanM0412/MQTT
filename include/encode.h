#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include <stdlib.h>

int necessaryBytes(unsigned int codePoint);
void encodeUTF8(unsigned int codePoint, unsigned char *buffer, int bytes);
unsigned char* encodeMessageToUTF8(const char *message);

int trailBytes(unsigned char c);

#endif /* ENCODE_H */