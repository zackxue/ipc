
static int unistruct_hi3507_inception_rw(SYSCONF_t* const sysconf, const char* xpath, char* io_text, SYS_BOOL_t opt_rw)
{
	char attr[128];
	int ain_ch = 0;
	int ain_encode_ch = 0;
	int vin_ch = 0;
	int vin_enc_h264_ch = 0;
	int vin_enc_h264_stream_ch = 0;
	int vin_enc_jpeg_ch = 0;
	int ptz_ch = 0;
	int ptz_tour_point_ch = 0;
	int port_ch = 0;
	UNISTRUCT_SYSCONF_t unistruct;

	// basically init
	STRUCT_ZERO(unistruct);
	unistruct.xpath = strdupa(xpath);

#define UNI_ATTR_MATCH(a_name) (0 == strcmp(attr, a_name))

	if(2 == sscanf(xpath, "ain%d@%s", &ain_ch, attr)){
		SYS_AIN_t* p_ain = &sysconf->ipcam.ain[ain_ch];
		UNISTRUCT_ASSERT(ain_ch < sysconf->ipcam.spec.ain, "ain ch=%d override!", ain_ch);
		if(UNI_ATTR_MATCH("sample_rate")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_ain->sample_rate;
			unistruct.map_enu = (void*)ain_sample_rate_map;
			unistruct.map_enu_size = ARRAY_ITEM(ain_sample_rate_map);
		}else if(UNI_ATTR_MATCH("sample_width")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_ain->sample_width;
			unistruct.map_enu = (void*)ain_sample_width_map;
			unistruct.map_enu_size = ARRAY_ITEM(ain_sample_width_map);
		}
	}else if(3 == sscanf(xpath, "ain%d/encode%d@%s", &ain_ch, &ain_encode_ch, attr)){
		SYS_AIN_ENC_t* p_aenc = &sysconf->ipcam.ain[ain_ch].enc[ain_encode_ch];
		UNISTRUCT_ASSERT(ain_ch < sysconf->ipcam.spec.ain && ain_encode_ch < sysconf->ipcam.ain[ain_ch].enc_ch,
			"ain ch=%d ain enc=%d override!", ain_ch, ain_encode_ch);
		if(UNI_ATTR_MATCH("engine")){
			unistruct.type = SYSCONF_TYPE_STRING;
			unistruct.val_ptr = &p_aenc->engine;
		}else if(UNI_ATTR_MATCH("packet")){
			unistruct.type = SYSCONF_TYPE_U16;
			unistruct.val_ptr = &p_aenc->packet;
		}
	}
	// vin
	else if(2 == sscanf(xpath, "vin%d@%s", &vin_ch, attr)){
		SYS_VIN_t* const p_vin = &sysconf->ipcam.vin[vin_ch];
		UNISTRUCT_ASSERT(vin_ch < sysconf->ipcam.spec.vin, "vin ch=%d override!", vin_ch);
		if(UNI_ATTR_MATCH("shutter")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_vin->digital_shutter;
			unistruct.map_enu = (void*)vin_digital_shutter_map;
			unistruct.map_enu_size = ARRAY_ITEM(vin_digital_shutter_map);
		}else if(UNI_ATTR_MATCH("standard")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_vin->analog_standard;
			unistruct.map_enu = (void*)vin_analog_standard_map;
			unistruct.map_enu_size = ARRAY_ITEM(vin_analog_standard_map);
		}else if(UNI_ATTR_MATCH("hue")){
			unistruct.type = SYSCONF_TYPE_RATE16;
			unistruct.val_ptr = &p_vin->hue;	
		}else if(UNI_ATTR_MATCH("contrast")){
			unistruct.type = SYSCONF_TYPE_RATE8;
			unistruct.val_ptr = &p_vin->contrast;
			unistruct.do_check = do_check_contrast;
			unistruct.do_policy = do_policy_contrast;
		}else if(UNI_ATTR_MATCH("brightness")){
			unistruct.type = SYSCONF_TYPE_RATE8;
			unistruct.val_ptr = &p_vin->brightness;
			unistruct.do_check = do_check_brightness;
			unistruct.do_policy = do_policy_brightness;
		}else if(UNI_ATTR_MATCH("saturation")){
			unistruct.type = SYSCONF_TYPE_RATE8;
			unistruct.val_ptr = &p_vin->saturation;
			unistruct.do_check = do_check_saturation;
			unistruct.do_policy = do_policy_saturation;
		}
		else if(UNI_ATTR_MATCH("flip")){
			unistruct.type = SYSCONF_TYPE_BOOL;
			unistruct.val_ptr = &p_vin->flip;
			unistruct.do_policy = do_policy_flip;
		}
		else if(UNI_ATTR_MATCH("mirror")){
			unistruct.type = SYSCONF_TYPE_BOOL;
			unistruct.val_ptr = &p_vin->mirror;
			unistruct.do_policy = do_policy_mirror;
		}
		else if(UNI_ATTR_MATCH("channel_name")){
			unistruct.type = SYSCONF_TYPE_STRING;
			unistruct.val_ptr = &p_vin->channel_name;
			unistruct.do_policy = do_policy_channel_name;
		}
	}else if(4 == sscanf(xpath, "vin%d/encode_h264%d/stream%d@%s",	&vin_ch, &vin_enc_h264_ch, &vin_enc_h264_stream_ch, attr)){
		SYS_VIN_ENC_H264_STREAM_t* const p_h264_stream = &sysconf->ipcam.vin[vin_ch].enc_h264[vin_enc_h264_ch].stream[vin_enc_h264_stream_ch];
		UNISTRUCT_ASSERT(vin_ch < sysconf->ipcam.spec.vin
			&& vin_enc_h264_ch < sysconf->ipcam.vin[vin_ch].enc_h264_ch
			&& vin_enc_h264_stream_ch < sysconf->ipcam.vin[vin_ch].enc_h264[vin_enc_h264_ch].stream_ch,
			"vin ch=%d enc=%d h264=%d override!", vin_ch, vin_enc_h264_ch, vin_enc_h264_stream_ch);
		
		if(UNI_ATTR_MATCH("name")){
			unistruct.type = SYSCONF_TYPE_STRING;
			unistruct.val_ptr = p_h264_stream->name;
		}else if(UNI_ATTR_MATCH("profile")){
			unistruct.type = SYSCONF_TYPE_STRING;
			unistruct.val_ptr = p_h264_stream->profile;
		}else if(UNI_ATTR_MATCH("size")){
			unistruct.type = SYSCONF_TYPE_ENUM;
			unistruct.val_ptr = &p_h264_stream->size;
			unistruct.map_enu = (void*)vin_size_enum;
			unistruct.map_enu_size = ARRAY_ITEM(vin_size_enum);
		}else if(UNI_ATTR_MATCH("mode")){
			unistruct.type = SYSCONF_TYPE_ENUM;
			unistruct.val_ptr = &p_h264_stream->mode;
			unistruct.map_enu = (void*)vin_enc_h264_mode_enum;
			unistruct.map_enu_size = ARRAY_ITEM(vin_enc_h264_mode_enum);
			unistruct.do_policy = do_policy_video;
		}else if(UNI_ATTR_MATCH("on_demand")){
			unistruct.type = SYSCONF_TYPE_U8;
			unistruct.val_ptr = &p_h264_stream->on_demand;
		}else if(UNI_ATTR_MATCH("fps")){
			unistruct.type = SYSCONF_TYPE_U8;
			unistruct.val_ptr = &p_h264_stream->fps;
			unistruct.do_policy = do_policy_video;
		}else if(UNI_ATTR_MATCH("gop")){
			unistruct.type = SYSCONF_TYPE_U16;
			unistruct.val_ptr = &p_h264_stream->gop;
		}else if(UNI_ATTR_MATCH("ain_bind")){
			unistruct.type = SYSCONF_TYPE_U8;
			unistruct.val_ptr = &p_h264_stream->ain_bind;
		}else if(UNI_ATTR_MATCH("quality")){
			unistruct.type = SYSCONF_TYPE_LEVEL;
			unistruct.val_ptr = &p_h264_stream->quality;
			unistruct.do_policy = do_policy_video;
		}else if(UNI_ATTR_MATCH("bps")){
			unistruct.type = SYSCONF_TYPE_U32;
			unistruct.val_ptr = &p_h264_stream->bps;
			unistruct.do_policy = do_policy_video;
		}
	}else if(3 == sscanf(xpath, "vin%d/encode_jpeg%d@%s", &vin_ch, &vin_enc_jpeg_ch, attr)){
		SYS_VIN_ENC_JPEG_t* const p_jpeg = &sysconf->ipcam.vin[vin_ch].enc_jpeg[vin_enc_jpeg_ch];
		if(vin_ch < sysconf->ipcam.spec.vin && vin_enc_jpeg_ch < sysconf->ipcam.vin[vin_ch].enc_jpeg_ch){
			if(UNI_ATTR_MATCH("name")){
				unistruct.type = SYSCONF_TYPE_STRING;
				unistruct.val_ptr = p_jpeg->name;
			}else if(UNI_ATTR_MATCH("quality")){
				unistruct.type = SYSCONF_TYPE_LEVEL;
				unistruct.val_ptr = &p_jpeg->quality;
			}else if(UNI_ATTR_MATCH("size")){
				unistruct.type = SYSCONF_TYPE_ENUM;
				unistruct.val_ptr = &p_jpeg->size;
				unistruct.map_enu = (void*)vin_size_enum;
				unistruct.map_enu_size = ARRAY_ITEM(vin_size_enum);
			}
		}
	}
	// ptz
	else if(2 == sscanf(xpath, "ptz%d@%s",	&ptz_ch, attr)){
		SYS_PTZ_t* const p_ptz = &sysconf->ipcam.ptz[ptz_ch];
		UNISTRUCT_ASSERT(ptz_ch < sysconf->ipcam.spec.ptz, "ptz ch=%d override!", ptz_ch);
		if(UNI_ATTR_MATCH("baudrate")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_ptz->baudrate;
			unistruct.map_enu = (void*)ptz_baudrate_map;
			unistruct.map_enu_size = ARRAY_ITEM(ptz_baudrate_map);
		}else if(UNI_ATTR_MATCH("addr")){
			unistruct.type = SYSCONF_TYPE_U8;
			unistruct.val_ptr = &p_ptz->addr;
		}else if(UNI_ATTR_MATCH("protocol")){
			unistruct.type = SYSCONF_TYPE_MAP;
			unistruct.val_ptr = &p_ptz->protocol;
			unistruct.map_enu = (void*)ptz_protocol_map;
			unistruct.map_enu_size = ARRAY_ITEM(ptz_protocol_map);
		}
	}else if(2 == sscanf(xpath, "ptz%d/tour@%s", &ptz_ch, attr)){
		SYS_PTZ_TOUR_t* const p_tour = &sysconf->ipcam.ptz[ptz_ch].tour;
		UNISTRUCT_ASSERT(ptz_ch < sysconf->ipcam.spec.ptz, "ptz ch=%d override!", ptz_ch);
		if(UNI_ATTR_MATCH("active")){
			unistruct.type = SYSCONF_TYPE_RATE16;
			unistruct.val_ptr = &p_tour->active;
		}
	}else if(3 == sscanf(xpath, "ptz%d/tour/point%d@%s", &ptz_ch, &ptz_tour_point_ch, attr)){
		SYS_PTZ_TOUR_POINT_t* const p_point = &sysconf->ipcam.ptz[ptz_ch].tour.point[ptz_tour_point_ch];
		UNISTRUCT_ASSERT(ptz_ch < sysconf->ipcam.spec.ptz
			&& ptz_tour_point_ch < sysconf->ipcam.ptz[ptz_ch].tour.active.max,
			"ptz ch=%d tour=%d override!", ptz_ch, ptz_tour_point_ch);
		if(UNI_ATTR_MATCH("preset")){
			unistruct.type = SYSCONF_TYPE_U8;
			unistruct.val_ptr = &p_point->preset;
		}else if(UNI_ATTR_MATCH("time")){
			unistruct.type = SYSCONF_TYPE_TIME;
			unistruct.val_ptr = &p_point->time;
		}
	}
	// network
	else if(2 == sscanf(xpath, "network/lan/port%d@%s", &port_ch, attr)){
		SYS_NETWORK_LAN_PORT_t* const p_port = &sysconf->ipcam.network.lan.port[port_ch];
		UNISTRUCT_ASSERT(port_ch < sysconf->ipcam.network.lan.port_active.val, "port ch=%d", port_ch);
		if(UNI_ATTR_MATCH("name")){
			unistruct.type = SYSCONF_TYPE_STRING;
			unistruct.val_ptr = &p_port->name;
		}else if(UNI_ATTR_MATCH("value")){
			unistruct.type = SYSCONF_TYPE_U16;
			unistruct.val_ptr = &p_port->value;
			unistruct.do_policy = do_policy_network;
		}
	}
	
	if(SYS_NULL != unistruct.val_ptr){
		return unistruct_item_rw(&unistruct, io_text, opt_rw);
	}
	return -1;
}




