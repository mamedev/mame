// license:BSD-3-Clause
// copyright-holders:
/*
    SGMB-PCB_VERB.1

    Forerunner1s SG700 SOC - unemulated

    Used at least for the Fantasy Shot medal games series.

    Dumped games:
    Snowball War - WakuWaku Yukigassen = わくわく雪合戦

    https://www.youtube.com/watch?v=nX3bDom1cEE
*/

#include "emu.h"
#include "screen.h"

class sealy_fr_state : public driver_device
{
public:
	sealy_fr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void sealy_fr(machine_config &config);

protected:

private:
};


static INPUT_PORTS_START( sealy_fr )
INPUT_PORTS_END


void sealy_fr_state::sealy_fr(machine_config &config)
{
	// unemulated Frontrunner1s SOC
}


ROM_START( snowbwar )
	DISK_REGION( "sdcard" ) // 4GB Micro SD Card (Kingston SDC4/4GB 111)
	DISK_IMAGE( "snowbwar", 0, SHA1(6d2f63871b73283555de8d24029234c736ff0ba3) )
ROM_END

GAME( 20??, snowbwar, 0, sealy_fr, sealy_fr, sealy_fr_state, empty_init, ROT0, "Sealy", "Snowball War - WakuWaku Yukigassen",  MACHINE_IS_SKELETON )
