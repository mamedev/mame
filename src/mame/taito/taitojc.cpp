// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap
/*************************************************************************

  Taito JC System

  Driver by Ville Linde, based on the preliminary driver by David Haywood

Taito custom chips on this hardware:
- TC0640FIO      : I/O
- TC0770CMU      : Math co-processor?
- TC0780FPA x 2  : Polygon/Texture renderer?
- TC0840GLU      : 2D graphics?
- TC0870HVP      : Vertex processor?

TODO:
- Games are running at wrong speed(unthrottled?) compared to pcb recordings, easily noticeable on sidebs/sidebs2,
  for example the selection screens are too fast, and the driving is almost twice as slow. Even slower after
  the m68k fpu/softfloat update since MAME 0.267.
- dendego intro object RAM usage has various gfx bugs (check video file)
- dendego title screen builds up and it shouldn't
- dendego attract mode train doesn't ride, demo mode doesn't set the throttle, but it does set the brake pressure
- landgear has some weird crashes (after playing one round, after a couple of loops in attract mode) (needs testing -AS)
- landgear has huge 3d problems on gameplay (CPU comms?)
- dangcurv DSP program crashes very soon, so no 3d is currently shown. - due to undumped rom? maybe not?
- add idle skips if possible

BTANB:
- incorrect perspective textures, visible when close to the camera such as sidebs rear-view mirror

--------------------------------------------------------------------------

PCB notes:


Side By Side 2
Taito, 1997

This game runs on Taito JC System hardware

PCB Layout
----------

Top board: MOTHER PCB-C K11X0838A  M43E0325A
|---------------------------------------------------------------------------------|
|                                                                 43256           |
|54MHz                                   E23-32-1.51              43256           |
|         424210 424210                                           43256           |
|                      E23-29.39                                                  |
|E23-25-1.3 TC0870HVP             E23-31.46                                       |
|           (QFP208)   E23-30.40             E23-34.72      93C46.87              |
|E23-26.4                                                                         |
|                              MC68040RC25            CXD1178Q    TC0640FIO       |
|424260     TMS320C51          (PGA TYPE)                         (QFP120)        |
|           (QFP132)                                                              |
|           labelled                                              TEST_SW         |
|424260     "Taito E07-11"                                      MB3771   RESET_SW |
|                                  E23-33.53                                      |
|                                             CY7B991       MB8421-90             |
|4218160    43256                    CY7B991                                      |
|                                                                                 |
|           43256        TC0770CMU                          E23-35.110            |
|4218160                 (QFP208)                                                 |
|                                    10MHz    MC68EC000     LC321664AJ-80         |
|E23-27.13  TC0780FPA                                                             |
|           (QFP240)                                        ENSONIQ               |
|                      D482445                TC51832       ESPR6 ES5510          |
|                                             TC51832                             |
|4218160               D482445                                                    |
|                                 TC0840GLU                 MC33274   TDA1543     |
|                      D482445    (QFP144)                                        |
|4218160                                      16MHz         MB87078               |
|           TC0780FPA  D482445           30.4761MHz                               |
|           (QFP240)              ENSONIQ                                         |
|E23-28.18                        OTISR2                                          |
|                                                                                 |
|---------------------------------------------------------------------------------|

Notes:
      All 3K files are PALs type PALCE 16V8 and saved in Jedec format.


Bottom board: JCG DAUGHTERL PCB-C K91E0677A
|---------------------------------------------------------------------------------|
|                                                                                 |
|                                                                                 |
|                    E38-19.30  E38-20.31                                         |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|  E38-01.5   E38-09.18   E23-15.32                                               |
|                                                                                 |
|  E38-02.6   E38-10.19   E38-17.33                                               |
|                                                                                 |
|  E38-03.7   E38-11.20   E38-18.34                     E17-23.65                 |
|                                                                                 |
|  E38-04.8   E38-12.21   E38-21.35                     TC5563                    |
|                                                                                 |
|  E38-05.9   E38-13.22   SBS2_P0.36                       MC68HC11M0             |
|                                                          (QFP80)                |
|  E38-06.10  E38-14.23   SBS2_P1.37                                              |
|                                                                                 |
|  E38-07.11  E38-15.24   SBS2_P2.38                                              |
|                                                          E23-37.69              |
|  E38-08.12  E38-16.25   SBS2_P3.39                             SMC_COM20020I    |
|                                                                                 |
|                                                                                 |
|                                                      E23-38.73                  |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|---------------------------------------------------------------------------------|

Notes:
      All 3K files are PALs type PALCE 16V8 and saved in Jedec format.
      ROMs .36-.39 are 27C4001, main program.
      ROMs .5-.7, .9-.12, .18-.20, .22-.25 are 16M MASK, graphics.
      ROMs .32-.35 are 16M MASK, ES5505 samples.
      ROMs .8 and .21 are 4M MASK, graphics.
      ROMs .30-.31 are 27C2001, 68000 sound program.
      ROM .65 is 27C512, MC68HC11 program.

----

--------------------------------------------------------------------------
SIDE BY SIDE  / 2       JC-SYSTEM TYPE-C
--------------------------------------------------------------------------
 SIDE BY SIDE (E23) (C)TAITO 1996 VER 2.5J 1996/6/20 18:13:14
 E23-01 to 24 + E17-23 (BIOS ?)   E17:LANDING GEAR

 SIDE BY SIDE 2 (E38)   Not dumped
 ROMKIT : E38-01 to 21 , 23* to 26*

--------------------------------------------------------------------------
DENSHA DE GO! (E35)     JC-SYSTEM TYPE-C with TRAIN BOARD (Ext.Sound)
--------------------------------------------------------------------------
 DENSHA DE GO! VER 2.2J 1997/2/4
 E35-01 to 26 + E35-28(TRAIN BOARD) + E17-23(BIOS?)

 DENSHA DE GO! EX VER 2.4J 1997/4/18 13:38:34
 ROMKIT : E35-30 to 33

--------------------------------------------------------------------------
DENSHA DE GO! 2 (E52)   JC-SYSTEM TYPE-C with TRAIN BOARD (Ext.Sound)
--------------------------------------------------------------------------
 DENSHA DE GO! 2 (KOUSOKUHEN RYOUSANSYA) VER 2.5 J 1998/3/2 15:30:55
 E52-01 to 24 , 25-1 to 28-1, 29, 30 + E35-28(TRAIN BOARD) + E17-23(BIOS?)

 DENSHA DE GO! 2 (3000BANDAI KOUSOKUHEN) VER 2.20 J 1998/7/15 17:42:38
 ROMKIT :  E52-31 to 38

----

Landing Gear
Taito, 1995

This is a flight simulator game running on Taito JC System hardware.
The system comprises two boards plugged together back-to-back via five high density connectors.
The top board contains the main CPU, all RAMs, graphics and sound hardware and the bottom
board contains the game ROMs, communication devices and a MC68HC11 MCU. Landing Gear
is a single player game so the board does not contain any communication hardware.
This hardware seems to be the earliest revision of the JC board. This is the only JC game that
had a 28-way edge connector, which is actually almost JAMMA. The power/GND, video and
most buttons (test/service/coin/start etc) work fine using any standard JAMMA cab. However the
analog controls are wired to another connector on the bottom PCB via an interface board.
When other JC bottom boards are swapped to this top board, they will run fine, but some of the
graphics are messed up. This is probably due to the different PALs and their changed locations.
This seems to mostly affect the texture mapping (tested with swapping Side By Side 2) as the title
and most of the background graphics are ok but the car textures are just a mess of pixels.


PCB Layout
----------

Top board: MOTHER PCB  K11X0835A  M43E0304A
|---------------------------------------------------------------------------------|
|                                                                 43256  43256    |
|54MHz                                                            43256           |
|                                                                                 |
|TC514260                                                                         |
|         TC0870HVP  uPD424210                                    TC0640FIO       |
|TC514260 (QFP208)   uPD424210             E07-08.65              (QFP120)        |
|E07-02.4                                                                         |
|                                                                                 |
|TMS320C51  43256                                                                 |
|(QFP132)   43256              MC68040RC25          E07-10.116  93C46.91          |
|labelled          TC0770CMU   (PGA TYPE)                    E07-04.115  TEST_SW  |
|"Taito E07-11"    (QFP208)                         E07-09.82      MB3771         |
|                                                                                 |
|                                                            MB8421-90            |
|TC528257   E07-06.37                                                             |
|                                                                                 |
|TC528257   TC514260                                                              |
|                              E07-07.49    CY7B991                               |
|TC528257   TC514260           E07-03.50                     TC511664             |
|                     TC0780FPA                                                   |
|TC528257   TC514260  (QFP240)           10MHz   MC68EC000   ENSONIQ              |
|                                 CY7B991                    ESPR6 ES5510         |
|TC528257   TC514260                                                              |
|                                                                                 |
|TC528257   TC514260              TC0840GLU   TC51832       MC33274   TDA1543     |
|                                 (QFP144)    TC51832                             |
|TC528257   TC514260                             16MHz      MB87078               |
|                     TC0780FPA          30.4761MHz                               |
|TC528257   TC514260  (QFP240)                                                    |
|                                           ENSONIQ                               |
|TC528257             E07-05.22             OTISR2                                |
|---------------------------------------------------------------------------------|

Notes:
      All 3K files are PALs type PALCE 16V8 and saved in Jedec format.


Bottom board: JCG DAUGHTER PCB-L K91E0603A
|---------------------------------------------------------------------------------|
|                                                                                 |
|                                                                                 |
|                    E17-21.30  E17-22.31                                         |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|  E17-01.5   E17-07.18   E17-13.32                                               |
|                                                                                 |
|  *          E17-08.19   E17-14.33                                               |
|                                                                                 |
|  *          *           E17-15.34                     E17-23.65                 |
|                                                                                 |
|  E17-02.8   *           E17-16.35                     TC5563                    |
|                                                                                 |
|  E17-03.9   E17-09.22   E17-37.36                        MC68HC11M0             |
|                                                          (QFP80)                |
|  E17-04.10  E17-10.23   E17-38.37                                               |
|                                                                                 |
|  E17-05.11  E17-11.24   E17-39.38                                               |
|                                                          E09-21.69              |
|  E17-06.12  E17-12.25   E17-40.39                                               |
|                                                                                 |
|                                                                                 |
|                                                      E09-22.73                  |
|                                                                                 |
|                                                                                 |
|                                                      E17-32.96                  |
|                                                                                 |
|                                                                                 |
|---------------------------------------------------------------------------------|

Notes:
      All 3K files are PALs type PALCE 16V8 and saved in Jedec format.
      ROMs .36-.39 are 27C4001, main program.
      ROM .65 is 27C512, 68HC11 MCU program.
      ROMs .30-.31 are 27C2001, sound program.
      ROMs .32-.35 are 16M MASK, sound.
      ROMs .5-.25 are 16M MASK, graphics.
      * Unpopulated ROM socket

----

Side By Side
Taito, 1996

This game runs on Taito JC System hardware and uses a 24kHz monitor.

PCB Layout
----------

Top board: MOTHER PCB-C K11X0838A  M43E0325A
|---------------------------------------------------------------------------------|
|                                                                 43256           |
|54MHz                                   E23-32-1.51              43256           |
|         424210 424210                                           43256           |
|                      E23-29.39                                                  |
|E23-25-1.3 TC0870HVP             E23-31.46                                       |
|           (QFP208)   E23-30.40             E23-34.72      93C46.87              |
|E23-26.4                                                                         |
|                              MC68040RC25            CXD1178Q    TC0640FIO       |
|424260     TMS320C51          (PGA TYPE)                         (QFP120)        |
|           (QFP132)                                                              |
|           marked                                                TEST_SW         |
|424260     "Taito E07-11"                                      MB3771   RESET_SW |
|                                  E23-33.53                                      |
|                                             CY7B991       MB8421-90             |
|4218160    43256                    CY7B991                                      |
|                                                                                 |
|           43256        TC0770CMU                          E23-35.110            |
|4218160                 (QFP208)                                                 |
|                                    10MHz    MC68EC000     LC321664AJ-80         |
|E23-27.13  TC0780FPA  D482445                                                    |
|           (QFP240)                                        ENSONIQ               |
|                      D482445                TC51832       ESPR6 ES5510          |
|                                             TC51832                             |
|4218160                                                                          |
|                                 TC0840GLU                 MC33274   TDA1543     |
|                                 (QFP144)                                        |
|4218160               D482445                16MHz         MB87078               |
|           TC0780FPA                    30.4761MHz                               |
|           (QFP240)   D482445    ENSONIQ                                         |
|E23-28.18                        OTISR2                                          |
|                                                                                 |
|---------------------------------------------------------------------------------|

Notes:
      All 3k files are PALs type PALCE 16V8 and saved in Jedec format.
      CY7B991 - Programmable Skew Clock Buffer (PLCC32)
      4218160 - 2M x8 / 1M x16 DRAM. Compatible with NEC 4218160 & Toshiba TC5118160
      424210  - 256k x16 DRAM
      424260  - 256k x16 DRAM
      43256   - 32k x8 SRAM
      D482445 - 256k x16 Video DRAM. Compatible with Toshiba TC524165/TC52V4165 (also used on Namco System 11 CPU boards)
      LC321664- 64k x16 DRAM
      TC51832 - 32k x8 SRAM
      MB8421  - 16k-bit (2kbytes) Dual Port SRAM

      Measurements:
                   HSync  - 24.639kHz / 24.690kHz (alternates between the two frequencies slowly every ~2 seconds)
                   VSync  - 55.6795Hz
                   68040  - 20.000MHz (10MHz*2, source = CY7C991)
                   68000  - 15.23805MHz (30.4761/2)
                   320C51 - 40.000MHz (pin96 X2/CLKIN. 10MHz*4, source = CY7C991)
                   OTISR2 - 3.80950MHz (pin12)
                   ES5510 - 2.22MHz, 2.666MHz, 3.8095125MHz (30.4761/8), 8.000MHz (16/2)

      X1 is labeled "54/33.333MHz" on MOTHER PCB-C, but only 54 MHz appears to have been used.

      X3 is labeled "30.47610MHz" on MOTHER PCB-C and "30.4762MHz" on at least one actual XTAL. Both of these values
      are likely rounded off from the 30.47618 MHz frequency used with the same Ensoniq chips in other Taito games.

Bottom board: JCG DAUGHTERL PCB-C K9100633A J9100434A (Sticker K91J0633A)
|---------------------------------------------------------------------------------|
|                                                                                 |
|                                                                                 |
|                    E23-23.30  E23-24.31                                         |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|  E23-01.5   E23-08.18   E23-15.32                                               |
|                                                                                 |
|  E23-02.6   E23-09.19   E23-16.33                                               |
|                                                                                 |
|  E23-03.7   E23-10.20   E23-17.34                     E17-23.65                 |
|                                                                                 |
|  E23-04.8   E23-11.21   E23-18.35                     6264                      |
|                                                                                 |
|  E23-05.9   E23-12.22   E23-19.36                        MC68HC11M0             |
|                                                          (QFP80)                |
|  E23-06.10  E23-13.23   E23-20.37                                               |
|                                                                                 |
|  *          *           E23-21.38                                               |
|                                                          E23-37.69              |
|  E23-07.12  E23-14.25   E23-22.39                              SMC_COM20020I    |
|                                                                                 |
|                                                                                 |
|                                                      E23-38.73                  |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|                                                                                 |
|---------------------------------------------------------------------------------|

Notes:
      All 3k files are PALs type PALCE 16V8 and saved in Jedec format.
      6264: 8k x8 SRAM
      SMC_COM20020I: Network communmication IC
      ROMs .36-.39 are 27C4001, main program.
      ROMs .5-.12, .18-.25 are 16M MASK, graphics.
      ROMs .32-.35 are 16M MASK, sound data.
      ROMs .30-.31 are 27C2001, sound program.
      ROM  .65 is 27C512, linked to 68HC11 MCU
      *    Unpopulated socket.

      Measurements:
                   68HC11 - 8.000MHz (16/2 on pin74)
                            (MCU is a MC68HC11M0CFU4 which could run twice as fast)
*/

#include "emu.h"
#include "taitojc.h"
#include "taito_en.h"

#include "cpu/m68000/m68040.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "cpu/tms32051/tms32051.h"
#include "machine/eepromser.h"
#include "taitoio.h"
#include "sound/es5506.h"
#include "sound/okim6295.h"

#include "speaker.h"

#include "dendego.lh"


// lookup tables for densha de go analog controls/meters
static const int dendego_odometer_table[0x100] =
{
	0,    3,    7,    10,   14,   17,   21,   24,   28,   31,   34,   38,   41,   45,   48,   52,
	55,   59,   62,   66,   69,   72,   76,   79,   83,   86,   90,   93,   97,   100,  105,  111,
	116,  121,  126,  132,  137,  142,  147,  153,  158,  163,  168,  174,  179,  184,  189,  195,
	200,  206,  211,  217,  222,  228,  233,  239,  244,  250,  256,  261,  267,  272,  278,  283,
	289,  294,  300,  306,  311,  317,  322,  328,  333,  339,  344,  350,  356,  361,  367,  372,
	378,  383,  389,  394,  400,  406,  412,  418,  424,  429,  435,  441,  447,  453,  459,  465,
	471,  476,  482,  488,  494,  500,  505,  511,  516,  521,  526,  532,  537,  542,  547,  553,
	558,  563,  568,  574,  579,  584,  589,  595,  600,  607,  613,  620,  627,  633,  640,  647,
	653,  660,  667,  673,  680,  687,  693,  700,  705,  711,  716,  721,  726,  732,  737,  742,
	747,  753,  758,  763,  768,  774,  779,  784,  789,  795,  800,  806,  812,  818,  824,  829,
	835,  841,  847,  853,  859,  865,  871,  876,  882,  888,  894,  900,  906,  911,  917,  922,
	928,  933,  939,  944,  950,  956,  961,  967,  972,  978,  983,  989,  994,  1000, 1005, 1011,
	1016, 1021, 1026, 1032, 1037, 1042, 1047, 1053, 1058, 1063, 1068, 1074, 1079, 1084, 1089, 1095,
	1100, 1107, 1113, 1120, 1127, 1133, 1140, 1147, 1153, 1160, 1167, 1173, 1180, 1187, 1193, 1200,
	1203, 1206, 1209, 1212, 1216, 1219, 1222, 1225, 1228, 1231, 1234, 1238, 1241, 1244, 1247, 1250,
	1253, 1256, 1259, 1262, 1266, 1269, 1272, 1275, 1278, 1281, 1284, 1288, 1291, 1294, 1297, 1300,
};

static const int dendego_pressure_table[0x100] =
{
	0,    0,    0,    0,    5,    10,   14,   19,   24,   29,   33,   38,   43,   48,   52,   57,
	62,   67,   71,   76,   81,   86,   90,   95,   100,  106,  112,  119,  125,  131,  138,  144,
	150,  156,  162,  169,  175,  181,  188,  194,  200,  206,  212,  219,  225,  231,  238,  244,
	250,  256,  262,  269,  275,  281,  288,  294,  300,  306,  312,  318,  324,  329,  335,  341,
	347,  353,  359,  365,  371,  376,  382,  388,  394,  400,  407,  413,  420,  427,  433,  440,
	447,  453,  460,  467,  473,  480,  487,  493,  500,  507,  514,  521,  529,  536,  543,  550,
	557,  564,  571,  579,  586,  593,  600,  607,  614,  621,  629,  636,  643,  650,  657,  664,
	671,  679,  686,  693,  700,  706,  712,  719,  725,  731,  738,  744,  750,  756,  762,  769,
	775,  781,  788,  794,  800,  807,  814,  821,  829,  836,  843,  850,  857,  864,  871,  879,
	886,  893,  900,  907,  914,  921,  929,  936,  943,  950,  957,  964,  971,  979,  986,  993,
	1000, 1008, 1015, 1023, 1031, 1038, 1046, 1054, 1062, 1069, 1077, 1085, 1092, 1100, 1108, 1115,
	1123, 1131, 1138, 1146, 1154, 1162, 1169, 1177, 1185, 1192, 1200, 1207, 1214, 1221, 1229, 1236,
	1243, 1250, 1257, 1264, 1271, 1279, 1286, 1293, 1300, 1307, 1314, 1321, 1329, 1336, 1343, 1350,
	1357, 1364, 1371, 1379, 1386, 1393, 1400, 1407, 1414, 1421, 1429, 1436, 1443, 1450, 1457, 1464,
	1471, 1479, 1486, 1493, 1500, 1504, 1507, 1511, 1515, 1519, 1522, 1526, 1530, 1533, 1537, 1541,
	1544, 1548, 1552, 1556, 1559, 1563, 1567, 1570, 1574, 1578, 1581, 1585, 1589, 1593, 1596, 1600,
};


// hmm, what is the pixel clock? let's assume it's same as the 68040
// 54MHz(/4) or 16MHz would make HTOTAL unrealistically short
#define PIXEL_CLOCK         (10000000*2)

// VSync - 55.6795Hz
// HSync - 24.639kHz / 24.690kHz (may be inaccurate)
// TODO: why different HSyncs? 24 kHz assumes medium res monitor, so it can't be interlacing.
#define HTOTAL              (812)
#define HBEND               (0)
#define HBSTART             (512)

#define VTOTAL              (443)
#define VBEND               (0)
#define VBSTART             (400)


#define DSP_IDLESKIP        1 /* dsp idle skipping speedup hack */


void taitojc_state::coin_control_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/***************************************************************************

  Interrupts

***************************************************************************/

// dsp common ram has similar interrupt capability as MB8421
void taitojc_state::dsp_to_main_7fe_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dsp_shared_ram[0x7fe]);

	// shared ram interrupt request from dsp side
	if (ACCESSING_BITS_0_7)
		m_maincpu->set_input_line(6, ASSERT_LINE);
}

uint16_t taitojc_state::dsp_to_main_7fe_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_maincpu->set_input_line(6, CLEAR_LINE);

	return m_dsp_shared_ram[0x7fe];
}

void taitojc_state::main_to_dsp_7ff_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dsp_shared_ram[0x7ff]);

	if (ACCESSING_BITS_0_7)
	{
		// shared ram interrupt request from maincpu side
		// this is hacky, acquiring the internal dsp romdump should allow it to be cleaned up(?)
		if (data & 0x08)
		{
			m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			/*
			regarding m_has_dsp_hack:
			All games minus Dangerous Curves tests if the DSP is alive with this code snippet:

			0008C370: 4A79 1000 1FC0                                      tst.w   $10001fc0.l
			0008C376: 33FC 0000 0660 0000                                 move.w  #$0, $6600000.l
			0008C37E: 66F0                                                bne     $8c370

			Problem is: that move.w in the middle makes the SR to always return a zero flag result,
			hence it never branches like it should. CPU bug?
			*/
			if (!m_first_dsp_reset || !m_has_dsp_hack)
			{
				m_dsp->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			m_first_dsp_reset = 0;
		}
	}
}

void taitojc_state::cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffffff4, 0xfffffff5).lr16(NAME([] () -> u16 { return 0x82; }));
}

INTERRUPT_GEN_MEMBER(taitojc_state::taitojc_vblank)
{
	device.execute().set_input_line(2, HOLD_LINE); // where does it come from?
}

void taitojc_state::jc_irq_unk_w(uint8_t data)
{
	// gets written to at the end of irq6 routine
	// writes $02 or $06, depending on a value in DSP RAM, what does it mean?
}


/***************************************************************************

  maincpu I/O

***************************************************************************/

uint16_t taitojc_state::dsp_shared_r(offs_t offset)
{
	return m_dsp_shared_ram[offset];
}

void taitojc_state::dsp_shared_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dsp_shared_ram[offset]);
}


uint8_t taitojc_state::mcu_comm_r(offs_t offset)
{
	switch (offset)
	{
		case 0x03:
			return m_mcu_data_main;

		case 0x04:
			return m_mcu_comm_main | 0x14;

		default:
			logerror("mcu_comm_r: %02X at %08X\n", offset, m_maincpu->pc());
			break;
	}

	return 0;
}

void taitojc_state::mcu_comm_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:
			m_mcu_data_hc11 = data;
			m_mcu_comm_hc11 &= ~0x04;
			m_mcu_comm_main &= ~0x20;
			break;

		case 0x04:
			break;

		default:
			logerror("mcu_comm_w: %02X, %02X at %08X\n", offset, data, m_maincpu->pc());
			break;
	}
}


uint8_t taitojc_state::jc_pcbid_r(offs_t offset)
{
	static const char pcb_id[0x40] =
	{ "DEV=TC0870HVP   SYS=CG  VER=1.0"};
	// - any more data after "VER=1."?
	// - can we assume it comes from the TC0870HVP chip?

	return pcb_id[offset];
}


/*

Some games (Dangerous Curves, Side by Side, Side by Side 2) were released as Twin cabinets,
allowing 2 players to compete each other via a SMSC COM20020I network IC

Not emulated yet...

*/

uint8_t taitojc_state::jc_lan_r()
{
	return 0xff;
}

void taitojc_state::jc_lan_w(uint8_t data)
{
}


void taitojc_state::taitojc_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().mirror(0x200000);
	map(0x00400000, 0x01bfffff).rom().region("maingfx", 0);
	map(0x04000000, 0x040f7fff).ram().share("vram");
	map(0x040f8000, 0x040fbfff).rw(FUNC(taitojc_state::taitojc_tile_r), FUNC(taitojc_state::taitojc_tile_w));
	map(0x040fc000, 0x040fefff).rw(FUNC(taitojc_state::taitojc_char_r), FUNC(taitojc_state::taitojc_char_w));
	map(0x040ff000, 0x040fffff).ram().share("objlist");
	map(0x05800000, 0x0580003f).r(FUNC(taitojc_state::jc_pcbid_r));
	map(0x05900000, 0x05900007).rw(FUNC(taitojc_state::mcu_comm_r), FUNC(taitojc_state::mcu_comm_w));
	map(0x06400000, 0x0641ffff).rw(FUNC(taitojc_state::taitojc_palette_r), FUNC(taitojc_state::taitojc_palette_w)).share("palette_ram");
	map(0x06600000, 0x0660001f).rw(m_tc0640fio, FUNC(tc0640fio_device::read), FUNC(tc0640fio_device::write)).umask32(0xff000000);
	map(0x0660004c, 0x0660004f).portw("EEPROMOUT");
	map(0x06800001, 0x06800001).w(FUNC(taitojc_state::jc_irq_unk_w));
	map(0x06a00000, 0x06a01fff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)).umask32(0xff000000);
	map(0x06c00000, 0x06c0001f).rw(FUNC(taitojc_state::jc_lan_r), FUNC(taitojc_state::jc_lan_w)).umask32(0x00ff0000);
	map(0x08000000, 0x080fffff).ram().share("main_ram");
	map(0x10000000, 0x10001fff).rw(FUNC(taitojc_state::dsp_shared_r), FUNC(taitojc_state::dsp_shared_w)).umask32(0xffff0000);
	map(0x10001ff8, 0x10001ff9).r(FUNC(taitojc_state::dsp_to_main_7fe_r));
	map(0x10001ffc, 0x10001ffd).w(FUNC(taitojc_state::main_to_dsp_7ff_w));
}


/*

Densha de Go games have odometers for speed and brakepressure.
There's a voltmeter too, but seems to be a dummy (always stuck on 1.5kV)

The OKI is used for seat vibration effects.

*/

void taitojc_state::dendego_speedmeter_w(uint8_t data)
{
	if (m_speed_meter != dendego_odometer_table[data])
	{
		m_speed_meter = dendego_odometer_table[data];
		m_counters[2] = m_speed_meter / 10;
		m_counters[3] = m_speed_meter % 10;
	}
}

void taitojc_state::dendego_brakemeter_w(uint8_t data)
{
	if (m_brake_meter != dendego_pressure_table[data])
	{
		m_brake_meter = dendego_pressure_table[data];
		m_counters[4] = m_brake_meter;
	}
}

void taitojc_state::dendego_map(address_map &map)
{
	taitojc_map(map);
	map(0x06e00001, 0x06e00001).w(FUNC(taitojc_state::dendego_speedmeter_w));
	map(0x06e00005, 0x06e00005).w(FUNC(taitojc_state::dendego_brakemeter_w));
	map(0x06e0000d, 0x06e0000d).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}




/***************************************************************************

  MCU I/O

***************************************************************************/

uint8_t taitojc_state::hc11_comm_r()
{
	return m_mcu_comm_hc11;
}

void taitojc_state::hc11_comm_w(uint8_t data)
{
}

uint8_t taitojc_state::hc11_data_r()
{
	m_mcu_comm_hc11 |= 0x04;
	m_mcu_comm_main |= 0x20;
	return m_mcu_data_hc11;
}

void taitojc_state::hc11_data_w(uint8_t data)
{
	m_mcu_data_main = data;
}

void taitojc_state::hc11_output_w(uint8_t data)
{
/*
    cabinet lamps, active high

    dendego/dendego2:
    d0: START
    d1: DOOR
    d2: JYOUYO (normal)
    d3: HIJYOU (emergency)
    d4: DENSEI (three-stage power)
    d5: POP L
    d6: POP R
    d7: ------- (?)

    landgear:
    unused?

    sidebs/sidebs2:
    ?
*/
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

template <int Ch>
uint8_t taitojc_state::hc11_analog_r()
{
	return m_analog_ports[Ch].read_safe(0);
}


void taitojc_state::hc11_pgm_map(address_map &map)
{
	map(0x4000, 0x5fff).ram();
	map(0x8000, 0xffff).rom();
}




/***************************************************************************

  DSP I/O

***************************************************************************/

/*
    Math co-processor memory map

    0x7000: Projection point Y
    0x7001: Projection point X
    0x7002: Projection point Z
    0x7003: Frustum Min Z(?)
    0x7004: Frustum Max Z(?)
    0x7010: Line intersection, parameter length
    0x7011: Line intersection, intersection point
    0x7012: Line intersection, line length
    0x7013: Viewport Width / 2
    0x7014: Viewport Height / 2
    0x7015: Viewport Z / 2 (?)
    0x701b: Line intersection, parameter interpolation read
    0x701d: Projected point Y read
    0x701f: Projected point X read
    0x7022: Unknown read
    0x7030: Unknown write
    0x703b: Unknown read/write
*/

void taitojc_state::dsp_math_projection_w(offs_t offset, uint16_t data)
{
	m_projection_data[offset] = data;
}

void taitojc_state::dsp_math_viewport_w(offs_t offset, uint16_t data)
{
	m_viewport_data[offset] = data;
}

uint16_t taitojc_state::dsp_math_projection_y_r()
{
	return (m_projection_data[2] != 0) ? (m_projection_data[0] * m_viewport_data[0]) / m_projection_data[2] : 0;
}

uint16_t taitojc_state::dsp_math_projection_x_r()
{
	return (m_projection_data[2] != 0) ? (m_projection_data[1] * m_viewport_data[1]) / m_projection_data[2] : 0;
}

void taitojc_state::dsp_math_intersection_w(offs_t offset, uint16_t data)
{
	m_intersection_data[offset] = data;
}

uint16_t taitojc_state::dsp_math_intersection_r()
{
	return (m_intersection_data[2] != 0) ? (m_intersection_data[0] * m_intersection_data[1]) / m_intersection_data[2] : 0;
}

uint16_t taitojc_state::dsp_math_unk_r()
{
	return 0x7fff;
}


/**************************************************************************/

uint16_t taitojc_state::dsp_rom_r()
{
	assert (m_dsp_rom_pos < 0x800000); // never happens
	return m_dspgfx[machine().side_effects_disabled() ? m_dsp_rom_pos : m_dsp_rom_pos++];
}

void taitojc_state::dsp_rom_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_dsp_rom_pos &= 0xffff;
		m_dsp_rom_pos |= data << 16;
	}
	else
	{
		m_dsp_rom_pos &= 0xffff0000;
		m_dsp_rom_pos |= data;
	}
}

void taitojc_state::tms_program_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().mirror(0x4000);
	map(0x6000, 0x7fff).ram();
}

void taitojc_state::tms_data_map(address_map &map)
{
	map(0x6a01, 0x6a02).w(m_tc0780fpa, FUNC(tc0780fpa_device::render_w));
	map(0x6a11, 0x6a12).noprw();     // same as 0x6a01..02 for the second renderer chip?
	map(0x6b20, 0x6b20).w(m_tc0780fpa, FUNC(tc0780fpa_device::poly_fifo_w));
	map(0x6b22, 0x6b22).w(m_tc0780fpa, FUNC(tc0780fpa_device::tex_w));
	map(0x6b23, 0x6b23).rw(m_tc0780fpa, FUNC(tc0780fpa_device::tex_addr_r), FUNC(tc0780fpa_device::tex_addr_w));
	map(0x6c00, 0x6c01).rw(FUNC(taitojc_state::dsp_rom_r), FUNC(taitojc_state::dsp_rom_w));
	map(0x7000, 0x7002).w(FUNC(taitojc_state::dsp_math_projection_w));
	map(0x7010, 0x7012).w(FUNC(taitojc_state::dsp_math_intersection_w));
	map(0x7013, 0x7015).w(FUNC(taitojc_state::dsp_math_viewport_w));
	map(0x701b, 0x701b).r(FUNC(taitojc_state::dsp_math_intersection_r));
	map(0x701d, 0x701d).r(FUNC(taitojc_state::dsp_math_projection_y_r));
	map(0x701f, 0x701f).r(FUNC(taitojc_state::dsp_math_projection_x_r));
	map(0x7022, 0x7022).r(FUNC(taitojc_state::dsp_math_unk_r));
	map(0x7800, 0x7fff).ram().share("dsp_shared");
	map(0x7ffe, 0x7ffe).w(FUNC(taitojc_state::dsp_to_main_7fe_w));
	map(0x8000, 0xffff).ram();
}




/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( common )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_DIPNAME( 0x02, 0x02, "Dev Skip RAM Test" ) // skips mainram test on page 1 of POST
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, "Dev Debug" ) // debug related in dendego/sidebs
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

// Mascon must always be in a defined state, Densha de Go 2 in particular returns black screen if the Mascon input is undefined
static const ioport_value dendego_mascon_table[6] = { 0x76, 0x67, 0x75, 0x57, 0x73, 0x37 };

static INPUT_PORTS_START( dendego )
	PORT_INCLUDE( common )

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Horn Pedal")

	PORT_MODIFY("BUTTONS")  // Throttle Lever at left, move down to speed up, 6 positions
	PORT_BIT( 0x77, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(6) PORT_REMAP_TABLE(dendego_mascon_table) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_NAME("Throttle Lever")
	PORT_BIT( 0x88, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN.0")   // Brake Lever at right, rotate handle right (anti clockwise) to increase pressure, 11 positions but not at constant intervals like the throttle lever
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xef) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_NAME("Brake Lever")
INPUT_PORTS_END

static INPUT_PORTS_START( landgear )
	PORT_INCLUDE( common )

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("View Switch")

	PORT_START("AN.0")       // Lever X
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("AN.1")       // Lever Y
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("AN.2")       // Throttle
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( sidebs )
	PORT_INCLUDE( common )

	PORT_MODIFY("START")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // no start button

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("View Switch")

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Shift Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_NAME("Shift Down")

	PORT_START("AN.0")       // Steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel")

	PORT_START("AN.1")       // Acceleration
	PORT_BIT( 0xff, 0x00, IPT_PEDAL )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("Gas Pedal")

	PORT_START("AN.2")       // Brake
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("Brake Pedal")
INPUT_PORTS_END

static INPUT_PORTS_START( dangcurv )
	PORT_INCLUDE( common )

	PORT_MODIFY("START")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // no start button

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("View Switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Rear Switch")

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Shift Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_NAME("Shift Down")

	PORT_START("AN.0")       // Steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_REVERSE PORT_NAME("Steering Wheel")

	PORT_START("AN.1")       // Acceleration
	PORT_BIT( 0xff, 0x00, IPT_PEDAL )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("Gas Pedal")

	PORT_START("AN.2")       // Brake
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("Brake Pedal")
INPUT_PORTS_END




/***************************************************************************

  Machine Config

***************************************************************************/

void taitojc_state::machine_reset()
{
	m_first_dsp_reset = 1;

	m_mcu_comm_main = 0;
	m_mcu_comm_hc11 = 0;
	m_mcu_data_main = 0;
	m_mcu_data_hc11 = 0;

	m_dsp_rom_pos = 0;

	memset(m_viewport_data, 0, sizeof(m_viewport_data));
	memset(m_projection_data, 0, sizeof(m_projection_data));
	memset(m_intersection_data, 0, sizeof(m_intersection_data));

	// hold the TMS in reset until we have code
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void taitojc_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_dsp_rom_pos));
	save_item(NAME(m_first_dsp_reset));
	save_item(NAME(m_viewport_data));
	save_item(NAME(m_projection_data));
	save_item(NAME(m_intersection_data));
	save_item(NAME(m_gfx_index));

	save_item(NAME(m_mcu_comm_main));
	save_item(NAME(m_mcu_comm_hc11));
	save_item(NAME(m_mcu_data_main));
	save_item(NAME(m_mcu_data_hc11));

	save_item(NAME(m_speed_meter));
	save_item(NAME(m_brake_meter));

	m_lamps.resolve();
	m_counters.resolve();
}


void taitojc_state::taitojc(machine_config &config)
{
	/* basic machine hardware */
	M68040(config, m_maincpu, XTAL(10'000'000)*2); // 20MHz, clock source = CY7C991
	m_maincpu->set_addrmap(AS_PROGRAM, &taitojc_state::taitojc_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitojc_state::taitojc_vblank));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &taitojc_state::cpu_space_map);

	mc68hc11_cpu_device &sub(MC68HC11M0(config, "sub", XTAL(16'000'000)/2));
	sub.set_addrmap(AS_PROGRAM, &taitojc_state::hc11_pgm_map);
	sub.in_pa_callback().set_constant(0); // ?
	sub.in_pg_callback().set(FUNC(taitojc_state::hc11_comm_r));
	sub.out_pg_callback().set(FUNC(taitojc_state::hc11_comm_w));
	sub.out_ph_callback().set(FUNC(taitojc_state::hc11_output_w));
	sub.in_spi2_data_callback().set(FUNC(taitojc_state::hc11_data_r));
	sub.out_spi2_data_callback().set(FUNC(taitojc_state::hc11_data_w));
	sub.in_an0_callback().set(FUNC(taitojc_state::hc11_analog_r<0>));
	sub.in_an1_callback().set(FUNC(taitojc_state::hc11_analog_r<1>));
	sub.in_an2_callback().set(FUNC(taitojc_state::hc11_analog_r<2>));
	sub.in_an3_callback().set(FUNC(taitojc_state::hc11_analog_r<3>));
	sub.in_an4_callback().set(FUNC(taitojc_state::hc11_analog_r<4>));
	sub.in_an5_callback().set(FUNC(taitojc_state::hc11_analog_r<5>));
	sub.in_an6_callback().set(FUNC(taitojc_state::hc11_analog_r<6>));
	sub.in_an7_callback().set(FUNC(taitojc_state::hc11_analog_r<7>));

	TMS32051(config, m_dsp, XTAL(10'000'000)*4); // 40MHz, clock source = CY7C991
	m_dsp->set_addrmap(AS_PROGRAM, &taitojc_state::tms_program_map);
	m_dsp->set_addrmap(AS_DATA, &taitojc_state::tms_data_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	EEPROM_93C46_16BIT(config, "eeprom");

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_0_callback().set_ioport("SERVICE");
	m_tc0640fio->read_1_callback().set_ioport("COINS");
	m_tc0640fio->read_2_callback().set_ioport("START");
	m_tc0640fio->read_3_callback().set_ioport("UNUSED");
	m_tc0640fio->write_4_callback().set(FUNC(taitojc_state::coin_control_w));
	m_tc0640fio->read_7_callback().set_ioport("BUTTONS");

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(taitojc_state::screen_update_taitojc));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(32768);

	TC0780FPA(config, m_tc0780fpa, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	taito_en_device &taito_en(TAITO_EN(config, "taito_en", 0));
	taito_en.add_route(0, "lspeaker", 1.0);
	taito_en.add_route(1, "rspeaker", 1.0);
}

void taitojc_state::dendego(machine_config &config)
{
	taitojc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitojc_state::dendego_map);

	/* video hardware */
	m_screen->set_screen_update(FUNC(taitojc_state::screen_update_dendego));

	/* sound hardware */
	SPEAKER(config, "vibration").seat();

	/* clock frequency & pin 7 not verified */
	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "vibration", 0.20);
}




/***************************************************************************

  Game Drivers

***************************************************************************/

uint16_t taitojc_state::taitojc_dsp_idle_skip_r()
{
	if (m_dsp->pc() == 0x404c)
		m_dsp->spin_until_time(attotime::from_usec(500));

	return m_dsp_shared_ram[0x7f0];
}

uint16_t taitojc_state::dendego2_dsp_idle_skip_r()
{
	if (m_dsp->pc() == 0x402e)
		m_dsp->spin_until_time(attotime::from_usec(500));

	return m_dsp_shared_ram[0x7f0];
}


void taitojc_state::init_taitojc()
{
	m_has_dsp_hack = 1;

	if (DSP_IDLESKIP)
		m_dsp->space(AS_DATA).install_read_handler(0x7ff0, 0x7ff0, read16smo_delegate(*this, FUNC(taitojc_state::taitojc_dsp_idle_skip_r)));
}

void taitojc_state::init_dendego2()
{
	init_taitojc();

	if (DSP_IDLESKIP)
		m_dsp->space(AS_DATA).install_read_handler(0x7ff0, 0x7ff0, read16smo_delegate(*this, FUNC(taitojc_state::dendego2_dsp_idle_skip_r)));
}

void taitojc_state::init_dangcurv()
{
	init_taitojc();

	m_has_dsp_hack = 0;
}


/**************************************************************************/

ROM_START( sidebs ) /* Side by Side ver 3.0 OK */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e23-43-1.ic36", 0x000000, 0x80000, CRC(1b87991a) SHA1(8ab4bcfb822906996e7dd24b7fd0def870e6f5f1) )
	ROM_LOAD32_BYTE( "e23-44-1.ic37", 0x000001, 0x80000, CRC(bc85493e) SHA1(d4ac7de5b72a450dcea3a9983f9761f715cc8f62) )
	ROM_LOAD32_BYTE( "e23-45-1.ic38", 0x000002, 0x80000, CRC(9920d89f) SHA1(b3f0f0f7d532a506f4018a10a4b98b8719c97e6b) )
	ROM_LOAD32_BYTE( "e23-46-1.ic39", 0x000003, 0x80000, CRC(354f6816) SHA1(0bfdc11b71d6a92fe40449e020b923229736bcfa) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e23-23.ic30", 0x100001, 0x40000, CRC(cffbffe5) SHA1(c01ac44390dacab4b49bb066a46d81a184b07a1e) )
	ROM_LOAD16_BYTE( "e23-24.ic31", 0x100000, 0x40000, CRC(64bae246) SHA1(f929f664881487615b1259db43a0721135830274) )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e23-05.ic9",   0x0800000, 0x200000, CRC(6e5d11ec) SHA1(e5c39d80577bf8ae9fc6162dc54571c6c8421160) )
	ROM_LOAD32_WORD_SWAP( "e23-12.ic22",  0x0800002, 0x200000, CRC(7365333c) SHA1(4f7b75088799ea37f714bc7e5c5b276a7e5d933f) )
	ROM_LOAD32_WORD_SWAP( "e23-06.ic10",  0x0c00000, 0x200000, CRC(ffcfd153) SHA1(65fa486cf0156e2988bd6e7060d66f87f765a123) )
	ROM_LOAD32_WORD_SWAP( "e23-13.ic23",  0x0c00002, 0x200000, CRC(16982d37) SHA1(134370f7dfadb1886f1e5e5dd16f8b72ad08fc68) )
	ROM_FILL(                        0x1000000, 0x400000, 0 )
	ROM_LOAD32_WORD_SWAP( "e23-07.ic12",  0x1400000, 0x200000, CRC(90f2a87c) SHA1(770bb89fa42cb2a1d5a58525b8d72ed7df3f93ed) )
	ROM_LOAD32_WORD_SWAP( "e23-14.ic25",  0x1400002, 0x200000, CRC(1bc5a914) SHA1(92f82a4e2fbac73dbb3293726fc09022bd11a8fe) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e23-01.ic5",   0x0000000, 0x200000, CRC(2cbe4bbd) SHA1(ed6fe4344c86d50914b5ddbc720dd15544f4d07f) )
	ROM_LOAD( "e23-02.ic6",   0x0200000, 0x200000, CRC(7ebada03) SHA1(d75c992aa33dd7f71de6a6d09aac471012b0daa3) )
	ROM_LOAD( "e23-03.ic7",   0x0400000, 0x200000, CRC(5bf1f30b) SHA1(6e0c07b9f92962eec55ee444732a10ac78f8b050) )
	ROM_LOAD( "e23-04.ic8",   0x0600000, 0x200000, CRC(0f860fb5) SHA1(47c0db4ec6d02e10d8abfacd1fa332f7af3976dd) )
	ROM_LOAD( "e23-08.ic18",  0x0800000, 0x200000, CRC(bceea63e) SHA1(eeec1e2306aa37431c5ba69bdb9c5524ab7b7ba4) )
	ROM_LOAD( "e23-09.ic19",  0x0a00000, 0x200000, CRC(3917c12f) SHA1(e3a568f638bb6b0cd6237c9fee5fc350983ea1e7) )
	ROM_LOAD( "e23-10.ic20",  0x0c00000, 0x200000, CRC(038370d9) SHA1(c02f68a25121d2d5aae62c419522b25cd6ec32b6) )
	ROM_LOAD( "e23-11.ic21",  0x0e00000, 0x200000, CRC(91fab03d) SHA1(1865d8b679faa6f2b3c14db2c6c461c00afd547c) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32",  0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) )
	ROM_LOAD16_BYTE( "e23-16.ic33",  0x400000, 0x200000, CRC(1d63d785) SHA1(0cf74bb433e9c453e35f7a552fdf9e22084b2f49) )
	ROM_LOAD16_BYTE( "e23-17.ic34",  0x800000, 0x200000, CRC(1c54021a) SHA1(a1efbdb02d23a5d32ebd25cb8e99dface3178ebd) )
	ROM_LOAD16_BYTE( "e23-18.ic35",  0xc00000, 0x200000, CRC(1816f38a) SHA1(6451bdb4b4297aaf4987451bc0ddd97b0072e113) )
ROM_END

ROM_START( sidebsj ) /* Side by Side ver 2.7 J */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e23-47.ic36", 0x000000, 0x80000, CRC(c67dc173) SHA1(fb16f75683b42cafc28c50d6c823d0987b09cd00) )
	ROM_LOAD32_BYTE( "e23-48.ic37", 0x000001, 0x80000, CRC(e3da7652) SHA1(a872e8d26e3e376786762f2571f75f282e2481f1) )
	ROM_LOAD32_BYTE( "e23-49.ic38", 0x000002, 0x80000, CRC(b2fdded3) SHA1(8e0f2d2d967a2e6b7d1954548941bc6257799d2d) )
	ROM_LOAD32_BYTE( "e23-50.ic39", 0x000003, 0x80000, CRC(76510731) SHA1(b8b9836fa121d2028e0218f4d439af0d7dae295e) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e23-23.ic30", 0x100001, 0x40000, CRC(cffbffe5) SHA1(c01ac44390dacab4b49bb066a46d81a184b07a1e) )
	ROM_LOAD16_BYTE( "e23-24.ic31", 0x100000, 0x40000, CRC(64bae246) SHA1(f929f664881487615b1259db43a0721135830274) )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e23-05.ic9",   0x0800000, 0x200000, CRC(6e5d11ec) SHA1(e5c39d80577bf8ae9fc6162dc54571c6c8421160) )
	ROM_LOAD32_WORD_SWAP( "e23-12.ic22",  0x0800002, 0x200000, CRC(7365333c) SHA1(4f7b75088799ea37f714bc7e5c5b276a7e5d933f) )
	ROM_LOAD32_WORD_SWAP( "e23-06.ic10",  0x0c00000, 0x200000, CRC(ffcfd153) SHA1(65fa486cf0156e2988bd6e7060d66f87f765a123) )
	ROM_LOAD32_WORD_SWAP( "e23-13.ic23",  0x0c00002, 0x200000, CRC(16982d37) SHA1(134370f7dfadb1886f1e5e5dd16f8b72ad08fc68) )
	ROM_FILL(                        0x1000000, 0x400000, 0 )
	ROM_LOAD32_WORD_SWAP( "e23-07.ic12",  0x1400000, 0x200000, CRC(90f2a87c) SHA1(770bb89fa42cb2a1d5a58525b8d72ed7df3f93ed) )
	ROM_LOAD32_WORD_SWAP( "e23-14.ic25",  0x1400002, 0x200000, CRC(1bc5a914) SHA1(92f82a4e2fbac73dbb3293726fc09022bd11a8fe) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e23-01.ic5",   0x0000000, 0x200000, CRC(2cbe4bbd) SHA1(ed6fe4344c86d50914b5ddbc720dd15544f4d07f) )
	ROM_LOAD( "e23-02.ic6",   0x0200000, 0x200000, CRC(7ebada03) SHA1(d75c992aa33dd7f71de6a6d09aac471012b0daa3) )
	ROM_LOAD( "e23-03.ic7",   0x0400000, 0x200000, CRC(5bf1f30b) SHA1(6e0c07b9f92962eec55ee444732a10ac78f8b050) )
	ROM_LOAD( "e23-04.ic8",   0x0600000, 0x200000, CRC(0f860fb5) SHA1(47c0db4ec6d02e10d8abfacd1fa332f7af3976dd) )
	ROM_LOAD( "e23-08.ic18",  0x0800000, 0x200000, CRC(bceea63e) SHA1(eeec1e2306aa37431c5ba69bdb9c5524ab7b7ba4) )
	ROM_LOAD( "e23-09.ic19",  0x0a00000, 0x200000, CRC(3917c12f) SHA1(e3a568f638bb6b0cd6237c9fee5fc350983ea1e7) )
	ROM_LOAD( "e23-10.ic20",  0x0c00000, 0x200000, CRC(038370d9) SHA1(c02f68a25121d2d5aae62c419522b25cd6ec32b6) )
	ROM_LOAD( "e23-11.ic21",  0x0e00000, 0x200000, CRC(91fab03d) SHA1(1865d8b679faa6f2b3c14db2c6c461c00afd547c) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32",  0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) )
	ROM_LOAD16_BYTE( "e23-16.ic33",  0x400000, 0x200000, CRC(1d63d785) SHA1(0cf74bb433e9c453e35f7a552fdf9e22084b2f49) )
	ROM_LOAD16_BYTE( "e23-17.ic34",  0x800000, 0x200000, CRC(1c54021a) SHA1(a1efbdb02d23a5d32ebd25cb8e99dface3178ebd) )
	ROM_LOAD16_BYTE( "e23-18.ic35",  0xc00000, 0x200000, CRC(1816f38a) SHA1(6451bdb4b4297aaf4987451bc0ddd97b0072e113) )
ROM_END

ROM_START( sidebsja ) /* Side by Side ver 2.6 J */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e23-19-1.ic36", 0x000000, 0x80000, CRC(ac84f553) SHA1(e050f02a1149edc2f4f5f3892ba085632e4e4602) )
	ROM_LOAD32_BYTE( "e23-20-1.ic37", 0x000001, 0x80000, CRC(603d9d4f) SHA1(4bfb5a351bd4daed024747f0e49faff69f6a6a4a) )
	ROM_LOAD32_BYTE( "e23-21-1.ic38", 0x000002, 0x80000, CRC(9393c9fc) SHA1(2ce12552c6f81e50e94968f89ccdbd3975a62fdf) )
	ROM_LOAD32_BYTE( "e23-22-1.ic39", 0x000003, 0x80000, CRC(8698c9f2) SHA1(97950da8c98572db6379d0c520927ed45b18956f) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e23-23.ic30", 0x100001, 0x40000, CRC(cffbffe5) SHA1(c01ac44390dacab4b49bb066a46d81a184b07a1e) )
	ROM_LOAD16_BYTE( "e23-24.ic31", 0x100000, 0x40000, CRC(64bae246) SHA1(f929f664881487615b1259db43a0721135830274) )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e23-05.ic9",   0x0800000, 0x200000, CRC(6e5d11ec) SHA1(e5c39d80577bf8ae9fc6162dc54571c6c8421160) )
	ROM_LOAD32_WORD_SWAP( "e23-12.ic22",  0x0800002, 0x200000, CRC(7365333c) SHA1(4f7b75088799ea37f714bc7e5c5b276a7e5d933f) )
	ROM_LOAD32_WORD_SWAP( "e23-06.ic10",  0x0c00000, 0x200000, CRC(ffcfd153) SHA1(65fa486cf0156e2988bd6e7060d66f87f765a123) )
	ROM_LOAD32_WORD_SWAP( "e23-13.ic23",  0x0c00002, 0x200000, CRC(16982d37) SHA1(134370f7dfadb1886f1e5e5dd16f8b72ad08fc68) )
	ROM_FILL(                        0x1000000, 0x400000, 0 )
	ROM_LOAD32_WORD_SWAP( "e23-07.ic12",  0x1400000, 0x200000, CRC(90f2a87c) SHA1(770bb89fa42cb2a1d5a58525b8d72ed7df3f93ed) )
	ROM_LOAD32_WORD_SWAP( "e23-14.ic25",  0x1400002, 0x200000, CRC(1bc5a914) SHA1(92f82a4e2fbac73dbb3293726fc09022bd11a8fe) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e23-01.ic5",   0x0000000, 0x200000, CRC(2cbe4bbd) SHA1(ed6fe4344c86d50914b5ddbc720dd15544f4d07f) )
	ROM_LOAD( "e23-02.ic6",   0x0200000, 0x200000, CRC(7ebada03) SHA1(d75c992aa33dd7f71de6a6d09aac471012b0daa3) )
	ROM_LOAD( "e23-03.ic7",   0x0400000, 0x200000, CRC(5bf1f30b) SHA1(6e0c07b9f92962eec55ee444732a10ac78f8b050) )
	ROM_LOAD( "e23-04.ic8",   0x0600000, 0x200000, CRC(0f860fb5) SHA1(47c0db4ec6d02e10d8abfacd1fa332f7af3976dd) )
	ROM_LOAD( "e23-08.ic18",  0x0800000, 0x200000, CRC(bceea63e) SHA1(eeec1e2306aa37431c5ba69bdb9c5524ab7b7ba4) )
	ROM_LOAD( "e23-09.ic19",  0x0a00000, 0x200000, CRC(3917c12f) SHA1(e3a568f638bb6b0cd6237c9fee5fc350983ea1e7) )
	ROM_LOAD( "e23-10.ic20",  0x0c00000, 0x200000, CRC(038370d9) SHA1(c02f68a25121d2d5aae62c419522b25cd6ec32b6) )
	ROM_LOAD( "e23-11.ic21",  0x0e00000, 0x200000, CRC(91fab03d) SHA1(1865d8b679faa6f2b3c14db2c6c461c00afd547c) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32",  0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) )
	ROM_LOAD16_BYTE( "e23-16.ic33",  0x400000, 0x200000, CRC(1d63d785) SHA1(0cf74bb433e9c453e35f7a552fdf9e22084b2f49) )
	ROM_LOAD16_BYTE( "e23-17.ic34",  0x800000, 0x200000, CRC(1c54021a) SHA1(a1efbdb02d23a5d32ebd25cb8e99dface3178ebd) )
	ROM_LOAD16_BYTE( "e23-18.ic35",  0xc00000, 0x200000, CRC(1816f38a) SHA1(6451bdb4b4297aaf4987451bc0ddd97b0072e113) )
ROM_END

ROM_START( sidebsjb ) /* Side by Side ver 2.5 J */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e23-19.ic36", 0x000000, 0x80000, CRC(7b75481b) SHA1(47332e045f92b31e4f35c38e6880a7287b9a5c2c) )
	ROM_LOAD32_BYTE( "e23-20.ic37", 0x000001, 0x80000, CRC(cbd857dd) SHA1(ae33ad8b0c3559a3a9096351e9aa07782d3cb841) )
	ROM_LOAD32_BYTE( "e23-21.ic38", 0x000002, 0x80000, CRC(357f2e10) SHA1(226922f2649d9ac78d253200f5bbff4fb3ac74c8) )
	ROM_LOAD32_BYTE( "e23-22.ic39", 0x000003, 0x80000, CRC(c793ba43) SHA1(0ddbf625320968b4e18309d8e732ce4a2b9f4bce) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e23-23.ic30", 0x100001, 0x40000, CRC(cffbffe5) SHA1(c01ac44390dacab4b49bb066a46d81a184b07a1e) )
	ROM_LOAD16_BYTE( "e23-24.ic31", 0x100000, 0x40000, CRC(64bae246) SHA1(f929f664881487615b1259db43a0721135830274) )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e23-05.ic9",   0x0800000, 0x200000, CRC(6e5d11ec) SHA1(e5c39d80577bf8ae9fc6162dc54571c6c8421160) )
	ROM_LOAD32_WORD_SWAP( "e23-12.ic22",  0x0800002, 0x200000, CRC(7365333c) SHA1(4f7b75088799ea37f714bc7e5c5b276a7e5d933f) )
	ROM_LOAD32_WORD_SWAP( "e23-06.ic10",  0x0c00000, 0x200000, CRC(ffcfd153) SHA1(65fa486cf0156e2988bd6e7060d66f87f765a123) )
	ROM_LOAD32_WORD_SWAP( "e23-13.ic23",  0x0c00002, 0x200000, CRC(16982d37) SHA1(134370f7dfadb1886f1e5e5dd16f8b72ad08fc68) )
	ROM_FILL(                        0x1000000, 0x400000, 0 )
	ROM_LOAD32_WORD_SWAP( "e23-07.ic12",  0x1400000, 0x200000, CRC(90f2a87c) SHA1(770bb89fa42cb2a1d5a58525b8d72ed7df3f93ed) )
	ROM_LOAD32_WORD_SWAP( "e23-14.ic25",  0x1400002, 0x200000, CRC(1bc5a914) SHA1(92f82a4e2fbac73dbb3293726fc09022bd11a8fe) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e23-01.ic5",   0x0000000, 0x200000, CRC(2cbe4bbd) SHA1(ed6fe4344c86d50914b5ddbc720dd15544f4d07f) )
	ROM_LOAD( "e23-02.ic6",   0x0200000, 0x200000, CRC(7ebada03) SHA1(d75c992aa33dd7f71de6a6d09aac471012b0daa3) )
	ROM_LOAD( "e23-03.ic7",   0x0400000, 0x200000, CRC(5bf1f30b) SHA1(6e0c07b9f92962eec55ee444732a10ac78f8b050) )
	ROM_LOAD( "e23-04.ic8",   0x0600000, 0x200000, CRC(0f860fb5) SHA1(47c0db4ec6d02e10d8abfacd1fa332f7af3976dd) )
	ROM_LOAD( "e23-08.ic18",  0x0800000, 0x200000, CRC(bceea63e) SHA1(eeec1e2306aa37431c5ba69bdb9c5524ab7b7ba4) )
	ROM_LOAD( "e23-09.ic19",  0x0a00000, 0x200000, CRC(3917c12f) SHA1(e3a568f638bb6b0cd6237c9fee5fc350983ea1e7) )
	ROM_LOAD( "e23-10.ic20",  0x0c00000, 0x200000, CRC(038370d9) SHA1(c02f68a25121d2d5aae62c419522b25cd6ec32b6) )
	ROM_LOAD( "e23-11.ic21",  0x0e00000, 0x200000, CRC(91fab03d) SHA1(1865d8b679faa6f2b3c14db2c6c461c00afd547c) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32",  0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) )
	ROM_LOAD16_BYTE( "e23-16.ic33",  0x400000, 0x200000, CRC(1d63d785) SHA1(0cf74bb433e9c453e35f7a552fdf9e22084b2f49) )
	ROM_LOAD16_BYTE( "e23-17.ic34",  0x800000, 0x200000, CRC(1c54021a) SHA1(a1efbdb02d23a5d32ebd25cb8e99dface3178ebd) )
	ROM_LOAD16_BYTE( "e23-18.ic35",  0xc00000, 0x200000, CRC(1816f38a) SHA1(6451bdb4b4297aaf4987451bc0ddd97b0072e113) )
ROM_END

ROM_START( sidebs2 ) /* Side by Side 2 Ver 2.6 OK */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "sbs2_u36.ic36", 0x000000, 0x80000, CRC(889bc5e2) SHA1(9d7c33b3097c956271e7ac6f9c38eb27190cce34) ) /* Need to identify the correct Taito E38-xx ID number for these 4 roms */
	ROM_LOAD32_BYTE( "sbs2_u37.ic37", 0x000001, 0x80000, CRC(19fd895a) SHA1(387ad735a2a2f381d77057123eb32f6cbdb0613b) )
	ROM_LOAD32_BYTE( "sbs2_u38.ic38", 0x000002, 0x80000, CRC(9bc3db02) SHA1(22e53fe073fc4632a31da1cb90708e7232e7e05b) )
	ROM_LOAD32_BYTE( "sbs2_u39.ic39", 0x000003, 0x80000, CRC(1d822e54) SHA1(ab77910c250cdefb747c22d798e5bdfaf6962f4c) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e38-19.ic30",  0x100001, 0x40000, CRC(3f50cb7b) SHA1(76af65c9b74ede843a3182f79cecda8c3e3febe6) )
	ROM_LOAD16_BYTE( "e38-20.ic31",  0x100000, 0x40000, CRC(d01340e7) SHA1(76ee48d644dc1ec415d47e0df4864c64ac928b9d) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e38-05.ic9",  0x0800000, 0x200000, CRC(bda366bf) SHA1(a7558e6d5e4583a2d8e3d2bfa8cabcc459d3ee83) )
	ROM_LOAD32_WORD_SWAP( "e38-13.ic22", 0x0800002, 0x200000, CRC(1bd7582b) SHA1(35763b9489f995433f66ef72d4f6b6ac67df5480) )
	ROM_LOAD32_WORD_SWAP( "e38-06.ic10", 0x0c00000, 0x200000, CRC(308d2783) SHA1(22c309273444bc6c1df78069850958a739664998) )
	ROM_LOAD32_WORD_SWAP( "e38-14.ic23", 0x0c00002, 0x200000, CRC(f1699f27) SHA1(3c9a9cefe6f215fd9f0a9746da97147d14df1da4) )
	ROM_LOAD32_WORD_SWAP( "e38-07.ic11", 0x1000000, 0x200000, CRC(226ba93d) SHA1(98e6342d070ddd988c1e9bff21afcfb28df86844) )
	ROM_LOAD32_WORD_SWAP( "e38-15.ic24", 0x1000002, 0x200000, CRC(2853c2e3) SHA1(046dbbd4bcb3b07cddab19a284fee9fe736f8def) )
	ROM_LOAD32_WORD_SWAP( "e38-08.ic12", 0x1400000, 0x200000, CRC(9c513b32) SHA1(fe26e39d3d65073d23d525bc17771f0c244a38c2) )
	ROM_LOAD32_WORD_SWAP( "e38-16.ic25", 0x1400002, 0x200000, CRC(fceafae2) SHA1(540ecd5d1aa64c0428a08ea1e4e634e00f7e6bd6) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e38-01.ic5",  0x0000000, 0x200000, CRC(a3c2e2c7) SHA1(538208534f996782167e4cf0d157ad93ce2937bd) )
	ROM_LOAD( "e38-02.ic6",  0x0200000, 0x200000, CRC(ecdfb75a) SHA1(85e7afa321846816fa3bd9074ad9dec95abe23fe) )
	ROM_LOAD( "e38-03.ic7",  0x0400000, 0x200000, CRC(28e9cb59) SHA1(a2651fd81a1263573f868864ee049f8fc4177ffa) )
	ROM_LOAD( "e38-04.ic8",  0x0600000, 0x080000, CRC(26cab05b) SHA1(d5bcf021646dade834840051e0ce083319c53985) )
	ROM_RELOAD(              0x0680000, 0x080000 )
	ROM_RELOAD(              0x0700000, 0x080000 )
	ROM_RELOAD(              0x0780000, 0x080000 )
	ROM_LOAD( "e38-09.ic18", 0x0800000, 0x200000, CRC(03c95a7f) SHA1(fc046cf5fcfcf5648f68af4bed78576f6d67b32f) )
	ROM_LOAD( "e38-10.ic19", 0x0a00000, 0x200000, CRC(0fb06794) SHA1(2d0e3b07ded698235572fe9e907a84d5779ac2c5) )
	ROM_LOAD( "e38-11.ic20", 0x0c00000, 0x200000, CRC(6a312351) SHA1(8037e377f8eef4cc6dd84aec9c829106f0bb130c) )
	ROM_LOAD( "e38-12.ic21", 0x0e00000, 0x080000, CRC(193a5774) SHA1(aa017ae4fec92bb87c0eb94f59d093853ddbabc9) )
	ROM_RELOAD(              0x0e80000, 0x080000 )
	ROM_RELOAD(              0x0f00000, 0x080000 )
	ROM_RELOAD(              0x0f80000, 0x080000 )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32", 0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) ) // from sidebs (redump)
	ROM_LOAD16_BYTE( "e38-17.ic33", 0x400000, 0x200000, CRC(61e81c7f) SHA1(aa650bc139685ad456c233b79aa60005a8fd6d79) )
	ROM_LOAD16_BYTE( "e38-18.ic34", 0x800000, 0x200000, CRC(43e2f149) SHA1(32ea9156948a886dda5bd071e9f493ddc2b06212) )
	ROM_LOAD16_BYTE( "e38-21.ic35", 0xc00000, 0x200000, CRC(25373c5f) SHA1(ab9f917dbde7c808be2cd836ce2d3fc558e290f1) )

	/* PALS
	e23-28.ic18    NOT A ROM
	e23-27.ic13    NOT A ROM
	e23-26.ic4     NOT A ROM
	e23-25-1.ic3   NOT A ROM
	e23-30.ic40    NOT A ROM
	e23-29.ic39    NOT A ROM
	e23-31.ic46    NOT A ROM
	e23-32-1.ic51  NOT A ROM
	e23-34.ic72    NOT A ROM
	e23-33.ic53    NOT A ROM
	e23-35.ic110   NOT A ROM
	e23-38.ic73    NOT A ROM
	e23-37.ic69    NOT A ROM
	*/
ROM_END

ROM_START( sidebs2u ) /* Side by Side 2 Ver 2.6 A */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "sbs2_p0.ic36", 0x000000, 0x80000, CRC(2dd78d09) SHA1(f0a0105c3f2827c8b55d1bc58ebeea0f71150fed) ) /* Need to identify the correct Taito E38-xx ID number for these 4 roms */
	ROM_LOAD32_BYTE( "sbs2_p1.ic37", 0x000001, 0x80000, CRC(befeda1d) SHA1(3171c87b0872f3206653900e3dbd210ea9beba61) )
	ROM_LOAD32_BYTE( "sbs2_p2.ic38", 0x000002, 0x80000, CRC(ade07d7e) SHA1(a5200ea3ddbfef37d302e7cb27015b6f6aa8a7c1) )
	ROM_LOAD32_BYTE( "sbs2_p3.ic39", 0x000003, 0x80000, CRC(94e943d6) SHA1(2bc7332526b969e5084b9d73063f1c0d18ec5181) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e38-19.ic30",  0x100001, 0x40000, CRC(3f50cb7b) SHA1(76af65c9b74ede843a3182f79cecda8c3e3febe6) )
	ROM_LOAD16_BYTE( "e38-20.ic31",  0x100000, 0x40000, CRC(d01340e7) SHA1(76ee48d644dc1ec415d47e0df4864c64ac928b9d) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e38-05.ic9",  0x0800000, 0x200000, CRC(bda366bf) SHA1(a7558e6d5e4583a2d8e3d2bfa8cabcc459d3ee83) )
	ROM_LOAD32_WORD_SWAP( "e38-13.ic22", 0x0800002, 0x200000, CRC(1bd7582b) SHA1(35763b9489f995433f66ef72d4f6b6ac67df5480) )
	ROM_LOAD32_WORD_SWAP( "e38-06.ic10", 0x0c00000, 0x200000, CRC(308d2783) SHA1(22c309273444bc6c1df78069850958a739664998) )
	ROM_LOAD32_WORD_SWAP( "e38-14.ic23", 0x0c00002, 0x200000, CRC(f1699f27) SHA1(3c9a9cefe6f215fd9f0a9746da97147d14df1da4) )
	ROM_LOAD32_WORD_SWAP( "e38-07.ic11", 0x1000000, 0x200000, CRC(226ba93d) SHA1(98e6342d070ddd988c1e9bff21afcfb28df86844) )
	ROM_LOAD32_WORD_SWAP( "e38-15.ic24", 0x1000002, 0x200000, CRC(2853c2e3) SHA1(046dbbd4bcb3b07cddab19a284fee9fe736f8def) )
	ROM_LOAD32_WORD_SWAP( "e38-08.ic12", 0x1400000, 0x200000, CRC(9c513b32) SHA1(fe26e39d3d65073d23d525bc17771f0c244a38c2) )
	ROM_LOAD32_WORD_SWAP( "e38-16.ic25", 0x1400002, 0x200000, CRC(fceafae2) SHA1(540ecd5d1aa64c0428a08ea1e4e634e00f7e6bd6) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e38-01.ic5",  0x0000000, 0x200000, CRC(a3c2e2c7) SHA1(538208534f996782167e4cf0d157ad93ce2937bd) )
	ROM_LOAD( "e38-02.ic6",  0x0200000, 0x200000, CRC(ecdfb75a) SHA1(85e7afa321846816fa3bd9074ad9dec95abe23fe) )
	ROM_LOAD( "e38-03.ic7",  0x0400000, 0x200000, CRC(28e9cb59) SHA1(a2651fd81a1263573f868864ee049f8fc4177ffa) )
	ROM_LOAD( "e38-04.ic8",  0x0600000, 0x080000, CRC(26cab05b) SHA1(d5bcf021646dade834840051e0ce083319c53985) )
	ROM_RELOAD(              0x0680000, 0x080000 )
	ROM_RELOAD(              0x0700000, 0x080000 )
	ROM_RELOAD(              0x0780000, 0x080000 )
	ROM_LOAD( "e38-09.ic18", 0x0800000, 0x200000, CRC(03c95a7f) SHA1(fc046cf5fcfcf5648f68af4bed78576f6d67b32f) )
	ROM_LOAD( "e38-10.ic19", 0x0a00000, 0x200000, CRC(0fb06794) SHA1(2d0e3b07ded698235572fe9e907a84d5779ac2c5) )
	ROM_LOAD( "e38-11.ic20", 0x0c00000, 0x200000, CRC(6a312351) SHA1(8037e377f8eef4cc6dd84aec9c829106f0bb130c) )
	ROM_LOAD( "e38-12.ic21", 0x0e00000, 0x080000, CRC(193a5774) SHA1(aa017ae4fec92bb87c0eb94f59d093853ddbabc9) )
	ROM_RELOAD(              0x0e80000, 0x080000 )
	ROM_RELOAD(              0x0f00000, 0x080000 )
	ROM_RELOAD(              0x0f80000, 0x080000 )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32", 0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) ) // from sidebs (redump)
	ROM_LOAD16_BYTE( "e38-17.ic33", 0x400000, 0x200000, CRC(61e81c7f) SHA1(aa650bc139685ad456c233b79aa60005a8fd6d79) )
	ROM_LOAD16_BYTE( "e38-18.ic34", 0x800000, 0x200000, CRC(43e2f149) SHA1(32ea9156948a886dda5bd071e9f493ddc2b06212) )
	ROM_LOAD16_BYTE( "e38-21.ic35", 0xc00000, 0x200000, CRC(25373c5f) SHA1(ab9f917dbde7c808be2cd836ce2d3fc558e290f1) )

	/* PALS
	e23-28.ic18    NOT A ROM
	e23-27.ic13    NOT A ROM
	e23-26.ic4     NOT A ROM
	e23-25-1.ic3   NOT A ROM
	e23-30.ic40    NOT A ROM
	e23-29.ic39    NOT A ROM
	e23-31.ic46    NOT A ROM
	e23-32-1.ic51  NOT A ROM
	e23-34.ic72    NOT A ROM
	e23-33.ic53    NOT A ROM
	e23-35.ic110   NOT A ROM
	e23-38.ic73    NOT A ROM
	e23-37.ic69    NOT A ROM
	*/
ROM_END

ROM_START( sidebs2j ) /* Side by Side 2 Evoluzione RR Ver 3.1 J */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e38-32.ic36", 0x000000, 0x80000, CRC(88385c6e) SHA1(935b4892a8322a73a8afdf0bd3799c4b11510ac9) )
	ROM_LOAD32_BYTE( "e38-33.ic37", 0x000001, 0x80000, CRC(d02a9963) SHA1(74d567869b79a7e129a2e5876900e7fecb70d568) )
	ROM_LOAD32_BYTE( "e38-34.ic38", 0x000002, 0x80000, CRC(7c4d8176) SHA1(89c5da4c60f88bca95980b1829015a6def2eabd5) )
	ROM_LOAD32_BYTE( "e38-35.ic39", 0x000003, 0x80000, CRC(8746188d) SHA1(de7f611195cd9359328821a0c0c63ac079fad341) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e38-19.ic30", 0x100001, 0x040000, CRC(3f50cb7b) SHA1(76af65c9b74ede843a3182f79cecda8c3e3febe6) )
	ROM_LOAD16_BYTE( "e38-20.ic31", 0x100000, 0x040000, CRC(d01340e7) SHA1(76ee48d644dc1ec415d47e0df4864c64ac928b9d) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e38-05.ic9",  0x0800000, 0x200000, CRC(bda366bf) SHA1(a7558e6d5e4583a2d8e3d2bfa8cabcc459d3ee83) )
	ROM_LOAD32_WORD_SWAP( "e38-13.ic22", 0x0800002, 0x200000, CRC(1bd7582b) SHA1(35763b9489f995433f66ef72d4f6b6ac67df5480) )
	ROM_LOAD32_WORD_SWAP( "e38-06.ic10", 0x0c00000, 0x200000, CRC(308d2783) SHA1(22c309273444bc6c1df78069850958a739664998) )
	ROM_LOAD32_WORD_SWAP( "e38-14.ic23", 0x0c00002, 0x200000, CRC(f1699f27) SHA1(3c9a9cefe6f215fd9f0a9746da97147d14df1da4) )
	ROM_LOAD32_WORD_SWAP( "e38-07.ic11", 0x1000000, 0x200000, CRC(226ba93d) SHA1(98e6342d070ddd988c1e9bff21afcfb28df86844) )
	ROM_LOAD32_WORD_SWAP( "e38-15.ic24", 0x1000002, 0x200000, CRC(2853c2e3) SHA1(046dbbd4bcb3b07cddab19a284fee9fe736f8def) )
	ROM_LOAD32_WORD_SWAP( "e38-08.ic12", 0x1400000, 0x200000, CRC(9c513b32) SHA1(fe26e39d3d65073d23d525bc17771f0c244a38c2) )
	ROM_LOAD32_WORD_SWAP( "e38-16.ic25", 0x1400002, 0x200000, CRC(fceafae2) SHA1(540ecd5d1aa64c0428a08ea1e4e634e00f7e6bd6) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e38-01.ic5",  0x0000000, 0x200000, CRC(a3c2e2c7) SHA1(538208534f996782167e4cf0d157ad93ce2937bd) )
	ROM_LOAD( "e38-02.ic6",  0x0200000, 0x200000, CRC(ecdfb75a) SHA1(85e7afa321846816fa3bd9074ad9dec95abe23fe) )
	ROM_LOAD( "e38-03.ic7",  0x0400000, 0x200000, CRC(28e9cb59) SHA1(a2651fd81a1263573f868864ee049f8fc4177ffa) )
	ROM_LOAD( "e38-04.ic8",  0x0600000, 0x080000, CRC(26cab05b) SHA1(d5bcf021646dade834840051e0ce083319c53985) )
	ROM_RELOAD(              0x0680000, 0x080000 )
	ROM_RELOAD(              0x0700000, 0x080000 )
	ROM_RELOAD(              0x0780000, 0x080000 )
	ROM_LOAD( "e38-09.ic18", 0x0800000, 0x200000, CRC(03c95a7f) SHA1(fc046cf5fcfcf5648f68af4bed78576f6d67b32f) )
	ROM_LOAD( "e38-10.ic19", 0x0a00000, 0x200000, CRC(0fb06794) SHA1(2d0e3b07ded698235572fe9e907a84d5779ac2c5) )
	ROM_LOAD( "e38-11.ic20", 0x0c00000, 0x200000, CRC(6a312351) SHA1(8037e377f8eef4cc6dd84aec9c829106f0bb130c) )
	ROM_LOAD( "e38-12.ic21", 0x0e00000, 0x080000, CRC(193a5774) SHA1(aa017ae4fec92bb87c0eb94f59d093853ddbabc9) )
	ROM_RELOAD(              0x0e80000, 0x080000 )
	ROM_RELOAD(              0x0f00000, 0x080000 )
	ROM_RELOAD(              0x0f80000, 0x080000 )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32", 0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) ) // from sidebs (redump)
	ROM_LOAD16_BYTE( "e38-17.ic33", 0x400000, 0x200000, CRC(61e81c7f) SHA1(aa650bc139685ad456c233b79aa60005a8fd6d79) )
	ROM_LOAD16_BYTE( "e38-18.ic34", 0x800000, 0x200000, CRC(43e2f149) SHA1(32ea9156948a886dda5bd071e9f493ddc2b06212) )
	ROM_LOAD16_BYTE( "e38-21.ic35", 0xc00000, 0x200000, CRC(25373c5f) SHA1(ab9f917dbde7c808be2cd836ce2d3fc558e290f1) )

	/* PALS
	e23-28.ic18    NOT A ROM
	e23-27.ic13    NOT A ROM
	e23-26.ic4     NOT A ROM
	e23-25-1.ic3   NOT A ROM
	e23-30.ic40    NOT A ROM
	e23-29.ic39    NOT A ROM
	e23-31.ic46    NOT A ROM
	e23-32-1.ic51  NOT A ROM
	e23-34.ic72    NOT A ROM
	e23-33.ic53    NOT A ROM
	e23-35.ic110   NOT A ROM
	e23-38.ic73    NOT A ROM
	e23-37.ic69    NOT A ROM
	*/
ROM_END

ROM_START( sidebs2ja ) /* Side by Side 2 Evoluzione Ver 2.4 J */
	ROM_REGION(0x200000, "maincpu", 0)      /* 68040 code */
	ROM_LOAD32_BYTE( "e38-23+.ic36", 0x000000, 0x80000, CRC(b3d8e2d9) SHA1(6de6a51c3d9ace532fa03517bab93101b5a3eaae) ) /* Actually labeled E38-23* */
	ROM_LOAD32_BYTE( "e38-24+.ic37", 0x000001, 0x80000, CRC(2a47d80d) SHA1(41b889e4a1397c7f0d4f6ef136ed8abfd7e1ed86) ) /* Actually labeled E38-24* */
	ROM_LOAD32_BYTE( "e38-25+.ic38", 0x000002, 0x80000, CRC(f1a8a4df) SHA1(e4cf75969fb0503df2290522194b097f5cb983a3) ) /* Actually labeled E38-25* */
	ROM_LOAD32_BYTE( "e38-26+.ic39", 0x000003, 0x80000, CRC(b550fbf2) SHA1(a0a461af7e71c6ad6468cfdee2bc7161ae31bbfb) ) /* Actually labeled E38-26* */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e38-19.ic30", 0x100001, 0x040000, CRC(3f50cb7b) SHA1(76af65c9b74ede843a3182f79cecda8c3e3febe6) )
	ROM_LOAD16_BYTE( "e38-20.ic31", 0x100000, 0x040000, CRC(d01340e7) SHA1(76ee48d644dc1ec415d47e0df4864c64ac928b9d) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e38-05.ic9",  0x0800000, 0x200000, CRC(bda366bf) SHA1(a7558e6d5e4583a2d8e3d2bfa8cabcc459d3ee83) )
	ROM_LOAD32_WORD_SWAP( "e38-13.ic22", 0x0800002, 0x200000, CRC(1bd7582b) SHA1(35763b9489f995433f66ef72d4f6b6ac67df5480) )
	ROM_LOAD32_WORD_SWAP( "e38-06.ic10", 0x0c00000, 0x200000, CRC(308d2783) SHA1(22c309273444bc6c1df78069850958a739664998) )
	ROM_LOAD32_WORD_SWAP( "e38-14.ic23", 0x0c00002, 0x200000, CRC(f1699f27) SHA1(3c9a9cefe6f215fd9f0a9746da97147d14df1da4) )
	ROM_LOAD32_WORD_SWAP( "e38-07.ic11", 0x1000000, 0x200000, CRC(226ba93d) SHA1(98e6342d070ddd988c1e9bff21afcfb28df86844) )
	ROM_LOAD32_WORD_SWAP( "e38-15.ic24", 0x1000002, 0x200000, CRC(2853c2e3) SHA1(046dbbd4bcb3b07cddab19a284fee9fe736f8def) )
	ROM_LOAD32_WORD_SWAP( "e38-08.ic12", 0x1400000, 0x200000, CRC(9c513b32) SHA1(fe26e39d3d65073d23d525bc17771f0c244a38c2) )
	ROM_LOAD32_WORD_SWAP( "e38-16.ic25", 0x1400002, 0x200000, CRC(fceafae2) SHA1(540ecd5d1aa64c0428a08ea1e4e634e00f7e6bd6) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e38-01.ic5",  0x0000000, 0x200000, CRC(a3c2e2c7) SHA1(538208534f996782167e4cf0d157ad93ce2937bd) )
	ROM_LOAD( "e38-02.ic6",  0x0200000, 0x200000, CRC(ecdfb75a) SHA1(85e7afa321846816fa3bd9074ad9dec95abe23fe) )
	ROM_LOAD( "e38-03.ic7",  0x0400000, 0x200000, CRC(28e9cb59) SHA1(a2651fd81a1263573f868864ee049f8fc4177ffa) )
	ROM_LOAD( "e38-04.ic8",  0x0600000, 0x080000, CRC(26cab05b) SHA1(d5bcf021646dade834840051e0ce083319c53985) )
	ROM_RELOAD(              0x0680000, 0x080000 )
	ROM_RELOAD(              0x0700000, 0x080000 )
	ROM_RELOAD(              0x0780000, 0x080000 )
	ROM_LOAD( "e38-09.ic18", 0x0800000, 0x200000, CRC(03c95a7f) SHA1(fc046cf5fcfcf5648f68af4bed78576f6d67b32f) )
	ROM_LOAD( "e38-10.ic19", 0x0a00000, 0x200000, CRC(0fb06794) SHA1(2d0e3b07ded698235572fe9e907a84d5779ac2c5) )
	ROM_LOAD( "e38-11.ic20", 0x0c00000, 0x200000, CRC(6a312351) SHA1(8037e377f8eef4cc6dd84aec9c829106f0bb130c) )
	ROM_LOAD( "e38-12.ic21", 0x0e00000, 0x080000, CRC(193a5774) SHA1(aa017ae4fec92bb87c0eb94f59d093853ddbabc9) )
	ROM_RELOAD(              0x0e80000, 0x080000 )
	ROM_RELOAD(              0x0f00000, 0x080000 )
	ROM_RELOAD(              0x0f80000, 0x080000 )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e23-15.ic32", 0x000000, 0x200000, CRC(8955b7c7) SHA1(767626bd5cf6810b0368ee85e487c12ef7e8a23d) ) // from sidebs (redump)
	ROM_LOAD16_BYTE( "e38-17.ic33", 0x400000, 0x200000, CRC(61e81c7f) SHA1(aa650bc139685ad456c233b79aa60005a8fd6d79) )
	ROM_LOAD16_BYTE( "e38-18.ic34", 0x800000, 0x200000, CRC(43e2f149) SHA1(32ea9156948a886dda5bd071e9f493ddc2b06212) )
	ROM_LOAD16_BYTE( "e38-21.ic35", 0xc00000, 0x200000, CRC(25373c5f) SHA1(ab9f917dbde7c808be2cd836ce2d3fc558e290f1) )

	/* PALS
	e23-28.ic18    NOT A ROM
	e23-27.ic13    NOT A ROM
	e23-26.ic4     NOT A ROM
	e23-25-1.ic3   NOT A ROM
	e23-30.ic40    NOT A ROM
	e23-29.ic39    NOT A ROM
	e23-31.ic46    NOT A ROM
	e23-32-1.ic51  NOT A ROM
	e23-34.ic72    NOT A ROM
	e23-33.ic53    NOT A ROM
	e23-35.ic110   NOT A ROM
	e23-38.ic73    NOT A ROM
	e23-37.ic69    NOT A ROM
	*/
ROM_END

ROM_START( dendego )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e35-21-1.ic36", 0x000000, 0x80000, CRC(47037231) SHA1(e53b854a3f8cf2eed68500f83f3cdf08942a2578) )
	ROM_LOAD32_BYTE( "e35-22-1.ic37", 0x000001, 0x80000, CRC(559fdd43) SHA1(5f0b8b7ea092f15e492d1257f28e4137f495e9d2) )
	ROM_LOAD32_BYTE( "e35-23-1.ic38", 0x000002, 0x80000, CRC(e457f024) SHA1(9e0eb40c877580be5a09fed48bacf60c3b0ed0cd) )
	ROM_LOAD32_BYTE( "e35-24-1.ic39", 0x000003, 0x80000, CRC(880c3b16) SHA1(d18379702f1efc4d0889f76d2d715ada0ab7695e) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e35-25.ic30", 0x100001, 0x40000, CRC(8104de13) SHA1(e518fbaf91704cf5cb8ffbb4833e3adba8c18658) )
	ROM_LOAD16_BYTE( "e35-26.ic31", 0x100000, 0x40000, CRC(61821cc9) SHA1(87cd5bd3bb22c9f4ca4b6d96f75434d48418321b) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )    /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e35-05.ic9",   0x0800000, 0x200000, CRC(a94486c5) SHA1(c3f869aa0557411f747038a1e0ed6eedcf91fda5) )
	ROM_LOAD32_WORD_SWAP( "e35-13.ic22",  0x0800002, 0x200000, CRC(2dc9dff1) SHA1(bc7ad64bc359f18a065e36749cc29c75e52202e2) )
	ROM_LOAD32_WORD_SWAP( "e35-06.ic10",  0x0c00000, 0x200000, CRC(cf328021) SHA1(709ce80f9338637172dbf4e9adcacdb3e5b4ccdb) )
	ROM_LOAD32_WORD_SWAP( "e35-14.ic23",  0x0c00002, 0x200000, CRC(cab50364) SHA1(e3272f844ecadfd63a1e3c965526a851f8cde94d) )
	ROM_LOAD32_WORD_SWAP( "e35-07.ic11",  0x1000000, 0x200000, CRC(a88a5a86) SHA1(a4a393fe9df3654e41d32e015ca3977d13206dfe) )
	ROM_LOAD32_WORD_SWAP( "e35-15.ic24",  0x1000002, 0x200000, CRC(aea86b35) SHA1(092f34f844fc6a779a6f18c03d44a9d39a101373) )
	ROM_LOAD32_WORD_SWAP( "e35-08.ic12",  0x1400000, 0x200000, CRC(99425ff6) SHA1(3bd6fe7204dece55459392170b42d4c6a9d3ef5b) )
	ROM_LOAD32_WORD_SWAP( "e35-16.ic25",  0x1400002, 0x200000, CRC(161481b6) SHA1(cc3c2939ac8911c197e9930580d316066f345772) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e35-01.ic5",   0x0000000, 0x200000, CRC(bd1975cb) SHA1(a08c6f4a84f9d4c2a5aa67cc2045aedd4580b8dc) )
	ROM_LOAD( "e35-02.ic6",   0x0200000, 0x200000, CRC(e5caa459) SHA1(c38d795b96fff193223cd3df9f51ebdc2971b719) )
	ROM_LOAD( "e35-03.ic7",   0x0400000, 0x200000, CRC(86ea5bcf) SHA1(1cee7f677b786b2fa9f50e723decd08cd69fbdef) )
	ROM_LOAD( "e35-04.ic8",   0x0600000, 0x200000, CRC(afc07c36) SHA1(f3f7b04766a9a2c8b298e1692aea24abc7001432) )
	ROM_LOAD( "e35-09.ic18",  0x0800000, 0x200000, CRC(be18ddf1) SHA1(d4fe26e427244c5b421b6421ff3bf9af303673d5) )
	ROM_LOAD( "e35-10.ic19",  0x0a00000, 0x200000, CRC(44ea9474) SHA1(161ff94b31c3dc2fa4b1e556ed62624147687443) )
	ROM_LOAD( "e35-11.ic20",  0x0c00000, 0x200000, CRC(dc8f5e88) SHA1(e311252db8a7232a5325a3eff5c1890d20bd3f8f) )
	ROM_LOAD( "e35-12.ic21",  0x0e00000, 0x200000, CRC(039b604c) SHA1(7e394e7cddc6bf42f3834d5331203e8496597a90) )

	ROM_REGION( 0x40000, "oki", 0 )     /* train board, OKI6295 sound samples */
	ROM_LOAD( "e35-28.trn",  0x000000, 0x040000, CRC(d1b571c1) SHA1(cac7d3f0285544fe36b8b744edfbac0190cdecab) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e35-17.ic32",  0x000000, 0x200000, CRC(3bc23aa1) SHA1(621e0f2f5f3dbd7d7ce05c9cd317958c361c1c26) )
	ROM_LOAD16_BYTE( "e35-18.ic33",  0x400000, 0x200000, CRC(a084d3aa) SHA1(18ea39366006e362e16d6732ce3e79cd3bfa041e) )
	ROM_LOAD16_BYTE( "e35-19.ic34",  0x800000, 0x200000, CRC(ebe2dcef) SHA1(16ae41e0f3bb242cbc2922f53cacbd99961a3f97) )
	ROM_LOAD16_BYTE( "e35-20.ic35",  0xc00000, 0x200000, CRC(a1d4b30d) SHA1(e02f613b93d3b3ee1eb23f5b7f62c5448ed3966d) )
ROM_END

ROM_START( dendegoa )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e35-21.ic36", 0x000000, 0x80000, CRC(bc70ca97) SHA1(724a24da9d6f163c26e7528ee2c15bd06f2c4382) )
	ROM_LOAD32_BYTE( "e35-22.ic37", 0x000001, 0x80000, CRC(83b17de8) SHA1(538ddc16727e08e9a2a8ff6b4f030dc044993aa0) )
	ROM_LOAD32_BYTE( "e35-23.ic38", 0x000002, 0x80000, CRC(1da4acd6) SHA1(2ce11c5f37287526bb1d39185f793d79fc73d5b5) )
	ROM_LOAD32_BYTE( "e35-24.ic39", 0x000003, 0x80000, CRC(0318afb0) SHA1(9c86330c85536fb1a093ed40610b1c3ddb7813c3) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e35-25.ic30", 0x100001, 0x40000, CRC(8104de13) SHA1(e518fbaf91704cf5cb8ffbb4833e3adba8c18658) )
	ROM_LOAD16_BYTE( "e35-26.ic31", 0x100000, 0x40000, CRC(61821cc9) SHA1(87cd5bd3bb22c9f4ca4b6d96f75434d48418321b) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )    /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e35-05.ic9",   0x0800000, 0x200000, CRC(a94486c5) SHA1(c3f869aa0557411f747038a1e0ed6eedcf91fda5) )
	ROM_LOAD32_WORD_SWAP( "e35-13.ic22",  0x0800002, 0x200000, CRC(2dc9dff1) SHA1(bc7ad64bc359f18a065e36749cc29c75e52202e2) )
	ROM_LOAD32_WORD_SWAP( "e35-06.ic10",  0x0c00000, 0x200000, CRC(cf328021) SHA1(709ce80f9338637172dbf4e9adcacdb3e5b4ccdb) )
	ROM_LOAD32_WORD_SWAP( "e35-14.ic23",  0x0c00002, 0x200000, CRC(cab50364) SHA1(e3272f844ecadfd63a1e3c965526a851f8cde94d) )
	ROM_LOAD32_WORD_SWAP( "e35-07.ic11",  0x1000000, 0x200000, CRC(a88a5a86) SHA1(a4a393fe9df3654e41d32e015ca3977d13206dfe) )
	ROM_LOAD32_WORD_SWAP( "e35-15.ic24",  0x1000002, 0x200000, CRC(aea86b35) SHA1(092f34f844fc6a779a6f18c03d44a9d39a101373) )
	ROM_LOAD32_WORD_SWAP( "e35-08.ic12",  0x1400000, 0x200000, CRC(99425ff6) SHA1(3bd6fe7204dece55459392170b42d4c6a9d3ef5b) )
	ROM_LOAD32_WORD_SWAP( "e35-16.ic25",  0x1400002, 0x200000, CRC(161481b6) SHA1(cc3c2939ac8911c197e9930580d316066f345772) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e35-01.ic5",   0x0000000, 0x200000, CRC(bd1975cb) SHA1(a08c6f4a84f9d4c2a5aa67cc2045aedd4580b8dc) )
	ROM_LOAD( "e35-02.ic6",   0x0200000, 0x200000, CRC(e5caa459) SHA1(c38d795b96fff193223cd3df9f51ebdc2971b719) )
	ROM_LOAD( "e35-03.ic7",   0x0400000, 0x200000, CRC(86ea5bcf) SHA1(1cee7f677b786b2fa9f50e723decd08cd69fbdef) )
	ROM_LOAD( "e35-04.ic8",   0x0600000, 0x200000, CRC(afc07c36) SHA1(f3f7b04766a9a2c8b298e1692aea24abc7001432) )
	ROM_LOAD( "e35-09.ic18",  0x0800000, 0x200000, CRC(be18ddf1) SHA1(d4fe26e427244c5b421b6421ff3bf9af303673d5) )
	ROM_LOAD( "e35-10.ic19",  0x0a00000, 0x200000, CRC(44ea9474) SHA1(161ff94b31c3dc2fa4b1e556ed62624147687443) )
	ROM_LOAD( "e35-11.ic20",  0x0c00000, 0x200000, CRC(dc8f5e88) SHA1(e311252db8a7232a5325a3eff5c1890d20bd3f8f) )
	ROM_LOAD( "e35-12.ic21",  0x0e00000, 0x200000, CRC(039b604c) SHA1(7e394e7cddc6bf42f3834d5331203e8496597a90) )

	ROM_REGION( 0x40000, "oki", 0 )     /* train board, OKI6295 sound samples */
	ROM_LOAD( "e35-28.trn",  0x000000, 0x040000, CRC(d1b571c1) SHA1(cac7d3f0285544fe36b8b744edfbac0190cdecab) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e35-17.ic32",  0x000000, 0x200000, CRC(3bc23aa1) SHA1(621e0f2f5f3dbd7d7ce05c9cd317958c361c1c26) )
	ROM_LOAD16_BYTE( "e35-18.ic33",  0x400000, 0x200000, CRC(a084d3aa) SHA1(18ea39366006e362e16d6732ce3e79cd3bfa041e) )
	ROM_LOAD16_BYTE( "e35-19.ic34",  0x800000, 0x200000, CRC(ebe2dcef) SHA1(16ae41e0f3bb242cbc2922f53cacbd99961a3f97) )
	ROM_LOAD16_BYTE( "e35-20.ic35",  0xc00000, 0x200000, CRC(a1d4b30d) SHA1(e02f613b93d3b3ee1eb23f5b7f62c5448ed3966d) )
ROM_END

ROM_START( dendegox )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e35-30.ic36", 0x000000, 0x80000, CRC(57ee0975) SHA1(c7741a7e0e9c1fdebc6b942587d7ac5a6f26f66d) ) //ex
	ROM_LOAD32_BYTE( "e35-31.ic37", 0x000001, 0x80000, CRC(bd5f2651) SHA1(73b760df351170ace019e4b61c82d8c6296a3632) ) //ex
	ROM_LOAD32_BYTE( "e35-32.ic38", 0x000002, 0x80000, CRC(66be29d5) SHA1(e73937f5bda709a606d5cdf7316b26051317c22f) ) //ex
	ROM_LOAD32_BYTE( "e35-33.ic39", 0x000003, 0x80000, CRC(76a6bde2) SHA1(ca456ec3f0410777362e3eb977ae156866271bd5) ) //ex

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e35-25.ic30", 0x100001, 0x40000, CRC(8104de13) SHA1(e518fbaf91704cf5cb8ffbb4833e3adba8c18658) )
	ROM_LOAD16_BYTE( "e35-26.ic31", 0x100000, 0x40000, CRC(61821cc9) SHA1(87cd5bd3bb22c9f4ca4b6d96f75434d48418321b) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e35-05.ic9",   0x0800000, 0x200000, CRC(a94486c5) SHA1(c3f869aa0557411f747038a1e0ed6eedcf91fda5) )
	ROM_LOAD32_WORD_SWAP( "e35-13.ic22",  0x0800002, 0x200000, CRC(2dc9dff1) SHA1(bc7ad64bc359f18a065e36749cc29c75e52202e2) )
	ROM_LOAD32_WORD_SWAP( "e35-06.ic10",  0x0c00000, 0x200000, CRC(cf328021) SHA1(709ce80f9338637172dbf4e9adcacdb3e5b4ccdb) )
	ROM_LOAD32_WORD_SWAP( "e35-14.ic23",  0x0c00002, 0x200000, CRC(cab50364) SHA1(e3272f844ecadfd63a1e3c965526a851f8cde94d) )
	ROM_LOAD32_WORD_SWAP( "e35-07.ic11",  0x1000000, 0x200000, CRC(a88a5a86) SHA1(a4a393fe9df3654e41d32e015ca3977d13206dfe) )
	ROM_LOAD32_WORD_SWAP( "e35-15.ic24",  0x1000002, 0x200000, CRC(aea86b35) SHA1(092f34f844fc6a779a6f18c03d44a9d39a101373) )
	ROM_LOAD32_WORD_SWAP( "e35-08.ic12",  0x1400000, 0x200000, CRC(99425ff6) SHA1(3bd6fe7204dece55459392170b42d4c6a9d3ef5b) )
	ROM_LOAD32_WORD_SWAP( "e35-16.ic25",  0x1400002, 0x200000, CRC(161481b6) SHA1(cc3c2939ac8911c197e9930580d316066f345772) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e35-01.ic5",   0x0000000, 0x200000, CRC(bd1975cb) SHA1(a08c6f4a84f9d4c2a5aa67cc2045aedd4580b8dc) )
	ROM_LOAD( "e35-02.ic6",   0x0200000, 0x200000, CRC(e5caa459) SHA1(c38d795b96fff193223cd3df9f51ebdc2971b719) )
	ROM_LOAD( "e35-03.ic7",   0x0400000, 0x200000, CRC(86ea5bcf) SHA1(1cee7f677b786b2fa9f50e723decd08cd69fbdef) )
	ROM_LOAD( "e35-04.ic8",   0x0600000, 0x200000, CRC(afc07c36) SHA1(f3f7b04766a9a2c8b298e1692aea24abc7001432) )
	ROM_LOAD( "e35-09.ic18",  0x0800000, 0x200000, CRC(be18ddf1) SHA1(d4fe26e427244c5b421b6421ff3bf9af303673d5) )
	ROM_LOAD( "e35-10.ic19",  0x0a00000, 0x200000, CRC(44ea9474) SHA1(161ff94b31c3dc2fa4b1e556ed62624147687443) )
	ROM_LOAD( "e35-11.ic20",  0x0c00000, 0x200000, CRC(dc8f5e88) SHA1(e311252db8a7232a5325a3eff5c1890d20bd3f8f) )
	ROM_LOAD( "e35-12.ic21",  0x0e00000, 0x200000, CRC(039b604c) SHA1(7e394e7cddc6bf42f3834d5331203e8496597a90) )

	ROM_REGION( 0x40000, "oki", 0 )     /* train board, OKI6295 sound samples */
	ROM_LOAD( "e35-28.trn",  0x000000, 0x040000, CRC(d1b571c1) SHA1(cac7d3f0285544fe36b8b744edfbac0190cdecab) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e35-17.ic32",  0x000000, 0x200000, CRC(3bc23aa1) SHA1(621e0f2f5f3dbd7d7ce05c9cd317958c361c1c26) )
	ROM_LOAD16_BYTE( "e35-18.ic33",  0x400000, 0x200000, CRC(a084d3aa) SHA1(18ea39366006e362e16d6732ce3e79cd3bfa041e) )
	ROM_LOAD16_BYTE( "e35-19.ic34",  0x800000, 0x200000, CRC(ebe2dcef) SHA1(16ae41e0f3bb242cbc2922f53cacbd99961a3f97) )
	ROM_LOAD16_BYTE( "e35-20.ic35",  0xc00000, 0x200000, CRC(a1d4b30d) SHA1(e02f613b93d3b3ee1eb23f5b7f62c5448ed3966d) )
ROM_END

ROM_START( dendego2 )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e52-25-1.ic36", 0x000000, 0x80000, CRC(fadf5b4c) SHA1(48f3e1425bb9552d472a2720e1c9a752db2b43ed) )
	ROM_LOAD32_BYTE( "e52-26-1.ic37", 0x000001, 0x80000, CRC(7cf5230d) SHA1(b3416886d7cfc88520f6bf378529086bf0095db5) )
	ROM_LOAD32_BYTE( "e52-27-1.ic38", 0x000002, 0x80000, CRC(25f0d81d) SHA1(c33c3e6b1ad49b63b31a2f1227d43141faef4eab) )
	ROM_LOAD32_BYTE( "e52-28-1.ic39", 0x000003, 0x80000, CRC(e76ff6a1) SHA1(674c00f19df034de8134d48a8c2d2e42f7eb1be7) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e52-29.ic30",   0x100001, 0x40000, CRC(6010162a) SHA1(f14920b26887f5387b3e261b63573d850195982a) )
	ROM_LOAD16_BYTE( "e52-30.ic31",   0x100000, 0x40000, CRC(2881af4a) SHA1(5918f6508b3cd3bef3751e3bda2a48152569c1cd) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e52-17.ic52",  0x0000000, 0x200000, CRC(4ac11921) SHA1(c4816e1d68bb52ee59e7a2e6de617c1093020944) )
	ROM_LOAD32_WORD_SWAP( "e52-18.ic53",  0x0000002, 0x200000, CRC(7f3e4af7) SHA1(ab35744014175a802e73c8b70de4e7508f0a1cd1) )
	ROM_LOAD32_WORD_SWAP( "e52-19.ic54",  0x0400000, 0x200000, CRC(2e5ff408) SHA1(91f95721b98198082e950c50f33324820719e9ed) )
	ROM_LOAD32_WORD_SWAP( "e52-20.ic55",  0x0400002, 0x200000, CRC(e90eb71e) SHA1(f07518c718f773e20412393c0ebb3243f9b1d96c) )
	ROM_LOAD32_WORD_SWAP( "e52-05.ic9",   0x0800000, 0x200000, CRC(1ad0c612) SHA1(4ffc373fca8c1e1a5edbad3305b08f0867e9809c) )
	ROM_LOAD32_WORD_SWAP( "e52-13.ic22",  0x0800002, 0x200000, CRC(943af3f4) SHA1(bfc81aa5e5c22e44601428b9e980f09d0c65e38e) )
	ROM_LOAD32_WORD_SWAP( "e52-06.ic10",  0x0c00000, 0x200000, CRC(aa35e536) SHA1(2c1b2ee0d2587db6d6dd60b081bfcef3bb0dd9fa) )
	ROM_LOAD32_WORD_SWAP( "e52-14.ic23",  0x0c00002, 0x200000, CRC(f311a9b4) SHA1(1f25571ac9468d453e92886e003400fffdc149f2) )
	ROM_LOAD32_WORD_SWAP( "e52-07.ic11",  0x1000000, 0x200000, CRC(80a27984) SHA1(57b8c41809de09582600b8ff30c135d5abd044b8) )
	ROM_LOAD32_WORD_SWAP( "e52-15.ic24",  0x1000002, 0x200000, CRC(b45a6199) SHA1(9339a1520b38d1f6538171a0b01df87de3e9c2d1) )
	ROM_LOAD32_WORD_SWAP( "e52-08.ic12",  0x1400000, 0x200000, CRC(d52e6b9c) SHA1(382a5fd4533ab641a09321208464d83f72e161e3) )
	ROM_LOAD32_WORD_SWAP( "e52-16.ic25",  0x1400002, 0x200000, CRC(db6dd6e2) SHA1(d345dbd745514d4777d52c4360787ea8c462ffb1) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e52-01.ic5",   0x0000000, 0x200000, CRC(8db39c3c) SHA1(74b3305ebdf679ae274c73b7b32d2adea602bedc) )
	ROM_LOAD( "e52-02.ic6",   0x0200000, 0x200000, CRC(b8d6f066) SHA1(99553ad66643ebf7fc71a9aee526d8f206b41dcc) )
	ROM_LOAD( "e52-03.ic7",   0x0400000, 0x200000, CRC(a37d164b) SHA1(767a7d2de8b91a00c5fe74710937457e8568a422) )
	ROM_LOAD( "e52-04.ic8",   0x0600000, 0x200000, CRC(403a4142) SHA1(aa5fea79a7a838642152586689d0f9b33311252c) )
	ROM_LOAD( "e52-09.ic18",  0x0800000, 0x200000, CRC(830e0a3c) SHA1(ab96f380e53f72945f73a6cfcc80d12e1b1afce8) )
	ROM_LOAD( "e52-10.ic19",  0x0a00000, 0x200000, CRC(671e41c6) SHA1(281899d83dba104daf7d7c2bd0834cab4c022472) )
	ROM_LOAD( "e52-11.ic20",  0x0c00000, 0x200000, CRC(1bc22680) SHA1(1f71db88d6df3b4bdf090b77bc83a67906bb31da) )
	ROM_LOAD( "e52-12.ic21",  0x0e00000, 0x200000, CRC(a8bb91c5) SHA1(959a9fedb7839e1e4e7658d920bd5da4fd8cae48) )

	ROM_REGION( 0x40000, "oki", 0 )     /* train board, OKI6295 sound samples */
	ROM_LOAD( "e35-28.trn",  0x000000, 0x040000, CRC(d1b571c1) SHA1(cac7d3f0285544fe36b8b744edfbac0190cdecab) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e52-21.ic32",  0x000000, 0x200000, CRC(ba58081d) SHA1(bcb6c8781191d48f906ed404a3e7388097a64781) )
	ROM_LOAD16_BYTE( "e52-22.ic33",  0x400000, 0x200000, CRC(dda281b1) SHA1(4851a6bf7902548c5033090a0e5c15f74c00ef58) )
	ROM_LOAD16_BYTE( "e52-23.ic34",  0x800000, 0x200000, CRC(ebe2dcef) SHA1(16ae41e0f3bb242cbc2922f53cacbd99961a3f97) ) // same as e35-19.ic34 from dendego
	ROM_LOAD16_BYTE( "e52-24.ic35",  0xc00000, 0x200000, CRC(a9a678da) SHA1(b980ae644ef0312acd63b017028af9bf2b084c29) )
ROM_END

ROM_START( dendego23k )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e52-35.ic36", 0x000000, 0x80000, CRC(d5b33eb8) SHA1(e05ad73986741827b7bbeac72af0a8324384bf6b) ) //2ex
	ROM_LOAD32_BYTE( "e52-36.ic37", 0x000001, 0x80000, CRC(f3f3fabd) SHA1(4f88080091af2208d671c491284d992b5036908c) ) //2ex
	ROM_LOAD32_BYTE( "e52-37.ic38", 0x000002, 0x80000, CRC(65b8ef31) SHA1(b61b391b160e81715ff355aeef65026d7e4dd9af) ) //2ex
	ROM_LOAD32_BYTE( "e52-38.ic39", 0x000003, 0x80000, CRC(cf61f321) SHA1(c8493d2499afba673174b26044aca537e384916c) ) //2ex

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e52-29.ic30", 0x100001, 0x40000, CRC(6010162a) SHA1(f14920b26887f5387b3e261b63573d850195982a) )
	ROM_LOAD16_BYTE( "e52-30.ic31", 0x100000, 0x40000, CRC(2881af4a) SHA1(5918f6508b3cd3bef3751e3bda2a48152569c1cd) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e52-17.ic52",  0x0000000, 0x200000, CRC(4ac11921) SHA1(c4816e1d68bb52ee59e7a2e6de617c1093020944) )
	ROM_LOAD32_WORD_SWAP( "e52-18.ic53",  0x0000002, 0x200000, CRC(7f3e4af7) SHA1(ab35744014175a802e73c8b70de4e7508f0a1cd1) )
	ROM_LOAD32_WORD_SWAP( "e52-19.ic54",  0x0400000, 0x200000, CRC(2e5ff408) SHA1(91f95721b98198082e950c50f33324820719e9ed) )
	ROM_LOAD32_WORD_SWAP( "e52-20.ic55",  0x0400002, 0x200000, CRC(e90eb71e) SHA1(f07518c718f773e20412393c0ebb3243f9b1d96c) )
	ROM_LOAD32_WORD_SWAP( "e52-05.ic9",   0x0800000, 0x200000, CRC(1ad0c612) SHA1(4ffc373fca8c1e1a5edbad3305b08f0867e9809c) )
	ROM_LOAD32_WORD_SWAP( "e52-13.ic22",  0x0800002, 0x200000, CRC(943af3f4) SHA1(bfc81aa5e5c22e44601428b9e980f09d0c65e38e) )
	ROM_LOAD32_WORD_SWAP( "e52-31.ic10",  0x0c00000, 0x200000, CRC(e9e2eb3d) SHA1(97e2dc907faa512d3fb140dafe3250f04991ff07) ) //2ex
	ROM_LOAD32_WORD_SWAP( "e52-33.ic23",  0x0c00002, 0x200000, CRC(6906f41f) SHA1(0d3f6fc4772417190d123ad38b0b8a8a532159c6) ) //2ex
	ROM_LOAD32_WORD_SWAP( "e52-32.ic11",  0x1000000, 0x200000, CRC(43d452fd) SHA1(20a4064904cf2f21d5f03236c99c9e21a49a1206) ) //2ex
	ROM_LOAD32_WORD_SWAP( "e52-34.ic24",  0x1000002, 0x200000, CRC(4ae1064b) SHA1(a5f1ad3262f8ffd09e9063a6dbe98d883b11a156) ) //2ex
	ROM_LOAD32_WORD_SWAP( "e52-08.ic12",  0x1400000, 0x200000, CRC(d52e6b9c) SHA1(382a5fd4533ab641a09321208464d83f72e161e3) )
	ROM_LOAD32_WORD_SWAP( "e52-16.ic25",  0x1400002, 0x200000, CRC(db6dd6e2) SHA1(d345dbd745514d4777d52c4360787ea8c462ffb1) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e52-01.ic5",   0x0000000, 0x200000, CRC(8db39c3c) SHA1(74b3305ebdf679ae274c73b7b32d2adea602bedc) )
	ROM_LOAD( "e52-02.ic6",   0x0200000, 0x200000, CRC(b8d6f066) SHA1(99553ad66643ebf7fc71a9aee526d8f206b41dcc) )
	ROM_LOAD( "e52-03.ic7",   0x0400000, 0x200000, CRC(a37d164b) SHA1(767a7d2de8b91a00c5fe74710937457e8568a422) )
	ROM_LOAD( "e52-04.ic8",   0x0600000, 0x200000, CRC(403a4142) SHA1(aa5fea79a7a838642152586689d0f9b33311252c) )
	ROM_LOAD( "e52-09.ic18",  0x0800000, 0x200000, CRC(830e0a3c) SHA1(ab96f380e53f72945f73a6cfcc80d12e1b1afce8) )
	ROM_LOAD( "e52-10.ic19",  0x0a00000, 0x200000, CRC(671e41c6) SHA1(281899d83dba104daf7d7c2bd0834cab4c022472) )
	ROM_LOAD( "e52-11.ic20",  0x0c00000, 0x200000, CRC(1bc22680) SHA1(1f71db88d6df3b4bdf090b77bc83a67906bb31da) )
	ROM_LOAD( "e52-12.ic21",  0x0e00000, 0x200000, CRC(a8bb91c5) SHA1(959a9fedb7839e1e4e7658d920bd5da4fd8cae48) )

	ROM_REGION( 0x40000, "oki", 0 )     /* train board, OKI6295 sound samples */
	ROM_LOAD( "e35-28.trn",  0x000000, 0x040000, CRC(d1b571c1) SHA1(cac7d3f0285544fe36b8b744edfbac0190cdecab) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e52-21.ic32",  0x000000, 0x200000, CRC(ba58081d) SHA1(bcb6c8781191d48f906ed404a3e7388097a64781) )
	ROM_LOAD16_BYTE( "e52-22.ic33",  0x400000, 0x200000, CRC(dda281b1) SHA1(4851a6bf7902548c5033090a0e5c15f74c00ef58) )
	ROM_LOAD16_BYTE( "e52-23.ic34",  0x800000, 0x200000, CRC(ebe2dcef) SHA1(16ae41e0f3bb242cbc2922f53cacbd99961a3f97) ) // same as e35-19.ic34 from dendego
	ROM_LOAD16_BYTE( "e52-24.ic35",  0xc00000, 0x200000, CRC(a9a678da) SHA1(b980ae644ef0312acd63b017028af9bf2b084c29) )
ROM_END

/* Landing Gear known program rom sets not dumped:

E17-28 through E17-31 (E17-32 is a PAL)

*/

ROM_START( landgear ) /* Landing Gear Ver 4.2 O */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e17-37.ic36", 0x000000, 0x80000, CRC(e6dda113) SHA1(786cbfae420b6ee820a93731e59da3442245b6b8) )
	ROM_LOAD32_BYTE( "e17-38.ic37", 0x000001, 0x80000, CRC(86fa29bd) SHA1(f711528143c042cdc4a26d9e6965a882a73f397c) )
	ROM_LOAD32_BYTE( "e17-39.ic38", 0x000002, 0x80000, CRC(ccbbcc7b) SHA1(52d91fcaa1683d2679ed4f14ebc11dc487527898) )
	ROM_LOAD32_BYTE( "e17-40.ic39", 0x000003, 0x80000, CRC(ce9231d2) SHA1(d2c3955d910dbd0cac95862047c58791af626722) ) /* 0x7ffff == 03 - One byte difference from E17-36.ic39 */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e17-21.ic30", 0x100001, 0x40000, CRC(8b54f46c) SHA1(c6d16197ab7768945becf9b49b6d286113b4d1cc) )
	ROM_LOAD16_BYTE( "e17-22.ic31", 0x100000, 0x40000, CRC(b96f6cd7) SHA1(0bf086e5dc6d524cd00e33df3e3d2a8b9231eb72) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e17-03.ic9",   0x0800000, 0x200000, CRC(64820c4f) SHA1(ee18e4e2b01ec21c33ec1f0eb43f6d0cd48d7225) )
	ROM_LOAD32_WORD_SWAP( "e17-09.ic22",  0x0800002, 0x200000, CRC(19e9a1d1) SHA1(26f1a91e3757da510d685a11add08e3e00317796) )
	ROM_LOAD32_WORD_SWAP( "e17-04.ic10",  0x0c00000, 0x200000, CRC(7dc2cae3) SHA1(90638a1efc353428ce4155ca29f67accaf0499cd) )
	ROM_LOAD32_WORD_SWAP( "e17-10.ic23",  0x0c00002, 0x200000, CRC(a6bdf6b8) SHA1(e8d76d38f2c7e428a3c2f555571e314351d74a69) )
	ROM_LOAD32_WORD_SWAP( "e17-05.ic11",  0x1000000, 0x200000, CRC(3f70acd4) SHA1(e8c1c6214631e3e39d1fc9df13d1862442a47e5d) )
	ROM_LOAD32_WORD_SWAP( "e17-11.ic24",  0x1000002, 0x200000, CRC(4e986d93) SHA1(b218a0360c1d0eca5a907f2b402f352e0329fe41) )
	ROM_LOAD32_WORD_SWAP( "e17-06.ic12",  0x1400000, 0x200000, CRC(107ff481) SHA1(2a48cedec9641ff08776e5d8b1bf1f5b250d4179) )
	ROM_LOAD32_WORD_SWAP( "e17-12.ic25",  0x1400002, 0x200000, CRC(0727ddfa) SHA1(68bf83a3c46cd042a7ad27a530c8bed6360d8492) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", ROMREGION_ERASE00 )      /* only accessible to the TMS */
	ROM_LOAD( "e17-01.ic5",   0x0000000, 0x200000, CRC(42aa56a6) SHA1(945c338515ceb946c01480919546869bb8c3d323) )
	ROM_LOAD( "e17-02.ic8",   0x0600000, 0x200000, CRC(df7e2405) SHA1(684d6fc398791c48101e6cb63acbf0d691ed863c) )
	ROM_LOAD( "e17-07.ic18",  0x0800000, 0x200000, CRC(0f180eb0) SHA1(5e1dd920f110a62a029bace6f4cb80fee0fdaf03) )
	ROM_LOAD( "e17-08.ic19",  0x0a00000, 0x200000, CRC(3107e154) SHA1(59a99770c2aa535cac6569f41b03be1554e0e800) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e17-13.ic32",  0x000000, 0x200000, CRC(6cf238e7) SHA1(0745d2dcfea26178adde3ad08650156e8e30651f) )
	ROM_LOAD16_BYTE( "e17-14.ic33",  0x400000, 0x200000, CRC(5efec311) SHA1(f253bc40f2567f59ddfb617fddb8b9a389bfac89) )
	ROM_LOAD16_BYTE( "e17-15.ic34",  0x800000, 0x200000, CRC(41d7a7d0) SHA1(f5a8b79c1d47611e93d46aaf921107b52090bb5f) )
	ROM_LOAD16_BYTE( "e17-16.ic35",  0xc00000, 0x200000, CRC(6cf9f277) SHA1(03ca51fadc6b0b6502804346f18eeb55ab87b0e7) )

	ROM_REGION( 0x1000, "pals", 0 ) /* PALCE 16V8, saved in Jedec format (unused now) */
	ROM_LOAD( "e07-02.ic4",    0x0000, 0x0bac, CRC(b10110e0) SHA1(574dfa70cbdc910973f4b47a9534f22839baf76d) )
	ROM_LOAD( "e07-03.ic50",   0x0000, 0x0bac, CRC(3fe03710) SHA1(bbccddea0cccb50ea361721e51a0489f6686312c) )
	ROM_LOAD( "e07-04.ic115",  0x0000, 0x0bac, CRC(6c83e648) SHA1(7ed4001d8f27933b31c09d98421dac5bdc265ff4) )
	ROM_LOAD( "e07-05.ic22",   0x0000, 0x0bac, CRC(d2530a9d) SHA1(2f5cafe9ac4e5b3121981dc7498a82b54b2aee08) )
	ROM_LOAD( "e07-06.ic37",   0x0000, 0x0bac, CRC(ade68194) SHA1(95322d49b1f6edac03a7a00bf8876e556ccc2581) )
	ROM_LOAD( "e07-07.ic49",   0x0000, 0x0bac, CRC(76136aac) SHA1(5f28de747e32a4e2622603a7d35e6c979e56977d) )
	ROM_LOAD( "e07-08.ic65",   0x0000, 0x0bac, CRC(33aa1678) SHA1(e0d941c048bec25994dea30fed989d9f9e1af6a6) )
	ROM_LOAD( "e07-09.ic82",   0x0000, 0x0bac, CRC(62e2bf9e) SHA1(fe7055f5f6292f4c613cc3a47245340722299b45) )
	ROM_LOAD( "e07-10.ic116",  0x0000, 0x0bac, CRC(97e36b54) SHA1(2181097bfd7d09a537cbcc4e7ead11532f0ddb20) )
	ROM_LOAD( "e09-21.ic69",   0x0000, 0x0bac, CRC(74b06310) SHA1(0e1f559247a43fbb3852b1b0b92753ed993b876d) )
	ROM_LOAD( "e09-22.ic73",   0x0000, 0x0bac, CRC(58aa9c49) SHA1(7c9b463ea1adc701f326773cc556ab11f290f87e) )
	ROM_LOAD( "e17-32.ic96",   0x0000, 0x0bac, CRC(1d590ca3) SHA1(733e9e34642c1e03fd880fda198b98927c1bb81d) )
ROM_END

ROM_START( landgearj ) /* Landing Gear Ver 4.2 J */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e17-33.ic36", 0x000000, 0x80000, CRC(e6dda113) SHA1(786cbfae420b6ee820a93731e59da3442245b6b8) ) /* matches E17-37.ic36, verified correct */
	ROM_LOAD32_BYTE( "e17-34.ic37", 0x000001, 0x80000, CRC(86fa29bd) SHA1(f711528143c042cdc4a26d9e6965a882a73f397c) ) /* matches E17-38.ic37, verified correct */
	ROM_LOAD32_BYTE( "e17-35.ic38", 0x000002, 0x80000, CRC(ccbbcc7b) SHA1(52d91fcaa1683d2679ed4f14ebc11dc487527898) ) /* matches E17-39.ic38, verified correct */
	ROM_LOAD32_BYTE( "e17-36.ic39", 0x000003, 0x80000, CRC(209c50fe) SHA1(42e0eaa182730e260ee4361d936b133ed85f8221) ) /* 0x7ffff == 01 - One byte difference from E17-40.ic39 */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e17-21.ic30", 0x100001, 0x40000, CRC(8b54f46c) SHA1(c6d16197ab7768945becf9b49b6d286113b4d1cc) )
	ROM_LOAD16_BYTE( "e17-22.ic31", 0x100000, 0x40000, CRC(b96f6cd7) SHA1(0bf086e5dc6d524cd00e33df3e3d2a8b9231eb72) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e17-03.ic9",   0x0800000, 0x200000, CRC(64820c4f) SHA1(ee18e4e2b01ec21c33ec1f0eb43f6d0cd48d7225) )
	ROM_LOAD32_WORD_SWAP( "e17-09.ic22",  0x0800002, 0x200000, CRC(19e9a1d1) SHA1(26f1a91e3757da510d685a11add08e3e00317796) )
	ROM_LOAD32_WORD_SWAP( "e17-04.ic10",  0x0c00000, 0x200000, CRC(7dc2cae3) SHA1(90638a1efc353428ce4155ca29f67accaf0499cd) )
	ROM_LOAD32_WORD_SWAP( "e17-10.ic23",  0x0c00002, 0x200000, CRC(a6bdf6b8) SHA1(e8d76d38f2c7e428a3c2f555571e314351d74a69) )
	ROM_LOAD32_WORD_SWAP( "e17-05.ic11",  0x1000000, 0x200000, CRC(3f70acd4) SHA1(e8c1c6214631e3e39d1fc9df13d1862442a47e5d) )
	ROM_LOAD32_WORD_SWAP( "e17-11.ic24",  0x1000002, 0x200000, CRC(4e986d93) SHA1(b218a0360c1d0eca5a907f2b402f352e0329fe41) )
	ROM_LOAD32_WORD_SWAP( "e17-06.ic12",  0x1400000, 0x200000, CRC(107ff481) SHA1(2a48cedec9641ff08776e5d8b1bf1f5b250d4179) )
	ROM_LOAD32_WORD_SWAP( "e17-12.ic25",  0x1400002, 0x200000, CRC(0727ddfa) SHA1(68bf83a3c46cd042a7ad27a530c8bed6360d8492) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", ROMREGION_ERASE00 )      /* only accessible to the TMS */
	ROM_LOAD( "e17-01.ic5",   0x0000000, 0x200000, CRC(42aa56a6) SHA1(945c338515ceb946c01480919546869bb8c3d323) )
	ROM_LOAD( "e17-02.ic8",   0x0600000, 0x200000, CRC(df7e2405) SHA1(684d6fc398791c48101e6cb63acbf0d691ed863c) )
	ROM_LOAD( "e17-07.ic18",  0x0800000, 0x200000, CRC(0f180eb0) SHA1(5e1dd920f110a62a029bace6f4cb80fee0fdaf03) )
	ROM_LOAD( "e17-08.ic19",  0x0a00000, 0x200000, CRC(3107e154) SHA1(59a99770c2aa535cac6569f41b03be1554e0e800) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e17-13.ic32",  0x000000, 0x200000, CRC(6cf238e7) SHA1(0745d2dcfea26178adde3ad08650156e8e30651f) )
	ROM_LOAD16_BYTE( "e17-14.ic33",  0x400000, 0x200000, CRC(5efec311) SHA1(f253bc40f2567f59ddfb617fddb8b9a389bfac89) )
	ROM_LOAD16_BYTE( "e17-15.ic34",  0x800000, 0x200000, CRC(41d7a7d0) SHA1(f5a8b79c1d47611e93d46aaf921107b52090bb5f) )
	ROM_LOAD16_BYTE( "e17-16.ic35",  0xc00000, 0x200000, CRC(6cf9f277) SHA1(03ca51fadc6b0b6502804346f18eeb55ab87b0e7) )

	ROM_REGION( 0x1000, "pals", 0 ) /* PALCE 16V8, saved in Jedec format (unused now) */
	ROM_LOAD( "e07-02.ic4",    0x0000, 0x0bac, CRC(b10110e0) SHA1(574dfa70cbdc910973f4b47a9534f22839baf76d) )
	ROM_LOAD( "e07-03.ic50",   0x0000, 0x0bac, CRC(3fe03710) SHA1(bbccddea0cccb50ea361721e51a0489f6686312c) )
	ROM_LOAD( "e07-04.ic115",  0x0000, 0x0bac, CRC(6c83e648) SHA1(7ed4001d8f27933b31c09d98421dac5bdc265ff4) )
	ROM_LOAD( "e07-05.ic22",   0x0000, 0x0bac, CRC(d2530a9d) SHA1(2f5cafe9ac4e5b3121981dc7498a82b54b2aee08) )
	ROM_LOAD( "e07-06.ic37",   0x0000, 0x0bac, CRC(ade68194) SHA1(95322d49b1f6edac03a7a00bf8876e556ccc2581) )
	ROM_LOAD( "e07-07.ic49",   0x0000, 0x0bac, CRC(76136aac) SHA1(5f28de747e32a4e2622603a7d35e6c979e56977d) )
	ROM_LOAD( "e07-08.ic65",   0x0000, 0x0bac, CRC(33aa1678) SHA1(e0d941c048bec25994dea30fed989d9f9e1af6a6) )
	ROM_LOAD( "e07-09.ic82",   0x0000, 0x0bac, CRC(62e2bf9e) SHA1(fe7055f5f6292f4c613cc3a47245340722299b45) )
	ROM_LOAD( "e07-10.ic116",  0x0000, 0x0bac, CRC(97e36b54) SHA1(2181097bfd7d09a537cbcc4e7ead11532f0ddb20) )
	ROM_LOAD( "e09-21.ic69",   0x0000, 0x0bac, CRC(74b06310) SHA1(0e1f559247a43fbb3852b1b0b92753ed993b876d) )
	ROM_LOAD( "e09-22.ic73",   0x0000, 0x0bac, CRC(58aa9c49) SHA1(7c9b463ea1adc701f326773cc556ab11f290f87e) )
	ROM_LOAD( "e17-32.ic96",   0x0000, 0x0bac, CRC(1d590ca3) SHA1(733e9e34642c1e03fd880fda198b98927c1bb81d) )
ROM_END

ROM_START( landgeara ) /* Landing Gear Ver 3.1 O, is there an alternate set without the "*" on the labels? */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e17-24+.ic36", 0x000000, 0x80000, CRC(6907e451) SHA1(330eecb5898942514b40e67cf3c9dcb82d4cafab) ) /* Actually labeled E17-24* */
	ROM_LOAD32_BYTE( "e17-25+.ic37", 0x000001, 0x80000, CRC(ecbc8875) SHA1(5f5e4850cbdbdfff4a7f0b781edb2e983c166962) ) /* Actually labeled E17-25* */
	ROM_LOAD32_BYTE( "e17-26+.ic38", 0x000002, 0x80000, CRC(3032bbe7) SHA1(201c61f236c81928f50815d8ad12e312a3c7427b) ) /* Actually labeled E17-26* */
	ROM_LOAD32_BYTE( "e17-27+.ic39", 0x000003, 0x80000, CRC(e936224c) SHA1(8699cbb756844d12b7585e66198b7faed2af8e24) ) /* Actually labeled E17-27* */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e17-21.ic30",  0x100001, 0x40000, CRC(8b54f46c) SHA1(c6d16197ab7768945becf9b49b6d286113b4d1cc) )
	ROM_LOAD16_BYTE( "e17-22.ic31",  0x100000, 0x40000, CRC(b96f6cd7) SHA1(0bf086e5dc6d524cd00e33df3e3d2a8b9231eb72) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e17-03.ic9",   0x0800000, 0x200000, CRC(64820c4f) SHA1(ee18e4e2b01ec21c33ec1f0eb43f6d0cd48d7225) )
	ROM_LOAD32_WORD_SWAP( "e17-09.ic22",  0x0800002, 0x200000, CRC(19e9a1d1) SHA1(26f1a91e3757da510d685a11add08e3e00317796) )
	ROM_LOAD32_WORD_SWAP( "e17-04.ic10",  0x0c00000, 0x200000, CRC(7dc2cae3) SHA1(90638a1efc353428ce4155ca29f67accaf0499cd) )
	ROM_LOAD32_WORD_SWAP( "e17-10.ic23",  0x0c00002, 0x200000, CRC(a6bdf6b8) SHA1(e8d76d38f2c7e428a3c2f555571e314351d74a69) )
	ROM_LOAD32_WORD_SWAP( "e17-05.ic11",  0x1000000, 0x200000, CRC(3f70acd4) SHA1(e8c1c6214631e3e39d1fc9df13d1862442a47e5d) )
	ROM_LOAD32_WORD_SWAP( "e17-11.ic24",  0x1000002, 0x200000, CRC(4e986d93) SHA1(b218a0360c1d0eca5a907f2b402f352e0329fe41) )
	ROM_LOAD32_WORD_SWAP( "e17-06.ic12",  0x1400000, 0x200000, CRC(107ff481) SHA1(2a48cedec9641ff08776e5d8b1bf1f5b250d4179) )
	ROM_LOAD32_WORD_SWAP( "e17-12.ic25",  0x1400002, 0x200000, CRC(0727ddfa) SHA1(68bf83a3c46cd042a7ad27a530c8bed6360d8492) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", ROMREGION_ERASE00 )      /* only accessible to the TMS */
	ROM_LOAD( "e17-01.ic5",   0x0000000, 0x200000, CRC(42aa56a6) SHA1(945c338515ceb946c01480919546869bb8c3d323) )
	ROM_LOAD( "e17-02.ic8",   0x0600000, 0x200000, CRC(df7e2405) SHA1(684d6fc398791c48101e6cb63acbf0d691ed863c) )
	ROM_LOAD( "e17-07.ic18",  0x0800000, 0x200000, CRC(0f180eb0) SHA1(5e1dd920f110a62a029bace6f4cb80fee0fdaf03) )
	ROM_LOAD( "e17-08.ic19",  0x0a00000, 0x200000, CRC(3107e154) SHA1(59a99770c2aa535cac6569f41b03be1554e0e800) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e17-13.ic32",  0x000000, 0x200000, CRC(6cf238e7) SHA1(0745d2dcfea26178adde3ad08650156e8e30651f) )
	ROM_LOAD16_BYTE( "e17-14.ic33",  0x400000, 0x200000, CRC(5efec311) SHA1(f253bc40f2567f59ddfb617fddb8b9a389bfac89) )
	ROM_LOAD16_BYTE( "e17-15.ic34",  0x800000, 0x200000, CRC(41d7a7d0) SHA1(f5a8b79c1d47611e93d46aaf921107b52090bb5f) )
	ROM_LOAD16_BYTE( "e17-16.ic35",  0xc00000, 0x200000, CRC(6cf9f277) SHA1(03ca51fadc6b0b6502804346f18eeb55ab87b0e7) )

	ROM_REGION( 0x1000, "pals", 0 ) /* PALCE 16V8, saved in Jedec format (unused now) */
	ROM_LOAD( "e07-02.ic4",    0x0000, 0x0bac, CRC(b10110e0) SHA1(574dfa70cbdc910973f4b47a9534f22839baf76d) )
	ROM_LOAD( "e07-03.ic50",   0x0000, 0x0bac, CRC(3fe03710) SHA1(bbccddea0cccb50ea361721e51a0489f6686312c) )
	ROM_LOAD( "e07-04.ic115",  0x0000, 0x0bac, CRC(6c83e648) SHA1(7ed4001d8f27933b31c09d98421dac5bdc265ff4) )
	ROM_LOAD( "e07-05.ic22",   0x0000, 0x0bac, CRC(d2530a9d) SHA1(2f5cafe9ac4e5b3121981dc7498a82b54b2aee08) )
	ROM_LOAD( "e07-06.ic37",   0x0000, 0x0bac, CRC(ade68194) SHA1(95322d49b1f6edac03a7a00bf8876e556ccc2581) )
	ROM_LOAD( "e07-07.ic49",   0x0000, 0x0bac, CRC(76136aac) SHA1(5f28de747e32a4e2622603a7d35e6c979e56977d) )
	ROM_LOAD( "e07-08.ic65",   0x0000, 0x0bac, CRC(33aa1678) SHA1(e0d941c048bec25994dea30fed989d9f9e1af6a6) )
	ROM_LOAD( "e07-09.ic82",   0x0000, 0x0bac, CRC(62e2bf9e) SHA1(fe7055f5f6292f4c613cc3a47245340722299b45) )
	ROM_LOAD( "e07-10.ic116",  0x0000, 0x0bac, CRC(97e36b54) SHA1(2181097bfd7d09a537cbcc4e7ead11532f0ddb20) )
	ROM_LOAD( "e09-21.ic69",   0x0000, 0x0bac, CRC(74b06310) SHA1(0e1f559247a43fbb3852b1b0b92753ed993b876d) )
	ROM_LOAD( "e09-22.ic73",   0x0000, 0x0bac, CRC(58aa9c49) SHA1(7c9b463ea1adc701f326773cc556ab11f290f87e) )
	ROM_LOAD( "e17-32.ic96",   0x0000, 0x0bac, CRC(1d590ca3) SHA1(733e9e34642c1e03fd880fda198b98927c1bb81d) )
ROM_END

ROM_START( landgearja ) /* Landing Gear Ver 3.0 J, is there an alternate set without the "*" on the labels? */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e17-17+.ic36", 0x000000, 0x80000, CRC(653e9c43) SHA1(b43c4baf1b3114977faa310c0815ea0940d548b3) ) /* Actually labeled E17-17* */
	ROM_LOAD32_BYTE( "e17-18+.ic37", 0x000001, 0x80000, CRC(4d90b321) SHA1(a919f15dcc105eaa12d7c9816aff4f0daffbb7a1) ) /* Actually labeled E17-18* */
	ROM_LOAD32_BYTE( "e17-19+.ic38", 0x000002, 0x80000, CRC(1c487204) SHA1(f6c8ddd80c57ed63b0785b240c4b00416a1a87f3) ) /* Actually labeled E17-19* */
	ROM_LOAD32_BYTE( "e17-20+.ic39", 0x000003, 0x80000, CRC(1311234f) SHA1(5211cae0d6dc1710bc669bcf81a247b01f8aebff) ) /* Actually labeled E17-20* */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e17-21.ic30",  0x100001, 0x40000, CRC(8b54f46c) SHA1(c6d16197ab7768945becf9b49b6d286113b4d1cc) )
	ROM_LOAD16_BYTE( "e17-22.ic31",  0x100000, 0x40000, CRC(b96f6cd7) SHA1(0bf086e5dc6d524cd00e33df3e3d2a8b9231eb72) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e17-23.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e17-03.ic9",   0x0800000, 0x200000, CRC(64820c4f) SHA1(ee18e4e2b01ec21c33ec1f0eb43f6d0cd48d7225) )
	ROM_LOAD32_WORD_SWAP( "e17-09.ic22",  0x0800002, 0x200000, CRC(19e9a1d1) SHA1(26f1a91e3757da510d685a11add08e3e00317796) )
	ROM_LOAD32_WORD_SWAP( "e17-04.ic10",  0x0c00000, 0x200000, CRC(7dc2cae3) SHA1(90638a1efc353428ce4155ca29f67accaf0499cd) )
	ROM_LOAD32_WORD_SWAP( "e17-10.ic23",  0x0c00002, 0x200000, CRC(a6bdf6b8) SHA1(e8d76d38f2c7e428a3c2f555571e314351d74a69) )
	ROM_LOAD32_WORD_SWAP( "e17-05.ic11",  0x1000000, 0x200000, CRC(3f70acd4) SHA1(e8c1c6214631e3e39d1fc9df13d1862442a47e5d) )
	ROM_LOAD32_WORD_SWAP( "e17-11.ic24",  0x1000002, 0x200000, CRC(4e986d93) SHA1(b218a0360c1d0eca5a907f2b402f352e0329fe41) )
	ROM_LOAD32_WORD_SWAP( "e17-06.ic12",  0x1400000, 0x200000, CRC(107ff481) SHA1(2a48cedec9641ff08776e5d8b1bf1f5b250d4179) )
	ROM_LOAD32_WORD_SWAP( "e17-12.ic25",  0x1400002, 0x200000, CRC(0727ddfa) SHA1(68bf83a3c46cd042a7ad27a530c8bed6360d8492) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", ROMREGION_ERASE00 )      /* only accessible to the TMS */
	ROM_LOAD( "e17-01.ic5",   0x0000000, 0x200000, CRC(42aa56a6) SHA1(945c338515ceb946c01480919546869bb8c3d323) )
	ROM_LOAD( "e17-02.ic8",   0x0600000, 0x200000, CRC(df7e2405) SHA1(684d6fc398791c48101e6cb63acbf0d691ed863c) )
	ROM_LOAD( "e17-07.ic18",  0x0800000, 0x200000, CRC(0f180eb0) SHA1(5e1dd920f110a62a029bace6f4cb80fee0fdaf03) )
	ROM_LOAD( "e17-08.ic19",  0x0a00000, 0x200000, CRC(3107e154) SHA1(59a99770c2aa535cac6569f41b03be1554e0e800) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e17-13.ic32",  0x000000, 0x200000, CRC(6cf238e7) SHA1(0745d2dcfea26178adde3ad08650156e8e30651f) )
	ROM_LOAD16_BYTE( "e17-14.ic33",  0x400000, 0x200000, CRC(5efec311) SHA1(f253bc40f2567f59ddfb617fddb8b9a389bfac89) )
	ROM_LOAD16_BYTE( "e17-15.ic34",  0x800000, 0x200000, CRC(41d7a7d0) SHA1(f5a8b79c1d47611e93d46aaf921107b52090bb5f) )
	ROM_LOAD16_BYTE( "e17-16.ic35",  0xc00000, 0x200000, CRC(6cf9f277) SHA1(03ca51fadc6b0b6502804346f18eeb55ab87b0e7) )

	ROM_REGION( 0x1000, "pals", 0 ) /* PALCE 16V8, saved in Jedec format (unused now) */
	ROM_LOAD( "e07-02.ic4",    0x0000, 0x0bac, CRC(b10110e0) SHA1(574dfa70cbdc910973f4b47a9534f22839baf76d) )
	ROM_LOAD( "e07-03.ic50",   0x0000, 0x0bac, CRC(3fe03710) SHA1(bbccddea0cccb50ea361721e51a0489f6686312c) )
	ROM_LOAD( "e07-04.ic115",  0x0000, 0x0bac, CRC(6c83e648) SHA1(7ed4001d8f27933b31c09d98421dac5bdc265ff4) )
	ROM_LOAD( "e07-05.ic22",   0x0000, 0x0bac, CRC(d2530a9d) SHA1(2f5cafe9ac4e5b3121981dc7498a82b54b2aee08) )
	ROM_LOAD( "e07-06.ic37",   0x0000, 0x0bac, CRC(ade68194) SHA1(95322d49b1f6edac03a7a00bf8876e556ccc2581) )
	ROM_LOAD( "e07-07.ic49",   0x0000, 0x0bac, CRC(76136aac) SHA1(5f28de747e32a4e2622603a7d35e6c979e56977d) )
	ROM_LOAD( "e07-08.ic65",   0x0000, 0x0bac, CRC(33aa1678) SHA1(e0d941c048bec25994dea30fed989d9f9e1af6a6) )
	ROM_LOAD( "e07-09.ic82",   0x0000, 0x0bac, CRC(62e2bf9e) SHA1(fe7055f5f6292f4c613cc3a47245340722299b45) )
	ROM_LOAD( "e07-10.ic116",  0x0000, 0x0bac, CRC(97e36b54) SHA1(2181097bfd7d09a537cbcc4e7ead11532f0ddb20) )
	ROM_LOAD( "e09-21.ic69",   0x0000, 0x0bac, CRC(74b06310) SHA1(0e1f559247a43fbb3852b1b0b92753ed993b876d) )
	ROM_LOAD( "e09-22.ic73",   0x0000, 0x0bac, CRC(58aa9c49) SHA1(7c9b463ea1adc701f326773cc556ab11f290f87e) )
	ROM_LOAD( "e17-32.ic96",   0x0000, 0x0bac, CRC(1d590ca3) SHA1(733e9e34642c1e03fd880fda198b98927c1bb81d) )
ROM_END

ROM_START( dangcurv ) /* Dangerous Curves Ver 2.9 O */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e09-32-1+.ic36", 0x000000, 0x80000, CRC(82c0af83) SHA1(307e2dd441116ffaad0d5783cd139bde8fc23460) ) /* Actually labeled E09 32-1* */
	ROM_LOAD32_BYTE( "e09-33-1+.ic37", 0x000001, 0x80000, CRC(be6eca0b) SHA1(edb4ad0d379132e64c1238191ab17bd4400f3322) ) /* Actually labeled E09 33-1* */
	ROM_LOAD32_BYTE( "e09-34-1+.ic38", 0x000002, 0x80000, CRC(553de745) SHA1(474e96c810fee8097059e201b95810b3235c6cad) ) /* Actually labeled E09 34-1* */
	ROM_LOAD32_BYTE( "e09-35-1+.ic39", 0x000003, 0x80000, CRC(a60273fd) SHA1(511191506f04320775787e294a59a94c07364006) ) /* Actually labeled E09 35-1* */

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e09-27.ic30", 0x100001, 0x40000, CRC(6d54839c) SHA1(a28c9b0727128b82bb0fa71dc951e3f03ee45e1b) )
	ROM_LOAD16_BYTE( "e09-28.ic31", 0x100000, 0x40000, CRC(566d7d83) SHA1(92661ccb631f843bf704c50d54fae28f6b5b272b) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e09-29.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e09-05.ic9",   0x0800000, 0x200000, CRC(a948782f) SHA1(2a2b0d2955e036ddf424c54131435a20dbba3dd4) )
	ROM_LOAD32_WORD_SWAP( "e09-13.ic22",  0x0800002, 0x200000, CRC(985859e2) SHA1(8af9a73eba2151a5ef60799682fe667663a42743) )
	ROM_LOAD32_WORD_SWAP( "e09-06.ic10",  0x0c00000, 0x200000, CRC(218dcb5b) SHA1(72aedd2890e076540195d738c76ba446769c8e89) )
	ROM_LOAD32_WORD_SWAP( "e09-14.ic23",  0x0c00002, 0x200000, CRC(6d123616) SHA1(01ac1e920f7c4a03adf365c8a7831b8385f0b78b) )
	ROM_LOAD32_WORD_SWAP( "e09-07.ic11",  0x1000000, 0x200000, CRC(37fd7efc) SHA1(24a275d302ec8940479d15f1aeb96a288868bd41) )
	ROM_LOAD32_WORD_SWAP( "e09-15.ic24",  0x1000002, 0x200000, CRC(0d773f3b) SHA1(f867a4d5956c2ebfa9858499d9716b4dc723d76b) )
	ROM_LOAD32_WORD_SWAP( "e09-08.ic12",  0x1400000, 0x200000, CRC(5c080485) SHA1(c950cd00df5b6d2d0a119ba318fa8b0a3f471b29) )
	ROM_LOAD32_WORD_SWAP( "e09-16.ic25",  0x1400002, 0x200000, CRC(35cb8346) SHA1(c2ecedd3c2a28213ef83e776f3007c974128189b) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e09-01.ic5",   0x0000000, 0x200000, CRC(22a6a53d) SHA1(6efa89151cd5ec43ab9bfa9b92694eb0018dd227) )
	ROM_LOAD( "e09-02.ic6",   0x0200000, 0x200000, CRC(405e2969) SHA1(376b9dd548d876af6798553a6da5deed4de00b76) )
	ROM_LOAD( "e09-03.ic7",   0x0400000, 0x200000, CRC(15327754) SHA1(bf08ab80875b400700241a66715e229dae6752d1) )
	ROM_LOAD( "e09-04.ic8",   0x0600000, 0x200000, CRC(fd598d6e) SHA1(679d9d64a0cd031a6c8cb5e170b77fc5811b6d73) )
	ROM_LOAD( "e09-09.ic18",  0x0800000, 0x200000, CRC(a527b387) SHA1(790240b4dfcdf2bf70edb943ec7aeb2f0d8cdfa9) )
	ROM_LOAD( "e09-10.ic19",  0x0a00000, 0x200000, CRC(4de6253c) SHA1(33517c0895b7ee04f4a84074d0b7bf42b53d5816) )
	ROM_LOAD( "e09-11.ic20",  0x0c00000, 0x200000, CRC(18cc0ba7) SHA1(626929a501def6f1b8bd6a468786efb0b0dda9fa) )
	ROM_LOAD( "e09-12.ic21",  0x0e00000, 0x200000, CRC(3273e438) SHA1(e9581d52f5db1c1924a860464579332a2f23e713) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e09-17.ic32",  0x000000, 0x200000, CRC(a8a6512e) SHA1(71bd3ccd65b731270b92da334b9fb99c28e267fe) )
	ROM_LOAD16_BYTE( "e09-18.ic33",  0x400000, 0x200000, CRC(bdf1f5eb) SHA1(a568a99a90e0afbcd26ddd320f515ed62cf0db1a) )
	ROM_LOAD16_BYTE( "e09-19.ic34",  0x800000, 0x200000, CRC(3626c7ed) SHA1(7535f0457b3d9fe1d54712a26322d6144b9e7de6) )
	ROM_LOAD16_BYTE( "e09-20.ic35",  0xc00000, 0x200000, CRC(9652a5c4) SHA1(2933e4e8e57ff618ce21721036d96347471c5539) )

	ROM_REGION( 0x1000, "pals", 0 ) /* PALCE 16V8 (unused now) */
	ROM_LOAD( "e07-02.ic4",    0x0000, 0x0117, CRC(21b3b349) SHA1(2c468a355caeb541fc649842f228fdc14b01afad) )
	ROM_LOAD( "e07-03.ic50",   0x0000, 0x0117, CRC(af68d73a) SHA1(11c9a6eee715066dc06766cc89774081ffedab3b) )
	ROM_LOAD( "e07-04.ic115",  0x0000, 0x0117, CRC(98383e84) SHA1(446e7a38f933b55706de71884f764c512912f571) )
	ROM_LOAD( "e07-05.ic22",   0x0000, 0x0117, CRC(3fe21bb8) SHA1(744dcfef2b2dbb2aa4b69d47e03cec7881e4cd4b) )
	ROM_LOAD( "e07-06.ic37",   0x0000, 0x0117, CRC(5cbeb11c) SHA1(e8cb4898d1285b72ae7ce003bfac33897417bb91) )
	ROM_LOAD( "e07-07.ic49",   0x0000, 0x0117, CRC(8914317b) SHA1(8b782ff4f2783d8531d0d9abe0215ecc10cd1f49) )
	ROM_LOAD( "e07-08.ic65",   0x0000, 0x0117, CRC(1831ddf3) SHA1(96bc0eb4b49c6d646e91630bb174f75c5d29337a) )
	ROM_LOAD( "e07-09.ic82",   0x0000, 0x0117, CRC(6ab127fb) SHA1(9beb74436968854fedacea915587e8fdffaec6c8) )
	ROM_LOAD( "e07-10.ic116",  0x0000, 0x0117, CRC(24d7939c) SHA1(fd3d87471252805de01c7c0abd60e55669885de0) )
	ROM_LOAD( "e09-21.ic69",   0x0000, 0x0117, CRC(ab2c7402) SHA1(df8b63a67bf693f9023794ad4e787e3248d10b7e) )
	ROM_LOAD( "e09-22.ic73",   0x0000, 0x0117, CRC(d43972cc) SHA1(ed9454d1a225a5fb6fefc0ae1aebe2b439ed460f) )
ROM_END

ROM_START( dangcurvj ) /* Dangerous Curves Ver 2.2 J */
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68040 code */
	ROM_LOAD32_BYTE( "e09-23.ic36", 0x000000, 0x80000, CRC(b4cdadd6) SHA1(84bd1d055ff15afb5438cd5151abf78b0000cebc) )
	ROM_LOAD32_BYTE( "e09-24.ic37", 0x000001, 0x80000, CRC(fb2fc795) SHA1(2f58d043ab9fc0269a5b6827009777cd7ab832fc) )
	ROM_LOAD32_BYTE( "e09-25.ic38", 0x000002, 0x80000, CRC(aa233404) SHA1(a2b14e54eb1b5f6d4ed9f289b30ecfa654f21c87) )
	ROM_LOAD32_BYTE( "e09-26.ic39", 0x000003, 0x80000, CRC(78337271) SHA1(bd29de6a5b6db3baddecf82c3b6c8b366c64289e) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )       /* 68000 Code */
	ROM_LOAD16_BYTE( "e09-27.ic30", 0x100001, 0x40000, CRC(6d54839c) SHA1(a28c9b0727128b82bb0fa71dc951e3f03ee45e1b) )
	ROM_LOAD16_BYTE( "e09-28.ic31", 0x100000, 0x40000, CRC(566d7d83) SHA1(92661ccb631f843bf704c50d54fae28f6b5b272b) )

	ROM_REGION( 0x4000, "dsp", ROMREGION_ERASE00 ) /* TMS320C51 internal rom */
	ROM_LOAD16_WORD( "e07-11.ic29", 0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x010000, "sub", 0 )        /* MC68HC11M0 code */
	ROM_LOAD( "e09-29.ic65",  0x000000, 0x010000, CRC(80ac1428) SHA1(5a2a1e60a11ecdb8743c20ddacfb61f9fd00f01c) )

	ROM_REGION32_BE( 0x1800000, "maingfx", 0 )
	ROM_LOAD32_WORD_SWAP( "e09-05.ic9",   0x0800000, 0x200000, CRC(a948782f) SHA1(2a2b0d2955e036ddf424c54131435a20dbba3dd4) )
	ROM_LOAD32_WORD_SWAP( "e09-13.ic22",  0x0800002, 0x200000, CRC(985859e2) SHA1(8af9a73eba2151a5ef60799682fe667663a42743) )
	ROM_LOAD32_WORD_SWAP( "e09-06.ic10",  0x0c00000, 0x200000, CRC(218dcb5b) SHA1(72aedd2890e076540195d738c76ba446769c8e89) )
	ROM_LOAD32_WORD_SWAP( "e09-14.ic23",  0x0c00002, 0x200000, CRC(6d123616) SHA1(01ac1e920f7c4a03adf365c8a7831b8385f0b78b) )
	ROM_LOAD32_WORD_SWAP( "e09-07.ic11",  0x1000000, 0x200000, CRC(37fd7efc) SHA1(24a275d302ec8940479d15f1aeb96a288868bd41) )
	ROM_LOAD32_WORD_SWAP( "e09-15.ic24",  0x1000002, 0x200000, CRC(0d773f3b) SHA1(f867a4d5956c2ebfa9858499d9716b4dc723d76b) )
	ROM_LOAD32_WORD_SWAP( "e09-08.ic12",  0x1400000, 0x200000, CRC(5c080485) SHA1(c950cd00df5b6d2d0a119ba318fa8b0a3f471b29) )
	ROM_LOAD32_WORD_SWAP( "e09-16.ic25",  0x1400002, 0x200000, CRC(35cb8346) SHA1(c2ecedd3c2a28213ef83e776f3007c974128189b) )

	ROM_REGION16_LE( 0x1000000, "dspgfx", 0 )      /* only accessible to the TMS */
	ROM_LOAD( "e09-01.ic5",   0x0000000, 0x200000, CRC(22a6a53d) SHA1(6efa89151cd5ec43ab9bfa9b92694eb0018dd227) )
	ROM_LOAD( "e09-02.ic6",   0x0200000, 0x200000, CRC(405e2969) SHA1(376b9dd548d876af6798553a6da5deed4de00b76) )
	ROM_LOAD( "e09-03.ic7",   0x0400000, 0x200000, CRC(15327754) SHA1(bf08ab80875b400700241a66715e229dae6752d1) )
	ROM_LOAD( "e09-04.ic8",   0x0600000, 0x200000, CRC(fd598d6e) SHA1(679d9d64a0cd031a6c8cb5e170b77fc5811b6d73) )
	ROM_LOAD( "e09-09.ic18",  0x0800000, 0x200000, CRC(a527b387) SHA1(790240b4dfcdf2bf70edb943ec7aeb2f0d8cdfa9) )
	ROM_LOAD( "e09-10.ic19",  0x0a00000, 0x200000, CRC(4de6253c) SHA1(33517c0895b7ee04f4a84074d0b7bf42b53d5816) )
	ROM_LOAD( "e09-11.ic20",  0x0c00000, 0x200000, CRC(18cc0ba7) SHA1(626929a501def6f1b8bd6a468786efb0b0dda9fa) )
	ROM_LOAD( "e09-12.ic21",  0x0e00000, 0x200000, CRC(3273e438) SHA1(e9581d52f5db1c1924a860464579332a2f23e713) )

	ROM_REGION16_BE( 0x1000000, "taito_en:ensoniq", ROMREGION_ERASE00  )
	ROM_LOAD16_BYTE( "e09-17.ic32",  0x000000, 0x200000, CRC(a8a6512e) SHA1(71bd3ccd65b731270b92da334b9fb99c28e267fe) )
	ROM_LOAD16_BYTE( "e09-18.ic33",  0x400000, 0x200000, CRC(bdf1f5eb) SHA1(a568a99a90e0afbcd26ddd320f515ed62cf0db1a) )
	ROM_LOAD16_BYTE( "e09-19.ic34",  0x800000, 0x200000, CRC(3626c7ed) SHA1(7535f0457b3d9fe1d54712a26322d6144b9e7de6) )
	ROM_LOAD16_BYTE( "e09-20.ic35",  0xc00000, 0x200000, CRC(9652a5c4) SHA1(2933e4e8e57ff618ce21721036d96347471c5539) )
ROM_END


GAME( 1995, dangcurv,  0,        taitojc, dangcurv, taitojc_state, init_dangcurv, ROT0, "Taito", "Dangerous Curves (Ver 2.9 O)",                         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN ) // DANGEROUS CURVES       VER 2.9 O   1995.08.24   17:45
GAME( 1995, dangcurvj, dangcurv, taitojc, dangcurv, taitojc_state, init_dangcurv, ROT0, "Taito", "Dangerous Curves (Ver 2.2 J)",                         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN ) // DANGEROUS CURVES       VER 2.2 J   1995.07.20   17:45
GAME( 1995, landgear,  0,        taitojc, landgear, taitojc_state, init_taitojc,  ROT0, "Taito", "Landing Gear (Ver 4.2 O)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING )                                              // LANDING GEAR           VER 4.2 O   Feb  8 1996  09:46:22
GAME( 1995, landgearj, landgear, taitojc, landgear, taitojc_state, init_taitojc,  ROT0, "Taito", "Landing Gear (Ver 4.2 J)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING )                                              // LANDING GEAR           VER 4.2 J   Feb  8 1996  09:46:22
GAME( 1995, landgeara, landgear, taitojc, landgear, taitojc_state, init_taitojc,  ROT0, "Taito", "Landing Gear (Ver 3.1 O)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING )                                              // LANDING GEAR           VER 3.1 O   Feb  8 1996  09:46:22
GAME( 1995, landgearja,landgear, taitojc, landgear, taitojc_state, init_taitojc,  ROT0, "Taito", "Landing Gear (Ver 3.0 J)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING )                                              // LANDING GEAR           VER 3.0 J   Feb  8 1996  09:46:22
GAME( 1996, sidebs,    0,        taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side (Ver 3.0 OK)",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE           VER 3.0 OK  1996/ 9/ 2   20:04:19
GAME( 1996, sidebsj,   sidebs,   taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side (Ver 2.7 J)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE           VER 2.7 J   1996/10/11   14:54:10
GAME( 1996, sidebsja,  sidebs,   taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side (Ver 2.6 J)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE           VER 2.6 J   1996/ 7/ 1   18:41:51
GAME( 1996, sidebsjb,  sidebs,   taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side (Ver 2.5 J)",                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE           VER 2.5 J   1996/ 6/20   18:13:14
GAMEL(1996, dendego,   0,        dendego, dendego,  taitojc_state, init_taitojc,  ROT0, "Taito", "Densha de GO! (Ver 2.3 J)",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING, layout_dendego )                              // DENSYA DE GO           VER 2.3 J   1997/ 3/10   20:49:44
GAMEL(1996, dendegoa,  dendego,  dendego, dendego,  taitojc_state, init_taitojc,  ROT0, "Taito", "Densha de GO! (Ver 2.2 J)",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING, layout_dendego )                              // DENSYA DE GO           VER 2.2 J   1997/ 2/ 4   12:00:28
GAMEL(1996, dendegox,  dendego,  dendego, dendego,  taitojc_state, init_taitojc,  ROT0, "Taito", "Densha de GO! EX (Ver 2.4 J)",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING, layout_dendego )                              // DENSYA DE GO           VER 2.4 J   1997/ 4/18   13:38:34
GAME( 1997, sidebs2,   0,        taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side 2 (Ver 2.6 OK)",                          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE2          VER 2.6 OK  1997/ 6/ 4   17:27:37
GAME( 1997, sidebs2u,  sidebs2,  taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side 2 (Ver 2.6 A)",                           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE2          VER 2.6 A   1997/ 6/19   09:39:22
GAME( 1997, sidebs2j,  sidebs2,  taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side 2 Evoluzione RR (Ver 3.1 J)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE2          VER 3.1 J   1997/10/ 7   13:55:38
GAME( 1997, sidebs2ja, sidebs2,  taitojc, sidebs,   taitojc_state, init_taitojc,  ROT0, "Taito", "Side by Side 2 Evoluzione (Ver 2.4 J)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )                       // SIDE BY SIDE2          VER 2.4 J   1997/ 5/26   13:06:37
GAMEL(1998, dendego2,  0,        dendego, dendego,  taitojc_state, init_dendego2, ROT0, "Taito", "Densha de GO! 2 Kousoku-hen (Ver 2.5 J)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING, layout_dendego )                              // DENSYA DE GO2          VER 2.5 J   1998/ 3/ 2   15:30:55
GAMEL(1998, dendego23k,dendego2, dendego, dendego,  taitojc_state, init_dendego2, ROT0, "Taito", "Densha de GO! 2 Kousoku-hen 3000-bandai (Ver 2.20 J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_TIMING, layout_dendego )                              // DENSYA DE GO! 2 3000   VER 2.20 J  1998/ 7/15   17:42:38
