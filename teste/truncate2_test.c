#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int h = open2("/arq");
	printf("File opened:\nSize in bytes: %i\n",(files_opened[h].record).bytesFileSize);

	seek2(h,17);
	printf("File current pointer was set to: %i\n",files_opened[h].current_pointer);

	truncate2(h);
	printf("File was truncated: \nSize in bytes: %i\n",(files_opened[h].record).bytesFileSize);

	return 0;
}