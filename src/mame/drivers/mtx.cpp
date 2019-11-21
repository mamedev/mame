// license:BSD-3-Clause
// copyright-holders:Lee Ward, Dirk Best, Curt Coder
/*************************************************************************

    Memotech MTX 500, MTX 512 and RS 128

**************************************************************************/

/*

    TODO:

    - FDX floppy
    - HDX hard disk
    - HRX high resolution graphics
    - "Silicon" disks
    - Multi Effect Video Wall

 */

#include "emu.h"
#include "includes/mtx.h"

#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "video/tms9928a.h"

#include "softlist.h"
#include "speaker.h"


/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( mtx_mem )
-------------------------------------------------*/

void mtx_state::mtx_mem(address_map &map)
{
}

/*-------------------------------------------------
    ADDRESS_MAP( mtx_io )
-------------------------------------------------*/

void mtx_state::mtx_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(mtx_state::mtx_strobe_r), FUNC(mtx_state::mtx_bankswitch_w));
	map(0x01, 0x01).rw("tms9929a", FUNC(tms9929a_device::vram_read), FUNC(tms9929a_device::vram_write));
	map(0x02, 0x02).rw("tms9929a", FUNC(tms9929a_device::register_read), FUNC(tms9929a_device::register_write));
	map(0x03, 0x03).rw(FUNC(mtx_state::mtx_sound_strobe_r), FUNC(mtx_state::mtx_cst_w));
	map(0x04, 0x04).r(FUNC(mtx_state::mtx_prt_r)).w("cent_data_out", FUNC(output_latch_device::bus_w));
	map(0x05, 0x05).rw(FUNC(mtx_state::mtx_key_lo_r), FUNC(mtx_state::mtx_sense_w));
	map(0x06, 0x06).rw(FUNC(mtx_state::mtx_key_hi_r), FUNC(mtx_state::mtx_sound_latch_w));
	//  map(0x07, 0x07) PIO
	map(0x08, 0x0b).rw(m_z80ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1f, 0x1f).w(FUNC(mtx_state::mtx_cst_motor_w));
	map(0x30, 0x31).w(FUNC(mtx_state::hrx_address_w));
	map(0x32, 0x32).rw(FUNC(mtx_state::hrx_data_r), FUNC(mtx_state::hrx_data_w));
	map(0x33, 0x33).rw(FUNC(mtx_state::hrx_attr_r), FUNC(mtx_state::hrx_attr_w));
//  map(0x38, 0x38).w(MC6845_TAG, FUNC(mc6845_device::address_w));
//  map(0x39, 0x39).rw(MC6845_TAG, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
/*  map(0x40, 0x43).rw(FD1791_TAG, FUNC(fd1791_device::read), FUNC(fd1791_device::write));
    map(0x44, 0x44).rw(FUNC(mtx_state::fdx_status_r), FUNC(mtx_state::fdx_control_w));
    map(0x45, 0x45).w(FUNC(mtx_state::fdx_drv_sel_w));
    map(0x46, 0x46).w(FUNC(mtx_state::fdx_dma_lo_w));
    map(0x47, 0x47).w(FUNC(mtx_state::fdx_dma_hi_w);*/
}

/*-------------------------------------------------
    ADDRESS_MAP( rs128_io )
-------------------------------------------------*/

void mtx_state::rs128_io(address_map &map)
{
	mtx_io(map);
	map(0x0c, 0x0f).rw(m_z80dart, FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_PORTS( mtx512 )
-------------------------------------------------*/

static INPUT_PORTS_START( mtx512 )
	PORT_START("ROW0")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)     PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)     PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)     PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)     PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)     PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7 Page") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(CANCEL)) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)    PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("ROW1")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)    PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)      PORT_CHAR('0')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8 EOL") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROW2")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('\t') PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)    PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("ROW3")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)    PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("ROW4")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AlphaLock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("ROW5")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("ROW6")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)       PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)       PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)       PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)   PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("ROW7")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)      PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)      PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('_')
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)  PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad Enter CLS") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)    PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("country_code")
	PORT_DIPNAME(0x04, 0x00, "Country Code Switch 1")
	PORT_DIPSETTING(0x04, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
	PORT_DIPNAME(0x08, 0x00, "Country Code Switch 0")
	PORT_DIPSETTING(0x08, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keyboard_rom")
	PORT_CONFNAME(0x03, 0x00, "Keyboard ROM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Denmark")
	PORT_CONFSETTING(0x02, "Finland")
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    Z80CTC
-------------------------------------------------*/

TIMER_DEVICE_CALLBACK_MEMBER(mtx_state::ctc_tick)
{
	m_z80ctc->trg1(1);
	m_z80ctc->trg1(0);
	m_z80ctc->trg2(1);
	m_z80ctc->trg2(0);
}

WRITE_LINE_MEMBER(mtx_state::ctc_trg1_w)
{
	if (m_z80dart)
	{
		m_z80dart->rxca_w(state);
		m_z80dart->txca_w(state);
	}
}

WRITE_LINE_MEMBER(mtx_state::ctc_trg2_w)
{
	if (m_z80dart)
	{
		m_z80dart->rxtxcb_w(state);
	}
}

/*-------------------------------------------------
    z80_daisy_config mtx_daisy_chain
-------------------------------------------------*/

static const z80_daisy_config mtx_daisy_chain[] =
{
	{ "z80ctc" },
	{ nullptr }
};

/*-------------------------------------------------
    z80_daisy_config rs128_daisy_chain
-------------------------------------------------*/

static const z80_daisy_config rs128_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80dart" },
	{ nullptr }
};


TIMER_DEVICE_CALLBACK_MEMBER(mtx_state::cassette_tick)
{
	bool cass_ws = (m_cassette->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cassold)
	{
		m_cassold = cass_ws;
		m_z80ctc->trg3(1);
		m_z80ctc->trg3(0);   // this causes interrupt
	}
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    machine_config( mtx512 )
-------------------------------------------------*/

void mtx_state::mtx512(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mtx_state::mtx_mem);
	m_maincpu->set_addrmap(AS_IO, &mtx_state::mtx_io);
	m_maincpu->set_daisy_config(mtx_daisy_chain);

	/* video hardware */
	tms9929a_device &vdp(TMS9929A(config, "tms9929a", 10.6875_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set(m_z80ctc, FUNC(z80ctc_device::trg0)).invert();
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76489A(config, m_sn, 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* devices */
	Z80CTC(config, m_z80ctc, 4_MHz_XTAL);
	m_z80ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80ctc->zc_callback<1>().set(FUNC(mtx_state::ctc_trg1_w));
	m_z80ctc->zc_callback<2>().set(FUNC(mtx_state::ctc_trg2_w));

	TIMER(config, "z80ctc_timer").configure_periodic(FUNC(mtx_state::ctc_tick), attotime::from_hz(4_MHz_XTAL/13));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(mtx_state::write_centronics_busy));
	m_centronics->fault_handler().set(FUNC(mtx_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(mtx_state::write_centronics_perror));
	m_centronics->select_handler().set(FUNC(mtx_state::write_centronics_select));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	SNAPSHOT(config, "snapshot", "mtx", attotime::from_seconds(1)).set_load_callback(FUNC(mtx_state::snapshot_cb));
	QUICKLOAD(config, "quickload", "run", attotime::from_seconds(1)).set_load_callback(FUNC(mtx_state::quickload_cb));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("mtx_cass");

	TIMER(config, "cassette_timer").configure_periodic(FUNC(mtx_state::cassette_tick), attotime::from_hz(44100));

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_extra_options("96K,128K,192K,320K,448K,512K");

	/* rom extension board */
	GENERIC_SOCKET(config, m_extrom, generic_plain_slot, "mtx_rom", "bin,rom");
	m_extrom->set_device_load(FUNC(mtx_state::extrom_load));

	/* rs232 board with disk drive bus */
	MTX_EXP_SLOT(config, m_exp, mtx_expansion_devices, nullptr);
	m_exp->set_program_space(m_maincpu, AS_PROGRAM);
	m_exp->set_io_space(m_maincpu, AS_IO);
	m_exp->int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_exp->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->busreq_handler().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);

	/* cartridge slot */
	GENERIC_CARTSLOT(config, m_rompak, generic_plain_slot, "mtx_cart", "bin,rom");
	m_rompak->set_device_load(FUNC(mtx_state::rompak_load));

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("mtx_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("mtx_flop");
	SOFTWARE_LIST(config, "cart_list").set_original("mtx_cart");
	SOFTWARE_LIST(config, "rom_list").set_original("mtx_rom");
}

void mtx_state::mtx500(machine_config &config)
{
	mtx512(config);

	/* internal ram */
	m_ram->set_default_size("32K").set_extra_options("64K,96K,128K,160K,288K,416K");
}

/*-------------------------------------------------
    machine_config( rs128 )
-------------------------------------------------*/

void mtx_state::rs128(machine_config &config)
{
	mtx512(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mtx_state::rs128_io);
	m_maincpu->set_daisy_config(rs128_daisy_chain);

	/* devices */
	Z80DART(config, m_z80dart, 4_MHz_XTAL);
	m_z80dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* internal ram */
	m_ram->set_default_size("128K").set_extra_options("192K,320K,448K,512K");
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( mtx512 )
	ROM_REGION( 0x02000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "uk", "UK" )
	ROMX_LOAD( "mtx1a.9h",   0x0000, 0x2000, CRC(9ca858cc) SHA1(3804503a58f0bcdea96bb6488833782ebd03976d), ROM_BIOS(0) ) /* OS */
	ROM_SYSTEM_BIOS( 1, "de", "German" )
	ROMX_LOAD( "mtx2a.9h",   0x0000, 0x2000, CRC(1c7bbe98) SHA1(8c950051b830e5e0c41072d6bd893151acd3839d), ROM_BIOS(1) ) /* OS */

	ROM_REGION( 0x10000, "user2", ROMREGION_ERASEFF )
	ROMX_LOAD( "mtx1b.8h",  0x0000, 0x2000, CRC(87b4e59c) SHA1(c49782a82a7f068c1195cd967882ba9edd546eaf), ROM_BIOS(0) ) /* BASIC */
	ROMX_LOAD( "mtx1c.10h", 0x2000, 0x2000, CRC(9d7538c3) SHA1(d1882c4ea61a68b1715bd634ded5603e18a99c5f), ROM_BIOS(0) ) /* ASSEM */
	ROMX_LOAD( "mtx2b.8h",  0x0000, 0x2000, CRC(599d5b6b) SHA1(3ec1f7f476a21ca3206012ded22198c020b47f7d), ROM_BIOS(1) ) /* BASIC */
	ROMX_LOAD( "mtx1c.10h", 0x2000, 0x2000, CRC(9d7538c3) SHA1(d1882c4ea61a68b1715bd634ded5603e18a99c5f), ROM_BIOS(1) ) /* ASSEM */

	/* Keyboard PROMs (N82S181N) are piggy-backed on top of the OS ROM, wired to be selected as ROM 7 */
	/* Country character sets documented in the Operators Manual are:
	    U.S.A, France, Germany, England, Denmark, Sweden, Italy, Spain */
	ROM_REGION( 0x4000, "keyboard_rom", 0 )
	ROM_LOAD( "danish.rom",  0x0000, 0x2000, CRC(9c1b3fae) SHA1(82bc021660d88eebcf0c4d3856558ee9acc1c348) )
	ROM_LOAD( "finnish.rom", 0x2000, 0x2000, CRC(9b96cf72) SHA1(b46d1a733e0e635ccdaf4752cc370d793c3b5c55) )

	/* Device GAL16V8 converted from PAL14L4 JEDEC map */
	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "memotech-mtx512.bin", 0x0000, 0x0117, CRC(31f88133) SHA1(5bef3ce764121b3510b538824b2768f082b422bb) )
ROM_END

#define rom_mtx500  rom_mtx512
#define rom_rs128   rom_mtx512

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS      INIT        COMPANY         FULLNAME   FLAGS
COMP( 1983, mtx512, 0,      0,      mtx512,  mtx512, mtx_state, empty_init, "Memotech Ltd", "MTX 512", 0 )
COMP( 1983, mtx500, mtx512, 0,      mtx500,  mtx512, mtx_state, empty_init, "Memotech Ltd", "MTX 500", 0 )
COMP( 1984, rs128,  mtx512, 0,      rs128,   mtx512, mtx_state, empty_init, "Memotech Ltd", "RS 128",  0 )


/*
The following roms are available should they be considered useful:

ROM_START( mtx_roms )
    ROM_LOAD( "assem.rom",    CRC(599d5b6b) SHA1(3ec1f7f476a21ca3206012ded22198c020b47f7d) )
    ROM_LOAD( "basic.rom",    CRC(d1e9ff36) SHA1(e89ae3a627716e6cee7e35054be8a2472bdd49d4) )
    ROM_LOAD( "boot.rom",     CRC(ed98d6dd) SHA1(4671ee49bb96262b0468f7122a49bf2588170903) )
    ROM_LOAD( "mtx3-an.rom",  CRC(54c9eca2) SHA1(3e628beaa360e635264c8c2c3a5b8312951a220b) )
    ROM_LOAD( "nboot.rom",    CRC(9caea81c) SHA1(93fca6e7ffbc7ae3283b8bda9f01c36b2bed1c54) )
ROM_END

BASIC.ROM   this contains the monitor ROM plus the BASIC ROM.
        It's good to view them as one, because the monitor
        ROM contains a good deal of BASIC code and the machine
        code just runs from the monitor ROM into the BASIC ROM.
        It's also handy if you want to disassemble it (which
        you don't need because it's already done).

MTX3-AN.ROM the monitor ROM, but slightly modified
        While detecting memory size, the startup code destroys
        RAM content. When you have a lot of RAM it is convenient
        to have a ramdisk for CP/M, but it is a nuisance if the
        ramdisk is trashed at each reset. The modification simply
        prevents RAM trashing.

ASSEM.ROM   assembler ROM

BOOT.ROM    FDX floppy boot ROM

NBOOT.ROM   replacement FDX boot ROM written by M. Kessler (supports
        booting from different disk formats and different drives)
*/
