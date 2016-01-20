// license:BSD-3-Clause
// copyright-holders:James Wallace
// thanks-to:Martin Buchholz,Juergen Oppermann,Volker Hann, Jan-Ole Christian
#include "sound/samples.h"

#define SAMPLE_LENGTH 32

#define POLYPLAY_MAIN_CLOCK XTAL_9_8304MHz

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
		m_in0_port(*this, "IN0"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_characterram;

	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_in0_port;
	required_device<palette_device> m_palette;

	int m_freq[2];
	INT16 m_backgroundwave[SAMPLE_LENGTH];
	int m_prescale[2];
	int m_channel_const[2];
	int m_light_state;
	timer_device* m_timer;
	timer_device* m_timer2;
	timer_device* m_light_timer;

	DECLARE_WRITE8_MEMBER(polyplay_sound_w);
	DECLARE_WRITE8_MEMBER(polyplay_timer_40hz);
	DECLARE_WRITE8_MEMBER(polyplay_timer_75hz);
	DECLARE_READ8_MEMBER(polyplay_in1_r);
	DECLARE_READ8_MEMBER(polyplay_porta_r);
	DECLARE_WRITE8_MEMBER(polyplay_characterram_w);
	DECLARE_WRITE8_MEMBER(polyplay_portb_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	SAMPLES_START_CB_MEMBER(sh_start);
	void process_channel(int channel, int data);	
	void play_channel(int channel, int data);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(polyplay);
	UINT32 screen_update_polyplay(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(polyplay_timer_callback_40);
	TIMER_DEVICE_CALLBACK_MEMBER(polyplay_timer_callback_75);
	TIMER_DEVICE_CALLBACK_MEMBER(polyplay_timer_callback_lights);
};
