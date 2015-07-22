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
int freq = -1;
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
					freq = atoi(optarg);
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

	sprintf(progInfoBuffer, "[freq] %d\n", freq);	
	tmpDatLen = strlen(progInfoBuffer);

	if(freq > 0)
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

		char* freqFromFile;
		int tmpFreq = 0;
		freqFromFile = (char*)malloc(tmpDatLen + 1);
		fread(freqFromFile, tmpDatLen, 1, datFp);

		if(sscanf(freqFromFile, "[freq] %d\n", &tmpFreq) > 0)
			freq = tmpFreq;

		free(freqFromFile);
	}

	if(datFp != NULL)
		fclose(datFp);

	if(freq == -1)
	{
		retval = FREQ_ERR;
		goto EXIT;
	}

	printf("Freqency is %d\n", freq);

//---------------------Start to parse ts---------------------
	startParse();

	if(!parseTS(freq))
	{
		retval = FREQ_ERR;
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
