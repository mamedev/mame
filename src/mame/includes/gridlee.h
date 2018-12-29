// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

***************************************************************************/
#ifndef MAME_INCLUDES_GRIDLEE_H
#define MAME_INCLUDES_GRIDLEE_H

#pragma once

#include "sound/samples.h"
#include "machine/74259.h"
#include "emupal.h"
#include "screen.h"


#define GRIDLEE_MASTER_CLOCK    (20000000)
#define GRIDLEE_CPU_CLOCK       (GRIDLEE_MASTER_CLOCK / 16)
#define GRIDLEE_PIXEL_CLOCK     (GRIDLEE_MASTER_CLOCK / 4)
#define GRIDLEE_HTOTAL          (0x140)
#define GRIDLEE_HBEND           (0x000)
#define GRIDLEE_HBSTART         (0x100)
#define GRIDLEE_VTOTAL          (0x108)
#define GRIDLEE_VBEND           (0x010)
#define GRIDLEE_VBSTART         (0x100)


class gridlee_state : public driver_device
{
public:
	gridlee_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_latch(*this, "latch"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_samples(*this, "samples")
	{ }

	void gridlee(machine_config &config);

private:
	DECLARE_READ8_MEMBER(analog_port_r);
	DECLARE_READ8_MEMBER(random_num_r);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_w);
	DECLARE_WRITE_LINE_MEMBER(cocktail_flip_w);
	DECLARE_WRITE8_MEMBER(gridlee_videoram_w);
	DECLARE_WRITE8_MEMBER(gridlee_palette_select_w);
	void gridlee_palette(palette_device &palette) const;
	uint32_t screen_update_gridlee(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(irq_off_tick);
	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	TIMER_CALLBACK_MEMBER(firq_off_tick);
	TIMER_CALLBACK_MEMBER(firq_timer_tick);
	void expand_pixels();
	void poly17_init();
	void cpu1_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_latch;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<samples_device> m_samples;

	uint8_t m_last_analog_input[2];
	uint8_t m_last_analog_output[2];
	std::unique_ptr<uint8_t[]> m_poly17;
	uint8_t *m_rand17;
	emu_timer *m_irq_off;
	emu_timer *m_irq_timer;
	emu_timer *m_firq_off;
	emu_timer *m_firq_timer;
	uint8_t m_cocktail_flip;
	std::unique_ptr<uint8_t[]> m_local_videoram;
	uint8_t m_palettebank_vis;
};


/*----------- defined in audio/gridlee.c -----------*/

class gridlee_sound_device : public device_t, public device_sound_interface
{
public:
	gridlee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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
	uint32_t m_tone_step;
	uint32_t m_tone_fraction;
	uint8_t m_tone_volume;

	/* sound streaming variables */
	sound_stream *m_stream;
	required_device<samples_device> m_samples;
	double m_freq_to_step;
	uint8_t m_sound_data[24];
};

DECLARE_DEVICE_TYPE(GRIDLEE, gridlee_sound_device)

#endif // MAME_INCLUDES_GRIDLEE_H
