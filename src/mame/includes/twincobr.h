// license:BSD-3-Clause
// copyright-holders:Quench
/***************************************************************************
        Twincobr/Flying Shark/Wardner  game hardware from 1986-1987
        -----------------------------------------------------------
****************************************************************************/

#include "video/mc6845.h"
#include "video/bufsprite.h"
#include "video/toaplan_scu.h"

class twincobr_state : public driver_device
{
public:
	twincobr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sharedram(*this, "sharedram"),
		m_spriteram8(*this, "spriteram8"),
		m_spriteram16(*this, "spriteram16"),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_spritegen(*this, "scu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	optional_shared_ptr<uint8_t> m_sharedram;
	optional_device<buffered_spriteram8_device> m_spriteram8;
	optional_device<buffered_spriteram16_device> m_spriteram16;

	int m_toaplan_main_cpu;
	int32_t m_fg_rom_bank;
	int32_t m_bg_ram_bank;
	int m_intenable;
	int m_dsp_on;
	int m_dsp_BIO;
	int m_fsharkbt_8741;
	int m_dsp_execute;
	uint32_t m_dsp_addr_w;
	uint32_t m_main_ram_seg;
	std::unique_ptr<uint16_t[]> m_bgvideoram16;
	std::unique_ptr<uint16_t[]> m_fgvideoram16;
	std::unique_ptr<uint16_t[]> m_txvideoram16;
	size_t m_bgvideoram_size;
	size_t m_fgvideoram_size;
	size_t m_txvideoram_size;
	int32_t m_txscrollx;
	int32_t m_txscrolly;
	int32_t m_fgscrollx;
	int32_t m_fgscrolly;
	int32_t m_bgscrollx;
	int32_t m_bgscrolly;
	int32_t m_txoffs;
	int32_t m_fgoffs;
	int32_t m_bgoffs;
	int32_t m_display_on;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	void twincobr_dsp_addrsel_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t twincobr_dsp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twincobr_dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wardner_dsp_addrsel_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t wardner_dsp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void wardner_dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_dsp_bio_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t fsharkbt_dsp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void fsharkbt_dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int twincobr_BIO_r();
	void twincobr_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wardner_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t twincobr_sharedram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twincobr_sharedram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fshark_coin_dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_coin_dsp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void twincobr_txoffs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t twincobr_txram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twincobr_txram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_bgoffs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t twincobr_bgram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twincobr_bgram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_fgoffs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t twincobr_fgram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void twincobr_fgram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_txscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_bgscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_fgscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void twincobr_exscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wardner_txlayer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_bglayer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_fglayer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_txscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_bgscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_fgscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wardner_exscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t wardner_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wardner_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t wardner_sprite_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wardner_sprite_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_twincobr();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_twincobr();
	void video_start_toaplan0();
	uint32_t screen_update_toaplan0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void twincobr_interrupt(device_t &device);
	void wardner_interrupt(device_t &device);
	void twincobr_restore_dsp();
	void twincobr_create_tilemaps();
	void twincobr_display(int enable);
	void twincobr_flipscreen(int flip);
	void twincobr_log_vram();
	void twincobr_dsp(int enable);
	void toaplan0_control_w(int offset, int data);
	void toaplan0_coin_dsp_w(address_space &space, int offset, int data);
	void twincobr_driver_savestate();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<toaplan_scu_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
