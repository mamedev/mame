// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot

***************************************************************************/

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  EXPANSION SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_EXPANSION_SLOT, apricot_expansion_slot_device, "apricot_exp_slot", "Apricot Expansion Slot")

//-------------------------------------------------
//  apricot_expansion_slot_device - constructor
//-------------------------------------------------

apricot_expansion_slot_device::apricot_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	apricot_expansion_slot_device(mconfig, APRICOT_EXPANSION_SLOT, tag, owner, clock)
{
}

apricot_expansion_slot_device::apricot_expansion_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface<device_apricot_expansion_card_interface>(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_expansion_slot_device::device_start()
{
	device_apricot_expansion_card_interface *dev = get_card_device();

	if (dev)
	{
		apricot_expansion_bus_device *bus = downcast<apricot_expansion_bus_device *>(m_owner);
		bus->add_card(dev);
	}
}


//**************************************************************************
//  EXPANSION BUS DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_EXPANSION_BUS, apricot_expansion_bus_device, "apricot_exp_bus", "Apricot Expansion Bus")

//-------------------------------------------------
//  apricot_expansion_bus_device - constructor
//-------------------------------------------------

apricot_expansion_bus_device::apricot_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_EXPANSION_BUS, tag, owner, clock),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_program_iop(*this, finder_base::DUMMY_TAG, -1),
	m_io_iop(*this, finder_base::DUMMY_TAG, -1),
	m_dma1_handler(*this),
	m_dma2_handler(*this),
	m_ext1_handler(*this),
	m_ext2_handler(*this),
	m_int2_handler(*this),
	m_int3_handler(*this)
{
}

//-------------------------------------------------
//  apricot_expansion_bus_device - destructor
//-------------------------------------------------

apricot_expansion_bus_device::~apricot_expansion_bus_device()
{
	m_dev.detach_all();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_expansion_bus_device::device_start()
{
	// resolve callbacks
	m_dma1_handler.resolve_safe();
	m_dma2_handler.resolve_safe();
	m_ext1_handler.resolve_safe();
	m_ext2_handler.resolve_safe();
	m_int2_handler.resolve_safe();
	m_int3_handler.resolve_safe();
}

//-------------------------------------------------
//  add_card - add new card to our bus
//-------------------------------------------------

void apricot_expansion_bus_device::add_card(device_apricot_expansion_card_interface *card)
{
	card->set_bus_device(this);
	m_dev.append(*card);
}

// callbacks from slot device to the host
WRITE_LINE_MEMBER( apricot_expansion_bus_device::dma1_w ) { m_dma1_handler(state); }
WRITE_LINE_MEMBER( apricot_expansion_bus_device::dma2_w ) { m_dma2_handler(state); }
WRITE_LINE_MEMBER( apricot_expansion_bus_device::ext1_w ) { m_ext1_handler(state); }
WRITE_LINE_MEMBER( apricot_expansion_bus_device::ext2_w ) { m_ext2_handler(state); }
WRITE_LINE_MEMBER( apricot_expansion_bus_device::int2_w ) { m_int2_handler(state); }
WRITE_LINE_MEMBER( apricot_expansion_bus_device::int3_w ) { m_int3_handler(state); }

//-------------------------------------------------
//  install_ram - attach ram to cpu/iop
//-------------------------------------------------

void apricot_expansion_bus_device::install_ram(offs_t addrstart, offs_t addrend, void *baseptr)
{
	m_program->install_ram(addrstart, addrend, baseptr);

	if (m_program_iop)
		m_program_iop->install_ram(addrstart, addrend, baseptr);
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_apricot_expansion_card_interface - constructor
//-------------------------------------------------

device_apricot_expansion_card_interface::device_apricot_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "apricotexp"),
	m_bus(nullptr),
	m_next(nullptr)
{
}

//-------------------------------------------------
//  ~device_apricot_expansion_card_interface - destructor
//-------------------------------------------------

device_apricot_expansion_card_interface::~device_apricot_expansion_card_interface()
{
}

void device_apricot_expansion_card_interface::set_bus_device(apricot_expansion_bus_device *bus)
{
	m_bus = bus;
}
