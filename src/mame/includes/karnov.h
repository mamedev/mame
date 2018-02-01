// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/deckarn.h"
#include "video/decrmc3.h"

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
	required_device<deco_rmc3_device> m_palette;
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

	DECLARE_WRITE16_MEMBER(karnov_control_w);
	DECLARE_READ16_MEMBER(karnov_control_r);
	DECLARE_WRITE16_MEMBER(karnov_videoram_w);
	DECLARE_WRITE16_MEMBER(karnov_playfield_swap_w);
	DECLARE_DRIVER_INIT(wndrplnt);
	DECLARE_DRIVER_INIT(karnov);
	DECLARE_DRIVER_INIT(karnovj);
	DECLARE_DRIVER_INIT(chelnovu);
	DECLARE_DRIVER_INIT(chelnovj);
	DECLARE_DRIVER_INIT(chelnov);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(karnov);
	DECLARE_VIDEO_START(wndrplnt);
	uint32_t screen_update_karnov(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(karnov_interrupt);
	void karnov_flipscreen_w( int data );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void karnov_i8751_w( int data );
	void wndrplnt_i8751_w( int data );
	void chelnov_i8751_w( int data );
	void chelnovjbl(machine_config &config);
	void karnov(machine_config &config);
	void wndrplnt(machine_config &config);
	void karnovjbl(machine_config &config);
	void base_sound_map(address_map &map);
	void chelnovjbl_mcu_io_map(address_map &map);
	void chelnovjbl_mcu_map(address_map &map);
	void karnov_map(address_map &map);
	void karnov_sound_map(address_map &map);
	void karnovjbl_sound_map(address_map &map);
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVU,
	CHELNOVJ,
	WNDRPLNT
};
