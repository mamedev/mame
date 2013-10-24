// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC (Databoard 4680) Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "abcbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ABCBUS_SLOT = &device_creator<abcbus_slot_device>;



//**************************************************************************
//  DEVICE ABCBUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_abcbus_card_interface - constructor
//-------------------------------------------------

device_abcbus_card_interface::device_abcbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<abcbus_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abcbus_slot_device - constructor
//-------------------------------------------------

abcbus_slot_device::abcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABCBUS_SLOT, "ABC bus slot", tag, owner, clock, "abcbus_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_int(*this),
	m_write_nmi(*this),
	m_write_rdy(*this),
	m_write_resin(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abcbus_slot_device::device_start()
{
	m_card = dynamic_cast<device_abcbus_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_rdy.resolve_safe();
	m_write_resin.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abcbus_slot_device::device_reset()
{
	if (m_card != NULL)
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  cs_w -
//-------------------------------------------------

void abcbus_slot_device::cs_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_cs(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::cs_w )
{
	cs_w(data);
}


//-------------------------------------------------
//  rst_r -
//-------------------------------------------------

UINT8 abcbus_slot_device::rst_r()
{
	device_reset();

	return 0xff;
}

READ8_MEMBER( abcbus_slot_device::rst_r )
{
	return rst_r();
}


//-------------------------------------------------
//  inp_r -
//-------------------------------------------------

UINT8 abcbus_slot_device::inp_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abcbus_inp();
	}

	return data;
}

READ8_MEMBER( abcbus_slot_device::inp_r )
{
	return inp_r();
}


//-------------------------------------------------
//  utp_w -
//-------------------------------------------------

void abcbus_slot_device::utp_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_utp(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::utp_w )
{
	utp_w(data);
}


//-------------------------------------------------
//  stat_r -
//-------------------------------------------------

UINT8 abcbus_slot_device::stat_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abcbus_stat();
	}

	return data;
}

READ8_MEMBER( abcbus_slot_device::stat_r )
{
	return stat_r();
}


//-------------------------------------------------
//  c1_w -
//-------------------------------------------------

void abcbus_slot_device::c1_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_c1(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::c1_w )
{
	c1_w(data);
}


//-------------------------------------------------
//  c2_w -
//-------------------------------------------------

void abcbus_slot_device::c2_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_c2(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::c2_w )
{
	c2_w(data);
}


//-------------------------------------------------
//  c3_w -
//-------------------------------------------------

void abcbus_slot_device::c3_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_c3(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::c3_w )
{
	c3_w(data);
}


//-------------------------------------------------
//  c4_w -
//-------------------------------------------------

void abcbus_slot_device::c4_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_c4(data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::c4_w )
{
	c4_w(data);
}


//-------------------------------------------------
//  xmemfl_r -
//-------------------------------------------------

UINT8 abcbus_slot_device::xmemfl_r(offs_t offset)
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abcbus_xmemfl(offset);
	}

	return data;
}

READ8_MEMBER( abcbus_slot_device::xmemfl_r )
{
	return xmemfl_r(offset);
}


//-------------------------------------------------
//  xmemw_w -
//-------------------------------------------------

void abcbus_slot_device::xmemw_w(offs_t offset, UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abcbus_xmemw(offset, data);
	}
}

WRITE8_MEMBER( abcbus_slot_device::xmemw_w )
{
	xmemw_w(offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( abcbus_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abcbus_cards )
	SLOT_INTERFACE("exp", ABC890)
	SLOT_INTERFACE("exp3", ABC894)
	SLOT_INTERFACE("hdc", ABC_HDC)
	SLOT_INTERFACE("hdd", ABC850)
	SLOT_INTERFACE("dos", ABC_DOS)
	SLOT_INTERFACE("fd2", ABC_FD2)
	SLOT_INTERFACE("sio", ABC_SIO)
	SLOT_INTERFACE("slow", LUXOR_55_10828)
	SLOT_INTERFACE("fast", LUXOR_55_21046)
	SLOT_INTERFACE("uni800", ABC_UNI800)
	SLOT_INTERFACE("slutprov", ABC_SLUTPROV)
	SLOT_INTERFACE("turbo", TURBO_KONTROLLER)
	SLOT_INTERFACE("xebec", LUXOR_55_21056)
SLOT_INTERFACE_END
