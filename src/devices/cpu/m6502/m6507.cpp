// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6507.c

    MOS Technology 6502, NMOS variant with reduced address bus

***************************************************************************/

#include "emu.h"
#include "m6507.h"

DEFINE_DEVICE_TYPE(M6507, m6507_device, "m6507", "MOS Technology 6507")

m6507_device::m6507_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6507, tag, owner, clock)
{
	program_config.m_addr_width = 13;
	sprogram_config.m_addr_width = 13;
}
