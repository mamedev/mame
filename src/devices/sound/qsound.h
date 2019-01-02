// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*********************************************************

    Capcom System QSound™

*********************************************************/
#ifndef MAME_SOUND_QSOUND_H
#define MAME_SOUND_QSOUND_H

#pragma once

#include "cpu/dsp16/dsp16.h"


class qsound_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	// default 60MHz clock (divided by 2 for DSP core clock, and then by 1248 for sample rate)
	qsound_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 60'000'000);

	DECLARE_WRITE8_MEMBER(qsound_w);
	DECLARE_READ8_MEMBER(qsound_r);

protected:
	// device_t implementation
	tiny_rom_entry const *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_reset() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface implementation
	virtual void rom_bank_updated() override;

	void dsp_io_map(address_map &map);

private:
	// DSP ROM access
	DECLARE_READ16_MEMBER(dsp_sample_r);
	DECLARE_WRITE16_MEMBER(dsp_pio_w);

	// for synchronised DSP communication
	DECLARE_WRITE_LINE_MEMBER(dsp_ock_w);
	DECLARE_READ16_MEMBER(dsp_pio_r);
	void set_dsp_ready(void *ptr, s32 param);
	void set_cmd(void *ptr, s32 param);

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

DECLARE_DEVICE_TYPE(QSOUND, qsound_device)

#if !defined(QSOUND_LLE) // && 0
#include "qsoundhle.h"
#define qsound_device qsound_hle_device
#define QSOUND QSOUND_HLE
#endif // QSOUND_LLE

#endif // MAME_SOUND_QSOUND_H
