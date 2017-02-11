// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    volt_reg.c

    Direct current.

***************************************************************************/

#include "emu.h"
#include "volt_reg.h"

const device_type VOLTAGE_REGULATOR = &device_creator<voltage_regulator_device>;

voltage_regulator_device::voltage_regulator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VOLTAGE_REGULATOR, "Voltage Regulator", tag, owner, clock, "volt_reg", __FILE__),
	device_sound_interface(mconfig, *this),
	m_output(0)
{
}

void voltage_regulator_device::device_start()
{
	m_stream = stream_alloc(0, 1, 500);
}

void voltage_regulator_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int samp = 0; samp < samples; samp++)
	{
		outputs[0][samp] = m_output;
	}
}
