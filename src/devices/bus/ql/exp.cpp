// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL expansion port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QL_EXPANSION_SLOT, ql_expansion_slot_device, "ql_expansion_slot", "QL expansion port")



//**************************************************************************
//  DEVICE QL_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ql_expansion_card_interface - constructor
//-------------------------------------------------

device_ql_expansion_card_interface::device_ql_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_slot(dynamic_cast<ql_expansion_slot_device *>(device.owner())),
	m_romoeh(0)
{
}


void device_ql_expansion_card_interface::interface_post_start()
{
	device_slot_card_interface::interface_post_start();

	device().save_item(NAME(m_romoeh));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_expansion_slot_device - constructor
//-------------------------------------------------

ql_expansion_slot_device::ql_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QL_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_write_ipl0l(*this),
	m_write_ipl1l(*this),
	m_write_berrl(*this),
	m_write_extintl(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_expansion_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_ql_expansion_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_ql_expansion_card_interface\n", card->tag(), card->name());
}

void ql_expansion_slot_device::device_resolve_objects()
{
	// resolve callbacks
	m_write_ipl0l.resolve_safe();
	m_write_ipl1l.resolve_safe();
	m_write_berrl.resolve_safe();
	m_write_extintl.resolve_safe();

	m_card = dynamic_cast<device_ql_expansion_card_interface *>(get_card_device());
}

void ql_expansion_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !m_card)
		throw emu_fatalerror("ql_expansion_slot_device: card device %s (%s) does not implement device_ql_expansion_card_interface\n", card->tag(), card->name());
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_expansion_cards )
//-------------------------------------------------

// slot devices
#include "cst_qdisc.h"
#include "cst_q_plus4.h"
#include "cumana_fdi.h"
#include "kempston_di.h"
#include "miracle_gold_card.h"
#include "mp_fdi.h"
#include "opd_basic_master.h"
#include "pcml_qdisk.h"
#include "qubide.h"
#include "sandy_superdisk.h"
#include "sandy_superqboard.h"
#include "trumpcard.h"

void ql_expansion_cards(device_slot_interface &device)
{
	device.option_add("qdisc", CST_QL_DISC_INTERFACE);
	device.option_add("qplus4", CST_Q_PLUS4);
	device.option_add("cumanafdi", CUMANA_FLOPPY_DISK_INTERFACE);
	device.option_add("kdi", KEMPSTON_DISK_INTERFACE);
	device.option_add("mpfdi", MICRO_PERIPHERALS_FLOPPY_DISK_INTERFACE);
	device.option_add("gold", MIRACLE_GOLD_CARD);
	device.option_add("pcmlqdi", PCML_Q_DISK_INTERFACE);
	device.option_add("qubide", QUBIDE);
	device.option_add("sdisk", SANDY_SUPER_DISK);
	device.option_add("sqboard", SANDY_SUPERQBOARD);
	device.option_add("sqboard512k", SANDY_SUPERQBOARD_512K);
	device.option_add("sqmouse", SANDY_SUPERQMOUSE);
	device.option_add("sqmouse512k", SANDY_SUPERQMOUSE_512K);
	device.option_add("opdbasic", OPD_BASIC_MASTER);
	device.option_add("trump", QL_TRUMP_CARD);
	device.option_add("trump256k", QL_TRUMP_CARD_256K);
	device.option_add("trump512k", QL_TRUMP_CARD_512K);
	device.option_add("trump768k", QL_TRUMP_CARD_768K);
}
