// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Bus Extension emulation

    Part No. 100,031

**********************************************************************/

#include "emu.h"
#include "bus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_BUS_SLOT, acorn_bus_slot_device, "acorn_bus_slot", "Acorn Bus Eurocard slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_bus_slot_device - constructor
//-------------------------------------------------
acorn_bus_slot_device::acorn_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_BUS_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_validity_check - device-specific checks
//-------------------------------------------------

void acorn_bus_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_acorn_bus_interface *>(card))
		osd_printf_error("acorn_bus_slot_device: card device %s (%s) does not implement device_acorn_bus_interface\n", card->tag(), card->name());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_bus_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card)
	{
		device_acorn_bus_interface *const intf(dynamic_cast<device_acorn_bus_interface *>(card));
		if (!intf)
			throw emu_fatalerror("acorn_bus_slot_device: card device %s (%s) does not implement device_acorn_bus_interface\n", card->tag(), card->name());
		intf->set_acorn_bus(*m_bus);
	}

	// tell acorn bus that there is one slot with the specified tag
	m_bus->add_slot(*this);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_BUS, acorn_bus_device, "acorn_bus", "Acorn Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_bus_device - constructor
//-------------------------------------------------

acorn_bus_device::acorn_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_BUS, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
{
}


void acorn_bus_device::add_slot(acorn_bus_slot_device &slot)
{
	m_slot_list.push_front(&slot);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_bus_device::device_start()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_bus_device::device_reset()
{
}

// interrupt request from acorn_bus card
WRITE_LINE_MEMBER(acorn_bus_device::irq_w) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER(acorn_bus_device::nmi_w) { m_out_nmi_cb(state); }



//**************************************************************************
//  DEVICE CONFIG ACORN BUS INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE ACORN BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_acorn_bus_interface - constructor
//-------------------------------------------------

device_acorn_bus_interface::device_acorn_bus_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_bus(nullptr)
{
}


//-------------------------------------------------
//  ~device_acorn_bus_interface - destructor
//-------------------------------------------------

device_acorn_bus_interface::~device_acorn_bus_interface()
{
}

void device_acorn_bus_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  SLOT_INTERFACE( acorn_bus_devices )
//-------------------------------------------------


// slot devices
#include "system/32k.h"
#include "system/8k.h"
#include "system/cass.h"
#include "system/econet.h"
#include "system/fdc.h"
#include "system/vdu40.h"
#include "system/vdu80.h"
#include "system/vib.h"

#include "atom/sid.h"
#include "atom/discpack.h"
#include "atom/econet.h"

#include "cms/4080term.h"
#include "cms/fdc.h"
#include "cms/hires.h"

void acorn_bus_devices(device_slot_interface &device)
{
	device.option_add("32k", ACORN_32K);           /* 32K Dynamic RAM Board */
	device.option_add("8k", ACORN_8K);             /* 8K Static RAM Board */
	//device.option_add("8kcmos", ACORN_8KCMOS);     /* 8K CMOS RAM Board */
	//device.option_add("analog", ACORN_ANALOG);     /* Analog Interface Board */
	device.option_add("cass", ACORN_CASS);         /* Cassette Interface Board */
	device.option_add("econet", ACORN_ECONET);     /* Econet Board */
	device.option_add("fdc", ACORN_FDC);           /* Floppy Disc Controller Board */
	device.option_add("vdu40", ACORN_VDU40);       /* 40 Column VDU Board */
	device.option_add("vdu80", ACORN_VDU80);       /* 80x25 VDU Board */
	device.option_add("vib", ACORN_VIB);           /* Versatile Interface Board */
}

void atom_bus_devices(device_slot_interface &device)
{
	device.option_add("32k", ACORN_32K);           /* 32K Dynamic RAM Board */
	device.option_add("sid", ATOM_SID);            /* AtomSID */
	device.option_add("discpack", ATOM_DISCPACK);  /* Acorn Atom Disc Pack */
	device.option_add("econet", ATOM_ECONET);      /* Econet Board */
}

void cms_bus_devices(device_slot_interface &device)
{
	device.option_add("4080term", CMS_4080TERM);  /* CMS 40/80 Video Terminal Card */
	device.option_add("fdc", CMS_FDC);            /* CMS Floppy Disc Controller */
	device.option_add("hires", CMS_HIRES);        /* CMS High Resolution Colour Graphics Card */
}
