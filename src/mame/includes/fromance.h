/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/

class fromance_state : public driver_device
{
public:
	fromance_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_videoram(*this, "videoram"),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers (used by pipedrm) */
	optional_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8    *m_local_videoram[2];
	UINT8    *m_local_paletteram;
	UINT8    m_selected_videoram;
	UINT8    m_selected_paletteram;
	UINT32   m_scrollx[2];
	UINT32   m_scrolly[2];
	UINT8    m_gfxreg;
	UINT8    m_flipscreen;
	UINT8    m_flipscreen_old;
	UINT32   m_scrolly_ofs;
	UINT32   m_scrollx_ofs;

	UINT8    m_crtc_register;
	UINT8    m_crtc_data[0x10];
	emu_timer *m_crtc_timer;

	/* misc */
	UINT8    m_directionflag;
	UINT8    m_commanddata;
	UINT8    m_portselect;
	UINT8    m_adpcm_reset;
	UINT8    m_adpcm_data;
	UINT8    m_vclk_left;
	UINT8    m_pending_command;
	UINT8    m_sound_command;

	/* devices */
	cpu_device *m_subcpu;
	DECLARE_READ8_MEMBER(fromance_commanddata_r);
	DECLARE_WRITE8_MEMBER(fromance_commanddata_w);
	DECLARE_READ8_MEMBER(fromance_busycheck_main_r);
	DECLARE_READ8_MEMBER(fromance_busycheck_sub_r);
	DECLARE_WRITE8_MEMBER(fromance_busycheck_sub_w);
	DECLARE_WRITE8_MEMBER(fromance_rombank_w);
	DECLARE_WRITE8_MEMBER(fromance_adpcm_w);
	DECLARE_WRITE8_MEMBER(fromance_portselect_w);
	DECLARE_READ8_MEMBER(fromance_keymatrix_r);
	DECLARE_WRITE8_MEMBER(fromance_coinctr_w);
	DECLARE_WRITE8_MEMBER(fromance_gfxreg_w);
	DECLARE_READ8_MEMBER(fromance_paletteram_r);
	DECLARE_WRITE8_MEMBER(fromance_paletteram_w);
	DECLARE_READ8_MEMBER(fromance_videoram_r);
	DECLARE_WRITE8_MEMBER(fromance_videoram_w);
	DECLARE_WRITE8_MEMBER(fromance_scroll_w);
	DECLARE_WRITE8_MEMBER(fromance_crtc_data_w);
	DECLARE_WRITE8_MEMBER(fromance_crtc_register_w);
	DECLARE_WRITE8_MEMBER(fromance_adpcm_reset_w);
	DECLARE_DRIVER_INIT(pipedrm);
	DECLARE_DRIVER_INIT(hatris);
	TILE_GET_INFO_MEMBER(get_fromance_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fromance_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_nekkyoku_fg_tile_info);
	DECLARE_MACHINE_START(fromance);
	DECLARE_MACHINE_RESET(fromance);
	DECLARE_VIDEO_START(nekkyoku);
	DECLARE_VIDEO_START(fromance);
	DECLARE_MACHINE_START(pipedrm);
	DECLARE_MACHINE_RESET(pipedrm);
	DECLARE_VIDEO_START(pipedrm);
	DECLARE_VIDEO_START(hatris);
	UINT32 screen_update_fromance(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pipedrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
