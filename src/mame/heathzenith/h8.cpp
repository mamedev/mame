// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Heathkit H8

    This system uses Octal and Split-Octal rather than the usual hexadecimal.

    STATUS:
        It runs, keyboard works, you can enter data.
        Serial console works. You can make it visible by setting Video
        Options in settings.

    Meaning of LEDs:
        PWR = Power is turned on (+5V is present at on front panel)
        MON = The front panel is being serviced by the cpu (controls should work)
        RUN = CPU is running (not halted)
        ION = Interrupts are enabled

    Pasting:
        H8    | mame key
    -----------------------
        0-F   | as is
        +     |   ^
        -     |   V
        MEM   |   -
        ALTER |   =

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
      - Move 8080 CPU to a card
      - Move Front Panel board to a card.

****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "sound/beep.h"

#include "speaker.h"

#include "bus/heathzenith/h8/h8bus.h"
#include "bus/heathzenith/h8/h_8_1.h"
#include "bus/heathzenith/h8/h_8_5.h"
#include "bus/heathzenith/h8/wh_8_64.h"
#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"

#include "h8.lh"

namespace {

class h8_state : public driver_device
{
public:
	h8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_intr_socket(*this, "intr_socket")
		, m_h8bus(*this, "h8bus")
		, m_beep(*this, "beeper")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_mon_led(*this, "mon_led")
		, m_pwr_led(*this, "pwr_led")
		, m_ion_led(*this, "ion_led")
		, m_run_led(*this, "run_led")
		{}

	void h8(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(button_0);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	template <int line> void slot_irq(int state);

private:
	u8 portf0_r();
	void portf0_w(u8 data);
	void portf1_w(u8 data);
	void h8_status_callback(u8 data);
	void h8_inte_callback(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(h8_irq_pulse);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_digit = 0U;
	u8 m_segment = 0U;
	u8 m_irq_ctl = 0U;
	bool m_ff_b = 0;

	// clocks
	static constexpr XTAL H8_CLOCK = XTAL(12'288'000) / 6; // 2.048 MHz
	static constexpr XTAL H8_BEEP_FRQ = H8_CLOCK / 2048;   // 1 kHz
	static constexpr XTAL H8_IRQ_PULSE = H8_BEEP_FRQ / 2;

	required_device<i8080_cpu_device> m_maincpu;
	required_device<heath_intr_socket> m_intr_socket;
	required_device<h8bus_device> m_h8bus;
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
	{
		m_intr_socket->set_irq_level(1, ASSERT_LINE);
	}
}

u8 h8_state::portf0_r()
{
	// reads the keyboard

	// The following can occur any time even if keyboard not being scanned
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

	m_digit = data & 0xf;
	if (m_digit) m_digits[m_digit] = m_segment;

	m_mon_led = !BIT(data, 5);
	m_beep->set_state(!BIT(data, 7));

	m_intr_socket->set_irq_level(1, CLEAR_LINE);

	m_irq_ctl &= 0xf0;
	if (BIT(data, 6)) m_irq_ctl |= 1;
	if (!BIT(data, 4)) m_irq_ctl |= 2;
}

void h8_state::portf1_w(u8 data)
{
	// d7 segment dot
	// d6 segment f
	// d5 segment e
	// d4 segment d
	// d3 segment c
	// d2 segment b
	// d1 segment a
	// d0 segment g

	m_segment = 0xff ^ bitswap<8>(data, 7, 0, 6, 5, 4, 3, 2, 1);
	if (m_digit) m_digits[m_digit] = m_segment;
}

void h8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom(); // main rom
	map(0x1400, 0x17ff).ram(); // fdc ram
	map(0x1800, 0x1fff).rom(); // fdc rom
}

void h8_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf0).rw(FUNC(h8_state::portf0_r), FUNC(h8_state::portf0_w));
	map(0xf1, 0xf1).w(FUNC(h8_state::portf1_w));
}


// Input ports
static INPUT_PORTS_START( h8 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0")           PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(h8_state::button_0), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 SP")        PORT_CODE(KEYCODE_1)      PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 AF")        PORT_CODE(KEYCODE_2)      PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 BC")        PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 DE GO")     PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 HL IN")     PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 PC OUT")    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 SI")        PORT_CODE(KEYCODE_7)      PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 LOAD")      PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 DUMP")      PORT_CODE(KEYCODE_9)      PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+")           PORT_CODE(KEYCODE_UP)     PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-")           PORT_CODE(KEYCODE_DOWN)   PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* CANCEL")    PORT_CODE(KEYCODE_ESC)    PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ALTER RST") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("# MEM RTM")   PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". REG")       PORT_CODE(KEYCODE_R)      PORT_CHAR('R')
#
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(h8_state::button_0)
{
	if (newval)
	{
		u8 data = m_io_keyboard[1]->read() ^ 0xff;
		if (BIT(data, 5))
		{
			m_maincpu->reset();
		}
		else if (BIT(data, 6))
		{
			m_intr_socket->set_irq_level(1, ASSERT_LINE);
		}
	}
}

void h8_state::machine_reset()
{
	m_pwr_led = 0;
	m_irq_ctl = 1;
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
}

void h8_state::h8_inte_callback(int state)
{
	// operate the ION LED
	m_ion_led = !state;
	m_irq_ctl &= 0x7f | ((state) ? 0 : 0x80);
}

void h8_state::h8_status_callback(u8 data)
{
	// This is rather messy, but basically there are 2 D flipflops, one drives the other,
	// the data is /INTE while the clock is /M1. If the system is in Single Instruction mode,
	// a int20 (output of 2nd flipflop) will occur after 4 M1 steps, to pause the running program.
	// But, all of this can only occur if bit 4 of port F0 is low. */

	bool state = (data & i8080_cpu_device::STATUS_M1) ? 0 : 1;
	bool c, a = BIT(m_irq_ctl, 7);

	if (BIT(m_irq_ctl, 1))
	{
		if (!state) // rising pulse to push data through flipflops
		{
			c = !m_ff_b; // from /Q of 2nd flipflop
			m_ff_b = a; // from Q of 1st flipflop
			if (c)
			{
				m_intr_socket->set_irq_level(2, ASSERT_LINE);
			}
		}
	}
	else
	{
		// flipflops are 'set'
		c = 0;
		m_ff_b = 1;
	}

	// operate the RUN LED
	m_run_led = state;
}

template <int line> void h8_state::slot_irq(int state)
{
	m_intr_socket->set_irq_level(line, state);
}

template void h8_state::slot_irq<0>(int state);
template void h8_state::slot_irq<1>(int state);
template void h8_state::slot_irq<2>(int state);
template void h8_state::slot_irq<3>(int state);
template void h8_state::slot_irq<4>(int state);
template void h8_state::slot_irq<5>(int state);
template void h8_state::slot_irq<6>(int state);
template void h8_state::slot_irq<7>(int state);


static void intr_ctrl_options(device_slot_interface &device)
{
	device.option_add("original", HEATH_INTR_CNTRL);
}

// TODO - move Front Panel functionality to a card.
// P1 was reserved for the Front Panel
static void h8_p1_cards(device_slot_interface &device)
{
}

// TODO - move CPU functionality on to a card, add options for 8080 and Z80
// P2 was reserved for the CPU board.
static void h8_p2_cards(device_slot_interface &device)
{
}

static void h8_cards(device_slot_interface &device)
{
	device.option_add("h_8_1",       H8BUS_H_8_1);
	device.option_add("h_8_1_4k",    H8BUS_H_8_1_4K);
	device.option_add("h_8_5",       H8BUS_H_8_5);
	device.option_add("wh_8_64",     H8BUS_WH_8_64);
	device.option_add("wh_8_64_48k", H8BUS_WH_8_64_48K);
	device.option_add("wh_8_64_32k", H8BUS_WH_8_64_32K);
}

void h8_state::h8(machine_config &config)
{
	// basic machine hardware
	I8080(config, m_maincpu, H8_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &h8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &h8_state::io_map);
	m_maincpu->out_status_func().set(FUNC(h8_state::h8_status_callback));
	m_maincpu->out_inte_func().set(FUNC(h8_state::h8_inte_callback));
	m_maincpu->set_irq_acknowledge_callback("intr_socket", FUNC(heath_intr_socket::irq_callback));

	// system layout
	config.set_default_layout(layout_h8);

	HEATH_INTR_SOCKET(config, m_intr_socket, intr_ctrl_options, nullptr);
	m_intr_socket->irq_line_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_intr_socket->set_default_option("original");
	m_intr_socket->set_fixed(true);

	H8BUS(config, m_h8bus, 0);
	m_h8bus->set_program_space(m_maincpu, AS_PROGRAM);
	m_h8bus->set_io_space(m_maincpu, AS_IO);
	m_h8bus->out_int0_callback().set(FUNC(h8_state::slot_irq<0>));
	m_h8bus->out_int1_callback().set(FUNC(h8_state::slot_irq<1>));
	m_h8bus->out_int2_callback().set(FUNC(h8_state::slot_irq<2>));
	m_h8bus->out_int3_callback().set(FUNC(h8_state::slot_irq<3>));
	m_h8bus->out_int4_callback().set(FUNC(h8_state::slot_irq<4>));
	m_h8bus->out_int5_callback().set(FUNC(h8_state::slot_irq<5>));
	m_h8bus->out_int6_callback().set(FUNC(h8_state::slot_irq<6>));
	m_h8bus->out_int7_callback().set(FUNC(h8_state::slot_irq<7>));

	H8BUS_SLOT(config,  "p1", "h8bus", h8_p1_cards, nullptr);
	H8BUS_SLOT(config,  "p2", "h8bus", h8_p2_cards, nullptr);
	H8BUS_SLOT(config,  "p3", "h8bus", h8_cards, "wh_8_64_32k");
	H8BUS_SLOT(config,  "p4", "h8bus", h8_cards, nullptr);
	H8BUS_SLOT(config,  "p5", "h8bus", h8_cards, nullptr);
	H8BUS_SLOT(config,  "p6", "h8bus", h8_cards, nullptr);
	H8BUS_SLOT(config,  "p7", "h8bus", h8_cards, nullptr);
	H8BUS_SLOT(config,  "p8", "h8bus", h8_cards, nullptr);
	H8BUS_SLOT(config,  "p9", "h8bus", h8_cards, "h_8_5");
	H8BUS_SLOT(config, "p10", "h8bus", h8_cards, nullptr);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, H8_BEEP_FRQ).add_route(ALL_OUTPUTS, "mono", 1.00);

	TIMER(config, "h8_timer").configure_periodic(FUNC(h8_state::h8_irq_pulse), attotime::from_hz(H8_IRQ_PULSE));
}

// ROM definition
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


// Driver

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT    CLASS,    INIT        COMPANY          FULLNAME                        FLAGS
COMP( 1977, h8,   0,      0,      h8,      h8,      h8_state, empty_init, "Heath Company", "Heathkit H8 Digital Computer", MACHINE_SUPPORTS_SAVE )
