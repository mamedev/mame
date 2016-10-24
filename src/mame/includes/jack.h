// license:BSD-3-Clause
// copyright-holders:Brad Oliver

/*************************************************************************

    Jack the Giant Killer

*************************************************************************/

#include "machine/gen_latch.h"

class jack_state : public driver_device
{
public:
	jack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* device- and memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int m_timer_rate;
	uint8_t m_joinem_nmi_enable;
	uint8_t m_joinem_palette_bank;
	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];

	void jack_sh_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void joinem_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void joinem_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t striv_question_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void jack_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jack_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t jack_flipscreen_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void jack_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void init_zzyzzyxx();
	void init_striv();
	void init_treahunt();
	void init_loverboy();
	void init_jack();

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_cols_flipy(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void joinem_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_joinem();
	void palette_init_joinem(palette_device &palette);
	void machine_start_striv();
	void machine_reset_striv();
	void machine_start_joinem();
	void machine_reset_joinem();

	uint32_t screen_update_jack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_striv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_joinem(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void joinem_vblank_irq(device_t &device);
	void jack_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void joinem_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void treahunt_decode(  );
};
