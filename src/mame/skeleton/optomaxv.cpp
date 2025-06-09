// license:BSD-3-Clause
// copyright-holders:
/*********************************************************************************************************

Optomax V image analyser, from AMS (Analytical Measuring Systems Ltd.).
The machine internally uses a CUBE EuroBEEB Single Board Computer (PCBs B6 and B7).

Back panel:
 - Camera input connector (DIN).
 - Camera video connector (3 x BNC).
 - VCR connector (BNC).
 - TGB monitor connector (DIN)
 - Printer connector (DB 26)
 - Data transfer serial connector (DB9).
 - Data transfer parallel connector (DB25).
 - Aux 1 connector (DIN).
 - Two sync mode switches (internal/external and interlace/non interlace).

Front panel:
 - Monitor (Philips M24-306GH/ED).
 - Control panel with the following layout:
 ______________________________________________________________________________________________
|           _____                                                    FIELD/                   |
|          /     \                     FRAME           TITLE          FEATURE         V (F1)  |
|         | DIAL  |        SET                                                                |
|         |       |       DETECTOR            CURRENT            INITIATE             W (F2)  |
|          \_____/                      EDIT          CALIBRATE         HOLD                  |
|                                                                                     X (F3)  |
|                                                                                             |
|     PREVIOUS    NEXT       A         B (7)   C (8)   D (9)     E (PRINT) F (ABORT)  Y (F4)  |
|                       (DETECT NULL)                                                         |
|             UP           G (HE)      H (4)   I (5)   J (6)     K (SPACE)     L      Z (F5)  |
|                                                                         (SEND DATA)         |
|      RIGHT HOME  LEFT    M (MR)      N (1)   O (2)   P (3)      Q (EXP)      R      / (F6)  |
|                                                                        (LOAD MATRIX)        |
|            DOWN           GREEN      S (0)   T (·)   U (DEL)     ENTER     GREEN    , (F7)  |
|_____________________________________________________________________________________________|


Six PCBs connected with two backplanes (Motorola VME 9 conns and a custom one with 13 conns):

OSD PCB (B6) (EURO-BEEB 65, CUBE EuroBEEB Single Board Computer PCB from Control Universal Ltd.)
 - Rockwell R6551AP ACIA
 - Rockwell R6522AP VIA
 - Toshiba TC5565PL SRAM
 - Xtal 1.8432
 - Hitachi HN613128PB05 Acorn BASIC Mask ROM (BASIC)
 - 2764 EPROM
 - M-3002-16PI Real Time Clock (RTC)
 - 2 x Signetics 82S147N PROMs
 - Rockwell R6502AP CPU
   __________________________________________
 _|                                          |_
| |__________________________________________| |
| Xtal                ________________________ |
| ___   ___________  | Rockwell              | |
||  |  |_82S147N__|  | R6502AP               | |
||  |   ___________  |_______________________| |
||  |  |_82S147N__|                            |
||_<-M-3002-16PI             _________________ |
|                           | EPROM          | |
|              __________   | M0             | |
|             |M74HC132B1   |________________| |
|              __________    _________________ |
|             |SN74S00N_|   | MASK (BASIC)   | |
|                           | M1             | |
|  _____                    |________________| |
| |    |                     _________________ |
| |BATT|                    | EMPTY          | |
| |2.4V|       __________   | M3             | |
| |____|      |_74F04N__|   |________________| |
| __________   __________    _________________ |
||AM26LS30PC  |AM26LS32PC   | TC5565PL-15    | |
|                  ..       |                | |
| Xtal 1.8432 MHz  ..       |________________| |
| _________________   ________________________ |
|| Rockwell       |  | Rockwell              | |
|| R6551AP        |  | R6522AP               | |
||________________|  |_______________________| |
|   _______      ___________________           |
|__| DIN  |_____||||||||||||||||||||___________|
   |______|


 TELETEXT PCB (B5) (CUBE TELETEXT Colour Video Interface from Control Universal Ltd.)
(this PCB lacks the buzzer present on other revisions)
 - Hitachi HD68B45SP CRT Controller
 - Xtal 6.000
 - 2 x NEC uPD2114LC SRAM
 - SAA 5050 Teletext Character Generator
 - Rockwell R6522-33 VIA
       _____________________
 _____|                     |__________________
|     |_____________________|                  |
|      __________     ____________________     |
|     |SN74LS174N    | Rockwell          |     |
|                    | R6522AP           |     |
|      __________    |___________________|     |
|     |_74LS86N_|      _________________       |
|                     |                |       |
|      __________     | SAA 5050       |       |
|     |_74LS04N_|     |________________|       |
|      __________   __________   __________    |
|     |_74LS04N_|  |_74LS92N_|  |uPD2114LC|    |
|        Xtal       __________   __________    |
|        6 MHz     |74LS245N_|  |uPD2114LC|    |
|      __________                              |
|     |_74LS00N_|     ____________________     |
|                    | Hitachi            |    |
|                    | HD46505SP-2        |    |
|                    |____________________|    |
|      __________   __________   __________    |
|     |74LS138N_|  |74LS157N_|  |74LS157N_|    |
|      __________   __________   __________    |
|     |74LS139N_|  |74LS136N_|  |74LS157N_|    |
|  __________________________________________  |
|_|                                          |_|
  |__________________________________________|


PCU PCB (PME 68-1B) (B4)
 - Signetics SCN68000 CPU
 - 16 x TMM41256P RAM
 - 2 x 27256 EPROM
 - 2 x 27128 EPROM
 - 3 x Hitachi HD46850P ACIA
 - AMPAL16L8 (labeled 0545)
 - PAL16L6C (labeled 0544)
 - 2 x PAL20L10C (labeled 0686 and 0620)
 - PAL12L10C (labeled 0546)
 - MM58167AN Real Time Clock (RTC)
 - MC68230L8 Parallel Interface / Timer 8MHz
 - Xtal 1.8432 MHz
 - MC14411P Bit Rate Generator
 - Xtal 32 MHz
      _____________________________________________________________________
     |    ________   _______________________________    __________        |
    _|_  |       |  |                              |   |SN74LS645-1N      |
 RES SW| |       |  |                              |    __________       _|_
    _|_  |       |  |  16 x TMM41256P-15           |   |SN74LS645-1N    |   |
 ABT SW| |       |  |                              |     ::::::         |   |
     |   |SCN68000  |______________________________|    __________      |   |
HTL LED  |       |  _______________  _______________   |SN74LS645-1N    |   |
    _|_  |       | | EPROM        | | EPROM        |    __________      |   |
   |   | |       | |              | |              |   |SN74LS645-1N    |   |
   |P3 | |       | |______________| |______________|    __________      |   |
   |DB25 |       |  _______________  _______________   |SN74LS645-1N    |   |
   |   | |       | | EPROM        | | EPROM        |    __________      |   |
   |___| |       | |              | |              |   |SN74LS645-1N    |   |
     |   |_______| |______________| |______________|    __________      |   |
    _|_  ..                           ::::::::         |SN74LS645-1N    |   |
   |   | ..   ____________   __________  ::::..         __________      |   |
   |P4 | ..  | HD46850P   | |SN74LS641_|  __________   |AMPAL16L8|      |   |
   |DB25 ..  |            | .. :::::::   |_74F74PC_|    __________      |___|
   |   | ..  |____________|                            |_74F241PC|        |
   |___| ..     __________   __________   __________    __________        |
     |         |_74S240N_|  |_PAL16L6_|  |PAL20l10_|   |SN74LS175N        |
    _|_  .. __________  __________  __________  __________  ___________   |
   |   | ..|_74S240N_| |SN74LS260N |SN74LS148N |SN74LS200N |MM58167AN |   |
   |P5 | .. __________  __________  __________  __________ |          |   |
   |SB25 ..|SN74LS393N |_74F74PC_| |SN74LS688N |_74F37N__| |__________|   |
   |   | .. ________    __________  :::::::::  ..... ::                  _|_
   |___| ..|SN74LS56P  |_74F241PC|       __________            .        |   |
     |      __________  __________      |PAL20L10_|            :        |   |
     |     |SN75188N_| |DM74LS01N|            ________________________  |   |
     |      ______  ______   __________      |                       |  |   |
     |     |     | |     |  |PAL12L10_|      | MC68230L8             |  |   |
     |     | 2 x HD46850P|   __________  ... |_______________________|  |   |
     |     |     | |     |  |SN75189N_| :::          Xtal               |   |
     |     |     | |     |   __________   _________1.8432 MHz_________  |   |
     | ..  |_____| |_____|  |SN75188N_|  |SN74LS556P       |SN74LS175N  |   |
     | ..  ______  ___  __________  __________  __________  __________  |   |
     | .. |     | |  | |_74F74PC_| |_SN74S08N| |_74F74PC_| |SN74LS32N|  |   |
     MC14411P-> | |  |  __________  __________  __________  __________  |   |
     | .. SN75189N->_| |_DL6311__| |_74F37N__| |_NE556N__| |_EP8304__|  |   |
     |    |     |       __________  __________  __________              |   |
     |    |_____|Xtal  |_74F74PC_| |_74F74PC_| |SN74LS56P|   ..         |___|
     |___________32 MHz___________________________________________________|


PCB (B3) (labeled "9000-0025 SS 1-1")
 - HM6116LP-3
 - MK48202B-20 NVRAM
 - 2 x HM6264LP-15
 - 3 x 27128 EPROM
 - Fujitsu MB7124H
         _______________________________________              _______________________________________
      __|                                       |____________|                                       |_
     |  |_______________________________________|            |_______________________________________| |
     |   ...                               :::::::                                                     |
     |    _______  __________  __________  __________  __________  __________  __________  __________  |
     |   |_P232_| |CD74HCT08E |SN74LS645| |SN74LS682N |SN74LS645N |SN74LS645N |SN74LS645N |SN74LS645N  |
     |             ______      ______     __________   ______    ______    ______    ______    ______  |
     |            |     |     |     |    |PC74HCT157P | <-HM6264LP-15 |   |     |   |     |   |     |  |
     |   HM6116LP-3->   |     |     |     __________  |     |   | <-HM6264LP-15 |   |     |   | <-EPROM|
     |            |  MK48202B-20->  |    |CD74HCT157E |     |   |     |   | <-EPROM |     |   |     |  |
     |            |     |     |     |     __________  |     |   |     |   |     |   | <-EPROM |     |  |
     |            |_____|     |_____|    |CD74HCT157E |_____|   |_____|   |____ |   |_____|   |_____|  |
     |  _________  __________  __________  __________  __________  __________  __________  __________  |
     | CD74HCT32E PC74HCT138P PC74HCT157P  CD74HCT04E PC74HCT164P CD74HCT273E |_MB7124H_|  CD74HCT32E  |
     |  ::::.                                                                                          |
     |  _________  __________  __________  __________  __________  __________  __________  __________  |
     | |__8429__| PC74HCT139P PC74HCT139P CD74HCT174E  CD74HCT74E  SN74LS645N PC74HCT374P  SN74LS645N  |
     |  _________  __________  __________     ::::                                                     |
     | CD74HCT04E  CD74HCT74E  CD74HCT08E                                           _____________      |
     |_____________________________________________________________________________|             |_____|
                                                                                   |_____________|

PCB MEASUREMENT (B2) (labeled "MEASUREMENT CIRCUIT 9000-0024")
 - AM2149-45DC
 - 12 x TMS4416-15NL
 - 2 x Intel P8254
 - Motorola MC68230P10
 - 3 x AM27S21APC PROM
 - Fujitsu MB7124H PROM


PCB VIDEO (B1) (labeled "OPTOMAX VIDEO PCB 9000-0022-2/2")
 - Xtal (unoknown frequency, labeled "Xtal Hy-O 2562-50 GE01S")
 - 2716 EPROM
 - 2 x Toshiba TC5565PL-15
 - Motorola MC68230P8
                   ___     ___     ___     ___
                  |   |   |   |   |   |   |   |
   _______________|   |___|   |___|   |___|   |________________________________
  |               |___|   |___|   |___|   |___|                               |
  |                           4 x BNC                                         |
 _|_                ······    __________  __________  __________    ······    |
|   |                        |_74128N__| |PC74HCT04P |PC74HCT74P              |
|   |                         __________  __________  __________  __________  |
|   |                        |HEF4046BP| |SN74LS221N |PC74HCT74P |I1-524-5_|  |
|   |                         ________    __________                          |
|   |                        |CA3140E|   |SN74LS221N                          |
|   |                                     __________                          |
|   |                                    |PC74HCT32P                          |
|   |  _________                          __________                          |
|   | |SN74LS05N                         |PC74HCT08P                          |
|   |                         __________  __________  __________  ________    |
|   |                        |MC74HC04_| |SN74LS123N |_LM361N__| |CA3240E|    |
|   |                                                                         |
|   |                         __________                          __________  |
|   |                        |_LM361N__|                         |I1-201-5_|  |
|___|              _______    __________                                      |
  |               |CA3240E|  |_LM361N__|                                      |
  |    _________  __________  __________                                      |
 _|_ CD74HCT151E |CD74HCT86E |_LM361N__|                          __________  |
|   |             ________                                       |I1-201-5_|  |
|   |            |CA3240E|                                        ________    |
|   |  _________  __________  __________                         |CA3140E|    |
|   | PC74HCT08P |AD7528KN_| |PC74HCT04P                                      |
|   |        ___                          __________  __________  __________  |
|   |       |  |  ..   ________          |_ZN448E__| |__DAC08__| |CD74HCT00E  |
| SN74LS682N-> |  ..  |       |                       __________  __________  |
|   |       |__|  ..  | MC68230P8                    PC74HCT374P |SN74LS221N  |
|   |        ___      |       |                                               |
|   |       |  |      |       |         ____________  __________  __________  |
|   DS75451N->_|      |       |        |TC5565PL-15| CD74HCT393E |SN74LS221N  |
|   |        ___      |       |        |___________|                          |
|   |       |  |      |       |                       __________  __________  |
| SN74LS645N-> |      |       |         ____________ |PC74HCT74P |CD74HCT00E  |
|___|       |__|      |_______|        |TC5565PL-15|  __________  __________  |
  |   BATT             _____           |___________| CD74HCT4040E SN74LS221N  |
  |   __________  _____________  __________           __________              |
  |   PC74HCT32P | EPROM      | CD74HCT4040E         CD74HCT4040E             |
  |              |____________|                                               |
  |   __________   __________    __________           __________              |
  |  |PC74HCT74P  |PC74HCT74P   |CD74HCT00E          |SN74LS682N              |
  |   __________   __________    __________           __________              |
  |  |_ZNA134H_|  |SN74LC221N   |PC74HCT157P         |PC74LS157P              |
  |                                                                           |
  |   Xtal (labeled "Hy-O 2562-50 GE01S")                                     |
  |___________________________________________________________________________|

*********************************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m68000/m68000.h"

#include "machine/6522via.h"
#include "machine/68230pit.h"
#include "machine/6850acia.h"
#include "machine/m3002.h"
#include "machine/mc14411.h"
#include "machine/mm58167.h"
#include "machine/mos6551.h"
//#include "machine/nvram.h"
#include "machine/pit8253.h"

#include "video/mc6845.h"
#include "video/saa5050.h"

#include "screen.h"


namespace {

class optomaxv_state : public driver_device
{
public:
	optomaxv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)

		// CPU PCB (B4)
		, m_maincpu(*this, "maincpu")
		, m_acia_1(*this, "acia1")
		, m_acia_2(*this, "acia2")
		, m_acia_3(*this, "acia3")
		, m_brg(*this, "brg")
		, m_rtc_1(*this, "rtc1")
		, m_pit_1(*this, "pit1")

		// OSD PCB (B6)
		, m_bbccpu(*this, "bbccpu")
		, m_acia_4(*this, "acia4")
		, m_rtc_2(*this, "rtc2")
		, m_via_1(*this, "via1")

		// Teletext PCB (B5)
		, m_trom(*this, "saa5050")
		, m_hd6845(*this, "hd6845")
		, m_screen(*this, "screen")
		, m_via_2(*this, "via2")

		// Measurement PCB (B2)
		, m_pit_2(*this, "pit2")
		, m_pit_3(*this, "pit3")
		, m_pit_4(*this, "pit4")

		// Video PCB (B1)
		, m_pit_5(*this, "pit5")
	{ }

	void optomaxv(machine_config &config);

private:
	// CPU PCB (B4)
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia_1;
	required_device<acia6850_device> m_acia_2;
	required_device<acia6850_device> m_acia_3;
	required_device<mc14411_device> m_brg;
	required_device<mm58167_device> m_rtc_1;
	required_device<pit68230_device> m_pit_1;

	// OSD PCB (B6)
	required_device<m6502_device> m_bbccpu;
	required_device<mos6551_device> m_acia_4;
	required_device<m3002_device> m_rtc_2;
	required_device<via6522_device> m_via_1;

	// Teletext PCB (B5)
	required_device<saa5050_device> m_trom;
	required_device<mc6845_device> m_hd6845;
	required_device<screen_device> m_screen;
	required_device<via6522_device> m_via_2;

	// Measurement PCB (B2)
	required_device<pit68230_device> m_pit_2;
	required_device<pit8254_device> m_pit_3;
	required_device<pit8254_device> m_pit_4;

	// Video PCB (B1)
	required_device<pit68230_device> m_pit_5;
};


static INPUT_PORTS_START(optomaxv)
INPUT_PORTS_END

void optomaxv_state::optomaxv(machine_config &config)
{
	// All clocks unverified

	// Main CPU PCB (B4)

	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // Signetics SCN68000

	ACIA6850(config, m_acia_1, 0); // Hitachi HD46850P
	ACIA6850(config, m_acia_2, 0); // Hitachi HD46850P
	ACIA6850(config, m_acia_3, 0); // Hitachi HD46850P

	MC14411(config, m_brg, 32_MHz_XTAL / 10); // Motorola MC14411P

	MM58167(config, m_rtc_1, 32_MHz_XTAL / 10); // National Semiconductor MM58167AN

	PIT68230(config, m_pit_1, 1.8432_MHz_XTAL); // Motorola MC68230L8

	// OSD PCB (B6) (CUBE EuroBEEB System CPU board)

	M6502(config, m_bbccpu, 1.8432_MHz_XTAL); // Rockwell R6502AP

	MOS6522(config, m_via_1, 1.8432_MHz_XTAL); // Rockwell R6551AP
	MOS6551(config, m_acia_4, 1.8432_MHz_XTAL); // Rockwell R6551AP

	M3002(config, m_rtc_2, 1.8432_MHz_XTAL); // uEM M-3002-16PI Real Time Clock

	// Teletext PCB (B5) (CUBE EuroBEEB System Teletext Video Card)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("hd6845", FUNC(hd6845s_device::screen_update));

	SAA5050(config, m_trom, 6_MHz_XTAL);
	m_trom->set_screen_size(40, 25, 40);

	HD6845S(config, m_hd6845, 6_MHz_XTAL / 3); // Hitachi HD46505SP-2
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(12);

	MOS6522(config, m_via_2, 6_MHz_XTAL / 6); // Rockwell R6551AP

	// PCB (B3)

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // Mostek MK48Z02B-20 ZeroPower

	// Measurement PCB (B2)

	PIT68230(config, m_pit_2, 2_MHz_XTAL); // Motorola MC68230P8, unknown xtal
	PIT8254(config, m_pit_3); // Intel P8254
	PIT8254(config, m_pit_4); // Intel P8254

	// VIDEO PCB (B1)

	PIT68230(config, m_pit_5, 2_MHz_XTAL); // Motorola MC68230P8, unknown xtal
}


ROM_START(optomaxv)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_LOAD16_BYTE( "b4_68up-12-01-sd-1-86_27256.j41", 0x00000, 0x08000, CRC(a3196db7) SHA1(06773dea886908673d9def406d4985a6eef71d0c) )
	ROM_LOAD16_BYTE( "b4_68lw-12-01-sd-1-86_27256.j25", 0x00001, 0x08000, CRC(c0a88d38) SHA1(836a616bb6df84e3e6dfb1d42cacd592f6c6b4e6) )
	ROM_LOAD( "b4_hn4827128g.j24",                      0x10000, 0x04000, CRC(a6399550) SHA1(602b7152a5a5cbe1f8f598e6622d2bdfccd5d322) )
	ROM_LOAD( "b4_hn4827128g.j40",                      0x14000, 0x04000, CRC(b85f37f7) SHA1(dc532ba0af735233c5a7308a5fbc90be746ee08c) )

	ROM_REGION(0x44000, "bbccpu", 0)
	ROM_LOAD( "b6_abmon4-abci-12-12_hn482764g.m0",      0x40000, 0x02000, CRC(8f9ce214) SHA1(37fca4a5184025ed034acd5f20a4614163246779) ) // On OSD PCB
	ROM_LOAD( "b6_hn613128pb05.m1",                     0x0c000, 0x04000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) ) // Standard BBC Micro Acorn BASIC, on OSD PCB

	ROM_REGION(0x10000, "unknown", 0)
	ROM_LOAD( "b3_6sr03-04-b03f-1-86_hn4827128g.ic22",  0x00000, 0x04000, CRC(705f0f4d) SHA1(5c836694d736b2d2a268e3a8acdfb76fde6698c4) ) // On PCB (B3)
	ROM_LOAD( "b3_6sr02-04-8140-1-86_d27128d.ic23",     0x04000, 0x04000, CRC(58c04246) SHA1(7eb0b276206f546b34bc5f32e05d755f1a700294) ) // On PCB (B3)
	ROM_LOAD( "b3_6sr01-04-4477-1-86_hn4827128g.ic24",  0x08000, 0x04000, CRC(e9110243) SHA1(c96f6f526b0b1c36971f7a0fc03a6e57d02213e5) ) // On PCB (B3)

	ROM_REGION(0x00800, "video", 0)
	ROM_LOAD( "b1_9000-vs-1.1_d2716d.ic46",             0x00000, 0x00800, CRC(ebaefb94) SHA1(ca6d194926a98b846443ce7393e3b44d3e5199f9) ) // On Video PCB (B1)

	ROM_REGION(0x00200, "proms", 0)
	ROM_LOAD( "b6_sp007mp_82s147n.ic9",                 0x00000, 0x00200, CRC(35aaa7a3) SHA1(ebc977ff748a19cd0e9d0626cf7cf97d07656f80) ) // On OSD PCB (B6), for CPU address decoding
	ROM_LOAD( "b6_502_82s147.ic10",                     0x00000, 0x00200, CRC(401fa579) SHA1(e6320f70da9dfed0daae47af7b6cf9f3a62313b2) ) // On OSD PCB (B6), for CPU address decoding. Same as the standard EURO-BEEB
	ROM_LOAD( "b3_b515c1-1.ic15",                       0x00000, 0x00200, CRC(c4b02b5f) SHA1(e7b3363974b8a1b61169f543a672dff37e8e0e11) ) // On PCB (B3)
	ROM_LOAD( "b2_cpi-sl-1.3_am27s21a.ic29",            0x00000, 0x00100, CRC(897071f9) SHA1(912154fd24d3601bcfd7fbd61be5c1ade62c12f3) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hb-11.2_mb7124h.ic6",                 0x00000, 0x00200, CRC(78bab798) SHA1(f5b88db41efed9c540801c367047d608fb086094) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hgb-22.2_am27s21a.ic7",               0x00000, 0x00100, CRC(5c3f4be5) SHA1(1ead926a5c71232c75f20673fe0a7c36ff4480bb) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hgw-1.2_am27s21a.ic8",                0x00000, 0x00100, CRC(b45f4d48) SHA1(7cfe7b19efc7d034a5795a99dda347ae742c904d) ) // On Measurement PCB (B2)

	ROM_REGION(0x00117, "plds", 0)
	ROM_LOAD( "b4_0544_pal16l6.j22",                    0x00000, 0x00054, CRC(7d325ea4) SHA1(723a9938b7e3a0edf38261d7b6349efe0443d2e0) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0545_pal16l8.j66",                    0x00000, 0x00104, CRC(f1837b78) SHA1(8eb40c7320bd626ec6037662234c5befba88e116) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0546_pal12l10c.j17",                  0x00000, 0x00040, CRC(b669fd4a) SHA1(f122719c62e797a1c514389d8ea013e86c8b2040) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0620_pal20l10c.j76",                  0x00000, 0x000cc, CRC(51e963ee) SHA1(81f0ad7e2505d31eb00be6742146c80e5631f3f2) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0686_pal20l10c.j54",                  0x00000, 0x000cc, CRC(7cf9018b) SHA1(dcbfb718ab076935eb74452ba97a505d0bd34546) ) // On main CPU PCB (B4)
ROM_END

} // anonymous namespace

SYST( 1986, optomaxv, 0, 0, optomaxv, optomaxv, optomaxv_state, empty_init, "AMS", "Optomax V", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING ) // Hardware from 1985, software on ROM from 1986.
