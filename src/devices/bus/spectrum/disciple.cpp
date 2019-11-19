// license:BSD-3-Clause
// copyright-holders:Twisted Tom
/**********************************************************************

    MGT DISCiPLE Multi-purpose Interface
	------------------------------------
	
	Produced by Miles Gordon Technology, UK, 1986-1990, who also produced the Sam Coupé home computer.
	
	Features:
	8KB ROM
	8KB RAM
	single floppy disk interface (2 drives)
	Centronics parallel interface
	"magic button" style memory snapshot grabber
	2 ATARI joystick ports (Sinclair 1/Kempston, Sinclair 2)
	2 network connectors (Interface 1 compatible, 3.5mm mono jack)
	inhibit button (to lock out the interface)
	pass-through expansion connector (to chain other devices)
	
	MGT's first disk interface, later replaced by a cost and feature reduced version, the +D.
	Unit is a large plastic base unit which the ZX Spectrum sits on top of, similar to Sinclair's official Interface 1.
	
	The official DOS was "GDOS", an earlier version of MGT's "G+DOS" which was used on the later +D unit.
	Both these were superseded by "SAM DOS" used by MGT's Sam Coupé (which is backwards-compatible with both).
	A 3rd party company SD Software released an alternative DOS "UNI-DOS" for both interfaces. (consisting of a disk and replacement ROM)
	
	Manual states any Shugart 400 DD drive should work
	"The disciple will accept 5.25" or 3.5" drives, whether they are 40 track or 80 track,
	 single sided or double sided, single density or double density."
	 
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
	The full v3 DOS survives a reset, so reloading of system disk is only required after full power cycle.
	v2 does not, system disk needs reloading after soft reset.
	
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
	Caps Shift + button   system freezes with a multi-coloured border effect
	then, key 3           save current SCREEN
	          4           save 48K PROGRAM
	          5           save 128K PROGRAM
	Caps Shift + number saves to drive 2 (or 1 if running from 2)
	
	GDOS versions:
	The rom/system disk versions must match,
	v2 rom: use system disk/system tape ver 2, 2b, 2c
	v3 rom: use system disk/system tape ver 3a, 3b or 3d
	
	Curiosities:
	The pass-through expansion connector has 4 extra pins (1 top/bottom each end)
	so 2x30 pins compared to ususal 2x28 of Spectrum expansion slot.
	Presumably this was intended for some unique expansion device that never appeared?
	One of these extra pins can be directly controlled via s/w by an IO write to 0x1f, bit 5.
	2 other pins appear to be able to override the /ce signal from PAL ic9 to the rom.
	4th pin is unused.
	
	The design allows for use of a larger 27128 (16KB) rom,
	with the highest address line A13 controllable via s/w by an IO write to 0x1f, bit 3.
	No larger roms seem to exist (or have not been found so far...?)
	Some 16KB dumps can be found but these are combined dumps of the 8KB rom and 8KB ram (with the full DOS loaded). 
	
	
	Current status:
	--------------
	issues with wd_fdc.cpp  see https://github.com/mamedev/mame/issues/5893
	wd1772 must spin-up disk in response to 0xd0 (force interrupt) command
	currently patching ROM to skip index pulse check which otherwise gives no disc error
	
	GDOS v3: all ok, occassional "no system file" when loading system disk, ok on 2nd attempt
	GDOS v2: all ok
	UNIDOS:  all ok
	
	Not working with 128K/+2 yet...
	
	todo: add .img support to coupedisk.cpp

**********************************************************************/


#include "emu.h"
#include "disciple.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_DISCIPLE, spectrum_disciple_device, "spectrum_disciple", "MGT DISCiPLE")

//-------------------------------------------------
//  INPUT_PORTS( disciple )
//-------------------------------------------------

INPUT_PORTS_START(disciple)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_disciple_device, snapshot_button, 0)

	// Joystick 1 (right hand) is both Kempston (port 0x1f) and Sinclair 1 (keys 6,7,8,9,0)
	// Joystick 2 (left hand) is Sinclair 2 (keys 1,2,3,4,5)
	
	PORT_START("KJOY") /* Kempston 0x1f */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston Right")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston Left")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston Down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston Up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)                  PORT_PLAYER(1) PORT_NAME("Kempston Button 1")

	PORT_START("SJOY1") /* Sinclair 1 (keys 6,7,8,9,0) 0xeffe */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(1) PORT_NAME("Sinclair P1 Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_NAME("Sinclair P1 Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Sinclair P1 Down")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Sinclair P1 Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Sinclair P1 Left")
	
	PORT_START("SJOY2") /* Sinclair 2 (keys 1,2,3,4,5) 0xf7fe */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Left")     PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Right")    PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Down")     PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Up")       PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(2) PORT_NAME("Sinclair P2 Button 1") PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_disciple_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(disciple);
}

//-------------------------------------------------
//  SLOT_INTERFACE( disciple_floppies )
//-------------------------------------------------

static void disciple_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_disciple_device::floppy_formats)
	FLOPPY_MGT_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( disciple )
//-------------------------------------------------

ROM_START(disciple)
	ROM_REGION(0x2000, "rom", 0)
	
	ROM_DEFAULT_BIOS("gdos")
	
	ROM_SYSTEM_BIOS(0, "gdos", "GDOS v3")
	ROMX_LOAD("disciple_g.rom", 0x0000, 0x2000, CRC(82047489) SHA1(9a75ed4b293f968985be4c9aa893cd88276d1ced), ROM_BIOS(0))
	// ROM_FILL(0x1059, 1, 0x18)
	// ROM_FILL(0x105a, 1, 0x0c)  // jr $3067
	
	ROM_SYSTEM_BIOS(1, "gdos2", "GDOS v2")
	ROMX_LOAD("disciple_g2.rom", 0x0000, 0x2000, CRC(9d971781) SHA1(a03e67e4ee275a85153843f42269fa980875d551), ROM_BIOS(1))
	
	ROM_SYSTEM_BIOS(2, "unidos", "UNI-DOS v2")
	ROMX_LOAD("disciple_uni.rom", 0x0000, 0x2000, CRC(1fe7f4fa) SHA1(6277abe6358c99ab894795536a1eb9393f25b9b1), ROM_BIOS(2))
	
	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal20l8.ic8", 0x000, 0x144, CRC(e53b2fcc) SHA1(85ce9634890d41be37cd9e0252698e5350a4c9c9) )
	ROM_LOAD( "pal20l8.ic9", 0x200, 0x144, CRC(43ff2e38) SHA1(b872377ea9f91b29a811b0d484699ffe87bdf9fd) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_disciple_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "fdc:0", disciple_floppies, "35dd", spectrum_disciple_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", disciple_floppies, "35dd", spectrum_disciple_device::floppy_formats).enable_sound(true);
	
	/* printer port */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_disciple_device::busy_w));
	
	/* pass-through */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	
	/* software list */
	SOFTWARE_LIST(config, "flop_list").set_original("spectrum_mgt_flop");
}


const tiny_rom_entry *spectrum_disciple_device::device_rom_region() const
{
	return ROM_NAME(disciple);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_disciple_device - constructor
//-------------------------------------------------

spectrum_disciple_device::spectrum_disciple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_DISCIPLE, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_centronics(*this, "centronics")
	, m_exp(*this, "exp")
	, m_kjoy(*this, "KJOY")
	, m_sjoy1(*this, "SJOY1")
	, m_sjoy2(*this, "SJOY2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_disciple_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_map));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_disciple_device::device_reset()
{
	m_romcs = 0;
	m_centronics_busy = false;
	m_map = false;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_disciple_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_disciple_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);
	
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
		case 0x0066:
		case 0x028e:
			m_romcs = 1;
			break;
		}
	}
}

uint8_t spectrum_disciple_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)
		{
		case 0x1b: // fdc status reg
			data = m_fdc->read(0);
			break;
		case 0x5b: // fdc track reg
			data = m_fdc->read(1);
			break;
		case 0x9b: // fdc sector reg
			data = m_fdc->read(2);
			break;
		case 0xdb: // fdc data reg
			data = m_fdc->read(3);
			break;
		case 0x1f: // bit 0-4: kempston joystick, bit 6: printer busy, bit 7: network
			data = m_kjoy->read() & 0x1f;
			data |= !m_centronics_busy << 6;  // inverted IC10 74ls240
			break;
		case 0x7b: // reset boot
			m_map = false;
			break;
		case 0xbb: // page in
			m_romcs = 1;
			break;
		case 0xfe: // sinclair joysticks
			if (((offset >> 8) & 16) == 0)
				data = m_sjoy1->read() | (0xff ^ 0x1f);
			if (((offset >> 8) & 8) == 0)
				data = m_sjoy2->read() | (0xff ^ 0x1f);
			break;
		}
	}
	
	return data;
}

void spectrum_disciple_device::iorq_w(offs_t offset, uint8_t data)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)
		{
		case 0x1b: // fdc command reg
			m_fdc->write(0, data);
			break;
		case 0x5b: // fdc track reg
			m_fdc->write(1, data);
			break;
		case 0x9b: // fdc sector reg
			m_fdc->write(2, data);
			break;
		case 0xdb: // fdc data reg
			m_fdc->write(3, data);
			break;
		case 0x1f: // bit 0: drive select, 1: side select, 2: density, 3: rom bank, 4: inhibit switch control, 5: exp select, 6: printer strobe, 7: network
			{
			floppy_image_device* floppy = m_floppy[~data & 1]->get_device();
			m_fdc->set_floppy(floppy);
			if (floppy) floppy->ss_w(BIT(data, 1));
			m_fdc->dden_w(BIT(data, 2));
			// 3: rom bank...
			// 4: inhibit switch control...
			// 5: exp select...
			m_centronics->write_strobe(BIT(data, 6));
			// 7: network...
			}
			break;
		case 0x3b: // wait when net=1
			// ...
			break;
		case 0x7b: // set boot
			m_map = true;
			break;
		case 0xbb: // page out
			m_romcs = 0;
			break;
		case 0xfb: // printer data
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
	
	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_disciple_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		if (m_map)
		{
			switch (offset & 0xe000)
			{
			case 0x0000:
				data = m_ram[offset & 0x1fff];
				break;
			case 0x2000:
				data = m_rom->base()[offset & 0x1fff];
				break;
			}
		}
		else
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
	}
	
	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_disciple_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		if (m_map)
		{
			switch (offset & 0xe000)
			{
			case 0x0000:
				m_ram[offset & 0x1fff] = data;
				break;
			}
		}
		else
		{
			switch (offset & 0xe000)
			{
			case 0x2000:
				m_ram[offset & 0x1fff] = data;
				break;
			}
		}
	}
	
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

INPUT_CHANGED_MEMBER(spectrum_disciple_device::snapshot_button)
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

WRITE_LINE_MEMBER(spectrum_disciple_device::busy_w)
{
	m_centronics_busy = state;
}
