// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************************

    Skeleton driver for Recreativos Franco "RF51W14-CG" and other games on similar hardware.
    "RF51W14-CG" is the legal registry name, the game commercial name is unknown.

    TODO:
        - Everything

   ________________________________________________________________
  |   ___J16_____  ___J17____  __J18__  _______________________   |
  |  | ······ · | | ···· ·· | | ·· · | | :::::::::::::::::::: |   |
  |                           ____                                |
  | __  __________           |___|    ____   __________________ : <- JMP4
  || | | PIA 5   |                    |___| | RAM             |   |
  ||J| | OKI     |                    ____  | M48T08/M48T18   |   |
  ||1| | 82C55A  |                    |___| |_________________|   |
  ||5| |_________|                           __________________ : <- JMP3
  ||_|                                      | LOW             | : <- JMP2
  | __  __________            ____________  |                 |   |
  ||J| | PIA 4   |   XTAL    | Intel     |  |_________________|   |
  ||11 | OKI     |  32.000   |M80C188XL20|  __________________  : <- JMP1
  | __ | 82C55A  |   MHz     |           |  | HIGH            |__ |
  ||J| |_________|           |           |  |                 ||J||
  ||1|                       |___________|  |_________________||1||
  ||4|  __________                                        ____ |9||
  | __ | PIA 6   |                ____  ____    ______   |___| |_||
  ||J| | OKI     |               HC573A HC573A  |OKI  |   ____ __ |
  ||6| | 82C55A  |                              82C51A|  |___| |J||
  ||_| |_________|     __________________   ____               |8||
  | __     _______     | SONIDO 2       |  |___|               |_||
  || |SW3->DIPSx4|     |                |                      __ |
  ||J|  __________     |________________|                      |J||
  ||7| | PIA 3   |     __________________         __________   |9||
  ||_| | OKI     |     | SONIDO 1       |        |ULN2803A_|   |_||
  | __ | 82C55A  |     |                | ::      ________     __ |
  || | |_________|     |________________| JMP8   |74HC238E|    |J||
  ||J|                                 _____                   |5||
  ||4|  __________                    |____|                   __ |
  || | | PIA 1   |        ___________  ......<-JMP10           |J||
  ||_| | OKI     |       | OKI      |                          |1||
  | __ | 82C55A  |       | M6376    |  XTAL 9.8304             |2||
  ||J| |_________|       |__________|          MHz             __ |
  ||2|  _____SW2__   ____JMP7 RS232-2 RS232-1  ____            |J||
  ||0| |_DIPSx8__|  HC4040  :   ::::    ::::   |___|           |1||
  ||_|  __________   ____     ______    ____    ____           |0||
  | __ | PIA 2   |  |___|    |OKI  |   |___|   |___|           __ |
  ||J| | OKI     |   ____    82C51A|            ____           |J||
  ||3| | 82C55A  |  |___|            _         |___|           |1||
  || | |_________|         VOLUMEN->(_)    _________           |3||
  ||_|                                    |_LM380N_|           |_||
  |     _______   ____________  ____________     ______           |
  |SW1->DIPSx4|  |___________| |___________|    |_____|           |
  |_______________________________________________________________|

 JPM7 = CLK/128 / CLK/64

********************************************************************************************/

#include "emu.h"

#include "cpu/i86/i186.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/i8255.h"
#include "sound/okim6376.h"

#include "speaker.h"

namespace
{

class rf51w14cg_state : public driver_device
{
public:
	rf51w14cg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_okim6376(*this, "oki")
	{
	}

	void rf51w14cg(machine_config &config);
	void rfsantafem(machine_config &config);

protected:
	required_device <cpu_device> m_maincpu;
	required_device <okim6376_device> m_okim6376;
};

static INPUT_PORTS_START( rf51w14cg )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END

// Only two dip switches banks, both on CPU PCB
static INPUT_PORTS_START( rfsantafem )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

void rf51w14cg_state::rf51w14cg(machine_config &config)
{
	I80188(config, m_maincpu, 32_MHz_XTAL / 2); // Intel N80C188XL-20, guess divisor

	I8255A(config, "ppi1"); // OKI M82C55A
	I8255A(config, "ppi2"); // OKI M82C55A
	I8255A(config, "ppi3"); // OKI M82C55A
	I8255A(config, "ppi4"); // OKI M82C55A
	I8255A(config, "ppi5"); // OKI M82C55A
	I8255A(config, "ppi6"); // OKI M82C55A

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_okim6376, XTAL(9'830'400)/64).add_route(ALL_OUTPUTS, "mono", 1.0); // Frecuency divisor as per JMP7
}

void rf51w14cg_state::rfsantafem(machine_config &config)
{
	I80188(config, m_maincpu, 32_MHz_XTAL / 2); // N80C188XL20

	// CPU PCB
	I8255A(config, "ppi1");
	I8255A(config, "ppi2");
	I8255A(config, "ppi3");

	// PIAs PCB
	I8255A(config, "ppi4");

	SPEAKER(config, "mono").front_center();

	// Sound PCB
	OKIM6376(config, m_okim6376, 153600).add_route(ALL_OUTPUTS, "mono", 1.0); // RC OSC, guessed frequency

	// Reels PCB
	PIC16C57(config, "pic_reels", 20_MHz_XTAL / 2); // PIC16C57-HS, guessed divisor

	// Driver PCB
	PIC16C57(config, "pic_driver", 4_MHz_XTAL); // PIC16C57-XT/P
}

// The board was found with the program ROMs sockets unpopulated and the M48T08 with the battery dead
ROM_START( rf51w14cg )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "high.bin", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD( "low.bin",  0x80000, 0x80000, NO_DUMP )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "recreativos_franco_sonido-1_321dabf_01083c.u3", 0x00000, 0x80000, CRC(bd9bb391) SHA1(f08da81544e6b8c518634d081bf68d862b90c099) )
	ROM_LOAD( "recreativos_franco_sonido-2_7c8bced_01083d.u4", 0x80000, 0x80000, CRC(cf8e7957) SHA1(5d30d7f15c1690b819e467fc308f12f97577b906) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "recreativos_franco_mod_rf51w14-cg_2003_m48t08.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

/* "Santa Fe Mine". Slot machine from Recreativos Franco.
   Complete manual with schematics can be downloaded from: https://www.recreativas.org/manuales/tragaperras
   18 PCBs:
    * CPU PCB (Ref. 90360010)
      -3 x 8255
      -1 x N80C188XL20
      -1 x Xtal 32.0000 MHz
      -1 x MK48Z08
    * "Summarizer" PCB (Ref. 90360802)
    * Coin return PCB (Ref. 90362101)
    * Reels light PCB [1 lamp] (Ref. 90366702)
    * 12V power source PCB (Ref. 90372306)
    * PIAs PCB (Ref. 90371901) [the PCB supports two 8255, but Santa Fe Mine uses only one]
      -1 x 8255
    * Reels lights PCB [3 lamps] (Ref. 90376802)
    * Driver PCB (Ref. 90372203)
      -1 x PIC16C57-XT/P
      -Several RF custom chips
      -1 x Xtal 4.0000 MHz
    * Sound PCB (Ref. 90372401)
      -MSM6376
    * Reels ligts PCB [3 lamps] (Ref. 90376902)
    * Displays PCB (Ref. 90377101)
    * Opto PCB (Ref. 90377501)
    * Reels control (Ref. 90377301 or Ref. 90377303)
     -1 x PIC16C57-HS
     -1 x RF custom chip
     -1 x Xtal 20.0000 MHz
    * Displays PCB (Ref. 60383201)
    * Lights PCB [27 lamps] (Ref. 90383301)
    * Lights PCB [29 lamps] (Ref. 90383401)
    * Lights PCB [5 lamps] (Ref. 90383501)
    * Fuses PCB (Ref. 90358601)
*/
ROM_START( rfsantafem )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "96-016427_ah_high.u8", 0x00000, 0x10000, CRC(841d5515) SHA1(c089301e50d6c7fd0dcfe943488b5558a6ac9c8d) )
	ROM_LOAD( "96-016427_ah_low.u7",  0x10000, 0x08000, CRC(9a6eb06a) SHA1(cb6b3646f7d5f749c0ab022b03987d0c749fb32f) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "11497a.u3",            0x00000, 0x80000, CRC(8395d6ad) SHA1(d09c2d86932330cc99c19d0f06cadb92c96c4db1) )

	ROM_REGION( 0x180000, "pic_reels", 0 )
	ROM_LOAD( "34996a.u2",            0x00000, 0x02000, CRC(503c364d) SHA1(6c93766e5bba61babc608ef8393dd2c6c948a625) )

	ROM_REGION( 0x100000, "pic_driver", 0 )
	ROM_LOAD( "pic16c57.u1",          0x00000, 0x02000, NO_DUMP )
ROM_END

ROM_START( rfsantafema )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "96-008735_af_high.u8", 0x00000, 0x10000, CRC(015e7dfe) SHA1(5c6fdf9462b7990d980ca7ef0f37aa1309a6fbd1) )
	ROM_LOAD( "96-016427_ah_low.u7",  0x10000, 0x08000, CRC(9a6eb06a) SHA1(cb6b3646f7d5f749c0ab022b03987d0c749fb32f) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "11497a.u3",            0x00000, 0x80000, CRC(8395d6ad) SHA1(d09c2d86932330cc99c19d0f06cadb92c96c4db1) )

	ROM_REGION( 0x180000, "pic_reels", 0 )
	ROM_LOAD( "34996a.u2",            0x00000, 0x02000, CRC(503c364d) SHA1(6c93766e5bba61babc608ef8393dd2c6c948a625) )

	ROM_REGION( 0x100000, "pic_driver", 0 )
	ROM_LOAD( "pic16c57.u1",          0x00000, 0x02000, NO_DUMP )
ROM_END


} // anonymous namespace

//    YEAR  NAME         PARENT      MACHINE     INPUT       CLASS            INIT        ROT   COMPANY               FULLNAME                                                      FLAGS
GAME( 2003, rf51w14cg,   0,          rf51w14cg,  rf51w14cg,  rf51w14cg_state, empty_init, ROT0, "Recreativos Franco", "unknown Recreativos Franco slot machine (model RF51W14-CG)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, rfsantafem,  0,          rfsantafem, rfsantafem, rf51w14cg_state, empty_init, ROT0, "Recreativos Franco", "Santa Fe Mine (set 1)",                                      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, rfsantafema, rfsantafem, rfsantafem, rfsantafem, rf51w14cg_state, empty_init, ROT0, "Recreativos Franco", "Santa Fe Mine (set 2)",                                      MACHINE_IS_SKELETON_MECHANICAL )
