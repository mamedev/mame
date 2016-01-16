// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Hikaru / 'Samurai' */

/*

Sega Hikaru Hardware Overview (last updated 5th August 2009 at 3:45pm)
-----------------------------

Note! This document will be updated from time to time when more dumps are available.

This document covers all the known Sega Hikaru games. The graphics are quite breathtaking
and this system is said to be one of the most expensive arcade boards developed by Sega.
The games on this system include....
Air Trix                     (C) Sega, 2001
Brave Fire Fighters          (C) Sega, 1999
*Cyber Troopers Virtual On 4 (C) Sega, 2001
Nascar Racing                (C) Sega, 2000
Planet Harriers              (C) Sega, 2001
Star Wars Racer Arcade       (C) Sega, 2000

! - denotes secured but not fully dumped yet
* - denotes not dumped yet.

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

/*
    New hardware notes from Stefano Teso:

  - The master SH-4 port A: the bit layout for this thing is (MSB left)

    xxxx xxii iiee eexM

    x = unused
    i = IRQ causes (active low)
    e = Mainboard EEPROM (active low)
    M = connected to the slave SH-4 NMI pin; writing a specific bit
    pattern here (3 ones, 3 zeros, 7 ones) sends an NMI to the slave

    Bit 1 of the slave port A is used for master-slave communication as
    well, but I have yet to figure out how. It doesn't seem to be needed
    for emulation at this point though. Master-slave communication seems
    to work okay after emulating the M bit here.

  - There's an additional indirect-DMA-like device on the GPU at
    150000(0C,10,14). The table indicating where the data is, how long it
    is, and how to operate on it, is located at the bottom of GPU command
    RAM (14xxxxxx), defaulting at 143FC000 (whose bus address, specified
    by 1500000C is 483FC000.) It is used for (my guess) on-the-fly texture
    format conversion. Upon termination, it raises a GPU IRQ. It looks
    like airtrix first uploads the data to be converted somewhere using
    the memory controller's DMA, then instructs the GPU indirect DMA thing
    to read it and perform the conversion.

  - GPU IRQs: it looks like the GPU IRQs are as follows:

    15000088:

    bit 0: the GPU indirect-DMA-like device is done
    bit 1: vblank (or hblank)
    bit 2: the GPU at 15xxxxxx is done and needs feeding (means that
    commands are uploaded to CMDRAM and registers fiddled with, see bit 2
    below.)
    bit 8: any bit of 1A000018 is set

    1A000018:

    bit 1: vblank (or hblank)
    bit 2: GPU at 1Axxxxxx is done and needs feeding (same as bit 2
    above; note that both IRQs must be raised -- not necessarily at the
    same time -- for the whole "let's upload data to the GPU" routine to
    be performed.)

    My gut feeling tells me that there are two different GPUs: one
    processes the command stream and does the rendering on even frames,
    the other does the same for odd frames. Registers 1500007x may specify
    where the command stream start is (the initial GPU PC) and possibly
    the location of two distinct stack pointers (the GPU supports
    subroutines, after all.)

  - the serial device at 0080000(A|C) is not an EEPROM; it's likely some
    weird device that is used to query the hierarchy of the attached
    input/output devices. Probably the naomi has something similar, I
    haven't looked into it yet.

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"

#define CPU_CLOCK (200000000)
									/* MD2 MD1 MD0 MD6 MD4 MD3 MD5 MD7 MD8 */
//static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };

class hikaru_state : public driver_device
{
public:
	hikaru_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }
	virtual void video_start() override;
	UINT32 screen_update_hikaru(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

void hikaru_state::video_start()
{
}

UINT32 hikaru_state::screen_update_hikaru(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( hikaru )
	PORT_START("IN0")
INPUT_PORTS_END

/*
 Area 0
  00000000-00200000    boot ROM
  00400000-00400003    ?
  00800000-0083ffff    MIE + Service/Test switches and more
  00c00000-00c0ffff    backup RAM
  01000000-01000003    ?
  01000100-01000103    ?
  02000000-02ffffff    banked area (ROMBD+AICA+COMM+other devices)
  03000000-03ffffff    banked area (ROMBD+EEPROM+COMM+other devices)
 Area 1
  04000000-0400003f    Memory controller (Master)
 Area 3
  0c000000-0dffffff    RAM
 Area 5
  14000000-140000ff    Master/Slave COMM
  14000100-143fffff    GPU command RAM
  15000000-150000ff    GPU Regs
  16000000-163fffff x2 ? \ these two overlap [selected by 040000xx = 0x04,0x06,0x40]
  16010000-17ffffff    Slave RAM /
 Area 6
  18001000-1800101f    ?
  1a000000-1a000103    GPU Regs
  1a000180-150001bf    GPU Texture Regs A
  1a000200-1500023f    GPU Texture Regs B
  1a040000-1a04000f    GPU Texture fIfO (?)
  1b000000-1b7fffff    GPU Texture RAM and framebuffer (a 2048x2048x16-bit sheet?)

*/

static ADDRESS_MAP_START( hikaru_map, AS_PROGRAM, 64, hikaru_state )
//  Area 0
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_SHARE("share1")  // boot ROM
	AM_RANGE(0x00400000, 0x00400007) AM_NOP // unknown
	AM_RANGE(0x00800000, 0x0083ffff) AM_NOP // MIE + Service/Test switches and more
	AM_RANGE(0x00c00000, 0x00c0ffff) AM_RAM // backup RAM
	AM_RANGE(0x01000000, 0x01000007) AM_NOP // unknown
	AM_RANGE(0x01000100, 0x01000107) AM_NOP // unknown
	AM_RANGE(0x02000000, 0x02ffffff) AM_NOP // banked area (ROMBD + AICA + COMM + other devices)
	AM_RANGE(0x03000000, 0x03ffffff) AM_NOP // banked area (ROMBD + EEPROM + COMM + other devices)
//  Area 1
	AM_RANGE(0x04000000, 0x0400003f) AM_NOP // memory controller (Master)
//  Area 3
	AM_RANGE(0x0c000000, 0x0dffffff) AM_RAM // main Work RAM
//  Area 5
	AM_RANGE(0x14000000, 0x140000ff) AM_NOP // Master/Slave COMM
	AM_RANGE(0x14000100, 0x143fffff) AM_RAM // GPU command RAM
	AM_RANGE(0x15000000, 0x150000ff) AM_NOP // GPU Regs
	AM_RANGE(0x16001000, 0x163fffff) AM_RAM // ? \ these two overlap [selected by 040000xx = 0x04,0x06,0x40]
	AM_RANGE(0x16010000, 0x17ffffff) AM_RAM // Slave Work RAM
//  Area 6
	AM_RANGE(0x18001000, 0x1800101f) AM_NOP // unknown
	AM_RANGE(0x1a000000, 0x1a000107) AM_NOP // GPU Regs
	AM_RANGE(0x1a000180, 0x1a0001bf) AM_NOP // GPU Texture Regs A
	AM_RANGE(0x1a000200, 0x1a00023f) AM_NOP // GPU Texture Regs B
	AM_RANGE(0x1a040000, 0x1a04000f) AM_NOP // GPU Texture FIFO (?)
	AM_RANGE(0x1b000000, 0x1b7fffff) AM_NOP // GPU Texture RAM and framebuffer (a 2048x2048x16-bit sheet?)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hikaru_map_slave, AS_PROGRAM, 64, hikaru_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x001FFFFF) AM_ROM AM_SHARE("share1")
	AM_RANGE(0x0C000000, 0x0DFFFFFF) AM_RAM
	AM_RANGE(0x10000000, 0x100000FF) AM_RAM
	AM_RANGE(0x1A800000, 0x1A8000FF) AM_RAM
	AM_RANGE(0x1B000000, 0x1B0001FF) AM_RAM
ADDRESS_MAP_END


static MACHINE_CONFIG_START( hikaru, hikaru_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, CPU_CLOCK)
//  MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(hikaru_map)
//  MCFG_CPU_IO_MAP(hikaru_port)
//  MCFG_CPU_VBLANK_INT("screen", hikaru,vblank)
	MCFG_CPU_ADD("slave", SH4LE, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(hikaru_map_slave)

//  MCFG_MACHINE_START_OVERRIDE(hikaru_state, hikaru )
//  MCFG_MACHINE_RESET_OVERRIDE(hikaru_state, hikaru )

//  MCFG_NVRAM_HANDLER(hikaru_eeproms)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(hikaru_state, screen_update_hikaru)

	MCFG_PALETTE_ADD("palette", 0x1000)


//  MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
//  MCFG_SOUND_ADD("aica", AICA, 0)
//  MCFG_SOUND_CONFIG(aica_config)
//  MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
//  MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)
MACHINE_CONFIG_END


#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */


#define HIKARU_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr23400a" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-23400a.ic94",  0x000000, 0x200000, CRC(2aa906a7) SHA1(098c9909b123ed6c338ac874f2ee90e3b2da4c02) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr23400" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-23400.ic94",   0x000000, 0x200000, CRC(3d557104) SHA1(d39879f5a1acbd54ad8ee4fbd412f870c9ff4aa5) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr21904" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-21904.ic94",   0x000000, 0x200000, CRC(d96298b6) SHA1(d10d837bc7d68eb7125c34beffe21a91305627b0) )
// bios 0 is SAMURAI boot rom 0.96 / 2000/8/10
// bios 1 is SAMURAI boot rom 0.92 / 1999/7/2
// bios 2 is SAMURAI boot rom 0.84 / 1999/7/22


ROM_START( hikaru )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x400000, "user1", ROMREGION_ERASE)
ROM_END


ROM_START( airtrix )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x800000, "user1", 0)
	ROM_LOAD32_WORD( "epr-23601a.ic29", 0x0000000, 0x0400000, CRC(cd3ccc05) SHA1(49de32d3588511f37486aff900773453739d706d) )
	ROM_LOAD32_WORD( "epr-23602a.ic30", 0x0000002, 0x0400000, CRC(24f1bca9) SHA1(719dc4e003c1d13fcbb39604c156c89042c47dfd) )
	/* ic31 unpopulated */
	/* ic32 unpopulated */
	/* ic33 unpopulated */
	/* ic34 unpopulated */
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 128M TSOP48 MASKROMs */
	ROM_REGION( 0x10000000, "user2", 0)
	ROM_LOAD32_WORD( "mpr-23573.ic37" , 0x0000000, 0x1000000, CRC(e22a0734) SHA1(fc06d5972d285d09473874aaeb1efed2d19c8f36) )
	ROM_LOAD32_WORD( "mpr-23577.ic38" , 0x0000002, 0x1000000, CRC(d007680d) SHA1(a795057c40b1851adb0e19e5dfb39e16206215bf) )
	ROM_LOAD32_WORD( "mpr-23574.ic41" , 0x2000000, 0x1000000, CRC(a77034a5) SHA1(e6e8e2f747e7a972144436103741acfd7030fe84) )
	ROM_LOAD32_WORD( "mpr-23578.ic42" , 0x2000002, 0x1000000, CRC(db612dd6) SHA1(e6813a1e16099094d67347027e058be582750ad7) )
	ROM_LOAD32_WORD( "mpr-23575.ic45" , 0x4000000, 0x1000000, CRC(fe660f06) SHA1(73916f67d852df719fd65b1ed0f8b977c0c33390) )
	ROM_LOAD32_WORD( "mpr-23579.ic46" , 0x4000002, 0x1000000, CRC(55e656d2) SHA1(5d0b26807cf915ab0ae5cc3a7c9dd6bec43da7b2) )
	ROM_LOAD32_WORD( "mpr-23576.ic49" , 0x6000000, 0x1000000, CRC(c01e0329) SHA1(df1a3c83f338925d69912af56f675197e14e1793) )
	ROM_LOAD32_WORD( "mpr-23580.ic50" , 0x6000002, 0x1000000, CRC(d260f39c) SHA1(e5cdf399defaaa7dbcee62f7ab64b898c28d8f7d) )
	/* ic53 unpopulated */
	/* ic54 unpopulated */
	/* ic57 unpopulated */
	/* ic58 unpopulated */
	/* ic61 unpopulated */
	/* ic62 unpopulated */
	/* ic65 unpopulated */
	/* ic66 unpopulated */

	// 834-14149   2000     317-0294-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "291b02c7" )
ROM_END


ROM_START( pharrier )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x2000000, "user1", 0)
	ROM_LOAD32_WORD("epr-23565a.ic29", 0x0000000, 0x0400000, CRC(ca9af8a7) SHA1(e7d6badc03ec5833ee89e49dd389ee19b45da29c) )
	ROM_LOAD32_WORD("epr-23566a.ic30", 0x0000002, 0x0400000, CRC(aad0057c) SHA1(c18c0f1797432c74dc21bcd806cb5760916e4936) )
	ROM_LOAD32_WORD("epr-23567.ic31",  0x0800000, 0x0400000, CRC(f0e3dcdc) SHA1(422978a13e39f439da54e43a65dcad1a5b1f2f27) )
	ROM_LOAD32_WORD("epr-23568.ic32",  0x0800002, 0x0400000, CRC(6eee734c) SHA1(0941761b1690ad4eeac0bf682459992c6f38a930) )
	ROM_LOAD32_WORD("epr-23569.ic33",  0x1000000, 0x0400000, CRC(867c7064) SHA1(5cf0d88a1c739ba69b33f1ba3a0e5544331f63f3) )
	ROM_LOAD32_WORD("epr-23570.ic34",  0x1000002, 0x0400000, CRC(556ff58b) SHA1(7eb527aee823d037d1045d850427efa42d5da787) )
	ROM_LOAD32_WORD("epr-23571.ic35",  0x1800000, 0x0400000, CRC(5a75fa92) SHA1(b5e0c8c995ecc954b74d5eb36f3ae2a732a5986b) )
	ROM_LOAD32_WORD("epr-23572.ic36",  0x1800002, 0x0400000, CRC(46054067) SHA1(449800bdc2c40c76aed9bc5e7e8831d8f03ef286) )

	/* ROM board using 128M TSOP48 MASKROMs */
	ROM_REGION( 0x10000000, "user2", 0)
	ROM_LOAD32_WORD( "mpr-23549.ic37", 0x0000000, 0x1000000, CRC(ed764200) SHA1(ad840a40347345f72a443f284b1bb0ae2b37f7ac) )
	ROM_LOAD32_WORD( "mpr-23553.ic38", 0x0000002, 0x1000000, CRC(5e70ae78) SHA1(2ae6bdb5aa1434bb60b2b9bca7af12d6476cd35f) )
	ROM_LOAD32_WORD( "mpr-23550.ic41", 0x2000000, 0x1000000, CRC(841b4d3b) SHA1(d442078b6b4926e6e32b911d88a4408d20a8f0df) )
	ROM_LOAD32_WORD( "mpr-23554.ic42", 0x2000002, 0x1000000, CRC(5cce99de) SHA1(c39330e4bcfb4cec8b0b59ab184fad5093188765) )
	ROM_LOAD32_WORD( "mpr-23551.ic45", 0x4000000, 0x1000000, CRC(71f61d04) SHA1(6f24f82ddc5aaf9bbb41b8baddbcb855f1d37a16) )
	ROM_LOAD32_WORD( "mpr-23555.ic46", 0x4000002, 0x1000000, CRC(582e5453) SHA1(cf9fb8b52a169446b98630d67cdce745de917edc) )
	ROM_LOAD32_WORD( "mpr-23552.ic49", 0x6000000, 0x1000000, CRC(32487181) SHA1(a885747428c280f77dd861bf802d953da133ef59) )
	ROM_LOAD32_WORD( "mpr-23556.ic50", 0x6000002, 0x1000000, CRC(45002955) SHA1(85a27c86692ca79fc4e51a64af63a5e970b86cfa) )
	ROM_LOAD32_WORD( "mpr-23557.ic53", 0x8000000, 0x1000000, CRC(c20dff1b) SHA1(d90d3d85f4fddf39c109502c8f9e9f25a7fc43d1) )
	ROM_LOAD32_WORD( "mpr-23561.ic54", 0x8000002, 0x1000000, CRC(01237844) SHA1(7a8c6bfdea1d4db5e9f6850fdf1a03d703df3958) )
	ROM_LOAD32_WORD( "mpr-23558.ic57", 0xa000000, 0x1000000, CRC(e93cc8d7) SHA1(05fc23b8382daaca7ccd1ca80e7c5e93cbf2b6b1) )
	ROM_LOAD32_WORD( "mpr-23562.ic58", 0xa000002, 0x1000000, CRC(85e0816c) SHA1(28106404d1eef4c85dd425d3535a53c5d71e47a0) )
	ROM_LOAD32_WORD( "mpr-23559.ic61", 0xc000000, 0x1000000, CRC(1a7f2ba0) SHA1(e2a20138f21297f5313f5368ef9992da8fa23937) )
	ROM_LOAD32_WORD( "mpr-23563.ic62", 0xc000002, 0x1000000, CRC(e3dc328b) SHA1(d04ccc4025442c98b96f84c1b300671f3687ec6c) )
	ROM_LOAD32_WORD( "mpr-23560.ic65", 0xe000000, 0x1000000, CRC(24bb7072) SHA1(dad5135c89d292e4a1f96bd0ad28be6a17154be0) )
	ROM_LOAD32_WORD( "mpr-23564.ic66", 0xe000002, 0x1000000, CRC(255724b6) SHA1(1b382fad165831de3f2e39352c031146759dfc69) )

	// 834-14144   2001     317-0297-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "2912c68a" )
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
	ROM_LOAD32_WORD("mpr-23086.ic37" ,  0x0000000, 0x0800000, CRC(ef6f20f1) SHA1(11fb66bf71223b4c6650d3adaea21e8709b8d67b))
	ROM_LOAD32_WORD("mpr-23087.ic38" ,  0x0000002, 0x0800000, CRC(54389822) SHA1(6357f0aa77ef0a5a08a751e085fa026d26ba47d1))
	ROM_LOAD32_WORD("mpr-23088.ic39" ,  0x1000000, 0x0800000, CRC(9f1a382e) SHA1(b846c3a091d04e49cc1e731237c9326ccac39a64))
	ROM_LOAD32_WORD("mpr-23089.ic40" ,  0x1000002, 0x0800000, CRC(6aae64fc) SHA1(392b6fba25d20bb41fd72be3a3a9ce95b2374065))
	ROM_LOAD32_WORD("mpr-23090.ic41" ,  0x2000000, 0x0800000, CRC(ba857872) SHA1(c07ff7955d3d07f2a60d9761b4bd692c0a9c9353))
	ROM_LOAD32_WORD("mpr-23091.ic42" ,  0x2000002, 0x0800000, CRC(66a73e27) SHA1(c4e7d190a80499225a78b7f788c2abc7ec4ebdca))
	ROM_LOAD32_WORD("mpr-23092.ic43" ,  0x3000000, 0x0800000, CRC(4f20a0f5) SHA1(0580feba6a6dd01a21d09ec2503ccf77030f8d2a))
	ROM_LOAD32_WORD("mpr-23093.ic44" ,  0x3000002, 0x0800000, CRC(e74d7d64) SHA1(c28e44319bf08aedd9aed625a12834ec76f1e5e0))
	ROM_LOAD32_WORD("mpr-23094.ic45" ,  0x4000000, 0x0800000, CRC(90f04c14) SHA1(b55846ea1edd920fd527e3257b13fea8df1f713f))
	ROM_LOAD32_WORD("mpr-23095.ic46" ,  0x4000002, 0x0800000, CRC(cc67cb5b) SHA1(85e99ec22d1c65139685a94f1ba0c52a0eb33a2e))
	ROM_LOAD32_WORD("mpr-23096.ic47" ,  0x5000000, 0x0800000, CRC(799ab79e) SHA1(c0ac85ad7f4cf46ff162f1ec2e85a3f22817de5e))
	ROM_LOAD32_WORD("mpr-23097.ic48" ,  0x5000002, 0x0800000, CRC(f68439de) SHA1(475d0f22e78e3c86431b742e37cbfd764ca8acee))
	ROM_LOAD32_WORD("mpr-23098.ic49" ,  0x6000000, 0x0800000, CRC(a1e2009c) SHA1(c6a600d47fd2a96d28c637631862150e6f303c3d))
	ROM_LOAD32_WORD("mpr-23099.ic50" ,  0x6000002, 0x0800000, CRC(ce36f642) SHA1(6cb2e69095efc7969255ebc637e2597c56442751))
	ROM_LOAD32_WORD("mpr-23100.ic51" ,  0x7000000, 0x0800000, CRC(0f966653) SHA1(1544af662188ea734e0a2e559e05e5f782fb292d))
	ROM_LOAD32_WORD("mpr-23101.ic52" ,  0x7000002, 0x0800000, CRC(2640fbaa) SHA1(59e9bd143734c71968beb9953122680d3350e69c))
	ROM_LOAD32_WORD("mpr-23102.ic53s" , 0x8000000, 0x0800000, CRC(080c5bcb) SHA1(0cf54348420ae9866edd64422cb82464990f1f2f))
	ROM_LOAD32_WORD("mpr-23103.ic54s" , 0x8000002, 0x0800000, CRC(19c7758f) SHA1(fed7f45dd91e1cb6bba7d8e80ed17dca27d92e43))
	ROM_LOAD32_WORD("mpr-23104.ic55s" , 0x9000000, 0x0800000, CRC(4ca74216) SHA1(0e65971359ba0e2b4fc032a26d1c10d8efadc205))
	ROM_LOAD32_WORD("mpr-23105.ic56s" , 0x9000002, 0x0800000, CRC(e2dd35ba) SHA1(2213e3195a49532a177086de8134ce8b753fc7ce))
	ROM_LOAD32_WORD("mpr-23106.ic57s" , 0xa000000, 0x0800000, CRC(dd325515) SHA1(8144e1a87f7d72a18791d1d452123a91cfb354dd))
	ROM_LOAD32_WORD("mpr-23107.ic58s" , 0xa000002, 0x0800000, CRC(a527a22a) SHA1(54c105b21797c9b0a2a6b2c7091de726c49a55e8))
	ROM_LOAD32_WORD("mpr-23108.ic59s" , 0xb000000, 0x0800000, CRC(47817d9a) SHA1(d2c6f1b2e800448eaf694d550733bba2280b6746))
	ROM_LOAD32_WORD("mpr-23109.ic60s" , 0xb000002, 0x0800000, CRC(8c61dec4) SHA1(25a1a5b236b3aed013fc94bd9695906ae5d7f305))
	ROM_LOAD32_WORD("mpr-23110.ic61s" , 0xc000000, 0x0800000, CRC(4ddae9f1) SHA1(d1c5e3f18932af806f166779cf14909ab17d052c))
	ROM_LOAD32_WORD("mpr-23111.ic62s" , 0xc000002, 0x0800000, CRC(c404cb1c) SHA1(e14855ec8a5a5ba243a2339c571928fdcc187157))
	ROM_LOAD32_WORD("mpr-23112.ic63s" , 0xd000000, 0x0800000, CRC(d001fe59) SHA1(ab395d2933b5d691259221168dfaa063cf9a4d1c))
	ROM_LOAD32_WORD("mpr-23113.ic64s" , 0xd000002, 0x0800000, CRC(f241cfd5) SHA1(5b85e8b50559becff7a76565c95487825bbd9351))
	ROM_LOAD32_WORD("mpr-23114.ic65s" , 0xe000000, 0x0800000, CRC(80049d7c) SHA1(56aab53e9317b1b5d10bd2af78fd83e7422d8939))
	ROM_LOAD32_WORD("mpr-23115.ic66s" , 0xe000002, 0x0800000, CRC(4fc540fe) SHA1(df580421d856566e067c2b319c8ac4671629682f))
	ROM_LOAD32_WORD("mpr-23116.ic67s" , 0xf000000, 0x0800000, CRC(9f567fce) SHA1(c35bcf968f139557e50ceafa9c6bad4deb87154f))
	ROM_LOAD32_WORD("mpr-23117.ic68s" , 0xf000002, 0x0800000, CRC(9d4d3529) SHA1(66008445629681ebf2f26b3f181d8524a8576d2f))

	// 834-14002   2001     317-0277-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "2903dad5" )
ROM_END

ROM_START( braveff )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x2000000, "user1", 0)
	ROM_LOAD32_WORD( "epr-21994.ic29", 0x0000000, 0x200000, CRC(31b0a754) SHA1(b49c998a15fbc790b780ed6665a56681d4edd369) )
	ROM_LOAD32_WORD( "epr-21995.ic30", 0x0000002, 0x200000, CRC(bcccb56b) SHA1(6e7a69934e5b47495ae8e90c57759573bc519d24) )
	ROM_LOAD32_WORD( "epr-21996.ic31", 0x0800000, 0x200000, CRC(a8f88e17) SHA1(dbbd2a73335c740bcf2ff9680c575841af29b340) )
	ROM_LOAD32_WORD( "epr-21997.ic32", 0x0800002, 0x200000, CRC(36641a7f) SHA1(37931bde1ddebef61fa6d8caca3cb67328fd0b90) )
	ROM_LOAD32_WORD( "epr-21998.ic33", 0x1000000, 0x200000, CRC(bd1df696) SHA1(fd937894763fab5cb50f33c40f8047e0d3adc93b) )
	ROM_LOAD32_WORD( "epr-21999.ic34", 0x1000002, 0x200000, CRC(9425eee0) SHA1(0f6a23163022bbd7ec54dd638094f3e317a87919) )
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 64M SOP44 MASKROM */
	ROM_REGION( 0xc000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-22000.ic37",  0x0000000, 0x800000, CRC(53d641d6) SHA1(f47d7c77d0e36c4ec3b7171fd7a017f9f58ca5a0) )
	ROM_LOAD32_WORD( "mpr-22001.ic38",  0x0000002, 0x800000, CRC(234bc48f) SHA1(177c46884de0ba4bac1f9b778f99c905410a9345) )
	ROM_LOAD32_WORD( "mpr-22002.ic39",  0x1000000, 0x800000, CRC(d8f3aa9e) SHA1(f73208034fdd51fed086e912cb8580d2270122b6) )
	ROM_LOAD32_WORD( "mpr-22003.ic40",  0x1000002, 0x800000, CRC(2560fe98) SHA1(9bb5ffb6212ec6aa3f92e437eb424141f3b15e43) )
	ROM_LOAD32_WORD( "mpr-22004.ic41",  0x2000000, 0x800000, CRC(4e24d71d) SHA1(503344dd8cdd8e65ec7c801b0efae83b3f1f9ae2) )
	ROM_LOAD32_WORD( "mpr-22005.ic42",  0x2000002, 0x800000, CRC(2b96c97f) SHA1(707070c85f4b044236694daa13970c241b242d4d) )
	ROM_LOAD32_WORD( "mpr-22006.ic43",  0x3000000, 0x800000, CRC(f793a3ba) SHA1(80acd1d4f71cafd7328ff9b9ce30e5169b8f4f8c) )
	ROM_LOAD32_WORD( "mpr-22007.ic44",  0x3000002, 0x800000, CRC(62616e31) SHA1(dbe0d4b8fc085ed97884c105fd527af5cd8fbe79) )
	ROM_LOAD32_WORD( "mpr-22008.ic45",  0x4000000, 0x800000, CRC(e6905de8) SHA1(6bb4e43b1394788add15f0b78ccd5ab14f86516c) )
	ROM_LOAD32_WORD( "mpr-22009.ic46",  0x4000002, 0x800000, CRC(c37dfa5c) SHA1(5a3a5f2eb5a13831e36ca215147ec3c9740c50fc) )
	ROM_LOAD32_WORD( "mpr-22010.ic47",  0x5000000, 0x800000, CRC(b570b46c) SHA1(6e512fd1a2c8835f6aee307865b42d57ddf90ef5) )
	ROM_LOAD32_WORD( "mpr-22011.ic48",  0x5000002, 0x800000, CRC(d1f5fb58) SHA1(08a1282e00bda52d8d938225c65f67d22abfea05) )
	ROM_LOAD32_WORD( "mpr-22012.ic49",  0x6000000, 0x800000, CRC(3ab79029) SHA1(d4708446ba700d5f7c89827c80177ad2d1c0b222) )
	ROM_LOAD32_WORD( "mpr-22013.ic50",  0x6000002, 0x800000, CRC(42d8d00b) SHA1(ddce3c95258d8cf51792f2115f89ca658ffe97b6) )
	ROM_LOAD32_WORD( "mpr-22014.ic51",  0x7000000, 0x800000, CRC(0f49c00f) SHA1(877c654268edc9526ae3e21e21e3ecca706f300b) )
	ROM_LOAD32_WORD( "mpr-22015.ic52",  0x7000002, 0x800000, CRC(d3696e61) SHA1(247161c99c7061b8f391543af1812764a82399cb) )
	ROM_LOAD32_WORD( "mpr-22016.ic53s", 0x8000000, 0x800000, CRC(c1015e00) SHA1(f2ce2009d4f4f0f3cbfcce7a36fab2c54e738b07) )
	ROM_LOAD32_WORD( "mpr-22017.ic54s", 0x8000002, 0x800000, CRC(222a7cb0) SHA1(9f98ae3f13f85fae4596b671ea508b07c2116ab6) )
	ROM_LOAD32_WORD( "mpr-22018.ic55s", 0x9000000, 0x800000, CRC(f160e115) SHA1(ecf7f9f58fce6bff220568972ba7763537c9d7d7) )
	ROM_LOAD32_WORD( "mpr-22019.ic56s", 0x9000002, 0x800000, CRC(468b2f10) SHA1(f3fc0af7d4dd3f30ba84e684f3d9c217730564bb) )
	ROM_LOAD32_WORD( "mpr-22020.ic57s", 0xa000000, 0x800000, CRC(0c018d8a) SHA1(0447d7ad64061cca4c1231733e660ba51de5a216) )
	ROM_LOAD32_WORD( "mpr-22021.ic58s", 0xa000002, 0x800000, CRC(43b08604) SHA1(681142d8b95b2f9664d70b23262a64938774d4e3) )
	ROM_LOAD32_WORD( "mpr-22022.ic59s", 0xb000000, 0x800000, CRC(abd3d888) SHA1(9654c3a38feab46b4983a602831fb29cccdd0526) )
	ROM_LOAD32_WORD( "mpr-22023.ic60s", 0xb000002, 0x800000, CRC(07f00869) SHA1(92282d09d72d3e65a91128e06bb0d4426bb90be5) )

	ROM_PARAMETER( ":rom_board:segam2crypt:key", "-1" ) // 315-5881 not populated
ROM_END

ROM_START( sgnascar )
	ROM_REGION( 0x200000, "maincpu", 0)
	HIKARU_BIOS

	ROM_REGION( 0x2000000, "user1", 0)
	ROM_LOAD32_WORD( "epr-23485a.ic35", 0x000000, 0x400000, CRC(1072f531) SHA1(ca07a8bfb7247e4aec57e18cb091d24dcef666c1) )
	ROM_LOAD32_WORD( "epr-23486a.ic36", 0x000002, 0x400000, CRC(02d4aab6) SHA1(b1b0e07dc71dc124177e27dfd8b459444e8ae4d3) )

	/* ROM board using 128M TSOP48 MASKROMs */
	ROM_REGION( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23469.ic19", 0x0000000, 0x1000000, CRC(89cbad8d) SHA1(e4f103b96a3a842a90182172ddcf3bc5dfe6cca8) )
	ROM_LOAD32_WORD( "mpr-23473.ic20", 0x0000002, 0x1000000, CRC(977b87d6) SHA1(079eeebc6f9c60d0a016a46386bbe846d8a354da) )
	ROM_LOAD32_WORD( "mpr-23470.ic21", 0x2000000, 0x1000000, CRC(faf4940f) SHA1(72fee9ea5b78da260ed99ebe80ca6300f62cdbd7) )
	ROM_LOAD32_WORD( "mpr-23474.ic22", 0x2000002, 0x1000000, CRC(faf69ac5) SHA1(875c748151bf0e9cd73d86384665414b2f7b6f5a) )
	ROM_LOAD32_WORD( "mpr-23471.ic23", 0x4000000, 0x1000000, CRC(a3aad8ac) SHA1(afc8f3d1546e50afab4f540d59c87fe27cfb2cdd) )
	ROM_LOAD32_WORD( "mpr-23475.ic24", 0x4000002, 0x1000000, CRC(5f51597c) SHA1(02c0a5d463714082b7ebb2bec4d0f88aff186f82) )
	ROM_LOAD32_WORD( "mpr-23472.ic25", 0x6000000, 0x1000000, CRC(2495f678) SHA1(94b3160aabaea0596855c38ab1b63b16b20f2bae) )
	ROM_LOAD32_WORD( "mpr-23476.ic26", 0x6000002, 0x1000000, CRC(927cf31c) SHA1(7cab22a4113d92080a52e1d235bf075ce95f985f) )
	ROM_LOAD32_WORD( "mpr-23477.ic27", 0x8000000, 0x1000000, CRC(b4b7c477) SHA1(bcbfe081d509f0b87c6685b9b6617ae146987fe7) )
	ROM_LOAD32_WORD( "mpr-23481.ic28", 0x8000002, 0x1000000, CRC(27b8eb7d) SHA1(087b1ed13a3e2a0dbda82c454243214784429d24) )
	ROM_LOAD32_WORD( "mpr-23478.ic29", 0xa000000, 0x1000000, CRC(1fac431c) SHA1(2e3903c8cfd55d414555a1d23ba3a97c335991b3) )
	ROM_LOAD32_WORD( "mpr-23482.ic30", 0xa000002, 0x1000000, CRC(2e9a0420) SHA1(376d5f0b8274d741a702dc08da50ea5679991740) )
	ROM_LOAD32_WORD( "mpr-23479.ic31", 0xc000000, 0x1000000, CRC(9704e393) SHA1(0cb1403f4a268def3ce88db42e55d89ca913e2a0) )
	ROM_LOAD32_WORD( "mpr-23483.ic32", 0xc000002, 0x1000000, CRC(c37adebe) SHA1(e84f6d2cc364c743f7f3b73d8c8d0271952bb093) )
	ROM_LOAD32_WORD( "mpr-23480.ic33", 0xe000000, 0x1000000, CRC(f517b8b3) SHA1(c04740adb612473c4c9f8186e7e93d2f73d1bb1a) )
	ROM_LOAD32_WORD( "mpr-23484.ic34", 0xe000002, 0x1000000, CRC(2ebe1aa1) SHA1(16b39f7422da1a334dde27169c2949e1d95bddb3) )

	// 317-0283-COM Actel A54SX32
	// ID 0x4252
	ROM_PARAMETER( ":rom_board:key", "56dedf33" )
ROM_END

GAME( 2000, hikaru,   0,        hikaru,   hikaru, driver_device,   0, ROT0, "Sega",            "Hikaru Bios", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )
GAME( 1999, braveff,  hikaru,   hikaru,   hikaru, driver_device,   0, ROT0, "Sega",            "Brave Fire Fighters", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, airtrix,  hikaru,   hikaru,   hikaru, driver_device,   0, ROT0, "Sega",            "Air Trix", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, sgnascar, hikaru,   hikaru,   hikaru, driver_device,   0, ROT0, "Sega / Electronic Arts", "NASCAR Racing", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2001, pharrier, hikaru,   hikaru,   hikaru, driver_device,   0, ROT0, "Sega",            "Planet Harriers", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2001, podrace,  hikaru,   hikaru,   hikaru, driver_device,   0, ROT0, "Sega",            "Star Wars Pod Racer", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
