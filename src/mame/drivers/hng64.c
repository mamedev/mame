/* Hyper NeoGeo 64

Driver by David Haywood, ElSemi, and Andrew Gardner.
Rasterizing code provided in part by Andrew Zaferakis.

Notes:
  * The top board is likely identical for all revisions and all "versions" of the hardware.
    It contains the main MIPS CPU and a secondary communications KL5C80.
  * The bottom board is what changes between hardware "versions".  It has a Toshiba MCU with
    a protected internal ROM.  This MCU controls (at least) the inputs per game and communicates
    with the main board through dualport RAM.
  * I believe that this secondary board is used as a protection device.
    The "board type" code comes from it in dualport RAM, and each game reads its inputs differently through dualport.
    It's capable of changing the input ports dynamically (maybe explaining Roads Edge's "do not touch" quote below).
    It probably has a lot to do with the network (Roads Edge network connectors are on this board).
    And since it can return any value at any time, it may be responsible for fatfurwa's missing palette.
  * The Toshiba CPU datasheet is here : http://www.semicon.toshiba.co.jp/openb2b/websearch/productDetails.jsp?partKey=TMP87CH40N
  * The internal ROM needs dumping.  Rumor has it, it's the same MCU as in Naomi hardware...

  * From the Roads Edge manual : "The Network Check screen will be displayed for about 40 seconds whether
                                  the cabinet is connected for communication competition or not.  After this,
                                  the game screen will then appear.  At the same time the Network Check screen
                                  is displayed, the steering wheel will automatically straighten itself out."
                                 "During the Network Check, absolutely do not touch or try to use the steering wheel,
                                  pedal, shift lever, and switches.  This will cause the cabinet to malfunction."

  * The Japanese text on the Roads Edge network screen says : "waiting to connect network... please wait without touching machine"

ToDo:
  * clean up I/O / MCU communication
  * work out the purpose of the interrupts and how many are needed
  * correct game speed (seems too fast)
  * figure out what 'network' Road Edge needs to boot, it should run as a standalone
  * make ss64 boot (io return 3 then 4 not just 4) then work out where the palette is

  2d:
  * scroll (base registers?)
  * roz (4th tilemap in fatal fury should be floor [in progress], background should zoom)
  * find registers to control tilemap mode (4bpp/8bpp, 8x8, 16x16)
  * fix zooming sprites (zoom registers not understood, center versus edge pivot)
  * priorities
  * still some bad sprites/tiles (waterfall level 'splash' - health bar at 50% - intro top and bottom - fatfurwa test mode)
  * is all the bitmap decoding right?
  * upgrade to modern video timing.

  3d:
  * Find where the remainder of the 3d display list information is 'hiding' (FIFO?)
  * Remaining 3d bits - glowing, etc.
  * Populate the display buffers
  * Does the hng64 do perspective-correct texture mapping?  Doesn't look like it...

  Other:
  * Translate KL5C80 docs and finish up the implementation
  * figure out what IO $54 & $72 are on the communications CPU
  * hook up CPU2 (v30 based?) no rom? (maybe its the 'sound driver' the game uploads?)
  * add sound
  * backup ram etc.
  * correct cpu speed and find idle skips
  * What is ROM1?  Data for the KL5C80?  There's plenty of physical space to map it to.
*/

/*
Known games on this system

Beast Busters 2nd Nightmare ( http://emustatus.rainemu.com/games/bbustr2nd.htm )
Buriki One ( http://emustatus.rainemu.com/games/buriki1.htm )
Fatal Fury: Wild Ambition ( http://emustatus.rainemu.com/games/ffurywa.htm )
Roads Edge / Round Trip? ( http://emustatus.rainemu.com/games/redge.htm )
Samurai Shodown 64 ( http://emustatus.rainemu.com/games/sams64.htm )
Samurai Shodown: Warrior's Rage ( http://emustatus.rainemu.com/games/samswr.htm )
Xtreme Rally / Offbeat Racer ( http://emustatus.rainemu.com/games/xrally.htm )
*/

/*
NeoGeo Hyper 64 (Main Board)
SNK, 1997

This is a 3D system comprising one large PCB with many custom smt components
on both sides, one interface PCB with JAMMA connector and sound circuitry, and
one game cartridge. Only the Main PCB and interface PCB are detailed here.

PCB Layout (Top)
----------------

LVS-MAC SNK 1997.06.02
|--------------------------------------------------------------|
|              CONN9                                           |
|                                                              |
|   ASIC1           ASIC3            CPU1                      |
|                                                              |
|                                               DPRAM1         |
|                        OSC2        ASIC5                 ROM1|
|   FSRAM1               OSC2                                  |
|   FSRAM2                                             FPGA1   |
|   FSRAM3                           ASIC10       OSC4         |
|                                                              |
|                                                 CPU3  IC4    |
|   PSRAM1  ASIC7   ASIC8            DSP1 OSC3    SRAM5        |
|   PSRAM2                                              FROM1  |
|                                                              |
|                                                              |
|              CONN10                                          |
|--------------------------------------------------------------|

No.  PCB Label  IC Markings               IC Package
----------------------------------------------------
01   ASIC1      NEO64-REN                 QFP304
02   ASIC3      NEO64-GTE                 QFP208
03   ASIC5      NEO64-SYS                 QFP208
04   ASIC7      NEO64-BGC                 QFP240
05   ASIC8      NEO64-SPR                 QFP208
06   ASIC10     NEO64-SCC                 QFP208
07   CPU1       NEC D30200GD-100 VR4300   QFP120
08   CPU3       KL5C80A12CFP              QFP80
09   DPRAM1     DT7133 LA35J              PLCC68
10   DSP1       L7A1045 L6028 DSP-A       QFP120
11   FPGA1      ALTERA EPF10K10QC208-4    QFP208
12   FROM1      MBM29F400B-12             TSOP48 (archived as FROM1.BIN)
13   FSRAM1     TC55V1664AJ-15            SOJ44
14   FSRAM2     TC55V1664AJ-15            SOJ44
15   FSRAM3     TC55V1664AJ-15            SOJ44
16   IC4        SMC COM20020-5ILJ         PLCC28
17   OSC1       M33.333 KDS 7M            -
18   OSC2       M50.113 KDS 7L            -
19   OSC3       A33.868 KDS 7M            -
20   OSC4       A40.000 KDS 7L            -
21   PSRAM1     TC551001BFL-70L           SOP32
22   PSRAM2     TC551001BFL-70L           SOP32
23   ROM1       ALTERA EPC1PC8            DIP8   (130817 bytes, archived as ROM1.BIN)
24   SRAM5      TC55257DFL-85L            SOP28


PCB Layout (Bottom)

|--------------------------------------------------------------|
|             CONN10                                           |
|                                                              |
|                                                              |
|   PSRAM4  ASIC9                   SRAM4     CPU1  Y1         |
|   PSRAM3         SRAM1            SRAM3                      |
|                  SRAM2                                       |
|                                                              |
|   FSRAM6                        DRAM3                        |
|   FSRAM5                                                     |
|   FSRAM4                        DRAM1                        |
|                   BROM1         DRAM2                        |
|                                                              |
|                                                              |
|   ASIC2           ASIC4         ASIC6                        |
|                                                              |
|             CONN9                                            |
|--------------------------------------------------------------|

No.  PCB Label  IC Markings               IC Package
----------------------------------------------------
01   ASIC2      NEO64-REN                 QFP304
02   ASIC4      NEO64-TRI2                QFP208
03   ASIC6      NEO64-CVR                 QFP120
04   ASIC9      NEO64-CAL                 QFP208
05   BROM1      MBM29F400B-12             TSOP48  (archived as BROM1.BIN)
06   CPU2       NEC D70326AGJ-16 V53A     QFP120
07   DRAM1      HY51V18164BJC-60          SOJ42
08   DRAM2      HY51V18164BJC-60          SOJ42
09   DRAM3      HY51V18164BJC-60          SOJ42
10   FSRAM4     TC55V1664AJ-15            SOJ44
11   FSRAM5     TC55V1664AJ-15            SOJ44
12   FSRAM6     TC55V1664AJ-15            SOJ44
13   PSRAM3     TC551001BFL-70L           SOP32
14   PSRAM4     TC551001BFL-70L           SOP32
15   SRAM1      TC55257DFL-85L            SOP28
16   SRAM2      TC55257DFL-85L            SOP28
17   SRAM3      TC551001BFL-70L           SOP32
18   SRAM4      TC551001BFL-70L           SOP32
19   Y1         D320L7                    XTAL


INTERFACE PCB
-------------

LVS-JAM SNK 1999.1.20
|---------------------------------------------|
|                 J A M M A                   |
|                                             |
|                                             |
|                                             |
|     SW3             SW1                     |
|                                             |
| IC6                       IOCTR1            |
|                           BACKUP            |
|                           BKRAM1            |
|     SW2   BT1  DPRAM1              IC1      |
|---------------------------------------------|

No.  PCB Label  IC Markings               IC Package
----------------------------------------------------
01   DPRAM1     DT 71321 LA55PF           QFP64
02   IC1        MC44200FT                 QFP44
03   IOCTR1     TOSHIBA TMP87CH40N-4828   SDIP64
04   BACKUP     EPSON RTC62423            SOP24
05   BKRAM1     W24258S-70LE              SOP28
06   IC6        NEC C1891ACY              DIP20
07   BT1        3V Coin Battery
08   SW1        2 position DIPSW  OFF = JAMMA       ON = MVS
09   SW2        4 position DIPSW
10   SW3        2 position DIPSW  OFF = MONO/JAMMA  ON = 2CH MVS

Notes:
       1. The game cart plugs into the main PCB on the TOP side into CONN9 & CONN10
       2. If the game cart is not plugged in, the hardware shows nothing on screen.




Hyper Neo Geo game cartridges
-----------------------------

The game carts contains nothing except a huge pile of surface mounted ROMs
on both sides of the PCB. On a DG1 cart all the roms are 32Mbits, for the
DG2 cart the SC and SP roms are 64Mbit.
The DG1 cart can accept a maximum of 96 ROMs
The DG2 cart can accept a maximum of 84 ROMs


The actual carts are mostly only about 1/3rd to 1/2 populated.
Some of the IC locations between DG1 and DG2 are different also. See the source code below
for the exact number of ROMs used per game and ROM placements.

Games that use the LVS-DG1 cart: Road's Edge

Games that use the LVS-DG2 cart: Fatal Fury: Wild Ambition, Buriki One, SS 64 II

Other games not dumped yet     : Samurai Spirits 64 / Samurai Shodown 64
                                 Samurai Spirits II: Asura Zanmaden / Samurai Shodown: Warrior's Rage
                                 Off Beat Racer / Xtreme Rally
                                 Beast Busters: Second Nightmare
                                 Garou Densetsu 64: Wild Ambition (=Fatal Fury Wild Ambition)*
                                 Round Trip RV (=Road's Edge)*

                               * Different regions might use the same roms, not known yet

                               There might be Rev.A boards for Buriki and Round Trip

pr = program
sc = scroll characters?
sd = sound
tx = textures
sp = sprites?
vt = vertex?

Top
---
LVS-DG1
(C) SNK 1997
1997.5.20
|----------------------------------------------------------------------------|
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.5    TX01A.13        VT03A.19   VT02A.18   VT01A.17   PR01B.81       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.6    TX02A.14        VT06A.22   VT05A.21   VT04A.20   PR03A.83       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.7    TX03A.15        VT09A.25   VT08A.24   VT07A.23   PR05A.85       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.8    TX04A.16        VT12A.28   VT11A.27   VT10A.26   PR07A.87       |
|                                                                            |
|                                                                            |
|                                                                            |
|                            PR15A.95   PR13A.93   PR11A.91   PR09A.89       |
|                                                                            |
|                                                                            |
|                                                                            |
| SC09A.49  SC10A.50   SP09A.61   SP10A.62   SP11A.63   SP12A.64    SD02A.78 |
|                                                                            |
|                                                                            |
|                                                                            |
| SC05A.45  SC06A.46   SP05A.57   SP06A.58   SP07A.59   SP08A.60             |
|                                                                            |
|                                                                            |
|                                                                            |
| SC01A.41  SC02A.42   SP01A.53   SP02A.54   SP03A.55   SP04A.56    SD01A.77 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|

Bottom
------

LVS-DG1
|----------------------------------------------------------------------------|
|                         |----------------------|                           |
|                         |----------------------|                           |
|                                                                            |
|                                                                            |
| SC03A.43  SC04A.44   SP13A.65   SP14A.66   SP15A.67   SP16A.68    SD03A.79 |
|                                                                            |
|                                                                            |
|                                                                            |
| SC07A.47  SC08A.48   SP17A.69   SP18A.70   SP19A.71   SP20A.72             |
|                                                                            |
|                                                                            |
|                                                                            |
| SC11A.51  SC12A.52   SP21A.73   SP22A.74   SP23A.75   SP24A.76    SD04A.80 |
|                                                                            |
|                                                                            |
|                                                                            |
|                            PR16A.96   PR14A.94   PR12A.92   PR10A.90       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.4    TX04A.12        VT24A.40   VT23A.39   VT22A.38   PR08A.88       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.3    TX03A.11        VT21A.37   VT20A.36   VT19A.35   PR06A.86       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.2    TX02A.10        VT18A.34   VT17A.33   VT16A.32   PR04A.84       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.1    TX01A.9         VT15A.31   VT14A.30   VT13A.29   PR02B.82       |
|                                                                            |
|                                                                            |
|                         |----------------------|                           |
|                         |----------------------|                           |
|----------------------------------------------------------------------------|


Top
---
LVS-DG2
(C) SNK 1998
1998.6.5
|----------------------------------------------------------------------------|
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.5    TX01A.13        VT03A.19   VT02A.18   VT01A.17   PR01B.81       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.6    TX02A.14        VT06A.22   VT05A.21   VT04A.20   PR03A.83       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.7    TX03A.15        VT09A.25   VT08A.24   VT07A.23   PR05A.85       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.8    TX04A.16        VT12A.28   VT11A.27   VT10A.26   PR07A.87       |
|                                                                            |
|                                                                            |
|                                                                            |
|                            PR15A.95   PR13A.93   PR11A.91   PR09A.89       |
|                                                                            |
|                                                                            |
|                                                                            |
| SC05A.98  SC06A.100  SP09A.107  SP10A.111  SP11A.115  SP12A.119   SD02A.78 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| SC01A.97  SC02A.99   SP01A.105  SP02A.109  SP03A.113  SP04A.117   SD01A.77 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|

Bottom
------
LVS-DG2
|----------------------------------------------------------------------------|
|                         |----------------------|                           |
|                         |----------------------|                           |
|                                                                            |
|                                                                            |
| SC03A.101 SC04A.103  SP13A.108  SP14A.112  SP15A.116  SP16A.120   SD03A.79 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| SC07A.102  SC08A.104  SP05A.106  SP06A.110  SP07A.114  SP08A.118  SD04A.80 |
|                                                                            |
|                                                                            |
|                                                                            |
|                            PR16A.96   PR14A.94   PR12A.92   PR10A.90       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.4    TX04A.12        VT24A.40   VT23A.39   VT22A.38   PR08A.88       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.3    TX03A.11        VT21A.37   VT20A.36   VT19A.35   PR06A.86       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.2    TX02A.10        VT18A.34   VT17A.33   VT16A.32   PR04A.84       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.1    TX01A.9         VT15A.31   VT14A.30   VT13A.29   PR02B.82       |
|                                                                            |
|                                                                            |
|                         |----------------------|                           |
|                         |----------------------|                           |
|----------------------------------------------------------------------------|
 Notes:
      Not all ROM positions are populated, check the source for exact ROM usage.
      ROMs are mirrored. i.e. TX/PR/SP/SC etc ROMs line up on both sides of the PCB.
      There are 4 copies of each TX ROM on the PCB.


----

info from Daemon

(MACHINE CODE ERROR):
Is given when you try to put a "RACING GAME" on a "FIGHTING" board.

There are various types of neogeo64 boards:
FIGHTING (revision 1 and 2)
RACING
SHOOTING
and
SAMURAI SHODOWN ONLY (Korean)

FIGHTING boards will ONLY play fighting games.

RACING boards will ONLY play racing games (and you need the extra gimmicks
to connect analog wheel and pedals, otherwise it gives you yet another
error).

Shooter boards will only work with Beast Busters 2.

And the Korean board only plays Samurai Shodown games (wont play Buriki One
or Fatal Fury for example).
*/

#define MASTER_CLOCK	50000000
#include "driver.h"
#include "deprecat.h"
#include "cpu/mips/mips3.h"

static UINT32 *rombase;
static UINT32 *hng_mainram;
static UINT32 *hng_cart;
static UINT32 *hng64_dualport;
static UINT32 *hng64_sram;
static UINT16 *hng64_soundram;

// Stuff from over in video...
extern tilemap *hng64_tilemap0, *hng64_tilemap1, *hng64_tilemap2, *hng64_tilemap3 ;
extern UINT32 *hng64_spriteram, *hng64_videoregs ;
extern UINT32 *hng64_videoram ;
extern UINT32 *hng64_tcram ;

extern UINT32 hng64_dls[2][0x81] ;

VIDEO_START( hng64 ) ;
VIDEO_UPDATE( hng64 ) ;

static UINT32 activeBuffer ;


static UINT32 no_machine_error_code;
static int hng64_interrupt_level_request;
static WRITE32_HANDLER( hng64_videoram_w );

/* 3D stuff */
static UINT32 *hng64_3d_1;
static UINT32 *hng64_3d_2;
static UINT32 *hng64_dl;
//static UINT32 *hng64_q2;

/* Communications stuff */
static UINT32 *hng64_com_ram ;

static UINT8 *hng64_com_virtual_mem;
static UINT8 *hng64_com_op_base;

static UINT8 *hng64_com_mmu_mem;

/* Hacky stuff */
//static char writeString[1024];
extern UINT32 hng64_hackTilemap3, hng64_hackTm3Count, hng64_rowScrollOffset;


#ifdef UNUSED_FUNCTION
WRITE32_HANDLER( trap_write )
{
    logerror("Remapped write... %08x %08x\n",offset,data);
}
#endif


static WRITE32_HANDLER( hng64_videoram_w )
{
	int realoff;
	COMBINE_DATA(&hng64_videoram[offset]);

	realoff = offset*4;

	if ((realoff>=0) && (realoff<0x10000))
	{
		tilemap_mark_tile_dirty(hng64_tilemap0,offset&0x3fff);
	}
	else if ((realoff>=0x10000) && (realoff<0x20000))
	{
		tilemap_mark_tile_dirty(hng64_tilemap1,offset&0x3fff);
	}
	else if ((realoff>=0x20000) && (realoff<0x30000))
	{
		tilemap_mark_tile_dirty(hng64_tilemap2,offset&0x3fff);
	}
	else if ((realoff>=0x30000) && (realoff<0x40000))
	{
		tilemap_mark_tile_dirty(hng64_tilemap3,offset&0x3fff);
	}

//  if ((realoff>=0x40000)) mame_printf_debug("offsw %08x %08x\n",realoff,data);


	///////////////////////////////////
	// For the scrolling ground, yo  //
	///////////////////////////////////

	// First, get the offset we're working with
	if ( (realoff&0x00000bf0) == 0xbf0)
	{
		hng64_hackTilemap3 = 1 ;
		hng64_rowScrollOffset = realoff & 0x000ff000 ;
	}

	// Next count the number of lines to be drawn to the screen.
	// This is probably really done per-scanline or something.
	if (hng64_rowScrollOffset)
	{
		if ((realoff & hng64_rowScrollOffset) == hng64_rowScrollOffset)
			hng64_hackTm3Count++ ;
	}

	/* 400000 - 7fffff is scroll regs etc. */
}


static READ32_HANDLER( hng64_random_read )
{
	return mame_rand(machine)&0xffffffff;
}


static READ32_HANDLER( hng64_com_r )
{
	logerror("com read  (PC=%08x): %08x %08x = %08x\n", activecpu_get_pc(), (offset*4)+0xc0000000, mem_mask, hng64_com_ram[offset]);
	return hng64_com_ram[offset] ;
}


static WRITE32_HANDLER( hng64_com_w )
{
	logerror("com write (PC=%08x): %08x %08x = %08x\n", activecpu_get_pc(), (offset*4)+0xc0000000, mem_mask, data);
	COMBINE_DATA(&hng64_com_ram[offset]);
}


static UINT32 hng64_com_shared_a;
static UINT32 hng64_com_shared_b;

static WRITE32_HANDLER( hng64_com_share_w )
{
	logerror("commw  (PC=%08x): %08x %08x %08x\n", activecpu_get_pc(), data, (offset*4)+0xc0001000, mem_mask);

	if (offset==0x0) COMBINE_DATA(&hng64_com_shared_a);
	if (offset==0x1) COMBINE_DATA(&hng64_com_shared_b);
}

static READ32_HANDLER( hng64_com_share_r )
{
	logerror("commr  (PC=%08x): %08x %08x\n", activecpu_get_pc(), (offset*4)+0xc0001000, mem_mask);

//  if(offset==0x0) return hng64_com_shared_a;
//  if(offset==0x1) return hng64_com_shared_b;

	if(offset==0x0) return 0x0000aaaa;
	if(offset==0x1)	return 0x00030000;		// fatfurwa : at bfc06624 it wants a 01 : at bfc06650 it wants a 02

	return 0x00;
}



static WRITE32_HANDLER( hng64_pal_w )
{
	int r,g,b,a;
	COMBINE_DATA(&paletteram32[offset]);

	b = ((paletteram32[offset] & 0x000000ff) >>0);
	g = ((paletteram32[offset] & 0x0000ff00) >>8);
	r = ((paletteram32[offset] & 0x00ff0000) >>16);
	a = ((paletteram32[offset] & 0xff000000) >>24);

	// a sure ain't alpha.
	// alpha_set_level(mame_rand(machine)) ;
	// mame_printf_debug("Alpha : %d %d %d %d\n", a, b, g, r) ;

	//if (a != 0)
	//  popmessage("Alpha is not zero!") ;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

static READ32_HANDLER( hng64_port_read )
{
	logerror("HNG64 port read (PC=%08x) 0x%08x\n", activecpu_get_pc(),offset*4);

	if(offset==0x421) return 0x00000002;
 	if(offset==0x441) return hng64_interrupt_level_request;

	if(offset==0x85b) return 0x00000010;

	return mame_rand(machine)&0xffffffff;
}


/* preliminary dma code, dma is used to copy program code -> ram */
static int hng_dma_start,hng_dma_dst,hng_dma_len;
static void hng64_do_dma (void)
{
	logerror("Performing DMA Start %08x Len %08x Dst %08x\n",hng_dma_start, hng_dma_len, hng_dma_dst);

	while (hng_dma_len>0)
	{
		UINT32 dat;

		dat = program_read_dword(hng_dma_start);
		program_write_dword(hng_dma_dst,dat);
		hng_dma_start+=4;
		hng_dma_dst+=4;
		hng_dma_len--;
	}

	hng_dma_start+=4;
	hng_dma_dst+=4;
}


static WRITE32_HANDLER( hng_dma_start_w )
{
	logerror ("DMA Start Write %08x\n",data);
	hng_dma_start = data;
}

static WRITE32_HANDLER( hng_dma_dst_w )
{
	logerror ("DMA Dst Write %08x\n",data);
	hng_dma_dst = data;
}

static WRITE32_HANDLER( hng_dma_len_w )
{
	logerror ("DMA Len Write %08x\n",data);
	hng_dma_len = data;
	hng64_do_dma();

}

#ifdef UNUSED_FUNCTION
READ32_HANDLER( hng64_videoram_r )
{
	return hng64_videoram[offset];
}

WRITE32_HANDLER( hng64_mainram_w )
{
	COMBINE_DATA (&hng_mainram[offset]);
}

READ32_HANDLER( hng64_cart_r )
{
	return hng_cart[offset];
}
#endif

static READ32_HANDLER( hng64_sram_r )
{
	logerror("HNG64 reading from SRAM 0x%08x == 0x%08x. (PC=%08x)\n", offset*4, hng64_sram[offset], activecpu_get_pc());
	return hng64_sram[offset];
}

static WRITE32_HANDLER( hng64_sram_w )
{
	logerror("HNG64 writing to SRAM 0x%08x == 0x%08x & 0x%08x. (PC=%08x)\n", offset*4, data, mem_mask, activecpu_get_pc());
	COMBINE_DATA (&hng64_sram[offset]);
}

static WRITE32_HANDLER( hng64_dualport_w )
{
	logerror("dualport WRITE %08x %08x (PC=%08x)\n", offset*4, hng64_dualport[offset], activecpu_get_pc());
	COMBINE_DATA (&hng64_dualport[offset]);
}

static READ32_HANDLER( hng64_dualport_r )
{
	logerror("dualport R %08x %08x (PC=%08x)\n", offset*4, hng64_dualport[offset], activecpu_get_pc());

	// These hacks create some red marks for the boot-up sequence
	switch (offset*4)
	{
		//SamSho64
//      case 0x00:  toggi^=1; if (toggi==1) {return 0x00000400;} else {return 0x00000300;};
		//RoadsEdge
//      case 0x00:  return input_port_read(machine, "IPT_TEST");

		//Fatfurwa
		case 0x00:  return 0x00000400;
		case 0x04:  return input_port_read(machine, "IPT_NONE")       | (input_port_read(machine, "FATFURWA_TST_ETC")<<16);
		case 0x08:  return input_port_read(machine, "FATFURWA_PLR_2") | (input_port_read(machine, "FATFURWA_PLR_1")<<16);

		// This takes care of the 'machine' error code
		case 0x600: return no_machine_error_code;
	}

	return mame_rand(machine)&0xffffffff;
	//return hng64_dualport[offset];
}

// Hardware calls these '3d buffers'
//   They're only read during the startup check of fatfurwa.  Z-buffer memory?  Front buffer, back buffer?
//   They're definitely mirrored in the startup test.  Elsemi says:
//   <ElSemi> 30100000-3011ffff is framebuffer A0
//   <ElSemi> 30120000-3013ffff is framebuffer A1
//   <ElSemi> 30140000-3015ffff is ZBuffer A
static READ32_HANDLER( hng64_3d_1_r )
{
	return hng64_3d_1[offset] ;
}

#ifdef UNUSED_FUNCTION
WRITE32_HANDLER( hng64_3d_1_w )
{
	fatalerror("WRITE32_HANDLER( hng64_3d_1_w )");
}
#endif

static READ32_HANDLER( hng64_3d_2_r )
{
	return hng64_3d_2[offset] ;
}

static WRITE32_HANDLER( hng64_3d_2_w )
{
	COMBINE_DATA (&hng64_3d_1[offset]) ;
	COMBINE_DATA (&hng64_3d_2[offset]) ;
}



// The 3d 'display list'
static WRITE32_HANDLER( dl_w )
{
	COMBINE_DATA (&hng64_dl[offset]) ;

	// !!! There are a few other writes over 0x80 as well - don't forget about those someday !!!

	// This method of finding the 85 writes and switching banks seems to work
	//  the problem is when there are a lot of things to be drawn (more than what can fit in 2*0x80)
	//  the list doesn't fit anymore, and parts aren't drawn every frame.

	if (offset == 0x85)
	{
		// Only if it's VALID
		if ((INT32)hng64_dl[offset] == 1 || (INT32)hng64_dl[offset] == 2)
			activeBuffer = (INT32)hng64_dl[offset] - 1 ;		// Subtract 1 to fit into my array...
	}

	if (offset <= 0x80)
		hng64_dls[activeBuffer][offset] = hng64_dl[offset] ;

	// For some reason, if the value at 0x86 and'ed with 0x02 is non-zero, the software just keeps
	//   checking 0x86 until and'ing it with 2 returns 0...  What could change 0x86 if cpu0 is in this infinite loop?
	//   Why 0x2?  And why (see below) does a read of 0x86 only happen when there is data 'hiding'
	//   in the display list?

	// Sends the program into an infinite loop...
//  if (offset == 0x85 && hng64_dl[offset] == 0x1)      // Just before the second half of the writes
//      hng64_dl[0x86] = 0x2 ;                          // set 0x86 to 2 (so it drops into the loop)

//  mame_printf_debug("dl W (%08x) : %.8x %.8x\n", activecpu_get_pc(), offset, hng64_dl[offset]) ;
}

static READ32_HANDLER( dl_r )
{
	// A read of 0x86 ONLY happens if there are more display lists than what are readily available.
	// (PC = 8006fe1c)

//  mame_printf_debug("dl R (%08x) : %x %x\n", activecpu_get_pc(), offset, hng64_dl[offset]) ;
//  usrintf_showmessage("dl R (%08x) : %x %x", activecpu_get_pc(), offset, hng64_dl[offset]) ;
	return hng64_dl[offset] ;
}

#ifdef UNUSED_FUNCTION
WRITE32_HANDLER( activate_3d_buffer )
{
    COMBINE_DATA (&active_3d_buffer[offset]) ;
    mame_printf_debug("COMBINED %d\n", active_3d_buffer[offset]) ;
}
#endif

// Transition Control memory.
static WRITE32_HANDLER( tcram_w )
{
	COMBINE_DATA (&hng64_tcram[offset]) ;
//  mame_printf_debug("Q1 W : %.8x %.8x\n", offset, hng64_tcram[offset]) ;

/*
    if (offset == 0x00000007)
    {
        sprintf(writeString, "%.8x ", hng64_tcram[offset]) ;
    }

    if (offset == 0x0000000a)
    {
        sprintf(writeString, "%s %.8x ", writeString, hng64_tcram[offset]) ;
    }

    if (offset == 0x0000000b)
    {
        sprintf(writeString, "%s %.8x ", writeString, hng64_tcram[offset]) ;
//      popmessage("%s", writeString) ;
    }
*/
}

static READ32_HANDLER( tcram_r )
{
//  mame_printf_debug("Q1 R : %.8x %.8x\n", offset, hng64_tcram[offset]) ;
	return hng64_tcram[offset] ;
}

/*
<ElSemi> 0xE0000000 sound
<ElSemi> 0xD0100000 3D bank A
<ElSemi> 0xD0200000 3D bank B
<ElSemi> 0xC0000000-0xC000C000 Sprite
<ElSemi> 0xC0200000-0xC0204000 palette
<ElSemi> 0xC0100000-0xC0180000 Tilemap
<ElSemi> 0xBF808000-0xBF808800 Dualport ram
<ElSemi> 0xBF800000-0xBF808000 S-RAM
<ElSemi> 0x60000000-0x60001000 Comm dualport ram
*/

static WRITE32_HANDLER( hng64_soundram_w )
{
	UINT32 mem_mask32 = mem_mask;
	UINT32 data32 = data;

	/* swap data around.. keep the v30 happy ;-) */
	data = data32 >> 16;
	data = FLIPENDIAN_INT16(data);
	mem_mask = mem_mask32 >> 16;
	mem_mask = FLIPENDIAN_INT16(mem_mask);
	COMBINE_DATA(&hng64_soundram[offset * 2 + 0]);

	data = data32 & 0xffff;
	data = FLIPENDIAN_INT16(data);
	mem_mask = mem_mask32 & 0xffff;
	mem_mask = FLIPENDIAN_INT16(mem_mask);
	COMBINE_DATA(&hng64_soundram[offset * 2 + 1]);
}

static READ32_HANDLER( hng64_soundram_r )
{
	UINT16 datalo = hng64_soundram[offset * 2 + 0];
	UINT16 datahi = hng64_soundram[offset * 2 + 1];

	return FLIPENDIAN_INT16(datahi) | (FLIPENDIAN_INT16(datalo) << 16);
}

static ADDRESS_MAP_START( hng_map, ADDRESS_SPACE_PROGRAM, 32 )

	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_BASE(&hng_mainram)
	AM_RANGE(0x04000000, 0x05ffffff) AM_WRITENOP AM_ROM AM_REGION("user3",0) AM_BASE(&hng_cart)

	// Ports
	AM_RANGE(0x1F700000, 0x1F702fff) AM_READ(hng64_port_read)

//  AM_RANGE(0x1F70100C, 0x1F70100F) AM_WRITE(SMH_NOP)        // ?? often
//  AM_RANGE(0x1F70101C, 0x1F70101F) AM_WRITE(SMH_NOP)        // ?? often
//  AM_RANGE(0x1F70106C, 0x1F70106F) AM_WRITE(SMH_NOP)        // fatfur,strange
//  AM_RANGE(0x1F701084, 0x1F701087) AM_RAM
//  AM_RANGE(0x1F70111C, 0x1F70111F) AM_WRITE(SMH_NOP)        // irq ack
	AM_RANGE(0x1F701204, 0x1F701207) AM_WRITE(hng_dma_start_w)
	AM_RANGE(0x1F701214, 0x1F701217) AM_WRITE(hng_dma_dst_w)
	AM_RANGE(0x1F701224, 0x1F701227) AM_WRITE(hng_dma_len_w)
//  AM_RANGE(0x1F70124C, 0x1F70124F) AM_WRITE(SMH_NOP)        // dma related?
//  AM_RANGE(0x1F70125C, 0x1F70125F) AM_WRITE(SMH_NOP)        // dma related?
//  AM_RANGE(0x1F7021C4, 0x1F7021C7) AM_WRITE(SMH_NOP)        // ?? often

	// SRAM.  Coin data, Player Statistics, etc.
	AM_RANGE(0x1F800000, 0x1F803fff) AM_READWRITE(hng64_sram_r, hng64_sram_w) AM_BASE(&hng64_sram)

	// Dualport RAM
	AM_RANGE(0x1F808000, 0x1F8087ff) AM_READWRITE(hng64_dualport_r, hng64_dualport_w) AM_BASE(&hng64_dualport)

	// BIOS
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0) AM_BASE(&rombase)

	// Video
	AM_RANGE(0x20000000, 0x2000bfff) AM_RAM AM_BASE(&hng64_spriteram)									// Sprites
	AM_RANGE(0x20010000, 0x20010013) AM_READ(hng64_random_read)
	AM_RANGE(0x20100000, 0x2017ffff) AM_RAM_WRITE(hng64_videoram_w) AM_BASE(&hng64_videoram)	// Tilemap
	AM_RANGE(0x20190000, 0x20190037) AM_RAM AM_BASE(&hng64_videoregs)									// Video Registers
	AM_RANGE(0x20200000, 0x20203fff) AM_READWRITE(SMH_RAM,hng64_pal_w) AM_BASE(&paletteram32)			// Palette
	AM_RANGE(0x20208000, 0x2020805f) AM_READWRITE(tcram_r, tcram_w) AM_BASE(&hng64_tcram)				// Transition Control
	AM_RANGE(0x20300000, 0x2030ffff) AM_READWRITE(dl_r, dl_w) AM_BASE(&hng64_dl)						// 3d Display List

	// 3d?
//  AM_RANGE(0x30000000, 0x3000002f) AM_READWRITE(q2_r, q2_w) AM_BASE(&hng64_q2)
	AM_RANGE(0x30100000, 0x3015ffff) AM_READWRITE(hng64_3d_1_r,hng64_3d_2_w) AM_BASE(&hng64_3d_1)		// 3D Display Buffer A
	AM_RANGE(0x30200000, 0x3025ffff) AM_READWRITE(hng64_3d_2_r,hng64_3d_2_w) AM_BASE(&hng64_3d_2)		// 3D Display Buffer B

	// Sound
	AM_RANGE(0x60000000, 0x601fffff) AM_RAM																// Sound ??
	AM_RANGE(0x60200000, 0x603fffff) AM_READWRITE(hng64_soundram_r, hng64_soundram_w)					// uploads the v53 sound program here, elsewhere on ss64-2 */

	// ?
//  AM_RANGE(0x68000000, 0x68000003) AM_WRITE(SMH_NOP)                                                // ??
//  AM_RANGE(0x68000004, 0x68000007) AM_READ(SMH_NOP)                                                 // ??
//  AM_RANGE(0x68000008, 0x6800000b) AM_WRITE(SMH_NOP)                                                // ??

	// Communications
	AM_RANGE(0xc0000000, 0xc0000fff) AM_READWRITE(hng64_com_r, hng64_com_w) AM_BASE(&hng64_com_ram)
	AM_RANGE(0xc0001000, 0xc0001007) AM_READWRITE(hng64_com_share_r, hng64_com_share_w)

	/* 6e000000-6fffffff */
	/* 80000000-81ffffff */
	/* 88000000-89ffffff */
	/* 90000000-97ffffff */
	/* 98000000-9bffffff */
	/* a0000000-a3ffffff */
ADDRESS_MAP_END

/*
roadedge_full:
index=00000000  pagesize=01000000  vaddr=00000000C0000000  paddr=0000000020000000  asid=00  r=0  c=2  dvg=dvg
index=00000000  pagesize=01000000  vaddr=00000000C1000000  paddr=0000000021000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D0000000  paddr=0000000030000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D1000000  paddr=0000000031000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000D2000000  paddr=0000000032000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000D3000000  paddr=0000000033000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000D4000000  paddr=0000000034000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000D5000000  paddr=0000000035000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E0000000  paddr=0000000060000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E1000000  paddr=0000000061000000  asid=00  r=0  c=2  dvg=dvg
index=00000005  pagesize=01000000  vaddr=00000000E8000000  paddr=0000000068000000  asid=00  r=0  c=2  dvg=dvg
index=00000005  pagesize=01000000  vaddr=00000000E9000000  paddr=0000000069000000  asid=00  r=0  c=2  dvg=dvg
index=00000006  pagesize=01000000  vaddr=00000000EE000000  paddr=000000006E000000  asid=00  r=0  c=2  dvg=dvg
index=00000006  pagesize=01000000  vaddr=00000000EF000000  paddr=000000006F000000  asid=00  r=0  c=2  dvg=dvg
index=00000007  pagesize=01000000  vaddr=0000000060000000  paddr=00000000C0000000  asid=00  r=0  c=2  dvg=dvg
index=00000007  pagesize=01000000  vaddr=0000000061000000  paddr=00000000C1000000  asid=00  r=0  c=2  dvg=dvg
index=00000008  pagesize=01000000  vaddr=0000000000000000  paddr=0000000004000000  asid=00  r=0  c=2  dvg=dvg
index=00000008  pagesize=01000000  vaddr=0000000001000000  paddr=0000000004000000  asid=00  r=0  c=2  dvg=dvg
index=00000009  pagesize=01000000  vaddr=0000000010000000  paddr=0000000080000000  asid=00  r=0  c=2  dvg=dvg
index=00000009  pagesize=01000000  vaddr=0000000011000000  paddr=0000000081000000  asid=00  r=0  c=2  dvg=dvg
index=0000000A  pagesize=01000000  vaddr=0000000020000000  paddr=0000000088000000  asid=00  r=0  c=2  dvg=dvg
index=0000000A  pagesize=01000000  vaddr=0000000021000000  paddr=0000000089000000  asid=00  r=0  c=2  dvg=dvg
index=0000000B  pagesize=01000000  vaddr=0000000030000000  paddr=0000000090000000  asid=00  r=0  c=2  dvg=dvg
index=0000000B  pagesize=01000000  vaddr=0000000031000000  paddr=0000000091000000  asid=00  r=0  c=2  dvg=dvg
index=0000000C  pagesize=01000000  vaddr=0000000032000000  paddr=0000000092000000  asid=00  r=0  c=2  dvg=dvg
index=0000000C  pagesize=01000000  vaddr=0000000033000000  paddr=0000000093000000  asid=00  r=0  c=2  dvg=dvg
index=0000000D  pagesize=01000000  vaddr=0000000034000000  paddr=0000000094000000  asid=00  r=0  c=2  dvg=dvg
index=0000000D  pagesize=01000000  vaddr=0000000035000000  paddr=0000000095000000  asid=00  r=0  c=2  dvg=dvg
index=0000000E  pagesize=01000000  vaddr=0000000036000000  paddr=0000000096000000  asid=00  r=0  c=2  dvg=dvg
index=0000000E  pagesize=01000000  vaddr=0000000037000000  paddr=0000000097000000  asid=00  r=0  c=2  dvg=dvg
index=0000000F  pagesize=01000000  vaddr=0000000040000000  paddr=0000000098000000  asid=00  r=0  c=2  dvg=dvg
index=0000000F  pagesize=01000000  vaddr=0000000041000000  paddr=0000000099000000  asid=00  r=0  c=2  dvg=dvg
index=00000010  pagesize=01000000  vaddr=0000000042000000  paddr=000000009A000000  asid=00  r=0  c=2  dvg=dvg
index=00000010  pagesize=01000000  vaddr=0000000043000000  paddr=000000009B000000  asid=00  r=0  c=2  dvg=dvg
index=00000011  pagesize=01000000  vaddr=0000000050000000  paddr=00000000A0000000  asid=00  r=0  c=2  dvg=dvg
index=00000011  pagesize=01000000  vaddr=0000000051000000  paddr=00000000A1000000  asid=00  r=0  c=2  dvg=dvg
index=00000012  pagesize=01000000  vaddr=0000000052000000  paddr=00000000A2000000  asid=00  r=0  c=2  dvg=dvg
index=00000012  pagesize=01000000  vaddr=0000000053000000  paddr=00000000A3000000  asid=00  r=0  c=2  dvg=dvg

fatfurwa_full:
index=00000000  pagesize=01000000  vaddr=00000000C0000000  paddr=0000000020000000  asid=00  r=0  c=2  dvg=dvg
index=00000000  pagesize=01000000  vaddr=00000000C1000000  paddr=0000000021000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D0000000  paddr=0000000030000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D1000000  paddr=0000000031000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000D2000000  paddr=0000000032000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000D3000000  paddr=0000000033000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000D4000000  paddr=0000000034000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000D5000000  paddr=0000000035000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E0000000  paddr=0000000060000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E1000000  paddr=0000000061000000  asid=00  r=0  c=2  dvg=dvg
index=00000006  pagesize=01000000  vaddr=00000000E8000000  paddr=0000000068000000  asid=00  r=0  c=2  dvg=dvg
index=00000006  pagesize=01000000  vaddr=00000000E9000000  paddr=0000000069000000  asid=00  r=0  c=2  dvg=dvg
index=00000009  pagesize=01000000  vaddr=00000000EE000000  paddr=000000006E000000  asid=00  r=0  c=2  dvg=dvg
index=00000009  pagesize=01000000  vaddr=00000000EF000000  paddr=000000006F000000  asid=00  r=0  c=2  dvg=dvg
index=0000000B  pagesize=01000000  vaddr=0000000000000000  paddr=0000000004000000  asid=00  r=0  c=2  dvg=dvg
index=0000000B  pagesize=01000000  vaddr=0000000001000000  paddr=0000000004000000  asid=00  r=0  c=2  dvg=dvg
index=0000000C  pagesize=01000000  vaddr=0000000010000000  paddr=0000000080000000  asid=00  r=0  c=2  dvg=dvg
index=0000000C  pagesize=01000000  vaddr=0000000011000000  paddr=0000000081000000  asid=00  r=0  c=2  dvg=dvg
index=0000000D  pagesize=01000000  vaddr=0000000020000000  paddr=0000000088000000  asid=00  r=0  c=2  dvg=dvg
index=0000000D  pagesize=01000000  vaddr=0000000021000000  paddr=0000000089000000  asid=00  r=0  c=2  dvg=dvg
index=0000000E  pagesize=01000000  vaddr=0000000030000000  paddr=0000000090000000  asid=00  r=0  c=2  dvg=dvg
index=0000000E  pagesize=01000000  vaddr=0000000031000000  paddr=0000000091000000  asid=00  r=0  c=2  dvg=dvg
index=0000000F  pagesize=01000000  vaddr=0000000032000000  paddr=0000000092000000  asid=00  r=0  c=2  dvg=dvg
index=0000000F  pagesize=01000000  vaddr=0000000033000000  paddr=0000000093000000  asid=00  r=0  c=2  dvg=dvg
index=00000010  pagesize=01000000  vaddr=0000000034000000  paddr=0000000094000000  asid=00  r=0  c=2  dvg=dvg
index=00000010  pagesize=01000000  vaddr=0000000035000000  paddr=0000000095000000  asid=00  r=0  c=2  dvg=dvg
index=00000011  pagesize=01000000  vaddr=0000000036000000  paddr=0000000096000000  asid=00  r=0  c=2  dvg=dvg
index=00000011  pagesize=01000000  vaddr=0000000037000000  paddr=0000000097000000  asid=00  r=0  c=2  dvg=dvg
index=00000012  pagesize=01000000  vaddr=0000000040000000  paddr=0000000098000000  asid=00  r=0  c=2  dvg=dvg
index=00000012  pagesize=01000000  vaddr=0000000041000000  paddr=0000000099000000  asid=00  r=0  c=2  dvg=dvg
index=00000013  pagesize=01000000  vaddr=0000000042000000  paddr=000000009A000000  asid=00  r=0  c=2  dvg=dvg
index=00000013  pagesize=01000000  vaddr=0000000043000000  paddr=000000009B000000  asid=00  r=0  c=2  dvg=dvg
index=00000014  pagesize=01000000  vaddr=0000000050000000  paddr=00000000A0000000  asid=00  r=0  c=2  dvg=dvg
index=00000014  pagesize=01000000  vaddr=0000000051000000  paddr=00000000A1000000  asid=00  r=0  c=2  dvg=dvg
index=00000015  pagesize=01000000  vaddr=0000000052000000  paddr=00000000A2000000  asid=00  r=0  c=2  dvg=dvg
index=00000015  pagesize=01000000  vaddr=0000000053000000  paddr=00000000A3000000  asid=00  r=0  c=2  dvg=dvg
index=00000016  pagesize=01000000  vaddr=0000000060000000  paddr=00000000C0000000  asid=00  r=0  c=2  dvg=dvg
index=00000016  pagesize=01000000  vaddr=0000000061000000  paddr=00000000C1000000  asid=00  r=0  c=2  dvg=dvg


roadedge/fatfurwa/buriki:
index=00000000  pagesize=01000000  vaddr=00000000C0000000  paddr=0000000020000000  asid=00  r=0  c=2  dvg=dvg
index=00000000  pagesize=01000000  vaddr=00000000C1000000  paddr=0000000021000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D0000000  paddr=0000000030000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000D1000000  paddr=0000000031000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E0000000  paddr=0000000060000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000E1000000  paddr=0000000061000000  asid=00  r=0  c=2  dvg=dvg
index=00000007  pagesize=01000000  vaddr=0000000060000000  paddr=00000000C0000000  asid=00  r=0  c=2  dvg=dvg
index=00000007  pagesize=01000000  vaddr=0000000061000000  paddr=00000000C1000000  asid=00  r=0  c=2  dvg=dvg

sams64_2:
index=00000000  pagesize=01000000  vaddr=00000000C0000000  paddr=0000000020000000  asid=00  r=0  c=2  dvg=dvg
index=00000000  pagesize=01000000  vaddr=00000000C1000000  paddr=0000000021000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000C2000000  paddr=0000000030000000  asid=00  r=0  c=2  dvg=dvg
index=00000001  pagesize=01000000  vaddr=00000000C3000000  paddr=0000000031000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000C4000000  paddr=0000000060000000  asid=00  r=0  c=2  dvg=dvg
index=00000002  pagesize=01000000  vaddr=00000000C5000000  paddr=0000000061000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000C6000000  paddr=000000006E000000  asid=00  r=0  c=2  dvg=dvg
index=00000003  pagesize=01000000  vaddr=00000000C7000000  paddr=000000006F000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000C8000000  paddr=0000000068000000  asid=00  r=0  c=2  dvg=dvg
index=00000004  pagesize=01000000  vaddr=00000000C9000000  paddr=0000000069000000  asid=00  r=0  c=2  dvg=dvg
*/


/* COMM CPU */
#define KL5C_MMU_A(xxx) ( (xxx==0) ? 0x0000 : (hng64_com_mmu_mem[((xxx-1)*2)+1] << 2) | ((hng64_com_mmu_mem[(xxx-1)*2] & 0xc0) >> 6) )
#define KL5C_MMU_B(xxx) ( (xxx==0) ? 0x0000 : (hng64_com_mmu_mem[(xxx-1)*2] & 0x3f) )

static OPBASE_HANDLER( KL5C80_opbase_handler )
{
	opbase->rom = opbase->ram = hng64_com_op_base;
	return ~0;
}

static UINT32 KL5C80_translate_address(UINT16 vAddr)
{
	int i ;
	UINT8 bNum = 4 ;

	/* Determine what B the vAddr is in */
	for (i = 1; i < 5; i++)
	{
		if ( ((KL5C_MMU_B(i)+1)*0x400) > vAddr)
		{
			bNum = i-1;
			break;
		}
	}

	/* Calculate the full address and return */
	if (bNum == 0)
	{
		return vAddr;
	}
	else
	{	/* the offset to the base physical address plus the vAddr's offset from the virtual base */
		return ((KL5C_MMU_A(bNum) + (KL5C_MMU_B(bNum)+1)) * 0x400) + (vAddr - ((KL5C_MMU_B(bNum)+1) * 0x400));
	}
}

static void KL5C80_virtual_mem_sync(void)
{
	/* The KL5C80 maps each progressive chunk from the beginning of the
       virtual region until the beginning of the next virtual region.
       This is implemented here in a lame way to simplify the code.  */

	int i,region;

	for (region = 0; region < 5; region++)
	{
		int logical_offset  = (KL5C_MMU_B(region)+1) * 0x400;
		int physical_offset = (KL5C_MMU_A(region) + (KL5C_MMU_B(region)+1)) * 0x400;

		/* The first MMU region is a special case */
		if (region == 0)
		{
			logical_offset  = 0x0000;
			physical_offset = 0x00000;
		}

		logerror("Now copying 0x%x to 0x%x\n", physical_offset, logical_offset);
		for (i = logical_offset; i <= 0xffff; i++)
		{
			if (physical_offset+i <= 0xfffff)
				hng64_com_op_base[i] = hng64_com_virtual_mem[physical_offset+i];
		}
	}
}

static void KL5C80_init(void)
{
	/* init the MMU */
	hng64_com_mmu_mem[0] =
	hng64_com_mmu_mem[2] =
	hng64_com_mmu_mem[4] =
	hng64_com_mmu_mem[6] = 0x3f;

	hng64_com_mmu_mem[1] =
	hng64_com_mmu_mem[3] =
	hng64_com_mmu_mem[5] = 0x00;
	hng64_com_mmu_mem[7] = 0xf0;
}

static READ8_HANDLER( hng64_comm_memory_r )
{
	UINT32 physical_address = KL5C80_translate_address(offset) ;
	logerror("READING 0x%02x from 0x%04x (0x%05x)\n", hng64_com_virtual_mem[physical_address], offset, physical_address) ;

	/* Custom "virtual" memory map */
	if (physical_address >= 0x26000 && physical_address <= 0x28000)
	{
		/* May very well be shared memory - it reads 16 bits from it at 0x309 */
	}


	return hng64_com_virtual_mem[physical_address] ;
}

static WRITE8_HANDLER( hng64_comm_memory_w )
{
//  UINT32 physical_address = KL5C80_translate_address(offset) ;
//  logerror("WRITING 0x%02x to 0x%04x (0x%05x)\n", hng64_com_virtual_mem[physical_address], offset, physical_address) ;

	// Write to both virtual and physical memory
}


/* KL5C80 I/O handlers */
static WRITE8_HANDLER( hng64_comm_io_mmu )
{
	hng64_com_mmu_mem[offset] = data;

	/* Debugging - you can't change A4 - the hardware doesn't let you */
	if (hng64_com_mmu_mem[7] != 0xf0 || ((hng64_com_mmu_mem[6] & 0xc0) != 0x00))
		logerror("KL5C MMU error !!! Code is trying to change A4!\n");

	logerror("COMM CPU MMU WRITE : ") ;
	logerror("B : %02x %02x %02x %02x  A : %03x %03x %03x %03x\n", KL5C_MMU_B(1), KL5C_MMU_B(2), KL5C_MMU_B(3), KL5C_MMU_B(4),
																   KL5C_MMU_A(1), KL5C_MMU_A(2), KL5C_MMU_A(3), KL5C_MMU_A(4)) ;
	KL5C80_virtual_mem_sync();
}

#ifdef UNUSED_FUNCTION
READ8_HANDLER( hng64_comm_shared_r )
{
    // I'm thinking 0x54 comes from an interrupt on the MIPS CPU?  Or maybe the Toshiba one?
    // Nothing from CPU0 seems to ping the COM CPU as often as it reads 0x54.
    // Sometimes it wants 0x54 to be a 0x01 and sometimes it wants a 0x02.

    // It's not an interrupt on the KL5C80 because they aren't enabled before 0x54 is ping'ed
    // There is a special onboard interrupt handler for the KL5C80 though...

    if (offset==0x00) logerror("COM CPU reading from 0x50.\n");
    if (offset==0x01) logerror("COM CPU reading from 0x51.\n");
    if (offset==0x02) logerror("COM CPU reading from 0x52.\n");
    if (offset==0x03) logerror("COM CPU reading from 0x53.\n");
    if (offset==0x04) (hng64_com_shared_b & 0x000000ff) ? return 0xff : return 0x00;
}

WRITE8_HANDLER( hng64_comm_shared_w )
{
    if (offset==0x00) hng64_com_shared_a = (hng64_com_shared_a & 0x00ffffff) | (data << 24);
    if (offset==0x01) hng64_com_shared_a = (hng64_com_shared_a & 0xff00ffff) | (data << 16);
    if (offset==0x02) hng64_com_shared_a = (hng64_com_shared_a & 0xffff00ff) | (data <<  8);
    if (offset==0x03) hng64_com_shared_a = (hng64_com_shared_a & 0xffffff00) | (data <<  0);
    if (offset==0x04) logerror("COM CPU writing to 0x54.\n");

    logerror("COM CPU wrote to com_shared_a : %08x\n", hng64_com_shared_a);
}
#endif

static ADDRESS_MAP_START( hng_comm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000,0xffff) AM_READWRITE( hng64_comm_memory_r, hng64_comm_memory_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( hng_comm_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* Reserved for the KL5C80 internal hardware */
	AM_RANGE(0x00,0x07) AM_WRITE( hng64_comm_io_mmu ) AM_BASE(&hng64_com_mmu_mem)
//  AM_RANGE(0x08,0x1f) AM_NOP              /* Reserved */
//  AM_RANGE(0x20,0x25) AM_READWRITE        /* Timer/Counter B */           /* hng64 writes here */
//  AM_RANGE(0x27,0x27) AM_NOP              /* Reserved */
//  AM_RANGE(0x28,0x2b) AM_READWRITE        /* Timer/Counter A */           /* hng64 writes here */
//  AM_RANGE(0x2c,0x2f) AM_READWRITE        /* Parallel port A */
//  AM_RANGE(0x30,0x33) AM_READWRITE        /* Parallel port B */
//  AM_RANGE(0x34,0x37) AM_READWRITE        /* Interrupt controller */      /* hng64 writes here */
//  AM_RANGE(0x38,0x39) AM_READWRITE        /* Serial port */               /* hng64 writes here */
//  AM_RANGE(0x3a,0x3b) AM_READWRITE        /* System control register */   /* hng64 writes here */
//  AM_RANGE(0x3c,0x3f) AM_NOP              /* Reserved */

	/* General IO */
	AM_RANGE(0x50,0x54) AM_NOP // AM_WRITE(hng64_comm_shared_r, hng64_comm_shared_w)
//  AM_RANGE(0x72,0x72) AM_WRITE            /* dunno yet */
ADDRESS_MAP_END




static ADDRESS_MAP_START( hng_sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x3ffff) AM_READ(SMH_BANK2)
	AM_RANGE(0xe0000, 0xfffff) AM_READ(SMH_BANK1)
ADDRESS_MAP_END


static INPUT_PORTS_START( hng64 )
	PORT_START_TAG("IPT_TEST")
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE( KEYCODE_Q )
	PORT_BIT(  0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IPT_NONE")

	PORT_START_TAG("FATFURWA_TST_ETC")
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(  0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_DIPNAME( 0x8000, 0x0000, "TST" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START_TAG("FATFURWA_PLR_1")
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(  0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START_TAG("FATFURWA_PLR_2")
	PORT_BIT(  0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(  0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END



/* the 4bpp gfx encoding is annoying */

static const gfx_layout hng64_4_even_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 40, 44, 8,12,  32,36,    0,4},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout hng64_4_odd_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{  56,60, 24, 28,  48,52,   16, 20 },


	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


static const gfx_layout hng64_4_16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{  56,60, 24, 28,  48,52,   16, 20,40, 44, 8,12,  32,36,    0,4 },


	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


static const gfx_layout hng64_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 56, 24, 48,16,  40, 8,  32, 0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


static const gfx_layout hng64_16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },

	{ 56, 24, 48,16,  40, 8,  32, 0,
	 1024+56, 1024+24, 1024+48,1024+16,  1024+40, 1024+8,  1024+32, 1024+0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	32*64
};


static GFXDECODE_START( hng64 )
	GFXDECODE_ENTRY( "gfx1", 0, hng64_4_even_layout, 0x0, 0x100 ) /* scrolltiles */
	GFXDECODE_ENTRY( "gfx1", 0, hng64_4_odd_layout,  0x0, 0x100 ) /* scrolltiles */
	GFXDECODE_ENTRY( "gfx1", 0, hng64_layout,        0x0, 0x10 )  /* scrolltiles */
	GFXDECODE_ENTRY( "gfx1", 0, hng64_16_layout,     0x0, 0x10 )  /* scroll tiles */
	GFXDECODE_ENTRY( "gfx2", 0, hng64_4_16_layout,   0x0, 0x100 ) /* sprite tiles */
	GFXDECODE_ENTRY( "gfx2", 0, hng64_16_layout,     0x0, 0x10 )  /* sprite tiles */
GFXDECODE_END

static DRIVER_INIT( hng64 )
{
	hng64_soundram=auto_malloc(0x200000);
}

static DRIVER_INIT(hng64_fght)
{
	no_machine_error_code=0x01010101;
	DRIVER_INIT_CALL(hng64);
}

static DRIVER_INIT(hng64_race)
{
	no_machine_error_code=0x02020202;
	DRIVER_INIT_CALL(hng64);
}



/* ?? */
static const mips3_config config =
{
	16384,				/* code cache size */
	16384				/* data cache size */
};

static TIMER_CALLBACK( irq_stop )
{
	cpunum_set_input_line(machine, 0, 0, CLEAR_LINE);
}

static INTERRUPT_GEN( irq_start )
{
	logerror("HNG64 interrupt level %x\n", cpu_getiloops());

	/* there are more, the sources are unknown at the moment */
	switch (cpu_getiloops())
	{
		case 0x00: hng64_interrupt_level_request = 0;
		break;
		case 0x01: hng64_interrupt_level_request = 1;
		break;
		case 0x02: hng64_interrupt_level_request = 2;
		break;
	}

	cpunum_set_input_line(machine, 0, 0, ASSERT_LINE);
	timer_set(ATTOTIME_IN_USEC(50), NULL, 0, irq_stop);
}




static MACHINE_RESET(hyperneo)
{
	int i ;
	const UINT8 *rom = memory_region(machine, "user2");

	/* Sound CPU */
	UINT8 *RAM = (UINT8*)hng64_soundram;
	memory_set_bankptr(1,&RAM[0x1e0000]);
	memory_set_bankptr(2,&RAM[0x001000]); // where..
	cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, ASSERT_LINE);
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);


	/* Comm CPU */
	KL5C80_init();

	/* 1 meg of virtual address space for the com cpu */
	hng64_com_virtual_mem = auto_malloc(0x100000);
	hng64_com_op_base     = auto_malloc(0x10000);

	/* Fill up virtual memory with ROM */
	for (i = 0x0; i < 0x100000; i++)
		hng64_com_virtual_mem[i] = rom[i] ;

	KL5C80_virtual_mem_sync();
	memory_set_opbase_handler(2, KL5C80_opbase_handler);

	cpunum_set_input_line(machine, 2, INPUT_LINE_RESET, PULSE_LINE);     // reset the CPU and let 'er rip
//  cpunum_set_input_line(machine, 2, INPUT_LINE_HALT, ASSERT_LINE);     // hold on there pardner...



	// "Display List" init - ugly
	activeBuffer = 0 ;

	/* set the fastest DRC options */
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_DRC_OPTIONS, MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY);

	/* configure fast RAM regions for DRC */
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_SELECT, 0);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_START, 0x00000000);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_END, 0x00ffffff);
	cpunum_set_info_ptr(0, CPUINFO_PTR_MIPS3_FASTRAM_BASE, hng_mainram);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_READONLY, 0);

	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_SELECT, 1);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_START, 0x04000000);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_END, 0x05ffffff);
	cpunum_set_info_ptr(0, CPUINFO_PTR_MIPS3_FASTRAM_BASE, hng_cart);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_READONLY, 1);

	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_SELECT, 2);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_START, 0x1fc00000);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_END, 0x1fc7ffff);
	cpunum_set_info_ptr(0, CPUINFO_PTR_MIPS3_FASTRAM_BASE, rombase);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_READONLY, 1);
}


static MACHINE_DRIVER_START( hng64 )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", R4600BE, MASTER_CLOCK)  	// actually R4300
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(hng_map, 0)
	MDRV_CPU_VBLANK_INT_HACK(irq_start,3)

	MDRV_CPU_ADD("audio", V30,8000000)		 		// v53, 16? mhz!
	MDRV_CPU_PROGRAM_MAP(hng_sound_map,0)

	MDRV_CPU_ADD("comm", Z80,MASTER_CLOCK/4)		/* KL5C80A12CFP - binary compatible with Z80. */
	MDRV_CPU_PROGRAM_MAP(hng_comm_map,0)
	MDRV_CPU_IO_MAP(hng_comm_io_map, 0)

	MDRV_GFXDECODE(hng64)
	MDRV_MACHINE_RESET(hyperneo)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(1024, 1024)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 16, 447)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(hng64)
	MDRV_VIDEO_UPDATE(hng64)
MACHINE_DRIVER_END



ROM_START( hng64 )
	ROM_REGION32_BE( 0x0100000, "user1", 0 ) /* 512k for R4300 BIOS code */
	ROM_LOAD ( "brom1.bin", 0x000000, 0x080000,  CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) )
	ROM_REGION( 0x0100000, "user2", 0 ) /* KL5C80 BIOS and unknown ROM */
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) )
	ROM_LOAD ( "rom1.bin",  0x080000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) )

	/* To placate MAME */
	ROM_REGION32_LE( 0x2000000, "user3", ROMREGION_ERASEFF ) /* Program Code, mapped at ??? maybe banked?  LE? */
	ROM_REGION( 0x4000, "gfx1", ROMREGION_DISPOSE | ROMREGION_ERASEFF )
	ROM_REGION( 0x4000, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "gfx3", ROMREGION_ERASEFF )
	ROM_REGION16_BE( 0x0c00000, "gfx4", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "samples", ROMREGION_ERASEFF ) /* Sound Samples? */
ROM_END

/* roads edge might need a different bios (driving board bios?) */
ROM_START( roadedge )

	/* BIOS */
	ROM_REGION32_BE( 0x0100000, "user1", 0 ) /* 512k for R4300 BIOS code */
	ROM_LOAD ( "brom1.bin", 0x000000, 0x080000,  CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) )
	ROM_REGION( 0x0100000, "user2", 0 ) /* KL5C80 BIOS and unknown ROM */
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) )
	ROM_LOAD ( "rom1.bin",  0x080000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) )
	/* END BIOS */

	ROM_REGION32_LE( 0x2000000, "user3", 0 ) /* Program Code, mapped at ??? maybe banked?  LE? */
	ROM_LOAD32_WORD( "001pr01b.81", 0x0000000, 0x400000, CRC(effbac30) SHA1(c1bddf3e511a8950f65ac7e452f81dbc4b7fd977) )
	ROM_LOAD32_WORD( "001pr02b.82", 0x0000002, 0x400000, CRC(b9aa4ad3) SHA1(9ab3c896dbdc45560b7127486e2db6ca3b15a057) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "001sc01a.41", 0x0000000, 0x400000, CRC(084395a1) SHA1(8bfea8fd3981fd45dcc04bd74840a5948aaf06a8) )
	ROM_LOAD32_BYTE( "001sc02a.42", 0x0000001, 0x400000, CRC(51dd19e3) SHA1(eeb3634294a049a357a75ee00aa9fce65b737395) )
	ROM_LOAD32_BYTE( "001sc03a.43", 0x0000002, 0x400000, CRC(0b6f3e19) SHA1(3b6dfd0f0633b0d8b629815920edfa39d92336bf) )
	ROM_LOAD32_BYTE( "001sc04a.44", 0x0000003, 0x400000, CRC(256c8c1c) SHA1(85935eea3722ec92f8d922f527c2e049c4185aa3) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "001sp01a.53",0x0000000, 0x400000, CRC(7a469453) SHA1(3738ca76f538243bb23ffd23a42b2a0558882889) )
	ROM_LOAD32_BYTE( "001sp02a.54",0x0000001, 0x400000, CRC(6b9a3de0) SHA1(464c652f7b193326e3a871dfe751dd83c14284eb) )
	ROM_LOAD32_BYTE( "001sp03a.55",0x0000002, 0x400000, CRC(efbbd391) SHA1(7447c481ba6f9ba154d48a4b160dd24157891d35) )
	ROM_LOAD32_BYTE( "001sp04a.56",0x0000003, 0x400000, CRC(1a0eb173) SHA1(a69b786a9957197d1cc950ab046c57c18ca07ea7) )

	/* Textures - 1024x1024x8 pages */
	ROM_REGION( 0x1000000, "gfx3", 0 )
	/* note: same roms are at different positions on the board, repeated a total of 4 times*/
	ROM_LOAD( "001tx01a.1", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.2", 0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.3", 0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.4", 0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )
	ROM_LOAD( "001tx01a.5", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.6", 0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.7", 0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.8", 0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )
	ROM_LOAD( "001tx01a.9", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.10",0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.11",0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.12",0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )
	ROM_LOAD( "001tx01a.13",0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.14",0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.15",0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.16",0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION( 0x0c00000, "gfx4", ROMREGION_NODISPOSE )
	ROMX_LOAD( "001vt01a.17", 0x0000000, 0x400000, CRC(1a748e1b) SHA1(376d40baa3b94890d4740045d053faf208fe43db), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "001vt02a.18", 0x0000002, 0x400000, CRC(449f94d0) SHA1(2228690532d82d2661285aeb4260689b027597cb), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "001vt03a.19", 0x0000004, 0x400000, CRC(50ac8639) SHA1(dd2d3689466990a7c479bb8f11bd930ea45e47b5), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "samples", ROMREGION_DISPOSE ) /* Sound Samples? */
	ROM_LOAD( "001sd01a.77", 0x0000000, 0x400000, CRC(a851da99) SHA1(2ba24feddafc5fadec155cdb7af305fdffcf6690) )
	ROM_LOAD( "001sd02a.78", 0x0400000, 0x400000, CRC(ca5cec15) SHA1(05e91a602728a048d61bf86aa8d43bb4186aeac1) )
ROM_END


ROM_START( sams64_2 )

	/* BIOS */
	ROM_REGION32_BE( 0x0100000, "user1", 0 ) /* 512k for R4300 BIOS code */
	ROM_LOAD ( "brom1.bin", 0x000000, 0x080000,  CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) )
	ROM_REGION( 0x0100000, "user2", 0 ) /* KL5C80 BIOS and unknown ROM */
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) )
	ROM_LOAD ( "rom1.bin",  0x080000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) )
	/* END BIOS */

	ROM_REGION32_LE( 0x2000000, "user3", 0 ) /* Program Code, mapped at ??? maybe banked?  LE? */
	ROM_LOAD32_WORD( "005pr01a.81", 0x0000000, 0x400000, CRC(a69d7700) SHA1(a580783a109bc3e24248d70bcd67f62dd7d8a5dd) )
	ROM_LOAD32_WORD( "005pr02a.82", 0x0000002, 0x400000, CRC(38b9e6b3) SHA1(d1dad8247d920cc66854a0096e1c7845842d2e1c) )
	ROM_LOAD32_WORD( "005pr03a.83", 0x0800000, 0x400000, CRC(0bc738a8) SHA1(79893b0e1c4a31e02ab385c4382684245975ae8f) )
	ROM_LOAD32_WORD( "005pr04a.84", 0x0800002, 0x400000, CRC(6b504852) SHA1(fcdcab432162542d249818a6cd15b8f2e8230f97) )
	ROM_LOAD32_WORD( "005pr05a.85", 0x1000000, 0x400000, CRC(32a743d3) SHA1(4088b930a1a4d6224a0939ef3942af1bf605cdb5) )
	ROM_LOAD32_WORD( "005pr06a.86", 0x1000002, 0x400000, CRC(c09fa615) SHA1(697d6769c16b3c8f73a6df4a1e268ec40cb30d51) )
	ROM_LOAD32_WORD( "005pr07a.87", 0x1800000, 0x400000, CRC(44286ad3) SHA1(1f890c74c0da0d34940a880468e68f7fb1417813) )
	ROM_LOAD32_WORD( "005pr08a.88", 0x1800002, 0x400000, CRC(d094eb67) SHA1(3edc8d608c631a05223e1d05157cd3daf2d6597a) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "005sc01a.97",  0x0000000, 0x800000, CRC(7f11cda9) SHA1(5fbdabd8423e9723a6ec38f8503e6ca7f4f69fdd) )
	ROM_LOAD32_BYTE( "005sc02a.99",  0x0000001, 0x800000, CRC(87d1e1a7) SHA1(00f2ef46ce64ab715add8cd47745c57944286f81) )
	ROM_LOAD32_BYTE( "005sc03a.101", 0x0000002, 0x800000, CRC(a5d4c535) SHA1(089a3cd07701f025024ce73b7b4d38063c33a59f) )
	ROM_LOAD32_BYTE( "005sc04a.103", 0x0000003, 0x800000, CRC(14930d77) SHA1(b4c613a8896e21fe2cac0595dd1ea30dc7fce0bd) )
	ROM_LOAD32_BYTE( "005sc05a.98",  0x2000000, 0x800000, CRC(4475a3f8) SHA1(f099baf766ee00d166cfa8402baa0b6ea25a0010) )
	ROM_LOAD32_BYTE( "005sc06a.100", 0x2000001, 0x800000, CRC(41c0fbbd) SHA1(1d9ac01c9499a6202ee59d15d498ec34edc05888) )
	ROM_LOAD32_BYTE( "005sc07a.102", 0x2000002, 0x800000, CRC(3505b198) SHA1(2fdfdd5a1f6f31f5fb1c0af70047108d1df44af2) )
	ROM_LOAD32_BYTE( "005sc08a.104", 0x2000003, 0x800000, CRC(3139e413) SHA1(38210541379ddeba8c0b9ef8fa5430c0090db7c7) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "005sp01a.105",0x0000000, 0x800000, CRC(68eefee5) SHA1(d95bd7b549900500633af07544423b0062ac07ce) )
	ROM_LOAD32_BYTE( "005sp02a.109",0x0000001, 0x800000, CRC(5d9a49b9) SHA1(50768c496a3e0b4379e121349f32edec4f18652f) )
	ROM_LOAD32_BYTE( "005sp03a.113",0x0000002, 0x800000, CRC(9b6530fe) SHA1(398433b98578a6b4b950afc4d6318916376e0760) )
	ROM_LOAD32_BYTE( "005sp04a.117",0x0000003, 0x800000, CRC(d4e422ce) SHA1(9bfaa533ab3d014cdb0c535cf6952e01925cc30b) )
	ROM_LOAD32_BYTE( "005sp05a.106",0x2000000, 0x400000, CRC(d8b1fb26) SHA1(7da767d8e817c52afc416ccfe8caf30f66c233ef) )
	ROM_LOAD32_BYTE( "005sp06a.110",0x2000001, 0x400000, CRC(87ed72a0) SHA1(0d7db4dc9f15a0377a83f020ffbe81621ca77cff) )
	ROM_LOAD32_BYTE( "005sp07a.114",0x2000002, 0x400000, CRC(8eb3c173) SHA1(d5763c19a3e2fd93f7784d957e7401c9152c40de) )
	ROM_LOAD32_BYTE( "005sp08a.118",0x2000003, 0x400000, CRC(05486fbc) SHA1(747d9ae03ce999be4ab697753e93c90ea85b7d44) )

	/* Textures - 1024x1024x8 pages */
	ROM_REGION( 0x1000000, "gfx3", 0 )
	/* note: same roms are at different positions on the board, repeated a total of 4 times*/
	ROM_LOAD( "005tx01a.1", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.2", 0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.3", 0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.4", 0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )
	ROM_LOAD( "005tx01a.5", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.6", 0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.7", 0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.8", 0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )
	ROM_LOAD( "005tx01a.9", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.10",0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.11",0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.12",0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )
	ROM_LOAD( "005tx01a.13",0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.14",0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.15",0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.16",0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION( 0x1800000, "gfx4", ROMREGION_NODISPOSE )
	ROMX_LOAD( "005vt01a.17", 0x0000000, 0x400000, CRC(48a61479) SHA1(ef982b1ecc6dfca2ad989391afcc1b3d1e7fe652), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt02a.18", 0x0000002, 0x400000, CRC(ba9100c8) SHA1(f7704fb8e5310ea7d0e6ae6b8935717ec9119b6d), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt03a.19", 0x0000004, 0x400000, CRC(f54a28de) SHA1(c445cf7fee71a516065cf37e05b898208f48b17e), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt04a.20", 0x0400000, 0x400000, CRC(57ad79c7) SHA1(bc382317323c1f8a31b69ae3100d3bba6b5d0838), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt05a.21", 0x0400002, 0x400000, CRC(49c82bec) SHA1(09255279edb9a204bbe1cce8cef58d5c81e86d1f), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt06a.22", 0x0400004, 0x400000, CRC(7ba05b6c) SHA1(729c1d182d74998dd904b587a2405f55af9825e0), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "samples", ROMREGION_DISPOSE ) /* Sound Samples? */
	ROM_LOAD( "005sd01a.77", 0x0000000, 0x400000, CRC(8f68150f) SHA1(a1e5efdfd1ed29f81e25c8da669851ddb7b0c826) )
	ROM_LOAD( "005sd02a.78", 0x0400000, 0x400000, CRC(6b4da6a0) SHA1(8606c413c129635bdaaa37254edbfd19b10426bb) )
	ROM_LOAD( "005sd03a.79", 0x0800000, 0x400000, CRC(a529fab3) SHA1(8559d402c8f66f638590b8b57ec9efa775010c96) )
	ROM_LOAD( "005sd04a.80", 0x0800000, 0x400000, CRC(dca95ead) SHA1(39afdfba0e5262b524f25706a96be00e5d14548e) )
ROM_END


ROM_START( fatfurwa )

	/* BIOS */
	ROM_REGION32_BE( 0x0100000, "user1", 0 ) /* 512k for R4300 BIOS code */
	ROM_LOAD ( "brom1.bin", 0x000000, 0x080000,  CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) )
	ROM_REGION( 0x0100000, "user2", 0 ) /* KL5C80 BIOS and unknown ROM */
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) )
	ROM_LOAD ( "rom1.bin",  0x080000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) )
	/* END BIOS */

	ROM_REGION32_LE( 0x2000000, "user3", 0 ) /* Program Code, mapped at ??? maybe banked?  LE? */
	ROM_LOAD32_WORD( "006pr01a.81", 0x0000000, 0x400000, CRC(3830efa1) SHA1(9d8c941ccb6cbe8d138499cf9d335db4ac7a9ec0) )
	ROM_LOAD32_WORD( "006pr02a.82", 0x0000002, 0x400000, CRC(8d5de84e) SHA1(e3ae014263f370c2836f62ab323f1560cb3a9cf0) )
	ROM_LOAD32_WORD( "006pr03a.83", 0x0800000, 0x400000, CRC(c811b458) SHA1(7d94e0df501fb086b2e5cf08905d7a3adc2c6472) )
	ROM_LOAD32_WORD( "006pr04a.84", 0x0800002, 0x400000, CRC(de708d6c) SHA1(2c9848e7bbf61c574370f9ecab5f5a6ba63339fd) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "006sc01a.97", 0x0000000, 0x800000, CRC(f13dffad) SHA1(86363aeae176fd4204e446c13a028da919dc2069) )
	ROM_LOAD32_BYTE( "006sc02a.99", 0x0000001, 0x800000, CRC(be79d42a) SHA1(f3eb950a62e2df1de116af9434027439f1305e1f) )
	ROM_LOAD32_BYTE( "006sc03a.101",0x0000002, 0x800000, CRC(16918b73) SHA1(ad0c751a301fe3c95fca19473869dfd55fb6b0de) )
	ROM_LOAD32_BYTE( "006sc04a.103",0x0000003, 0x800000, CRC(9b63cd98) SHA1(62519a3a531c4493a5a85dc01ca69413977120ca) )
	ROM_LOAD32_BYTE( "006sc05a.98", 0x2000000, 0x800000, CRC(0487297b) SHA1(d3fa4d691559327739c96717312faf09b498001d) )
	ROM_LOAD32_BYTE( "006sc06a.100",0x2000001, 0x800000, CRC(34a76c31) SHA1(be05dc75afb7cde65ba5d29c0e66a7b1b62c41cb) )
	ROM_LOAD32_BYTE( "006sc07a.102",0x2000002, 0x800000, CRC(7a1c371e) SHA1(1cd4ad66dd007adc9ab0c29720cbf9955c7337e0) )
	ROM_LOAD32_BYTE( "006sc08a.104",0x2000003, 0x800000, CRC(88232ade) SHA1(4ae2a572c3525087f77c95185e8697a1fc720512) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "006sp01a.105",0x0000000, 0x800000, CRC(087b8c49) SHA1(bb1eb2baef7da91f904bf45414f21dd6bac30749) )
	ROM_LOAD32_BYTE( "006sp02a.109",0x0000001, 0x800000, CRC(da28631e) SHA1(ea7e2d9195cfa4f954f4d542296eec1323223653) )
	ROM_LOAD32_BYTE( "006sp03a.113",0x0000002, 0x800000, CRC(bb87b55b) SHA1(8644ebb356ae158244a6e03254b0212cb359e167) )
	ROM_LOAD32_BYTE( "006sp04a.117",0x0000003, 0x800000, CRC(2367a536) SHA1(304b5b7f7e5d41e69fbd4ac2a938c42f3766630e) )
	ROM_LOAD32_BYTE( "006sp05a.106",0x2000000, 0x800000, CRC(0eb8fd06) SHA1(c2b6fab1b0104910d7bb39d0a496ada39c5cc122) )
	ROM_LOAD32_BYTE( "006sp06a.110",0x2000001, 0x800000, CRC(dccc3f75) SHA1(fef8d259c17a78e2266fed965fba1e15f1cd01dd) )
	ROM_LOAD32_BYTE( "006sp07a.114",0x2000002, 0x800000, CRC(cd7baa1b) SHA1(4084f3a73aae623d69bd9de87cecf4a33b628b7f) )
	ROM_LOAD32_BYTE( "006sp08a.118",0x2000003, 0x800000, CRC(9c3044ac) SHA1(24b28bcc6be51ab3ff59c2894094cd03ec377d84) )

	/* Textures - 1024x1024x8 pages */
	ROM_REGION( 0x1000000, "gfx3", 0 )
	/* note: same roms are at different positions on the board, repeated a total of 4 times*/
	ROM_LOAD( "006tx01a.1", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.2", 0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.3", 0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.4", 0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )
	ROM_LOAD( "006tx01a.5", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.6", 0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.7", 0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.8", 0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )
	ROM_LOAD( "006tx01a.9", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.10",0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.11",0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.12",0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )
	ROM_LOAD( "006tx01a.13",0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.14",0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.15",0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.16",0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_BE( 0x0c00000, "gfx4", ROMREGION_NODISPOSE )
	ROMX_LOAD( "006vt01a.17", 0x0000000, 0x400000, CRC(5c20ed4c) SHA1(df679f518292d70b9f23d2bddabf975d56b96910), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "006vt02a.18", 0x0000002, 0x400000, CRC(150eb717) SHA1(9acb067346eb386256047c0f1d24dc8fcc2118ca), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "006vt03a.19", 0x0000004, 0x400000, CRC(021cfcaf) SHA1(fb8b5f50d3490b31f0a4c3e6d3ae1b98bae41c97), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "samples", ROMREGION_DISPOSE ) /* Sound Samples? */
	ROM_LOAD( "006sd01a.77", 0x0000000, 0x400000, CRC(790efb6d) SHA1(23ddd3ee8ae808e58cbcaf92a9ef56d3ca6289b5) )
	ROM_LOAD( "006sd02a.78", 0x0400000, 0x400000, CRC(f7f020c7) SHA1(b72fde4ff6384b80166a3cb67d31bf7afda750bc) )
	ROM_LOAD( "006sd03a.79", 0x0800000, 0x400000, CRC(1a678084) SHA1(f52efb6145102d289f332d8341d89a5d231ba003) )
	ROM_LOAD( "006sd04a.80", 0x0800000, 0x400000, CRC(3c280a5c) SHA1(9d3fc78e18de45382878268db47ff9d9716f1505) )
ROM_END

ROM_START( buriki )

	/* BIOS */
	ROM_REGION32_BE( 0x0100000, "user1", 0 ) /* 512k for R4300 BIOS code */
	ROM_LOAD ( "brom1.bin", 0x000000, 0x080000,  CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) )
	ROM_REGION( 0x0100000, "user2", 0 ) /* KL5C80 BIOS and unknown ROM */
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) )
	ROM_LOAD ( "rom1.bin",  0x080000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) )
	/* END BIOS */

	ROM_REGION32_LE( 0x2000000, "user3", 0 ) /* Program Code, mapped at ??? maybe banked?  LE? */
	ROM_LOAD32_WORD( "007pr01b.81", 0x0000000, 0x400000, CRC(a31202f5) SHA1(c657729b292d394ced021a0201a1c5608a7118ba) )
	ROM_LOAD32_WORD( "007pr02b.82", 0x0000002, 0x400000, CRC(a563fed6) SHA1(9af9a021beb814e35df968abe5a99225a124b5eb) )
	ROM_LOAD32_WORD( "007pr03a.83", 0x0800000, 0x400000, CRC(da5f6105) SHA1(5424cf5289cef66e301e968b4394e551918fe99b) )
	ROM_LOAD32_WORD( "007pr04a.84", 0x0800002, 0x400000, CRC(befc7bce) SHA1(83d9ecf944e03a40cf25ee288077c2265d6a588a) )
	ROM_LOAD32_WORD( "007pr05a.85", 0x1000000, 0x400000, CRC(013e28bc) SHA1(45e5ac45b42b26957c2877ac1042472c4b5ec914) )
	ROM_LOAD32_WORD( "007pr06a.86", 0x1000002, 0x400000, CRC(0620fccc) SHA1(e0bffc56b019c79276a4ef5ec7354edda15b0889) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "007sc01a.97", 0x0000000, 0x800000, CRC(4e8300db) SHA1(f1c9e6fddc10efc8f2a530027cca062f48b8c8d4) )
	ROM_LOAD32_BYTE( "007sc02a.99", 0x0000001, 0x800000, CRC(d5855944) SHA1(019c0bd2f8de7ffddd53df6581b40940262f0053) )
	ROM_LOAD32_BYTE( "007sc03a.101",0x0000002, 0x800000, CRC(ff45c9b5) SHA1(ddcc2a10ccac62eb1f3671172ad1a4d163714fca) )
	ROM_LOAD32_BYTE( "007sc04a.103",0x0000003, 0x800000, CRC(e4cb59e9) SHA1(4e07ff374890217466a53d5bfb1fa99eb7402360) )
	ROM_LOAD32_BYTE( "007sc05a.98", 0x2000000, 0x400000, CRC(27f848c1) SHA1(2ee9cca4e68e56c7c17c8e2d7e0f55a34a5960bd) )
	ROM_LOAD32_BYTE( "007sc06a.100",0x2000001, 0x400000, CRC(c39e9b4c) SHA1(3c8a0494c2a6866ecc0df2c551619c57ee072440) )
	ROM_LOAD32_BYTE( "007sc07a.102",0x2000002, 0x400000, CRC(753e7e3d) SHA1(39b2e9fd23878d8fc4f98fe88b466e963d8fc959) )
	ROM_LOAD32_BYTE( "007sc08a.104",0x2000003, 0x400000, CRC(b605928e) SHA1(558042b84115273fa581606daafba0e9688fa002) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD32_BYTE( "007sp01a.105",0x0000000, 0x800000, CRC(160acae6) SHA1(37c15e1d2544ec6f3b61d06200345d6abdd28edf) )
	ROM_LOAD32_BYTE( "007sp02a.109",0x0000001, 0x800000, CRC(1a55331d) SHA1(0b03d5c7312e01874365b31f1ff3d9766abd00f1) )
	ROM_LOAD32_BYTE( "007sp03a.113",0x0000002, 0x800000, CRC(3f308444) SHA1(0acd52312c15a2ed3bacf60a2fd820cb09ebbb55) )
	ROM_LOAD32_BYTE( "007sp04a.117",0x0000003, 0x800000, CRC(6b81aa51) SHA1(55f7702e1d7a2bef7f050d0358de9036a0139877) )
	ROM_LOAD32_BYTE( "007sp05a.106",0x2000000, 0x400000, CRC(32d2fa41) SHA1(b16a0bbd397be2a8d532c85951b924e2e086a189) )
	ROM_LOAD32_BYTE( "007sp06a.110",0x2000001, 0x400000, CRC(b6f8d7f3) SHA1(70ce94f2193ee39218022da617413c42f6753574) )
	ROM_LOAD32_BYTE( "007sp07a.114",0x2000002, 0x400000, CRC(5caa1cc9) SHA1(3e40b10ea3bcf1239d0015da4be869632b805ddd) )
	ROM_LOAD32_BYTE( "007sp08a.118",0x2000003, 0x400000, CRC(7a158c67) SHA1(d66f4920a513208d45b908a1934d9afb894debf1) )

	/* Textures - 1024x1024x8 pages */
	ROM_REGION( 0x1000000, "gfx3", 0 )
	/* note: same roms are at different positions on the board, repeated a total of 4 times*/
	ROM_LOAD( "007tx01a.1", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.2", 0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.3", 0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.4", 0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )
	ROM_LOAD( "007tx01a.5", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.6", 0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.7", 0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.8", 0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )
	ROM_LOAD( "007tx01a.9", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.10",0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.11",0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.12",0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )
	ROM_LOAD( "007tx01a.13",0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.14",0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.15",0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.16",0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_BE( 0x0c00000, "gfx4", ROMREGION_NODISPOSE )
	ROMX_LOAD( "007vt01a.17", 0x0000000, 0x400000, CRC(f78a0376) SHA1(fde4ddd4bf326ae5f1ed10311c237b13b62e060c), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "007vt02a.18", 0x0000002, 0x400000, CRC(f365f608) SHA1(035fd9b829b7720c4aee6fdf204c080e6157994f), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "007vt03a.19", 0x0000004, 0x400000, CRC(ba05654d) SHA1(b7fe532732c0af7860c8eded3c5abd304d74e08e), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "samples", ROMREGION_DISPOSE ) /* Sound Samples? */
	ROM_LOAD( "007sd01a.77", 0x0000000, 0x400000, CRC(1afb48c6) SHA1(b072d4fe72d6c5267864818d300b32e85b426213) )
	ROM_LOAD( "007sd02a.78", 0x0400000, 0x400000, CRC(c65f1dd5) SHA1(7f504c585a10c1090dbd1ac31a3a0db920c992a0) )
	ROM_LOAD( "007sd03a.79", 0x0800000, 0x400000, CRC(356f25c8) SHA1(5250865900894232960686f40c5da35b3868b78c) )
	ROM_LOAD( "007sd04a.80", 0x0800000, 0x400000, CRC(dabfbbad) SHA1(7d58d5181705618e0e2d69c6fdb81b9b3d2b9e0f) )
ROM_END

/* Bios */
GAME( 1997, hng64,  0,        hng64, hng64, hng64,      ROT0, "SNK", "Hyper NeoGeo 64 Bios", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_IS_BIOS_ROOT )

/* Games */
GAME( 1997, roadedge, hng64,  hng64, hng64, hng64_race, ROT0, "SNK", "Roads Edge / Round Trip (rev.B)",	  GAME_NOT_WORKING|GAME_NO_SOUND )	/* 001 */
/* Samurai Shodown 64           002 */
/* Xtreme Rally / Offbeat Racer 003 */
/* Beast Busters 2nd Nightmare  004 */
GAME( 1998, sams64_2, hng64,  hng64, hng64, hng64_fght, ROT0, "SNK", "Samurai Shodown: Warrior's Rage",	  GAME_NOT_WORKING|GAME_NO_SOUND )	/* 005 */
GAME( 1998, fatfurwa, hng64,  hng64, hng64, hng64_fght, ROT0, "SNK", "Fatal Fury: Wild Ambition (rev.A)", GAME_NOT_WORKING|GAME_NO_SOUND )	/* 006 */
GAME( 1999, buriki,   hng64,  hng64, hng64, hng64_fght, ROT0, "SNK", "Buriki One (rev.B)",				  GAME_NOT_WORKING|GAME_NO_SOUND )	/* 007 */

