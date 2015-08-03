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

static unsigned char* fileBuffer;
static int fileBufferP;
static int fileBufferLen;
static char PATOkFlag = -1;
static char PMTOkFlag = -1;

static int programCount = 0;
static int programPointer = 0;

static int mapSectionLen = -1;
static int lastPATVersion = -1;
static int lastPMTVersion = -1;
static int lastSDTVersionActual = -1;
static int lastSDTVersionOther = -1;
static int lastNITVersionActual = -1;
static int lastNITVersionOther = -1;

static int NITSectionCount = 0;
static unsigned int currentFreq = 0;

static int freqList[FREQ_LIST_SIZE];
static int scannedList[FREQ_LIST_SIZE];

extern int programWantToPlay;

char isFreqAvailable(int freq)
{
	FILE *testFp;
	char fileFreq[200];
	sprintf(fileFreq, "../analyseTS_data/ocn_%d.ts", freq);
	testFp = fopen(fileFreq, "rb");
	if(testFp != NULL)
	{
		fclose(testFp);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}	

static void listInit(int* list, int value)
{
	int i = 0;
	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		*(list + i) = value;
	}
}

static char addFreq2List(int* list, int freq)
{
	int i = 0;

	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		if(*(list + i) == freq)
		{
			return TRUE;
		}
	}

	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		if(*(list + i) == -1)
		{
			*(list + i) = freq;
		}
		return TRUE;
	}
	return FALSE;
}

static void delFreq2List(int* list, int freq)
{
	int i = 0;

	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		if(*(list + i) == freq)
		{
			*(list + i) = -1;
		}
	}
}

static int findInList(int* list, int freq)
{
	int i = 0;

	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		if(*(list + i) == freq)
		{
			return i;
		}
	}

	return -1;
}

static int popList(int* list)
{
	int i = 0;
	int ret = -1;

	for(i = 0; i < FREQ_LIST_SIZE; i++)
	{
		if(*(list + i) != -1)
		{
			ret = *(list + i);
			*(list + i) = -1;
			return ret;
		}
	}

	return ret;
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
			{
				free(mapElm[pointer].mapPESElm[i]);
				mapElm[pointer].mapPESElm[i] = NULL;
			}
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
				{
					free(mapElm[i].mapPESElm[j]);
					mapElm[i].mapPESElm[j] = NULL;
				}
			}
			mapElm[i].PMT_PID = 0;
			mapElm[i].mapNumber = 0;
		}
		
	}
}

static void releaseServiceList()
{
	int i = 0, j = 0;

	for(i = 0; i < MAX_SERVICE_NUMBER; i++)
	{
		for(j = 0; j < 2; j++)
			if(SDTElm[j].serviceTab[i] != NULL )
			{
				free(SDTElm[j].serviceTab[i]);
				SDTElm[j].serviceTab[i] = NULL;
			}
		
	}
}

static void initTmpSpace()
{
	int i = 0;

	for(i = 0; i < TMP_SPACE_LIST_SIZE; i++)
	{
		tmpSpaceList[i].emptyFlag = -1;
		tmpSpaceList[i].PID = 0;
		tmpSpaceList[i].length = 0;		
	}
}

static void releaseTmpSpace()
{
	int i = 0;

	for(i = 0; i < TMP_SPACE_LIST_SIZE; i++)
	{
		tmpSpaceList[i].emptyFlag = -1;
		tmpSpaceList[i].length = 0;		
	}
}

static int getTmpSpace()
{
	int i = 0;

	for(i = 0; i < TMP_SPACE_LIST_SIZE; i++)
	{
		if(tmpSpaceList[i].emptyFlag == -1)
		{
			return i;
		}
	}	

	return -1;
}

static int findTmpSpaceByPID(unsigned int PID)
{
	int i = 0;

	for(i = 0; i < TMP_SPACE_LIST_SIZE; i++)
	{
		if(tmpSpaceList[i].PID == PID && tmpSpaceList[i].emptyFlag != -1)
			return i;
	}

	return -1;
}

static char startParse()
{

	fileBuffer = (unsigned char*)malloc(FILE_BUFFER_SIZE);	

	initProgramList();
	initTmpSpace();
	memset(&header, 0, sizeof(header));

	if((outPutFp = fopen("./outPutProgram.ts", "wr+")) == NULL)
	{
		printf("Output file stream open error!\n");
		return FALSE;
	}

	listInit(scannedList, -1);
	listInit(freqList, -1);
	return TRUE;
}

static void initValue()
{
	fileBufferP = 0;
	fileBufferLen = 0;
	PATOkFlag = -1;
	PMTOkFlag = -1;

	lastPATVersion = -1;
	lastPMTVersion = -1;
	lastSDTVersionActual = -1;
	lastSDTVersionOther = -1;
	lastNITVersionActual = -1;
	lastNITVersionOther = -1;

	NITSectionCount = 0;
}

static char endParse()
{
	if(outPutFp != NULL)
	{
		fclose(outPutFp);
		outPutFp = NULL;
	}

	if(fileBuffer != NULL)
	{
		free(fileBuffer);
		fileBuffer = NULL;
	}

	if(PATElm.programInfoElm != NULL)
	{
		free(PATElm.programInfoElm);
		PATElm.programInfoElm = NULL;
	}

	releaseProgramList();
	releaseServiceList();
	releaseTmpSpace();
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
	int i = 0, j = 0;

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

		case 5:
			for(j = 0; j < 2; j++)
			{
				printf("---------------------------------------------\n");
				printf("SDT Info:\n");
				printf("table_id:\t 0x%x\n", SDTElm[j].table_id);
				printf("section_syntax_indicator:\t 0x%x\n", SDTElm[j].section_syntax_indicator);
				printf("section_length:\t %d\n", SDTElm[j].section_length);
				printf("transport_stream_id:\t 0x%x\n", SDTElm[j].transport_stream_id);
				printf("version_number:\t 0x%x\n", SDTElm[j].version_number);
				printf("current_next_indicator:\t 0x%x\n", SDTElm[j].current_next_indicator);
				printf("section_number:\t 0x%x\n", SDTElm[j].section_number);
				printf("last_section_number:\t 0x%x\n", SDTElm[j].last_section_number);
				printf("original_network_id:\t 0x%x\n", SDTElm[j].original_network_id);

				for(i = 0; i < SDTElm[j].serviceTableNum; i++)
				{
					printf("service_id:\t 0x%x\n", SDTElm[j].serviceTab[i]->service_id);
					printf("descriptors_loop_length:\t 0x%x\n", SDTElm[j].serviceTab[i]->descriptors_loop_length);
					int m = 0;
					struct multilingual_service_name_desp_content* msdc =  SDTElm[j].serviceTab[i]->msdc;
					for(m = 0; m < SDTElm[j].serviceTab[i]->service_count; m++)
					{
						printf("******************************\n");
						printf("\t language code:\t 0x%x\n", msdc[m].ISO_639_language_code);
						printf("\t service provider name length:\t 0x%x\n", msdc[m].service_provider_name_length);
						printf("\t service provider:\t %s\n", msdc[m].service_provider);
						printf("\t service name:\t %s\n", msdc[m].service_name);
						printf("******************************\n");
					}
				}
				printf("---------------------------------------------\n");
			}
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

static int parsePAT(unsigned char* buffer, int bufferLen, int* offset)
{
	int i = 0;

	if(bufferLen <= *offset + 7)
		return FALSE;

	lastPATVersion = PATElm.version_number;

	PATElm.table_id = buffer[*offset];
	PATElm.section_syntax_indicator = (buffer[*offset + 1] & 0x80) >> 7;

	if(PATElm.section_syntax_indicator != 1)
		return FALSE;

	PATElm.section_length = ((buffer[*offset + 1] & 0x0f) << 8) | (buffer[*offset + 2] & 0xff);

	if(PATElm.section_length > 1021 || (PATElm.section_length & 0xc000) != 0 || PATElm.section_length + 3 > bufferLen)
	{
		return FALSE;
	}

	PATElm.transport_stream_id = (buffer[*offset + 3] << 8) | buffer[*offset + 4];
	PATElm.version_number = (buffer[*offset + 5] & 0x3e) >> 1;
	PATElm.current_next_indicator = (buffer[*offset + 5] & 0x01);
	PATElm.section_number = buffer[*offset + 6];
	PATElm.last_section_number = buffer[*offset + 7];

	if(PATOkFlag == -1)
		PATOkFlag = 1;

	if(lastPATVersion >= PATElm.version_number)
	{
		*offset = *offset + PATElm.section_length + 3 + 1;
		return TRUE;
	}

	programCount = (PATElm.section_length - 9) / 4 + 1;
	
	if(PATElm.programInfoElm == NULL)
		PATElm.programInfoElm = (struct programInfo *)malloc(sizeof(struct programInfo) * programCount);
	else
	{
		free(PATElm.programInfoElm);
		PATElm.programInfoElm = (struct programInfo *)malloc(sizeof(struct programInfo) * programCount);
	}

	for(i = 0; i < programCount; i++)
	{
		PATElm.programInfoElm[i].program_number = (buffer[*offset + i*4 + 8] << 8) | buffer[*offset + i*4 + 9];
		PATElm.programInfoElm[i].network_program_PID = ((buffer[*offset + i*4 + 10] & 0x01) << 8) | buffer[*offset + i*4 + 11];
	}

//	printStructInfo(1);
	*offset = *offset + PATElm.section_length + 3 + 1;

	return TRUE;
}

static int parsePMT(unsigned int PID, unsigned char* buffer, int bufferLen, int* offset)
{
	int i = 0, j = 0;
	int jump = 0;
	unsigned int skipLen = 0;

	if(bufferLen <= *offset + 7)
		return FALSE;

	lastPMTVersion = PMTElm.version_number;

	if(lastPATVersion >= PATElm.version_number)
	{
		return FALSE;
	}

	PMTElm.table_id = buffer[*offset];
	PMTElm.section_syntax_indicator = (buffer[*offset + 1] & 0x80) >> 7;

	if(PMTElm.section_syntax_indicator != 1)
		return FALSE;

	PMTElm.section_length = ((buffer[*offset + 1] & 0x0f) << 8) | buffer[*offset + 2];

	if(PMTElm.section_length <= 1021 && (PMTElm.section_length & 0xc000) != 0)
	{
		*offset = *offset + PMTElm.section_length + 3 + 1;
		return FALSE;
	}

	PMTElm.program_number = (buffer[*offset + 3] << 8) | buffer[*offset + 4];
	PMTElm.version_number = (buffer[*offset + 5] & 0x3e) >> 1;
	PMTElm.current_next_indicator = (buffer[*offset + 5] & 0x01);
	PMTElm.section_number = buffer[*offset + 6];
	PMTElm.last_section_number = buffer[*offset + 7];
	PMTElm.PCR_PID = ((buffer[*offset + 8] & 0x1f) << 8) | buffer[*offset + 9];
	PMTElm.program_info_length = ((buffer[*offset + 10] & 0xf) << 8) | buffer[*offset + 11];

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

	jump = *offset + 12 + PMTElm.program_info_length;

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

	*offset = *offset + PMTElm.section_length + 3 + 1;

//	printStructInfo(2);
	return TRUE;
}

static int parsePES(unsigned char* buffer, int bufferLen, int* offset, int flag)
{
	if(bufferLen <= *offset + 7)
		return FALSE;

	PESElm.stream_id = buffer[*offset + 3];
	PESElm.PES_packet_length = (buffer[*offset + 4] <<  8)| buffer[*offset + 5];
	*offset += 6;

	if(PESElm.stream_id != program_stream_map && PESElm.stream_id != padding_stream \
		&& PESElm.stream_id != private_stream_2 && PESElm.stream_id != ECM \
		&& PESElm.stream_id != EMM && PESElm.stream_id != program_stream_directory \
		&& PESElm.stream_id != DSMCC_stream && PESElm.stream_id != ITU_T_Rec_H_222_1_type_E_stream)
	{
		PESElm.PES_scrambling_control = (buffer[*offset] & 0x30) >> 4;
		PESElm.PES_priority = (buffer[*offset] & 0x8) >> 3;
		PESElm.data_alignment_indicator = (buffer[*offset] & 0x4) >> 2;
		PESElm.copyright = (buffer[*offset] & 0x2) >> 1;
		PESElm.original_or_copy = (buffer[*offset] & 0x1);
		PESElm.PTS_DTS_flags = (buffer[*offset + 1] & 0xc0) >> 6;
		PESElm.ESCR_flag = (buffer[*offset + 1] & 0x20) >> 5;
		PESElm.ES_rate_flag = (buffer[*offset + 1] & 0x10) >> 4;
		PESElm.DSM_trick_mode_flag = (buffer[*offset + 1] & 0x8) >> 3;
		PESElm.additional_copy_info_flag = (buffer[*offset + 1] & 0x4) >> 2;
		PESElm.PES_CRC_flag = (buffer[*offset + 1] & 0x2) >> 1;
		PESElm.PES_extension_flag = buffer[*offset + 1] & 0x1;
		PESElm.PES_header_data_length = buffer[*offset + 2];
		
		*offset += 3;

		if(PESElm.PTS_DTS_flags == 0x2)
		{
			*offset += 5;
		}

		if(PESElm.PTS_DTS_flags == 0x3)
		{
			*offset += 10;
		}

		if(PESElm.ESCR_flag == 0x1)
		{
			*offset += 6;
		}

		if(PESElm.ES_rate_flag == 0x1)
		{
			*offset += 3;
		}

		if(PESElm.DSM_trick_mode_flag == 0x1)
		{
			PESElm.trick_mode_control = (buffer[*offset] & 0xe0) >> 5;
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

			*offset += 1;
		}

		if(PESElm.additional_copy_info_flag == 0x1)
		{
			*offset += 1;
		}

		if(PESElm.PES_CRC_flag == 0x1)
		{
			*offset += 1;
		}

		if(PESElm.PES_extension_flag == 0x1)
		{
			PESElm.PES_private_data_flag = (buffer[*offset] & 0x80) >> 7;
			PESElm.pack_header_field_flag = (buffer[*offset] & 0x40) >> 6;
			PESElm.program_packet_sequence_counter_flag = (buffer[*offset] & 0x20) >> 5;
			PESElm.P_STD_buffer_flag = (buffer[*offset] & 0x10) >> 4;
			PESElm.PES_extension_flag_2 = buffer[*offset] & 0x1;

			*offset += 1;

			if(PESElm.PES_private_data_flag == 0x1)
			{
				*offset += 16;
			}

			if(PESElm.pack_header_field_flag == 0x1)
			{
				PESElm.pack_field_length = buffer[*offset];
				*offset = *offset + 1 + PESElm.pack_field_length;
			}

			if(PESElm.program_packet_sequence_counter_flag == 0x1)
			{
				*offset += 2;
			}

			if(PESElm.P_STD_buffer_flag == 0x1)
			{
				*offset += 2;
			}

			if(PESElm.PES_extension_flag_2 == 0x1)
			{
				PESElm.PES_extension_field_length = buffer[*offset] & 0x7f;
				*offset = *offset + 1 + PESElm.PES_extension_field_length;
			}

		}
	}

	

	*offset = *offset + PMTElm.section_length + 3 + 1;

//	printStructInfo(4);
	return TRUE;
}

static int parseDescriptor(unsigned char* buffer, unsigned int bufferLen, int actualOtherFlag, int stIndex)
{
	int i = 0, j = 0, m = 0;
	unsigned char desp_tag;
	unsigned int desp_length = 0;
	struct multilingual_service_name_desp_content* msdc = SDTElm[actualOtherFlag].serviceTab[stIndex]->msdc; 
	unsigned char servicePLen = 0, serviceNLen = 0;
	int count = 0;
	double frequency = 0;

	for(i = 0; i < bufferLen; i = i + desp_length + 2)
	{
		desp_tag = buffer[i];
		desp_length = buffer[i + 1];
		switch(desp_tag)
		{
			case multilingual_service_name_descriptor:
				SDTElm[actualOtherFlag].serviceTab[stIndex]->service_count = 0;
				for(m = 0; m < desp_length; m = m + servicePLen + serviceNLen + 5 )
				{
					msdc[j].ISO_639_language_code = (buffer[m + i + 2] << 16) | (buffer[m + i + 3] << 8) | (buffer[m + i + 4]);
					servicePLen = buffer[m + i + 5];
					msdc[j].service_provider_name_length = servicePLen;

					if(msdc[j].service_provider == NULL)
						msdc[j].service_provider = (char*)malloc(msdc[j].service_provider_name_length + 1);
					memcpy(msdc[j].service_provider, buffer + m + i + 6, msdc[j].service_provider_name_length);

					*(msdc[j].service_provider + msdc[j].service_provider_name_length) = '\0';

					serviceNLen = buffer[m + i + 6 + servicePLen];
					msdc[j].service_name_length = serviceNLen;

					if(msdc[j].service_name == NULL)
						msdc[j].service_name = (char*)malloc(msdc[j].service_name_length + 1);
					memcpy(msdc[j].service_name, buffer + m + i + 7 + servicePLen, msdc[j].service_name_length);

					*(msdc[j].service_name + msdc[j].service_name_length) = '\0';

					SDTElm[actualOtherFlag].serviceTab[stIndex]->service_count++;

					j++;
				}
				break;

			case cable_delivery_system_descriptor:
				frequency = ((buffer[i + 2] & 0xf0) >> 4)*1000 + (buffer[i + 2] & 0xf)*100 + ((buffer[i + 3] & 0xf0) >> 4)*10 + (buffer[i + 3] & 0xf) + ((buffer[i + 4] & 0xf0) >> 4)*0.1 + (buffer[i + 4] & 0xf)*0.01 + ((buffer[i + 5] & 0xf0) >> 4)*0.001 + (buffer[i + 5] & 0xf)*0.0001;

				if(currentFreq != (int)frequency\
					&& isFreqAvailable((int)frequency))
				{
					if(-1 == findInList(scannedList, frequency)\
						&& -1 == findInList(freqList, frequency))
					{
						addFreq2List(freqList, frequency);
						printf("Get valid frequency %f\n", frequency);
					}
				}

				break;
		}
	}

	return TRUE;
}

static int parseTranStreams(unsigned char* buffer, unsigned int bufferLen, int actualOtherFlag, int stIndex)
{
	int i = 0, j = 0, m = 0;
	unsigned char streamId;
	unsigned int streamLen = 0;
	int count = 0;

	for(i = 0; i < bufferLen; i = i + streamLen + 6)
	{
		streamId = (buffer[i] << 8) | buffer[i + 1];
		streamLen = ((buffer[i + 4] & 0xf) << 8) | buffer[i + 5];
		parseDescriptor(buffer + i + 6, streamLen, actualOtherFlag, stIndex);
	}

	return TRUE;
}

static int parseSI(unsigned char* buffer, int bufferLen, int* offset)
{
	int i = 0, j = 0;
	int jump = 0;
	int spacePos = 0;
	int versionNumber = 0;
	unsigned int skipLen = 0;
	unsigned char table_id;
	unsigned sectionLen = 0;
	int actualOtherFlag = 1;

	if(bufferLen <= *offset + 7)
		return FALSE;

	if(header.payload_unit_start_indicator == 0x1)
	{
		table_id = buffer[*offset];
		switch(table_id)
		{
			case service_description_section_actual:
				actualOtherFlag = 0;
			case service_description_section_other:

				versionNumber = (buffer[*offset + 5] & 0x3e) >> 1; 
				if((table_id == service_description_section_actual && versionNumber <= lastSDTVersionActual) \
						|| (table_id == service_description_section_other && versionNumber <= lastSDTVersionOther))
					return TRUE;

				SDTElm[actualOtherFlag].table_id = table_id;	
				SDTElm[actualOtherFlag].section_syntax_indicator = (buffer[*offset + 1] & 0x80) >> 7;
				SDTElm[actualOtherFlag].section_length = ((buffer[*offset + 1] & 0xf) << 8) | buffer[*offset + 2];
				SDTElm[actualOtherFlag].transport_stream_id = (buffer[*offset + 3] << 8) | buffer[*offset + 4];
				SDTElm[actualOtherFlag].version_number = versionNumber; 
				SDTElm[actualOtherFlag].current_next_indicator = buffer[*offset + 5] & 0x1;
				SDTElm[actualOtherFlag].section_number = buffer[*offset + 6];
				SDTElm[actualOtherFlag].last_section_number = buffer[*offset + 7];
				SDTElm[actualOtherFlag].original_network_id = (buffer[*offset + 8] << 8) | buffer[*offset + 9];

				if(SDTElm[actualOtherFlag].section_length <= TS_PACK_SIZE - 3)
				{

					if(table_id == service_description_section_actual)
						lastSDTVersionActual = SDTElm[actualOtherFlag].version_number;
					else if(table_id == service_description_section_other)
						lastSDTVersionOther = SDTElm[actualOtherFlag].version_number;

					sectionLen = SDTElm[actualOtherFlag].section_length - 12;

					jump = *offset + 11;

					SDTElm[actualOtherFlag].serviceTableNum = 0;

					j = 0;

					for(i = 0; i < sectionLen; i = i + skipLen)
					{
						if(SDTElm[actualOtherFlag].serviceTab[j] == NULL)
							SDTElm[actualOtherFlag].serviceTab[j] = (struct serviceTable*) malloc(sizeof(struct serviceTable));

						SDTElm[actualOtherFlag].serviceTab[j] -> service_id = (buffer[jump + i] << 8) | buffer[jump + 1 + i];
						SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length = ((buffer[jump + i + 3] & 0xf) << 8) | buffer[jump + i + 4];
						SDTElm[actualOtherFlag].serviceTableNum++;
						
						parseDescriptor(buffer + jump + i + 5, SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length, actualOtherFlag, j);
						skipLen = 5 + SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length;  

						j++;
					}

				}
				else
				{
					if((spacePos = getTmpSpace()) != -1)
					{
						memcpy(tmpSpaceList[spacePos].space, buffer + *offset, bufferLen - *offset);
						tmpSpaceList[spacePos].length = bufferLen - *offset;
						tmpSpaceList[spacePos].section_length = SDTElm[actualOtherFlag].section_length;
						tmpSpaceList[spacePos].PID = header.PID;
						tmpSpaceList[spacePos].continuity_counter = header.continuity_counter;
						tmpSpaceList[spacePos].emptyFlag = 1;
					}
					else
					{
						printf("Error: no empty space left!");
						return FALSE;
					}
				}
				break;
		}

	}

	if(header.payload_unit_start_indicator == 0)
	{
		if((spacePos = findTmpSpaceByPID(header.PID)) != -1)
		{
			if(tmpSpaceList[spacePos].length + bufferLen - *offset >= tmpSpaceList[spacePos].section_length)
			{
				memcpy(tmpSpaceList[spacePos].space + tmpSpaceList[spacePos].length, buffer + *offset, bufferLen - *offset);

				table_id = tmpSpaceList[spacePos].space[0];

				versionNumber = (tmpSpaceList[spacePos].space[5] & 0x3e) >> 1; 

				if((table_id == service_description_section_actual && versionNumber <= lastSDTVersionActual) \
						|| (table_id == service_description_section_other && versionNumber <= lastSDTVersionOther))
				{
					tmpSpaceList[spacePos].emptyFlag = -1;
					return TRUE;
				}

				switch(table_id)
				{
					case service_description_section_actual:
						actualOtherFlag = 0;
					case service_description_section_other:

						SDTElm[actualOtherFlag].table_id = table_id;	
						SDTElm[actualOtherFlag].section_syntax_indicator = (tmpSpaceList[spacePos].space[1] & 0x80) >> 7;
						SDTElm[actualOtherFlag].section_length = ((tmpSpaceList[spacePos].space[1] & 0xf) << 8) | tmpSpaceList[spacePos].space[2];
						SDTElm[actualOtherFlag].transport_stream_id = (tmpSpaceList[spacePos].space[3] << 8) | tmpSpaceList[spacePos].space[4];
						SDTElm[actualOtherFlag].version_number = versionNumber;
						SDTElm[actualOtherFlag].current_next_indicator = tmpSpaceList[spacePos].space[5] & 0x1;
						SDTElm[actualOtherFlag].section_number = tmpSpaceList[spacePos].space[6];
						SDTElm[actualOtherFlag].last_section_number = tmpSpaceList[spacePos].space[7];
						SDTElm[actualOtherFlag].original_network_id = (tmpSpaceList[spacePos].space[8] << 8) | tmpSpaceList[spacePos].space[9];

						tmpSpaceList[spacePos].section_length = SDTElm[actualOtherFlag].section_length;

						if(table_id == service_description_section_actual)
							lastSDTVersionActual = SDTElm[actualOtherFlag].version_number;
						else if(table_id == service_description_section_other)
							lastSDTVersionOther = SDTElm[actualOtherFlag].version_number;

						sectionLen = tmpSpaceList[spacePos].section_length - 12;

						jump = 11;

						SDTElm[actualOtherFlag].serviceTableNum = 0;

						j = 0;

						for(i = 0; i < sectionLen; i = i + skipLen)
						{
							if(SDTElm[actualOtherFlag].serviceTab[j] == NULL)
								SDTElm[actualOtherFlag].serviceTab[j] = (struct serviceTable*) malloc(sizeof(struct serviceTable));

							SDTElm[actualOtherFlag].serviceTab[j] -> service_id = (tmpSpaceList[spacePos].space[jump + i] << 8) | tmpSpaceList[spacePos].space[jump + 1 + i];
							SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length = ((tmpSpaceList[spacePos].space[jump + i + 3] & 0xf) << 8) | tmpSpaceList[spacePos].space[jump + i + 4];
							SDTElm[actualOtherFlag].serviceTableNum++;

							skipLen = 5 + SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length;  

							parseDescriptor(tmpSpaceList[spacePos].space + jump + i + 5, SDTElm[actualOtherFlag].serviceTab[j] -> descriptors_loop_length, actualOtherFlag, j);

							j++;
						}

						tmpSpaceList[spacePos].emptyFlag = -1;
//						printStructInfo(5);
						break;
				}
			}
			else
			{
				if((header.continuity_counter & 0xf)\
						== ((tmpSpaceList[spacePos].continuity_counter + 1) & 0xf))
				{
					memcpy(tmpSpaceList[spacePos].space + tmpSpaceList[spacePos].length, buffer + *offset, bufferLen - *offset);
					tmpSpaceList[spacePos].length += (bufferLen - *offset);
					tmpSpaceList[spacePos].continuity_counter = header.continuity_counter;
				}
				else
					printf("Error: pack not in order!\n");
			}
		}
		else
		{
//			printf("Error[in parseSDT]: no start of pack!\n");
		}
	}

	*offset = bufferLen;

	return TRUE;
}

static int parseNIT(unsigned char* buffer, int bufferLen, int* offset)
{
	unsigned int table_id = 0;
	unsigned int despLen = 0;
	unsigned int transLen = 0;

	int sectionLen = 0;
	int versionNumber = 0;
	int actualOtherFlag = 1;
	int spacePos = 0;
	int sectionNum = 0;
	int lastSectionNum = 0;

	if(bufferLen <= *offset + 7)
		return FALSE;

	if(header.payload_unit_start_indicator == 0x1)
	{
		table_id = buffer[*offset];
		switch(table_id)
		{
			case network_information_section_actual:
				actualOtherFlag = 0;
			case network_information_section_other:

				versionNumber = (buffer[*offset + 5] & 0x3e) >> 1; 
				sectionNum = buffer[*offset + 6];
				lastSectionNum = buffer[*offset + 7];

				if((table_id == network_information_section_actual && versionNumber <= lastNITVersionActual) \
					|| (table_id == network_information_section_other && versionNumber <= lastNITVersionOther))
					return TRUE;

				sectionLen = ((buffer[*offset + 1] & 0xf) << 8) | buffer[*offset + 2];
				despLen = ((buffer[*offset + 8] & 0xf) << 8) | buffer[*offset + 9];
				transLen = ((buffer[*offset + 10 + despLen] & 0xf) << 8) | buffer[*offset + 11 + despLen];

				NITSectionCount++;

				if(sectionLen <= TS_PACK_SIZE - 3)
				{
					if(despLen > 0)
						parseDescriptor(buffer + *offset + 10, despLen, actualOtherFlag, 0);

					if(transLen > 0)
						parseTranStreams(buffer + *offset + 12 + despLen, transLen, actualOtherFlag, 0);

					if(sectionNum == lastSectionNum && NITSectionCount == lastSectionNum + 1)
					{
						if(table_id == network_information_section_actual)
							lastNITVersionActual = versionNumber;
						else if(table_id == network_information_section_other)
							lastNITVersionOther = versionNumber;
					}
					else if(sectionNum == lastSectionNum)
					{
						NITSectionCount = 0;
					}

				}
				else
				{
					if((spacePos = getTmpSpace()) != -1)
					{
						memcpy(tmpSpaceList[spacePos].space, buffer + *offset, bufferLen - *offset);
						tmpSpaceList[spacePos].length = bufferLen - *offset;
						tmpSpaceList[spacePos].section_length = sectionLen;
						tmpSpaceList[spacePos].PID = header.PID;
						tmpSpaceList[spacePos].continuity_counter = header.continuity_counter;
						tmpSpaceList[spacePos].emptyFlag = 1;
					}
					else
					{
						printf("Error: no empty space left!");
						return FALSE;
					}
				}

				break;
		}
	}

	if(header.payload_unit_start_indicator == 0x0)
	{
		if((spacePos = findTmpSpaceByPID(header.PID)) != -1)
		{
			if(tmpSpaceList[spacePos].length + bufferLen - *offset >= tmpSpaceList[spacePos].section_length)
			{
				memcpy(tmpSpaceList[spacePos].space + tmpSpaceList[spacePos].length, buffer + *offset, bufferLen - *offset);

				table_id = tmpSpaceList[spacePos].space[0];
				versionNumber = (tmpSpaceList[spacePos].space[5] & 0x3e) >> 1; 
				despLen = ((tmpSpaceList[spacePos].space[8] & 0xf) << 8) | tmpSpaceList[spacePos].space[9];
				sectionNum = tmpSpaceList[spacePos].space[6];
				lastSectionNum = tmpSpaceList[spacePos].space[7];

				if(((table_id == network_information_section_actual && versionNumber <= lastNITVersionActual) \
					|| (table_id == network_information_section_other && versionNumber <= lastNITVersionOther)))
				{
					tmpSpaceList[spacePos].emptyFlag = -1;
					return TRUE;
				}

				if(despLen > 0)
					parseDescriptor(tmpSpaceList[spacePos].space + 10, despLen, actualOtherFlag, 0);

				transLen = ((tmpSpaceList[spacePos].space[10 + despLen] & 0xf) << 8)\
						   | tmpSpaceList[spacePos].space[11 + despLen];

				tmpSpaceList[spacePos].section_count++;

				if(transLen > 0)
					parseTranStreams(tmpSpaceList[spacePos].space + 12 + despLen, transLen, actualOtherFlag, 0);

				if(sectionNum == lastSectionNum && tmpSpaceList[spacePos].section_count == lastSectionNum + 1)
				{
					if(table_id == network_information_section_actual)
						lastNITVersionActual = versionNumber;
					else if(table_id == network_information_section_other)
						lastNITVersionOther = versionNumber;
				}
				else if(sectionNum == lastSectionNum)
				{
					tmpSpaceList[spacePos].section_count = 0;
				}

				tmpSpaceList[spacePos].emptyFlag = -1;
			}
			else
			{
				if((header.continuity_counter & 0xf)\
					== ((tmpSpaceList[spacePos].continuity_counter + 1) & 0xf)\
				   	|| tmpSpaceList[spacePos].length == 0)
				{
					memcpy(tmpSpaceList[spacePos].space + tmpSpaceList[spacePos].length, buffer + *offset, bufferLen - *offset);
					tmpSpaceList[spacePos].length += (bufferLen - *offset);
					tmpSpaceList[spacePos].continuity_counter = header.continuity_counter;
				}
				else
					printf("Error: pack not in order!\n");
			}
		}
		else
		{
//			printf("Error[in parseNIT]: no start of pack!\n");
		}
	}


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
				parsePAT(buffer, bufferLen, &offset);	
				break;

			case 0x0010:
				parseNIT(buffer, bufferLen, &offset);
				break;

			case 0x0011:
				parseSI(buffer, bufferLen, &offset);
				break;

			default:
				if(isPMTPID(header.PID))
				{
					parsePMT(header.PID, buffer, bufferLen, &offset);
				}

				audioVideoFlag = isPESPID(header.PID, &proNum);
				if(audioVideoFlag == AUDIO_FLAG)
				{
//					parsePES(buffer, bufferLen, &offset, audioVideoFlag);
				}
				else if(audioVideoFlag == VIDEO_FLAG)
				{
//					parsePES(buffer, bufferLen, &offset, audioVideoFlag);
					if(proNum == programWantToPlay)
						fwrite(buffer + offset, sizeof(char), bufferLen - offset, outPutFp);
				}
		}
	}	
	return offset;
}

static int cutTSPacket(unsigned char* buffer, int* bufferP, int bufferLen)
{
	int p = 0;
	int baseP = 0;
	
	if(*bufferP < 0 || bufferLen < 0 || buffer == NULL)
		return FALSE;

	while(baseP < bufferLen - TS_PACK_SIZE)
	{
		if(buffer[p + baseP] == 0x47)
		{
			baseP = baseP + p;

			if(p < bufferLen - baseP)
			{
				analysePacket(buffer + baseP, TS_PACK_SIZE);
#ifdef PRINT_DEBUG
//			printStructInfo(0);
#endif
			}
			else
				printf("Special pack size!\n");
			
			baseP += TS_PACK_SIZE;
			p = 0;
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

	startParse();
	addFreq2List(freqList, freq);

	while((freq = popList(freqList)) != -1)
	{

		printf("Seek freq %d\n", freq);
		if(openTSFile(freq) != TRUE)
			return FALSE;

		initValue();

		currentFreq = freq;

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

		addFreq2List(scannedList, currentFreq);

		currentFreq = 0;
	}

	endParse();
	return TRUE;
}
