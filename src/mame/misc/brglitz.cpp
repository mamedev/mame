// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************

 Skeleton driver for "Ballroom Glitz", by Harry Levy (2014)

  Main PCB (FL0898)

   ___CN1______________________________CN17____CN18___________________
  |  _____                      CN15   ____  __________              |
  | |o o |                       ··   |o o| |o o o o o          _____|
  | |o o |   _  _                                              |o o ||
  | |o_o_|  (_)(_) <-LEDS                                      |o o |CN14
  |        24V 12V         L7805CV                             |o_o_||
  | _____                                                            |
  ||o o |                                                            |
  ||o o |CN6                                                         |
  ||o o |                                                         .. |
  ||o_o_|                                                         .. CN13
  ||o o |             FL0898                           ___        .. |
  ||o o |CN5           REV2                           |  |        .. |
  ||o o |                                   ULN2803A->|  |        .. |
  ||o_o_|                                             |  |           |
  ||o o |                             __________      |__|    _____  |
  ||o o |CN9                         |ULN2803A_|             |o o |  |
  ||o o |                                                    |o o | CN16
  ||o_o_|_                                                   |o_o_|  |
  | |o o |                                                           |
  | |o_o_|CN4                                                        |
  ||o o |                   __________                               |
  ||o o |CN3               |M74HC373B1      __________               |
  ||o o |                                  |M74HC373B1               |
  ||o_o_|                   __________                               |
  |                        |M74HC373B1      __________               |
  |                                        |M74HC373B1               |
  |                         __________                               |
  | :                      |M74HC373B1      __________          _____|
  | : CN11                                 |M74HC373B1         |o o ||
  | :        __________     __________                         |o o |CN10
  | :       |M74HC244B1    |M74HC373B1      __________         |o_o_||
  | :                                      |M74HC373B1          ___  |
  |  _____   __________                        XT1    ULN2803A->|  | |
 CN8|o o |  |M74HC244B1         _______________160__            |  | |
  | |o o |                     | PIC17C43-8        |     ___    |__| |
  | |o_o_|   __________        |                   |     |  |   _____|
  |  _____  |M74HC244B1        |_______________SP232ACP->|  |  |o o |CN12
 CN2|o o |                             _____    _____    |__|  |o_o_||
  | |o_o_|   __________               M24C04B  TL7705ACP             |
  |  _____  |M74HC244B1                                              |
 CN7|o_o_|                                                           |
  |__________________________________________________________________|

  Sound PCB (FL0907)
   ______________________________________________
  |  ···················               ___      |
  |                                TL7705ACP    |
  |  __________                        ___ ___  |
  | |M74HC244B1            OKI M6585->|  ||  |  |
  |              __________           |  ||  |<-M74HC244B1
  |  __________ |  DIPSx8 |           |  ||  |  |
  | |M74HC244B1 |_________|        XT4|__||__|  |
  |               ____                  ______  |
  |             TL7705ACP     ___      |     |  |
  |         __________       (VOL)     |EPROM|  |
  |        |HCF4042BE|  XT3            |     |  |
  |                  __________        |     |  |
  | XT1              |OKI M6585|  ____ |     |  |
  |  ______________  ······     TL072CN      |  |
  | |PIC16C55A    |  __________        |_____|  |
  | |_____________|  |M74HC138B1                |
  | ___  ______  ______  ______   ___   ______  |
  ||  | |     | |     | |     |  |  |  |     |  |
  ||  | |EPROM| |EMPTY| |EMPTY|  |  |<-M74HC373B1
  ||  | |     | |     | |     |  |  |  |     |  |
  ||__|<-M74HC373B1   | |     |  |__|  |     |<-PIC16C55A
  |     |     | |     | |     |        |     |  |
  |     |_____| |_____| |_____|     XT2|_____|  |
  | ___  ______  ______  ______  ______  ______ |
  ||  | |     | |     | |     | |     | |     | |
  ||  | |EMPTY| |EMPTY| |EMPTY| |EMPTY| |EMPTY| |
  ||  | |     | |     | |     | |     | |     | |
  ||__|<-M74HC38B1    | |     | |     | |     | |
  |     |     | |     | |     | |     | |     | |
  |     |_____| |_____| |_____| |_____| |_____| |
  |_____________________________________________|

*****************************************************************************************/

#include "emu.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/pic17/pic17c4x.h"
#include "machine/i2cmem.h"
#include "sound/msm5205.h"

#include "speaker.h"

namespace
{

class brglitz_state : public driver_device
{
public:
	brglitz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_soundcpu_2(*this, "soundcpu_2")
		, m_eeprom(*this, "eeprom")
	{
	}

	void brglitz(machine_config &config);

protected:
	required_device<pic17c4x_device> m_maincpu;
	required_device<pic16c55_device> m_soundcpu;
	required_device<pic16c55_device> m_soundcpu_2;
	required_device<i2cmem_device> m_eeprom;
};

INPUT_PORTS_START(brglitz)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

void brglitz_state::brglitz(machine_config &config)
{
	// Main CPU
	PIC17C43(config, m_maincpu, 16'000'000/2); // PIC17C43-8, 16 MHz resonator

	I2C_24C04(config, m_eeprom);

	// Sound
	PIC16C55(config, m_soundcpu,   8_MHz_XTAL); // Divider unknown, ZTA 8.00MT resonator
	PIC16C55(config, m_soundcpu_2, 8_MHz_XTAL); // Divider unknown, ZTA 8.00MT resonator

	SPEAKER(config, "speaker", 2).front();

	msm6585_device &oki1(MSM6585(config, "oki1", 640_kHz_XTAL)); // CSB 640 P resonator
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.45, 0); // Guess
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.45, 1); // Guess

	msm6585_device &oki2(MSM6585(config, "oki2", 640_kHz_XTAL)); // CSB 640 P resonator
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.45, 0); // Guess
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.45, 1); // Guess
}

ROM_START(brglitz)
	ROM_REGION(0x02000, "maincpu", 0)
	ROM_LOAD("bg_04_v1.3.u1",    0x00000, 0x02000, BAD_DUMP CRC(f414b736) SHA1(0280adb1de085f2774ad58872cb171a65cf85fbe)) // PIC17C43-8, protected

	ROM_REGION(0x00400, "soundcpu", 0)
	/*
	  User ID
	    ID0=0x0000
	    ID1=0x0006
	    ID2=0x0008
	    ID3=0x0007
	  Config Word: 0x0005
	*/
	ROM_LOAD("qsound47_v1.0.u1", 0x00000, 0x00400, CRC(bd3156fb) SHA1(416674ed7b24ab7da3f98b3ddff86a35b9056c1f)) // PIC16C55A

	ROM_REGION(0x80000, "samples", 0) // EPROM near "qsound47_v1.0.u1" PIC
	ROM_LOAD("glitz_a_v1.0.u3",  0x00000, 0x80000, CRC(1ba2ca2d) SHA1(04f3bf7654388011df60879c155848324b6c0321))

	ROM_REGION(0x00400, "soundcpu_2", 0)
	/*
	  User ID
	    ID0=0x0000
	    ID1=0x0007
	    ID2=0x0006
	    ID3=0x0000
	  Config Word: 0x0005
	*/
	ROM_LOAD("qsound54_v1.0.u2", 0x00000, 0x00400, CRC(c0b0d229) SHA1(3ecc48a31da77639a82f8a41521b2ad5e7e1450b)) // PIC16C55A

	ROM_REGION(0x80000, "samples_2", 0) // EPROM near "qsound54_v1.0.u2" PIC
	ROM_LOAD("m7snd_c_v1.1.u11", 0x00000, 0x80000, CRC(b283ae44) SHA1(966c80bc27890380f1e81f1c223fe240df0e9e03))
ROM_END

} // anonymous namespace

//    YEAR   NAME     PARENT MACHINE  INPUT    CLASS          INIT        ROT   COMPANY       FULLNAME          FLAGS
GAME( 2014,  brglitz, 0,     brglitz, brglitz, brglitz_state, empty_init, ROT0, "Harry Levy", "Ballroom Glitz", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
