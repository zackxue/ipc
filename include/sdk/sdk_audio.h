
#include "sdk_def.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_AUDIO_H_
#define SDK_AUDIO_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum SDK_AUDIO_HW_SPEC {
	kSDK_AUDIO_HW_SPEC_IGNORE = 0,
	//
}enSDK_AUDIO_HW_SPEC;

typedef enum SDK_AUDIO_SAMPLE_RATE {
	kSDK_AUDIO_SAMPLE_RATE_AUTO = 0,
	kSDK_AUDIO_SAMPLE_RATE_8000,
	kSDK_AUDIO_SAMPLE_RATE_12000, 
	kSDK_AUDIO_SAMPLE_RATE_11025,
	kSDK_AUDIO_SAMPLE_RATE_16000,
	kSDK_AUDIO_SAMPLE_RATE_22050,
	kSDK_AUDIO_SAMPLE_RATE_24000,
	kSDK_AUDIO_SAMPLE_RATE_32000,
	kSDK_AUDIO_SAMPLE_RATE_44100,
	kSDK_AUDIO_SAMPLE_RATE_48000,
}enSDK_AUDIO_SAMPLE_RATE;

typedef struct SDK_AUDIO_API {

	int (*init_ain)(enSDK_AUDIO_SAMPLE_RATE sample_rate, int sample_width);
	int (*destroy_ain)(void);
	
	int (*create_ain_ch)(int ch);
	int (*release_ain_ch)(int ch);
	int (*get_ain_ch_attr)(int ch, enSDK_AUDIO_SAMPLE_RATE *sample_rate, int *sample_width, int *packet_size);

	int (*set_aout_loop)(int ain);
	int (*set_aout_play)(int aout);

}stSDK_AUDIO_API, *lpSDK_AUDIO_API;

// could be used after 'SDK_init_ain' success to call
extern lpSDK_AUDIO_API sdk_audio;

extern int SDK_init_audio(enSDK_AUDIO_HW_SPEC hw_spec);
extern int SDK_destroy_audio();

#ifdef __cplusplus
};
#endif
#endif //SDK_AUDIO_H_

