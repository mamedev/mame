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


****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "speaker.h"

#include "formats/h8_cas.h"
#include "h8.lh"

namespace {

class h8_state : public driver_device
{
public:
	h8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_cass(*this, "cassette")
		, m_beep(*this, "beeper")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_mon_led(*this, "mon_led")
		, m_pwr_led(*this, "pwr_led")
		, m_ion_led(*this, "ion_led")
		, m_run_led(*this, "run_led")
	{ }

	void h8(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(button_0);

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	u8 portf0_r();
	void portf0_w(u8 data);
	void portf1_w(u8 data);
	void h8_status_callback(u8 data);
	void h8_inte_callback(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(h8_irq_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_digit = 0U;
	u8 m_segment = 0U;
	u8 m_irq_ctl = 0U;
	bool m_ff_b = 0;
	u8 m_cass_data[4]{};
	bool m_cassbit = 0;
	bool m_cassold = 0;

	// clocks
	static constexpr XTAL H8_CLOCK = XTAL(12'288'000) / 6; // 2.048 MHz
	static constexpr XTAL H8_BEEP_FRQ = H8_CLOCK / 2048;   // 1 kHz
	static constexpr XTAL H8_IRQ_PULSE = H8_BEEP_FRQ / 2;

	required_device<i8080_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	required_ioport_array<2> m_io_keyboard;
	output_finder<16> m_digits;
	output_finder<> m_mon_led;
	output_finder<> m_pwr_led;
	output_finder<> m_ion_led;
	output_finder<> m_run_led;
};


TIMER_DEVICE_CALLBACK_MEMBER(h8_state::h8_irq_pulse)
{
	if (BIT(m_irq_ctl, 0))
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xcf); // I8080
}

u8 h8_state::portf0_r()
{
	// reads the keyboard

	// The following not emulated, can occur any time even if keyboard not being scanned
	// - if 0 and RTM pressed, causes int10
	// - if 0 and RST pressed, resets cpu

	u8 i,keyin,data = 0xff;

	keyin = m_io_keyboard[0]->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<1);
		data &= 0xfe;
	}

	keyin = m_io_keyboard[1]->read();
	if (keyin != 0xff)
	{
		for (i = 1; i < 8; i++)
			if (!BIT(keyin,i))
				data &= ~(i<<5);
		data &= 0xef;
	}
	return data;
}

void h8_state::portf0_w(u8 data)
{
	// this will always turn off int10 that was set by the timer
	// d0-d3 = digit select
	// d4 = int20 is allowed
	// d5 = mon led
	// d6 = int10 is allowed
	// d7 = beeper enable

	m_digit = data & 15;
	if (m_digit) m_digits[m_digit] = m_segment;

	m_mon_led = !BIT(data, 5);
	m_beep->set_state(!BIT(data, 7));

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	m_irq_ctl &= 0xf0;
	if (BIT(data, 6)) m_irq_ctl |= 1;
	if (!BIT(data, 4)) m_irq_ctl |= 2;
}

void h8_state::portf1_w(u8 data)
{
	//d7 segment dot
	//d6 segment f
	//d5 segment e
	//d4 segment d
	//d3 segment c
	//d2 segment b
	//d1 segment a
	//d0 segment g

	m_segment = 0xff ^ bitswap<8>(data, 7, 0, 6, 5, 4, 3, 2, 1);
	if (m_digit) m_digits[m_digit] = m_segment;
}

void h8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom(); // main rom
	map(0x1400, 0x17ff).ram(); // fdc ram
	map(0x1800, 0x1fff).rom(); // fdc rom
	map(0x2000, 0x9fff).ram(); // main ram
}

void h8_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf0).rw(FUNC(h8_state::portf0_r), FUNC(h8_state::portf0_w));
	map(0xf1, 0xf1).w(FUNC(h8_state::portf1_w));
	map(0xf8, 0xf9).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	// optional connection to a serial terminal @ 600 baud
	//map(0xfa, 0xfb).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( h8 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, h8_state, button_0, 0)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* CANCEL") PORT_CODE(KEYCODE_ESC) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ALTER RST") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("# MEM RTM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(h8_state::button_0)
{
	if (newval)
	{
		u8 data = m_io_keyboard[1]->read() ^ 0xff;
		if (BIT(data, 5))
			m_maincpu->reset();
		else
		if (BIT(data, 6))
			m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xcf); // INT 10   // I8080
	}
}

void h8_state::machine_reset()
{
	m_pwr_led = 0;
	m_irq_ctl = 1;
	m_cassbit = 1;
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_uart->write_cts(0);
	m_uart->write_dsr(0);
	m_uart->write_rxd(0);
	m_cass_data[3] = 0;
	m_ff_b = 1;
}

void h8_state::machine_start()
{
	m_digits.resolve();
	m_mon_led.resolve();
	m_pwr_led.resolve();
	m_ion_led.resolve();
	m_run_led.resolve();

	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
	save_item(NAME(m_irq_ctl));
	save_item(NAME(m_ff_b));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void h8_state::h8_inte_callback(int state)
{
	// operate the ION LED
	m_ion_led = !state;
	m_irq_ctl &= 0x7f | ((state) ? 0 : 0x80);
}

void h8_state::h8_status_callback(u8 data)
{
/* This is rather messy, but basically there are 2 D flipflops, one drives the other,
the data is /INTE while the clock is /M1. If the system is in Single Instruction mode,
a int20 (output of 2nd flipflop) will occur after 4 M1 steps, to pause the running program.
But, all of this can only occur if bit 4 of port F0 is low. */

	bool state = (data & i8080_cpu_device::STATUS_M1) ? 0 : 1;
	bool c,a = BIT(m_irq_ctl, 7);

	if (BIT(m_irq_ctl, 1))
	{
		if (!state) // rising pulse to push data through flipflops
		{
			c = !m_ff_b; // from /Q of 2nd flipflop
			m_ff_b = a; // from Q of 1st flipflop
			if (c)
				m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xd7); // I8080
		}
	}
	else
	{ // flipflops are 'set'
		c = 0;
		m_ff_b = 1;
	}


	// operate the RUN LED
	m_run_led = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(h8_state::kansas_w)
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(h8_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void h8_state::h8(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, H8_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &h8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &h8_state::io_map);
	m_maincpu->out_status_func().set(FUNC(h8_state::h8_status_callback));
	m_maincpu->out_inte_func().set(FUNC(h8_state::h8_inte_callback));

	/* video hardware */
	config.set_default_layout(layout_h8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, H8_BEEP_FRQ).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* Devices */
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &cassette_clock(CLOCK(config, "cassette_clock", 4800));
	cassette_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	cassette_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	CASSETTE(config, m_cass);
	m_cass->set_formats(h8_cassette_formats);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("h8_cass");

	TIMER(config, "kansas_w").configure_periodic(FUNC(h8_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(h8_state::kansas_r), attotime::from_hz(40000));
	TIMER(config, "h8_timer").configure_periodic(FUNC(h8_state::h8_irq_pulse), attotime::from_hz(H8_IRQ_PULSE));
}

/* ROM definition */
ROM_START( h8 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	// H17 fdc bios - needed by bios2&3
	ROM_LOAD( "2716_444-19_h17.rom", 0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "bios0", "Standard")
	ROMX_LOAD( "2708_444-13_pam8.rom", 0x0000, 0x0400, CRC(e0745513) SHA1(0e170077b6086be4e5cd10c17e012c0647688c39), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "bios1", "Alternate")
	ROMX_LOAD( "2708_444-13_pam8go.rom", 0x0000, 0x0400, CRC(9dbad129) SHA1(72421102b881706877f50537625fc2ab0b507752), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "bios2", "Disk OS")
	ROMX_LOAD( "2716_444-13_pam8at.rom", 0x0000, 0x0800, CRC(fd95ddc1) SHA1(eb1f272439877239f745521139402f654e5403af), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "bios3", "Disk OS Alt")
	ROMX_LOAD( "2732_444-70_xcon8.rom", 0x0000, 0x1000, CRC(b04368f4) SHA1(965244277a3a8039a987e4c3593b52196e39b7e7), ROM_BIOS(3) )

	// this one runs off into the weeds
	ROM_SYSTEM_BIOS(4, "bios4", "not working")
	ROMX_LOAD( "2732_444-140_pam37.rom", 0x0000, 0x1000, CRC(53a540db) SHA1(90082d02ffb1d27e8172b11fff465bd24343486e), ROM_BIOS(4) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT    CLASS,    INIT          COMPANY           FULLNAME                      FLAGS */
COMP( 1977, h8,   0,      0,      h8,      h8,      h8_state, empty_init, "Heath Company", "Heathkit H8 Digital Computer", MACHINE_SUPPORTS_SAVE )
