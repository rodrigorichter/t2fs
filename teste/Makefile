#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./teste


all: close2_test.o close2_test closedir2_test.o closedir2_test create2_test.o create2_test delete2_test.o delete2_test identify2_test.o identify2_test mkdir2_test.o mkdir2_test open2_test.o open2_test openclose2_test.o openclose2_test openclosedir2_test.o openclosedir2_test opendir2_test.o opendir2_test read2_test.o read2_test readdir2_test.o readdir2_test rmdir2_test.o rmdir2_test truncate2_test.o truncate2_test write2_test.o write2_test; mv *_test ../$(BIN_DIR)/

close2_test: close2_test.c
	$(CC) -o close2_test close2_test.c -L../lib -lt2fs -Wall -lm

close2_test.o: close2_test.c
	$(CC) -c close2_test.c -L../lib -lt2fs -Wall -lm

closedir2_test: closedir2_test.c
	$(CC) -o closedir2_test closedir2_test.c -L../lib -lt2fs -Wall -lm

closedir2_test.o: closedir2_test.c
	$(CC) -c closedir2_test.c -L../lib -lt2fs -Wall -lm

create2_test: create2_test.c
	$(CC) -o create2_test create2_test.c -L../lib -lt2fs -Wall -lm

create2_test.o: create2_test.c
	$(CC) -c create2_test.c -L../lib -lt2fs -Wall -lm

delete2_test: delete2_test.c
	$(CC) -o delete2_test delete2_test.c -L../lib -lt2fs -Wall -lm

delete2_test.o: delete2_test.c
	$(CC) -c delete2_test.c -L../lib -lt2fs -Wall -lm

identify2_test: identify2_test.c
	$(CC) -o identify2_test identify2_test.c -L../lib -lt2fs -Wall -lm

identify2_test.o: identify2_test.c
	$(CC) -c identify2_test.c -L../lib -lt2fs -Wall -lm

mkdir2_test: mkdir2_test.c
	$(CC) -o mkdir2_test mkdir2_test.c -L../lib -lt2fs -Wall -lm

mkdir2_test.o: mkdir2_test.c
	$(CC) -c mkdir2_test.c -L../lib -lt2fs -Wall -lm

open2_test: open2_test.c
	$(CC) -o open2_test open2_test.c -L../lib -lt2fs -Wall -lm

open2_test.o: open2_test.c
	$(CC) -c open2_test.c -L../lib -lt2fs -Wall -lm

openclose2_test: openclose2_test.c
	$(CC) -o openclose2_test openclose2_test.c -L../lib -lt2fs -Wall -lm

openclose2_test.o: openclose2_test.c
	$(CC) -c openclose2_test.c -L../lib -lt2fs -Wall -lm

openclosedir2_test: openclosedir2_test.c
	$(CC) -o openclosedir2_test openclosedir2_test.c -L../lib -lt2fs -Wall -lm

openclosedir2_test.o: openclosedir2_test.c
	$(CC) -c openclosedir2_test.c -L../lib -lt2fs -Wall -lm

opendir2_test: opendir2_test.c
	$(CC) -o opendir2_test opendir2_test.c -L../lib -lt2fs -Wall -lm

opendir2_test.o: opendir2_test.c
	$(CC) -c opendir2_test.c -L../lib -lt2fs -Wall -lm

read2_test: read2_test.c
	$(CC) -o read2_test read2_test.c -L../lib -lt2fs -Wall -lm

read2_test.o: read2_test.c
	$(CC) -c read2_test.c -L../lib -lt2fs -Wall -lm

readdir2_test: readdir2_test.c
	$(CC) -o readdir2_test readdir2_test.c -L../lib -lt2fs -Wall -lm

readdir2_test.o: readdir2_test.c
	$(CC) -c readdir2_test.c -L../lib -lt2fs -Wall -lm

rmdir2_test: rmdir2_test.c
	$(CC) -o rmdir2_test rmdir2_test.c -L../lib -lt2fs -Wall -lm

rmdir2_test.o: rmdir2_test.c
	$(CC) -c rmdir2_test.c -L../lib -lt2fs -Wall -lm

truncate2_test: truncate2_test.c
	$(CC) -o truncate2_test truncate2_test.c -L../lib -lt2fs -Wall -lm

truncate2_test.o: truncate2_test.c
	$(CC) -c truncate2_test.c -L../lib -lt2fs -Wall -lm

write2_test: write2_test.c
	$(CC) -o write2_test write2_test.c -L../lib -lt2fs -Wall -lm

write2_test.o: write2_test.c
	$(CC) -c write2_test.c -L../lib -lt2fs -Wall -lm

clean:
	rm ../$(BIN_DIR)/*_test; rm *.o
