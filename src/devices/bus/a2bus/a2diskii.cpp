// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.c

    Implementation of the Apple II Disk II controller card

*********************************************************************/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(A2BUS_DISKII,    a2bus_diskii_device,    "a2diskii",   "Apple Disk II controller")
DEFINE_DEVICE_TYPE(A2BUS_IWM_FDC,   a2bus_iwmflop_device,   "a2iwm_flop", "Apple IWM floppy card")
DEFINE_DEVICE_TYPE(A2BUS_AGAT7_FDC, a2bus_agat7flop_device, "agat7_flop", "Agat-7 140K floppy card")

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

ROM_START( diskii )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5", 0x000000, 0x000100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) )
ROM_END

ROM_START( agat7 )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "shugart7.rom", 0x0000, 0x0100, CRC(c6e4850c) SHA1(71626d3d2d4bbeeac2b77585b45a5566d20b8d34) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_floppy_device::device_add_mconfig(machine_config &config)
{
	APPLEFDC(config, m_fdc, &fdc_interface);
	FLOPPY_APPLE(config, FLOPPY_0, &floppy_interface, 15, 16);
	FLOPPY_APPLE(config, FLOPPY_1, &floppy_interface, 15, 16);
}

void a2bus_iwmflop_device::device_add_mconfig(machine_config &config)
{
	IWM(config, m_fdc, &fdc_interface);
	FLOPPY_APPLE(config, FLOPPY_0, &floppy_interface, 15, 16);
	FLOPPY_APPLE(config, FLOPPY_1, &floppy_interface, 15, 16);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_floppy_device::device_rom_region() const
{
	return ROM_NAME( diskii );
}

const tiny_rom_entry *a2bus_agat7flop_device::device_rom_region() const
{
	return ROM_NAME( agat7 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_floppy_device::a2bus_floppy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_fdc(*this, FDC_TAG), m_rom(nullptr)
{
}

a2bus_diskii_device::a2bus_diskii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_floppy_device(mconfig, A2BUS_DISKII, tag, owner, clock)
{
}

a2bus_iwmflop_device::a2bus_iwmflop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_floppy_device(mconfig, A2BUS_IWM_FDC, tag, owner, clock)
{
}

a2bus_agat7flop_device::a2bus_agat7flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_floppy_device(mconfig, A2BUS_AGAT7_FDC, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_floppy_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void a2bus_floppy_device::device_reset()
{
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_floppy_device::read_c0nx(uint8_t offset)
{
	return m_fdc->read(offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_floppy_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_fdc->write(offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_floppy_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}
