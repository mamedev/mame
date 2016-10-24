// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"
#include "video/vsystem_spr2.h"

class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram") { }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr2_device> m_spr_old;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_charvideoram;

	tilemap_t *m_char_tilemap;
	int m_pending_command;
	uint8_t m_gfxbank[2];
	uint16_t m_charpalettebank;
	uint16_t m_spritepalettebank;
	uint16_t m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;

	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfxbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scrollreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void charvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	ioport_value pending_sound_r(ioport_field &field, void *param);

	void init_quiz18k();
	void init_welltris();
	virtual void machine_start() override;
	virtual void video_start() override;

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setbank(int num, int bank);
};
