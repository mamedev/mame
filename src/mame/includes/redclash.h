// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Zero Hour / Red Clash

*************************************************************************/

#include "includes/ladybug.h"

class redclash_state : public ladybug_state
{
public:
	redclash_state(const machine_config &mconfig, device_type type, const char *tag)
		: ladybug_state(mconfig, type, tag) { }

	tilemap_t    *m_fg_tilemap; // redclash
	int        m_gfxbank;   // redclash only

	/* misc */
	UINT8      m_sraider_0x30;
	UINT8      m_sraider_0x38;

	DECLARE_READ8_MEMBER(sraider_sound_low_r);
	DECLARE_READ8_MEMBER(sraider_sound_high_r);
	DECLARE_WRITE8_MEMBER(sraider_sound_low_w);
	DECLARE_WRITE8_MEMBER(sraider_sound_high_w);
	DECLARE_READ8_MEMBER(sraider_8005_r);
	DECLARE_WRITE8_MEMBER(sraider_misc_w);
	DECLARE_WRITE8_MEMBER(sraider_io_w);
	DECLARE_INPUT_CHANGED_MEMBER(left_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(right_coin_inserted);	
	DECLARE_DRIVER_INIT(redclash);
	DECLARE_MACHINE_START(sraider);
	DECLARE_MACHINE_RESET(sraider);
	DECLARE_VIDEO_START(sraider);
	DECLARE_PALETTE_INIT(sraider);
	DECLARE_MACHINE_START(redclash);
	DECLARE_MACHINE_RESET(redclash);
	DECLARE_VIDEO_START(redclash);
	DECLARE_PALETTE_INIT(redclash);
	UINT32 screen_update_sraider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_redclash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sraider(screen_device &screen, bool state);
	void screen_eof_redclash(screen_device &screen, bool state);
	DECLARE_WRITE8_MEMBER( redclash_videoram_w );
	DECLARE_WRITE8_MEMBER( redclash_gfxbank_w );
	DECLARE_WRITE8_MEMBER( redclash_flipscreen_w );
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	DECLARE_WRITE8_MEMBER( redclash_star0_w );
	DECLARE_WRITE8_MEMBER( redclash_star1_w );
	DECLARE_WRITE8_MEMBER( redclash_star2_w );
	DECLARE_WRITE8_MEMBER( redclash_star_reset_w );
	DECLARE_WRITE8_MEMBER( irqack_w );

	/* sraider uses the zerohour star generator board */
	void redclash_set_stars_enable(UINT8 on);
	void redclash_update_stars_state();
	void redclash_set_stars_speed(UINT8 speed);
	void redclash_draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx);
	void redclash_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void redclash_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
