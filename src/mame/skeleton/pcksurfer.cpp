// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

Datawind Pocket Surfer.

Pre-smartphone era pocket Internet terminal.

https://web.archive.org/web/20060221032558/http://www.datawind.com/
https://web.archive.org/web/20060221032551/http://www.datawind.com/specs.html
https://uk.pcmag.com/first-looks/7579/datawind-pocketsurfer

CPU Sharp LH79524-NOF (package BGA-208).
SIM300 v7.03 GSM/GPRS modem.

Tear down of a production unit: https://www.youtube.com/watch?v=lQi8veu3ugU

PCB:
                                 _____
  _______________________________|   |__  _______________
 _|               _____________  |USB|  |_|              \_____________________
_|    ________    |            |                                _______        |
|     |DW-    |   |   SIM      |                                |STB5610       |
|     |SBTM1  |   |  SOCKET    |                                |      |       |
|     |Bluetooth  |   AMD      |                                |______|       |
|     |_______|   |  COMMS     | ___         _____________            _______  |
|   ___________   |            ||___|  ___   | IS42S16400B|           |      | |
|   |U4 Flash  |  |SIM300 v7.03|       |  |  | DRAM       | ________  |IS42S16400B
|   |          |  |  GSM/GPRS  | LVX457->_|  |____________| |SHARP  | |DRAM  | |
|   |__________|  |____________|       ___                  |LH79524| |      | |
|                           ___        |  |                 |NOF    | |      | |
|                    LCX125->__|       |_<-LCX125           |_______| |______| |
|______________________________________________________________________________|

Chip labeled "31314 3A05U 511AD" inside the DW-SBTM1 module.


    TODO:
    - Everything
*/

#include "emu.h"

#include "cpu/arm7/arm7.h"


namespace {

class pcksurfer_state : public driver_device
{
public:
	pcksurfer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void pcksurfer(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START( pcksurfer )
INPUT_PORTS_END


void pcksurfer_state::pcksurfer(machine_config &config)
{
	// Basic machine hardware
	ARM7(config, m_maincpu, 76'205'000); // Sharp LH79524-NOF (BGA-208)
}

// ROM definitions

ROM_START( pcksurfer )
	ROM_REGION32_LE( 0x800100, "maincpu", 0 )
	ROM_LOAD( "mxl29lv640mbt.u4", 0x000000, 0x800100, CRC(39896d0b) SHA1(98904409483b22c77adb9495147c2e433a607e61) )

	ROM_REGION32_LE( 0x2000, "bootrom", 0 )
	ROM_LOAD( "lh79524.bootrom.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "attiny28l.bin", 0x000, 0x800, NO_DUMP ) // 2K bytes of Flash
ROM_END

} // anonymous namespace

//    YEAR  NAME       PARENT COMPAT MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME                     FLAGS
COMP( 2006, pcksurfer, 0,     0,     pcksurfer, pcksurfer, pcksurfer_state, empty_init, "Datawind", "Pocket Surfer (prototype)", MACHINE_IS_SKELETON )
