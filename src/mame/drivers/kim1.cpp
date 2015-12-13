// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
    KIM-1

    system driver

    Juergen Buchmueller, Oct 1999

    KIM-1 memory map

    range     short     description
    0000-03FF RAM
    1400-16FF ???
    1700-173F 6530-003  see 6530
    1740-177F 6530-002  see 6530
    1780-17BF RAM       internal 6530-003
    17C0-17FF RAM       internal 6530-002
    1800-1BFF ROM       internal 6530-003
    1C00-1FFF ROM       internal 6530-002

    6530
    offset  R/W short   purpose
    0       X   DRA     Data register A
    1       X   DDRA    Data direction register A
    2       X   DRB     Data register B
    3       X   DDRB    Data direction register B
    4       W   CNT1T   Count down from value, divide by 1, disable IRQ
    5       W   CNT8T   Count down from value, divide by 8, disable IRQ
    6       W   CNT64T  Count down from value, divide by 64, disable IRQ
            R   LATCH   Read current counter value, disable IRQ
    7       W   CNT1KT  Count down from value, divide by 1024, disable IRQ
            R   STATUS  Read counter statzs, bit 7 = 1 means counter overflow
    8       X   DRA     Data register A
    9       X   DDRA    Data direction register A
    A       X   DRB     Data register B
    B       X   DDRB    Data direction register B
    C       W   CNT1I   Count down from value, divide by 1, enable IRQ
    D       W   CNT8I   Count down from value, divide by 8, enable IRQ
    E       W   CNT64I  Count down from value, divide by 64, enable IRQ
            R   LATCH   Read current counter value, enable IRQ
    F       W   CNT1KI  Count down from value, divide by 1024, enable IRQ
            R   STATUS  Read counter statzs, bit 7 = 1 means counter overflow

    6530-002 (U2)
        DRA bit write               read
        ---------------------------------------------
            0-6 segments A-G        key columns 1-7
            7   PTR                 KBD

        DRB bit write               read
        ---------------------------------------------
            0   PTR                 -/-
            1-4 dec 1-3 key row 1-3
                    4   RW3
                    5-9 7-seg   1-6
    6530-003 (U3)
        DRA bit write               read
        ---------------------------------------------
            0-7 bus PA0-7           bus PA0-7

        DRB bit write               read
        ---------------------------------------------
            0-7 bus PB0-7           bus PB0-7


The cassette interface
======================

The KIM-1 stores data on cassette using 2 frequencies: ~3700Hz (high) and ~2400Hz
(low). A high tone is output for 9 cycles and a low tone for 6 cycles. A logic bit
is encoded using 3 sequences of high and low tones. It always starts with a high
tone and ends with a low tone. The middle tone is high for a logic 0 and low for
0 logic 1.

These high and low tone signals are fed to a circuit containing a LM565 PLL and
a 311 comparator. For a high tone a 1 is passed to DB7 of 6530-U2 for a low tone
a 0 is passed. The KIM-1 software measures the time it takes for the signal to
change from 1 to 0.



Pasting:
        0-F : as is
        + (inc) : ^
        AD : -
        DA : =
        GO : X

(note: DA only works when addressing RAM)

Test Paste:
        =11^22^33^44^55^66^77^88^99^-0000
        Now press up-arrow to confirm the data has been entered.

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "machine/mos6530.h"
#include "imagedev/cassette.h"
#include "formats/kim1_cas.h"
#include "kim1.lh"


class kim1_state : public driver_device
{
public:
	kim1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_riot2(*this, "miot_u2"),
		m_cass(*this, "cassette"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3") { }

	required_device<cpu_device> m_maincpu;
	required_device<mos6530_device> m_riot2;
	required_device<cassette_image_device> m_cass;
	DECLARE_READ8_MEMBER(kim1_u2_read_a);
	DECLARE_WRITE8_MEMBER(kim1_u2_write_a);
	DECLARE_READ8_MEMBER(kim1_u2_read_b);
	DECLARE_WRITE8_MEMBER(kim1_u2_write_b);
	UINT8 m_u2_port_b;
	UINT8 m_311_output;
	UINT32 m_cassette_high_count;
	UINT8 m_led_time[6];
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_INPUT_CHANGED_MEMBER(kim1_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(kim1_cassette_input);
	TIMER_DEVICE_CALLBACK_MEMBER(kim1_update_leds);

protected:
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
};





static ADDRESS_MAP_START(kim1_map, AS_PROGRAM, 8, kim1_state)
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0xe000) AM_RAM
	AM_RANGE(0x1700, 0x173f) AM_MIRROR(0xe000) AM_DEVREADWRITE("miot_u3", mos6530_device, read, write )
	AM_RANGE(0x1740, 0x177f) AM_MIRROR(0xe000) AM_DEVREADWRITE("miot_u2", mos6530_device, read, write )
	AM_RANGE(0x1780, 0x17bf) AM_MIRROR(0xe000) AM_RAM
	AM_RANGE(0x17c0, 0x17ff) AM_MIRROR(0xe000) AM_RAM
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0xe000) AM_ROM
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0xe000) AM_ROM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(kim1_state::kim1_reset)
{
	if (newval == 0)
		m_maincpu->reset();
}


static INPUT_PORTS_START( kim1 )
	PORT_START("LINE0")         /* IN0 keys row 0 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("LINE1")         /* IN1 keys row 1 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("LINE2")         /* IN2 keys row 2 */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, 0x10, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x08, 0x08, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x04, 0x04, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, 0x02, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x01, 0x01, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_F6)

	PORT_START("LINE3")         /* IN3 STEP and RESET keys, MODE switch */
	PORT_BIT( 0x80, 0x00, IPT_UNUSED )
	PORT_BIT( 0x40, 0x40, IPT_KEYBOARD ) PORT_NAME("sw1: ST") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x20, 0x20, IPT_KEYBOARD ) PORT_NAME("sw2: RS") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, kim1_state, kim1_reset, NULL)
	PORT_DIPNAME(0x10, 0x10, "sw3: SS") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE
	PORT_DIPSETTING( 0x00, "single step")
	PORT_DIPSETTING( 0x10, "run")
	PORT_BIT( 0x08, 0x00, IPT_UNUSED )
	PORT_BIT( 0x04, 0x00, IPT_UNUSED )
	PORT_BIT( 0x02, 0x00, IPT_UNUSED )
	PORT_BIT( 0x01, 0x00, IPT_UNUSED )
INPUT_PORTS_END


READ8_MEMBER( kim1_state::kim1_u2_read_a )
{
	UINT8 data = 0xff;

	switch( ( m_u2_port_b >> 1 ) & 0x0f )
	{
	case 0:
		data = m_line0->read();
		break;
	case 1:
		data = m_line1->read();
		break;
	case 2:
		data = m_line2->read();
		break;
	}
	return data;
}


WRITE8_MEMBER( kim1_state::kim1_u2_write_a )
{
	UINT8 idx = ( m_u2_port_b >> 1 ) & 0x0f;

	if ( idx >= 4 && idx < 10 )
	{
		if ( data & 0x80 )
		{
			output_set_digit_value( idx-4, data & 0x7f );
			m_led_time[idx - 4] = 15;
		}
	}
}

READ8_MEMBER( kim1_state::kim1_u2_read_b )
{
	if ( m_riot2->portb_out_get() & 0x20 )
		return 0xFF;

	return 0x7F | ( m_311_output ^ 0x80 );
}


WRITE8_MEMBER( kim1_state::kim1_u2_write_b )
{
	m_u2_port_b = data;

	if ( data & 0x20 )
		/* cassette write/speaker update */
		m_cass->output(( data & 0x80 ) ? -1.0 : 1.0 );

	/* Set IRQ when bit 7 is cleared */
}


TIMER_DEVICE_CALLBACK_MEMBER(kim1_state::kim1_cassette_input)
{
	double tap_val = m_cass->input();

	if ( tap_val <= 0 )
	{
		if ( m_cassette_high_count )
		{
			m_311_output = ( m_cassette_high_count < 8 ) ? 0x80 : 0;
			m_cassette_high_count = 0;
		}
	}

	if ( tap_val > 0 )
		m_cassette_high_count++;
}


TIMER_DEVICE_CALLBACK_MEMBER(kim1_state::kim1_update_leds)
{
	UINT8 i;

	for ( i = 0; i < 6; i++ )
	{
		if ( m_led_time[i] )
			m_led_time[i]--;
		else
			output_set_digit_value( i, 0 );
	}
}


void kim1_state::machine_start()
{
	save_item(NAME(m_u2_port_b));
	save_item(NAME(m_311_output));
	save_item(NAME(m_cassette_high_count));
}


void kim1_state::machine_reset()
{
	UINT8 i;

	for ( i = 0; i < 6; i++ )
		m_led_time[i] = 0;

	m_311_output = 0;
	m_cassette_high_count = 0;
}


static MACHINE_CONFIG_START( kim1, kim1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1000000)        /* 1 MHz */
	MCFG_CPU_PROGRAM_MAP(kim1_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video */
	MCFG_DEFAULT_LAYOUT( layout_kim1 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_DEVICE_ADD("miot_u2", MOS6530, 1000000)
	MCFG_MOS6530_IN_PA_CB(READ8(kim1_state, kim1_u2_read_a))
	MCFG_MOS6530_OUT_PA_CB(WRITE8(kim1_state, kim1_u2_write_a))
	MCFG_MOS6530_IN_PB_CB(READ8(kim1_state, kim1_u2_read_b))
	MCFG_MOS6530_OUT_PB_CB(WRITE8(kim1_state, kim1_u2_write_b))

	MCFG_DEVICE_ADD("miot_u3", MOS6530, 1000000)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(kim1_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("led_timer", kim1_state, kim1_update_leds, attotime::from_hz(60))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("cassette_timer", kim1_state, kim1_cassette_input, attotime::from_hz(44100))
MACHINE_CONFIG_END


ROM_START(kim1)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("6530-003.bin",    0x1800, 0x0400, CRC(a2a56502) SHA1(60b6e48f35fe4899e29166641bac3e81e3b9d220))
	ROM_LOAD("6530-002.bin",    0x1c00, 0x0400, CRC(2b08e923) SHA1(054f7f6989af3a59462ffb0372b6f56f307b5362))
ROM_END


/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT    COMPANY          FULLNAME        FLAGS */
COMP( 1975, kim1,     0,        0,      kim1,     kim1, driver_device,     0,    "MOS Technologies", "KIM-1" , MACHINE_SUPPORTS_SAVE)
