// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/***************************************************************************

    Osborne-1 driver file

The Osborne-1 memory is divided into 3 "banks".

Bank 1 simply consists of 64KB of RAM. The upper 4KB is used for the lower 8
bit of video RAM entries.

Bank 2 holds the BIOS ROM and I/O area. Only addresses 0000-3FFF are used
by bank 2 (4000-FFFF mirrors bank 1). Bank 2 is divided as follows:
3000-3FFF Nominally unused but acts as mirror of 2000-2FFF
2C00-2C03 Video PIA
2A00-2A01 Serial interface
2900-2903 488 PIA
2400-2400 SCREEN-PAC (if present)
2201-2280 Keyboard
2100-2103 Floppy
1000-1FFF Nominally unused but acts as read mirror of BIOS ROM
0000-0FFF BIOS ROM

The logic is actually quite sloppy, and will cause bus fighting under many
circumstances since it doesn't actually check all four bits, just that two
are in the desired state.

Bank 3 has the ninth bit needed to complete the full Video RAM. These bits
are stored at F000-FFFF. Only the highest bit is used.

On bootup bank 2 is active.

Banking is controlled by writes to I/O space.  Only two low address bits are
used, and the value on the data bus is completley ignored.
00 - Activate bank 2 (also triggered by CPU reset)
01 - Activate bank 1
02 - Set BIT 9 signal (map bank 3 into F000-FFFF)
03 - Clear BIT 9 signal (map bank 1/2 into F000-FFFF)

Selecting between bank 1 and bank 2 is also affected by M1 and IRQACK
conditions using a set of three flipflops.

The serial speed configuration implements wiring changes recommended in the
Osborne 1 Technical Manual.  There's no way for software to read the
selected baud rates, so it will always call the low speed "300" and the high
speed "1200".  You as the user have to keep this in mind using the system.

Serial communications can be flaky when 600/2400 is selected.  This is not a
bug in MAME.  I've checked and double-checked the schematics to confirm it's
an original bug.  The division ratio from the master clock to the baud rates
in this mode is effectively 16*24*64 or 16*24*16 giving actual data rates of
650 baud or 2600 baud, about 8.3% too fast (16*26*64 and 16*26*16 would give
the correct rates).  MAME's bitbanger seems to be able to accept the ACIA
output at this rate, but the ACIA screws up when consuming data from MAME's
bitbanger.

Schematics specify a WD1793 floppy controller, but we're using the Fujitsu
equivalent MB8877 here.  Is it known that the original machines used one or
the other exclusively?  In any case MAME emulates them identically.

Installation of the SCREEN-PAC requires the CPU and character generator ROM
to be transplanted to the add-on board, and cables run to the sockets that
previously held these chips.  It contains additional RAM clocked at twice
the speed of the main system RAM.  Writes to video memory get sent to this
RAM as well as the main system RAM, so there are actually two live copies
of video RAM at all times.  The SCREEN-PAC supports switching between
normal and double horizontal resolution (52x24 or 104x24) at exactly 60Hz.

The Nuevo Video board also requires the CPU to be transplanted to it and has
a pair of RAMs holding a copy of video memory.  However it has its own
character generator ROM, so the mainboard's character generator ROM doesn't
need to be moved.  However, it doesn't behave like the SCREEN-PAC.  It uses
a Synertek SY6545-1 with its pixel clock derived from a 12.288MHz crystal
mapped at 0x04/0x05 in I/O space.  It runs at 640x240 (80x24) at just below
60Hz and doesn't allow resolution switching.  We don't know how contention
for video RAM is handled, or whether the CRTC can generate VBL interrupts.


TODO:

* Hook up the port direction control bits in the IEEE488 interface properly
  and test it with some emulated peripheral.  Also the BIOS can speak
  Centronics parallel over the same physical interface, so this should be
  tested, too.

* Complete emulation of the Nuevo Video board (interrupts, CRTC video RAM
  updates).  It would be nice to get a schematic for this.

***************************************************************************/

#include "emu.h"
#include "includes/osborne1.h"

#include "bus/rs232/rs232.h"
#include "speaker.h"

#include "softlist.h"


static constexpr XTAL MAIN_CLOCK = 15.9744_MHz_XTAL;


void osborne1_state::osborne1_mem(address_map &map)
{
	map(0x0000, 0x0FFF).bankr(m_bank_0xxx).w(FUNC(osborne1_state::bank_0xxx_w));
	map(0x1000, 0x1FFF).bankr(m_bank_1xxx).w(FUNC(osborne1_state::bank_1xxx_w));
	map(0x2000, 0x3FFF).rw(FUNC(osborne1_state::bank_2xxx_3xxx_r), FUNC(osborne1_state::bank_2xxx_3xxx_w));
	map(0x4000, 0xEFFF).ram();
	map(0xF000, 0xFFFF).bankr(m_bank_fxxx).w(FUNC(osborne1_state::videoram_w));
}


void osborne1_state::osborne1_op(address_map &map)
{
	map(0x0000, 0xFFFF).r(FUNC(osborne1_state::opcode_r));
}


void osborne1_state::osborne1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x03).mirror(0xfc).w(FUNC(osborne1_state::bankswitch_w));
}

void osborne1_state::osborne1nv_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map( 0x00, 0x03 ).w(FUNC(osborne1_state::bankswitch_w));
	map( 0x04, 0x04 ).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map( 0x05, 0x05 ).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	// seems to be something at 0x06 as well, but no idea what - BIOS writes 0x07 on boot
}


static INPUT_PORTS_START( osborne1 )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)       PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE PORT_NAME("Alpha Lock")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, osborne1_state, reset_key, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CNF")
	PORT_BIT(0xF8, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x06, 0x00, "Serial Speed")
	PORT_CONFSETTING(0x00, "300/1200")
	PORT_CONFSETTING(0x02, "600/2400")
	PORT_CONFSETTING(0x04, "1200/4800")
	PORT_CONFSETTING(0x06, "2400/9600")
	PORT_CONFNAME(0x01, 0x00, "Video Output")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x01, "SCREEN-PAC")
INPUT_PORTS_END

INPUT_PORTS_START( osborne1nv )
	PORT_INCLUDE(osborne1)

	PORT_MODIFY("CNF")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


/*
 * The Osborne-1 supports the following disc formats:
 * - Osborne single density: 40 tracks, 10 sectors per track, 256-byte sectors (100 KByte)
 * - Osborne double density: 40 tracks, 5 sectors per track, 1024-byte sectors (200 KByte)
 * - IBM Personal Computer: 40 tracks, 8 sectors per track, 512-byte sectors (160 KByte)
 * - Xerox 820 Computer: 40 tracks, 18 sectors per track, 128-byte sectors (90 KByte)
 * - DEC 1820 double density: 40 tracks, 9 sectors per track, 512-byte sectors (180 KByte)
 *
 */

static void osborne1_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD); // Siemens FDD 100-5, custom Osborne electronics
	device.option_add("525ssdd", FLOPPY_525_QD); // SSDD) // MPI 52(?), custom Osborne electronics
}


/* F4 Character Displayer */
static const gfx_layout osborne1_charlayout =
{
	8, 10,              // 8 x 10 characters
	128,                // 128 characters
	1,                  // 1 bits per pixel
	{ 0 },              // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*128*8, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8 },
	8                   // every char takes 16 x 1 bytes
};

static GFXDECODE_START( gfx_osborne1 )
	GFXDECODE_ENTRY("chargen", 0x0000, osborne1_charlayout, 0, 1)
GFXDECODE_END


void osborne1_state::osborne1(machine_config &config)
{
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &osborne1_state::osborne1_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &osborne1_state::osborne1_op);
	m_maincpu->set_addrmap(AS_IO, &osborne1_state::osborne1_io);
	m_maincpu->irqack_cb().set(FUNC(osborne1_state::irqack_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(osborne1_state::screen_update));
	m_screen->set_raw(MAIN_CLOCK, 1024, 0, 104*8, 260, 0, 24*10);
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_osborne1);
	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 1.00);

	PIA6821(config, m_pia0);
	m_pia0->readpa_handler().set(m_ieee, FUNC(ieee488_device::dio_r));
	m_pia0->readpb_handler().set(FUNC(osborne1_state::ieee_pia_pb_r));
	m_pia0->writepa_handler().set(m_ieee, FUNC(ieee488_device::host_dio_w));
	m_pia0->writepb_handler().set(FUNC(osborne1_state::ieee_pia_pb_w));
	m_pia0->ca2_handler().set(m_ieee, FUNC(ieee488_device::host_ifc_w));
	m_pia0->cb2_handler().set(m_ieee, FUNC(ieee488_device::host_ren_w));
	m_pia0->irqa_handler().set(FUNC(osborne1_state::ieee_pia_irq_a_func));

	IEEE488(config, m_ieee, 0);
	m_ieee->srq_callback().set(m_pia0, FUNC(pia6821_device::ca2_w));

	PIA6821(config, m_pia1);
	m_pia1->writepa_handler().set(FUNC(osborne1_state::video_pia_port_a_w));
	m_pia1->writepb_handler().set(FUNC(osborne1_state::video_pia_port_b_w));
	m_pia1->cb2_handler().set(FUNC(osborne1_state::video_pia_out_cb2_dummy));
	m_pia1->irqa_handler().set(FUNC(osborne1_state::video_pia_irq_a_func));

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(FUNC(osborne1_state::serial_acia_irq_func));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.ri_handler().set(m_pia1, FUNC(pia6821_device::ca2_w));

	MB8877(config, m_fdc, MAIN_CLOCK/16);
	m_fdc->set_force_ready(true);
	FLOPPY_CONNECTOR(config, m_floppy0, osborne1_floppies, "525ssdd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, osborne1_floppies, "525ssdd", floppy_image_device::default_floppy_formats);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("68K"); // 64kB main RAM and 4kbit video attribute RAM

	SOFTWARE_LIST(config, "flop_list").set_original("osborne1");
}

void osborne1nv_state::osborne1nv(machine_config &config)
{
	osborne1(config);
	m_maincpu->set_addrmap(AS_IO, &osborne1nv_state::osborne1nv_io);

	m_screen->set_no_palette();
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	sy6545_1_device &crtc(SY6545_1(config, "crtc", XTAL(12'288'000)/8));
	crtc.set_screen(m_screen);
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(osborne1nv_state::crtc_update_row));
	crtc.set_on_update_addr_change_callback(FUNC(osborne1nv_state::crtc_update_addr_changed));
}


ROM_START( osborne1 )
	ROM_DEFAULT_BIOS("ver144")
	ROM_SYSTEM_BIOS( 0, "vera",   "BIOS version A" )
	ROM_SYSTEM_BIOS( 1, "ver12",  "BIOS version 1.2" )
	ROM_SYSTEM_BIOS( 2, "ver121", "BIOS version 1.2.1" )
	ROM_SYSTEM_BIOS( 3, "ver13",  "BIOS version 1.3" )
	ROM_SYSTEM_BIOS( 4, "ver14",  "BIOS version 1.4" )
	ROM_SYSTEM_BIOS( 5, "ver143", "BIOS version 1.43" )
	ROM_SYSTEM_BIOS( 6, "ver144", "BIOS version 1.44" )

	ROM_REGION( 0x1000, "maincpu", 0 )
	ROMX_LOAD( "osba.bin",               0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(0) )
	ROMX_LOAD( "osb12.bin",              0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(1) )
	ROMX_LOAD( "osb121.bin",             0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(2) )
	ROMX_LOAD( "osb13.bin",              0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(3) )
	ROMX_LOAD( "rev1.40.ud11",           0x0000, 0x1000, CRC(3d966335) SHA1(0c60b97a3154a75868efc6370d26995eadc7d927), ROM_BIOS(4) )
	ROMX_LOAD( "rev1.43.ud11",           0x0000, 0x1000, CRC(91a48e3c) SHA1(c37b83f278d21e6e92d80f9c057b11f7f22d88d4), ROM_BIOS(5) )
	ROMX_LOAD( "3a10082-00rev-e.ud11",   0x0000, 0x1000, CRC(c0596b14) SHA1(ee6a9cc9be3ddc5949d3379351c1d58a175ce9ac), ROM_BIOS(6) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(0) ) // this is CHRROM from v1.4 BIOS MB
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(1) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(2) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(3) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(4) )
	ROMX_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801), ROM_BIOS(5) )
	ROMX_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801), ROM_BIOS(6) )
ROM_END

ROM_START( osborne1nv )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "monrom-rev1.51-12.ud11", 0x0000, 0x1000, CRC(298da402) SHA1(7fedd070936ccfe98f96d6e0ac71689666da79cb) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801) )

	ROM_REGION( 0x0800, "nuevo", 0 )
	ROM_LOAD( "character_generator_6-29-84.14", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801) )
ROM_END

//    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       CLASS             INIT           COMPANY          FULLNAME                   FLAGS
COMP( 1981, osborne1,   0,        0,      osborne1,   osborne1,   osborne1_state,   init_osborne1, "Osborne",       "Osborne-1",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, osborne1nv, osborne1, 0,      osborne1nv, osborne1nv, osborne1nv_state, init_osborne1, "Osborne/Nuevo", "Osborne-1 (Nuevo Video)", MACHINE_SUPPORTS_SAVE )
