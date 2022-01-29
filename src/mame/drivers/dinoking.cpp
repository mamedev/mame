// license:BSD-3-Clause
// copyright-holders:

/*

Taito mid-2000s medal hardware

Currently two games are dumped:
Dinoking Kids - ダイノキングキッズ (2004) - K11J0985A - F39 ROM code
Dinoking Battle - ダイノキングバトル (2005) - K11J0998A - F54 ROM code

Main CPU: H8S-2394
Video chip: Axell AG-1 AX51102A (should be similar to the chip in gunpey.cpp)
Sound chip: OKI M9810B
XTALs: 20 MHz near CPU and sound chip, 25 MHz near video chip
*/



#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/h8/h8s2357.h"
#include "sound/okim9810.h"


class dinoking_state : public driver_device
{
public:
	dinoking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void dinoking(machine_config &config);

protected:

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void dinoking_state::mem_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
}



static INPUT_PORTS_START( dinoking )
INPUT_PORTS_END


void dinoking_state::dinoking(machine_config &config)
{
	H8S2394(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dinoking_state::mem_map);
}


ROM_START( dkkids )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f39-03.ic3", 0x000000, 0x200000, CRC(061ea9e6) SHA1(9a6957a24e228aa1f44bf91c635894ecd4871623) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f39-01.ic30", 0x000000, 0x1000000, CRC(1eb9efa6) SHA1(08aa33cb9a5e1e57702d23d7692ad9a289b9a4ed) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f39-02.ic27", 0x000000, 0x400000, CRC(f0a38e0e) SHA1(30ee275d4cce5bb50141e807dc64d6509eaf4937) ) // MBM29F033C-90PTN
ROM_END

ROM_START( dkbattle )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f54-03.ic3", 0x000000, 0x200000, CRC(eeb61085) SHA1(33304c1fd068dc4fd0bacfd4e47e699aa4ae0b82) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f54-01.ic30", 0x000000, 0x1000000, CRC(f2006d02) SHA1(3ba1832eb3440b175014a4d00b56e24a9d8d1994) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f54-02.ic27", 0x000000, 0x400000, CRC(bd8e10e9) SHA1(a86fde5860501b06f63bafbc02ebc6160c682b1e) ) // MBM29F033C-90PTN
ROM_END


GAME( 2004, dkkids,   0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito Corporation", "Dinoking Kids",   MACHINE_IS_SKELETON )
GAME( 2005, dkbattle, 0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito Corporation", "Dinoking Battle", MACHINE_IS_SKELETON )
