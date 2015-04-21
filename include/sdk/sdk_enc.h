
#include "sdk_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>

#ifndef SDK_ENC_H_
#define SDK_ENC_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kSDK_ENC_BUF_DATA_MAGIC (0xff00a0a0UL)
typedef enum SDK_ENC_BUF_DATA_TYPE {
	// video
	kSDK_ENC_BUF_DATA_H264 = (0x00000000UL),
	kSDK_ENC_BUF_DATA_JPEG,
	// audio
	kSDK_ENC_BUF_DATA_PCM = (0x80000000UL),
	kSDK_ENC_BUF_DATA_G711A,
	kSDK_ENC_BUF_DATA_G711U,
}enSDK_ENC_BUF_DATA_TYPE, *lpSDK_ENC_BUF_DATA_TYPE;

typedef struct SDK_ENC_BUF_ATTR {
	// public attr
	uint32_t magic; // must be "kSdkEncBufDataMagic"

	enSDK_ENC_BUF_DATA_TYPE type;
	uint64_t timestamp_us; // the timestamp of soc engine, unit: us
	uint64_t time_us; // the timestamp of system clock, unit: us
	size_t data_sz;
	union {
		// kSdkEncBufDataH264
		struct {
			bool keyframe; // TRUE / FALSE
			uint32_t ref_counter; // ref frame counter
			uint32_t fps;
			uint32_t width;
			uint32_t height;
		}h264;
		// kSdkEncBufDataPcm, kSdkEncBufDataG711a, kSdkEncBufDataG711u
		struct {
			uint32_t sample_rate;
			uint32_t sample_width;
			uint32_t packet;
			float compression_ratio; // if g711a.u == 2.0
		}pcm, g711a, g711u;
	};
}stSDK_ENC_BUF_ATTR, *lpSDK_ENC_BUF_ATTR;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef enum SDK_ENC_H264_RC_MODE {
	kSDK_ENC_H264_RC_MODE_AUTO,
	kSDK_ENC_H264_RC_MODE_VBR,
	kSDK_ENC_H264_RC_MODE_CBR,
	kSDK_ENC_H264_RC_MODE_ABR,
	kSDK_ENC_H264_RC_MODE_FIXQP,
}enSDK_ENC_H264_RC_MODE, *lpSDK_ENC_H264_RC_MODE;

typedef enum SDK_ENC_H264_PROFILE {
	kSDK_ENC_H264_PROFILE_AUTO,
	kSDK_ENC_H264_PROFILE_BASELINE,
	kSDK_ENC_H264_PROFILE_MAIN,
	kSDK_ENC_H264_PROFILE_HIGH,
}enSDK_ENC_H264_PROFILE, *lpSDK_ENC_H264_PROFILE;

typedef struct SDK_ENC_H264_STREAM_ATTR {
	uint32_t magic; // ignore when init
	char name[32]; // ignore when init
	int vin; // ignore when init
	int stream; // ignore when init
	uint32_t vin_fps;
	size_t  width, height;
	uint32_t fps, gop;
	enSDK_ENC_H264_PROFILE profile;
	enSDK_ENC_H264_RC_MODE rc_mode;
	uint32_t bps;
	uint32_t quality;
	size_t frame_limit;
	int buf_id;
	bool start;
	
}stSDK_ENC_H264_STREAM_ATTR, *lpSDK_ENC_H264_STREAM_ATTR;

typedef struct SDK_ENC_G711A_STREAM_ATTR {
	uint32_t magic; // ignore when init
	int ain; // ignore when init
	int vin_ref; // ignore when init
	uint16_t sample_rate, sample_width;
	size_t packet_size;
	bool start;
	
}stSDK_ENC_G711A_STREAM_ATTR, *lpSDK_ENC_G711A_STREAM_ATTR;

typedef enum SDK_ENC_SNAPSHOT_QUALITY {
	kSDK_ENC_SNAPSHOT_QUALITY_HIGHEST,
	kSDK_ENC_SNAPSHOT_QUALITY_HIGH,
	kSDK_ENC_SNAPSHOT_QUALITY_MEDIUM,
	kSDK_ENC_SNAPSHOT_QUALITY_LOW,
	kSDK_ENC_SNAPSHOT_QUALITY_LOWEST,
}enSDK_ENC_SNAPSHOT_QUALITY, *lpSDK_ENC_SNAPSHOT_QUALITY;

#define kSDK_ENC_SNAPSHOT_SIZE_AUTO (0)
#define kSDK_ENC_SNAPSHOT_SIZE_MAX (-1)
#define kSDK_ENC_SNAPSHOT_SIZE_MIN (-2)

typedef union SDK_ENC_VIDEO_OVERLAY_PIXEL {
	uint32_t argb8888;
	struct {
		uint8_t blue, green, red, alpha;
	};
}stSDK_ENC_VIDEO_OVERLAY_PIXEL, *lpSDK_ENC_VIDEO_OVERLAY_PIXEL;

#define SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(_pixel, _a, _r, _g, _b) \
	do{\
		_pixel.alpha = (_a), _pixel.red = (_r), _pixel.green = (_g), _pixel.blue = (_b);\
	}while(0)

typedef struct SDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT {
	uint32_t rmask, gmask, bmask, amask;
}stSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT, *lpSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT;

typedef struct SDK_ENC_VIDEO_OVERLAY_CANVAS {
	size_t width, height;
	stSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT pixel_format;
	void* pixels;

	int (*put_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*get_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		lpSDK_ENC_VIDEO_OVERLAY_PIXEL ret_pixel);
	bool (*match_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel1, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel2);
	int (*put_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		size_t width, size_t height,stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*fill_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		size_t width, size_t height, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*erase_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y, size_t width, size_t height);
	
}stSDK_ENC_VIDEO_OVERLAY_CANVAS, *lpSDK_ENC_VIDEO_OVERLAY_CANVAS;

typedef int (*fSDK_ENC_DO_BUFFER_REQUEST)(int buf_id, size_t data_size, bool key_flag);
typedef int (*fSDK_ENC_DO_BUFFER_APPEND)(int buf_id, const void *data_piece, ssize_t data_piece_size);
typedef int (*fSDK_ENC_DO_BUFFER_COMMIT)(int buf_id);

typedef struct SDK_ENC_API {
	
	// enc buffering callback
	fSDK_ENC_DO_BUFFER_REQUEST do_buffer_request;
	fSDK_ENC_DO_BUFFER_APPEND do_buffer_append;
	fSDK_ENC_DO_BUFFER_COMMIT do_buffer_commit;
	
	//
	int (*lookup_stream_byname)(const char* name, int* ret_vin, int* ret_stream);
	int (*create_h264_stream)(const char* name, int vin, int stream, lpSDK_ENC_H264_STREAM_ATTR stream_attr);
	int (*release_h264_stream)(int vin, int stream);
	
	int (*start_h264_stream)(int vin, int stream);
	int (*stop_h264_stream)(int vin, int stream);
	
	int (*request_h264_keyframe)(int vin, int stream);

	int (*create_g711a_stream)(int ain, int vin_ref);
	int (*release_g711a_stream)(int ain);
	
	int (*start)(void);
	int (*stop)(void);

	int (*snapshot)(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, FILE* stream);

	// overlay
	lpSDK_ENC_VIDEO_OVERLAY_CANVAS (*create_overlay_canvas)(size_t width, size_t height);
	void (*release_overlay_canvas)(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas);

	int (*create_overlay)(int vin, int stream, const char* overlay_name,
		int x, int y, size_t width, size_t height, lpSDK_ENC_VIDEO_OVERLAY_CANVAS const canvas);
	int (*release_overlay)(int vin, int stream, const char* overlay_name);

	lpSDK_ENC_VIDEO_OVERLAY_CANVAS (*get_overlay_canvas)(int vin, int stream, const char* overlay_name);

	int (*show_overlay)(int vin, int stream, const char* overlay_name, bool show);
	int (*update_overlay)(int vin, int stream, const char* overlay_name);

}stSDK_ENC_API, *lpSDK_ENC_API;

// could be used after 'SDK_init_enc' success to call
extern lpSDK_ENC_API sdk_enc;

extern int SDK_init_enc();
extern int SDK_destroy_enc();

#ifdef __cplusplus
};
#endif
#endif //SDK_ENC_H_

