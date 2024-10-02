// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    ARM7TDMI CPU component of the AP2010 LSI

***************************************************************************/

#include "emu.h"
#include "ap2010cpu.h"

#include "arm7core.h"


DEFINE_DEVICE_TYPE(AP2010CPU, ap2010cpu_device, "ap2010cpu", "AP2010 CPU")

ap2010cpu_device::ap2010cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, AP2010CPU, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_BIG)
{ }

void ap2010cpu_device::add_hotspot(offs_t pc)
{
	if (m_hotspot_select < std::size(m_hotspot)) {
		m_hotspot[m_hotspot_select] = pc;
		m_hotspot_select++;
	}
}

void ap2010cpu_device::execute_run()
{
	for (size_t i = 0; i < ARM7_MAX_HOTSPOTS; i++) {
		if (m_hotspot[i] == 0) {
			break;
		}
		if (m_hotspot[i] == pc()) {
			int32_t icount = *m_icountptr;
			if (icount > 30) {
				eat_cycles(icount - 30);
				break;
			}
		}
	}

	arm7_cpu_device::execute_run();
}
