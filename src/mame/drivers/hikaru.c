/* Sega Hikaru / 'Samurai' */

/*

Sega Hikaru Hardware Overview (last updated 5th August 2009 at 3:45pm)
-----------------------------

Note! This document will be updated from time to time when more dumps are available.

This document covers all the known Sega Hikaru games. The graphics are quite breath-taking
and this system is said to be one of the most expensive arcade boards developed by Sega.
The games on this system include....
Air Trix                     (C) Sega, 2001
!Brave Fire Fighters         (C) Sega, 1999
*Cyber Troopers Virtual On 4 (C) Sega, 2001
!Nascar Racing               (C) Sega, 2000
!Planet Harriers             (C) Sega, 2001
!Star Wars Racer Arcade      (C) Sega, 2000

! - denotes secured but not fully dumped yet
* - denotes not dumped yet. If you can help with the remaining undumped games,
    please contact http://guru.mameworld.info/ or http://mamedev.org/

The Sega Hikaru system comprises the following PCBs.....
Main board     - 2 known versions exists. They're mostly the same. It contains many thin BGAs,
                 lots of RAM, 2x SH4 CPUs and 1x 16MBit boot EPROM. Because of the use of many thin BGA
                 ICs, the main board is VERY fragile and is mounted inside a metal box.
ROM PCB        - 2 known versions exist. They contain EPROMs, up to 16x TSOP48 flashROMs or SOP44
                 maskROMs, a small amount of SRAM and some security ICs that are also used on
                 NAOMI ROM carts (315-5881 - with a unique 317-xxxx number per game, a couple
                 of PLCC FPGAs and a X76F100 secured EEPROM). Plugs into mainboard into CONN3 and CONN4
Network PCB    - Contains a 68000, some SRAM, a few Sega ICs and a CPLD. Plugs into the top of the ROM board
AICA PCB       - Contains one RAM and one QFP100 IC. Plugs into the mainboard into CONN5
I/O PCB        - Uses a standard Sega JVS I/O board. Contains USB connectors type A & B, a
                 couple of long IDC cable connectors for controls and a Toshiba TMP90PH44 MCU
Filter PCB     - Contains external connectors for power, network, USB, audio and video. This board is mounted
                 to the metal box and plugs into the main board into CONN1 and CONN2


Main Board
----------
Version 1 - 837-13402 171-7639E SEGA 1998
Version 2 - 837-14097 171-7639F SEGA 1998
|------------------------------------------------------------------------------------|
|       MC33470.103            DSW(4)                           CONN4                |
|                                    93C46.115                                       |
|                          SH-4.13                          62256.72       93C46.78  |-|
|        SH-4.21                               PAL1         62256.73                 | |
|                        5264165FTTA60.15      (GAL16V8)                    3771     | |
|                        5264165FTTA60.16                      315-6146.74           | |
|    5264165FTTA60.22    5264165FTTA60.17S                               14.7456MHz  | |
|    5264165FTTA60.23    5264165FTTA60.18S                               32MHz       | |CONN1
|    5264165FTTA60.24S                                                               | |
|    5264165FTTA60.25S                                         3771 3771             | |
|                         315-6154.19                                                | |
|      315-6154.26                                                          PC910.102| |
|               33.3333MHz                                        CY37128.95         | |
|               CY2308SC.4                         CY2292SL.1     (=315-6202)        |-|
|HY57V161610.28                                                           ADM485.80  |
| PAL2                      CONN3                                 62256.91S          |
| (GAL16V8)                                                       62256.92S          |
|HY57V161610.35                                                                      |
|HY57V161610.36S                 315-6087.43  HY57V161610.44                         |-|
|HY57V161610.33 315-6083A.31                  HY57V161610.45S         EPR-23400(A).94| |
|HY57V161610.34S          D432232.32                   24.576MHz                     | |
|(PAL on V2 MB)                              HY57V161610.46   ADV7120      BATTERY   | |CONN2
|LED1                                        HY57V161610.47S  ADV7120                | |
|LED2                                                                                | |
|LED3                                                                                |-|
|LED4               315-6197.38                     315-6084.41  CONN5               |
|SW1                                315-6085.40                        K4S641632.98  |
|SW2                                                                                 |
|SW3    315-6086.37                                                   315-6232.96    |
|                                                   315-6084.42                      |
|                   315-6197.39     CY2308SC.2                  33.8688MHz           |
|(LED 1-4 moved here)          41.6666MHz                       25MHz                |
|(on V2 MB)                                                                          |
|------------------------------------------------------------------------------------|
Notes:
      In test mode, the ROM test will show only 'GOOD' for all ROMs. On test completion
      the screen will show 'PRESS TEST BUTTON TO EXIT'
      However if the SERVICE button is pressed, the screen will show the byte
      checksums of all ROMs.


ROM PCB
-------

Type 1 - 837-14140   171-8144B
|------------------------------------------------------------------------------------|
|                                                                                    |
|   IC45   IC41   IC37   IC50   IC46   IC42   IC38                                   |
|                                                                                    |
|                                                                        LATTICE     |
|   IC49   IC57   IC53   IC66   IC62   IC58   IC54                       M4A3-32/32  |
|                                                                        (315-6323)  |
|                                                                  X76F100           |
|   IC61                                                                             |
|                                                                         CY7C1399   |
|                                                                         CY7C1399   |
|   IC65                                                                             |
|           IC29  IC30  IC31  IC32  IC33  IC34  IC35  IC36                           |
|                                                                       28MHz        |
|                                                                            LATTICE |
|                                                                 315-5881   PLSI2032|
|                                                                          (315-6050)|
|------------------------------------------------------------------------------------|
ROM usage -                                                   CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Air Trix -
                      MPR-23573.IC37    128M TSOP48 MASKROM   B9A5   9E67
                      MPR-23577.IC38    "                     A52A   BCE0
                      MPR-23574.IC41    "                     DABB   B621
                      MPR-23578.IC42    "                     4BD4   5E6B
                      MPR-23575.IC45    "                     0D06   AD63
                      MPR-23579.IC46    "                     790F   A27E
                      MPR-23576.IC49    "                     BDBB   4F01
                      MPR-23580.IC50    "                     14A7   6A4E
                      IC53, IC54, IC57, \
                      IC58, IC61, IC62,  Not Used
                      IC65, IC66        /

                      EPR-23601A.IC29   27C322 EPROM          FEE8   C889
                      EPR-23602A.IC30   "                     97C2   620C
                      other EPROM sockets not used

                      315-5881 stamped 317-0294-COM


                                                              CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Planet Harriers -
                      MPR-23549.IC37    128M TSOP48 MASKROM   7F16   2C37
                      MPR-23553.IC38    "                     1F9F   AAE5
                      MPR-23550.IC41    "                     986C   8D7A
                      MPR-23554.IC42    "                     BD1D   5304
                      MPR-23551.IC45    "                     9784   B33D
                      MPR-23555.IC46    "                     CB75   B08B
                      MPR-23552.IC49    "                     5056   B3A9
                      MPR-23556.IC50    "                     CBDE   BE85
                      MPR-23557.IC53    "                     3D36   05BF
                      MPR-23561.IC54    "                     D629   8ED6
                      MPR-23558.IC57    "                     B9F5   0082
                      MPR-23562.IC58    "                     5875   8163
                      MPR-23559.IC61    "                     B19D   E7CC
                      MPR-23563.IC62    "                     158C   D180
                      MPR-23560.IC65    "                     34BC   677B
                      MPR-23564.IC66    "                     5524   349E

                      EPR-23565A.IC29   27C322 EPROM          8AC6
                      EPR-23566A.IC30   "                     1CC3
                      EPR-23567A.IC31   "                     388C   13B5
                      EPR-23568A.IC32   "                     D289   5910
                      EPR-23569A.IC33   "                     09A8   578F
                      EPR-23570A.IC34   "                     73E5   94F7
                      EPR-23571A.IC35   "                     B1DE   6DAD
                      EPR-23572A.IC36   "                     9019   6DD5
                      all EPROM sockets populated

                      315-5881 stamped 317-0297-COM


Type 2 - 837-13403   171-7640B
|------------------------------------------------------------------------------------|
|               LATTICE        CY7C199               X76F100                         |
|  315-5881     PLSI2032                                                             |
|                       28MHz  CY7C199               MACH111                         |
|                                                    (315-6203A)         IC30   IC32 |
|                                                                                    |
| IC37  IC39  IC41  IC43  IC45  IC47  IC49  IC51                                     |
|                                                                                    |
|                                                                                    |
| IC53  IC55  IC57  IC59  IC61  IC63  IC65  IC67  <-- these on other side of PCB     |
|                                                                                    |
|                                                                                    |
| IC38  IC40  IC42  IC44  IC46  IC48  IC50  IC52                                     |
|                                                                                    |
|                                                IC29  IC31  IC33  IC35  IC34  IC36  |
| IC54  IC56  IC58  IC60  IC62  IC64  IC66  IC68 <-- these on other side of PCB      |
|                                                                                    |
|------------------------------------------------------------------------------------|
ROM usage -                                                   CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Star Wars Racer Arcade
                      MPR-23086.IC37    64M SOP44 MASKROM     7993  8E18
                      MPR-23087.IC38    "                     4D44  D239
                      MPR-23088.IC39    "                     4135  BEAB
                      MPR-23089.IC40    "                     F0C8  04E2
                      MPR-23090.IC41    "                     9532  4C1C
                      MPR-23091.IC42    "                     925D  02FB
                      MPR-23092.IC43    "                     0809  7050
                      MPR-23093.IC44    "                     72BC  9311
                      MPR-23094.IC45    "                     DE84  9D8A
                      MPR-23095.IC46    "                     7A5C  E7FC
                      MPR-23096.IC47    "                     6806  1392
                      MPR-23097.IC48    "                     EDF1  7BD1
                      MPR-23098.IC49    "                     B82D  E114
                      MPR-23099.IC50    "                     5792  E5E5
                      MPR-23100.IC51    "                     3AF3  A97C
                      MPR-23101.IC52    "                     A8CC  721D
                      MPR-23102.IC53    "                     CED7  D3CF
                      MPR-23103.IC54    "                     6B67  FC76
                      MPR-23104.IC55    "                     586C  6954
                      MPR-23105.IC56    "                     13A0  DB38
                      MPR-23106.IC57    "                     4F03  42BF
                      MPR-23107.IC58    "                     8EA6  ADB6
                      MPR-23108.IC59    "                     8645  FC30
                      MPR-23109.IC60    "                     3847  CA6B
                      MPR-23110.IC61    "                     4140  01C4
                      MPR-23111.IC62    "                     EBE6  8085
                      MPR-23112.IC63    "                     B68B  7467
                      MPR-23113.IC64    "                     4715  4787
                      MPR-23114.IC65    "                     3CD6  144A
                      MPR-23115.IC66    "                     E5D3  BA35
                      MPR-23116.IC67    "                     E668  08ED
                      MPR-23117.IC68    "                     1FE8  C4A1

                      EPR-23174.IC29    27C322 EPROM          3B2E
                      EPR-23175.IC30    "                     F377
                      EPR-23176.IC31    "                     5F01   4174
                      EPR-23177.IC32    "                     3594   38B3
                      other EPROM sockets not used

                      315-5881 stamped 317-0277-COM


AICA PCB
--------------

837-13629  171-7911B  SEGA 1998
|------------------|
|    K4S641632     |
|                  |
|                  |
|                  |
|     315-6232     |
|                  |
|                  |
|------------------|
Notes:
      K4S641632 - Samsung K4S641632 64Mbit SDRAM (1M x 16Bit x 4 Banks Synchronous DRAM)


Network PCB
-----------

837-13404  171-7641B
|-----------------------------------|
| 40MHz      LATTICE     315-5917   |
|            PLSI2032               |
|            (315-5958A)    315-5917|
|   PAL                             |
|                          62256    |
|             315-5804     62256    |
|                          62256    |
|                          62256    |
|  68000          PAL               |
|  62256                            |
|  62256          PAL               |
|-----------------------------------|
Notes:
      62256 - 32k x8 SRAM


I/O PCB
-------

837-13551  171-7780D
|----------------------------|
|CN4  CN5     CN6     CN7 CN8|
| LED2  ADM485               |
|       TMP90PH44            |
|    14.745MHz          LED1 |
|CN1 CN2     CN3             |
|----------------------------|
Notes:
      TMP90PH44 - Toshiba TMP90PH44 TLCS-90 series microcontroller with
                  16k internal OTP PROM and 512 bytes internal RAM
            CN1 - 5 pin connector for +12v
            CN2 - 5 pin connector for +12v
            CN3 - 60 pin flat cable connector for digital controls & buttons
            CN4 - USB connector
            CN5 - USB connector
            CN6 - 26 pin flat cable connector for analog controls
            CN7 - 4 pin connector for +5v
            CN8 - 4 pin connector for +5v


Filter PCB
----------

839-1079  171-7789B SEGA 1998
|----------------------------------------------------|
|                CN6                CN11        CN9  |
|                               CN12   CN10          |
|                CN5  CN4   CN3           CN2   CN1  |
|                                                    |
|                                                    |
|  CNTX CNRX CN7       DN2               DN1         |
|----------------------------------------------------|
Notes:
      CN1  - 8 pin JVS power connector
      CN2  - 6 pin JVS power connector
      CN3  - 15 pin VGA connector 24k/31k
      CN4  - 15 pin VGA connector 24k/31k. Not populated
      CN5  - red/white RCA audio connectors
      CN6  - red/white RCA audio connectors
      CN7  - USB connector. Plugs into JVS I/O PCB using common A-B USB cable
      CN8  - Optical network connector, re-labelled CNTX. Sometimes not populated
      CN9  - Optical network connector, re-labelled CNTR. Sometimes not populated
      CN10 - Unknown. Not populated
      CN11 - RS422 connector. Not populated
      CN12 - Midi connector. Not populated
   DN1/DN2 - Connectors joining to main board. These connectors
             are on the other side of the PCB

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"

#define CPU_CLOCK (200000000)
                                 /* MD2 MD1 MD0 MD6 MD4 MD3 MD5 MD7 MD8 */
//static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };


static VIDEO_START(hikaru)
{

}

static VIDEO_UPDATE(hikaru)
{
	return 0;
}

static INPUT_PORTS_START( hikaru )
	PORT_START("IN0")
INPUT_PORTS_END

static ADDRESS_MAP_START( hikaru_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001FFFFF) AM_ROM AM_SHARE("share1")
	AM_RANGE(0x00400000, 0x004000FF) AM_NOP // unknown
	AM_RANGE(0x00800000, 0x008000FF) AM_NOP // unknown
	AM_RANGE(0x00830000, 0x00831FFF) AM_NOP // unknown
	AM_RANGE(0x00838000, 0x008380ff) AM_NOP // unknown
	AM_RANGE(0x0082F000, 0x0082F0ff) AM_NOP // unknown
	AM_RANGE(0x00C00000, 0x00C002FF) AM_RAM // unknown nvram?
	AM_RANGE(0x01000000, 0x010001FF) AM_NOP // unknown
	AM_RANGE(0x02000000, 0x020000FF) AM_NOP // unknown
	AM_RANGE(0x02710000, 0x027100FF) AM_NOP // unknown
	AM_RANGE(0x03000000, 0x030000FF) AM_NOP // unknown
	AM_RANGE(0x04000000, 0x040000FF) AM_NOP // unknown
	AM_RANGE(0x0C000000, 0x0DFFFFFF) AM_RAM
	AM_RANGE(0x14000000, 0x140000FF) AM_NOP // unknown
	AM_RANGE(0x14004000, 0x140041FF) AM_RAM // unknown
	AM_RANGE(0x15000000, 0x150000FF) AM_NOP // unknown
	AM_RANGE(0x16001000, 0x160010FF) AM_RAM // unknown
	AM_RANGE(0x1A000000, 0x1A0000FF) AM_NOP // unknown
ADDRESS_MAP_END

static ADDRESS_MAP_START( hikaru_map_slave, ADDRESS_SPACE_PROGRAM, 64 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x001FFFFF) AM_ROM AM_SHARE("share1")
	AM_RANGE(0x0C000000, 0x0DFFFFFF) AM_RAM
	AM_RANGE(0x10000000, 0x100000FF) AM_RAM
	AM_RANGE(0x1A800000, 0x1A8000FF) AM_RAM
	AM_RANGE(0x1B000000, 0x1B0001FF) AM_RAM
ADDRESS_MAP_END


static MACHINE_DRIVER_START( hikaru )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", SH4, CPU_CLOCK)
//  MDRV_CPU_CONFIG(sh4cpu_config)
	MDRV_CPU_PROGRAM_MAP(hikaru_map)
//  MDRV_CPU_IO_MAP(hikaru_port)
//  MDRV_CPU_VBLANK_INT("screen", hikaru,vblank)
	MDRV_CPU_ADD("slave", SH4, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(hikaru_map_slave)

//  MDRV_MACHINE_START( hikaru )
//  MDRV_MACHINE_RESET( hikaru )

//  MDRV_NVRAM_HANDLER(hikaru_eeproms)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(hikaru)
	MDRV_VIDEO_UPDATE(hikaru)

//  MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
//  MDRV_SOUND_ADD("aica", AICA, 0)
//  MDRV_SOUND_CONFIG(aica_config)
//  MDRV_SOUND_ROUTE(0, "lspeaker", 2.0)
//  MDRV_SOUND_ROUTE(0, "rspeaker", 2.0)
MACHINE_DRIVER_END


#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */


#define HIKARU_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr23400a" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr23400a.ic94",   0x000000, 0x200000, CRC(2aa906a7) SHA1(098c9909b123ed6c338ac874f2ee90e3b2da4c02) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr23400" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-23400.ic94",   0x000000, 0x200000, CRC(3d557104) SHA1(d39879f5a1acbd54ad8ee4fbd412f870c9ff4aa5) ) \

// bios 0 is SAMURAI boot rom 0.96 / 2000/8/10
// bios 1 is SAMURAI boot rom 0.92 / 1999/7/2


ROM_START( hikaru )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x400000, "user1", ROMREGION_ERASE)
ROM_END


ROM_START( airtrix )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x800000, "user1", 0)
	ROM_LOAD32_WORD("epr23601a.ic29", 0x0000000, 0x0400000, CRC(cd3ccc05) SHA1(49de32d3588511f37486aff900773453739d706d) )
	ROM_LOAD32_WORD("epr23602a.ic30", 0x0000002, 0x0400000, CRC(24f1bca9) SHA1(719dc4e003c1d13fcbb39604c156c89042c47dfd) )
	/* ic31 unpopulated */
	/* ic32 unpopulated */
	/* ic33 unpopulated */
	/* ic34 unpopulated */
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 128M TSOP48 MASKROMs */
	ROM_REGION( 0x10000000, "user2", 0)
	ROM_LOAD("mpr-23573.ic37" , 0x0000000, 0x1000000, CRC(e22a0734) SHA1(fc06d5972d285d09473874aaeb1efed2d19c8f36) )
	ROM_LOAD("mpr-23577.ic38" , 0x1000000, 0x1000000, CRC(d007680d) SHA1(a795057c40b1851adb0e19e5dfb39e16206215bf) )
	ROM_LOAD("mpr-23574.ic41" , 0x2000000, 0x1000000, CRC(a77034a5) SHA1(e6e8e2f747e7a972144436103741acfd7030fe84) )
	ROM_LOAD("mpr-23578.ic42" , 0x3000000, 0x1000000, CRC(db612dd6) SHA1(e6813a1e16099094d67347027e058be582750ad7) )
	ROM_LOAD("mpr-23575.ic45" , 0x4000000, 0x1000000, CRC(fe660f06) SHA1(73916f67d852df719fd65b1ed0f8b977c0c33390) )
	ROM_LOAD("mpr-23579.ic46" , 0x5000000, 0x1000000, CRC(55e656d2) SHA1(5d0b26807cf915ab0ae5cc3a7c9dd6bec43da7b2) )
	ROM_LOAD("mpr-23576.ic49" , 0x6000000, 0x1000000, CRC(c01e0329) SHA1(df1a3c83f338925d69912af56f675197e14e1793) )
	ROM_LOAD("mpr-23580.ic50" , 0x7000000, 0x1000000, CRC(d260f39c) SHA1(e5cdf399defaaa7dbcee62f7ab64b898c28d8f7d) )
	/* ic53 unpopulated */
	/* ic54 unpopulated */
	/* ic57 unpopulated */
	/* ic58 unpopulated */
	/* ic61 unpopulated */
	/* ic62 unpopulated */
	/* ic65 unpopulated */
	/* ic66 unpopulated */
ROM_END


ROM_START( pharrier )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x2000000, "user1", 0)
	ROM_LOAD32_WORD("epr23565a.ic29", 0x0000000, 0x0400000, CRC(ca9af8a7) SHA1(e7d6badc03ec5833ee89e49dd389ee19b45da29c) )
	ROM_LOAD32_WORD("epr23566a.ic30", 0x0000002, 0x0400000, CRC(aad0057c) SHA1(c18c0f1797432c74dc21bcd806cb5760916e4936) )
	ROM_LOAD32_WORD("epr23567.ic31",  0x0800000, 0x0400000, CRC(f0e3dcdc) SHA1(422978a13e39f439da54e43a65dcad1a5b1f2f27) )
	ROM_LOAD32_WORD("epr23568.ic32",  0x0800002, 0x0400000, CRC(6eee734c) SHA1(0941761b1690ad4eeac0bf682459992c6f38a930) )
	ROM_LOAD32_WORD("epr23569.ic33",  0x1000000, 0x0400000, CRC(867c7064) SHA1(5cf0d88a1c739ba69b33f1ba3a0e5544331f63f3) )
	ROM_LOAD32_WORD("epr23570.ic34",  0x1000002, 0x0400000, CRC(556ff58b) SHA1(7eb527aee823d037d1045d850427efa42d5da787) )
	ROM_LOAD32_WORD("epr23571.ic35",  0x1800000, 0x0400000, CRC(5a75fa92) SHA1(b5e0c8c995ecc954b74d5eb36f3ae2a732a5986b) )
	ROM_LOAD32_WORD("epr23572.ic36",  0x1800002, 0x0400000, CRC(46054067) SHA1(449800bdc2c40c76aed9bc5e7e8831d8f03ef286) )

	/* ROM board using 128M TSOP48 MASKROMs */
	ROM_REGION( 0x10000000, "user2", 0)
        ROM_LOAD( "mpr-23549.ic37", 0x0000000, 0x1000000, CRC(ed764200) SHA1(ad840a40347345f72a443f284b1bb0ae2b37f7ac) )
        ROM_LOAD( "mpr-23553.ic38", 0x1000000, 0x1000000, CRC(5e70ae78) SHA1(2ae6bdb5aa1434bb60b2b9bca7af12d6476cd35f) )
        ROM_LOAD( "mpr-23550.ic41", 0x2000000, 0x1000000, CRC(841b4d3b) SHA1(d442078b6b4926e6e32b911d88a4408d20a8f0df) )
        ROM_LOAD( "mpr-23554.ic42", 0x3000000, 0x1000000, CRC(5cce99de) SHA1(c39330e4bcfb4cec8b0b59ab184fad5093188765) )
        ROM_LOAD( "mpr-23551.ic45", 0x4000000, 0x1000000, CRC(71f61d04) SHA1(6f24f82ddc5aaf9bbb41b8baddbcb855f1d37a16) )
        ROM_LOAD( "mpr-23555.ic46", 0x5000000, 0x1000000, CRC(582e5453) SHA1(cf9fb8b52a169446b98630d67cdce745de917edc) )
        ROM_LOAD( "mpr-23552.ic49", 0x6000000, 0x1000000, CRC(32487181) SHA1(a885747428c280f77dd861bf802d953da133ef59) )
        ROM_LOAD( "mpr-23556.ic50", 0x7000000, 0x1000000, CRC(45002955) SHA1(85a27c86692ca79fc4e51a64af63a5e970b86cfa) )
        ROM_LOAD( "mpr-23557.ic53", 0x8000000, 0x1000000, CRC(c20dff1b) SHA1(d90d3d85f4fddf39c109502c8f9e9f25a7fc43d1) )
        ROM_LOAD( "mpr-23561.ic54", 0x9000000, 0x1000000, CRC(01237844) SHA1(7a8c6bfdea1d4db5e9f6850fdf1a03d703df3958) )
        ROM_LOAD( "mpr-23558.ic57", 0xa000000, 0x1000000, CRC(e93cc8d7) SHA1(05fc23b8382daaca7ccd1ca80e7c5e93cbf2b6b1) )
        ROM_LOAD( "mpr-23562.ic58", 0xb000000, 0x1000000, CRC(85e0816c) SHA1(28106404d1eef4c85dd425d3535a53c5d71e47a0) )
        ROM_LOAD( "mpr-23559.ic61", 0xc000000, 0x1000000, CRC(1a7f2ba0) SHA1(e2a20138f21297f5313f5368ef9992da8fa23937) )
        ROM_LOAD( "mpr-23563.ic62", 0xd000000, 0x1000000, CRC(e3dc328b) SHA1(d04ccc4025442c98b96f84c1b300671f3687ec6c) )
        ROM_LOAD( "mpr-23560.ic65", 0xe000000, 0x1000000, CRC(24bb7072) SHA1(dad5135c89d292e4a1f96bd0ad28be6a17154be0) )
        ROM_LOAD( "mpr-23564.ic66", 0xf000000, 0x1000000, CRC(255724b6) SHA1(1b382fad165831de3f2e39352c031146759dfc69) )
ROM_END

ROM_START( podrace )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x2000000, "user1", 0)
	ROM_LOAD32_WORD("epr-23174.ic29", 0x0000000, 0x0400000, CRC(eae62b46) SHA1(f1458072b002d64bbb7c43c582e3191e8031e19a) )
	ROM_LOAD32_WORD("epr-23175.ic30", 0x0000002, 0x0400000, CRC(b92da060) SHA1(dd9ecbd0977aef7629441ff45f4ad807b2408603) )
	ROM_LOAD32_WORD("epr-23176.ic31", 0x0800000, 0x0400000, CRC(2f2824a7) SHA1(a375719e3cababab5b33d00d8696c7cd62c5af30) )
	ROM_LOAD32_WORD("epr-23177.ic32", 0x0800002, 0x0400000, CRC(7a5e3f0f) SHA1(e8ca00cfaaa9be4f9d269e4d8f6bcbbd7de8f6d6) )
	/* ic33 unpopulated */
	/* ic34 unpopulated */
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 64M SOP44 MASKROM */
	ROM_REGION( 0x10000000, "user2", 0)
	ROM_LOAD("mpr-23086.ic37" , 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23087.ic38" , 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23088.ic39" , 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23089.ic40" , 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23090.ic41" , 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23091.ic42" , 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23092.ic43" , 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23093.ic44" , 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23094.ic45" , 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23095.ic46" , 0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23096.ic47" , 0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23097.ic48" , 0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23098.ic49" , 0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23099.ic50" , 0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23100.ic51" , 0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23101.ic52" , 0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23102.ic53" , 0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23103.ic54" , 0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23104.ic55" , 0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23105.ic56" , 0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23106.ic57" , 0xa000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23107.ic58" , 0xa800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23108.ic59" , 0xb000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23109.ic60" , 0xb800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23110.ic61" , 0xc000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23111.ic62" , 0xc800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23112.ic63" , 0xd000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23113.ic64" , 0xd800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23114.ic65" , 0xe000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23115.ic66" , 0xe800000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23116.ic67" , 0xf000000, 0x0800000, NO_DUMP )
	ROM_LOAD("mpr-23117.ic68" , 0xf800000, 0x0800000, NO_DUMP )
ROM_END

GAME( 2000, hikaru,   0,        hikaru,   hikaru,   0, ROT0, "Sega",            "Hikaru Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2000, airtrix,  hikaru,   hikaru,   hikaru,   0, ROT0, "Sega",            "Air Trix", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2001, pharrier, hikaru,   hikaru,   hikaru,   0, ROT0, "Sega",            "Planet Harriers", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2001, podrace,  hikaru,   hikaru,   hikaru,   0, ROT0, "Sega",            "Star Wars Pod Racer", GAME_NO_SOUND|GAME_NOT_WORKING )
