#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	printf("Trying to open file named /arq\n");
	int a = open2("/arq");
	if (a != -1) {
		printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
		puts((files_opened[a].record).name);
	}
	else {
		printf("Could not open file\n");
	}

	printf("Trying to open file named /arq$@\n");
	a = open2("/arq$@");
	if (a != -1) {
		printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
		puts((files_opened[a].record).name);
	}
	else {
		printf("Could not open file\n");
	}

	printf("Trying to open file named /nonexistent\n");
	a = open2("/nonexistent");
	if (a != -1) {
		printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
		puts((files_opened[a].record).name);
	}
	else {
		printf("Could not open file\n");
	}
	return 0;
}