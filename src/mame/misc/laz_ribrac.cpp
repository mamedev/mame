// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR
/*

ribbit racing -- prog rom 27c512  @ u7
sound roms -----u7-u10 and u11-u14  = 27c512

IC positions look similar to Awesome Toss 'em
sound rom dumps weren't present even tho mentioned??

this appears to be the operators manual
http://ohwow-arcade.com/Assets/Game_Manuals/RIBBIT%20RACIN.PDF


Awesome tossem u21 = 27c512
               u7  = 27c512  cpu
                u10-u7 and u11-u14 = 27c512 sound board

 probably http://www.highwaygames.com/arcade-machines/awesome-toss-em-7115/

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "speaker.h"


namespace {

class ribrac_state : public driver_device
{
public:
	ribrac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki%u", 1U)
		, m_sound_data(0)
	{ }

	void ribrac(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	void sound_data_w(u8 data);
	void sound_control_w(u8 data);
	void motor_w(u8 data);
	void lights_w(u8 data);
	void bonus_w(u8 data);
	void led_w(u8 data);
	void extra_w(u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device_array<okim6295_device, 2> m_oki;

	u8 m_sound_data;
};



void ribrac_state::machine_start()
{
	save_item(NAME(m_sound_data));
}


void ribrac_state::sound_data_w(u8 data)
{
	m_sound_data = data;
}

void ribrac_state::sound_control_w(u8 data)
{
	if (!BIT(data, 0))
		m_oki[0]->reset();
	else if (!BIT(data, 1) && !BIT(data, 2))
		m_oki[0]->write(m_sound_data);

	if (!BIT(data, 3))
		m_oki[1]->reset();
	else if (!BIT(data, 4) && !BIT(data, 5))
		m_oki[1]->write(m_sound_data);
}

void ribrac_state::motor_w(u8 data)
{
}

void ribrac_state::lights_w(u8 data)
{
}

void ribrac_state::bonus_w(u8 data)
{
}

void ribrac_state::led_w(u8 data)
{
}

void ribrac_state::extra_w(u8 data)
{
}

void ribrac_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void ribrac_state::ext_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0x700).w(FUNC(ribrac_state::sound_data_w));
	map(0x0800, 0x0800).w(FUNC(ribrac_state::sound_control_w));
	map(0x1000, 0x1000).w(FUNC(ribrac_state::motor_w));
	map(0x1800, 0x1800).w(FUNC(ribrac_state::lights_w));
	map(0x2000, 0x2000).w(FUNC(ribrac_state::bonus_w));
	map(0x2800, 0x2800).w(FUNC(ribrac_state::led_w));
	map(0x3000, 0x3000).w(FUNC(ribrac_state::extra_w));
	map(0x4000, 0x4000).portr("INPUTS1");
	map(0x4800, 0x4800).portr("INPUTS2");
	map(0x5000, 0x5000).portr("INPUTS3");
	map(0x5800, 0x5800).portr("INPUTS4");
	map(0x6000, 0x6000).portr("INPUTS5");
	map(0x8000, 0x9fff).ram().share("nvram");
}


static INPUT_PORTS_START( ribrac )
	PORT_START("INPUTS1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS3")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_START("INPUTS4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS5")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:1")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:2")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:3")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:4")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( awetoss )
	PORT_START("INPUTS1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS3")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SET1:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_START("INPUTS4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("INPUTS5")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("SET2:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:1")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:2")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:3")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("SET3:4")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END


void ribrac_state::ribrac(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ribrac_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &ribrac_state::ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 6264 + MAX694 + battery

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki[0], 2.097152_MHz_XTAL, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	OKIM6295(config, m_oki[1], 2.097152_MHz_XTAL, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



ROM_START( ribrac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ribbitr.u7", 0x00000, 0x10000, CRC(9eb78ca3) SHA1(4fede7bdd30449602a01489dc72dbbd5452d6b5a) )

	ROM_REGION( 0xc0000, "oki1", 0 )
	ROM_LOAD( "ribbitr_snd.u10", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u9", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u8", 0x20000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u7", 0x30000, 0x10000, NO_DUMP )

	ROM_REGION( 0xc0000, "oki2", 0 )
	ROM_LOAD( "ribbitr_snd.u14", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u13", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u12", 0x20000, 0x10000, NO_DUMP )
	ROM_LOAD( "ribbitr_snd.u11", 0x30000, 0x10000, NO_DUMP )
ROM_END

ROM_START( awetoss )
	// based on the IC positions differing I don't think this is 2 different sets?
	// both program roms look similar tho
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "awsmtoss.u7", 0x00000, 0x10000, CRC(2c48469c) SHA1(5ccca03d6b9cbcaddd73c8a95425f55d9e6af238) )

	ROM_REGION( 0xc0000, "oki1", 0 )
	ROM_LOAD( "awsmtoss.u10", 0x00000, 0x10000, CRC(84c8a6b9) SHA1(26dc8c0f2098c9b0ef0e06e5dd69c897a9af69a2) )
	ROM_LOAD( "awsmtoss.u9s", 0x10000, 0x10000, CRC(5c7bbbd9) SHA1(89713058d03f982647217e4c6cbe37969f2537a5) )
	ROM_LOAD( "awsmtoss.u8s", 0x20000, 0x10000, CRC(9852e0bd) SHA1(930cca65e3f7774334dd0513a261f874f94886ac) )
	ROM_LOAD( "awsmtoss.u7s", 0x30000, 0x10000, CRC(32fa11f5) SHA1(70914eac64f53bcb07c0eb9fcc1b4fbeab2fc453) )

	ROM_REGION( 0x10000, "maincpu2", 0 )
	ROM_LOAD( "awsmtoss.u21", 0x00000, 0x10000, CRC(2b66d952) SHA1(b95f019d007cbd1f0325c33ffd1208f2afa6b996) )

	ROM_REGION( 0xc0000, "oki2", 0 )
	ROM_LOAD( "awsmtoss.u14", 0x00000, 0x10000, CRC(6217daaf) SHA1(3036e7f941f787374ef130d3ae6d57813d9e9aac) )
	ROM_LOAD( "awsmtoss.u13", 0x10000, 0x10000, CRC(4ed3c827) SHA1(761d2796d4f40deeb2caa61c4a9c56ced156084b) )
	ROM_LOAD( "awsmtoss.u12", 0x20000, 0x10000, CRC(9ddf6dd9) SHA1(c115828ab261ae6d83cb500057313c3a5570b4b0) )
	ROM_LOAD( "awsmtoss.u11", 0x30000, 0x10000, CRC(8ae9d4f0) SHA1(58d1d8972c8e4c9a7c63e9d63e267ea81515d22a) )
ROM_END

} // anonymous namespace


GAME( 1993, ribrac, 0, ribrac, ribrac, ribrac_state, empty_init, ROT0, "Lazer-Tron", "Ribbit Racin (Lazer-Tron)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 19??, awetoss, 0, ribrac, awetoss, ribrac_state, empty_init, ROT0, "Lazer-Tron", "Awesome Toss 'Em (Lazer-Tron)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
