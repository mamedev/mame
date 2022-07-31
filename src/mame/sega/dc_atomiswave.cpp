// license:LGPL-2.1+
// copyright-holders: Samuele Zannoli, R. Belmont, ElSemi, David Haywood, Angelo Salese, Olivier Galibert, MetalliC
// thanks-to: CaH4e3, Deunan Knute, Stefanos Kornilios Mitsis Poiiitidis, Guru, Psyman, ZeZu
/**************************************************************************************************

    Atomiswave (c) 2003 Sammy

===================================================================================================

Guru's Readme
-------------

Sammy Atomiswave (codename SYSTEM X) system overview
(high likely) developed by SI Electroncs Ltd, former Sammy subsidiary
released April 2003

The Atomiswave System is basically just a Sega Dreamcast using ROM carts.

PCB Layout
----------

Sammy AM3AGA-04 Main PCB 2002 (top side)
1111-00006701 (bottom side)
   |--------------------------------------------|
  |-      TA8210AH               D4721   3V_BATT|
  |VGA                              BS62LV1023TC|
  |VOL                                          |
  |SER      |-----|               PCM1725U      |
  |-        |ROMEO|       D4516161              |
   |CN3     |     |                    BIOS.IC23|
|--|SW1     |-----|                             |
|                     |-----|           |-----| |
|           33.8688MHz|315- |           |ROMEO| |
|           |-----|   |6232 |           |     | |
|           |315- |   |-----|           |-----| |
|           |6258 |  32.768kHz                  |
|J          |-----|                             |
|A                 |-------------|    D4564323  |
|M                 |             |              |
|M                 |             |  |--------|  |
|A        D4516161 |  315-6267   |  |        |  |
|                  |             |  |  SH4   |  |
|                  |             |  |        |  |
|  TD62064         |             |  |        |  |
|         D4516161 |             |  |--------|  |
|                  |-------------|              |
|                                     D4564323  |
|--|             D4516161  D4516161             |
   |                             W129AG  13.5MHz|
   |--------------------------------------------|
Notes:
------
BS62LV1023TC-70 - Brilliance Semiconductor Low Power 128k x8 CMOS SRAM (TSOP32)
TA8210AH        - Toshiba 19W x 2 Channel Power Amplifier
D4516161AG5     - NEC uPD4516161AG5-A80 1M x16 (16MBit) SDRAM (SSOP50)
D4564323        - NEC uPD4564323G5-A10 512K x 32 x 4 (64MBit) SDRAM (SSOP86)
D4721           - NEC uPD4721 RS232 Line Driver Receiver IC (SSOP20)
PCM1725U        - Burr-Brown PCM1725U 16Bit Digital to Analog Converter (SOIC14)
2100            - New Japan Radio Co. Ltd. NJM2100 Dual Op Amp (SOIC8)
ROMEO           - Sammy AX0201A01 'ROMEO' 4111-00000501 0250 K13 rebadged Fujitsu MBCG61723P-102 TP2H50 - CG61P series CMOS Gate Array ASIC (TQFP100)
315-6232        - SEGA 315-6232 custom ASIC (QFP100)
315-6258        - SEGA 315-6258 custom ASIC (QFP56)
315-6267        - SEGA 315-6267 custom ASIC (BGAxxx)
TD62064         - Toshiba TD62064 NPN 50V 1.5A Quad Darlington Driver (SOIC16)
SH4             - Hitachi SH-4 HD6417091RA CPU (BGA256)
BIOS.IC23       - Macronix 29L001MC-10 3.3volt (1MBit) FlashROM (SOP44, byte mode, reverse pinout: use reverse adapter or "dead bug" method, pinout equivalent to reversed MX29LV160 with four high address lines unused)
W129AG          - IC Works W129AG Programmable Clock Frequency Generator, clock input of 13.5MHz (SOIC16)
SW1             - 2-position Dip Switch
VGA             - 15 pin VGA out connector @ 31.5kHz
SER             - 9 pin Serial connector  \
VOL             - Volume pot              / These are on a small daughterboard that plugs into the main PCB via a multi-wire cable.
CN3             - 10 pin Speaker output & Extension serial connector
3V_BATT         - Panasonic ML2020 3 Volt Coin Battery

Sega 837-14624R
later PCB revision from Sega, mostly same as above, notable differences:
ROMEO           - SEGA 315-6434
315-6232        - SEGA 315-6232A
315-6258        - SEGA 315-6258A
315-6267        - SEGA 315-6318A
SH4             - Hitachi HD6417091T
BIOS.IC48       - Amic A29L004A 4MBit flash ROM (TSOP40) labeled FPR-24363

CN3 Pinout
Pin     Function    I/O    Pin   Function  I/O
----------------------------------------------
 1    Stereo L (+)  Out  |  2      TXD     Out
 3    Stereo L (-)  Out  |  4      RXD      In
 5    Stereo R (+)  Out  |  6      GND      -
 7    Stereo R (-)  Out  |  8      +5V     Out
 9      No Connection    | 10  No Connection

The bottom of the PCB contains nothing significant except some connectors. One for the game cart, one for special controls
or I/O, one for a communication module, one for a cooling fan and one for the serial connection daughterboard.


Atomiswave cart PCB layout and game usage (revision 1.0 26/2/2011 5:59pm)
-----------------------------------------

Type 1 ROM Board:

AM3AGB-04
MROM PCB
2002
|----------------------------|
| XC9536                     |
|         IC18 IC17*   IC10  |
|                            |
|                            |
|              IC16*   IC11  |
|                            |
|                            |
||-|           IC15*   IC12  |
|| |                         |
|| |                         |
|| |CN1        IC14*   IC13  |
|| |                         |
||-|                         |
|----------------------------|
Notes:
           * - Denotes those devices are on the other side of the PCB
         CN1 - This connector plugs into the main board.
      XC9536 - Xilinx XC9536 in-system programmable CPLD (PLCC44), stamped with a
               game code. This code is different for each different game.
               The last 3 digits seems to be for the usage.
               F01 = CPLD/protection device and M01 = mask ROM

               Game (sorted by code)                 Code
               -----------------------------------------------
               Sports Shooting USA                   AX0101F01
               Dolphin Blue                          AX0401F01
               Maximum Speed                         AX0501F01
               Demolish Fist                         AX0601F01
               Guilty Gear X Ver. 1.5                AX0801F01
               Guilt Gear Isuka                      AX1201F01
               Knights Of Valour Seven Spirits       AX1301F01
               Salaryman Kintaro                     AX1401F01
               Ranger Mission                        AX1601F01
               Faster Than Speed                     AX1701F01
               Rumble Fish                           AX1801F01
               Fist Of The North Star                AX1901F01
               Victory Furlong : Horse Racing        AX2001F01
               King Of Fighters NEOWAVE              AX2201F01
               Extreme Hunting                       AX2401F01

        IC18 - Fujitsu 29DL640E 64M TSOP48 FlashROM. This ROM has no additional custom markings
               The name in the archive has been devised purely for convenience.
               This ROM holds the main program.

IC10 to IC17 - Custom-badged 128M TSOP48 mask ROMs.
               IC10 - Not Populated for 7 ROMs or less (ROM 01 if 8 ROMs are populated)
               IC11 - ROM 01 (or ROM 02 if 8 ROMs are populated)
               IC12 - ROM 02 (or ROM 03 if 8 ROMs are populated)
               IC13 - ROM 03 (or ROM 04 if 8 ROMs are populated)
               IC14 - ROM 04 (or ROM 05 if 8 ROMs are populated)
               IC15 - ROM 05 (or ROM 06 if 8 ROMs are populated)
               IC16 - ROM 06 (or ROM 07 if 8 ROMs are populated)
               IC17 - ROM 07 (or ROM 08 if 8 ROMs are populated)

               ROM Codes
               ---------
                                                                               Number
               Game (sorted by code)                 Code                      of ROMs
               -----------------------------------------------------------------------
               Sports Shooting USA                   AX0101M01 to AX0104M01    4
               Dolphin Blue                          AX0401M01 to AX0405M01    5
               Maximum Speed                         AX0501M01 to AX0505M01    5
               Demolish Fist                         AX0601M01 to AX0607M01    7
               Guilty Gear X Ver. 1.5                AX0801M01 to AX0807M01    7
               Guilty Gear Isuka                     AX1201M01 to AX1208M01    8
               Knights Of Valour Seven Spirits       AX1301M01 to AX1307M01    7
               Salaryman Kintaro                     AX1401M01 to AX1407M01    7
               Ranger Mission                        AX1601M01 to AX1605M01    5
               Faster Than Speed                     AX1701M01 to AX1706M01    6
               Rumble Fish                           AX1801M01 to AX1807M01    7
               Fist Of The North Star                AX1901M01 to AX1907M01    7
               Victory Furlong : Horse Racing        AX2001M01 to AX2007M01    7
               King Of Fighters NEOWAVE              AX2201M01 to AX2206M01    6
               Extreme Hunting                       AX2401M01 to AX2406M01    6


Type 2 ROM Board:

AM3ALW-02
MROM2 PCB
2005
|----------------------------|
|     FMEM1                  |
|     FMEM2*   MROM12        |
|              MROM11*       |
|                      MROM9 |
|              MROM10  MROM8*|
| XCR3128XL*   MROM7*        |
|                            |
||-|           MROM6         |
|| |           MROM3*  MROM4 |
|| |                   MROM5*|
|| |CN1        MROM2         |
|| |           MROM1*        |
||-|                         |
|----------------------------|
Notes:
           * - Denotes those devices are on the other side of the PCB
         CN1 - This connector plugs into the main board.
   XCR3128XL - Xilinx XCR3128XL in-system programmable 128 Macro-cell CPLD (TQFP100)
               stamped with a game code. This code is different for each different game.
               The last 3 digits seems to be for the usage.
               F01 = CPLD/protection device and M01 = mask ROM

               Game (sorted by code)                 Code
               -----------------------------------------------
               Samurai Spirits Tenkaichi Kenkakuden  AX2901F01
               Metal Slug 6                          AX3001F01
               The King Of Fighters XI               AX3201F01
               Neogeo Battle Coliseum                AX3301F01
               Rumble Fish 2                         AX3401F01

 FMEM1/FMEM2 - Fujitsu 29DL640E 64M TSOP48 FlashROM. This ROM has no additional custom markings
               The name in the archive has been devised purely for convenience.
               This ROM holds the main program.
               This location is wired to accept TSOP56 ROMs, however the actual chip populated
               is a TSOP48, using the middle pins. The other 2 pins on each side of the ROM
               are not connected to anything.

       MROM* - Custom-badged 256M SSOP70 mask ROMs

               ROM Codes
               ---------
                                                                               Number
               Game (sorted by code)                 Code                      of ROMs
               -----------------------------------------------------------------------
               Samurai Spirits Tenkaichi Kenkakuden  AX2901M01 to AX2907M01    7
               Metal Slug 6                          AX3001M01 to AX3004M01    4
               The King Of Fighters XI               AX3201M01 to AX3207M01    7
               Neogeo Battle Coliseum                AX3301M01 to AX3307M01    7
               Rumble Fish 2                         AX3401M01 to AX3405M01    5


Type 3 ROM Board:

This type is manufactured by Sega when Sammy merged with Sega.

171-8355A
PC BD A/W 128M FLASH
837-14608 (sticker for Extreme Hunting 2 Tournament Edition, Sega Bass Fishing Challenge, Sega Clay Challenge)
837-14695 (sticker for Dirty Pigskin Football)
|----------------------------|
| XC9536*         U16   U2*  |
|                            |
|J2                          |
|                            |
|                 U4*   U14  |
|                            |
|                            |
||-|                         |
|| |              U17   U1*  |
|| |                         |
|| |J1*                      |
|| |                         |
||-|              U3*   U15  |
|----------------------------|
Notes:
           * - Denotes those parts are on the other side of the PCB
          J1 - This connector plugs into the main board.
          J2 - 6 pin connector for programming the XC9536 CPLD and/or the flash ROMs
      XC9536 - Xilinx XC9536 in-system programmable CPLD (PLCC44), stamped with a
               Sega 315-xxxx part number with a sticker over the top of it.
               all known games on this type ROM BD have the same part# and decryption key.

               Game                                  Part#      Sticker
               ---------------------------------------------------------
               Extreme Hunting 2 Tournament Edition  315-6428P  315-6248
               Dirty Pigskin Football                315-6248   -
               Sega Bass Fishing Challenge           315-6248   -
               Sega Bass Fishing Challenge Version A 315-6248   -
               Sega Clay Challenge                   315-6248   -
               Animal Basket                         VM2001F01  -
               Block Pong-Pong                       VM2001F01  -
               WaiWai Drive                          VM2001F01  -
               Faster Than Speed 837-14681           315-6248   -         (not dumped, known to exists)

          U* - Fujitsu MBM29PL12LM-10PCN 128M MirrorFlash TSOP56 flash ROM.
               (configured as 16Mbytes x8bit or 8Mwords x16bit)
               This ROM has no additional custom markings. The name in the archive has been devised
               purely for convenience using the Sega 837- sticker number. The number of ROMs may vary
               between games. So far all 8 positions have been seen populated. It's also possible all
               positions are present when manufactured, each board is programmed to requirements and
               depending on the total data length, some ROMs may be empty.


Development ROM board:

There are a few unreleased and many prototype game versions known to exist on this ROM board.
Currently Rumble Fish 1, Rumble Fish 2 prototypes and Sushi Bar are dumped.

PC BD SYSTEMX 3MODE FLASH Rev.B
1111-00001402
|--------------------------------------------|
|    CN3                                     |
|                                            |
||-|           IC14 IC17 IC20 IC23           |
|| | XC9536                                  |
|| |                                         |
|| |                                         |
||CN1          IC12 IC15 IC18 IC21 IC24 IC26 |
|| |                                         |
|| |                                         |
|| | XC2S30                                  |
||-|           IC13 IC16 IC19 IC22 IC25 IC27 |
|    17S30                                   |
| CN2                                        |
|--------------------------------------------|
Notes:
         CN1 - This connector plugs into the main board through 'PC RELAY BD SX CRTG V1' adapter.
         CN2 - 8 pin connector
         CN3 - 6 pin connector for programming the XC9536 CPLD
      XC9536 - Xilinx XC9536XL in-system programmable CPLD (PLCC44), stamped JULIE_DEV on RF2 proto
      XC2S30 - Xilinx XC2S30 Spartan-II FPGA (TQFP144), Rumble Fish 2 have printed sticker A08
      17S30  - Xilinx 17S30APC OTP Configuration PROM, stamped SXFLS
   IC12-IC27 - Fujitsu MBM29DL640E 64M TSOP48 flash ROMs


Network Board
-------------

This board is required for Extreme Hunting 2 Tournament Edition (and possibly some other Sega-made Atomiswave carts)
although it doesn't need to be connected to a network or another Atomiswave unit to boot up. However it must be
plugged into the PCB in the communication slot or the game will not go in-game. It will boot but then displays
NETWORK BOARD ERROR if not present. Externally there's a hole for an RJ45 network cable and a slot for a
PIC16C621/PIC16C622 PIC enclosed in a black plastic housing. This is the same type as used in NAOMI etc. This
board probably acts like the NAOMI network DIMM board minus the on-board DIMM RAM storage.

837-14508R
171-8324C
(C) SEGA 2005
|-----------------------------------|
| RJ45    24LC0241* IC2       CN3   |
|LLLL        K4S643232*       IC14  |
|  RTL8201 LLLL                     |
|25MHz                              |
|       6417710          IC4S*      |
|                        XC3S200    |
|MAX3221        JP1     XCF01S*     |
|CN5  33.333MHz JP2             CN2*|
|-----------------------------------|
Notes:
      *        - Denotes those parts are on the other side of the PCB
      L        - LED
      RJ45     - RJ45 network connector
      24LC0241 - EEPROM (SOIC8)
      K4S643232- Samsung K4S643232 512k x 32bit x 4 banks synchronous DRAM (TSOPII-86)
      RTL8201  - Realtek RTL8201 Single Port 10/100M Fast Ethernet IC (QFP48)
      6417710  - Renesas HD6417710BPV SH3-DSP 32-Bit RISC Microcomputer SuperHTM RISC engine Family / SH7700 Series (BGA256)
      XC3S200  - Xilinx Spartan XC3S200 FPGA (QFP100)
      XCF01S   - Xilinx XCF01S In-System Programmable 1Mbit PROM for Configuration of Xilinx FPGAs (TSSOP20)
      MAX3221  - Maxim MAX3221 3.0V to 5.5V, 250kbps, RS-232 Transceivers with Auto Shutdown (TSSOP16)
      JP1/2    - Jumpers, both set to 1-2
      CN2      - This connector plugs into the main board
      CN3      - 6 pin connector
      CN5      - 3 position connector
      IC2      - ST M29DW324DB 32M flash ROM (TSOP48)
      IC4S     - Spartan S29GL128N10TFIO1 128M flash ROM (TSOP56)
      IC14     - socket for PIC16C621 mounted in a plastic plug-in case
                 PIC Usage:
                           Game                                   Sega Part#
                           ---------------------------------------------------
                           Extreme Hunting 2 Tournament Edition   317-0445-COM


AW-NET Network Board
--------------------

Sammy
AM3AJG-01
LAN PCB
2003
SAMLAN Rev: D
|-----------------------------------|
| RJ45                              |
|PULSE                              |
|        25Mz                       |
|                                   |
|      RTL8139CL+*           CN*    |
|                  315-6310*        |
|                                   |
|       L46R                        |
|-----------------------------------|
Notes:
      *          - Denotes those parts are on the other side of the PCB
      RJ45       - RJ45 network connector
      PULSE      - Pulse H0011 10/100 LAN Magnetics Module (SOIC16)
      L46R       - 93C46 compatible 128x8 EEPROM (SOIC8)
      RTL8139CL+ - Realtek RTL8139CL+ 3.3 volt Single Chip Fast Ethernet Controller with Power Management (QFP120)
      315-6310   - Sega 315-6310 Custom IC (QFP100)
      CN         - This connector plugs into the main board


Gun Sub Board
-------------

AM3AGT-02 GUN SUB PCB
|------------------|
|CN5  CN4  CN3  CN2|
|       74HC74     |
|                  |
| 74HC74    7CHC74 |
|      74HC74      |
|                  |
|       CN1        |
|------------------|
Notes:
      CN1 - 8 pin connector joining to I/O Expansion Module (which is plugged into main board)
      CN2 - Gun connection for player 2 trigger and optical
      CN3 - Gun connection for player 2 pump switch
      CN4 - Gun connection for player 1 trigger and optical
      CN5 - Gun connection for player 1 pump switch


Other games not dumped
----------------------
Miracle Stadium

Cancelled, prototypes known to exists
----------------------
Chase 1929
Force Five
Kenju
Premier Eleven

**************************************************************************************************/

#include "emu.h"
#include "dc_atomiswave.h"

//#include "machine/gunsense.h"
//#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*
    0x00600280 r  0000dcba
    a/b - 1P/2P coin inputs (JAMMA), active low
    c/d - 3P/4P coin inputs (EX. IO board), active low

    (ab == 0) -> BIOS skip RAM test
*/
uint32_t atomiswave_state::aw_modem_r(offs_t offset, uint32_t mem_mask)
{
	switch(offset)
	{
		case 0x280/4:
			return (ioport("COINS")->read() & 0x0f);
		case 0x284/4:
			// CHECKME: any game that uses non-canonical input method should be checked from here
			return m_exid_in.read_safe(m_aw_ctrl_type);
	}

	osd_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+offset*4);
	return 0;
}

/*
    0x00600284 rw ddcc0000
        cc/dd - set type of Maple devices at ports 2/3 (EX. IO board)
    0 - regular Atomiswave controller
    1 - DC lightgun
    2,3 - DC mouse/trackball

    0x00600288 rw 0000dcba
        a - 1P coin counter
        b - 2P coin counter
        c - 1P coin lockout
        d - 2P coin lockout

    0x0060028C rw POUT CN304 (EX. IO board)
        counter/lockout for 3P/4P in ggisuka (same bit mapping as above)
        dolphin known to read it too (verify)
*/
void atomiswave_state::aw_modem_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch(offset)
	{
		case 0x284/4:
			// EX ID output
			// - initialized with 0xf0 only by:
			//   waidrive, sprtshot, samsptk, rangrmsn, claychal, basschal, anmlbskt
			// - 0xf0 plus double sequence of 0x10->0x50->0x40->0x00 by:
			//   xtrmhnt2, xtrmhunt
			// Real HW tests shows that writing to this will ping the value back to reads
			// (open bus/drain?)
			m_aw_ctrl_type = data & 0xf0;
			if (m_exid_out)
				m_exid_out->write(m_aw_ctrl_type);

			logerror("%s: write to ctrl port %02x %08x\n", machine().describe_context(), data, mem_mask);
			break;
	}

	osd_printf_verbose("MODEM: [%08x] write %x to %x, mask %x\n", 0x600000+offset*4, data, offset, mem_mask);
}

void atomiswave_state::aw_map(address_map &map)
{
	/* Area 0 */
	map(0x00000000, 0x0001ffff).rw(m_awflash, FUNC(macronix_29l001mc_device::read), FUNC(macronix_29l001mc_device::write));
	map(0xa0000000, 0xa001ffff).rw(m_awflash, FUNC(macronix_29l001mc_device::read), FUNC(macronix_29l001mc_device::write));

	map(0x00200000, 0x0021ffff).ram().share("sram");     // battery backed up RAM
	map(0x005f6800, 0x005f69ff).rw(FUNC(atomiswave_state::dc_sysctrl_r), FUNC(atomiswave_state::dc_sysctrl_w));
	map(0x005f6c00, 0x005f6cff).mirror(0x02000000).m(m_maple, FUNC(maple_dc_device::amap));
	map(0x005f7000, 0x005f70ff).mirror(0x02000000).m(m_naomig1, FUNC(naomi_g1_device::submap)).umask64(0x0000ffff0000ffff);
	map(0x005f7400, 0x005f74ff).mirror(0x02000000).m(m_naomig1, FUNC(naomi_g1_device::amap));
	map(0x005f7800, 0x005f78ff).mirror(0x02000000).m(m_g2if, FUNC(dc_g2if_device::amap));
	map(0x005f7c00, 0x005f7cff).mirror(0x02000000).m(m_powervr2, FUNC(powervr2_device::pd_dma_map));
	map(0x005f8000, 0x005f9fff).mirror(0x02000000).m(m_powervr2, FUNC(powervr2_device::ta_map));
	map(0x00600000, 0x006007ff).rw(FUNC(atomiswave_state::aw_modem_r), FUNC(atomiswave_state::aw_modem_w)).umask64(0xffffffffffffffff);
	map(0x00700000, 0x00707fff).rw(FUNC(atomiswave_state::dc_aica_reg_r), FUNC(atomiswave_state::dc_aica_reg_w));
	map(0x00710000, 0x0071000f).mirror(0x02000000).rw("aicartc", FUNC(aicartc_device::read), FUNC(aicartc_device::write)).umask64(0x0000ffff0000ffff);
	map(0x00800000, 0x00ffffff).rw(FUNC(atomiswave_state::soundram_r), FUNC(atomiswave_state::soundram_w));           // sound RAM (8 MB)

	/* Area 1 - half the texture memory, like dreamcast, not naomi */
	// texture memory 64 bit access
	map(0x04000000, 0x047fffff).mirror(0x02800000).ram().share("dc_texture_ram");
	// apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now
	map(0x05000000, 0x057fffff).mirror(0x02800000).ram().share("frameram");
	//  0x067xxxxx written by maxspeed title screen animation

	/* Area 2 */
	map(0x08000000, 0x0bffffff).noprw(); // 'Unassigned'

	/* Area 3 */
	map(0x0c000000, 0x0cffffff).ram().share("dc_ram");
	map(0x0d000000, 0x0dffffff).ram().share("dc_ram"); // extra ram on Naomi (mirror on DC)
	map(0x0e000000, 0x0effffff).ram().share("dc_ram"); // mirror
	map(0x0f000000, 0x0fffffff).ram().share("dc_ram"); // mirror

	map(0x8c000000, 0x8cffffff).ram().share("dc_ram"); // RAM access through cache
	map(0x8d000000, 0x8dffffff).ram().share("dc_ram"); // RAM access through cache

	/* Area 4 - half the texture memory, like dreamcast, not naomi */
	map(0x10000000, 0x107fffff).mirror(0x02000000).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x10800000, 0x10ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x11000000, 0x117fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath0_w)).mirror(0x00800000);  // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue
	/*       0x12000000 -0x13ffffff Mirror area of  0x10000000 -0x11ffffff */
	map(0x13000000, 0x137fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath1_w)).mirror(0x00800000); // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue


	/* Area 5 */
	//map(0x14000000, 0x17ffffff).noprw(); // MPX Ext.

	/* Area 6 */
	//map(0x18000000, 0x1bffffff).noprw(); // Unassigned

	/* Area 7 */
	//map(0x1c000000, 0x1fffffff).noprw(); // SH4 Internal
}

void atomiswave_state::aw_port(address_map &map)
{
//  ???
}

/*
 ALL.Net board
 -------------
 Block diagram:
                       GPIO Port A - Jumpers, LEDs
             PIC16     GPIO Port B - I2C EEPROM
              |        |
  G2 Bus <-> FPGA <-> SH-3 <-> Ethernet PHY <-> Ethernet
                       |
                       |--- boot flash ROM 4MB
                       |--- SDRAM 8MB
                       |--- data buffer flash ROM 16MB

 SH-3 external address space
  00000000 - 003fffff boot flash ROM (IC2)
  0c000000 - 0c7fffff SDRAM
  10000000 - 10ffffff data buffer flash ROM (IC4)
  16000000 rw FPGA_INT_OUT  interrupt to host, bit 0 active low
  16000004 rw FPGA_INT_IN   interrupt from host (SH-3 IRQ0 line), bit 0 active low
  16000008 rw FPGA_INT_MASK enable interrupt from host, bit 0 active low
  1600000c rw FPGA_READY    bit 0 active high
  16000010 r  FPGA_REVISION 8bit value
  16002000 - 16007fff shared RAM (FPGA internal)

 SH-3 GPIO PortA bits
  2-3 - jumpers x2 (inputs, active low)
  4-7 - LEDs x4 (outputs, active low)

 SH-3 GPIO PortB bits (24LC024 I2C EEPROM)
  0 - I2C SCL
  6 - unknown, set to output 1
  7 - I2C SDA

 SH-4 address space
  01000000 r  ID "G2IFSOJ AM"
  01000020  w interrupt to net board, bit 0 active low
  01000024  w interrupt from net board (HOLLY EXT IRQ line), bit 0 active low
  01000028  w enable interrupt from net board, bit 0 active low
  0100002c  w net board reset?, bit 0 active high
  01000030 r  FPGA_READY?
  01000100 - 01000114 protection registers, probably mirror of 010000xx
  01000400/01000800/01001400/01001800 r at POST, tries to read these, discards the other three values and put the result to 0xc00efc8, expecting a value to "SPAG" / 0x53504147 for a flag at PC=cf1a108, connection handshake related?
  01010000 - 01016000 shared RAM (FPGA internal)

  protection registers (16bit):
  00-09 r  ID mirror? (unused)
  0a    r  some ID? value (unused)
  0c     w RNG seed?, game write here random value during init
  0e     w data offset/index (0-3 in xtrmhnt2)
  10    r  data read, xtrmhnt2 expecting: 1f9f, 1f03, 1f1c, 1f57
  12    rw control reg?, write 0001 after offset set, wait for bit1=1 before data read
  14    rw PIC detect/init reg, game check if bit0=1 (probably means if PIC present), then write RNG register, then write 0003, then wait for bit2=1 (probably means PIC initialised OK)
*/
uint64_t atomiswave_xtrmhnt2_state::allnet_hack_r(offs_t offset, uint64_t mem_mask)
{
	// disable ALL.Net board check
	// PC checks refers to -nodrc || -drc || -drc -debug
	logerror("%s: ALL.net check %x (%x)\n", machine().describe_context(), offset * 8, mem_mask);
	// "100 NETBD NOT RESPOND" right off the bat
	if (m_maincpu->pc() == 0xc03cb30 ||
		m_maincpu->pc() == 0xc03cb10 ||
		m_maincpu->pc() == 0xc03cb2e)
	{
		dc_ram[0x357fe/8] |= (uint64_t)0x200 << 48;
		dc_ram[0x358e2/8] |= (uint64_t)0x200 << 16;
		dc_ram[0x38bb2/8] |= (uint64_t)0x200 << 16;
		dc_ram[0x38bee/8] |= (uint64_t)0x200 << 48;
	}
	// "ERROR: THIS IS NOT ACCEPTABLE BY MAIN BOARD"
	if (m_maincpu->pc() == 0xc108240 ||
		m_maincpu->pc() == 0xc108210 ||
		m_maincpu->pc() == 0xc10823e)
		dc_ram[0x9acc8/8] = (dc_ram[0x9acc8/8] & 0xffffffffffff0000U) | (uint64_t)0x0009;
	return 0;
}

void atomiswave_xtrmhnt2_state::aw_map(address_map &map)
{
	atomiswave_state::aw_map(map);
	// ALL.net
	map(0x01000000, 0x0100011f).r(FUNC(atomiswave_xtrmhnt2_state::allnet_hack_r));
//  map(0x01001800, 0x01001803).lr32(NAME([]() -> u32 { return 0x53504147; }));
}

// Atomiswave - inputs are read as standard Dreamcast controllers.
// Controller bit patterns:
//
// SHOT3   (1<<0)
// SHOT2   (1<<1)
// SHOT1   (1<<2)
// START   (1<<3)
// UP      (1<<4)
// DOWN    (1<<5)
// LEFT    (1<<6)
// RIGHT   (1<<7)
// SHOT5   (1<<9)
// SHOT4   (1<<10)
// SERVICE (1<<13)
// TEST    (1<<14)

// 2 joysticks variant
static INPUT_PORTS_START( aw2c )
	PORT_START("P1.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("P1.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)

	PORT_START("P2.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P2.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// ggisuka
static INPUT_PORTS_START( aw4c )
	PORT_INCLUDE( aw2c )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("EXID_IN")
	// return 0x0x for p3/p4 connectors to work properly
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)

	PORT_START("P3.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)

	PORT_START("P4.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("P4.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
INPUT_PORTS_END

// Single-player wheel variant
static INPUT_PORTS_START( aw1w )
	PORT_START("P1.0")
	PORT_BIT( 0xf1, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("P1.1")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2.A0") /* steering */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("P2.A1") /* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("P2.A2") /* brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)

	PORT_START("P2.A3") /* steering */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("P2.A4") /* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("P2.A5") /* brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void atomiswave_state::aw_base(machine_config &config)
{
	naomi_aw_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &atomiswave_state::aw_map);
	m_maincpu->set_addrmap(AS_IO, &atomiswave_state::aw_port);
	MACRONIX_29L001MC(config, "awflash");
	aw_rom_board &rom_board(AW_ROM_BOARD(config, "rom_board", 0));
	rom_board.irq_callback().set(FUNC(dc_state::g1_irq));

	MCFG_MACHINE_RESET_OVERRIDE(dc_state,dc_console)
	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);
}

void atomiswave_state::aw1c(machine_config &config)
{
	aw_base(config);
	dc_controller_device &dcctrl0(DC_CONTROLLER(config, "dcctrl0", 0, m_maple, 0));
	dcctrl0.set_port_tags("P1.0", "P1.1", "P1.A0", "P1.A1", "P1.A2", "P1.A3", "P1.A4", "P1.A5");
	// TODO: isn't it supposed to be just one controller?
	dc_controller_device &dcctrl1(DC_CONTROLLER(config, "dcctrl1", 0, m_maple, 1));
	dcctrl1.set_port_tags("P2.0", "P2.1", "P2.A0", "P2.A1", "P2.A2", "P2.A3", "P2.A4", "P2.A5");
}

void atomiswave_state::aw2c(machine_config &config)
{
	aw_base(config);
	dc_controller_device &dcctrl0(DC_CONTROLLER(config, "dcctrl0", 0, m_maple, 0));
	dcctrl0.set_port_tags("P1.0", "P1.1", "P1.A0", "P1.A1", "P1.A2", "P1.A3", "P1.A4", "P1.A5");
	dc_controller_device &dcctrl1(DC_CONTROLLER(config, "dcctrl1", 0, m_maple, 1));
	dcctrl1.set_port_tags("P2.0", "P2.1", "P2.A0", "P2.A1", "P2.A2", "P2.A3", "P2.A4", "P2.A5");
}

void atomiswave_state::aw4c(machine_config &config)
{
	aw2c(config);
	dc_controller_device &dcctrl2(DC_CONTROLLER(config, "dcctrl2", 0, m_maple, 2));
	dcctrl2.set_port_tags("P3.0", "P3.1", "P3.A0", "P3.A1", "P3.A2", "P3.A3", "P3.A4", "P3.A5");
	dc_controller_device &dcctrl3(DC_CONTROLLER(config, "dcctrl3", 0, m_maple, 3));
	dcctrl3.set_port_tags("P4.0", "P4.1", "P4.A0", "P4.A1", "P4.A2", "P4.A3", "P4.A4", "P4.A5");
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

#define AW_BIOS \
	ROM_REGION64_LE( 0x200000, "awflash", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Sammy BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "bios0.ic23", 0x000000, 0x020000, CRC(719b2b0b) SHA1(b4c1a26bc8906d5275eb28c701dff2b9365bcdfa) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "bios1.ic23", 0x000000, 0x020000, CRC(d3e80a9f) SHA1(33024f9d51c04884c2b44ce146f340e7a857b959) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "Sega BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "fpr-24363.ic48", 0x000000, 0x020000, CRC(82a105f0) SHA1(5128fe2ddcced77332bdcab691c09958051fa564) ) \
	ROM_CONTINUE( 0x000000, 0x020000 ) \
	ROM_CONTINUE( 0x000000, 0x020000 ) \
	ROM_CONTINUE( 0x000000, 0x020000 )

ROM_START( awbios )
	AW_BIOS

	ROM_REGION( 0x8400000, "rom_board", ROMREGION_ERASE)
ROM_END

/**********************************************
 *
 * Atomiswave cart defines
 *
 *********************************************/

// note: games with AW-NET features, i.e. NGBC or KOF NW, have "CUSTOMER ID" data (shown in NETWORK SETTINGS) in ROM @ 7FE000 (not encrypted, 8 bytes of data followed by 2 bytes of bytesumm)
// EN cartridges have this area empty (FF-filled), i.e. AW-NET features not used.
// JP cartridges have it filled with unique ID, which also means dumps of several JP cartridges will differ by this few bytes.

void atomiswave_state::init_atomiswave()
{
	uint64_t *ROM = (uint64_t *)memregion("awflash")->base();

	// patch out long startup delay
	// (Sammy logo on -bios 0 before "ALL BACKUP DATA WAS CLEARED" msg)
	// Notice that you also need to zap NVRAM contents in order to boot without this patch.
	ROM[0x98e/8] = (ROM[0x98e/8] & 0xffffffffffffU) | (uint64_t)0x0009<<48;

	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0001ffff, true, ROM);
	m_maincpu->sh2drc_add_fastram(0xa0000000, 0xa001ffff, true, ROM);
}

ROM_START( fotns )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1901p01.ic18", 0x0000000, 0x0800000,  CRC(a06998b0) SHA1(d617691db5170f6db176e40fc732966d523fd8cf) )
	ROM_LOAD( "ax1901m01.ic11", 0x1000000, 0x1000000,  CRC(ff5a1642) SHA1(49cefcce173f9a811fe9c0c07bee53aeba2bc3a8) )
	ROM_LOAD( "ax1902m01.ic12", 0x2000000, 0x1000000,  CRC(d9aae8a9) SHA1(bf87034088be0847b6e297b7665e0ea4d8cba631) )
	ROM_LOAD( "ax1903m01.ic13", 0x3000000, 0x1000000,  CRC(1711b23d) SHA1(ab628b2ec678839c75245e245297818ef1592d3b) )
	ROM_LOAD( "ax1904m01.ic14", 0x4000000, 0x1000000,  CRC(443bfb26) SHA1(6f7751afa0ca55dd0679758b27bed92b31c1b050) )
	ROM_LOAD( "ax1905m01.ic15", 0x5000000, 0x1000000,  CRC(eb1cada0) SHA1(459d21d622c72606f1d3095e8a25b6c4adccf8ab) )
	ROM_LOAD( "ax1906m01.ic16", 0x6000000, 0x1000000,  CRC(fe6da168) SHA1(d4ab6443383469bb5a4337005de917627a2e21cc) )
	ROM_LOAD( "ax1907m01.ic17", 0x7000000, 0x1000000,  CRC(9d3a0520) SHA1(78583fd171b34439f77a04a97ebe3c9d1bab61cc) )

	ROM_PARAMETER(":rom_board:key", "c2") // ax1901f01
ROM_END

ROM_START( rangrmsn )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1601p01.ic18", 0x0000000, 0x0800000, CRC(00a74fbb) SHA1(57cc1eedd22d1f553956a825e69a597309ee2bef) )
	ROM_LOAD( "ax1601m01.ic11", 0x1000000, 0x1000000, CRC(f34eed33) SHA1(1c171fb8aa95877f81ed78652d4a9ff80f7713ff) )
	ROM_LOAD( "ax1602m01.ic12", 0x2000000, 0x1000000, CRC(a7d59efb) SHA1(a40938ce1399babefc8cf02f579a86cf08e211ef) )
	ROM_LOAD( "ax1603m01.ic13", 0x3000000, 0x1000000, CRC(7c0aa241) SHA1(3e0e5ff3307dcfa52998fb9b4b14bf54bd056a99) )
	ROM_LOAD( "ax1604m01.ic14", 0x4000000, 0x1000000, CRC(d2369144) SHA1(da1eae9957d27d1682c4191780cf51b32dfe6659) )
	ROM_LOAD( "ax1605m01.ic15", 0x5000000, 0x1000000, CRC(0c11c1f9) SHA1(0585db60618c5b97f9b7c203baf7e5ac90883ca6) )

	ROM_PARAMETER(":rom_board:key", "88") // ax1601f01
ROM_END

ROM_START( sprtshot )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax0101p01.ic18", 0x0000000, 0x0800000, CRC(b3642b5d) SHA1(85eabd9551aefb825ae8eb6422092fb5a58d60f6) )
	ROM_LOAD( "ax0101m01.ic11", 0x1000000, 0x1000000, CRC(1e39184d) SHA1(663e0cb9f43a0f89d9841e04b3d009f6c5e88d5e) )
	ROM_LOAD( "ax0102m01.ic12", 0x2000000, 0x1000000, CRC(700764d1) SHA1(310f1606f7bbed1012c119f1ef5d89d231d8489e) )
	ROM_LOAD( "ax0103m01.ic13", 0x3000000, 0x1000000, CRC(6144e7a8) SHA1(4d4341082f008dfd93ef5bf32a44c80869ef02a8) )
	ROM_LOAD( "ax0104m01.ic14", 0x4000000, 0x1000000, CRC(ccb72150) SHA1(a1032d321c27f9ff43da41f20b8687bf1958ddc9) )

	ROM_PARAMETER(":rom_board:key", "64") // ax0101f01
ROM_END

ROM_START( xtrmhunt )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax2401p01.ic18", 0x0000000, 0x0800000,  CRC(8e2a11f5) SHA1(b5106314fb8d4483254e83ac3982039bb60a78e8) )
	ROM_LOAD( "ax2401m01.ic11", 0x1000000, 0x1000000,  CRC(76dbc286) SHA1(8f36ca94b8e67c76e0f90b21debc5ac7890f0da1) )
	ROM_LOAD( "ax2402m01.ic12", 0x2000000, 0x1000000,  CRC(cd590ea2) SHA1(ee5e38bf68e95da665be478ebba9cc5ffed52bb7) )
	ROM_LOAD( "ax2403m01.ic13", 0x3000000, 0x1000000,  CRC(06f62eb5) SHA1(f7e8d1dda6bb59ca2bc7cfa1105889b9e8e6d55d) )
	ROM_LOAD( "ax2404m01.ic14", 0x4000000, 0x1000000,  CRC(759ef5cb) SHA1(27ac2d12c6fb358b3d631c017c7b693e5ad95fd7) )
	ROM_LOAD( "ax2405m01.ic15", 0x5000000, 0x1000000,  CRC(940d77f1) SHA1(eefdfcb92873032dc7d9ff9310bf5ed715c8bf4f) )
	ROM_LOAD( "ax2406m01.ic16", 0x6000000, 0x1000000,  CRC(cbcf2c5d) SHA1(61362fabcbb3bfc01c996748a7ca65f8a0e02f2f) )

	ROM_PARAMETER(":rom_board:key", "e4") // ax2401f01
ROM_END

ROM_START( xtrmhnt2 )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "610-0752.u3",  0x0000000, 0x1000000, CRC(bab6182e) SHA1(4d25256c81941316887cbb4524a203922f5b7104) )
	ROM_LOAD( "610-0752.u1",  0x1000000, 0x1000000, CRC(3086bc47) SHA1(eb7b04db90d296985528f0cfdd4545f184c40b64) )
	ROM_LOAD( "610-0752.u4",  0x2000000, 0x1000000, CRC(9787f145) SHA1(8445ede0477f70fbdc113810b80356945ce498d2) )
	ROM_LOAD( "610-0752.u2",  0x3000000, 0x1000000, CRC(d3a88b31) SHA1(ccf14367e4e7efbc2cc835f3b001fd6d64302a5e) )
	ROM_LOAD( "610-0752.u15", 0x4000000, 0x1000000, CRC(864a6342) SHA1(fb97532d5dd00f8520fdaf68dfcd1ea627bdf90a) )
	ROM_LOAD( "610-0752.u17", 0x5000000, 0x1000000, CRC(a79fb1fa) SHA1(f75c5b574fd79677b926c595b369e95605a3c848) )
	ROM_LOAD( "610-0752.u14", 0x6000000, 0x1000000, CRC(ce83bcc7) SHA1(e2d324a5a7eacbec7b0df9a4b9e276521bb9ab80) )
	ROM_LOAD( "610-0752.u16", 0x7000000, 0x1000000, CRC(8ac71c76) SHA1(080e41e633bf082fc536781541c6031d1ac81939) )

	ROM_PARAMETER(":rom_board:key", "2a") // 315-6248

	ROM_REGION( 0x1400000, "network", 0)    // network board
	ROM_LOAD( "fpr-24330a.ic2", 0x000000, 0x400000, CRC(8d89877e) SHA1(6caafc49114eb0358e217bc2d1a3ab58a93c8d19) )
	ROM_LOAD( "flash128.ic4s", 0x400000, 0x1000000, CRC(866ed675) SHA1(2c4c06935b7ab1876e640cede51713b841833567) )
ROM_END

// Sammy AM3AHC-01 type board
// Build:Jan 24 2005 14:12:29
ROM_START( anmlbskt )
	AW_BIOS

	ROM_REGION( 0x4000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "vm2001f01.u3",  0x0000000, 0x800000, CRC(4fb33380) SHA1(9070990515544e6e9a1d24b1e0597cdea926a4c9) )
	// U1 Populated, Empty
	ROM_LOAD( "vm2001f01.u4",  0x1000000, 0x800000, CRC(7cb2e7c3) SHA1(8b4e46cf19fbc1d613af75c52faebefb2776280b) )
	ROM_LOAD( "vm2001f01.u2",  0x1800000, 0x800000, CRC(386070a1) SHA1(bf46980ea822b4cfe67c622f0104bf793031f4ad) )
	ROM_LOAD( "vm2001f01.u15", 0x2000000, 0x800000, CRC(2bb1be28) SHA1(fda7967d6c0341a608c52087ae3d461554760435) ) // data doesn't belong to this game, possibly a left-over from another game
	// U17 Populated, Empty
	// U14 Populated, Empty
	// U16 Populated, Empty

	ROM_PARAMETER(":rom_board:key", "45") // vm2001f01
ROM_END

// no case, Sega 171-8355A type board, stickers: VM20-6101-1 V20T0069
// Build:Jan 19 2005 13:09:07
ROM_START( anmlbskta )
	AW_BIOS

	ROM_REGION( 0x4000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "u3",  0x0000000, 0x1000000, CRC(cd082af3) SHA1(d0bcce79fc9bdce04ebfe57fe8b5f7c59ef5fdf3) )
	ROM_LOAD( "u1",  0x1000000, 0x1000000, CRC(4a2a01d3) SHA1(20d98b137efec539451c0573b2928c3a92be5743) ) // identical to parent set U4+U2
	ROM_LOAD( "u4",  0x2000000, 0x1000000, CRC(646e9773) SHA1(dd081a9a42edca956f96124545772d4687ca2113) ) // data not belongs to this game, more looks like random trash
	ROM_LOAD( "u2",  0x3000000, 0x1000000, CRC(b9162d97) SHA1(7f561617fa0538da554ad6f6c4d6a20e739491dc) ) // data not belongs to this game, more looks like random trash
	// U14-U17 not populated

	ROM_PARAMETER(":rom_board:key", "45") // vm2001f01
ROM_END

// game have 2 sets of graphics switched via "LOCATION" setting: SC (Shopping Center) kids oriented, and NORMAL
// since this version SC by default, possible it is special SC ver and there also was earlier regular game version
// no case, Sega 171-8355A type board, stickers: VM20-6101-1 V20T0004
ROM_START( blokpong )
	AW_BIOS

	ROM_REGION( 0x4000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "u3",  0x0000000, 0x1000000, CRC(debaf8bd) SHA1(8f007e48828697ff7371998f04b2fdd84329fa22) )
	ROM_LOAD( "u1",  0x1000000, 0x1000000, CRC(ca097a3f) SHA1(280fc0c9c36fc988b2ab57c229bbec760d09d5eb) )
	ROM_LOAD( "u4",  0x2000000, 0x1000000, CRC(d235dd29) SHA1(bdd2318a7975fd985b4731700e290a4d0d9cde74) ) // 1st half is game data, 2nd half is garbage
	//ROM_LOAD( "u2",  0x3000000, 0x1000000, CRC(b9162d97) SHA1(7f561617fa0538da554ad6f6c4d6a20e739491dc) ) // garbage data not used by this game, match anmlbskta U2
	// U14-U17 not populated

	ROM_PARAMETER(":rom_board:key", "45") // vm2001f01
ROM_END

ROM_START( dolphin )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax0401p01.ic18", 0x0000000, 0x0800000, CRC(195d6328) SHA1(cf3b5699f81235919dd3b1974d2ecb0376cb4552) )
	ROM_LOAD( "ax0401m01.ic11", 0x1000000, 0x1000000, CRC(5e5dca57) SHA1(e0623c84f66cada37d4c9399a7a8fc6866933144) )
	ROM_LOAD( "ax0402m01.ic12", 0x2000000, 0x1000000, CRC(77dd4771) SHA1(dcd23b8ddc82eab2f325266ffd7ed3fbc1bcdf71) )
	ROM_LOAD( "ax0403m01.ic13", 0x3000000, 0x1000000, CRC(911d0674) SHA1(eec35badcfbfe412b7104a86c2111f5a1b5fb5cd) )
	ROM_LOAD( "ax0404m01.ic14", 0x4000000, 0x1000000, CRC(f82a4ca3) SHA1(da686d86e176a9f24874d2916b1932f03a99a52d) )
	ROM_LOAD( "ax0405m01.ic15", 0x5000000, 0x1000000, CRC(b88298d7) SHA1(490c3ec471018895b7268ee33498dddaccbbfd5a) )

	ROM_PARAMETER(":rom_board:key", "40") // ax0401f01
ROM_END

ROM_START( demofist )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax0601p01.ic18", 0x0000000, 0x0800000, CRC(0efb38ad) SHA1(9400e37efe3e936474d74400ebdf28ad0869b67b) )
	ROM_LOAD( "ax0601m01.ic11", 0x1000000, 0x1000000, CRC(12fda2c7) SHA1(3afbac221ffe249386e4cb50b4edd013d9a40062) )
	ROM_LOAD( "ax0602m01.ic12", 0x2000000, 0x1000000, CRC(aea61fdf) SHA1(0a088848bbf7a47df8b44b69bf72ed0d4a1088f8) )
	ROM_LOAD( "ax0603m01.ic13", 0x3000000, 0x1000000, CRC(d5879d35) SHA1(977cd3b373c6f94eb21ffb24ff564971d3d633e5) )
	ROM_LOAD( "ax0604m01.ic14", 0x4000000, 0x1000000, CRC(a7b09048) SHA1(229fa2332b58fec2a712c3ebd672662f35a9485a) )
	ROM_LOAD( "ax0605m01.ic15", 0x5000000, 0x1000000, CRC(18d8437e) SHA1(fe2e189e40a89141335e754268d29d46e3eb3bb8) )
	ROM_LOAD( "ax0606m01.ic16", 0x6000000, 0x1000000, CRC(42c81617) SHA1(1cc686af5e3fc56143836e3dcc0067893f82fcf9) )
	ROM_LOAD( "ax0607m01.ic17", 0x7000000, 0x1000000, CRC(96e5aa84) SHA1(e9841f550f2ef409d97004542bcadabb6b9e84af) )

	ROM_PARAMETER(":rom_board:key", "90") // ax0601f01
ROM_END

// (C)Dimps Wed Mar 10 19:08:51 2004 TANAKA (build 0028)
ROM_START( rumblef )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1801p01.ic18", 0x0000000, 0x0800000, CRC(2f7fb163) SHA1(bf819d798d9a3a7bc754e111a3f53b9db6d6042a) )
	ROM_LOAD( "ax1801m01.ic11", 0x1000000, 0x1000000, CRC(c38aa61c) SHA1(e2f688a0aa8b0119f5fd3d53c8904e035d43a4b1) )
	ROM_LOAD( "ax1802m01.ic12", 0x2000000, 0x1000000, CRC(72e0ebc8) SHA1(e85300a405ea14c4c9d857eb9685c93faaca1d56) )
	ROM_LOAD( "ax1803m01.ic13", 0x3000000, 0x1000000, CRC(d0f59d98) SHA1(b854796087e9f76a13a21da8249f7224e451e129) )
	ROM_LOAD( "ax1804m01.ic14", 0x4000000, 0x1000000, CRC(15595cba) SHA1(8dd06d1f986cd21a58d20b662b11ed7ba8a6ff7a) )
	ROM_LOAD( "ax1805m01.ic15", 0x5000000, 0x1000000, CRC(3d3f8e0d) SHA1(364a0bda890722b9fb72171f96c742b8f3fef23e) )
	ROM_LOAD( "ax1806m01.ic16", 0x6000000, 0x1000000, CRC(ac2751bb) SHA1(5070fa12bf109ab87e8f7ea46ac4ae78a73105da) )
	ROM_LOAD( "ax1807m01.ic17", 0x7000000, 0x1000000, CRC(3b2fbdb0) SHA1(f9f7e06785d3a07282247aaedd9999aa7c2670b9) )

	ROM_PARAMETER(":rom_board:key", "aa") // ax1801f01
ROM_END

// Prototype, (C)Dimps Fri Feb 20 11:00:43 2004 TANAKA (build 0028)
ROM_START( rumblefp )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD("ic12", 0x00000000, 0x00800000, CRC(79866072) SHA1(aa9decd8878ab5a21fe72afb96ee841e94ee07b5) )
	ROM_LOAD("ic13", 0x00800000, 0x00800000, CRC(5630bc83) SHA1(46848b58a55c180d9a92df6914a1a8b9af35cc57) )
	ROM_LOAD("ic14", 0x01000000, 0x00800000, CRC(bcd49846) SHA1(d8ab1253a2904ec4f7126880a55c780986cefd66) )
	ROM_LOAD("ic15", 0x01800000, 0x00800000, CRC(61257cfb) SHA1(bbb8cdd265a55a9d4c9b133b68aa0434de9e0f5b) )
	ROM_LOAD("ic16", 0x02000000, 0x00800000, CRC(c2eb7c61) SHA1(6284ff0fb670011ca9b6ade5acb33211b60cbe43) )
	ROM_LOAD("ic17", 0x02800000, 0x00800000, CRC(dcf673d3) SHA1(0abb2087bd35221cd5ec5f4d6b2f03a2234b5634) )
	ROM_LOAD("ic18", 0x03000000, 0x00800000, CRC(72c066bb) SHA1(a9d457b17d9dd79f54b71bdab24096ec3fbd00ea) )
	ROM_LOAD("ic19", 0x03800000, 0x00800000, CRC(b20bf301) SHA1(3f5754b11f4b621703a21ddbf4762d6ada9f3ca3) )
	ROM_LOAD("ic20", 0x04000000, 0x00800000, CRC(d27e7393) SHA1(62b9a880550067829f26ee120fad330257d349ff) )
	ROM_LOAD("ic21", 0x04800000, 0x00800000, CRC(c2da1ecf) SHA1(26d14843c256eaf7196e59463adb6581a25e9cca) )
	ROM_LOAD("ic22", 0x05000000, 0x00800000, CRC(730e0e1c) SHA1(469a2b34c492408aa70b60c1293481d218b76086) )
	ROM_LOAD("ic23", 0x05800000, 0x00800000, CRC(d93afcac) SHA1(69e2d873e5a384d1e14ef47d6f6a3cbcbe782eec) )
	ROM_LOAD("ic24", 0x06000000, 0x00800000, CRC(262d97b9) SHA1(1ae41ebea41035d21e174a03532dbaff9fe1ece2) )
	ROM_LOAD("ic25", 0x06800000, 0x00800000, CRC(e45cf169) SHA1(3b080d6306262db36c6857e11b8ec506fa20f0f5) )
	ROM_LOAD("ic26", 0x07000000, 0x00800000, CRC(6421720d) SHA1(6eaeb93d462542c3cf3e815d5fb309c337a8673b) )
	// IC27 populated, empty

	ROM_PARAMETER(":rom_board:key", "25") // Julie
ROM_END

// Build:Jun 25 2005 17:00:38
ROM_START( ngbc )
	AW_BIOS

	ROM_REGION( 0x14000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ax3301en_p01.fmem1", 0x00000000, 0x0800000, CRC(f7e24e67) SHA1(8eef26d44b294faa509304b1b04f4d801337bc99) )
	ROM_LOAD( "ax3301m01.mrom1", 0x02000000, 0x2000000, CRC(e6013de9) SHA1(ccbc7c2e76153348646d75938d5c008dc80df17d) )
	ROM_LOAD( "ax3302m01.mrom2", 0x04000000, 0x2000000, CRC(f7cfef6c) SHA1(c9e6231499a9c9c8650d9e61f34ff1fcce8d442c) )
	ROM_LOAD( "ax3303m01.mrom3", 0x06000000, 0x2000000, CRC(0cdf8647) SHA1(0423f96842bef2c2ff454318dc6960b5052c0551) )
	ROM_LOAD( "ax3304m01.mrom4", 0x0a000000, 0x2000000, CRC(2f031db0) SHA1(3214735f04fadf160137f0585bfc1a27eeecfac6) )
	ROM_LOAD( "ax3305m01.mrom5", 0x0c000000, 0x2000000, CRC(f6668aaa) SHA1(6a78f8f0c7d7a71854ff87329290d38970cfb476) )
	ROM_LOAD( "ax3306m01.mrom6", 0x0e000000, 0x2000000, CRC(5cf32fbd) SHA1(b6ae0abe5791b3d6f8db07b8c8ca22219a153801) )
	ROM_LOAD( "ax3307m01.mrom7", 0x12000000, 0x2000000, CRC(26d9da53) SHA1(0015b4be670005a451274de68279b4302fc42a97) )

	ROM_PARAMETER(":rom_board:key", "a0") // ax3301f01
ROM_END

// same as above EN-dump, but CustomerID not FF-filled
// Build:Jun 25 2005 17:00:38
ROM_START( ngbcj )
	AW_BIOS

	ROM_REGION( 0x14000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ax3301p01.fmem1", 0x00000000, 0x0800000, CRC(6dd78275) SHA1(72d4cab58dbcebd666db21aeef190378ef447580) )
	ROM_LOAD( "ax3301m01.mrom1", 0x02000000, 0x2000000, CRC(e6013de9) SHA1(ccbc7c2e76153348646d75938d5c008dc80df17d) )
	ROM_LOAD( "ax3302m01.mrom2", 0x04000000, 0x2000000, CRC(f7cfef6c) SHA1(c9e6231499a9c9c8650d9e61f34ff1fcce8d442c) )
	ROM_LOAD( "ax3303m01.mrom3", 0x06000000, 0x2000000, CRC(0cdf8647) SHA1(0423f96842bef2c2ff454318dc6960b5052c0551) )
	ROM_LOAD( "ax3304m01.mrom4", 0x0a000000, 0x2000000, CRC(2f031db0) SHA1(3214735f04fadf160137f0585bfc1a27eeecfac6) )
	ROM_LOAD( "ax3305m01.mrom5", 0x0c000000, 0x2000000, CRC(f6668aaa) SHA1(6a78f8f0c7d7a71854ff87329290d38970cfb476) )
	ROM_LOAD( "ax3306m01.mrom6", 0x0e000000, 0x2000000, CRC(5cf32fbd) SHA1(b6ae0abe5791b3d6f8db07b8c8ca22219a153801) )
	ROM_LOAD( "ax3307m01.mrom7", 0x12000000, 0x2000000, CRC(26d9da53) SHA1(0015b4be670005a451274de68279b4302fc42a97) )

	ROM_PARAMETER(":rom_board:key", "a0") // ax3301f01
ROM_END

// Build:Jul 09 2004 15:05:53
ROM_START( kofnw )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax2201en_p01.ic18", 0x0000000, 0x0800000, CRC(27aab918) SHA1(41c5ddd8bd4c91481750606ab44aa115b5fe01d0) )
	ROM_LOAD( "ax2201m01.ic11", 0x1000000, 0x1000000, CRC(22ea665b) SHA1(292c92c9ae43eea2d1c27cedfb89c3956b8dea32) )
	ROM_LOAD( "ax2202m01.ic12", 0x2000000, 0x1000000, CRC(7fad1bea) SHA1(89f3f88af48973a4685955d86ef97a1487b8e7a8) )
	ROM_LOAD( "ax2203m01.ic13", 0x3000000, 0x1000000, CRC(78986ca4) SHA1(5a6c8c12955573f33361d2c6f20f85de35ac7bae) )
	ROM_LOAD( "ax2204m01.ic14", 0x4000000, 0x1000000, CRC(6ffbeb04) SHA1(975062cf364589dbdd5c5cb5ca945f76d87fc120) )
	ROM_LOAD( "ax2205m01.ic15", 0x5000000, 0x1000000, CRC(2851b791) SHA1(566ef95ea066b7bf548986085670242be217befc) )
	ROM_LOAD( "ax2206m01.ic16", 0x6000000, 0x1000000, CRC(e53eb965) SHA1(f50cd53a5859f081d8a278d24a519c9d9b49ab96) )

	ROM_PARAMETER(":rom_board:key", "99") // ax2201f01
ROM_END

// Build:Sep 10 2004 12:05:34
ROM_START( kofnwj )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax2201jp_p01.ic18", 0x0000000, 0x0800000, CRC(ecc4a5c7) SHA1(97c2ef2be95b39bc978474a8243740df50255a8b) )

	/* these are taken from the above set, game *seems* to work fine with these ... */
	ROM_LOAD( "ax2201m01.ic11", 0x1000000, 0x1000000, CRC(22ea665b) SHA1(292c92c9ae43eea2d1c27cedfb89c3956b8dea32) )
	ROM_LOAD( "ax2202m01.ic12", 0x2000000, 0x1000000, CRC(7fad1bea) SHA1(89f3f88af48973a4685955d86ef97a1487b8e7a8) )
	ROM_LOAD( "ax2203m01.ic13", 0x3000000, 0x1000000, CRC(78986ca4) SHA1(5a6c8c12955573f33361d2c6f20f85de35ac7bae) )
	ROM_LOAD( "ax2204m01.ic14", 0x4000000, 0x1000000, CRC(6ffbeb04) SHA1(975062cf364589dbdd5c5cb5ca945f76d87fc120) )
	ROM_LOAD( "ax2205m01.ic15", 0x5000000, 0x1000000, CRC(2851b791) SHA1(566ef95ea066b7bf548986085670242be217befc) )
	ROM_LOAD( "ax2206m01.ic16", 0x6000000, 0x1000000, CRC(e53eb965) SHA1(f50cd53a5859f081d8a278d24a519c9d9b49ab96) )

	ROM_PARAMETER(":rom_board:key", "99") // ax2201f01
ROM_END

ROM_START( kov7sprt )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1301p01.ic18", 0x0000000, 0x0800000, CRC(6833a334) SHA1(646aaa578e09ad23bc9c7f4fbdfb3c1486916fd3) )
	ROM_LOAD( "ax1301m01.ic11", 0x1000000, 0x1000000, CRC(58ae7ca1) SHA1(e91975697b797ea05488ace649cbb9964b4cd500) )
	ROM_LOAD( "ax1301m02.ic12", 0x2000000, 0x1000000, CRC(871ea03f) SHA1(6806910832ca271a9240aca8e91279556e5b0cb7) )
	ROM_LOAD( "ax1301m03.ic13", 0x3000000, 0x1000000, CRC(abc328bc) SHA1(d9271d4e5abe76d31de0f60a5c106260338d42e9) )
	ROM_LOAD( "ax1301m04.ic14", 0x4000000, 0x1000000, CRC(25a176d1) SHA1(6d815bf6acb645fead060733660e24fb0d44282d) )
	ROM_LOAD( "ax1301m05.ic15", 0x5000000, 0x1000000, CRC(e6573a93) SHA1(0666e52d0088263f28938e4c8aae201e604ec1f2) )
	ROM_LOAD( "ax1301m06.ic16", 0x6000000, 0x1000000, CRC(cb8cacb4) SHA1(5d008e8a934451b9bfa33fedfd492c86d9226ef5) )
	ROM_LOAD( "ax1301m07.ic17", 0x7000000, 0x1000000, CRC(0ca92213) SHA1(115c50fa55e6de3439de23e74621695510c6a7ba) )

	ROM_PARAMETER(":rom_board:key", "35") // ax1301f01
ROM_END

ROM_START( ggisuka )
	AW_BIOS

	ROM_REGION( 0x9000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1201p01.ic18", 0x0000000, 0x0800000, CRC(0a78d52c) SHA1(e9006dc43cd11d5ba49a092a1dff31dc10700c28) )
	ROM_LOAD( "ax1201m01.ic10", 0x0800000, 0x1000000, CRC(df96ce30) SHA1(25a9f743b1c2b11896d0c7a2dc1c198fc977aaca) )    // 2x mirrored 8MB data, TODO: check if IC10 mask ROM not 16MB but 8MB
	ROM_LOAD( "ax1202m01.ic11", 0x1000000, 0x1000000, CRC(dfc6fd67) SHA1(f9d35b18a03d22f70feda42d314b0f9dd54eea55) )
	ROM_LOAD( "ax1203m01.ic12", 0x2000000, 0x1000000, CRC(bf623df9) SHA1(8b9a8e8100ff6d2ce9a982ab8eb1d542f1c7af03) )
	ROM_LOAD( "ax1204m01.ic13", 0x3000000, 0x1000000, CRC(c80c3930) SHA1(5c39fde36e2ebbfe72967d7d0202eb454a8d3bbe) )
	ROM_LOAD( "ax1205m01.ic14", 0x4000000, 0x1000000, CRC(e99a269d) SHA1(a52148b82b0338b8bad8b52985302eaf81a4cfde) )
	ROM_LOAD( "ax1206m01.ic15", 0x5000000, 0x1000000, CRC(807ab795) SHA1(17c86b1a56333c05b68ff84f83e964d013c1819c) )
	ROM_LOAD( "ax1207m01.ic16", 0x6000000, 0x1000000, CRC(6636d1b8) SHA1(9bd8fc114557f6fbe772f85eeb246f7336d4255e) )
	ROM_LOAD( "ax1208m01.ic17", 0x7000000, 0x1000000, CRC(38bda476) SHA1(0234a6f5fbaf5e958b3ba0db311dff157f80addc) )

	ROM_PARAMETER(":rom_board:key", "ed") // ax1201f01
/*
  Sammy AM3AHT/AWMPSYSTEM
    4-cabinet splitter device, likely was used in GG Isuka, and maybe in 2-player AW-NET games too.

  Case contains 3x PCB stack:
    AM3AHT-01:
      Renesas 12363VTE33 H8S/2363 MCU
      ROM
      HM62V8512CLFP-5 512K x8bit SRAM
      33MHz OSC
      LEDs 4x
      DIPSW 4x
    AM3AHW-01: 1-to-4 Video-splitter
      DSUB connectors 4x
    AM3AHU-01:
      Card reader connectors 4x
*/
	ROM_REGION( 0x80000, "awmpsystem", 0 )
	// AM3AHT AWMPSYSTEM
	// WA0101E03
	// U1 SUM:A17C Sammy
	ROM_LOAD("am3aht.u1", 0, 0x80000, CRC(c4a21dbf) SHA1(3200fdf209f8a6c8c9acdb60a2bc1d70af28fc5b) ) // H8S/2363 code
ROM_END

ROM_START( maxspeed )
	AW_BIOS

	ROM_REGION( 0x9000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax0501p01.ic18", 0x0000000, 0x0800000, CRC(e1651867) SHA1(49caf82f4b111da312b14bb0a9c31e3732b4b24e) )
	ROM_LOAD( "ax0501m01.ic11", 0x1000000, 0x1000000, CRC(4a847a59) SHA1(7808bcd357b85861082b426dbe34a20ae7016f6a) )
	ROM_LOAD( "ax0502m01.ic12", 0x2000000, 0x1000000, CRC(2580237f) SHA1(2e92c940f95edae33d6a7e8a071544a9083a0fd6) )
	ROM_LOAD( "ax0503m01.ic13", 0x3000000, 0x1000000, CRC(e5a3766b) SHA1(1fe6e072adad27ac43c0bff04e3c448678aabc18) )
	ROM_LOAD( "ax0504m01.ic14", 0x4000000, 0x1000000, CRC(7955b55a) SHA1(927f58d6961e702c2a8afce79bac5e5cff3dfed6) )
	ROM_LOAD( "ax0505m01.ic15", 0x5000000, 0x1000000, CRC(e8ccc660) SHA1(a5f414f200a0d41e958430d0fc2d4e1fda1cc67c) )

	ROM_PARAMETER(":rom_board:key", "55") // ax0501f01
ROM_END

ROM_START( vfurlong )
	AW_BIOS

	ROM_REGION( 0x9000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax2001p01.ic18", 0x0000000, 0x0800000, CRC(17ea9aa9) SHA1(c68500e9b3407a9d4b20f2678718ce475f179f7d) )
	ROM_LOAD( "ax2001p01_alt.ic18", 0x0000000, 0x0800000, CRC(845399dd) SHA1(21565b13292cead2b532861d2998e381df2672d5) ) // same as above except for Customer ID, included for reference
	ROM_LOAD( "ax2001m01.ic11", 0x1000000, 0x1000000, CRC(64460b24) SHA1(044857d6593897d303622e005a63ca7b3acd7453) )
	ROM_LOAD( "ax2002m01.ic12", 0x2000000, 0x1000000, CRC(d4da357f) SHA1(c462cddec9a369a1a5595676de76499d56683ea9) )
	ROM_LOAD( "ax2003m01.ic13", 0x3000000, 0x1000000, CRC(aa1e1246) SHA1(788cc9c070f82aeff1704361e3c72116fd5c4c9d) )
	ROM_LOAD( "ax2004m01.ic14", 0x4000000, 0x1000000, CRC(4d555d7c) SHA1(a5eccc920bdb7ad9cf57c0e1ef6a905c6b9eee45) )
	ROM_LOAD( "ax2005m01.ic15", 0x5000000, 0x1000000, CRC(785208e2) SHA1(6ba5bd3901c5b1d71abcc8d833a011bd4abae6b6) )
	ROM_LOAD( "ax2006m01.ic16", 0x6000000, 0x1000000, CRC(8134ec55) SHA1(843e473d4f99237ded641cce9515b7802cfe3742) )
	ROM_LOAD( "ax2007m01.ic17", 0x7000000, 0x1000000, CRC(d0557e8a) SHA1(df8057597eb690bd18c5d26736f5d4f86e3b1225) )

	ROM_PARAMETER(":rom_board:key", "db") // ax2001f01
ROM_END

ROM_START( salmankt )
	AW_BIOS

	ROM_REGION( 0x9000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1401p01.ic18", 0x0000000, 0x0800000, CRC(28d779e0) SHA1(dab785a595de5f474c18c713e672949176a5b1b5) )
	ROM_LOAD( "ax1401m01.ic11", 0x1000000, 0x1000000, CRC(fd7af845) SHA1(0c8f5a91662e46d5c187a0758af95082183cdf69) )
	ROM_LOAD( "ax1402m01.ic12", 0x2000000, 0x1000000, CRC(f6006f85) SHA1(9275603673c663f73b977d0d14ed1d2a7002c627) )
	ROM_LOAD( "ax1403m01.ic13", 0x3000000, 0x1000000, CRC(074f7c4b) SHA1(4955e4f333d15d5d9cc69bb9658f29a37e912012) )
	ROM_LOAD( "ax1404m01.ic14", 0x4000000, 0x1000000, CRC(af4e3829) SHA1(18d9e8a8d8e930ad697b686a98f31ea175f5fd4a) )
	ROM_LOAD( "ax1405m01.ic15", 0x5000000, 0x1000000, CRC(b548446f) SHA1(8b4661e601e36c067dff2530aff4f7ea76e1c21e) )
	ROM_LOAD( "ax1406m01.ic16", 0x6000000, 0x1000000, CRC(437673e6) SHA1(66f7e5f246ebbb1bdbf074da41ec16bf32720a82) )
	ROM_LOAD( "ax1407m01.ic17", 0x7000000, 0x1000000, CRC(6b6acc0a) SHA1(a8c692c875271a0806460caa79c67fd756231273) )

	ROM_PARAMETER(":rom_board:key", "77") // ax1401f01
ROM_END

ROM_START( ftspeed )
	AW_BIOS

	ROM_REGION( 0x9000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax1701p01.ic18", 0x0000000, 0x0800000, CRC(480cade7) SHA1(487d4b27d7e5196d8321c5a80175ec7b1b32c1e8) )
	ROM_LOAD( "ax1701m01.ic11", 0x1000000, 0x1000000, CRC(7dcdc784) SHA1(5eeef9a760a0b090ed5aad8b7bdee2baa69a088b) )
	ROM_LOAD( "ax1702m01.ic12", 0x2000000, 0x1000000, CRC(06c9bf85) SHA1(636262dca7140397436646754fb204b97aa08ce9) )
	ROM_LOAD( "ax1703m01.ic13", 0x3000000, 0x1000000, CRC(8f8e0224) SHA1(2a9a17ed726913c00bf1c6a94bdd4fb32e800868) )
	ROM_LOAD( "ax1704m01.ic14", 0x4000000, 0x1000000, CRC(fbb4bb16) SHA1(b582680a880166c5bbdd2ad77b7903fedf9b01ad) )
	ROM_LOAD( "ax1705m01.ic15", 0x5000000, 0x1000000, CRC(996f68e1) SHA1(3fa505c641127d9027bfc7ec0ab16905344a4e2c) )
	ROM_LOAD( "ax1706m01.ic16", 0x6000000, 0x1000000, CRC(804b2eb2) SHA1(fcca02a5a8c09eb16548255115fb105c9c49c4e0) )

	ROM_PARAMETER(":rom_board:key", "6b") // ax1701f01
ROM_END

// contents of cartridges labeled as JP and EN is the same
// Build:Aug 07 2005 18:11:25
ROM_START( kofxi )
	AW_BIOS

	ROM_REGION( 0x14000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ax3201p01.fmem1", 0x00000000, 0x0800000, CRC(6dbdd71b) SHA1(cce3897b104f5d923d8136485fc80eb9717ff4b5) )
	ROM_LOAD( "ax3201m01.mrom1", 0x02000000, 0x2000000, CRC(7f9d6af9) SHA1(001064ad1b8c3408efe799dc766c2728dc6512a9) )
	ROM_LOAD( "ax3202m01.mrom2", 0x04000000, 0x2000000, CRC(1ae40afa) SHA1(9ee7957c86cc3a71e6971ddcd906a82c5b1e16f1) )
	ROM_LOAD( "ax3203m01.mrom3", 0x06000000, 0x2000000, CRC(8c5e3bfd) SHA1(b5443e2a1b88642cc57c5287a3122376c2d48de9) )
	ROM_LOAD( "ax3204m01.mrom4", 0x0a000000, 0x2000000, CRC(ba97f80c) SHA1(36f672fe833e13f0bab036b02c39123066327e20) )
	ROM_LOAD( "ax3205m01.mrom5", 0x0c000000, 0x2000000, CRC(3c747067) SHA1(54b7ff73d618e2e4e40c125c6cfe99016e69ad1a) )
	ROM_LOAD( "ax3206m01.mrom6", 0x0e000000, 0x2000000, CRC(cb81e5f5) SHA1(07faee02a58ac9c600ab3cdd525d22c16b35222d) )
	ROM_LOAD( "ax3207m01.mrom7", 0x12000000, 0x2000000, CRC(164f6329) SHA1(a72c8cbe4ac7b98edda3d4434f6c81a370b8c39b) )

	ROM_PARAMETER(":rom_board:key", "d3") // ax3201f01
ROM_END

ROM_START( dirtypig )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "695-0014.u3",  0x0000000, 0x1000000, CRC(9fdd7d07) SHA1(56d580dda116823ea5dc5e1bd5154463a476866a) )
	ROM_LOAD( "695-0014.u1",  0x1000000, 0x1000000, CRC(a91d2fcb) SHA1(8414386c09ba36ea581c8161f6cf2a13cc5ae516) )
	ROM_LOAD( "695-0014.u4",  0x2000000, 0x1000000, CRC(3342f237) SHA1(e617b0e1f8d8da9783c58ab98eb91de2363ec36f) )
	ROM_LOAD( "695-0014.u2",  0x3000000, 0x1000000, CRC(4d82152f) SHA1(a448983d4e81eb6485b62f23a6c99d1112a20c21) )
	ROM_LOAD( "695-0014.u15", 0x4000000, 0x1000000, CRC(d239a549) SHA1(71f3c1c2ae2a9b6f09f30e7be3bb11ba111276ae) )
	ROM_LOAD( "695-0014.u17", 0x5000000, 0x1000000, CRC(16bb5992) SHA1(18772587272aba1d50a48d384f472276c3b48d96) )
	ROM_LOAD( "695-0014.u14", 0x6000000, 0x1000000, CRC(55470242) SHA1(789036189ae5488a9da565774bdf91b49cd8264e) )
	ROM_LOAD( "695-0014.u16", 0x7000000, 0x1000000, CRC(730180a4) SHA1(017b82e2d2744695e3e521d35a8511ecc1c8ab43) )

	ROM_PARAMETER(":rom_board:key", "2a") // 315-6248
ROM_END

// Ver 2005/12/16
// Build:Jan 13 2006 00:49:12
ROM_START( mslug6 )
	AW_BIOS

	ROM_REGION( 0xc000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ax3001p01.fmem1", 0x0000000, 0x0800000, CRC(af67dbce) SHA1(5aba108caf3e4ced6994bc26e752d4e225c231e8) )
	ROM_LOAD( "ax3001m01.mrom1", 0x2000000, 0x2000000, CRC(e56417ee) SHA1(27692ad5c1093aff0973d2aafd01a5e30c7bfbbe) )
	ROM_LOAD( "ax3002m01.mrom2", 0x4000000, 0x2000000, CRC(1be3bbc1) SHA1(d75ce5c855c9c4eeacdbf84d440c73a94de060fe) )
	ROM_LOAD( "ax3003m01.mrom3", 0x6000000, 0x2000000, CRC(4fe37370) SHA1(85d51db94c3e34265e37b636d6545ed2801ba5a6) )
	ROM_LOAD( "ax3004m01.mrom4", 0xa000000, 0x2000000, CRC(2f4c4c6f) SHA1(5815c28fdaf0429003986e725c0015fe4c08721f) )

	ROM_PARAMETER(":rom_board:key", "82") // ax3001f01
ROM_END

// Build:Aug 05 2005 16:43:48
ROM_START( samsptk )
	AW_BIOS

	ROM_REGION( 0x14000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ax2901p01.fmem1", 0x00000000, 0x0800000, CRC(58e0030b) SHA1(ed8a66833beeb56d83770123eff28df0f25221d1) )
	ROM_LOAD( "ax2901m01.mrom1", 0x02000000, 0x2000000, CRC(dbbbd90d) SHA1(102ee0b249a3e0ca2f659b6c515816c522ad78d0) )
	ROM_LOAD( "ax2902m01.mrom2", 0x04000000, 0x2000000, CRC(a3bd7890) SHA1(9b8d934d6ebc3ef688cd8a6de47657a0663fea10) )
	ROM_LOAD( "ax2903m01.mrom3", 0x06000000, 0x2000000, CRC(56f50fdd) SHA1(8a5a4a99108c0279056998046c7b332e80121dee) )
	ROM_LOAD( "ax2904m01.mrom4", 0x0a000000, 0x2000000, CRC(8a3ae175) SHA1(966f527a92e24c8eb770344697f2edf6140cf971) )
	ROM_LOAD( "ax2905m01.mrom5", 0x0c000000, 0x2000000, CRC(429877ba) SHA1(88e1f3bc682b18d331e328ef8754065109cf9bda) )
	ROM_LOAD( "ax2906m01.mrom6", 0x0e000000, 0x2000000, CRC(cb95298d) SHA1(5fb5d5a0d6801df61101a1b23de0c14ff29ef654) )
	ROM_LOAD( "ax2907m01.mrom7", 0x12000000, 0x2000000, CRC(48015081) SHA1(3c0a0a6dc9ab7bf889579477699e612c3092f9bf) )

	ROM_PARAMETER(":rom_board:key", "1d") // ax2901f01
ROM_END

ROM_START( ggx15 )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax0801p01.ic18", 0x0000000, 0x0800000, CRC(d920c6bb) SHA1(ab34bbef3c71396447bc5322d8e8786041fc832a) )
	ROM_LOAD( "ax0801m01.ic11", 0x1000000, 0x1000000, CRC(61879b2d) SHA1(9592fbd979cef9d8f465cd92d0f00b9c13ecf7ba) )
	ROM_LOAD( "ax0802m01.ic12", 0x2000000, 0x1000000, CRC(c0ff124d) SHA1(dd403d10de2f097fbaa6b93bc311e2b9e893828d) )
	ROM_LOAD( "ax0803m01.ic13", 0x3000000, 0x1000000, CRC(4400c89a) SHA1(4e13536c01103ecfbfc9e3e33746ceae7a91a520) )
	ROM_LOAD( "ax0804m01.ic14", 0x4000000, 0x1000000, CRC(70f58ab4) SHA1(cd2def19bbad945c87567f8d28f3a2a179a7f7f6) )
	ROM_LOAD( "ax0805m01.ic15", 0x5000000, 0x1000000, CRC(72740e45) SHA1(646eded89f10008c9176cd6772a8ac9d1bf4271a) )
	ROM_LOAD( "ax0806m01.ic16", 0x6000000, 0x1000000, CRC(3bf8ecba) SHA1(43e7fbf21d8ee60bab72ce558640730fd9c3e3b8) )
	ROM_LOAD( "ax0807m01.ic17", 0x7000000, 0x1000000, CRC(e397dd79) SHA1(5fec32dc19dd71ef0d451f8058186f998015723b) )

	ROM_PARAMETER(":rom_board:key", "c9") // ax0801f01
ROM_END

// (C)Dimps Fri Mar 4 19:27:57 2005 NONAME (build 2319)
ROM_START( rumblef2 )
	AW_BIOS

	ROM_REGION( 0xe000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ax3401p01.fmem1", 0x0000000, 0x0800000, CRC(a33601cf) SHA1(2dd60a9c3a2517f2257ab69288fa95645de133fa) )
	ROM_LOAD( "ax3401m01.mrom1", 0x2000000, 0x2000000, CRC(60894d4c) SHA1(5b21af3c7c82d4d64bfd8498c26283111ada1298) )
	ROM_LOAD( "ax3402m01.mrom2", 0x4000000, 0x2000000, CRC(e4224cc9) SHA1(dcab06fcf48cda286f93d2b37f03a83abf3230cb) )
	ROM_LOAD( "ax3403m01.mrom3", 0x6000000, 0x2000000, CRC(081c0edb) SHA1(63a3f1b5f9d7ca4367868c492236406f23996cc3) )
	ROM_LOAD( "ax3404m01.mrom4", 0xa000000, 0x2000000, CRC(a426b443) SHA1(617aab42e432a80b0663281fb7faa6c14ef4f149) )
	ROM_LOAD( "ax3405m01.mrom5", 0xc000000, 0x2000000, CRC(4766ce56) SHA1(349b82013a75905ae5520b14a87702c9038a5def) )

	ROM_PARAMETER(":rom_board:key", "07") // ax3401f01
ROM_END

// Prototype ROM board
// (C)Dimps Tue Jan 11 14:32:45 2005 NONAME (build 0001)
ROM_START( rumblf2p )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD("ic12", 0x00000000, 0x00800000, CRC(1a0e74ab) SHA1(679787e5fcc0e197f97a00544c1f277d3695df80) )
	ROM_LOAD("ic13", 0x00800000, 0x00800000, CRC(5630bc83) SHA1(46848b58a55c180d9a92df6914a1a8b9af35cc57) )
	ROM_LOAD("ic14", 0x01000000, 0x00800000, CRC(7fcfc59c) SHA1(ca2b71fe6dd959d89a7e30363090d38032c3697a) )
	ROM_LOAD("ic15", 0x01800000, 0x00800000, CRC(eee00692) SHA1(ee630a77c130e64435be544b13cd885ecc7bfeb4) )
	ROM_LOAD("ic16", 0x02000000, 0x00800000, CRC(cd029db9) SHA1(d5d70dbb3822538afc67efa1e905c520b63cc978) )
	ROM_LOAD("ic17", 0x02800000, 0x00800000, CRC(223a5b58) SHA1(ab540c994598f5cbe34ec8a62fa96181cd2be6e2) )
	ROM_LOAD("ic18", 0x03000000, 0x00800000, CRC(5e2d2f67) SHA1(fe348e8e342d0d642a21cd24c57387384f20fa0e) )
	ROM_LOAD("ic19", 0x03800000, 0x00800000, CRC(3cfb2adc) SHA1(d731674d80e924c250fe3519aef1392d38167aa3) )
	ROM_LOAD("ic20", 0x04000000, 0x00800000, CRC(2c216a05) SHA1(0677146ecf5abe00368e205fd7da19234a997dd2) )
	ROM_LOAD("ic21", 0x04800000, 0x00800000, CRC(79540865) SHA1(8ad7b789f25df5405949fef96d0db35ca8e424c3) )
	ROM_LOAD("ic22", 0x05000000, 0x00800000, CRC(c91d95a0) SHA1(a50e4fffa3cf70459d9bb36f0155e768d4281f39) )
	ROM_LOAD("ic23", 0x05800000, 0x00800000, CRC(5c39ca18) SHA1(6a29c4b1dd6b8eca5824687a1b501594a6676606) )
	ROM_LOAD("ic24", 0x06000000, 0x00800000, CRC(858d2775) SHA1(34ca97f348a810c6f950840ef2390334011c6034) )
	ROM_LOAD("ic25", 0x06800000, 0x00800000, CRC(975d35fb) SHA1(a4cf97a05cbeb830090426915067b3dd15224939) )
	ROM_LOAD("ic26", 0x07000000, 0x00800000, CRC(ff9a2c4c) SHA1(81ac8fb41d7af605da0dcd92104cef0f045777bf) )
	// IC27 populated, empty

	ROM_PARAMETER(":rom_board:key", "25") // Julie
ROM_END

ROM_START( claychal )
	AW_BIOS

	ROM_REGION( 0x8000100, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "608-2161.u3",  0x0000000, 0x1000100, CRC(5bb65194) SHA1(5fa8c38e6aadf5d999e260da24b001c0c7805d48) )
	ROM_LOAD( "608-2161.u1",  0x1000000, 0x1000100, CRC(526fc1af) SHA1(dd8a37fa73a9ef193b6f4fb962345bdfc4854b5d) )
	ROM_LOAD( "608-2161.u4",  0x2000000, 0x1000100, CRC(55f4e762) SHA1(a11f7d69458e647dd2b8d86c98a54f309b1f1bbc) )
	ROM_LOAD( "608-2161.u2",  0x3000000, 0x1000100, CRC(c40dae68) SHA1(29ec47c76373eeaa686684f10907d551de7d9c59) )
	ROM_LOAD( "608-2161.u15", 0x4000000, 0x1000100, CRC(b82dcb0a) SHA1(36dc89a388ac0c7e0a0e72428c8149cbda12805a) )
	ROM_LOAD( "608-2161.u17", 0x5000000, 0x1000100, CRC(2f973eb4) SHA1(45409b5517cda119315f198892224889ac3a0f53) )
	ROM_LOAD( "608-2161.u14", 0x6000000, 0x1000100, CRC(2e7d966f) SHA1(3304fd0c5140a13f6fe2ea9aaa74d7885e1505e1) )
	ROM_LOAD( "608-2161.u16", 0x7000000, 0x1000100, CRC(14f8ca87) SHA1(778c048da9434ffda600e35ad5aca29e02cc98c0) )

	ROM_PARAMETER(":rom_board:key", "2a") // 315-6248
ROM_END

// Build:Feb 08 2009 22:35:34
ROM_START( basschalo )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD("610-0811.u3",  0x00000000, 0x01000000, CRC(ef31abe7) SHA1(bf8a66da4ceead350c200626792af55b7f258db4) )
	ROM_LOAD("610-0811.u1",  0x01000000, 0x01000000, CRC(44c3cf90) SHA1(620a55f8f971f86fd65a3e3e9f0784fed5ca891e) )
	ROM_LOAD("vera.u4",      0x02000000, 0x01000000, CRC(bd1f13aa) SHA1(1ef9a7e684418baf8a61fef2610839fd72887d4c) )
	ROM_LOAD("610-0811.u2",  0x03000000, 0x01000000, CRC(1c61ed69) SHA1(e5a53362ea3e285a05d69d0fb56e1f8625272c18) )
	ROM_LOAD("610-0811.u15", 0x04000000, 0x01000000, CRC(e8f02238) SHA1(f762a0a93e80899f71af6d2e0126a3fe9586815a) )
	ROM_LOAD("610-0811.u17", 0x05000000, 0x01000000, CRC(db799f5a) SHA1(f85f35388ffb4399382b676773314f9a651e38df) )
	ROM_LOAD("610-0811.u14", 0x06000000, 0x01000000, CRC(f2769383) SHA1(c580577df9d140bb6ecce192efafb0284d22c32d) )
	ROM_LOAD("vera.u16",     0x07000000, 0x01000000, CRC(3590072d) SHA1(3375a0334c35de1d7d8231d7cc27775451042f91) )

	ROM_PARAMETER(":rom_board:key", "2a") // 315-6248
ROM_END

// Version A
// Build:Jul 15 2009 16:27:40
ROM_START( basschal )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD("vera.u3",  0x00000000, 0x01000000, CRC(8cbec9d7) SHA1(080f5edd817993946b1008ebe8ba489f818d3f99) )
	ROM_LOAD("vera.u1",  0x01000000, 0x01000000, CRC(cfef27e5) SHA1(e0e27adc1b3635a310c50c6374d85572db608675) )
	ROM_LOAD("vera.u4",  0x02000000, 0x01000000, CRC(bd1f13aa) SHA1(1ef9a7e684418baf8a61fef2610839fd72887d4c) )
	ROM_LOAD("vera.u2",  0x03000000, 0x01000000, CRC(0a463c37) SHA1(630ad98d2f80fd458729bd56e8d665a88263da28) )
	ROM_LOAD("vera.u15", 0x04000000, 0x01000000, CRC(e588afd1) SHA1(0ce3aeb2bcea66beaec2410d1df6857c4365aecf) )
	ROM_LOAD("vera.u17", 0x05000000, 0x01000000, CRC(d78389a4) SHA1(50babfe3d58929a26a69dd4a4120fd87f507a95e) )
	ROM_LOAD("vera.u14", 0x06000000, 0x01000000, CRC(35df044f) SHA1(eeac6c4062f697205558846d6ac262cb5c1b10cf) )
	ROM_LOAD("vera.u16", 0x07000000, 0x01000000, CRC(3590072d) SHA1(3375a0334c35de1d7d8231d7cc27775451042f91) )

	ROM_PARAMETER(":rom_board:key", "2a") // 315-6248
ROM_END

// no case, Sega 171-8355A type board, stickers: VM20-6101-1 V20T0031
ROM_START( waidrive )
	AW_BIOS

	ROM_REGION( 0x4000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "u3",  0x0000000, 0x1000000, CRC(7acfb499) SHA1(002a0a84dd7f55e1630e1ec2530d0760b0c12b4e) )
	ROM_LOAD( "u1",  0x1000000, 0x1000000, CRC(b3c1c3bb) SHA1(bda9b1e214a733cb1716ab130ecd986709ac136e) )
	//ROM_LOAD( "u4",  0x2000000, 0x1000000, CRC(646e9773) SHA1(dd081a9a42edca956f96124545772d4687ca2113) ) // garbage data not used by this game, match anmlbskta U4
	//ROM_LOAD( "u2",  0x3000000, 0x1000000, CRC(b9162d97) SHA1(7f561617fa0538da554ad6f6c4d6a20e739491dc) ) // garbage data not used by this game, match anmlbskta U2
	// U14-U17 not populated

	ROM_PARAMETER(":rom_board:key", "45") // vm2001f01
ROM_END

// Prototype ROM board
// Build:May 23 2003 14:40:15
ROM_START( sushibar )
	AW_BIOS

	ROM_REGION( 0x8000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD("ic12", 0x00000000, 0x00800000, CRC(06a2ed58) SHA1(a807fa8c1c83cb8b18595c210479d5f1dd6be4ca) )
	// IC 13 populated, empty
	ROM_LOAD("ic14", 0x01000000, 0x00800000, CRC(4860f944) SHA1(55c75630c33cba35512a1349650f28fd56757f9f) )
	ROM_LOAD("ic15", 0x01800000, 0x00800000, CRC(7113506c) SHA1(0548d67b3a1c0b8f17fcafd4fe5c1e6b0e91b6e7) )
	ROM_LOAD("ic16", 0x02000000, 0x00800000, CRC(77e8e39e) SHA1(1286010cdba5c3c0ad5cbe19718fd0f8e5f579db) )
	ROM_LOAD("ic17", 0x02800000, 0x00800000, CRC(0eba54ea) SHA1(51842ec326a5ba65bd280e652e7d9b395a2586c6) )
	ROM_LOAD("ic18", 0x03000000, 0x00800000, CRC(b9957c76) SHA1(6d72c7ac8e1e0cbed7eb01b66f71bedf46a833e1) )
	// IC19 - IC27 populated, empty

	ROM_PARAMETER(":rom_board:key", "25") // Julie
ROM_END

#define GAME_FLAGS (MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_SOUND|MACHINE_NOT_WORKING)

// Released in April 2003, boot ROM has (c) 2001 $be0
GAME( 2001, awbios,    0,        aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Atomiswave BIOS", GAME_FLAGS | MACHINE_IS_BIOS_ROOT )

// game "exe" build timestamps, shown in SYSTEM MENU -> TEST MODE
GAME( 2003, ggx15,     awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Arc System Works / Sammy", "Guilty Gear X ver. 1.5", GAME_FLAGS ) // none
GAME( 2003, sprtshot,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy USA",                "Sports Shooting USA", GAME_FLAGS ) // May 02 2003 09:40:31
GAME( 2003, sushibar,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Sushi Bar / Toretore! Sushi", GAME_FLAGS ) // May 23 2003 14:40:15
GAME( 2003, demofist,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Polygon Magic / Dimps",    "Demolish Fist", GAME_FLAGS ) // Jun 02 2003 16:45:35
GAME( 2003, maxspeed,  awbios,   aw1c, aw1w, atomiswave_state, init_atomiswave, ROT0,   "SIMS / Sammy",             "Maximum Speed", GAME_FLAGS ) // Jun 09 2003 10:20:37
GAME( 2003, dolphin,   awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Dolphin Blue", GAME_FLAGS ) // Jun 27 2003 09:00:03
GAME( 2003, kov7sprt,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "IGS / Sammy",              "Knights of Valour - The Seven Spirits", GAME_FLAGS ) // Nov 24 2003 16:56:01
GAME( 2004, ggisuka,   awbios,   aw4c, aw4c, atomiswave_state, init_atomiswave, ROT0,   "Arc System Works / Sammy", "Guilty Gear Isuka", GAME_FLAGS ) // Jan 14 2004 10:04:24
GAME( 2004, rumblefp,  rumblef,  aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / Dimps",            "The Rumble Fish (prototype)", GAME_FLAGS ) // Feb 20 2004 09:15:34
GAME( 2004, rangrmsn,  awbios,   aw2c, aw1w, atomiswave_state, init_atomiswave, ROT0,   "RIZ Inc./ Sammy",          "Ranger Mission", GAME_FLAGS ) // Mar 01 2004 19:08:15
GAME( 2004, rumblef,   awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / Dimps",            "The Rumble Fish", GAME_FLAGS ) // Mar 10 2004 19:07:43
GAME( 2004, salmankt,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Yuki Enterprise / Sammy",  "Net@Select: Salaryman Kintaro", GAME_FLAGS ) // Jun 14 2004 22:50:03
GAME( 2004, kofnw,     awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "The King of Fighters Neowave", GAME_FLAGS ) // Jul 09 2004 15:05:53
GAME( 2004, kofnwj,    kofnw,    aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "The King of Fighters Neowave (Japan)", GAME_FLAGS ) // Jul 09 2004 15:05:53
GAME( 2004, ftspeed,   awbios,   aw1c, aw1w, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Faster Than Speed", GAME_FLAGS ) // Aug 24 2004 18:40:24
GAME( 2004, xtrmhunt,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Extreme Hunting", GAME_FLAGS ) // Nov 23 2004 10:14:14
GAME( 2004, blokpong,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT270, "MOSS / Sammy",             "Block Pong-Pong", GAME_FLAGS ) // Dec 22 2004 12:32:52
GAME( 2005, rumblf2p,  rumblef2, aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / Dimps",            "The Rumble Fish 2 (prototype)", GAME_FLAGS ) // Jan 11 2005 14:31:05
GAME( 2005, anmlbskta, anmlbskt, aw2c, aw2c, atomiswave_state, init_atomiswave, ROT270, "MOSS / Sammy",             "Animal Basket / Hustle Tamaire Kyousou (19 Jan 2005)", GAME_FLAGS ) // Jan 19 2005 13:09:07
GAME( 2005, anmlbskt,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT270, "MOSS / Sammy",             "Animal Basket / Hustle Tamaire Kyousou (24 Jan 2005)", GAME_FLAGS ) // Jan 24 2005 14:12:29
GAME( 2005, waidrive,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT270, "MOSS / Sammy",             "WaiWai Drive", GAME_FLAGS ) // Jan 27 2005 16:21:21
GAME( 2005, vfurlong,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Progress / Sammy",         "Net@Select: Horse Racing - Victory Furlong", GAME_FLAGS ) // Mar 02 2005 22:10:33
GAME( 2005, rumblef2,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / Dimps",            "The Rumble Fish 2", GAME_FLAGS ) // Mar 04 2005 19:26:32
GAME( 2005, ngbc,      awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "NeoGeo Battle Coliseum", GAME_FLAGS ) // Jun 25 2005 17:00:38
GAME( 2005, ngbcj,     ngbc,     aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "NeoGeo Battle Coliseum (Japan)", GAME_FLAGS ) // Jun 25 2005 17:00:38
GAME( 2005, samsptk,   awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "Samurai Spirits Tenkaichi Kenkakuden", GAME_FLAGS ) // Aug 05 2005 16:43:48
GAME( 2005, kofxi,     awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy / SNK Playmore",     "The King of Fighters XI", GAME_FLAGS ) // Aug 07 2005 18:11:25
GAME( 2005, fotns,     awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Arc System Works / Sega",  "Fist Of The North Star / Hokuto no Ken", GAME_FLAGS ) // Nov 28 2005 21:04:40
GAME( 2006, mslug6,    awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sega / SNK Playmore",      "Metal Slug 6", GAME_FLAGS ) // Jan 13 2006 00:49:12
GAME( 2006, xtrmhnt2,  awbios,   aw2c, aw2c, atomiswave_xtrmhnt2_state, init_atomiswave, ROT0, "Sega",              "Extreme Hunting 2", GAME_FLAGS | MACHINE_UNEMULATED_PROTECTION ) // May 26 2006 14:03:22
GAME( 2006, dirtypig,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sammy",                    "Dirty Pigskin Football", GAME_FLAGS ) // Sep 10 2006 20:24:14
GAME( 2008, claychal,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sega",                     "Sega Clay Challenge", GAME_FLAGS ) // Oct 15 2008 16:08:20
GAME( 2009, basschalo, basschal, aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sega",                     "Sega Bass Fishing Challenge", GAME_FLAGS ) // Feb 08 2009 22:35:34
GAME( 2009, basschal,  awbios,   aw2c, aw2c, atomiswave_state, init_atomiswave, ROT0,   "Sega",                     "Sega Bass Fishing Challenge Version A", GAME_FLAGS ) // Jul 25 2009 16:27:40
