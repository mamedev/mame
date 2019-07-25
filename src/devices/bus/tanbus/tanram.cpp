// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANRAM (MT013 Iss2)

    http://www.microtan.ukpc.net/pageProducts.html#RAM

**********************************************************************/


#include "emu.h"
#include "tanram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TANRAM, tanbus_tanram_device, "tanbus_tanram", "Tangerine Tanram Board")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tanram_device - constructor
//-------------------------------------------------

tanbus_tanram_device::tanbus_tanram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TANRAM, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tanram_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x9c00);

	save_pointer(NAME(m_ram), 0x9c00);
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tanram_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	if (be && !inhram)
	{
		/* 32K dynamic ram */
		if ((offset >= 0x2000) && (offset < 0xa000))
		{
			data = m_ram[offset - 0x2000];
		}

		/* 7K static ram */
		if ((offset >= 0xa000) && (offset < 0xbc00))
		{
			data = m_ram[offset - 0xa000];
		}
	}
	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tanram_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	if (be && !inhram)
	{
		/* 32K dynamic ram */
		if ((offset >= 0x2000) && (offset < 0xa000))
		{
			m_ram[offset - 0x2000] = data;
		}

		/* 7K static ram */
		if ((offset >= 0xa000) && (offset < 0xbc00))
		{
			m_ram[offset - 0xa000] = data;
		}
	}
}
