// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "video/decbac06.h"
#include "video/deckarn.h"
#include "video/decmxc06.h"

class dec8_state : public driver_device
{
public:
	enum
	{
		TIMER_DEC8_I8751
	};

	dec8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_spriteram(*this, "spriteram") ,
		m_msm(*this, "msm"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_spritegen_krn(*this, "spritegen_krn"),
		m_spritegen_mxc(*this, "spritegen_mxc"),
		m_videoram(*this, "videoram"),
		m_bg_data(*this, "bg_data"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<buffered_spriteram8_device> m_spriteram;
	optional_device<msm5205_device> m_msm;
	optional_device<deco_bac06_device> m_tilegen1;
	optional_device<deco_bac06_device> m_tilegen2;
	optional_device<deco_karnovsprites_device> m_spritegen_krn;
	optional_device<deco_mxc06_device> m_spritegen_mxc;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_bg_data;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 *  m_pf1_data;
	UINT8 *  m_row;
	UINT16   m_buffered_spriteram16[0x800/2]; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_pf1_tilemap;
	tilemap_t  *m_fix_tilemap;
	//int      m_scroll1[4];
	int      m_scroll2[4];
	int      m_bg_control[0x20];
	int      m_pf1_control[0x20];
	int      m_game_uses_priority;

	/* misc */
	int      m_i8751_port0;
	int      m_i8751_port1;
	int      m_nmi_enable;
	int      m_i8751_return;
	int      m_i8751_value;
	int      m_coinage_id;
	int      m_coin1;
	int      m_coin2;
	int      m_need1;
	int      m_need2;
	int      m_cred1;
	int      m_cred2;
	int      m_credits;
	int      m_latch;
	int      m_snd;
	int      m_msm5205next;
	int      m_toggle;

	DECLARE_WRITE8_MEMBER(dec8_mxc06_karn_buffer_spriteram_w);
	DECLARE_READ8_MEMBER(i8751_h_r);
	DECLARE_READ8_MEMBER(i8751_l_r);
	DECLARE_WRITE8_MEMBER(i8751_reset_w);
	DECLARE_READ8_MEMBER(gondo_player_1_r);
	DECLARE_READ8_MEMBER(gondo_player_2_r);
	DECLARE_WRITE8_MEMBER(dec8_i8751_w);
	DECLARE_WRITE8_MEMBER(lastmisn_i8751_w);
	DECLARE_WRITE8_MEMBER(shackled_i8751_w);
	DECLARE_WRITE8_MEMBER(csilver_i8751_w);
	DECLARE_WRITE8_MEMBER(srdarwin_i8751_w);
	DECLARE_WRITE8_MEMBER(dec8_bank_w);
	DECLARE_WRITE8_MEMBER(ghostb_bank_w);
	DECLARE_WRITE8_MEMBER(csilver_control_w);
	DECLARE_WRITE8_MEMBER(dec8_sound_w);
	DECLARE_WRITE8_MEMBER(csilver_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(csilver_sound_bank_w);
	DECLARE_WRITE8_MEMBER(oscar_int_w);
	DECLARE_WRITE8_MEMBER(shackled_int_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_READ8_MEMBER(dec8_mcu_from_main_r);
	DECLARE_WRITE8_MEMBER(dec8_mcu_to_main_w);
	DECLARE_WRITE8_MEMBER(dec8_bg_data_w);
	DECLARE_READ8_MEMBER(dec8_bg_data_r);
	DECLARE_WRITE8_MEMBER(dec8_videoram_w);
	DECLARE_WRITE8_MEMBER(srdarwin_videoram_w);
	DECLARE_WRITE8_MEMBER(dec8_scroll2_w);
	DECLARE_WRITE8_MEMBER(srdarwin_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_control_w);
	DECLARE_WRITE8_MEMBER(shackled_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrollx_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrolly_w);
	DECLARE_WRITE8_MEMBER(gondo_scroll_w);
	DECLARE_READ8_MEMBER(csilver_adpcm_reset_r);
	DECLARE_DRIVER_INIT(dec8);
	DECLARE_DRIVER_INIT(deco222);
	DECLARE_DRIVER_INIT(meikyuh);
	DECLARE_DRIVER_INIT(garyoret);
	DECLARE_DRIVER_INIT(shackled);
	DECLARE_DRIVER_INIT(cobracom);
	DECLARE_DRIVER_INIT(csilver);
	DECLARE_DRIVER_INIT(ghostb);
	DECLARE_DRIVER_INIT(srdarwin);
	DECLARE_DRIVER_INIT(lastmisn);
	DECLARE_DRIVER_INIT(gondo);
	DECLARE_DRIVER_INIT(oscar);
	TILE_GET_INFO_MEMBER(get_cobracom_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_ghostb_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_oscar_fix_tile_info);
	TILEMAP_MAPPER_MEMBER(lastmisn_scan_rows);
	TILE_GET_INFO_MEMBER(get_lastmisn_tile_info);
	TILE_GET_INFO_MEMBER(get_lastmisn_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_srdarwin_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_srdarwin_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_gondo_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_VIDEO_START(lastmisn);
	DECLARE_VIDEO_START(shackled);
	DECLARE_VIDEO_START(gondo);
	DECLARE_VIDEO_START(garyoret);
	DECLARE_VIDEO_START(ghostb);
	DECLARE_PALETTE_INIT(ghostb);
	DECLARE_VIDEO_START(oscar);
	DECLARE_VIDEO_START(srdarwin);
	DECLARE_VIDEO_START(cobracom);
	UINT32 screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gondo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ghostb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_oscar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_srdarwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cobracom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_dec8(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(gondo_interrupt);
	INTERRUPT_GEN_MEMBER(oscar_interrupt);
	void srdarwin_draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	DECLARE_WRITE_LINE_MEMBER(csilver_adpcm_int);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
