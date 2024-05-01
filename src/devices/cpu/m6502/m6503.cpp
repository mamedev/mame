// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6503.cpp

    MOS Technology 6502, NMOS variant with reduced address bus

    28-pin package, address bus is 12 bits, no NMI, no SO, no SYNC, no RDY.

***************************************************************************/

#include "emu.h"
#include "m6503.h"

DEFINE_DEVICE_TYPE(M6503, m6503_device, "m6503", "MOS Technology 6503")

m6503_device::m6503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6503, tag, owner, clock)
{
	program_config.m_addr_width = 12;
	program_config.m_logaddr_width = 12;
	sprogram_config.m_addr_width = 12;
	sprogram_config.m_logaddr_width = 12;
}
