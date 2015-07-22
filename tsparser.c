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

FILE* tsFp;

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

