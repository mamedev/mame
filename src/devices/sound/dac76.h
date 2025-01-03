// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    PMI DAC-76 COMDAC

    Companding D/A Converter

    Equivalent to the AM6070, which is an "improved pin-for-pin replacement for
    DAC-76" (according to the AM6070 datasheet).

              ___ ___
      E/D  1 |*  u   | 18  V+
       SB  2 |       | 17  IOD-
       B1  3 |       | 16  IOD+
       B2  4 |       | 15  IOE-
       B3  5 |       | 14  IOE+
       B4  6 |       | 13  V-
       B5  7 |       | 12  VR-
       B6  8 |       | 11  VR+
       B7  9 |_______| 10  VLC

***************************************************************************/

#ifndef MAME_SOUND_DAC76_H
#define MAME_SOUND_DAC76_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dac76_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dac76_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// chord
	void b1_w(int state) { m_chord &= ~(1 << 2); m_chord |= (state << 2); }
	void b2_w(int state) { m_chord &= ~(1 << 1); m_chord |= (state << 1); }
	void b3_w(int state) { m_chord &= ~(1 << 0); m_chord |= (state << 0); }

	// step
	void b4_w(int state) { m_step &= ~(1 << 3); m_step |= (state << 3); }
	void b5_w(int state) { m_step &= ~(1 << 2); m_step |= (state << 2); }
	void b6_w(int state) { m_step &= ~(1 << 1); m_step |= (state << 1); }
	void b7_w(int state) { m_step &= ~(1 << 0); m_step |= (state << 0); }

	// sign bit
	void sb_w(int state) { m_sb = bool(state); }

	void update() { m_stream->update(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	static constexpr int m_level[8] = { 0, 33, 99, 231, 495, 1023, 2079, 4191 };

	sound_stream *m_stream;

	uint8_t m_chord; // 3-bit
	uint8_t m_step; // 4-bit
	bool m_sb;
};

// device type definition
DECLARE_DEVICE_TYPE(DAC76, dac76_device)

#endif // MAME_SOUND_DAC76_H
