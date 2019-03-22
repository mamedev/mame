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
//  device_validity_check - device-specific checks
//-------------------------------------------------

void spectrum_expansion_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_spectrum_expansion_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_spectrum_expansion_interface\n", card->tag(), card->name());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_expansion_slot_device::device_start()
{
	device_t *const card_device(get_card_device());
	m_card = dynamic_cast<device_spectrum_expansion_interface *>(card_device);
	if (card_device && !m_card)
		throw emu_fatalerror("spectrum_expansion_slot_device: card device %s (%s) does not implement device_spectrum_expansion_interface\n", card_device->tag(), card_device->name());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_expansion_slot_device::device_reset()
{
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
// fetch_r
//-------------------------------------------------

void spectrum_expansion_slot_device::opcode_fetch(offs_t offset)
{
	if (m_card)
		 m_card->opcode_fetch(offset);
}

//-------------------------------------------------
//  iorq_r
//-------------------------------------------------

uint8_t spectrum_expansion_slot_device::iorq_r(offs_t offset)
{
	if (m_card)
		return m_card->iorq_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  iorq_w
//-------------------------------------------------

void spectrum_expansion_slot_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->iorq_w(offset, data);
}

//-------------------------------------------------
//  mreq_r
//-------------------------------------------------

uint8_t spectrum_expansion_slot_device::mreq_r(offs_t offset)
{
	if (m_card)
		return m_card->mreq_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  mreq_w
//-------------------------------------------------

void spectrum_expansion_slot_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->mreq_w(offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( spectrum_expansion_devices )
//-------------------------------------------------


// slot devices
#include "beta.h"
//#include "disciple.h"
#include "intf1.h"
#include "intf2.h"
#include "fuller.h"
#include "kempjoy.h"
#include "melodik.h"
#include "mface.h"
#include "mikroplus.h"
#include "opus.h"
#include "plus2test.h"
//#include "plusd.h"
#include "protek.h"
#include "specdrum.h"
#include "uslot.h"
#include "usource.h"
#include "uspeech.h"


void spectrum_expansion_devices(device_slot_interface &device)
{
	device.option_add("beta128", SPECTRUM_BETA128);
	//device.option_add("disciple", SPECTRUM_DISCIPLE);
	device.option_add("intf1", SPECTRUM_INTF1);
	device.option_add("intf2", SPECTRUM_INTF2);
	device.option_add("fuller", SPECTRUM_FULLER);
	device.option_add("kempjoy", SPECTRUM_KEMPJOY);
	device.option_add("melodik", SPECTRUM_MELODIK);
	device.option_add("mface1", SPECTRUM_MFACE1);
	device.option_add("mface128", SPECTRUM_MFACE128);
	device.option_add("mikroplus", SPECTRUM_MIKROPLUS);
	device.option_add("mprint", SPECTRUM_MPRINT);
	device.option_add("opus", SPECTRUM_OPUS);
	//device.option_add("plusd", SPECTRUM_PLUSD);
	device.option_add("protek", SPECTRUM_PROTEK);
	device.option_add("specdrum", SPECTRUM_SPECDRUM);
	device.option_add("uslot", SPECTRUM_USLOT);
	device.option_add("usource", SPECTRUM_USOURCE);
	device.option_add("uspeech", SPECTRUM_USPEECH);
}

void spec128_expansion_devices(device_slot_interface &device)
{
	device.option_add("beta128", SPECTRUM_BETA128);
	//device.option_add("disciple", SPECTRUM_DISCIPLE);
	device.option_add("intf1", SPECTRUM_INTF1);
	device.option_add("intf2", SPECTRUM_INTF2);
	device.option_add("kempjoy", SPECTRUM_KEMPJOY);
	device.option_add("mface128", SPECTRUM_MFACE128);
	device.option_add("mikroplus", SPECTRUM_MIKROPLUS);
	device.option_add("mprint", SPECTRUM_MPRINT);
	device.option_add("opus", SPECTRUM_OPUS);
	device.option_add("plus2test", SPECTRUM_PLUS2TEST);
	device.option_add("protek", SPECTRUM_PROTEK);
	device.option_add("specdrum", SPECTRUM_SPECDRUM);
}

void specpls3_expansion_devices(device_slot_interface &device)
{
	device.option_add("mface3", SPECTRUM_MFACE3);
}

