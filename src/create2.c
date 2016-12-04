#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

FILE2 create2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') (*idx)++;

		while (filename[j] != '\0' && filename[*idx] != '/') {
			curfile[j] = filename[*idx];
			(*idx)++;
			j++;
			curfile[j] = '\0';
		}
		return curfile;
	}

	// read superblock
	unsigned char sector[SECTOR_SIZE];
	read_sector(0,sector);
	
	int superblockSize =  sector[6] + (sector[7]<<8);
	int bitmapBlocksSize =  sector[8] + (sector[9]<<8);
	int bitmapInodesSize =  sector[10] + (sector[11]<<8);
	int inodesSize = sector[12] + (sector[13]<<8);

	int inodesOffset = superblockSize+bitmapBlocksSize+bitmapInodesSize;
	int blocksOffset = superblockSize+bitmapBlocksSize+bitmapInodesSize+inodesSize;

	int blockSize = sector[14] + (sector[15]<<8);

	int currentinode[4];
	int inodeNr = 0;
	// read root inode
	read_sector(inodesOffset,sector);
	int inodeRoot[4];
	memcpy(inodeRoot,sector,sizeof(struct t2fs_inode));
	//printf("dataptr1: %i\n dataptr2: %i\n singleindr: %i\n doubleindir: %i",inodeRoot[0],inodeRoot[1],inodeRoot[2],inodeRoot[3]);
	// search root directory for next file
	int filenameIdx = 0;
	char *nextFileName;
	nextFileName = getNextName(filename, &filenameIdx);

	memcpy(currentinode,inodeRoot,sizeof(struct t2fs_inode));
	if (currentinode[0] != INVALID_PTR) { // first direct pointer from inode is ok
		int i=0;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]+i,sector);

			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				char possibleNextFileName[32];
				memcpy(possibleNextFileName,record+1,31);
				possibleNextFileName[31] = '\0';

				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2) { // it is a directory
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+inodeNr,sector);
						memcpy(currentinode,sector,sizeof(struct t2fs_inode));
						nextFileName = getNextName(filename, &filenameIdx);
						i=0;j=0;
					}
				}
			}
		}
		// could not find any directory, so lets create the file!
		i=0;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]+i,sector);

			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				printf("%i",record[0]);

	//printf("%s\n",nextFileName);
				if (record[0] != 1 && record[0] != 2) { // found an available record
					// create inode
					int amountinodes = inodesSize*SECTOR_SIZE/sizeof(struct t2fs_inode);

					int k=0;
					for (k=0;k<amountinodes;k++) { // search through bitmap of inodes
						if (getBitmap2(BITMAP_INODE,k) == 0) { // found an empty inode entry
							setBitmap2(BITMAP_INODE,k,1);
							 inodeNr = k;
						}
					}
					// populate the inode
					unsigned char buffer[SECTOR_SIZE];
					read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer);
					int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
					
					int *auxInode = malloc(sizeof(struct t2fs_inode));
					auxInode[0] = -1;
					auxInode[1] = -1;
					auxInode[2] = -1;
					auxInode[3] = -1;

					memcpy(buffer+sectorOffset,auxInode,sizeof(struct t2fs_inode));
					write_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer);

					// create record in current directory
					record[0] = 0x01;
					memcpy(record+1,nextFileName,31);
					int newblocksfilesize = 0;
					int newbytesfilesize = 0;

					memcpy(record+32,&newblocksfilesize,4);
					memcpy(record+36,&newbytesfilesize,4);
					memcpy(record+40,&inodeNr,4);

					memcpy(sector+j*64,record,64);
					write_sector(blocksOffset+currentinode[0]+i,sector);


				}
			}
		}


	}
	// read first record
	read_sector(blocksOffset+inodeRoot[0],sector);
	BYTE record[64];
	memcpy(record,sector,64);
	char s[32];
	memcpy(s,record+1,31);
	s[31] = '\0';



	return 0;
}