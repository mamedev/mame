// license:BSD-3-Clause
// copyright-holders:

// Skeleton driver for medal games on NMK hardware.

#include "emu.h"
#include "cpu/tlcs90/tlcs90.h"
#include "sound/okim6376.h"
#include "speaker.h"


/*
Trocana by NMK / NTC. Possibly distributed by Face?

Video of the game: https://www.youtube.com/watch?v=s63Gokcyn8M
Recording of some of the music: https://www.youtube.com/watch?v=TZMr-MX_M0w

PCBs:

NMK MEC95110 - maincpu board
- Toshiba TMP90C041AN
- 16.5000 MHz XTAL
- maincpu ROM
- Oki M6650
- OKI ROM
- 8 x connectors

NMK MEC95110-SUB2
- 4 x connectors

NMK MEC95110-SUB3
- 3 x connectors
*/

class nmkmedal_state : public driver_device
{
public:
	nmkmedal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
	{ }

	void trocana(machine_config &config);

private:
	void adpcm_control_w(u8 data);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<okim6650_device> m_oki;
};

void nmkmedal_state::adpcm_control_w(u8 data)
{
	m_oki->ch2_w(BIT(data, 0));
	m_oki->cmd_w(BIT(data, 1));
	m_oki->st_w(BIT(data, 2) || !BIT(data, 1));
}

void nmkmedal_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8000).portr("IN0");
	map(0x8001, 0x8001).portr("IN1");
	map(0x8002, 0x8002).portr("IN2");
	map(0x8003, 0x8003).portr("IN3");
	map(0x8004, 0x8004).portr("IN4");
	map(0x8005, 0x8005).portr("IN5");
	map(0xa003, 0xa003).w(m_oki, FUNC(okim6376_device::write));
	map(0xa004, 0xa004).w(FUNC(nmkmedal_state::adpcm_control_w));
	map(0xc000, 0xc7ff).ram();
}

static INPUT_PORTS_START( trocana )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.0") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.1") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.2") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.3") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.4") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.5") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.6") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0.7") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.0") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.1") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.2") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.3") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.4") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.5") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.6") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1.7") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.0") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.1") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.2") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.3") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.4") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.5") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.6") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2.7") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.0") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.1") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.2") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.3") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.4") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.5") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.6") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3.7") PORT_CODE(KEYCODE_M)

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4.0") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4.1") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4.2") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4.3") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5.0") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5.1") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5.2") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5.3") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void nmkmedal_state::trocana(machine_config &config)
{
	TMP90841(config, m_maincpu, 16500000 / 2); // actually TMP90C041AN
	m_maincpu->set_addrmap(AS_PROGRAM, &nmkmedal_state::mem_map);

	SPEAKER(config, "mono").front_center();
	OKIM6650(config, m_oki, 16500000 / 4).add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( trocana)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "tro1e.u12", 0x00000, 0x10000, CRC(f285043f) SHA1(6691091c1ecdab10c390db1d82c9d1d1dd0ded1f) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD( "tro2.u16",  0x00000, 0x80000, CRC(c801d8ca) SHA1(f57026f5386467c054299556dd8665e62557aa91) )
ROM_END


GAME( 1996, trocana, 0, trocana, trocana, nmkmedal_state, empty_init, ROT0, "NTC / NMK", "Trocana", MACHINE_IS_SKELETON_MECHANICAL ) // NMK LTD, NTC LTD, V96313 strings
