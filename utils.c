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


