#include "emu.h"

class snowbros_state : public driver_device
{
public:
	snowbros_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_hyperpac_ram;
	int m_sb3_music_is_playing;
	int m_sb3_music;
	UINT8 m_semicom_prot_offset;
	UINT8 *m_spriteram;
	UINT16 *m_bootleg_spriteram16;
	size_t m_spriteram_size;
};

