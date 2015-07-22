/*
 *Project name: tsparser
 *File: tsparser.c
 *Author: Wagner
 *Date: 2015-7
 */

#include <stdio.h>
#include <stdlib.h>

#include <tsparser.h>
#include <utils.h>

#define PRINT_DEBUG

FILE* tsFp;
char* fileBuffer;
int fileBufferP;
int fileBufferLen;

char isFreqAvailable(int freq)
{
	char fileName[200];
	sprintf(fileName, "../analyseTS_data/ocn_%d.ts", freq);
	tsFp = fopen(fileName, "rb");
	printf("fp=%p\n", tsFp);
	if(tsFp != NULL)
	{
		fclose(tsFp);
		return TRUE;
	}
	else
		return FALSE;
}	

char startParse()
{
	fileBuffer = (char*)malloc(FILE_BUFFER_SIZE);	
}

char endParse()
{
	if(fileBuffer != NULL)
		free(fileBuffer);			
}

int openTSFile(int freq)
{
	char fileName[200];
	sprintf(fileName, "../analyseTS_data/ocn_%d.ts", freq);
	tsFp = fopen(fileName, "rb");
	if(tsFp != NULL)
	{
		return TRUE;
	}
	else
		return FALSE;

}

int closeTSFile()
{
	int ret = 0;

	if(tsFp != NULL)
		ret = fclose(tsFp);

	return ret;
}

int analyseTSPacket(char *buffer, int* bufferP, int bufferLen)
{
	int p = 0;
	int baseP = 0;
	

	if(*bufferP < 0 || bufferLen < 0 || buffer == NULL)
		return FALSE;

	p = *bufferP;

	while(p < bufferLen)
	{
		if(buffer[p + baseP] == 0x47)
		{
#ifdef PRINT_DEBUG
			printf("sync_byte found\n");
#endif
			baseP = p + baseP;
			p = 0;
		}



		p++;
	}

	return TRUE;
}

char parseTS(int freq)
{
	if(openTSFile(freq) != TRUE)
		return FALSE;

	while(1)
	{
		fileBufferP = 0;
		if((fileBufferLen = fread(fileBuffer, FILE_BUFFER_SIZE, 1, tsFp)) <= 0)
			break;
		
		if(!analyseTSPacket(fileBuffer, &fileBufferP, fileBufferLen))
		{
			break;	
		}

	}
	
	closeTSFile();

	return TRUE;
}
