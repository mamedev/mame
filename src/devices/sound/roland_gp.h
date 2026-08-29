// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_ROLAND_GP_H
#define MAME_SOUND_ROLAND_GP_H

#pragma once

#include "dirom.h"

// Roland GP PCM sound gate array family: GP-2 (TC24SC201AF) and GP-4 (TC6116AF)
//
// Register interface only: the slot register file, the key mask handshake (with the keyed bit
// of the voice control registers), the wave ROM byte readback and the output timing. No sound
// generation or sample-end interrupt yet.
//
// The wave ROM space is the raw 24-bit address the chip presents (bank in bits 20-23);
// the board's chip-select decoding is the driver's ROM map.

class roland_gp_device : public device_t, public device_sound_interface, public device_rom_interface<24>
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	auto int_callback() { return m_int_callback.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	roland_gp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool gp4);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	static constexpr int MAX_VOICES = 28;
	static constexpr u32 MASK20 = 0xfffff;

	// The per-slot registers, in bus order. Slots 0 .. count-1 are voices; slots 28-31 are
	// the effect processor's parameter and work registers.
	//
	// 20-bit registers (three bytes at 0x04-0x0f and 0x24-0x2f, high nibble in the first byte,
	// committed on the third byte; reads load the latch on the first byte):
	//   ADDRESS    current sample address within the ROM bank, start address on key-on
	//   LOOP       loop start
	//   END        loop end
	//   DPCM_REF   DPCM accumulator (last decoded sample), cleared by the firmware on key-on
	//   FILTER_LP  state variable filter lowpass state
	//   FILTER_BP  state variable filter bandpass state
	enum wide_index { ADDRESS, LOOP, END, DPCM_REF, FILTER_LP, FILTER_BP };

	// 16-bit registers (two bytes at 0x10-0x1f and 0x30-0x37, high byte first, committed on the
	// low byte; reads load the latch on the high byte):
	//   PITCH       phase increment, 2.14 fixed point (0x4000 = one ROM sample per frame)
	//   PAN         left (15-8) / right (7-0) gain, signed 8-bit
	//   SEND        reverb (15-8) / chorus (7-0) send gain, signed 8-bit
	//   TVA1, TVA2  amplitude envelopes: target (15-8) / rate (7-0)
	//   TVF         filter cutoff envelope: target / rate
	//   FLAGS       bit 0 interrupt enable, bit 1 output highpass instead of lowpass, bits 14-8 resonance
	//   CONTROL     bits 4-0 slot whose PITCH drives this voice, bit 5 keyed (set by the chip from the
	//               key mask, written 0 by the firmware on key-on), bit 6 alternate loop, bit 7 reverse,
	//               bits 11-8 ROM bank (address bits 23-20), bits 15-12 DPCM block shift (chip)
	//   PHASE       bits 13-0 sub-sample phase (chip), bit 14 interrupt raised for this note (chip),
	//               bit 15 alternate loop running backwards (chip)
	//   *_LEVEL     current envelope levels (target << 7 when settled), maintained by the chip
	enum narrow_index { PITCH, PAN, SEND, TVA1, TVA2, TVF, FLAGS, CONTROL, PHASE, TVA1_LEVEL, TVA2_LEVEL, TVF_LEVEL };

	struct slot
	{
		u32 wide[6] = {};
		u16 narrow[12] = {};
	};

	int slot_count() const { return (m_slot_config & 31) + 1; }
	bool double_rate() const { return m_output_config & 0x40; }
	u32 output_rate() const;
	void update_rate();

	devcb_write_line m_int_callback;
	const bool m_gp4;

	sound_stream *m_stream;

	slot m_slots[32];

	u32 m_key_mask;
	u32 m_key_mask_pending;
	bool m_key_mask_dirty;

	u32 m_write_latch;
	u32 m_read_latch;
	u32 m_rom_address;
	u8 m_rom_byte;

	u8 m_output_config;
	u8 m_slot_config;
	u8 m_selected_slot;
};

// GP-2, TC24SC201AF: SC-55, CM-300 ...
class roland_gp2_device : public roland_gp_device
{
public:
	roland_gp2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// GP-4, TC6116AF: SC-55mkII, JV-80, JV-880 ... (bug fixes and H8/500 bus glue)
class roland_gp4_device : public roland_gp_device
{
public:
	roland_gp4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(ROLAND_GP2, roland_gp2_device)
DECLARE_DEVICE_TYPE(ROLAND_GP4, roland_gp4_device)

#endif // MAME_SOUND_ROLAND_GP_H
