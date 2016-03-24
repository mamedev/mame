// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2corvus.c

    Implementation of the Corvus flat-cable hard disk interface
    for the Apple II.

    This same card was used in the Corvus Concept.

    C0n0 = drive read/write
    C0n1 = read status (busy in bit 7, data direction in bit 6)

    Reads and writes to C0n2+ happen; the contents of the reads are thrown away
    immediately by all the code I've examined, and sending the writes to the
    drive's write port makes it not work so they're intended to be ignored too.

    5 MB: -chs 144,4,20 -ss 512
    10 MB: -chs 358,3,20 -ss 512
    20 MB: -chs 388,5,20 -ss 512

    To set up a disk from scratch on the Apple II:
    1) Create a disk of your desired capacity using CHDMAN -c none and the parameters
       listed above for each of the possible sizes.
    2) Boot apple2p with the corvus in slot 2 and a diskii(ng) in slot 6 with the
       "Corvus Hard Drive - Diagnostics.dsk" mounted.
    3) Press F to format.  Accept all the default options from now on;
       there is no "format switch" to worry about with the current emulation.
    4) Quit MESS.  Restart with the corvus in slot 6 and a diskii(ng) in slot 7
       with the "Corvus Hard Drive - Utilities Disk 1.dsk" mounted.
    5) When you get the BASIC prompt, "RUN BSYSGEN"
    6) Choose drive 1 and press Y at "OK TO BSYSGEN?"
    7) When the format completes, type "RUN APPLESOFT BOOT PREP" and press Enter.
    8) Once it finishes, quit MESS.  Remove the diskii(ng) from slot 7 and
       the system will boot into DOS 3.3 from the Corvus HD.

    TODO: but there are no Corvus drivers present after that, only
    Disk II?

*********************************************************************/

#include "a2corvus.h"
#include "includes/apple2.h"
#include "imagedev/harddriv.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_CORVUS = &device_creator<a2bus_corvus_device>;

#define CORVUS_ROM_REGION  "corvus_rom"
#define CORVUS_HD_TAG      "corvushd"

static MACHINE_CONFIG_FRAGMENT(corvus)
	MCFG_DEVICE_ADD(CORVUS_HD_TAG, CORVUS_HDC, 0)
	MCFG_HARDDISK_ADD("harddisk1")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk2")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk3")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk4")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
MACHINE_CONFIG_END

ROM_START( corvus )
	ROM_REGION(0x800, CORVUS_ROM_REGION, 0)
	ROM_LOAD( "a4.7.u10", 0x0000, 0x0800, CRC(1cf6e32a) SHA1(dbd6efeb3b54c0523b8b4eda8b3d737413f6a91a) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_corvus_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( corvus );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_corvus_device::device_rom_region() const
{
	return ROM_NAME( corvus );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_corvus_device::a2bus_corvus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_corvushd(*this, CORVUS_HD_TAG), m_rom(nullptr)
{
}

a2bus_corvus_device::a2bus_corvus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_CORVUS, "Corvus Flat Cable interface", tag, owner, clock, "a2corvus", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_corvushd(*this, CORVUS_HD_TAG), m_rom(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_corvus_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(CORVUS_ROM_REGION).c_str())->base();
}

void a2bus_corvus_device::device_reset()
{
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_corvus_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			return m_corvushd->read(space, 0);

		case 1:
			return m_corvushd->status_r(space, 0);

		default:
			logerror("Corvus: read unhandled c0n%x (PC=%x)\n", offset, space.device().safe_pc());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_corvus_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	if (offset == 0)
	{
		m_corvushd->write(space, 0, data);
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_corvus_device::read_cnxx(address_space &space, UINT8 offset)
{
	// one slot image at the end of the ROM, it appears
	return m_rom[offset+0x700];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_corvus_device::read_c800(address_space &space, UINT16 offset)
{
	return m_rom[offset & 0x7ff];
}
