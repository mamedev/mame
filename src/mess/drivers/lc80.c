// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    LC-80

    12/05/2009 Skeleton driver.

    When first started, the screen is blank. Wait about 8 seconds for
    it to introduce itself, then you may use it or paste to it.
    The decimal points indicate which side of the display you will
    be updating.

    Pasting:
        0-F : as is
        + (inc) : ^
        - (dec) : V
        ADR : -
        DAT : =
        GO : X

    Test Paste: (lc80_2 only)
        -2000=11^22^33^44^55^66^77^88^99^-2000
        Now press up-arrow to confirm the data has been entered.


    ToDo:
    - Most characters are lost when pasting (lc80, sc80).

****************************************************************************/

/*

    TODO:

    - HALT led
    - KSD11 switch
    - banking for ROM 4-5
    - Schachcomputer SC-80
    - CTC clock inputs

*/

#include "includes/lc80.h"
#include "lc80.lh"

/* Memory Maps */

static ADDRESS_MAP_START( lc80_mem, AS_PROGRAM, 8, lc80_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x07ff) AM_ROMBANK("bank1")
	AM_RANGE(0x0800, 0x0fff) AM_ROMBANK("bank2")
	AM_RANGE(0x1000, 0x17ff) AM_ROMBANK("bank3")
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x2fff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( sc80_mem, AS_PROGRAM, 8, lc80_state )
	AM_IMPORT_FROM(lc80_mem)
	AM_RANGE(0xc000, 0xcfff) AM_ROM
ADDRESS_MAP_END
#endif

static ADDRESS_MAP_START( lc80_io, AS_IO, 8, lc80_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1f)
	AM_RANGE(0xf4, 0xf7) AM_DEVREADWRITE(Z80PIO1_TAG, z80pio_device, read, write)
	AM_RANGE(0xf8, 0xfb) AM_DEVREADWRITE(Z80PIO2_TAG, z80pio_device, read, write)
	AM_RANGE(0xec, 0xef) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( lc80_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( lc80_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( lc80 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LD") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ST") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DAT") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("EX") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, lc80_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, lc80_state, trigger_nmi, 0)
INPUT_PORTS_END

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( lc80_state::ctc_z0_w )
{
}

WRITE_LINE_MEMBER( lc80_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( lc80_state::ctc_z2_w )
{
}

/* Z80-PIO Interface */

void lc80_state::update_display()
{
	int i;

	for (i = 0; i < 6; i++)
	{
		if (!BIT(m_digit, i)) output_set_digit_value(5 - i, m_segment);
	}
}

WRITE8_MEMBER( lc80_state::pio1_pa_w )
{
	/*

	    bit     description

	    PA0     VQE23 segment B
	    PA1     VQE23 segment F
	    PA2     VQE23 segment A
	    PA3     VQE23 segment G
	    PA4     VQE23 segment DP
	    PA5     VQE23 segment C
	    PA6     VQE23 segment E
	    PA7     VQE23 segment D

	*/

	m_segment = BITSWAP8(~data, 4, 3, 1, 6, 7, 5, 0, 2);

	update_display();
}

READ8_MEMBER( lc80_state::pio1_pb_r )
{
	/*

	    bit     description

	    PB0     tape input
	    PB1     tape output
	    PB2     digit 0
	    PB3     digit 1
	    PB4     digit 2
	    PB5     digit 3
	    PB6     digit 4
	    PB7     digit 5

	*/

	return (m_cassette->input() < +0.0);
}

WRITE8_MEMBER( lc80_state::pio1_pb_w )
{
	/*

	    bit     description

	    PB0     tape input
	    PB1     tape output, speaker output, OUT led
	    PB2     digit 0
	    PB3     digit 1
	    PB4     digit 2
	    PB5     digit 3
	    PB6     digit 4
	    PB7     digit 5

	*/

	/* tape output */
	m_cassette->output( BIT(data, 1) ? +1.0 : -1.0);

	/* speaker */
	m_speaker->level_w(!BIT(data, 1));

	/* OUT led */
	output_set_led_value(0, !BIT(data, 1));

	/* keyboard */
	m_digit = data >> 2;

	/* display */
	update_display();
}

READ8_MEMBER( lc80_state::pio2_pb_r )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     key row 0
	    PB5     key row 1
	    PB6     key row 2
	    PB7     key row 3

	*/

	UINT8 data = 0xf0;
	int i;

	for (i = 0; i < 6; i++)
	{
		if (!BIT(m_digit, i))
		{
			if (!BIT(m_y0->read(), i)) data &= ~0x10;
			if (!BIT(m_y1->read(), i)) data &= ~0x20;
			if (!BIT(m_y2->read(), i)) data &= ~0x40;
			if (!BIT(m_y3->read(), i)) data &= ~0x80;
		}
	}

	return data;
}

#if 0
/* Z80 Daisy Chain */

static const z80_daisy_config lc80_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80PIO2_TAG },
	{ Z80PIO1_TAG },
	{ NULL }
};
#endif

/* Machine Initialization */

void lc80_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	UINT8 *ROM = memregion(Z80_TAG)->base();

	/* setup memory banking */
	membank("bank1")->configure_entry(0, &ROM[0]); // TODO
	membank("bank1")->configure_entry(1, &ROM[0]);
	membank("bank1")->set_entry(1);

	membank("bank2")->configure_entry(0, &ROM[0x800]); // TODO
	membank("bank2")->configure_entry(1, &ROM[0x800]);
	membank("bank2")->set_entry(1);

	membank("bank3")->configure_entry(0, &ROM[0x1000]); // TODO
	membank("bank3")->configure_entry(1, &ROM[0x1000]);
	membank("bank3")->set_entry(1);

	membank("bank4")->configure_entry(0, &ROM[0x2000]);
	membank("bank4")->set_entry(0);

	program.install_readwrite_bank(0x0000, 0x07ff, "bank1");
	program.install_readwrite_bank(0x0800, 0x0fff, "bank2");
	program.install_readwrite_bank(0x1000, 0x17ff, "bank3");

	switch (m_ram->size())
	{
	case 1*1024:
		program.install_readwrite_bank(0x2000, 0x23ff, "bank4");
		program.unmap_readwrite(0x2400, 0x2fff);
		break;

	case 2*1024:
		program.install_readwrite_bank(0x2000, 0x27ff, "bank4");
		program.unmap_readwrite(0x2800, 0x2fff);
		break;

	case 3*1024:
		program.install_readwrite_bank(0x2000, 0x2bff, "bank4");
		program.unmap_readwrite(0x2c00, 0x2fff);
		break;

	case 4*1024:
		program.install_readwrite_bank(0x2000, 0x2fff, "bank4");
		break;
	}

	/* register for state saving */
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
}

/* Machine Driver */

static MACHINE_CONFIG_START( lc80, lc80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 900000) /* UD880D */
	MCFG_CPU_PROGRAM_MAP(lc80_mem)
	MCFG_CPU_IO_MAP(lc80_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_lc80 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, 900000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(lc80_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(lc80_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(lc80_state, ctc_z2_w))

	MCFG_DEVICE_ADD(Z80PIO1_TAG, Z80PIO, 900000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(lc80_state, pio1_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(lc80_state, pio1_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(lc80_state, pio1_pb_w))

	MCFG_DEVICE_ADD(Z80PIO2_TAG, Z80PIO, 900000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PB_CB(READ8(lc80_state, pio2_pb_r))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
	MCFG_RAM_EXTRA_OPTIONS("2K,3K,4K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lc80_2, lc80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 1800000) /* UD880D */
	MCFG_CPU_PROGRAM_MAP(lc80_mem)
	MCFG_CPU_IO_MAP(lc80_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_lc80 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, 900000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(lc80_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(lc80_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(lc80_state, ctc_z2_w))

	MCFG_DEVICE_ADD(Z80PIO1_TAG, Z80PIO, 900000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(lc80_state, pio1_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(lc80_state, pio1_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(lc80_state, pio1_pb_w))

	MCFG_DEVICE_ADD(Z80PIO2_TAG, Z80PIO, 900000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PB_CB(READ8(lc80_state, pio2_pb_r))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
MACHINE_CONFIG_END

#if 0
static MACHINE_CONFIG_DERIVED( sc80, lc80_2 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_PROGRAM_MAP(sc80_mem)
MACHINE_CONFIG_END
#endif

/* ROMs */

ROM_START( lc80 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "u505", "LC 80 (2x U505)" )
	ROMX_LOAD( "lc80.d202", 0x0000, 0x0400, CRC(e754ef53) SHA1(044440b13e62addbc3f6a77369cfd16f99b39752), ROM_BIOS(1) )
	ROMX_LOAD( "lc80.d203", 0x0800, 0x0400, CRC(2b544da1) SHA1(3a6cbd0c57c38eadb7055dca4b396c348567d1d5), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "2716", "LC 80 (2716)" )
	ROMX_LOAD( "lc80_2716.bin", 0x0000, 0x0800, CRC(b3025934) SHA1(6fff953f0f1eee829fd774366313ab7e8053468c), ROM_BIOS(2))
ROM_END

ROM_START( lc80_2 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "lc80_2.bin", 0x0000, 0x1000, CRC(2e06d768) SHA1(d9cddaf847831e4ab21854c0f895348b7fda20b8) )
ROM_END

ROM_START( sc80 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "lc80e-0000-schach.rom", 0x0000, 0x1000, CRC(e3cca61d) SHA1(f2be3f2a9d3780d59657e49b3abeffb0fc13db89) )
	ROM_LOAD( "lc80e-1000-schach.rom", 0x1000, 0x1000, CRC(b0323160) SHA1(0ea019b0944736ae5b842bf9aa3537300f259b98) )
	ROM_LOAD( "lc80e-c000-schach.rom", 0xc000, 0x1000, CRC(9c858d9c) SHA1(2f7b3fd046c965185606253f6cd9372da289ca6f) )
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                 FULLNAME                FLAGS */
COMP( 1984, lc80,   0,      0,      lc80,   lc80, driver_device,   0, "VEB Mikroelektronik", "Lerncomputer LC 80", MACHINE_SUPPORTS_SAVE )
COMP( 1984, lc80_2, lc80,   0,      lc80_2, lc80, driver_device,   0, "VEB Mikroelektronik", "Lerncomputer LC 80.2", MACHINE_SUPPORTS_SAVE )
COMP( 1984, sc80,   lc80,   0,      lc80_2, lc80, driver_device,   0, "VEB Mikroelektronik", "Schachcomputer SC-80", MACHINE_SUPPORTS_SAVE )
