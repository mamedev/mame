// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/***************************************************************************

Sesame 2

K11J

Main CPU: Renesas HD6412394TE20 H8S/2394 (ROMless microcontroller @ 20MHz)
   Sound: OKI MSM9810B 8-channel ADPCM audio

     OSC: 
  EEPROM: 
     RAM: Cypress CY7C1021B 64K x 16 Static RAM (44-pin TSOP) x 3 (silkscreened WORK, FRAM0 & FRAM1)

***************************************************************************/

#include "emu.h"

#include "cpu/h8/h8s2357.h"
#include "sound/okim9810.h"

#include "speaker.h"


namespace {

class sesame2_state : public driver_device
{
public:
	sesame2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sesame2(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t port3_r();
	void port3_w(uint8_t data);
	uint8_t port5_r();
	void port5_w(uint8_t data);
	uint8_t port6_r();
	void port6_w(uint8_t data);
	uint8_t porta_r();
	uint8_t portg_r();

	void prg_map(address_map &map) ATTR_COLD;

	// devices
	required_device<h8s2394_device> m_maincpu;
};



uint8_t sesame2_state::port3_r()
{
	return 0;
}

void sesame2_state::port3_w(uint8_t data)
{

}

uint8_t sesame2_state::port5_r()
{
	return 0;
}

void sesame2_state::port5_w(uint8_t data)
{
}

uint8_t sesame2_state::port6_r()
{
	return 0;
}

void sesame2_state::port6_w(uint8_t data)
{
}

uint8_t sesame2_state::porta_r()
{
	return 0xf0;
}

uint8_t sesame2_state::portg_r()
{
	return 0;
}

void sesame2_state::prg_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("program", 0);
	map(0x200000, 0x21ffff).ram();
	map(0x400001, 0x400001).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x400000, 0x400000).w("oki", FUNC(okim9810_device::write));
	map(0x400002, 0x400002).r("oki", FUNC(okim9810_device::read));
}

static INPUT_PORTS_START( sesame2 )
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) // coin 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Left 1 player start")   // start A-1 ("left start" - picks Space Invaders)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   // coin 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Left 2 players start")   // start A-2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("Right 2 players start") // start B-2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Right 1 player start")   // start B-1 ("Right start" - picks Qix)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void sesame2_state::sesame2(machine_config &config)
{
	H8S2394(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sesame2_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(sesame2_state::irq1_line_hold));
	m_maincpu->set_periodic_int(FUNC(sesame2_state::irq0_line_hold), attotime::from_hz(60));
	m_maincpu->read_port1().set_ioport("P1");
	m_maincpu->read_port2().set_ioport("SYSTEM");
	m_maincpu->write_port2().set_nop();
	m_maincpu->read_port3().set(FUNC(sesame2_state::port3_r));
	m_maincpu->write_port3().set(FUNC(sesame2_state::port3_w));
	m_maincpu->read_port4().set_ioport("P4");
	m_maincpu->read_port5().set(FUNC(sesame2_state::port5_r));
	m_maincpu->write_port5().set(FUNC(sesame2_state::port5_w));
	m_maincpu->read_port6().set(FUNC(sesame2_state::port6_r));
	m_maincpu->write_port6().set(FUNC(sesame2_state::port6_w));
	m_maincpu->read_porta().set(FUNC(sesame2_state::porta_r));
	m_maincpu->read_portg().set(FUNC(sesame2_state::portg_r));
	m_maincpu->write_portg().set_nop();

	SPEAKER(config, "speaker", 2).front();

	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "speaker", 0.80, 0);
	oki.add_route(1, "speaker", 0.80, 1);

}

ROM_START( sesame2 )
	ROM_REGION16_BE(0x080000, "program", 0)
	ROM_LOAD16_WORD_SWAP( "f25-05.ic56",   0x000000, 0x080000, CRC(03509e40) SHA1(061f180024102c7a106b1c7f904c01e5dc69801e) )

	ROM_REGION(0x1000000, "oki", 0)
	ROM_LOAD( "f26-04.ic58",  0x000000, 0x200000, CRC(4d013fc5) SHA1(4ae6dfe57db341a2bc52068149966ca2842d20e4) )

ROM_END

} // anonymous namespace


GAME( 200?, sesame2, 0, sesame2, sesame2, sesame2_state, empty_init, ROT270, "Taito", "Sesame 2", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
