// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "sound/hc55516.h"

class redalert_state : public driver_device
{
public:
	redalert_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bitmap_videoram(*this, "bitmap_videoram"),
		m_charmap_videoram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_bitmap_color(*this, "bitmap_color"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cvsd(*this, "cvsd"),
		m_screen(*this, "screen") { }

	UINT8 m_ay8910_latch_1;
	UINT8 m_ay8910_latch_2;

	required_shared_ptr<UINT8> m_bitmap_videoram;
	required_shared_ptr<UINT8> m_charmap_videoram;
	required_shared_ptr<UINT8> m_video_control;
	required_shared_ptr<UINT8> m_bitmap_color;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<hc55516_device> m_cvsd;
	required_device<screen_device> m_screen;

	std::unique_ptr<UINT8[]> m_bitmap_colorram;
	UINT8 m_control_xor;
	DECLARE_READ8_MEMBER(redalert_interrupt_clear_r);
	DECLARE_WRITE8_MEMBER(redalert_interrupt_clear_w);
	DECLARE_READ8_MEMBER(panther_interrupt_clear_r);
	DECLARE_READ8_MEMBER(panther_unk_r);
	DECLARE_WRITE8_MEMBER(redalert_bitmap_videoram_w);
	DECLARE_WRITE8_MEMBER(redalert_audio_command_w);
	DECLARE_READ8_MEMBER(redalert_ay8910_latch_1_r);
	DECLARE_WRITE8_MEMBER(redalert_ay8910_latch_2_w);
	DECLARE_WRITE8_MEMBER(redalert_voice_command_w);
	DECLARE_WRITE8_MEMBER(demoneye_audio_command_w);
	DECLARE_VIDEO_START(redalert);
	DECLARE_VIDEO_START(ww3);
	DECLARE_SOUND_START(redalert);
	DECLARE_SOUND_START(demoneye);
	UINT32 screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(redalert_vblank_interrupt);
	DECLARE_WRITE8_MEMBER(redalert_analog_w);
	DECLARE_WRITE8_MEMBER(redalert_AY8910_w);
	DECLARE_WRITE_LINE_MEMBER(sod_callback);
	DECLARE_READ_LINE_MEMBER(sid_callback);
	DECLARE_WRITE8_MEMBER(demoneye_ay8910_latch_1_w);
	DECLARE_READ8_MEMBER(demoneye_ay8910_latch_2_r);
	DECLARE_WRITE8_MEMBER(demoneye_ay8910_data_w);
	void get_pens(pen_t *pens);
	void get_panther_pens(pen_t *pens);
};
/*----------- defined in audio/redalert.c -----------*/

MACHINE_CONFIG_EXTERN( redalert_audio );
MACHINE_CONFIG_EXTERN( ww3_audio );
MACHINE_CONFIG_EXTERN( demoneye_audio );

/*----------- defined in video/redalert.c -----------*/

MACHINE_CONFIG_EXTERN( ww3_video );
MACHINE_CONFIG_EXTERN( panther_video );
MACHINE_CONFIG_EXTERN( redalert_video );
MACHINE_CONFIG_EXTERN( demoneye_video );
