/*************************************************************************

    Atari Jaguar hardware

*************************************************************************/

#ifndef ENABLE_SPEEDUP_HACKS
#ifndef MESS
#define ENABLE_SPEEDUP_HACKS 1
#else
#define ENABLE_SPEEDUP_HACKS 0
#endif /* MESS */
#endif


/*----------- defined in drivers/cojag.c -----------*/

extern UINT8 cojag_is_r3000;

extern UINT32 *jaguar_shared_ram;
extern UINT32 *jaguar_gpu_ram;
extern UINT32 *jaguar_gpu_clut;
extern UINT32 *jaguar_dsp_ram;
extern UINT32 *jaguar_wave_rom;


/*----------- defined in audio/jaguar.c -----------*/

void jaguar_dsp_suspend(void);
void jaguar_dsp_resume(void);

void cojag_sound_init(void);
void cojag_sound_reset(void);

void jaguar_external_int(int state);

READ16_HANDLER( jaguar_jerry_regs_r );
WRITE16_HANDLER( jaguar_jerry_regs_w );
READ32_HANDLER( jaguar_jerry_regs32_r );
WRITE32_HANDLER( jaguar_jerry_regs32_w );

READ32_HANDLER( jaguar_serial_r );
WRITE32_HANDLER( jaguar_serial_w );


/*----------- defined in video/jaguar.c -----------*/

void jaguar_gpu_suspend(void);
void jaguar_gpu_resume(void);

void jaguar_gpu_cpu_int(void);
void jaguar_dsp_cpu_int(void);

READ32_HANDLER( jaguar_blitter_r );
WRITE32_HANDLER( jaguar_blitter_w );

READ16_HANDLER( jaguar_tom_regs_r );
WRITE16_HANDLER( jaguar_tom_regs_w );
READ32_HANDLER( jaguar_tom_regs32_r );
WRITE32_HANDLER( jaguar_tom_regs32_w );

READ32_HANDLER( cojag_gun_input_r );

VIDEO_START( cojag );
VIDEO_UPDATE( cojag );
