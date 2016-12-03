#include <stdio.h>
#include "../include/apidisk.h"

int main() {
	unsigned char i[256];
	read_sector(0, i);
	printf("%i",i[12]);
	return 0;
}