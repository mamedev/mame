// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_Y8950_H
#define MAME_SOUND_Y8950_H

#pragma once

#include "ymfm.h"


// ======================> y8950_device

DECLARE_DEVICE_TYPE(Y8950, y8950_device);

class y8950_device : public device_t, public device_sound_interface, public device_rom_interface<21>
{
	static constexpr u8 STATUS_ADPCM_B_PLAYING = 0x01;
	static constexpr u8 STATUS_ADPCM_B_BRDY = 0x08;
	static constexpr u8 STATUS_ADPCM_B_EOS = 0x10;
	static constexpr u8 ALL_IRQS = STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_EOS | ymopl_registers::STATUS_TIMERA | ymopl_registers::STATUS_TIMERB;

public:
	// constructor
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = Y8950);

	// configuration helpers
	auto irq_handler() { return m_opl.irq_handler(); }
	auto keyboard_read() { return m_keyboard_read_handler.bind(); }
	auto keyboard_write() { return m_keyboard_write_handler.bind(); }
	auto io_read() { return m_io_read_handler.bind(); }
	auto io_write() { return m_io_write_handler.bind(); }

	// read/write access
	u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	u8 read_port_r() { return read(0); }
	u8 status_port_r() { return read(1); }
	void control_port_w(u8 data) { write(0, data); }
	void write_port_w(u8 data) { write(1, data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void rom_bank_updated() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal state
	u8 m_address;                    // address register
	u8 m_io_ddr;                     // data direction register for I/O
	u8 m_irq_mask;                   // current IRQ mask bits
	sound_stream *m_stream;          // sound stream
	ymopl2_engine m_opl;             // core OPL engine
	ymadpcm_b_engine m_adpcm_b;      // ADPCM-B engine
	address_space_config const m_adpcm_b_config; // address space config (ADPCM-B)
	optional_memory_region m_adpcm_b_region; // ADPCM-B memory region
	devcb_read8 m_keyboard_read_handler;
	devcb_write8 m_keyboard_write_handler;
	devcb_read8 m_io_read_handler;
	devcb_write8 m_io_write_handler;
};


#endif // MAME_SOUND_Y8950_H
