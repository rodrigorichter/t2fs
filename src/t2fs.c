#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

op_file files_opened[20];

int identify2 (char *name, int size) {
	return 0;
}

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
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				char possibleNextFileName[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
				int z=0;

				strcpy(possibleNextFileName,record+1);
				for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2) { // it is a directory
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
						int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
						memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));

						nextFileName = getNextName(filename, &filenameIdx);
						i=0;j=0;
					}
					else return -1;
				}
			}
		}
		// could not find any directory, so lets create the file!
		i=0;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				printf("%i",record[0]);

				int z=0;
				for (z=0;z<32;z++) printf("%c",record[z+1]);
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

					memcpy(buffer+sectorOffset*sizeof(struct t2fs_inode),auxInode,sizeof(struct t2fs_inode));
					write_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer);

					// create record in current directory
					record[0] = 0x01;
					memcpy(record+1,nextFileName,32);
					int newblocksfilesize = 0;
					int newbytesfilesize = 0;

					memcpy(record+33,&newblocksfilesize,4);
					memcpy(record+37,&newbytesfilesize,4);
					memcpy(record+41,&inodeNr,4);

					memcpy(sector+j*64,record,64);
					write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

					// set as an opened file
					int l=0;
					while (files_opened[l].is_valid) l++;
					files_opened[l].is_valid = 1;
					files_opened[l].current_pointer = 0;


					(files_opened[l].record).TypeVal = record[0];
					strcpy((files_opened[l].record).name,record+1);

					files_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize+i;
					files_opened[l].positionInSector = j;
					return l;
				}
			}
		}
	}
	return -1;
}

int delete2 (char *filename) {
	return 0;
}

FILE2 open2 (char *filename) {
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
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				char possibleNextFileName[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
				int z=0;
				memcpy(possibleNextFileName,record+1,32);
				for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 1) { // it is a file

						// set as an opened file
						int l=0;
						while (files_opened[l].is_valid) l++;
						files_opened[l].is_valid = 1;
						files_opened[l].current_pointer = 0;

						(files_opened[l].record).TypeVal = record[0];
						memcpy((files_opened[l].record).name,record+1,32);
						(files_opened[l].record).inodeNumber = record[41]+(record[42]<<8)+(record[43]<<16)+(record[44]<<24);

						files_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize+i;
						files_opened[l].positionInSector = j;
						return l;
					}
					else return -1;
				}
			}
		}
	}
	return -1;
}

int close2 (FILE2 handle) {
	return 0;
}

int read2 (FILE2 handle, char *buffer, int size) {
	op_file f = files_opened[handle];
	int bytesLeft = size;
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
	int inodeNr = (f.record).inodeNumber;
	if ((f.record).TypeVal != 1) return -1;

	// find inode of the opened file
	read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
	int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
	printf("sectorOffset:%i",sectorOffset);
	memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
	int i=0;
	if (currentinode[0] != INVALID_PTR) {
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			int j=0;
			while (j<256 && bytesLeft > 0) { // copy to buffer one byte at a time from the file
				memcpy(buffer+size-bytesLeft,sector+j,1);
				bytesLeft--;
				j++;
				if (bytesLeft == 0) return size;
			}
		}
		return size-bytesLeft;
	}
	return -1;
}

int write2 (FILE2 handle, char *buffer, int size) {
	op_file f = files_opened[handle];
	int bytesLeft = size;
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
	int inodeNr = (f.record).inodeNumber;
	if ((f.record).TypeVal != 1) return -1;
	// find inode of the opened file
	read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
	int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
	memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
	int i=0;
	if (currentinode[0] != INVALID_PTR) {
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			int j=0;
			while (j<256 && bytesLeft > 0) { // copy to buffer one byte at a time from the file
				memcpy(sector+j,buffer+size-bytesLeft,1);
				bytesLeft--;
				j++;
				if (bytesLeft == 0) {
					write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
					return size;
				}
			}
			write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
		}
		return bytesLeft;
	}
	return -1;
}

int truncate2 (FILE2 handle) {
	return 0;
}

int seek2 (FILE2 handle, unsigned int offset) {
	return 0;
}

int mkdir2 (char *pathname) {
	return 0;
}

int rmdir2 (char *pathname) {
	return 0;
}

DIR2 opendir2 (char *pathname) {
	return 0;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry) {
	return 0;
}

int closedir2 (DIR2 handle) {
	return 0;
}