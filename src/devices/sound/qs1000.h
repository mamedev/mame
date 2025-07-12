// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    qs1000.h

    QS1000 device emulator.

***************************************************************************/

#ifndef MAME_SOUND_QS1000_H
#define MAME_SOUND_QS1000_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "sound/okiadpcm.h"
#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qs1000_device

class qs1000_device :   public device_t,
						public device_sound_interface,
						public device_rom_interface<24>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	qs1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_external_rom(bool external_rom) { m_external_rom = external_rom; }
	auto p1_in() { return m_in_p1_cb.bind(); }
	auto p2_in() { return m_in_p2_cb.bind(); }
	auto p3_in() { return m_in_p3_cb.bind(); }
	auto p1_out() { return m_out_p1_cb.bind(); }
	auto p2_out() { return m_out_p2_cb.bind(); }
	auto p3_out() { return m_out_p3_cb.bind(); }

	// external
	i8052_device &cpu() const { return *m_cpu; }
	void set_irq(int state);

	void wave_w(offs_t offset, uint8_t data);

	uint8_t p0_r();
	void p0_w(uint8_t data);

	uint8_t p1_r();
	void p1_w(uint8_t data);

	uint8_t p2_r();
	void p2_w(uint8_t data);

	uint8_t p3_r();
	void p3_w(uint8_t data);

	void qs1000_io_map(address_map &map) ATTR_COLD;
	void qs1000_prg_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	static constexpr unsigned QS1000_CHANNELS       = 32;
	static constexpr offs_t   QS1000_ADDRESS_MASK   = 0x00ffffff;

	enum
	{
		QS1000_KEYON   = 1,
		QS1000_PLAYING = 2,
		QS1000_ADPCM   = 4
	};

	void start_voice(int ch);

	bool                    m_external_rom;

	// Callbacks
	devcb_read8             m_in_p1_cb;
	devcb_read8             m_in_p2_cb;
	devcb_read8             m_in_p3_cb;

	devcb_write8            m_out_p1_cb;
	devcb_write8            m_out_p2_cb;
	devcb_write8            m_out_p3_cb;

	// Internal state
	sound_stream *                  m_stream;
	required_device<i8052_device>   m_cpu;

	// Wavetable engine
	uint8_t                           m_wave_regs[18];

	struct qs1000_channel
	{
		uint32_t          m_acc;
		int32_t           m_adpcm_signal;
		uint32_t          m_start;
		uint32_t          m_addr;
		uint32_t          m_adpcm_addr;
		uint32_t          m_loop_start;
		uint32_t          m_loop_end;
		uint16_t          m_freq;
		uint16_t          m_flags;

		uint8_t           m_regs[16]; // FIXME

		oki_adpcm_state m_adpcm;
	};

	qs1000_channel                  m_channels[QS1000_CHANNELS];
};


// device type definition
DECLARE_DEVICE_TYPE(QS1000, qs1000_device)

#endif // MAME_SOUND_QS1000_H
