// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1802.c

    Hitachi MP-1802 floppy disk controller card for the MB-6890
    Hitachi MP-3550 floppy drive is attached

*********************************************************************/

#include "emu.h"
#include "bml3mp1802.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BML3BUS_MP1802 = &device_creator<bml3bus_mp1802_device>;

static SLOT_INTERFACE_START( mp1802_floppies )
	SLOT_INTERFACE("dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( bml3bus_mp1802_device::bml3_wd17xx_intrq_w )
{
	if (state) {
		m_bml3bus->set_nmi_line(PULSE_LINE);
	}
}

#define MP1802_ROM_REGION  "mp1802_rom"

ROM_START( mp1802 )
	ROM_REGION(0x10000, MP1802_ROM_REGION, 0)
	// MP-1802 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1802.rom", 0xf800, 0x800, BAD_DUMP CRC(8d0dc101) SHA1(92f7d1cebecafa7472e45c4999520de5c01c6dbc))
ROM_END

MACHINE_CONFIG_FRAGMENT( mp1802 )
	MCFG_MB8866_ADD("fdc", XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(bml3bus_mp1802_device, bml3_wd17xx_intrq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", mp1802_floppies, "dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", mp1802_floppies, "dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", mp1802_floppies, "", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", mp1802_floppies, "", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor bml3bus_mp1802_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mp1802 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *bml3bus_mp1802_device::device_rom_region() const
{
	return ROM_NAME( mp1802 );
}

READ8_MEMBER( bml3bus_mp1802_device::bml3_mp1802_r)
{
	return m_fdc->drq_r() ? 0x00 : 0x80;
}

WRITE8_MEMBER( bml3bus_mp1802_device::bml3_mp1802_w)
{
	floppy_image_device *floppy = NULL;

	switch (data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	case 2: floppy = m_floppy2->get_device(); break;
	case 3: floppy = m_floppy3->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(!BIT(data, 3));
		floppy->ss_w(BIT(data, 4));
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1802_device::bml3bus_mp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, BML3BUS_MP1802, "Hitachi MP-1802 Floppy Controller Card", tag, owner, clock, "bml3mp1802", __FILE__),
	device_bml3bus_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_floppy2(*this, "fdc:2"),
	m_floppy3(*this, "fdc:3")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1802_device::device_start()
{
	// set_bml3bus_device makes m_slot valid
	set_bml3bus_device();

	m_rom = memregion(MP1802_ROM_REGION)->base();

	// install into memory
	address_space &space_prg = machine().firstcpu->space(AS_PROGRAM);
	space_prg.install_readwrite_handler(0xff00, 0xff03, read8_delegate(FUNC(mb8866_t::read),(mb8866_t*)m_fdc), write8_delegate(FUNC(mb8866_t::write),(mb8866_t*)m_fdc));
	space_prg.install_readwrite_handler(0xff04, 0xff04, read8_delegate(FUNC(bml3bus_mp1802_device::bml3_mp1802_r), this), write8_delegate(FUNC(bml3bus_mp1802_device::bml3_mp1802_w), this));
	// overwriting the main ROM (rather than using e.g. install_rom) should mean that bank switches for RAM expansion still work...
	UINT8 *mainrom = device().machine().root_device().memregion("maincpu")->base();
	memcpy(mainrom + 0xf800, m_rom + 0xf800, 0x800);
}

void bml3bus_mp1802_device::device_reset()
{
}
