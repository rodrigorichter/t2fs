#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	printf("Trying to create directory named /pasta25\n");
	int a = create2("/pasta25");
	if (a==-1) {
		printf("Could not create directory\n");
		return 0;
	}
	printf("Directory created:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
	puts((files_opened[a].record).name);

	printf("Trying to create directory named /pasta@$\n");
	a = create2("/pasta@$");
	if (a==-1) {
		printf("Could not create directory\n");
		return 0;
	}
	printf("Directory created:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(files_opened[a].record).TypeVal,files_opened[a].sectorNr,files_opened[a].positionInSector,a);
	puts((files_opened[a].record).name);

	return 0;
}