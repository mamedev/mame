/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vic1010.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1010 = &device_creator<vic1010_device>;


//-------------------------------------------------
//  VIC20_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vic1010_device::irq_w )
{
	m_slot->irq_w(state);
}

WRITE_LINE_MEMBER( vic1010_device::nmi_w )
{
	m_slot->nmi_w(state);
}

WRITE_LINE_MEMBER( vic1010_device::res_w )
{
	m_slot->res_w(state);
}

static VIC20_EXPANSION_INTERFACE( expansion_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vic1010_device, irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vic1010_device, nmi_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vic1010_device, res_w)
};


//-------------------------------------------------
//  MACHINE_DRIVER( vic1010 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1010 )
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot1", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot2", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot3", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot4", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot5", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
	MCFG_VIC20_EXPANSION_SLOT_ADD("slot6", 0, expansion_intf, vic20_expansion_cards, NULL, NULL)
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
	: device_t(mconfig, VIC1010, "VIC1010", tag, owner, clock),
		device_vic20_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1010_device::device_start()
{
	// find devices
	m_expansion_slot[0] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot1"));
	m_expansion_slot[1] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot2"));
	m_expansion_slot[2] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot3"));
	m_expansion_slot[3] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot4"));
	m_expansion_slot[4] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot5"));
	m_expansion_slot[5] = dynamic_cast<vic20_expansion_slot_device *>(subdevice("slot6"));
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic1010_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		UINT8 slot_data = m_expansion_slot[i]->cd_r(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);

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
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->cd_w(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}
}


//-------------------------------------------------
//  vic20_res_w - reset write
//-------------------------------------------------

void vic1010_device::vic20_res_w(int state)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->port_res_w(state);
	}
}
