// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for 4-player dart board by Merit-Nomac Industries, Inc.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {

class pubtimed_state : public driver_device
{
public:
	pubtimed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void pubtimed(machine_config &mconfig);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void pubtimed_state::mem_map(address_map &map)
{
	map(0x0800, 0x080f).ram(); // LCD segment driver?
	map(0x0820, 0x083f).ram(); // LCD segment driver?
	map(0x4000, 0x4000).portr("UNKNOWN");
	map(0x6000, 0x6000).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe000, 0xffff).rom().region("roms", 0);
}


static INPUT_PORTS_START(pubtimed)
	PORT_START("UNKNOWN")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0xfd, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	// TODO: DIP switches
INPUT_PORTS_END

void pubtimed_state::pubtimed(machine_config &config)
{
	M6802(config, m_maincpu, 4'000'000); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &pubtimed_state::mem_map);

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


ROM_START(pubtimed)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "9278-13_u12-2_c1990_mii.u12", 0x0000, 0x1000, CRC(76af40ae) SHA1(57d85067a8d0bf09b7c14851f0eb08f68c5bd318) )
	ROM_LOAD( "9278-13_u13-2_c1990_mii.u13", 0x1000, 0x1000, CRC(2092583a) SHA1(825e6e7c6fe7b4e74cb64fab87b27d09db798b8b) )

	ROM_REGION(0x200, "plds", 0)
	ROM_LOAD( "sc3922_pal20l10.u11", 0x000, 0x117, NO_DUMP )
ROM_END

/*
  Pub Time Darts.
  V1.4, Merit, 1990.

  No PLDs. The U11 position has a SN74LS42N

  PCB etched:
  PUB TIME GAMES INC.
  COPYRIGHT 1984
  MERIT/NOMAC

  D-LOGIC
  MB5654 REV C

  1x MC6802P (U15)  CPU.
  1x EF6810P (U25)  128x8 SRAM.
  1x AD558   (U9)   8bit DAC.

  4x 3x7seg LEDs for 4 players.
  1x 3x7seg LEDs for temp1, temp2, temp3.
  1x 7seg LED for game select.
  1x 7seg LED for credit round.

  1x Xtal 4.000 MHz.

*/
ROM_START(pubtimeda)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "u12_tms2732a.u12",      0x0000, 0x1000, CRC(0073cec9) SHA1(300c1e0a8e6e6aae49f866bc8d7b066c8e93fa26) )
	ROM_LOAD( "u13_v1.4_tms2732a.u13", 0x1000, 0x1000, CRC(fcbcc8b3) SHA1(af21fd41950cb04215c8621b66a8724bd4d7525b) )
ROM_END


} // anonymous namespace


GAME(1987, pubtimed,  0, pubtimed, pubtimed, pubtimed_state, empty_init, ROT0, "Merit-Nomac Industries", "Pub Time Darts", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
GAME(1990, pubtimeda, 0, pubtimed, pubtimed, pubtimed_state, empty_init, ROT0, "Merit-Nomac Industries", "Pub Time Darts v1.4", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
