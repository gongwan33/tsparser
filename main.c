/*
 *Project name: tsparser
 *File: main.c
 *Author: Wagner
 *Date: 2015-7
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <tsparser.h>
#include <utils.h>
#include <errnum.h>

#define PROG_INFO_BUFFER_SIZE 1024*4

int programNum = -1;
char fileName[200];
char* progInfoBuffer;
int name = -1;
int retval = 0;

FILE* datFp;
FILE* targetTsFp;

int main(int argc, char** argv)
{
//-------------------------------Analyse params---------------------------
	int ch;

	if(argc <= 1)
	{
		retval = NO_PARAM;
		goto EXIT;
	}
	else if(argv[1][0] != '-' && isNum(argv[1]))
	{
		programNum = atoi(argv[1]);
	}
	else if(argv[1][0] != '-' && !isNum(argv[1]))
	{
		retval = NO_PARAM;
		goto EXIT;
	}

	while((ch = getopt(argc, argv, "s:a:h")) != -1)
	{
		switch(ch)
		{
			case 's':
				if(isNum(optarg))
					name = atoi(optarg);
				else
				{
					retval = PARAM_ERR;
					goto EXIT;
				}

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


//----------------------------Init memory----------------------------
    progInfoBuffer = (char *)malloc(PROG_INFO_BUFFER_SIZE);
	if(progInfoBuffer == NULL)
	{
		printf("Erro occurred! Maybe lack of memory.");
		return MALLOC_ERR;
	}

//---------------------------Get dat from profile file----------------

	int tmpDatLen = 0;

	sprintf(progInfoBuffer, "[name] %d\n", name);	
	tmpDatLen = strlen(progInfoBuffer);

	if(name > 0)
	{
		if((datFp = fopen("tmp.dat", "w+")) == NULL)
		{
			retval = DAT_OPEN_ERR;
			goto EXIT;
		}

		fwrite(progInfoBuffer, tmpDatLen,1, datFp);
	}
	else
	{
		if((datFp = fopen("tmp.dat", "r")) == NULL)
		{
			retval = DAT_OPEN_ERR;
			goto EXIT;
		}

		char* nameFromFile;
		int tmpName = 0;
		nameFromFile = (char*)malloc(tmpDatLen + 1);
		fread(nameFromFile, tmpDatLen, 1, datFp);

		if(sscanf(nameFromFile, "[name] %d\n", &tmpName) > 0)
			name = tmpName;

		free(nameFromFile);
	}

	if(datFp != NULL)
		fclose(datFp);

	if(name == -1)
	{
		retval = NAME_ERR;
		goto EXIT;
	}

	printf("Name is %d\n", name);

//---------------------Start to parse ts---------------------
	startParse();

	if(!parseTS(name))
	{
		retval = NAME_ERR;
		goto EXIT;
	}

EXIT:
	endParse();

	if(progInfoBuffer != NULL)
		free(progInfoBuffer);

	if(retval != 0)
		errInfo(retval);

	return retval;	
}
