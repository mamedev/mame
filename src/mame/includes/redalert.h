// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    To Do:
    - Device-ify video and audio hardware to turn optional_devices into
      required_devices.

****************************************************************************/

#ifndef MAME_INCLUDES_REDALERT_H
#define MAME_INCLUDES_REDALERT_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "screen.h"

class redalert_state : public driver_device
{
public:
	redalert_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bitmap_videoram(*this, "bitmap_videoram"),
		m_charmap_videoram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_bitmap_color(*this, "bitmap_color"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_voicecpu(*this, "voice"),
		m_ay8910(*this, "aysnd"),
		m_ay(*this, "ay%u", 1U),
		m_cvsd(*this, "cvsd"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2")
	{
	}

	void redalert_video_common(machine_config &config);
	void redalert_video(machine_config &config);
	void ww3_video(machine_config &config);
	void panther_video(machine_config &config);
	void demoneye_video(machine_config &config);
	void redalert_audio_m37b(machine_config &config);
	void redalert_audio_voice(machine_config &config);
	void redalert_audio(machine_config &config);
	void ww3_audio(machine_config &config);
	void demoneye_audio(machine_config &config);
	void demoneye(machine_config &config);
	void ww3(machine_config &config);
	void panther(machine_config &config);
	void redalert(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_bitmap_videoram;
	required_shared_ptr<uint8_t> m_charmap_videoram;
	required_shared_ptr<uint8_t> m_video_control;
	required_shared_ptr<uint8_t> m_bitmap_color;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<i8085a_cpu_device> m_voicecpu;
	optional_device<ay8910_device> m_ay8910;
	optional_device_array<ay8910_device, 2> m_ay;
	optional_device<hc55516_device> m_cvsd;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;

	std::unique_ptr<uint8_t[]> m_bitmap_colorram;
	uint8_t m_control_xor;
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
	uint32_t screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
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

	virtual void sound_start() override;

	void redalert_main_map(address_map &map);
	void ww3_main_map(address_map &map);
	void panther_main_map(address_map &map);
	void demoneye_main_map(address_map &map);

	void redalert_audio_map(address_map &map);
	void demoneye_audio_map(address_map &map);

	void redalert_voice_map(address_map &map);

	uint8_t m_ay8910_latch_1;
	uint8_t m_ay8910_latch_2;
};

#endif // MAME_INCLUDES_REDALERT_H
