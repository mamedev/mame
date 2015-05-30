// license:BSD-3-Clause
// copyright-holders:David Haywood, Mike Coates
#include "emu.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h" // for the original pandora

class snowbros_state : public driver_device
{
public:
	snowbros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_pandora(*this, "pandora"),
		m_hyperpac_ram(*this, "hyperpac_ram"),
		m_bootleg_spriteram16(*this, "spriteram16b")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_device<kaneko_pandora_device> m_pandora;
	optional_shared_ptr<UINT16> m_hyperpac_ram;
	optional_shared_ptr<UINT16> m_bootleg_spriteram16;

	int m_sb3_music_is_playing;
	int m_sb3_music;
	UINT8 m_semicom_prot_offset;

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
	DECLARE_READ16_MEMBER(_4in1_02_read);
	DECLARE_READ16_MEMBER(_3in1_read);
	DECLARE_READ16_MEMBER(cookbib3_read);
	DECLARE_WRITE8_MEMBER(twinadv_oki_bank_w);
	DECLARE_WRITE16_MEMBER(sb3_sound_w);
	DECLARE_READ16_MEMBER(toto_read);

	DECLARE_DRIVER_INIT(pzlbreak);
	DECLARE_DRIVER_INIT(snowbro3);
	DECLARE_DRIVER_INIT(cookbib3);
	DECLARE_DRIVER_INIT(4in1boot);
	DECLARE_DRIVER_INIT(3in1semi);
	DECLARE_DRIVER_INIT(cookbib2);
	DECLARE_DRIVER_INIT(toto);
	DECLARE_DRIVER_INIT(hyperpac);
	DECLARE_MACHINE_RESET(semiprot);
	DECLARE_MACHINE_RESET(finalttr);

	UINT32 screen_update_snowbros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_honeydol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_twinadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_snowbro3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_wintbob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_snowbros(screen_device &screen, bool state);

	TIMER_DEVICE_CALLBACK_MEMBER(snowbros_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(snowbros3_irq);

	void sb3_play_music(int data);
	void sb3_play_sound(int data);
};
