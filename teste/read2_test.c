#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

int main() {
	BYTE buffer[3];
	int arqHandle = open2("/arq");
	int amountOfBytes = read2(arqHandle,buffer,3);
	printf("File read: ");
	puts((files_opened[arqHandle].record).name);
	printf("Content in buffer: \n");
	int z=0;
	for (z=0;z<amountOfBytes;z++) printf("%c\n",buffer[z]);

	amountOfBytes = read2(arqHandle,buffer,3);
	printf("Reading again.. File read: ");
	puts((files_opened[arqHandle].record).name);
	printf("Content in buffer: \n");
	z=0;
	for (z=0;z<amountOfBytes;z++) printf("%c\n",buffer[z]);

	amountOfBytes = read2(arqHandle,buffer,3);
	printf("Reading again.. File read: ");
	puts((files_opened[arqHandle].record).name);
	printf("Content in buffer: \n");
	z=0;
	for (z=0;z<amountOfBytes;z++) printf("%c\n",buffer[z]);

	amountOfBytes = read2(arqHandle,buffer,3);
	printf("Reading again.. File read: ");
	puts((files_opened[arqHandle].record).name);
	printf("Content in buffer: \n");
	z=0;
	for (z=0;z<amountOfBytes;z++) printf("%c\n",buffer[z]);
	return 0;
}