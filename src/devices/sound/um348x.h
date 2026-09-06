// license:BSD-3-Clause
// copyright-holders: Tomás García-Merás (ClawGrip)

/***************************************************************************

	UMC UM348x multi-instrument melody generator family

***************************************************************************/

#ifndef MAME_SOUND_UM348X_H
#define MAME_SOUND_UM348X_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class um348x_device : public device_t, public device_sound_interface
{
public:
	// Select the melody to play (0-based). Latched; takes effect at the next
	// trigger. Both parts have 16 pointer slots, but only the first 8 (UM3481A)
	// or 12 (UM3482A) are melodies the part will play; higher values are
	// ignored.
	void melody_w(u8 data);

	// Start playing the selected melody from its beginning. Asserting while a
	// melody is already playing restarts it.
	void trigger_w(int state);

	// Stop immediately and silence the output.
	void reset_w(int state);

	// True while a melody is sounding. Some boards wire the chip's busy pin
	// back to the driving CPU.
	int busy_r() { m_stream->update(); return m_playing ? 1 : 0; }

protected:
	um348x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 melodies);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// Per-part tone table: oscillator half-period (the divisor N) for each of
	// the 16 raw tone codes. 0 marks a code that produces no sound.
	virtual const u8 *tone_divisors() const = 0;

private:
	void start_word(u16 index);
	void advance_word();
	u16 melody_start(u8 melody) const;

	required_memory_region m_notes;
	required_memory_region m_offsets;
	required_memory_region m_tempos;

	sound_stream *m_stream;
	const u8 m_melodies;
	u16 m_data_end;         // last word that can sound; everything after is filler

	// latched inputs
	u8  m_melody;
	u8  m_trigger;
	u8  m_reset;

	// playback state
	bool m_playing;
	u16  m_note_index;      // current word, 0..511
	u16  m_note_start;      // first word of the current melody
	u16  m_note_end;        // one past the last word of the current melody
	u8   m_multiplier;      // tempo multiplier for the current melody
	u32  m_word_cycles;     // oscillator cycles left in the current word
	u8   m_divisor;         // half-period in oscillator cycles, 0 = silent
	u8   m_div_count;       // countdown to the next output toggle
	s8   m_out;             // current output level, -1 or +1
};


class um3481a_device : public um348x_device
{
public:
	um3481a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual const u8 *tone_divisors() const override;
};


class um3482a_device : public um348x_device
{
public:
	um3482a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual const u8 *tone_divisors() const override;
};


DECLARE_DEVICE_TYPE(UM3481A, um3481a_device)
DECLARE_DEVICE_TYPE(UM3482A, um3482a_device)

#endif // MAME_SOUND_UM348X_H
