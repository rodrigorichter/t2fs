#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	printf("Trying to open file named /arq\n");
	int h = open2("/arq");
	if (h == 0) {
		printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[h].record).TypeVal,files_opened[h].sectorNr,files_opened[h].positionInSector,h);
		puts((files_opened[h].record).name);
	}
	else {
		printf("Could not open file\n");
	}
	
	delete2("/arq");
	printf("File deleted\n");

	printf("Trying to open file named /arq\n");
	h = open2("/arq");
	if (h == 0) {
		printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[h].record).TypeVal,files_opened[h].sectorNr,files_opened[h].positionInSector,h);
		puts((files_opened[h].record).name);
	}
	else {
		printf("Could not open file\n");
	}
	return 0;
}