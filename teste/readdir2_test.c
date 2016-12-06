#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int subHandle = opendir2("/sub");
	DIRENT2 d;
	readdir2(subHandle,&d);
	printf("Directory read:\nCurrent entry: %i\nRecord name: ",dirs_opened[subHandle].current_entry);
	puts(d.name);
	return 0;
}