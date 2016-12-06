#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	printf("Trying to open directory named /sub\n");
	int a = opendir2("/sub");
	if (a != -1) {
		printf("Directory opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(dirs_opened[a].record).TypeVal,dirs_opened[a].sectorNr,dirs_opened[a].positionInSector,a);
		puts((dirs_opened[a].record).name);
	}
	else {
		printf("Could not open directory\n");
	}

	printf("Trying to open directory named /sub!&\n");
	a = opendir2("/sub!&");
	if (a != -1) {
		printf("Directory opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(dirs_opened[a].record).TypeVal,dirs_opened[a].sectorNr,dirs_opened[a].positionInSector,a);
		puts((dirs_opened[a].record).name);
	}
	else {
		printf("Could not open directory\n");
	}

	printf("Trying to open directory named /nonexistent\n");
	a = opendir2("/nonexistent");
	if (a != -1) {
		printf("Directory opened:\nRecord typeval: %i\nSectorNr: %i\nPosition in sector: %i\nFile handle: %i\nRecord name: ",(dirs_opened[a].record).TypeVal,dirs_opened[a].sectorNr,dirs_opened[a].positionInSector,a);
		puts((dirs_opened[a].record).name);
	}
	else {
		printf("Could not open directory\n");
	}
	
	int h = opendir2("/sub");
	if (h != -1){
		printf("Directory opened:\nDirectory handle: %i\nIs handle valid?: %i\nRecord name: ",h,dirs_opened[h].is_valid);
		puts((dirs_opened[h].record).name);
		
		closedir2(h);
		printf("Directory closed: \nDirectory handle: %i\nIs handle valid?: %i\n",h,dirs_opened[h].is_valid);
	}
	else {
		printf("Could not open directory\n");
	}
	return 0;
}