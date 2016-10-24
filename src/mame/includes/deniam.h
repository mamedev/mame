// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

/*************************************************************************

    Deniam games

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/okim6295.h"

class deniam_state : public driver_device
{
public:
	deniam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_textram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_paletteram;

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_display_enable;
	int            m_bg_scrollx_offs;
	int            m_bg_scrolly_offs;
	int            m_fg_scrollx_offs;
	int            m_fg_scrolly_offs;
	int            m_bg_scrollx_reg;
	int            m_bg_scrolly_reg;
	int            m_bg_page_reg;
	int            m_fg_scrollx_reg;
	int            m_fg_scrolly_reg;
	int            m_fg_page_reg;
	int            m_bg_page[4];
	int            m_fg_page[4];
	uint16_t         m_coinctrl;

	/* devices */
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void deniam_irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void deniam_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void deniam_textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void deniam_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t deniam_coinctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void deniam_coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void deniam16b_oki_rom_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void deniam16c_oki_rom_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_karianx();
	void init_logicpro();
	tilemap_memory_index scan_pages(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_deniam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deniam_common_init(  );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void set_bg_page( int page, int value );
	void set_fg_page( int page, int value );
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu; // system 16c does not have sound CPU
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
};
