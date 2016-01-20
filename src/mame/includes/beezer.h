// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
#include "machine/6522via.h"
#include "cpu/m6809/m6809.h"

class beezer_sound_device;

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_custom(*this, "custom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	int m_pbus;
	int m_banklatch;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<beezer_sound_device> m_custom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(beezer_bankswitch_w);
	DECLARE_WRITE8_MEMBER(beezer_map_w);
	DECLARE_READ8_MEMBER(beezer_line_r);
	DECLARE_DRIVER_INIT(beezer);
	virtual void machine_start() override;
	UINT32 screen_update_beezer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(beezer_interrupt);
	DECLARE_READ8_MEMBER(b_via_0_pa_r);
	DECLARE_READ8_MEMBER(b_via_0_pb_r);
	DECLARE_WRITE8_MEMBER(b_via_0_pa_w);
	DECLARE_WRITE8_MEMBER(b_via_0_pb_w);
	DECLARE_READ8_MEMBER(b_via_1_pa_r);
	DECLARE_READ8_MEMBER(b_via_1_pb_r);
	DECLARE_WRITE8_MEMBER(b_via_1_pa_w);
	DECLARE_WRITE8_MEMBER(b_via_1_pb_w);
};

/*----------- defined in audio/beezer.c -----------*/

/* 6840 variables */
struct sh6840_timer_channel_beez
{
	UINT8   cr;
	UINT8   state;
	UINT8   leftovers;
	UINT16  timer;
	UINT32  clocks;
	UINT8   int_flag;
	union
	{
#ifdef LSB_FIRST
		struct { UINT8 l, h; } b;
#else
		struct { UINT8 h, l; } b;
#endif
		UINT16 w;
	} counter;
};

class beezer_sound_device : public device_t,
									public device_sound_interface
{
public:
	beezer_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~beezer_sound_device() {}

	DECLARE_READ8_MEMBER( sh6840_r );
	DECLARE_WRITE8_MEMBER( sh6840_w );
	DECLARE_WRITE8_MEMBER( sfxctrl_w );
	DECLARE_WRITE8_MEMBER( timer1_w );
	DECLARE_READ8_MEMBER( noise_r );

	//DECLARE_WRITE_LINE_MEMBER( update_irq_state );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
	cpu_device *m_maincpu;

	/* IRQ variable */
	//UINT8 m_ptm_irq_state;

	struct sh6840_timer_channel_beez m_sh6840_timer[3];
	UINT8 m_sh6840_volume[4];
	UINT8 m_sh6840_MSB_latch;
	UINT8 m_sh6840_LSB_latch;
	UINT32 m_sh6840_LFSR;
	UINT32 m_sh6840_LFSR_clocks;
	UINT32 m_sh6840_clocks_per_sample;
	UINT32 m_sh6840_clock_count;

	UINT32 m_sh6840_latchwrite;
	UINT32 m_sh6840_latchwriteold;
	UINT32 m_sh6840_noiselatch1;
	UINT32 m_sh6840_noiselatch3;

	/* sound streaming variables */
	sound_stream *m_stream;
	//double m_freq_to_step;

	int sh6840_update_noise(int clocks);
};

extern const device_type BEEZER;
