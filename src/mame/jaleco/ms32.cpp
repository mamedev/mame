// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
/* Jaleco MegaSystem 32 (Preliminary Driver)

 - hardware tests are needed to establish how the mixing really works (and interrupt source etc.)

Used by Jaleco in the Mid-90's this system, based on the V70 processor consisted
of a two board set up, the first a standard mainboard and the second a 'cartridge'

The actual Mega System 32 PCB mobo only outputs mono sound.  There is a connector on
the PCB for a second speaker but it is still only in mono (no stereo effects in the music).

-- Hardware Information (from Guru) --

MS32 Motherboard
----------------

PCB ID  : MB-93140A EB91022-20079-1
CPU     : NEC D70632GD-20 (V70)
SOUND   : Z80, YMF271, YAC513
OSC     : 48.000MHz, 16.9344MHz, 40.000MHz
RAM     : Cypress CY7C199-25 (x10)
          Sharp LH528256-70 (x5)
          Sharp LH5168D-10 (x1)
          OKI M511664-80 (x8)
DIPs    : 8 position (x3)
OTHER   : 5.5v battery
          Some PALs
          2 pin connector for right speaker (sound out is STEREO)

          Custom chips:
                       JALECO SS91022-01 (208 PIN PQFP)
                       JALECO SS91022-02 (100 PIN PQFP)
                       JALECO SS91022-03 (176 PIN PQFP) *
                       JALECO SS91022-05 (120 PIN PQFP) *
                       JALECO SS91022-07 (208 PIN PQFP)
                       JALECO GS91022-01 (120 PIN PQFP)
                       JALECO GS91022-02 (160 PIN PQFP)
                       JALECO GS91022-03 (100 PIN PQFP)
                       JALECO GS91022-04 (100 PIN PQFP) *

ROMs:     None

Chips marked * also appear on a non-megasystem 32 tetris 2 plus board

MS32 Cartridge
--------------

Game Roms + Custom Chip

The Custom chip provides the encryption:

Desert War       - Custom chip: JALECO SS91022-10 9513EV 367821 06441
Game Paradise    - Custom chip: JALECO SS91022-10 9515EV 420201 06441
Gratia (set 2)   - Custom chip: JALECO SS91022-10 9513EV 370121 06441
Tetris Plus 2    - Custom chip: JALECO SS91022-10 9513EV 370121 06441
Best Bout Boxing - Custom chip: JALECO SS92046-01 9338EV 436091 06441
H.Quiz Nettou    - Custom chip: JALECO SS92046-01 9338EV 436091 06441
PK Soccer V2     - Custom chip: JALECO SS92046-01 9338EV 436091 06441
Tetris Plus      - Custom chip: JALECO SS92046-01 9412EV 450891 06441
Angel Kiss       - Custom chip: JALECO SS92047-01 9423EV 450891 06441
Gratia (set 1)   - Custom chip: JALECO SS92047-01 9423EV 450891 06441
kirarast         - Custom chip: JALECO SS92047-01 9425EV 367821 06441
P47-Aces         - Custom chip: JALECO SS92048-01 9410EV 436091 06441

Custom chips are 144 pin PQFP, some times mounted on a small plug-in board silkscreened SE93139 EB91022-30056
Other examples are surface mounted directly the to ROM cart.

others are unknown

Notes
-----

Some of the roms for each game are encrypted:
  16x16x8 'Scroll' Tiles (Non-Roz BG Layer)
  8x8x8 'Ascii' Tiles (FG Layer)

The only difference between the two Gratia sets are the encrypted ROMs in each set (they use
different custom chips). The program ROMs are the same, as is all non encrypted graphics data.
It's been verified that when the encrypted data is decrypted with it's respective algorithms
the data in both sets match 100%


ToDo / Notes
------------

Z80 + Sound Bits

Priorities (code in tetrisp2.cpp doesn't use all of the priority ram.. and doesn't work here)
 - some games require completely reversed list processing!

Dip switches/inputs in t2m32 and f1superb
some games (hayaosi2) don't seem to have service mode even if it's listed among the dips
service mode is still accessible through F1 though

Fix Anything Else (Palette etc.)


Not sure about the main "global brightness" control register, I don't think it can make the palette
completely black because of kirarast attract mode, so I'm making it cut by 50% at most.
 - brightness control also breaks other games in various places, eg gametngk everything going dark
   when bomb is used, p47 aces intro?

gametngk seems to need some kind of shadow sprites but the only difference in the sprite attributes is one of the
    priority bits, forcing sprites of that priority to be shadows doesn't work
tetrisp needs shadows as well, see the game selection screen.

The above might be related to the second "global brightness" control register, which is 000000 in all games
except gametngk, tetrisp, tp2m32 and gratia.

horizontal position of tx and bg tilemaps is off by 1 pixel in some games

bbbxing: some sprite/roz/bg alignment issues

gratia: at the beginning of a level it shows the level name in the bottom right corner, scrolling it up
    and making the score display scroll out of the screen. Is this correct or should there be a raster
    effect keeping the score on screen? And why didn't they just use sprites to do that?

gratia: the 3d sky shown at the beginning of the game has a black gap near the end. It would not be visible
    if I made the "global brightness" register cut to 100% instead of 50%. Mmmm...

gratia: the 3d sky seems to be the only place needed the "wrap" parameter to draw_roz to be set. All other
    games work fine with it not set, and there are many places where it definitely must not be set.

gratia: at the beginning of the game, before the sky appears, the city background appears for
    an instant. Missing layer enable register?

background color: pen 0 is correct for gametngk, but wrong for f1superb. Maybe it depends on the layer
    priority order?

roz layer wrapping: currently it's always ON, breaking places where it gets very small so it gets
    repeated on the screen (p47aces, kirarast, bbbxing, gametngk need it OFF).
    gratia and desertwr need it ON.

there are sprite lag issues - sprites should be framebuffered

missing clipping window effect in gametngk intro


Not Working Games
-----------------

f1superb - the road is always rendered as straight.
         - the game has a road layer and extra roms for it
         - there is an unknown maths DSP for protection

Jaleco Megasystem 32 Game List - thanks to Yasuhiro
---------------------------------------------------

P-47 Aces (p47aces)
Game Tengoku / Game Paradise (gametngk)
Tetris Plus (tetrisp)
Tetris Plus 2 (tp2m32)
Best Bout Boxing (bbbxing)
Wangan Sensou / Desert War (desertwr)
Second Earth Gratia (92047-01 version) (gratia)
Second Earth Gratia (91022-10 version) (gratiaa)
F-1 Super Battle (f1superb)

Idol Janshi Su-Chi-Pi 2 (suchie2)
Ryuusei Janshi Kirara Star (kirarast)
Mahjong Angel Kiss
Vs. Janshi Brand New Stars


Hayaoshi Quiz Nettou Namahousou ( hayaosi3 )
Hayaoshi Quiz Grand Champion Taikai (hayaosi2)

Not Dumped:

Super Strong Warriors
Shutokou Red Zone / Super Circuit Red Zone

NOTE: Several games use the ROM cart board number for the EPROM numbers.  Though several
      use MB93166 for the MB-94166 cart

************************************************************************************

Notes from Charles MacDonald

----------------------------------------------------------------------------
 Z80 communication
 ----------------------------------------------------------------------------

 The system has two 8-bit registers which store bytes going from the Z80
 to the V70 and vice-versa.

 V70 side

 $FC800000 : Writes load the Z80 sound latch and trigger a NMI.

             Reads return D31-D16 = open bus, D15-D0 = $FFFF.

 $FD000000 : Reads return D31-D16 = open bus, D15-D8 = $FF, and
             D7-D0 =  V70 sound latch, inverted.

             Writes halt the system.

 Z80 side

     $3F10 : Reads return the contents of the Z80 sound latch, inverted.
             Writes load the V70 sound latch.

 To handle the inversion of sound latch data, both CPUs should invert the
 data read from their respective read addresses.

 *** Does NMI stay low such that further NMIs can't occur until it's ACK'd?

     Well, reading 3F10-3F1F allows further NMIs which are otherwise masked.

     Is /NMI line really physically held low during this time?
     Or is there just a flip-flop that remains set until read, which
     gates NMI?

 *** Does $3F10 cause interrupt on V60 side when accessed? Or $3F20?
     Could 3F20 be a V70-side interrupt request clear register?

 ----------------------------------------------------------------------------
 Sound reset register
 ----------------------------------------------------------------------------

 Writing a '1' to bit 0 of $FCE00038 temporarily pulses the Z80 /RESET pin
 low for approximately one second, at which point it goes high again and the
 Z80 resumes operation.

 Setting this bit does *not* keep the Z80 reset for the duration that
 it is set. It's function is that of a trigger for an automatically timed
 reset pulse.

 *** Measure /RESET pulse width in units of Z80 clocks.

 ----------------------------------------------------------------------------
 V70 memory map
 ----------------------------------------------------------------------------

 Overview

 The hardware maps memory and other devices to a 64MB chunk that is
 repeatedly mirrored throughout the last 1GB of the address space.

 The system is set up so the V70 is halted when it accesses an unused
 memory address or accesses it in an unintended way (reading write-only
 locations, writing to read-only locations). Shortly thereafter the
 watchdog resets the system due to inactivity.

 The V70 data bus is 32-bit but is connected to a mix of 8, 16, and 32-bit
 hardware. The undriven data bus bits tend to float high, though my
 board had a lot of extra pull-up resistors someone added in one of
 the expansion sockets. I'll try to give approximate garbage values read
 from these locations when possible.

 V70 memory map

 C0000000-FBFFFFFF : Mirror of FC000000-FFFFFFFF

 Range               Acc  Repeat  Size  Width Description

 FC000000-FC1FFFFF : R/W : 32K  : 8K   : 8  : NVRAM
 FC600000-FC7FFFFF : R/W : ---  : ---  : -- : Unused (return $FFFFFFFF)
 FC800000-FC9FFFFF : R/W : 4b   :      : 16 : Z80 sound latch (out)
 FCC00000-FCDFFFFF : R/W : 32b  :      : 32 : I/O area
 FCE00000-FCFFFFFF : W/O : 8K   : 4K   : 16 : Video registers
 FD000000-FD03FFFF : R/O : 4b   :      : 16 : Z80 sound latch (in)
 FD180000-FD1BFFFF : R/W : 32K  : 8K   : 8  : Priority RAM
 FD400000-FD5FFFFF : R/W : 256K : 128K : 16 : Color RAM
 FE000000-FE1FFFFF : R/W : 128K : 64K  : 16 : Rotate RAM
 FE200000-FE3FFFFF : R/W : 8K   : 4K   : 16 : Line RAM
 FE800000-FE9FFFFF : R/W : 256K : 128K : 16 : Object RAM
 FEC00000-FEDFFFFF : R/W : 64K  : 32K  : 16 : ASCII RAM / Scroll RAM
 FEE00000-FEEFFFFF : R/W : 128K : 128K : 32 : Work RAM
 FF000000-FFFFFFFF : R/O : 2MB  : 2MB  : 32 : Program ROM

 For example, the object RAM is 128Kx16, mapped to D15-D0 of each word.
 This corresponds to a 256K space (128K x 32-bits, 16 of which are used)
 that repeats every 256K within FE800000-FE9FFFFF.

 1.) Data written to the LSB is stored inverted in $3F10 and triggers NMI.
     Writing to D15-D8 does nothing and value read is $FF.
     Writing to D31-D16 resets the machine, values read are open bus (opcodes).

 All items listed are repeatedly mirrored throughout the memory ranges
 they are assigned to.

 This is the memory map for a Desert War boardset. Other games can add
 additional hardware on the ROM board which take up memory ranges not listed
 here. Consider it to be the memory map for a stock Mega System 32 mainboard.

 ----------------------------------------------------------------------------
 I/O ports
 ----------------------------------------------------------------------------

 The I/O area consists of 16 word locations that are mirrored repeatedly
 throughout the range they are mapped to:

 FCC00000 : ?
 FCC00004 : Player 1, 2 and control panel inputs
 FCC00008 : ?
 FCC0000C : ?
 FCC00010 : DIP switch inputs
 FCC00014 : ?
 FCC00018 : ?
 FCC0001C : ?

 Input details

 FCC00004 : ---- ---- ---- ---- ---- ---- 4321 rldu : 1P buttons, joystick
          : ---- ---- ---- ---- 4321 rldu ---- ---- : 2P buttons, joystick
          : ---- ---- ---- --21 ---- ---- ---- ---- : 2P coin, 1P coin
          : ---- ---- ---- ts-- ---- ---- ---- ---- : Test, service
          : ---- ---- --21 ---- ---- ---- ---- ---- : 2P start, 1P start

 * All inputs are active-high (1= switch released, 0= switch pressed)

 * When the TILT input is asserted, the system is reset. This continues
   until TILT is released. The state of TILT cannot be read.

 FCC00010 : ---- ---- ---- ---- ---- ---- 1234 5678 : DIP SW2 #1-8
          : ---- ---- ---- ---- 1234 5678 ---- ---- : DIP SW1 #1-8
          : ---- ---- 1234 5678 ---- ---- ---- ---- : DIP SW3 #1-8

 * All inputs are active-low (1= switch OFF, 0= switch ON)


 ----------------------------------------------------------------------------
 System and video registers
 ----------------------------------------------------------------------------

 This area is 8K long and repeats every 8K. All registers are write-only
 and are mapped to D15-D0 of each word.

 $FCE00000 : Screen mode control

 D0 : Dot clock control (1= 24 KHz?, 0= 15 KHz)

 $FCE00004 : Horizontal timing
 $FCE00008 : Horizontal timing
 $FCE0000C : Horizontal timing
 $FCE00010 : Horizontal viewport start
 $FCE00014 : Frame height
 $FCE00018 : Display height
 $FCE0001C : Horizontal positioning
 $FCE00020 : Fine positioning adjust

 $FCE00045 : IRQ acknowledge
 $FCE00038 : Sound CPU reset
 $FCE00050 : Watchdog reset
 $FCE006xx : ROZ

 $FCE00A00 : Text layer horizontal scroll #1
 $FCE00A04 : Text layer vertical scroll #1
 $FCE00A08 : Text layer horizontal scroll #2
 $FCE00A0C : Text layer vertical scroll #2

 $FCE00A2x : BG layer
 $FCE00A7C : Layer related
 $FCE00Exx : Coin meter + lockout

 ----------------------------------------------------------------------------
 NVRAM
 ----------------------------------------------------------------------------

 NVRAM is 8K, occupying D7-D0 of each word. It is mirrored every 8K-words
 (32K bytes) in memory.

 Remaining data bits return $FFFFF4xx.

 The NVRAM consists of a low-power 8K SRAM connected to a .1F capacitor for
 short-term data retention and a CR2032 lithium battery for long-term
 retention. It also has a write inhibit circuit to protect RAM from spurious
 writes when the voltage drops low enough to trigger a system reset.
 During normal operation the write protection is transparent to the
 programmer and the SRAM can be accessed normally.

 ----------------------------------------------------------------------------
 Priority RAM
 ----------------------------------------------------------------------------

 Priority RAM is 8K, occupying D7-D0 of each word. It is mirrored
 every 8K-words (32K bytes) in memory.

 Remaining data bits return $00FFFFxx.

 Note that the priority RAM chip is actually 32K. The upper address lines
 are tied low or high, so perhaps priority RAM is banked.

 ----------------------------------------------------------------------------
 Color RAM
 ----------------------------------------------------------------------------

 Color RAM is implemented with three 32Kx8 SRAMs. Every eight-byte area
 within color RAM addresses one location in color RAM. The red and green
 color RAMs are connected in parallel to D15-D0 respectively for even words,
 and the blue color RAM is connected to D7-D0 for odd words:

        MSB                                  LSB
 +$00 : ---- ---- ---- ---- rrrr rrrr gggg gggg : Red, green components
 +$04 : ---- ---- ---- ---- ---- ---- bbbb bbbb : Blue component

 - = Bit isn't used. Usually returns '1'.

 The color RAM area is 256K in size (32K entries x 8 bytes per entry) and
 is mirrored every 256K bytes in memory.

 ----------------------------------------------------------------------------
 Rotate RAM
 ----------------------------------------------------------------------------

 Rotate RAM is 64K, occupying D15-D0 of each word. It is mirrored every
 64K-words (128K bytes) in memory.

 Remaining data bits return $00FFxxxx or $0000xxxx randomly.

 ----------------------------------------------------------------------------
 Object RAM
 ----------------------------------------------------------------------------

 Object RAM is 128K, occupying D15-D0 of each word. It is mirrored every
 128K-words (256K bytes) in memory.

 Remaining data bits return $FFFFxxxx.

 ----------------------------------------------------------------------------
 ASCII / Scroll RAM
 ----------------------------------------------------------------------------

 ASCII / Scroll RAM is 32K, occupying D15-D0 of each word. It is mirrored
 every 64K-words (128K bytes) in memory.

 Remaining data bits return $0000xxxx.

 ----------------------------------------------------------------------------
 Work RAM
 ----------------------------------------------------------------------------

 Work RAM is 128K, occupying D31-D0 of each word. It is mirrored every 128K
 bytes in memory.

 ----------------------------------------------------------------------------
 Program ROM
 ----------------------------------------------------------------------------

 Program ROM is 512K, occupying D31-D0 of each word. It is mirrored every
 512K bytes in memory.

 ----------------------------------------------------------------------------
 CPU information
 ----------------------------------------------------------------------------

 Main CPU:  NEC uPD70632GD-20 (200-pin PQFP, 20 MHz)

 * The value of PIR for this particular chip is $00007007.

 * The instruction MOV.D with a register operand uses the register
   pair "rn:rn+1" as the source data. R31 is a special case; the second
   register of the pair is still R31 rather than wrapping to R0.

   mov.d r2, [r0]       ; Write pair R2:R3 to [R0]
   mov.d r3, [r0]       ; Write pair R3:R4 to [R0]
   mov.d r31, [r0]      ; Write pair R31:R31 to [R0]

   Using the immediate or quick immediate addressing mode for the source
   operand causes an Addressing Mode exception, just like the uPD70616.

 Sound CPU: Zilog Z80840008PSC (40-pin DIP, 8 MHz)

 * NMOS type. Undocumented instruction "out (c), 0" functions normally.

*/

/********** BITS & PIECES **********/

#include "emu.h"
#include "ms32.h"
#include "jalcrpt.h"

#include "mahjong.h"

#include "cpu/z80/z80.h"
#include "cpu/v60/v60.h"
#include "speaker.h"

#include "f1superb.lh"


/********** READ INPUTS **********/

ioport_value ms32_state::mahjong_ctrl_r()
{
	u32 mj_input = 0xff;

	for (int i = 0; i < m_io_mj.size(); i++)
	{
		if (BIT(m_mahjong_input_select[0], i))
			mj_input &= m_io_mj[i]->read();
	}

	return bitswap<8>(mj_input, 6, 5, 4, 2, 3, 1, 0, 7);
}

void ms32_base_state::sound_command_w(u32 data)
{
	m_soundlatch->write(data & 0xff);

	// give the Z80 time to respond
	m_maincpu->spin_until_time(attotime::from_usec(40));
}

u32 ms32_base_state::sound_result_r()
{
	// tp2m32 never pings the sound ack, so irq ack is most likely done here
	irq_raise(1, false);
	return m_to_main ^ 0xff;
}

u8 ms32_state::ms32_nvram_r8(offs_t offset)
{
	return m_nvram_8[offset];
}

void ms32_state::ms32_nvram_w8(offs_t offset, u8 data)
{
	m_nvram_8[offset] = data;
}

u8 ms32_state::ms32_priram_r8(offs_t offset)
{
	return m_priram[offset];
}

void ms32_state::ms32_priram_w8(offs_t offset, u8 data)
{
	m_priram[offset] = data;
}

u16 ms32_state::ms32_palram_r16(offs_t offset)
{
	return m_palram[offset];
}

void ms32_state::ms32_palram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_palram[offset]);
}

u16 ms32_state::ms32_rozram_r16(offs_t offset)
{
	return m_rozram[offset];
}

void ms32_state::ms32_rozram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rozram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset / 2);
}

u16 ms32_state::ms32_lineram_r16(offs_t offset)
{
	return m_lineram[offset];
}

void ms32_state::ms32_lineram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lineram[offset]);
}

u16 ms32_state::ms32_sprram_r16(offs_t offset)
{
	return m_sprram[offset];
}

void ms32_state::ms32_sprram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sprram[offset]);
}

u16 ms32_state::ms32_txram_r16(offs_t offset)
{
	return m_txram[offset];
}

void ms32_state::ms32_txram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

u16 ms32_state::ms32_bgram_r16(offs_t offset)
{
	return m_bgram[offset];
}

void ms32_state::ms32_bgram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
	m_bg_tilemap_alt->mark_tile_dirty(offset / 2);
}

void ms32_state::bgmode_w(u32 data)
{
	m_tilemaplayoutcontrol = data;

	if ((data) && (data != 1))
		popmessage("fce00a7c = %02x",data);
}

void ms32_state::coin_counter_w(u32 data)
{
	// desertwr/p47aces sets 4 here
	// f1superb sets 2
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}

void ms32_state::ms32_map(address_map &map)
{
	/* RAM areas verified by testing on real hw - usually accessed at the 0xfc000000 + mirror */
//  0xfc000000 NVRAM (8-bits wide, 0x2000 in size)
	map(0xc0000000, 0xc0007fff).rw(FUNC(ms32_state::ms32_nvram_r8), FUNC(ms32_state::ms32_nvram_w8)).umask32(0x000000ff).mirror(0x3c1f8000);
//  map(0xc0008000, 0xc01fffff) // mirrors of nvramram, handled above

//  0xfd180000 Priority RAM (8-bits wide, 0x2000 in size)
	map(0xc1180000, 0xc1187fff).rw(FUNC(ms32_state::ms32_priram_r8), FUNC(ms32_state::ms32_priram_w8)).umask32(0x000000ff).mirror(0x3c038000);
//  map(0xc1188000, 0xc11bffff) // mirrors of priram, handled above

//  0xfd200000 palette related, unknown
//  0xfd400000 paletteram (16-bits wide, 0x20000 in size)
//  0xfd400000 object palette
//  0xfd408000 Background palette
//  0xfd410000 ROZ1 palette
//  0xfd420000 ROZ0 palette?
//  0xfd430000 ASCII palette
	map(0xc1400000, 0xc143ffff).rw(FUNC(ms32_state::ms32_palram_r16), FUNC(ms32_state::ms32_palram_w16)).umask32(0x0000ffff).mirror(0x3c1c0000);
//  map(0xc1440000, 0xc145ffff) // mirrors of palram, handled above

//  0xfe000000 ROZ1 VRAM (16-bits wide, 0x10000 in size)
	map(0xc2000000, 0xc201ffff).rw(FUNC(ms32_state::ms32_rozram_r16), FUNC(ms32_state::ms32_rozram_w16)).umask32(0x0000ffff).mirror(0x3c1e0000);
//  map(0xc2020000, 0xc21fffff) // mirrors of rozram, handled above

//  0xfe200000 ROZ1 line RAM (16-bits wide, 0x1000 in size)
	map(0xc2200000, 0xc2201fff).rw(FUNC(ms32_state::ms32_lineram_r16), FUNC(ms32_state::ms32_lineram_w16)).umask32(0x0000ffff).mirror(0x3c1fe000);
//  map(0xc2202000, 0xc23fffff) // mirrors of lineram, handled above

//  0xfe400000 ROZ0 VRAM?
//  0xfe600000 ROZ0 line RAM?
//  0xfe800000 object layer VRAM (16-bits wide, 0x10000 in size)
	map(0xc2800000, 0xc281ffff).rw(FUNC(ms32_state::ms32_sprram_r16), FUNC(ms32_state::ms32_sprram_w16)).umask32(0x0000ffff).mirror(0x3c1e0000);
//  map(0xc2820000, 0xc29fffff) // mirrors of sprram, handled above

//  0xfec00000 ASCII layer VRAM (16-bits wide, 0x4000 in size)
	map(0xc2c00000, 0xc2c07fff).rw(FUNC(ms32_state::ms32_txram_r16), FUNC(ms32_state::ms32_txram_w16)).umask32(0x0000ffff).mirror(0x3c1f0000);

//  0xfec08000 Background layer VRAM (16-bits wide, 0x4000 in size)
	map(0xc2c08000, 0xc2c0ffff).rw(FUNC(ms32_state::ms32_bgram_r16), FUNC(ms32_state::ms32_bgram_w16)).umask32(0x0000ffff).mirror(0x3c1f0000);
//  map(0xc2c10000, 0xc2dfffff) // mirrors of txram / bg, handled above

//  0xfee00000 Scratch RAM (32-bit wide, 0x20000 in size)
	map(0xc2e00000, 0xc2e1ffff).ram().mirror(0x3c0e0000);
//  0xffc00000 ROM (32-bit wide, 0x200000 in size)
	map(0xc3e00000, 0xc3ffffff).rom().region("maincpu", 0).mirror(0x3c000000);

	// I/O section
	// TODO: mirrors like above?
	map(0xfc800000, 0xfc800003).nopr().w(FUNC(ms32_state::sound_command_w));    // open bus on read?
//  map(0xfcc00000, 0xfcc0001f)                                                 // input
	map(0xfcc00004, 0xfcc00007).portr("INPUTS");
	map(0xfcc00010, 0xfcc00013).portr("DSW");
	// System Registers
	map(0xfce00000, 0xfce0005f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap)).umask32(0x0000ffff);
	map(0xfce00200, 0xfce0027f).ram().share(m_sprite_ctrl);
	map(0xfce00280, 0xfce0028f).w(FUNC(ms32_state::ms32_brightness_w));         // global brightness control
//  map(0xfce00400, 0xfce0045f)                                                 // ROZ0 control registers
/**/map(0xfce00600, 0xfce0065f).ram().share(m_roz_ctrl);                        // ROZ1 control registers
/**/map(0xfce00a00, 0xfce00a17).ram().share(m_tx_scroll);                       // ASCII layer scroll
/**/map(0xfce00a20, 0xfce00a37).ram().share(m_bg_scroll);                       // Background layer scroll
	map(0xfce00a7c, 0xfce00a7f).w(FUNC(ms32_state::bgmode_w));
//  map(0xfce00c00, 0xfce00c1f)                                                 // ???
	map(0xfce00e00, 0xfce00e03).w(FUNC(ms32_state::coin_counter_w));            // coin counters + something else
	map(0xfd000000, 0xfd000003).r(FUNC(ms32_state::sound_result_r));
	//  Extended I/O
//  map(0xfd040000, 0xfd040003)
//  map(0xfd080000, 0xfd080003)
//  map(0xfd0c0000, 0xfd0c0003)
	map(0xfd1c0000, 0xfd1c0003).writeonly().share(m_mahjong_input_select);
}


/* F1 Super Battle has an extra linemap for the road, and am unknown maths chip (mcu?) handling perspective calculations for the road / corners etc. */
/* it should use its own memory map */

void ms32_f1superbattle_state::road_vram_w16(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_road_vram[offset]);
	m_extra_tilemap->mark_tile_dirty(offset / 2);
}

u16 ms32_f1superbattle_state::road_vram_r16(offs_t offset)
{
	return m_road_vram[offset];
}

void ms32_f1superbattle_state::ms32_irq2_guess_w(u32 data)
{
	irq_raise(2, true);
}

void ms32_f1superbattle_state::ms32_irq5_guess_w(u32 data)
{
	irq_raise(5, true);
}

u32 ms32_f1superbattle_state::analog_r()
{
	int a,b,c,d;
	a = m_io_analog[2]->read(); // unused?
	b = m_io_analog[2]->read(); // unused?
	c = m_io_analog[1]->read();
	d = (m_io_analog[0]->read() - 0xb0) & 0xff;
	return a << 24 | b << 16 | c << 8 | d << 0;
}

void ms32_f1superbattle_state::f1superb_map(address_map &map)
{
	ms32_map(map);

	// comms RAM, 8-bit resolution?
	// unsurprisingly it seems to use similar mechanics to other Jaleco games
	// Throws "FLAM ERROR" in test mode if irq 11 is enabled
	// irq 11 is cleared to port $1e (irq_ack_w) in sysctrl.
	map(0xfd0c0000, 0xfd0c0fff).ram();
	map(0xfd0d0000, 0xfd0d0003).portr("DSW2"); // MB-93159
	map(0xfd0e0000, 0xfd0e0003).r(FUNC(ms32_f1superbattle_state::analog_r)).nopw(); // writes 7-led seg at very least

	map(0xfce00800, 0xfce0085f).ram(); // ROZ0 control register (mirrored from 0x400?)

	/* these two are almost certainly wrong, they just let you see what
	   happens if you generate the FPU ints without breaking other games */
//  map(0xfce00e00, 0xfce00e03).w(FUNC(ms32_f1superbattle_state::ms32_irq5_guess_w));
	// bit 1: steering shock
	// bit 0: seat motor
//  map(0xfd0f0000, 0xfd0f0003).w(FUNC(ms32_f1superbattle_state::ms32_irq2_guess_w));

	// Note: it is unknown how COPRO irqs actually acks,
	// most likely candidate is a 0x06 ping at both $fd1024c8 / $fd1424c8
	// irq_2: 0xffe00878 (really unused)
	// irq_5: 0xffe008ac
	// irq_7: 0xffe008ea (basically identical to irq_5)
	// COPRO 1
	map(0xfd100000, 0xfd103fff).ram(); // used when you start enabling fpu ints
	map(0xfd104000, 0xfd105fff).ram(); // uploads data here

	// COPRO 2
	map(0xfd140000, 0xfd143fff).ram(); // used when you start enabling fpu ints
	map(0xfd144000, 0xfd145fff).ram(); // same data here
//  map(0xfd440000, 0xfd47ffff).ram(); // color?

	map(0xfdc00000, 0xfdc1ffff).rw(FUNC(ms32_f1superbattle_state::road_vram_r16), FUNC(ms32_f1superbattle_state::road_vram_w16)).umask32(0x0000ffff);
	map(0xfde00000, 0xfde1ffff).ram(); // scroll info for lineram?
//  map(0xfe202000, 0xfe2fffff).ram(); // vram?
}

/* F1 Super Battle speculation from nuapete

Hi David,

I had a first look at f1superb, this is what I found so far.

The sprite RAM is updated in a few places, but taking one of the
sprite RAM updating routines at 0xFFE47B6F which is used to draw stuff
that should move around, I see that the information is copied from
buffers with these bases:
FEE11000 = x position data
FEE11100 = y position data
FEE11200 = priority etc data

You already spotted that loop at FFE19D1C that fills the "y position"
buffer from an array of static values. The buffers are refilled again
by the routine at 0xFFE17581. You can see that the data is sourced
from another buffer at 0xFD100000 (currently not mapped in the driver,
but I mapped it in for testing...)

If you backtrace that call, it comes from irq 5 and 7. (They have
identical ISR code) so I tried adding in a trigger for irq 7. Now the
values are populated, and stuff moves, but everything moves way too
much, it's all over the place, eventually the car zooms off up into
the sky. It gets clearer as you map less and less of the RAM at
0xFD100000. With just 1K or so mapped, you can see the buildings in
the background veer about in quite a promising manner. (The patch you
put in loses effect with irq 5 or 7 hooked up, because they repopulate
the Y coords too.)

I took a closer look at the interrupts. Handily enough they left some
strings in the ROM with names for each interrupt  :)  They aren't quite
in the order of the interrupts, but I matched the unreferenced strings
up to the valid interrupts as well as I could and then tried to
confirm them by looking at the code. The ROM string is in quotes.

FFE00DE8  ; irq_0  probably "1msec interrupt"
FFE00DF4  ; irq_1  "sound cpu interrupt"
FFE00878  ; irq_2  probably "fpu 1-1 interrupt"
FFE00884  ; irq_3  unused and labeled "fpu 0-1 interrupt"
FFE00898  ; irq_4  unused and labeled "fpu 1-0 interrupt"
FFE008AC  ; irq_5  probably "fpu 0-0 interrupt", x coords (and y) populated from here
FFE008D0  ; irq_6  unused and labeled "option 2 interrupt"
FFE008E4  ; irq_7  same as 5 - probably "option 1 interrupt"
FFE01034  ; irq_9  VBL at 60Hz this would be "16msec interrupt"
FFE01094  ; irq_10 loads of processing in the 0xfc000000 area : must be "32msec interrupt"
FFE00E14  ; irq_11 "communication interrupt"

irq_0 is sort of confirmed by the "hayaosi2 needs at least 12 IRQ
0..." comment. irq_1 I see is already known. irq_9 is known, irq_10 I
tried halving the frequency it runs at, no effect. irq_11 can be
pretty much confirmed as comms by the code there and the use of
MOVT/MOVZ to i/o with 16 bit device based at FEE00000, so that leaves
the ones that do the sprite info loading, this is where it starts to
look less promising  :(

Between irqs 2,3,4,5,7 the only unused strings in the ROM are the four "fpu
* interrupt" and the "option 1".
Irqs 2,5,7 all do "or.w    #6, FD1424C8[PC]" ,so they are all probably
"fpu * interrupt".

I'm thinking that the stuff is dumped in that RAM at 0xFD100000 that's
not used by other games and then some FPU operation is carried out on it
before it's grabbed by the sprite copy code. The "option" stuff, may be
they tried a few different ways to work out the sprite coords? There's
one more intersting string at 0xFFE481FC referenced from unused code
at 0xFFE47FBC. It looks like debugging dump of sprite coordinate and
angle information.

I had a scout around for photos or anything of the PCB to see if there
is some sort of DSP or FPU on it, but I can't find anything useful. I
suspect it's not a generic MegaSystem 32 PCB, going by the extra
stuff, and also the IRQ 5/7 breaks the other games. I see a vanilla
one for sale, but I'd guess there's no point me picking it up for a
closer look because it won't have whatever the extras are.

I'll keep looking a bit more, but I think that 0xFD100000 buffer is
processed by something external that triggers irq 5 or 7 when it's
done. I might see something obvious by looking at the values in
fact... I'd be interested to know what you think, or if there's any
chance of finding out if there's stuff like dual port RAM or something
that might qualify as a FPU?


Hi David,

On the f1superb stuff, I've traced the sprite elements right back to
the arrays for each race track in ROM that they are sourced from. The
only processing they get is in that "fpu" interrupt. I've figured out
the structure of the "fpu" device, it uses two arrays of static info
loaded from the ROM at boot, two identical sets of registers, and 4
banks of volatile data read in every frame or so from the ROM race
track arrays (although it looks like one was not used in the end).
There's a sequence of four operations ivolving the "fpu" carried out
to prepare the info which then gets copied back to RAM and then to
sprite RAM. I'll capture the relevant info and see if I can figure out
what the operations might be, my maths isn't up to much though...

*/



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( ms32 )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	// mapping to F1 key because there may be a specific service dip as well
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x00000001, 0x00000001, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000002, 0x00000002, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00000004, 0x00000004, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00000008, 0x00000008, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00000010, 0x00000010, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00000020, 0x00000020, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00000040, 0x00000040, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x00000080, 0x00000080, "SW2:1" )
	PORT_SERVICE_DIPLOC( 0x00000100, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x00000200, 0x00000200, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(          0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00001c00, 0x00001c00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(          0x00001000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00000800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00001800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00001c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00000c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x00001400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00000400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0000e000, 0x0000e000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(          0x00008000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00004000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0000c000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0000e000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00006000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x0000a000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00002000, DEF_STR( 1C_4C ) )

	PORT_DIPUNUSED_DIPLOC( 0x00010000, 0x00010000, "SW3:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00020000, 0x00020000, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00040000, 0x00040000, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00080000, 0x00080000, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00100000, 0x00100000, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00200000, 0x00200000, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00400000, 0x00400000, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x00800000, 0x00800000, "SW3:1" )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )   // Unused?
INPUT_PORTS_END

static INPUT_PORTS_START( ms32_mahjong )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ms32_state::mahjong_ctrl_r))    // here we read mahjong keys
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNUSED )   /* Start 1 is already mapped in mahjong inputs */
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED )   /* ms32.cpp mahjongs don't have P2 inputs -> no Start 2*/
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(mahjong_matrix_1p)

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x00000060, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 ) // rotated into LSB

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( bbbxing )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000002, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, "Timer Speed" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "60/60" )
	PORT_DIPSETTING(          0x00000020, "50/60" )
	PORT_DIPSETTING(          0x00000010, "40/60" )
	PORT_DIPSETTING(          0x00000030, "35/60" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00020000, 0x00020000, "Grute's Attacking Power" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(          0x00020000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "Biff's Attacking Power" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(          0x00040000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "Carolde's Attacking Power" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(          0x00080000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "Jose's Attacking Power" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(          0x00100000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Thamalatt's Attacking Power" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(          0x00200000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "Kim's Attacking Power" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "Jyoji's Attacking Power" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Harder ) )
INPUT_PORTS_END

static INPUT_PORTS_START( desertwr )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, "Armors" ) PORT_DIPLOCATION("SW2:4,3")
//  PORT_DIPSETTING(          0x00000000, "2" )           // duplicate setting ?
	PORT_DIPSETTING(          0x00000010, "2" )
	PORT_DIPSETTING(          0x00000030, "3" )
	PORT_DIPSETTING(          0x00000020, "4" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00100000, 0x00000000, "Title screen" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(          0x00100000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gametngk )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPSETTING(          0x00000030, "3" )
	PORT_DIPSETTING(          0x00000010, "4" )
	PORT_DIPSETTING(          0x00000020, "5" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00010000, 0x00010000, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(          0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Voice" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tetrisp )
	PORT_INCLUDE( ms32 )

	/* There are inputs for players 3 and 4 in the "test mode",
	         but NO addresses are read to check them ! */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0000000c, 0x0000000c, "Join In" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(          0x0000000c, "All Modes" )
	PORT_DIPSETTING(          0x00000004, "Normal and Puzzle Modes" )
	PORT_DIPSETTING(          0x00000008, "VS Mode" )
//  PORT_DIPSETTING(          0x00000000, "Normal and Puzzle Modes" )             // "can't play normal mode" in "test mode"
	PORT_DIPNAME( 0x00000030, 0x00000030, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "1/1" )
	PORT_DIPSETTING(          0x00000030, "2/3" )
	PORT_DIPSETTING(          0x00000010, "3/5" )
	PORT_DIPSETTING(          0x00000020, "4/7" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00010000, 0x00010000, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "After VS Mode" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(          0x00080000, "Game Ends" )
	PORT_DIPSETTING(          0x00000000, "Winner Continues" )
	PORT_DIPNAME( 0x00100000, 0x00100000, "Voice" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(          0x00000000, "English Only" )
	PORT_DIPSETTING(          0x00100000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tp2m32 )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0000000c, 0x0000000c, "VS Match" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPSETTING(          0x0000000c, "3" )
	PORT_DIPSETTING(          0x00000004, "5" )
	PORT_DIPSETTING(          0x00000008, "7" )
	PORT_DIPNAME( 0x00000030, 0x00000030, "Puzzle Difficulty" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x00000030, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, "Endless Difficulty" ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Very_Hard ) )

	PORT_DIPNAME( 0x00010000, 0x00010000, "Freeze" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(          0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00060000, 0x00060000, "Bomb" ) PORT_DIPLOCATION("SW3:7,6")
	PORT_DIPSETTING(          0x00040000, "0" )
	PORT_DIPSETTING(          0x00020000, "1" )
	PORT_DIPSETTING(          0x00060000, "2" )
	PORT_DIPSETTING(          0x00000000, "3" )
	PORT_DIPNAME( 0x00080000, 0x00080000, "Join In" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "Voice" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( p47aces )
	PORT_INCLUDE( ms32 )

	/* The Dip Switches for this game are completely wrong in the "test mode" ! */
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000004, "500k" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPSETTING(          0x00000020, "2" )
	PORT_DIPSETTING(          0x00000030, "3" )
	PORT_DIPSETTING(          0x00000010, "4" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00030000, 0x00030000, "FG/BG X offset" ) PORT_DIPLOCATION("SW3:8,7")
	PORT_DIPSETTING(          0x00030000, "0/0" )
	PORT_DIPSETTING(          0x00020000, "5/5" )
//  PORT_DIPSETTING(          0x00010000, "5/5" )
	PORT_DIPSETTING(          0x00000000, "2/4" )
	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gratia )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000004, "200k and every 1000k" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPSETTING(          0x00000020, "2" )
	PORT_DIPSETTING(          0x00000030, "3" )
	PORT_DIPSETTING(          0x00000010, "4" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x00400000, 0x00400000, "FBI Logo" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00800000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(          0x00800000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )

INPUT_PORTS_END

static INPUT_PORTS_START( hayaosi2 )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("INPUTS")
	// fast button is actually mapped as button 1 in hayaosi2 and button 5 for hayaosi3.
	// We use latter layout for convenience
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Fast Button")
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3) PORT_NAME("P3 Fast Button")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Fast Button")
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START3 )

	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000100, 0x00000100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(          0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x00000001, 0x00000001, "SW2:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00000002, 0x00000002, "SW2:7" )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ) )
	/* Round   Default    More */
	/*   1       10        15  */
	PORT_DIPNAME( 0x00000008, 0x00000008, "Questions (VS Mode)" ) PORT_DIPLOCATION("SW2:5") // TO DO : check all rounds
	PORT_DIPSETTING(          0x00000008, "Default" )
	PORT_DIPSETTING(          0x00000000, "More" )
	/*  Lap    Time    Questions */
	/*   1     1:00        4     */
	/*   2     1:00        6     */
	/*   3     1:30        8     */
	/*   4     1:30       10     */
	/*   4     2:00       14     */
	/* final   2:00       18     */
	PORT_DIPNAME( 0x00000030, 0x00000030, "Time (Race Mode)" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x00000000, "Default - 0:15" )
	PORT_DIPSETTING(          0x00000020, "Default - 0:10" )
	PORT_DIPSETTING(          0x00000030, "Default" )
	PORT_DIPSETTING(          0x00000010, "Default + 0:15" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, "Computer's AI (VS Mode)" ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000040, DEF_STR( Low ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( High ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Highest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hayaosi3 )
	PORT_INCLUDE( hayaosi2 )

	// TODO: dips are somehow different than hayaosi2, cfr. service mode
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000002, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x00000004, 0x00000004, "SW2:6" )
INPUT_PORTS_END

static INPUT_PORTS_START( kirarast )
	PORT_INCLUDE( ms32_mahjong )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x00000400, 0x00000400, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00000800, 0x00000800, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00001000, 0x00001000, "SW1:4" )
	PORT_DIPNAME( 0x0000e000, 0x0000e000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(          0x00008000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00004000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0000c000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0000e000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00006000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x0000a000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00002000, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x00000001, 0x00000001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Campaign Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, "Tsumo Pinfu" ) PORT_DIPLOCATION("SW2:5") // "Tumo Pinfu" (sic)
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( On ) )
	PORT_DIPNAME( 0x000000e0, 0x000000e0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easiest ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Easier ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000e0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000060, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x000000a0, DEF_STR( Harder ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( suchie2 )
	PORT_INCLUDE( kirarast )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNUSED )    /* coin 2 is unused */

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000400, 0x00000400, "Campaign Mode" ) PORT_DIPLOCATION("SW1:6") // "Campain Mode" (sic)
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000400, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000800, 0x00000800, "Tsumo Pinfu" ) PORT_DIPLOCATION("SW1:5") // "Tumo Pinfu" (sic)
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000800, DEF_STR( On ) )
	PORT_DIPNAME( 0x00001000, 0x00001000, DEF_STR( Demo_Sounds) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00001000, DEF_STR( On ) )

	PORT_DIPUNUSED_DIPLOC( 0x00000001, 0x00000001, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000002, 0x00000002, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00000004, 0x00000004, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00000008, 0x00000008, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00000010, 0x00000010, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00000020, 0x00000020, "SW2:3" )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wpksocv2 )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("INPUTS")
	// TODO: Still missing the correct input for begin the left right movement
	PORT_BIT( 0x0000000f, 0x00000000, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(7) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x00000100, 0x00000100, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000200, 0x00000200, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00000400, 0x00000400, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00000800, 0x00000800, "SW1:5" )
	PORT_DIPNAME( 0x00001000, 0x00001000, DEF_STR( Demo_Sounds) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00001000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0000e000, 0x0000e000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(          0x00008000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00004000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0000c000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0000e000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(          0x00006000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x0000a000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00002000, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x00200000, 0x00000000, "Tickets" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c00000, 0x00000000, DEF_STR( Region ) ) PORT_DIPLOCATION("SW3:2,1")
	PORT_DIPSETTING(          0x00400000, DEF_STR( USA ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Asia ) )
//  PORT_DIPSETTING(          0x00800000, "?" )
	PORT_DIPSETTING(          0x00c00000, DEF_STR( Japan ) )
INPUT_PORTS_END

static INPUT_PORTS_START( f1superb )
	PORT_INCLUDE( ms32 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Shift") PORT_TOGGLE
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Brake")
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Credit Counter" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(          0x00000000, "Eremite" ) // no idea what this means, taken from manual
	PORT_DIPSETTING(          0x00000001, "Display" )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Speed Meter Display" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(          0x00000000, "mph" )
	PORT_DIPSETTING(          0x00000002, "km/h" )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, "Time Up Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(          0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x0020, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Region ) ) PORT_DIPLOCATION("SW2:2,1") // English manual only gives 0x48 and 0x80 as valid
	PORT_DIPSETTING(          0x0080, DEF_STR( USA ) )
	PORT_DIPSETTING(          0x0000, DEF_STR( Europe ) )
	PORT_DIPSETTING(          0x0040, "Europe" )
	PORT_DIPSETTING(          0x00c0, DEF_STR( Japan ) )
	PORT_DIPUNUSED_DIPLOC( 0x00000100, 0x00000100, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000200, 0x00000200, "SW1:7" )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   // Acceleration
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN1")   // Steering
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN2")   // Shift + Brake (AN2?)
	PORT_DIPNAME( 0x80, 0x80, "Shift Brake" ) // ???
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "DSW2" )
	PORT_DIPNAME( 0x01, 0x00, "Communication Mode" )    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, "Master" )
	PORT_DIPSETTING(    0x01, "Slave" )
	PORT_DIPNAME( 0x0e, 0x00, "Car ID" ) PORT_DIPLOCATION("SW3:2,3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0a, "6" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPSETTING(    0x0e, "8" )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/********** GFX DECODE **********/

static GFXLAYOUT_RAW( f1layout, 2048, 1, 2048*8, 2048*8 )


static GFXDECODE_START( gfx_ms32 )
	GFXDECODE_ENTRY( "roztiles", 0, gfx_16x16x8_raw, 0x2000, 0x10 )
	GFXDECODE_ENTRY( "bgtiles",  0, gfx_16x16x8_raw, 0x1000, 0x10 )
	GFXDECODE_ENTRY( "txtiles",  0, gfx_8x8x8_raw,   0x6000, 0x10 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1superb )
	GFXDECODE_ENTRY( "roztiles", 0, gfx_16x16x8_raw, 0x2000, 0x10 )
	GFXDECODE_ENTRY( "bgtiles",  0, gfx_16x16x8_raw, 0x1000, 0x10 )
	GFXDECODE_ENTRY( "txtiles",  0, gfx_8x8x8_raw,   0x6000, 0x10 )
	GFXDECODE_ENTRY( "gfx5",     0, f1layout,        0x0000, 0x80 ) // not tilemap data?
GFXDECODE_END



/********** INTERRUPTS **********/

/* Irqs used in desertwr:
   1  - 6a0 - minimal
   9  - 6c8 - minimal
   10 - 6d4 - big, vbl?
*/


IRQ_CALLBACK_MEMBER(ms32_base_state::irq_callback)
{
	int i;
	// TODO: confirm irq priority
	for (i = 15; i >= 0 && !(m_irqreq & (1 << i)); i--) { }
	return i;
}

void ms32_base_state::irq_init()
{
	m_irqreq = 0;
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void ms32_base_state::irq_raise(int level, bool state)
{
	if (state == true)
		m_irqreq |= (1 << level);
	else
		m_irqreq &= ~(1 << level);

	if (m_irqreq)
		m_maincpu->set_input_line(0, ASSERT_LINE);
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void ms32_base_state::timer_irq_w(int state)
{
	irq_raise(0, state);
}

void ms32_base_state::vblank_irq_w(int state)
{
	irq_raise(10, state);
}

void ms32_base_state::field_irq_w(int state)
{
	irq_raise(9, state);
}

void ms32_base_state::sound_reset_line_w(int state)
{
	if (state)
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


/********** SOUND **********/

/*
 Jaleco Mega System 32 sound Z80

 RAM 62256 - the highest RAM adr line is grounded, only 16k is used

 0000-3eff: program ROM (fixed)
 3f00-3f0f: YMF271-F
 3f10     : RW :command latch
 3f20     : RW :2nd command latch  ?? (not connected on PCB)
 3f40     : W : YMF271 pin 4 (bit 1) , YMF271 pin 39 (bit 4)
 3f70     : W : unknown ? connected to JALECO GS91022-04 pin 55 (from GAL)
 3f80     : Bank select - $bB
 4000-7fff: RAM
 8000-bfff: banked ROM area #1 - bank B+1
 c000-ffff: banked ROM area #2 - bank b+1

 IRQ is unused (YMF271 timers are polled to control tempo)
 NMI reads the command latch
 code at $38 reads the 2nd command latch ??
*/

u8 ms32_base_state::latch_r()
{
	return m_soundlatch->read() ^ 0xff;
}

void ms32_base_state::ms32_snd_bank_w(u8 data)
{
	m_z80bank[0]->set_entry((data >> 0) & 0x0f);
	m_z80bank[1]->set_entry((data >> 4) & 0x0f);
}

void ms32_base_state::to_main_w(u8 data)
{
	m_to_main = data;
	irq_raise(1, true);
}


void ms32_base_state::sound_ack_w(int state)
{
	// used by f1superb, is it the reason for sound dying?
	if (state)
		m_to_main = 0xff;
}

void ms32_base_state::base_sound_map(address_map &map)
{
	map(0x0000, 0x3eff).rom();
//  map(0x3f00, 0x3f0f).rw("ymf", FUNC(ymf271_device::read), FUNC(ymf271_device::write));
	map(0x3f10, 0x3f10).rw(FUNC(ms32_state::latch_r), FUNC(ms32_state::to_main_w));
	map(0x3f20, 0x3f20).nopr(); /* 2nd latch ? */
	map(0x3f20, 0x3f20).nopw(); /* to_main2_w  ? */
	map(0x3f40, 0x3f40).nopw();   /* YMF271 pin 4 (bit 1) , YMF271 pin 39 (bit 4) */
	map(0x3f70, 0x3f70).nopw();   // watchdog? banking? very noisy
	map(0x3f80, 0x3f80).w(FUNC(ms32_state::ms32_snd_bank_w));
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0xbfff).bankr(m_z80bank[0]);
	map(0xc000, 0xffff).bankr(m_z80bank[1]);
}

void ms32_state::ms32_sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x3f00, 0x3f0f).rw(m_ymf, FUNC(ymf271_device::read), FUNC(ymf271_device::write));

}


/********** MACHINE INIT **********/

void ms32_base_state::machine_start()
{
	save_item(NAME(m_irqreq));
}

void ms32_base_state::machine_reset()
{
	for (int bank = 0; bank < 2; bank++)
		m_z80bank[bank]->set_entry(bank);

	irq_init();
}

/********** MACHINE DRIVER **********/

void ms32_state::ms32(machine_config &config)
{
	/* basic machine hardware */
	V70(config, m_maincpu, XTAL(40'000'000) / 2); // D70632GD-20 20MHz (40MHz / 2)
	m_maincpu->set_addrmap(AS_PROGRAM, &ms32_state::ms32_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(ms32_state::irq_callback));

	Z80(config, m_audiocpu, 8000000); // Z0840008PSC, Clock from notes (40MHz / 5 or 48MHz / 6?)
	m_audiocpu->set_addrmap(AS_PROGRAM, &ms32_state::ms32_sound_map);

	config.set_maximum_quantum(attotime::from_hz(60000));

	JALECO_MS32_SYSCTRL(config, m_sysctrl, XTAL(48'000'000), m_screen);
	m_sysctrl->flip_screen_cb().set(FUNC(ms32_state::flipscreen_w));
	m_sysctrl->vblank_cb().set(FUNC(ms32_state::vblank_irq_w));
	m_sysctrl->field_cb().set(FUNC(ms32_state::field_irq_w));
	m_sysctrl->prg_timer_cb().set(FUNC(ms32_state::timer_irq_w));
	m_sysctrl->sound_ack_cb().set(FUNC(ms32_state::sound_ack_w));
	m_sysctrl->sound_reset_cb().set(FUNC(ms32_state::sound_reset_line_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// dot clock is 48/8 MHz, settable with /6 thru system register [0]
	m_screen->set_raw(XTAL(48'000'000) / 8, 384, 0, 320, 263, 0, 224); // default CRTC setup
	m_screen->set_screen_update(FUNC(ms32_state::screen_update));
	m_screen->screen_vblank().set(FUNC(ms32_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ms32);
	PALETTE(config, m_palette).set_entries(0x8000);

	JALECO_MEGASYSTEM32_SPRITE(config, m_sprite, XTAL(48'000'000) / 8);
	m_sprite->set_palette(m_palette);
	m_sprite->set_color_base(0);
	m_sprite->set_color_entries(16);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YMF271(config, m_ymf, XTAL(16'934'400)); // 16.9344MHz
	m_ymf->add_route(0, "speaker", 1.0, 0);
	m_ymf->add_route(1, "speaker", 1.0, 1);
// Output 2/3 not used?
//  m_ymf->add_route(2, "speaker", 1.0);
//  m_ymf->add_route(3, "speaker", 1.0);
}

void ms32_state::ms32_invert_lines(machine_config &config)
{
	ms32(config);
	m_sysctrl->set_invert_vblank_lines(true);
}

void ms32_f1superbattle_state::f1superb(machine_config &config)
{
	ms32(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms32_f1superbattle_state::f1superb_map);

	m_gfxdecode->set_info(gfx_f1superb);

//  MCFG_VIDEO_START_OVERRIDE(ms32_state,f1superb)
}



/********** ROM LOADING **********/

ROM_START( bbbxing )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93138a_25_ver1.3.25", 0x000003, 0x80000, CRC(b526b41e) SHA1(44945931b159646468a954d5acdd2c6c61daf098) ) /* uses MB-93138A EB91022-20078-1 rom board */
	ROM_LOAD32_BYTE( "mb93138a_27_ver1.3.27", 0x000002, 0x80000, CRC(45b27ad8) SHA1(0af415b17400aabecdcb6d1d069f28b64780017f) )
	ROM_LOAD32_BYTE( "mb93138a_29_ver1.3.29", 0x000001, 0x80000, CRC(85bbbe79) SHA1(bc5ebb96491762e6a0d202ddf7faeb57c66211b4) )
	ROM_LOAD32_BYTE( "mb93138a_31_ver1.3.31", 0x000000, 0x80000, CRC(e0c865ed) SHA1(f21e8dc174c50d7afdd3f82c1c66dfcc002bdd07) )

	ROM_REGION( 0x1100000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr92042-07.1",          0x0000002, 0x200000, CRC(c1c10c3b) SHA1(e1f739f38e148c4d7aff6b81b3e42131c5c6c3dd) )
	ROM_LOAD32_WORD( "mr92042-06.13",         0x0000000, 0x200000, CRC(4b8c1574) SHA1(c389c70b532d54528a175f460ca3f329b34cf67c) )
	ROM_LOAD32_WORD( "mr92042-09.2",          0x0400002, 0x200000, CRC(03b77c1e) SHA1(f156ae6a4f2a8ae99815eb5a7b28425d273c1c3e) )
	ROM_LOAD32_WORD( "mr92042-08.14",         0x0400000, 0x200000, CRC(e9cfd83b) SHA1(8580c571a4144ea27c7fca7e86e3e4f5e4f5facb) )
	ROM_LOAD32_WORD( "mr92042-11.3",          0x0800002, 0x200000, CRC(bba0d1a4) SHA1(15f8de7478182c36927a84ee8de8eb8ac3e65d7b) )
	ROM_LOAD32_WORD( "mr92042-10.15",         0x0800000, 0x200000, CRC(6ab64a10) SHA1(4ee2cec6b9f8d729ff53851f7949c5cd3a8029d8) )
	ROM_LOAD32_WORD( "mr92042-13.4",          0x0c00002, 0x200000, CRC(97f97e3a) SHA1(260603a10fe742986aa4f7fb90e4f141bdadae17) )
	ROM_LOAD32_WORD( "mr92042-12.16",         0x0c00000, 0x200000, CRC(e001d6cb) SHA1(c887dbf192e6b46c86fd86bb5f58b44ab8fe8d73) )
	ROM_LOAD32_WORD( "mb93138a_5_ver1.0.5",   0x1000002, 0x080000, CRC(64989edf) SHA1(033eab0e8a53607b2bb420f6356804b2cfa1544c) )
	ROM_LOAD32_WORD( "mb93138a_17_ver1.0.17", 0x1000000, 0x080000, CRC(1d7ebaf0) SHA1(5aac7cb01013ce3be206318678aced5812bff9a9) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr92042-05.9",   0x000000, 0x200000, CRC(a41cb650) SHA1(1c55a4afe55c1250806f2d93c25842dc3fb7f987) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr92042-04.11",  0x000000, 0x200000, CRC(85238ca9) SHA1(1ce32d585fe66764d621c11882ef9d2abaea6256) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mr92042-01.32",0x000000, 0x080000, CRC(3ffdae75) SHA1(2b837d28f7ecdd49e8525bd5c249e83021d5fe9f) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93138a_21_ver1.0.21",  0x000000, 0x040000, CRC(5f3ea01f) SHA1(761f6a5852312d2b12de009f3cf0476f5b2e906c) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr92042-02.23",  0x200000, 0x200000, CRC(b7875a23) SHA1(62bb4c1318f98ea68894658d92ce08e84d386d0c) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( suchie2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb-93166-26_ver1.1.26", 0x000003, 0x80000, CRC(e4e62134) SHA1(224b3e8dba56009bf2af6eceb7495e60302a6360) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb-93166-27_ver1.1.27", 0x000002, 0x80000, CRC(7bd00919) SHA1(60565b5e1da5fee00ac4a7fb1202d7150dab49ee) )
	ROM_LOAD32_BYTE( "mb-93166-28_ver1.1.28", 0x000001, 0x80000, CRC(aa49eec2) SHA1(173afc596caa1c464fc3247cb64d36c1d97a1520) )
	ROM_LOAD32_BYTE( "mb-93166-29_ver1.1.29", 0x000000, 0x80000, CRC(92763e41) SHA1(eb593bbb586661c4c4e8728d845b146974d0bdf8) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mb94019-02.1",  0x000002, 0x200000, CRC(f9d692f2) SHA1(666df55d26e00be39073173fa3616ac9dafbe615) )
	ROM_LOAD32_WORD( "mb94019-01.13", 0x000000, 0x200000, CRC(1ddfe825) SHA1(27fbf492fdb4f0b4b8db18330e840c130213e15e) )
	ROM_LOAD32_WORD( "mb94019-04.2",  0x400002, 0x200000, CRC(24ca77ec) SHA1(a5c575224ab276cbed5785f40fc0d35dd2748e74) )
	ROM_LOAD32_WORD( "mb94019-03.14", 0x400000, 0x200000, CRC(b26426c4) SHA1(2d95137edea7d0c380dd8fd99852254ad3e4c837) )
	ROM_LOAD32_WORD( "mb94019-06.3",  0x800002, 0x200000, CRC(c8aa4b57) SHA1(55da6a43ba6f0cb32f5d024f67cae21d04019d2a) )
	ROM_LOAD32_WORD( "mb94019-05.15", 0x800000, 0x200000, CRC(1da63eb4) SHA1(8193ad27ddfe6ba242b73082d1b4a400e88401ba) )
	ROM_LOAD32_WORD( "mb94019-08.4",  0xc00002, 0x200000, CRC(4b07edc9) SHA1(22aaa923a94a7bec997d2adabc8ec2c7696c33a5) )
	ROM_LOAD32_WORD( "mb94019-07.16", 0xc00000, 0x200000, CRC(34f471a8) SHA1(4c9c358a9bfdb436a211caa14d085e631609681d) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "94019-09.11", 0x000000, 0x200000, CRC(cde7bb6f) SHA1(47454dac4ce67ce8d7e0c5ef3a732477ac8170a7) )

	ROM_REGION( 0x100000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mb94019-12.10", 0x000000, 0x100000, CRC(15ae05d9) SHA1(ac00d3766c42ccba4585b9acfacc81bcb940ac26) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb-93166-30_ver1.0.30", 0x000000, 0x080000, CRC(0c738883) SHA1(e552c1842d759e5e617eb9c6cc178620a461b4dd) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb-93166-21_ver1.0.21", 0x000000, 0x040000, CRC(e7fd1bf4) SHA1(74567530364bfd93bffddb588758d8498e197668) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mb94019-10.22", 0x000000, 0x200000, CRC(745d41ec) SHA1(9118d0f27b65c9d37970326ccf86fdccb81d32f5) )
	ROM_LOAD( "mb94019-11.23", 0x200000, 0x200000, CRC(021dc350) SHA1(c71936091f86440201fdbdc94b0d1d22c4018188) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( suchie2o )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb-93166-26_ver1.0.26", 0x000003, 0x80000, CRC(21dc94dd) SHA1(faf2eea891cb061d5df47ef31d9538feb0c1233c) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb-93166-27_ver1.0.27", 0x000002, 0x80000, CRC(5bf18a7d) SHA1(70869dc37e6ad79ce4e85db71a03c5cccf9d732b) )
	ROM_LOAD32_BYTE( "mb-93166-28_ver1.0.28", 0x000001, 0x80000, CRC(b1261d51) SHA1(3f393aeb7a076c4d2d2cc7f22ead05f405186d80) )
	ROM_LOAD32_BYTE( "mb-93166-29_ver1.0.29", 0x000000, 0x80000, CRC(9211c82a) SHA1(0aa3f93293b81e0f66b985046eb5e91708693959) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mb94019-02.1",  0x000002, 0x200000, CRC(f9d692f2) SHA1(666df55d26e00be39073173fa3616ac9dafbe615) )
	ROM_LOAD32_WORD( "mb94019-01.13", 0x000000, 0x200000, CRC(1ddfe825) SHA1(27fbf492fdb4f0b4b8db18330e840c130213e15e) )
	ROM_LOAD32_WORD( "mb94019-04.2",  0x400002, 0x200000, CRC(24ca77ec) SHA1(a5c575224ab276cbed5785f40fc0d35dd2748e74) )
	ROM_LOAD32_WORD( "mb94019-03.14", 0x400000, 0x200000, CRC(b26426c4) SHA1(2d95137edea7d0c380dd8fd99852254ad3e4c837) )
	ROM_LOAD32_WORD( "mb94019-06.3",  0x800002, 0x200000, CRC(c8aa4b57) SHA1(55da6a43ba6f0cb32f5d024f67cae21d04019d2a) )
	ROM_LOAD32_WORD( "mb94019-05.15", 0x800000, 0x200000, CRC(1da63eb4) SHA1(8193ad27ddfe6ba242b73082d1b4a400e88401ba) )
	ROM_LOAD32_WORD( "mb94019-08.4",  0xc00002, 0x200000, CRC(4b07edc9) SHA1(22aaa923a94a7bec997d2adabc8ec2c7696c33a5) )
	ROM_LOAD32_WORD( "mb94019-07.16", 0xc00000, 0x200000, CRC(34f471a8) SHA1(4c9c358a9bfdb436a211caa14d085e631609681d) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mb94019-09.11", 0x000000, 0x200000, CRC(cde7bb6f) SHA1(47454dac4ce67ce8d7e0c5ef3a732477ac8170a7) )

	ROM_REGION( 0x100000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mb94019-12.10", 0x000000, 0x100000, CRC(15ae05d9) SHA1(ac00d3766c42ccba4585b9acfacc81bcb940ac26) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb-93166-30_ver1.0.30", 0x000000, 0x080000, CRC(0c738883) SHA1(e552c1842d759e5e617eb9c6cc178620a461b4dd) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb-93166-21_ver1.0.21", 0x000000, 0x040000, CRC(e7fd1bf4) SHA1(74567530364bfd93bffddb588758d8498e197668) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mb94019-10.22", 0x000000, 0x200000, CRC(745d41ec) SHA1(9118d0f27b65c9d37970326ccf86fdccb81d32f5) )
	ROM_LOAD( "mb94019-11.23", 0x200000, 0x200000, CRC(021dc350) SHA1(c71936091f86440201fdbdc94b0d1d22c4018188) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( desertwr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-26.26", 0x000003, 0x80000, CRC(582b9584) SHA1(027a987cde7e9e1b24aef6a3086eba61679ad0b6) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-27.27", 0x000002, 0x80000, CRC(cb60dda3) SHA1(0499b8ab19abdf8db8c18d778b3f9f6e0d277ff0) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-28.28", 0x000001, 0x80000, CRC(0de40efb) SHA1(c49c3b27939e428dec1f642b7fdb9a1ff760289a) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-29.29", 0x000000, 0x80000, CRC(fc25eae2) SHA1(a4d47fcb4d4c3285cf67d77d8a21478f344b98ca) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mb94038-01.13", 0x000000, 0x200000, CRC(f11f83e2) SHA1(e3c99e6583003210483163c79182cb14aa334702) )
	ROM_LOAD32_WORD( "mb94038-02.1",  0x000002, 0x200000, CRC(3d1fa710) SHA1(5fae3e8c714cca88e22e432dd7275c6898c631a8) )
	ROM_LOAD32_WORD( "mb94038-03.14", 0x400000, 0x200000, CRC(84fd5790) SHA1(6187ff32a63f3b4105ea875f52237f0d4314f8b6) )
	ROM_LOAD32_WORD( "mb94038-04.2",  0x400002, 0x200000, CRC(b9ef5b78) SHA1(e2f160a93532f67948a557db717d44c926ae0e49) )
	ROM_LOAD32_WORD( "mb94038-05.15", 0x800000, 0x200000, CRC(feee1b8d) SHA1(39e1d424dd56257139ab5ab8e897caa1c9cd4e71) )
	ROM_LOAD32_WORD( "mb94038-06.3",  0x800002, 0x200000, CRC(d417f289) SHA1(39cca11989bb5dc95ef03013d23a7c100bcb36ab) )
	ROM_LOAD32_WORD( "mb94038-07.16", 0xc00000, 0x200000, CRC(426f4193) SHA1(98a16a70c225d7cd061fcd6e88992d393e6ef9fd) )
	ROM_LOAD32_WORD( "mb94038-08.4",  0xc00002, 0x200000, CRC(f4088399) SHA1(9d53880996f85776815840bca1f8c3958de4c275) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mb94038-11.11", 0x000000, 0x200000, CRC(bf2ec3a3) SHA1(904cd5ab2e855bdb94bc70efa6db42af672337d7) )
	ROM_LOAD( "mb94038-12.12", 0x200000, 0x200000, CRC(d0e113da) SHA1(57f27cbd58421a0afe724fec5628c4a29aad8868) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mb94038-09.10", 0x000000, 0x200000, CRC(72ec1ce7) SHA1(88bd9ca3aa7a6410e8fcf6fd70304f12724653bb) ) /* YES, the ROM number & socket number are backwards - it's correct */
	ROM_LOAD( "mb94038-10.9",  0x200000, 0x200000, CRC(1e17f2a9) SHA1(19e6be1daa157593fbab84149f1f739dd39c9226) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93166_ver1.0-29.30", 0x000000, 0x080000, CRC(980ab89c) SHA1(8468fc13a5988e25750e8d99ff464f46e86ab412) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93166_ver1.0-21.21", 0x000000, 0x040000, CRC(9300be4c) SHA1(a8e9c1704abf26545aeb9a5d28fd0cafd38f2d84) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mb92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mb94038-13.23", 0x200000, 0x200000, CRC(b0cac8f2) SHA1(f7d2e32d9c2f301341f7c02678c2c1e09ce655ba) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( f1superb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "f1sb26.bin", 0x000003, 0x80000, CRC(042fccd5) SHA1(4a69de3aef51adad502d54987468170b9e7bb8ac) )
	ROM_LOAD32_BYTE( "f1sb27.bin", 0x000002, 0x80000, CRC(5f96cf32) SHA1(c9c64576a8bb81a8e8bbe30b054ed33afd760b93) )
	ROM_LOAD32_BYTE( "f1sb28.bin", 0x000001, 0x80000, CRC(cfda8003) SHA1(460146556f606bf213d7e2ab29d2eb8827131bd0) )
	ROM_LOAD32_BYTE( "f1sb29.bin", 0x000000, 0x80000, CRC(f21f1481) SHA1(97a97ff3b9a71b1a024d8f2cfe57a1d02cec5ea4) )

	ROM_REGION( 0x2000000, "sprite", 0 ) /* 8x8 not all? */
	ROM_LOAD32_WORD( "f1sb1.bin",  0x0000002, 0x200000, CRC(53a3a97b) SHA1(c8cd501ae10d9eb48a02e4e59a1ce389a27afc44) )
	ROM_LOAD32_WORD( "f1sb13.bin", 0x0000000, 0x200000, CRC(36565a99) SHA1(db08ff3107dcc07ca31140715d7d4b7bd27fa4c5) )
	ROM_LOAD32_WORD( "f1sb2.bin",  0x0400002, 0x200000, CRC(a452f50a) SHA1(3782a37203b652ea5df5b04dc74a0fdace7b14cc) )
	ROM_LOAD32_WORD( "f1sb14.bin", 0x0400000, 0x200000, CRC(c0c20490) SHA1(9e93beae38c5cfca9f381b4d134c1d95cfa2223a) )
	ROM_LOAD32_WORD( "f1sb3.bin",  0x0800002, 0x200000, CRC(265d068c) SHA1(a2a7850fbc2a04e448f772544d2f6925f9aaf99c) )
	ROM_LOAD32_WORD( "f1sb15.bin", 0x0800000, 0x200000, CRC(575a146e) SHA1(bf67a89ac3145d38564b9dbc3c650c9d5f2bcd92) )
	ROM_LOAD32_WORD( "f1sb4.bin",  0x0c00002, 0x200000, CRC(0ccc66fd) SHA1(07ffef821300386224a7743e8f83088e3437c6db) )
	ROM_LOAD32_WORD( "f1sb16.bin", 0x0c00000, 0x200000, CRC(a2d017a1) SHA1(6c1dd67a1c9c18d69f7dbf7d4637671809be5c89) )
	ROM_LOAD32_WORD( "f1sb5.bin",  0x1000002, 0x200000, CRC(bff4271b) SHA1(dc5672f51348fe0a79352eeafaeeefd78428fe5a) )
	ROM_LOAD32_WORD( "f1sb17.bin", 0x1000000, 0x200000, CRC(2b9739d5) SHA1(db9e93fe79dfa041584730df9cc678caad073251) )
	ROM_LOAD32_WORD( "f1sb6.bin",  0x1400002, 0x200000, CRC(6caf48ec) SHA1(18ea445970285950c61c8eff74a3ab6387816990) )
	ROM_LOAD32_WORD( "f1sb18.bin", 0x1400000, 0x200000, CRC(c49055ff) SHA1(6038dc497583229060ad686090d6940c0a3d1558) )
	ROM_LOAD32_WORD( "f1sb7.bin",  0x1800002, 0x200000, CRC(a5458947) SHA1(7743d67a167f6eecfac6614d1336c6df425e5e95) )
	ROM_LOAD32_WORD( "f1sb19.bin", 0x1800000, 0x200000, CRC(b7cacf0d) SHA1(cea52b1062cf154ccba11640798902b0f9ddeb77) )
	ROM_LOAD32_WORD( "f1sb8.bin",  0x1c00002, 0x200000, CRC(ba3f1533) SHA1(3ff1c4cca8358fc8daf0d2c381672569085ac9ae) )
	ROM_LOAD32_WORD( "f1sb20.bin", 0x1c00000, 0x200000, CRC(fa349897) SHA1(31e08aa2821e409057e3094333b9ecbe04a6a38a) )

	ROM_REGION( 0x800000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "f1sb9.bin",  0x000000, 0x200000, CRC(66b12e1f) SHA1(4dc3f162a5116403cc0c491af335208672c8e9af) )
	ROM_LOAD( "f1sb10.bin", 0x200000, 0x200000, CRC(893d7f4b) SHA1(b2734f20f703a0dcf8b1fdaebf2b6198b2fb0f51) )
	ROM_LOAD( "f1sb11.bin", 0x400000, 0x200000, CRC(0b848bb5) SHA1(e4c0e434add151112352d6068e5de1a7098e6346) )
	ROM_LOAD( "f1sb12.bin", 0x600000, 0x200000, CRC(edecd5f4) SHA1(9b86802d08e5c8ec1a6dcea75dc8f050d3e96970) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "f1sb31.bin", 0x000000, 0x200000, CRC(1d0d2efd) SHA1(e6446ef9c71be9316c286157f71e0043347c6a5c) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "f1sb32.bin", 0x000000, 0x080000, CRC(1b31fcce) SHA1(354cc6f43cd3bf3ba921ac8c5631ab993bedf563) )

	ROM_REGION( 0x800000, "gfx5", 0 ) /* extra data? doesn't seem to be tiles */
	ROM_LOAD( "f1sb2b.bin", 0x000000, 0x200000, CRC(18d73b16) SHA1(f06f4d31b15658cc1e1b559ae3b8a90b797546ca) )
	ROM_LOAD( "f1sb3b.bin", 0x200000, 0x200000, CRC(ce728fe0) SHA1(e0fd7b8f4d3dc9e2b15ddf027c61e67e5c1f22b5) )
	ROM_LOAD( "f1sb4b.bin", 0x400000, 0x200000, CRC(077180c5) SHA1(ab16739da709ecdbbb1264beba349ef6ecf3f8b1) )
	ROM_LOAD( "f1sb5b.bin", 0x600000, 0x200000, CRC(efabc47d) SHA1(195afde8a1f45da4fc04c3080a3cf5fdfff7be5e) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "f1sb21.bin", 0x000000, 0x040000, CRC(e131e1c7) SHA1(33f95a074930c49548069518d8c6dcde7fa25627) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "f1sb23.bin", 0x200000, 0x200000, CRC(bfefa3ab) SHA1(7770cc9b091e258ede7f2780df61a592cc008dd7) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( gratia )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "94019_26_ver1.0.26", 0x000003, 0x80000, CRC(f398cba5) SHA1(11e06abebfdfc8a99b5c56e9f6ed389f645b6c72) ) /* Labeled 94019  (26)Ver1,0  with the Kanji version of the game name before "(26)" */
	ROM_LOAD32_BYTE( "94019_27_ver1.0.27", 0x000002, 0x80000, CRC(ba3318c5) SHA1(9b100988b998c39b586b51fe9fee874dbf711610) ) /* Labeled 94019  (27)Ver1,0  with the Kanji version of the game name before "(27)" */
	ROM_LOAD32_BYTE( "94019_28_ver1.0.28", 0x000001, 0x80000, CRC(e0762e89) SHA1(a567c347e7f73f1ef1c753d14ac4f58311380fac) ) /* Labeled 94019  (28)Ver1,0  with the Kanji version of the game name before "(28)" */
	ROM_LOAD32_BYTE( "94019_29_ver1.0.29", 0x000000, 0x80000, CRC(8059800b) SHA1(7548d01b6ea15e962353b3585db6515e5819e5ce) ) /* Labeled 94019  (29)Ver1,0  with the Kanji version of the game name before "(29)" */

	ROM_REGION( 0x0c00000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94019-01.13", 0x000000, 0x200000, CRC(92d8ae9b) SHA1(02b36e6e14b28a9830e07fd328772dbb20b76889) )
	ROM_LOAD32_WORD( "mr94019-02.1",  0x000002, 0x200000, CRC(f7bd9cc4) SHA1(5658bfb4081439ab06c6ade2531581aa60d1c6be) )
	ROM_LOAD32_WORD( "mr94019-03.14", 0x400000, 0x200000, CRC(62a69590) SHA1(d95cc1e1ec85161ee6cd1ae77b405cf8ef81217a) )
	ROM_LOAD32_WORD( "mr94019-04.2",  0x400002, 0x200000, CRC(5a76a39b) SHA1(fc7c1ff9a0a3c2639fc52720aefe8b2a9e5d2857) )
	ROM_LOAD32_WORD( "mr94019-05.15", 0x800000, 0x200000, CRC(a16994df) SHA1(9170b1fd9252d7a9601c3b2e6b1ba86686730b86) )
	ROM_LOAD32_WORD( "mr94019-06.3",  0x800002, 0x200000, CRC(01d52ef1) SHA1(1585c7eb3729bab78467f627b7b671d619451a74) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94019-08.11", 0x000000, 0x200000, CRC(abd124e0) SHA1(2da1d818c909e4abbb79ed03f3dbf15d744439ce) )
	ROM_LOAD( "mr94019-09.12", 0x200000, 0x200000, CRC(711ab08b) SHA1(185b80b965ac3aba4857b4f83637008c2c1cc6ff) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "94019_2.07", 0x000000, 0x200000, CRC(043f969b) SHA1(ad10339e561c1a65451a2e9a8e79ceda3787674a) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "94019_2.030",0x000000, 0x080000, CRC(f9543fcf) SHA1(8466c7893bc6c43e2a80b8f91a776fd0a345ea6c) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "94019_21ver1.0.21",0x000000, 0x040000, CRC(6e8dd039) SHA1(f1e69c9b40b14ba0f8377a6d9b6c3933919bc803) ) /* Labeled 94019  (21)Ver1,0  with the Kanji version of the game name before "(21)" */

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94019-10.23", 0x200000, 0x200000, CRC(a751e316) SHA1(3d658370c71b83582fd132b3da441089df9bfd05) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( gratiaa )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "94019_26_ver1.0.26", 0x000003, 0x80000, CRC(f398cba5) SHA1(11e06abebfdfc8a99b5c56e9f6ed389f645b6c72) ) /* Labeled 94019  (26)Ver1,0  with the Kanji version of the game name before "(26)" */
	ROM_LOAD32_BYTE( "94019_27_ver1.0.27", 0x000002, 0x80000, CRC(ba3318c5) SHA1(9b100988b998c39b586b51fe9fee874dbf711610) ) /* Labeled 94019  (27)Ver1,0  with the Kanji version of the game name before "(27)" */
	ROM_LOAD32_BYTE( "94019_28_ver1.0.28", 0x000001, 0x80000, CRC(e0762e89) SHA1(a567c347e7f73f1ef1c753d14ac4f58311380fac) ) /* Labeled 94019  (28)Ver1,0  with the Kanji version of the game name before "(28)" */
	ROM_LOAD32_BYTE( "94019_29_ver1.0.29", 0x000000, 0x80000, CRC(8059800b) SHA1(7548d01b6ea15e962353b3585db6515e5819e5ce) ) /* Labeled 94019  (29)Ver1,0  with the Kanji version of the game name before "(29)" */

	ROM_REGION( 0x0c00000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94019-01.13", 0x000000, 0x200000, CRC(92d8ae9b) SHA1(02b36e6e14b28a9830e07fd328772dbb20b76889) )
	ROM_LOAD32_WORD( "mr94019-02.1",  0x000002, 0x200000, CRC(f7bd9cc4) SHA1(5658bfb4081439ab06c6ade2531581aa60d1c6be) )
	ROM_LOAD32_WORD( "mr94019-03.14", 0x400000, 0x200000, CRC(62a69590) SHA1(d95cc1e1ec85161ee6cd1ae77b405cf8ef81217a) )
	ROM_LOAD32_WORD( "mr94019-04.2",  0x400002, 0x200000, CRC(5a76a39b) SHA1(fc7c1ff9a0a3c2639fc52720aefe8b2a9e5d2857) )
	ROM_LOAD32_WORD( "mr94019-05.15", 0x800000, 0x200000, CRC(a16994df) SHA1(9170b1fd9252d7a9601c3b2e6b1ba86686730b86) )
	ROM_LOAD32_WORD( "mr94019-06.3",  0x800002, 0x200000, CRC(01d52ef1) SHA1(1585c7eb3729bab78467f627b7b671d619451a74) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94019-08.11", 0x000000, 0x200000, CRC(abd124e0) SHA1(2da1d818c909e4abbb79ed03f3dbf15d744439ce) )
	ROM_LOAD( "mr94019-09.12", 0x200000, 0x200000, CRC(711ab08b) SHA1(185b80b965ac3aba4857b4f83637008c2c1cc6ff) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94019-07.10", 0x000000, 0x200000, CRC(561a786b) SHA1(23df08d50801bd6e4a2f12dd3bb50632ff77f0f2) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "94019_30ver1.0.30",0x000000, 0x080000, CRC(026b5379) SHA1(b9237477f1bf8ae83174e8231492fe667e6d6a13) ) /* Labeled 94019  (21)Ver1,0  with the Kanji version of the game name before "(30)" */

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "94019_21ver1.0.21",0x000000, 0x040000, CRC(6e8dd039) SHA1(f1e69c9b40b14ba0f8377a6d9b6c3933919bc803) ) /* Labeled 94019  (21)Ver1,0  with the Kanji version of the game name before "(21)" */

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94019-10.23", 0x200000, 0x200000, CRC(a751e316) SHA1(3d658370c71b83582fd132b3da441089df9bfd05) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( gametngk )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb94166_ver1.0-26.26", 0x000003, 0x80000, CRC(e622e774) SHA1(203c2a3563a337af4cec92a66e0fa410d901b01f) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb94166_ver1.0-27.27", 0x000002, 0x80000, CRC(da862b9c) SHA1(17dc6da08d7f5551c8f4bc4d9c416dbfc82d8397) )
	ROM_LOAD32_BYTE( "mb94166_ver1.0-28.28", 0x000001, 0x80000, CRC(b3738934) SHA1(cd07572e55e83807e76179cfc6b97e0410067911) )
	ROM_LOAD32_BYTE( "mb94166_ver1.0-29.29", 0x000000, 0x80000, CRC(45154a45) SHA1(4c7c2c6738fdfe54ebe41a0ac6222cbfce5d7757) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94041-01.13", 0x0000000, 0x200000, CRC(3f99adf7) SHA1(cbb8d2fc253b0c58e7eb9286c66e6b36daf9d4af) )
	ROM_LOAD32_WORD( "mr94041-02.1",  0x0000002, 0x200000, CRC(c3c5ae69) SHA1(5ed7f57a7139f87c680c68e44ea4022b917a9381) )
	ROM_LOAD32_WORD( "mr94041-03.14", 0x0400000, 0x200000, CRC(d858b6de) SHA1(a06cf529c9508c8c8508894e2e004373edd9debf) )
	ROM_LOAD32_WORD( "mr94041-04.2",  0x0400002, 0x200000, CRC(8c96ca20) SHA1(097cab64ef8e515c59178c36171f87bed4b3d1e5) )
	ROM_LOAD32_WORD( "mr94041-05.15", 0x0800000, 0x200000, CRC(ac664a0b) SHA1(bd002822a38369599a1b5a7456957de1d9cd976e) )
	ROM_LOAD32_WORD( "mr94041-06.3",  0x0800002, 0x200000, CRC(70dd0dd4) SHA1(da648c16ad0cb12ac66656522da14392be7772c9) )
	ROM_LOAD32_WORD( "mr94041-07.16", 0x0c00000, 0x200000, CRC(a6966af5) SHA1(3a65824f3f325af39d8e9932357ce9f8878f0321) )
	ROM_LOAD32_WORD( "mr94041-08.4",  0x0c00002, 0x200000, CRC(d7d2f73a) SHA1(0eb28f4cdea73aa8fed0b62cbac6cd7d7694c2ee) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94041-11.11", 0x000000, 0x200000, CRC(00dcdbc3) SHA1(f7e34bc9f714ea81fc9855d90db792dd1e99bae8) )
	ROM_LOAD( "mr94041-12.12", 0x200000, 0x200000, CRC(0ce48329) SHA1(9c198cef998eb3b9c33123bd8cc02210498f82d9) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94041-09.10", 0x000000, 0x200000, CRC(a33e6051) SHA1(d6e34b022eb36dcfa8cfe6d6d1254f994b3b3dca) ) /* YES, the ROM number & socket number are backwards - it's correct */
	ROM_LOAD( "mr94041-10.9",  0x200000, 0x200000, CRC(b3497147) SHA1(df7d8ea7ec3e3df5e0c6658f14995df5479181bf) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb94166_ver1.0-30.30", 0x000000, 0x080000, CRC(c0f27b7f) SHA1(874fe80aa4b46520f844ef6efa61f28eabccbc4f) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb94166_ver1.0-21.21", 0x000000, 0x040000, CRC(38dcb837) SHA1(29fdde54e52dec4ee39a6f2db8e0d67774320d15) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr94041-13.22", 0x000000, 0x200000, CRC(fba84caf) SHA1(318270dbf825a8e0f315992c49a2dc34dd1df7c1) )
	ROM_LOAD( "mr94041-14.23", 0x200000, 0x200000, CRC(2d6308bd) SHA1(600b6ccdbb976301075e0b287124a9fd5fe7fc1b) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( hayaosi2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93138a.25", 0x000003, 0x80000, CRC(563c6f2f) SHA1(bc2a61fd2e0adf58256feeef8491b67af6d6eacf) ) /* uses MB-93138A EB91022-20078-1 rom board */
	ROM_LOAD32_BYTE( "mb93138a.27", 0x000002, 0x80000, CRC(fe8e283a) SHA1(fc6c06ae296110b1f5794187d5208b17541614cb) )
	ROM_LOAD32_BYTE( "mb93138a.29", 0x000001, 0x80000, CRC(e6fe3d0d) SHA1(9a0caab82b160991b4f2ac993e7e4b4c5d3bb15e) )
	ROM_LOAD32_BYTE( "mb93138a.31", 0x000000, 0x80000, CRC(d944bf8c) SHA1(ce93b5d2ebe886b38dc42b1e554b17dc951a51b4) )

	ROM_REGION( 0x900000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr93038.04",  0x000000, 0x200000, CRC(ab5edb11) SHA1(b7742aefbce9efc512c3526714b6f20a6c03af60) )
	ROM_LOAD32_WORD( "mr93038.05",  0x000002, 0x200000, CRC(274522f1) SHA1(717435d6bf1b2a2220a2f0a53b070bb81cc2ed2b) )
	ROM_LOAD32_WORD( "mr93038.06",  0x400000, 0x200000, CRC(f9961ebf) SHA1(e91b160cb5a76e3f6044cc71681dadf2fbff7e8b) )
	ROM_LOAD32_WORD( "mr93038.07",  0x400002, 0x200000, CRC(1abef1c5) SHA1(4b40adaebf9d9963493bfb285badbb19a5b181be) )
	ROM_LOAD32_WORD( "mb93138a.15", 0x800000, 0x080000, CRC(a5f64d87) SHA1(11bf017f700faba57a5a2edced7a5d81a581bc50) )
	ROM_LOAD32_WORD( "mb93138a.3",  0x800002, 0x080000, CRC(a2ae2b21) SHA1(65cee4e5e0a9b8dcac578e34210e1af7d7b2e6f7) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr93038.03",  0x000000, 0x200000, CRC(6999dec9) SHA1(eb4c6ba200cd08b41509314c659feb3af12117ee) )

	ROM_REGION( 0x100000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr93038.08",  0x000000, 0x100000, CRC(21282cb0) SHA1(52ea94a6457f7684674783c362052bcc40086dd0) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93138a.32", 0x000000, 0x080000, CRC(f563a144) SHA1(14d86e4992329811857e1faf282cd9ec530a364c) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93138a.21", 0x000000, 0x040000, CRC(8e8048b0) SHA1(93285a0570ed829b36f4e8c57d133a7dd14f123d) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042.01",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr93038.01",  0x200000, 0x200000, CRC(b8a38bfc) SHA1(1aa7b69beebceb6f09a1ee006de054cb84002e94) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

/*

Hayaoshi Quiz Nettou Namahousou
(c)1993 Jaleco

MB-93140A EB91022-20079-1 (Motherboard. Mega System 32?)
MB-93138A EB91022-20078-1 (ROM board)
SE-93139 EB91022-30056 (extended board on ROM board)

CPU  : NEC JAPAN D70632GD-20 (V70)
Sound: Z80 YMF271-F
OSC  : 48.0000MHz (OSC1) 40.0000MHz (OSC2) 16.9344MHz (X1)

ROMs:
MR94027.02 (16M mask, location IC 3( 1)) [59976568]
MR94027.04 (16M mask, location IC 4( 2)) [6a16d13a]
MR94027.06 (16M mask, location IC 5( 3)) [1618785a]
MR94027.08 (16M mask, location IC 6( 4)) [753b05e0]

MR94027.09 (16M mask, location IC11( 9)) [32ead437]
MR94027.11 (16M mask, location IC13(11)) [b65d5096]

MR94027.01 (16M mask, location IC20(13)) [c72e5c6e]
MR94027.03 (16M mask, location IC21(14)) [3ff68f4f]
MR94027.05 (16M mask, location IC22(15)) [59545977]
MR94027.07 (16M mask, location IC23(16)) [c66099c4]

MB93138.21 (M27C2001, location IC30(21)) [008bc217] Ver1.0
actual label is "MB-93138 21 Ver1.0"

MR92042.01 (16M mask, location IC33(22)) [0fa26f65]
MR94027.10 (16M mask, locaiton IC34(23)) [e7cabe41]

MB93138.25 (M27C4001, location IC36(25)) [ba8cec03] Ver1.5
MB93138.27 (M27C4001, location IC38(27)) [571725df] Ver1.5
MB93138.29 (M27C4001, location IC40(29)) [da891976] Ver1.5
MB93138.31 (M27C4001, location IC42(31)) [2d17bb06] Ver1.5
actual label is "???????N?C?Y?M?????????IMB-93138-?? Ver.1.5"

MB93138.32 (M27C4001, location IC43(32)) [df5d00b4] Ver1.0
actual label is "MB-93138 32 Ver1.0"


PALs (not dumped):
91022-01.2 (18CV8,  IC83(2))
91022-02.1 (22CV10, IC62(1))

Custom chips:
SS91022-01 9348 ACBA (IC36, 208pin PQFP)
SS91022-02 9350 IAHA (IC 9, 100pin PQFP)
SS91022-03 9343EX006 (IC11, 176pin PQFP)
SS91022-05 9347EX002 (IC31, 120pin PQFP)
SS91022-07 9345EV 450881 06440 (IC70, 208pin PQFP)

GS91022-01 9340EK002 (IC46, 120pin PQFP)
GS91022-02 9334EK709 (IC6,  160pin PQFP)
GS91022-03 9335PP711 (IC7,  100pin PQFP)
GS91022-04 9334PP712 (IC24, 100pin PQFP)

SS92046-01 9338EV 436091 06441 (IC1 of EB91022-30056, 144pin PQFP)

Others:
Lithium battery + LH5168D-10L(SRAM)

*/


ROM_START( hayaosi3 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93138_25_ver1.5.25", 0x000003, 0x80000, CRC(ba8cec03) SHA1(edaa52e0b07307bb21168205ee0d5d6ff8168de9) ) /* uses MB-93138A EB91022-20078-1 rom board */
	ROM_LOAD32_BYTE( "mb93138_27_ver1.5.27", 0x000002, 0x80000, CRC(571725df) SHA1(66575ec1a29d6fc1b50ae5a5ce8025bb1043deaf) )
	ROM_LOAD32_BYTE( "mb93138_29_ver1.5.29", 0x000001, 0x80000, CRC(da891976) SHA1(27e8c395e92ca01b47bffdf766bc95a6c2150815) )
	ROM_LOAD32_BYTE( "mb93138_31_ver1.5.31", 0x000000, 0x80000, CRC(2d17bb06) SHA1(623b603c4002734427c882424a1e0dc889cf7e02) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94027.01",  0x000000, 0x200000, CRC(c72e5c6e) SHA1(b98cd656c48c775953d00b5d8bafd4ffde76d8df) )
	ROM_LOAD32_WORD( "mr94027.02",  0x000002, 0x200000, CRC(59976568) SHA1(a280c352d612913834c76b8e23d86c937fd21281) )
	ROM_LOAD32_WORD( "mr94027.03",  0x400000, 0x200000, CRC(3ff68f4f) SHA1(1e367b92560c32c87e27fc0e99be3bdb5eb0510b) )
	ROM_LOAD32_WORD( "mr94027.04",  0x400002, 0x200000, CRC(6a16d13a) SHA1(65a7751c248c966fd01149418ce6bedba7a0d48a) )
	ROM_LOAD32_WORD( "mr94027.05",  0x800000, 0x200000, CRC(59545977) SHA1(2e0a83efd7ae210c0b4360e9572dd7eec38cd974) )
	ROM_LOAD32_WORD( "mr94027.06",  0x800002, 0x200000, CRC(1618785a) SHA1(3f2698d07a52947429313a78ebcedfdae478efd7) )
	ROM_LOAD32_WORD( "mr94027.07",  0xc00000, 0x200000, CRC(c66099c4) SHA1(5a6edffa39a98f38cc3cffbad9191fb2e794a812) )
	ROM_LOAD32_WORD( "mr94027.08",  0xc00002, 0x200000, CRC(753b05e0) SHA1(0424e92b32a73c27ecb549e6e9449446ea938e40) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94027.09",  0x000000, 0x200000, CRC(32ead437) SHA1(b94175cf186b4ebcc180a4c092d2ffcdd9ff3b1d) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94027.11",  0x000000, 0x200000, CRC(b65d5096) SHA1(2c4e1e3e9f96be8369cb2de142a82f94506f85c0) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93138_32_ver1.0.32", 0x000000, 0x080000, CRC(df5d00b4) SHA1(2bbbcd546d5b5170d81bf33b37b46b70b417c9c7) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93138.21", 0x000000, 0x040000, CRC(008bc217) SHA1(eec66a86f285ccbc47eba17a4bb83cc1f8a5f425) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042.01",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94027.10",  0x200000, 0x200000, CRC(e7cabe41) SHA1(5d903baed690a98856f7581319cf4dbfe1db47bb) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( hayaosi3a )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93138_25_ver1.2.25", 0x000003, 0x80000, CRC(71b1f51b) SHA1(bd1c4f75c2949a998ce0f5acaf6def7e7069e40b) ) /* uses MB-93138A EB91022-20078-1 rom board */
	ROM_LOAD32_BYTE( "mb93138_27_ver1.2.27", 0x000002, 0x80000, CRC(2657e8dc) SHA1(efeafe8c890d447ab4584fd7509538fc86fd555b) )
	ROM_LOAD32_BYTE( "mb93138_29_ver1.2.29", 0x000001, 0x80000, CRC(8999b41b) SHA1(95b94112105bfa2b708bad44bbbdc33616ad2182) )
	ROM_LOAD32_BYTE( "mb93138_31_ver1.2.31", 0x000000, 0x80000, CRC(f5d4ef54) SHA1(ed208cb6ed171acac312cb282b2fabc8af70610e) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94027.01",  0x000000, 0x200000, CRC(c72e5c6e) SHA1(b98cd656c48c775953d00b5d8bafd4ffde76d8df) )
	ROM_LOAD32_WORD( "mr94027.02",  0x000002, 0x200000, CRC(59976568) SHA1(a280c352d612913834c76b8e23d86c937fd21281) )
	ROM_LOAD32_WORD( "mr94027.03",  0x400000, 0x200000, CRC(3ff68f4f) SHA1(1e367b92560c32c87e27fc0e99be3bdb5eb0510b) )
	ROM_LOAD32_WORD( "mr94027.04",  0x400002, 0x200000, CRC(6a16d13a) SHA1(65a7751c248c966fd01149418ce6bedba7a0d48a) )
	ROM_LOAD32_WORD( "mr94027.05",  0x800000, 0x200000, CRC(59545977) SHA1(2e0a83efd7ae210c0b4360e9572dd7eec38cd974) )
	ROM_LOAD32_WORD( "mr94027.06",  0x800002, 0x200000, CRC(1618785a) SHA1(3f2698d07a52947429313a78ebcedfdae478efd7) )
	ROM_LOAD32_WORD( "mr94027.07",  0xc00000, 0x200000, CRC(c66099c4) SHA1(5a6edffa39a98f38cc3cffbad9191fb2e794a812) )
	ROM_LOAD32_WORD( "mr94027.08",  0xc00002, 0x200000, CRC(753b05e0) SHA1(0424e92b32a73c27ecb549e6e9449446ea938e40) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94027.09",  0x000000, 0x200000, CRC(32ead437) SHA1(b94175cf186b4ebcc180a4c092d2ffcdd9ff3b1d) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94027.11",  0x000000, 0x200000, CRC(b65d5096) SHA1(2c4e1e3e9f96be8369cb2de142a82f94506f85c0) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93138_32_ver1.0.32", 0x000000, 0x080000, CRC(df5d00b4) SHA1(2bbbcd546d5b5170d81bf33b37b46b70b417c9c7) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93138_21_ver1.0.21", 0x000000, 0x040000, CRC(008bc217) SHA1(eec66a86f285ccbc47eba17a4bb83cc1f8a5f425) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042.01",  0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94027.10",  0x200000, 0x200000, CRC(e7cabe41) SHA1(5d903baed690a98856f7581319cf4dbfe1db47bb) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( kirarast )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mr95025.26", 0x000003, 0x80000, CRC(eb7faf5f) SHA1(5b79ff3043db5ef2622ae1665145462d949c9bb8) )
	ROM_LOAD32_BYTE( "mr95025.27", 0x000002, 0x80000, CRC(80644d05) SHA1(6da8bf8aeb1477112f9022c0c5f472cbcd27df8e) )
	ROM_LOAD32_BYTE( "mr95025.28", 0x000001, 0x80000, CRC(6df8c384) SHA1(3ad01d3d51cfc1f48029c16ee1cc74fc59d7603c) )
	ROM_LOAD32_BYTE( "mr95025.29", 0x000000, 0x80000, CRC(3b6e681b) SHA1(148fa10631db53a4ad1dcdfb60b4f0654e077396) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr95025.01",  0x000000, 0x200000, CRC(02279069) SHA1(fb3ce00701271d0163f72e4f2e56faa9f16d8fd0) )
	ROM_LOAD32_WORD( "mr95025.02",  0x000002, 0x200000, CRC(885161d4) SHA1(1bc82de0b2759758d437db3ef9f0f7805f759b59) )
	ROM_LOAD32_WORD( "mr95025.03",  0x400000, 0x200000, CRC(1ae06df9) SHA1(e1493a386fd8c54c88afab43d13d73869ae467ee) )
	ROM_LOAD32_WORD( "mr95025.04",  0x400002, 0x200000, CRC(91ab7006) SHA1(0b99c352a696e21b2f31207cbf9b4a64edf543ce) )
	ROM_LOAD32_WORD( "mr95025.05",  0x800000, 0x200000, CRC(e61af029) SHA1(685315e833a168383c4c5cdaf72de172f14995b6) )
	ROM_LOAD32_WORD( "mr95025.06",  0x800002, 0x200000, CRC(63f64ffc) SHA1(a2a109be24b5f1ec2e41e423d4194394ea8c3c8b) )
	ROM_LOAD32_WORD( "mr95025.07",  0xc00000, 0x200000, CRC(0263a010) SHA1(b9c85647b406c89f0e839eac93eaf5d2e6963f7d) )
	ROM_LOAD32_WORD( "mr95025.08",  0xc00002, 0x200000, CRC(8efc00d6) SHA1(f750e0e21310ceceeae3ad80eb2fe2920f5a0076) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr95025.10",  0x000000, 0x200000, CRC(ba7ad413) SHA1(b1f1c218dea3217f21d5e2f44db3786055ed879a) )
	ROM_LOAD( "mr95025.11",  0x200000, 0x200000, CRC(11557299) SHA1(6efa56f897ab026f965376a0d4032f7a0d20cafe) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr95025.09",  0x000000, 0x200000, CRC(ca6cbd17) SHA1(9d16ef187b062590315066218e89bdf33cfd9865) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mr95025.30",  0x000000, 0x080000, CRC(aee6e0c2) SHA1(dee985f7a9773ba7a4d31a3833a7775d778bbe5a) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mr95025.21",  0x000000, 0x040000, CRC(a6c70c7f) SHA1(fe2108f3e8d46ed53d8c5c98e8d0fdb19b77075d) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr95025.12",  0x000000, 0x200000, CRC(1dd4f766) SHA1(455befd3a216f2197cd2e7e4899d4f1af7d20bf7) )
	ROM_LOAD( "mr95025.13",  0x200000, 0x200000, CRC(0adfe5b8) SHA1(02309e5789b58896e5f68603502c76d4a917bd91) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( akiss )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-26.26", 0x000003, 0x80000, CRC(5bdd01ee) SHA1(21b8e07bb7ef6b437a43719b02deeba970330900) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-27.27", 0x000002, 0x80000, CRC(bb11b2c9) SHA1(86ba06d28bc8f560ac3d05515d061e05c90d1628) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-28.28", 0x000001, 0x80000, CRC(20565478) SHA1(d532ab55be287f45d8d81317bb844c675eb1292c) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-29.29", 0x000000, 0x80000, CRC(ff454f0d) SHA1(db81aaaf4160eb62badbe08fc01543463470ac97) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mb95008-01.13", 0x000000, 0x200000, CRC(1be66420) SHA1(9fc85e6108f230418e012ad05586010235139039) )
	ROM_LOAD32_WORD( "mb95008-02.1",  0x000002, 0x200000, CRC(1cc4808e) SHA1(70a19d66b4f187320c67760bc453b6afb7d66f9a) )
	ROM_LOAD32_WORD( "mb95008-03.14", 0x400000, 0x200000, CRC(4045f0dc) SHA1(5ba9786618ecad9410dbdf3664f9dda848a754f7) )
	ROM_LOAD32_WORD( "mb95008-04.2",  0x400002, 0x200000, CRC(ef3c139d) SHA1(3de374e77443dd4e967dbb5da820fe1c8c78aa1b) )
	ROM_LOAD32_WORD( "mb95008-05.15", 0x800000, 0x200000, CRC(43ea4a84) SHA1(d9d9898edcf432998ed6b9a1622812def45cf369) )
	ROM_LOAD32_WORD( "mb95008-06.3",  0x800002, 0x200000, CRC(24f23d4e) SHA1(8a7b6f28f25227391df73edb096695c5fe8df7dc) )
	ROM_LOAD32_WORD( "mb95008-07.16", 0xc00000, 0x200000, CRC(bf47747e) SHA1(b97121953f41039182e25ea023802df4524cf9bd) )
	ROM_LOAD32_WORD( "mb95008-08.4",  0xc00002, 0x200000, CRC(34829a09) SHA1(7229c56fee53a9d4d29cf0c9dec471b6cc4dc30b) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mb95008-10.11",  0x000000, 0x200000, CRC(52da2e9e) SHA1(d7a29bdd1c6801aa8d36bc098e75091c63ba0766) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mb95008-09.10",  0x000000, 0x200000,CRC(7236f6a0) SHA1(98dbb55f08d669ef3bf69394bb9739d0e6137fcb) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93166_ver1.0-30.30",  0x000000, 0x080000, CRC(1807c1ea) SHA1(94696b8319c4982cb5d33423f56e2348f210cdb5) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93166_ver1.0-21.21",  0x000000, 0x040000, CRC(01a03687) SHA1(2340c4ed19f434e8c23709edfc93259313aefaf9) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mb95008-11.22",  0x000000, 0x200000, CRC(23b9af76) SHA1(98b4087c142500dc759bda94d71c77634452a7ad) )
	ROM_LOAD( "mb95008-12.23",  0x200000, 0x200000, CRC(780a2f45) SHA1(770cbf04e34ae7d72e6eb2304bcdfaff483cd8c1) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

/*
P-47 Aces Ver 1.1  -  Observed fixes:
  Sound effects bug. Due to a timer bug, sound effects may be delayed by multiple seconds.
  The sound test in the test menu now works.
*/
ROM_START( p47aces )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "p-47_aces_3-31_rom_26_ver1.1.26", 0x000003, 0x80000, CRC(99c0e211) SHA1(6fc3b1e5ddadda85934145a2e62b55ccb2011fb5) ) /* Labeled "P-47 ACES 3/31  ROM 26 Ver1.1" */
	ROM_LOAD32_BYTE( "p-47_aces_3-31_rom_27_ver1.1.27", 0x000002, 0x80000, CRC(2a0c107a) SHA1(1d83bd55acaad62a5823f09b1683f846631fdeca) ) /* Labeled "P-47 ACES 3/31  ROM 27 Ver1.1" */
	ROM_LOAD32_BYTE( "p-47_aces_3-31_rom_28_ver1.1.28", 0x000001, 0x80000, CRC(53509d28) SHA1(44e6388ade514bb747a84bfef17f852393b44a37) ) /* Labeled "P-47 ACES 3/31  ROM 28 Ver1.1" */
	ROM_LOAD32_BYTE( "p-47_aces_3-31_rom_29_ver1.1.29", 0x000000, 0x80000, CRC(91e7b7da) SHA1(f3a0e193c59e285d97a7ea9bc92a7b3c5c009532) ) /* Labeled "P-47 ACES 3/31  ROM 29 Ver1.1" */

	ROM_REGION( 0xe00000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94020-02.1",  0x000002, 0x200000, CRC(28732d3c) SHA1(15b2687bcad31793fc7d6a9dc3eccb7ad9b5f659) )
	ROM_LOAD32_WORD( "mr94020-01.13", 0x000000, 0x200000, CRC(a6ccf999) SHA1(5d32fb6f6987ede6c125bec9581da4695ad64dff) )
	ROM_LOAD32_WORD( "mr94020-04.2",  0x400002, 0x200000, CRC(128db576) SHA1(f6561f54f6b95842a5f14d29682449bf0d837a85) )
	ROM_LOAD32_WORD( "mr94020-03.14", 0x400000, 0x200000, CRC(efc52b38) SHA1(589caaaba4e3ddaf41e05f0f12b8d4bc6d63fa5c) )
	ROM_LOAD32_WORD( "mr94020-06.3",  0x800002, 0x200000, CRC(324cd504) SHA1(79b3ef3ae0aa14d903113ccf5b57d459c329cf12) )
	ROM_LOAD32_WORD( "mr94020-05.15", 0x800000, 0x200000, CRC(ca164b17) SHA1(ea1cb0894632442f40d321b5843125f874768aae) )
	ROM_LOAD32_WORD( "mr94020-08.4",  0xc00002, 0x100000, CRC(4b3372be) SHA1(cdc7d7615b6b5d45ca071b2967980dc6c6294ac0) )
	ROM_LOAD32_WORD( "mr94020-07.16", 0xc00000, 0x100000, CRC(c23c5467) SHA1(5ff51ecb86ccbae2af160599890e13a7cc70072d) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94020-11.11", 0x000000, 0x200000, CRC(c1fe16b3) SHA1(8b9d2483ba06ab8072676e73d949c696535b3d26) )
	ROM_LOAD( "mr94020-12.12", 0x200000, 0x200000, CRC(75871325) SHA1(9191263a52ec6ac325cf6130b35be7cdd1ec2f50) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94020-10.10", 0x000000, 0x200000, CRC(a44e9e06) SHA1(ff51796e160d996e931b92049e6214982f270caa) ) /* unlike other sets, the ROM number & socket number match - it's correct */
	ROM_LOAD( "mr94020-09.9",  0x200000, 0x200000, CRC(226014a6) SHA1(090bdc1f6d2b9d33b431dbb49a457a4bb36cd3ad) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "p-47_ver1.0-30.30", 0x000000, 0x080000, CRC(7ba90fad) SHA1(c0a3d4458816f00b8f5eb4b6d4531d1abeaccbe5) ) /* Labeled "P-47 Ver1.0 -30" */

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "p-47_ver1.0-21.21", 0x000000, 0x040000, CRC(f2d43927) SHA1(69ac20f339a515d58cafbcd6f7d7982ca5cda681) ) /* Labeled "P-47 Ver1.0 -21" */

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94020-13.23", 0x200000, 0x200000, CRC(547fa4d4) SHA1(8a5ecb3300646762f63d37a27e643e1f6ce5e775) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( p47acesa )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "p47_ver1.0-26.26", 0x000003, 0x80000, CRC(e017b819) SHA1(942fb48e8bb3a263534a0351a1a9979d786bc475) ) /* Labeled "P-47 Ver1.0 -26" */
	ROM_LOAD32_BYTE( "p47_ver1.0-27.27", 0x000002, 0x80000, CRC(bd1b81e0) SHA1(b15f157fe3a30295f999a4c285da2d6f22d7fba6) ) /* Labeled "P-47 Ver1.0 -27" */
	ROM_LOAD32_BYTE( "p47_ver1.0-28.28", 0x000001, 0x80000, CRC(4742a5f7) SHA1(cd297aa150082c545647c9a755cf2cdbdc98c988) ) /* Labeled "P-47 Ver1.0 -28" */
	ROM_LOAD32_BYTE( "p47_ver1.0-29.29", 0x000000, 0x80000, CRC(86e17d8b) SHA1(73004f243c6dfb86ce4cc61475dc7caaf452750e) ) /* Labeled "P-47 Ver1.0 -29" */

	ROM_REGION( 0xe00000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr94020-02.1",  0x000002, 0x200000, CRC(28732d3c) SHA1(15b2687bcad31793fc7d6a9dc3eccb7ad9b5f659) )
	ROM_LOAD32_WORD( "mr94020-01.13", 0x000000, 0x200000, CRC(a6ccf999) SHA1(5d32fb6f6987ede6c125bec9581da4695ad64dff) )
	ROM_LOAD32_WORD( "mr94020-04.2",  0x400002, 0x200000, CRC(128db576) SHA1(f6561f54f6b95842a5f14d29682449bf0d837a85) )
	ROM_LOAD32_WORD( "mr94020-03.14", 0x400000, 0x200000, CRC(efc52b38) SHA1(589caaaba4e3ddaf41e05f0f12b8d4bc6d63fa5c) )
	ROM_LOAD32_WORD( "mr94020-06.3",  0x800002, 0x200000, CRC(324cd504) SHA1(79b3ef3ae0aa14d903113ccf5b57d459c329cf12) )
	ROM_LOAD32_WORD( "mr94020-05.15", 0x800000, 0x200000, CRC(ca164b17) SHA1(ea1cb0894632442f40d321b5843125f874768aae) )
	ROM_LOAD32_WORD( "mr94020-08.4",  0xc00002, 0x100000, CRC(4b3372be) SHA1(cdc7d7615b6b5d45ca071b2967980dc6c6294ac0) )
	ROM_LOAD32_WORD( "mr94020-07.16", 0xc00000, 0x100000, CRC(c23c5467) SHA1(5ff51ecb86ccbae2af160599890e13a7cc70072d) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr94020-11.11", 0x000000, 0x200000, CRC(c1fe16b3) SHA1(8b9d2483ba06ab8072676e73d949c696535b3d26) )
	ROM_LOAD( "mr94020-12.12", 0x200000, 0x200000, CRC(75871325) SHA1(9191263a52ec6ac325cf6130b35be7cdd1ec2f50) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr94020-10.10", 0x000000, 0x200000, CRC(a44e9e06) SHA1(ff51796e160d996e931b92049e6214982f270caa) ) /* unlike other sets, the ROM number & socket number match - it's correct */
	ROM_LOAD( "mr94020-09.9",  0x200000, 0x200000, CRC(226014a6) SHA1(090bdc1f6d2b9d33b431dbb49a457a4bb36cd3ad) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "p-47_ver1.0-30.30", 0x000000, 0x080000, CRC(7ba90fad) SHA1(c0a3d4458816f00b8f5eb4b6d4531d1abeaccbe5) ) /* Labeled "P-47 Ver1.0 -30" */

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "p-47_ver1.0-21.21", 0x000000, 0x040000, CRC(f2d43927) SHA1(69ac20f339a515d58cafbcd6f7d7982ca5cda681) ) /* Labeled "P-47 Ver1.0 -21" */

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr94020-13.23", 0x200000, 0x200000, CRC(547fa4d4) SHA1(8a5ecb3300646762f63d37a27e643e1f6ce5e775) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( tetrisp )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-26.26", 0x000003, 0x80000, CRC(d318a9ba) SHA1(cae86d86518fdfeb736e7b2040277c76cc3b4017) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mb93166_ver1.0-27.27", 0x000002, 0x80000, CRC(2d69b6d3) SHA1(f0a513f449aa25808672fb27e3691ccabfba48a1) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-28.28", 0x000001, 0x80000, CRC(87522e16) SHA1(4f0d8abec046884d89c559e3a4a5ac9e0e47a0dc) )
	ROM_LOAD32_BYTE( "mb93166_ver1.0-29.29", 0x000000, 0x80000, CRC(43a61941) SHA1(a097c88c45d8486eb6ffdd13904b6eb2a3fa45b9) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr95024-01.01", 0x000002, 0x200000, CRC(cb0e92b9) SHA1(179cc9e2d819d7f6238e924184e8a383d172aa72) )
	ROM_LOAD32_WORD( "mr95024-02.13", 0x000000, 0x200000, CRC(4a825990) SHA1(f99ba9f88f5582259ba0e50480451d4e9d1d03b7) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr95024-04.11", 0x000000, 0x200000, CRC(c0d5246f) SHA1(413285f6b40001281c4fcec1ce73400c3ae610ed) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr95024-03.10", 0x000000, 0x200000, CRC(a03e4a8d) SHA1(d52c78d5e9d874dce514ffb035f2424409d8fb7a) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mb93166_ver1.0-30.30", 0x000000, 0x080000, CRC(cea7002d) SHA1(5462edaeb9339790b95ed15a4bfaab8fae655b12) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93166_ver1.0-21.21", 0x000000, 0x040000, CRC(5c565e3b) SHA1(d349a8ca50d03c06d8978e6d3632b624f019dee4) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr95024-05.23", 0x200000, 0x200000, CRC(57502a17) SHA1(ce880188854dc17d9ebbfa3c373469cf5e6858c2) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

ROM_START( tp2m32 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mp2_26.ver1.0.26", 0x000003, 0x80000, CRC(152f0ccf) SHA1(1e318e125a54216ebf3f85740db1dd85aacac819) ) /* labeled M  P2 26 Ver1.0  - game name in Kanji between M and P2 - uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "mp2_27.ver1.0.27", 0x000002, 0x80000, CRC(d89468d0) SHA1(023fbc13b0f6332217904c89225b330aa5742f20) ) /* labeled M  P2 27 Ver1.0  - game name in Kanji between M and P2 */
	ROM_LOAD32_BYTE( "mp2_28.ver1.0.28", 0x000001, 0x80000, CRC(041aac23) SHA1(3f7863ffa897978493e98445fe020dccbe521752) ) /* labeled M  P2 28 Ver1.0  - game name in Kanji between M and P2 */
	ROM_LOAD32_BYTE( "mp2_29.ver1.0.29", 0x000000, 0x80000, CRC(4e83b2ca) SHA1(2766793f050a6952f4f53a763686f95bd7544f3f) ) /* labeled M  P2 29 Ver1.0  - game name in Kanji between M and P2 */

	ROM_REGION( 0x800000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr96019-01.13", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "mr96019-02.1",  0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr96019-04.11", 0x000000, 0x200000, CRC(b5a03129) SHA1(a50d8b70615c49216f647534d1658c1a6d58a783) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr96019-03.10", 0x000000, 0x400000, CRC(94af8057) SHA1(e3bc6e02fe4c503ae51284770a76abbeff989147) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "mp2_30.ver1.0.30", 0x000000, 0x080000, CRC(6845e476) SHA1(61c33714db2e2b5ccdcef0e0d3efdc391fe6aba2) ) /* labeled M  P2 30 Ver1.0  - game name in Kanji between M and P2 */

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mp2_21.ver1.0.21", 0x000000, 0x040000, CRC(2bcc4176) SHA1(74740fa13ab81b9819b4cfbe9d34a0749ba23b8f) ) /* labeled M  P2 21 Ver1.0  - game name in Kanji between M and P2 */

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr96019-05.22", 0x000000, 0x200000, CRC(74aa5c31) SHA1(7e3f86198fb678244fab76bee9c72bbdfc818118) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END



ROM_START( bnstars ) /* ver 1.1 */
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "vsjanshi_26_ver1.1.26", 0x000003, 0x80000, CRC(75eeec8f) SHA1(26315381baa0abb470203dc565ad98c52fe17b20) ) /* uses MB-94166 EB91022-20101 rom board */
	ROM_LOAD32_BYTE( "vsjanshi_27_ver1.1.27", 0x000002, 0x80000, CRC(69f24ab9) SHA1(e019a444111e4ed7f9a378d6e2d13ddb9324bc49) )
	ROM_LOAD32_BYTE( "vsjanshi_28_ver1.1.28", 0x000001, 0x80000, CRC(d075cfb6) SHA1(f70741e9f536d5c7604126d36c7aa8ed8f25c329) )
	ROM_LOAD32_BYTE( "vsjanshi_29_ver1.1.29", 0x000000, 0x80000, CRC(bc395b50) SHA1(84d7cc492a11a5a9402e929f0bd138ad63e3d079) )

	ROM_REGION( 0x1000000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr96004-01.13", 0x000000, 0x200000, CRC(3366d104) SHA1(2de0cabe2ead777b5b02cade7f2003ef7f90b75b) )
	ROM_LOAD32_WORD( "mr96004-02.1",  0x000002, 0x200000, CRC(ad556664) SHA1(4b36f8d8d9efa37cf515af41d14433e7eafa27a2) )
	ROM_LOAD32_WORD( "mr96004-03.14", 0x400000, 0x200000, CRC(b399e2b1) SHA1(9b6a00a219db8d66dcf592160b7b5f7a86b8f0c9) )
	ROM_LOAD32_WORD( "mr96004-04.2",  0x400002, 0x200000, CRC(f4f4cf4a) SHA1(fe497989cf96c68602f68f14920aed44fd934573) )
	ROM_LOAD32_WORD( "mr96004-05.15", 0x800000, 0x200000, CRC(cd6c357e) SHA1(44cd2d0607c7ccd80f701cf1675fd283acb07252) )
	ROM_LOAD32_WORD( "mr96004-06.3",  0x800002, 0x200000, CRC(fc6daad7) SHA1(99f14ac6b06ad9a8a3d2e9f69b693c7ce420a47d) )
	ROM_LOAD32_WORD( "mr96004-07.16", 0xc00000, 0x200000, CRC(177e32fa) SHA1(3ca1f397dc28f1fa3a4136705b92c63e4e438f05) )
	ROM_LOAD32_WORD( "mr96004-08.4",  0xc00002, 0x200000, CRC(f6df27b2) SHA1(60590976020d86bdccd4eaf57b349ea31bec6830) )

	ROM_REGION( 0x400000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr96004-09.11",  0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr96004-11.10", 0x000000, 0x200000,  CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "vsjanshi_30_ver1.0.30",  0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "vsjanshi_21_ver1.0.21",  0x000000, 0x040000, CRC(d622bce1) SHA1(059fcc3c7216d3ea4f3a4226a06219375ce8c2bf) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.22",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END

/*

World PK Soccer V2
(c)1996 Jaleco

MegaSystem32 with I/O subboard OZ-93155

ROM board:
MB-93138A EB91022-20078-1

25 (actual label is "PK SOCCER V2 ROM 25 VER. 1.1") IC36
27 (actual label is "PK SOCCER V2 ROM 27 VER. 1.1") IC38
29 (actual label is "PK SOCCER V2 ROM 29 VER. 1.1") IC40
31 (actual label is "PK SOCCER V2 ROM 31 VER. 1.1") IC42
32 (actual label is "PK SOCCER V2 ROM 32 VER. 1.1") IC43

MR92042-01.22 (16M mask) IC33
MR92042-08.23 (16M mask) IC34

ws-21 25 (actual label is "MB93138 Ver1.0 WS-21") IC30

MR95033-01.13 (16M mask) IC20
MR95033-02.1  (16M mask) IC3
MR95033-03.14 (16M mask) IC21
MR95033-04.2  (16M mask) IC4
MR95033-05.15 (16M mask) IC22
MR95033-06.3  (16M mask) IC5

MR95033-07.9  (16M mask) IC11

MR95033-09.11 (16M mask) IC13

Daughter board:
SE-93139 EB91022-30056
Custom chip: SS92046-01 9338EV 436091 06441

*/

ROM_START( wpksocv2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "pk_soccer_v2_rom_25_ver._1.1.25", 0x000003, 0x80000, CRC(6c22a56c) SHA1(a03cbcfc024b39d2776f9e9897d1da07df6ae2d7) ) /* uses MB-93138A EB91022-20078-1 rom board */
	ROM_LOAD32_BYTE( "pk_soccer_v2_rom_27_ver._1.1.27", 0x000002, 0x80000, CRC(50c594a8) SHA1(454a63d7b2a07399a64449205271b797bca1dec1) )
	ROM_LOAD32_BYTE( "pk_soccer_v2_rom_29_ver._1.1.29", 0x000001, 0x80000, CRC(22acd835) SHA1(0fa96a6dfde737d541842f85dc257776044e15b5) )
	ROM_LOAD32_BYTE( "pk_soccer_v2_rom_31_ver._1.1.31", 0x000000, 0x80000, CRC(f25e50f5) SHA1(b58722f11a8b94ef053caf531ac94a959350288a) )

	ROM_REGION( 0xc00000, "sprite", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr95033-01.13", 0x000000, 0x200000, CRC(1f76ed57) SHA1(af9076b4b4c26b362825d892f46d2c04b4bb9d07) )
	ROM_LOAD32_WORD( "mr95033-02.1",  0x000002, 0x200000, CRC(5b119910) SHA1(aff44e355227dd159e388ab85a5b6d48644ff421) )
	ROM_LOAD32_WORD( "mr95033-03.14", 0x400000, 0x200000, CRC(8b6099ed) SHA1(c514cec1491aed00a5714c0b8d17c96e87ba50aa) )
	ROM_LOAD32_WORD( "mr95033-04.2",  0x400002, 0x200000, CRC(59144dc6) SHA1(0e192001d668791c91ca2af6b367067a5106a4b2) )
	ROM_LOAD32_WORD( "mr95033-05.15", 0x800000, 0x200000, CRC(cc5b8d0b) SHA1(70a5b9db600fc168d13ad54653cf1c8d2a45d991) )
	ROM_LOAD32_WORD( "mr95033-06.3",  0x800002, 0x200000, CRC(2f79942f) SHA1(73417d10f37bcd539b8081312226cf142a5a0d3d) )

	ROM_REGION( 0x200000, "roztiles", 0 ) /* roz tiles */
	ROM_LOAD( "mr95033-07.9", 0x000000, 0x200000, CRC(76cd2e0b) SHA1(41aa18dfb4e06547d1f6d7ce49e5225027d16bbb) )

	ROM_REGION( 0x200000, "bgtiles", 0 ) /* bg tiles */
	ROM_LOAD( "mr95033-09.11", 0x000000, 0x200000, CRC(8a6dae81) SHA1(e235f2865a9a003330bff1e4d0a017e5d10efd2a) )

	ROM_REGION( 0x080000, "txtiles", 0 ) /* tx tiles */
	ROM_LOAD( "pk_soccer_v2_rom_32_ver._1.1.32", 0x000000, 0x080000, CRC(becc25c2) SHA1(4ae7665cd45ebd9586068e99327145194ba216fc) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "mb93139_ver1.0_ws-21.21", 0x000000, 0x040000, CRC(bdeff5d6) SHA1(920a6fc983d53f09510887e4e81ee89ccd5079e6) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* samples */
	ROM_LOAD( "mr92042-01.22", 0x000000, 0x200000, CRC(0fa26f65) SHA1(e92b14862fbce33ea4ab4567ec48199bfcbbdd84) ) // common samples
	ROM_LOAD( "mr95033-08.23", 0x200000, 0x200000, CRC(89a291fa) SHA1(7746a0490134fc902ce2dc7b0d33b455d792c105) )

	ROM_REGION( 0x000001, "motherbrd_pals", 0) /* Motherboard PAL */
	ROM_LOAD( "91022-01.ic83", 0x00000, 0x00001, NO_DUMP ) /* AMI 18CV8-15. */
ROM_END


void ms32_base_state::configure_banks()
{
	save_item(NAME(m_to_main));
	for (int bank = 0; bank < 2; bank++)
		m_z80bank[bank]->configure_entries(0, 16, memregion("audiocpu")->base() + 0x4000, 0x4000);

}

void ms32_state::init_ms32_common()
{
	m_nvram_8 = std::make_unique<u8[]>(0x2000);
	configure_banks();
}

/* SS91022-10: desertwr, gratiaa, tp2m32, gametngk */
void ms32_state::init_ss91022_10()
{
	init_ms32_common();
	decrypt_ms32_tx(machine(), 0x00000,0x35, "txtiles");
	decrypt_ms32_bg(machine(), 0x00000,0xa3, "bgtiles");
}

/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi2 */
void ms32_state::init_ss92046_01()
{
	init_ms32_common();
	decrypt_ms32_tx(machine(), 0x00020,0x7e, "txtiles");
	decrypt_ms32_bg(machine(), 0x00001,0x9b, "bgtiles");
}

/* SS92047-01: gratia, kirarast */
void ms32_state::init_ss92047_01()
{
	init_ms32_common();
	decrypt_ms32_tx(machine(), 0x24000,0x18, "txtiles");
	decrypt_ms32_bg(machine(), 0x24000,0x55, "bgtiles");
}

/* SS92048-01: p47aces, suchie2, suchie2o */
void ms32_state::init_ss92048_01()
{
	init_ms32_common();
	decrypt_ms32_tx(machine(), 0x20400,0xd6, "txtiles");
	decrypt_ms32_bg(machine(), 0x20400,0xd4, "bgtiles");
}

void ms32_state::init_kirarast()
{
	init_ss92047_01();
}

void ms32_state::init_suchie2()
{
	init_ss92048_01();
}

void ms32_f1superbattle_state::init_f1superb()
{
#if 0
	// hack for ?, game needs FPUs emulated anyway, eventually remove me
	u32 *pROM = (u32 *)memregion("maincpu")->base();
	pROM[0x19d04/4]=0x167a021a; // bne->br  : sprite Y offset table is always copied to RAM
	// the x offsets are never copied either ...
#endif
	init_ss92046_01();
}

void ms32_state::init_bnstars()
{
	init_ss92046_01();
}

/********** GAME DRIVERS **********/


// TODO: inputs in akiss, bnstars (former has no dip display in service mode)
GAME( 1994, hayaosi2,  0,        ms32,              hayaosi2, ms32_state,               init_ss92046_01, ROT0,   "Jaleco",        "Hayaoshi Quiz Grand Champion Taikai", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, hayaosi3,  0,        ms32,              hayaosi3, ms32_state,               init_ss92046_01, ROT0,   "Jaleco",        "Hayaoshi Quiz Nettou Namahousou (ver 1.5)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, hayaosi3a, hayaosi3, ms32,              hayaosi3, ms32_state,               init_ss92046_01, ROT0,   "Jaleco",        "Hayaoshi Quiz Nettou Namahousou (ver 1.2)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, bbbxing,   0,        ms32,              bbbxing,  ms32_state,               init_ss92046_01, ROT0,   "Jaleco",        "Best Bout Boxing (ver 1.3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1994, suchie2,   0,        ms32,              suchie2,  ms32_state,               init_suchie2,    ROT0,   "Jaleco",        "Idol Janshi Suchie-Pai II (ver 1.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, suchie2o,  suchie2,  ms32,              suchie2,  ms32_state,               init_suchie2,    ROT0,   "Jaleco",        "Idol Janshi Suchie-Pai II (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, desertwr,  0,        ms32,              desertwr, ms32_state,               init_ss91022_10, ROT270, "Jaleco",        "Desert War / Wangan Sensou (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, gametngk,  0,        ms32,              gametngk, ms32_state,               init_ss91022_10, ROT270, "Jaleco",        "The Game Paradise - Master of Shooting! / Game Tengoku - The Game Paradise (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, tetrisp,   0,        ms32,              tetrisp,  ms32_state,               init_ss92046_01, ROT0,   "Jaleco / BPS",  "Tetris Plus (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, p47aces,   0,        ms32,              p47aces,  ms32_state,               init_ss92048_01, ROT0,   "Jaleco",        "P-47 Aces (ver 1.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, p47acesa,  p47aces,  ms32,              p47aces,  ms32_state,               init_ss92048_01, ROT0,   "Jaleco",        "P-47 Aces (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, akiss,     0,        ms32,              suchie2,  ms32_state,               init_kirarast,   ROT0,   "Jaleco",        "Mahjong Angel Kiss (ver 1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, gratia,    0,        ms32,              gratia,   ms32_state,               init_ss92047_01, ROT0,   "Jaleco",        "Gratia - Second Earth (ver 1.0, 92047-01 version)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, gratiaa,   gratia,   ms32,              gratia,   ms32_state,               init_ss91022_10, ROT0,   "Jaleco",        "Gratia - Second Earth (ver 1.0, 91022-10 version)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, kirarast,  0,        ms32,              kirarast, ms32_state,               init_kirarast,   ROT0,   "Jaleco",        "Ryuusei Janshi Kirara Star", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, tp2m32,    tetrisp2, ms32_invert_lines, tp2m32,   ms32_state,               init_ss91022_10, ROT0,   "Jaleco",        "Tetris Plus 2 (ver 1.0, MegaSystem 32 Version)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, bnstars,   bnstars1, ms32,              suchie2,  ms32_state,               init_bnstars,    ROT0,   "Jaleco",        "Vs. Janshi Brandnew Stars (Ver 1.1, MegaSystem 32 Version)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, wpksocv2,  0,        ms32_invert_lines, wpksocv2, ms32_state,               init_ss92046_01, ROT0,   "Jaleco",        "World PK Soccer V2 (ver 1.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

/* these boot and show something */
GAMEL(1994, f1superb,  0,        f1superb,          f1superb, ms32_f1superbattle_state, init_f1superb,   ROT0,   "Jaleco",        "F-1 Super Battle", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_f1superb )
