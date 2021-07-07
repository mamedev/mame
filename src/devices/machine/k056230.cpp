// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

Konami IC 056230 (LANC)

Device Notes:
-The custom IC itself
-64k shared ram
-LS161 4-bit binary counter
-PAL(056787) for racinfrc's sub board and plygonet.cpp
-PAL(056787A) for zr107.cpp, gticlub.cpp and thunderh's I/O board
-HYC2485S RS485 transceiver

TODO: nearly everything

***************************************************************************/

#include "emu.h"
#include "k056230.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(K056230, k056230_device, "k056230", "K056230 LANC")

//-------------------------------------------------
//  k056230_device - constructor
//-------------------------------------------------

k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K056230, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k056230_device::device_start()
{
	save_item(NAME(m_ram));
}


uint8_t k056230_device::read(offs_t offset)
{
	switch (offset)
	{
		case 0:     // Status register
		{
			return 0x08;
		}
		case 1:		// CRC Error register
		{
			return 0x00;
		}
	}

//  logerror("k056230_r: %d %s\n", offset, machine().describe_context());

	return 0;
}

void k056230_device::write(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:     // Mode register
		{
			break;
		}
		case 1:     // Control register
		{
			if(data & 0x20)
			{
				if (m_cpu)
					m_cpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);			
			}
			if ((data & 1) == 0)
			{
				if (m_cpu)
					m_cpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
			}

			break;
		}
		case 2:     // Sub ID register
		{
			break;
		}
	}
//  logerror("k056230_w: %d, %02X at %08X\n", offset, data, machine().describe_context());
}

uint32_t k056230_device::lanc_ram_r(offs_t offset, uint32_t mem_mask)
{
	//logerror("LANC_RAM_r: %08X, %08X %s\n", offset, mem_mask, machine().describe_context());
	return m_ram[offset & 0x7ff];
}

void k056230_device::lanc_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//logerror("LANC_RAM_w: %08X, %08X, %08X %s\n", data, offset, mem_mask, machine().describe_context());
	COMBINE_DATA(m_ram + (offset & 0x7ff));
}
