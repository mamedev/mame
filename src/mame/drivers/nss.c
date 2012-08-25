/***************************************************************************

  nss.c

  Driver file to handle emulation of the Nintendo Super System.

  R. Belmont
  Anthony Kruize
  Based on the original MESS driver by Lee Hammerton (aka Savoury Snax)

  Driver is preliminary right now.

  The memory map included below is setup in a way to make it easier to handle
  Mode 20 and Mode 21 ROMs.

  Todo (in no particular order):
    - Fix additional sound bugs
    - Emulate extra chips - superfx, dsp2, sa-1 etc.
    - Add horizontal mosaic, hi-res. interlaced etc to video emulation.
    - Fix support for Mode 7. (In Progress)
    - Handle interleaved roms (maybe even multi-part roms, but how?)
    - Add support for running at 3.58Mhz at the appropriate time.
    - I'm sure there's lots more ...

    Nintendo Super System

  There is a second processor and Menu system for selecting the games
  controlling timer etc.? which still needs emulating there are dipswitches too

***************************************************************************

Nintendo Super System Hardware Overview
Nintendo, 1992

This system is basically a Super Nintendo with a timer.
The main board has 3 slots on it and can accept up to 3 plug-in carts. The player
can choose to play any of the available games, although I'm not sure why anyone
would have wanted to pay money to play these games when the home SNES was selling as well ;-)
The control panel was also just some SNES pads mounted into the arcade machine control panel....
not very usable as-is and very cheaply presented.


PCB Layouts
-----------
NSS-01-CPU MADE IN JAPAN
(C) 1991 Nintendo
|----------------------------------------------------|
|           HA13001             AN5836               |
|     SL4          SL2  CL2          SL3             |
|-|                CL1  SL1                          |
  |         |--------------|                         |
|-|        14.31818MHz     |         SL5             |
|           |              |                         |
|   IR3P32A |         M50458          |-|  |-|  |-|  |
|J          | VC1         21.47724MHz | |  | |  | |  |
|A          |              |          | |  | |  | |  |
|M          |   CN6        |          | |  | |  | |  |
|M          |--------------|          | |  | |  | |  |
|A             |-------|   |-------|  | |  | |  | |  |
|      84256   |S-PPU1 |   |S-CPU  |  | |  | |  | |  |
|              |5C77-01|   |5A22-02|  | |  | |  | |  |
|-|            |-------|   |-------|  | |  | |  | |  |
  |                                   | |  | |  | |  |
|-|            |-------|   |-------|  | |  | |  | |  |
|      84256   |S-PPU2 |   |S-WRAM |  | |  | |  | |  |
|CN4           |5C78-01|   |LH68120|  | |  | |  | |  |
|              |-------|   |-------|  | |  | |  | |  |
|                                     | |  | |  | |  |
|                                     | |  | |  | |  |
|CN3                                  | |  | |  | |  |
|                             4MHz    | |  | |  | |  |
|                                     |-|  |-|  |-|  |
|                                    CN11 CN12  CN13 |
|                             Z84C0006               |
|CN5                                      LH5168     |
|                                                    |
|                M6M80011                            |
|                                                    |
|                               S-3520               |
|CN2                       32.678kHz   *MM1026       |
|                               5.5V    NSS-C_IC14_02|
|----------------------------------------------------|
Notes:
      (The main board has many surface mounted logic chips and transistors on the lower side of the PCB
       which are not documented here. There's also a lot of custom Nintendo parts)
      IR3P32A - Sharp IR3P32A Special Function TV Interface Circuit, Conversion of color diff sig. & lumin. to RGB (NDIP30)
      M50458  - Mitsubishi M50458-001SP On-Screen Display (OSD) Chip (NDIP32)
      HA13001 - Hitachi Dual 5.5W Power Amplifier IC
      AN5836  - Matsushita AN5836 DC Volume and Tone Control IC (SIL12)
      84256   - Fujitsu MB84256-10L 32k x8 SRAM (SOP28)
      LH5168  - Sharp LH5168N-10L 8k x8 SRAM (SOP28)
      Z84C0006- Zilog Z84C0006FEC Z80 CPU, clock input 4.000MHz (QFP44)
      M6M80011- Mitsubishi M6M80011 64 x16 Serial EEPROM (DIP8). Pinout..... 1 CS, 2 CLK, 3 DATA IN, 4 DATA OUT, 5 VSS, 6 RESET, 7 RDY, 8 VCC
      S-3520  - Seiko Epson S-3520 Real Time Clock (SOIC14)
      5.5V    - 5.5 volt supercap
      MM1026  - Mitsumi Monolithic IC MM1026BF System Reset with Battery Backup (SOIC8)
      VSync   - 60Hz
      HSync   - 15.57kHz
      *       - This IC located underneath PCB
      NSS-C_IC14_02 - 27C256 EPROM (DIP28)
      CN11/12/13    - 50 pin connectors for game carts
      CN2           - 10 pin connector
      CN3           - 13 pin connector
      CN4           - 8 pin connector
      CN5           - 7 pin connector
      CN6           - 24 pin connector for plug in custom sound module
      Custom IC's -
                   S-CPU (QFP100)
                   S-PPU1 (QFP100)
                   S-PPU2 (QFP100)
                   S-WRAM (SOP64)


Custom Sound Module (plugs in CN6)
----------------------------------

Note - This board is encased in a metal shield which is soldered together.

MITSUMI ELEC CO. LTD.
(C) 1990 Nintendo Co. Ltd.
|-----------------------|
|  CN1                  |
|            JRC2904    |---|
|  |-------|                |
|  |S-SMP  |                |
|  |       |  D6376         |
|  |-------|         51832  |
|                           |
|  |-------|                |
|  |S-DSP  |                |
|  |       |     51832      |
|  |-------|                |
|---|                       |
    |                       |
    |-----------------------|
Notes:
      JRC2904 - Japan Radio Co. JRC2904 Dual Low Power Op Amp (SOIC8)
      D6376   - NEC D6376 Audio 2-Channel 16-Bit D/A Converter (SOIC16)
      51832   - Toshiba TC51832FL-12 32k x8 SRAM (SOP28)
      CN1     - 24 pin connector to plug in custom sound module to main board
      S-SMP   - Stamped 'Nintendo S-SMP (M) SONY (C) Nintendo '89' custom sound chip (QFP80)
      S-DSP   - Stamped 'Nintendo S-DSP (M) (C) SONY '89' custom sound DSP (QFP80)

      Note - Without this PCB, the board will boot up and work, displaying the game
             selection menu, but once the game tries to load attract mode, the PCB resets.


Game Carts
----------
There are 3 types of carts. The carts have only a few components on them, including some
ROMs/sockets, a few logic chips/resistors/caps, a DIPSW8 block, one unknown DIP8 chip
(with it's surface scratched) and some solder-jumper pads to config the ROM types.
The unknown DIP8 chip is different per game also. There is a sticker on top with a 2-digit game code (I.E. MW/AT/L3 etc)

NSS-01-ROM-A
|-------------------------------------------------------|
|     SL3  CL1                                          |
|     CL3  SL1  IC1_PRG_ROM     IC3_INST_ROM       IC4  |
|CL4  SL2  SL5                                          |
|SL4  CL2  CL5                                          |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      IC1 - Program ROM
      IC2 - Instruction ROM
      IC4 - Unknown DIP8 chip

Game Name            IC1             IC1 Type    IC3            IC3 Type    Jumpers
-------------------------------------------------------------------------------------------------------
Super Mario World    NSS-MW-0_PRG    LH534J      NSS-R_IC3_MW   27C256      CL1 CL2 CL3 CL4 CL5 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 - Open
Super Tennis         NSS-ST-0        LH534J      NSS-R_IC3_ST   27C256      CL1 CL2 CL3 CL4 CL5 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 - Open
Super Soccer         NSS-R_IC1_FS    TC574000    NSS-R_IC3_FS   27C256      CL1 CL2 CL3 CL4 CL5 - Open
                                                                            SL1 SL2 SL3 SL4 SL5 - Short
-------------------------------------------------------------------------------------------------------
Note - By setting the jumpers to 'Super Soccer', the other 2 games can use standard EPROMs if required.

LH534J is a 4MBit x8 MaskROM with non-standard pinout. An adapter can be made easily to read them.

  Sharp LH534J           Common 27C040
    +--\/--+               +--\/--+
A17 |1   32| +5V       VPP |1   32| +5V
A18 |2   31| /OE       A16 |2   31| A18
A15 |3   30| NC        A15 |3   30| A17
A12 |4   29| A14       A12 |4   29| A14
 A7 |5   28| A13       A7  |5   28| A13
 A6 |6   27| A8        A6  |6   27| A8
 A5 |7   26| A9        A5  |7   26| A9
 A4 |8   25| A11       A4  |8   25| A11
 A3 |9   24| A16       A3  |9   24| OE/
 A2 |10  23| A10       A2  |10  23| A10
 A1 |11  22| /CE       A1  |11  22| CE/,PGM/
 A0 |12  21| D7        A0  |12  21| D7
 D0 |13  20| D6        D0  |13  20| D6
 D1 |14  19| D5        D1  |14  19| D5
 D2 |15  18| D4        D2  |15  18| D4
GND |16  17| D3        GND |16  17| D3
    +------+               +------+


NSS-01-ROM-B
|-------------------------------------------------------|
|  BAT1  SL3 CL1 CL2 SL5 CL6 CL7                        |
|        CL3 SL1 SL2 CL5 SL6 SL7                        |
|                 CL4 SL4                               |
|                   IC1                                 |
|                                                       |
|                                                       |
|                   IC2_PRG_ROM      IC7_INST_ROM  IC9  |
|                                                       |
|                                                       |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is populated on this board, type CR2032 3V coin battery
      IC1 - LH5168 8k x8 SRAM (DIP28)
      IC2 - Program ROM
      IC7 - Instruction ROM
      IC9 - Unknown DIP8 chip

Game Name            IC2             IC2 Type    IC7            IC7 Type    Jumpers
---------------------------------------------------------------------------------------------------------------
F-Zero               NSS-FZ-0        LH534J      NSS-R_IC3_FZ   27C256      CL1 CL2 CL3 CL4 CL5 CL6 CL7 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 SL6 SL7 - Open
---------------------------------------------------------------------------------------------------------------


NSS-01-ROM-C
|-------------------------------------------------------|
|  BAT1 CL19 SL22   IC1_SRAM                    DIPSW8  |
|       CL18 SL21                                       |
|       CL17 SL20                                       |
|       CL15 SL16   IC2_PRG_ROM-1                       |
|       SL12                                            |
|       CL13 SL14                                       |
|       CL5  SL11                                       |
|       CL6  SL10   IC3_PRG_ROM-0    IC8_INST_ROM  IC10 |
|       CL3  SL9                                        |
|       CL4  SL8                                        |
|   SL1 CL2  SL7                                        |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is not populated on this board for any games
      IC1   - 6116 2k x8 SRAM, not populated (DIP24)
      IC2/3 - Program ROM
      IC8   - Instruction ROM
      IC10  - Unknown DIP8 chip

Game Name       IC2            IC2 Type   IC3            IC3 Type    IC8            IC8 Type    Jumpers
---------------------------------------------------------------------------------------------------------------------------------------------------------
Actraiser       NSS-R_IC2_AR   TC574000   NSS-R_IC3_AR   TC574000    NSS-R_IC8_AR   27C256      CL2 CL3 CL4 CL5 CL6 CL12 CL13 CL15 CL17 CL18 CL19 - Short
                                                                                                SL1 SL7 SL8 SL9 SL10 SL11 SL12 SL14 SL16 SL20 SL21 SL22 - Open
Addams Family   NSS-R_IC2_AF   TC574000   NSS-R_IC3_AF   TC574000    NSS-R_IC8_AF   27C256      All games use the above jumper configuration.
Amazing Tennis  NSS-R_IC2_AT   TC574000   NSS-R_IC3_AT   TC574000    NSS-R_IC8_AT   27C256
Irem Skins Game NSS-R_IC2_MT   TC574000   NSS-R_IC3_MT   TC574000    NSS-R_IC8_MT   27C256
Lethal Weapon   NSS-R_IC2_L3   TC574000   NSS-R_IC3_L3   TC574000    NSS-R_IC8_L3   27C256
NCAA Basketball NSS-R_IC2_DU   TC574000   NSS-R_IC3_DU   TC574000    NSS-R_IC8_DU   27C256
Robocop 3       NSS-R_IC2_R3   TC574000   NSS-R_IC3_R3   TC574000    NSS-R_IC8_R3   27C256
---------------------------------------------------------------------------------------------------------------------------------------------------------


NSS-01-ROM-C
Sticker - NSS-X1-ROM-C (this is just a ROM-C board with a sticker over the top)
                       (the differences being the SRAM and battery are populated)
|-------------------------------------------------------|
|  BAT1 CL19 SL22   IC1_SRAM                    DIPSW8  |
|       CL18 SL21                                       |
|       CL17 SL20                                       |
|       CL15 SL16   IC2_PRG_ROM-1                       |
|       SL12                                            |
|       CL13 SL14                                       |
|       CL5  SL11                                       |
|       CL6  SL10   IC3_PRG_ROM-0    IC8_INST_ROM  IC10 |
|       CL3  SL9                                        |
|       CL4  SL8                                        |
|   SL1 CL2  SL7                                        |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is populated on this board, type CR2032 3V coin battery
      IC1   - 6116 2k x8 SRAM, populated (DIP24)
      IC2/3 - Program ROM
      IC8   - Instruction ROM
      IC10  - Unknown DIP8 chip

Game Name    IC2            IC2 Type   IC3            IC3 Type    IC8           IC8 Type    Jumpers
-----------------------------------------------------------------------------------------------------------------------------------------------------
Contra III   CONTRA_III_1   TC574000   CONTRA_III_0   TC574000    GAME1_NSSU    27C256      CL2 CL3 CL4 CL5 CL6 CL12 CL13 CL15 CL17 CL18 CL19 - Short
                                                                                            SL1 SL7 SL8 SL9 SL10 SL11 SL12 SL14 SL16 SL20 SL21 SL22 - Open
-----------------------------------------------------------------------------------------------------------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "machine/s3520cf.h"
#include "video/m50458.h"
#include "includes/snes.h"
#include "rendlay.h"


class nss_state : public snes_state
{
public:
	nss_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag),
		m_m50458(*this,"m50458"),
		m_s3520cf(*this, "s3520cf")
		{ }

	required_device<m50458_device> m_m50458;
	required_device<s3520cf_device> m_s3520cf;
	UINT8 m_wram_wp_flag;
	UINT8 *m_wram;
	UINT8 m_nmi_enable;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(spc_ram_100_r);
	DECLARE_WRITE8_MEMBER(spc_ram_100_w);
	DECLARE_READ8_MEMBER(port_00_r);
	DECLARE_WRITE8_MEMBER(port_00_w);
	DECLARE_READ8_MEMBER(ram_wp_r);
	DECLARE_WRITE8_MEMBER(ram_wp_w);
	DECLARE_WRITE8_MEMBER(rtc_osd_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
};

UINT32 nss_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	m_m50458->screen_update(screen,bitmap,cliprect);
	return 0;
}



static ADDRESS_MAP_START( snes_map, AS_PROGRAM, 8, nss_state )
	AM_RANGE(0x000000, 0x2fffff) AM_READWRITE_LEGACY(snes_r_bank1, snes_w_bank1)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE_LEGACY(snes_r_bank2, snes_w_bank2)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x400000, 0x5fffff) AM_READ_LEGACY(snes_r_bank3)						/* ROM (and reserved in Mode 20) */
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE_LEGACY(snes_r_bank4, snes_w_bank4)	/* used by Mode 20 DSP-1 */
	AM_RANGE(0x700000, 0x7dffff) AM_READWRITE_LEGACY(snes_r_bank5, snes_w_bank5)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM					/* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE_LEGACY(snes_r_bank6, snes_w_bank6)	/* Mirror and ROM */
	AM_RANGE(0xc00000, 0xffffff) AM_READWRITE_LEGACY(snes_r_bank7, snes_w_bank7)	/* Mirror and ROM */
ADDRESS_MAP_END

READ8_MEMBER(nss_state::spc_ram_100_r)
{
	device_t *device = machine().device("spc700");
	return spc_ram_r(device, offset + 0x100);
}

WRITE8_MEMBER(nss_state::spc_ram_100_w)
{
	device_t *device = machine().device("spc700");
	spc_ram_w(device, offset + 0x100, data);
}

static ADDRESS_MAP_START( spc_mem, AS_PROGRAM, 8, nss_state )
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE_LEGACY("spc700", spc_ram_r, spc_ram_w)	/* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE_LEGACY("spc700", spc_io_r, spc_io_w)	/* spc io */
	AM_RANGE(0x0100, 0xffff) AM_WRITE(spc_ram_100_w)
	AM_RANGE(0x0100, 0xffbf) AM_READ(spc_ram_100_r)
	AM_RANGE(0xffc0, 0xffff) AM_DEVREAD_LEGACY("spc700", spc_ipl_r)
ADDRESS_MAP_END

/* NSS specific */
/*
Notes of interest:

nss_smw

bp 2914 onward is a crc check with the Instruction ROM
c0fe - c0ff pointers to the checksum

bp 6b9e EEPROM write
bp 6bf9 EEPROM read
8080 - 8081 EEPROM checksummed value

8700 - 870c  EEPROM2 checksummed value (0x8707 value and'ed with 0x03)
bp 6f8d check the EEPROM2 results
870d EEPROM2 result of checksum

bp 6dce onward looks bogus, but it's probably the way it's intended to be

M50458 charset is checked at 1382, a word checksum is provided at offsets 0xffe-0xfff of the given ROM

*/
/*
Nocash info: http://nocash.emubase.de/fullsnes.htm
map
0x0000 - 0x7fff BIOS
0x8000 - 0x8fff RAM
0x9000 - 0x9fff RAM with write protection
0xa000          EEPROM Read
0xc000 - 0xdfff instruction ROM
0xe000          EEPROM Write
0xe000 - 0xffff PROM Input & Output & Program Code (protection RP5H01, used also in earlier Nintendo systems)
Data Write:
  7-5  Unknown/unused
  4    PROM Test Mode (0=Low=6bit Address, 1=High=7bit Address)
  3    PROM Clock     (0=Low, 1=High) ;increment address on 1-to-0 transition
  2-1  Unknown/unused
  0    PROM Address Reset (0=High=Reset to zero, 1=Low=No Change)

Data Read and Opcode Fetch:

  7-5  Always set (MSBs of RST Opcode)
  4    PROM Counter Out (0=High=One, 1=Low=Zero) ;PROM Address Bit5
  3    PROM Data Out    (0=High=One, 1=Low=Zero)
  2-0  Always set (LSBs of RST Opcode)

i/o
Input
0x00 Joypad
0x01 Front-Panel Buttons and Game Over Flag
  7   From SNES Port 4016h.W.Bit2 (0=Game Over Flag, 1=Normal) (Inverted!)
0x02 Coin and Service Button Inputs
  7-3 Unknown/unused (maybe the (unused) Test button hides here)
  2   Service Button (1=Pressed: Add Credit; with INST button: Config)
  1   Coin Input 2   (1=Coin inserted in coin-slot 2)
  0   Coin Input 1   (1=Coin inserted in coin-slot 1)
0x03 RTC
Output
0x00/0x80 NMI Control and RAM protect
0x01/0x81 Unknown and Slot Select
  7     Maybe SNES Joypad Enable? (0=Disable/Demo, 1=Enable/Game)
  6   Unknown/unused        (should be always 0)
  5   SNES Sound Mute       (0=Normal, 1=Mute) (for optional mute in demo mode)
  4   Unknown  ;from INST-ROM flag! (Lo/HiROM, 2-player, zapper, volume or so?)
  3-2 Slot Select        (0..2 for Slot 1..3) (mapping to both SNES and Z80)
  1     Maybe SNES CPU pause?  (cleared on deposit coin to continue) (1=Run)
  0     Maybe SNES CPU/PPU reset?   (0=Reset, 1=Run)
0x02/0x82 RTC and OSD
0x03/0x83 Unknown and LED control
  7     Layer SNES Enable?             (used by token proc, see 7A46h) SNES?
  6     Layer OSD Enable?
  5-4 Unknown/unused (should be always 0)
  3   LED Instructions (0=Off, 1=On)  ;-glows in demo (prompt for INST button)
  2   LED Game 3       (0=Off, 1=On)  ;\
  1   LED Game 2       (0=Off, 1=On)  ; blinked when enough credits inserted
  0   LED Game 1       (0=Off, 1=On)  ;/
0x84 Coin Counter Outputs
  7-2 Unknown/unused (should be always 0)
  1   Coin Counter 2 (0=No change, 1=Increment external counter)
  0   Coin Counter 1 (0=No change, 1=Increment external counter)
0x05 Unknown
0x07 SNES Watchdog / Acknowledge SNES Joypad Read Flag

SNES part:
0x4100 DSW
0x4016 bit 0 Joypad Strobe?
0x4016 bit 2 Game Over Flag



*/

READ8_MEMBER(nss_state::ram_wp_r)
{
	return m_wram[offset];
}

WRITE8_MEMBER(nss_state::ram_wp_w)
{
	if(m_wram_wp_flag)
		m_wram[offset] = data;
}

WRITE8_MEMBER(nss_state::eeprom_w)
{
/*
	x--- ----   Unknown/set     (should be always 1) (EEPROM Reset (active low)?)
	-xx- ---- Unknown/unused  (should be always 0)
	---x ----   EEPROM Clock    (0=Low=Clock, 1=High=Idle) ;(Data In/Out must be stable
	---- x---   EEPROM Data Out (0=Low=Zero, 1=High=One)   ;on raising CLK edge)
	---- -xx- Unknown/unused  (should be always 0)       ;(and updated on falling edge)
	---- ---x   EEPROM Select   (0=High=No, 1=Low=Select)
*/
	ioport("EEPROMOUT")->write(data, 0xff);
}

static ADDRESS_MAP_START( bios_map, AS_PROGRAM, 8, nss_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9fff) AM_READWRITE(ram_wp_r,ram_wp_w)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("EEPROMIN")
	AM_RANGE(0xc000, 0xdfff) AM_ROM AM_REGION("ibios_rom", 0x6000 )
	AM_RANGE(0xe000, 0xe000) AM_WRITE(eeprom_w)
ADDRESS_MAP_END

READ8_MEMBER(nss_state::port_00_r)
{
/*
	x--- ----   SNES Watchdog (0=SNES did read Joypads, 1=Didn't do so) (ack via 07h.W)
	-x-- ----   Vblank or Vsync or so       (0=What, 1=What?)
	--x- ----   Button "Joypad Button B?"   (0=Released, 1=Pressed)
	---x ----   Button "Joypad Button A"    (0=Released, 1=Pressed)
	---- x---   Button "Joypad Down"        (0=Released, 1=Pressed)
	---- -x--   Button "Joypad Up"          (0=Released, 1=Pressed)
	---- --x-   Button "Joypad Left"        (0=Released, 1=Pressed)
	---- ---x   Button "Joypad Right"       (0=Released, 1=Pressed)
*/
	UINT8 res;

	res = (machine().primary_screen->vblank() & 1) << 6;
	res|= (((ioport("SERIAL1_DATA1_H")->read() & 0x80) >> 7) << 5);
	res|= (((ioport("SERIAL1_DATA1_L")->read() & 0x80) >> 7) << 4);
	res|= (((ioport("SERIAL1_DATA1_L")->read() & 0x08) >> 3) << 3);
	res|= (((ioport("SERIAL1_DATA1_L")->read() & 0x04) >> 2) << 2);
	res|= (((ioport("SERIAL1_DATA1_L")->read() & 0x02) >> 1) << 1);
	res|= (((ioport("SERIAL1_DATA1_L")->read() & 0x01) >> 0) << 0);

	return res;
}

WRITE8_MEMBER(nss_state::port_00_w)
{
/*
	xxxx ---- Unknown/unused      (should be always 0)
 	---- x--- Maybe SNES CPU/PPU reset (usually same as Port 01h.W.Bit1)
	---- -x-- RAM at 9000h-9FFFh  (0=Disable/Protect, 1=Enable/Unlock)
	---- --x- Looks like maybe somehow NMI Related ?    ;\or one of these is PC10-style
	---- ---x Looks like NMI Enable                     ;/hardware-watchdog reload?
*/
	m_wram_wp_flag = (data & 4) >> 2;
	m_nmi_enable = data & 1;

}

WRITE8_MEMBER(nss_state::rtc_osd_w)
{
/*
	x--- ----  OSD Clock ?       (usually same as Bit6)  ;\Chip Select when Bit6=Bit7 ?
	-x-- ----  OSD Clock ?       (usually same as Bit7)  ;/
	--x- ----  OSD Data Out      (0=Low=Zero, 1=High=One)
	---x ----  OSD Special       (?)  ... or just /CS ? (or software index DC3F/DD3F?)
	---- x---  RTC /CLK          (0=Low=Clock,  1=High=Idle)              ;S-3520
	---- -x--  RTC Data Out      (0=Low=Zero,   1=High=One)
	---- --x-  RTC Direction     (0=Low=Write,  1=High=Read)
	---- ---x  RTC /CS           (0=Low/Select, 1=High/No)
*/
//	printf("%02x\n",data & 0xf);
	ioport("RTC_OSD")->write(data, 0xff);
}

static ADDRESS_MAP_START( bios_io_map, AS_IO, 8, nss_state )
	/* TODO: I think that this actually masks to 0x7? */
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port_00_r) AM_WRITE(port_00_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("FP")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM") AM_WRITE(rtc_osd_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("RTC")

	AM_RANGE(0x72, 0x72) AM_WRITE(rtc_osd_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(port_00_w)
	AM_RANGE(0x82, 0x82) AM_WRITE(rtc_osd_w)
	AM_RANGE(0xea, 0xea) AM_WRITE(rtc_osd_w)
ADDRESS_MAP_END

/* Mitsubishi M6M80011 */
static const eeprom_interface nss_eeprom_intf =
{
	8,				/* address bits */
	16,				/* data bits */
	"10101000",		/*  read command */
	"10100100",		/* write command */
	0,				/* erase command */
	"10100000",		/*  lock command */
	"10100011"		/* unlock command*/
	/* "10101001" TODO: status output? */
};


static MACHINE_START( nss )
{
	nss_state *state = machine.driver_data<nss_state>();

	MACHINE_START_CALL(snes);

	state->m_wram = auto_alloc_array_clear(machine, UINT8, 0x1000);
}

static INPUT_PORTS_START( snes )
	PORT_START("SYSTEM")
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("FP")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Game Over Flag, TODO
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Restart Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Page Up Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Page Down Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Instructions Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Game 3 Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Game 2 Button")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Game 1 Button")

	PORT_START("EEPROMIN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // EEPROM Ready
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)

	PORT_START("RTC_OSD")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", m50458_device, set_clock_line)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", m50458_device, write_bit)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", m50458_device, set_cs_line)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", s3520cf_device, set_clock_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", s3520cf_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", s3520cf_device, set_dir_line)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", s3520cf_device, set_cs_line)

	PORT_START("RTC")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("s3520cf", s3520cf_device, read_bit)

	PORT_START("SERIAL1_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_START("SERIAL1_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("SERIAL2_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_START("SERIAL2_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("SERIAL1_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL1_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL2_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard )  )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Time limit per level?" ) // taken from the scan of nss_adam
	PORT_DIPSETTING(    0x10, "104 sec." )
	PORT_DIPSETTING(    0x20, "112 sec." )
	PORT_DIPSETTING(    0x00, "120 sec." )
	PORT_DIPSETTING(    0x30, "? sec." )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


static MACHINE_CONFIG_START( snes, nss_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", _5A22, 3580000*6)	/* 2.68Mhz, also 3.58Mhz */
	MCFG_CPU_PROGRAM_MAP(snes_map)

	MCFG_CPU_ADD("soundcpu", SPC700, 2048000/2)	/* 2.048 Mhz, but internal divider */
	MCFG_CPU_PROGRAM_MAP(spc_mem)

//  MCFG_QUANTUM_TIME(attotime::from_hz(24000))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_MACHINE_START( snes )
	MCFG_MACHINE_RESET( snes )

	/* video hardware */
	MCFG_VIDEO_START( snes )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE_STATIC( snes )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_CONFIG_END

static INTERRUPT_GEN ( nss_vblank_irq )
{
	nss_state *state = device->machine().driver_data<nss_state>();

	if(state->m_nmi_enable)
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_DERIVED( nss, snes )

	MCFG_CPU_ADD("bios", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(bios_map)
	MCFG_CPU_IO_MAP(bios_io_map)
	MCFG_CPU_VBLANK_INT("screen", nss_vblank_irq)

	MCFG_M50458_ADD("m50458",4000000) /* TODO: clock */
	MCFG_S3520CF_ADD("s3520cf") /* RTC */

	/* TODO: the screen should actually superimpose, but for the time being let's just separate outputs for now */
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("osd", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(24*12+22, 12*18+22)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*12-1, 0*8, 12*18-1)
	MCFG_SCREEN_UPDATE_DRIVER(nss_state,screen_update)

	MCFG_EEPROM_ADD("eeprom", nss_eeprom_intf)

	MCFG_MACHINE_START( nss )
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define NSS_BIOS \
	ROM_REGION(0x100,           "user5", 0)		/* IPL ROM */ \
	ROM_LOAD("spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) \
	ROM_REGION(0x10000,           "addons", ROMREGION_ERASE00)		/* add-on chip ROMs (DSP1 will be needed if we dump the NSS version of Super Mario Kart)*/\
	ROM_LOAD( "dsp1b.bin", SNES_DSP1B_OFFSET, 0x002800, CRC(453557e0) SHA1(3a218b0e4572a8eba6d0121b17fdac9529609220) ) \
	ROM_REGION(0x20000,         "bios",  0)		/* Bios CPU (what is it?) */ \
	ROM_LOAD("nss-c.dat"  , 0x00000, 0x8000, CRC(a8e202b3) SHA1(b7afcfe4f5cf15df53452dc04be81929ced1efb2) )	/* bios */ \
	ROM_LOAD("nss-ic14.02", 0x10000, 0x8000, CRC(e06cb58f) SHA1(62f507e91a2797919a78d627af53f029c7d81477) )	/* bios */ \
	ROM_REGION( 0x2000, "dspprg", ROMREGION_ERASEFF) \
	ROM_REGION( 0x800, "dspdata", ROMREGION_ERASEFF)



ROM_START( nss )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", ROMREGION_ERASEFF )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", ROMREGION_ERASEFF )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
ROM_END

ROM_START( nss_actr )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "act-rais.ic3", 0x00000, 0x80000, CRC(c9f788c2) SHA1(fba2331fd5bcbe51d74115528fd3a9becf072e8d) )
	ROM_LOAD( "act-rais.ic2", 0x80000, 0x80000, CRC(4df9cc63) SHA1(3e98d9693d60d125a1257ba79701f27bda688261) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "act-rais.ic8", 0x0000, 0x8000, CRC(08b38ce6) SHA1(4cbb7fd28d98ffef0f17747201625883af954e3a) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
    ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(4b74ac55) SHA1(51ea71b06367b4956a4b737385e2d4d15bd43980) )
ROM_END

ROM_START( nss_con3 )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "contra3.ic3", 0x00000, 0x80000, CRC(33b03501) SHA1(c7f4835d5ec4983e487b00f0b4c49fede2f03b9c) )
	ROM_LOAD( "contra3.ic2", 0x80000, 0x80000, CRC(2f3e3b5b) SHA1(0186b92f022701f6ae29984252e6d346acf6550b) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "contra3.ic8", 0x0000, 0x8000, CRC(0fbfa23b) SHA1(e7a1a78a58c64297e7b9623350ec57aed8035a4f) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, NO_DUMP )
ROM_END

ROM_START( nss_adam )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "addams.ic3", 0x00000, 0x80000, CRC(44643930) SHA1(a45204b2eb13c6befca30d130061b5b8ba054270) )
	ROM_LOAD( "addams.ic2", 0x80000, 0x80000, CRC(6196adcf) SHA1(a450f278a37d5822f607aa3631831a461e8b147e) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "addams.ic8", 0x0000, 0x8000, CRC(57c7f72c) SHA1(2e3642b4b5438f6c535d6d1eb668e1663062cf78) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(154d10c2) SHA1(6829e149c341b753ee9bc72055c0634db4e81884) )
ROM_END

ROM_START( nss_aten )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "amtennis.ic3", 0x00000, 0x80000, CRC(aeabaf2a) SHA1(b355e0a322b57454e767785a49c14d4c7f492488) )
	ROM_LOAD( "amtennis.ic2", 0x80000, 0x80000, CRC(7738c5f2) SHA1(eb0089e9724c7b3834d9f6c47b92f5a1bb26fc77) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "amtennis.ic8", 0x0000, 0x8000, CRC(d2cd3926) SHA1(49fc253b1b9497ef1374c7db0bd72c163ffb07e7) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010,CRC(3e640fa2) SHA1(ac530610a9d4979f070d5f57dfd4886c530aa20f) )
ROM_END

ROM_START( nss_rob3 )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "robocop3.ic3", 0x00000, 0x80000, CRC(60916c42) SHA1(462d9645210a58bfd5204bd209eae2cdadb4493e) )
	ROM_LOAD( "robocop3.ic2", 0x80000, 0x80000, CRC(a94e1b56) SHA1(7403d70504310ad5949a3b45b4a1e71e7d2bce77) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "robocop3.ic8", 0x0000, 0x8000, CRC(90d13c51) SHA1(6751dab14b7d178350ac333f07dd2c3852e4ae23) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, NO_DUMP )
ROM_END

ROM_START( nss_ncaa )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "ncaa.ic3", 0x00000, 0x80000, CRC(ef49ad8c) SHA1(4c40f3746b995b53f006434b9ccec06d8fe16e1f) )
	ROM_LOAD( "ncaa.ic2", 0x80000, 0x80000, CRC(83ef6936) SHA1(8e0f38c763861e33684c6ddb742385b0522af78a) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "ncaa.ic8", 0x0000, 0x8000, CRC(b9fa28d5) SHA1(bc538bcff5c19eae4becc6582b5c111d287b76fa) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(a2e9ad5b) SHA1(a41f82451fc185f8e989a0d4f38700dc7813bb50) )
ROM_END

ROM_START( nss_skin )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "skins.ic3", 0x00000, 0x80000, CRC(ee1bb84d) SHA1(549ad9319e94a5d75cd4af017e63ea93ab407c87) )
	ROM_LOAD( "skins.ic2", 0x80000, 0x80000, CRC(365fd19e) SHA1(f60d7ac39fe83fb98730e73fbef410c90a4ff35b) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "skins.ic8", 0x0000, 0x8000, CRC(9f33d5ce) SHA1(4d279ad3665bd94c7ca9cb2778572bed42c5b298) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(86f8cd1d) SHA1(d567d194058568f4ae32b7726e433918b06bca54) )
ROM_END

ROM_START( nss_lwep )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-lw.ic3", 0x00000, 0x80000, CRC(32564666) SHA1(bf371218fa303ce95eab09fb6017a522071dcd7e) )
	ROM_LOAD( "nss-lw.ic2", 0x80000, 0x80000, CRC(86365042) SHA1(f818024c6f858fd2780396b6c83d3a37a97fa08a) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "nss-lw.ic8", 0x0000, 0x8000, CRC(1acc1d5d) SHA1(4c8b100ac5847915aaf3b5bfbcb4f632606c97de) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(e9755c14) SHA1(d8dbebf3536dcbd18c50ba11a6b729dc7085f74b) )
ROM_END

ROM_START( nss_ssoc )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "s-soccer.ic1", 0x00000, 0x80000,  CRC(70b7f50e) SHA1(92856118528995e3a0b7d22340d440bef5fd61ac) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "s-soccer.ic3", 0x0000, 0x8000, CRC(c09211c3) SHA1(b274a57f93ae0a8774664df3d3615fb7dbecfa2e) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, NO_DUMP )
ROM_END

ROM_START( nss_smw )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-mw-0_prg.ic1", 0x000000, 0x80000, CRC(c46766f2) SHA1(06a6efc246c6fdb83efab1d402d61d2179a84494) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "mw.ic3", 0x0000, 0x8000, CRC(f2c5466e) SHA1(e116f01342fcf359498ed8750741c139093b1fb2) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, NO_DUMP )
ROM_END

ROM_START( nss_fzer )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-fz-0.ic2", 0x000000, 0x100000, CRC(e9b3cdf1) SHA1(ab616eecd292b94ca74c55446bddd23e9dc3e3bb) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "fz.ic7", 0x0000, 0x8000, CRC(48ae570d) SHA1(934f9fec47dcf9e49936388968d2db50c69950da) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, CRC(9650a7d0) SHA1(59d57ab2720cff3a24105a7250560c41def45acc) )
ROM_END

ROM_START( nss_sten )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-st-0.ic1", 0x000000, 0x100000, CRC(f131611f) SHA1(0797936e1fc9e705cd7e029097fc013a58e69002) )

	/* instruction / data rom for bios */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "st.ic3", 0x0000, 0x8000, CRC(8880596e) SHA1(ec6d68fc2f51f7d94f496cd72cf898db65324542) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x000000, 0x000010, NO_DUMP )
ROM_END



GAME( 199?, nss,       0,     nss,      snes, snes_state,    snes,    ROT0, "Nintendo",                    "Nintendo Super System BIOS", GAME_IS_BIOS_ROOT )
GAME( 1992, nss_actr,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Enix",                        "Act Raiser (Nintendo Super System)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // sound sometimes dies, timing issues
GAME( 1992, nss_adam,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Ocean",                       "The Addams Family (Nintendo Super System)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1992, nss_aten,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Absolute Entertainment Inc.", "David Crane's Amazing Tennis (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1992, nss_con3,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Konami",                      "Contra 3: The Alien Wars (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1992, nss_lwep,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Ocean",                       "Lethal Weapon (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1992, nss_ncaa,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Sculptured Software Inc.",    "NCAA Basketball (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1992, nss_rob3,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Ocean",                       "Robocop 3 (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) // any sprite minus Robocop is missing
GAME( 1992, nss_skin,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Irem",                        "Skins Game (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) // gfx issue caused by timing at start-up
GAME( 1992, nss_ssoc,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Human Inc.",                  "Super Soccer (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1991, nss_smw,   nss,   nss,      snes, snes_state,    snes,    ROT0, "Nintendo",                    "Super Mario World (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1991, nss_fzer,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Nintendo",                    "F-Zero (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1991, nss_sten,  nss,   nss,      snes, snes_state,    snes,    ROT0, "Nintendo",                    "Super Tennis (Nintendo Super System)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
