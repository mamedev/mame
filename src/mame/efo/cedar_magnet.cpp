// license:BSD-3-Clause
// copyright-holders:David Haywood
// thanks to: Ricky2001, ArcadeHacker, IFW

/*
 TODO:
  - Fix sound emulation (speed needs verifying + sample playback)
  - Fix sprite communication / banking
    * bit "output bit 0x02 %d (IC21)" at 0x42 might be important
    * mag_exzi currently requires a gross hack to stop the sprite CPU crashing on startup
    * mag_xain sometimes leaves old sprites on the screen, probably due to a lost clear
      command
  - Fix flipscreen
  - Verify behavior of unknown / unused ports / interrupt sources etc.
  - Verify the disk images, convert to a better format that can natively store protection
    * RAW data also available if required
    * as mentioned, the disks are copy protected, see notes below
  - Use proper floppy drive emulation code that originally came from MESS (tied with above)
  - Verify all clocks and screen params (50hz seems to match original videos)
  - Work out why we need a protection hack and replace it with proper emulation
    * there are no per-game protection devices, so it's something to do with the base hardware
    * there seem to be 2 checks, one based on a weird sector on the discs, the other based on
      a port read
  - Layer enables on War Mission? (transitions from title screen etc.)

 notes:
  - high scores will be defaulted if the data in the table is corrupt, the games give no
    option to do this otherwise.  A backup copy of the score table is kept, so you also
    have to enter and exit service mode.

*/

/*

 Magnet System by EFO SA (Electrónica Funcional Operativa SA).
 Based on Cedar hardware

 http://retrolaser.es/cedar-computer-el-ordenador-profesional-de-efo-sa/
 http://www.recreativas.org/magnet-system-2427-efosa

 A number of original games as well as conversions were advertised for this system, it is however
 believed that EFO went bankrupt before anything hit the market. The conversions are not simply
 bootlegs, they're completely original pieces of code more akin to home computer ports.

 The following were advertised
  Original Games
  - A Day in Space ** *
  - Crazy Driver
  - Formula
  - Jungle Trophy
  - Paris Dakar ** *
  - Quadrum
  - Sailing Race
  - Scorpio
  - The Burning Cavern *
  - War Mission ** *

  Ports / Conversions
  - Booby Kids *
  - Cocomania (Pacmania)
  - Dodge Ball
  - Double Dragon
  - Dracula's Castle (Haunted House)
  - Exzisus *
  - Flying Shark
  - Super Contra
  - Time Scanner *
  - Twin Eagle
  - World Wars (Bermuda Triangle)
  - Xain d'Sleena *

  ** screenshots present on flyer
  * dumps exist


Disk Protection

Sectors are 1024 (0x400) bytes long but marked on the disc as 512 bytes as a copy protection
Sector numbering for each track starts at 200 (0xC8) again, presumably as a protection.
Each track has 6 sectors (200 - 205)
The drive runs at 240 RPM allowing for ~25% extra data. (attempting to dump at other speeds won't work)

 data order / sector marking
 track 0, side 0, sector 200...205 (instead of sector 0...5)
 track 0, side 1, sector 200...205
 track 1, side 0, sector 200...205
 track 1, side 1, sector 200...205

Note, the games store settings / scores to the disk and don't provide any kind of 'factory reset'
option, so if used the data will not be pristine.

PCB Setup

The hardware consists of 5 main PCBs in a cage.
1x Audio PCB (on top)
1x Master PCB
2x Plane PCBs (both identical aside from jumper settings)
1x Sprite PCB

On some systems, there are small memory sub-boards on the Master PCB and Sprite PCB; due to the awkwardness of
the banking at times (and the fact that even with 4 banks of 256 colours, only one can be active).
I suspect the additional memory was an afterthought.

SOUND BOARD
   __________________________________________________________________________
   |  __________      __________      ___________________      __________   |
   | |OKI M5205|     |_MC74HC02|     |Z843004PSC Z80 CTC|     |_MC74HC74|   |
   |                  __________     |__________________|      __________   |
   |  __________     |MC74HC393|      ___________________     |_MC74HC00|   |
   | |__MF10CN_|      __________     |Z843004PSC Z80 CTC|      __________   |
   |                 |_MC74HC03|     |__________________|     |_MC74HC14|   |
   |  __________                                               __________   |
   | |__MF10CN_|      __________    _____________________     |_MC74HC32|   |
   |                 |HCF40105BE   |AY-3-8910A          |    ____________   _| 5
   |                  __________   |____________________|   |_SS74HC241E|   _| 0
   |  __________     |_MC74HC74|                             ____________   _|
   | |_TC4066BP|      __________    _____________________   |_MC74HC374_|   _| P
   |                 |MC74HC138|   |AY-3-8910A          |    ____________   _| I
   |                  __________   |____________________|   |_MC74HC245_|   _| N
   |                 |TC4013BP_|    _____________________    ____________   _|
   |                  __________   |Z840004PSC Z80 CPU  |   |_MC74HC244_|   _| C
   |                 |MC74HC74_|   |____________________|    ____________   _| O
   |                                __________ __________   |_MC74HC244_|   _| N
   |                  __________   |MC74HC157||MC74HC157|      __________   |
   |                 |CD74HC04E|                              |MC74HC157|   |
   |                  __________    __________ __________      __________   |
   |                 |CD74HC393|  TMM41464P-15 TMM41464P-15   |MC74HC393|   |
   |________________________________________________________________________|


MASTER BOARD
   __________________________________________________________________________
   |                                                               J10 CONN |
   |   _____________________                                   __________   |
   |  | SIEMENS SAB 2797B P|                                  |MC74HC244|   |
   |  |____________________|        __________                 __________   |
   |                               |_MC74HC00|                |_MC74HC74|   |
   |                                              _____________________     _| 5
   |   __________    __________     __________   |Z0842004PSC Z80 PIO  |    _| 0
   |  |MC74HC393|   |CD74HC04E|    |TC74HC138P   |_____________________|    _|
   |   __________    __________     __________    _____________________     _| P
   |  |_SN7406N_|   |CD74HC04E|    |MC74HC139|   |Z0842004PSC Z80 PIO  |    _| I
   |   __________    __________   ____________   |_____________________|    _| N
   |  |SN74LS14N|   |_MC74HC10|  |AMPAL16R8PC|    _____________________     _|
   |   __________ __________  ________________   |Z0840004PSC Z80 CPU  |    _| C
   |  |_MC74HC32||_MC74HC08| |VID E03 MBM2764|   |_____________________|    _| O
   |   __________            |_______________|                              _| N
   |  |_MC74HC74|                                                           |
   |      __________   __________   __________    __________                |
   |     |_MC74HC86|  |_MC74HC74|  |MC74HC174|   |MC74HC157|                |
   |      __________   __________   __________    __________  __________    |
   |     |CD74HC04E|  |_MC74F04N|  |_MC74HC74|   |MC74HC157| M74ALS161AP    |
   |      __________                __________    __________  __________    |
   |     |_MC74HC14|  ____   ____  |_MC74HC27|   HYB41256-12 M74ALS161AP    |
   |                  XTAL1  XTAL2  __________    __________  __________    _| 5
   |                               |_MC74HC10|   HYB41256-12 M74ALS161AP    _| 0
 __|    ____________   __________   __________    __________  __________    _|
|__    |_MC74HC244_|  |_SN7406N_|  |_MC74HC08|   |MC74HC157| AMPAL16R4APC   _| P
|__     ____________   __________   __________    __________  __________    _| I
|__    |_MC74HC244_|  |_MC74HC00|  |TC4040BP_|   |MC74HC157| |CD74HC374E    _| N
|__     ____________   __________  ___________    __________  __________    _|
|__    |_MC74HC244_|  |_MC74HC00| |_MC74HC244|   |MC74HC157| |CD74HC374E    _| C
|__                   ___________   __________    __________  __________    _| O
|__                  |_MC74HC273|  |MM2114N-3L   |MC74HC157| |MC74HC244|    _| N
|__                    __________   __________    __________   _________    |
|__                   |MC74HC174|  |MM2114N-3L   |MC74HC157|  |MC74HC157    |
|__                    __________   __________    __________   _________    |
|__                   |MC74HC174|  |MM2114N-3L   |MC74HC157|  |MC74HC393    |
|__     CONN                                                                |
   |________________________________________________________________________|

Xtal 1 = 16.000 MHz
Xtal 2 = 20.000 MHz

 PLANES BOARD
   ___________________________________________________________________________
   |    __________   __________   __________   __________    __________      |
   |   |KM4164B-15  |_MC74HC02|  |_MC74HC74|  |MC74HC157|   |_MC74HC02|      |
   |    __________   __________                __________    __________      |
   |   |KM4164B-15  |_SN7406N_|               |MC74HC139|   |_MC74HC04|      |
   |    __________   __________   __________   __________    __________ CONN |
   |   |KM4164B-15  |_MC74HC74|  |MC74HC157|  |MC74HC393|   |_MC74HC00|      _| 5
   |    __________   __________   __________   __________    __________      _| 0
   |   |KM4164B-15  |_MC74HC86|  |MC74HC161|  |_MC74HC74|   |_MC74HC04|      _|
   |    __________   __________   ___________  __________    __________      _| P
   |   |KM4164B-15  |_MC74HC86|  |_MC74HC273| |MC74HC153|   |MC74HC153|      _| I
   |    __________   __________   __________   __________    ___________     _| N
   |   |KM4164B-15  |MC74HC153|  |MC74HC161|  |MC74HC161|   |_MC74HC241|     _|
   |    __________   __________   __________   __________    __________      _| C
   |   |KM4164B-15  |MC74HC153|  |MC74HC157|  |MC74HC161|   |_MC74HC74|      _| O
   |    __________   __________   __________   __________    ___________     _| N
   |   |KM4164B-15  |_MC74HC08|  |MC74HC393|  |MC74HC157|   |_MC74HC245|     |
   |    _______________________                __________    ___________     |
   |   |Z0842004PSC Z80 PIO   |               SN74ALS161BN  |_MC74HC244|     |
   |   |______________________|                                              |
   |    _______________________                                              |
   |   |Z0842004PSC Z80 PIO   |   __________   __________    ___________     |
   |   |______________________|  |_MC74HC86|  |MC74HC157|   |_MC74HC244|     |
   |    __________   __________   __________   ________________________      _| 5
   |   |KM4164B-15  SN74ALS161BN |MC74HC161|  |Z0840004PSC Z80 CPU    |      _| 0
   |    __________                            |_______________________|      _|
   |   |KM4164B-15                                                           _| P
   |    __________   __________   __________   __________    __________      _| I
   |   |KM4164B-15  SN74ALS161BN |_MC74HC86|  |_MC74HC32_|  |_MC74HC157| ··  _| N
   |    __________   ___________  __________   __________    __________  ··  _|
   |   |KM4164B-15  |_MC74HC273| |MC74HC161|  |_MC74HC08_|  |_MC74HC38N| ··  _| C
   |    __________   __________                              __________      _| O
   |   |KM4164B-15  |_MC74HC27|                             |_MC74HC38N| ··  _| N
   |    __________   ___________  __________   __________    __________  ··  |
   |   |KM4164B-15  |_MC74HC273| |_MC74HC10|  |_MC74F32N_|  |_MC74HC38N| ··  |
   |    __________                                                       ··  |
   |   |KM4164B-15                                                       ··  |
   |    __________   ___________  __________   __________    __________  ··  |
   |   |KM4164B-15  |_MC74HC245| |_MC74HC14|  |_MC74HC11_|  |_MC74HC164| ··  |
   |_________________________________________________________________________|


 SPRITES BOARD

   ___________________________________________________________________________
   |    __________   __________   __________   ________________________      |
   |   SN74ALS161BN |MC74HC153|  |MC74HC161|  |MK3881N-4 Z80 PIO      |      |
   |                                          |_______________________|      |
   |    __________   __________   __________   ________________________      |
   |   |MC74HC153|  |MC74HC153|  |MC74HC161|  |MK3881N-4 Z80 PIO      | CONN |
   |                                          |_______________________|      _| 5
   |    ___________  __________   __________   ________________________      _| 0
   |   |_MC74HC273| |_MC74HC86|  |MC74HC161|  |MK3881N-4 Z80 PIO      |      _|
   |    __________   __________   __________  |_______________________|      _| P
   |   |MC74HC86_|  |_MC74HC74|  |_MC74HC86|   ________________________      _| I
   |    __________   __________   __________  |X0840004PSC Z80 CPU    |      _| N
   |   |MC74HC138|  |MC74HC153|  SN74ALS161BN |_______________________|      _|
   |    __________   __________   __________   __________    __________      _| C
   |   |MC74HC245|  |_MC74HC04|  |_MC74HC86|  SN74ALS161BN  |_MC74HC04|      _| O
   |                 __________   __________   __________    ___________     _| N
   |                |MC74HC153|  |MC74HC161|  |MC74HC393|   |_MC74HC241|     |
   |    __________   __________   __________   __________    __________      |
   |   |KM4164B-15  TMS4256-12NL |MC74HC157|  |MC74HC157|   |MC74HC161|      |
   |    __________   __________   __________   __________    ___________     |
   |   |KM4164B-15  TMS4256-12NL |_MC74HC74|  |MC74HC161|   |_MC74HC245|     |
   |    __________   __________   __________   __________    ___________     |
   |   |KM4164B-15  TMS4256-12NL |_MC74F32N|  |MC74HC161|   |_MC74HC244|     |
   |    __________   __________   __________   __________    ___________     _| 5
   |   |KM4164B-15  TMS4256-12NL |MC74HC153|  |MC74HC161|   |_MC74HC244|     _| 0
   |    __________   __________   __________   __________    ___________     _|
   |   |KM4164B-15  TMS4256-12NL |_MC74HC08|  |MC74HC157|   |_MC74HC244|     _| P
   |    __________   __________   __________   __________    ___________     _| I
   |   |KM4164B-15  TMS4256-12NL |MC74HC157|  |MC74HC161|   |_MC74HC244|     _| N
   |    __________   __________   __________   __________    __________      _|
   |   |KM4164B-15  TMS4256-12NL |MC74HC139|  |MC74HC161|   |_MC74HC11|      _| C
   |    __________   __________   __________   __________    __________      _| O
   |   |KM4164B-15  TMS4256-12NL |_MC74HC00|  |MC74HC164|   |SN74LS38N|      _| N
   |    ___________  ___________  __________   __________    __________     |
   |   |_MC74HC273| |_MC74HC273| |_MC74HC20|  |_MC74HC10|   |_MC74HC27|     |
   |    __________   __________   __________   __________    __________     |
   |   |_MC74HC10|  |MC74HC157|  |_MC74HC74|  |_MC74HC08|   |MC74HC38N|     |
   |    __________   __________   __________   __________    __________     |
   |   |_MC74HC74|  |MC74HC153|  |_MC74HC27|  |_MC74HC02|   |MC74HC38N|     |
   |    __________   __________   __________   __________    __________     |
   |   |_MC74HC74|  |MC74HC153|  |_MC74HC08|  |_MC74HC04|   |_MC74HC14|     |
   |    __________   __________   __________   __________    __________     |
   |   |_MC74HC74|  |_MC74HC00|  |MC74HC393|  |_MC74HC08|   |MC74HC164|     |
   |    __________   __________   __________   __________    __________     |
   |   |_SN7406N_|  |_MC74HC74|  |_MC74HC32|  |_MC74HC02|   M74ALS161AP     |
   |                                                         __________     |
   |                                                        |MC74HC157|     |
   |________________________________________________________________________|
*/

/*
Additional hardware notes from ArcadeHacker:


Z80 main @ 3.9986 Mhz
---------------------

Memory map
‘Bank bits’ are these from IC 48 PIO  (0x20)
A5 -> 4 74HC10 3NAND IC19  // Main CPU Memory bank switch
A6 -> 2 74HC10 3NAND IC19  // Main CPU Memory bank switch

Initial state  (bank bits 11?)
0000-1FFF eprom
2000-3FFF eprom mirror
4000-5FFF eprom mirror
6000-7FFF eprom mirror
8000-9FFF eprom mirror
A000-BFFF eprom mirror
C000-DFFF eprom mirror
E000-FFFF eprom mirror

Upload to other CPUs state (Bank bits 10 )

This is complex and not 100% understood.

 0000 - BFFF   - RAM?  (same as in state 01 / 00)
 C000  FFFF   - a 0x4000 sized window into the RAM for the other CPUs, uploads programs**

**see 0x43 IC 49 PIO
B2 -> 36 J7
B3 -> 35 J7
and MAYBE  B1 -> 3 74HC04 IC21
2 bank bits gives a full 0x10000 (64kb view) but could be more?

WHICH cpu we get a window into accessible seems to be determined by the busreq bits (main CPU must have bus?)
IC48 A1 -> 47 J6 - output (DH: busreq plane board a?)
IC48 B1 -> 48 J6 - output (DH: busreq plane board b?)
IC48 B5 -> 43 J6 - output (DH: busreq sprite board?)
IC49 B5 -> 8 J3  - output (DH: sound CPU busreq)

Is it possible to upload to multiple CPUs at once this way??


Simple Ram/ROM state (Banks bits 01)
     0000 - DFFF   - RAM?    (same as in state 10 / 00)
     E000-FFFF eprom 

ALL RAM (Banks bits 00)
     0000 - DFFF   - RAM?    (same as in state 10 / 01)
     E000-FFFF - more ram, only accessible in this state?


I/O map

0x18 <-> 0001 1000 ?
0x19 <-> 0001 1001 ?
0x1A ->  0001 1010 ?
0x1B <-  0001 1011 ?

Loop checking bit 2 on port 19
// 9496 in a,($19)
// 9498 bit 2,a
Writes a stream of data from rom to the other ports (garbage??)

IC 48 PIO 
0x20 0010 0000 IC48 Data port A    // Bits 5&6 manage banked mem
0x21 0010 0001 IC48 Command port A

A0 -> 49 J6 - input from plane board a (main CPU checks before accessing)
A1 -> 47 J6 - output (DH: busreq plane board a?)
A2 -> 45 J6 - output (DH: reset plane board a CPU?)
A3 <- 4 74HC14P (inverter) IC4 <- EDGE 21 COIN1
A4 <- 9 74HC74 IC20 <- input from 18 74LS244 IC61   - Interrupt source related? (DH - gets checked in IRQ code)
A5 -> 4 74HC10 3NAND IC19  // Main CPU Memory bank switch
A6 -> 2 74HC10 3NAND IC19  // Main CPU Memory bank switch
A7 -> 12 J4 unpopulated

0x22 0010 0010 IC48 Data port B 
0x23 0010 0011 IC48 Command port B

B0 -> 50 J6 - input from plane board b (main CPU checks before accessing)
B1 -> 48 J6 - output (DH: busreq plane board b?)
B2 -> 46 J6 - output (DH: reset plane board b CPU?)
B3 -> Q8 transistor (DH: unknown purpose)
B4 -> 44 J6 - input from sprite board (main CPU checks before accessing)
B5 -> 43 J6 - output (DH: busreq sprite board?)
B6 -> 41 J6 - output (DH: reset sprite board CPU?)
B7 <- 2 74HC14P (inverter) IC4 <- EDGE 22 COIN2

IC 49 PIO
0x40 NOT USED IC49 Data port A
0x41 0100 0001   IC49 Command port A
A0 -> 8 IC36 unpopulated
A1 -> 7 IC36 unpopulated
A2 -> 6 IC36 unpopulated
A3 -> 5 IC36 unpopulated
A4 -> 4 IC36 unpopulated
A5 -> 3 IC36 unpopulated
A6 -> 2 IC36 unpopulated
A7 -> 1 IC36 unpopulated

0x42 0100 0010 IC49 Data port B 
0x43 0100 0011 IC49 Command port B
	
B0 -> LED LD1  (DH: only? Or does it go somewhere else too?)
B1 -> 3 74HC04 IC21   (DH: some kind of bank/writeprotect? Gets set during weird sprite CPU comms important??)
B2 -> 36 J7  (DH: bank bits for main CPU window into Sprite RAM)
B3 -> 35 J7  (DH: bank bits for main CPU window into Sprite RAM)
B4 -> 7 J3  (from sound CPU)
B5 -> 8 J3  (sound CPU busreq)
B6 -> 9 J3  (sound CPU - Z80CTC reset?)
B7 -> Q9 transistor  (unknown purpose)

Disk controller
0x60 0110 0000 // Status (read) / Command (write)
0x61 0110 0001 // Current Track # (read)
0x62 0110 0010 // Requested sector (write)
0x63 0110 0011 // Data (read) / Parameter for Command (write)

Player controls
0x64 1 74LS244 IC3 /OE edge  Player 1 inputs
D0 P1 up - D1 P1 right - D2 P1 down - D3 P1 left - D4 Action1 - D5 P1 Action2 - D6 P1 Action3 - D7 P1 Start
0x68 1 74LS244 IC2 /OE edge  Player 2 inputs
D0 P2 up - D1 P2 right - D2 P2 down - D3 P2 left - D4 P2 Action 1 - D5  Action2 - D6 P2 Action3 - D7 P2 Start
0x6C 1 74LS244 IC1 /OE edge  16-19 not used

0x70 0111 0000 IC42 1  Memory daughter board 74LS273 10 Clock input
0x74 0111 0100 IC42 2  Memory daughter board 74LS00 1A
0x78 0111 1000 IC42 3 ← Watchdog (disable watchdog IC5 3 to ground after initial cpu reset) Memory daughter board 74LS374 11 Clock input

0x70/0x74/0x78 are used for data upload, presumably to the 3x MOTOROLA MM2114N SRAM chips on the memory sub-board, palette?

0x70 also seems to be used for banking of other RAM (writes of 1c/1d/1e/1f, and maybe 00) 
CONFIRMED
Lower 2 bits are used to select 64kb main CPU ram banks
Upper 6 bits are used to select where palette writes go


0x7C 0111 1100 ?

7c is read at the end of an interrupt routine
There’s also an odd loop waiting for the low 7 bits to be non-zero while accessing 0xFFFF
Does 0xFFFF do something more than just act as ram? 

088E: 00        	nop
088F: F3        	di
0890: 3E 54     	ld   a,$54
0892: F6 28     	or   $28
0894: 4F        	ld   c,a
0895: 21 FF FF  	ld   hl,$FFFF
0898: 46        	ld   b,(hl)
0899: 70        	ld   (hl),b
089A: 70        	ld   (hl),b
089B: 70        	ld   (hl),b
089C: ED 50     	in   d,(c)   << port 7c
089E: 70        	ld   (hl),b
089F: ED 58     	in   e,(c)  << port 7c
08A1: 7B        	ld   a,e
08A2: E6 7F     	and  $7F
08A4: 20 08     	jr   nz,$08AE
08A6: 2E 1A     	ld   l,$1A
08A8: 26 FF     	ld   h,$FF
08AA: 46        	ld   b,(hl)
08AB: 70        	ld   (hl),b
08AC: 18 E7     	jr   $0895

08AE: 7A        	ld   a,d
08AF: 07        	rlca
08B0: 07        	rlca
08B1: 07        	rlca
08B2: 07        	rlca
08B3: AA        	xor  d
08B4: 07        	rlca
08B5: 07        	rlca
08B6: 7A        	ld   a,d
08B7: 17        	rla
08B8: AB        	xor  e
08B9: E6 7F     	and  $7F
08BB: C8        	ret  z
08BC: 3E FF     	ld   a,$FF
08BE: 32 8C 08  	ld   ($088C),a
08C1: C9        	ret


0xFF - out, sound latch? Coin counter? Written after reading coins at least.



Z80 sound @ 3.9986 Mhz
----------------------

Memory map
0000-FFFF Ram (full 64kb or mirrored? check mem chip count on pcb)

J2 sound board connector (labeled as J3 on main board)

1 EDGE25 PARTS SPEAKER +
2 EDGE25 SOLDER SPEAKER -
3 GND
4 GND
5 +5v
6 +12v
7 Ic28 74244 1 19 /ENA   ic29  74244 1 19 /ENA   ic32 74241 1 19 ENA   ic33 7432 3 12
8 ic34 74HC14  9 -> IC35 74HC00 5 -> IC19 Z80 CPU 25 /Busreq
9 ic34 74HC14 11 -> IC22 IC23 Z80CTC 17 /Reset


Z80 sound port map

0x00 - 0x03 AM_DEVREADWRITE("ctc0", z80ctc_device, read, write)
0x04 -0x07 AM_DEVREADWRITE("ctc1", z80ctc_device, read, write)

0x0c - 0x0d AM AY8910 #1
0x10 - 0x11 AM_AY8910 #2
    
0x14 input from main cpu? (maybe read what was written to 0xff?)

There’s also an M5205 ADPCM playback chip, not seen writes for it.


Sound board CPU iorq's

Z80 IORQ goes into IC11 (74138) /ENA1 control together with ENA3 (the output of IC35 (7400) watching over the z80 RD/WR signals), /ENA2 is gnd.
Z80 A0 -> 28 IC20/21 AY-3-8910A BC2 (BC1 is grounded)
Z80 A0 A1 -> 18/19 IC22/23 Z80 CTC CS0 CS1 Selects
Z80 A2 A3 A4 -> IC11 decoder  inputs A0 A1 A2

Decoder IC11 outputs as follows:

15 /Y0 -> 16 IC22 Z80CTC /CE
14 /Y1 -> 16 IC23 Z80CTC /CE
13 /Y2 -> 3 IC8 (7404 inverter) -> 3 IC13 HCF40105BE (FIFO) SI (Shift in)
12 /Y3 -> 1 IC8 (7404 inverter) -> 27 IC20 AY-3-8910A BDIR
11 /Y4 -> 5 IC8 (7404 inverter) -> 27 IC21 AY-3-8910A BDIR
10 /Y5 -> 13 IC12 (7474 FF) CLEAR & 1 IC31 (74HC374 FF) /OE
9 /Y6 -> N.C.
7 /Y7 -> N.C.



Z80 sprite notes
----------------

Port maps

  0xc0 - 0xc3 AM_DEVREADWRITE("z80pio0", z80pio_device, read_alt, write_alt)
  0xc4 - 0xc7 AM_DEVREADWRITE("z80pio1", z80pio_device, read_alt, write_alt)
  0xc8 - 0xcb AM_DEVREADWRITE("z80pio2", z80pio_device, read_alt, write_alt)

   0x88  write, upper 8 bits of address for port-based data upload
   0x8c  data for port upload
   (0x9c -0x9c) unknown

Z80pio2
Addr 0xc8 (data port A) - write, lower 8 bits of address fo port-based data upload

Can upload 64kb of data (to sprite ram sub-board?) using address provided in ports 0x88 and 0xc8, then data write at 0x8c

Z80pio0
Address 0xc0 (data port A) - write mask 0x20 seems important, deasserted after some startup code is executed 3 times.  Where does it go?



Z80 plane board notes
---------------------

Each board has jumpers set differently (only difference? Probably controls addressing / mixing?)

Port maps

0xc0 - 0xc3 AM_DEVREADWRITE("z80pio0", z80pio_device, read_alt, write_alt)
0xc4 - 0xc7 AM_DEVREADWRITE("z80pio1", z80pio_device, read_alt, write_alt)

0xcc unknown
0xcd unknown
0xce unknown
0xcf unknown



Main memory daughter board
--------------------------
1.5kbytes 3x MOTOROLA MM2114N SRAM 4096 bit (4x1024bit) http://data.datasheetlib.com/pdf1/73/22/732266/mm2114n_5dc843bb0d.pdf

256kbytes 8x SIEMENS HYB 41256-15 AA - 262,144 bit DRAM (32kbytes)
http://matthieu.benoit.free.fr/cross/data_sheets/HYB41256.pdf



Disk controller
---------------
SAB 2797B (FM - MFM type) @ 1.25mhz (oscillates 1.250~1.251)
http://cygnus.speccy.cz/download/datasheety/sab2793.pdf


Format description
------------------

Disks are written at 240RPM which increases the capacity by 25% compared to a standard disk. A normal track written with a 300 RPM drive would be 12500 bytes, a 240 RPM drive would fill 15625 bytes.

Each track has 6 sectors, and each sector is 1024 bytes long.
The sector headers are marked as if the sector length was 512 bytes, this effectively prevents copying the second half of the data in each sector unless you know the real sector size. (plus have a drive running at 240 RPM...)

Sectors are numbered from 200...205, instead of 0...5.
e.g. 
track 0, side 0, sector 200...205 (instead of sector 0...5)
track 0, side 1, sector 200...205
track 1, side 0, sector 200...205
track 1, side 1, sector 200...205



Z80 main -  partial schem
-------------------------

A0 - A0 eprom IC31 & i9 pal16r8pc ic32 & 5 PIO IC48 CONTROL/DATA SEL 
A1 - A1 eprom IC31 & i8 pal16r8pc ic32 & 6 PIO IC48 PORT B/A SEL 
A2 - A2 eprom IC31 & i7 pal16r8pc ic32
A3 - A3 eprom IC31 & i6 pal16r8pc ic32
A4 - A4 eprom IC31 & i5 pal16r8pc ic32
A5 - A5 eprom IC31 & i4 pal16r8pc ic32 & 2 74LS139 IC33
A6 - A6 eprom IC31 & i3 pal16r8pc ic32 & 3 74LS139 IC33
A7 - A7 eprom IC31 & i2 pal16r8pc ic32 & 1 74LS139 IC33
A8 - A8 eprom IC31 
…
A11 - A11 eprom IC31 
A12 - A12 eprom IC31 & 14 74HC157 IC45
A13 - 11 74HC157 IC45
A14 -   5 74HC157 IC45
A15 -   2 74HC157 IC45 

D0 - D0 eprom ic31 & o12 pal16r8pc ic32 & memory adhoc board & PIO
D1 - D1 eprom ic31 & o13 pal16r8pc ic32 & memory adhoc board 3 IC43 Socket -> 11 Clock 74HC374 daughter board & PIO
D2 - D2 eprom ic31 & o14 pal16r8pc ic32 & memory adhoc board & PIO
D3 - D3 eprom ic31 & o15 pal16r8pc ic32 & memory adhoc board & PIO
D4 - D4 eprom ic31 & o16 pal16r8pc ic32 & memory adhoc board & PIO
D5 - D5 eprom ic31 & o17 pal16r8pc ic32 & memory adhoc board & PIO
D6 - D6 eprom ic31 & o18 pal16r8pc ic32 & memory adhoc board & PIO
D7 - D7 eprom ic31 & o19 pal16r8pc ic32 & memory adhoc board & PIO

MREQ 19 -> 15 74LS244 IC61
IORQ 20 -> 36 IORQ PIO IC48/49 & J7 19 & 5 (/E2) 74HC138 IC34
RD 21 -> 22 /OE EPROM & 35 RD PIO 48/49 & J7 23 & 5 74HC04 IC21 & 5 74HC00 IC35 & 5 74HC32 IC8
WR 22 -> J7 21

3 74HC157 IC45 -> 4 74HC157 IC51



IC33 74LS139
------------

A7 Z80 -> /ENA 1 74LS139 IC33 
A5 Z80 -> A0A 2 74LS139 IC33 (H = enable pio 48)
A6 Z80 -> A1A 3 74LS139 IC33 (H = enable pio 49)
/O0A 4 ?
/O1A 5 74LS139 IC33 -> /ENA PIO IC48
/O2A 6 74LS139 IC33 -> /ENA PIO IC49
/O3A 7 74LS139 IC33 -> 4 (/E1) 74LS138 IC34



IC34 74LS138 (0x60 & 0x70 IO)
-----------------------------

1 (A0) <- A2
2 (A1) <- A3
3 (A2) <- A4
4 (/E1) <- 7 74LS139 IC33
5 (/E2) <- IORQ 20 Z80
6 (E3) <- 6 74HC00 IC35

15 (O0) -> 4 74HC32 IC8                           (0x60)
14 (O1) -> 1&19 74LS244 IC3 OE edge               (0x64) IC3 Player 1
13 (O2) -> 1&19 74LS244 IC2 OE edge               (0x68) IC2 Player 2
12 (O3) -> 1&19 74LS244 IC1 OE edge               (0x6C) IC1 Not used
11 (O4) -> 1 MB IC42 & 13 J8 unpopulated          
10 (O5) -> 2 MB IC42 & 12 J8 unpopulated	       
9   (O6) -> 3 MB IC42 & 11 J8 unpopulated         
7   (O7) -> 11 /OE PAL16R8PC IC32 & 4 (SET1) 74HC74 IC20  (0x7C?)



IC32 PAL16R8PC
--------------

11 <- 7 O7 IC34



IC 31 Eprom
-----------

20 /CE & 5 74HC08 IC18 <- 6 74HC10 IC19
22 /OE <- Z80 RD



IC18 74HC08 2AND
----------------

4 <- 12 74HC10 IC19
5 <- ? (/20 CE EPROM IC 31)
6 -> 9 IC26 74HC08 

12 <- RFSH Z80 
13 <- 11 74HC00 IC35
11 -> 12 74HC32 IC8


IC19 74HC10 3NAND
-----------------

1 <- 8 74HC04 IC21 
2 <- A6 IC48 PIO 
13 <- 8 74HC00 IC35
12 -> 4 74HC08 IC18

3 <- 8 74HC04 IC21
4 <- A5 IC48 PIO
5 <- 8 74HC00 IC35
6 -> 20 /CE IC31 EPROM & 5 74HC08 IC18



IC26 74HC08
-----------

9 <- IC18 74HC08 6
10 <- RFSH Z80 
8 -> 4  74HC74 IC17

*/

#include "emu.h"
#include "cedar_magnet_plane.h"
#include "cedar_magnet_sprite.h"
#include "cedar_magnet_flop.h"

#include "efo_zsu.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/bankdev.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"

#define LOG_IC49_PIO_PB (1U << 1)
#define LOG_IC48_PIO_PB (1U << 2)
#define LOG_IC48_PIO_PA (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class cedar_magnet_state : public driver_device
{
public:
	cedar_magnet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bank0(*this, "bank0")
		, m_sub_ram_bankdev(*this, "mb_sub_ram")
		, m_sub_pal_bankdev(*this, "mb_sub_pal")
		, m_ram0(*this, "ram0")
		, m_pal_r(*this, "pal_r")
		, m_pal_g(*this, "pal_g")
		, m_pal_b(*this, "pal_b")
		, m_ic48_pio(*this, "z80pio_ic48")
		, m_ic49_pio(*this, "z80pio_ic49")
		, m_io_coin(*this, "COIN%u", 1U)
		, m_ic48_pio_pa_val(0xff)
		, m_ic48_pio_pb_val(0xff)
		, m_ic49_pio_pb_val(0xff)
		, m_address1hack(-1)
		, m_address2hack(-1)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_cedsound(*this, "cedtop")
		, m_cedplane0(*this, "cedplane0")
		, m_cedplane1(*this, "cedplane1")
		, m_cedsprite(*this, "cedsprite")
	{
	}

	void cedar_magnet(machine_config &config);

private:
	required_device<address_map_bank_device> m_bank0;
	required_device<address_map_bank_device> m_sub_ram_bankdev;
	required_device<address_map_bank_device> m_sub_pal_bankdev;

	required_shared_ptr<u8> m_ram0;
	required_shared_ptr<u8> m_pal_r;
	required_shared_ptr<u8> m_pal_g;
	required_shared_ptr<u8> m_pal_b;

	required_device<z80pio_device> m_ic48_pio;
	required_device<z80pio_device> m_ic49_pio;

	optional_ioport_array<2> m_io_coin;

	u8 ic48_pio_pa_r();
	void ic48_pio_pa_w(u8 data);

	u8 ic48_pio_pb_r();
	void ic48_pio_pb_w(u8 data);

	u8 ic49_pio_pb_r();
	void ic49_pio_pb_w(u8 data);

	// 1x range ports
	void port18_w(u8 data);
	void port19_w(u8 data);
	void port1b_w(u8 data);

	u8 port18_r();
	u8 port19_r();
	u8 port1a_r();

	// 7x range ports
	void rambank_palbank_w(u8 data);
	void palupload_w(u8 data);
	void paladdr_w(u8 data);
	u8 watchdog_r();
	u8 port7c_r();

	// other ports
	u8 other_cpu_r(offs_t offset);
	void other_cpu_w(offs_t offset, u8 data);

	u8 m_paladdr = 0;
	int m_palbank = 0;

	u8 m_ic48_pio_pa_val;
	u8 m_ic48_pio_pb_val;
	u8 m_ic49_pio_pb_val;

	void set_palette(int offset);
	void palette_r_w(offs_t offset, u8 data);
	void palette_g_w(offs_t offset, u8 data);
	void palette_b_w(offs_t offset, u8 data);

	void handle_sub_board_cpu_lines(cedar_magnet_board_interface &dev, int old_data, int data);
	INTERRUPT_GEN_MEMBER(irq);
	void kludge_protection();
	int m_address1hack;
	int m_address2hack;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;

	required_device<cedar_magnet_sound_device> m_cedsound;
	required_device<cedar_magnet_plane_device> m_cedplane0;
	required_device<cedar_magnet_plane_device> m_cedplane1;
	required_device<cedar_magnet_sprite_device> m_cedsprite;

	void cedar_bank0(address_map &map);
	void cedar_magnet_io(address_map &map);
	void cedar_magnet_mainboard_sub_pal_map(address_map &map);
	void cedar_magnet_mainboard_sub_ram_map(address_map &map);
	void cedar_magnet_map(address_map &map);
};

/***********************

  Memory maps

***********************/

void cedar_magnet_state::cedar_magnet_mainboard_sub_pal_map(address_map &map)
{
// these are 3x MOTOROLA MM2114N SRAM 4096 bit RAM (twice the size because we map bytes, but only 4 bits are used)
// these are on the master board memory sub-board
	map(0x2400, 0x27ff).ram().w(FUNC(cedar_magnet_state::palette_r_w)).share("pal_r");
	map(0x2800, 0x2bff).ram().w(FUNC(cedar_magnet_state::palette_g_w)).share("pal_g");
	map(0x3000, 0x33ff).ram().w(FUNC(cedar_magnet_state::palette_b_w)).share("pal_b");
}

void cedar_magnet_state::cedar_magnet_mainboard_sub_ram_map(address_map &map)
{
// these are 8x SIEMENS HYB 41256-15 AA - 262,144 bit DRAM (32kbytes)
// these are on the master board memory sub-board
	map(0x00000, 0x3ffff).ram().share("ram0");
}

void cedar_magnet_state::cedar_magnet_map(address_map &map)
{
	map(0x0000, 0xffff).m(m_bank0, FUNC(address_map_bank_device::amap8));
}

void cedar_magnet_state::cedar_magnet_io(address_map &map)
{
	map.global_mask(0xff);

	map(0x18, 0x18).rw(FUNC(cedar_magnet_state::port18_r), FUNC(cedar_magnet_state::port18_w));
	map(0x19, 0x19).rw(FUNC(cedar_magnet_state::port19_r), FUNC(cedar_magnet_state::port19_w));
	map(0x1a, 0x1a).r(FUNC(cedar_magnet_state::port1a_r));
	map(0x1b, 0x1b).w(FUNC(cedar_magnet_state::port1b_w));

	map(0x20, 0x23).rw(m_ic48_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x40, 0x43).rw(m_ic49_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));

	map(0x60, 0x63).rw("flop", FUNC(cedar_magnet_flop_device::read), FUNC(cedar_magnet_flop_device::write));

	map(0x64, 0x64).portr("P1_IN");
	map(0x68, 0x68).portr("P2_IN");
	map(0x6c, 0x6c).portr("TEST");

	// banking / access controls to the sub-board memory
	map(0x70, 0x70).w(FUNC(cedar_magnet_state::rambank_palbank_w));
	map(0x74, 0x74).w(FUNC(cedar_magnet_state::palupload_w));
	map(0x78, 0x78).rw(FUNC(cedar_magnet_state::watchdog_r), FUNC(cedar_magnet_state::paladdr_w));
	map(0x7c, 0x7c).r(FUNC(cedar_magnet_state::port7c_r)); // protection??

	map(0xff, 0xff).w(m_cedsound, FUNC(cedar_magnet_sound_device::sound_command_w));
}

void cedar_magnet_state::cedar_bank0(address_map &map)
{
	// Memory configuration 0
	map(0x00000, 0x0ffff).m(m_sub_ram_bankdev, FUNC(address_map_bank_device::amap8));

	// Memory configuration  1
	map(0x10000, 0x1dfff).m(m_sub_ram_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x1e000, 0x1ffff).rom().region("maincpu", 0x0000);

	// memory configuration  2
	map(0x20000, 0x2bfff).m(m_sub_ram_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x2c000, 0x2ffff).rw(FUNC(cedar_magnet_state::other_cpu_r), FUNC(cedar_magnet_state::other_cpu_w));

	// memory configuration 3
	map(0x30000, 0x31fff).rom().region("maincpu", 0x0000).mirror(0x0e000);
}


/***********************

  7x - ports
  Main board RAM sub-board

***********************/

void cedar_magnet_state::rambank_palbank_w(u8 data)
{
	// ---- --xx
	// xx = program bank
	m_sub_ram_bankdev->set_bank(data & 0x03);

	// yyy? yy-- palette bank
	m_palbank = data;
	int palbank = ((data & 0xc0) >> 6) | (data & 0x3c);
	m_sub_pal_bankdev->set_bank(palbank);
}

void cedar_magnet_state::palupload_w(u8 data)
{
	m_sub_pal_bankdev->write8(m_paladdr, data);
}

void cedar_magnet_state::paladdr_w(u8 data)
{
	m_paladdr = data;
}

u8 cedar_magnet_state::watchdog_r()
{
	// watchdog
	return 0x00;
}


/***********************

  7c - protection??

***********************/

u8 cedar_magnet_state::port7c_r()
{
	//logerror("%s: port7c_r\n", machine().describe_context());
	return 0x01;
}


/***********************

  1x ports
  Unknown, debug? protection?

***********************/

u8 cedar_magnet_state::port18_r()
{
//  logerror("%s: port18_r\n", machine().describe_context());
	return 0x00;
}

void cedar_magnet_state::port18_w(u8 data)
{
//  logerror("%s: port18_w %02x\n", machine().describe_context(), data);
}

u8 cedar_magnet_state::port19_r()
{
	u8 ret = 0x00;
//  logerror("%s: port19_r\n", machine().describe_context());

// 9496 in a,($19)
// 9498 bit 2,a

	ret |= 0x04;

	return ret;
}

u8 cedar_magnet_state::port1a_r()
{
//  logerror("%s: port1a_r\n", machine().describe_context());
	return 0x00;
}


void cedar_magnet_state::port19_w(u8 data)
{
//  logerror("%s: port19_w %02x\n", machine().describe_context(), data);
}

void cedar_magnet_state::port1b_w(u8 data)
{
//  logerror("%s: port1b_w %02x\n", machine().describe_context(), data);
}

/***********************

  Palette / Video

***********************/

void cedar_magnet_state::set_palette(int offset)
{
	m_palette->set_pen_color(offset^0xff, pal4bit(m_pal_r[offset]), pal4bit(m_pal_g[offset]), pal4bit(m_pal_b[offset]));
}

void cedar_magnet_state::palette_r_w(offs_t offset, u8 data)
{
	m_pal_r[offset] = data;
	set_palette(offset);
}

void cedar_magnet_state::palette_g_w(offs_t offset, u8 data)
{
	m_pal_g[offset] = data;
	set_palette(offset);
}

void cedar_magnet_state::palette_b_w(offs_t offset, u8 data)
{
	m_pal_b[offset] = data;
	set_palette(offset);
}

u32 cedar_magnet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	int pal = (m_palbank >> 6);

	m_cedplane1->draw(screen, bitmap, cliprect,pal);
	m_cedplane0->draw(screen, bitmap, cliprect,pal);
	m_cedsprite->draw(screen, bitmap, cliprect,pal);

	return 0;
}

void cedar_magnet_state::video_start()
{
}

/***********************

  Access to other CPUs

***********************/

u8 cedar_magnet_state::other_cpu_r(offs_t offset)
{
	int bankbit0 = (m_ic48_pio_pa_val & 0x60) >> 5;
	int plane0select = (m_ic48_pio_pa_val & 0x07) >> 0;
	int plane1select = (m_ic48_pio_pb_val & 0x07) >> 0;
	int spriteselect = (m_ic48_pio_pb_val & 0x70) >> 4;
	int soundselect = (m_ic49_pio_pb_val & 0x70) >> 4;
	int windowbank = (m_ic49_pio_pb_val & 0x0c) >> 2;
	int unk2 = (m_ic49_pio_pb_val & 0x03) >> 0;

	int cpus_accessed = 0;
	u8 ret = 0x00;

	int offset2 = offset + windowbank * 0x4000;

	if (spriteselect == 0x1)
	{
		cpus_accessed++;
		ret |= m_cedsprite->read_cpu_bus(offset2);
	}

	if (plane0select == 0x1)
	{
		cpus_accessed++;
		ret |= m_cedplane0->read_cpu_bus(offset2);
	}

	if (plane1select == 0x1)
	{
		cpus_accessed++;
		ret |= m_cedplane1->read_cpu_bus(offset2);
	}

	if (soundselect == 0x1)
	{
		cpus_accessed++;
		ret |= m_cedsound->read_cpu_bus(offset2);
		logerror("%s: reading soundselect! %04x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);
	}

	if (cpus_accessed != 1)
		logerror("%s: reading multiple CPUS!!! %04x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);

//  if ((offset==0) || (offset2 == 0xe) || (offset2 == 0xf) || (offset2 == 0x68))
//      logerror("%s: reading banked bus area %04x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);

	return ret;
}

void cedar_magnet_state::other_cpu_w(offs_t offset, u8 data)
{
	int bankbit0 = (m_ic48_pio_pa_val & 0x60) >> 5;
	int plane0select = (m_ic48_pio_pa_val & 0x07) >> 0;
	int plane1select = (m_ic48_pio_pb_val & 0x07) >> 0;
	int spriteselect = (m_ic48_pio_pb_val & 0x70) >> 4;
	int soundselect = (m_ic49_pio_pb_val & 0x70) >> 4;
	int windowbank = (m_ic49_pio_pb_val & 0x0c) >> 2;
	int unk2 = (m_ic49_pio_pb_val & 0x03) >> 0;

	int cpus_accessed = 0;

	int offset2 = offset + windowbank * 0x4000;

	if (spriteselect == 0x1)
	{
		cpus_accessed++;
		m_cedsprite->write_cpu_bus(offset2, data);
	}

	if (plane0select == 0x1)
	{
		cpus_accessed++;
		m_cedplane0->write_cpu_bus(offset2, data);
	}

	if (plane1select == 0x1)
	{
		cpus_accessed++;
		m_cedplane1->write_cpu_bus(offset2, data);
	}

	if (soundselect == 0x1)
	{
		cpus_accessed++;
		m_cedsound->write_cpu_bus(offset2, data);
	//  logerror("%s: sound cpu write %04x %02x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,data, bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);
	}

	if (cpus_accessed != 1)
		logerror("%s: writing multiple CPUS!!! %04x %02x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,data, bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);

//  if ((offset==0) || (offset2 == 0xe) || (offset2 == 0xf) || (offset2 == 0x68))
//      logerror("%s: other cpu write %04x %02x - bank bits %d %d %d %d %d %d %d\n", machine().describe_context(), offset,data, bankbit0, plane0select, plane1select, spriteselect, soundselect, windowbank, unk2);
}


void cedar_magnet_state::handle_sub_board_cpu_lines(cedar_magnet_board_interface &dev, int old_data, int data)
{
	if (old_data != data)
	{
		if (data & 0x04)
			dev.reset_assert();
		else
			dev.reset_clear();

		if (data & 0x02)
			dev.halt_clear();
		else
			dev.halt_assert();
	}
}

/***********************

  IC 48 PIO handlers
   (mapped at 0x20 / 0x22)

***********************/

u8 cedar_magnet_state::ic48_pio_pa_r() // 0x20
{
	u8 ret = m_ic48_pio_pa_val & ~0x08;

	ret |= m_io_coin[0]->read()<<3;
	if (!m_cedplane0->is_running()) ret &= ~0x01;

	// interrupt source stuff??
	ret &= ~0x10;

	LOGMASKED(LOG_IC48_PIO_PA, "%s: ic48_pio_pa_r (returning %02x)\n", machine().describe_context(), ret);
	return ret;
}

void cedar_magnet_state::ic48_pio_pa_w(u8 data) // 0x20
{
	int oldplane0select = (m_ic48_pio_pa_val & 0x07) >> 0;

	// bits 19 are set to input?
	m_ic48_pio_pa_val = data;

	// address 0x20 - pio ic48 port a
	LOGMASKED(LOG_IC48_PIO_PA, "%s: ic48_pio_pa_w %02x (memory banking etc.)\n", machine().describe_context(), data);

	LOGMASKED(LOG_IC48_PIO_PA, "output bit 0x80 %d (unused)\n", (data >> 7)&1); // A7 -> 12 J4 unpopulated
	LOGMASKED(LOG_IC48_PIO_PA, "output bit 0x40 %d (bank)\n", (data >> 6)&1); // A6 -> 2 74HC10 3NAND IC19
	LOGMASKED(LOG_IC48_PIO_PA, "output bit 0x20 %d (bank)\n", (data >> 5)&1); // A5 -> 4 74HC10 3NAND IC19
	LOGMASKED(LOG_IC48_PIO_PA, "input  bit 0x10 %d (interrupt source related?)\n", (data >> 4)&1); // 10 in // A4 <- 9 74HC74 IC20 <- input from 18 74LS244 IC61
	LOGMASKED(LOG_IC48_PIO_PA, "input  bit 0x08 %d (COIN1)\n", (data >> 3)&1); // 08 in // A3 <- 4 74HC14P (inverter) IC4 <- EDGE 21 COIN1
	LOGMASKED(LOG_IC48_PIO_PA, "output bit 0x04 %d (plane0 CPU/bus related?)\n", (data >> 2)&1); // A2 -> 45 J6
	LOGMASKED(LOG_IC48_PIO_PA, "output bit 0x02 %d (plane0 CPU/bus related?)\n", (data >> 1)&1); // A1 -> 47 J6
	LOGMASKED(LOG_IC48_PIO_PA, "input  bit 0x01 %d (plane0 CPU/bus related?)\n", (data >> 0)&1); // A0 -> 49 J6

	int bankbit0 = (m_ic48_pio_pa_val & 0x60) >> 5;
	m_bank0->set_bank(bankbit0);

	int plane0select = (m_ic48_pio_pa_val & 0x07) >> 0;

	handle_sub_board_cpu_lines(*m_cedplane0, oldplane0select, plane0select);
}


u8 cedar_magnet_state::ic48_pio_pb_r() // 0x22
{
	u8 ret = m_ic48_pio_pb_val & ~0x80;

	ret |= m_io_coin[1]->read()<<7;

	if (!m_cedsprite->is_running()) ret &= ~0x10;
	if (!m_cedplane1->is_running()) ret &= ~0x01;

	LOGMASKED(LOG_IC48_PIO_PB, "%s: ic48_pio_pb_r (returning %02x)\n", machine().describe_context(), ret);
	return ret;
}

void cedar_magnet_state::ic48_pio_pb_w(u8 data) // 0x22
{
	int oldplane1select = (m_ic48_pio_pb_val & 0x07) >> 0;
	int oldspriteselect = (m_ic48_pio_pb_val & 0x70) >> 4;

	m_ic48_pio_pb_val = data;

	LOGMASKED(LOG_IC48_PIO_PB, "%s: ic48_pio_pb_w %02x\n", machine().describe_context(), data);

	// address 0x22 - pio ic48 port b
	LOGMASKED(LOG_IC48_PIO_PB, "input  bit 0x80 %d (COIN2)\n", (data >> 7)&1); // B7 <- 2 74HC14P (inverter) IC4 <- EDGE 22 COIN2
	LOGMASKED(LOG_IC48_PIO_PB, "output bit 0x40 (J6) (sprite CPU/bus related?) %d\n", (data >> 6)&1); // B6 -> 41 J6
	LOGMASKED(LOG_IC48_PIO_PB, "output bit 0x20 (J6) (sprite CPU/bus related?) %d\n", (data >> 5)&1); // B5 -> 43 J6
	LOGMASKED(LOG_IC48_PIO_PB, "input  bit 0x10 (J6) (sprite CPU/bus related?) %d\n", (data >> 4)&1); // B4 -> 44 J6
	LOGMASKED(LOG_IC48_PIO_PB, "output bit 0x08 (Q8) %d\n", (data >> 3)&1); // B3 -> Q8 transistor
	LOGMASKED(LOG_IC48_PIO_PB, "output bit 0x04 (J6) (plane1 CPU/bus related?) %d\n", (data >> 2)&1); // B2 -> 46 J6
	LOGMASKED(LOG_IC48_PIO_PB, "output bit 0x02 (J6) (plane1 CPU/bus related?) %d\n", (data >> 1)&1); // B1 -> 48 J6
	LOGMASKED(LOG_IC48_PIO_PB, "input  bit 0x01 (J6) (plane1 CPU/bus related?) %d\n", (data >> 0)&1); // B0 -> 50 J6

	int plane1select = (m_ic48_pio_pb_val & 0x07) >> 0;
	int spriteselect = (m_ic48_pio_pb_val & 0x70) >> 4;

	handle_sub_board_cpu_lines(*m_cedplane1, oldplane1select, plane1select);
	handle_sub_board_cpu_lines(*m_cedsprite, oldspriteselect, spriteselect);
}

/***********************

  IC 49 PIO handlers
     (mapped at 0x42)

***********************/

u8 cedar_magnet_state::ic49_pio_pb_r() // 0x42
{
	u8 ret = m_ic49_pio_pb_val;

	if (!m_cedsound->is_running()) ret &= ~0x10;

	LOGMASKED(LOG_IC49_PIO_PB, "%s: ic49_pio_pb_r (returning %02x)\n", machine().describe_context(), ret);
	return ret;
}

void cedar_magnet_state::ic49_pio_pb_w(u8 data) // 0x42
{
	int oldsoundselect = (m_ic49_pio_pb_val & 0x70) >> 4;

	m_ic49_pio_pb_val = data;

	//logerror("%s: ic49_pio_pb_w %02x\n", machine().describe_context(), data);

	// address 0x42 - pio ic49 port b
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x80 %d (Q9)\n", (data >> 7)&1); // B7 -> Q9 transistor
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x40 %d (sound CPU bus related) (J3)\n", (data >> 6)&1); // B6 -> 9 J3
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x20 %d (sound CPU bus related) (J3)\n", (data >> 5)&1); // B5 -> 8 J3
	LOGMASKED(LOG_IC49_PIO_PB, "input  bit 0x10 %d (sound CPU bus related) (J3)\n", (data >> 4)&1); // B4 -> 7 J3       // input?
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x08 %d (J7)\n", (data >> 3)&1); // B3 -> 35 J7  bank bits
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x04 %d (J7)\n", (data >> 2)&1); // B2 -> 36 J7  bank bits
	// there is code to mask out both bottom bits here before load operations?
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x02 %d (IC21)\n", (data >> 1)&1); // B1 -> 3 74HC04 IC21 (set before some SPRITE cpu operations, possibly halts the blitter?)
	LOGMASKED(LOG_IC49_PIO_PB, "output bit 0x01 (LED) %d\n", (data >> 0)&1); // B0 -> LED LD1


	int soundselect = (m_ic49_pio_pb_val & 0x70) >> 4;

	handle_sub_board_cpu_lines(*m_cedsound, oldsoundselect, soundselect);
}

/***********************

  Init / Inputs / Machine

***********************/

void cedar_magnet_state::machine_start()
{
	save_item(NAME(m_paladdr));
}

void cedar_magnet_state::machine_reset()
{
	m_ic48_pio_pa_val = 0xff;

	int bankbit0 = (m_ic48_pio_pa_val & 0x60) >> 5;
	m_bank0->set_bank(bankbit0);
	m_sub_ram_bankdev->set_bank(3);
	m_sub_pal_bankdev->set_bank(0);
}


static INPUT_PORTS_START( cedar_magnet )
	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("COIN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("P1_IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2_IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("TEST")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(cedar_magnet_state::irq)
{
	kludge_protection();

	m_maincpu->set_input_line(0, HOLD_LINE);
	m_cedplane0->irq_hold();
	m_cedplane1->irq_hold();
	m_cedsprite->irq_hold();
}

void cedar_magnet_state::cedar_magnet(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 4000000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cedar_magnet_state::cedar_magnet_map);
	m_maincpu->set_addrmap(AS_IO, &cedar_magnet_state::cedar_magnet_io);
	m_maincpu->set_vblank_int("screen", FUNC(cedar_magnet_state::irq));

	ADDRESS_MAP_BANK(config, "bank0").set_map(&cedar_magnet_state::cedar_bank0).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "mb_sub_ram").set_map(&cedar_magnet_state::cedar_magnet_mainboard_sub_ram_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "mb_sub_pal").set_map(&cedar_magnet_state::cedar_magnet_mainboard_sub_pal_map).set_options(ENDIANNESS_LITTLE, 8, 8+6, 0x100);

	Z80PIO(config, m_ic48_pio, 4000000/2);
//  m_ic48_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ic48_pio->in_pa_callback().set(FUNC(cedar_magnet_state::ic48_pio_pa_r));
	m_ic48_pio->out_pa_callback().set(FUNC(cedar_magnet_state::ic48_pio_pa_w));
	m_ic48_pio->in_pb_callback().set(FUNC(cedar_magnet_state::ic48_pio_pb_r));
	m_ic48_pio->out_pb_callback().set(FUNC(cedar_magnet_state::ic48_pio_pb_w));

	Z80PIO(config, m_ic49_pio, 4000000/2);
//  m_ic49_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
//  m_ic49_pio->in_pa_callback().set(FUNC(cedar_magnet_state::ic49_pio_pa_r)); // NOT USED
//  m_ic49_pio->out_pa_callback().set(FUNC(cedar_magnet_state::ic49_pio_pa_w)); // NOT USED
	m_ic49_pio->in_pb_callback().set(FUNC(cedar_magnet_state::ic49_pio_pb_r));
	m_ic49_pio->out_pb_callback().set(FUNC(cedar_magnet_state::ic49_pio_pb_w));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-8-1, 0, 192-1);
	screen.set_screen_update(FUNC(cedar_magnet_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x400);

	CEDAR_MAGNET_SOUND(config, m_cedsound, 0);
	CEDAR_MAGNET_PLANE(config, m_cedplane0, 0);
	CEDAR_MAGNET_PLANE(config, m_cedplane1, 0);
	CEDAR_MAGNET_SPRITE(config, m_cedsprite, 0);

	CEDAR_MAGNET_FLOP(config, "flop", 0);

	config.set_perfect_quantum(m_maincpu);
}

/*
    Protection? (Time Scanner note)

    One part of the code is a weird loop checking values from port 0x7c while doing other nonsensical stuff, a flag gets set to 0xff if it fails.

    The other part is after reading the weird extra block on the disk (score / protection data at 0xea400 in the disk image*) and again a flag
    gets set to 0xff in certain conditions there's then a check after inserting a coin, these values can't be 0xff at that point, and there
    doesn't appear to be any code to reset them.

    *0xea400 is/was track 4e, side 00, sector 01 for future reference if the floppy format changes

    All games have the same code in them but at different addresses
*/
void cedar_magnet_state::kludge_protection()
{
	const int max_addr = 0x3ffff;

	if (m_address1hack == -1)
	{
		for (int i = 0; i < max_addr - 4; i++)
		{
			if ((m_ram0[i + 0] == 0x7f) && (m_ram0[i + 1] == 0xc8) && (m_ram0[i + 2] == 0x3e) && (m_ram0[i + 3] == 0xff))
			{
				m_address1hack = i + 2;
				logerror("found patch at %06x\n", i + 2);
				break;
			}
		}
	}
	else
	{
		if ((m_ram0[m_address1hack] == 0x3e) && (m_ram0[m_address1hack + 1] == 0xff)) m_ram0[m_address1hack] = 0xc9;
	}

	if (m_address2hack == -1)
	{
		for (int i = 0; i < max_addr - 4; i++)
		{
			if ((m_ram0[i + 0] == 0x10) && (m_ram0[i + 1] == 0xdd) && (m_ram0[i + 2] == 0x3e) && (m_ram0[i + 3] == 0xff))
			{
				m_address2hack = i + 2;
				logerror("found patch at %06x\n", i + 2);
				break;
			}
		}
	}
	else
	{
		if ((m_ram0[m_address2hack] == 0x3e) && (m_ram0[m_address2hack + 1] == 0xff)) m_ram0[m_address2hack] = 0xc9;
	}
}



#define BIOS_ROM \
	ROM_REGION( 0x10000, "maincpu", 0 ) \
	ROM_LOAD( "magnet-master-vid-e03.bin", 0x00000, 0x02000, CRC(86c4a4f0) SHA1(6db1a006b2e0b2a7cc9748ade881debb098b6757) )


ROM_START( cedmag )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	// no disk inserted
ROM_END

// Marked as BAD_DUMP because of the missing tracks (hence the different size).
ROM_START( mag_boob )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "boobykid.img", 0x00000, 0xde000, BAD_DUMP CRC(3196ffb4) SHA1(99732f74bb907ed6a93ed097c7b211c709d8bf85) ) // Floppy labeled "BOOBY KID CC / TUBO VERTICAL / 1 joystick, 2 pulsadores"
ROM_END

// Data read 100% consistently with multiple drives
ROM_START( mag_burn )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 ) //
	ROM_LOAD( "theburningcavern 31_3_87.img", 0x00000, 0xf0000, CRC(c95911f8) SHA1(eda3bdbbcc3e00a7da83253209e832855c2968b1) )
ROM_END

/*
    Data read 100% consistently with non-original drive (usually gives worse results)
    later tracks showed differences with original drive on each read (around 0xeef80 onwards, doesn't seem to be game data)

    weirdly there's was a single byte in an earlier track that read consistently, but in a different way for each drive
    0x2480e: 9d (non-original) vs 1d (original drive)
    1d seems to be correct as the same data is also elsewhere on the disc
*/
ROM_START( mag_day )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "adayinspace 31_3_87.img", 0x00000, 0xf0000, CRC(bc65302d) SHA1(6ace68a0b5f7a07a8f5c318c5359011074e7f2ec) )
ROM_END

/*
The following tracks/sides failed to read (bad disk)

track:68:0 (file offset:0x0cc000 - 0x0cd7ff)
track:69:0 (file offset:0x0cf000 - 0x0d07ff)
track:70:0 (file offset:0x0d2000 - 0x0d37ff)
track:71:0 (file offset:0x0d5000 - 0x0d67ff)
track:72:0 (file offset:0x0d8000 - 0x0d97ff)
track:73:0 (file offset:0x0db000 - 0x0dc7ff)
track:74:0 (file offset:0x0de000 - 0x0df7ff)
track:75:0 (file offset:0x0e1000 - 0x0e27ff)
track:76:0 (file offset:0x0e4000 - 0x0e57ff)

These areas aren't read by the code that currently loads, but other areas also didn't read consistently.

The 3 dumps in the set below contain different reads of tracks 0-67.
*/
ROM_START( mag_drac )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "drac.dsk", 0x00000, 0xf0000, BAD_DUMP CRC(2b5ca6f8) SHA1(063ea3b55bf95d05c866c0fcdb41c307c484a4f8) )
	ROM_LOAD( "drac2.dsk", 0x00000, 0xf0000, BAD_DUMP CRC(cf6c1dd2) SHA1(7adb5146b050172090556927bf6d30ba8265107a) )
	ROM_LOAD( "drac3.dsk", 0x00000, 0xf0000, BAD_DUMP CRC(7060e4a2) SHA1(b8e5437afff11d57a40c092d005d6b075819537a))
ROM_END

ROM_START( mag_exzi )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "exzisus.img", 0x00000, 0xf0000, CRC(3705e9dc) SHA1(78c8010d224f5deb202a29bd273ea7dc85ddcdb4) )
ROM_END

/*
	Track 79 side 1 (file offset 0x0ee800) would not read, but it appears to be outside of the used data
	this also has many scores stored on the disk at offset 0x01cc00, invalidating these does reset the
	score table to 'EFO 100000' scores, but then it never writes new scores?
*/
ROM_START( mag_pdak )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "paris.dsk 31_3_87.img", 0x00000, 0xf0000, BAD_DUMP CRC(2c4ee9e1) SHA1(22c2b75c16aca95ecf2199451c1bd12dd3a3844c) )
ROM_END

ROM_START( mag_time )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", 0 )
	ROM_LOAD( "timescanner.img", 0x00000, 0xf0000, CRC(214c558c) SHA1(9c71fce35acaf17ac685f77aebb1b0a930060f0b) )
ROM_END

/*
    Data after 0xd56b0 would not read consistently, however the game only appears to use the first 24 tracks (up to 0x48fff)
    as it loads once on startup, not during gameplay, and all tracks before that gave consistent reads.  There is data after this
    point but it is likely leftovers from another game / whatever was on the disk before, so for our purposes this should be fine.

    Some bullets do seem to spawn from locations where there are no enemies, but I think this is just annoying game design.
*/
ROM_START( mag_war )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "war mission wm 4_6_87.img", 0x00000, 0xf0000, CRC(7c813520) SHA1(2ba5999709a52302aa367fb46199b331421a0d56) )
ROM_END

// Data read 100% consistently with multiple drives
ROM_START( mag_wara )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "war mission wm 9_4_87.img", 0x00000, 0xf0000, CRC(6296ea6f) SHA1(c0aaf51362bfa3362ef39c3fb1e1c848b73fd780) )
ROM_END

ROM_START( mag_xain )
	BIOS_ROM

	ROM_REGION( 0x100000, "flop:disk", ROMREGION_ERASE00 )
	ROM_LOAD( "xain.img", 0x00000, 0xf0000, CRC(5647849f) SHA1(edd2f3f6359424583bf526bf4601476dc849e617) )
ROM_END


} // anonymous namespace


GAME( 1987, cedmag,   0,       cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "Magnet System",                         MACHINE_IS_BIOS_ROOT )
GAME( 1987, mag_boob, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT90, "EFO SA / Cedar", "Booby Kids (Magnet System)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Original game (Kid no Hore Hore Daisakusen) by Nichibutsu
GAME( 1987, mag_burn, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "The Burning Cavern (31/03/87)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Date on label
GAME( 1987, mag_day,  cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT90, "EFO SA / Cedar", "A Day In Space (31/03/87)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Date on label
GAME( 1987, mag_drac, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "Dracula's Castle (Magnet System)",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1987, mag_exzi, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "Exzisus (EX 1.0, Magnet System)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Original game was by Taito
GAME( 1987, mag_pdak, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "Paris Dakar (31/03/87, Spanish)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Date on label, has unemulated 'handlebar' option that can be enabled in service mode
GAME( 1987, mag_time, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT90, "EFO SA / Cedar", "Time Scanner (TS 2.0, Magnet System)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Original game was by Sega
GAME( 1987, mag_war,  cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT90, "EFO SA / Cedar", "War Mission (WM 04/06/87)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Date in program
GAME( 1987, mag_wara, mag_war, cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT90, "EFO SA / Cedar", "War Mission (WM 09/04/87)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // The '9' was handwritten over a printed letter on disk label, date not in program
GAME( 1987, mag_xain, cedmag,  cedar_magnet, cedar_magnet, cedar_magnet_state, empty_init, ROT0,  "EFO SA / Cedar", "Xain'd Sleena (SC 3.0, Magnet System)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // Original game was by Technos
