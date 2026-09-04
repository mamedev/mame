// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/***************************************************************************

Capriccio Sesame 2 crane game board

セサミ2 ゴ カソボード
K11J0969A

Main CPU: Renesas HD6412394TE20 H8S/2394 (ROMless microcontroller @ 20MHz)
   Sound: OKI MSM9810B 8-channel ADPCM audio

     OSC: 20MHz
  EEPROM: Atmel 93C46
     RAM: None on board

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
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki")
	{ }

	void sesame2(machine_config &config) ATTR_COLD;

private:
	void prg_map(address_map &map) ATTR_COLD;

	uint8_t portf_r();
	void portf_w(uint8_t data);

	// devices
	required_device<h8s2394_device> m_maincpu;
	optional_device<okim9810_device> m_oki;
};

uint8_t sesame2_state::portf_r()
{
	return 0;
}

void sesame2_state::portf_w(uint8_t data)
{
}

void sesame2_state::prg_map(address_map &map)
{
	map(0x000000, 0x7ffff).rom().region("program", 0);
	map(0x200000, 0x21ffff).ram();

	//map(0x400000, 0x400000).w(m_oki, FUNC(okim9810_device::write));
	//map(0x400001, 0x400001).w(m_oki, FUNC(okim9810_device::tmp_register_w));
	//map(0x400002, 0x400002).r(m_oki, FUNC(okim9810_device::read));
}

static INPUT_PORTS_START( sesame2 )
	PORT_START("DSW2") //74HC165A
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("DSW3") //74HC165A
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW3:8")

	PORT_START("DSW4") //unpopulated
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW4:8")

	PORT_START("DSW5") //unpopulated
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW5:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW5:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW5:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW5:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW5:8")

INPUT_PORTS_END

void sesame2_state::sesame2(machine_config &config)
{
	H8S2394(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sesame2_state::prg_map);

	m_maincpu->read_portf().set(FUNC(sesame2_state::portf_r));
	m_maincpu->write_portf().set(FUNC(sesame2_state::portf_w));

	//connector N
	//port6
	//port5
	//porta
	//port1
	//porth
	//port2

	SPEAKER(config, "speaker", 2).front();

	OKIM9810(config, m_oki, XTAL(4'096'000));
	m_oki->add_route(0, "speaker", 0.80, 0);
	m_oki->add_route(1, "speaker", 0.80, 1);
}

ROM_START( csesame2 )
	ROM_REGION16_BE(0x080000, "program", 0)
	ROM_LOAD( "f25-05.ic56",   0x000000, 0x080000, CRC(2205de06) SHA1(a67c2c2d626d805ff7e2d26a6c7d3e217ec0b3de) )

	ROM_REGION(0x200000, "oki", 0)
	ROM_LOAD( "f26-04.ic58",  0x000000, 0x200000, CRC(4d013fc5) SHA1(4ae6dfe57db341a2bc52068149966ca2842d20e4) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "at93c46.ic67", 0x00, 0x80, CRC(652d544c) SHA1(cd5bd20e9a0f22d7367cc169e2844a02751c7c91) ) // empty
ROM_END

} // anonymous namespace


GAME( 2004, csesame2, 0, sesame2, sesame2, sesame2_state, empty_init, ROT270, "Taito", "Capriccio Sesame 2", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_NO_SOUND ) // between F19 (hkuranai) and F34 (invqix)
