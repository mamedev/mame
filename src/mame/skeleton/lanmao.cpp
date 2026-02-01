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
10.7386 MHz XTAL
KC8279P KDC (I8279 compatible)
24C02 EEPROM
2x KC89C72 (AY8910 compatible)
YM2413
3.579 MHz XTAL
Oki M6295 (or clone, not readable)
1 MHz resonator
3x switch

TODO (both):
- SVG / less simplistic layout?

TODO (panda2):
- stuck at error 02 (missing meter counter)

Schematics and manual with list of error codes are available.

Initialization (not necessary in MAME due to pre-initalized NVRAM, unless coinage DIPs are changed):
On first power-up the machine must be zeroed:
- Hold K0 + K3 while switching on
- Press the Start button once
- Power-cycle the machine
After this sequence it will run normally.
*/


#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "machine/at28c16.h"
#include "machine/i2cmem.h"
#include "machine/i8279.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
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

class panda2_state : public driver_device
{
public:
	panda2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hopper(*this, "hopper"),
		m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" }),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void panda2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<i8052_device> m_maincpu;
	required_device<hopper_device> m_hopper;

	required_ioport_array<4> m_inputs;
	output_finder<32> m_digits;
	output_finder<31> m_leds;

	uint8_t m_kbd_line = 0;

	template <uint8_t Which> void leds_w(uint8_t data);
	void display_w(uint8_t data);
	uint8_t keyboard_r();
	void port1_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};

class lanmao_state : public panda2_state
{
public:
	lanmao_state(const machine_config &mconfig, device_type type, const char *tag) :
		panda2_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem"),
		m_oki(*this, "oki")
	{ }

	void lanmao(machine_config &config) ATTR_COLD;

private:
	required_device<i2cmem_device> m_i2cmem;
	required_device<okim6295_device> m_oki;

	void port1_w(uint8_t data);
	void port3_w(uint8_t data);

	void data_map(address_map &map) ATTR_COLD;
};


void panda2_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_kbd_line));
}

template <uint8_t Which>
void panda2_state::leds_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if ((i + (Which * 8)) < 31) // only 31 LEDs
			m_leds[i + (Which * 8)] = BIT(data, i);
	}
}

void panda2_state::display_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 };

	m_digits[2 * m_kbd_line + 0] = patterns[data & 0x0f];
	m_digits[2 * m_kbd_line + 1] = patterns[data >> 4];
}

uint8_t panda2_state::keyboard_r()
{
	switch (m_kbd_line & 0x07)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return m_inputs[m_kbd_line & 0x07]->read();
		default:
			return 0x00;
	}
}

void panda2_state::port1_w(uint8_t data)
{
	m_hopper->motor_w(BIT(data, 3));

	if ((data & 0xf7) != 0xf7)
		logerror("unknown port1 write: %02x\n", data);
}

void lanmao_state::port1_w(uint8_t data)
{
	m_i2cmem->write_sda(BIT(data, 1));
	m_i2cmem->write_scl(BIT(data, 2));
	m_hopper->motor_w(BIT(data, 3));

	if ((data & 0xf1) != 0xf1)
		logerror("unknown port1 write: %02x\n", data);
}

void lanmao_state::port3_w(uint8_t data)
{
	if ((data & 0xdf) != 0xdf)
		logerror("unknown port3 write: %02x\n", data);

	m_oki->set_rom_bank(BIT(data, 5));
}

void panda2_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void panda2_state::data_map(address_map &map)
{
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x9000, 0x9001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xa000, 0xa7ff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write));
	map(0xb000, 0xb001).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write)); // according to schematics and present on PCB, but doesn't seem used?
}

void lanmao_state::data_map(address_map &map)
{
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x9000, 0x9001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write)); // according to schematics and present on PCB, but doesn't seem used?
	map(0xd000, 0xd000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( panda2 ) // TODO: check everything once possible
	PORT_START("KEYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 1" ) PORT_CODE( KEYCODE_1_PAD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 2" ) PORT_CODE( KEYCODE_2_PAD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 3" ) PORT_CODE( KEYCODE_3_PAD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 4" ) PORT_CODE( KEYCODE_4_PAD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 5" ) PORT_CODE( KEYCODE_5_PAD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 6" ) PORT_CODE( KEYCODE_6_PAD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 7" ) PORT_CODE( KEYCODE_7_PAD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 8" ) PORT_CODE( KEYCODE_8_PAD )

	PORT_START("KEYS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Single" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Shift Right" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Shift Left" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME( "Double" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Running Lights Difficulty" )  PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x03, "3" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x02, "2" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x01, "1" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x00, "0" ) // Manual doesn't list the settings
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Double Up Difficulty" )  PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "1" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x00, "0" ) // Manual doesn't list the settings
	PORT_DIPNAME( 0x10, 0x10, "Bet Ratio" )  PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "1/1" )
	PORT_DIPSETTING(    0x00, "1/5" )
	// the following will give error 21 (coin ratio changed) and machine will need to be reinitialized
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x80, 0x80, "Maximum Bet" )  PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x00, "95" )

	PORT_START("PUSHBUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // K0
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // K1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // K2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // K3
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// what's marked as DIP hereunder is actually something else, but left as DIP for easier testing
	PORT_START("P1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "P1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P1:6" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper sensor, gives error 31 if high
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MEMORY_RESET )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "P3:4" ) // needs to be low to avoid error 30 (coin acceptor)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "P3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "P3:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( lanmao )
	PORT_START("KEYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 1" ) PORT_CODE( KEYCODE_1_PAD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 2" ) PORT_CODE( KEYCODE_2_PAD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 3" ) PORT_CODE( KEYCODE_3_PAD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 4" ) PORT_CODE( KEYCODE_4_PAD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 5" ) PORT_CODE( KEYCODE_5_PAD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 6" ) PORT_CODE( KEYCODE_6_PAD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 7" ) PORT_CODE( KEYCODE_7_PAD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 8" ) PORT_CODE( KEYCODE_8_PAD )

	PORT_START("KEYS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Single" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Shift Right" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Shift Left" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME( "Double" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Running Lights Difficulty" )  PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x03, "3" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x02, "2" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x01, "1" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x00, "0" ) // Manual doesn't list the settings
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Double Up Difficulty" )  PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "1" ) // Manual doesn't list the settings
	PORT_DIPSETTING(    0x00, "0" ) // Manual doesn't list the settings
	PORT_DIPNAME( 0x10, 0x10, "Bet Ratio" )  PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, "1/1" )
	PORT_DIPSETTING(    0x00, "1/5" )
	// the following will give error 21 (coin ratio changed) and machine will need to be reinitialized
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x80, 0x80, "Maximum Bet" )  PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x00, "95" )

	PORT_START("PUSHBUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // K0
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // K1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // K2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // K3
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// what's marked as DIP hereunder is actually something else, but left as DIP for easier testing
	PORT_START("P1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P1:1" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) // scl
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "P1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P1:6" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper sensor, gives error 31 if high
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MEMORY_RESET )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "P3:4" ) // needs to be low to avoid error 30 (coin acceptor)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "P3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "P3:8" )
INPUT_PORTS_END


void panda2_state::panda2(machine_config &config)
{
	I8052(config, m_maincpu, 10.738635_MHz_XTAL); // actually W78E065
	m_maincpu->set_addrmap(AS_PROGRAM, &panda2_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &panda2_state::data_map);
	m_maincpu->port_in_cb<0>().set([this] () { LOGPORTS8052("%s CPU port 0 read\n", machine().describe_context()); return 0xff; });
	m_maincpu->port_in_cb<1>().set_ioport("P1");
	m_maincpu->port_in_cb<2>().set([this] () { LOGPORTS8052("%s CPU port 2 read\n", machine().describe_context()); return 0xff; });
	m_maincpu->port_in_cb<3>().set_ioport("P3");
	m_maincpu->port_out_cb<0>().set([this] (uint8_t data) { LOGPORTS8052("%s CPU port 0 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<1>().set(FUNC(panda2_state::port1_w));
	m_maincpu->port_out_cb<2>().set([this] (uint8_t data) { LOGPORTS8052("%s CPU port 2 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS8052("%s CPU port 3 write: %02x\n", machine().describe_context(), data); });

	i8279_device &kdc(I8279(config, "kdc", 10.738635_MHz_XTAL / 6 )); // TODO: divider
	kdc.out_irq_callback().set([this] (int state) { LOGPORTS8279("%s I8279 irq write: %02x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.out_sl_callback().set([this] (uint8_t data) { m_kbd_line = data; }); // 4 bit port
	kdc.out_disp_callback().set(FUNC(panda2_state::display_w)); // to 7-seg LEDs through to 2 CD4511 according to schematics
	kdc.out_bd_callback().set([this] (int state) { LOGPORTS8279("%s I8279 bd write: %01x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.in_rl_callback().set(FUNC(panda2_state::keyboard_r));
	kdc.in_shift_callback().set([this] () { LOGPORTS8279("%s I8279 shift read\n", machine().describe_context()); return 1; }); // not connected according to schematics
	kdc.in_ctrl_callback().set([this] () { LOGPORTS8279("%s I8279 ctrl read\n", machine().describe_context()); return 1; }); // not connected according to schematics

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	AT28C16(config, "at28c16", 0);

	HOPPER(config, m_hopper, attotime::from_msec(100)); // Guessed

	config.set_default_layout(layout_marywu);

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay1(AY8910(config, "ay1", 10.738635_MHz_XTAL / 6)); // TODO: divider
	ay1.port_a_write_callback().set(FUNC(panda2_state::leds_w<0>));
	ay1.port_b_write_callback().set(FUNC(panda2_state::leds_w<1>));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay2(AY8910(config, "ay2", 10.738635_MHz_XTAL / 6)); // TODO: divider
	ay2.port_a_write_callback().set(FUNC(panda2_state::leds_w<2>));
	ay2.port_b_write_callback().set(FUNC(panda2_state::leds_w<3>));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.5);
}

void lanmao_state::lanmao(machine_config &config)
{
	I8052(config, m_maincpu, 10.738635_MHz_XTAL); // actually W78E065
	m_maincpu->set_addrmap(AS_PROGRAM, &lanmao_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &lanmao_state::data_map);
	m_maincpu->port_in_cb<1>().set_ioport("P1");
	m_maincpu->port_in_cb<3>().set_ioport("P3");
	m_maincpu->port_out_cb<1>().set(FUNC(lanmao_state::port1_w));
	m_maincpu->port_out_cb<3>().set(FUNC(lanmao_state::port3_w));

	i8279_device &kdc(I8279(config, "kdc", 10.738635_MHz_XTAL / 6 )); // TODO: divider
	kdc.out_irq_callback().set([this] (int state) { LOGPORTS8279("%s I8279 irq write: %02x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.out_sl_callback().set([this] (uint8_t data) { m_kbd_line = data; }); // 4 bit port
	kdc.out_disp_callback().set(FUNC(lanmao_state::display_w)); // to 7-seg LEDs through to 2 CD4511 according to schematics
	kdc.out_bd_callback().set([this] (int state) { LOGPORTS8279("%s I8279 bd write: %01x\n", machine().describe_context(), state); }); // not connected according to schematics
	kdc.in_rl_callback().set(FUNC(lanmao_state::keyboard_r));
	kdc.in_shift_callback().set([this] () { LOGPORTS8279("%s I8279 shift read\n", machine().describe_context()); return 1; }); // not connected according to schematics
	kdc.in_ctrl_callback().set([this] () { LOGPORTS8279("%s I8279 ctrl read\n", machine().describe_context()); return 1; }); // not connected according to schematics

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I2C_24C02(config, "i2cmem");

	HOPPER(config, m_hopper, attotime::from_msec(100)); // Guessed

	config.set_default_layout(layout_marywu);

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay1(AY8910(config, "ay1", 10.738635_MHz_XTAL / 6)); // TODO: divider
	ay1.port_a_write_callback().set(FUNC(lanmao_state::leds_w<0>));
	ay1.port_b_write_callback().set(FUNC(lanmao_state::leds_w<1>));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay2(AY8910(config, "ay2", 10.738635_MHz_XTAL / 6)); // TODO: divider
	ay2.port_a_write_callback().set(FUNC(lanmao_state::leds_w<2>));
	ay2.port_b_write_callback().set(FUNC(lanmao_state::leds_w<3>));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // verified
}


ROM_START( panda2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "w78e065", 0x0000, 0x8000, CRC(e6a28090) SHA1(0b9b6f205c1ced586a9d15e28d470e055e037800) )

	ROM_REGION( 0x800, "at28c16", 0 )
	ROM_LOAD( "at28c16.u9", 0x000, 0x800, CRC(bd67af27) SHA1(19d19bf00fbe8573e13fb6026db31889023d4194) )
ROM_END

ROM_START( lanmao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w78e065", 0x00000, 0x10000, CRC(57a89c7f) SHA1(be54c40ae17df1155b2aaa67b4b0bf45d307eba1) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "w27e040-12", 0x00000, 0x80000, CRC(4481a891) SHA1(09448bdca052b27828a4797243eee96b98f3d924) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD( "24c02", 0x000, 0x100, CRC(daf84e5e) SHA1(7aa6ba2a7e74e0f59efc8d9f67d06bf41f2e3d6d) )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "nvram", 0x000, 0x800, CRC(5d051021) SHA1(06c1c78f7d2d53b98a2f010c4372d9c7135c1e62) ) // pre-initialized
ROM_END

} // anonymous namespace


GAME( 1996, panda2, 0, panda2, panda2, panda2_state, empty_init, ROT0, "Kelly",                       "Panda 2", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2003, lanmao, 0, lanmao, lanmao, lanmao_state, empty_init, ROT0, "Changsheng Electric Company", "Lan Mao", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
