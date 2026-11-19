// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert

/***************************************************************************

    Astro Corp. CPU Code (protection key inside the V102PX custom CPU)

    The 16-bit key comes from the region with the same tag as the device.

    The games read it serially: after CS goes active, one bit per read of
    the DO line, MSB first. Most of them read it while reading the external
    EEPROM, sampling DO once per data bit (e.g. hapfarm $5B28, pengprty
    $19944), so the clock pulses and the length of the command part don't
    matter. keno21 ($705E) reads the 16 bits with no clock at all, so it's
    the read that advances.

    do_read() has no side effects because drivers may reach it through an
    I/O port, and the input system reads every port once per frame
    (ioport_manager::frame_update): advancing there loses bits whenever a
    frame ends between CS and the reads. Drivers call next_bit() from their
    memory read handler instead.

***************************************************************************/

#include "emu.h"

#include "astro_cpucode.h"


DEFINE_DEVICE_TYPE(ASTRO_CPUCODE, astro_cpucode_device, "astro_cpucode", "Astro Corp. CPU code")

astro_cpucode_device::astro_cpucode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ASTRO_CPUCODE, tag, owner, clock)
	, m_region_key(*this, DEVICE_SELF)
	, m_cs_state(CLEAR_LINE)
	, m_bit_index(0)
{
}

void astro_cpucode_device::device_start()
{
	save_item(NAME(m_cs_state));
	save_item(NAME(m_bit_index));
}

void astro_cpucode_device::device_reset()
{
	m_bit_index = 0;
}

int astro_cpucode_device::do_read()
{
	return BIT(*m_region_key, 15 - m_bit_index);
}

void astro_cpucode_device::next_bit()
{
	if (!machine().side_effects_disabled())
		m_bit_index = (m_bit_index + 1) & 0x0f;
}

void astro_cpucode_device::cs_write(int state)
{
	if (state != m_cs_state)
	{
		m_cs_state = state;
		if (state == ASSERT_LINE)
			m_bit_index = 0;
	}
}
