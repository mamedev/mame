// license:BSD-3-Clause
// copyright-holders:R. Belmont, Philip Bennett
/***************************************************************************

    qs1000.h

    QS1000 device emulator.

***************************************************************************/

#ifndef MAME_SOUND_QS1000_H
#define MAME_SOUND_QS1000_H

#pragma once

#include "cpu/mcs51/i8052.h"
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

	void qs1000_data_map(address_map &map) ATTR_COLD;
	void qs1000_prg_map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	static constexpr unsigned QS1000_CHANNELS       = 32;
	static constexpr offs_t   QS1000_ADDRESS_MASK   = 0x00ffffff;
	static constexpr unsigned QS1000_ACC_FRAC_BITS  = 26;
	static constexpr uint32_t QS1000_RELEASE_UNIT   = 768 * 16;   // stream samples per (byte 10 + 1) unit of full-scale release

	enum
	{
		QS1000_KEYON   = 1,
		QS1000_PLAYING = 2,
		QS1000_ADPCM   = 4
	};

	struct qs1000_adpcm
	{
		void reset();
		int16_t clock(uint8_t nibble);
		void save();
		void restore();

		int32_t           m_signal;
		int32_t           m_step;
		int32_t           m_loop_signal;
		int32_t           m_loop_step;
		bool              m_saved;
	};

	struct qs1000_channel
	{
		uint32_t          m_acc;
		float             m_previous_sample;
		int32_t           m_adpcm_signal;
		uint32_t          m_addr;
		uint32_t          m_adpcm_addr;
		uint32_t          m_loop_start;
		uint32_t          m_loop_end;
		uint16_t          m_freq;
		uint16_t          m_flags;
		uint16_t          m_env_addr;
		uint16_t          m_env_period;       // source samples per curve point
		uint32_t          m_release_period;   // stream samples for a full-scale release
		uint32_t          m_env_remaining;
		uint32_t          m_env_level;        // 8.16 fixed point
		int32_t           m_env_step;
		uint8_t           m_env_target;

		uint8_t           m_regs[0x11];

		qs1000_adpcm      m_adpcm;
	};

	void start_voice(int ch);
	static void set_envelope_target(qs1000_channel &chan, uint8_t target, uint32_t period);
	void advance_envelope(qs1000_channel &chan, uint32_t samples);

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
	uint8_t                         m_wave_regs[18];
	qs1000_channel                  m_channels[QS1000_CHANNELS];
};


// device type definition
DECLARE_DEVICE_TYPE(QS1000, qs1000_device)

#endif // MAME_SOUND_QS1000_H
