#include "emu.h"

class snowbros_state : public driver_device
{
public:
	snowbros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_hyperpac_ram;
	int m_sb3_music_is_playing;
	int m_sb3_music;
	UINT8 m_semicom_prot_offset;
	UINT8 *m_spriteram;
	UINT16 *m_bootleg_spriteram16;
	size_t m_spriteram_size;

	required_device<cpu_device> m_maincpu;
};

