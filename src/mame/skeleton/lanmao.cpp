// license:BSD-3-Clause
// copyright-holders:

/*
蓝猫 - Lán Māo - Blue Cat

Mechanical redemption game
Seems to be very similar to misc/marywu.cpp, only with more sound chips
and a different CPU of the MCS51 family
Same "Music by: SunKiss Chen" strings
Same or at least extremely similar lamps / LEDs layout

'Custom Made By LZY-P' PCB

W78E065 CPU (I8052 compatible)
11 MHz XTAL
KC8279P KDC (I8279 compatible)
24C02 EEPROM
2x KC89C72 (AY8910 compatible)
YM2413
3.579 MHz XTAL
Oki M6295 (or clone, not readable)
3x switch

TODO:
- most everything
- stuck at error 5 (永久记忆部分数据出倍)

Schematics and manual with list of error codes are available.

Initialization (currently doesn't work due to preliminary emulation):
On first power-up the machine must be zeroed:
- Hold K0 + K3 while switching on
- Press the Start button once
- Power-cycle the machine
After this sequence it will run normally.
*/


#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "machine/i2cmem.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "speaker.h"

#include "marywu.lh"


// configurable logging
#define LOG_PORTS8052     (1U << 1)
#define LOG_PORTS8279     (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_PORTS8052)

#include "logmacro.h"

#define LOGPORTS8052(...)     LOGMASKED(LOG_PORTS8052,     __VA_ARGS__)
#define LOGPORTS8279(...)     LOGMASKED(LOG_PORTS8279,     __VA_ARGS__)


namespace {

class lanmao_state : public driver_device
{
public:
	lanmao_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" }),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void lanmao(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8052_device> m_maincpu;

	required_ioport_array<4> m_inputs;
	output_finder<32> m_digits;
	output_finder<30> m_leds;

	uint8_t m_kbd_line = 0;

	template <uint8_t Which> void leds_w(uint8_t data);
	void display_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void lanmao_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_kbd_line));
}

template <uint8_t Which>
void lanmao_state::leds_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if ((i + (Which * 8)) < 30) // only 30 LEDs
			m_leds[i + (Which * 8)] = BIT(data, i);
	}
}

void lanmao_state::display_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 };

	m_digits[2 * m_kbd_line + 0] = patterns[data & 0x0f];
	m_digits[2 * m_kbd_line + 1] = patterns[data >> 4];
}


void lanmao_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void lanmao_state::data_map(address_map &map)
{
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write)); // according to schematics
	map(0xd000, 0xd000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


// TODO apart from DSW, the other are inputs that are only set as DIPs for easier testing)
static INPUT_PORTS_START( lanmao )
	PORT_START("KEYS1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "KEYS1:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "KEYS1:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "KEYS1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "KEYS1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "KEYS1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "KEYS1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "KEYS1:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "KEYS1:8")

	PORT_START("KEYS2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "KEYS2:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "KEYS2:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "KEYS2:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "KEYS2:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "KEYS2:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "KEYS2:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "KEYS2:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "KEYS2:8")

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "PB:1") // should be K0
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "PB:2") // should be K1
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "PB:3") // should be K2
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "PB:4") // should be K3
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "PB:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "PB:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "PB:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "PB:8")

	PORT_START("P1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P1:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P1:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "P1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "P1:7") // hopper sensor, gives error 31 if high
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "P1:8") // reset switch

	PORT_START("P3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P3:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P3:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P3:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "P3:4") // needs to be low to avoid error 30 (coin acceptor)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P3:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P3:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "P3:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "P3:8")
INPUT_PORTS_END


void lanmao_state::lanmao(machine_config &config)
{
	I8052(config, m_maincpu, 11_MHz_XTAL); // actually W78E065
	m_maincpu->set_addrmap(AS_PROGRAM, &lanmao_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &lanmao_state::data_map);
	m_maincpu->port_in_cb<1>().set_ioport("P1");
	m_maincpu->port_in_cb<3>().set_ioport("P3");
	m_maincpu->port_out_cb<1>().set([this] (uint8_t data) { LOGPORTS8052("%s I8052 port 1 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS8052("%s I8052 port 3 write: %02x\n", machine().describe_context(), data); });

	i8279_device &kdc(I8279(config, "kdc", 11_MHz_XTAL / 6 )); // TODO: divider
	kdc.out_irq_callback().set([this] (int state) { LOGPORTS8279("%s I8279 irq write: %02x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.out_sl_callback().set([this] (uint8_t data) { m_kbd_line = data; }); // 4 bit port
	kdc.out_disp_callback().set(FUNC(lanmao_state::display_w)); // to 7-seg LEDs through to 2 CD4511 according to schematics
	kdc.out_bd_callback().set([this] (int state) { LOGPORTS8279("%s I8279 bd write: %01x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.in_rl_callback().set_ioport(m_inputs[m_kbd_line & 0x0f]);
	kdc.in_shift_callback().set([this] () { LOGPORTS8279("%s I8279 shift read\n", machine().describe_context()); return 1; }); // not connected according to schematics
	kdc.in_ctrl_callback().set([this] () { LOGPORTS8279("%s I8279 ctrl read\n", machine().describe_context()); return 1; }); // not connected according to schematics

	I2C_24C02(config, "i2cmem");

	config.set_default_layout(layout_marywu);

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay1(AY8910(config, "ay1", 11_MHz_XTAL / 6)); // TODO: divider
	ay1.port_a_write_callback().set(FUNC(lanmao_state::leds_w<0>));
	ay1.port_b_write_callback().set(FUNC(lanmao_state::leds_w<1>));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay2(AY8910(config, "ay2", 11_MHz_XTAL / 6)); // TODO: divider
	ay2.port_a_write_callback().set(FUNC(lanmao_state::leds_w<2>));
	ay2.port_b_write_callback().set(FUNC(lanmao_state::leds_w<3>));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki", 11_MHz_XTAL / 10, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // TODO: divider, pin 7
}


ROM_START( lanmao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w78e065", 0x00000, 0x10000, CRC(57a89c7f) SHA1(be54c40ae17df1155b2aaa67b4b0bf45d307eba1) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "w27e040-12", 0x00000, 0x80000, CRC(4481a891) SHA1(09448bdca052b27828a4797243eee96b98f3d924) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD( "24c02", 0x000, 0x100, CRC(1b95e93a) SHA1(c2697dc5f7ee1e3d73f98ea4ae3a1b71eeacbf07) )
ROM_END

} // anonymous namespace


GAME( 2003, lanmao, 0, lanmao, lanmao, lanmao_state, empty_init, ROT0, "Changsheng Electric Company", "Lan Mao", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
