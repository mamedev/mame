// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET 64KB RAM Expansion emulation

**********************************************************************/

#include "emu.h"
#include "64k.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_64K, pet_64k_expansion_device, "pet_64k", "PET 64KB RAM")



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_ram -
//-------------------------------------------------

inline uint8_t pet_64k_expansion_device::read_ram(offs_t offset)
{
	uint8_t data;

	if (offset < 0xc000)
	{
		data = m_ram[(BIT(m_ctrl, 2) << 14) | (offset & 0x3fff)];
	}
	else
	{
		data = m_ram[0x8000 | (BIT(m_ctrl, 3) << 14) | (offset & 0x3fff)];
	}

	return data;
}


//-------------------------------------------------
//  write_ram -
//-------------------------------------------------

inline void pet_64k_expansion_device::write_ram(offs_t offset, uint8_t data)
{
	if (offset < 0xc000)
	{
		if (!BIT(m_ctrl, 0))
		{
			m_ram[(BIT(m_ctrl, 2) << 14) | (offset & 0x3fff)] = data;
		}
	}
	else
	{
		if (!BIT(m_ctrl, 1))
		{
			m_ram[0x8000 | (BIT(m_ctrl, 3) << 14) | (offset & 0x3fff)] = data;
		}
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_64k_expansion_device - constructor
//-------------------------------------------------

pet_64k_expansion_device::pet_64k_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_64K, tag, owner, clock),
	device_pet_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram"),
	m_ctrl(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_64k_expansion_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x10000);

	// state saving
	save_item(NAME(m_ctrl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pet_64k_expansion_device::device_reset()
{
	m_ctrl = 0;
}


//-------------------------------------------------
//  pet_norom_r - NO ROM read
//-------------------------------------------------

int pet_64k_expansion_device::pet_norom_r(offs_t offset, int sel)
{
	return !BIT(m_ctrl, 7);
}


//-------------------------------------------------
//  pet_bd_r - buffered data read
//-------------------------------------------------

uint8_t pet_64k_expansion_device::pet_bd_r(offs_t offset, uint8_t data, int &sel)
{
	if (BIT(m_ctrl, 7))
	{
		switch (sel)
		{
		case pet_expansion_slot_device::SEL8:
			if (!BIT(m_ctrl, 5))
			{
				data = read_ram(offset);
				sel = pet_expansion_slot_device::SEL_NONE;
			}
			break;

		case pet_expansion_slot_device::SELE:
			if (!BIT(m_ctrl, 6) || !BIT(offset, 11))
			{
				data = read_ram(offset);
				sel = pet_expansion_slot_device::SEL_NONE;
			}
			break;

		case pet_expansion_slot_device::SEL9:
		case pet_expansion_slot_device::SELA:
		case pet_expansion_slot_device::SELB:
		case pet_expansion_slot_device::SELC:
		case pet_expansion_slot_device::SELD:
		case pet_expansion_slot_device::SELF:
			data = read_ram(offset);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  pet_bd_w - buffered data write
//-------------------------------------------------

void pet_64k_expansion_device::pet_bd_w(offs_t offset, uint8_t data, int &sel)
{
	if (BIT(m_ctrl, 7))
	{
		switch (sel)
		{
		case pet_expansion_slot_device::SEL8:
			if (!BIT(m_ctrl, 5))
			{
				write_ram(offset, data);
				sel = pet_expansion_slot_device::SEL_NONE;
			}
			break;

		case pet_expansion_slot_device::SELE:
			if (!BIT(m_ctrl, 6) || !BIT(offset, 11))
			{
				write_ram(offset, data);
				sel = pet_expansion_slot_device::SEL_NONE;
			}
			break;

		case pet_expansion_slot_device::SEL9:
		case pet_expansion_slot_device::SELA:
		case pet_expansion_slot_device::SELB:
		case pet_expansion_slot_device::SELC:
		case pet_expansion_slot_device::SELD:
		case pet_expansion_slot_device::SELF:
			write_ram(offset, data);
			break;
		}
	}

	if (offset == 0xfff0)
	{
		m_ctrl = data;
	}
}
