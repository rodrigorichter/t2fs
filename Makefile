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

all: libt2fs.a *.o

libt2fs.a: *.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o
	ar crs $(LIB_DIR)/libt2fs.a $(LIB_DIR)/*.o

*.o: $(SRC_DIR)/t2fs.c $(INC_DIR)/apidisk.h $(INC_DIR)/bitmap2.h $(INC_DIR)/t2fs.h
	$(CC) -c $(SRC_DIR)/t2fs.c -Wall -lm; mv *.o $(LIB_DIR)/

clean:
	rm $(LIB_DIR)/t2fs.o $(LIB_DIR)/libt2fs.a


