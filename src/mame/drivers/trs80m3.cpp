// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Cassette baud rates:
        Model III/4 - 500 and 1500 baud selectable at boot time
        - When it says "Cass?" press L for 500 baud, or Enter otherwise.

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

F8:
- Write to printer (Model III/4)
- Read printer status (Model III/4)

EB:
- UART data (read and write) on a Model III/4

EA:
- UART status (read and write) on a Model III/4

E9:
- Set baud rate (Model III/4)

E8:
- UART Modem Status register (read) on a Model III/4
- UART Master Reset (write) on a Model III/4

Model 4 - C0-CF = hard drive (optional)
    - 90-93 write sound (optional)
    - 80-8F hires graphics (optional)

Shift and Right-arrow will enable 32 cpl.

About the RTC - The time is incremented while ever the cursor is flashing. It is stored in a series
    of bytes in the computer's work area. The bytes are in a certain order, this is:
    seconds, minutes, hours, year, day, month. The seconds are stored at 0x4217.
    A reboot always sets the time to zero.

Model 4 memory organisation -
    Mode 0: ROM=0-37E7 and 37EA-3FFF; Printer=37E8-37E9; Keyboard=3800-3BFF; Video=3C00-3FFF
    Mode 1: Keyboard and Video as above; 0-3FFF read=ROM and write=RAM
    Mode 2: Keyboard=F400-F7FF; Video=F800-FFFF; the rest is RAM
    Mode 3: All RAM
    The video is organised as 2 banks of 0x400 bytes, except in Mode 2 where it becomes contiguous.

Model 4P - is the same as Model 4 except:
    - ROM is only 0000-0FFF, while 1000-37FF is given over to RAM
    - There is no cassette support in hardware.

***************************************************************************


To Do / Status:
--------------

trs80m3:   works

trs80m4:   works
           will boot model 3 floppies, but not model 4 ones

trs80m4p:  floppy not working, so machine is useless

***************************************************************************/

#include "emu.h"
#include "includes/trs80m3.h"

#include "screen.h"
#include "speaker.h"

#include "formats/trs80_dsk.h"
#include "formats/dmk_dsk.h"



void trs80m3_state::m3_mem(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x37e8, 0x37e9).rw(FUNC(trs80m3_state::printer_r), FUNC(trs80m3_state::printer_w));
	map(0x3800, 0x3bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0xffff).ram();
}

void trs80m3_state::m3_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xe0, 0xe3).rw(FUNC(trs80m3_state::port_e0_r), FUNC(trs80m3_state::port_e0_w));
	map(0xe4, 0xe4).rw(FUNC(trs80m3_state::port_e4_r), FUNC(trs80m3_state::port_e4_w));
	map(0xe8, 0xe8).rw(FUNC(trs80m3_state::port_e8_r), FUNC(trs80m3_state::port_e8_w));
	map(0xe9, 0xe9).w(m_brg, FUNC(com8116_device::stt_str_w));
	map(0xea, 0xea).rw(FUNC(trs80m3_state::port_ea_r), FUNC(trs80m3_state::port_ea_w));
	map(0xeb, 0xeb).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xec, 0xef).rw(FUNC(trs80m3_state::port_ec_r), FUNC(trs80m3_state::port_ec_w));
	map(0xf0, 0xf0).r(FUNC(trs80m3_state::wd179x_r));
	map(0xf0, 0xf0).w(m_fdc, FUNC(fd1793_device::cmd_w));
	map(0xf1, 0xf1).rw(m_fdc, FUNC(fd1793_device::track_r), FUNC(fd1793_device::track_w));
	map(0xf2, 0xf2).rw(m_fdc, FUNC(fd1793_device::sector_r), FUNC(fd1793_device::sector_w));
	map(0xf3, 0xf3).rw(m_fdc, FUNC(fd1793_device::data_r), FUNC(fd1793_device::data_w));
	map(0xf4, 0xf7).w(FUNC(trs80m3_state::port_f4_w));
	map(0xf8, 0xfb).rw(FUNC(trs80m3_state::printer_r), FUNC(trs80m3_state::printer_w));
	map(0xfc, 0xff).rw(FUNC(trs80m3_state::port_ff_r), FUNC(trs80m3_state::port_ff_w));
}

void trs80m3_state::m4_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_m4_bank, FUNC(address_map_bank_device::amap8));
}

void trs80m3_state::m4_banked_mem(address_map &map)
{
	// Memory Map I - Model III Mode
	map(0x00000, 0x037ff).rom().region("maincpu", 0);
	map(0x037e8, 0x037e9).rw(FUNC(trs80m3_state::printer_r), FUNC(trs80m3_state::printer_w));
	map(0x03800, 0x03bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x03c00, 0x03fff).bankrw(m_vidbank);        // Video RAM (Page bit selects 1K of 2K)
	map(0x04000, 0x07fff).bankrw(m_16kbank);        // RAM
	map(0x08000, 0x0ffff).bankrw(m_32kbanks[1]);    // RAM

	// Memory Map II
	map(0x10000, 0x137ff).bankrw(m_32kbanks[0]);    // RAM (14K)
	map(0x13800, 0x13bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x13c00, 0x13fff).bankrw(m_vidbank);        // Video RAM
	map(0x14000, 0x17fff).bankrw(m_16kbank);        // RAM (16K)
	map(0x18000, 0x1ffff).bankrw(m_32kbanks[1]);    // RAM (32K)

	// Memory Map III
	map(0x20000, 0x27fff).bankrw(m_32kbanks[0]);    // RAM (32K)
	map(0x28000, 0x2f3ff).bankrw(m_32kbanks[1]);    // RAM (29K)
	map(0x2f400, 0x2f7ff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x2f800, 0x2ffff).ram().share(m_p_videoram);    // Video RAM

	// Memory Map IV
	map(0x30000, 0x37fff).bankrw(m_32kbanks[0]);    // RAM (32K)
	map(0x38000, 0x3ffff).bankrw(m_32kbanks[1]);    // RAM (32K)
}

void trs80m3_state::m4_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	m3_io(map);
	map(0x84, 0x87).w(FUNC(trs80m3_state::port_84_w));
	map(0x88, 0x89).w(FUNC(trs80m3_state::port_88_w));
	map(0x90, 0x93).w(FUNC(trs80m3_state::port_90_w));
}

void trs80m3_state::m4p_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_m4p_bank, FUNC(address_map_bank_device::amap8));
}

void trs80m3_state::m4p_banked_mem(address_map &map)
{
	// Memory Map I - Model III Mode
	map(0x00000, 0x00fff).rom().region("maincpu", 0);
	map(0x01000, 0x037ff).bankr(m_32kbanks[0]);      // readonly RAM
	map(0x037e8, 0x037e9).rw(FUNC(trs80m3_state::printer_r), FUNC(trs80m3_state::printer_w));
	map(0x03800, 0x03bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x03c00, 0x03fff).bankrw(m_vidbank);        // Video RAM (Page bit selects 1K of 2K)
	map(0x04000, 0x07fff).bankrw(m_16kbank);        // RAM
	map(0x08000, 0x0ffff).bankrw(m_32kbanks[1]);    // RAM
	// Memory Map II
	map(0x10000, 0x137ff).bankrw(m_32kbanks[0]);    // RAM
	map(0x10000, 0x10fff).rom().region("maincpu", 0);   // the ram under here is writeonly
	map(0x13800, 0x13bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x13c00, 0x13fff).bankrw(m_vidbank);        // Video RAM
	map(0x14000, 0x17fff).bankrw(m_16kbank);        // RAM (16K)
	map(0x18000, 0x1ffff).bankrw(m_32kbanks[1]);    // RAM (32K)
	// Memory Map III
	map(0x20000, 0x27fff).bankrw(m_32kbanks[0]);    // RAM (32K)
	map(0x28000, 0x2f3ff).bankrw(m_32kbanks[1]);    // RAM (29K)
	map(0x2f400, 0x2f7ff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x2f800, 0x2ffff).ram().share(m_p_videoram);    // Video RAM
	// Memory Map IV
	map(0x30000, 0x37fff).bankrw(m_32kbanks[0]);    // RAM (32K)
	map(0x38000, 0x3ffff).bankrw(m_32kbanks[1]);    // RAM (32K)
	// Map I with no rom
	map(0x40000, 0x437ff).bankr(m_32kbanks[0]);      // readonly RAM
	map(0x437e8, 0x437e9).rw(FUNC(trs80m3_state::printer_r), FUNC(trs80m3_state::printer_w));
	map(0x43800, 0x43bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x43c00, 0x43fff).bankrw(m_vidbank);        // Video RAM
	map(0x44000, 0x47fff).bankrw(m_16kbank);        // RAM (16K)
	map(0x48000, 0x4ffff).bankrw(m_32kbanks[1]);    // RAM (32K)
	// Map II with no rom
	map(0x50000, 0x537ff).bankrw(m_32kbanks[0]);    // RAM (32K)
	map(0x53800, 0x53bff).r(FUNC(trs80m3_state::keyboard_r));
	map(0x53c00, 0x53fff).bankrw(m_vidbank);        // Video RAM
	map(0x54000, 0x57fff).bankrw(m_16kbank);        // RAM (16K)
	map(0x58000, 0x5ffff).bankrw(m_32kbanks[1]);    // RAM (32K)
}

void trs80m3_state::m4p_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	m4_io(map);
	map(0x9c, 0x9f).w(FUNC(trs80m3_state::port_9c_w));
	map(0xf0, 0xf0).rw(m_fdc, FUNC(fd1793_device::status_r), FUNC(fd1793_device::cmd_w));
}

void trs80m3_state::cp500_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	m3_io(map);
	map(0xf4, 0xf7).r(FUNC(trs80m3_state::cp500_port_f4_r));
}


static INPUT_PORTS_START( trs80m4p )
	PORT_START("LINE0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("LINE1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("LINE2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("LINE3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0xF8, 0x00, IPT_UNUSED) // these bits were tested and do nothing useful

	PORT_START("LINE4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	/* backspace do the same as cursor left */
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	// These keys are only on a Model 4. These bits do nothing on Model 3.
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) // When activated, lowercase entry is possible
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) // prints tic character
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x80, 0x00, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( trs80m3 )
	PORT_INCLUDE( trs80m4p )
	PORT_START("CONFIG")
	PORT_CONFNAME(    0x80, 0x00,   "Floppy Disc Drives")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(   0x80, DEF_STR( On ) )
	PORT_BIT(0x7f, 0x7f, IPT_UNUSED)
INPUT_PORTS_END


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
static const gfx_layout trs80m3_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8        /* every char takes 8 bytes */
};

static GFXDECODE_START(gfx_trs80m3)
	GFXDECODE_ENTRY( "chargen", 0, trs80m3_charlayout, 0, 1 )
GFXDECODE_END


FLOPPY_FORMATS_MEMBER( trs80m3_state::floppy_formats )
	FLOPPY_TRS80_FORMAT,
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static void trs80_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_QD);
}


void trs80m3_state::model3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 20.2752_MHz_XTAL / 10); // FIXME: actual Model III XTAL is 10.1376 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m3_state::m3_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m3_state::m3_io);
	m_maincpu->set_periodic_int(FUNC(trs80m3_state::rtc_interrupt), attotime::from_hz(20.2752_MHz_XTAL / 10 / 67584));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12.672_MHz_XTAL, 800, 0, 640, 264, 0, 240); // FIXME: these are Model 4 80-column parameters
	screen.set_screen_update(FUNC(trs80m3_state::screen_update_trs80m3));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_trs80m3);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(trs80l2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "cmd", attotime::from_seconds(1)).set_load_callback(FUNC(trs80m3_state::quickload_cb), this);

	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(trs80m3_state::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(trs80m3_state::drq_w));

	// Internal drives
	FLOPPY_CONNECTOR(config, "fdc:0", trs80_floppies, "sssd", trs80m3_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", trs80_floppies, "sssd", trs80m3_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit6));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit5));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit4));

	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	COM8116(config, m_brg, 20.2752_MHz_XTAL / 4);   // BR1943 (or BR1941L)
	m_brg->fr_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_brg->ft_handler().set(m_uart, FUNC(ay31015_device::write_tcp));

	AY31015(config, m_uart);
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	//MCFG_AY31015_WRITE_DAV_CB(WRITELINE( , , ))
	m_uart->set_auto_rdav(true);
	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);
}

void trs80m3_state::model4(machine_config &config)
{
	model3(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m3_state::m4_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m3_state::m4_io);

	RAM(config, m_mainram, 0);
	m_mainram->set_default_size("64K");
	m_mainram->set_extra_options("16K,128K");

	ADDRESS_MAP_BANK(config, m_m4_bank, 0);
	m_m4_bank->set_map(&trs80m3_state::m4_banked_mem);
	m_m4_bank->set_endianness(ENDIANNESS_LITTLE);
	m_m4_bank->set_data_width(8);
	m_m4_bank->set_addr_width(18);
	m_m4_bank->set_stride(0x10000);
}

void trs80m3_state::model4p(machine_config &config)
{
	model3(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m3_state::m4p_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m3_state::m4p_io);

	RAM(config, m_mainram, 0);
	m_mainram->set_default_size("64K");
	m_mainram->set_extra_options("128K");

	ADDRESS_MAP_BANK(config, m_m4p_bank, 0);
	m_m4p_bank->set_map(&trs80m3_state::m4p_banked_mem);
	m_m4p_bank->set_endianness(ENDIANNESS_LITTLE);
	m_m4p_bank->set_data_width(8);
	m_m4p_bank->set_addr_width(19);
	m_m4p_bank->set_stride(0x10000);

	config.device_remove("quickload");
}

void trs80m3_state::cp500(machine_config &config)
{
	model3(config);
	m_maincpu->set_addrmap(AS_IO, &trs80m3_state::cp500_io);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(trs80m3)
/* ROMS we have and are missing:
HAVE    TRS-80 Model III Level 1 ROM (U104)
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC BBC4
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC DA75
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM A (U104) ver. CRC 9639
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM B (U105) ver. CRC 407C
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2B91 - early mfg. #80040316
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 278A - no production REV A
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2EF8 - Manufacturing #80040316 REV B
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C (U106) ver. CRC 2F84 - Manufacturing #80040316 REV C
MISSING TRS-80 Model III Level 2 (ENGLISH) ROM C ver. CRC 2764 - Network III v1
HAVE    TRS-80 Model III Level 2 (ENGLISH) ROM C ver. CRC 276A - Network III v2
MISSING TRS-80 Model III Level 2 (BELGIUM) CRC ????
Note: Be careful when dumping rom C: if dumped on the trs-80 m3 with software, bytes 0x7e8 and 0x7e9 (addresses 0x37e8, 0x0x37e9)
      will read as 0xFF 0xFF; on the original rom, these bytes are 0x00 0x00 (for eproms) or 0xAA 0xAA (for mask roms), those two bytes are used for printer status on the trs-80 and are mapped on top of the rom; This problem should be avoided by pulling the rom chips and dumping them directly.
*/
	ROM_REGION(0x3800, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "trs80m3_revc", "Level 2 bios, RomC Rev C")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(0)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(0)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117", (Level 2 bios ROM B '407c')
	ROMX_LOAD("8040316c.u106", 0x3000, 0x0800, CRC(c8f79433) SHA1(6f395bba822d39d3cd2b73c8ea25aab4c4c26da7), ROM_BIOS(0)) // Label: "SCM91692P // Tandy (c) 81 // 8040316-C // QQ8220" (Level 2 bios ROM C REV C '2f84')
	ROM_SYSTEM_BIOS(1, "trs80m3_revb", "Level 2 bios, RomC Rev B")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(1)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(1)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117", (Level 2 bios ROM B '407c')
	ROMX_LOAD("8040316b.u106", 0x3000, 0x0800, CRC(84a5702d) SHA1(297dca756a9d3c6fd13e0fa6f93d172ff795b520), ROM_BIOS(1)) // Label: "SCM91692P // Tandy (c) 80 // 8040316B // QQ8040" (Level 2 bios ROM C REV B '2ef8')
	ROM_SYSTEM_BIOS(2, "trs80m3_n3v2", "Level 2 bios, Network III v2 (student)")
	ROMX_LOAD("8041364.u104", 0x0000, 0x2000, CRC(ec0c6daa) SHA1(257cea6b9b46912d4681251019ec2b84f1b95fc8), ROM_BIOS(2)) // Label: "SCM91248C // Tandy (c) 80 // 8041364 // 8134" (Level 2 bios ROM A '9639')
	ROMX_LOAD("8040332.u105", 0x2000, 0x1000, CRC(ed4ee921) SHA1(ec0a19d4b72f71e51965de63250009c3c4e4cab3), ROM_BIOS(2)) // Label: "SCM91619P // Tandy (c) 80 // 8040332 // QQ8117" (Level 2 bios ROM B '407c')
	ROMX_LOAD("276a.u106", 0x3000, 0x0800, CRC(7d38720a) SHA1(bef621e5ae2a8c1f9e7f6325b7841f5ab8ab7e6a), ROM_BIOS(2)) // 2716 EPROM Label: "MOD.III // ROM C // (276A)" (Network III v2 ROM C '276a')
	ROM_SYSTEM_BIOS(3, "trs80m3_l1", "Level 1 bios")
	ROMX_LOAD("8040032.u104", 0x0000, 0x1000, CRC(6418d641) SHA1(f823ab6ceb102588d27e5f5c751e31175289291c), ROM_BIOS(3) ) // Label: "8040032 // (M) QQ8028 // SCM91616P"; Silkscreen: "TANDY // (C) '80"; (Level 1 bios)

	ROM_REGION(0x0800, "chargen",0)    /* correct for later systems; the trs80m3_l1 bios uses the non-a version of this rom, dump is pending */
	ROM_LOAD("8044316.u36", 0x0000, 0x0800, NO_DUMP) // Label: "(M) // SCM91665P // 8044316 // QQ8029" ('no-letter' revision)
	ROM_LOAD("8044316a.u36", 0x0000, 0x0800, CRC(444c8b60) SHA1(c52ee41439bd5e57c3b113ebfd61c951e2af4446)) // Label: "Tandy (C) 81 // 8044316A // 8206" (rev A)
ROM_END

// for model 4 and 4p info, see http://vt100.net/mirror/harte/Radio%20Shack/TRS-80%20Model%204_4P%20Soft%20Tech%20Ref.pdf
ROM_START(trs80m4)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("trs80m4.rom",  0x0000, 0x3800, BAD_DUMP CRC(1a92d54d) SHA1(752555fdd0ff23abc9f35c6e03d9d9b4c0e9677b)) // should be split into 3 roms, roms A, B, C, exactly like trs80m3; in fact, roms A and B are shared between both systems.

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("8044316a.u36", 0x0000, 0x0800, CRC(444c8b60) SHA1(c52ee41439bd5e57c3b113ebfd61c951e2af4446)) // according to parts catalog, this is the correct rom for both model 3 and 4
ROM_END

ROM_START(trs80m4p) // uses a completely different memory map scheme to the others; the trs-80 model 3 roms are loaded from a boot disk, the only rom on the machine is a bootloader; bootloader can be banked out of 0x0000-0x1000 space which is replaced with ram; see the tech ref pdf, pdf page 62
	ROM_REGION(0x3800, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "trs80m4p", "Level 2 bios, gate array machine")
	ROMX_LOAD("8075332.u69", 0x0000, 0x1000, CRC(3a738aa9) SHA1(6393396eaa10a84b9e9f0cf5930aba73defc5c52), ROM_BIOS(0)) // Label: "SCM95060P // 8075332 // TANDY (C) 1983 // 8421" at location U69 (may be located at U70 on some pcb revisions)
	ROM_SYSTEM_BIOS(1, "trs80m4p_hack", "Disk loader hack")
	ROMX_LOAD("trs80m4p_loader_hack.rom", 0x0000, 0x01f8, CRC(7ff336f4) SHA1(41184f5240b4b54f3804f5a22b4d78bbba52ed1d), ROM_BIOS(1))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("8049007.u103", 0x0000, 0x0800, CRC(1ac44bea) SHA1(c9426ab2b2aa5380dc97a7b9c048ccd1bbde92ca)) // Label: "SCM95987P // 8049007 // TANDY (C) 1983 // 8447" at location U103 (may be located at U43 on some pcb revisions)
ROM_END

ROM_START( cp500 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("s_8407_cn62516n_cp500a_prologica_83.ci111", 0x0000, 0x4000, CRC(c2fc1b92) SHA1(0eb07baee80f1ee1f28a609eb63a9245dcb68adb))

	ROM_REGION(0x4000, "bootrom", 0)
	ROM_LOAD("s_8407_cn62516n_cp500a_prologica_83.ci111", 0x0000, 0x4000, CRC(c2fc1b92) SHA1(0eb07baee80f1ee1f28a609eb63a9245dcb68adb))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "100.105.ci36", 0x0000, 0x800, CRC(1765931e) SHA1(49176ceea6cc003efa04fad2f31829b9432fe10f))
ROM_END

void trs80m3_state::init_trs80m3()
{
	m_model4 = 0;
}

void trs80m3_state::init_trs80m4()
{
	m_model4 = 2;
}

void trs80m3_state::init_trs80m4p()
{
	m_model4 = 4;
}


//    YEAR  NAME         PARENT    COMPAT    MACHINE   INPUT     CLASS          INIT             COMPANY               FULLNAME                FLAGS
COMP( 1980, trs80m3,     0,        trs80l2,  model3,   trs80m3,  trs80m3_state, init_trs80m3,  "Tandy Radio Shack", "TRS-80 Model III",        0 )
COMP( 1980, trs80m4,     trs80m3,  0,        model4,   trs80m3,  trs80m3_state, init_trs80m4,  "Tandy Radio Shack", "TRS-80 Model 4",          0 )
COMP( 1983, trs80m4p,    trs80m3,  0,        model4p,  trs80m4p, trs80m3_state, init_trs80m4p, "Tandy Radio Shack", "TRS-80 Model 4P",         MACHINE_NOT_WORKING )
COMP( 1982, cp500,       trs80m3,  0,        cp500,    trs80m3,  trs80m3_state, init_trs80m3,  "Prológica",         "CP-500 (PVIII REV.3)",    0 )
