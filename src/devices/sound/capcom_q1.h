// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*********************************************************

    Capcom-Q1 DL 1425 DSP

*********************************************************/
#ifndef MAME_SOUND_CAPCOM_Q1_H
#define MAME_SOUND_CAPCOM_Q1_H

#pragma once

#include "dirom.h"
#include "cpu/dsp16/dsp16.h"


class capcom_q1_device : public device_t, public device_sound_interface, public device_rom_interface<24>
{
public:
	// default 60MHz clock (divided by 2 for DSP core clock, and then by 1248 for sample rate)
	capcom_q1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~capcom_q1_device();

	void write(offs_t offset, u8 data);
	u8 read();

protected:
	// device_t implementation
	tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_post_change() override;

	void dsp_io_map(address_map &map) ATTR_COLD;

private:
	// DSP ROM access
	u16 dsp_sample_r(offs_t offset);
	void dsp_pio_w(offs_t offset, u16 data);

	// for synchronised DSP communication
	void dsp_ock_w(int state);
	u16 dsp_pio_r();
	void set_dsp_ready(s32 param);
	void set_cmd(s32 param);

	// MAME resources
	required_device<dsp16_device_base> m_dsp;
	sound_stream *m_stream;

	// DSP communication
	u16 m_rom_bank, m_rom_offset;
	u16 m_cmd_addr, m_cmd_data, m_new_data;
	u8  m_cmd_pending, m_dsp_ready;

	// serial sample recovery
	s16 m_samples[2];
	u16 m_sr, m_fsr;
	u8 m_ock, m_old, m_ready, m_channel;
};

DECLARE_DEVICE_TYPE(CAPCOM_Q1, capcom_q1_device)

// alias CAPCOM_Q1 to CAPCOM_Q1_HLE by default
// define CAPCOM_Q1_LLE to use the DSP emulation
#if !defined(CAPCOM_Q1_LLE)
#include "capcom_q1_hle.h"
#define capcom_q1_device capcom_q1_hle_device
#define CAPCOM_Q1 CAPCOM_Q1_HLE
#endif // CAPCOM_Q1_LLE

#endif // MAME_SOUND_CAPCOM_Q1_H
