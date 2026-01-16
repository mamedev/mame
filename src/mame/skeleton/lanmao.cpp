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
*/


#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "machine/i2cmem.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "speaker.h"


// configurable logging
#define LOG_PORTS     (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class lanmao_state : public driver_device
{
public:
	lanmao_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void lanmao(machine_config &config) ATTR_COLD;

private:
	required_device<i8052_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void lanmao_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

// TODO: verify everything. Where's the YM2413?
void lanmao_state::data_map(address_map &map)
{
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9003).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xd000, 0xd000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( lanmao )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void lanmao_state::lanmao(machine_config &config)
{
	I8052(config, m_maincpu, 11_MHz_XTAL); // actually W78E065, TODO: clock
	m_maincpu->set_addrmap(AS_PROGRAM, &lanmao_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &lanmao_state::data_map);
	m_maincpu->port_in_cb<1>().set([this] () { LOGPORTS("%s I8052 port 1 read\n", machine().describe_context()); return 0xff; });
	m_maincpu->port_out_cb<1>().set([this] (uint8_t data) { LOGPORTS("%s I8052 port 1 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->port_in_cb<3>().set([this] () { LOGPORTS("%s I8052 port 3 read\n", machine().describe_context()); return 0xff; });
	m_maincpu->port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS("%s I8052 port 3 write: %02x\n", machine().describe_context(), data); });

	i8279_device &kdc(I8279(config, "kdc", 11_MHz_XTAL / 6 )); // TODO: divider
	kdc.out_irq_callback().set_inputline(m_maincpu, 1); // maybe
	kdc.out_sl_callback().set([this] (uint8_t data) { LOGPORTS("%s I8279 sl write: %02x\n", machine().describe_context(), data); });
	kdc.out_disp_callback().set([this] (uint8_t data) { LOGPORTS("%s I8279 disp write: %02x\n", machine().describe_context(), data); });
	kdc.out_bd_callback().set([this] (int state) { LOGPORTS("%s I8279 bd write: %01x\n", machine().describe_context(), state); });
	kdc.in_rl_callback().set([this] () { LOGPORTS("%s I8279 rl read\n", machine().describe_context()); return 0xff; });
	kdc.in_shift_callback().set([this] () { LOGPORTS("%s I8279 shift read\n", machine().describe_context()); return 0; });
	kdc.in_ctrl_callback().set([this] () { LOGPORTS("%s I8279 ctrl read\n", machine().describe_context()); return 0; });

	I2C_24C02(config, "i2cmem");

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay1(AY8910(config, "ay1", 11_MHz_XTAL / 6)); // TODO: divider
	ay1.port_a_write_callback().set([this] (uint8_t data) { LOGPORTS("%s AY1 port A write: %02x\n", machine().describe_context(), data); });
	ay1.port_b_write_callback().set([this] (uint8_t data) { LOGPORTS("%s AY1 port B write: %02x\n", machine().describe_context(), data); });
	ay1.add_route(ALL_OUTPUTS, "mono", 0.5);

	ay8910_device &ay2(AY8910(config, "ay2", 11_MHz_XTAL / 6)); // TODO: divider
	ay2.port_a_write_callback().set([this] (uint8_t data) { LOGPORTS("%s AY2 port A write: %02x\n", machine().describe_context(), data); });
	ay2.port_b_write_callback().set([this] (uint8_t data) { LOGPORTS("%s AY2 port B write: %02x\n", machine().describe_context(), data); });
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
