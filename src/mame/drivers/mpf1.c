// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
/************************************************\
* Multitech Micro Professor 1                    *
*                                                *
*     CPU: Z80 @ 1.79 MHz                        *
*     ROM: 4-kilobyte ROM monitor                *
*     RAM: 4 kilobytes                           *
*   Input: Hex keypad                            *
* Storage: Cassette tape                         *
*   Video: 6x 7-segment LED display              *
*   Sound: Speaker                               *
\************************************************/

/*

    Keys:
        0-9,A-F : hexadecimal numbers
        ADR : enter an address to work with. After the 4 digits are entered,
              the data at that address shows, and you can modify the data.
        +   : Enter the data into memory, and increment the address by 1.
        GO  : execute the program located at the current address.

    Pasting:
        0-F : as is
        +   : ^
        -   : V
        ADDR : -
        DATA : =
        GO : X
        PC : P

    Test Paste:
        -1800=11^22^33^44^55^66^77^88^99^-1800
        Now press up-arrow to confirm the data has been entered.

    TODO:

    - remove halt callback
    - crt board
    - speech board
    - printers
    - clickable artwork
    - video board (has 6845)
    - mpf1p has 49-key keyboard
    - computer can't keep up with paste
    - paste only set up for mpf1

*/

#include "includes/mpf1.h"
#include "mpf1.lh"
#include "mpf1b.lh"
#include "mpf1p.lh"

/* Address Maps */

static ADDRESS_MAP_START( mpf1_map, AS_PROGRAM, 8, mpf1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1b_map, AS_PROGRAM, 8, mpf1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x5000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1p_map, AS_PROGRAM, 8, mpf1_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1_io_map, AS_IO, 8, mpf1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1b_io_map, AS_IO, 8, mpf1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0xfe, 0xfe) AM_MIRROR(0x01) AM_DEVREADWRITE(TMS5220_TAG, tms5220_device, status_r, data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1p_io_map, AS_IO, 8, mpf1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( mpf1_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( mpf1_state::trigger_irq )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( mpf1_state::trigger_res )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( mpf1 )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 HL") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 HL'") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B I*IF") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F *PNC'") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 DE") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 DE'") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A SP") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E SZ*H'") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 BC") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 BC'") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 IY") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D *PNC") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE RD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 AF") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 AF'") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 IX") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C SZ*H") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE WR") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBR") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADDR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RELA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SBR") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DATA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MOVE") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("USER KEY") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_res, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( mpf1b )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 /") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 >") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B STOP") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F LET") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 *") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 <") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A CALL") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E INPUT") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 -") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 =") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 P") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D PRINT") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONT") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 +") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 * *") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 M") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C NEXT") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SAVE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("IF/\xE2\x88\xA7") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TO/\xE2\x86\x93") PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("THEN/\xE2\x88\xA8") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOTO") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET/\xE2\x89\x81") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOSUB") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FOR/\xE2\x86\x91") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LIST") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR/\xE2\x86\x92") PORT_CODE(KEYCODE_INSERT) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL/\xE2\x86\x90") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_irq, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_res, 0)
INPUT_PORTS_END

/* Intel 8255A Interface */

TIMER_CALLBACK_MEMBER(mpf1_state::led_refresh)
{
	if (BIT(m_lednum, 5)) output_set_digit_value(0, param);
	if (BIT(m_lednum, 4)) output_set_digit_value(1, param);
	if (BIT(m_lednum, 3)) output_set_digit_value(2, param);
	if (BIT(m_lednum, 2)) output_set_digit_value(3, param);
	if (BIT(m_lednum, 1)) output_set_digit_value(4, param);
	if (BIT(m_lednum, 0)) output_set_digit_value(5, param);
}

READ8_MEMBER( mpf1_state::ppi_pa_r )
{
	UINT8 data = 0x7f;

	/* bit 0 to 5, keyboard rows 0 to 5 */
	if (!BIT(m_lednum, 0)) data &= m_pc0->read();
	if (!BIT(m_lednum, 1)) data &= m_pc1->read();
	if (!BIT(m_lednum, 2)) data &= m_pc2->read();
	if (!BIT(m_lednum, 3)) data &= m_pc3->read();
	if (!BIT(m_lednum, 4)) data &= m_pc4->read();
	if (!BIT(m_lednum, 5)) data &= m_pc5->read();

	/* bit 6, user key */
	data &= m_special->read() & 1 ? 0xff : 0xbf;

	/* bit 7, tape input */
	data |= ((m_cassette)->input() > 0) ? 0x80 : 0;

	return data;
}

WRITE8_MEMBER( mpf1_state::ppi_pb_w )
{
	/* swap bits around for the mame 7-segment emulation */
	UINT8 led_data = BITSWAP8(data, 6, 1, 2, 0, 7, 5, 4, 3);

	/* timer to update segments */
	m_led_refresh_timer->adjust(attotime::from_usec(70), led_data);
}

WRITE8_MEMBER( mpf1_state::ppi_pc_w )
{
	/* bits 0-5, led select and keyboard latch */
	m_lednum = data & 0x3f;
	m_led_refresh_timer->adjust(attotime::never);

	/* bit 6, monitor break control */
	m_break = BIT(data, 6);

	if (m_break)
	{
		m_m1 = 0;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}

	/* bit 7, tape output, tone and led */
	set_led_status(machine(), 0, !BIT(data, 7));
	m_speaker->level_w(BIT(data, 7));
	m_cassette->output( BIT(data, 7) ? 1.0 : -1.0);
}

/* Z80 Daisy Chain */

static const z80_daisy_config mpf1_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};

/* Machine Initialization */

TIMER_DEVICE_CALLBACK_MEMBER(mpf1_state::check_halt_callback)
{
	// halt-LED; the red one, is turned on when the processor is halted
	// TODO: processor seems to halt, but restarts(?) at 0x0000 after a while -> fix
	INT64 led_halt = m_maincpu->state_int(Z80_HALT);
	set_led_status(machine(), 1, led_halt);
}

void mpf1_state::machine_start()
{
	m_led_refresh_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mpf1_state::led_refresh),this));

	/* register for state saving */
	save_item(NAME(m_break));
	save_item(NAME(m_m1));
	save_item(NAME(m_lednum));
}

void mpf1_state::machine_reset()
{
	m_lednum = 0;
}

/* Machine Drivers */

static MACHINE_CONFIG_START( mpf1, mpf1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(mpf1_map)
	MCFG_CPU_IO_MAP(mpf1_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_3_579545MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_3_579545MHz/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mpf1_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mpf1_state, ppi_pb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mpf1_state, ppi_pc_w))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("halt_timer", mpf1_state, check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpf1b, mpf1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(mpf1b_map)
	MCFG_CPU_IO_MAP(mpf1b_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_3_579545MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_3_579545MHz/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mpf1_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mpf1_state, ppi_pb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mpf1_state, ppi_pc_w))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1b)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD(TMS5220_TAG, TMS5220, 680000L)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("halt_timer", mpf1_state, check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpf1p, mpf1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(mpf1p_map)
	MCFG_CPU_IO_MAP(mpf1p_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1p)

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, 2500000)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, 2500000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(I8255A_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mpf1_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mpf1_state, ppi_pb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mpf1_state, ppi_pc_w))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("halt_timer", mpf1_state, check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

/* ROMs */

ROM_START( mpf1 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "mpf.u6", 0x0000, 0x1000, CRC(b60249ce) SHA1(78e0e8874d1497fabfdd6378266d041175e3797f) )
ROM_END

ROM_START( mpf1b )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "c55167.u6", 0x0000, 0x1000, CRC(28b06dac) SHA1(99cfbab739d71a914c39302d384d77bddc4b705b) )
	ROM_LOAD( "basic.u7", 0x2000, 0x1000, CRC(d276ed6b) SHA1(a45fb98961be5e5396988498c6ed589a35398dcf) )
	ROM_LOAD( "ssb-mpf.u", 0x5000, 0x1000, CRC(f926334f) SHA1(35847f8164eed4c0794a8b74e5d7fa972b10eb90) )
	ROM_LOAD( "prt-mpf.u5", 0x6000, 0x1000, CRC(730f2fb0) SHA1(f31536ee9dbb9babb9ce16a7490db654ca0b5749) )
ROM_END

ROM_START( mpf1p )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "mpf1pmon.bin", 0x0000, 0x2000, BAD_DUMP CRC(91ace7d3) SHA1(22e3c16a81ac09f37741ad1b526a4456b2ba9493) ) // A9 stuck low when dumped
	ROM_LOAD( "prt-mpf-ip.u5", 0x6000, 0x1000, CRC(4dd2a4eb) SHA1(6a3e7daa7834d67fd572261ed4a9a62c4594fe3f) )
ROM_END

/* System Drivers */

DIRECT_UPDATE_MEMBER(mpf1_state::mpf1_direct_update_handler)
{
	if (!m_break)
	{
		m_m1++;

		if (m_m1 == 5)
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		}
	}

	return address;
}

DRIVER_INIT_MEMBER(mpf1_state,mpf1)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(mpf1_state::mpf1_direct_update_handler), this));
}

COMP( 1979, mpf1,  0,    0, mpf1, mpf1, mpf1_state,  mpf1, "Multitech", "Micro Professor 1", 0)
COMP( 1979, mpf1b, mpf1, 0, mpf1b,mpf1b, mpf1_state, mpf1, "Multitech", "Micro Professor 1B", 0)
COMP( 1982, mpf1p, mpf1, 0, mpf1p,mpf1b, mpf1_state, mpf1, "Multitech", "Micro Professor 1 Plus", MACHINE_NOT_WORKING)
