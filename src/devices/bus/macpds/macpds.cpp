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
	m_macpds(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_slot_device::device_start()
{
	device_macpds_card_interface *dev = dynamic_cast<device_macpds_card_interface *>(get_card_device());

	if (dev) dev->set_macpds_and_slot(m_macpds, this);
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
	m_maincpu(*this, finder_base::DUMMY_TAG)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_device::device_start()
{
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

template<typename R, typename W> void macpds_device::install_device(offs_t start, offs_t end, R rhandler, W whandler, uint32_t mask)
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

template void macpds_device::install_device<read8_delegate,     write8_delegate    >(offs_t start, offs_t end, read8_delegate rhandler,     write8_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read8s_delegate,    write8s_delegate   >(offs_t start, offs_t end, read8s_delegate rhandler,    write8s_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read8sm_delegate,   write8sm_delegate  >(offs_t start, offs_t end, read8sm_delegate rhandler,   write8sm_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read8smo_delegate,  write8smo_delegate >(offs_t start, offs_t end, read8smo_delegate rhandler,  write8smo_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read16_delegate,    write16_delegate   >(offs_t start, offs_t end, read16_delegate rhandler,    write16_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read16s_delegate,   write16s_delegate  >(offs_t start, offs_t end, read16s_delegate rhandler,   write16s_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read16sm_delegate,  write16sm_delegate >(offs_t start, offs_t end, read16sm_delegate rhandler,  write16sm_delegate whandler, uint32_t mask);
template void macpds_device::install_device<read16smo_delegate, write16smo_delegate>(offs_t start, offs_t end, read16smo_delegate rhandler, write16smo_delegate whandler, uint32_t mask);

void macpds_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
//  printf("install_bank: %s @ %x->%x\n", tag, start, end);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(start, end, data);
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
	: device_interface(device, "macpds"),
		m_macpds(nullptr), m_macpds_slot(nullptr), m_next(nullptr)
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
	m_macpds->add_macpds_card(this);
}

void device_macpds_card_interface::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_macpds->install_bank(start, end, data);
}

void device_macpds_card_interface::install_rom(device_t *dev, const char *romregion, uint32_t addr)
{
	uint8_t *rom = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->base();
	uint32_t romlen = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->bytes();

	m_macpds->install_bank(addr, addr+romlen-1, rom);
}
