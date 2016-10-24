// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Paul Leaman
// thanks-to: Steven Frew (the author of Slutte)
/***************************************************************************

    Bionic Commando

***************************************************************************/

#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/tigeroad_spr.h"

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint16_t    m_scroll[4];

	uint16_t    m_inp[3];
	uint16_t    m_soundcommand;

	void hacked_controls_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hacked_controls_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bionicc_mpu_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hacked_soundcommand_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hacked_soundcommand_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bionicc_bgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bionicc_fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bionicc_txvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bionicc_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bionicc_gfxctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	static rgb_t RRRRGGGGBBBBIIII_decoder(uint32_t raw);
	uint32_t screen_update_bionicc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bionicc_scanline(timer_device &timer, void *ptr, int32_t param);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;
};
