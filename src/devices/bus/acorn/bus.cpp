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
	, device_single_card_slot_interface<device_acorn_bus_interface>(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
	, m_card(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_bus_slot_device::device_start()
{
	m_card = get_card_device();
	if (m_card)
		m_bus->add_card(*m_card);
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
	, device_memory_interface(mconfig, *this)
	, m_space_config("program", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor())
	, m_space(nullptr)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
	, m_cb1_handler(*this)
	, m_cb2_handler(*this)
	, m_blk0(0x00)
{
}

device_memory_interface::space_config_vector acorn_bus_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_space_config) };
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_bus_device::device_start()
{
	m_space = &space(AS_PROGRAM);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_bus_device::device_reset()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void acorn_bus_device::add_card(device_acorn_bus_interface &card)
{
	m_device_list.append(card);

	card.m_bus = this;
}


//-------------------------------------------------
//  address space
//-------------------------------------------------

uint8_t acorn_bus_device::read(offs_t offset)
{
	return m_space->read_byte(offset);
}

void acorn_bus_device::write(offs_t offset, uint8_t data)
{
	m_space->write_byte(offset, data);
}


//-------------------------------------------------
//  pb
//-------------------------------------------------

uint8_t acorn_bus_device::pb_r()
{
	uint8_t data = 0xff;

	device_acorn_bus_interface *entry = m_device_list.first();

	while (entry)
	{
		data &= entry->pb_r();
		entry = entry->next();
	}

	return data;
}


void acorn_bus_device::pb_w(uint8_t data)
{
	device_acorn_bus_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->pb_w(data);
		entry = entry->next();
	}
}


void acorn_bus_device::write_cb1(int state)
{
	device_acorn_bus_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->write_cb1(state);
		entry = entry->next();
	}
}


void acorn_bus_device::write_cb2(int state)
{
	device_acorn_bus_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->write_cb2(state);
		entry = entry->next();
	}
}


//-------------------------------------------------
//  device_acorn_bus_interface - constructor
//-------------------------------------------------

device_acorn_bus_interface::device_acorn_bus_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "acornbus")
	, m_bus(nullptr)
	, m_slot(dynamic_cast<acorn_bus_slot_device *>(device.owner()))
	, m_next(nullptr)
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

#include "atom/discpack.h"
#include "atom/econet.h"
#include "atom/gdos.h"
#include "atom/gdos2015.h"
#include "atom/mdcr.h"
#include "atom/sid.h"
#include "atom/speech.h"
#include "atom/switch.h"
#include "atom/tube.h"
#include "atom/vdu80.h"

#include "cms/4080term.h"
#include "cms/fdc.h"
#include "cms/hires.h"
#include "cms/ieee.h"

#include "cu/cubio.h"
#include "cu/cugraph.h"
#include "cu/teletext.h"

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
	device.option_add("discpack", ATOM_DISCPACK);  /* Atom Disc Pack */
	device.option_add("gdos", ATOM_GDOS);          /* Atom GDOS */
	device.option_add("gdos2015", ATOM_GDOS2015);  /* Atom GDOS-2015 */
	device.option_add("mdcr", ATOM_MDCR);          /* Atom MDCR */
	device.option_add("atomsid", ATOM_SID);        /* AtomSID */
	device.option_add("speech", ATOM_SPEECH);      /* Atom Speech Module */
	device.option_add("switch", ATOM_SWITCH);      /* EPROM Switch Card */
	device.option_add("atomtube", ATOM_TUBE);      /* Atom Tube Interface */
	device.option_add("vib", ACORN_VIB);           /* Versatile Interface Board */
	device.option_add("vdu40", ACORN_VDU40);       /* 40 Column VDU Board */
	device.option_add("vdu80", ATOM_VDU80);        /* 80 Column VDU Board */
}

void atom_pl8_devices(device_slot_interface &device)
{
	device.option_add("econet", ATOM_ECONET);      /* Econet Board */
}

void eurocube_bus_devices(device_slot_interface &device)
{
	device.option_add("cubio_r",  CU_CUBIO_R);     /* CUBIO w/ Race Controller */
	device.option_add("cugraphc", CU_GRAPHC);      /* CU-GRAPH (colour) */
	device.option_add("cugraphm", CU_GRAPHM);      /* CU-GRAPH (monochrome) */
	device.option_add("teletext", CU_TELETEXT);    /* Teletext Video Interface */
}

void cms_bus_devices(device_slot_interface &device)
{
	device.option_add("4080term", CMS_4080TERM);  /* CMS 40/80 Video Terminal Card */
	device.option_add("fdc", CMS_FDC);            /* CMS Floppy Disc Controller */
	device.option_add("hires", CMS_HIRES);        /* CMS High Resolution Colour Graphics Card */
	device.option_add("ieee", CMS_IEEE);          /* CMS IEEE Interface */
}
