// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    d002.c

    KC85 D002 Bus Driver emulation

***************************************************************************/

#include "emu.h"
#include "d002.h"

#include "ram.h"
#include "rom.h"
#include "d004.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static INPUT_PORTS_START( kc_d002 )
	// device ID is selected using 4 jumpers on the board
	// 0x00 and 0xf0 should not be used because they are
	// reserved for base system and FDC the others
	// 14 ID are theoretically usable.
	PORT_START("ID")
	PORT_DIPNAME( 0xf0, 0x10, "Device ID" )
	PORT_DIPSETTING( 0x00, "0x00" )     // reserved for base system
	PORT_DIPSETTING( 0x10, "0x10" )
	PORT_DIPSETTING( 0x20, "0x20" )
	PORT_DIPSETTING( 0x30, "0x30" )
	PORT_DIPSETTING( 0x40, "0x40" )
	PORT_DIPSETTING( 0x50, "0x50" )
	PORT_DIPSETTING( 0x60, "0x60" )
	PORT_DIPSETTING( 0x70, "0x70" )
	PORT_DIPSETTING( 0x80, "0x80" )
	PORT_DIPSETTING( 0x90, "0x90" )
	PORT_DIPSETTING( 0xA0, "0xA0" )
	PORT_DIPSETTING( 0xB0, "0xB0" )
	PORT_DIPSETTING( 0xC0, "0xC0" )
	PORT_DIPSETTING( 0xD0, "0xD0" )
	PORT_DIPSETTING( 0xE0, "0xE0" )
	PORT_DIPSETTING( 0xF0, "0xF0" )     // reserved for FDC D004
INPUT_PORTS_END

// defined in drivers/kc.c
// FIXME: if src/devices depends on src/mame, you're doing it wrong!
void kc85_cart(device_slot_interface &device);
void kc85_exp(device_slot_interface &device);

WRITE_LINE_MEMBER(kc_d002_device::out_irq_w)
{
	m_slot->m_out_irq_cb(state);
}

WRITE_LINE_MEMBER(kc_d002_device::out_nmi_w)
{
	m_slot->m_out_nmi_cb(state);
}

WRITE_LINE_MEMBER(kc_d002_device::out_halt_w)
{
	m_slot->m_out_halt_cb(state);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KC_D002, kc_d002_device, "kc_d002", "D002 Bus Driver")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_d002_device - constructor
//-------------------------------------------------

kc_d002_device::kc_d002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KC_D002, tag, owner, clock)
	, device_kcexp_interface(mconfig, *this)
	, m_slot(nullptr)
	, m_expansions(*this, { "m0", "m4", "m8", "mc", "exp" })
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_d002_device::device_start()
{
	m_slot = dynamic_cast<kcexp_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d002_device::device_reset()
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void kc_d002_device::device_add_mconfig(machine_config &config)
{
	KCCART_SLOT(config, m_expansions[0], kc85_cart, nullptr);
	m_expansions[0]->set_next_slot("m4");
	m_expansions[0]->irq().set(FUNC(kc_d002_device::out_irq_w));
	m_expansions[0]->nmi().set(FUNC(kc_d002_device::out_nmi_w));
	m_expansions[0]->halt().set(FUNC(kc_d002_device::out_halt_w));
	KCCART_SLOT(config, m_expansions[1], kc85_cart, nullptr);
	m_expansions[1]->set_next_slot("m8");
	m_expansions[1]->irq().set(FUNC(kc_d002_device::out_irq_w));
	m_expansions[1]->nmi().set(FUNC(kc_d002_device::out_nmi_w));
	m_expansions[1]->halt().set(FUNC(kc_d002_device::out_halt_w));
	KCCART_SLOT(config, m_expansions[2], kc85_cart, nullptr);
	m_expansions[2]->set_next_slot("mc");
	m_expansions[2]->irq().set(FUNC(kc_d002_device::out_irq_w));
	m_expansions[2]->nmi().set(FUNC(kc_d002_device::out_nmi_w));
	m_expansions[2]->halt().set(FUNC(kc_d002_device::out_halt_w));
	KCCART_SLOT(config, m_expansions[3], kc85_cart, nullptr);
	m_expansions[3]->set_next_slot("exp");
	m_expansions[3]->irq().set(FUNC(kc_d002_device::out_irq_w));
	m_expansions[3]->nmi().set(FUNC(kc_d002_device::out_nmi_w));
	m_expansions[3]->halt().set(FUNC(kc_d002_device::out_halt_w));

	// expansion interface
	KCCART_SLOT(config, m_expansions[4], kc85_exp, nullptr);
	m_expansions[4]->set_next_slot(nullptr);
	m_expansions[4]->irq().set(FUNC(kc_d002_device::out_irq_w));
	m_expansions[4]->nmi().set(FUNC(kc_d002_device::out_nmi_w));
	m_expansions[4]->halt().set(FUNC(kc_d002_device::out_halt_w));
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor kc_d002_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kc_d002 );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void kc_d002_device::read(offs_t offset, uint8_t &data)
{
	for (auto & elem : m_expansions)
		elem->read(offset, data);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void kc_d002_device::write(offs_t offset, uint8_t data)
{
	for (auto & elem : m_expansions)
		elem->write(offset, data);
}

//-------------------------------------------------
//  IO read
//-------------------------------------------------

void kc_d002_device::io_read(offs_t offset, uint8_t &data)
{
	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if ((slot_id & 0xf0) == ioport("ID")->read() && !(slot_id & 0x03))
			data = m_expansions[(slot_id>>2) & 3]->module_id_r();
		else
			m_expansions[4]->io_read(offset, data);
	}
	else
	{
		for (auto & elem : m_expansions)
			elem->io_read(offset, data);
	}
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void kc_d002_device::io_write(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if ((slot_id & 0xf0) == ioport("ID")->read() && !(slot_id & 0x03))
			m_expansions[(slot_id>>2) & 3]->control_w(data);
		else
			m_expansions[4]->io_write(offset, data);
	}
	else
	{
		for (auto & elem : m_expansions)
			elem->io_write(offset, data);
	}

}

/*-------------------------------------------------
   MEI line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kc_d002_device::mei_w )
{
	m_expansions[0]->mei_w(state);
}
