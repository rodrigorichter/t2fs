#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	unsigned char i[256];
	read_sector(0, i);
	printf("%i",i[12]);
	create2("/hehe");
	files_opened[0] = 1;
	printf("file opened: %i",files_opened[0]);
	return 0;
}