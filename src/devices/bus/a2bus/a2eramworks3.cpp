// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eramworks3.c

    Applied Engineering RamWorks III


*********************************************************************/

#include "emu.h"
#include "a2eramworks3.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2EAUX_RAMWORKS3, a2eaux_ramworks3_device, "a2erwks3", "Applied Engineering RamWorks III")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2eaux_ramworks3_device::a2eaux_ramworks3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2eaux_ramworks3_device(mconfig, A2EAUX_RAMWORKS3, tag, owner, clock)
{
}

a2eaux_ramworks3_device::a2eaux_ramworks3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2eauxslot_card_interface(mconfig, *this),
		m_bank(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2eaux_ramworks3_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_bank));
}

void a2eaux_ramworks3_device::device_reset()
{
	m_bank = 0;
}

uint8_t a2eaux_ramworks3_device::read_auxram(uint16_t offset)
{
	return m_ram[offset+m_bank];
}

void a2eaux_ramworks3_device::write_auxram(uint16_t offset, uint8_t data)
{
	m_ram[offset+m_bank] = data;
}

uint8_t *a2eaux_ramworks3_device::get_vram_ptr()
{
	return &m_ram[0];
}

uint8_t *a2eaux_ramworks3_device::get_auxbank_ptr()
{
	return &m_ram[m_bank];
}

/*
    These cards are split into 64k logical banks.

    On a RW3:
    Banks 00-0F is the first MB
    Banks 10-17 are the next 512K
    Banks 30-37 are the next 512K
    Banks 50-57 are the next 512K
    Banks 70-77 are the next 512K

    However, the software will recognize and correctly use a configuration in which
    all of banks 00-7F are populated for a total of 8 megabytes.  So that's what we do.
*/
void a2eaux_ramworks3_device::write_c07x(uint8_t offset, uint8_t data)
{
	// write to C073?
	if (offset == 3)
	{
		m_bank = 0x10000 * (data & 0x7f);
	}
}
