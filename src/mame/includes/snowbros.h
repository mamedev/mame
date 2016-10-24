// license:BSD-3-Clause
// copyright-holders:David Haywood, Mike Coates

#include "machine/gen_latch.h"
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

	void snowbros_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void snowbros_irq4_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void snowbros_irq3_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void snowbros_irq2_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t snowbros_68000_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void snowbros_68000_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void semicom_soundcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t prot_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void prot_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void twinadv_68000_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sb3_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t _4in1_02_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t _3in1_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t cookbib3_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twinadv_oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sb3_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t toto_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void init_pzlbreak();
	void init_snowbro3();
	void init_cookbib3();
	void init_4in1boot();
	void init_3in1semi();
	void init_cookbib2();
	void init_toto();
	void init_hyperpac();
	void init_yutnori();
	void machine_reset_semiprot();
	void machine_reset_finalttr();

	uint32_t screen_update_snowbros(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_honeydol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_twinadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_snowbro3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wintbob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_snowbros(screen_device &screen, bool state);

	void snowbros_irq(timer_device &timer, void *ptr, int32_t param);
	void snowbros3_irq(timer_device &timer, void *ptr, int32_t param);

	void sb3_play_music(int data);
	void sb3_play_sound(int data);
};
