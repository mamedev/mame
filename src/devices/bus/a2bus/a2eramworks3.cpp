// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eramworks3.c

    Applied Engineering RamWorks III

    The AE RamWorks patent is US 4601018.

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
DEFINE_DEVICE_TYPE(A2EAUX_FRANKLIN384, a2eaux_franklin384_device, "a2ef384", "Franklin ACE 500 expansion RAM")
DEFINE_DEVICE_TYPE(A2EAUX_FRANKLIN512, a2eaux_franklin512_device, "a2ef512", "Franklin ACE 2x00 expansion RAM")

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

a2eaux_franklin384_device::a2eaux_franklin384_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2eaux_ramworks3_device(mconfig, A2EAUX_FRANKLIN384, tag, owner, clock)
{
}

a2eaux_franklin512_device::a2eaux_franklin512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2eaux_ramworks3_device(mconfig, A2EAUX_FRANKLIN512, tag, owner, clock)
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
	// write to C071/3/5/7?
	if ((offset & 0x9) == 1)
	{
		m_bank = 0x10000 * (data & 0x7f);
	}
}

void a2eaux_franklin384_device::write_c07x(uint8_t offset, uint8_t data)
{
	if ((offset & 0x9) == 1)
	{
	   // RamWorks/Z-RAM bank order is 0 3 4 7 8 11 12 15
	   // so cut off access above bank 11 to limit to 384K.
	   if (data > 11)
	   {
		  data = 0;
	   }
	   m_bank = 0x10000 * (data & 0xf);
	}
}

void a2eaux_franklin512_device::write_c07x(uint8_t offset, uint8_t data)
{
   if ((offset & 0x9) == 1)
   {
	  m_bank = 0x10000 * (data & 0x0f);
   }
}

