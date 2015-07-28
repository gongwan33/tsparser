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
#include <errnum.h>

#define PRINT_DEBUG

static FILE* tsFp;
static FILE* outPutFp;

unsigned char* fileBuffer;
int fileBufferP;
int fileBufferLen;
unsigned char* packBuffer;
int packBufferP;
int packBufferLen;
char PATOkFlag = -1;
char PMTOkFlag = -1;

int programCount = 0;
int programPointer = 0;

int mapSectionLen = 0;
unsigned int lastPATVersion = 0;
unsigned int lastPMTVersion = 0;

extern int programWantToPlay;

char isFreqAvailable(int freq)
{
	char fileFreq[200];
	sprintf(fileFreq, "../analyseTS_data/ocn_%d.ts", freq);
	tsFp = fopen(fileFreq, "rb");
	if(tsFp != NULL)
	{
		fclose(tsFp);
		return TRUE;
	}
	else
		return FALSE;
}	

static void initProgramList()
{
	int i = 0, j = 0;

	for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
	{
		mapElm[i].PMT_PID = -1;
		for(j = 0; j < MAX_PER_PMT_NUMBER; j++)
		{	
			mapElm[i].mapPESElm[j] = NULL;
		}
		mapElm[i].mapNumber = 0;
	}
}

static int findInProgramList(unsigned int PID)
{
	int i = 0;

	for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
	{
		if(mapElm[i].PMT_PID == PID)
			return i;
	}

	return -1;
}

static int getProgramListPointer()
{
	int i = 0;

	for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
	{
		if(mapElm[i].PMT_PID == -1)
			return i;
	}

	return -1;
}

static char delProgramListPointer(int pointer)
{
	int i = 0;

	if(pointer < MAX_PROGRAM_NUMBER && pointer >= 0 && mapElm[pointer].PMT_PID != -1)
	{
		for(i = 0; i < mapElm[pointer].mapNumber; i++)
		{
			if(mapElm[pointer].mapPESElm[i] != NULL)
				free(mapElm[pointer].mapPESElm[i]);
		}
		mapElm[pointer].PMT_PID = -1;
		mapElm[pointer].mapNumber = 0;
	}

	return TRUE;
}

static void releaseProgramList()
{
	int i = 0, j = 0;

	for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
	{
		if(mapElm[i].PMT_PID != -1)
		{
			for(j = 0; j < mapElm[i].mapNumber; j++)
			{	
				if(mapElm[i].mapPESElm[j] != NULL)
					free(mapElm[i].mapPESElm[j]);
			}
			mapElm[i].PMT_PID = 0;
			mapElm[i].mapNumber = 0;
		}
		
	}
}

char startParse()
{
	fileBuffer = (unsigned char*)malloc(FILE_BUFFER_SIZE);	
	packBuffer = (unsigned char*)malloc(PACK_BUFFER_SIZE);

	initProgramList();
	memset(&header, 0, sizeof(header));

	if((outPutFp = fopen("./outPutProgram.ts", "wr+")) == NULL)
	{
		printf("Output file stream open error!\n");
		return FALSE;
	}

	return TRUE;
}

char endParse()
{
	if(outPutFp != NULL)
		fclose(outPutFp);

	if(fileBuffer != NULL)
		free(fileBuffer);			

	if(packBuffer != NULL)
		free(packBuffer);

	if(PATElm.programInfoElm != NULL)
		free(PATElm.programInfoElm);

	releaseProgramList();
}

int openTSFile(int freq)
{
	char fileFreq[200];
	sprintf(fileFreq, "../analyseTS_data/ocn_%d.ts", freq);
	tsFp = fopen(fileFreq, "rb");
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
	int i = 0;

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

		case 1:
			printf("---------------------------------------------\n");
			printf("PAT Info:\n");
			printf("table_id:\t 0x%x\n", PATElm.table_id);
			printf("section_syntax_indicator:\t 0x%x\n", PATElm.section_syntax_indicator);
			printf("section_length:\t 0x%x\n", PATElm.section_length);
			printf("transport_stream_id:\t 0x%x\n", PATElm.transport_stream_id);
			printf("version_number:\t 0x%x\n", PATElm.version_number);
			printf("current_next_indicator:\t 0x%x\n", PATElm.current_next_indicator);
			printf("section_number:\t 0x%x\n", PATElm.section_number);
			printf("last_section_number:\t 0x%x\n", PATElm.last_section_number);
			for(i = 0; i < programCount; i++)
			{
				printf("program_number:\t 0x%x\n", PATElm.programInfoElm[i].program_number);
				if(PATElm.programInfoElm -> program_number == 0)
					printf("network_PID:\t 0x%x\n", PATElm.programInfoElm[i].network_program_PID);
				else
					printf("program_map_PID:\t 0x%x\n", PATElm.programInfoElm[i].network_program_PID);
			}
				printf("---------------------------------------------\n");
			break;

		case 2:
			printf("---------------------------------------------\n");
			printf("PMT Info:\n");
			printf("table_id:\t 0x%x\n", PMTElm.table_id);
			printf("section_syntax_indicator:\t 0x%x\n", PMTElm.section_syntax_indicator);
			printf("section_length:\t 0x%x\n", PMTElm.section_length);
			printf("program_nubmer:\t 0x%x\n", PMTElm.program_number);
			printf("version_number:\t 0x%x\n", PMTElm.version_number);
			printf("current_next_indicator:\t 0x%x\n", PMTElm.current_next_indicator);
			printf("section_number:\t 0x%x\n", PMTElm.section_number);
			printf("last_section_number:\t 0x%x\n", PMTElm.last_section_number);
			printf("PCR_PID:\t 0x%x\n", PMTElm.PCR_PID);
			printf("program_info_length:\t 0x%x\n", PMTElm.program_info_length);
			printf("---------------------------------------------\n");
			break;

		case 3:
			printf("---------------------------------------------\n");
			printf("Program Info:\n");
			for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
			{
				if(mapElm[i].PMT_PID == -1)
					continue;

				int j = 0;

				printf("PMT_PID:\t 0x%x\n", mapElm[i].PMT_PID);
			printf("---------------------------------------------\n");
				for(j = 0; j < mapElm[i].mapNumber; j++)
				{
					printf("stream_type:\t 0x%x\n", mapElm[i].mapPESElm[j] -> stream_type);
					printf("elementary_PID:\t 0x%x\n", mapElm[i].mapPESElm[j] -> elementary_PID);
					printf("ES_info_length:\t 0x%x\n", mapElm[i].mapPESElm[j] -> ES_info_length);

				}
			printf("---------------------------------------------\n");
			}
			break;

		case 4:
			printf("---------------------------------------------\n");
			printf("PES Info:\n");
			printf("stream_id:\t 0x%x\n", PESElm.stream_id);
			printf("---------------------------------------------\n");
			break;

		default:
			printf("In printStructInfo: unknow flag!\n");
	}
}

static int isPMTPID(unsigned int PID)
{
	int i = 0;

	if(PATElm.programInfoElm == NULL)
		return FALSE;

	for(i = 0; i < programCount; i++)
	{
		if(PID == PATElm.programInfoElm[i].network_program_PID && PATElm.programInfoElm[i].program_number != 0)
			return TRUE;
	}

	return FALSE;
}

static int isPESPID(unsigned int PID, int* proNum)
{
	int i = 0, j = 0;

	for(i = 0; i < MAX_PROGRAM_NUMBER; i++)
	{
		if(mapElm[i].PMT_PID != -1)
		{
			for(j = 0; j < mapElm[i].mapNumber; j++)
			{	
				if(mapElm[i].mapPESElm[j] != NULL && mapElm[i].mapPESElm[j] -> elementary_PID == PID)
				{
					*proNum = mapElm[i].program_number;
					if(mapElm[i].mapPESElm[j] -> stream_type == 0x02)
					{
						return VIDEO_FLAG;
					}
					else if(mapElm[i].mapPESElm[j] -> stream_type == 0x04)
					{
						return AUDIO_FLAG;
					}
				}
			}
		}
		
	}
	return FALSE;
}

static int parsePAT(unsigned char* buffer, int bufferLen, int offset)
{
	int i = 0;

	if(bufferLen <= offset + 7)
		return FALSE;

	lastPATVersion = PATElm.version_number;

	PATElm.table_id = buffer[offset];
	PATElm.section_syntax_indicator = (buffer[offset + 1] & 0x80) >> 7;

	if(PATElm.section_syntax_indicator != 1)
		return FALSE;

	PATElm.section_length = ((buffer[offset + 1] & 0x0f) << 8) | (buffer[offset + 2] & 0xff);

	if(PATElm.section_length > 1021 || (PATElm.section_length & 0xc000) != 0 || PATElm.section_length + 3 > bufferLen)
		return FALSE;

	PATElm.transport_stream_id = (buffer[offset + 3] << 8) | buffer[offset + 4];
	PATElm.version_number = (buffer[offset + 5] & 0x3e) >> 1;
	PATElm.current_next_indicator = (buffer[offset + 5] & 0x01);
	PATElm.section_number = buffer[offset + 6];
	PATElm.last_section_number = buffer[offset + 7];

	if(PATOkFlag == -1)
		PATOkFlag = 1;

	if(lastPATVersion >= PATElm.version_number)
	{
		return TRUE;
	}

	programCount = (PATElm.section_length - 9) / 4;
	
	if(PATElm.programInfoElm == NULL)
		PATElm.programInfoElm = (struct programInfo *)malloc(sizeof(struct programInfo) * programCount);

	for(i = 0; i < programCount; i++)
	{
		PATElm.programInfoElm[i].program_number = (buffer[offset + i*4 + 8] << 8) | buffer[offset + i*4 + 9];
		PATElm.programInfoElm[i].network_program_PID = ((buffer[offset + i*4 + 10] & 0x01) << 8) | buffer[offset + i*4 + 11];
	}

//	printStructInfo(1);
	offset = offset + PATElm.section_length + 3 + 1;

	return TRUE;
}

static int parsePMT(unsigned int PID, unsigned char* buffer, int bufferLen, int offset)
{
	int i = 0, j = 0;
	int jump = 0;
	unsigned int skipLen = 0;

	if(bufferLen <= offset + 7)
		return FALSE;

	lastPMTVersion = PMTElm.version_number;

	PMTElm.table_id = buffer[offset];
	PMTElm.section_syntax_indicator = (buffer[offset + 1] & 0x80) >> 7;

	if(PMTElm.section_syntax_indicator != 1)
		return FALSE;

	PMTElm.section_length = ((buffer[offset + 1] & 0x0f) << 8) | (buffer[offset + 2] & 0xff);

	if(PMTElm.section_length <= 1021 && (PMTElm.section_length & 0xc000) != 0)
		return FALSE;

	PMTElm.program_number = (buffer[offset + 3] << 8) | buffer[offset + 4];
	PMTElm.version_number = (buffer[offset + 5] & 0x3e) >> 1;
	PMTElm.current_next_indicator = (buffer[offset + 5] & 0x01);
	PMTElm.section_number = buffer[offset + 6];
	PMTElm.last_section_number = buffer[offset + 7];
	PMTElm.PCR_PID = ((buffer[offset + 8] & 0x1f) << 8) | buffer[offset + 9];
	PMTElm.program_info_length = ((buffer[offset + 10] & 0xf) << 8) | buffer[offset + 11];

	if(PMTOkFlag == -1)
		PMTOkFlag = 1;

	mapSectionLen = PMTElm.section_length - 13 - PMTElm.program_info_length;

	if((programPointer = findInProgramList(PID)) == -1)
		programPointer = getProgramListPointer();
	else
	{
		delProgramListPointer(programPointer);
	}

	if(programPointer == -1)
		return PROGRAM_LIST_FULL_ERR;

	mapElm[programPointer].PMT_PID = PID;
	mapElm[programPointer].program_number = PMTElm.program_number;

	jump = offset + 12 + PMTElm.program_info_length;

	for(i = 0; i < mapSectionLen; i = i + skipLen)
	{
		mapElm[programPointer].mapPESElm[j] = (struct mapPES*) malloc(sizeof(struct mapPES));
		mapElm[programPointer].mapPESElm[j] -> stream_type = buffer[jump + i];
		mapElm[programPointer].mapPESElm[j] -> elementary_PID = ((buffer[jump + i + 1] & 0x1f) << 8) | buffer[jump + i + 2];
		mapElm[programPointer].mapPESElm[j] -> ES_info_length = ((buffer[jump + i + 3] & 0xf) << 8) | buffer[jump + i + 4];
		skipLen = 5 + mapElm[programPointer].mapPESElm[j] -> ES_info_length;  

		mapElm[programPointer].mapNumber = j + 1;

		j++;

		if(j > MAX_PER_PMT_NUMBER)
			break;
	}

	offset = offset + PMTElm.section_length + 3 + 1;

//	printStructInfo(3);
	return TRUE;
}

static int parsePES(unsigned char* buffer, int bufferLen, int offset, int flag)
{
	if(bufferLen <= offset + 7)
		return FALSE;

	PESElm.stream_id = buffer[offset + 3];
	PESElm.PES_packet_length = (buffer[offset + 4] <<  8)| buffer[offset + 5];
	offset += 6;

	if(PESElm.stream_id != program_stream_map && PESElm.stream_id != padding_stream \
		&& PESElm.stream_id != private_stream_2 && PESElm.stream_id != ECM \
		&& PESElm.stream_id != EMM && PESElm.stream_id != program_stream_directory \
		&& PESElm.stream_id != DSMCC_stream && PESElm.stream_id != ITU_T_Rec_H_222_1_type_E_stream)
	{
		PESElm.PES_scrambling_control = (buffer[offset] & 0x30) >> 4;
		PESElm.PES_priority = (buffer[offset] & 0x8) >> 3;
		PESElm.data_alignment_indicator = (buffer[offset] & 0x4) >> 2;
		PESElm.copyright = (buffer[offset] & 0x2) >> 1;
		PESElm.original_or_copy = (buffer[offset] & 0x1);
		PESElm.PTS_DTS_flags = (buffer[offset + 1] & 0xc0) >> 6;
		PESElm.ESCR_flag = (buffer[offset + 1] & 0x20) >> 5;
		PESElm.ES_rate_flag = (buffer[offset + 1] & 0x10) >> 4;
		PESElm.DSM_trick_mode_flag = (buffer[offset + 1] & 0x8) >> 3;
		PESElm.additional_copy_info_flag = (buffer[offset + 1] & 0x4) >> 2;
		PESElm.PES_CRC_flag = (buffer[offset + 1] & 0x2) >> 1;
		PESElm.PES_extension_flag = buffer[offset + 1] & 0x1;
		PESElm.PES_header_data_length = buffer[offset + 2];
		
		offset += 3;

		if(PESElm.PTS_DTS_flags == 0x2)
		{
			offset += 5;
		}

		if(PESElm.PTS_DTS_flags == 0x3)
		{
			offset += 10;
		}

		if(PESElm.ESCR_flag == 0x1)
		{
			offset += 6;
		}

		if(PESElm.ES_rate_flag == 0x1)
		{
			offset += 3;
		}

		if(PESElm.DSM_trick_mode_flag == 0x1)
		{
			PESElm.trick_mode_control = (buffer[offset] & 0xe0) >> 5;
			if(PESElm.trick_mode_control == fast_forward)
			{
			}
			else if(PESElm.trick_mode_control == slow_motion)
			{
			}
			else if(PESElm.trick_mode_control == freeze_frame)
			{
			}
			else if(PESElm.trick_mode_control == fast_reverse)
			{
			}
			else if(PESElm.trick_mode_control == slow_reverse)
			{
			}
			else
			{
			}

			offset += 1;
		}

		if(PESElm.additional_copy_info_flag == 0x1)
		{
			offset += 1;
		}

		if(PESElm.PES_CRC_flag == 0x1)
		{
			offset += 1;
		}

		if(PESElm.PES_extension_flag == 0x1)
		{
			PESElm.PES_private_data_flag = (buffer[offset] & 0x80) >> 7;
			PESElm.pack_header_field_flag = (buffer[offset] & 0x40) >> 6;
			PESElm.program_packet_sequence_counter_flag = (buffer[offset] & 0x20) >> 5;
			PESElm.P_STD_buffer_flag = (buffer[offset] & 0x10) >> 4;
			PESElm.PES_extension_flag_2 = buffer[offset] & 0x1;

			offset += 1;

			if(PESElm.PES_private_data_flag == 0x1)
			{
				offset += 16;
			}

			if(PESElm.pack_header_field_flag == 0x1)
			{
				PESElm.pack_field_length = buffer[offset];
				offset = offset + 1 + PESElm.pack_field_length;
			}

			if(PESElm.program_packet_sequence_counter_flag == 0x1)
			{
				offset += 2;
			}

			if(PESElm.P_STD_buffer_flag == 0x1)
			{
				offset += 2;
			}

			if(PESElm.PES_extension_flag_2 == 0x1)
			{
				PESElm.PES_extension_field_length = buffer[offset] & 0x7f;
				offset = offset + 1 + PESElm.PES_extension_field_length;
			}

		}
	}

	

//	offset = offset + PMTElm.section_length + 3 + 1;

	printStructInfo(4);
	return TRUE;
}

static int analysePacket(unsigned char* buffer, int bufferLen)
{
	int offset = 4;
	char audioVideoFlag = 0;
	int proNum = -1;

	if(bufferLen <= offset)
		return FALSE;

	header.sync_byte = buffer[0];
	header.transport_error_indicator = (buffer[1] & 0x80) >> 7;

	if(header.transport_error_indicator == 1)
		return FALSE;

	header.payload_unit_start_indicator = (buffer[1] & 0x40) >> 6;
	header.transport_priority = (buffer[1] & 0x20) >> 5;
	header.PID = ((buffer[1] & 0x1f) << 8) | buffer[2];
	header.transport_scrambling_control = (buffer[3] & 0xc0) >> 6;
	header.adaptation_field_control = (buffer[3] & 0x30) >> 4;
	header.continuity_counter = buffer[3] & 0x0f;

	if(header.PID != 0x0000 && PATOkFlag == -1)
	{
		return TRUE;
	}

/*	if(!isPMTPID(header.PID) && PMTOkFlag == -1 && PATOkFlag == 1)
	{
		return TRUE;
	}
*/

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

			default:
				if(isPMTPID(header.PID))
				{
					parsePMT(header.PID, buffer, bufferLen, offset);
				}

				audioVideoFlag = isPESPID(header.PID, &proNum);
				if(audioVideoFlag == AUDIO_FLAG)
				{
//					parsePES(buffer, bufferLen, offset, audioVideoFlag);
				}
				else if(audioVideoFlag == VIDEO_FLAG)
				{
					if(proNum == programWantToPlay)
						fwrite(buffer + offset, sizeof(char), bufferLen - offset, outPutFp);
				}
		}
	}	

	return TRUE;
}

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
			p = 188;
		}
		else	
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
