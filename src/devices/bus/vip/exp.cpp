// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA Cosmac VIP Expansion Interface emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIP_EXPANSION_SLOT = &device_creator<vip_expansion_slot_device>;



//**************************************************************************
//  DEVICE VIP_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vip_expansion_card_interface - constructor
//-------------------------------------------------

device_vip_expansion_card_interface::device_vip_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<vip_expansion_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vip_expansion_slot_device - constructor
//-------------------------------------------------

vip_expansion_slot_device::vip_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VIP_EXPANSION_SLOT, "VIP expansion port", tag, owner, clock, "vip_expansion_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_int(*this),
	m_write_dma_out(*this),
	m_write_dma_in(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vip_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_vip_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_dma_out.resolve_safe();
	m_write_dma_in.resolve_safe();
}


//-------------------------------------------------
//  program_r - program read
//-------------------------------------------------

UINT8 vip_expansion_slot_device::program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh)
{
	UINT8 data = 0;

	if (m_card != NULL)
	{
		data = m_card->vip_program_r(space, offset, cs, cdef, minh);
	}

	return data;
}


//-------------------------------------------------
//  program_w - program write
//-------------------------------------------------

void vip_expansion_slot_device::program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh)
{
	if (m_card != NULL)
	{
		m_card->vip_program_w(space, offset, data, cdef, minh);
	}
}


//-------------------------------------------------
//  io_r - io read
//-------------------------------------------------

UINT8 vip_expansion_slot_device::io_r(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (m_card != NULL)
	{
		data = m_card->vip_io_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  io_w - io write
//-------------------------------------------------

void vip_expansion_slot_device::io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->vip_io_w(space, offset, data);
	}
}


//-------------------------------------------------
//  dma_r - dma read
//-------------------------------------------------

UINT8 vip_expansion_slot_device::dma_r(address_space &space, offs_t offset)
{
	UINT8 data = 0;

	if (m_card != NULL)
	{
		data = m_card->vip_dma_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  dma_w - dma write
//-------------------------------------------------

void vip_expansion_slot_device::dma_w(address_space &space, offs_t offset, UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->vip_dma_w(space, offset, data);
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 vip_expansion_slot_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool value = false;

	if (m_card != NULL)
	{
		value = m_card->vip_screen_update(screen, bitmap, cliprect);
	}

	return value;
}

READ_LINE_MEMBER( vip_expansion_slot_device::ef1_r ) { int state = CLEAR_LINE; if (m_card != NULL) state = m_card->vip_ef1_r(); return state; }
READ_LINE_MEMBER( vip_expansion_slot_device::ef3_r ) { int state = CLEAR_LINE; if (m_card != NULL) state = m_card->vip_ef3_r(); return state; }
READ_LINE_MEMBER( vip_expansion_slot_device::ef4_r ) { int state = CLEAR_LINE; if (m_card != NULL) state = m_card->vip_ef4_r(); return state; }
void vip_expansion_slot_device::sc_w(int data) { if (m_card != NULL) m_card->vip_sc_w(data); }
WRITE_LINE_MEMBER( vip_expansion_slot_device::q_w ) { if (m_card != NULL) m_card->vip_q_w(state); }
WRITE_LINE_MEMBER( vip_expansion_slot_device::run_w ) { if (m_card != NULL) m_card->vip_run_w(state); }



//-------------------------------------------------
//  SLOT_INTERFACE vip_expansion_cards )
//-------------------------------------------------

// slot devices
#include "vp550.h"
#include "vp570.h"
#include "vp575.h"
#include "vp585.h"
#include "vp590.h"
#include "vp595.h"
#include "vp700.h"

SLOT_INTERFACE_START( vip_expansion_cards )
	SLOT_INTERFACE("super", VP550)
	//SLOT_INTERFACE("eprom", VP560)
	//SLOT_INTERFACE("eprommer", VP565)
	SLOT_INTERFACE("ram", VP570)
	SLOT_INTERFACE("exp", VP575)
	//SLOT_INTERFACE("exp2", VP576_EXP)
	SLOT_INTERFACE("keypad", VP585)
	SLOT_INTERFACE("color", VP590)
	SLOT_INTERFACE("simple", VP595)
	SLOT_INTERFACE("basic", VP700)
SLOT_INTERFACE_END
