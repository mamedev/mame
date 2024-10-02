// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Candy Crane by A.G.E.  (Advanced Games & Engineering, Inc.)

(company no longer in business)


Device is a 27c256   location U3


*/

#include "emu.h"
#include "cpu/hpc/hpc.h"
#include "speaker.h"


namespace {

class age_candy_state : public driver_device
{
public:
	age_candy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void age_candy(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void age_candy_map(address_map &map) ATTR_COLD;
	required_device<hpc_device> m_maincpu;
};

static INPUT_PORTS_START( age_candy )
INPUT_PORTS_END



void age_candy_state::machine_start()
{
}

void age_candy_state::machine_reset()
{
}


void age_candy_state::age_candy_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void age_candy_state::age_candy(machine_config &config)
{
	HPC46104(config, m_maincpu, 16_MHz_XTAL); // HPC still not actually emulated
	m_maincpu->set_addrmap(AS_PROGRAM, &age_candy_state::age_candy_map);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


ROM_START( age_cand )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "agecandy.u3", 0x0000, 0x8000, CRC(c8cfc666) SHA1(a1c475ae105746e984741af0723a712f09d7b847) )
ROM_END

} // anonymous namespace


GAME( 19??, age_cand, 0, age_candy, age_candy, age_candy_state, empty_init, ROT0, "Advanced Games & Engineering", "Candy Crane (AGE)", MACHINE_IS_SKELETON_MECHANICAL )
