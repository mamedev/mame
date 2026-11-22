// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Klaus Hotsummer
/**********************************************************************

    Toshiba TC8802AF PARCOR speech synthesizer

    See tc8802.cpp for a full description of the bus protocol, the
    condition register and the frame format.

**********************************************************************/

#ifndef MAME_SOUND_TC8802_H
#define MAME_SOUND_TC8802_H

#pragma once

#include "dirom.h"


class tc8802_device : public device_t,
					  public device_sound_interface,
					  public device_rom_interface<20>
{
public:
	// the frame format is reverse engineered, but the quantizer tables are
	// estimates (see tc8802.cpp), so the output is not bit exact
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	tc8802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// region holding the 8 KiB customer mask ROM inside the chip
	template <typename T> tc8802_device &set_internal_rom_tag(T &&tag) { m_internal_rom.set_tag(std::forward<T>(tag)); return *this; }

	// SEL0-SEL5 on D0-D5, 64UP on D6, START on D7
	void write(uint8_t data);
	// data nibble latched by the last DTRD command on D0-D3
	uint8_t data_r();
	// high while speaking (from START until the end-of-speech frame has been played)
	int busy_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	// condition register bits
	enum : uint8_t
	{
		CNDT_98BIT    = 0x01, // 98 bit frames (unsupported, no known data)
		CNDT_MELODY   = 0x02, // melody mode on the T6803; absent on the TC8802?
		CNDT_EXTROM   = 0x04, // read speech data from the external VSM bus instead of the internal ROM
		CNDT_SOURCE_B = 0x08, // voiced excitation source B (glottal pulse) instead of A (triangle)
		CNDT_LPC8     = 0x10, // 8 filter stages, k9/k10 are not stored in the frames
		CNDT_UNUSED   = 0x20,
		CNDT_10MS     = 0x40, // 10 ms frames instead of 20 ms
		CNDT_50BIT    = 0x80  // 50 bit frames (LPC-8 frames, same layout without k9/k10)
	};

	static constexpr unsigned STAGES = 10;
	static constexpr unsigned INTERP_STEPS = 8;

	void execute_command(uint8_t sel);
	void start_speech();
	void stop_speech();
	uint8_t rom_nibble(uint32_t nibble_address);
	int read_bit();
	unsigned read_bits(unsigned count);
	bool parse_frame();
	unsigned frame_samples() const { return (m_condition & CNDT_10MS) ? 100 : 200; }
	unsigned stages() const { return ((m_condition & CNDT_LPC8) || (m_condition & CNDT_50BIT)) ? 8 : 10; }
	int32_t next_sample();

	required_region_ptr<uint8_t> m_internal_rom;
	sound_stream *m_stream;

	// bus interface state
	uint8_t m_last_write;
	uint8_t m_condition;
	uint8_t m_cndt_count;
	uint8_t m_adld_count;
	uint32_t m_address;          // byte address loaded by ADLD
	uint32_t m_read_nibble;      // nibble pointer used by DTRD
	bool m_dummy_read;
	uint8_t m_data_latch;

	// synthesis state
	bool m_speaking;
	bool m_stopping;             // end of speech seen, fading out the last frame
	uint32_t m_bit_address;      // bit pointer into the selected speech ROM
	uint16_t m_sample;           // sample position inside the current frame
	uint16_t m_pitch_count;
	uint8_t m_pitch_code;        // last pitch code, kept by frames without a pitch field
	uint16_t m_noise;
	int32_t m_glottal[2];
	uint8_t m_kidx[STAGES];      // last decoded coefficient indices (kept by repeat frames)
	bool m_kvalid[STAGES];       // false: coefficient not present in the last full frame (k = 0)
	int32_t m_previous[STAGES + 2]; // energy, pitch period, k1-k10 at the start of the frame
	int32_t m_target[STAGES + 2];   // energy, pitch period, k1-k10 at the end of the frame
	int32_t m_delay[STAGES];
};


DECLARE_DEVICE_TYPE(TC8802, tc8802_device)

#endif // MAME_SOUND_TC8802_H
