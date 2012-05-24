/*************************************************************************

    Atari Jaguar hardware

*************************************************************************/

#ifndef ENABLE_SPEEDUP_HACKS
#define ENABLE_SPEEDUP_HACKS 1
#endif

/* CoJag and Jaguar have completely different XTALs, pixel clock in Jaguar is the same as the GPU one */
#define COJAG_PIXEL_CLOCK		XTAL_14_31818MHz
#define JAGUAR_CLOCK			XTAL_25_590906MHz // NTSC
// XTAL_25_593900MHz PAL, TODO

/*----------- defined in drivers/cojag.c -----------*/

extern UINT8 cojag_is_r3000;

extern UINT32 *jaguar_shared_ram;
extern UINT32 *jaguar_gpu_ram;
extern UINT32 *jaguar_gpu_clut;
extern UINT32 *jaguar_dsp_ram;
extern UINT32 *jaguar_wave_rom;
extern bool jaguar_hacks_enabled;

/*----------- defined in audio/jaguar.c -----------*/

TIMER_DEVICE_CALLBACK( jaguar_serial_callback );

void jaguar_dsp_suspend(running_machine &machine);
void jaguar_dsp_resume(running_machine &machine);

void cojag_sound_init(running_machine &machine);

void jaguar_external_int(device_t *device, int state);

READ16_HANDLER( jaguar_jerry_regs_r );
WRITE16_HANDLER( jaguar_jerry_regs_w );

READ32_HANDLER( jaguar_serial_r );
WRITE32_HANDLER( jaguar_serial_w );


/*----------- defined in video/jaguar.c -----------*/

extern UINT8 blitter_status;

void jaguar_gpu_suspend(running_machine &machine);
void jaguar_gpu_resume(running_machine &machine);

void jaguar_gpu_cpu_int(device_t *device);
void jaguar_dsp_cpu_int(device_t *device);

READ32_HANDLER( jaguar_blitter_r );
WRITE32_HANDLER( jaguar_blitter_w );

READ16_HANDLER( jaguar_tom_regs_r );
WRITE16_HANDLER( jaguar_tom_regs_w );

READ32_HANDLER( cojag_gun_input_r );

VIDEO_START( cojag );
VIDEO_START( jaguar );
SCREEN_UPDATE_RGB32( cojag );
