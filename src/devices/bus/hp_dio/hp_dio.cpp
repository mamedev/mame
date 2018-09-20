// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

        HP DIO and DIO-II bus devices

***************************************************************************/

#include "emu.h"
#include "hp_dio.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO16_SLOT, dio16_slot_device, "dio16_slot", "16-bit DIO slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

dio16_slot_device::dio16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_slot_device(mconfig, DIO16_SLOT, tag, owner, clock)
{
}

dio16_slot_device::dio16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_dio(*this, finder_base::DUMMY_TAG)
{
}

void dio16_slot_device::device_resolve_objects()
{
	device_dio16_card_interface *const card(dynamic_cast<device_dio16_card_interface *>(get_card_device()));
	if (card && m_dio)
		card->set_diobus(*m_dio);
}

void dio16_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_dio16_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_dio16_card_interface\n", card->tag(), card->name());
}

void dio16_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_dio16_card_interface *>(card))
		throw emu_fatalerror("dio16_slot_device: card device %s (%s) does not implement device_dio16_card_interface\n", card->tag(), card->name());
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO32_SLOT, dio32_slot_device, "dio32_slot", "32-bit DIO-II slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio32_slot_device - constructor
//-------------------------------------------------
dio32_slot_device::dio32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_slot_device(mconfig, DIO32_SLOT, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio32_slot_device::device_start()
{
	dio16_slot_device::device_start();
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO16, dio16_device, "dio16", "16-bit DIO bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_device - constructor
//-------------------------------------------------

dio16_device::dio16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_device(mconfig, DIO16, tag, owner, clock)
{
}

dio16_device::dio16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_prgspace(nullptr),
	m_out_irq3_cb(*this),
	m_out_irq4_cb(*this),
	m_out_irq5_cb(*this),
	m_out_irq6_cb(*this)
{
	m_prgwidth = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_device::device_start()
{
	m_out_irq3_cb.resolve_safe();
	m_out_irq4_cb.resolve_safe();
	m_out_irq5_cb.resolve_safe();
	m_out_irq6_cb.resolve_safe();

	m_prgspace = &m_maincpu->space(AS_PROGRAM);
	m_prgwidth = m_maincpu->space_config(AS_PROGRAM)->data_width();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_device::device_reset()
{
}


void dio16_device::install_memory(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler)
{
	switch (m_prgwidth)
	{
		case 16:
			m_prgspace->install_readwrite_handler(start, end, rhandler, whandler);
			break;
		case 32:
			m_prgspace->install_readwrite_handler(start, end, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("DIO: Bus width %d not supported\n", m_prgwidth);
	}
}

void dio16_device::install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data)
{
	m_prgspace->install_readwrite_bank(start, end, 0, tag );
	machine().root_device().membank(m_prgspace->device().siblingtag(tag).c_str())->set_base(data);
}

void dio16_device::unmap_bank(offs_t start, offs_t end)
{
	m_prgspace->unmap_readwrite(start, end);
}

void dio16_device::install_rom(offs_t start, offs_t end, const char *tag, uint8_t *data)
{
	m_prgspace->install_read_bank(start, end, 0, tag);
	machine().root_device().membank(m_prgspace->device().siblingtag(tag).c_str())->set_base(data);
}

void dio16_device::unmap_rom(offs_t start, offs_t end)
{
	m_prgspace->unmap_read(start, end);
}


//**************************************************************************
//  DEVICE DIO16 CARD INTERFACE
//**************************************************************************

device_dio16_card_interface::device_dio16_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_dio_dev(nullptr), m_next(nullptr)
{
}

device_dio16_card_interface::~device_dio16_card_interface()
{
}

void device_dio16_card_interface::interface_pre_start()
{
	if (!m_dio_dev)
		throw emu_fatalerror("device_dio16_card_interface: DIO bus not configured\n");
}


//**************************************************************************
//  DIO32 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO32, dio32_device, "dio32", "32-bit DIO-II bus")

dio32_device::dio32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_device(mconfig, DIO32, tag, owner, clock)
{
}

void dio32_device::device_start()
{
	dio16_device::device_start();
}


//**************************************************************************
//  DEVICE DIO32 CARD INTERFACE
//**************************************************************************

device_dio32_card_interface::device_dio32_card_interface(const machine_config &mconfig, device_t &device) :
	device_dio16_card_interface(mconfig, device)
{
}

device_dio32_card_interface::~device_dio32_card_interface()
{
}

void device_dio32_card_interface::interface_pre_start()
{
	device_dio16_card_interface::interface_pre_start();

	if (m_dio_dev && !dynamic_cast<dio32_device *>(m_dio_dev))
		throw emu_fatalerror("device_dio32_card_interface: DIO32 device %s (%s) in DIO16 slot %s\n", device().tag(), device().name(), m_dio_dev->name());
}
