// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    PMI DAC-76 COMDAC

    Companding D/A Converter

              ___ ___
      E/D  1 |*  u   | 10  VLC
       SB  2 |       | 11  VR+
       B1  3 |       | 12  VR-
       B2  4 |       | 13  V-
       B3  5 |       | 14  IOE+
       B4  6 |       | 15  IOE-
       B5  7 |       | 16  IOD+
       B6  8 |       | 17  IOD-
       B7  9 |_______| 18  V+

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
	DECLARE_WRITE_LINE_MEMBER(b1_w) { m_chord &= ~(1 << 2); m_chord |= (state << 2); }
	DECLARE_WRITE_LINE_MEMBER(b2_w) { m_chord &= ~(1 << 1); m_chord |= (state << 1); }
	DECLARE_WRITE_LINE_MEMBER(b3_w) { m_chord &= ~(1 << 0); m_chord |= (state << 0); }

	// step
	DECLARE_WRITE_LINE_MEMBER(b4_w) { m_step &= ~(1 << 3); m_step |= (state << 3); }
	DECLARE_WRITE_LINE_MEMBER(b5_w) { m_step &= ~(1 << 2); m_step |= (state << 2); }
	DECLARE_WRITE_LINE_MEMBER(b6_w) { m_step &= ~(1 << 1); m_step |= (state << 1); }
	DECLARE_WRITE_LINE_MEMBER(b7_w) { m_step &= ~(1 << 0); m_step |= (state << 0); }

	// sign bit
	DECLARE_WRITE_LINE_MEMBER(sb_w) { m_sb = bool(state); }

	void update() { m_stream->update(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	static constexpr int m_level[8] = { 0, 33, 99, 231, 495, 1023, 2079, 4191 };

	sound_stream *m_stream;

	uint8_t m_chord; // 4-bit
	uint8_t m_step; // 3-bit
	bool m_sb;
};

// device type definition
DECLARE_DEVICE_TYPE(DAC76, dac76_device)

#endif // MAME_SOUND_DAC76_H
