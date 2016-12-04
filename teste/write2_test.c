#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	BYTE bufferToWrite[] = "kljsdfhkujshfklskljsdfhkujshfklsaaaaaaa";
	BYTE bufferToRead[40];
	int arqHandle = open2("/arq");
	write2(arqHandle,bufferToWrite,40);
	int amountOfBytes = read2(arqHandle,bufferToRead,40);
	// return 0;
	printf("bytes: %i\n", amountOfBytes);
	int z=0;
	
	printf("File written: ");
	puts((files_opened[arqHandle].record).name);
	printf("Content written: \n");
	for (z=0;z<amountOfBytes;z++) printf("%c\n",bufferToRead[z]);
		
	return 0;
}