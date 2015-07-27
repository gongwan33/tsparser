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

#define FILE_BUFFER_SIZE 100*1024
#define PACK_BUFFER_SIZE 4*1024
#define MAX_PROGRAM_NUMBER 600
#define MAX_PER_PMT_NUMBER 10

char isFreqAvailable(int freq);
char startParse();
char endParse();
int openTSFile(int freq);
int closeTSFile();
char parseTS(int freq);

struct tsHeader
{
	unsigned char sync_byte:						8;
	unsigned int transport_error_indicator:			1;
	unsigned int payload_unit_start_indicator:		1;
	unsigned int transport_priority:				1;
	unsigned int PID:								13;
	unsigned int transport_scrambling_control:		2;
	unsigned int adaptation_field_control:			2;
	unsigned int continuity_counter:				4;	
} header;

struct PCR
{
	unsigned long long int program_clock_reference_base:			33;
	unsigned long long int reserved:								6;
	unsigned long long int program_clock_reference_extension:		9;
};

struct OPCR
{
	unsigned long long int original_program_clock_reference_base:		33;
	unsigned long long int reserved:									6;
	unsigned long long int original_program_clock_reference_extension:	9; 
};

struct ltw
{
	unsigned int ltw_valid_flag:		1;
	unsigned int ltw_offset:			15;
};

struct piecewiseRate
{
	unsigned int reserved:				2;
	unsigned int piecewise_rate:		22;
};

struct seamlessSplice
{
	unsigned char splice_type:			4;
	unsigned char DTS_next_AU32_30:		3;
	unsigned char marker_bit0:			1;
	unsigned int DTS_next_AU29_15:		15;
	unsigned int marker_bit1:			1;
	unsigned int DTS_net_AU14_0:		15;
	unsigned int marker_bit2:			1;
};

struct adaptationFieldExtension
{
	unsigned char adaptation_field_extension_length:			8;
	unsigned char ltw_flag:										1;
	unsigned char piecewise_rate_flag:							1;
	unsigned char seamless_splice_flag:							1;
	unsigned char reserved:										5;
	struct ltw ltwElm;
	struct piecewiseRate piecewiseElm;
	struct seamlessSplice seamlessElm;
};

struct adaptationField
{
	unsigned char adaptation_field_length:				8;
	unsigned char discontinuity_indicator:			    1;
	unsigned char random_access_indicator:				1;
	unsigned char elementary_stream_priority_indicator:	1;
	unsigned char PCR_flag:								1;
	unsigned char OPCR_flag:							1;
	unsigned char splicing_point_flag:					1;
	unsigned char transport_private_data_flag:			1;
	unsigned char adaptation_field_extension_flag:		1;
	struct PCR PCRElm;
	struct OPCR OPCRElm;
	unsigned char splice_countdown:						8;
	unsigned char tansport_private_data_length:			8;
} aField;

struct PES
{
	unsigned int packet_start_code_prefix:				24;
	unsigned int stream_id:								8;
	unsigned int PES_packet_length:						16;
	unsigned char PES_scrambling_control:				2;
	unsigned char PES_priority:							1;
	unsigned char data_alignment_indicator:				1;
	unsigned char copyright:							1;
	unsigned char original_or_copy:						1;
	unsigned char PTS_DTS_flags:						2;
	unsigned int ESCR_flag:								1;
	unsigned int ES_rate_flag:							1;
	unsigned int DSM_trick_mode_flag:					1;
	unsigned int additional_copy_info_flag:				1;
	unsigned int PES_CRC_flag:							1;
	unsigned int PES_extension_flag:					1;
	unsigned int PES_header_data_length:				8;
	unsigned int PTS;
	unsigned int DTS;
	unsigned int ESCR_base;
	unsigned int ES_rate;
} PESheader;

struct programInfo
{
	unsigned int program_number:				16;
	unsigned int reserved:						3;
	unsigned int network_program_PID:			13;
};

struct PAT
{
	unsigned char table_id:						8;
	unsigned char section_syntax_indicator:		1;
	unsigned int section_length:				12;
	unsigned int transport_stream_id:			16;
	unsigned char version_number:				5;
	unsigned char current_next_indicator:		1;
	unsigned char section_number:				8;
	unsigned char last_section_number:			8;
	struct programInfo* programInfoElm;
	unsigned int CRC_32;
} PATElm;

struct PMT
{
	unsigned char table_id:						8;
	unsigned char section_syntax_indicator:		1;
	unsigned int section_length:				12;
	unsigned int program_number:				16;
	unsigned char version_number:				5;
	unsigned char current_next_indicator:		1;
	unsigned char section_number:				8;
	unsigned char last_section_number:			8;
	unsigned int PCR_PID:						13;
	unsigned int program_info_length:			12;
	
	unsigned int CRC_32;
} PMTElm;

struct pointerField
{
	unsigned char pointer_field;
} pointerFieldElm;

struct mapPES
{
	unsigned char stream_type;
	unsigned int elementary_PID:		13;
	unsigned int ES_info_length:		12;
};

struct MAP
{
	int PMT_PID;
	struct mapPES* mapPESElm[MAX_PER_PMT_NUMBER];
	unsigned int mapNumber;
} mapElm[MAX_PROGRAM_NUMBER];

#endif




