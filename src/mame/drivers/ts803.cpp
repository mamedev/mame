// license:BSD-3-Clause
// copyright-holders: Gabriele D'Antona, Robbbert
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
#include "cpu/z80/z80daisy.h"
#include "machine/keyboard.h"
#include "machine/timer.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"
#include "machine/z80sti.h"
#include "video/mc6845.h"
#include "screen.h"


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

	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_READ8_MEMBER(porta0_r);
	DECLARE_WRITE8_MEMBER(porta0_w);
	DECLARE_WRITE8_MEMBER(disk_0_control_w);
	DECLARE_READ8_MEMBER(disk_0_control_r);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);
	DECLARE_WRITE8_MEMBER( crtc_controlreg_w );
	DECLARE_DRIVER_INIT(ts803);
	uint32_t screen_update_ts803(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ts803(machine_config &config);
	void ts803_io(address_map &map);
	void ts803_mem(address_map &map);
private:
	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_56kram;
	bool m_graphics_mode;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_region_ptr<u8> m_p_chargen;
	required_ioport m_io_dsw;
};

ADDRESS_MAP_START(ts803_state::ts803_mem)
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x4000, 0xbfff) AM_RAMBANK("bank4")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

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
ADDRESS_MAP_START(ts803_state::ts803_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x0f) AM_READ_PORT("DSW")
	AM_RANGE(0x10, 0x1f) AM_READWRITE(port10_r, port10_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE("sti", z80sti_device, read, write)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("dart", z80dart_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("fdc", fd1793_device, read, write)
	AM_RANGE(0x90, 0x9f) AM_READWRITE(disk_0_control_r,disk_0_control_w)
	AM_RANGE(0xa0, 0xbf) AM_READWRITE(porta0_r, porta0_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("crtc", sy6545_1_device, status_r, address_w)
	AM_RANGE(0xc2, 0xc2) AM_DEVREADWRITE("crtc", sy6545_1_device, register_r, register_w)
	AM_RANGE(0xc4, 0xc4) AM_WRITE(crtc_controlreg_w)
ADDRESS_MAP_END

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

static SLOT_INTERFACE_START( ts803_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

WRITE8_MEMBER( ts803_state::disk_0_control_w )
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

READ8_MEMBER( ts803_state::disk_0_control_r )
{
	printf("Disk0 control register read\n");
	return 0xff;
}

READ8_MEMBER( ts803_state::porta0_r)
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

WRITE8_MEMBER( ts803_state::porta0_w )
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

READ8_MEMBER( ts803_state::port10_r )
{
	offset += 0x10;
	printf("Port read [%x]\n",offset);

	return 0xff;
}

WRITE8_MEMBER( ts803_state::port10_w )
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
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx,inv;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (rv ^ (x == cursor_x)) ? 0xff : 0;

		if (m_graphics_mode)
		{
			mem = (ra*0x2000 + ma + x) & 0x7fff;
			gfx = m_videoram[mem] ^ inv;
		}
		else
		{
			mem = 0x1800 + ((ma + x) & 0x7ff);
			chr = m_videoram[mem];
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

WRITE8_MEMBER( ts803_state::crtc_controlreg_w )
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
	save_pointer(NAME(m_videoram.get()), 0x8000);
	save_pointer(NAME(m_56kram.get()), 0xc000);
}

void ts803_state::machine_reset()
{
	m_graphics_mode = false;

	membank("bankr0")->set_entry(0);
	membank("bankw0")->set_entry(0);
	membank("bank4")->set_entry(0);
}

DRIVER_INIT_MEMBER( ts803_state, ts803 )
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

MACHINE_CONFIG_START(ts803_state::ts803)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(16'000'000)/4)
	MCFG_CPU_PROGRAM_MAP(ts803_mem)
	MCFG_CPU_IO_MAP(ts803_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640,240)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", sy6545_1_device, screen_update)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* crtc */
	MCFG_MC6845_ADD("crtc", SY6545_1, "screen", 13608000 / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(ts803_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(ts803_state, crtc_update_addr)

	MCFG_DEVICE_ADD("sti_clock", CLOCK, XTAL(16'000'000) / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sti", z80sti_device, tc_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sti", z80sti_device, rc_w))

	MCFG_DEVICE_ADD("dart_clock", CLOCK, (XTAL(16'000'000) / 13) / 8)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("dart", z80dart_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart", z80dart_device, rxca_w))

	MCFG_DEVICE_ADD("sti", Z80STI, XTAL(16'000'000)/4)
	MCFG_Z80STI_OUT_TBO_CB(DEVWRITELINE("dart", z80dart_device, rxtxcb_w))
	MCFG_Z80STI_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("dart", Z80DART, XTAL(16'000'000) / 4)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("dart", z80dart_device, rxa_w))

	/* floppy disk */
	MCFG_FD1793_ADD("fdc", XTAL(1'000'000))
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("sti", z80sti_device, i7_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ts803_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ts803_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts803h )
	ROM_REGION(0x4000, "roms", ROMREGION_ERASEFF) // includes space for optional expansion rom
	ROM_LOAD( "180001-37 rev d 803 5 23 84.a57", 0x0000, 0x2000, CRC(0aa658a7) SHA1(42d0a89c2ff9b6588cd88bdb1f800fac540dccbb) )

	ROM_REGION(0x0100, "proms", 0)
	ROM_LOAD( "8000134.a59", 0x0000, 0x0100, CRC(231fe6d6) SHA1(3c052ba4b74547e0e2451fa1ae67bbcb83a18bab) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "803h_vid.a119", 0x0000, 0x0800, CRC(d5ce2814) SHA1(ce527479464757223dffac384a85ab74b174952c) )
ROM_END



//   YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS        INIT    COMPANY     FULLNAME  FLAGS
COMP(1983, ts803h,  0,      0,      ts803,     ts803, ts803_state, ts803, "Televideo", "TS803H", MACHINE_NOT_WORKING )
