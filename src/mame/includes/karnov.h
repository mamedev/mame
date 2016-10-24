// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/deckarn.h"

class karnov_state : public driver_device
{
public:
	karnov_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spriteram(*this, "spriteram") ,
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_pf_data(*this, "pf_data") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<deco_karnovsprites_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_pf_data;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_bitmap_f;
	tilemap_t     *m_fix_tilemap;
	int         m_flipscreen;
	uint16_t      m_scroll[2];

	/* misc */
	uint16_t      m_i8751_return;
	uint16_t      m_i8751_needs_ack;
	uint16_t      m_i8751_coin_pending;
	uint16_t      m_i8751_command_queue;
	int         m_i8751_level;  // needed by chelnov
	int         m_microcontroller_id;
	int         m_coin_mask;
	int         m_latch;

	void karnov_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t karnov_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void karnov_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void karnov_playfield_swap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_wndrplnt();
	void init_karnov();
	void init_karnovj();
	void init_chelnovu();
	void init_chelnovj();
	void init_chelnov();
	void get_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_karnov(palette_device &palette);
	void video_start_karnov();
	void video_start_wndrplnt();
	uint32_t screen_update_karnov(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void karnov_interrupt(device_t &device);
	void karnov_flipscreen_w( int data );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void karnov_i8751_w( int data );
	void wndrplnt_i8751_w( int data );
	void chelnov_i8751_w( int data );
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVU,
	CHELNOVJ,
	WNDRPLNT
};
