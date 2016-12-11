#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

op_file files_opened[20];
op_dir dirs_opened[20];

int identify2 (char *name, int size) {

  char alunos[] = "Rodrigo Ronconi Richter cartao 217445 e Roberta Robert cartao 194285";
  
  if (strlen(alunos) > size) {
     printf("nomes maiores que o tamanho informado");
     return -1;
  }

  strcpy(name, alunos);
  
  return 0;

}

FILE2 create2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') (*idx)++;

		while (filename[j] != '\0' && filename[*idx] != '/') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
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
	if (nextFileName == -1) return -1;

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
				//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2) { // it is a directory
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
						int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
						memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));

						nextFileName = getNextName(filename, &filenameIdx);
						if (nextFileName == -1) return -1;
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
				//printf("%i",record[0]);

				int z=0;
				//for (z=0;z<32;z++) printf("%c",record[z+1]);
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
					memcpy(record+1,nextFileName,31);
					int newblocksfilesize = 0;
					int newbytesfilesize = 0;

					memcpy(record+32,&newblocksfilesize,4);
					memcpy(record+36,&newbytesfilesize,4);
					memcpy(record+40,&inodeNr,4);

					memcpy(sector+j*64,record,64);
					write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

					// set as an opened file
					int l=0;
					while (files_opened[l].is_valid) l++;
					files_opened[l].is_valid = 1;
					files_opened[l].current_pointer = 0;


					(files_opened[l].record).TypeVal = record[0];
					(files_opened[l].record).inodeNumber = record[40]+(record[41]>>8)+(record[42]>>16)+(record[43]>>24);
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
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') (*idx)++;

		while (filename[j] != '\0' && filename[*idx] != '/') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
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
	if (nextFileName == -1) return -1;
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
				//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 1) { // it is a file
						// set record as invalid
						record[0] = 0;
						memcpy(sector+j*64,record,64);
						write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

						// // set its block and inode as free
						int fileinode[4];
						unsigned char buffer[SECTOR_SIZE];
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer);
						int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
						memcpy(fileinode,buffer+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
						// set bitmaps as free
						setBitmap2(BITMAP_DADOS,fileinode[0],0);
						setBitmap2(BITMAP_INODE,inodeNr,0);
						// set 
						// fileinode[0] = -1;
						// memcpy(buffer+sectorOffset*sizeof(struct t2fs_inode),fileinode,4);
						// write_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer)

						
						return 0;
					}
				}
			}
		}
	}
	return -1;
}

FILE2 open2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') (*idx)++;

		while (filename[j] != '\0' && filename[*idx] != '/') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
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
	if (nextFileName == -1) return -1;
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
				//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 1) { // it is a file

						// set as an opened file
						int l=0;
						while (files_opened[l].is_valid) l++;
						files_opened[l].is_valid = 1;
						files_opened[l].current_pointer = 0;

						(files_opened[l].record).TypeVal = record[0];
						memcpy((files_opened[l].record).name,record+1,32);
						(files_opened[l].record).bytesFileSize = record[37]+(record[38]<<8)+(record[39]<<16)+(record[40]<<24);
						(files_opened[l].record).inodeNumber = record[41]+(record[42]<<8)+(record[43]<<16)+(record[44]<<24);

						files_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize+i;
						files_opened[l].positionInSector = j;
						return l;
					}
					else return -1;
				}
			}
		}
		if (currentinode[1] != INVALID_PTR) { // second direct pointer from inode is ok
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
					//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
					if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
						if (record[0] == 1) { // it is a file

							// set as an opened file
							int l=0;
							while (files_opened[l].is_valid) l++;
							files_opened[l].is_valid = 1;
							files_opened[l].current_pointer = 0;

							(files_opened[l].record).TypeVal = record[0];
							memcpy((files_opened[l].record).name,record+1,32);
							(files_opened[l].record).bytesFileSize = record[37]+(record[38]<<8)+(record[39]<<16)+(record[40]<<24);
							(files_opened[l].record).inodeNumber = record[41]+(record[42]<<8)+(record[43]<<16)+(record[44]<<24);

							files_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize+i;
							files_opened[l].positionInSector = j;
							return l;
						}
						else return -1;
					}
				}
			}
			if (currentinode[2] != INVALID_PTR) { // indirect pointer is ok
				unsigned char buffer[SECTOR_SIZE];
				int i=0;
				for (i=0;i<blockSize;i++) { // iterate through sectors in the block
					read_sector(blocksOffset+currentinode[2]*blockSize+i,sector);

					int h=0;
					for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
						int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);
						if (indPointer != INVALID_PTR) {
							int j=0;
							for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
								read_sector(blocksOffset+indPointer*blockSize+j,buffer);
								int k=0;
								BYTE record[64];
								for (k=0;k<4;k++) { // iterate through records in sector
									memcpy(record,buffer+k*64,64);
									char possibleNextFileName[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
									int z=0;
									memcpy(possibleNextFileName,record+1,32);
									//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
									if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
										if (record[0] == 1) { // it is a file

											// set as an opened file
											int l=0;
											while (files_opened[l].is_valid) l++;
											files_opened[l].is_valid = 1;
											files_opened[l].current_pointer = 0;

											(files_opened[l].record).TypeVal = record[0];
											memcpy((files_opened[l].record).name,record+1,32);
											(files_opened[l].record).bytesFileSize = record[37]+(record[38]<<8)+(record[39]<<16)+(record[40]<<24);
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
					}
				}
				if (currentinode[3] != INVALID_PTR) { // double indirect pointer is ok
					unsigned char buffer[SECTOR_SIZE];
					int i=0;
					for (i=0;i<blockSize;i++) { // iterate through sectors in the block
						read_sector(blocksOffset+currentinode[2]*blockSize+i,sector);

						int h=0;
						for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
							int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);
							if (indPointer != INVALID_PTR) {
								int j=0;
								for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
									read_sector(blocksOffset+indPointer*blockSize+j,buffer);

									int m=0;
									for (m=0;m<SECTOR_SIZE;m+=4) { // iterate through pointers in sector
										int secondindPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

										if (secondindPointer != INVALID_PTR) {
											unsigned char bufferInd[SECTOR_SIZE];
											int n=0;
											for (n=0;n<blockSize;n++) { // iterate through sectors in the block pointed by the second indirect pointer
												read_sector(blocksOffset+secondindPointer*blockSize+n,bufferInd);

												int k=0;
												BYTE record[64];
												for (k=0;k<4;k++) { // iterate through records in sector
													memcpy(record,bufferInd+k*64,64);
													char possibleNextFileName[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
													int z=0;
													memcpy(possibleNextFileName,record+1,32);
													//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
													if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
														if (record[0] == 1) { // it is a file

															// set as an opened file
															int l=0;
															while (files_opened[l].is_valid) l++;
															files_opened[l].is_valid = 1;
															files_opened[l].current_pointer = 0;

															(files_opened[l].record).TypeVal = record[0];
															memcpy((files_opened[l].record).name,record+1,32);
															(files_opened[l].record).bytesFileSize = record[37]+(record[38]<<8)+(record[39]<<16)+(record[40]<<24);
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
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return -1;
}

int close2 (FILE2 handle) {
	files_opened[handle].is_valid = 0;
	return 0;
}

int read2 (FILE2 handle, char *buffer, int size) {
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
	int inodeNr = (files_opened[handle].record).inodeNumber;
	if ((files_opened[handle].record).TypeVal != 1) return -1;

	// find inode of the opened file
	read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
	int sectorOffset = (((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
	
	sectorOffset+=(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE);

	memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
	int i=0;
	if (currentinode[0] != INVALID_PTR) {
		int j=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			while (j<SECTOR_SIZE && bytesLeft > 0) { // copy to buffer one byte at a time from the file
				memcpy(buffer+size-bytesLeft,sector+j,1);
				bytesLeft--;
				j++;
				if (bytesLeft == 0) {
					(files_opened[handle].current_pointer)+=size;
					return size;
				}
			}
			j=0;
		}
		if (currentinode[1] != INVALID_PTR) {
			j=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
			for (i=0;i<blockSize;i++) { // iterate through sectors in the block
				read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
				while (j<SECTOR_SIZE && bytesLeft > 0) { // copy to buffer one byte at a time from the file
					memcpy(buffer+size-bytesLeft,sector+j,1);
					bytesLeft--;
					j++;
					if (bytesLeft == 0) {
						(files_opened[handle].current_pointer)+=size;
						return size;
					}
				}
				j=0;
			}
			if (currentinode[2] != INVALID_PTR) {
				unsigned char indSector[SECTOR_SIZE];
				for (i=0;i<blockSize;i++) { // iterate through sectors in the block
					read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

					int h=0;
					for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
						int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

						if (indPointer != INVALID_PTR) {
							int j=0;
							int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
							
							for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
								read_sector(blocksOffset+indPointer*blockSize+j,indSector);

								while (k<SECTOR_SIZE && bytesLeft > 0) { // copy to buffer one byte at a time from the file
									memcpy(buffer+size-bytesLeft,indSector+k,1);
									bytesLeft--;
									k++;

									if (bytesLeft == 0) {
										(files_opened[handle].current_pointer)+=size;
										return size;
									}
								}
								k=0;
							}
						}
					}
				}
				if (currentinode[3] != INVALID_PTR) {
					unsigned char indSector[SECTOR_SIZE];
					for (i=0;i<blockSize;i++) { // iterate through sectors in the block
						read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

						int h=0;
						for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
							int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

							if (indPointer != INVALID_PTR) {
								int j=0;
								int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
								
									unsigned char secondindSector[SECTOR_SIZE];
								for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
									read_sector(blocksOffset+indPointer*blockSize+j,secondindSector);

									int h=0;
									for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
										int secondindPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

										if (secondindPointer != INVALID_PTR) {
											int j=0;
											int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
											
											for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
												read_sector(blocksOffset+secondindPointer*blockSize+j,secondindSector);



												while (k<SECTOR_SIZE && bytesLeft > 0) { // copy to buffer one byte at a time from the file
													memcpy(buffer+size-bytesLeft,secondindSector+k,1);
													bytesLeft--;
													k++;

													if (bytesLeft == 0) {
														(files_opened[handle].current_pointer)+=size;
														return size;
													}
												}
												k=0;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		(files_opened[handle].current_pointer)+=(size-bytesLeft);
		return size-bytesLeft;
	}
	printf("nao leu\n");
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
	
	sectorOffset+=(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE);
	memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
	int i=0;
	int blockNr;
	if (currentinode[0] == INVALID_PTR) {
		// allocate block
		int amountblocks = 20000;
		blockNr = -10;
		int k=0;
		while (k != blockNr+1 && k<amountblocks) { // search through bitmap of data
			if (getBitmap2(BITMAP_DADOS,k) == 0) { // found an empty block entry
				setBitmap2(BITMAP_DADOS,k,1);
				blockNr = k;
				// printf("bloco: %i\n", blockNr);
			}
			k++;
		}

		memcpy(currentinode,&blockNr,4);
		memcpy(sector+sectorOffset*sizeof(struct t2fs_inode),currentinode,sizeof(struct t2fs_inode));
		write_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
		// printf("bloco escrito: %i\n", currentinode[0]);
	}


	if (currentinode[0] != INVALID_PTR) {
		// printf("entrou\n");
		int j=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			while (j<256 && bytesLeft > 0) { // copy from buffer one byte at a time from the file
				memcpy(sector+j,buffer+size-bytesLeft,1);
				bytesLeft--;
				j++;
				if (bytesLeft == 0) {
					(files_opened[handle].current_pointer)+=size;
					write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
					return size;
				}
			}
			write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			(files_opened[handle].current_pointer)+=SECTOR_SIZE;
		}


		if (currentinode[1] != INVALID_PTR) {
			j=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
			for (i=0;i<blockSize;i++) { // iterate through sectors in the block
				read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
				while (j<256 && bytesLeft > 0) { // copy from buffer one byte at a time from the file
					memcpy(sector+j,buffer+size-bytesLeft,1);
					bytesLeft--;
					j++;
					if (bytesLeft == 0) {
						(files_opened[handle].current_pointer)+=size;
						write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
						return size;
					}
				}
				write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
				(files_opened[handle].current_pointer)+=SECTOR_SIZE;
			}
			if (currentinode[2] != INVALID_PTR) {
				unsigned char indSector[SECTOR_SIZE];
				for (i=0;i<blockSize;i++) { // iterate through sectors in the block
					read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

					int h=0;
					for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
						int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

						if (indPointer != INVALID_PTR) {
							int j=0;
							int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
							
							for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
								read_sector(blocksOffset+indPointer*blockSize+j,indSector);

								while (j<256 && bytesLeft > 0) { // copy from buffer one byte at a time from the file
									memcpy(indSector+j,buffer+size-bytesLeft,1);
									bytesLeft--;
									j++;
									if (bytesLeft == 0) {
										(files_opened[handle].current_pointer)+=size;
										write_sector(blocksOffset+indPointer*blockSize+i,indSector);
										return size;
									}
								}
								write_sector(blocksOffset+indPointer*blockSize+i,indSector);
								(files_opened[handle].current_pointer)+=SECTOR_SIZE;
							}
						}
					}
				}
				if (currentinode[3] != INVALID_PTR) {
					unsigned char indSector[SECTOR_SIZE];
					for (i=0;i<blockSize;i++) { // iterate through sectors in the block
						read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

						int h=0;
						for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
							int indPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

							if (indPointer != INVALID_PTR) {
								int j=0;
								int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
								
									unsigned char secondindSector[SECTOR_SIZE];
								for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
									read_sector(blocksOffset+indPointer*blockSize+j,secondindSector);

									int h=0;
									for (h=0;h<SECTOR_SIZE;h+=4) { // iterate through pointers in sector
										int secondindPointer = sector[h]+(sector[h+1]<<8)+(sector[h+2]<<16)+(sector[h+3]<<24);

										if (secondindPointer != INVALID_PTR) {
											int j=0;
											int k=(((float)(files_opened[handle].current_pointer)/SECTOR_SIZE)-(int)((float)(files_opened[handle].current_pointer)/SECTOR_SIZE))*SECTOR_SIZE;
											
											for (j=0;j<blockSize;j++) { // iterate through sectors in the block pointed by the indirect pointer
												read_sector(blocksOffset+secondindPointer*blockSize+j,secondindSector);

												while (j<256 && bytesLeft > 0) { // copy from buffer one byte at a time from the file
													memcpy(secondindSector+j,buffer+size-bytesLeft,1);
													bytesLeft--;
													j++;
													if (bytesLeft == 0) {
														(files_opened[handle].current_pointer)+=size;
														write_sector(blocksOffset+secondindPointer*blockSize+i,secondindSector);
														return size;
													}
												}
												write_sector(blocksOffset+secondindPointer*blockSize+i,secondindSector);
												(files_opened[handle].current_pointer)+=SECTOR_SIZE;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		(files_opened[handle].current_pointer)+=(size-bytesLeft);
		return size-bytesLeft;
	}













	return -1;
}

int truncate2 (FILE2 handle) {
	((files_opened[handle].record).bytesFileSize)-=((files_opened[handle].record).bytesFileSize)-files_opened[handle].current_pointer;
	unsigned char sector[SECTOR_SIZE];
	read_sector(files_opened[handle].sectorNr,sector);

	int bytes = (files_opened[handle].record).bytesFileSize;


	memcpy(37+sector+64*files_opened[handle].positionInSector,&bytes,4);
	write_sector(files_opened[handle].sectorNr,sector);
	return 0;
}

int seek2 (FILE2 handle, unsigned int offset) {
	files_opened[handle].current_pointer+=offset;
	if (files_opened[handle].current_pointer < 0) files_opened[handle].current_pointer = 0;
	return 1;
}

int mkdir2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') (*idx)++;

		while (filename[j] != '\0' && filename[*idx] != '/') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
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
	if (nextFileName == -1) return -1;

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
				// for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2) { // it is a directory
						if (filename[filenameIdx] == '\0') return -1; // directory already exists

						// enter the directory and continue search
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
						int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
						memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));

						nextFileName = getNextName(filename, &filenameIdx);
						i=0;j=0;
					}
				}
			}
		}
		// could not find any directory to enter, so lets create the new directory
		i=0;
		for (i=0;i<blockSize;i++) { // iterate through sectors in the block
			read_sector(blocksOffset+currentinode[0]*blockSize+i,sector);
			int j=0;
			BYTE record[64];
			for (j=0;j<4;j++) { // iterate through records in sector
				memcpy(record,sector+j*64,64);
				//printf("%i",record[0]);

				int z=0;
				//for (z=0;z<32;z++) printf("%c",record[z+1]);
				if (record[0] != 1 && record[0] != 2) { // found an available record
					// create inode
					int amountinodes = inodesSize*SECTOR_SIZE/sizeof(struct t2fs_inode);
					inodeNr = -10;
					int k=0;
					while (k != inodeNr+1 && k<amountinodes) { // search through bitmap of inodes
						if (getBitmap2(BITMAP_INODE,k) == 0) { // found an empty inode entry
							setBitmap2(BITMAP_INODE,k,1);
							inodeNr = k;
						}
						k++;
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
					record[0] = 0x02;
					memcpy(record+1,nextFileName,32);
					int newblocksfilesize = 0;
					int newbytesfilesize = 0;

					memcpy(record+33,&newblocksfilesize,4);
					memcpy(record+37,&newbytesfilesize,4);
					memcpy(record+41,&inodeNr,4);
					memcpy(sector+j*64,record,64);
					write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

					return 0;
				}
			}
		}
	}
	return -1;
}

int rmdir2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') {
			curfile[j] = filename[*idx];
			j++;
			(*idx)++;
			if (filename[*idx] == '\0' || filename[*idx] == '\t') return curfile;
			else {
				j = 0;
			}
		}

		while (filename[j] != '\0' && filename[*idx] != '/' && filename[j] != '\t') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
			curfile[j] = filename[*idx];
			(*idx)++;
			j++;
			curfile[j] = '\0';
		}
		(*idx)--;
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
	if (nextFileName == -1) return -1;
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
				//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2 && (filename[filenameIdx] == '\0' || filename[filenameIdx] == '\t')) { // it is a directory and is the one we want
						// set record as invalid
						record[0] = 0;
						memcpy(sector+j*64,record,64);
						write_sector(blocksOffset+currentinode[0]*blockSize+i,sector);

						// set its block and inode as free
						int dirinode[4];
						unsigned char buffer[SECTOR_SIZE];
						inodeNr = record[40] + (record[41]<<8) + (record[42]<<16) + (record[43]<<24);
						read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),buffer);
						int sectorOffset = ((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))-(int)(float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode)))*sizeof(struct t2fs_inode);
						memcpy(dirinode,buffer+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));

						// set bitmaps as free
						setBitmap2(BITMAP_DADOS,dirinode[0],0);
						setBitmap2(BITMAP_INODE,inodeNr,0);

						
						return 0;
					}
				}
			}
		}
	}
	return -1;
}

DIR2 opendir2 (char *filename) {
	char * getNextName(char *filename, int *idx) {
		char *curfile = malloc(32);
		int j=0;
		if (filename[*idx] == '/') {
			curfile[j] = filename[*idx];
			j++;
			(*idx)++;
			if (filename[*idx] == '\0' || filename[*idx] == '\t') return curfile;
			else {
				j = 0;
			}
		}

		while (filename[j] != '\0' && filename[*idx] != '/' && filename[j] != '\t') {
			if (!(filename[*idx] == '.' || filename[*idx] == '\0' || (filename[*idx] >= '0' && filename[*idx] <= '9') || (filename[*idx] >= 'a' && filename[*idx] <= 'z') || (filename[*idx] >= 'A' && filename[*idx] <= 'Z'))) return -1;
			curfile[j] = filename[*idx];
			(*idx)++;
			j++;
			curfile[j] = '\0';
		}
		(*idx)--;
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
	if (nextFileName == -1) return -1;
	if (nextFileName[0] == '/') {
		// set as an opened directory
		int l=0;
		while (dirs_opened[l].is_valid) l++;
		dirs_opened[l].is_valid = 1;
		dirs_opened[l].current_entry = 0;

		(dirs_opened[l].record).TypeVal = 2;
		memcpy((dirs_opened[l].record).name,nextFileName,2);
		(dirs_opened[l].record).inodeNumber = 0;

		dirs_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize;
		dirs_opened[l].positionInSector = 0;
		return l;
	}
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
				//for (z=0;z<32;z++) printf("%c",possibleNextFileName[z]);
				if (strcmp(possibleNextFileName,nextFileName) == 0) { // found file
					if (record[0] == 2) { // it is a directory
						if (filename[filenameIdx] == '\0' || filename[filenameIdx] == '\t') { // it is the one we want
							// set as an opened directory
							int l=0;
							while (dirs_opened[l].is_valid) l++;
							dirs_opened[l].is_valid = 1;
							dirs_opened[l].current_entry = 0;

							(dirs_opened[l].record).TypeVal = record[0];
							memcpy((dirs_opened[l].record).name,record+1,32);
							(dirs_opened[l].record).inodeNumber = record[40]+(record[41]<<8)+(record[42]<<16)+(record[43]<<24);

							dirs_opened[l].sectorNr = blocksOffset+currentinode[0]*blockSize+i;
							dirs_opened[l].positionInSector = j;
							return l;
						}
					}
				}
			}
		}
	}
	return -1;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry) {
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
	if (dirs_opened[handle].is_valid == 0) return -1;
	int currentinode[4];
	int inodeNr = (dirs_opened[handle].record).inodeNumber;

	// find inode of the opened directory
	read_sector(inodesOffset+(int)((float)inodeNr/(SECTOR_SIZE/sizeof(struct t2fs_inode))),sector);
	
	int sectorOffset=(int)((float)((dirs_opened[handle].current_entry)*64)/SECTOR_SIZE);

	//printf("entry: %i",((dirs_opened[handle].current_entry)*64));
	//printf("sectorOffset:%i",sectorOffset);
	memcpy(currentinode,sector+sectorOffset*sizeof(struct t2fs_inode),sizeof(struct t2fs_inode));
	if (currentinode[0] != INVALID_PTR) {
		read_sector(blocksOffset+currentinode[0]*blockSize+sectorOffset,sector);
		int j=(((float)((dirs_opened[handle].current_entry)*64)/SECTOR_SIZE)-(int)((float)((dirs_opened[handle].current_entry)*64)/SECTOR_SIZE))*SECTOR_SIZE;
		BYTE record[64];
		memcpy(record,sector+j,64);
		if (record[0] != 0) {
			memcpy(dentry->name,record+1,32);
			dentry->fileType = record[0];
			dentry->fileSize = record[37]+(record[38]>>8)+(record[39]>>16)+(record[40]>>24);
			(dirs_opened[handle].current_entry)++;
			return 0;
		}
	}
	return -1;
}

int closedir2 (DIR2 handle) {
	dirs_opened[handle].is_valid = 0;
	return 0;
}