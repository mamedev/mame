// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the MCGA device found on Intergraph InterPro family workstations. There is no
* public documentation on this device, so the implementation is being built to follow the logic of the
* system boot ROM and its diagnostic tests.
*
* Please be aware that code in here is not only broken, it's likely wrong in many cases.
*
* TODO
*   - too long to list
*/
#include "emu.h"
#include "interpro_mcga.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_MCGA(...) logerror(__VA_ARGS__)
#else
#define LOG_MCGA(...) {}
#endif

DEVICE_ADDRESS_MAP_START(map, 16, interpro_mcga_device)
  AM_RANGE(0x00, 0x3f) AM_READWRITE16(read, write, 0xffff)
ADDRESS_MAP_END

const device_type INTERPRO_MCGA = device_creator<interpro_mcga_device>;

interpro_mcga_device::interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_MCGA, "InterPro MCGA", tag, owner, clock, "mcga", __FILE__)
{
}

void interpro_mcga_device::device_start()
{
}

void interpro_mcga_device::device_reset()
{
	m_reg[0] = 0x00ff;  // 0x00
	m_reg[2] = MCGA_CTRL_ENREFRESH | MCGA_CTRL_CBITFRCSUB | MCGA_CTRL_CBITFRCRD;  // 0x08 ctrl
																				   //m_mcga[4] = 0x8000;  // 0x10 error
	m_reg[10] = 0x00ff; // 0x28
	m_reg[14] = 0x0340; // 0x38 memsize
}

WRITE16_MEMBER(interpro_mcga_device::write)
{
	/*
	read  MEMSIZE 0x38 mask 0xffff
	read  0x00 mask 0x0000
	write CBSUB   0x20 mask 0x00ff data 0
	write FRCRD   0x18 mask 0x00ff data 0
	read  ERROR   0x10 mask 0xffff
	read  0x00 mask 0xffff

	(0x38 >> 8) & 0xF == 3?

	if (0x00 != 0xFF) -> register reset error

	0x00 = 0x0055 (test value & 0xff)
	r7 = 0x00 & 0xff
	*/
	LOG_MCGA("mcga write offset = 0x%08x, mask = 0x%08x, data = 0x%08x, pc = 0x%08x\n", offset, mem_mask, data, space.device().safe_pc());
	switch (offset)
	{
	case 0x02: // MCGA_CTRL
			   // HACK: set or clear error status depending on ENMMBE bit
		if (data & MCGA_CTRL_ENMMBE)
			m_reg[4] |= MCGA_ERROR_VALID;
		//      else
		//          m_reg[4] &= ~MCGA_ERROR_VALID;

	default:
		m_reg[offset] = data;
		break;
	}
}

READ16_MEMBER(interpro_mcga_device::read)
{
	LOG_MCGA("mcga read offset = 0x%08x, mask = 0x%08x, pc = 0x%08x\n", offset, mem_mask, space.device().safe_pc());

	switch (offset)
	{
	default:
		return m_reg[offset];
	}
}
