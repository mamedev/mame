// license:BSD-3-Clause
// copyright-holders:David Haywood, Mike Coates

#include "machine/gen_latch.h"
#include "machine/timer.h"
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
		m_soundlatch(*this, "soundlatch"),
		m_pandora(*this, "pandora"),
		m_hyperpac_ram(*this, "hyperpac_ram"),
		m_bootleg_spriteram16(*this, "spriteram16b")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not snowbro3

	optional_device<kaneko_pandora_device> m_pandora;
	optional_shared_ptr<uint16_t> m_hyperpac_ram;
	optional_shared_ptr<uint16_t> m_bootleg_spriteram16;

	int m_sb3_music_is_playing;
	int m_sb3_music;
	uint8_t m_semicom_prot_offset;

	DECLARE_WRITE8_MEMBER(snowbros_flipscreen_w);
	DECLARE_WRITE8_MEMBER(bootleg_flipscreen_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq4_ack_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq3_ack_w);
	DECLARE_WRITE16_MEMBER(snowbros_irq2_ack_w);
	DECLARE_WRITE8_MEMBER(prot_p0_w);
	DECLARE_WRITE8_MEMBER(prot_p1_w);
	DECLARE_WRITE8_MEMBER(prot_p2_w);
	DECLARE_READ16_MEMBER(sb3_sound_r);
	DECLARE_READ16_MEMBER(_4in1_02_read);
	DECLARE_READ16_MEMBER(_3in1_read);
	DECLARE_READ16_MEMBER(cookbib3_read);
	DECLARE_WRITE8_MEMBER(twinadv_oki_bank_w);
	DECLARE_WRITE16_MEMBER(sb3_sound_w);
	DECLARE_READ16_MEMBER(toto_read);

	void init_pzlbreak();
	void init_snowbro3();
	void init_cookbib3();
	void init_4in1boot();
	void init_3in1semi();
	void init_cookbib2();
	void init_toto();
	void init_hyperpac();
	void init_yutnori();
	DECLARE_MACHINE_RESET(semiprot);
	DECLARE_MACHINE_RESET(finalttr);

	uint32_t screen_update_snowbros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_honeydol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_twinadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_snowbro3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wintbob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_snowbros);

	TIMER_DEVICE_CALLBACK_MEMBER(snowbros_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(snowbros3_irq);

	void sb3_play_music(int data);
	void sb3_play_sound(int data);
	void _4in1(machine_config &config);
	void semiprot(machine_config &config);
	void semicom_mcu(machine_config &config);
	void yutnori(machine_config &config);
	void snowbros(machine_config &config);
	void semicom(machine_config &config);
	void twinadv(machine_config &config);
	void wintbob(machine_config &config);
	void honeydol(machine_config &config);
	void snowbro3(machine_config &config);
	void finalttr(machine_config &config);
	void finalttr_map(address_map &map);
	void honeydol_map(address_map &map);
	void honeydol_sound_io_map(address_map &map);
	void honeydol_sound_map(address_map &map);
	void hyperpac_map(address_map &map);
	void hyperpac_sound_map(address_map &map);
	void snowbros3_map(address_map &map);
	void snowbros_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
	void twinadv_map(address_map &map);
	void twinadv_sound_io_map(address_map &map);
	void wintbob_map(address_map &map);
	void yutnori_map(address_map &map);
};
