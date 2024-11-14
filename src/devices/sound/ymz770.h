// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    ymz770.h

***************************************************************************/

#ifndef MAME_SOUND_YMZ770_H
#define MAME_SOUND_YMZ770_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definition
class mpeg_audio;

// ======================> ymz770_device

class ymz770_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual void internal_reg_write(uint8_t reg, uint8_t data);
	virtual uint32_t get_phrase_offs(int phrase) { return m_rom[(4 * phrase) + 1] << 16 | m_rom[(4 * phrase) + 2] << 8 | m_rom[(4 * phrase) + 3]; }
	virtual uint32_t get_seq_offs(int sqn) { return m_rom[(4 * sqn) + 1 + 0x400] << 16 | m_rom[(4 * sqn) + 2 + 0x400] << 8 | m_rom[(4 * sqn) + 3 + 0x400]; }
	virtual void sequencer();
	uint8_t get_rom_byte(uint32_t offset) { return m_rom[offset % m_rom.bytes()]; } // need optimise or its good as is ?

	ymz770_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t sclock);

	sound_stream *m_stream;
	uint32_t m_sclock;

	// data
	uint8_t m_cur_reg;
	uint8_t m_mute;         // mute chip
	uint8_t m_doen;         // digital output enable
	uint8_t m_vlma;         // overall volume L0/R0
	uint8_t m_vlma1;        // overall volume L1/R1
	uint8_t m_bsl;          // boost level
	uint8_t m_cpl;          // clip limiter
	required_region_ptr<uint8_t> m_rom;

	struct ymz_channel
	{
		uint16_t phrase;
		uint8_t pan;
		uint8_t pan_delay;
		uint8_t pan1;
		uint8_t pan1_delay;
		int32_t volume;
		uint8_t volume_target;
		uint8_t volume_delay;
		uint8_t volume2;
		uint8_t loop;

		bool is_playing, last_block, is_paused;

		mpeg_audio *decoder;

		int16_t output_data[0x1000];
		int output_remaining;
		int output_ptr;
		int atbl;
		int pptr;
	};
	struct ymz_sequence
	{
		uint32_t delay;
		uint16_t sequence;
		uint16_t timer;
		uint16_t stopchan;
		uint8_t loop;
		uint32_t offset;
		uint8_t bank;
		bool is_playing;
		bool is_paused;
	};
	struct ymz_sqc
	{
		uint8_t sqc;
		uint8_t loop;
		uint32_t offset;
		bool is_playing;
		bool is_waiting;
	};

	ymz_channel m_channels[16];
	ymz_sequence m_sequences[8];
	ymz_sqc m_sqcs[8];
};

// ======================> ymz774_device

class ymz774_device : public ymz770_device
{
public:
	// construction/destruction
	ymz774_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);

protected:
	virtual void internal_reg_write(uint8_t reg, uint8_t data) override;
	virtual uint32_t get_phrase_offs(int phrase) override { int ph = phrase * 4; return ((m_rom[ph] & 0x0f) << 24 | m_rom[ph + 1] << 16 | m_rom[ph + 2] << 8 | m_rom[ph + 3]) * 2; }
	virtual uint32_t get_seq_offs(int sqn) override { int sq = sqn * 4 + 0x2000; return ((m_rom[sq] & 0x0f) << 24 | m_rom[sq + 1] << 16 | m_rom[sq + 2] << 8 | m_rom[sq + 3]) * 2; }
	uint32_t get_sqc_offs(int sqc) { int sq = sqc * 4 + 0x6000; return ((m_rom[sq] & 0x0f) << 24 | m_rom[sq + 1] << 16 | m_rom[sq + 2] << 8 | m_rom[sq + 3]) * 2; }
	virtual void sequencer() override;

private:
	int m_bank;
	uint32_t volinc[256];
};

// device type definition
DECLARE_DEVICE_TYPE(YMZ770, ymz770_device)
DECLARE_DEVICE_TYPE(YMZ774, ymz774_device)

#endif // MAME_SOUND_YMZ770_H
