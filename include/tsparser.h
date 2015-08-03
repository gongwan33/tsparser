/*
 *Project name: tsparser
 *File: tsparser.h
 *Author: Wagner
 *Date: 2015-7
 */

#ifndef TSPARSER_H
#define TSPARSER_H

#define TRUE									1
#define FALSE									0

#define TS_PACK_SIZE							188
#define TMP_SPACE_LIST_SIZE						1000
#define MAX_PACK_SIZE							4*1024
#define FREQ_LIST_SIZE							1000

#define AUDIO_FLAG								2
#define VIDEO_FLAG								3

#define FILE_BUFFER_SIZE						1024*1024
#define MAX_PROGRAM_NUMBER						600
#define MAX_PER_PMT_NUMBER						10
#define MAX_SERVICE_NUMBER						600

#define program_stream_map						0xbc
#define padding_stream							0xbe
#define private_stream_2						0xbf
#define ECM										0xf0
#define EMM										0xf1
#define program_stream_directory				0xff
#define DSMCC_stream							0xf2
#define ITU_T_Rec_H_222_1_type_E_stream			0xf8

#define fast_forward							0x00
#define slow_motion								0x01
#define	freeze_frame							0x02
#define fast_reverse							0x03
#define slow_reverse							0x04

#define service_description_section_actual		0x42
#define service_description_section_other		0x46

#define network_information_section_actual		0x40
#define network_information_section_other		0x41

#define multilingual_service_name_descriptor	0x5d
#define cable_delivery_system_descriptor		0x44

char isFreqAvailable(int freq);
int openTSFile(int freq);
int closeTSFile();
char parseTS(int freq);

struct tmpSpace
{
	char emptyFlag;
	unsigned int PID;
	unsigned char continuity_counter;
	unsigned char space[MAX_PACK_SIZE];
	unsigned int length;
	unsigned int section_length;
	unsigned int section_count;
} tmpSpaceList[TMP_SPACE_LIST_SIZE];

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
	unsigned char trick_mode_control;
	unsigned char PES_private_data_flag:				1;
	unsigned char pack_header_field_flag:				1;
	unsigned char program_packet_sequence_counter_flag:	1;
	unsigned char P_STD_buffer_flag:					1;
	unsigned char PES_extension_flag_2:					1;
	unsigned char pack_field_length;
	unsigned char PES_extension_field_length;
	unsigned int PTS;
	unsigned int DTS;
	unsigned int ESCR_base;
	unsigned int ES_rate;
} PESElm;

struct programInfo
{
	unsigned int program_number:				16;
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

struct multilingual_service_name_desp_content
{
	unsigned int ISO_639_language_code;
	unsigned char service_provider_name_length;
	char* service_provider;
	unsigned char service_name_length;
	char* service_name;
};

struct serviceTable
{
	unsigned int service_id:					16;
	unsigned char EIT_schedule_flag;
	unsigned char EIT_present_following_flag;
	unsigned int running_status:				3;
	unsigned int free_CA_mode:					1;
	unsigned int descriptors_loop_length:		12;
	struct multilingual_service_name_desp_content msdc[51];
	unsigned int service_count;
};

struct SDT
{
	unsigned char table_id;
	unsigned int section_syntax_indicator:		1;
	unsigned int section_length:				12;
	unsigned int transport_stream_id;
	unsigned char version_number:				5;
	unsigned char current_next_indicator:		1;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned int original_network_id;	
	struct serviceTable* serviceTab[MAX_SERVICE_NUMBER];
	unsigned int serviceTableNum;
} SDTElm[2];

struct NIT
{
	unsigned char table_id;
	unsigned int section_syntax_indicator:		1;
	unsigned int section_length:				12;
	unsigned int network_id:					16;
	unsigned char version_number:				5;
	unsigned char current_next_indicator:		1;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned int original_network_id;	
} NITElm[2];

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
	unsigned int program_number;
	struct mapPES* mapPESElm[MAX_PER_PMT_NUMBER];
	unsigned int mapNumber;
} mapElm[MAX_PROGRAM_NUMBER];

#endif




