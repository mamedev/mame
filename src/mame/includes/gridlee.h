// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

***************************************************************************/

#include "sound/samples.h"


#define GRIDLEE_MASTER_CLOCK    (20000000)
#define GRIDLEE_CPU_CLOCK       (GRIDLEE_MASTER_CLOCK / 16)
#define GRIDLEE_PIXEL_CLOCK (GRIDLEE_MASTER_CLOCK / 4)
#define GRIDLEE_HTOTAL          (0x140)
#define GRIDLEE_HBEND           (0x000)
#define GRIDLEE_HBSTART     (0x100)
#define GRIDLEE_VTOTAL          (0x108)
#define GRIDLEE_VBEND           (0x010)
#define GRIDLEE_VBSTART     (0x100)


class gridlee_state : public driver_device
{
public:
	gridlee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 m_last_analog_input[2];
	UINT8 m_last_analog_output[2];
	std::unique_ptr<UINT8[]> m_poly17;
	UINT8 *m_rand17;
	emu_timer *m_irq_off;
	emu_timer *m_irq_timer;
	emu_timer *m_firq_off;
	emu_timer *m_firq_timer;
	UINT8 m_cocktail_flip;
	UINT8 *m_local_videoram;
	UINT8 m_palettebank_vis;

	DECLARE_READ8_MEMBER(analog_port_r);
	DECLARE_READ8_MEMBER(random_num_r);
	DECLARE_WRITE8_MEMBER(led_0_w);
	DECLARE_WRITE8_MEMBER(led_1_w);
	DECLARE_WRITE8_MEMBER(gridlee_coin_counter_w);
	DECLARE_WRITE8_MEMBER(gridlee_cocktail_flip_w);
	DECLARE_WRITE8_MEMBER(gridlee_videoram_w);
	DECLARE_WRITE8_MEMBER(gridlee_palette_select_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(gridlee);
	UINT32 screen_update_gridlee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(irq_off_tick);
	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	TIMER_CALLBACK_MEMBER(firq_off_tick);
	TIMER_CALLBACK_MEMBER(firq_timer_tick);
	void expand_pixels();
	void poly17_init();
};


/*----------- defined in audio/gridlee.c -----------*/

class gridlee_sound_device : public device_t,
								public device_sound_interface
{
public:
	gridlee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~gridlee_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( gridlee_sound_w );

private:
	/* tone variables */
	UINT32 m_tone_step;
	UINT32 m_tone_fraction;
	UINT8 m_tone_volume;

	/* sound streaming variables */
	sound_stream *m_stream;
	samples_device *m_samples;
	double m_freq_to_step;
	UINT8 m_sound_data[24];
};

extern const device_type GRIDLEE;
