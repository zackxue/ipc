
#include "generic.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "hls_debug.h"
#include "hls_segmentor.h"

#define TS_PACKET_MAX_SIZE (188)
#define TS_LEN (4)
#define ADFIELD_LEN_LEN (1)
#define ADFIELD_LEN (1)
#define PCR_LEN (6)
#define PES_LEN (9)
#define PTS_LEN (5)
#define AU_LEN (6)


static const uint8_t fixed_packet_pat[188] =
{
	0x47, 0x40, 0x00, 0x10, 0x00, 0x00, 0xB0, 0x0D, 0x00, 0x01, 0xC1, 0x00, 0x00, 0x00, 0x01, 0xF0,
	0x00, 0x2A, 0xB1, 0x04, 0xB2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t fixed_packet_pmt[188] = 
{
	0x47, 0x50, 0x00, 0x10, 0x00, 0x02, 0xB0, 0x12, 0x00, 0x01, 0xC1, 0x00, 0x00, 0xE1, 0x00, 0xF0,
	0x00, 0x1B, 0xE1, 0x00, 0xF0, 0x00, 0x15, 0xBD, 0x4D, 0x56, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


static int hls_segmentor_append(HLS_SEGMENTOR_t* segmentor, MPEGTS_PACKET* trans_packet_item)
{
	if(segmentor->n_trans_packet < ARRAY_ITEM(segmentor->trans_packet)){
		segmentor->trans_packet[segmentor->n_trans_packet++] = *trans_packet_item;
		return 0;
	}
	return -1;
}

int HLS_segmentor_init(HLS_SEGMENTOR_t* segmentor)
{
	segmentor->frame_counter = 0;
	segmentor->continuity_counter = 0;
	
	segmentor->n_trans_packet = 0;	
	hls_segmentor_append(segmentor, (MPEGTS_PACKET*)fixed_packet_pat);
	hls_segmentor_append(segmentor, (MPEGTS_PACKET*)fixed_packet_pmt);
	return 0;
}

int HLS_segmentor_append_video(HLS_SEGMENTOR_t* segmentor, void* frame_buf, ssize_t frame_buf_len, bool is_idr, uint64_t ti_us)
{
	unsigned char* current_ptr = frame_buf;
	int rest_len = frame_buf_len;

	int i;
	//HLS_TRACE("Append video size %d", frame_buf_len);
	for(i = 0; 1; i++)
	{
		int need_pcr = 0;
		if(segmentor->frame_counter % 2 == 0 && i == 0)
		{
			need_pcr = 1;
		}
		else
		{
			need_pcr = 0;
		}

		int need_adfield = 0;
		if(need_pcr == 1) //¨®Dpcr
		{
			need_adfield = 1;
		}
		else if(rest_len <= (TS_PACKET_MAX_SIZE - 5)) //2??¨²¡ã¨¹¡ê?D¨¨¨°a¨¬?3?¨ºy?Y
		{
			need_adfield = 1;
		}
		else
		{
			need_adfield = 0;
		}

		int need_pes = 0;
		if(i == 0)
		{
			need_pes = 1;
		}
		else
		{
			need_pes = 0;
		}

		int need_au = 0;
		if(i == 0)
		{
			need_au = 1;
		}
		else
		{
			need_au = 0;
		}

		int fill_size = TS_PACKET_MAX_SIZE;
		fill_size -= TS_LEN;
		if(need_adfield == 1 && need_pcr == 1)
		{
			fill_size -= (ADFIELD_LEN + ADFIELD_LEN_LEN + PCR_LEN);
		}
		else if(need_adfield == 1)
		{
			fill_size -= (ADFIELD_LEN + ADFIELD_LEN_LEN);
			if(rest_len == 183)
			{
				fill_size -= ADFIELD_LEN_LEN;
			}
		}
		if(need_pes == 1)
		{
			fill_size -= (PES_LEN + PTS_LEN);
		}
		if(need_au == 1)
		{
			fill_size -= AU_LEN;
		}
		if(fill_size > rest_len)
		{
			fill_size = fill_size - rest_len;
		}
		else
		{
			fill_size = 0;
		}

		int written_size = 0;
		MPEGTS_PACKET packet;
		packet.sync_byte = 0x47;
		packet.transport_error_indicator = 0;
		packet.payload_unit_start_indicator = (i==0)?1:0; //var
		packet.transport_priority = 0;
		MPEG_SET_PID(&packet, 0x0100);
		packet.transport_scrambling_control = 0;
		packet.adaptation_field_control = (need_adfield==1)?3:1;//var
		packet.continuity_counter = segmentor->continuity_counter++ &0x0f;//var
		if(segmentor->continuity_counter > 15)
		{
			segmentor->continuity_counter = 0;
		}
		written_size += TS_LEN;

		if(need_adfield == 1)
		{
			MPEGTS_ADAPTATION_FIELD* p_adfield = (MPEGTS_ADAPTATION_FIELD*)(((unsigned char*)&packet) + written_size);
			p_adfield->adaptation_field_length = (need_pcr==1)?(1+6+fill_size):(1+fill_size);//var
			if(rest_len == 183)
			{
				p_adfield->adaptation_field_length = 0;//var
			}
			p_adfield->discontinuity_indicator = 0;
			p_adfield->random_access_indicator = (is_idr && i==0)?1:0;//var
			p_adfield->elementary_stream_priority_indicator = 0;
			p_adfield->PCR_flag = (need_pcr==1)?1:0;//var
			p_adfield->OPCR_flag = 0;
			p_adfield->splicing_point_flag = 0;
			p_adfield->transport_private_data_flag = 0;
			p_adfield->adaptation_field_extension_flag = 0;
			written_size += (ADFIELD_LEN + ADFIELD_LEN_LEN);
			if(rest_len == 183)
			{
				written_size -= ADFIELD_LEN;
			}
		}
		if(need_pcr == 1)
		{
			MPEGTS_PCR* p_pcr = (MPEGTS_PCR*)(((unsigned char*)&packet) + written_size);
			uint32_t pcr_base = mp2ts_pcr_base_us(ti_us);
			uint32_t pcr_ext = mp2ts_pcr_ext_us(ti_us);
			
			MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_BASE(p_pcr, MPEGTS_NS2PTS_PCR(pcr_base)); //var
			p_pcr->reserved = MPEG_PCR_PROGRAM_CLOCK_RESERVED_CONST;
			MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_EXTENSION(p_pcr, pcr_ext);
			written_size += PCR_LEN;
		}
		if(fill_size > 0)
		{
			unsigned char* p = (unsigned char*)((unsigned char*)(&packet) + written_size);
			memset(p, 0xff, fill_size);
			written_size += fill_size;
		}
		if(need_pes == 1)
		{
			MPEGTS_PES* p_pes = (MPEGTS_PES*)(((unsigned char*)&packet) + written_size);
			MPEGTS_PES_SET_START_CODE(p_pes);
			p_pes->stream_id = 0xe0;
			MPEGTS_PES_SET_PES_PACKET_LENGTH(p_pes, frame_buf_len + 14); //var why 14?
			p_pes->reserved = MPEGTS_PES_RESERVED_CONST;
			p_pes->PES_scrambling_control = 0;
			p_pes->PES_priority = 0;
			p_pes->data_alignment_indicator = 0;
			p_pes->copyright = 0;
			p_pes->original_or_copy = 0;
			p_pes->PTS_DTS_flag = 2;
			p_pes->PES_extension_flag = 0;
			p_pes->PES_CRC_flag = 0;
			p_pes->additional_copy_info_flag = 0;
			p_pes->DSM_trick_mode_flag = 0;
			p_pes->ES_rate_flag = 0;
			p_pes->ESCR_flag = 0;
			p_pes->PES_header_data_length = 5;
			written_size += PES_LEN;

			MPEGTS_DTS_PTS* p_pts = (MPEGTS_DTS_PTS*)(((unsigned char*)&packet) + written_size);
			p_pts->reserved = MPEGTS_DTS_PTS_RESERVED_CONST;
			p_pts->reserved2 = MPEGTS_DTS_PTS_RESERVED2_CONST;
			p_pts->reserved3 = MPEGTS_DTS_PTS_RESERVED3_CONST;
			p_pts->reserved4 = MPEGTS_DTS_PTS_RESERVED4_CONST;
			MPEGTS_DTS_PTS_SET_VALUE(p_pts, MPEGTS_NS2PTS_PCR(ti_us)); //var
			written_size += PTS_LEN;
		}
		if(need_au == 1)
		{
			char au[] = {0x00, 0x00, 0x00, 0x01, 0x09, 0xf0};
			unsigned char* p = (unsigned char*)((unsigned char*)(&packet) + written_size);
			memcpy(p, au, sizeof(au));
			written_size += AU_LEN;
		}
		{
			int w_size = TS_PACKET_MAX_SIZE - written_size;
			unsigned char* p = (unsigned char*)((unsigned char*)(&packet) + written_size);
			memcpy(p, current_ptr, w_size);
			written_size += w_size;
			current_ptr += w_size;
			rest_len -= w_size;
		}
//		fwrite(&packet, 1, sizeof(packet), _fd);
		//CGI_chunked_write((unsigned char*)&packet, sizeof(packet));
//		add_ts_packet(_ctx, &packet);
		hls_segmentor_append(segmentor, &packet);


		if(rest_len == 0)
		{
			break;
		}
	}

	++segmentor->frame_counter;
	return 0;
}


#define HLS_SEGMENT_KEYFRAME_BACKLOG (1)

static void* hls_segmentor_task(void* arg)
{
	/*
	int ret = 0;
	HLS_SEGMENT_TASK_t* const task = (HLS_SEGMENT_TASK_t*)arg;
	lpMEDIABUF_USER user = NULL;
	int buf_speed = 0;
	HLS_SEGMENTOR_t hls_segmentor;
	uint64_t segmentor_in = 0;
	uint64_t segmentor_out = 0;
	int segment_counter = 0;
	int keyframe_counter = 0;
	char ts_buf_name[16] = {""};
	int ts_buf_id = 0;

	int ts_buf_speed = 1000000;
	int ts_buf_key_contain = 2;
	int ts_buf_user_permit = 5;
	int ts_buf_backlog = (ts_buf_key_contain + 1 + ts_buf_user_permit) * 2;

	user = MEDIABUF_attach(task->buf_id);
	HLS_ASSERT(NULL != user, "Media buffer user attach to %d failed!", task->buf_id);
	buf_speed = MEDIABUF_in_speed(task->buf_id);

	ret = snprintf(ts_buf_name, ARRAY_ITEM(ts_buf_name), "%s.ts", ts_buf_name);
	ret = MEDIABUF_new(0, ts_buf_name, "HLS ts segment", ts_buf_speed, ts_buf_key_contain, ts_buf_backlog, ts_buf_user_permit);
	HLS_ASSERT(0 == ret, "TS buffer create failed");

	ts_buf_id = MEDIABUF_lookup_byname(ts_buf_name);
	HLS_TRACE("TS segment buffer id = %d", ts_buf_id);

	HLS_segmentor_init(&hls_segmentor);
	while(task->trigger)
	{
		bool read_success = false;

		// start to make an live ts file
		if(0 == MEDIABUF_out_lock(user)){
			SDK_AVENC_BUF_ATTR_t* frame_attr = NULL;
			void* frame_ptr = NULL;
			ssize_t frame_size = 0;
			// out a new frame from media buffer
			if(0 == MEDIABUF_out(user, &frame_attr, NULL, &frame_size)){
				// frame data and size
				frame_ptr = frame_attr + 1;
				frame_size = frame_size - sizeof(SDK_AVENC_BUF_ATTR_t);

				if(frame_attr->h264.keyframe){
					++keyframe_counter;
				}

				if(0 == segmentor_in){
					segmentor_in = frame_attr->pts_sys;
				}
				segmentor_out = frame_attr->pts_sys;

				if(keyframe_counter >= (HLS_SEGMENT_KEYFRAME_BACKLOG + 1)){
					// packet as a ts file
					const void* ts_file = hls_segmentor.trans_packet;
					int const ts_size = hls_segmentor.n_trans_packet * 188;
					float const segment_duration = (float)(segmentor_out - segmentor_in);
					
				
					if(1){
						// to file for testing
						char file_name[64] = {""};
						FILE* fid = NULL;
						sprintf(file_name, "%d_%.3f.ts", segment_counter, segment_duration / 1000000);
						fid = fopen(file_name, "w+b");
						fwrite(ts_file, 1, ts_size, fid);
						fclose(fid);
						fid = NULL;
					}

					// to mediabuf for HLS on demand buffering
					if(0 == MEDIABUF_in_request(ts_buf_id, ts_size + sizeof(HLS_SEGMENT_INFO_t), (0 == (segment_counter % 5)) ? true : false)){
						HLS_SEGMENT_INFO_t segment_info;
						STRUCT_ZERO(segment_info);
						segment_info.duration_s = segment_duration / 1000000;
						
						if(0 == MEDIABUF_in_attach(ts_buf_id, &segment_info, sizeof(segment_info))){
							if(0 == MEDIABUF_in_attach(ts_buf_id, ts_file, ts_size)){
								//HLS_TRACE("Insert TS file size %d duration %.3f", ts_size, segment_info.duration_s);
							}
						}
						MEDIABUF_in_committ(ts_buf_id);
					}

					// re-init the hls segmentor
					keyframe_counter = 1;
					HLS_segmentor_init(&hls_segmentor);
					segmentor_in = frame_attr->pts_sys;

					// segment counter increase
					++segment_counter;
				}

				HLS_segmentor_append_video(&hls_segmentor, frame_ptr, frame_size, frame_attr->h264.keyframe, frame_attr->pts_sys);

				read_success = true;
			}
			MEDIABUF_out_unlock(user);
		}

		if(true != read_success){
			usleep(buf_speed);
		}
	}
	
	MEDIABUF_detach(user);
	user = NULL;
	*/
	
	pthread_exit(NULL);
}

int HLS_start_segmentor_task(HLS_SEGMENT_TASK_t* task)
{
	int ret = 0;
	task->trigger = true;
	ret = pthread_create(&task->tid, NULL, hls_segmentor_task, (void*)task);
	HLS_ASSERT(0 == ret, "Create segmentor task failed");
	return 0;
}

int HLS_stop_segmentor_task(HLS_SEGMENT_TASK_t* task)
{
	task->trigger = false;
	pthread_join(task->tid, NULL);
	return 0;
}

