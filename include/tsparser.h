/*
 *Project name: tsparser
 *File: tsparser.h
 *Author: Wagner
 *Date: 2015-7
 */

#ifndef TSPARSER_H
#define TSPARSER_H

#define TRUE 1
#define FALSE 0

#define FILE_BUFFER_SIZE 10*1024

char isFreqAvailable(int freq);
char startParse();
char endParse();
int openTSFile(int freq);
int closeTSFile();
char parseTS(int freq);

struct tsHeader
{
	char sync_byte:									8;
	unsigned int transport_error_indicator:			1;
	unsigned int payload_unit_start_indicator:		1;
	unsigned int transport_priority:				1;
	unsigned int PID:								13;
	unsigned int transport_scrambling_control:		2;
	unsigned int adaptation_field_control:			2;
	unsigned int continuity_counter:				4;	
} header;

#endif




