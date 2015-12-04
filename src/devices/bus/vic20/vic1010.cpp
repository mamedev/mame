// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

**********************************************************************/

#include "vic1010.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1010 = &device_creator<vic1010_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( vic1010 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1010 )
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot1")
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot2")
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot3")
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot4")
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot5")
	MCFG_VIC20_PASSTHRU_EXPANSION_SLOT_ADD("slot6")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic1010_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic1010 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1010_device - constructor
//-------------------------------------------------

vic1010_device::vic1010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC1010, "VIC1010", tag, owner, clock, "vic1010", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this),
		m_slot1(*this, "slot1"),
		m_slot2(*this, "slot2"),
		m_slot3(*this, "slot3"),
		m_slot4(*this, "slot4"),
		m_slot5(*this, "slot5"),
		m_slot6(*this, "slot6")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1010_device::device_start()
{
	// find devices
	m_expansion_slot[0] = m_slot1;
	m_expansion_slot[1] = m_slot2;
	m_expansion_slot[2] = m_slot3;
	m_expansion_slot[3] = m_slot4;
	m_expansion_slot[4] = m_slot5;
	m_expansion_slot[5] = m_slot6;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1010_device::device_reset()
{
	for (auto & elem : m_expansion_slot)
	{
		elem->reset();
	}
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic1010_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	for (auto elem : m_expansion_slot)
	{
		UINT8 slot_data = elem->cd_r(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);

		if (data != slot_data)
		{
			data = slot_data;
		}
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1010_device::vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->cd_w(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}
}
