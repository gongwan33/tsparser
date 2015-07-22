/*
 *Project name: tsparser
 *File: utils.c
 *Author: Wagner
 *Date: 2015-7
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <utils.h>
#include <tsparser.h>
#include <errnum.h>

void helpInfo()
{
	printf("-s[frequency]\t Search specific ts for program information\n-a[frequency]\t Get NIT and search all ts files by frequency info.\n-h\t Help information\n[number] Play specific program (shold search first to make sure which ts to play).\n");
}

char isNum(char* str)
{
	int i = 0;
	while(str[i])
	{
		if(!isdigit(str[i]))
			return FALSE;
		i++;
	}
	return TRUE;
}

void errInfo(int retval)
{
	switch(retval)
	{
		case NO_PARAM:
			helpInfo();
			break;
		case PARAM_ERR:
			printf("Param error! Please check your params!\n");
			helpInfo();
			break;
		case DAT_OPEN_ERR:
			printf("Can't open profile file! Maybe no permission or not set frequency first!\n");
			break;
		case MALLOC_ERR:
			printf("Memory error!\n");
			break;
		case FREQ_ERR:
			printf("Frequency not right! Please check!\n");
			break;
		default:
			printf("Unknown error!\n");
	}
}
