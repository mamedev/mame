// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem M27 hardware

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
#include "machine/6821pia.h"
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
		m_sndpia(*this, "sndpia"),
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
	void panther_audio(machine_config &config);
	void demoneye_audio(machine_config &config);
	void demoneye(machine_config &config);
	void ww3(machine_config &config);
	void panther(machine_config &config);
	void redalert(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_CUSTOM_INPUT_MEMBER(sound_status_r);

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
	optional_device<pia6821_device> m_sndpia;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;

	std::unique_ptr<uint8_t[]> m_bitmap_colorram;
	uint8_t m_control_xor;
	uint8_t redalert_interrupt_clear_r();
	void redalert_interrupt_clear_w(uint8_t data);
	uint8_t panther_interrupt_clear_r();
	void redalert_bitmap_videoram_w(offs_t offset, uint8_t data);
	void redalert_audio_command_w(uint8_t data);
	uint8_t redalert_ay8910_latch_1_r();
	void redalert_ay8910_latch_2_w(uint8_t data);
	void redalert_voice_command_w(uint8_t data);
	void demoneye_audio_command_w(uint8_t data);
	DECLARE_VIDEO_START(redalert);
	DECLARE_VIDEO_START(ww3);
	DECLARE_VIDEO_START(demoneye);
	uint32_t screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(redalert_vblank_interrupt);
	TIMER_CALLBACK_MEMBER(audio_irq_on);
	TIMER_CALLBACK_MEMBER(audio_irq_off);
	void redalert_analog_w(uint8_t data);
	void redalert_AY8910_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(sod_callback);
	DECLARE_READ_LINE_MEMBER(sid_callback);
	void demoneye_ay8910_latch_1_w(uint8_t data);
	uint8_t demoneye_ay8910_latch_2_r();
	void demoneye_ay8910_data_w(uint8_t data);
	void get_redalert_pens(pen_t *pens);
	void get_panther_pens(pen_t *pens);
	void get_demoneye_pens(pen_t *pens);
	WRITE8_MEMBER(demoneye_bitmap_layer_w);
	void demoneye_bitmap_ypos_w(u8 data);

	virtual void sound_start() override;

	void redalert_main_map(address_map &map);
	void ww3_main_map(address_map &map);
	void panther_main_map(address_map &map);
	void demoneye_main_map(address_map &map);

	void redalert_audio_map(address_map &map);
	void panther_audio_map(address_map &map);
	void demoneye_audio_map(address_map &map);

	void redalert_voice_map(address_map &map);

	emu_timer *m_audio_irq_on_timer;
	emu_timer *m_audio_irq_off_timer;

	uint8_t m_ay8910_latch_1;
	uint8_t m_ay8910_latch_2;
	u8 m_demoneye_bitmap_reg[4];
	u8 m_demoneye_bitmap_yoffs;
	u8 m_sound_hs;
};

#endif // MAME_INCLUDES_REDALERT_H
