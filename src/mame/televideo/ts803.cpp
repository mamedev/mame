// license:BSD-3-Clause
// copyright-holders: Gabriele D'Antona
/*************************************************************************************************

TeleVideo TS-803(H)
Driver by friol
Started: 27/12/2015

Addressable memory space has 2 configurations:

PAGE SEL bit in PORT0 set to 0:

  0000-2000 - 8k rom
  2000-4000 - 8k expansion rom
  4000-BFFF - videoram
  C000-FFFF - 16k cpu RAM

PAGE SEL bit in PORT0 set to 1:

  0000-dFFF - 56k ram
  e000-FFFF - OS RAM

  Z80STI:
  - provides baud-rate clock for DART ch B (serial printer)
  - merges FDC and HDC irq into the daisy chain. FDC works without it though.

  Keyboard / Z80DART:
  - Keyboard has 8048 plus undumped rom. There's no schematic.
  - Protocol: 9600 baud, 8 data bits, 2 stop bits, no parity, no handshaking.
  - Problem: every 2nd keystroke is ignored. The bios does this on purpose.
  - Problem: some of the disks cause the keyboard to produce rubbish, unable to find
             a baud rate that works.

  To Do:
  - Fix weird problem with keyboard, or better, get the rom and emulate it.
  - Hard Drive
  - Videoram has 4 banks, only first bank is currently used for display
  - Option of a further 64k of main ram.
  - 2 more Serial ports (Serial Printer & Mouse; Modem)
  - Optional RS422 board (has Z80SIO +others)
  - Diagnostic LEDs
  - Cleanup
  - The demo refers to video attributes, but there's no mention of them anywhere else

**************************************************************************************************/

#include "emu.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/keyboard.h"
#include "machine/timer.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "machine/z80sti.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ts803_state : public driver_device
{
public:
	ts803_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_p_chargen(*this, "chargen")
		, m_io_dsw(*this, "DSW")
	{ }

	void ts803(machine_config &config);

	void init_ts803();

private:
	uint8_t port10_r(offs_t offset);
	void port10_w(offs_t offset, uint8_t data);
	uint8_t porta0_r(offs_t offset);
	void porta0_w(offs_t offset, uint8_t data);
	void disk_0_control_w(uint8_t data);
	uint8_t disk_0_control_r();
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);
	void crtc_controlreg_w(uint8_t data);
	uint32_t screen_update_ts803(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ts803_io(address_map &map) ATTR_COLD;
	void ts803_mem(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_56kram;
	bool m_graphics_mode;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_region_ptr<u8> m_p_chargen;
	required_ioport m_io_dsw;
};

void ts803_state::ts803_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bankr0").bankw("bankw0");
	map(0x4000, 0xbfff).bankrw("bank4");
	map(0xc000, 0xffff).ram();
}

/*

I/0 Port Addresses

System Status Switch 1                                  00 (SW CE)
Diagnostic Indicators 1 and 2                           10 (10-1F = CTRL PORT CE)
Diagnostic Indicators 3 and 4                           11
RS-422 Control and Auto Wait                            12
Memory Bank Select                                      13
STI Device (modem)                                      20-2F (STI CE)
DART Dual Asynchronous Receiver Transmitter
Device (keyboard, printer, mouse)                       30-33 (DART CE)
RS-422 SIO Device                                       40-43 (I/O CE1)
                                                        5x (I/O CE2)
                                                        6x (PAR C/S CE)
                                                        7x (PAR DATA CE)
Floppy Disk Controller                                  80-83 (FDC CE)
Floppy Disk Drive Decoder                               90 (FDD CE)
Winchester Disk Controller Reset                        A0 (WDC RST CE)
Winchester Disk Controller                              B0-BF (WDC CE)
Graphics Controller                                     C0-CF (GIO SEL)

*/
void ts803_state::ts803_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x0f).portr("DSW");
	map(0x10, 0x1f).rw(FUNC(ts803_state::port10_r), FUNC(ts803_state::port10_w));
	map(0x20, 0x2f).rw("sti", FUNC(z80sti_device::read), FUNC(z80sti_device::write));
	map(0x30, 0x33).rw("dart", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0x80, 0x83).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x90, 0x9f).rw(FUNC(ts803_state::disk_0_control_r), FUNC(ts803_state::disk_0_control_w));
	map(0xa0, 0xbf).rw(FUNC(ts803_state::porta0_r), FUNC(ts803_state::porta0_w));
	map(0xc0, 0xc0).rw("crtc", FUNC(sy6545_1_device::status_r), FUNC(sy6545_1_device::address_w));
	map(0xc2, 0xc2).rw("crtc", FUNC(sy6545_1_device::register_r), FUNC(sy6545_1_device::register_w));
	map(0xc4, 0xc4).w(FUNC(ts803_state::crtc_controlreg_w));
}

/* Input ports */
static INPUT_PORTS_START( ts803 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Printer Baud" ) PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "150" )
	PORT_DIPSETTING(    0x07, "75" )
	PORT_DIPNAME( 0x08, 0x00, "Model" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x00, "TS803" )
	PORT_DIPSETTING(    0x08, "TS803H" )
	PORT_DIPNAME( 0x10, 0x00, "Model specific #1" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, "On - TS803 leave here, TS803H Local" )
	PORT_DIPSETTING(    0x10, "Off - TS803H remote" )
	PORT_DIPNAME( 0x20, 0x00, "Model specific #2" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, "On - TS803 leave here, TS803H 2-head" )
	PORT_DIPSETTING(    0x20, "Off - TS803H 4-head" )
	PORT_DIPNAME( 0x40, 0x00, "Mains frequency" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "60 Hz" )
	PORT_DIPSETTING(    0x40, "50 Hz" )
	PORT_DIPNAME( 0x80, 0x80, "Model specific #3" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0x80, "Off - TS803H leave here" )
	PORT_DIPNAME( 0x100,0x100, "Reverse Video" ) PORT_DIPLOCATION("SW:9")
	PORT_DIPSETTING(    0x000, "Reverse" )
	PORT_DIPSETTING(    0x100, "Normal" )
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_UNUSED ) // SW10 is not connected to anything
INPUT_PORTS_END

/* disk drive */

static void ts803_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void ts803_state::disk_0_control_w(uint8_t data)
{
/*
d0 ready
d1 motor on
d2 Side select (active low)
d3 Double density (active low)
d4 Drive select 0 (active low)
d5 Drive select 1 (active low)
d6 Drive select 2 (active low)
d7 Drive select 3 (active low)
*/
	if ((data & 0xc0)!=0xc0) return;
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 4)==0)
		floppy = m_floppy0->get_device();
	else
	if (BIT(data, 5)==0)
		floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(BIT(data, 1));
		floppy->ss_w(BIT(data, 2) ? 0: 1);
	}
	m_fdc->dden_w(BIT(data, 3));
}

uint8_t ts803_state::disk_0_control_r()
{
	printf("Disk0 control register read\n");
	return 0xff;
}

uint8_t ts803_state::porta0_r(offs_t offset)
{
	offset += 0xa0;
	switch(offset)
	{
		case 0xb2:
			printf("B2 WDC read\n");
			return 0x11;

		case 0xb3:
			printf("B3 WDC read\n");
			return 0x15;

		case 0xb4:
			printf("B4 WDC read\n");
			return 0x55;

		case 0xb5:
			printf("B5 WDC read\n");
			return 0x01;

		case 0xb6:
			printf("B6 WDC read\n");
			return 0x25;

	}

	return 0x00;
}

void ts803_state::porta0_w(offs_t offset, uint8_t data)
{
	offset += 0xa0;
	switch (offset)
	{
		case 0xc4:
			//printf("Control Register for Alpha or Graphics Mode Selection\n");
			break;

		default:
			break;
	}
}

uint8_t ts803_state::port10_r(offs_t offset)
{
	offset += 0x10;
	printf("Port read [%x]\n",offset);

	return 0xff;
}

void ts803_state::port10_w(offs_t offset, uint8_t data)
{
	offset += 0x10;
	switch (offset)
	{
		case 0x10:
			data &= 3;
			printf("Writing to diagnostic indicators 1 & 2: %X\n", data);
			break;

		case 0x11:
			data &= 3;
			printf("Writing to diagnostic indicators 3 & 4: %X\n", data);
			break;

		case 0x12:
			data &= 3;
			printf("RS-422 control: %X\n", data);
			break;

		case 0x13:
			data &= 3;
			if (BIT(data, 1)==0)
			{
				membank("bankr0")->set_entry(BIT(data, 0));
				membank("bank4")->set_entry(BIT(data, 0));
			}
			else
				printf("Error: unknown memory config: %X.\n", data);

			break;

		default:
			printf("unknown port [%2.2x] write of [%2.2x]\n",offset,data);
			break;
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED( ts803_state::crtc_update_addr )
{
	//printf("CRTC::address update [%x]\n",address);
}

MC6845_UPDATE_ROW( ts803_state::crtc_update_row )
{
	bool rv = BIT(m_io_dsw->read(), 8) ? 0 : 1;
	rgb_t const *const pens = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint8_t inv = (rv ^ (x == cursor_x)) ? 0xff : 0;

		uint8_t gfx;
		if (m_graphics_mode)
		{
			uint16_t mem = (ra*0x2000 + ma + x) & 0x7fff;
			gfx = m_videoram[mem] ^ inv;
		}
		else
		{
			uint16_t mem = 0x1800 + ((ma + x) & 0x7ff);
			uint8_t chr = m_videoram[mem];
			gfx = (ra > 7) ? inv : m_p_chargen[(chr<<3) | ((ra+1)&7)] ^ inv;
		}

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}

void ts803_state::crtc_controlreg_w(uint8_t data)
{
/*
Bit 0 = 0 alpha mode
                1 graphics mode
Bit 1 = 0 page 1 (alpha mode only)
                1 page 2 (alpha mode only)
Bit 2 = 0 alpha memory access (round off)
                1 graphics memory access (normal CPU address)
*/

	//printf("CRTC::c4 write [%2x]\n",data);
	m_graphics_mode = BIT(data, 0);
}

void ts803_state::machine_start()
{
	//save these 2 so we can examine them in the debugger
	save_pointer(NAME(m_videoram), 0x8000);
	save_pointer(NAME(m_56kram), 0xc000);
}

void ts803_state::machine_reset()
{
	m_graphics_mode = false;

	membank("bankr0")->set_entry(0);
	membank("bankw0")->set_entry(0);
	membank("bank4")->set_entry(0);
}

void ts803_state::init_ts803()
{
	m_videoram = std::make_unique<uint8_t[]>(0x8000);
	m_56kram = std::make_unique<uint8_t[]>(0xc000);

	uint8_t *rom = memregion("roms")->base();
	membank("bankr0")->configure_entry(0, &rom[0]); // rom
	membank("bankr0")->configure_entry(1, m_56kram.get()); // ram
	membank("bankw0")->configure_entry(0, m_56kram.get()); // ram
	membank("bank4")->configure_entry(0, m_videoram.get()); // vram
	membank("bank4")->configure_entry(1, m_56kram.get()+0x4000); // ram
}

/* Interrupt priority:
0: RS-422 option board (highest priority)
1: Z80A DART (RS-232C serial I/O)
2: Z80 STI (RS 232C modem port)
3: FD 1793 floppy disk controller
4: Winchester disk controller board
5: Time-of-day clock (no info in the manual about this)
Interrupts 0 through 2 are prioritized in a daisy-chain
arrangement. Interrupts 3 through 5 are wired to the Z80 STI
interrupt input pins. */
static const z80_daisy_config daisy_chain[] =
{
	//{ "rs422" }, // not emulated
	{ "dart" },
	{ "sti" },
	{ nullptr }
};

void ts803_state::ts803(machine_config &config)
{
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ts803_state::ts803_mem);
	m_maincpu->set_addrmap(AS_IO, &ts803_state::ts803_io);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_size(640,240);
	screen.set_visarea(0, 640-1, 0, 240-1);
	screen.set_screen_update("crtc", FUNC(sy6545_1_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* crtc */
	sy6545_1_device &crtc(SY6545_1(config, "crtc", 13608000 / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(ts803_state::crtc_update_row));
	crtc.set_on_update_addr_change_callback(FUNC(ts803_state::crtc_update_addr));

	clock_device &sti_clock(CLOCK(config, "sti_clock", 16_MHz_XTAL / 13));
	sti_clock.signal_handler().set("sti", FUNC(z80sti_device::tc_w));
	sti_clock.signal_handler().append("sti", FUNC(z80sti_device::rc_w));

	clock_device &dart_clock(CLOCK(config, "dart_clock", 16_MHz_XTAL / 13 / 8));
	dart_clock.signal_handler().set("dart", FUNC(z80dart_device::txca_w));
	dart_clock.signal_handler().append("dart", FUNC(z80dart_device::rxca_w));

	z80sti_device& sti(Z80STI(config, "sti", 16_MHz_XTAL / 4));
	sti.out_tbo_cb().set("dart", FUNC(z80dart_device::rxtxcb_w));
	sti.out_int_cb().set_inputline("maincpu", INPUT_LINE_IRQ0);

	z80dart_device& dart(Z80DART(config, "dart", 16_MHz_XTAL / 4));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dart.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set("dart", FUNC(z80dart_device::rxa_w));

	/* floppy disk */
	FD1793(config, m_fdc, 1_MHz_XTAL);
	m_fdc->intrq_wr_callback().set("sti", FUNC(z80sti_device::i7_w));
	FLOPPY_CONNECTOR(config, "fdc:0", ts803_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", ts803_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( ts803h )
	ROM_REGION(0x4000, "roms", ROMREGION_ERASEFF) // includes space for optional expansion rom
	ROM_LOAD( "180001-37 rev d 803 5 23 84.a57", 0x0000, 0x2000, CRC(0aa658a7) SHA1(42d0a89c2ff9b6588cd88bdb1f800fac540dccbb) )

	ROM_REGION(0x0100, "proms", 0)
	ROM_LOAD( "8000134.a59", 0x0000, 0x0100, CRC(231fe6d6) SHA1(3c052ba4b74547e0e2451fa1ae67bbcb83a18bab) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "803h_vid.a119", 0x0000, 0x0800, CRC(d5ce2814) SHA1(ce527479464757223dffac384a85ab74b174952c) )
ROM_END

} // anonymous namespace


//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP(1983, ts803h, 0,      0,      ts803,   ts803, ts803_state, init_ts803, "Televideo", "TS803H", MACHINE_NOT_WORKING )
