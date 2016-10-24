// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

#include "machine/gen_latch.h"

class inufuku_state : public driver_device
{
public:
	inufuku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_rasterram(*this, "bg_rasterram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_bg_rasterram;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr<uint16_t> m_spriteram1;
	required_shared_ptr<uint16_t> m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	int       m_bg_scrollx;
	int       m_bg_scrolly;
	int       m_tx_scrollx;
	int       m_tx_scrolly;
	int       m_bg_raster;
	int       m_bg_palettebank;
	int       m_tx_palettebank;
	std::unique_ptr<uint16_t[]>     m_spriteram1_old;
	uint32_t  inufuku_tile_callback( uint32_t code );

	/* misc */
	uint16_t    m_pending_command;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;

	void inufuku_soundcommand_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void inufuku_soundrombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void inufuku_palettereg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void inufuku_scrollreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t inufuku_bg_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void inufuku_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t inufuku_tx_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void inufuku_tx_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value soundflag_r(ioport_field &field, void *param);
	void get_inufuku_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_inufuku_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_inufuku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_inufuku(screen_device &screen, bool state);
};
