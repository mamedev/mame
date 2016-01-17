// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria
#include "sound/msm5205.h"

class gladiatr_state : public driver_device
{
public:
	gladiatr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_textram(*this, "textram"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_nvram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_textram;
	required_shared_ptr<UINT8> m_generic_paletteram_8;

	int m_data1;
	int m_data2;
	int m_flag1;
	int m_flag2;
	int m_video_attributes;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sprite_bank;
	int m_sprite_buffer;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_tile_bank;
	int m_bg_tile_bank;

	// common
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(textram_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_WRITE8_MEMBER(spritebuffer_w);

	// gladiator specific
	DECLARE_READ8_MEMBER(gladiator_dsw1_r);
	DECLARE_READ8_MEMBER(gladiator_dsw2_r);
	DECLARE_READ8_MEMBER(gladiator_controls_r);
	DECLARE_READ8_MEMBER(gladiator_button3_r);
	DECLARE_WRITE8_MEMBER(gladiatr_spritebank_w);
	DECLARE_WRITE8_MEMBER(gladiatr_video_registers_w);
	DECLARE_WRITE8_MEMBER(gladiatr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gladiator_cpu_sound_command_w);
	DECLARE_READ8_MEMBER(gladiator_cpu_sound_command_r);
	DECLARE_WRITE8_MEMBER(gladiatr_flipscreen_w);
	DECLARE_WRITE8_MEMBER(gladiatr_irq_patch_w);
	DECLARE_WRITE8_MEMBER(gladiator_int_control_w);
	DECLARE_WRITE8_MEMBER(gladiator_adpcm_w);
	DECLARE_WRITE_LINE_MEMBER(gladiator_ym_irq);

	// ppking specific
	DECLARE_READ8_MEMBER(ppking_f1_r);
	DECLARE_READ8_MEMBER(ppking_f6a3_r);
	DECLARE_WRITE8_MEMBER(ppking_qx0_w);
	DECLARE_WRITE8_MEMBER(ppking_qx1_w);
	DECLARE_WRITE8_MEMBER(ppking_qx2_w);
	DECLARE_WRITE8_MEMBER(ppking_qx3_w);
	DECLARE_READ8_MEMBER(ppking_qx2_r);
	DECLARE_READ8_MEMBER(ppking_qx3_r);
	DECLARE_READ8_MEMBER(ppking_qx0_r);
	DECLARE_READ8_MEMBER(ppking_qx1_r);
	DECLARE_WRITE8_MEMBER(ppking_video_registers_w);

	DECLARE_DRIVER_INIT(gladiatr);
	DECLARE_DRIVER_INIT(ppking);

	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);

	DECLARE_MACHINE_RESET(ppking);
	DECLARE_VIDEO_START(ppking);
	DECLARE_MACHINE_RESET(gladiator);
	DECLARE_VIDEO_START(gladiatr);

	UINT32 screen_update_ppking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gladiatr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void swap_block(UINT8 *src1,UINT8 *src2,int len);
};
