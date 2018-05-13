// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  The Konami_1 CPU is a 6809 with opcodes scrambled.

***************************************************************************/

#include "emu.h"
#include "konami1.h"

DEFINE_DEVICE_TYPE(KONAMI1, konami1_device, "konami1", "KONAMI-1")

konami1_device::konami1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6809_base_device(mconfig, tag, owner, clock, KONAMI1, 1)
{
	m_boundary = 0x0000;
}

void konami1_device::device_start()
{
	m_mintf = std::make_unique<mi_konami1>(m_boundary);
	m6809_base_device::device_start();
}

void konami1_device::set_encryption_boundary(uint16_t adr)
{
	m_boundary = adr;
	if(m_mintf)
		downcast<mi_konami1 *>(m_mintf.get())->m_boundary = adr;
}

konami1_device::mi_konami1::mi_konami1(uint16_t adr)
{
	m_boundary = adr;
}

uint8_t konami1_device::mi_konami1::read_opcode(uint16_t adr)
{
	uint8_t val = m_scache->read_byte(adr);
	if(adr < m_boundary)
		return val;
	switch(adr & 0xa) {
	default:
	case 0x0: return val ^ 0x22;
	case 0x2: return val ^ 0x82;
	case 0x8: return val ^ 0x28;
	case 0xa: return val ^ 0x88;
	}
}
