// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macpds.c - Mac 68000 PDS implementation (SE, Portable)

  by R. Belmont

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "macpds.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type MACPDS_SLOT = &device_creator<macpds_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_slot_device - constructor
//-------------------------------------------------
macpds_slot_device::macpds_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, MACPDS_SLOT, "Mac 68000 Processor-Direct Slot", tag, owner, clock, "macpds_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}

macpds_slot_device::macpds_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this)
{
}

void macpds_slot_device::static_set_macpds_slot(device_t &device, const char *tag, const char *slottag)
{
	macpds_slot_device &macpds_card = dynamic_cast<macpds_slot_device &>(device);
	macpds_card.m_macpds_tag = tag;
	macpds_card.m_macpds_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_slot_device::device_start()
{
	device_macpds_card_interface *dev = dynamic_cast<device_macpds_card_interface *>(get_card_device());

	if (dev) device_macpds_card_interface::static_set_macpds_tag(*dev, m_macpds_tag, m_macpds_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type MACPDS = &device_creator<macpds_device>;

void macpds_device::static_set_cputag(device_t &device, const char *tag)
{
	macpds_device &macpds = downcast<macpds_device &>(device);
	macpds.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_device - constructor
//-------------------------------------------------

macpds_device::macpds_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, MACPDS, "MACPDS", tag, owner, clock, "macpds", __FILE__)
{
}

macpds_device::macpds_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void macpds_device::device_reset()
{
}

void macpds_device::add_macpds_card(device_macpds_card_interface *card)
{
	m_device_list.append(*card);
}

void macpds_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

void macpds_device::install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

void macpds_device::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
//  printf("install_bank: %s @ %x->%x mask %x mirror %x\n", tag, start, end, mask, mirror);
	m_maincpu = machine().device<cpu_device>(m_cputag);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_bank(start, end, mask, mirror, tag );
	machine().root_device().membank(tag)->set_base(data);
}

void macpds_device::set_irq_line(int line, int state)
{
	m_maincpu->set_input_line(line, state);
}

//**************************************************************************
//  DEVICE CONFIG MACPDS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE MACPDS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_macpds_card_interface - constructor
//-------------------------------------------------

device_macpds_card_interface::device_macpds_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_macpds(NULL),
		m_macpds_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_macpds_card_interface - destructor
//-------------------------------------------------

device_macpds_card_interface::~device_macpds_card_interface()
{
}

void device_macpds_card_interface::static_set_macpds_tag(device_t &device, const char *tag, const char *slottag)
{
	device_macpds_card_interface &macpds_card = dynamic_cast<device_macpds_card_interface &>(device);
	macpds_card.m_macpds_tag = tag;
	macpds_card.m_macpds_slottag = slottag;
}

void device_macpds_card_interface::set_macpds_device()
{
	m_macpds = dynamic_cast<macpds_device *>(device().machine().device(m_macpds_tag));
	m_macpds->add_macpds_card(this);
}

void device_macpds_card_interface::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
	char bank[256];

	// append an underscore and the slot name to the bank so it's guaranteed unique
	strcpy(bank, tag);
	strcat(bank, "_");
	strcat(bank, m_macpds_slottag);

	m_macpds->install_bank(start, end, mask, mirror, bank, data);
}

void device_macpds_card_interface::install_rom(device_t *dev, const char *romregion, UINT32 addr)
{
	UINT8 *rom = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->base();
	UINT32 romlen = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->bytes();
	char bankname[128];
	sprintf(bankname, "rom_%x", addr);

	m_macpds->install_bank(addr, addr+romlen-1, 0, 0, bankname, rom);
}
