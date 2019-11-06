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

DEFINE_DEVICE_TYPE(BML3BUS_MP1805, bml3bus_mp1805_device, "bml3mp1805", "Hitachi MP-1805 Floppy Controller Card")

static const floppy_interface bml3_mp1805_floppy_interface =
{
	FLOPPY_STANDARD_3_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	nullptr
};

WRITE_LINE_MEMBER( bml3bus_mp1805_device::bml3_mc6843_intrq_w )
{
	if (state)
	{
		raise_slot_nmi();
		lower_slot_nmi();
	}
}

#define MP1805_ROM_REGION  "mp1805_rom"

ROM_START( mp1805 )
	ROM_REGION(0x10000, MP1805_ROM_REGION, 0)
	// MP-1805 disk controller ROM, which replaces part of the system ROM
	ROM_LOAD( "mp1805.rom", 0xf800, 0x0800, BAD_DUMP CRC(b532d8d9) SHA1(6f1160356d5bf64b5926b1fdb60db414edf65f22))
ROM_END


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bml3bus_mp1805_device::device_add_mconfig(machine_config &config)
{
	MC6843(config, m_mc6843, 0);
	m_mc6843->set_floppy_drives(m_floppy[0], m_floppy[1], m_floppy[2], m_floppy[3]);
	m_mc6843->irq().set(FUNC(bml3bus_mp1805_device::bml3_mc6843_intrq_w));

	for (auto &floppy : m_floppy)
		LEGACY_FLOPPY(config, floppy, 0, &bml3_mp1805_floppy_interface);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bml3bus_mp1805_device::device_rom_region() const
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
	m_mc6843->set_drive(drive);
	m_floppy[drive]->floppy_mon_w(motor);
	m_floppy[drive]->floppy_drive_set_ready_state(ASSERT_LINE, 0);
	m_mc6843->set_side(side);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_mp1805_device::bml3bus_mp1805_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BML3BUS_MP1805, tag, owner, clock),
	device_bml3bus_card_interface(mconfig, *this),
	m_floppy(*this, "floppy%u", 0U),
	m_mc6843(*this, "mc6843"), m_rom(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_mp1805_device::device_start()
{
	m_rom = memregion(MP1805_ROM_REGION)->base();

	// install into memory
	address_space &space_prg = space();
	space_prg.install_readwrite_handler(0xff18, 0xff1f, read8_delegate(*m_mc6843, FUNC(mc6843_device::read)), write8_delegate(*m_mc6843, FUNC(mc6843_device::write)));
	space_prg.install_readwrite_handler(0xff20, 0xff20, read8_delegate(*this, FUNC(bml3bus_mp1805_device::bml3_mp1805_r)), write8_delegate(*this, FUNC(bml3bus_mp1805_device::bml3_mp1805_w)));
	// overwriting the main ROM (rather than using e.g. install_rom) should mean that bank switches for RAM expansion still work...
	uint8_t *mainrom = device().machine().root_device().memregion("maincpu")->base();
	memcpy(mainrom + 0xf800, m_rom + 0xf800, 0x800);
}

void bml3bus_mp1805_device::device_reset()
{
}
