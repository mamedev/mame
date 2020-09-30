// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    volt_reg.cpp

    Direct current.

    TODO:
    - If we continue having this device in MAME, add support for default voltage other
      than (currently hardcoded) 5.0

***************************************************************************/

#include "emu.h"
#include "volt_reg.h"

DEFINE_DEVICE_TYPE(VOLTAGE_REGULATOR, voltage_regulator_device, "volt_reg", "Voltage Regulator")

voltage_regulator_device::voltage_regulator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VOLTAGE_REGULATOR, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_output(1.0)
{
}

void voltage_regulator_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
}

void voltage_regulator_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(m_output);
}
