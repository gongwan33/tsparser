/*
 *Project name: tsparser
 *File: main.c
 *Author: Wagner
 *Date: 2015-7
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <tsparser.h>
#include <errnum.h>

#define PROG_INFO_BUFFER_SIZE 1024*4

unsigned int programNum = -1;
char fileName[200];
char* progInfoBuffer;
unsigned int freq = -1;

FILE* datFp;
FILE* targetTsFp;

void helpInfo()
{
	printf("-s\t Search specific ts for program information\n-a\t Get NIT and search all ts files by frequency info.\n-h\t Help information\n[number] Play specific program (shold search first to make sure which ts to play).\n");
}

int isNum(char* str)
{
	int i = 0;
	while(str[i])
	{
		if(!isdigit(str[i]))
			return 0;
		i++;
	}
	return 1;
}

int main(int argc, char** argv)
{
	int ch;

	if(argc <= 1)
	{
		helpInfo();
		return NO_PARAM;
	}
	else if(argv[1][0] != '-' && isNum(argv[1]))
	{
		programNum = atoi(argv[1]);
	}

	while((ch = getopt(argc, argv, "s:a:h")) != -1)
	{
		switch(ch)
		{
			case 's':
				printf("option s:'%s'\n", optarg);
				if(isNum(optarg))
					freq = atoi(optarg);
				break;
			case 'a':
				break;
			case 'h':
				helpInfo();
				break;
			default:
				helpInfo();
		}
	}	

	if(opterr != 0 && programNum == -1)
		printf("Please check your command or use -h\n");

	printf("pr num %d\n", programNum);

    progInfoBuffer = (char *)malloc(PROG_INFO_BUFFER_SIZE);
	if(progInfoBuffer == NULL)
	{
		printf("Erro occurred! Maybe lack of memory.");
		return MALLOC_ERR;
	}

	if((datFp = fopen("tmp.dat", "wt+")) == NULL)
		return DAT_OPEN_ERR;

	sprintf(progInfoBuffer, "program num %d\n", programNum);	
	fwrite(progInfoBuffer, strlen(progInfoBuffer),1, datFp);

	if(datFp != NULL)
		fclose(datFp);
	free(progInfoBuffer);
	return 0;	
}
