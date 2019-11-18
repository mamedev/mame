// license:BSD-3-Clause
// copyright-holders:Twisted Tom
/**********************************************************************

    MGT +D Disk and Printer Interface
	---------------------------------
	
	Produced by Miles Gordon Technology, UK, 1986-1990, who also produced the Sam Coupé home computer.
	
	Features:
	8KB ROM
	8KB RAM
	single floppy disk interface (2 drives)
	Centronics parallel interface
	"magic button" style memory snapshot grabber
	
	MGT's second (and last) disk interface, a cost and feature reduced version of the earlier "Disciple" unit.
	Rather than being a large plastic base unit like its predecessor (Interface I style), this unit is a small metal-cased
	stand-alone unit which connects to ZX Spectrum's expansion slot via a ribbon cable.
	A second "official" version exists which was licensed and produced by Datel following MGT's demise.
	Many "unofficial" versions exist and the deisgn remains available today as DIY-style projects/kits.
	It's said the device's design and roms were officially released into the public domain at some point?
	
	The official DOS was "G+DOS" and is compatible with the earlier "GDOS" for earlier Disciple unit.
	Both these were superseded by "SAM DOS" used by MGT's Sam Coupé (which is backwards-compatible with both).
	A 3rd party company SD Software released an alternative DOS "UNI-DOS" for both interfaces. (consisting of a disk and replacement ROM)
	
	Manual states any Shugart 400 DD drive should work (but not SD)
	"we recommend 3.5" or 5.25" 80-track double sided and double density drives,
     which will give you up to 780K of storage per drive. But Shugart
     400-type 3" drives will also work."
	Pin 26 /DDEN of the WD1772 is tied to ground, so permanent DD mode.
	 
	Disks use "MGT filesystem".
	A good description available at https://faqwiki.zxnet.co.uk/wiki/MGT_filesystem
	
	Disk format is 512 bytes/sector, 10 sectors/track
	40 track, 1 side = 204,800 bytes (512*10*40*1)
	40 track, 2 side = 409,600 bytes
	80 track, 1 side = 409,600 bytes
	80 track, 2 side = 819,200 bytes  
	
	.mgt files work ok
	.img files don't work (not in coupedsk.cpp)
	
	The DOS must be loaded from a "System Disk" which is itself created from "System Tape" which was supplied with the unit.
	The ROM provides just the RUN command, which boots the system disk and loads the full DOS.
	Presumably the unit wasn't supplied with a system disk due to wide range of drives that can be used? (3", 3.5", 5.25")
	The full DOS survives a reset, so reloading of system disk is only required after full power cycle.
	
	A few useful commands:
	RUN                            Boots the system
	CAT 1                          Displays catalogue (drive 1)
	CAT 1!                         Displays shortened catalogue (drive 1)
	SAVE D1 "filename"             Saves file
	VERIFY D1 "filename"           Confirms save has been made
	LOAD D1 "filename"             Loads file (except Snapshot files)
	LOAD D1 "filename" S           Loads 48K Snapshot file
	LOAD D1 "filename" K           Loads 128K Snapshot file
	LOAD D1 "filename" SCREEN$     Loads screen file
	LOAD Pn                        Loads the program (from its number)
	ERASE D1 "file" TO "new file"  Renames a file
	ERASE D1 "filename"            Erases a file
	SAVE D1 "file" TO D2           Copies a file from drive1 to drive2
	FORMAT D1                      Formats disc in drive 1
	FORMAT D1 TO 2                 Formats drive 1; copies from 2 to 1
	
	Snapshot button:
	Press button   system freezes with a multi-coloured border effect
	then, key 3    save current SCREEN
	          4    save 48K PROGRAM
	          5    save 128K PROGRAM
			  X    do nothing, return to running program
	Caps Shift + number saves to drive 2 (or 1 if running from 2)
	
	
	Current status:
	--------------
	issues with wd_fdc.cpp  see https://github.com/mamedev/mame/issues/5893
	currently patching ROM to skip index pulse check which otherwise gives no disc error
	
	G+DOS: (w/ patch)
	disk read/write  ok
	snapshot save/load  ok
	disk format  ng
	
	UNIDOS: (no patch)
	no disk read/write, won't boot uni-bios system disk
	
	Not working with 128K/+2 yet...

**********************************************************************/


#include "emu.h"
#include "plusd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_PLUSD, spectrum_plusd_device, "spectrum_plusd", "MGT +D")

//-------------------------------------------------
//  INPUT_PORTS( plusd )
//-------------------------------------------------

INPUT_PORTS_START(plusd)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_plusd_device, snapshot_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_plusd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(plusd);
}

//-------------------------------------------------
//  SLOT_INTERFACE( plusd_floppies )
//-------------------------------------------------

static void plusd_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_plusd_device::floppy_formats)
	FLOPPY_MGT_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( plusd )
//-------------------------------------------------

ROM_START(plusd)
	ROM_REGION(0x2000, "rom", 0)
	
	ROM_DEFAULT_BIOS("gdos")
	
	ROM_SYSTEM_BIOS(0, "gdos", "G+DOS v1a")
	ROMX_LOAD("plusd_g.rom", 0x0000, 0x2000, CRC(569f7e55) SHA1(6b841dc5797ef7eb219ad455cd1e434ca3b9d30d), ROM_BIOS(0))
	ROM_FILL(0x06c8, 1, 0x18)
	ROM_FILL(0x06c9, 1, 0x11)  // jr $06db
	
	ROM_SYSTEM_BIOS(1, "unidos", "UNI-DOS")
	ROMX_LOAD("plusd_uni.rom", 0x0000, 0x2000, CRC(60920496) SHA1(399c8c7c8335bc59849a2182c32347603fd0288a), ROM_BIOS(1))
	
	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "alice_pal20l8.ic4", 0x000, 0x144, CRC(60135856) SHA1(41273f13a3680b29ba84ae1e85829482c783c55e) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_plusd_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "fdc:0", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);
	
	/* printer port */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_plusd_device::busy_w));
	
	/* software list */
	SOFTWARE_LIST(config, "flop_list").set_original("spectrum_mgt_flop");
}


const tiny_rom_entry *spectrum_plusd_device::device_rom_region() const
{
	return ROM_NAME(plusd);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_plusd_device - constructor
//-------------------------------------------------

spectrum_plusd_device::spectrum_plusd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_PLUSD, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_plusd_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_centronics_busy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_plusd_device::device_reset()
{
	m_romcs = 0;
	m_centronics_busy = false;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_plusd_device::romcs)
{
	return m_romcs;
}

void spectrum_plusd_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
		case 0x003a:
		case 0x0066:
			m_romcs = 1;
			break;
		}
	}
}

uint8_t spectrum_plusd_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)
		{
		case 0xe3: // fdc status reg
			data = m_fdc->read(0);
			break;
		case 0xeb: // fdc track reg
			data = m_fdc->read(1);
			break;
		case 0xf3: // fdc sector reg
			data = m_fdc->read(2);
			break;
		case 0xfb: // fdc data reg
			data = m_fdc->read(3);
			break;
		case 0xe7: // page in
			m_romcs = 1;
			break;
		case 0xf7: // bit 7: printer busy
			data = m_centronics_busy << 7;
			break;
		}
	}
	
	return data;
}

void spectrum_plusd_device::iorq_w(offs_t offset, uint8_t data)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)
		{
		case 0xe3: // fdc command reg
			m_fdc->write(0, data);
			break;
		case 0xeb: // fdc track reg
			m_fdc->write(1, data);
			break;
		case 0xf3: // fdc sector reg
			m_fdc->write(2, data);
			break;
		case 0xfb: // fdc data reg
			m_fdc->write(3, data);
			break;
		case 0xef: // bit 0-1: drive select, 6: printer strobe, 7: side select
			{
			uint8_t drive = data & 3;
			floppy_image_device* floppy = m_floppy[drive > 0 ? drive-1 : drive]->get_device();
			m_fdc->set_floppy(floppy);
			m_centronics->write_strobe(BIT(data, 6));
			if (floppy) floppy->ss_w(BIT(data, 7));
			}
			break;
		case 0xe7: // page out
			m_romcs = 0;
			break;
		case 0xf7: // printer data
			m_centronics->write_data0(BIT(data, 0));
			m_centronics->write_data1(BIT(data, 1));
			m_centronics->write_data2(BIT(data, 2));
			m_centronics->write_data3(BIT(data, 3));
			m_centronics->write_data4(BIT(data, 4));
			m_centronics->write_data5(BIT(data, 5));
			m_centronics->write_data6(BIT(data, 6));
			m_centronics->write_data7(BIT(data, 7));
			break;
		}
	}
}

uint8_t spectrum_plusd_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x0000:
			data = m_rom->base()[offset & 0x1fff];
			break;
		case 0x2000:
			data = m_ram[offset & 0x1fff];
			break;
		}
	}

	return data;
}

void spectrum_plusd_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x2000:
			m_ram[offset & 0x1fff] = data;
			break;
		}
	}
}

INPUT_CHANGED_MEMBER(spectrum_plusd_device::snapshot_button)
{
	if (newval && !oldval)
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
	else
	{
		m_slot->nmi_w(ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(spectrum_plusd_device::busy_w)
{
	m_centronics_busy = state;
}
