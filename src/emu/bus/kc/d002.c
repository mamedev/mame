// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    d002.c

    KC85 D002 Bus Driver emulation

***************************************************************************/

#include "emu.h"
#include "d002.h"

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
SLOT_INTERFACE_EXTERN(kc85_cart);
SLOT_INTERFACE_EXTERN(kc85_exp);

WRITE_LINE_MEMBER(kc_d002_device::out_irq_w)
{
	m_slot->m_out_irq_func(state);
}

WRITE_LINE_MEMBER(kc_d002_device::out_nmi_w)
{
	m_slot->m_out_nmi_func(state);
}

WRITE_LINE_MEMBER(kc_d002_device::out_halt_w)
{
	m_slot->m_out_halt_func(state);
}

static const kcexp_interface kc_d002_interface =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, kc_d002_device, out_irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, kc_d002_device, out_nmi_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, kc_d002_device, out_halt_w)
};

static MACHINE_CONFIG_FRAGMENT( kc_d002 )
	MCFG_KC85_CARTRIDGE_ADD("m0", "m4", kc_d002_interface, kc85_cart, NULL)
	MCFG_KC85_CARTRIDGE_ADD("m4", "m8", kc_d002_interface, kc85_cart, NULL)
	MCFG_KC85_CARTRIDGE_ADD("m8", "mc", kc_d002_interface, kc85_cart, NULL)
	MCFG_KC85_CARTRIDGE_ADD("mc", "exp", kc_d002_interface, kc85_cart, NULL)

	// expansion interface
	MCFG_KC85_EXPANSION_ADD("exp", NULL, kc_d002_interface, kc85_exp, NULL)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type KC_D002 = &device_creator<kc_d002_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_d002_device - constructor
//-------------------------------------------------

kc_d002_device::kc_d002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, KC_D002, "D002 Bus Driver", tag, owner, clock, "kc_d002", __FILE__),
		device_kcexp_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_d002_device::device_start()
{
	m_slot = dynamic_cast<kcexp_slot_device *>(owner());

	m_expansions[0] = downcast<kcexp_slot_device *>(subdevice("m0"));
	m_expansions[1] = downcast<kcexp_slot_device *>(subdevice("m4"));
	m_expansions[2] = downcast<kcexp_slot_device *>(subdevice("m8"));
	m_expansions[3] = downcast<kcexp_slot_device *>(subdevice("mc"));
	m_expansions[4] = downcast<kcexp_slot_device *>(subdevice("exp"));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d002_device::device_reset()
{
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor kc_d002_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kc_d002 );
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

void kc_d002_device::read(offs_t offset, UINT8 &data)
{
	for (int i=0; i<5; i++)
		m_expansions[i]->read(offset, data);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void kc_d002_device::write(offs_t offset, UINT8 data)
{
	for (int i=0; i<5; i++)
		m_expansions[i]->write(offset, data);
}

//-------------------------------------------------
//  IO read
//-------------------------------------------------

void kc_d002_device::io_read(offs_t offset, UINT8 &data)
{
	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

		if ((slot_id & 0xf0) == ioport("ID")->read() && !(slot_id & 0x03))
			data = m_expansions[(slot_id>>2) & 3]->module_id_r();
		else
			m_expansions[4]->io_read(offset, data);
	}
	else
	{
		for (int i=0; i<5; i++)
			m_expansions[i]->io_read(offset, data);
	}
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void kc_d002_device::io_write(offs_t offset, UINT8 data)
{
	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

		if ((slot_id & 0xf0) == ioport("ID")->read() && !(slot_id & 0x03))
			m_expansions[(slot_id>>2) & 3]->control_w(data);
		else
			m_expansions[4]->io_write(offset, data);
	}
	else
	{
		for (int i=0; i<5; i++)
			m_expansions[i]->io_write(offset, data);
	}

}

/*-------------------------------------------------
   MEI line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kc_d002_device::mei_w )
{
	m_expansions[0]->mei_w(state);
}
