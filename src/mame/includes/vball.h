// license:BSD-3-Clause
// copyright-holders:Paul Hampson

#include "machine/gen_latch.h"

class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_attribram(*this, "attribram"),
		m_videoram(*this, "videoram"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_attribram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrolly_lo;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_scrollx_hi;
	int m_scrolly_hi;
	int m_scrollx_lo;
	int m_gfxset;
	int m_scrollx[256];
	int m_bgprombank;
	int m_spprombank;
	tilemap_t *m_bg_tilemap;

	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void attrib_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	tilemap_memory_index background_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vball_scanline(timer_device &timer, void *ptr, int32_t param);
	void bgprombank_w(int bank);
	void spprombank_w(int bank);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scanline_to_vcount(int scanline);
};
