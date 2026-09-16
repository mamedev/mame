// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "mpu4.h"

#include "sound/ymz280b.h"

namespace {

class mpu4redpoint_state : public mpu4_state
{
public:
	mpu4redpoint_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag)
		, m_ympcm(*this, "ympcm")
	{
	}

	void base_f(machine_config &config);

	template <typename... T>
	auto base(T... traits)
	{
		return trait_wrapper(this, &mpu4redpoint_state::base_f, traits...);
	}

private:
	void add_ympcm(machine_config &config);

	void memmap_ympcm(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(mpu4redpoint);

	required_device<ymz280b_device> m_ympcm;
};

void mpu4redpoint_state::add_ympcm(machine_config &config)
{
	YMZ280B(config, m_ympcm, 16'934'400);
	m_ympcm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mpu4redpoint_state::memmap_ympcm(address_map &map)
{
	mpu4_memmap(map);
	map(0x1100, 0x1101).rw(m_ympcm, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
}

void mpu4redpoint_state::base_f(machine_config &config)
{
	mpu4base(config);
	MCFG_MACHINE_START_OVERRIDE(mpu4redpoint_state,mpu4redpoint)

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4redpoint_state::memmap_ympcm);

	add_ympcm(config);
}

MACHINE_START_MEMBER(mpu4redpoint_state,mpu4redpoint)
{
	mpu4_config_common();

	m_link7a_connected=false;
	m_link7b_connected=true;
}

} // anonymous namespace


#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)

using namespace mpu4_traits;

static INPUT_PORTS_START( m4cbing )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x0f, 0x0f, "Percentage Key" ) PORT_DIPLOCATION("DIL1:01,02,03,04")
	PORT_DIPSETTING(    0x00, "Invalid" )
	PORT_DIPSETTING(    0x01, "76" )
	PORT_DIPSETTING(    0x02, "77" )
	PORT_DIPSETTING(    0x03, "78" )
	PORT_DIPSETTING(    0x04, "79" )
	PORT_DIPSETTING(    0x05, "80" )
	PORT_DIPSETTING(    0x06, "81" )
	PORT_DIPSETTING(    0x07, "82" )
	PORT_DIPSETTING(    0x08, "83" )
	PORT_DIPSETTING(    0x09, "84" )
	PORT_DIPSETTING(    0x0a, "85" )
	PORT_DIPSETTING(    0x0b, "86" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "88" )
	PORT_DIPSETTING(    0x0e, "89" )
	PORT_DIPSETTING(    0x0f, "90" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIL2:01,02")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END


ROM_START( m4cbing )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cherrybingoprg.bin", 0x0000, 0x010000, CRC(00c1d4f3) SHA1(626df7f2f597ed13c32ce0fa8846f2e27ca68eae) )

	ROM_REGION( 0x200000, "ympcm", 0 ) // not oki!
	ROM_LOAD( "cherrybingosnd.p1", 0x000000, 0x100000, CRC(11bed9f9) SHA1(63ed45122dda8e412bb1eaeb967d8a0f925d4bde) )
	ROM_LOAD( "cherrybingosnd.p2", 0x100000, 0x100000, CRC(b2a7ec28) SHA1(307f19ffb46f4a2e8e93923ddb666e50de43a00e) )
ROM_END


GAME( 1998, m4cbing,   0,          base(R4, RT1), m4cbing, mpu4redpoint_state, init_m4, ROT0,   "Redpoint Systems", "Cherry Bingo (Redpoint Systems) (MPU4)", GAME_FLAGS )
