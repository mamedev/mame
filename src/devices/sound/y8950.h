// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_Y8950_H
#define MAME_SOUND_Y8950_H

#pragma once

#include "ymfm.h"
#include "ymadpcm.h"


// ======================> y8950_device

DECLARE_DEVICE_TYPE(Y8950, y8950_device);

class y8950_device : public device_t, public device_sound_interface, public device_rom_interface<21>
{
public:
	// YM2151 is OPL
	using fm_engine = ymopl_engine;

	static constexpr u8 STATUS_ADPCM_B_PLAYING = 0x01;
	static constexpr u8 STATUS_ADPCM_B_BRDY = 0x08;
	static constexpr u8 STATUS_ADPCM_B_EOS = 0x10;
	static constexpr u8 ALL_IRQS = STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_EOS | fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB;

	// constructor
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = Y8950);

	// configuration helpers
	auto irq_handler() { return m_fm.irq_handler(); }
	auto keyboard_read() { return m_keyboard_read_handler.bind(); }
	auto keyboard_write() { return m_keyboard_write_handler.bind(); }
	auto io_read() { return m_io_read_handler.bind(); }
	auto io_write() { return m_io_write_handler.bind(); }

	// read access
	u8 status_r();  // A0=0
	u8 data_r();    // A0=1
	u8 read(offs_t offset);

	// write access
	void address_w(u8 data);    // A0=0
	void data_w(u8 data);       // A0=1
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// ROM device overrides
	virtual void rom_bank_updated() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// combine ADPCM and OPN statuses
	u8 combine_status();

	// ADPCM read/write callbacks
	u8 adpcm_b_read(offs_t address);
	void adpcm_b_write(offs_t address, u8 data);

	// internal state
	u8 m_address;                    // address register
	u8 m_io_ddr;                     // data direction register for I/O
	sound_stream *m_stream;          // sound stream
	fm_engine m_fm;                  // core FM engine
	ymadpcm_b_engine m_adpcm_b;      // ADPCM-B engine
	devcb_read8 m_keyboard_read_handler; // keyboard port read
	devcb_write8 m_keyboard_write_handler; // keyboard port write
	devcb_read8 m_io_read_handler;   // I/O port read
	devcb_write8 m_io_write_handler; // I/O port write
};


#endif // MAME_SOUND_Y8950_H
