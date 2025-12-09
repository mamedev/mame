// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    Skeleton driver for "Main Event" (electromechanical boxing machine from Destron).
 _______________________________________________________________________________
|      ________________                                                        |
|     |_DSWx8_|_DSWx8_|     ___________      Xtal     ___________________      |
|                          |M74LS245P_|   4.0000 MHz | HD46802P         |      |
|                                                    |__________________|      |
|          ___              ___________                                        |
|         |  |             |SN74LS244N|                _____________           |
|         |  |<-16-1-102                              | EPROM      |           |
|         |  |              ___________               |____________|           |
|         |__|             |SN74LS244N|                                        |
|                                                  ___________  ___________    |
|                           ___________           |_54LS139J__||_SN74LS04N_|   |
|          ___             |MM74C244N_|                                        |
|         |  |                                     ___________  ___________    |
|         |  |<-16-1-471    ___________           |SN74LS138N| |__74LS00__|    |
|         |  |             |DM74LS374N|                                        |
|         |__|                                     ___________  ___________    |
|                           ___________           |__TC4040BP_| |M74LS74AP|    |
|                          |DM74LS374N|                                        |
|                                                         ____  ___________    |
|                                                        NE555P |DM74LS197N    |
|                                                                              |
|                       ____                       ___________  ___________    |
|                      MOC3010                    |DM74LS374N| |DM74LS374N|    |
|                                                                              |
|                       ____                         ______________________    |
|                      MOC3010                      | AY-3-8910           |    |
|                                                   |_____________________|    |
·                                                                              ·
·                                  DESTRON, INC.                               ·
·                                  (c) Copyright 1983                          ·
|______________________________________________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"

namespace
{

class dmainevent_state : public driver_device
{
public:
	dmainevent_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void dmainevent(machine_config &config);

protected:
	required_device<m6802_cpu_device> m_maincpu;
};

INPUT_PORTS_START(dmainevent)
INPUT_PORTS_END

void dmainevent_state::dmainevent(machine_config &config)
{
	M6802(config, m_maincpu, 4_MHz_XTAL); // Actually a HD46802P
}

ROM_START(dmainevent)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("destron_main_evn_ver_2.1_6-27-83_rev_0-1_2732.u9", 0x0000, 0x1000, CRC(51b1762d) SHA1(5387af9b7d7c326b61a6dd3b2515a8e571b0d88c))
ROM_END

} // anonymous namespace

//   YEAR  NAME        PARENT MACHINE     INPUT       CLASS             INIT        ROT   COMPANY    FULLNAME                 FLAGS
GAME(1983, dmainevent, 0,     dmainevent, dmainevent, dmainevent_state, empty_init, ROT0, "Destron", "The Main Event (Destron)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
