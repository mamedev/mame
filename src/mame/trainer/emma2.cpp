// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Emma and Emma II by L.J.Technical Systems
These were produced by L.J.Electronics Ltd, Norwich in 1979.

2018-09-20 Working driver.

We don't have roms for the Emma, so only the Emma II is emulated.

Keys:
0-9,A-F: hex input
M      : memory mode (modify address, or modify data)
G      : go
R      : single step (not emulated)
P      : insert a forced break
S      : save to cassette
L      : load from cassette
+      : up (use UP-arrow key)
-      : down (use DOWN-arrow key)

Pasting example:
M0020M01^02^03^04^05^06^07^08^09^M000020
Press up arrow to see your input.

To save, press S, enter beginning address, press S, enter end address+1,
press S, start the recorder, press 0 for 300 baud or 1 for 1200 baud. The
data is saved.

To load, press L, press play, while leader is playing press 0 or 1 as above.
When a row of dots appears, the data has been loaded.


There's an expansion unit with its own rom, a 8255, an eprom programmer,
and 16k of static RAM.



To Do:
- Cassette LED doesn't work.
- Code up the expansion unit (rom is there but no schematic exists)
- Need software.


****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "imagedev/cassette.h"
#include "speaker.h"
#include "video/pwm.h"
#include "emma2.lh"


namespace {

class emma2_state : public driver_device
{
public:
	emma2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_via(*this, "via")
		, m_pia(*this, "pia")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void emma2(machine_config &config);

private:
	void segment_w(uint8_t data);
	void digit_w(uint8_t data);
	uint8_t keyboard_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_digit = 0U;
	uint8_t m_seg = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<via6522_device> m_via;
	required_device<pia6821_device> m_pia;
	required_device<pwm_display_device> m_display;
	required_ioport_array<8> m_io_keyboard;
};

void emma2_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x0900, 0x090f).mirror(0x02f0).m(m_via, FUNC(via6522_device::map));
	map(0x0a00, 0x0a03).mirror(0x00fc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0fff).ram();
	map(0x9000, 0x97ff).rom().region("maincpu", 0);
	map(0xd800, 0xdfff).mirror(0x2000).rom().region("maincpu", 0x0800);
}


/* Input ports */
static INPUT_PORTS_START( emma2 )
/*
M   L   0   4   8   C
G   R   1   5   9   D
P   +   2   6   A   E
S   -   3   7   B   F
*/

	PORT_START("X0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("X1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')

	PORT_START("X2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')

	PORT_START("X3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("X4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')

	PORT_START("X5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("X6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("X7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
INPUT_PORTS_END


void emma2_state::digit_w(uint8_t data)
{
	m_cassette->output( BIT(data, 6) ? +1.0 : -1.0);

	m_digit = data & 7;
	m_display->matrix(1 << m_digit, m_seg);
}

void emma2_state::segment_w(uint8_t data)
{
	m_seg = data;
	m_display->matrix(1 << m_digit, m_seg);
}

uint8_t emma2_state::keyboard_r()
{
	u8 data = m_io_keyboard[m_digit]->read();
	data |= ((m_cassette)->input() < 0.0) ? 0x80 : 0;
	return data;
}

void emma2_state::machine_reset()
{
	m_seg = 0;
	m_digit = 0;
}

void emma2_state::machine_start()
{
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}

void emma2_state::emma2(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &emma2_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_emma2);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	/* Devices */
	MOS6522(config, m_via, 1'000'000);  // #2 from cpu
	m_via->irq_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(emma2_state::segment_w));
	m_pia->writepb_handler().set(FUNC(emma2_state::digit_w));
	m_pia->readpb_handler().set(FUNC(emma2_state::keyboard_r));
	m_pia->ca2_handler().set_output("led0");
	m_pia->irqa_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	m_pia->irqb_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}


/* ROM definition */
ROM_START( emma2 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "se118a_90-97", 0x0000, 0x0800, CRC(32d36938) SHA1(910fd1c18c7deae83933c7c4f397103a35bf574a) ) // 0x9000
	ROM_LOAD( "se116a_d8-dd_fe-ff", 0x0800, 0x0800, CRC(ef0f1513) SHA1(46089ba0402828b4204812a04134b313d9be0f93) ) // 0xd800
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME           FLAGS
COMP( 1979, emma2,  0,      0,      emma2,   emma2,  emma2_state,  empty_init, "L.J.Technical Systems",   "Emma II trainer", MACHINE_SUPPORTS_SAVE )
