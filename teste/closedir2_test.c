#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"


int main() {
	int h = opendir2("/sub");
	printf("Directory opened:\nDirectory handle: %i\nIs handle valid?: %i\nRecord name: ",h,dirs_opened[h].is_valid);
	puts((dirs_opened[h].record).name);
	
	closedir2(h);
	printf("Directory closed: \nDirectory handle: %i\nIs handle valid?: %i\n",h,dirs_opened[h].is_valid);

	return 0;
}