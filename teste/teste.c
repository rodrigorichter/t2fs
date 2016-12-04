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

	op_file f;
	struct t2fs_record r;


	f.current_pointer= 1;
	f.record= r;
	f.sectorNr= 2;
	f.positionInSector= 3;

	create2("/hehe");
	printf("ele: %i",files_opened[0].is_valid);
	return 0;
}