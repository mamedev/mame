// license:BSD-3-Clause
// copyright-holders:R. Belmont, Fabio Priuli

/***************************************************************************

    mmc5snd.h
    Nintendo MMC5 add-on sound

***************************************************************************/

#ifndef MAME_SOUND_MMC5SND_H
#define MAME_SOUND_MMC5SND_H

#pragma once


class mmc5snd_device : public device_t, public device_sound_interface
{
public:
	mmc5snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	
	void pcm_read(u8 data);

	bool irq_pending() const;
	void clear_irq();
	void schedule_irq_delay();
	bool clock_irq_delay();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	struct pulse_t
	{
		u8 control = 0;
		u8 timer_low = 0;
		u8 timer_high = 0;
		u16 timer = 0;
		u8 duty_step = 0;
		u8 length_counter = 0;
		u8 envelope_divider = 0;
		u8 envelope_decay = 0;
		bool envelope_start = false;
		bool enabled = false;
	};

	static constexpr u8 LENGTH_TABLE[32] =
	{
		10, 254, 20, 2, 40, 4, 80, 6,
		160, 8, 60, 10, 14, 12, 26, 14,
		12, 16, 24, 18, 48, 20, 96, 22,
		192, 24, 72, 26, 16, 28, 32, 30
	};

	static constexpr u8 DUTY_TABLE[4] =
	{
		0x40, // 12.5%: 01000000
		0x60, // 25%:   01100000
		0x78, // 50%:   01111000
		0x9f  // 25% negated
	};

	u16 period(int chan) const;
	u8 pulse_output(int chan) const;

	void clock_pulse_timers();
	void clock_envelopes();
	void clock_length_counters();
	void clock_frame_sequencer();

	pulse_t m_pulse[2];

	u8 m_pcm_mode;
	u8 m_pcm_dac;
	bool m_pcm_irq_pending;

	u32 m_frame_accum;
	bool m_timer_divider;
	int m_pcm_irq_delay;

	sound_stream *m_stream;
};


DECLARE_DEVICE_TYPE(MMC5SND, mmc5snd_device)

#endif // MAME_SOUND_MMC5SND_H
