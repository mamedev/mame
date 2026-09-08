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
	void ce_w(int state);
	void lp_w(int state);
	void sl_w(int state);
	void as_w(int state);

	int tsp_r() { m_stream->update(); return m_playing ? 1 : 0; }

protected:
	um348x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const u8 *multipliers);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	void decode_tone_rom() ATTR_COLD;

private:
	void stop();
	void start_song();
	void next_song();
	void start_word(u16 index);
	void advance_word();
	u16 melody_start(u8 melody) const;

	required_memory_region m_notes;
	required_memory_region m_offsets;
	required_memory_region m_tones;

	sound_stream *m_stream;
	const u8 *const m_multipliers;
	u8   m_divisors[16];    // oscillator half-period per tone code, 0 = silent
	u16 m_data_end;         // last word that can sound; everything after is filler

	u8  m_ce;
	u8  m_lp;
	u8  m_sl;
	u8  m_as;

	u8  m_song;             // where the select counter is pointing

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
};


class um3482a_device : public um348x_device
{
public:
	um3482a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(UM3481A, um3481a_device)
DECLARE_DEVICE_TYPE(UM3482A, um3482a_device)

#endif // MAME_SOUND_UM348X_H
