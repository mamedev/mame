// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MC 1000

    12/05/2009 Skeleton driver.

    http://ensjo.wikispaces.com/MC-1000+on+JEMU
    http://ensjo.blogspot.com/2006/11/color-artifacting-no-mc-1000.html

****************************************************************************/

/*

    TODO:

    - xtal frequency?
    - Z80 wait at 0x0000-0x1fff when !hsync & !vsync
    - 80-column card (MC6845) character generator ROM
    - Charlemagne / GEM-1000 / Junior Computer ROMs

*/

#include "includes/mc1000.h"
#include "softlist.h"

/* Memory Banking */

void mc1000_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* MC6845 video RAM */
	membank("bank2")->set_entry(m_mc6845_bank);

	/* extended RAM */
	if (m_ram->size() > 16*1024)
	{
		program.install_readwrite_bank(0x4000, 0x7fff, "bank3");
	}
	else
	{
		program.unmap_readwrite(0x4000, 0x7fff);
	}

	/* MC6847 video RAM */
	if (m_mc6847_bank)
	{
		if (m_ram->size() > 16*1024)
		{
			program.install_readwrite_bank(0x8000, 0x97ff, "bank4");
		}
		else
		{
			program.unmap_readwrite(0x8000, 0x97ff);
		}
	}
	else
	{
		program.install_readwrite_bank(0x8000, 0x97ff, "bank4");
	}

	membank("bank4")->set_entry(m_mc6847_bank);

	/* extended RAM */
	if (m_ram->size() > 16*1024)
	{
		program.install_readwrite_bank(0x9800, 0xbfff, "bank5");
	}
	else
	{
		program.unmap_readwrite(0x9800, 0xbfff);
	}
}

/* Read/Write Handlers */

WRITE_LINE_MEMBER( mc1000_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

READ8_MEMBER( mc1000_state::printer_r )
{
	return m_centronics_busy;
}

WRITE8_MEMBER( mc1000_state::printer_w )
{
	m_centronics->write_strobe(BIT(data, 0));
}

WRITE8_MEMBER( mc1000_state::mc6845_ctrl_w )
{
	m_mc6845_bank = BIT(data, 0);

	bankswitch();
}

WRITE8_MEMBER( mc1000_state::mc6847_attr_w )
{
	/*

	    bit     description

	    0       enable CPU video RAM access
	    1       CSS
	    2       GM0
	    3       GM1
	    4       GM2
	    5       _INT/EXT
	    6       _A/S
	    7       _A/G

	*/

	m_mc6847_bank = BIT(data, 0);
	m_vdg->css_w(BIT(data, 1));
	m_vdg->gm0_w(BIT(data, 2));
	m_vdg->gm1_w(BIT(data, 3));
	m_vdg->gm2_w(BIT(data, 4));
	m_vdg->intext_w(BIT(data, 5));
	m_vdg->as_w(BIT(data, 6));
	m_vdg->ag_w(BIT(data, 7));

	bankswitch();
}

/* Memory Maps */

static ADDRESS_MAP_START( mc1000_mem, AS_PROGRAM, 8, mc1000_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000, 0x27ff) AM_RAMBANK("bank2") AM_SHARE("mc6845_vram")
	AM_RANGE(0x2800, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank3")
	AM_RANGE(0x8000, 0x97ff) AM_RAMBANK("bank4") AM_SHARE("mc6847_vram")
	AM_RANGE(0x9800, 0xbfff) AM_RAMBANK("bank5")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mc1000_io, AS_IO, 8, mc1000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_READWRITE(printer_r, printer_w)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE("cent_data_out", output_latch_device, write)
//  AM_RANGE(0x10, 0x10) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
//  AM_RANGE(0x11, 0x11) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(mc6845_ctrl_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE(AY8910_TAG, ay8910_device, address_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD(AY8910_TAG, ay8910_device, data_r)
	AM_RANGE(0x60, 0x60) AM_DEVWRITE(AY8910_TAG, ay8910_device, data_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(mc6847_attr_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( mc1000 )
	PORT_START("JOYA") /* Player 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    /* = 'I' */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  /* = 'Q' */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  /* = 'Y' */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) /* = '1' */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )        /* = '9' */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOYB") /* Player 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)        /* = '@' */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)    /* = 'H' */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)  /* = 'P' */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)  /* = 'X' */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) /* = '0' */
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUBOUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MODIFIERS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("JOYAKEYMAP")
	PORT_CONFNAME( 0x01, 0x00, "JOYSTICK A (P1) keyboard mapping" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("JOYBKEYMAP")
	PORT_CONFNAME( 0x01, 0x00, "JOYSTICK B (P2) keyboard mapping" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

/* Video */

WRITE_LINE_MEMBER( mc1000_state::fs_w )
{
	m_vsync = state;
}

WRITE_LINE_MEMBER( mc1000_state::hs_w )
{
	m_hsync = state;
}

READ8_MEMBER( mc1000_state::videoram_r )
{
	if (offset == ~0) return 0xff;

	m_vdg->inv_w(BIT(m_mc6847_video_ram[offset], 7));

	return m_mc6847_video_ram[offset];
}

/* AY-3-8910 Interface */

WRITE8_MEMBER( mc1000_state::keylatch_w )
{
	m_keylatch = data;

	m_cassette->output(BIT(data, 7) ? -1.0 : +1.0);
}

READ8_MEMBER( mc1000_state::keydata_r )
{
	UINT8 data = 0xff;

	if (!BIT(m_keylatch, 0))
	{
		data &= m_y0->read();
		if (m_joybkeymap->read()) data &= m_joyb->read();
	}
	if (!BIT(m_keylatch, 1))
	{
		data &= m_y1->read();
		if (m_joyakeymap->read()) data &= m_joya->read();
	}
	if (!BIT(m_keylatch, 2)) data &= m_y2->read();
	if (!BIT(m_keylatch, 3)) data &= m_y3->read();
	if (!BIT(m_keylatch, 4)) data &= m_y4->read();
	if (!BIT(m_keylatch, 5)) data &= m_y5->read();
	if (!BIT(m_keylatch, 6)) data &= m_y6->read();
	if (!BIT(m_keylatch, 7)) data &= m_y7->read();

	data = (m_modifiers->read() & 0xc0) | (data & 0x3f);

	if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
	{
		if (m_cassette->input() >= +0.0) data &= 0x7f;
	}

	return data;
}

/* Machine Initialization */

void mc1000_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* setup memory banking */
	UINT8 *rom = m_rom->base();

	program.install_readwrite_bank(0x0000, 0x1fff, "bank1");
	membank("bank1")->configure_entry(0, rom);
	membank("bank1")->configure_entry(1, rom + 0xc000);
	membank("bank1")->set_entry(1);

	m_rom0000 = 1;

	program.install_readwrite_bank(0x2000, 0x27ff, "bank2");
	membank("bank2")->configure_entry(0, rom + 0x2000);
	membank("bank2")->configure_entry(1, m_mc6845_video_ram);
	membank("bank2")->set_entry(0);

	membank("bank3")->configure_entry(0, rom + 0x4000);
	membank("bank3")->set_entry(0);

	membank("bank4")->configure_entry(0, m_mc6847_video_ram);
	membank("bank4")->configure_entry(1, rom + 0x8000);
	membank("bank4")->set_entry(0);

	membank("bank5")->configure_entry(0, rom + 0x9800);
	membank("bank5")->set_entry(0);

	bankswitch();

	/* register for state saving */
	save_item(NAME(m_rom0000));
	save_item(NAME(m_mc6845_bank));
	save_item(NAME(m_mc6847_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
}

void mc1000_state::machine_reset()
{
	membank("bank1")->set_entry(1);

	m_rom0000 = 1;
}

/* Machine Driver */

/*

 Interrupt generator:
 NE555 chip in astable circuit.

  +---------*---*---o V+
  |         |   |
 +-+        |   |
 | |390K    |   |
 | |R17     |8  |4
 +-+      +-------+
  |      7|       |3
  *-------|       |-------> /INT (Z80)
  |       |       |
  |       |       |
 +-+R16  2| IC 28 |
 | |1K +--|       |
 | |   |  |  555  |
 +-+   |  |       |
  |    | 6|       |5
  *----*--|       |---+
  |       |       |   |
 ---C30   +-------+  ---C29
 ---10n       |1     ---10n
 _|_         _|_     _|_
 ///         ///     ///

 Calculated properties:

 * 99.74489795918367 Duty Cycle Percentage
 * 367.3469387755102 Frequency in Hertz
 * 0.00000693 Seconds Low
 * 0.00270963 Seconds High

 */

#define MC1000_NE555_FREQ       (367) /* Hz */
#define MC1000_NE555_DUTY_CYCLE (99.745) /* % */

TIMER_DEVICE_CALLBACK_MEMBER(mc1000_state::ne555_tick)
{
	// (m_ne555_int not needed anymore and can be done with?)
	m_ne555_int = param;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, param);
}

static MACHINE_CONFIG_START( mc1000, mc1000_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(mc1000_mem)
	MCFG_CPU_IO_MAP(mc1000_io)

	/* timers */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ne555clear", mc1000_state, ne555_tick, attotime::from_hz(MC1000_NE555_FREQ))
	MCFG_TIMER_PARAM(CLEAR_LINE)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ne555assert", mc1000_state, ne555_tick, attotime::from_hz(MC1000_NE555_FREQ))
	MCFG_TIMER_START_DELAY(attotime::from_hz(MC1000_NE555_FREQ * 100 / MC1000_NE555_DUTY_CYCLE))
	MCFG_TIMER_PARAM(ASSERT_LINE)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD(SCREEN_TAG, MC6847_TAG)

	MCFG_DEVICE_ADD(MC6847_TAG, MC6847_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(mc1000_state, hs_w))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(mc1000_state, fs_w))
	MCFG_MC6847_INPUT_CALLBACK(READ8(mc1000_state, videoram_r))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(AY8910_TAG, AY8910, 3579545/2)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(2.2), 0, 0)
	MCFG_AY8910_PORT_B_READ_CB(READ8(mc1000_state, keydata_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(mc1000_state, keylatch_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("mc1000_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list", "mc1000_cass")

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(mc1000_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("48K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( mc1000 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "mc1000.ic17", 0xc000, 0x2000, CRC(8e78d80d) SHA1(9480270e67a5db2e7de8bc5c8b9e0bb210d4142b) )
	ROM_LOAD( "mc1000.ic12", 0xe000, 0x2000, CRC(750c95f0) SHA1(fd766f5ea4481ef7fd4df92cf7d8397cc2b5a6c4) )
ROM_END


/* Driver Initialization */

DIRECT_UPDATE_MEMBER(mc1000_state::mc1000_direct_update_handler)
{
	if (m_rom0000)
	{
		if (address >= 0xc000)
		{
			membank("bank1")->set_entry(0);
			m_rom0000 = 0;
		}
	}

	return address;
}

DRIVER_INIT_MEMBER(mc1000_state,mc1000)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(mc1000_state::mc1000_direct_update_handler), this));
}

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT        COMPANY             FULLNAME        FLAGS */
COMP( 1985, mc1000,     0,          0,      mc1000,     mc1000, mc1000_state,       mc1000,     "CCE",              "MC-1000",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
