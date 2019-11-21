// license:BSD-3-Clause
// copyright-holders:TwistedTom
/**********************************************************************

	DISCiPLE Multi-purpose Interface
	+D Disk and Printer Interface
	
	Miles Gordon Technology, UK, 1986-1990 (also produced the Sam Coupé home computer.)
	
	DISCiPLE was MGT's first disk interface, a large plastic base that sat under the Spectrum, similar to Sinclair's official Interface 1.
	+D was MGT's second (and last) disk interface, a cost and feature reduced version of the Disciple, a small metal-cased,
	stand-alone unit which connects to ZX Spectrum's expansion slot via a ribbon cable.
	A second Datel version of the +D exists (following the closure of MGT, it was licensed and produced by Datel.)
	Many unofficial versions of +D exist and modern versions are still seen today as DIY-style projects/kits.
	It's said the device's design and roms were officially released into the public domain at some point?
	
	DISCiPLE features:
	8KB ROM
	8KB RAM
	single floppy disk interface (2 drives)
	Centronics parallel printer interface
	"magic button" style memory snapshot grabber
	2 ATARI joystick ports (Sinclair 1/Kempston, Sinclair 2)
	2 network connectors (Interface 1 compatible, 3.5mm mono jack)
	inhibit button (to lock out the interface)
	pass-through expansion connector (to chain other devices)
	
	+D features:
	same as DISCiPLE
	lost the joystick/network ports, inhibit button and pass-through expansion connector
	
	DISCiPLE's official DOS was "GDOS".
	+D's official DOS was "G+DOS".
	Both of these were later superseded by "SAM DOS" (used by MGT's Sam Coupé.)
	A 3rd party company SD Software released an alternative DOS "UNI-DOS" for both interfaces. (consisting of a disk and replacement ROM)
	
	FDD support:
	
	DISCiPLE's manual states any Shugart 400 SD/DD drive should work:
	"The disciple will accept 5.25" or 3.5" drives, whether they are 40 track or 80 track,
	 single sided or double sided, single density or double density."
	 
	+D's manual states any Shugart 400 DD drive should work (but not SD)
	"we recommend 3.5" or 5.25" 80-track double sided and double density drives,
	 which will give you up to 780K of storage per drive. But Shugart
	 400-type 3" drives will also work."
	
	+D only: Pin 26 /DDEN of the WD1772 is tied to ground, so permanent DD mode.
	DISCiPLE only: /DDEN can be directly controlled via s/w by an IO write to 0x1f, bit 2.
	
	Disks use "MGT filesystem".
	A good description available at https://faqwiki.zxnet.co.uk/wiki/MGT_filesystem
	
	Disk format is 512 bytes/sector, 10 sectors/track
	40 track, 1 side = 204,800 bytes (512*10*40*1)
	40 track, 2 side = 409,600 bytes
	80 track, 1 side = 409,600 bytes
	80 track, 2 side = 819,200 bytes  <-- only this one supported so far
	
	.mgt files work ok
	.img files don't work (not in coupedsk.cpp)
	
	The DOS must be loaded from a "System Disk" which is itself created from "System Tape" which was supplied with the unit.
	The ROM provides just the RUN command, which boots the system disk and loads the full DOS.
	Presumably the unit wasn't supplied with a system disk due to wide range of drives that can be used? (3", 3.5", 5.25")
	The DOS survives a reset, so reloading of system disk is only required after full power cycle.
	
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
	
	DISCiPLE snapshot button:
	Caps Shift + button   system freezes with a multi-coloured border effect
	then, key 3           save current SCREEN
	          4           save 48K PROGRAM
	          5           save 128K PROGRAM
	Caps Shift + number saves to drive 2 (or 1 if running from 2)
	
	+D snapshot button:
	button           system freezes with a multi-coloured border effect  (don't need to hold caps shift)
	then, key 3      save current SCREEN
	          4      save 48K PROGRAM
	          5      save 128K PROGRAM
	          X      do nothing, return to running program
	Caps Shift + number saves to drive 2 (or 1 if running from 2)
	
	DISCiPLE GDOS versions:
	The rom/system disk versions must match,
	v2 rom: use system disk/system tape ver 2, 2b, 2c
	v3 rom: use system disk/system tape ver 3a, 3b or 3d
	
	+D G+DOS versions:
	a v1 (-non a) exists but has yet to be found/dumped,
	from "pick-poke-it" user manual:
	 "A few PLUS D users are still using Version 1 of the ROM which was used in PLUS D's sold in December 1987-January 1988.
	  ... check the serial number on the bottom of your PLUS D. If it's a 4-figure number commencing with 1,
	  then you have a PLUS D with the Version 1 ROM."
	
	DISCiPLE only curiosities:
	The pass-through expansion connector has 4 extra pins (1 top/bottom each end)
	so 2x30 pins compared to ususal 2x28 of Spectrum expansion slot.
	Presumably this was intended for some unique expansion device that never appeared?
	One of these extra pins can be directly controlled via s/w by an IO write to 0x1f, bit 5.
	2 other pins appear to be able to override the /ce signal from PAL ic9 to the rom.
	4th pin is unused.
	
	The design allows for use of a larger 27128 (16KB) rom,
	with the highest address line A13 controllable via s/w by an IO write to 0x1f, bit 3.
	No larger roms seem to exist (or perhaps not yet found...?)
	Some 16KB dumps can be found but these are combined dumps of the 8KB rom and 8KB ram (with the full DOS loaded). 
	
	
	Current status:
	--------------
	
	DISCiPLE
	GDOS v3: all ok, occassional "no system file" when loading system disk, ok on 2nd attempt
	GDOS v2: all ok
	UNIDOS:  all ok
	
	+D
	G+DOS:  all ok
	UNIDOS: all ok
	
	
	Not working with 128K/+2 yet...

**********************************************************************/


#include "emu.h"
#include "mgt.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_PLUSD, spectrum_plusd_device, "spectrum_plusd", "MGT +D")
DEFINE_DEVICE_TYPE(SPECTRUM_DISCIPLE, spectrum_disciple_device, "spectrum_disciple", "MGT DISCiPLE")

//-------------------------------------------------
//  INPUT_PORTS( plusd )
//-------------------------------------------------

INPUT_PORTS_START(plusd)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_plusd_device, snapshot_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  INPUT_PORTS( disciple )
//-------------------------------------------------

INPUT_PORTS_START(disciple)
	PORT_INCLUDE(plusd)
	
	// Joystick 1 (right-hand) is both Kempston (port 0x1f) and Sinclair 1 (keys 6,7,8,9,0)
	// Joystick 2 (left-hand) is Sinclair 2 (keys 1,2,3,4,5)
	
	PORT_START("JOY1") /* Sinclair 1 (keys 6,7,8,9,0) 0xeffe , Kempston 0x1f */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(1) PORT_NAME("Kempston\\Sinclair P1 Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston\\Sinclair P1 Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston\\Sinclair P1 Down")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston\\Sinclair P1 Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Kempston\\Sinclair P1 Left")
	
	PORT_START("JOY2") /* Sinclair 2 (keys 1,2,3,4,5) 0xf7fe */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Left")     PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Right")    PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Down")     PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2) PORT_NAME("Sinclair P2 Up")       PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(2) PORT_NAME("Sinclair P2 Button 1") PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_plusd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(plusd);
}

ioport_constructor spectrum_disciple_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(disciple);
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
	
	ROM_SYSTEM_BIOS(1, "unidos", "UNI-DOS v2")
	ROMX_LOAD("plusd_uni.rom", 0x0000, 0x2000, CRC(60920496) SHA1(399c8c7c8335bc59849a2182c32347603fd0288a), ROM_BIOS(1))
	
	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "alice_pal20l8.ic4", 0x000, 0x144, CRC(60135856) SHA1(41273f13a3680b29ba84ae1e85829482c783c55e) )
ROM_END

//-------------------------------------------------
//  ROM( disciple )
//-------------------------------------------------

ROM_START(disciple)
	ROM_REGION(0x2000, "rom", 0)
	
	ROM_DEFAULT_BIOS("gdos")
	
	ROM_SYSTEM_BIOS(0, "gdos", "GDOS v3")
	ROMX_LOAD("disciple_g.rom", 0x0000, 0x2000, CRC(82047489) SHA1(9a75ed4b293f968985be4c9aa893cd88276d1ced), ROM_BIOS(0))
	
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

void spectrum_plusd_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "fdc:0", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);
	
	/* printer port */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_plusd_device::busy_w));
	
	/* software list */
	SOFTWARE_LIST(config, "flop_list").set_original("spectrum_mgt_flop");
}

void spectrum_disciple_device::device_add_mconfig(machine_config &config)
{
	spectrum_plusd_device::device_add_mconfig(config);
	
	/* pass-through */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *spectrum_plusd_device::device_rom_region() const
{
	return ROM_NAME(plusd);
}

const tiny_rom_entry *spectrum_disciple_device::device_rom_region() const
{
	return ROM_NAME(disciple);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_plusd_device - constructor
//-------------------------------------------------

spectrum_plusd_device::spectrum_plusd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_centronics(*this, "centronics")
	, m_centronics_busy(false)
{
}

spectrum_plusd_device::spectrum_plusd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_plusd_device(mconfig, SPECTRUM_PLUSD, tag, owner, clock)
{
}

//-------------------------------------------------
//  spectrum_disciple_device - constructor
//-------------------------------------------------

spectrum_disciple_device::spectrum_disciple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_plusd_device(mconfig, SPECTRUM_DISCIPLE, tag, owner, clock)
	, m_exp(*this, "exp")
	, m_joy1(*this, "JOY1")
	, m_joy2(*this, "JOY2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_plusd_device::device_start()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_centronics_busy));
}

void spectrum_disciple_device::device_start()
{
	spectrum_plusd_device::device_start();
	save_item(NAME(m_map));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_plusd_device::device_reset()
{
	m_romcs = 0;
}

void spectrum_disciple_device::device_reset()
{
	spectrum_plusd_device::device_reset();
	m_map = false;
}

//**************************************************************************
//  IMPLEMENTATION  spectrum_plusd_device
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

	switch (offset & 0xff)
	{
	case 0xe3: // fdc status reg
	case 0xeb: // fdc track reg
	case 0xf3: // fdc sector reg
	case 0xfb: // fdc data reg
		data = m_fdc->read((offset >> 3) & 0x03);
		break;
	case 0xe7: // page in
		if (!machine().side_effects_disabled())
			m_romcs = 1;
		break;
	case 0xf7: // bit 7: printer busy
		data = m_centronics_busy ? 0x80 : 0x00;
		break;
	}
	
	return data;
}

void spectrum_plusd_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
	case 0xe3: // fdc command reg
	case 0xeb: // fdc track reg
	case 0xf3: // fdc sector reg
	case 0xfb: // fdc data reg
		m_fdc->write((offset >> 3) & 0x03, data);
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

//**************************************************************************
//  IMPLEMENTATION  spectrum_disciple_device
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

	switch (offset & 0xff)
	{
	case 0x1b: // fdc status reg
	case 0x5b: // fdc track reg
	case 0x9b: // fdc sector reg
	case 0xdb: // fdc data reg
		data = m_fdc->read((offset >> 6) & 0x03);
		break;
	case 0x1f: // bit 0-4: kempston joystick, bit 6: printer busy, bit 7: network
		data = bitswap<8>(~m_joy1->read(), 7, 6, 5, 0, 1, 2, 4, 3 ) & 0x1f;
		data |= m_centronics_busy ? 0x00 : 0x40;  // inverted by IC10 74ls240
		// 7: network...
		break;
	case 0x7b: // reset boot
		if (!machine().side_effects_disabled())
			m_map = false;
		break;
	case 0xbb: // page in
		if (!machine().side_effects_disabled())
			m_romcs = 1;
		break;
	case 0xfe: // sinclair joysticks
		if (((offset >> 8) & 16) == 0)
			data = m_joy1->read() | (0xff ^ 0x1f);
		if (((offset >> 8) & 8) == 0)
			data = m_joy2->read() | (0xff ^ 0x1f);
		break;
	}
	
	return data;
}

void spectrum_disciple_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
	case 0x1b: // fdc command reg
	case 0x5b: // fdc track reg
	case 0x9b: // fdc sector reg
	case 0xdb: // fdc data reg
		m_fdc->write((offset >> 6) & 0x03, data);
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
