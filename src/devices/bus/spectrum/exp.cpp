// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        ZX Spectrum Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_EXPANSION_SLOT, spectrum_expansion_slot_device, "spectrum_expansion_slot", "ZX Spectrum Expansion port")


//**************************************************************************
//  DEVICE SPECTRUM_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_spectrum_expansion_interface - constructor
//-------------------------------------------------

device_spectrum_expansion_interface::device_spectrum_expansion_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<spectrum_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_spectrum_expansion_interface - destructor
//-------------------------------------------------

device_spectrum_expansion_interface::~device_spectrum_expansion_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_expansion_slot_device - constructor
//-------------------------------------------------

spectrum_expansion_slot_device::spectrum_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SPECTRUM_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
{
}


//-------------------------------------------------
//  expansion_slot_device - destructor
//-------------------------------------------------

spectrum_expansion_slot_device::~spectrum_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_spectrum_expansion_interface *>(get_card_device());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}

//-------------------------------------------------
//  port_fe_r
//-------------------------------------------------

READ8_MEMBER(spectrum_expansion_slot_device::port_fe_r)
{
	if (m_card)
		return m_card->port_fe_r(space, offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  romcs
//-------------------------------------------------

READ_LINE_MEMBER(spectrum_expansion_slot_device::romcs)
{
	if (m_card)
		return m_card->romcs();
	else
		return 0;
}

//-------------------------------------------------
//  mreq_r
//-------------------------------------------------

READ8_MEMBER(spectrum_expansion_slot_device::mreq_r)
{
	if (m_card)
		return m_card->mreq_r(space, offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  mreq_w
//-------------------------------------------------

WRITE8_MEMBER(spectrum_expansion_slot_device::mreq_w)
{
	if (m_card)
		m_card->mreq_w(space, offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( spectrum_expansion_devices )
//-------------------------------------------------


// slot devices
#include "intf1.h"
#include "intf2.h"
#include "fuller.h"
#include "kempjoy.h"
#include "melodik.h"
#include "mikroplus.h"
#include "plus2test.h"
#include "protek.h"
#include "uslot.h"
#include "usource.h"
#include "uspeech.h"


SLOT_INTERFACE_START( spectrum_expansion_devices )
	SLOT_INTERFACE("intf1", SPECTRUM_INTF1)
	SLOT_INTERFACE("intf2", SPECTRUM_INTF2)
	SLOT_INTERFACE("fuller", SPECTRUM_FULLER)
	SLOT_INTERFACE("kempjoy", SPECTRUM_KEMPJOY)
	SLOT_INTERFACE("melodik", SPECTRUM_MELODIK)
	SLOT_INTERFACE("mikroplus", SPECTRUM_MIKROPLUS)
	SLOT_INTERFACE("protek", SPECTRUM_PROTEK)
	SLOT_INTERFACE("uslot", SPECTRUM_USLOT)
	SLOT_INTERFACE("usource", SPECTRUM_USOURCE)
	SLOT_INTERFACE("uspeech", SPECTRUM_USPEECH)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( spec128_expansion_devices )
	SLOT_INTERFACE("intf1", SPECTRUM_INTF1)
	SLOT_INTERFACE("intf2", SPECTRUM_INTF2)
	SLOT_INTERFACE("kempjoy", SPECTRUM_KEMPJOY)
	SLOT_INTERFACE("mikroplus", SPECTRUM_MIKROPLUS)
	SLOT_INTERFACE("plus2test", SPECTRUM_PLUS2TEST)
	SLOT_INTERFACE("protek", SPECTRUM_PROTEK)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( specpls3_expansion_devices )
SLOT_INTERFACE_END

