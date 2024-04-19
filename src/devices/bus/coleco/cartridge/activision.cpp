// license: BSD-3-Clause
// copyright-holders: Dirk Best
/**********************************************************************

    ColecoVision 'Activision' cartridge emulation

**********************************************************************/

#include "emu.h"
#include "activision.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECOVISION_ACTIVISION, colecovision_activision_cartridge_device, "coleco_activision", "ColecoVision Activision Cartridge")
DEFINE_DEVICE_TYPE(COLECOVISION_ACTIVISION_256B, colecovision_activision_256b_cartridge_device, "coleco_activision_256b", "ColecoVision Activision Cartridge (256B EEPROM)")
DEFINE_DEVICE_TYPE(COLECOVISION_ACTIVISION_32K, colecovision_activision_32k_cartridge_device, "coleco_activision_32k", "ColecoVision Activision Cartridge (32K EEPROM)")

colecovision_activision_cartridge_device::colecovision_activision_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECOVISION_ACTIVISION, tag, owner, clock),
	device_colecovision_cartridge_interface(mconfig, *this),
	m_eeprom(*this, "eeprom"),
	m_active_bank(0)
{
}

colecovision_activision_cartridge_device::colecovision_activision_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_colecovision_cartridge_interface(mconfig, *this),
	m_eeprom(*this, "eeprom"),
	m_active_bank(0)
{
}

colecovision_activision_256b_cartridge_device::colecovision_activision_256b_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	colecovision_activision_cartridge_device(mconfig, COLECOVISION_ACTIVISION_256B, tag, owner, clock)
{
}

colecovision_activision_32k_cartridge_device::colecovision_activision_32k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	colecovision_activision_cartridge_device(mconfig, COLECOVISION_ACTIVISION_32K, tag, owner, clock)
{
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void colecovision_activision_256b_cartridge_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_eeprom, 0);
}

void colecovision_activision_32k_cartridge_device::device_add_mconfig(machine_config &config)
{
	I2C_24C256(config, m_eeprom, 0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void colecovision_activision_cartridge_device::device_start()
{
	// register for save states
	save_item(NAME(m_active_bank));
}

uint8_t colecovision_activision_cartridge_device::read(offs_t offset, int _8000, int _a000, int _c000, int _e000)
{
	uint8_t data = 0xff;

	if (!_8000 || !_a000 || !_c000 || !_e000)
	{
		if (offset < 0x4000)
		{
			// fixed first rom bank
			data = m_rom[offset];
		}
		else if (offset == 0x7f80)
		{
			// eeprom data
			if (m_eeprom.found())
				data = m_eeprom->read_sda();
			else
				data = 0xff;
		}
		else if (offset > 0x7f80)
		{
			// "dead" area
			data = 0xff;
		}
		else
		{
			// bankswitched rom
			data = m_rom[(m_active_bank << 14) | (offset & 0x3fff)];
		}
	}

	return data;
}

void colecovision_activision_cartridge_device::write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000)
{
	switch (offset)
	{
		// bankswitch
		case 0x7f90:
		case 0x7fa0:
		case 0x7fb0:
			m_active_bank = (offset >> 4) & 0x03;
			break;

		// eeprom scl
		case 0x7fc0:
		case 0x7fd0:
			if (m_eeprom.found())
				m_eeprom->write_scl(BIT(offset, 4));
			break;

		// eeprom sda
		case 0x7fe0:
		case 0x7ff0:
			if (m_eeprom.found())
				m_eeprom->write_sda(BIT(offset, 4));
			break;
	}
}
