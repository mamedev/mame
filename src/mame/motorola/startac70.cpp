// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************

Skeleton driver for Motorola StarTAC 70 mobile phone.

Double-Sided main PCB. Labeled "8409063M05 ISSUE A".
      _______________________________   ___    ___   _______________________________
   __|    ____            _____      |_|   |  |   |_|                        ____   |__
  |      TC682           CHB-04H           |  |  ____                  Motorola 32D91  |
  |                                        |  | |___|                                  |
 [                            IIIIIII      |  |                                        |
  |               ________   |       |     |  |              SIM                       |
  |   IIIIII     |       |   |       |     |  |             SOCKET                     |
 [   |      |  M5M51016ATP   |       |     |  |                       ____             |
  |  |28BV64|    |       |   29LV800BA     |  |                       9424       ____  |
  |  |      |    |       |   |       |     |  |                       ____     Motorola|
 [   |      |    |       |   |       |     |  |                       9424      S08K07 |
  |   IIIIII     |_______|    IIIIIII      |  |   ____         ____   ____             |
  |                                        |  |  |C01|         9424   9424             |
  |        ________      ___________       |  |  |9811           ____                  |
  |       |Motorola     | Lucent   |       |  |  |___|         Motorola                |
  |       |43E13  |     |5199305A01|       |  |                 32D94                  |
  |       SCS138EC10   1627T36PCH12I       |  |                CTAH828                 |
  |        AAYS9825    9810S 1193943       |  |                                        |
  | ____               (c) 95 LUCENT       |  |    ________                            |
  ||9424           ____                    |  |   |Motorola           ___________      |
  |                7W04       ____         |  |   |32D92  |          | Motorola |      |
  |                     DIALOG 9814AC5     |  |   PNHPT9819          |XC390234PU1      |
  |         _______                        |  |   |_______|          |81C06 0J95A      |
  |        |TI    |     ||||||||           |  |                      | HGG9830  |      |
  |        |32D69 |                ______  |  | _____                |__________|      |
  |        |______|               |MC145|  |  | Motorola                               |
  |_                                       |  | 12J20 T9831                           _|
    |___       ___|             |__________|  |__________|             |___       ___|
        |_____|   |_____________|                        |_____________|   |_____|

 Screen and keyboard on a separate EPSON-labeled PCB.
****************************************************************************************/

#include "emu.h"

#include "cpu/m68000/tmp68301.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class startac70_state : public driver_device
{
public:
	startac70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mic(*this, "mic"),
			m_speaker(*this, "speaker")
	{ }

	void startac70(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<microphone_device> m_mic;
	required_device<speaker_device> m_speaker;

	void program_map(address_map &map) ATTR_COLD;
};


void startac70_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
}


static INPUT_PORTS_START(startac70)
INPUT_PORTS_END


void startac70_state::startac70(machine_config &config)
{
	// Actually a Motorola XC390234PU1, likely a Motorola 68331
	TMP68301(config, m_maincpu, 16'000'000); // Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &startac70_state::program_map);

	MICROPHONE(config, m_mic, 1).front_center();
	SPEAKER(config, m_speaker).front_center();
}


// GSM, Movistar locked
ROM_START( startac70 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "startac70_29lv800ba_0004225b.bin", 0x000000, 0x100000, CRC(34cf65e3) SHA1(97c01ddaa60789c4527585a2653f46b4b3a430a1) )

	ROM_REGION( 0x2000, "user", 0 )
	ROM_LOAD( "startac70_28bv64.bin", 0x0000, 0x2000, BAD_DUMP CRC(2bf6a687) SHA1(b74bdb9acdce0a87de09af90ee4aefeeee2cf0a7) ) // BAD_DUMP because it contains user and operator data, needs a factory reset
ROM_END


} // anonymous namespace

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME      FLAGS
SYST( 1990, startac70, 0,      0,      startac70, startac70, startac70_state, empty_init, "Motorola", "StarTAC 70", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // year taken from copyright in ROM
