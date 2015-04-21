/*
 * mpegts.c
 *
 *  Created on: 2012-2-8
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>

#include "generic.h"
#include "mpegts.h"


u_int64_t _MPEGTS_DTS_PTS_VALUE(MPEGTS_DTS_PTS* _ptr)
{
	u_int64_t ret = (_ptr->value_32_30 << 30) | (_ptr->value_29_22 << 22) | (_ptr->value_21_15 << 15) | (_ptr->value_14_07 << 7) | (_ptr->value_06_00);
	return ret;
}

void _MPEGTS_DTS_PTS_SET_VALUE(MPEGTS_DTS_PTS* _ptr, u_int64_t _n)
{
	_ptr->value_32_30 = (_n >> 30) & 0x07;
	_ptr->value_29_22 = (_n >> 22) & 0xff;
	_ptr->value_21_15 = (_n >> 15) & 0x7f;
	_ptr->value_14_07 = (_n >> 7) & 0xff;
	_ptr->value_06_00 = _n & 0x07f;
}

u_int64_t MPEGTS_NS2PTS_PCR(u_int64_t _ns)
{
	u_int64_t ret = _ns * 27 / 300;
	return ret;
}

u_int64_t MPEGTS_PTS_PCR2NS(u_int64_t _pts)
{
	u_int64_t ns = _pts * 300 / 27;
	return ns;
}

char* MPEGTS_NS2STR(u_int64_t _ns)
{
	static char buf[128];
	int hour = _ns / 1000 / 1000 / 60 / 60 % 24;
	int min = _ns / 1000 / 1000 / 60 % 60;
	int sec = _ns /1000 / 1000 % 60;
	int ms = _ns /1000 % 1000; //毫秒
	int ns = _ns %1000; //纳秒
	sprintf(buf, "%02d:%02d:%02d.%04d.%04d", hour, min, sec, ms, ns);
	return buf;
}

void MPEGTS_CRC32_print(MPEGTS_CRC32* _crc)
{
	printf("\tMPEGTS CRC32 BEGIN =================\n");
	printf("\tcrc32 = 0x");
	int i;
	int tmp;
	for(i = 0; i < 4; i++)
	{
		tmp = _crc->crc32[i];
		printf("%02x", tmp);
	}
	printf("\n");
}

u_int64_t _MPEG_PROGRAM_CLOCK_REFERENCE_BASE(MPEGTS_PCR* _ptr)
{
	u_int64_t ret = _ptr->program_clock_reference_base_26_33;
	ret = ( ret << 8) | _ptr->program_clock_reference_base_18_25;
	ret = ( ret << 8) | _ptr->program_clock_reference_base_10_17;
	ret = ( ret << 8) | _ptr->program_clock_reference_base_2_9;
	ret = ( ret << 1) | _ptr->program_clock_reference_base_1;
	return ret;
}

void _MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_BASE(MPEGTS_PCR* _ptr, u_int64_t _n)
{
	_ptr->program_clock_reference_base_26_33 = (_n >> 25) & 0xFF;
	_ptr->program_clock_reference_base_18_25 = (_n >> 17) & 0xFF;
	_ptr->program_clock_reference_base_10_17 = (_n >> 9) & 0xFF;
	_ptr->program_clock_reference_base_2_9 = (_n >> 1) & 0xFF;
	_ptr->program_clock_reference_base_1 = _n & 0x01;
}

//
//inline short MPEG_PROGRAM_CLOCK_REFERENCE_EXTENSION(MPEGTS_PCR* _ptr)
//{
//	short ret = _ptr->program_clock_reference_extension_high1;
//	ret = ( ret << 8) | _ptr->program_clock_reference_extension_low8;
//	return ret;
//}

void MPEGTS_PCR_print(MPEGTS_PCR* _pcr)
{
	printf("\t\tMPEGTS PCR BEGIN =================\n");
	printf("\t\tprogram_clock_reference_base      = %llu(%s)\n", MPEG_PCR_PROGRAM_CLOCK_REFERENCE_BASE(_pcr), MPEGTS_NS2STR(MPEGTS_PTS_PCR2NS(MPEG_PCR_PROGRAM_CLOCK_REFERENCE_BASE(_pcr))));
	printf("\t\treserved                          = 0x%02x(0x3f)\n", _pcr->reserved);
	printf("\t\tprogram_clock_reference_extension = 0x%04x\n", MPEG_PCR_PROGRAM_CLOCK_REFERENCE_EXTENSION(_pcr));
}

void MPEGTS_DATA_print(unsigned char* _buf, int _len)
{
	printf("\tMPEGTS DATA BEGIN =================\n");
	printf("\tdata_len = 0x%02x(%d)\n", _len, _len);
	printf("\tdata     = 0x%02x\n", *_buf);
}

void MPEGTS_ADAPTATION_FIELD_print(MPEGTS_ADAPTATION_FIELD* _field)
{
	printf("\tMPEGTS ADAPTATION FIELD BEGIN =================\n");
	if(_field->adaptation_field_length == 0)
	{
		printf("adaptation_field_length = 0, no adaptation_field.\n");
		return;
	}
	printf("\tadaptation_field_length              = 0x%02x(%d)\n",
			_field->adaptation_field_length, _field->adaptation_field_length);
	printf("\tdiscontinuity_indicator              = 0x%02x\n",
			_field->discontinuity_indicator);
	printf("\trandom_access_indicator              = 0x%02x\n",
			_field->random_access_indicator);
	printf("\telementary_stream_priority_indicator = 0x%02x\n",
			_field->elementary_stream_priority_indicator);
	printf("\tPCR_flag                             = 0x%02x\n",
			_field->PCR_flag);
	printf("\tOPCR_flag                            = 0x%02x\n",
			_field->OPCR_flag);
	printf("\tsplicing_point_flag                  = 0x%02x\n",
			_field->splicing_point_flag);
	printf("\ttransport_private_data_flag          = 0x%02x\n",
			_field->transport_private_data_flag);
	printf("\tadaptation_field_extension_flag      = 0x%02x\n",
			_field->adaptation_field_extension_flag);

	MPEGTS_PCR* p_pcr = NULL;
	if(_field->PCR_flag == 1)
	{
		p_pcr = (MPEGTS_PCR*)(_field+1);
		MPEGTS_PCR_print(p_pcr);
	}
}

void MPEGTS_DTS_PTS_print(MPEGTS_DTS_PTS* _ptr, int _pts_or_dts)//1 pts 0 dts
{
	char* type = NULL;
	if(_pts_or_dts == 1)
	{
		type = "PTS";
	}
	else
	{
		type = "DTS";
	}
	printf("\t\tMPEGTS %s BEGIN =================\n", type);
	printf("\t\t%s  = %llu(%s)\n", type,  MPEGTS_DTS_PTS_VALUE(_ptr), MPEGTS_NS2STR(MPEGTS_PTS_PCR2NS(MPEGTS_DTS_PTS_VALUE(_ptr))));

//	printf("\t\t%s  = 0x", type);
//	int i;
//	int tmp;
//	for(i = 0; i < 8; i++)
//	{
//		tmp = *(((unsigned char*)_ptr) + i);
//		printf("%02x", tmp);
//	}
//	printf("(%llu)\n", MPEGTS_DTS_PTS_VALUE(_ptr));
}

void MPEGTS_PES_print(MPEGTS_PES* _pes)
{
	printf("\tMPEGTS PES BEGIN =================\n");
	printf("\tstart_code                = 0x%02x%02x%02x\n", _pes->start_code[0], _pes->start_code[1], _pes->start_code[2]);
	printf("\tstream_id                 = 0x%02x\n", _pes->stream_id);
	printf("\tPES_packet_length         = 0x%04x(%d)\n", MPEGTS_PES_PES_PACKET_LENGTH(_pes), MPEGTS_PES_PES_PACKET_LENGTH(_pes));
	printf("\tresvered                  = 0x%02x\n", _pes->reserved);
	printf("\tPES_scrambling_control    = 0x%02x\n", _pes->PES_scrambling_control);
	printf("\tPES_priority              = 0x%02x\n", _pes->PES_priority);
	printf("\tdata_alignment_indicator  = 0x%02x\n", _pes->data_alignment_indicator);
	printf("\tcopyright                 = 0x%02x\n", _pes->copyright);
	printf("\toriginal_or_copy          = 0x%02x\n", _pes->original_or_copy);
	printf("\tPTS_DTS_flag              = 0x%02x\n", _pes->PTS_DTS_flag);
	printf("\tESCR_flag                 = 0x%02x\n", _pes->ESCR_flag);
	printf("\tES_rate_flag              = 0x%02x\n", _pes->ES_rate_flag);
	printf("\tDSM_trick_mode_flag       = 0x%02x\n", _pes->DSM_trick_mode_flag);
	printf("\tadditional_copy_info_flag = 0x%02x\n", _pes->additional_copy_info_flag);
	printf("\tPES_CRC_flag              = 0x%02x\n", _pes->PES_CRC_flag);
	printf("\tPES_extension_flag        = 0x%02x\n", _pes->PES_extension_flag);
	printf("\tPES_header_data_length    = 0x%02x\n", _pes->PES_header_data_length);

	MPEGTS_DTS_PTS* p_pts_or_dts = NULL;
	switch(_pes->PTS_DTS_flag)
	{
	case 0://00 no PTS or DTS data present
		break;
	case 1://01 is forbidden
		break;
	case 2://10 only pts
		p_pts_or_dts = (MPEGTS_DTS_PTS*)(_pes + 1);
		MPEGTS_DTS_PTS_print(p_pts_or_dts, 1);
		break;
	case 3://11 pts and dts
		p_pts_or_dts = (MPEGTS_DTS_PTS*)(_pes + 1);
		MPEGTS_DTS_PTS_print(p_pts_or_dts, 1);
		p_pts_or_dts = (MPEGTS_DTS_PTS*)(_pes + 2);
		MPEGTS_DTS_PTS_print(p_pts_or_dts, 0);
		break;
	default:
		break;
	}
}

void MPEGTS_SDT_print(MPEGTS_SDT* _sdt)
{
	printf("\tMPEGTS SDT BEGIN =================\n");
	printf("\ttable_id                 = 0x%02x(0x42)\n", _sdt->table_id);
	printf("\tsection_syntax_indicator = 0x%02x\n", _sdt->section_syntax_indicator);
	printf("\treserved_future_used     = 0x%02x\n", _sdt->reserved_future_used);
	printf("\treserved                 = 0x%02x\n", _sdt->reserved);
	printf("\tsection_length           = 0x%02x(%d)\n", MPEGTS_SDT_SECTION_LENGTH(_sdt), MPEGTS_SDT_SECTION_LENGTH(_sdt));
	printf("\ttransport_stream_id      = 0x%04x(%d)\n", MPEGTS_SDT_TRANSPORT_STREAM_ID(_sdt), MPEGTS_SDT_TRANSPORT_STREAM_ID(_sdt));
	printf("\treserved2                = 0x%02x\n", _sdt->reserved2);
	printf("\tversion_number           = 0x%02x\n", _sdt->version_number);
	printf("\tcurrent_next_indicator   = 0x%02x\n", _sdt->current_next_indicator);
	printf("\tsection_number           = 0x%02x\n", _sdt->section_number);
	printf("\tlast_section_number      = 0x%02x\n", _sdt->last_section_number);
	printf("\toriginal_netword_id      = 0x%02x\n", MPEGTS_SDT_ORIGINAL_NETWORK_ID(_sdt));
	printf("\treserved_future_use      = 0x%02x\n", _sdt->reserved_future_use);

	int crc_len = (MPEGTS_PAT_SECTION_LENGTH(_sdt) + 3 - 4);//+len之前的3个字节 -crc32_len
	unsigned char crc_bytes[4];
	crc32((unsigned char*)_sdt, crc_len, crc_bytes);
	printf("\tcrc32_should_be          = 0x%02x%02x%02x%02x\n", crc_bytes[0], crc_bytes[1], crc_bytes[2], crc_bytes[3]);

//	int entry_count = (MPEGTS_PAT_SECTION_LENGTH(_pat) + 3 - 8 - 4) / sizeof(MPEGTS_PAT_ENTRY);//+len之前的3个字节 -PAT包头 -crc32_len
////	printf("\tentry_count              = 0x%02x\n", entry_count);
//	int i;
//	for(i = 0; i < entry_count; i++)
//	{
//		MPEGTS_PAT_ENTRY* p_pat_entry = NULL;
//		p_pat_entry = (MPEGTS_PAT_ENTRY*)(_pat+1);
//		MPEGTS_PAT_ENTRY_print(p_pat_entry);
//	}
	MPEGTS_SDT_ENTRY* p_sdt_entry = NULL;
	p_sdt_entry = (MPEGTS_SDT_ENTRY*)(_sdt+1);
	MPEGTS_SDT_ENTRY_print(p_sdt_entry);
}

void MPEGTS_SDT_ENTRY_print(MPEGTS_SDT_ENTRY* _entry)
{
	printf("\t\tMPEGTS SDT ENTRY BEGIN =================\n");
	printf("\t\tservice_id                 = 0x%02x\n", MPEGTS_SDT_ENTRY_SERVICE_ID(_entry));
	printf("\t\treserved_future_use        = 0x%02x\n", _entry->reserved_future_use);
	printf("\t\tEIT_schedule_flag          = 0x%02x\n", _entry->EIT_schedule_flag);
	printf("\t\tEIT_present_following_flag = 0x%02x\n", _entry->EIT_present_following_flag);
	printf("\t\trunning_status             = 0x%02x\n", _entry->running_status);
	printf("\t\tfree_CA_mode               = 0x%02x\n", _entry->free_CA_mode);
	printf("\t\tdescriptor_loop_length     = 0x%02x\n", MPEGTS_SDT_ENTRY_DESCRIPTOR_LOOP_LENGTH(_entry));


	printf("\t\tthere is something here,service data and crc32.\n");

//	MPEGTS_CRC32* p_crc32 = NULL;
//	p_crc32 = (MPEGTS_CRC32*)(_entry + 1);
//	MPEGTS_CRC32_print(p_crc32);
}

void MPEGTS_PAT_print(MPEGTS_PAT* _pat)
{
	printf("\tMPEGTS PAT BEGIN =================\n");
	printf("\ttable_id                 = 0x%02x(0x00)\n", _pat->table_id);
	printf("\tsection_syntax_indicator = 0x%02x\n", _pat->section_syntax_indicator);
	printf("\tzero                     = 0x%02x(0x00)\n", _pat->zero);
	printf("\treserved                 = 0x%02x\n", _pat->reserved);
	printf("\tsection_length           = 0x%02x(%d)\n", MPEGTS_PAT_SECTION_LENGTH(_pat), MPEGTS_PAT_SECTION_LENGTH(_pat));
	printf("\ttransport_stream_id      = 0x%04x(%d)\n", MPEGTS_PAT_TRANSPORT_STREAM_ID(_pat), MPEGTS_PAT_TRANSPORT_STREAM_ID(_pat));
	printf("\treserved2                = 0x%02x\n", _pat->reserved2);
	printf("\tversion_number           = 0x%02x\n", _pat->version_number);
	printf("\tcurrent_next_indicator   = 0x%02x\n", _pat->current_next_indicator);
	printf("\tsection_number           = 0x%02x\n", _pat->section_number);
	printf("\tlast_section_number      = 0x%02x\n", _pat->last_section_number);

	int crc_len = (MPEGTS_PAT_SECTION_LENGTH(_pat) + 3 - 4);//+len之前的3个字节 -crc32_len
	unsigned char crc_bytes[4];
	crc32((unsigned char*)_pat, crc_len, crc_bytes);
	printf("\tcrc32_should_be          = 0x%02x%02x%02x%02x\n", crc_bytes[0], crc_bytes[1], crc_bytes[2], crc_bytes[3]);

	int entry_count = (MPEGTS_PAT_SECTION_LENGTH(_pat) + 3 - 8 - 4) / sizeof(MPEGTS_PAT_ENTRY);//+len之前的3个字节 -PAT包头 -crc32_len
//	printf("\tentry_count              = 0x%02x\n", entry_count);
	int i;
	for(i = 0; i < entry_count; i++)
	{
		MPEGTS_PAT_ENTRY* p_pat_entry = NULL;
		p_pat_entry = (MPEGTS_PAT_ENTRY*)(_pat+1);
		MPEGTS_PAT_ENTRY_print(p_pat_entry);
	}
}

void MPEGTS_PAT_ENTRY_print(MPEGTS_PAT_ENTRY* _entry)
{
	printf("\t\tMPEGTS PAT ENTRY BEGIN =================\n");
	printf("\t\tprogram_number = 0x%02x\n", MPEGTS_PAT_ENTRY_PROGROM_NUMBER(_entry));
	printf("\t\treserved       = 0x%02x\n", _entry->reserved);
	printf("\t\tPID            = 0x%02x\n", MPEGTS_PAT_ENTRY_PID(_entry));

	MPEGTS_CRC32* p_crc32 = NULL;
	p_crc32 = (MPEGTS_CRC32*)(_entry + 1);
	MPEGTS_CRC32_print(p_crc32);
}

void MPEGTS_PMT_print(MPEGTS_PMT* _pmt)
{
	printf("\tMPEGTS PMT BEGIN =================\n");
	printf("\ttable_id                 = 0x%02x\n", _pmt->table_id);
	printf("\tsection_syntax_indicator = 0x%02x\n", _pmt->section_syntax_indicator);
	printf("\tzero                     = 0x%02x\n", _pmt->zero);
	printf("\treserved                 = 0x%02x\n", _pmt->reserved);
	printf("\tsection_length           = 0x%04x\n", MPETTS_PMT_SECTION_LENGTH(_pmt));
	printf("\tprogram_number           = 0x%04x\n", MPETTS_PMT_PROGRAM_NUMBER(_pmt));
	printf("\treserved2                = 0x%02x\n", _pmt->reserved2);
	printf("\tversion_number           = 0x%02x\n", _pmt->version_number);
	printf("\tcurrent_next_indicator   = 0x%02x\n", _pmt->current_next_indicator);
	printf("\tsection_number           = 0x%02x\n", _pmt->section_number);
	printf("\tlast_section_number      = 0x%02x\n", _pmt->last_section_number);
	printf("\treserved3                = 0x%02x\n", _pmt->reserved3);
	printf("\tPCR_PID                  = 0x%04x\n", MPETTS_PMT_PCR_PID(_pmt));
	printf("\treserved4                = 0x%02x\n", _pmt->reserved4);
	printf("\tprogram_info_length      = 0x%04x\n", MPETTS_PMT_PROGRAM_INFO_LENGTH(_pmt));

	int crc_len = (MPETTS_PMT_SECTION_LENGTH(_pmt) + 3 - 4);//+len之前的3个字节 -crc32_len
	unsigned char crc_bytes[4];
	crc32((unsigned char*)_pmt, crc_len, crc_bytes);
	printf("\tcrc32_should_be          = 0x%02x%02x%02x%02x\n", crc_bytes[0], crc_bytes[1], crc_bytes[2], crc_bytes[3]);

	int entry_count = (MPETTS_PMT_SECTION_LENGTH(_pmt) + 3 - 12 - 4) / sizeof(MPEGTS_PMT_ENTRY);//+len之前的3个字节 -PMT包头 -crc32_len
//	printf("\tentry_count              = 0x%02x\n", entry_count);
	int i;
	for(i = 0; i < entry_count; i++)
	{
		MPEGTS_PMT_ENTRY* p_pmt_entry = NULL;
		p_pmt_entry = (MPEGTS_PMT_ENTRY*)(_pmt+1);
		MPEGTS_PMT_ENTRY_print(p_pmt_entry);
	}
}

void MPEGTS_PMT_ENTRY_print(MPEGTS_PMT_ENTRY* _entry)
{
	printf("\t\tMPEGTS PMT ENTRY BEGIN =================\n");
	printf("\t\tstream_type    = 0x%02x\n", _entry->stream_type);
	printf("\t\treserved       = 0x%02x\n", _entry->reserved);
	printf("\t\telementary_PID = 0x%02x\n", MPEGTS_PMT_ENTRY_ELEMENTARY_PID(_entry));
	printf("\t\tES_info_length = 0x%02x\n", MPEGTS_PMT_ENTRY_ES_INFO_LENGTN(_entry));
	printf("\t\treserved2      = 0x%02x\n", _entry->reserved2);

	MPEGTS_CRC32* p_crc32 = NULL;
	p_crc32 = (MPEGTS_CRC32*)(_entry + 1);
	MPEGTS_CRC32_print(p_crc32);
}

void MPEGTS_dump(MPEGTS_PACKET* _packet)
{
	static int pack_count = 0;
	printf("MPEG PACKET (%d) BEGIN =================\n", pack_count++);
	if (!IS_MPEG_HEAD(_packet))
	{
		printf("not a mepgts_head\n");
		return;
	}
	printf("sync_byte                   =0x%02x(%d)\n",
			_packet->sync_byte, MPEG_SYNC_BYTE_CONST);
	printf("transport_error_indicator   =0x%02x\n",
			_packet->transport_error_indicator);
	printf("payload_unit_start_indicator=0x%02x\n",
			_packet->payload_unit_start_indicator);
	printf("transport_priority          =0x%02x\n",
			_packet->transport_priority);
	printf("PID                         =0x%04x\n",
			MPEG_PID(_packet));
	printf("transport_scrambling_control=0x%02x\n",
			_packet->transport_scrambling_control);
	printf("adaptation_field_control    =0x%02x\n",
			_packet->adaptation_field_control);
	printf("continuity_counter          =0x%02x\n",
			_packet->continuity_counter);

	if (_packet->transport_error_indicator != 0)
	{
		printf("transport_error_indicator set, found error.\n");
		return;
	}

	if (_packet->payload_unit_start_indicator == 0)
	{

	}

	if(pack_count == 1)
	{
		MPEGTS_SDT* p_sdt = NULL;
		p_sdt = (MPEGTS_SDT*)&_packet->data[1];//field_len
		MPEGTS_SDT_print(p_sdt);
	}
	else if(pack_count == 2)
	{
		MPEGTS_PAT* p_pat = NULL;
		p_pat = (MPEGTS_PAT*)&_packet->data[1];//field_len
		MPEGTS_PAT_print(p_pat);
	}
	else if(pack_count == 3)
	{
		MPEGTS_PMT* p_pmt = NULL;
		p_pmt = (MPEGTS_PMT*)&_packet->data[1];//field_len
		MPEGTS_PMT_print(p_pmt);
	}
	else
	{
		MPEGTS_PES* p_pes = NULL;
		MPEGTS_ADAPTATION_FIELD* p_adaptation_field = NULL;
//		unsigned char* p_data = NULL;
//		int data_len = 0;
		//00空分组没有调整字段,解码器不进行处理
		//01仅含有效负载
		//10仅含调整字段
		switch(_packet->adaptation_field_control)
		{
		case 0:
			break;
		case 1:
//			p_data = (unsigned char*)&_packet->data[0];
//			data_len = 188 - 4;//head_size
//			MPEGTS_DATA_print(p_data, data_len);
			p_pes = (MPEGTS_PES*)&_packet->data[0];
			if(IS_MPEGTS_PES_HEAD(p_pes))
			{
				MPEGTS_PES_print(p_pes);
			}
			break;
		case 2:
			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
			MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);
			break;
		case 3:
			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
			MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);

//			p_data = (unsigned char*)&_packet->data[0];
//			p_data += p_adaptation_field->adaptation_field_length + 1;
//			data_len = 188 - 4 - 1 - p_adaptation_field->adaptation_field_length;//head_size+field_len+field_size
//			MPEGTS_DATA_print(p_data, data_len);
			p_pes = (MPEGTS_PES*)&_packet->data[p_adaptation_field->adaptation_field_length + 1];
			if(IS_MPEGTS_PES_HEAD(p_pes))
			{
				MPEGTS_PES_print(p_pes);
			}
			break;
		default:
			break;
		}
	}

//	if (MPEG_PID(_packet) == 0)
//	{
//		//PAT (Program Association Table )
//		//节目关联表PAT所在分组的PID=0
//		//PAT中列出了传输流中存在的节目流
//		//PAT指定了传输流中每个节目对应PMT所在分组的PID
//		//PAT的第一条数据指定了NIT所在分组的PID ，其他数据指定了PMT所在分组的PID。
//		MPEGTS_PAT* p_pat = NULL;
//		p_pat = (MPEGTS_PAT*)&_packet->data[1];//field_len
//		MPEGTS_PAT_print(p_pat);
//	}
//	else
//	{
//		MPEGTS_ADAPTATION_FIELD* p_adaptation_field = NULL;
//		unsigned char* p_data = NULL;
//		int data_len = 0;
//		//00空分组没有调整字段,解码器不进行处理
//		//01仅含有效负载
//		//10仅含调整字段
//		switch(_packet->adaptation_field_control)
//		{
//		case 0:
//			break;
//		case 1:
//			p_data = (unsigned char*)&_packet->data[0];
//			data_len = 188 - 4;//head_size
//			MPEGTS_DATA_print(p_data, data_len);
//			break;
//		case 2:
//			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
//			MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);
//			break;
//		case 3:
//			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
//			MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);
//
//			p_data = (unsigned char*)&_packet->data[0];
//			p_data += p_adaptation_field->adaptation_field_length + 1;
//			data_len = 188 - 4 - 1 - p_adaptation_field->adaptation_field_length;//head_size+field_len+field_size
//			MPEGTS_DATA_print(p_data, data_len);
//			break;
//		default:
//			break;
//		}
//	}
}

