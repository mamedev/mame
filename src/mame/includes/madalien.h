// license:GPL-2.0+
// copyright-holders:Norbert Kehrer
/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "machine/gen_latch.h"
#include "sound/discrete.h"


#define MADALIEN_MAIN_CLOCK     XTAL_10_595MHz


class madalien_state : public driver_device
{
public:
	madalien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_shift_hi(*this, "shift_hi"),
		m_shift_lo(*this, "shift_lo"),
		m_video_flags(*this, "video_flags"),
		m_headlight_pos(*this, "headlight_pos"),
		m_edge1_pos(*this, "edge1_pos"),
		m_edge2_pos(*this, "edge2_pos"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_video_control;
	required_shared_ptr<uint8_t> m_shift_hi;
	required_shared_ptr<uint8_t> m_shift_lo;
	required_shared_ptr<uint8_t> m_video_flags;
	required_shared_ptr<uint8_t> m_headlight_pos;
	required_shared_ptr<uint8_t> m_edge1_pos;
	required_shared_ptr<uint8_t> m_edge2_pos;
	required_shared_ptr<uint8_t> m_scroll;

	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_edge1[4];
	tilemap_t *m_tilemap_edge2[4];
	std::unique_ptr<bitmap_ind16> m_headlight_bitmap;
	uint8_t shift_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t shift_rev_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void madalien_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void madalien_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t madalien_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void madalien_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void madalien_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void madalien_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void madalien_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index scan_mode0(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index scan_mode1(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index scan_mode2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index scan_mode3(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info_BG_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_BG_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_FG(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_madalien();
	void palette_init_madalien(palette_device &palette);
	uint32_t screen_update_madalien(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scan_helper(int col, int row, int section);
	void draw_edges(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, int scroll_mode);
	void draw_headlight(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_foreground(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	inline uint8_t shift_common(uint8_t hi, uint8_t lo);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
};
/*----------- defined in video/madalien.c -----------*/

MACHINE_CONFIG_EXTERN( madalien_video );

/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA         NODE_01
#define MADALIEN_8910_PORTB         NODE_02
