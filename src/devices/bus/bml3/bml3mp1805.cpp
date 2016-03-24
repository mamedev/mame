// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1805.c

    Hitachi MP-1805 floppy disk controller card for the MB-6890
    Floppy drive is attached

*********************************************************************/

#include "emu.h"
#include "bml3mp1805.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BML3BUS_MP1805 = &device_creator<bml3bus_mp1805_device>;

static const floppy_interface bml3_mp1805_floppy_interface =
{
	FLOPPY_STANDARD_3_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	nullptr
};

WRITE_LINE_MEMBER( bml3bus_mp1805_device::bml3_mc6843_intrq_w )
{
	if (state) {
		m_bml3bus->set_nmi_line(PULSE_LINE);
	}
}

#define MP1805_ROM_REGION  "mp1805_rom"

ROM_START( mp1805 )
	ROM_REGION(0x10000, MP1805_ROM_REGION, 0)
	// MP-1805 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1805.rom", 0xf800, 0x0800, BAD_DUMP CRC(b532d8d9) SHA1(6f1160356d5bf64b5926b1fdb60db414edf65f22))
ROM_END

MACHINE_CONFIG_FRAGMENT( mp1805 )
	MCFG_DEVICE_ADD( "mc6843", MC6843, 0 )
	MCFG_MC6843_IRQ_CALLBACK(WRITELINE(bml3bus_mp1805_device, bml3_mc6843_intrq_w))
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(bml3_mp1805_floppy_interface)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor bml3bus_mp1805_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mp1805 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *bml3bus_mp1805_device::device_rom_region() const
{
	return ROM_NAME( mp1805 );
}

READ8_MEMBER( bml3bus_mp1805_device::bml3_mp1805_r)
{
	// TODO: read supported or not?
	//  return mc6843_drq_r(m_mc6843) ? 0x00 : 0x80;
	return -1;
}

WRITE8_MEMBER( bml3bus_mp1805_device::bml3_mp1805_w)
{
	// b7 b6 b5 b4 b3 b2 b1 b0
	// MT ?  ?  ?  D3 D2 D1 D0
	// MT: 0=motor off, 1=motor on
	// Dn: 1=select drive <n>
	int drive_select = data & 0x0f;
	int drive;
	// TODO: MESS UI for flipping disk? Note that D88 images are double-sided, but the physical drive is single-sided
	int side = 0;
	int motor = BIT(data, 7);
	const char *floppy_name = nullptr;
	switch (drive_select) {
	case 1:
		drive = 0;
		break;
	case 2:
		drive = 1;
		break;
	case 4:
		drive = 2;
		break;
	case 8:
		drive = 3;
		break;
	default:
		// TODO: what's the correct behaviour if more than one drive select bit is set? Or no bit set?
		drive = 0;
		break;
	}
	switch (drive) {
	case 0:
		floppy_name = FLOPPY_0;
		break;
	case 1:
		floppy_name = FLOPPY_1;
		break;
	case 2:
		floppy_name = FLOPPY_2;
		break;
	case 3:
		floppy_name = FLOPPY_3;
		break;
	}
	legacy_floppy_image_device *floppy = subdevice<legacy_floppy_image_device>(floppy_name);
	m_mc6843->set_drive(drive);
	floppy->floppy_mon_w(motor);
	floppy->floppy_drive_set_ready_state(ASSERT_LINE, 0);
	m_mc6843->set_side(side);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1805_device::bml3bus_mp1805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, BML3BUS_MP1805, "Hitachi MP-1805 Floppy Controller Card", tag, owner, clock, "bml3mp1805", __FILE__),
	device_bml3bus_card_interface(mconfig, *this),
	m_mc6843(*this, "mc6843"), m_rom(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1805_device::device_start()
{
	// set_bml3bus_device makes m_slot valid
	set_bml3bus_device();

	m_rom = memregion(MP1805_ROM_REGION)->base();

	// install into memory
	address_space &space_prg = machine().firstcpu->space(AS_PROGRAM);
	space_prg.install_readwrite_handler(0xff18, 0xff1f, read8_delegate( FUNC(mc6843_device::read), (mc6843_device*)m_mc6843), write8_delegate(FUNC(mc6843_device::write), (mc6843_device*)m_mc6843) );
	space_prg.install_readwrite_handler(0xff20, 0xff20, read8_delegate( FUNC(bml3bus_mp1805_device::bml3_mp1805_r), this), write8_delegate(FUNC(bml3bus_mp1805_device::bml3_mp1805_w), this) );
	// overwriting the main ROM (rather than using e.g. install_rom) should mean that bank switches for RAM expansion still work...
	UINT8 *mainrom = device().machine().root_device().memregion("maincpu")->base();
	memcpy(mainrom + 0xf800, m_rom + 0xf800, 0x800);
}

void bml3bus_mp1805_device::device_reset()
{
}
