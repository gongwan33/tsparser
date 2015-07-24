/*
 *Project name: tsparser
 *File: tsparser.c
 *Author: Wagner
 *Date: 2015-7
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tsparser.h>
#include <utils.h>

#define PRINT_DEBUG

FILE* tsFp;
unsigned char* fileBuffer;
int fileBufferP;
int fileBufferLen;
unsigned char* packBuffer;
int packBufferP;
int packBufferLen;
char PATOkFlag = -1;
char PMTOkFlag = -1;

char isFreqAvailable(int freq)
{
	char fileName[200];
	sprintf(fileName, "../analyseTS_data/ocn_%d.ts", freq);
	tsFp = fopen(fileName, "rb");
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
	fileBuffer = (unsigned char*)malloc(FILE_BUFFER_SIZE);	
	packBuffer = (unsigned char*)malloc(PACK_BUFFER_SIZE);

	memset(&header, 0, sizeof(header));
}

char endParse()
{
	if(fileBuffer != NULL)
		free(fileBuffer);			

	if(packBuffer != NULL)
		free(packBuffer);
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

static void printStructInfo(int flag)
{
	switch(flag)
	{
		case 0:
			printf("---------------------------------------------\n");
			printf("Header Info:\n");
			printf("sync_byte:\t 0x%x\n", header.sync_byte);
			printf("transport_error_indicator:\t 0x%x\n", header.transport_error_indicator);
			printf("payload_unit_start_indicator:\t 0x%x\n", header.payload_unit_start_indicator);
			printf("transport_priority:\t 0x%x\n", header.transport_priority);
			printf("PID:\t 0x%x\n", header.PID);
			printf("transport_scrambling_control:\t 0x%x\n", header.transport_scrambling_control);
			printf("adaptation_field_control:\t 0x%x\n", header.adaptation_field_control);
			printf("continuity_counter:\t 0x%x\n", header.continuity_counter);
			printf("---------------------------------------------\n");
			break;

		default:
			printf("In printStructInfo: unknow flag!\n");
	}
}

static int parsePAT(unsigned char* buffer, int bufferLen, int offset)
{
	if(bufferLen <= offset + 7)
		return FALSE;

	PATElm.table_id = buffer[offset];
	PATElm.section_syntax_indicator = buffer[offset + 1] & 0x80;
	PATElm.section_length = (buffer[offset + 1] & 0x0f) | (buffer[offset + 2] & 0xff);
	PATElm.transport_stream_id = buffer[offset + 3] | buffer[offset + 4];
	PATElm.version_number = (buffer[offset + 5] & 0x3e) >> 1;
	PATElm.current_next_indicator = (buffer[offset + 5] & 0x01);
	PATElm.section_number = buffer[offset + 6];
	PATElm.last_section_number = buffer[offset + 7];

	return TRUE;
}

static int analysePacket(unsigned char* buffer, int bufferLen)
{
	int offset = 4;

	if(bufferLen <= offset)
		return FALSE;

	header.sync_byte = buffer[0];
	header.transport_error_indicator = (buffer[1] & 0x80) >> 7;
	header.payload_unit_start_indicator = (buffer[1] & 0x40) >> 6;
	header.transport_priority = (buffer[1] & 0x20) >> 5;
	header.PID = (buffer[1] & 0x1f) << 8 | buffer[2];
	header.transport_scrambling_control = (buffer[3] & 0xc0) >> 6;
	header.adaptation_field_control = (buffer[3] & 0x30) >> 4;
	header.continuity_counter = buffer[3] & 0x0f;


	if(header.PID != 0x0000 && PATOkFlag == -1)
		return TRUE;

	if(header.adaptation_field_control == 0x2 || header.adaptation_field_control == 0x3)
	{
		if(bufferLen <= offset + 1)
			return FALSE;

		aField.adaptation_field_length = buffer[offset];

		offset = offset + aField.adaptation_field_length + 1;
	}

	if(header.adaptation_field_control == 0x1 || header.adaptation_field_control == 0x3)
	{
		if(header.payload_unit_start_indicator == 0x1)
		{
			if(bufferLen <= offset + 1)
				return FALSE;
			
			pointerFieldElm.pointer_field = buffer[offset];		

			offset = offset + 1 + pointerFieldElm.pointer_field;			
		}

		switch(header.PID)
		{
			case 0x0000:
			parsePAT(buffer, bufferLen, offset);	
			break;

		}
	}	

	return TRUE;
}
//-------------------------------T
int i = 0;
//--------------------------------
static int cutTSPacket(unsigned char* buffer, int* bufferP, int bufferLen)
{
	int p = 0;
	int baseP = 0;
	
	if(*bufferP < 0 || bufferLen < 0 || buffer == NULL)
		return FALSE;

	while(p + baseP < bufferLen)
	{
		if(buffer[p + baseP] == 0x47)
		{
			if(p < PACK_BUFFER_SIZE)
			{
				memcpy(packBuffer, buffer + baseP, p);
				analysePacket(packBuffer, p);
#ifdef PRINT_DEBUG
//			printStructInfo(0);
#endif
			}
			else
				printf("Special pack size!\n");


			baseP = p + baseP;
			p = 0;
//------------------------------------------------------------T
i++;
if(i > 1000)
break;
//-------------------------------------------------------------
		}
		
		p++;
	}

	*bufferP = baseP;	
	return TRUE;
}

char parseTS(int freq)
{
	int readLen = 0;

	if(openTSFile(freq) != TRUE)
		return FALSE;

	while(1)
	{
		if((readLen = fread(fileBuffer + fileBufferP, sizeof(unsigned char), FILE_BUFFER_SIZE - fileBufferP, tsFp)) <= 0)
			break;

		fileBufferLen = readLen + fileBufferP;
		if(!cutTSPacket(fileBuffer, &fileBufferP, fileBufferLen))
		{
			break;	
		}
		
		memcpy(fileBuffer, fileBuffer + fileBufferP, fileBufferLen - fileBufferP);
		fileBufferP = fileBufferLen - fileBufferP;
	}
	
	closeTSFile();

	return TRUE;
}
