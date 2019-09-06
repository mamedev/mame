// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/deckarn.h"
#include "video/decrmc3.h"
#include "tilemap.h"

class karnov_state : public driver_device
{
public:
	karnov_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_screen(*this, "screen"),
		m_spriteram(*this, "spriteram") ,
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_pf_data(*this, "pf_data"),
		m_scroll(*this, "scroll") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<mcs51_cpu_device> m_mcu;
	required_device<screen_device> m_screen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<deco_karnovsprites_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_pf_data;
	required_shared_ptr<uint16_t> m_scroll;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fix_tilemap;

	/* misc */
	uint16_t      m_i8751_return;
	uint16_t      m_i8751_needs_ack;
	uint16_t      m_i8751_coin_pending;
	uint16_t      m_i8751_command_queue;
	int         m_i8751_level;  // needed by chelnov
	int         m_microcontroller_id;
	int         m_coin_mask;
	int         m_latch;

	u16 mcu_r();
	void mcu_w(u16 data);
	DECLARE_WRITE16_MEMBER(mcu_ack_w);
	DECLARE_WRITE16_MEMBER(mcu_reset_w);
	DECLARE_WRITE16_MEMBER(vint_ack_w);
	DECLARE_WRITE16_MEMBER(videoram_w);
	void playfield_w(offs_t offset, u16 data, u16 mem_mask);
	void init_wndrplnt();
	void init_karnov();
	void init_karnovj();
	void init_chelnovu();
	void init_chelnovj();
	void init_chelnov();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(karnov);
	DECLARE_VIDEO_START(wndrplnt);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vbint_w);
	void chelnovjbl_vbint_w(int state);
	void karnov_i8751_w( int data );
	void wndrplnt_i8751_w( int data );
	void chelnov_i8751_w( int data );
	void chelnovjbl(machine_config &config);
	void karnov(machine_config &config);
	void wndrplnt(machine_config &config);
	void karnovjbl(machine_config &config);
	void base_sound_map(address_map &map);
	void chelnovjbl_mcu_map(address_map &map);
	void chelnovjbl_mcu_io_map(address_map &map);
	void karnov_map(address_map &map);
	void chelnovjbl_map(address_map &map);
	void karnov_sound_map(address_map &map);
	void karnovjbl_sound_map(address_map &map);

private:
	// protection mcu (bootleg)
	void chelnovjbl_mcu_ack_w(uint16_t data);
	uint16_t chelnovjbl_mcu_r();
	void chelnovjbl_mcu_w(uint16_t data);

	uint8_t mcu_data_l_r();
	void mcu_data_l_w(uint8_t data);
	uint8_t mcu_data_h_r();
	void mcu_data_h_w(uint8_t data);

	void mcu_p1_w(uint8_t data);

	uint8_t m_mcu_p1;

	uint16_t m_mcu_to_maincpu;
	uint16_t m_maincpu_to_mcu;
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVU,
	CHELNOVJ,
	WNDRPLNT
};
