// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Heathkit H8

        2009-05-12 Skeleton driver.

        This system uses Octal rather than the usual hexadecimal.

STATUS:
        It runs, keyboard works, you can enter data.

Meaning of LEDs:
        PWR = power is turned on
        MON = controls should work
        RUN = CPU is running (not halted)
        ION = Interrupts are enabled

Pasting:
        0-F : as is
        + : ^
        - : V
        MEM : -
        ALTER : =

        Addresses must have all 6 digits entered.
        Data must have all 3 digits entered.
        System has a short beep for each key, and a slightly longer beep
            for each group of 3 digits. The largest number allowed is 377 (=0xFF).

Test Paste:
        -041000=123 245 333 144 255 366 077=-041000
        Now press up-arrow to confirm the data has been entered.

Official test program from pages 4 to 8 of the operator's manual:
        -040100=076 002 062 010 040 006 004 041 170 040 021 013 040 016 011 176
                022 043 023 015 302 117 040 016 003 076 377 315 053 000 015 302
                131 040 005 302 112 040 076 062 315 140 002 076 062 315 053 000
                076 062 315 140 002 303 105 040 377 262 270 272 275 377 222 200
                377 237 244 377 272 230 377 220 326 302 377 275 272 271 271 373
                271 240 377 236 376 362 236 376 362 236 376 362 R6=040100=4

TODO:
        - Cassette (coded but not working)

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "h8.lh"


class h8_state : public driver_device
{
public:
	h8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_uart(*this, "uart"),
		m_cass(*this, "cassette"),
		m_beep(*this, "beeper")
	{ }

	DECLARE_READ8_MEMBER(portf0_r);
	DECLARE_WRITE8_MEMBER(portf0_w);
	DECLARE_WRITE8_MEMBER(portf1_w);
	DECLARE_WRITE8_MEMBER(h8_status_callback);
	DECLARE_WRITE_LINE_MEMBER(h8_inte_callback);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);
	DECLARE_WRITE_LINE_MEMBER(write_cassette_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(h8_irq_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(h8_c);
	TIMER_DEVICE_CALLBACK_MEMBER(h8_p);
private:
	UINT8 m_digit;
	UINT8 m_segment;
	UINT8 m_irq_ctl;
	bool m_ff_b;
	UINT8 m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
};


#define H8_CLOCK (XTAL_12_288MHz / 6)
#define H8_BEEP_FRQ (H8_CLOCK / 1024)
#define H8_IRQ_PULSE (H8_BEEP_FRQ / 2)


TIMER_DEVICE_CALLBACK_MEMBER(h8_state::h8_irq_pulse)
{
	if (m_irq_ctl & 1)
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xcf);
}

READ8_MEMBER( h8_state::portf0_r )
{
	// reads the keyboard

	// The following not emulated, can occur any time even if keyboard not being scanned
	// - if 0 and RTM pressed, causes int10
	// - if 0 and RST pressed, resets cpu

	UINT8 i,keyin,data = 0xff;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<1);
		data &= 0xfe;
	}

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<5);
		data &= 0xef;
	}
	return data;
}

WRITE8_MEMBER( h8_state::portf0_w )
{
	// this will always turn off int10 that was set by the timer
	// d0-d3 = digit select
	// d4 = int20 is allowed
	// d5 = mon led
	// d6 = int10 is allowed
	// d7 = beeper enable

	m_digit = data & 15;
	if (m_digit) output().set_digit_value(m_digit, m_segment);

	output().set_value("mon_led", !BIT(data, 5));
	m_beep->set_state(!BIT(data, 7));

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	m_irq_ctl &= 0xf0;
	if (BIT(data, 6)) m_irq_ctl |= 1;
	if (!BIT(data, 4)) m_irq_ctl |= 2;
}

WRITE8_MEMBER( h8_state::portf1_w )
{
	//d7 segment dot
	//d6 segment f
	//d5 segment e
	//d4 segment d
	//d3 segment c
	//d2 segment b
	//d1 segment a
	//d0 segment g

	m_segment = 0xff ^ BITSWAP8(data, 7, 0, 6, 5, 4, 3, 2, 1);
	if (m_digit) output().set_digit_value(m_digit, m_segment);
}

static ADDRESS_MAP_START(h8_mem, AS_PROGRAM, 8, h8_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM // main rom
	AM_RANGE(0x1400, 0x17ff) AM_RAM // fdc ram
	AM_RANGE(0x1800, 0x1fff) AM_ROM // fdc rom
	AM_RANGE(0x2000, 0x9fff) AM_RAM // main ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( h8_io, AS_IO, 8, h8_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_READWRITE(portf0_r,portf0_w)
	AM_RANGE(0xf1, 0xf1) AM_WRITE(portf1_w)
	AM_RANGE(0xf8, 0xf8) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0xf9, 0xf9) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	// optional connection to a serial terminal @ 600 baud
	//AM_RANGE(0xfa, 0xfa) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	//AM_RANGE(0xfb, 0xfb) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( h8 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 SP") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 AF") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 BC") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 DE GO") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 HL IN") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 PC OUT") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 SI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 LOAD") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 DUMP") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CANCEL") PORT_CODE(KEYCODE_ESC) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ALTER") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
INPUT_PORTS_END

void h8_state::machine_reset()
{
	m_beep->set_frequency(H8_BEEP_FRQ);
	output().set_value("pwr_led", 0);
	m_irq_ctl = 1;
	m_cass_state = 1;
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_uart->write_rxd(0);
	m_cass_data[3] = 0;
	m_ff_b = 1;
}

WRITE_LINE_MEMBER( h8_state::h8_inte_callback )
{
		// operate the ION LED
	output().set_value("ion_led", !state);
	m_irq_ctl &= 0x7f | ((state) ? 0 : 0x80);
}

WRITE8_MEMBER( h8_state::h8_status_callback )
{
/* This is rather messy, but basically there are 2 D flipflops, one drives the other,
the data is /INTE while the clock is /M1. If the system is in Single Instruction mode,
a int20 (output of 2nd flipflop) will occur after 4 M1 steps, to pause the running program.
But, all of this can only occur if bit 5 of port F0 is low. */

	bool state = (data & I8085_STATUS_M1) ? 0 : 1;
	bool c,a = (m_irq_ctl & 0x80) ? 1 : 0;

	if (m_irq_ctl & 2)
	{
		if (!state) // rising pulse to push data through flipflops
		{
			c = !m_ff_b; // from /Q of 2nd flipflop
			m_ff_b = a; // from Q of 1st flipflop
			if (c)
				m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xd7);
		}
	}
	else
	{ // flipflops are 'set'
		c = 0;
		m_ff_b = 1;
	}


		// operate the RUN LED
	output().set_value("run_led", state);
}

WRITE_LINE_MEMBER( h8_state::txdata_callback )
{
	m_cass_state = state;
}

WRITE_LINE_MEMBER( h8_state::write_cassette_clock )
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(h8_state::h8_c)
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(h8_state::h8_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

static MACHINE_CONFIG_START( h8, h8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, H8_CLOCK)
	MCFG_CPU_PROGRAM_MAP(h8_mem)
	MCFG_CPU_IO_MAP(h8_io)
	MCFG_I8085A_STATUS(WRITE8(h8_state, h8_status_callback))
	MCFG_I8085A_INTE(WRITELINE(h8_state, h8_inte_callback))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_h8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE(h8_state, txdata_callback))

	MCFG_DEVICE_ADD("cassette_clock", CLOCK, 4800)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(h8_state, write_cassette_clock))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("h8_cass")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("h8_c", h8_state, h8_c, attotime::from_hz(4800))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("h8_p", h8_state, h8_p, attotime::from_hz(40000))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("h8_timer", h8_state, h8_irq_pulse, attotime::from_hz(H8_IRQ_PULSE))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( h8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// H17 fdc bios - needed by bios2&3
	ROM_LOAD( "2716_444-19_h17.rom", 0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "bios0", "Standard")
	ROMX_LOAD( "2708_444-13_pam8.rom", 0x0000, 0x0400, CRC(e0745513) SHA1(0e170077b6086be4e5cd10c17e012c0647688c39), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(1, "bios1", "Alternate")
	ROMX_LOAD( "2708_444-13_pam8go.rom", 0x0000, 0x0400, CRC(9dbad129) SHA1(72421102b881706877f50537625fc2ab0b507752), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(2, "bios2", "Disk OS")
	ROMX_LOAD( "2716_444-13_pam8at.rom", 0x0000, 0x0800, CRC(fd95ddc1) SHA1(eb1f272439877239f745521139402f654e5403af), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(3, "bios3", "Disk OS Alt")
	ROMX_LOAD( "2732_444-70_xcon8.rom", 0x0000, 0x1000, CRC(b04368f4) SHA1(965244277a3a8039a987e4c3593b52196e39b7e7), ROM_BIOS(4) )

	// this one runs off into the weeds
	ROM_SYSTEM_BIOS(4, "bios4", "not working")
	ROMX_LOAD( "2732_444-140_pam37.rom", 0x0000, 0x1000, CRC(53a540db) SHA1(90082d02ffb1d27e8172b11fff465bd24343486e), ROM_BIOS(5) )
ROM_END

/* Driver */

/*    YEAR  NAME PARENT  COMPAT MACHINE INPUT    CLASS,         INIT    COMPANY       FULLNAME       FLAGS */
COMP( 1977, h8,  0,       0,    h8,     h8,      driver_device,   0, "Heath, Inc.", "Heathkit H8", MACHINE_NOT_WORKING )
