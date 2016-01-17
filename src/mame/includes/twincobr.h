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
	twincobr_state(const machine_config &mconfig, device_type type, std::string tag)
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

	optional_shared_ptr<UINT8> m_sharedram;
	optional_device<buffered_spriteram8_device> m_spriteram8;
	optional_device<buffered_spriteram16_device> m_spriteram16;

	int m_toaplan_main_cpu;
	INT32 m_fg_rom_bank;
	INT32 m_bg_ram_bank;
	int m_intenable;
	int m_dsp_on;
	int m_dsp_BIO;
	int m_fsharkbt_8741;
	int m_dsp_execute;
	UINT32 m_dsp_addr_w;
	UINT32 m_main_ram_seg;
	std::unique_ptr<UINT16[]> m_bgvideoram16;
	std::unique_ptr<UINT16[]> m_fgvideoram16;
	std::unique_ptr<UINT16[]> m_txvideoram16;
	size_t m_bgvideoram_size;
	size_t m_fgvideoram_size;
	size_t m_txvideoram_size;
	INT32 m_txscrollx;
	INT32 m_txscrolly;
	INT32 m_fgscrollx;
	INT32 m_fgscrolly;
	INT32 m_bgscrollx;
	INT32 m_bgscrolly;
	INT32 m_txoffs;
	INT32 m_fgoffs;
	INT32 m_bgoffs;
	INT32 m_display_on;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	DECLARE_WRITE16_MEMBER(twincobr_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(twincobr_dsp_r);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_w);
	DECLARE_WRITE16_MEMBER(wardner_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(wardner_dsp_r);
	DECLARE_WRITE16_MEMBER(wardner_dsp_w);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_bio_w);
	DECLARE_READ16_MEMBER(fsharkbt_dsp_r);
	DECLARE_WRITE16_MEMBER(fsharkbt_dsp_w);
	DECLARE_READ16_MEMBER(twincobr_BIO_r);
	DECLARE_WRITE16_MEMBER(twincobr_control_w);
	DECLARE_WRITE8_MEMBER(wardner_control_w);
	DECLARE_READ16_MEMBER(twincobr_sharedram_r);
	DECLARE_WRITE16_MEMBER(twincobr_sharedram_w);
	DECLARE_WRITE16_MEMBER(fshark_coin_dsp_w);
	DECLARE_WRITE8_MEMBER(twincobr_coin_w);
	DECLARE_WRITE8_MEMBER(wardner_coin_dsp_w);
	DECLARE_WRITE16_MEMBER(twincobr_txoffs_w);
	DECLARE_READ16_MEMBER(twincobr_txram_r);
	DECLARE_WRITE16_MEMBER(twincobr_txram_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_bgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_bgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_fgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_fgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_txscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_exscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_txlayer_w);
	DECLARE_WRITE8_MEMBER(wardner_bglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_fglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_txscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_bgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_fgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_exscroll_w);
	DECLARE_READ8_MEMBER(wardner_videoram_r);
	DECLARE_WRITE8_MEMBER(wardner_videoram_w);
	DECLARE_READ8_MEMBER(wardner_sprite_r);
	DECLARE_WRITE8_MEMBER(wardner_sprite_w);
	DECLARE_DRIVER_INIT(twincobr);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_MACHINE_RESET(twincobr);
	DECLARE_VIDEO_START(toaplan0);
	UINT32 screen_update_toaplan0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(twincobr_interrupt);
	INTERRUPT_GEN_MEMBER(wardner_interrupt);
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
