// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
#include "sound/samples.h"

#define SAMPLE_LENGTH 32

class polyplay_state : public driver_device
{
public:
	polyplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_characterram(*this, "characterram"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_characterram;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_freq1;
	int m_freq2;
	int m_channel_playing1;
	int m_channel_playing2;
	INT16 m_backgroundwave[SAMPLE_LENGTH];
	int m_prescale1;
	int m_prescale2;
	int m_channel1_active;
	int m_channel1_const;
	int m_channel2_active;
	int m_channel2_const;
	timer_device* m_timer;
	int m_last;

	DECLARE_WRITE8_MEMBER(polyplay_sound_channel);
	DECLARE_WRITE8_MEMBER(polyplay_start_timer2);
	DECLARE_READ8_MEMBER(polyplay_random_read);
	DECLARE_WRITE8_MEMBER(polyplay_characterram_w);
	SAMPLES_START_CB_MEMBER(sh_start);
	void set_channel1(int active);
	void set_channel2(int active);
	void play_channel1(int data);
	void play_channel2(int data);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(polyplay);
	UINT32 screen_update_polyplay(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(periodic_interrupt);
	INTERRUPT_GEN_MEMBER(coin_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(polyplay_timer_callback);
};
