// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Glenside IDE Adapter

***************************************************************************/

#include "emu.h"
#include "coco_ide.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_IDE, coco_ide_device, "coco_ide", "Glenside IDE Adapter")


//-------------------------------------------------
//  INPUT_PORTS( ide )
//-------------------------------------------------

INPUT_PORTS_START(ide)
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x01, 0x00, "J2 Base Address")
	PORT_CONFSETTING(0x00, "$FF5x")
	PORT_CONFSETTING(0x01, "$FF7x")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_ide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ide);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_ide_device - constructor
//-------------------------------------------------

coco_ide_device::coco_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_IDE, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_ata(*this, "ata")
	, m_slot(*this, "slot")
	, m_jumpers(*this, "JUMPERS")
	, m_latch(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_ide_device::device_start()
{
	save_item(NAME(m_latch));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_ide_device::device_reset()
{
	if (m_jumpers->read() == 0x01)
		install_readwrite_handler(0xff70, 0xff78, read8sm_delegate(*this, FUNC(coco_ide_device::ide_read)), write8sm_delegate(*this, FUNC(coco_ide_device::ide_write)));
}


//-------------------------------------------------
//  coco_cart
//-------------------------------------------------

static void coco_cart(device_slot_interface &device)
{
	coco_cart_add_basic_devices(device);
	coco_cart_add_fdcs(device);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void coco_ide_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);

	COCOCART_SLOT(config, m_slot, DERIVED_CLOCK(1, 1), coco_cart, "pak");
	m_slot->cart_callback().set([this](int state) { set_line_value(line::CART, state); });
	m_slot->nmi_callback().set([this](int state) { set_line_value(line::NMI, state); });
	m_slot->halt_callback().set([this](int state) { set_line_value(line::HALT, state); });
}


//-------------------------------------------------
//  ide_read
//-------------------------------------------------

u8 coco_ide_device::ide_read(offs_t offset)
{
	u8 result = 0x00;

	switch (offset)
	{
	case 0:
	{
		u16 temp = m_ata->cs0_r(offset);
		m_latch = temp >> 8;
		result = temp & 0xff;
		break;
	}
	case 1: case 2: case 3: case 4: case 5: case 6: case 7:
		result = m_ata->cs0_r(offset, 0xff);
		break;

	case 8:
		result = m_latch;
		break;

	default:
		logerror("ide_read: %02x %04x\n", offset, result);
		break;
	}

	return result;
}


//-------------------------------------------------
//  ide_write
//-------------------------------------------------

void coco_ide_device::ide_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
	{
		u16 temp = (m_latch << 8) | data;
		m_ata->cs0_w(offset, temp);
		break;
	}
	case 1: case 2: case 3: case 4: case 5: case 6: case 7:
		m_ata->cs0_w(offset, data, 0xff);
		break;

	case 8:
		m_latch = data;
		break;

	default:
		logerror("ide_write: %02x %04x\n", offset, data);
		break;
	}
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void coco_ide_device::set_sound_enable(bool sound_enable)
{
	m_slot->set_line_value(line::SOUND_ENABLE, sound_enable ? line_value::ASSERT : line_value::CLEAR);
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *coco_ide_device::get_cart_base()
{
	return m_slot->get_cart_base();
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 coco_ide_device::get_cart_size()
{
	return m_slot->get_cart_size();
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_ide_device::cts_read(offs_t offset)
{
	return m_slot->cts_read(offset);
}

//-------------------------------------------------
//  cts_write
//-------------------------------------------------

void coco_ide_device::cts_write(offs_t offset, u8 data)
{
	m_slot->cts_write(offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_ide_device::scs_read(offs_t offset)
{
	if (m_jumpers->read() == 0x00 && (offset & 0x10))
		return ide_read(offset & 0x0f);
	else
		return m_slot->scs_read(offset);
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_ide_device::scs_write(offs_t offset, u8 data)
{
	if (m_jumpers->read() == 0x00 && (offset & 0x10))
		ide_write(offset & 0x0f, data);
	else
		m_slot->scs_write(offset, data);
}


//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &coco_ide_device::cartridge_space()
{
	return device_cococart_interface::cartridge_space();
}
