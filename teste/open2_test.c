#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int a = open2("/arq");
	printf("File opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
	puts((files_opened[a].record).name);
	return 0;
}