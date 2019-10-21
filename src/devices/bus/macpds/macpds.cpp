// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macpds.c - Mac 68000 PDS implementation (SE, Portable)

  by R. Belmont

***************************************************************************/

#include "emu.h"
#include "macpds.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MACPDS_SLOT, macpds_slot_device, "macpds_slot", "Mac 68000 Processor-Direct Slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_slot_device - constructor
//-------------------------------------------------
macpds_slot_device::macpds_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	macpds_slot_device(mconfig, MACPDS_SLOT, tag, owner, clock)
{
}

macpds_slot_device::macpds_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_macpds_tag(nullptr),
	m_macpds_slottag(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_slot_device::device_start()
{
	device_macpds_card_interface *dev = dynamic_cast<device_macpds_card_interface *>(get_card_device());

	if (dev) dev->set_macpds_tag(m_macpds_tag, m_macpds_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MACPDS, macpds_device, "macpds", "Mac 68000 Processor-Direct Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_device - constructor
//-------------------------------------------------

macpds_device::macpds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	macpds_device(mconfig, MACPDS, tag, owner, clock)
{
}

macpds_device::macpds_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_maincpu(nullptr),
	m_cputag(nullptr)
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

void macpds_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, uint32_t mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

void macpds_device::install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

void macpds_device::install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data)
{
//  printf("install_bank: %s @ %x->%x\n", tag, start, end);
	m_maincpu = machine().device<cpu_device>(m_cputag);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_bank(start, end, 0, tag );
	machine().root_device().membank(siblingtag(tag).c_str())->set_base(data);
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
		m_macpds(nullptr),
		m_macpds_tag(nullptr), m_macpds_slottag(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_macpds_card_interface - destructor
//-------------------------------------------------

device_macpds_card_interface::~device_macpds_card_interface()
{
}

void device_macpds_card_interface::set_macpds_device()
{
	m_macpds = dynamic_cast<macpds_device *>(device().machine().device(m_macpds_tag));
	m_macpds->add_macpds_card(this);
}

void device_macpds_card_interface::install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data)
{
	char bank[256];

	// append an underscore and the slot name to the bank so it's guaranteed unique
	snprintf(bank, sizeof(bank), "%s_%s", tag, m_macpds_slottag);

	m_macpds->install_bank(start, end, bank, data);
}

void device_macpds_card_interface::install_rom(device_t *dev, const char *romregion, uint32_t addr)
{
	uint8_t *rom = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->base();
	uint32_t romlen = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->bytes();
	char bankname[128];
	sprintf(bankname, "rom_%x", addr);

	m_macpds->install_bank(addr, addr+romlen-1, bankname, rom);
}
