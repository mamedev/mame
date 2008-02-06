/*

  Sega Naomi
 + Related Systems (possibly to be split at a later date)

  Driver by Samuele Zannoli, R. Belmont, and ElSemi.

Sega Naomi is Dreamcast based Arcade hardware.

---------------------------------------------------------------------------------------------------------------------------------------------------

Guru's Readme
-------------

Sega NAOMI Mainboard
Sega, 1998-2005

PCB Layout
----------
837-13544-01
171-7772F
837-13707 (sticker)
(C) SEGA 1999
|---------------------------------------------------|
|    CN1                           CN3              |
|PC910  62256   EPF8452AQC160-3                     |
|    BATTERY EPC1064   JP4  3771  93C46             |
|A179B    315-6188.IC31     3771                3773|
|ADM485  BIOS.IC27   5264165            5264165     |
|                    5264165  |-----|   5264165     |
|    CN3                      |POWER|               |
|                             |VR2  | 33.3333MHZ    |
|CN26                         |-----|          27MHZ|
|                                         CY2308SC-3|
|        KM416S4030      |------|     HY57V161610   |
|                        | SH4  |     HY57V161610   |
| C844G        315-6232  |      |             32MHZ |
|            33.8688MHZ  |------|     HY57V161610   |
|                         xMHz        HY57V161610   |
|      PCM1725    JP1                    62256      |
|                     HY57V161610                   |
|                          HY57V161610              |
|              315-6145                             |
|CN25                CY2308SC-1          315-6146   |
|          LED1                              93C46  |
|          LED2                     14.7456MHZ      |
|---------------------------------------------------|
Notes:
      CN1/2/3         - Connectors for ROM cart
      CN25/26         - Connectors for filter board
      EPF8452AQC160-3 - Altera FLEX EPF8452AQC160-3 FPGA (QFP160)
      315-6188.IC31   - Altera EPC1064 (DIP8)
      JP1             - set to 2-3. Alt setting is 1-2
      JP4             - set to 2-3. Alt setting is 1-2
      93C46           - 128 bytes EEPROM
      A179B 96K       - ?, made by TI
      ADM485          - Analog Devices ADM485
      BIOS.IC27       - 27C160 EPROM
      5264165         - Hitachi 5264165FTTA60 (video RAM)
      HY57V161610     - Hyundai 57V161610DTC-8 (main program RAM)
      CY2308SC-3      - Clock generator IC
      KM416S4030      - Samsung KM416S4030 16MBit SDRAM (sound related RAM?)
      315-6232        - Sega Custom IC (QFP100)
      315-6145        - Sega Custom IC (QFP56)
      315-6146        - Sega Custom IC (QFP176)
      C844G           - ? (SOIC14)
      62256           - 32kx8 SRAM
      PCM1725         - Burr-Brown PCM1725
      xMHz            - Small round XTAL (possibly 32.768kHz for a clock?)
      SH4             - Hitachi SH4 CPU (BGAxxx, with heatsink and fan)
      POWERVR2        - POWERVR2 video generator (BGAxxx, with heatsink)

Filter Board
------------
839-1069
|----------------------------------------------------|
|SW2 SW1   DIPSW   CN5                      CN12 CN10|
|                                                    |
|                                                    |
|           DIN1                     DIN2            |
|                                                    |
|               CNTX  CNRX                           |
| CN9    CN8                                         |
|    CN7     CN6           CN4       CN3    CN2   CN1|
|----------------------------------------------------|
Notes:
      CN1/CN2   - Power input
      CN3       - HD15 (i.e. VGA connector) RGB Video Output @ 15kHz or 31.5kHz
      CN4       - RCA Audio Output connectors
      CN5       - USB connector (connection to I/O board)
      CN6       - 10 pin connector labelled 'MAPLE 0-1'
      CN7       - 11 pin connector labelled 'MAPLE 2-3'
      CN8       - RS422 connector
      CN9       - Midi connector
      CNTX/CNRX - Network connectors
      DIN1/DIN2 - Connectors joining to mainboard CN25/26
      SW1       - Test Switch
      SW2       - Service Switch
      DIPSW     - 4-position DIP switch block
      CN10      - 12 volt output for internal case exhaust fan
      CN11      - RGB connector (not populated)
      CN12      - 5 volt output connector

---------------------------------------------------------------------------------------------------------------------------------------------------

Guru's Readme
-------------

Atomis Wave (system overview)
Sammy/SEGA, 2002

This is one of the latest arcade systems so far, basically just a Dreamcast using ROM carts.

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
ROMEO           - Sammy AX0201A01 'ROMEO' 4111-00000501 0250 K13 custom ASIC (TQFP100)
315-6232        - SEGA 315-6232 custom ASIC (QFP100)
315-6258        - SEGA 315-6258 custom ASIC (QFP56)
315-6267        - SEGA 315-6267 custom ASIC (BGAxxx)
TD62064         - Toshiba TD62064 NPN 50V 1.5A Quad Darlington Driver (SOIC16)
SH4             - Hitachi SH-4 HD6417091RA CPU (BGA256)
BIOS.IC23       - Macronix 29L001MC-10 3.3volt FlashROM (SOP44)
                  This is a little strange, the ROM appears to be a standard SOP44 with reverse pinout but the
                  address lines are shifted one pin out of position compared to industry-standard pinouts.
                  The actual part number doesn't exist on the Macronix web site, even though they have datasheets for
                  everything else! So it's probably a custom design for Sammy and top-secret!
                  The size is assumed to be 1MBit and is 8-bit (D0-D7 only). The pinout appears to be this.....

                               +--\/--+
tied to WE of BS62LV1023   VCC | 1  44| VCC - tied to a transistor, probably RESET
                           NC  | 2  43| NC
                           A9  | 3  42| NC
                           A10 | 4  41| A8
                           A11 | 5  40| A7
                           A12 | 6  39| A6
                           A13 | 7  38| A5
                           A14 | 8  37| A4
                           A15 | 9  36| A3
                           A16 |10  35| A2
                           NC  |11  34| A1
                           NC  |12  33| CE - tied to 315-6267
                           GND |13  32| GND
                           A0  |14  31| OE
                           D7  |15  30| D0
                           NC  |16  29| NC
                           D6  |17  28| D1
                           NC  |18  27| NC
                           D5  |19  26| D2
                           NC  |20  25| NC
                           D4  |21  24| D3
                           VCC |22  23| NC
                               +------+

W129AG          - IC Works W129AG Programmable Clock Frequency Generator, clock input of 13.5MHz (SOIC16)
SW1             - 2-position Dip Switch
VGA             - 15 pin VGA out connector @ 31.5kHz
SER             - 9 pin Serial connector  \
VOL             - Volume pot              / These are on a small daughterboard that plugs into the main PCB via a multi-wire cable.
CN3             - Unknown 10 pin connector labelled 'CN3'
3V_BATT         - Panasonic ML2020 3 Volt Coin Battery

The bottom of the PCB contains nothing significant except some connectors. One for the game cart, one for special controls
or I/O, one for a communication module, one for a cooling fan and one for the serial connection daughterboard.

---------------------------------------------------------------------------------------------------------------------------------------------------



Most of the information below is taken from other sites and has no yet been verified to be correct


Known Games on this Hardware

System
    Title                                                    Notes
---------------------------------------------------------------------------------------------------------------------------------------------------
Naomi                                                      |
                                                           |
    18 Wheeler                                             |
    Airline Pilot                                          |
    Cannon Spike                                           |
    Capcom vs SNK : Millenium Fight 2000                   |
    Charge N' Blast                                        |
    Cosmic Smash                                           |
    Crackin' DJ                                            |
    Crackin' DJ Part 2                                     |
    Crazy Taxi                                             |
    Dead or Alive 2                                        |
    Dead or Alive 2 Version 2000                           |
    Death Crimson OX                                       |
    Derby Owners Club                                      |
    Dynamite Baseball                                      |
    Dynamite Baseball '99                                  |
    F1 World Grand Prix                                    |
    F335 Challenge                                         |
    F355 Challenge Twin                                    |
    F355 2 - International Course Edition                  |
    F355 2 - International Course Edition Twin             |
    Fish Live                                              |
    Formation Battle In May                                |
    Giant Gram 2                                           |
    Giant Gram 2000                                        |
    Gigawing 2                                             |
    Guilty Gear X                                          |
    Gun Beat                                               |
    Gun Spike                                              |
    Gun Survivor 2 Biohazard - Code Veronica               |
    Heavy Metal : Geomatrix                                |
    Idol Janshi Suchie-Pai 3                               |
    Jambo! Safari                                          |
    Marvel vs Capcom 2                                     |
    Mazan : Flash Of The Blade                             |
    Mobile Suit Gundam : Federation VS Zeon                |  GDL-0001 253-5509-5069
    Ninja Assault                                          |
    OutTrigger                                             |
    Power Stone                                            |
    Power Stone 2                                          |
    Project Justice: Rival Schools 2/Moero! Justice Gakuen |
    Puyo Puyo Da!                                          |
    Puyo Puyo Fever                                        |
    Quiz Ah My Goddess                                     |
    Ring Out 4x4                                           |
    Samba de Amigo                                         |
    Sambo de Amigo ver.2000                                |
    Sega Marine Fishing                                    |
    Sega Srike Fighter                                     |
    Sega Tetris                                            |
    Shangri-La                                             |
    Shootout Pool                                          |
    Slashout                                               |
    Spawn                                                  |
    The House of the Dead II                               |
    The Typing of the Dead                                 |
    Tokyo Bus Tour                                         |
    Touch de Uno!                                          |
    Touch de Uno! 2                                        |
    Toukon Retsuden 4/New Japan Pro Wrestling              |
    Toy Fighters/Waffupu                                   |
    Virtua NBA                                             |
    Virtua Striker 2 ver.2000                              |
    Virtua Tennis / Power Smash                            |
    Virtua Tennis 2 / Power Smash 2                        |
    Virtual-On Oratorio Tangram MSBS Version 5.66          |
    Wave Runner GP                                         |
    World Kicks                                            |
    World Series '99                                       |
    WWF Royal Rumble                                       |
    Zero Gunner 2                                          |
    Zombie Revenge                                         |
                                                           |
---------------------------------------------------------------------------------------------------------------------------------------------------
Naomi GD-Rom                                               |
                                                           |
    Alienfront Online                                      |
    Azumanga Daioh Puzzle Bobble                           |
    Border Down                                            |
    Capcom vs SNK : Millenium Fight 2000 Pro               |
    Capcom vs SNK 2 : Millionare Fighting 2001             |
    Chaos Breaker                                          |
    Chaos Field                                            |
    Cleopatra Fortune Plus                                 |
    Confidential Mission                                   |
    Dimm Firm Update                                       |
    Dog Walking / InuNoOsanpo                              |
    Dragon Treasure                                        |
    Dragon Treasure II                                     |
    Dragon Treasure III                                    |
    Get Bass 2                                             |
    Guilty Gear XX : The Midnight Carnival                 |
    Guilty Gear XX #Reload                                 |
    Guilty Gear XX Slash                                   |
    Guilty Gear XX Accent Core                             |
    Ikaruga                                                |
    Jingi Storm                                            |
    Karous                                                 |
    Kuru Kuru Chameleon                                    |
    La keyboard                                            |
    Lupin 3 : The Shooting                                 |
    Lupin : The Typing                                     |
    Melty Blood - Act Cadenza                              |
    Mobile Suit Gundam: Federation VS Zeon                 |
    Mobile Suit Gundam: Federation VS Zeon DX              |
    Moeru Kajinyo                                          |
    Monkey Ball                                            |
    Musapey's Choco Marker                                 |
    Pochinya                                               |
    Psyvariar 2 - The Will To Fabricate                    |
    Quiz k tie Q mode                                      |
    Radilgy                                                |
    Senko No Ronde                                         |
    Shikigami No Shiro II / The Castle of Shikigami II     |
    Shakka to Tambourine                                   |
    Shakka to Tambourine 2001                              |
    Shakka to Tambourine 2001 Power Up!                    |
    Slashout                                               |
    Spikers Battle                                         |
    Sports Jam                                             |
    Street Fighter Zero 3 Upper                            |  GDL-0002 253-5509-5072
    Super Major League / World Series Baseball             |
    Super Shanghai 2005                                    |
    Tetris Kiwamemichi                                     |
    The Maze Of Kings                                      |
    Trigger Heart Exelica                                  |
    TriZeal                                                |
    Under Defeat                                           |
    Usagi Yasei no Topai - Yamashiro Mahjongg Compilation  |
    Virtua Athletics                                       |
    Virtua Golf / Dynamic Golf                             |
    Virtua Tennis / Power Smash                            |
    Virtua Tennis 2 / Power Smash 2                        |
    Wild Riders                                            |
    Zooo                                                   |
                                                           |
---------------------------------------------------------------------------------------------------------------------------------------------------
Naomi 2                                                    |
                                                           |
    Club Kart                                              |
    Club Kart : European Session                           |
    Club Kart Prize                                        |
    King Of Route 66                                       |
    Quest Of D                                             |
    Sega Driving Simulator                                 |
    Soul Surfer                                            |
    Virtua Fighter 4                                       |
    Virtua Fighter 4 Evolution                             |
    Virtua Striker III                                     |
    Wild Riders                                            |
                                                           |
---------------------------------------------------------------------------------------------------------------------------------------------------
Naomi 2 GD-ROM                                             |
                                                           |
    Beach Spikers                                          |
    Club Kart Cycraft edition                              |
    Initial D : Arcade Stage                               |
    Initial D : Arcade Stage 2                             | GDS-0026 253-5508-0345J (Japan),  GDS-0027 253-5508-0357 (Export)
    Initial D : Arcade Stage 3                             |
    Initial D3 Cycraft Edition                             |
    Virtua Fighter 4                                       |
    Virtua Fighter 4 Evolution                             |
    Virtua Fighter 4 Final Tuned                           |
    Virtua Striker III                                     |
    Wild Riders                                            |
                                                           |
---------------------------------------------------------------------------------------------------------------------------------------------------
Sammy Atomiswave                                           |
                                                           |
    Chase 1929                                             | prototype
    Demolish Fist                                          |
    Dirty Pigskin Football                                 |
    Dolphin Blue                                           |
    Extreme Hunting                                        |
    Faster Than Speed                                      |
    Force Five                                             |
    Guilty Gear Isuka                                      |
    Guilty Gear X Ver. 1.5                                 |
    Hokuto No Ken / Fist Of The Northstar                  |
    Horse Racing                                           |
    Kenju                                                  |
    Knights Of Valour : The Seven Spirits                  |
    Maximum Speed                                          |
    Metal Slug 6                                           |
    Neogeo Battle Coliseum                                 |
    Premier Eleven                                         |
    Ranger Mission                                         |
    Rumble Fish                                            |
    Rumble Fish 2                                          |
    Salaried Worker Golden Taro                            |
    Sammy Vs. Capcom                                       | development cancelled pre-prototype
    Samurai Spirits Tenkaichi Kenkakuden                   |
    Sports Shooting USA                                    |
    Sushi Bar                                              |
    The King Of Fighters Neowave                           |
    The King Of Fighters XI                                |
                                                           |
---------------------------------------------------------------------------------------------------------------------------------------------------



---------------------------------------------------------
Bios Version Information                                |
---------------------------------------------------------
    Bios                     |   Support | Support      |
    Label                    |   GD-ROM  | Cabinet Link |
---------------------------------------------------------
Naomi / GD-ROM               |           |              |
    EPR-21576D (and earlier) |   No      |    No        |
    EPR-21576E               |   Yes     |    No        |
    EPR-21576F               |   Yes     |    Yes       |
    EPR-21576G (and newer)   |   Yes     |    Yes       |
---------------------------------------------------------
Naomi 2 / GD-ROM             |           |              |
    EPR-23605                |   Yes     |    No        |
    EPR-23605A               |   Yes     |    Yes       |
    EPR-23605B (and newer)   |   Yes     |    Yes       |
---------------------------------------------------------



Sega NAOMI ROM cart usage
-------------------------

837-13668  171-7919A (C) Sega 1998
|----------------------------------------------------------|
|IC1   IC2   IC3   IC4   IC5   IC6         CN2             |
|ROM1  ROM2  ROM3  ROM4  ROM5  ROM6                     JP1|
|                                              IC42        |
|                                       71256              |
|                                       71256         IC22 |
|            IC7  IC8  IC9  IC10  IC11                     |
|            ROM7 ROM8 ROM9 ROM10 ROM11        IC41        |
|                                                          |
|                                              28MHz       |
|                                                          |
|    CN3              X76F100               CN1            |
|----------------------------------------------------------|
Notes:
      Not all MASKROM positions are populated.
      All MASKROMs (IC1 to IC21) are SOP44, either 32M or 64M
      The other side of the cart PCB just has more locations for
      SOP44 MASKROMs... IC12 to IC21 (ROM12 to ROM21)

      IC22    - DIP42 EPROM, either 27C160 or 27C322
      JP1     - Sets the size of the EPROM. 1-2 = 32M, 2-3 = 16M
      IC41    - Xilinx XC9536 (PLCC44)
      IC42    - SEGA 315-5881 (QFP100). Probably some kind of FPGA or CPLD. Usually different per game.
                On the end of the number, -JPN means it requires Japanese BIOS, -COM will run with any BIOS
      X76F100 - Xicor X76F100 secured EEPROM (SOIC8)
      71256   - IDT 71256 32kx8 SRAM (SOJ28)
      CN1/2/3 - connectors joining to main board

      Note! Generally, games that require a special I/O board or controller will not boot at all with a
            standard NAOMI I/O board. Usually they display a message saying the I/O board is not acceptable
            or not connected properly.


Games known to use this PCB include....

                           Sticker    EPROM        # of SOP44
Game                       on cart    IC22#        MASKROMs   IC41#      IC42#          Notes
--------------------------------------------------------------------------------------------------------------
Cosmic Smash               840-0044C  23428A       8          315-6213   317-0289-COM
*Dead Or Alive 2           841-0003C  22121        21         315-6213   317-5048-COM
*Dead Or Alive 2 Millenium 841-0003C  DOA2 Ver.M   21         315-6213   317-5048-COM
*Derby Owners Club         840-0016C  22099B       14         315-6213   317-0262-JPN
*Dynamite Baseball '99     840-0019C  22141B       19         315-6213   317-0269-JPN
*Dynamite Baseball Naomi   840-0001C  21575        21         315-6213   317-0246-JPN
*Giant Gram Pro Wrestle 2  840-0007C  21820        9          315-6213   317-0253-JPN
*Heavy Metal Geo Matrix    HMG016007  23716A       11         315-6213   317-5071-COM
Idol Janshi Suchie-Pai 3   841-0002C  21979        14         315-6213   317-5047-JPN   requires special I/O board and mahjong panel
*Out Trigger               840-0017C  22163        19         315-6213   317-0266-COM   requires analog controllers/special panel
*Power Stone               841-0001C  21597        8          315-6213   317-5046-COM
*Power Stone 2             841-0008C  23127        9          315-6213   317-5054-COM
*Samba de Amigo            840-0020C  22966B       16         315-6213   317-0270-COM   will boot but requires special controller to play it
Sega Marine Fishing        840-0027C  22221        10         315-6213   not populated  ROM 3&4 not populated. Requires special I/O board and fishing controller
*Slash Out                 840-0041C  23341        17         315-6213   317-0286-COM
*Spawn                     841-0005C  22977B       10         315-6213   317-5051-COM
Virtua Striker 2 2000      840-0010C  21929C       15         315-6213   317-0258-COM
*Zombie Revenge            840-0003C  21707        19         315-6213   317-0249-COM

* denotes not dumped yet


*/

#include "driver.h"
#include "video/generic.h"
#include "cpu/sh4/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "dc.h"

#define CPU_CLOCK 200000000
                                 /* MD2 MD1 MD0 MD6 MD4 MD3 MD5 MD7 MD8 */
static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };

static UINT32 *dc_sound_ram;
extern UINT64 *dc_texture_ram;
static UINT32 rom_offset, dma_offset;

static INTERRUPT_GEN( naomi_vblank )
{
	dc_vblank();
}

static READ64_HANDLER( naomi_arm_r )
{
	return *((UINT64 *)dc_sound_ram+offset);
}

static WRITE64_HANDLER( naomi_arm_w )
{
	COMBINE_DATA((UINT64 *)dc_sound_ram + offset);
}

static READ64_HANDLER( naomi_unknown1_r )
{
	if ((offset * 8) == 0xc0) // trick so that it does not "wait for multiboard sync"
		return -1;
	return 0;
}

static WRITE64_HANDLER( naomi_unknown1_w )
{
}

static READ32_HANDLER( dc_aica_arm_r )
{
	return 0;
}

static WRITE32_HANDLER( dc_aica_arm_w )
{
}

/*
    Naomi ROM board info from ElSemi:

    NAOMI_ROM_OFFSETH = 0x5f7000,
    NAOMI_ROM_OFFSETL = 0x5f7004,
    NAOMI_ROM_DATA = 0x5f7008,
    NAOMI_DMA_OFFSETH = 0x5f700C,
    NAOMI_DMA_OFFSETL = 0x5f7010,
    NAOMI_DMA_COUNT = 0x5f7014,
    NAOMI_COMM_OFFSET = 0x5F7050,
    NAOMI_COMM_DATA = 0x5F7054,
    NAOMI_BOARDID_WRITE = 0x5F7078,
    NAOMI_BOARDID_READ = 0x5F707C,
    each port is 16 bit wide, to access the rom in PIO mode, just set an offset in ROM_OFFSETH/L and read from ROM_DATA, each access reads 2 bytes and increases the offset by 2.

    the BOARDID regs access the password protected eeprom in the game board. the main board eeprom is read through port 0x1F800030

    To access the board using DMA, use the DMA_OFFSETL/H. DMA_COUNT is in units of 0x20 bytes. Then trigger a GDROM DMA request.
*/

// NOTE: all accesses are 16 or 32 bits wide but only 16 bits are valid

static READ64_HANDLER( naomi_rom_board_r )
{
	UINT8 *ROM = (UINT8 *)memory_region(REGION_USER1);

	// ROM_DATA
	if ((offset == 1) && ((mem_mask & 0xffff) == 0))
	{
		UINT64 ret;

		ret = (UINT64)(ROM[rom_offset] | (ROM[rom_offset+1]<<8));

		rom_offset += 2;

		return ret;
	}
	else
	{
		mame_printf_verbose("ROM: read mask %llx @ %x (PC=%x)\n", mem_mask, offset, activecpu_get_pc());
	}

	return U64(0xffffffffffffffff);
}

static WRITE64_HANDLER( naomi_rom_board_w )
{
	if ((offset == 1) && ((mem_mask & U64(0xffff)) == 0))
	{
		// DMA_OFFSETH
		dma_offset &= 0xffff;
		dma_offset |= (data & 0x1fff)<<16;
	}
	else if ((offset == 1) && ((mem_mask & U64(0xffff00000000)) == 0))
	{
		// DMA_OFFSETL
		dma_offset &= 0xffff0000;
		dma_offset |= (data & 0xffff);
	}
	else if ((offset == 0) && ((mem_mask & U64(0xffff)) == 0))
	{
		// ROM_OFFSETH
		rom_offset &= 0xffff;
		rom_offset |= (data & 0x1fff)<<16;
	}
	else if ((offset == 0) && ((mem_mask & U64(0xffff00000000)) == 0))
	{
		// ROM_OFFSETL
		rom_offset &= 0xffff0000;
		rom_offset |= (data & 0xffff);
	}
	else
	{
		mame_printf_verbose("ROM: write %llx to %x, mask %llx (PC=%x)\n", data, offset, mem_mask, activecpu_get_pc());
	}
}

static ADDRESS_MAP_START( naomi_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM									// BIOS
	AM_RANGE(0x00200000, 0x00207fff) AM_RAM									// bios uses it (battery backed ram ?)
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE( dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_READWRITE( dc_maple_r, dc_maple_w )
	AM_RANGE(0x005f7000, 0x005f70ff) AM_READWRITE( naomi_rom_board_r, naomi_rom_board_w )
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE( dc_g1_ctrl_r, dc_g1_ctrl_w )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE( dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_READWRITE( pvr_ctrl_r, pvr_ctrl_w )
	AM_RANGE(0x005f8000, 0x005f9fff) AM_READWRITE( pvr_ta_r, pvr_ta_w )
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE( dc_modem_r, dc_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_READWRITE( dc_aica_reg_r, dc_aica_reg_w )
	AM_RANGE(0x00710000, 0x0071000f) AM_READWRITE( dc_rtc_r, dc_rtc_w )
	AM_RANGE(0x00800000, 0x009fffff) AM_READWRITE( naomi_arm_r, naomi_arm_w ) // sound RAM
	AM_RANGE(0x0103ff00, 0x0103ffff) AM_READWRITE( naomi_unknown1_r, naomi_unknown1_w ) 	// bios uses it, actual start and end addresses not known
	AM_RANGE(0x04000000, 0x04ffffff) AM_RAM	AM_SHARE(2) AM_BASE( &dc_texture_ram ) 			// texture memory
	AM_RANGE(0x05000000, 0x05ffffff) AM_RAM AM_SHARE(2)							// mirror of texture RAM
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x0d000000, 0x0dffffff) AM_RAM AM_SHARE(1)							// mirror of main RAM
	AM_RANGE(0x10000000, 0x107fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x10800000, 0x10ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x11000000, 0x11ffffff) AM_RAM AM_SHARE(2)							// another mirror of texture memory
	AM_RANGE(0xa0000000, 0xa01fffff) AM_ROM AM_REGION(REGION_CPU1, 0)
ADDRESS_MAP_END

static READ32_HANDLER( test1 )
{
	return -1;
}

static ADDRESS_MAP_START( dc_audio_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM	AM_BASE( &dc_sound_ram )	/* shared with SH-4 */
	AM_RANGE(0x00200000, 0x002000ff) AM_READ( test1 )	// for bug (?) in sound bios
	AM_RANGE(0x00800000, 0x00807fff) AM_READWRITE( dc_aica_arm_r, dc_aica_arm_w )	/* shared with SH-4 */
ADDRESS_MAP_END

static INPUT_PORTS_START( naomi )
	PORT_START
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)
INPUT_PORTS_END

static MACHINE_DRIVER_START( naomi )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", SH4, CPU_CLOCK) // SH4!!!
	MDRV_CPU_CONFIG(sh4cpu_config)
	MDRV_CPU_PROGRAM_MAP(naomi_map,0)
	MDRV_CPU_VBLANK_INT(naomi_vblank,479)

	MDRV_CPU_ADD_TAG("sound", ARM7, 45000000)
	MDRV_CPU_PROGRAM_MAP(dc_audio_map, 0)

	MDRV_MACHINE_RESET( dc )

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(dc)
	MDRV_VIDEO_UPDATE(dc)
MACHINE_DRIVER_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

/* BIOS info:

Revisions A through C can handle game carts only
Revisions D and later can also handle GD-Rom board
Revisions F and later can also handle GD-Rom board and or the network GD-Rom board

F355 has it's own BIOS (3 screen version) and different JVS I/O Board

Info from roms starting at 0x1ffd60

EPR-21576g - NAOMI BOOT ROM 2001 09/10  1.70 (Japan)
EPR-21577e - NAOMI BOOT ROM 2000 08/25  1.50 (USA)
EPR-21577d - NAOMI BOOT ROM 1999 06/04  1.40 (USA)
EPR-21578e - NAOMI BOOT ROM 2000 08/25  1.50 (Export)
EPR-21578d - NAOMI BOOT ROM 1999 06/04  1.40 (Export)
EPR-21578a - NAOMI BOOT ROM 1999 02/15  1.20 (Export)
EPR-21579  - No known dump (Korea)
EPR-21580  - No known dump (Australia)
EPR-22851  - NAOMI BOOT ROM 1999 08/30  1.35 (Multisystem 3 screen Ferrari F355)

EPR-21577e & EPR-2178e differ by 7 bytes:

0x53e20 is the region byte (only one region byte)
0x1ffffa-0x1fffff is the BIOS checksum


House of the Dead 2 specific Naomi BIOS roms:

Info from roms starting at 0x1ff060

EPR-21330  - HOUSE OF THE DEAD 2 IPL ROM 1998 11/14 (USA)
EPR-21331  - HOUSE OF THE DEAD 2 IPL ROM 1998 11/14 (Export)

EPR-21330 & EPR-21331 differ by 7 bytes:

0x40000 is the region byte (only one region byte)
0x1ffffa-0x1fffff is the BIOS checksum


Region byte encoding is as follows:

0x00 = Japan
0x01 = USA
0x02 = Export
0x?? = Korea
0x?? = Australia

Scan ROM for the text string "LOADING TEST MODE NOW" back up four (4) bytes for the region byte.
  NOTE: this doesn't work for the HOTD2 or multi screen boot roms

*/

#define NAOMI_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr-21578e (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-21578e.bin",  0x000000, 0x200000, CRC(087f09a3) SHA1(0418eb2cf9766f0b1b874a4e92528779e22c0a4a) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr-21578d (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-21578d.bin",  0x000000, 0x200000, CRC(dfd5f42a) SHA1(614a0db4743a5e5a206190d6786ade24325afbfd) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr-21578a (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-21578a.bin",  0x000000, 0x200000, CRC(6c9aad83) SHA1(555918de76d8dbee2a97d8a95297ef694b3e803f) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "epr-21577e (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "epr-21577e.bin",  0x000000, 0x200000, CRC(cf36e97b) SHA1(b085305982e7572e58b03a9d35f17ae319c3bbc6) ) \
	ROM_SYSTEM_BIOS( 4, "bios4", "epr-21577d (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "epr-21577d.bin",  0x000000, 0x200000, CRC(60ddcbbe) SHA1(58b15096d269d6df617ca1810b66b47deb184958) ) \
	ROM_SYSTEM_BIOS( 5, "bios5", "epr-21576g (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "epr-21576g.bin",  0x000000, 0x200000, CRC(d2a1c6bf) SHA1(6d27d71aec4dfba98f66316ae74a1426d567698a) ) \
	ROM_SYSTEM_BIOS( 6, "bios6", "Ferrari F355" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "epr-22851.bin",   0x000000, 0x200000, CRC(62483677) SHA1(3e3bcacf5f972c376b569f45307ee7fd0b5031b7) ) \
	ROM_SYSTEM_BIOS( 7, "bios7", "HOTD2 (US)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "epr-21330.bin",   0x000000, 0x200000, CRC(9e3bfa1b) SHA1(b539d38c767b0551b8e7956c1ff795de8bbe2fbc) ) \
	ROM_SYSTEM_BIOS( 8, "bios8", "HOTD2 (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "epr-21331.bin",   0x000000, 0x200000, CRC(065f8500) SHA1(49a3881e8d76f952ef5e887200d77b4a415d47fe) ) \
	ROM_SYSTEM_BIOS( 9, "bios9", "Naomi Dev BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9, "dcnaodev.bios",   0x000000, 0x080000, CRC(7a50fab9) SHA1(ef79f448e0bf735d1264ad4f051d24178822110f) ) /* This one comes from a dev / beta board. The eprom was a 27C4096 */


/* NAOMI2 BIOS:

EPR-23605  - NAOMI BOOT ROM 2001 01/19  1.50
EPR-23607  - NAOMI BOOT ROM 2001 01/19  1.50
EPR-23608  - NAOMI BOOT ROM 2001 01/19  1.50
EPR-2360xB - NAOMI BOOT ROM 2001 09/10  1.70

EPR-23605B, EPR-23607B & EPR-23608B all differ by 8 bytes:

0x0553a0 is the first region byte
0x1ecf40 is a second region byte (value is the same as the first region byte )
0x1fffa-1ffff is the BIOS rom checksum

EPR-23605 & EPR-23605b - Japan  (region = 0x00)
EPR-23607 & EPR-23607b - USA    (region = 0x01)
EPR-23608 & EPR-23608b - Export (region = 0x02)

*/

#define NAOMI2_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr-23608b" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-23608b.bin",   0x000000, 0x200000, CRC(a554b1e3) SHA1(343b727a3619d1c75a9b6d4cc156a9050447f155) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr-23608"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-23608.bin",    0x000000, 0x200000, CRC(929cc3a6) SHA1(47d00c818de23f733a4a33b1bbc72eb8aa729246) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr-23607b" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-23607b.bin",   0x000000, 0x200000, CRC(f308c5e9) SHA1(5470ab1cee6afecbd8ca8cf40f8fbe4ec2cb1471) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "epr-23607"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "epr-23607.bin",    0x000000, 0x200000, CRC(2b55add2) SHA1(547de5f97d3183c8cd069c4fa3c09f13d8b637d9) ) \
	ROM_SYSTEM_BIOS( 4, "bios4", "epr-23605b" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "epr-23605b.bin",   0x000000, 0x200000, CRC(3a3242d4) SHA1(aaca4df51ef91d926f8191d372f3dfe1d20d9484) ) \
	ROM_SYSTEM_BIOS( 5, "bios5", "epr-23605"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "epr-23605.bin",    0x000000, 0x200000, CRC(5731e446) SHA1(787b0844fc408cf124c12405c095c59948709ea6) )

/* this is one flashrom, however the second half looks like it's used for game settings, may differ between dumps, and may not be needed / could be blanked */
#define AW_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "Atomiswave BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "bios.ic23_l",                         0x000000, 0x010000, BAD_DUMP CRC(e5693ce3) SHA1(1bde3ed87af64b0f675ebd47f12a53e1fc5709c1) ) /* Might be bad.. especially. bytes 0x0000, 0x6000, 0x8000 which gave different reads */ \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "bios.ic23_h-dolhpin_blue_settings",   0x010000, 0x010000, BAD_DUMP CRC(5d5687c7) SHA1(2600ce09c44872d1793f6b55bf44342673da5ad1) ) /* it appears to flash settings game data here */ /* this is one flashrom, however the second half looks like it's used for game settings, may differ between dumps, and may not be needed / could be blanked */


ROM_START( naomi )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x8400000, REGION_USER1, ROMREGION_ERASE)
ROM_END

ROM_START( naomi2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI2_BIOS

	ROM_REGION( 0x8400000, REGION_USER1, ROMREGION_ERASE)
ROM_END

ROM_START( awbios )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	AW_BIOS

	ROM_REGION( 0x8400000, REGION_USER1, ROMREGION_ERASE)
ROM_END

/* Info above each set is automatically generated from the IC22 rom and may not be accurate */

/*
SYSTEMID: NAOMI
JAP: GUN SPIKE
USA: CANNON SPIKE
EXP: CANNON SPIKE

NO.     Type    Byte    Word
IC22    32M     0000*   0000* invalid value
IC1     64M     7AC6    C534
IC2     64M     3959    6667
IC3     64M     F60D    69E5
IC4     64M     FBD4    AE40
IC5     64M     1717    F3EC
IC6     64M     A622    1D3D
IC7     64M     33A3    4480
IC8     64M     FC26    A49D
IC9     64M     528D    5206
IC10    64M     7C94    8779
IC11    64M     271E    BEF7
IC12    64M     BA24    102F
*/

ROM_START( cspike )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23210.22", 0x00000, 0x400000,  CRC(a15c54b5) SHA1(5c7872244d3d648e4c04751f120d0e9d47239921) )

	ROM_REGION( 0x8000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: CAPCOM VS SNK  JAPAN
USA: CAPCOM VS SNK  USA
EXP: CAPCOM VS SNK  EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     B836    4AA4
IC2     64M     19C1    9965
IC3     64M     B98C    EFB2
IC4     64M     2458    31CD
IC5     64M     59D2    E957
IC6     64M     1004    7E0B
IC7     64M     C63F    B2A7
IC8     64M     9D78    342F
IC9     64M     681F    D97A
IC10    64M     7544    E4D3
IC11    64M     8351    8A4C
IC12    64M     B713    2408
IC13    64M     A12E    8DE4

*/

ROM_START( capsnk )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23511a.ic22", 0x00000, 0x400000,  CRC(fe00650f) SHA1(ca8e9e9178ed2b6598bdea83be1bf0dd7aa509f9) )

	ROM_REGION( 0x8000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: COSMIC SMASH IN JAPAN
USA: COSMIC SMASH IN USA
EXP: COSMIC SMASH IN EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000     EPR23428A.22
IC1     64M     C82B    E769     MPR23420.1
IC2     64M     E0C3    43B6     MPR23421.2
IC3     64M     C896    F766     MPR23422.3
IC4     64M     2E60    4CBF     MPR23423.4
IC5     64M     BB81    7E26     MPR23424.5
IC6     64M     B3A8    F2EA     MPR23425.6
IC7     64M     05C5    A084     MPR23426.7
?IC8     64M     9E13    7535     MPR23427.8

Serial: BCHE-01A0803

*/

ROM_START( csmash )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x4800000, REGION_USER1, ROMREGION_ERASE00)
	ROM_LOAD("epr23428a.22", 0x0000000, 0x400000, CRC(d628dbce) SHA1(91ec1296ead572a64c37f8ac2c1a96742f19d50b) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr23420.1", 0x0800000, 0x0800000, CRC(9d5991f2) SHA1(c75871db314b01935d1daaacf1a762e73e5fd411) )
	ROM_LOAD("mpr23421.2", 0x1000000, 0x0800000, CRC(6c351db3) SHA1(cdd601321a38fc34152517abdc473b73a4c6f630) )
	ROM_LOAD("mpr23422.3", 0x1800000, 0x0800000, CRC(a1d4bd29) SHA1(6c446fd1819f55412351f15cf57b769c0c56c1db) )
	ROM_LOAD("mpr23423.4", 0x2000000, 0x0800000, CRC(08cbf373) SHA1(0d9a593f5cc5d632d85d7253c135eef2e8e01598) )
	ROM_LOAD("mpr23424.5", 0x2800000, 0x0800000, CRC(f4404000) SHA1(e49d941e47e63bb7f3fddc3c3d2c1653611914ee) )
	ROM_LOAD("mpr23425.6", 0x3000000, 0x0800000, CRC(47f51da2) SHA1(af5ecd460114caed3a00157ffd3a2df0fbf348c0) )
	ROM_LOAD("mpr23426.7", 0x3800000, 0x0800000, CRC(7f91b13f) SHA1(2d534f77291ebfedc011bf0e803a1b9243fb477f) )
	ROM_LOAD("mpr23427.8", 0x4000000, 0x0800000, CRC(5851d525) SHA1(1cb1073542d75a3bcc0d363ed31d49bcaf1fd494) )
ROM_END

/*

SYSTEMID: NAOMI
JAP:  DERBY OWNERS CLUB ------------
USA:  DERBY OWNERS CLUB ------------
EXP:  DERBY OWNERS CLUB IN EXPORT --

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     8AF3    D0BC
IC2     64M     1E79    0410
IC3     64M     146D    C51E
IC4     64M     E9AD    86BE
IC5     64M     BBB2    8685
IC6     64M     A0E1    C2E0
IC7     64M     B8CF    67B5
IC8     64M     005E    C1D6
IC9     64M     1F53    9304
IC10    64M     FAC9    8AA4
IC11    64M     B6B1    5665
IC12    64M     21DB    74F5
IC13    64M     A991    A8AB
IC14    64M     05BD    428D

Serial: BAXE-02A1386

*/

ROM_START( derbyoc )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22099b.22", 0x0000000, 0x0400000,  CRC(5e708879) SHA1(fada4f4bf29fc8f77f354167f8db4f904610fe1a) )

	ROM_REGION( 0x8000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DYNAMITE BASEBALL '99
USA: WORLD SERIES 99
EXP: WORLD SERIES 99

NO.     Type    Byte    Word
IC22    16M     0000    0000
IC1     64M     77B9    3C1B
IC2     64M     F7FB    025A
IC3     64M     B3D4    22C1
IC4     64M     060F    6279
IC5     64M     FE49    CAEB
IC6     64M     E34C    5FAD
IC7     64M     CC04    498C
IC8     64M     388C    DF17
IC9     64M     5B91    C458
IC10    64M     AF73    4A18
IC11    64M     2E5B    A198
IC12    64M     FFDB    41CA
IC13    64M     04E1    EA4C
IC14    64M     5B22    DA9A
IC15    64M     64E7    0873
IC16    64M     1EE7    BE11
IC17    64M     79C3    3608
IC18    64M     D4CE    5AEB
IC19    64M     E846    60B8

Serial: BBDE-01A0097

*/

ROM_START( dybb99 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22141b.22", 0x0000000, 0x0200000,  CRC(6d0e0785) SHA1(aa19e7bac4c266771d1e65cffa534a49d7566f51) )

	ROM_REGION( 0x9800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: GIANT GRAM 2000
USA: GIANT GRAM 2000
EXP: GIANT GRAM 2000

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     904A    81AE
IC2     64M     E9F7    B152
IC3     64M     A4D0    8FB7
IC4     64M     A869    64FB
IC5     64M     30CB    3483
IC6     64M     94DD    7F14
IC7     64M     BA8B    EA07
IC8     64M     6ADA    5CDA
IC9     64M     7CDA    86C1
IC10    64M     86F2    73A3
IC11    64M     44D8    1D11
IC12    64M     F25E    EDA8
IC13    64M     4804    6251
IC14    64M     E4FE    3808
IC15    64M     FD3D    D37A
IC16    64M     6D48    F5B3
IC17    64M     F0C6    CA29
IC18    64M     07C3    E4AE
IC19    64M     50F8    8500
IC20    64M     4EA2    D0CE
IC21    64M     090B    5667

Serial: BCCG-21A0451

*/

ROM_START( gram2000 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23377.22", 0x0000000, 0x0400000,  CRC(4ca3149c) SHA1(9d25fc659658b416202b033754669be2f3abcdbe) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END

/*
SYSTEMID: NAOMI
JAP: GIANT GRAM
USA: GIANT GRAM
EXP: GIANT GRAM

NO.     Type    Byte    Word
IC22    16M     0000    1111
IC1     64M     E504    548E

Serial: BAJE-01A0021
*/

ROM_START( ggram2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr21820.22", 0x0000000, 0x0200000,  CRC(0a198278) SHA1(0df5fc8b56ddafc66d92cb3923b851a5717b551d) )

	ROM_REGION( 0x0800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: HEAVY METAL JAPAN
USA: HEAVY METAL USA
EXP: HEAVY METAL EURO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     CBA3    16D2
IC2     64M     087A    079B
IC3     64M     CDB0    804C
IC4     64M     326A    E815
IC5     64M     C164    5DB4
IC6     64M     38A0    AAFC
IC7     64M     1134    DFCC
IC8     64M     6597    6975
IC9     64M     D6FB    8917
IC10    64M     6442    18AC
IC11    64M     4F77    EEFE

*/


ROM_START( hmgeo )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23716a.22", 0x0000000, 0x0400000,  CRC(c5cb0d3b) SHA1(20de8f5ee183e996ccde77b10564a302939662db) )

	ROM_REGION( 0x5800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: GIGAWING2 JAPAN
USA: GIGAWING2 USA
EXP: GIGAWING2 EXPORT

NO.     Type    Byte    Word
IC22    16M     C1C3    618F
IC1     64M     8C09    3A15
IC2     64M     91DC    C17F
IC3     64M     25CB    2AA0
IC4     64M     EB35    C1FF
IC5     64M     8B25    914E
IC6     64M     72CB    68FA
IC7     64M     191E    2AF3
IC8     64M     EACA    12CD
IC9     64M     717F    40ED
IC10    64M     1E43    0F1A

*/

ROM_START( gwing2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22270.22", 0x0000000, 0x0200000,  CRC(876b3c97) SHA1(eb171d4a0521c3bea42b4aae3607faec63e10581) )

	ROM_REGION( 0x5800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
ROM_END

/*
SYSTEMID: NAOMI
JAP: IDOL JANSHI SUCHIE-PAI 3
USA: DISABLE
EXP: DISABLE

NO.     Type    Byte    Word
IC22    16M     0000    0000
IC1     64M     E467    524B
IC2     64M     9D05    4992
IC3     64M     E3F7    6481
IC4     64M     6C22    25E3
IC5     64M     180F    E89F
IC6     64M     60C9    2B86
IC7     64M     4EDE    4539
IC8     64M     3AD3    0046
IC9     64M     8D37    BA16
IC10    64M     8AE3    4D71
IC11    64M     B519    1393
IC12    64M     4695    B159
IC13    64M     536F    D0C6
IC14    32M     81F9    DA1B
*/

ROM_START( suchie3 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, REGION_USER1, 0)
	ROM_LOAD("epr21979.22",   0x0000000, 0x0200000, CRC(335c9e25) SHA1(476790fdd99a8c13336e795b4a39b071ed86a97c) )

	ROM_LOAD("mpr-21980.ic1", 0x0800000, 0x0800000, CRC(2b5f958a) SHA1(609585dda27c5e111378a92f04fa03ae11d42540) )
	ROM_LOAD("mpr-21981.ic2", 0x1000000, 0x0800000, CRC(b4fff4ee) SHA1(333fb5a662775662881154b654233f207782a8aa) )
	ROM_LOAD("mpr-21982.ic3", 0x1800000, 0x0800000, CRC(923ee0ff) SHA1(4f92cc1abfd948a1ed15fdca11251aba96bdc022) )
	ROM_LOAD("mpr-21983.ic4", 0x2000000, 0x0800000, CRC(dd659ab1) SHA1(96d9825fc5cf72a9ef83f10e480fd8925b1d6762) )
	ROM_LOAD("mpr-21984.ic5", 0x2800000, 0x0800000, CRC(b34de0c7) SHA1(dbb7a6a19af2571441b5ecbddddae6891809ffcf) )
	ROM_LOAD("mpr-21985.ic6", 0x3000000, 0x0800000, CRC(f1516e0a) SHA1(246d287df592cd69df689dc10e8647a9dbf804b7) )
	ROM_LOAD("mpr-21986.ic7", 0x3800000, 0x0800000, CRC(2779c418) SHA1(8d1a89ddf0c68f1eaf6eb0dafadf9b614492fff1) )
	ROM_LOAD("mpr-21987.ic8", 0x4000000, 0x0800000, CRC(6aaaacdd) SHA1(f5e67c88db8bce8f2f4cab73a5d0a24ba57c812b) )
	ROM_LOAD("mpr-21988.ic9", 0x4800000, 0x0800000, CRC(ed61b155) SHA1(679124f0f7c7bc4791025cff274d903cf5bcae70) )
	ROM_LOAD("mpr-21989.ic10",0x5000000, 0x0800000, CRC(ae8562cf) SHA1(e31986e53159729434a7952e8c4ed2adf8dd8e9d) )
	ROM_LOAD("mpr-21990.ic11",0x5800000, 0x0800000, CRC(57fd9fdd) SHA1(62b3bc4a2828751459557b63d900ca6d46792e24) )
	ROM_LOAD("mpr-21991.ic12",0x6000000, 0x0800000, CRC(d82f834a) SHA1(06902713bdf6f68182749916cacc9ae6528dc355) )
	ROM_LOAD("mpr-21992.ic13",0x6800000, 0x0800000, CRC(599a2fb8) SHA1(2a0007064ad2ee1e1a0fda1d5676df4ff19a9f2f) )
	ROM_LOAD("mpr-21993.ic14",0x7000000, 0x0400000, CRC(fb28cf0a) SHA1(d51b1d4514a93074d1f77bd1bc5995739604cf56) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: MOERO JUSTICE GAKUEN  JAPAN
USA: PROJECT JUSTICE  USA
EXP: PROJECT JUSTICE  EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     3E87    5491
IC2     64M     2789    9802
IC3     64M     60E7    E775
IC4     64M     36F4    9353
IC5     64M     31B6    CEF6
IC6     64M     3F79    7B58
IC7     64M     620C    A31F
IC8     64M     A093    160C
IC9     64M     4DD9    4184
IC10    64M     AF3F    C64A
IC11    64M     0EE1    A0C2
IC12    64M     2EF9    E0A3
IC13    64M     72A5    3156
IC14    64M     D414    B896
IC15    64M     7BCE    3A7A
IC16    64M     E371    962D
IC17    64M     E813    E342
IC18    64M     D2B8    3989
IC19    64M     3A4B    4614
IC20    64M     11B0    9921
IC21    64M     698C    7A39

Serial: BCLE-01A2130

*/

ROM_START( pjustic )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23548a.22", 0x0000000, 0x0400000,  CRC(f4ccf1ec) SHA1(97485b2a4b9452ffeea2501f42d20d718410e716) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: POWER STONE JAPAN
USA: POWER STONE USA
EXP: POWER STONE EURO

NO. Type    Byte    Word
IC22    16M 0000    0000
IC1 64M 0258    45D8
IC2 64M 0DF2    0810
IC3 64M 5F93    9FAF
IC4 64M 05E0    C80F
IC5 64M F023    3F68
IC6 64M 941E    F563
IC7 64M 374E    46F6
IC8 64M C529    0501

*/

ROM_START( pstone )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr21597.22", 0x0000000, 0x0200000,  CRC(62c7acc0) SHA1(bb61641a7f3650757132cde379447bdc9bd91c78) )

	ROM_REGION( 0x4000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: POWER STONE 2 JAPAN
USA: POWER STONE 2 USA
EXP: POWER STONE 2 EURO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     04FF    B3D4
IC2     64M     52D4    0BF0
IC3     64M     5273    0EB8
IC4     64M     B39A    21F5
IC5     64M     53CB    6540
IC6     64M     0AC8    74ED
IC7     64M     D05A    EB30
IC8     64M     8217    4E66
IC9     64M     193C    6851

Serial: BBJE-01A1613

*/

ROM_START( pstone2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23127.22", 0x0000000, 0x0400000,  CRC(185761d6) SHA1(8c91b594dd59313d249c9da7b39dee21d3c9082e) )

	ROM_REGION( 0x4800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: OUTTRIGGER     JAPAN
USA: OUTTRIGGER     USA
EXP: OUTTRIGGER     EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     362E    D34B
IC2     64M     4EF4    FF8D
IC3     64M     5E77    9052
IC4     64M     E123    41B3
IC5     64M     43A0    58D4
IC6     64M     C946    D3EE
IC7     64M     5313    3F17
IC8     64M     2591    FEB7
IC9     64M     CBA3    E150
IC10    64M     2639    D291
IC11    64M     3A96    86EA
IC12    64M     8586    3ED5
IC13    64M     9028    E59C
IC14    64M     8A42    26E2
IC15    64M     98C4    1618
IC16    64M     122B    8C85
IC17    64M     3D5E    F9B0
IC18    64M     1EFA    490E
IC19    64M     9F22    6F77

Serial (from 2 carts): BAZE-01A0288
                       BAZE-02A0217

*/

ROM_START( otrigger )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22163.22", 0x0000000, 0x0400000, CRC(3bdafb6a) SHA1(c4c5a4ba94d85c4353df22d70bb08be67e9c22c3) )

	ROM_REGION( 0x9800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SAMBA DE AMIGO
USA: SAMBADEAMIGO
EXP: SAMBADEAMIGO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     B1FA    1BE9
IC2     64M     51FD    0C32
IC3     64M     8AA0    6E7A
IC4     64M     3B30    E31D
IC5     64M     D604    FBE3
IC6     64M     1D51    FF2D
IC7     64M     EE89    720D
IC8     64M     0551    7046
IC9     64M     6883    6427
IC10    64M     70E5    CEC3
IC11    64M     E70E    0C63
IC12    64M     0FD0    B1F8
IC13    64M     2D48    6B19
IC14    64M     CBFF    F163
IC15    64M     10D1    E09D
IC16    64M     A10B    DDB4

*/

ROM_START( samba )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22966b.ic22", 0x0000000, 0x0400000,  CRC(893116b8) SHA1(35cb4f40690ff21af5ab7cc5adbc53228d6fb0b3) )

	ROM_REGION( 0x8000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SLASHOUT JAPAN VERSION
USA: SLASHOUT USA VERSION
EXP: SLASHOUT EXPORT VERSION

NO.  Type   Byte    Word
IC22 32M    0000    0000
IC1  64M    D1BF    FB18
IC2  64M    1F98    4295
IC3  64M    5F61    67E3
IC4  64M    C6A4    449B
IC5  64M    BB2A    58AB
IC6  64M    60B2    5262
IC7  64M    178B    3705
IC8  64M    E4B9    FF46
IC9  64M    D4FC    2273
IC10 64M    6BA5    8087
IC11 64M    7DBA    A143
IC12 64M    B708    0C61
IC13 64M    0C4A    8DF0
IC14 64M    B2FF    A057
IC15 64M    60DB    3D06
IC16 64M    B5EA    4965
IC17 64M    6586    1F3F

*/

ROM_START( slasho )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr23341.22", 0x0000000, 0x0400000,  CRC(477fa123) SHA1(d2474766dcd0b0e5fe317a858534829eb1c26789) )

	ROM_REGION( 0x8800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SPAWN JAPAN
USA: SPAWN USA
EXP: SPAWN EURO

NO.     Type    Byte    Word
IC22    32M     FFFF    FFFF
IC1     64M     C56E    3D11
IC2     64M     A206    CC87
IC3     64M     FD3F    C5DF
IC4     64M     5833    09A4
IC5     64M     B42C    AA08
IC6     64M     C7A4    E2DE
IC7     64M     58CB    5DFD
IC8     64M     144B    783D
IC9     64M     A4A8    D0BE
IC10    64M     94A8    401F

Serial: BAVE-02A1305

*/

ROM_START( spawn )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22977b.22", 0x0000000, 0x0400000,  CRC(814ff5d1) SHA1(5a0a9e55878927f98750000eb7d9391cbecfe21d) )

	ROM_REGION( 0x5000000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: VIRTUA NBA
USA: VIRTUA NBA
EXP: VIRTUA NBA

NO.     Type    Byte    Word
IC22    32M 0000    0000
IC1     64M 5C4A    BB88
IC2     64M 1799    B55E
IC3     64M FB19    6FE8
IC4     64M 6207    33FE
IC5     64M 38F0    F24C
IC6     64M A3B1    FF6F
IC7     64M 737F    B4DD
IC8     64M FD19    49CE
IC9     64M 424E    76D5
IC10    64M 84CC    B74C
IC11    64M 8FC6    D9C8
IC12    64M A838    143A
IC13    64M 88C3    456F
IC14    64M 1C72    971E
IC15    64M B950    F203
IC16    64M 39F6    54CE
IC17    64M 91C7    47B0
IC18    64M 5B94    7E77
IC19    64M DE42    F390
IC20    64M B876    73CE
IC21    64M AD60    2F74

*/

ROM_START( virnba )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22949.22", 0x0000000, 0x0400000,  CRC(fd91447e) SHA1(0759d6517aeb684d0cb809c1ae1350615cc0aecc) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: VIRTUA STRIKER 2 VER.2000
USA: VIRTUA STRIKER 2 VER.2000
EXP: VIRTUA STRIKER 2 VER.2000

NO.     Type    Byte    Word
IC22    32M     2B49    A054    EPR21929C.22
IC1     64M     F5DD    E983    MPR21914
IC2     64M     4CB7    198B    MPR21915
IC3     64M     5661    47C0    MPR21916
IC4     64M     CD15    DC9A    MPR21917
IC5     64M     7855    BCC7    MPR21918
IC6     64M     59D2    CB75    MPR21919
IC7     64M     B795    BE9C    MPR21920
IC8     64M     D2DE    5AF2    MPR21921
IC9     64M     7AAD    0DD5    MPR21922
IC10    64M     B31B    2C4E    MPR21923
IC11    64M     5C32    D746    MPR21924
IC12    64M     1886    D5EA    MPR21925
IC13    64M     D7B3    24D7    MPR21926
IC14    64M     9EF2    E513    MPR21927
IC15    32M     0DF9    FC01    MPR21928

*/

ROM_START( vs2_2k )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, REGION_USER1, 0)
	ROM_LOAD("epr21929c.22", 0x0000000, 0x0400000,  CRC(b5e609f4) SHA1(92b98bad94268a80f6e4506b812d01c0a7850161) )
	ROM_LOAD("mpr21914.u1", 0x0800000, 0x0800000, CRC(f91ef69b) SHA1(4ed23091efad7ddf1878a0bfcdcbba3cf151af84) )
	ROM_LOAD("mpr21915.u2", 0x1000000, 0x0800000, CRC(40128a67) SHA1(9d191c4ec33465f29bbc09491dde62f354a9ab15) )
	ROM_LOAD("mpr21916.u3", 0x1800000, 0x0800000, CRC(19708b3c) SHA1(7d1ef995ce870ffcb68f420a571efb084f5bfcf2) )
	ROM_LOAD("mpr21917.u4", 0x2000000, 0x0800000, CRC(b082379b) SHA1(42f585279da1de7e613e42b76e1b81986c48e6ea) )
	ROM_LOAD("mpr21918.u5", 0x2800000, 0x0800000, CRC(a3bc1a47) SHA1(0e5043ab6e118feb59f68c84c095cf5b1dba7d09) )
	ROM_LOAD("mpr21919.u6", 0x3000000, 0x0800000, CRC(b1dfada7) SHA1(b4c906bc96b615975f6319a1fdbd5b990e7e4124) )
	ROM_LOAD("mpr21920.u7", 0x3800000, 0x0800000, CRC(1c189e28) SHA1(93400de2cb803357fa17ae7e1a5297177f9bcfa1) )
	ROM_LOAD("mpr21921.u8", 0x4000000, 0x0800000, CRC(55bcb652) SHA1(4de2e7e584dd4999dc8e405837a18a904dfee0bf) )
	ROM_LOAD("mpr21922.u9", 0x4800000, 0x0800000, CRC(2ecda3ff) SHA1(54afc77a01470662d580f5676b4e8dc4d04f63f8) )
	ROM_LOAD("mpr21923.u10",0x5000000, 0x0800000, CRC(a5cd42ad) SHA1(59f62e995d45311b1592434d1ffa42c261fa8ba1) )
	ROM_LOAD("mpr21924.u11",0x5800000, 0x0800000, CRC(cc1a4ed9) SHA1(0e3aaeaa55f1d145fb4877b6d187a3ee78cf214e) )
	ROM_LOAD("mpr21925.u12",0x6000000, 0x0800000, CRC(9452c5fb) SHA1(5a04f96d83cca6248f513de0c6240fc671bcadf9) )
	ROM_LOAD("mpr21926.u13",0x6800000, 0x0800000, CRC(d6346491) SHA1(830971cbc14cab022a09ad4c6e11ee49c550e308) )
	ROM_LOAD("mpr21927.u14",0x7000000, 0x0800000, CRC(a1901e1e) SHA1(2281f91ac696cc14886bcdf4b0685ce2f5bb8117) )
	ROM_LOAD("mpr21928.u15",0x7800000, 0x0400000, CRC(d127d9a5) SHA1(78c95357344ea15469b84fa8b1332e76521892cd) )
ROM_END

ROM_START( vs2_2ka )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, REGION_USER1, 0)
	ROM_LOAD("u22", 0x0000000, 0x0400000, CRC(831af08a) SHA1(af4c74623be823fd061765cede354c6a9722fd10) )
	ROM_LOAD("mpr21914.u1", 0x0800000, 0x0800000, CRC(f91ef69b) SHA1(4ed23091efad7ddf1878a0bfcdcbba3cf151af84) )
	ROM_LOAD("mpr21915.u2", 0x1000000, 0x0800000, CRC(40128a67) SHA1(9d191c4ec33465f29bbc09491dde62f354a9ab15) )
	ROM_LOAD("mpr21916.u3", 0x1800000, 0x0800000, CRC(19708b3c) SHA1(7d1ef995ce870ffcb68f420a571efb084f5bfcf2) )
	ROM_LOAD("mpr21917.u4", 0x2000000, 0x0800000, CRC(b082379b) SHA1(42f585279da1de7e613e42b76e1b81986c48e6ea) )
	ROM_LOAD("mpr21918.u5", 0x2800000, 0x0800000, CRC(a3bc1a47) SHA1(0e5043ab6e118feb59f68c84c095cf5b1dba7d09) )
	ROM_LOAD("mpr21919.u6", 0x3000000, 0x0800000, CRC(b1dfada7) SHA1(b4c906bc96b615975f6319a1fdbd5b990e7e4124) )
	ROM_LOAD("mpr21920.u7", 0x3800000, 0x0800000, CRC(1c189e28) SHA1(93400de2cb803357fa17ae7e1a5297177f9bcfa1) )
	ROM_LOAD("mpr21921.u8", 0x4000000, 0x0800000, CRC(55bcb652) SHA1(4de2e7e584dd4999dc8e405837a18a904dfee0bf) )
	ROM_LOAD("mpr21922.u9", 0x4800000, 0x0800000, CRC(2ecda3ff) SHA1(54afc77a01470662d580f5676b4e8dc4d04f63f8) )
	ROM_LOAD("mpr21923.u10",0x5000000, 0x0800000, CRC(a5cd42ad) SHA1(59f62e995d45311b1592434d1ffa42c261fa8ba1) )
	ROM_LOAD("mpr21924.u11",0x5800000, 0x0800000, CRC(cc1a4ed9) SHA1(0e3aaeaa55f1d145fb4877b6d187a3ee78cf214e) )
	ROM_LOAD("mpr21925.u12",0x6000000, 0x0800000, CRC(9452c5fb) SHA1(5a04f96d83cca6248f513de0c6240fc671bcadf9) )
	ROM_LOAD("mpr21926.u13",0x6800000, 0x0800000, CRC(d6346491) SHA1(830971cbc14cab022a09ad4c6e11ee49c550e308) )
	ROM_LOAD("mpr21927.u14",0x7000000, 0x0800000, CRC(a1901e1e) SHA1(2281f91ac696cc14886bcdf4b0685ce2f5bb8117) )
	ROM_LOAD("mpr21928.u15",0x7800000, 0x0400000, CRC(d127d9a5) SHA1(78c95357344ea15469b84fa8b1332e76521892cd) )
ROM_END

/* Sega Marine Fishing */

ROM_START( smarinef )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, REGION_USER1, 0)
	ROM_LOAD("epr-22221.ic22", 0x0000000, 0x0400000, CRC(9d984375) SHA1(fe1185d70b4bc1529e3579fd6b2b678c7d548400) )
	ROM_LOAD("mpr-22208.ic1", 0x0800000, 0x0800000, CRC(6a1e418c) SHA1(7092c6a34ac0c2c6fb2b4b78415d08ef473785d9) )
	ROM_LOAD("mpr-22209.ic2", 0x1000000, 0x0800000, CRC(ecf5be54) SHA1(d7c264da4e232ce6f9b05c9920394f8027fa4a1d) )
	/* IC3 empty */
	/* IC4 empty */
	ROM_LOAD("mpr-22212.ic5", 0x2800000, 0x0800000, CRC(8305f462) SHA1(7993231fa71f509b3b7fec691b5a6139947a01e7) )
	ROM_LOAD("mpr-22213.ic6", 0x3000000, 0x0800000, CRC(0912eaea) SHA1(e4cb1262f3b53d3c619900767cfa192115a53d4b) )
	ROM_LOAD("mpr-22214.ic7", 0x3800000, 0x0800000, CRC(661526b6) SHA1(490321a893f706eaea49c6c35c01af6ae45adf01) )
	ROM_LOAD("mpr-22215.ic8", 0x4000000, 0x0800000, CRC(a80714fa) SHA1(b32dde5cc79a9ae9f7f34064c2382115e9303070) )
	ROM_LOAD("mpr-22216.ic9", 0x4800000, 0x0800000, CRC(cf3d1049) SHA1(a390304256dfac623b6fe1b205d918ce3eb67723) )
	ROM_LOAD("mpr-22217.ic10",0x5000000, 0x0800000, CRC(48c92fd6) SHA1(26b17a8d0130512807cf533a60c10c6d1e769de0) )
	ROM_LOAD("mpr-22218.ic11",0x5800000, 0x0800000, CRC(f9ca31b8) SHA1(ea3d0f38ca1a46c896c06f038a6362ad3c9f90b2) )
	ROM_LOAD("mpr-22219.ic12",0x6000000, 0x0800000, CRC(b3b45811) SHA1(045e7236b814f848d4c9767618ddcd4344d880ec) )
ROM_END


/*

SYSTEMID: NAOMI
JAP:  POWER SMASH --------------
USA:  VIRTUA TENNIS IN USA -----
EXP:  VIRTUA TENNIS IN EXPORT --

NO. Type    Byte    Word
IC22    32M 0000    1111
IC1 64M 7422    83DD
IC2 64M 7F26    A93D
IC3 64M 8E02    D3FC
IC4 64M 2545    F734
IC5 64M E197    B75D
IC6 64M 9453    CF75
IC7 64M 29AC    2FEB
IC8 64M 0434    2E9E
IC9 64M C86E    79E6
IC10    64M C67A    BF14
IC11    64M F590    D280

*/

ROM_START( vtennis )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22927.22", 0x0000000, 0x0400000,  CRC(89781723) SHA1(cf644aa66abcec6964d77485a0292f11ba80dd0d) )

	ROM_REGION( 0x5800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: ZOMBIE REVENGE IN JAPAN
USA: ZOMBIE REVENGE IN USA
EXP: ZOMBIE REVENGE IN EXPORT

NO. Type    Byte    Word
IC22    16M 0000    0000
IC1 64M 899B    97E1
IC2 64M 6F0B    2D2D
IC3 64M 4328    C898
IC4 64M 0205    57C5
IC5 64M 93A7    A717
IC6 64M 936B    A35B
IC7 64M 2F51    2BFC
IC8 64M D627    54C5
IC9 64M D2F9    B84C
IC10    64M 9B5A    B70B
IC11    64M 3F0F    9AEB
IC12    64M 5778    EBCA
IC13    64M 75DB    8563
IC14    64M 427A    577C
IC15    64M A7B7    D0D6
IC16    64M 9F01    FCFE
IC17    64M DFB4    58F7
IC18    64M C453    B313
IC19    64M 04B8    49FB

*/

ROM_START( zombrvn )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr21707.22", 0x0000000, 0x0200000,  CRC(4daa11e9) SHA1(2dc219a5e0d0b41cce6d07631baff0495c479e13) )

	ROM_REGION( 0x9800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DEAD OR ALIVE 2
USA: DEAD OR ALIVE 2 USA ------
EXP: DEAD OR ALIVE 2 EXPORT----

NO.     Type    Byte    Word
IC22    32M     2B49    A054
IC1     64M     B74A    1815
IC2     64M     6B34    AB5A
IC3     64M     7EEF    EA1F
IC4     64M     0700    8C2F
IC5     64M     E365    B9CC
IC6     64M     7FE0    DC66
IC7     64M     BF8D    439B
IC8     64M     84DC    2F86
IC9     64M     15CF    8961
IC10    64M     7776    B985
IC11    64M     BCE9    21E9
IC12    64M     87FA    E9C0
IC13    64M     B82E    47A7
IC14    64M     3821    846E
IC15    64M     B491    C66E
IC16    64M     5774    918D
IC17    64M     219B    A171
IC18    64M     4848    643A
IC19    64M     6E1F    2570
IC20    64M     0CED    F2A8
IC21    64M     002C    8ECA

*/

ROM_START( doa2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr22121.22", 0x0000000, 0x0400000,  CRC(30f93b5e) SHA1(0e33383e7ab9a721dab4708b063598f2e9c9f2e7) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DEAD OR ALIVE 2
USA: DEAD OR ALIVE 2 USA ------
EXP: DEAD OR ALIVE 2 EXPORT----

NO.     Type    Byte    Word
IC22    32M     2B49    A054
IC1     64M     B74A    1815
IC2     64M     6B34    AB5A
IC3     64M     7EEF    EA1F
IC4     64M     0700    8C2F
IC5     64M     E365    B9CC
IC6     64M     7FE0    DC66
IC7     64M     BF8D    439B
IC8     64M     84DC    2F86
IC9     64M     15CF    8961
IC10    64M     7776    B985
IC11    64M     BCE9    21E9
IC12    64M     87FA    E9C0
IC13    64M     B82E    47A7
IC14    64M     3821    846E
IC15    64M     B491    C66E
IC16    64M     5774    918D
IC17    64M     219B    A171
IC18    64M     4848    643A
IC19    64M     6E1F    2570
IC20    64M     0CED    F2A8
IC21    64M     002C    8ECA

Serial: BALH-13A0175

*/

ROM_START( doa2m )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("doa2verm.22", 0x0000000, 0x0400000,  CRC(94b16f08) SHA1(225cd3e5dd5f21facf0a1d5e66fa17db8497573d) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DYNAMITE BASEBALL NAOMI
USA: SAMPLE GAME IN USA--------
EXP: SAMPLE GAME

NO.     Type    Byte    Word
IC22    16M     EF41    1DBC
IC1     64M     2743    8DE9
IC2     64M     1D2B    B4D5
IC3     64M     9127    8536
IC4     64M     946A    851B
IC5     64M     BDF4    AF2C
IC6     64M     78A2    DADB
IC7     64M     9816    06D3
IC8     64M     F8D9    9C38
IC9     64M     3C7D    532A
IC10    64M     37A2    D3F1
IC11    64M     5BF2    05FC
IC12    64M     694F    A25A
IC13    64M     685C    CDA8
IC14    64M     3DFA    32A9
IC15    64M     071F    820F
IC16    64M     1E89    D6B5
IC17    64M     889C    504B
IC18    64M     8B78    1BB5
IC19    64M     9816    7EE9
IC20    64M     E5C2    CECB
IC21    64M     5C65    8F82

Serial: ??? (sticker removed)

*/

ROM_START( dybbnao )
	ROM_REGION( 0x200000, REGION_CPU1, 0)
	NAOMI_BIOS

	ROM_REGION( 0x400000, REGION_USER1, 0)
	ROM_LOAD("epr21575.22", 0x0000000, 0x0200000, CRC(ba61e248) SHA1(3cce5d8b307038515d7da7ec567bfa2e3aafc274) )

	ROM_REGION( 0xa800000, REGION_USER2, 0)
	ROM_LOAD("ic1", 0x0000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic2", 0x0800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic3", 0x1000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic4", 0x1800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic5", 0x2000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic6", 0x2800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic7", 0x3000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic8", 0x3800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic9", 0x4000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic10",0x4800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic11",0x5000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic12",0x5800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic13",0x6000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic14",0x6800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic15",0x7000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic16",0x7800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic17",0x8000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic18",0x8800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic19",0x9000000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic20",0x9800000, 0x0800000, NO_DUMP )
	ROM_LOAD("ic21",0xa000000, 0x0800000, NO_DUMP )
ROM_END


/* All games have the regional titles at the start of the IC22 rom in the following order

  JAPAN
  USA
  EXPORT (EURO in some titles)
  KOREA (ASIA in some titles)
  AUSTRALIA
  UNUSED
  UNUSED
  UNUSED

  with the lists below it has been assumed that if the title is listed for a region
  then it is available / works in that region, this has not been confirmed as correct.

*/

/* Naomi & Naomi GD-ROM */
GAME( 1998, naomi,    0,        naomi,    naomi,    0, ROT0, "Sega",            "Naomi Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
// Complete Dumps
GAME( 2001, csmash,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Cosmic Smash (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, suchie3,  naomi,    naomi,    naomi,    0, ROT0, "Jaleco",          "Idol Janshi Suchie-Pai 3 (JPN)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, vs2_2k,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Virtua Striker 2 Ver. 2000 (JPN, USA, EXP, KOR, AUS) (set 1)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, vs2_2ka,  vs2_2k,   naomi,    naomi,    0, ROT0, "Sega",            "Virtua Striker 2 Ver. 2000 (JPN, USA, EXP, KOR, AUS) (set 2)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, smarinef, naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Sega Marine Fishing", GAME_NO_SOUND|GAME_NOT_WORKING )
// Incomplete Dumps (just IC22)
GAME( 2000, cspike,   naomi,    naomi,    naomi,    0, ROT0, "Psikyo / Capcom", "Gun Spike (JPN) / Cannon Spike (USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, capsnk,   naomi,    naomi,    naomi,    0, ROT0, "Capcom / SNK",    "Capcom Vs. SNK Millennium Fight 2000 (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( ????, derbyoc,  naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Derby Owners Club (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1998, dybb99,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Dynamite Baseball '99 (JPN) / World Series '99 (USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, gram2000, naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Giant Gram 2000 (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, ggram2,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Giant Gram (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING ) // strings in rom don't contain '2'
GAME( 2001, hmgeo,    naomi,    naomi,    naomi,    0, ROT0, "Capcom",          "Heavy Metal Geomatrix (JPN, USA, EUR, ASI, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, gwing2,   naomi,    naomi,    naomi,    0, ROT0, "Takumi / Capcom", "Giga Wing 2 (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, pjustic,  naomi,    naomi,    naomi,    0, ROT0, "Capcom",          "Moero Justice Gakuen (JPN) / Project Justice (USA, EXP, KOR, AUS) ", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, pstone,   naomi,    naomi,    naomi,    0, ROT0, "Capcom",          "Power Stone (JPN, USA, EUR, ASI, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, pstone2,  naomi,    naomi,    naomi,    0, ROT0, "Capcom",          "Power Stone 2 (JPN, USA, EUR, ASI, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, otrigger, naomi,    naomi,    naomi,    0, ROT0, "Sega",            "OutTrigger (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, samba,    naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Samba De Amigo (JPN)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, slasho,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Slashout (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, spawn,    naomi,    naomi,    naomi,    0, ROT0, "Capcom",          "Spawn (JPN, USA, EUR, ASI, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, virnba,   naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Virtua NBA (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, vtennis,  naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Power Smash (JPN) / Virtua Tennis (USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, zombrvn,  naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Zombie Revenge (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1999, doa2,     naomi,    naomi,    naomi,    0, ROT0, "Tecmo",           "Dead or Alive 2 (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2000, doa2m,    doa2,     naomi,    naomi,    0, ROT0, "Tecmo",           "Dead or Alive 2 Millennium (JPN, USA, EXP, KOR, AUS)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1998, dybbnao,  naomi,    naomi,    naomi,    0, ROT0, "Sega",            "Dynamite Baseball NAOMI (JPN)", GAME_NO_SOUND|GAME_NOT_WORKING )



// No GD-Rom Sets Supported */

/* Naomi 2 & Naomi 2 GD-ROM */
GAME( 2001, naomi2,   0,        naomi,    naomi,    0, ROT0, "Sega",            "Naomi 2 Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
// No Supported Sets

/* Atomiswave */
GAME( 2001, awbios,   0,        naomi,    naomi,    0, ROT0, "Sammy",           "Atomiswave Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
// No Supported Sets
