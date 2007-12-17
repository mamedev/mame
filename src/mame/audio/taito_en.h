
READ16_HANDLER(f3_68000_share_r);
WRITE16_HANDLER(f3_68000_share_w);
READ16_HANDLER(f3_68681_r);
WRITE16_HANDLER(f3_68681_w);
READ16_HANDLER(es5510_dsp_r);
WRITE16_HANDLER(es5510_dsp_w);
WRITE16_HANDLER(f3_volume_w);
WRITE16_HANDLER(f3_es5505_bank_w);
void f3_68681_reset(void);

void taito_f3_soundsystem_reset(void);

#define TAITO_F3_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD(M68000, freq)										\
	MDRV_CPU_PROGRAM_MAP(f3_sound_map,0)							\


#define TAITO_F3_SOUND_SYSTEM_ES5505(freq)							\
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")					\
	MDRV_SOUND_ADD(ES5505, freq)									\
	MDRV_SOUND_CONFIG(es5505_interface)								\
	MDRV_SOUND_ROUTE(0, "left", 1.0)								\
	MDRV_SOUND_ROUTE(1, "right", 1.0)								\

ADDRESS_MAP_EXTERN(f3_sound_map);

extern struct ES5505interface es5505_interface;
