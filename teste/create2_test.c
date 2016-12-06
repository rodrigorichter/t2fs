#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	printf("Creating file named firstfile\n");
	int a = create2("/firstfile");
	if (a != -1) {
		printf("File created:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
		puts((files_opened[a].record).name);
	}
	else {
		printf("File could not be created\n");
	}

	printf("Creating file named second!file&$\n");
	a = create2("/second!file&$");
	if (a != -1) {
		printf("File created:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
		puts((files_opened[a].record).name);
	}
	else {
		printf("File could not be created\n");
	}



	return 0;
}