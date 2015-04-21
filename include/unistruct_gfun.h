#ifndef __UNISTRUCT_GFUN_H__
#define __UNISTRUCT_GFUN_H__

#ifdef  __cplusplus
extern "C" {
#endif


typedef int (*UNISTRUCT_DO_CHECK)(void* val_ptr, void* arg);
typedef int (*UNISTRUCT_DO_POLICY)(void* val_ptr, void* arg);

extern void proc_module_reboot();

extern UNISTRUCT_DO_CHECK do_check_saturation(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_saturation(void *val_ptr, void *arg);

extern UNISTRUCT_DO_CHECK do_check_contrast(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_contrast(void *val_ptr, void *arg);

extern UNISTRUCT_DO_CHECK do_check_hue(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_hue(void *val_ptr, void *arg);

extern UNISTRUCT_DO_CHECK do_check_brightness(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_brightness(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_network(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_system_operation(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_video(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_time(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_channel_name(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_flip(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_mirror(void *val_ptr, void *arg);

extern UNISTRUCT_DO_POLICY do_policy_timezone(void *val_ptr, void *arg);

extern UNISTRUCT_DO_CHECK do_check_sharpen(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_sharpen(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_scene_mode(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_white_balance_mode(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_ircut_control_mode(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_ircut_mode(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_wdr_enable(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_wdr_strength(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_exposure_mode(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_exposure_ae_compensation(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_denoise_enable(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_denoise_strength(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_anti_fog_enable(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_lowlight_enable(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_gamma(void *val_ptr, void *arg);
extern UNISTRUCT_DO_POLICY do_policy_isp_defect_pixel_enable(void *val_ptr, void *arg);


#ifdef  __cplusplus
}
#endif


#endif //__UNISTRUCT_GFUN_H__
