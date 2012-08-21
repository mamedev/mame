/**********************************************************************

    Luxor ABC 890 bus expander emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "abc890.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC890 = &device_creator<abc890_device>;
const device_type ABC894 = &device_creator<abc894_device>;
const device_type ABC850 = &device_creator<abc850_device>;


//-------------------------------------------------
//  ABCBUS_INTERFACE( abcbus_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( abc890_device::int_w )
{
	m_bus->int_w(state);
}

WRITE_LINE_MEMBER( abc890_device::nmi_w )
{
	m_bus->nmi_w(state);
}

WRITE_LINE_MEMBER( abc890_device::rdy_w )
{
	m_bus->rdy_w(state);
}

WRITE_LINE_MEMBER( abc890_device::resin_w )
{
	m_bus->resin_w(state);
}

static ABCBUS_INTERFACE( abcbus_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, abc890_device, int_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, abc890_device, nmi_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, abc890_device, rdy_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, abc890_device, resin_w)
};


//-------------------------------------------------
//  MACHINE_DRIVER( abc890 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc890 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem1", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem2", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem3", abcbus_intf, abcbus_cards, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc890_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc890 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( abc894 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc894 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_intf, abcbus_cards, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc894_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc894 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( abc850 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc850 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_intf, abcbus_cards, "fast", abc850_fast)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_intf, abcbus_cards, "hdc", NULL)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_intf, abcbus_cards, NULL, NULL)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_intf, abcbus_cards, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc850_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc850 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc890_device - constructor
//-------------------------------------------------

abc890_device::abc890_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_abcbus_card_interface(mconfig, *this)
{
}

abc890_device::abc890_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, ABC890, "ABC 890", tag, owner, clock),
	  device_abcbus_card_interface(mconfig, *this),
	  m_slots(7)
{
}

abc894_device::abc894_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : abc890_device(mconfig, ABC894, "ABC 894", tag, owner, clock)
{
	m_slots = 3;
}

abc850_device::abc850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : abc890_device(mconfig, ABC850, "ABC 850", tag, owner, clock)
{
	m_slots = 7;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc890_device::device_start()
{
	// find devices
	m_expansion_slot[0] = dynamic_cast<abcbus_slot_device *>(subdevice("io1"));
	m_expansion_slot[1] = dynamic_cast<abcbus_slot_device *>(subdevice("io2"));
	m_expansion_slot[2] = dynamic_cast<abcbus_slot_device *>(subdevice("io3"));
	m_expansion_slot[3] = dynamic_cast<abcbus_slot_device *>(subdevice("io4"));
	m_expansion_slot[4] = dynamic_cast<abcbus_slot_device *>(subdevice("mem1"));
	m_expansion_slot[5] = dynamic_cast<abcbus_slot_device *>(subdevice("mem2"));
	m_expansion_slot[6] = dynamic_cast<abcbus_slot_device *>(subdevice("mem3"));
}

void abc894_device::device_start()
{
	// find devices
	m_expansion_slot[0] = dynamic_cast<abcbus_slot_device *>(subdevice("io1"));
	m_expansion_slot[1] = dynamic_cast<abcbus_slot_device *>(subdevice("io2"));
	m_expansion_slot[2] = dynamic_cast<abcbus_slot_device *>(subdevice("io3"));
}

void abc850_device::device_start()
{
	// find devices
	m_expansion_slot[0] = dynamic_cast<abcbus_slot_device *>(subdevice("io1"));
	m_expansion_slot[1] = dynamic_cast<abcbus_slot_device *>(subdevice("io2"));
	m_expansion_slot[2] = dynamic_cast<abcbus_slot_device *>(subdevice("io3"));
	m_expansion_slot[3] = dynamic_cast<abcbus_slot_device *>(subdevice("io4"));
	m_expansion_slot[4] = dynamic_cast<abcbus_slot_device *>(subdevice("io5"));
	m_expansion_slot[5] = dynamic_cast<abcbus_slot_device *>(subdevice("io6"));
	m_expansion_slot[6] = dynamic_cast<abcbus_slot_device *>(subdevice("io7"));
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

void abc890_device::abcbus_cs(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->cs_w(data);
	}
}

void abc890_device::abcbus_rst(int state)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->rst_r();
	}
}

UINT8 abc890_device::abcbus_inp()
{
	UINT8 data = 0xff;

	for (int i = 0; i < m_slots; i++)
	{
		data &= m_expansion_slot[i]->inp_r();
	}

	return data;
}

void abc890_device::abcbus_utp(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->utp_w(data);
	}
}

UINT8 abc890_device::abcbus_stat()
{
	UINT8 data = 0xff;

	for (int i = 0; i < m_slots; i++)
	{
		data &= m_expansion_slot[i]->stat_r();
	}

	return data;
}

void abc890_device::abcbus_c1(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->c1_w(data);
	}
}

void abc890_device::abcbus_c2(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->c2_w(data);
	}
}

void abc890_device::abcbus_c3(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->c3_w(data);
	}
}

void abc890_device::abcbus_c4(UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->c4_w(data);
	}
}

UINT8 abc890_device::abcbus_xmemfl(offs_t offset)
{
	UINT8 data = 0xff;

	for (int i = 0; i < m_slots; i++)
	{
		data &= m_expansion_slot[i]->xmemfl_r(offset);
	}

	return data;
}

void abc890_device::abcbus_xmemw(offs_t offset, UINT8 data)
{
	for (int i = 0; i < m_slots; i++)
	{
		m_expansion_slot[i]->xmemw_w(offset, data);
	}
}
