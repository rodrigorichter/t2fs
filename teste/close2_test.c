#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int h = open2("/arq");
	printf("File opened:\nFile handle: %i\nIs handle valid?: %i\nRecord name: ",h,files_opened[h].is_valid);
	puts((files_opened[h].record).name);
	
	close2(h);
	printf("File closed: \nFile handle: %i\nIs handle valid?: %i\n",h,files_opened[h].is_valid);

	return 0;
}