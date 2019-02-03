// license:BSD-3-Clause
// copyright-holders:AJR
/*

Ameri-Hockey?

One of an unknown number of mechanical games developed by Ameri Corporation,
an Illinois-based company in business between 1988 and 1995.

U3 -  27C512
U8 -  27C020
U9 -  27C020
U10- 27C020

12 MHz crystal

Processor is a ROMless MCU from the Z8 family.

*/

#include "emu.h"
#include "cpu/z8/z8.h"
#include "sound/okim6376.h"
#include "speaker.h"


class amerihok_state : public driver_device
{
public:
	amerihok_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
	{ }

	void amerihok(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER(control_w);

	required_device<cpu_device> m_maincpu;
	required_device<okim6376_device> m_oki;
	void amerihok_data_map(address_map &map);
	void amerihok_map(address_map &map);
};

WRITE8_MEMBER(amerihok_state::control_w)
{
	m_oki->st_w(!BIT(data, 4));
	m_oki->ch2_w(!BIT(data, 7));
}

void amerihok_state::amerihok_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void amerihok_state::amerihok_data_map(address_map &map)
{
	map(0x2000, 0x2000).w(FUNC(amerihok_state::control_w));
	map(0x4000, 0x4000).w(m_oki, FUNC(okim6376_device::write));
}

static INPUT_PORTS_START( amerihok )
INPUT_PORTS_END



void amerihok_state::machine_start()
{
}

void amerihok_state::machine_reset()
{
}


void amerihok_state::amerihok(machine_config &config)
{
	Z8681(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &amerihok_state::amerihok_map);
	m_maincpu->set_addrmap(AS_DATA, &amerihok_state::amerihok_data_map);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_oki, 12_MHz_XTAL / 96); // 64-pin QFP, type/clock unverified (probably clocked by Z8681 TOUT)
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( amerihok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "air-h-u3", 0x00000, 0x10000, CRC(f43eaa25) SHA1(b73e3f6db9fe277dab3fd9d1161f3b71b5805048) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "air-h-u8", 0x00000, 0x40000, CRC(17a84f88) SHA1(33a5a66b1e7c8bf79c99e442c62d8ce0c7d1c22c) )
	ROM_LOAD( "air-h-u9", 0x40000, 0x40000, CRC(be01ca4a) SHA1(87513a5c547633d5a3f09e931bd7ec78bcaa94dc) )
	ROM_LOAD( "airh-u10", 0x80000, 0x40000, CRC(71ee6421) SHA1(10131fc7c009158308c4a8bb2b037101622c07a1) )
ROM_END

GAME( 199?, amerihok, 0, amerihok, amerihok, amerihok_state, empty_init, ROT0, "Ameri", "Ameri-Hockey", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
