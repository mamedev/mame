// license:BSD-3-Clause
// copyright-holders:

/*
Pitching Try (2006, Atlus)

GIANTEK VU-001C PCB (K49H273A)

Main components:

Z86C9116 CPU (lazily scratched off, so it could be read)
SKC22.118D5 XTAL
W24257AK-15 RAM
M6242B RTC
32.768 kHz XTAL
aP89085 voice / ADPCM chip with undumped embedded 2M EPROM
aP89170 voice / ADPCM chip with undumped embedded 4M EPROM
4x bank of 8 switches
bank of 4 switches
reset push-button
*/

#include "emu.h"

#include "cpu/z8/z8.h"
#include "machine/msm6242.h"

#include "speaker.h"


namespace {

class pitchtry_state : public driver_device
{
public:
	pitchtry_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void pitchtry(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void pitchtry_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void pitchtry_state::data_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x7fff).ram();
	map(0xa000, 0xa00f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)); // ??
}


static INPUT_PORTS_START( pitchtry )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")

	PORT_START("DSW5")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW5:4")
	PORT_BIT(              0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void pitchtry_state::pitchtry(machine_config &config)
{
	// basic machine hardware
	Z86C91(config, m_maincpu, 22'118'000 / 2); // Chip rated for 16MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &pitchtry_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &pitchtry_state::data_map);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	// no video

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	// unemulated sound chips
}


ROM_START( pitchtry )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a61008_2e26_v1_3cj_10_12_06.u2", 0x0000, 0x8000, CRC(5df2a7eb) SHA1(a73127a85f9cabe5865ccc3b952ae41b718d9f18) )
ROM_END

} // anonymous namespace


GAME( 2006, pitchtry, 0, pitchtry, pitchtry, pitchtry_state, empty_init, ROT0, "Atlus", "Pitching Try (v1.3CJ)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
