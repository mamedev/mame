// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/


#define MYSTSTON_MASTER_CLOCK   (XTAL_12MHz)


class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_ay8910_data(*this, "ay8910_data"),
		m_ay8910_select(*this, "ay8910_select"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"),
		m_video_control(*this, "video_control") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* machine state */
	required_shared_ptr<UINT8> m_ay8910_data;
	required_shared_ptr<UINT8> m_ay8910_select;

	/* video state */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	emu_timer *m_interrupt_timer;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_video_control;
	DECLARE_WRITE8_MEMBER(irq_clear_w);
	DECLARE_WRITE8_MEMBER(mystston_ay8910_select_w);
	DECLARE_WRITE8_MEMBER(mystston_video_control_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_VIDEO_START(mystston);
	DECLARE_VIDEO_RESET(mystston);
	UINT32 screen_update_mystston(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void set_palette();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip);
	void mystston_on_scanline_interrupt();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in video/mystston.c -----------*/

MACHINE_CONFIG_EXTERN( mystston_video );
