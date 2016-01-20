// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.c

    Implementation of the Apple II Disk II controller card

*********************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "imagedev/flopdrv.h"
#include "formats/ap2_dsk.h"
#include "machine/appldriv.h"
#include "a2diskii.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_DISKII = &device_creator<a2bus_diskii_device>;
const device_type A2BUS_IWM_FDC = &device_creator<a2bus_iwmflop_device>;

#define DISKII_ROM_REGION  "diskii_rom"
#define FDC_TAG            "diskii_fdc"

const applefdc_interface fdc_interface =
{
	apple525_set_lines,         /* set_lines */
	apple525_set_enable_lines,  /* set_enable_lines */

	apple525_read_data,         /* read_data */
	apple525_write_data,    /* write_data */
	apple525_read_status    /* read_status */
};

static const floppy_interface floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple2),
	"floppy_5_25"
};

MACHINE_CONFIG_FRAGMENT( diskii )
	MCFG_APPLEFDC_ADD(FDC_TAG, fdc_interface)
	MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(floppy_interface,15,16)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( iwmflop )
	MCFG_IWM_ADD(FDC_TAG, fdc_interface)
	MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(floppy_interface,15,16)
MACHINE_CONFIG_END

ROM_START( diskii )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5", 0x000000, 0x000100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) )
ROM_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_floppy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( diskii );
}

machine_config_constructor a2bus_iwmflop_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( iwmflop );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_floppy_device::device_rom_region() const
{
	return ROM_NAME( diskii );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_floppy_device::a2bus_floppy_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a2bus_card_interface(mconfig, *this),
		m_fdc(*this, FDC_TAG), m_rom(nullptr)
{
}

a2bus_diskii_device::a2bus_diskii_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_floppy_device(mconfig, A2BUS_DISKII, "Apple Disk II controller", tag, owner, clock, "a2diskii", __FILE__)
{
}

a2bus_iwmflop_device::a2bus_iwmflop_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_floppy_device(mconfig, A2BUS_IWM_FDC, "Apple IWM floppy card", tag, owner, clock, "a2iwm_flop", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_floppy_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void a2bus_floppy_device::device_reset()
{
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_floppy_device::read_c0nx(address_space &space, UINT8 offset)
{
	return m_fdc->read(offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_floppy_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	m_fdc->write(offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_floppy_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset];
}
