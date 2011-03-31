#include "emu.h"

class snowbros_state : public driver_device
{
public:
	snowbros_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *hyperpac_ram;
	int sb3_music_is_playing;
	int sb3_music;
	UINT8 semicom_prot_offset;
	UINT8 *spriteram;
	UINT16 *bootleg_spriteram16;
	size_t spriteram_size;
};

