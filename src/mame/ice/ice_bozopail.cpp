// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Bozo Pail toss by ICE  (ice_tbd notes say Innovative Creations in Entertainment - same company?)

Devices are 27c080

U9 is version 2.07


PCB uses a 68HC11A1P for a processor/security......

could be related to (or the same thing as - our name could be incorrect)
http://www.highwaygames.com/arcade-machines/bozo-s-grand-prize-game-6751/


*/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {

class ice_bozopail_state : public driver_device
{
public:
	ice_bozopail_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_inputs(*this, "INPUTS")
		, m_rombank(*this, "rombank")
		, m_input_shifter(0)
		, m_sndh_latch(0)
	{ }

	void ice_bozo(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void bank_w(u8 data);
	void sndl_w(u8 data);
	void sndh_w(u8 data);
	void clk_w(u8 data);
	void load_w(u8 data);
	u8 pa_r();

	void ice_bozo_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<dac_word_interface> m_dac;
	required_ioport m_inputs;
	required_memory_bank m_rombank;

	u16 m_input_shifter;
	u8 m_sndh_latch;
};

u8 ice_bozopail_state::pa_r()
{
	return 0x81 | (m_input_shifter & 0x0003) << 1;
}

void ice_bozopail_state::bank_w(u8 data)
{
	m_rombank->set_entry(data);
}

void ice_bozopail_state::sndl_w(u8 data)
{
	m_dac->write(m_sndh_latch << 8 | data);
}

void ice_bozopail_state::sndh_w(u8 data)
{
	m_sndh_latch = data;
}

void ice_bozopail_state::clk_w(u8 data)
{
	m_input_shifter = (m_input_shifter >> 2) | 0xc000;
}

void ice_bozopail_state::load_w(u8 data)
{
	m_input_shifter = m_inputs->read();
}

void ice_bozopail_state::ice_bozo_map(address_map &map)
{
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x4000, 0x4000).w(FUNC(ice_bozopail_state::bank_w));
	map(0x8000, 0x8000).nopw();
	map(0x9000, 0x9000).nopw();
	map(0x9800, 0x9800).nopw();
	map(0xa000, 0xa000).w(FUNC(ice_bozopail_state::sndl_w));
	map(0xa800, 0xa800).w(FUNC(ice_bozopail_state::sndh_w));
	map(0xb000, 0xb000).w(FUNC(ice_bozopail_state::clk_w));
	map(0xb800, 0xb800).w(FUNC(ice_bozopail_state::load_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0x3fc000);
}

static INPUT_PORTS_START( ice_bozo )
	PORT_START("INPUTS")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
INPUT_PORTS_END



void ice_bozopail_state::machine_start()
{
	m_rombank->configure_entries(0, 0x100, memregion("maincpu")->base(), 0x4000);
	m_rombank->set_entry(0);

	save_item(NAME(m_input_shifter));
	save_item(NAME(m_sndh_latch));
}

void ice_bozopail_state::machine_reset()
{
}


void ice_bozopail_state::ice_bozo(machine_config &config)
{
	/* basic machine hardware */
	MC68HC11A1(config, m_maincpu, 12'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ice_bozopail_state::ice_bozo_map);
	m_maincpu->in_pa_callback().set(FUNC(ice_bozopail_state::pa_r));
	m_maincpu->in_pe_callback().set_ioport("COINS");

	/* sound hardware */
	DAC_16BIT_R2R(config, m_dac).add_route(0, "mono", 1.0);
	SPEAKER(config, "mono").front_center();
}



ROM_START( ice_bozo )
	ROM_REGION( 0x400000, "maincpu", 0 ) // mostly sound data, some code
	ROM_LOAD( "ice-bozo.u18", 0x000000, 0x100000, CRC(00500a8b) SHA1(50b8a784ae61510a08cafbfb8529ec2a8ac1bf06) )
	ROM_LOAD( "ice-bozo.u9",  0x300000, 0x100000, CRC(26fd9d60) SHA1(41fe8d42db1eb16b413bd5a0f16bf0d081c3cc97) )
ROM_END

} // anonymous namespace


GAME( 1997?, ice_bozo, 0, ice_bozo, ice_bozo, ice_bozopail_state, empty_init, ROT0, "Innovative Creations in Entertainment", "Bozo's Pail Toss (v2.07)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
