#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int h = opendir2("/sub");
	if (h == 0) {
		printf("Directory opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(dirs_opened[h].record).TypeVal,dirs_opened[h].sectorNr,dirs_opened[h].positionInSector,h);
		puts((dirs_opened[h].record).name);
	}
	else {
		printf("Could not open directory\n");
	}
	
	rmdir2("/sub");
	printf("Directory deleted\n");

	h = opendir2("/sub");
	if (h == 0) {
		printf("Directory opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(dirs_opened[h].record).TypeVal,dirs_opened[h].sectorNr,dirs_opened[h].positionInSector,h);
		puts((dirs_opened[h].record).name);
	}
	else {
		printf("Could not open directory\n");
	}
	return 0;
}