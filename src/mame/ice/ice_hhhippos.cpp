// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hungry Hungry Hippos redemption game by I.C.E. Inc.

  _________________________________________________________________________
  | ___   ___        _____   ___  ___     _                                |
  | |__|  FUSE       |ooo|   |o|  |··|   (_)                               |
  |                  |ooo|   |o| COUNTER VOLUME   ____ ____                |
  |                  |ooo|  DOME         CONTROL LM358 LM358               |
  |         ___      |ooo|  LIGHT                                          |
  |  _  ___ |__|    O<-LED   ___         ______________   ______________   |
  | (_) |__|        ___      FUSE        | U119        |  | U122        |  |
  |SPEED       ____FUSE                  |_____________|  |_____________|  |
  | ___        |oo|                      _______ _______  _______ _______  |
  |FUSE        |oo|                    74HC4040 74HC4060 74HC4040 74HC4060 |
  |   O<-LED   |oo|                        ____   SW   _________           |
  |      __    ____              LED->O    XTAL  START |__CONN__|          |
  |     | |    |oo|           LM340AT    2.000 ________ ________           |
  |     | |    |oo|                            74HC08N  74HC138AN    BATT  |
  |     |_|    |oo|                        ________________          3.2V  |
  |    LM723CN ____           TIP115       | MC68HC705C8P  |               |
  |   O<-LED   |oo|           _____        |_______________|    _____      |
  |      __    |oo|           |ooo|        ________ ________    |ooo|      |
  |     | |    |oo|           |ooo|        74HC174P 74HC174P    |ooo|      |
  |     | |    ____           |ooo|        ________ ________    |ooo|      |
  |     |_|    |oo|           |ooo|        74HC174P 74HC174P    |ooo| TIP115
  |    LM723CN |oo|  TIP115   _____        ________ ________    _____      |
  |            |oo|           |ooo|        74HC174P 74HC174P    |ooo|      |
  |   ___    ___              |ooo|        ________ ________    |ooo|      |
  |   |__|   |__|             |ooo|        74HC174P 74HC174P    |ooo|      |
  |                           |ooo|        ________ ________    |ooo| TIP115
  |   ___    ___               SW SW SW SW 74HC153N 74HC153N               |
  |   |__|   |__|           PROG SEL STEP TST                              |
  |________________________________________________________________________|

For each hippo there's an eaten ball counter (two digits 7-seg display with 8 leds)
        _______________________________
        |  ___   _______  _______   ___ |
        | |  |  | ___  | | ___  |  |  | |
MC14499P->|  |  | |__| | | |__| |  |  |<- TP03904
        | |  |  |o|__|o| |o|__|o|  |  | |
        | |__|  |______| |______|  |__| |
        |     O  O  O  O  O  O  O  O <- 8 leds
        |_______________________________|

****************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"

namespace
{

class ice_hhhippos_state : public driver_device
{
public:
	ice_hhhippos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hhhippos(machine_config &config);

private:
	required_device<m68hc05_device> m_maincpu;
};

static INPUT_PORTS_START(hhhippos)
INPUT_PORTS_END

void ice_hhhippos_state::hhhippos(machine_config &config)
{
	M68HC705C8A(config, m_maincpu, 2_MHz_XTAL);

	// TODO: sound (R2R DACs streamed from ROMs using HCMOS ripple counters)
}

ROM_START(hhhippos)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("68hc705c8.bin", 0x0000, 0x2000, CRC(5c74bcd7) SHA1(3c30ae38647c8f69f7bbcdbeb35b748c8f4c4cd8))

	ROM_REGION(0x10000, "audio0", 0)
	ROM_LOAD("u119.bin", 0x00000, 0x10000, CRC(77c8bd90) SHA1(e9a044d83f39fb617961f8985bc4bed06a03e07b))

	ROM_REGION(0x20000, "audio1", 0)
	ROM_LOAD("u122.bin", 0x00000, 0x20000, CRC(fc188905) SHA1(7bab8feb1f304c9fe7cde31aff4b40e2db56d525))
ROM_END

} // anonymous namespace

GAME(1991, hhhippos, 0, hhhippos, hhhippos, ice_hhhippos_state, empty_init, ROT0, "ICE (Innovative Concepts in Entertainment)", "Hungry Hungry Hippos (redemption game)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
