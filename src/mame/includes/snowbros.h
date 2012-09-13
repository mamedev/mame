#include "emu.h"

class snowbros_state : public driver_device
{
public:
	snowbros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_hyperpac_ram(*this, "hyperpac_ram"),
		m_bootleg_spriteram16(*this, "spriteram16b"),
		m_maincpu(*this,"maincpu"){ }

	optional_shared_ptr<UINT16> m_hyperpac_ram;
	int m_sb3_music_is_playing;
	int m_sb3_music;
	UINT8 m_semicom_prot_offset;
	UINT8 *m_spriteram;
	optional_shared_ptr<UINT16> m_bootleg_spriteram16;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE16_MEMBER(snowbros_flipscreen_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq4_ack_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq3_ack_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq2_ack_w);
	DECLARE_READ16_MEMBER(snowbros_68000_sound_r);
	DECLARE_WRITE16_MEMBER(snowbros_68000_sound_w);
	DECLARE_WRITE16_MEMBER(semicom_soundcmd_w);
	DECLARE_READ8_MEMBER(prot_io_r);
	DECLARE_WRITE8_MEMBER(prot_io_w);
	DECLARE_WRITE16_MEMBER(twinadv_68000_sound_w);
	DECLARE_READ16_MEMBER(sb3_sound_r);
	DECLARE_READ16_MEMBER(moremorp_0a_read);
	DECLARE_READ16_MEMBER(_4in1_02_read);
	DECLARE_READ16_MEMBER(_3in1_read);
	DECLARE_READ16_MEMBER(cookbib3_read);
	DECLARE_WRITE8_MEMBER(twinadv_oki_bank_w);
	DECLARE_WRITE16_MEMBER(sb3_sound_w);
	DECLARE_DRIVER_INIT(pzlbreak);
	DECLARE_DRIVER_INIT(moremorp);
	DECLARE_DRIVER_INIT(snowbro3);
	DECLARE_DRIVER_INIT(cookbib3);
	DECLARE_DRIVER_INIT(4in1boot);
	DECLARE_DRIVER_INIT(3in1semi);
	DECLARE_DRIVER_INIT(cookbib2);
	DECLARE_MACHINE_RESET(semiprot);
	DECLARE_MACHINE_RESET(finalttr);
};

