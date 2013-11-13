// license:BSD-3-Clause
// copyright-holders:Curt Coder
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
const device_type ABC852 = &device_creator<abc852_device>;
const device_type ABC856 = &device_creator<abc856_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( abc890 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc890 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem1", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem2", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("mem3", abcbus_cards, NULL)
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
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, NULL)
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
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_DEVICE_CARD_DEFAULT_BIOS("xebec", "ro202")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc850_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc850 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( abc852 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc852 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_DEVICE_CARD_DEFAULT_BIOS("xebec", "basf6185")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc852_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc852 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( abc856 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc856 )
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_DEVICE_CARD_DEFAULT_BIOS("xebec", "micr1325")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, NULL)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc856_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc856 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc890_device - constructor
//-------------------------------------------------

abc890_device::abc890_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_abcbus_card_interface(mconfig, *this)
{
}

abc890_device::abc890_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC890, "ABC 890", tag, owner, clock, "abc890", __FILE__),
		device_abcbus_card_interface(mconfig, *this)
{
}

abc894_device::abc894_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: abc890_device(mconfig, ABC894, "ABC 894", tag, owner, clock, "abc894", __FILE__)
{
}

abc850_device::abc850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: abc890_device(mconfig, ABC850, "ABC 850", tag, owner, clock, "abc850", __FILE__)
{
}

abc852_device::abc852_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: abc890_device(mconfig, ABC852, "ABC 852", tag, owner, clock, "abc852", __FILE__)
{
}

abc856_device::abc856_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: abc890_device(mconfig, ABC856, "ABC 856", tag, owner, clock, "abc856", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc890_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc890_device::device_reset()
{
	for (device_t *device = first_subdevice(); device != NULL; device = device->next())
	{
		device->reset();
	}
}


//-------------------------------------------------
//  abcbus_cs - card select
//-------------------------------------------------

void abc890_device::abcbus_cs(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->cs_w(data);
	}
}


//-------------------------------------------------
//  abcbus_inp - input
//-------------------------------------------------

UINT8 abc890_device::abcbus_inp()
{
	UINT8 data = 0xff;

	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		data &= slot->inp_r();
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out - output
//-------------------------------------------------

void abc890_device::abcbus_out(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->out_w(data);
	}
}


//-------------------------------------------------
//  abcbus_stat - status
//-------------------------------------------------

UINT8 abc890_device::abcbus_stat()
{
	UINT8 data = 0xff;

	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		data &= slot->stat_r();
	}

	return data;
}


//-------------------------------------------------
//  abcbus_c1 - command 1
//-------------------------------------------------

void abc890_device::abcbus_c1(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->c1_w(data);
	}
}


//-------------------------------------------------
//  abcbus_c2 - command 2
//-------------------------------------------------

void abc890_device::abcbus_c2(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->c2_w(data);
	}
}


//-------------------------------------------------
//  abcbus_c3 - command 3
//-------------------------------------------------

void abc890_device::abcbus_c3(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->c3_w(data);
	}
}


//-------------------------------------------------
//  abcbus_c4 - command 4
//-------------------------------------------------

void abc890_device::abcbus_c4(UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->c4_w(data);
	}
}


//-------------------------------------------------
//  abcbus_xmemfl - extended memory read
//-------------------------------------------------

UINT8 abc890_device::abcbus_xmemfl(offs_t offset)
{
	UINT8 data = 0xff;

	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		data &= slot->xmemfl_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_xmemw - extended memory write
//-------------------------------------------------

void abc890_device::abcbus_xmemw(offs_t offset, UINT8 data)
{
	abcbus_slot_device_iterator iter(*this);

	for (abcbus_slot_device *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		slot->xmemw_w(offset, data);
	}
}
