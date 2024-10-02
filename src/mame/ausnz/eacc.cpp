// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************
*
*    EA Car Computer
*        by Robbbert, March 2011.
*
*    Described in Electronics Australia magazine during 1982.
*    Construction and usage: http://messui.polygonal-moogle.com/comp/eacc.pdf
*
*    The only RAM is the 128 bytes that comes inside the CPU.
*
*    This computer is mounted in a car, and various sensors (fuel flow, etc)
*    are connected up. By pressing the appropriate buttons various statistics
*    may be obtained.
*
*    Memory Map
*    0000-007F internal ram
*    4000-7FFF ROM
*    8000-BFFF 6821
*    C000-FFFF ROM (mirror)
*
*    The ROM was typed in twice from the dump in the magazine article, and the
*    results compared. Only one byte was different, so I can be confident that
*    it has been typed in properly.
*
*    Setting up: You need to enter the number of expected pulses from the fuel
*    and distance sensors. Paste this: 5 6M123N 7M400N  (start, set litres cal to
*    123 pulses. set km cal to 400 pulses). Then paste this: 1950M0N 1845M0N (set
*    petrol tank capacity to 50 litres, set current amount of petrol to 45).
*    Now enter: 28M100N (the journey is 100km). Press 5 to start the journey.
*    All settings are saved in nvram.
*
*    Stats you can see while travelling:
*    0  - time elapsed
*    08 - time remaining
*    1  - fuel used
*    18 - fuel left
*    2  - km travelled
*    28 - km remaining
*    29 - km that could be travelled with the fuel you have left
*    3  - speed now
*    39 - average speed
*    4  - fuel consumption now (litres per 100km)
*    49 - fuel average consumption
*
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "eacc.lh"
#include "machine/6821pia.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/nvram.h"


namespace {

class eacc_state : public driver_device
{
public:
	eacc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
		, m_p_nvram(*this, "nvram")
		, m_7474(*this, "7474")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void eacc(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void scan_w(int state);
	void cb2_w(int state);
	void inputs_w(int state);
	uint8_t keyboard_r();
	void digit_w(uint8_t data);
	void segment_w(uint8_t data);
	void mem_map(address_map &map) ATTR_COLD;
	void do_nmi(bool, bool);
	uint8_t m_digit = 0U;
	bool m_cb2 = false;
	bool m_scan = false;
	bool m_disp = false;

	required_device<m6802_cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_shared_ptr<uint8_t> m_p_nvram;
	required_device<ttl7474_device> m_7474;
	required_ioport_array<4> m_io_keyboard;
	output_finder<7> m_digits;
	output_finder<8> m_leds;
};




/******************************************************************************
 Address Maps
******************************************************************************/

void eacc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xc7ff); // A11,A12,A13 not connected
	map(0x0000, 0x001f).ram().share("nvram"); // inside cpu, battery-backed
	map(0x0020, 0x007f).ram(); // inside cpu
	map(0x4000, 0x47ff).rom().mirror(0x8000).region("maincpu",0);
	map(0x8000, 0x8003).mirror(0x7fc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(eacc)
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 Litres Cal") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 km") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0xf8, 0, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 START") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("END") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('N')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 km/h") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0xf8, 0, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 Km Cal") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 hour.min") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 l/100km") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0xf8, 0, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 REM") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 litres") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 AV") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0xf8, 0, IPT_UNUSED )
INPUT_PORTS_END

void eacc_state::machine_reset()
{
	m_cb2 = false; // pia is supposed to set this low at reset
	m_digit = 0;
}

void eacc_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_cb2));
	save_item(NAME(m_scan));
	save_item(NAME(m_digit));
	save_item(NAME(m_disp));
}

void eacc_state::inputs_w(int state)
{
	if (state)
		m_pia->ca1_w(machine().rand() & 1); // movement
	else
		m_pia->ca2_w(machine().rand() & 1); // fuel usage
}

void eacc_state::do_nmi(bool in_scan, bool in_cb2)
{
	if (in_scan && in_cb2)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_7474->clock_w(0);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_7474->clock_w(1);
	}
}

void eacc_state::cb2_w(int state)
{
	m_cb2 = state ? 1 : 0;
	do_nmi(m_scan, m_cb2);
}

void eacc_state::scan_w(int state)
{
	m_scan = state ? 1 : 0;
	do_nmi(m_scan, m_cb2);
}

uint8_t eacc_state::keyboard_r()
{
	uint8_t data = m_digit;

	for (uint8_t i = 3; i < 7; i++)
		if (BIT(m_digit, i))
			data |= m_io_keyboard[i - 3]->read();

	return data;
}

void eacc_state::segment_w(uint8_t data)
{
	//d7 segment dot
	//d6 segment c
	//d5 segment d
	//d4 segment e
	//d3 segment a
	//d2 segment b
	//d1 segment f
	//d0 segment g

	if (m_disp)
	{
		m_disp = false;
		if (BIT(m_digit, 7))
		{
			data ^= 0xff;

			for (uint8_t i = 0; i < 8; i++)
				m_leds[i] = BIT(data, i);
		}
		else
		{
			for (uint8_t i = 3; i < 7; i++)
				if (BIT(m_digit, i))
					m_digits[i] = bitswap<8>(data, 7, 0, 1, 4, 5, 6, 2, 3);
		}
	}
}

void eacc_state::digit_w(uint8_t data)
{
	m_digit = data & 0xf8;
	m_disp = true;
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

void eacc_state::eacc(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(3'579'545));  /* Divided by 4 inside the m6802*/
	m_maincpu->set_ram_enable(false); // FIXME: needs standby support
	m_maincpu->set_addrmap(AS_PROGRAM, &eacc_state::mem_map);

	config.set_default_layout(layout_eacc);

	PIA6821(config, m_pia);
	m_pia->readpb_handler().set(FUNC(eacc_state::keyboard_r));
	m_pia->writepa_handler().set(FUNC(eacc_state::segment_w));
	m_pia->writepb_handler().set(FUNC(eacc_state::digit_w));
	m_pia->cb2_handler().set(FUNC(eacc_state::cb2_w));
	m_pia->irqa_handler().set_inputline("maincpu", M6802_IRQ_LINE);
	m_pia->irqb_handler().set_inputline("maincpu", M6802_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TTL7474(config, m_7474, 0);
	m_7474->output_cb().set(m_pia, FUNC(pia6821_device::cb1_w));

	clock_device &eacc_scan(CLOCK(config, "eacc_scan", 600)); // 74C14 with 100k & 10nF = 1200Hz, but article says 600.
	eacc_scan.signal_handler().set(FUNC(eacc_state::scan_w));

	clock_device &eacc_cb1(CLOCK(config, "eacc_cb1", 15)); // cpu E -> MM5369 -> 15Hz
	eacc_cb1.signal_handler().set(m_7474, FUNC(ttl7474_device::d_w));

	clock_device &eacc_rnd(CLOCK(config, "eacc_rnd", 30)); // random pulse for distance and fuel
	eacc_rnd.signal_handler().set(FUNC(eacc_state::inputs_w));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(eacc)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("eacc.bin", 0x0000, 0x0800, CRC(287a63c0) SHA1(f61b397d33ea40e5742e34d5f5468572125e8b39) )
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                  FULLNAME           FLAGS
COMP( 1982, eacc, 0,      0,      eacc,    eacc,  eacc_state, empty_init, "Electronics Australia", "EA Car Computer", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
