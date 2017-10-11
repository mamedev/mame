// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS

    77-pin slot

***************************************************************************/

#include "emu.h"
#include "nasbus.h"


//**************************************************************************
//  NASBUS SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(NASBUS_SLOT, nasbus_slot_device, "nasbus_slot", "NASBUS Slot")

//-------------------------------------------------
//  nasbus_slot_device - constructor
//-------------------------------------------------

nasbus_slot_device::nasbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nasbus_slot_device(mconfig, NASBUS_SLOT, tag, owner, clock)
{
}

nasbus_slot_device::nasbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_nasbus_tag(nullptr)
{
}

void nasbus_slot_device::set_nasbus_slot(device_t &device, device_t *owner, const char *nasbus_tag)
{
	nasbus_slot_device &nasbus_card = dynamic_cast<nasbus_slot_device &>(device);
	nasbus_card.m_owner = owner;
	nasbus_card.m_nasbus_tag = nasbus_tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nasbus_slot_device::device_start()
{
	device_nasbus_card_interface *dev = dynamic_cast<device_nasbus_card_interface *>(get_card_device());

	if (dev)
	{
		nasbus_device *m_nasbus = downcast<nasbus_device *>(m_owner->subdevice(m_nasbus_tag));
		m_nasbus->add_card(dev);
	}
}


//**************************************************************************
//  NASBUS DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(NASBUS, nasbus_device, "nasbus", "NASBUS Backplane")

//-------------------------------------------------
//  nasbus_device - constructor
//-------------------------------------------------

nasbus_device::nasbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NASBUS, tag, owner, clock),
	m_program(nullptr),
	m_io(nullptr),
	m_ram_disable_handler(*this)
{
}

//-------------------------------------------------
//  nasbus_device - destructor
//-------------------------------------------------

nasbus_device::~nasbus_device()
{
	m_dev.detach_all();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nasbus_device::device_start()
{
	// resolve callbacks
	m_ram_disable_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nasbus_device::device_reset()
{
}

//-------------------------------------------------
//  add_card - add new card to our bus
//-------------------------------------------------

void nasbus_device::add_card(device_nasbus_card_interface *card)
{
	card->set_nasbus_device(this);
	m_dev.append(*card);
}

//-------------------------------------------------
//  set_program_space - set address space we are attached to
//-------------------------------------------------

void nasbus_device::set_program_space(address_space *program)
{
	m_program = program;
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void nasbus_device::set_io_space(address_space *io)
{
	m_io = io;
}

// callbacks from slot device to the host
WRITE_LINE_MEMBER( nasbus_device::ram_disable_w ) { m_ram_disable_handler(state); }


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nasbus_card_interface - constructor
//-------------------------------------------------

device_nasbus_card_interface::device_nasbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_nasbus(nullptr),
	m_next(nullptr)
{
}

//-------------------------------------------------
//  ~device_nasbus_card_interface - destructor
//-------------------------------------------------

device_nasbus_card_interface::~device_nasbus_card_interface()
{
}

void device_nasbus_card_interface::set_nasbus_device(nasbus_device *nasbus)
{
	m_nasbus = nasbus;
}
