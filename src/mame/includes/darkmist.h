// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria, Tomasz Slanina
#include "audio/t5182.h"

class darkmist_state : public driver_device
{
public:
	darkmist_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t5182(*this, "t5182"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	required_device<t5182_device> m_t5182;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spritebank;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	int m_hw;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_txtilemap;

	void hw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bgtile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fgtile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_txttile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void init_darkmist();
	virtual void video_start() override;
	void palette_init_darkmist(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decrypt_fgbgtiles(uint8_t* rgn, int size);
	void decrypt_gfx();
	void decrypt_snd();

	void scanline(timer_device &timer, void *ptr, int32_t param);
};
