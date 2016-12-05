#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

int main() 
{
	int tam = 100; // tamanho maximo da string que vai armazenar o nome dos alunos
	char aluno[tam];
	
	int ret = identify2(aluno, tam);
	if (ret==-1)
		printf("Erro: funcao tentou gravar mais que %d caracteres\n", tam);
	else
		printf("%s", aluno);

	return 0;
	exit(0);
}