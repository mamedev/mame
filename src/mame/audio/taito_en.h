#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"

READ16_HANDLER(f3_68000_share_r);
WRITE16_HANDLER(f3_68000_share_w);
READ16_HANDLER(f3_68681_r);
WRITE16_HANDLER(f3_68681_w);
READ16_HANDLER(es5510_dsp_r);
WRITE16_HANDLER(es5510_dsp_w);
WRITE16_HANDLER(f3_volume_w);
WRITE16_HANDLER(f3_es5505_bank_w);
void f3_68681_reset(running_machine *machine);

void taito_f3_soundsystem_reset(running_machine *machine);

#define TAITO_F3_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD("audiocpu",  M68000, freq)							\
	MDRV_CPU_PROGRAM_MAP(f3_sound_map,0)							\


#define TAITO_F3_SOUND_SYSTEM_ES5505(freq)							\
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")					\
	MDRV_SOUND_ADD("ensoniq", ES5505, freq)							\
	MDRV_SOUND_CONFIG(es5505_taito_f3_config)						\
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)								\
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)								\

ADDRESS_MAP_EXTERN(f3_sound_map, 16);

extern const es5505_interface es5505_taito_f3_config;
