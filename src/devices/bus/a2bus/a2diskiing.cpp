// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.c

    Implementation of the Apple II Disk II controller card

*********************************************************************/

#include "emu.h"
#include "a2diskiing.h"

#include "formats/ap2_dsk.h"
#include "formats/as_dsk.h"
#include "formats/fs_prodos.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_DISKIING, a2bus_diskiing_device, "a2diskiing", "Apple Disk II NG controller (16-sector)")
DEFINE_DEVICE_TYPE(A2BUS_DISKIING13, a2bus_diskiing13_device, "diskii13", "Apple Disk II NG controller (13-sector)")
DEFINE_DEVICE_TYPE(A2BUS_APPLESURANCE, a2bus_applesurance_device, "a2surance", "Applesurance Diagnostic Controller")
DEFINE_DEVICE_TYPE(A2BUS_AGAT7_FDC, a2bus_agat7flop_device, "agat7_flop", "Agat-7 140K floppy card")
DEFINE_DEVICE_TYPE(A2BUS_AGAT9_FDC, a2bus_agat9flop_device, "agat9_flop", "Agat-9 140K floppy card")

#define WOZFDC_TAG         "wozfdc"
#define DISKII_ROM_REGION  "diskii_rom"

static void a2_floppies(device_slot_interface &device)
{
	device.option_add("525", FLOPPY_525_SD);
}

ROM_START( diskiing )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5",  0x0000, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) ) /* 341-0027-a: 16-sector disk drive (older version), PROM P5 */
ROM_END

ROM_START( diskiing13 )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0009.bin", 0x000000, 0x000100, CRC(d34eb2ff) SHA1(afd060e6f35faf3bb0146fa889fc787adf56330a) )
ROM_END

ROM_START( applesurance )
	ROM_REGION(0x1000, DISKII_ROM_REGION, 0)
	ROM_LOAD( "applesurance 3.0 - 2732.bin", 0x000000, 0x001000, CRC(64eafec7) SHA1(723dc6cd32de5a0f27af7503764185ac58904c05) )
ROM_END

ROM_START( agat7 )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "shugart7.rom", 0x0000, 0x0100, CRC(c6e4850c) SHA1(71626d3d2d4bbeeac2b77585b45a5566d20b8d34) )
ROM_END

ROM_START( agat9 )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "shugart9.rom", 0x0000, 0x0100, CRC(964a0ce2) SHA1(bf955189ebffe874c20ef649a3db8177dc16af61) )
ROM_END

void diskiing_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_A213S_FORMAT);
	fr.add(FLOPPY_A216S_DOS_FORMAT);
	fr.add(FLOPPY_A216S_PRODOS_FORMAT);
	fr.add(FLOPPY_RWTS18_FORMAT);
	fr.add(FLOPPY_EDD_FORMAT);
	fr.add(FLOPPY_WOZ_FORMAT);
	fr.add(FLOPPY_NIB_FORMAT);

	fr.add(fs::PRODOS);
}

void a2bus_diskiing13_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_A213S_FORMAT);
	fr.add(FLOPPY_EDD_FORMAT);
	fr.add(FLOPPY_WOZ_FORMAT);
	fr.add(FLOPPY_NIB_FORMAT);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void diskiing_device::device_add_mconfig(machine_config &config)
{
	DISKII_FDC(config, m_wozfdc, 1021800*2);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, a2_floppies, "525", diskiing_device::floppy_formats).enable_sound(true);
}

void a2bus_diskiing13_device::device_add_mconfig(machine_config &config)
{
	DISKII_FDC(config, m_wozfdc, 1021800*2);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, a2_floppies, "525", a2bus_diskiing13_device::floppy_formats).enable_sound(true);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *diskiing_device::device_rom_region() const
{
	return ROM_NAME( diskiing );
}

const tiny_rom_entry *a2bus_diskiing13_device::device_rom_region() const
{
	return ROM_NAME( diskiing13 );
}

const tiny_rom_entry *a2bus_applesurance_device::device_rom_region() const
{
	return ROM_NAME( applesurance );
}

const tiny_rom_entry *a2bus_agat7flop_device::device_rom_region() const
{
	return ROM_NAME(agat7);
}

const tiny_rom_entry *a2bus_agat9flop_device::device_rom_region() const
{
	return ROM_NAME(agat9);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

diskiing_device::diskiing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_wozfdc(*this, WOZFDC_TAG),
	m_floppy(*this, "%u", 0U),
	m_rom(nullptr)
{
}

a2bus_diskiing_device::a2bus_diskiing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_DISKIING, tag, owner, clock)
{
}

a2bus_diskiing13_device::a2bus_diskiing13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_DISKIING13, tag, owner, clock)
{
}

a2bus_applesurance_device::a2bus_applesurance_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_APPLESURANCE, tag, owner, clock),
	m_c800_bank(1)
{
}

a2bus_agat7flop_device::a2bus_agat7flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_AGAT7_FDC, tag, owner, clock)
{
}

a2bus_agat9flop_device::a2bus_agat9flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_AGAT9_FDC, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diskiing_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void diskiing_device::device_reset()
{
	m_wozfdc->set_floppies(m_floppy[0], m_floppy[1]);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t diskiing_device::read_c0nx(uint8_t offset)
{
	return m_wozfdc->read(offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void diskiing_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_wozfdc->write(offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t diskiing_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}

uint8_t a2bus_applesurance_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset+0x800];
}

uint8_t a2bus_applesurance_device::read_c800(uint16_t offset)
{
	if (offset == 0x7ff)
	{
		m_c800_bank = 1;
	}

	if (!m_c800_bank)
	{
		return m_rom[offset];
	}

	return m_rom[offset+0x800];
}

void a2bus_applesurance_device::device_reset()
{
	m_c800_bank = 1;
	diskiing_device::device_reset();
}
