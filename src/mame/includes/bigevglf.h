// license:???
// copyright-holders:Jarek Burczynski, Tomasz Slanina
#include "sound/msm5232.h"

class bigevglf_state : public driver_device
{
public:
	bigevglf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spriteram1;
	required_shared_ptr<UINT8> m_spriteram2;

	/* video-related */
	bitmap_ind16 m_tmp_bitmap[4];
	UINT8    *m_vidram;
	UINT32   m_vidram_bank;
	UINT32   m_plane_selected;
	UINT32   m_plane_visible;

	/* sound-related */
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	UINT8    m_for_sound;
	UINT8    m_from_sound;
	UINT8    m_sound_state;

	/* MCU related */
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
	int      m_mcu_coin_bit5;

	/* misc */
	UINT32   m_beg_bank;
	UINT8    m_beg13_ls74[2];
	UINT8    m_port_select;     /* for muxed controls */

	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	DECLARE_WRITE8_MEMBER(beg_banking_w);
	DECLARE_WRITE8_MEMBER(beg_fromsound_w);
	DECLARE_READ8_MEMBER(beg_fromsound_r);
	DECLARE_READ8_MEMBER(beg_soundstate_r);
	DECLARE_READ8_MEMBER(soundstate_r);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(beg13_a_clr_w);
	DECLARE_WRITE8_MEMBER(beg13_b_clr_w);
	DECLARE_WRITE8_MEMBER(beg13_a_set_w);
	DECLARE_WRITE8_MEMBER(beg13_b_set_w);
	DECLARE_READ8_MEMBER(beg_status_r);
	DECLARE_READ8_MEMBER(beg_trackball_x_r);
	DECLARE_READ8_MEMBER(beg_trackball_y_r);
	DECLARE_WRITE8_MEMBER(beg_port08_w);
	DECLARE_READ8_MEMBER(sub_cpu_mcu_coin_port_r);
	DECLARE_READ8_MEMBER(bigevglf_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(bigevglf_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(bigevglf_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(bigevglf_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(bigevglf_mcu_w);
	DECLARE_READ8_MEMBER(bigevglf_mcu_r);
	DECLARE_READ8_MEMBER(bigevglf_mcu_status_r);
	DECLARE_WRITE8_MEMBER(bigevglf_palette_w);
	DECLARE_WRITE8_MEMBER(bigevglf_gfxcontrol_w);
	DECLARE_WRITE8_MEMBER(bigevglf_vidram_addr_w);
	DECLARE_WRITE8_MEMBER(bigevglf_vidram_w);
	DECLARE_READ8_MEMBER(bigevglf_vidram_r);
	DECLARE_DRIVER_INIT(bigevglf);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_bigevglf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(from_sound_latch_callback);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	TIMER_CALLBACK_MEMBER(deferred_ls74_w);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
