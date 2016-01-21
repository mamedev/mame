// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/msm5232.h"

class flstory_state : public driver_device
{
public:
	flstory_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrlram(*this, "scrlram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scrlram;
	optional_shared_ptr<UINT8> m_workram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	std::vector<UINT8> m_paletteram;
	std::vector<UINT8> m_paletteram_ext;
	UINT8    m_gfxctrl;
	UINT8    m_char_bank;
	UINT8    m_palette_bank;

	/* sound-related */
	UINT8    m_snd_data;
	UINT8    m_snd_flag;
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	int      m_vol_ctrl[16];
	UINT8    m_snd_ctrl0;
	UINT8    m_snd_ctrl1;
	UINT8    m_snd_ctrl2;
	UINT8    m_snd_ctrl3;

	/* protection */
	UINT8    m_from_main;
	UINT8    m_from_mcu;
	int      m_mcu_sent;
	int      m_main_sent;
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	UINT8    m_port_c_in;
	UINT8    m_port_c_out;
	UINT8    m_ddr_c;
	int      m_mcu_select;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* mcu */
	UINT8 m_mcu_cmd;
	UINT8 m_mcu_counter;
	UINT8 m_mcu_b4_cmd;
	UINT8 m_mcu_param;
	UINT8 m_mcu_b2_res;
	UINT8 m_mcu_b1_res;
	UINT8 m_mcu_bb_res;
	UINT8 m_mcu_b5_res;
	UINT8 m_mcu_b6_res;
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(rumba_mcu_r);
	DECLARE_WRITE8_MEMBER(rumba_mcu_w);
	DECLARE_READ8_MEMBER(flstory_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(flstory_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(flstory_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(flstory_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(flstory_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(flstory_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(flstory_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(flstory_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(flstory_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(flstory_mcu_w);
	DECLARE_READ8_MEMBER(flstory_mcu_r);
	DECLARE_READ8_MEMBER(flstory_mcu_status_r);
	DECLARE_WRITE8_MEMBER(onna34ro_mcu_w);
	DECLARE_READ8_MEMBER(onna34ro_mcu_r);
	DECLARE_READ8_MEMBER(onna34ro_mcu_status_r);
	DECLARE_WRITE8_MEMBER(victnine_mcu_w);
	DECLARE_READ8_MEMBER(victnine_mcu_r);
	DECLARE_READ8_MEMBER(victnine_mcu_status_r);
	DECLARE_WRITE8_MEMBER(flstory_videoram_w);
	DECLARE_WRITE8_MEMBER(flstory_palette_w);
	DECLARE_READ8_MEMBER(flstory_palette_r);
	DECLARE_WRITE8_MEMBER(flstory_gfxctrl_w);
	DECLARE_READ8_MEMBER(victnine_gfxctrl_r);
	DECLARE_WRITE8_MEMBER(victnine_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(flstory_scrlram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(victnine_mcu_status_bit01_r);
	DECLARE_WRITE8_MEMBER(sound_control_0_w);
	DECLARE_WRITE8_MEMBER(sound_control_1_w);
	DECLARE_WRITE8_MEMBER(sound_control_2_w);
	DECLARE_WRITE8_MEMBER(sound_control_3_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(victnine_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rumba_tile_info);
	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(flstory);
	DECLARE_VIDEO_START(flstory);
	DECLARE_VIDEO_START(victnine);
	DECLARE_MACHINE_RESET(rumba);
	DECLARE_VIDEO_START(rumba);
	DECLARE_MACHINE_RESET(ta7630);
	UINT32 screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	void flstory_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void victnine_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
