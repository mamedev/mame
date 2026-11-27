// license:BSD-3-Clause
// copyright-holders:superctr, Carl Lom
/***************************************************************************

    Fujitsu MB654419U (Roland TVF interface gate array)

***************************************************************************/

#ifndef MAME_ROLAND_MB654419U_H
#define MAME_ROLAND_MB654419U_H

#pragma once

#include "cpu/mb8764/mb8764.h"

class mb654419u_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	static constexpr int NUM_VOICES = 16;
	static constexpr int NUM_OUTPUTS = 8;

	mb654419u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_dsp(T &&tag) { m_dsp.set_tag(std::forward<T>(tag)); }

	// host interface: 16 registers
	void map(address_map &map) ATTR_COLD;

	// the TVF16's data bus output (REQ)
	void dsp_w(u16 data);

	// the word on the DAC
	u16 dac_r() const { return m_dac; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void address_w(offs_t offset, u8 data);
	void data_w(offs_t offset, u8 data);
	void control_w(u8 data);
	template <u32 Base> void setup_w(offs_t offset, u8 data);

	required_device<tvf16_device> m_dsp;
	sound_stream *m_stream;

	u16 m_address;
	u16 m_data;
	u16 m_pair[2];
	u16 m_out[NUM_OUTPUTS];
	u8 m_out_count;
	u16 m_dac;
	bool m_load;
};

DECLARE_DEVICE_TYPE(MB654419U, mb654419u_device)

#endif // MAME_ROLAND_MB654419U_H
