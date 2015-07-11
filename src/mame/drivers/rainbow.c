// license:BSD-3-Clause
// copyright-holders:Mike Coates
/***************************************************************************

Rainbow Islands  (c) Taito 1987   + Jumping
===============

driver by Mike Coates

c-chip enhanced by Robert Gallagher, with many thanks to Tormod Tjaberg for providing
his PCB for dumping/collecting the c-chip data from Rainbow Islands Extra.

                            ***

Rainbow Islands & Rainbow Islands Extra
Taito, 1987

PCB Layout
----------

MAIN PCB
J1100128A
K1100277A
|----------------------------------------------------|
| MB3735 Z80 B22-14.43    16MHz      DIP28    DIP28  |
|       YM2151  5563                                 |
|VOL          PC060HA    B22-05.33          B22-01.2 |
| 4556  TL074 YM3012               |-------|         |
|Y   6116 6116                     |TAITO  |   43256 |
|                                  |PC080SN|         |
|J    TC0070RGB  2018   |-------|  |-------|   43256 |
|A               2018   |TAITO  |                    |
|M     PC050CM   2018   |PC0900J|           B22-02.5 |
|M               2018   |-------|           B22-13.6 |
|A                                          B22-12.7 |
|        TC0030CMD  12MHz                        3771|
|  26.686MHz                   B39-02.20  B22-01.19  |
|        68000                 B39-04.22  B39-03.21  |
|        B22-07.56  B22-06.41  B22-04.24  B22-03.23  |
|   DSWB   DSWA                8464       8464       |
|----------------------------------------------------|
Notes:
      68000       - clock 8.000MHz [16/2]
      Z80         - clock 4.000MHz [16/4]
      YM2151      - clock 4.000MHz [16/4]
      6116        - 2kx8 SRAM (color RAM)
      43256       - 32kx8 SRAM (tile RAM)
      8464        - 8kx8 SRAM (68000 work RAM)
      5563        - 8kx8 SRAM (Z80 work RAM)
      2018        - 2kx8 SRAM (sprite RAM)
      4556        - NEC uPC4556 dual operational amplifier IC (DIP8)
      TL074       - Texas Instruments TL074 low noise j-fet quad operational amplifier (DIP14)
      YM3012      - Yamaha YM3012 2-channel serial input floating DAC (DIP16)
      MB3735      - Fujitsu MB3735 20 watt BTL audio power amplifier (SIP9)
      PC080SN     - Taito custom tilemap generator IC (connected to 43256 SRAM)
      PC0900J     - Taito custom sprite generator IC (connected to 2018 SRAM)
      PC060HA     - Taito custom IC (sound-related)
      TC0030CMD   - Taito custom C-Chip marked 'TC0030CMD, clock input 12.000MHz on pin 20
                      - For Rainbow Islands with sticker 'B22-15' at IC53
                      - For Rainbow Islands Extra with sticker 'B39-05' at IC53
      TC0070RGB   - Taito custom ceramic module RGB mixer IC
      PC050CM     - Taito custom ceramic module (input related functions)
      3771        - Fujitsu MB3771 master reset IC (DIP8)
      Y           - 3 pin connector for 2nd speaker output
      DIP28       - unpopulated DIP28 socket(s)

      ROMs -
         |----------------------------------------------------------------------|
         | Common to Rainbow Islands and Rainbow Islands Extra                  |
         |--------------|---------------|---------------|-----------------------|
         |   234000     |   23C1000     |   27C512      |   PALs                |
         |--------------|---------------|---------------|-----------------------|
         |   B22-01.2   |   B22-03.23   |   B22-12.7    |   B22-05.33 (PAL16L8) |
         |   B22-02.5   |   B22-04.24   |   B22-13.6    |   B22-06.41 (PAL16L8) |
         |              |               |   B22-14.43   |   B22-07.56 (PAL20L8) |
         |--------------|---------------|---------------|-----------------------|
         |---------------------------------------|------------------------------|
         | Used only on Rainbow Islands Extra    | Used only on Rainbow Islands |
         |--------------|------------------------|--------------|---------------|
         |   27C512     |  C-Chip                |   27C512     |  C-Chip       |
         |--------------|------------------------|--------------|---------------|
         |   B39-01.19  |  B39-05.53             |   B22-10.19  |  B22-15.53    |
         |   B39-02.20  |                        |   B22-11.20  |               |
         |   B39-03.21  |                        |   B22-08.21  |               |
         |   B39-04.22  |                        |   B22-09.22  |               |
         |--------------|------------------------|--------------|---------------|

      Measurements
      ------------
      XTAL1 - 26.68599MHz
      XTAL2 - 16.00000MHz
      XTAL3 - 12.00000MHz
      VSync - 60.0559Hz
      HSync - 15.6745kHz

Notes on Rainbow Islands romsets by Robert Gallagher
----------------------------------------------------

There are 3 code segments that differ between Rainbow Islands old and new
version. They are all related to secret rooms;

   The first code segment is entered at the start of a secret room.


         rbislando                             rbisland
   $55EE lea    $C01308,a0              $55EE lea    $C00B08,a0
         move.w #$13,d0                       bsr    $561C
   $55F8 move.w #$15fa,2(a0)                  lea    $C00D08,a0
         move.w #$15fa,6(a0)                  bsr    $561C
         move.w #$15fa,$102(a0)               lea    $C00F08,a0
         move.w #$15fa,$106(a0)               bsr    $561C
         adda.l #8,a0                         lea    $C01108,a0
         dbf    d0,$55F8                      bsr    $561C
         rts                            $561C lea    $C01308,a0
                                              move.w #$13,d0
                                              ...

   The next code segment fixes the bonus related to entering ALL 10 Secret rooms.
   In Rainbow Islands, If you enter the last secret room, you receive a 10,000,000
   point bonus. If you have entered all 10 secret rooms, you receive a 50,000,000 point
   bonus. (BUG THAT'S NOT A BUG - the games says 1,000,000 and 5,000,000, but the code gives
   you 10mil and 50mil)
   The counter is in $10D05C.b, the (rbislando) romset never increments this counter, making
   it impossible to achieve.

   $56A2 cmpi.w #$31,$11c6(a5)          $56CA addi.b #1,$105C(a5)   ;increment the secret room count
         bne                                  cmpi.w #$31,$11c6(a5) ;is it round 49 (50)?
                                              bne

   The final change fixes a Secret room bug that was noted by Stefan Jokisch.
   In (rbislando) it is possible to scroll the screen inside a secret room.
   You can walk right off the top of the screen, and scroll the 'next' secret
   room into view. This is fixed in (rbisland) with the following code;

   $5F06 move.b #1,d0                   $5F34 tst.b  $11c4(a5)  ; in secret room?
         jsr    $1736                         bne    $600A      ; exit this routine, which is getting
                                              move.b #1,d0      ; the room height from c-chip bank 1
                                              jsr    $1736      ; which we don't need
                                              ...
                                        $600A rts

   The (jumping) bootleg is based on the (rbislando) roms.

Notes on Rainbow Islands Extra by Robert Gallagher
--------------------------------------------------
   In Rainbow Islands Extra, there are more changes than just the differing enemies/rounds.

   In RIE if you end a level with the same 2 digits in your score as xxxx220, you will receive
   items at the "GOAL IN" worth 3000 points each (instead of 500pts) as:

   00 = french fries
   11 = neopolaton ice cream
   22 = creme caramel
   33 = hamburgers
   44 = cake slice
   55 = iced bun
   66 = mug of beer
   77 = hotdog on a stick
   88 = vanilla ice cream
   99 = blue popsicle

   There are two other possibilities:
   moneybags - worth 500pts if you don't have double digits, or worth 3000 points if you do (any)
               if you got the 'Money Coin' powerup in a secret room.
   hearts - worth 10,000 each, this is based on a timer in 0x0010DBA4 && 128 == 0, making this a
            1 in 128 chance.


   The items received in 'Secret Room' are not the same as Rainbow Islands. They are based on
   the LAST diamond you collect in the round before entering. That is; you must still collect
   the 7 diamonds in order, Red, Orange, Yellow... to enter a secret room, however, AFTER completing
   this, you can still collect diamonds. If you collect (last), you get:

   Red - Book of Continues - this allows you to continue after world 7
   Orange - Money Bag - this will make a 100,000pt Money bag fall from the sky on every round.
            it will also make all fruit at 'GOAL IN' into money bags.
   Yellow - Key - when taken, this will show the Secret room code completely for about 8 seconds.
   Green - Protection Fairy, gives you the protection fairy for about 8 seconds at the beginning
           of each round, or after you die. (NOT perm. like RI, code at $C528 in RIE handles this,
           and is distinctly different from the code at $BC8C in RI)
   Blue - Yellow Potion, gives you perm. fast rainbow power
   Indigo - Red Potion, gives you perm. 2X rainbow power
   Violet - Shoes, gives you perm. fast feet.

   Unlike the secret rooms in Rainbow Islands, in Rainbow Islands Extra, only 2 letters of the code
   for that room are revealed. The first letter, and the second letter will corespond to the last
   diamond collected before entering the secret room. (You can see this with the coloured hearts
   that remain). The idea is that you must enter the secret room 7X to get the full code, making the
   game harder. Otherwise, collecting a yellow diamond will give you the 'key' to the room, and reveal
   the code for about 5 seconds.

   In the secret rooms of RIE there is an added bonus; If you exit the room without collecting the
   power-up item, you will receive a 1,000,000pt bonus. The text in the room in Japanese reads:
   "KYOUKA-SOUBI WO TORAZUNI DERUTO 1000000 TEN HAIRUYO !!"

   If you enter all 10 secret rooms in Rainbow Islands Extra, and do NOT collect the powerups in
   _any_ of the rooms, you will receive "SPECIAL BIG BONUS !! 50000000 PTS."

   If you enter all 10 secret rooms, and do NOT collect the bubble-rainbow power in secret room 10,
   (though you may collect them in others) you will receive "SPECIAL BIG BONUS !! 10000000 PTS."

   If you collect the bubble-rainbow power in secret room 10, you will receive "ALL ROOM CLEAR
   5000000" bonus instead.

   In Rainbow Islands Extra (as in Rainbow Islands), if you collect 2 of any colour 'cane'...
     (canes are collected for collecting 7 of one colour of diamond, WITHOUT 'complete' happening, or
     if 'complete', you must collect 7 of the same colour diamond within one round. (the diamond
     counters are reset to one at the beginning of each round once 'complete' has occurred).
   ...you will receive a 'Potion' (not Rainbow powerup potions)
   of the appropriate colour. Once collected, you will receive a
   100,000pts. large fruit item at 'GOAL IN' for that round. Although the code for this is present in
   Rainbow Islands, it is rarely seen. This is not a result of poor emulation, but rather poor coding;
   In both RI and RIE, there are tables that indicate which special item is deserved, and what actions
   are required to earn them. (the number of times the event must occur).
   In both RI and RIE, you receive a special item for every 3rd enemy you kill as:
   1. Shoes
   2. Red Potion
   3. Yellow Potion
   4. Red Potion
   5. Crystal Ball
   6. Yellow star
   7. Red Star
   8. - Special item - depending on what you deserve.

   The Special item table can be found at $ADC0 in RI, and $B5E8 in RIE. This table is checked in
   linear fashion until a special item requirement is met.
   items 24/25/26 are: Blue Ring/Violet Ring/Red Ring.
   To get these items you collect: Red Potion/Yellow Potion/Shoes. (note that each potion collect =+2)
   In Rainbow Islands, this table shows that we need only collect:
   3 red potions (but each one counts as +2 if you created it, or +1 if you 'found' it in a level),
   3 yellow potions ""
   3 red shoes (only count as one)
   The 'canes', and 'potions' occur -after- these entries (cane index 27-33, potion index 34-40)
   So the reason that you (rarely!) see the canes, and hence the potions in RI, is because you will
   almost ALWAYS satisfy at least one of these requirements: 2 RED potions are given before 1 'special'
   In Rainbow Islands Extra however, the canes are more common. This is because the index shows that
   we need to collect 12/12/12 of the above before we can get the rings.

   One more condition has been added to RIE regards this;
   if _no_ special item has been earned, you will receive a Potion that is the same colour as the
   LAST diamond you have collected in the round.



Bugs in Jumping
---------------

The bootleggers didn't defeat the protection completely: Secret
rooms are broken, some dying enemy sprites have obviously wrong
graphics, monsters are falling through platforms etc.


Secret rooms in Rainbow Islands
-------------------------------

Getting the small diamonds in order (red through to purple) opens
a secret door in the boss room. The trick is to turn an enemy into
a diamond of a specific color: It depends on its x position and
direction. There is a cheat code at the top of each secret room
that can be entered on the copyright screen:

    L -> left
    R -> right
    J -> jump
    B -> rainbow
    S -> start

                  |  regular   |  extra
    --------------+------------+------------
    Fast Feet     |  BLRBJSBJ  |  SLLSRJRR
    Red Potion    |  RJSBJLBR  |  JLSSSBRJ
    Yellow Potion |  SSSLLRRS  |  BRSLJSLJ
    Hint A        |  BJBJBJRS  |  BJBJBJRS
    Hint B        |  LJLSLBLS  |  LJLSLBLS
    Continue      |  LBSJRLJL  |  LJLRSJJJ
    Money Bags    |  RRLLBBJS  |  LLSBRRJB
    Money + Cont  |  RRRRSBSJ  |  RSJRLBRS
    Hint C        |  SJBLRJSR  |  SJBLRJSR
    100M Counter  |  SRBJSLSB  |  BBSSJJJJ
    Hint D        |  N/A       |  BBJJRLSL
    Hint E        |  N/A       |  LRSLRSJR
    Hint F        |  N/A       |  SBJLSBRR



Stephh's notes (based on the game M68000 code and some tests) :

1) 'rbisland*'

  - Region stored at 0x02fffe.w
  - Sets :
      * 'rbisland'  : region = 0x0000
      * 'rbislando' : region = 0x0000
      * 'rbislande' : region = 0x0000
  - Possible regions :
      * 0x0000 : Japan
      * 0x0001 : US
      * 0x0002 : World
      * 0x0003 : US, licensed to ROMSTAR
      * 0x0004 : licensed to PHOENIX ELECTRONICS CO.
  - Coinage relies on "Coin Mode" Dip Switch (code at 0x0030fc) :
      * "Mode A" uses TAITO_COINAGE_JAPAN_OLD
      * "Mode B" uses TAITO_COINAGE_WORLD


2) 'jumping'

  - Region stored at 0x02fffe.w
  - Sets :
      * 'jumping' : region = 0x0000
  - If you change the region, you'll notice that the copyright strings
    haven't been correcly erased !
  - Some Dip Switches have no effect due to code at 0x000850;
    this means the following things :
      * always upright cabinet
      * screen never flipped
      * impossible to enter the "test mode"
      * always English language

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "includes/rainbow.h"



WRITE16_MEMBER(rbisland_state::jumping_sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_jumping_latch = data & 0xff; /*M68000 writes .b to $400007*/
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}


/***************************************************************************
                            MEMORY STRUCTURES
***************************************************************************/

static ADDRESS_MAP_START( rbisland_map, AS_PROGRAM, 16, rbisland_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x10c000, 0x10ffff) AM_RAM             /* main RAM */
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x201000, 0x203fff) AM_RAM             /* r/w in initial checks */
	AM_RANGE(0x390000, 0x390003) AM_READ_PORT("DSWA")
	AM_RANGE(0x3a0000, 0x3a0001) AM_WRITE(rbisland_spritectrl_w)
	AM_RANGE(0x3b0000, 0x3b0003) AM_READ_PORT("DSWB")
	AM_RANGE(0x3c0000, 0x3c0003) AM_WRITENOP        /* written very often, watchdog? */
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x800000, 0x8007ff) AM_READWRITE(rbisland_cchip_ram_r,rbisland_cchip_ram_w)
	AM_RANGE(0x800802, 0x800803) AM_READWRITE(rbisland_cchip_ctrl_r,rbisland_cchip_ctrl_w)
	AM_RANGE(0x800c00, 0x800c01) AM_WRITE(rbisland_cchip_bank_w)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)
	AM_RANGE(0xc20000, 0xc20003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xc50000, 0xc50003) AM_DEVWRITE("pc080sn", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram + other stuff */
ADDRESS_MAP_END

static ADDRESS_MAP_START( jumping_map, AS_PROGRAM, 16, rbisland_state )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM
	AM_RANGE(0x10c000, 0x10ffff) AM_RAM             /* main RAM */
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x201000, 0x203fff) AM_RAM             /* r/w in initial checks */
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("DSWA")
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("DSWB")
	AM_RANGE(0x401000, 0x401001) AM_READ_PORT("401001")
	AM_RANGE(0x401002, 0x401003) AM_READ_PORT("401003")
	AM_RANGE(0x3a0000, 0x3a0001) AM_WRITE(jumping_spritectrl_w)
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITENOP        /* watchdog? */
	AM_RANGE(0x400006, 0x400007) AM_WRITE(jumping_sound_w)
	AM_RANGE(0x420000, 0x420001) AM_READNOP         /* read, but result not used */
	AM_RANGE(0x430000, 0x430003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0x440000, 0x4407ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x800000, 0x80ffff) AM_WRITENOP        /* original c-chip location (not used) */
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)
	AM_RANGE(0xc20000, 0xc20003) AM_WRITENOP        /* seems it is a leftover from rbisland: scroll y written here too */
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xd00000, 0xd01fff) AM_RAM             /* original spriteram location, needed for Attract Mode */
ADDRESS_MAP_END


/**********************************************************
                         SOUND

              Rainbow uses a YM2151 and YM3012
              Jumping uses two YM2203's
***********************************************************/

WRITE8_MEMBER(rbisland_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 3);
}

READ8_MEMBER(rbisland_state::jumping_latch_r)
{
	return m_jumping_latch;
}


static ADDRESS_MAP_START( rbisland_sound_map, AS_PROGRAM, 8, rbisland_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)
	AM_RANGE(0x9002, 0x9100) AM_READNOP
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jumping_sound_map, AS_PROGRAM, 8, rbisland_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xb000, 0xb001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xb400, 0xb401) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0xb800, 0xb800) AM_READ(jumping_latch_r)
	AM_RANGE(0xbc00, 0xbc00) AM_WRITENOP    /* looks like a bankswitch, but sound works with or without it */
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( rbisland_generic )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "100k 1000k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, "Complete Bonus" )
	PORT_DIPSETTING(    0x08, "1 Up" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Mode" )
	PORT_DIPSETTING(    0x80, "Mode A (Japan)" ) /* Mode A is TAITO_COINAGE_JAPAN_OLD */
	PORT_DIPSETTING(    0x00, "Mode B (World)" ) /* Mode B is TAITO_COINAGE_WORLD */
INPUT_PORTS_END

static INPUT_PORTS_START( rbisland )
	PORT_INCLUDE(rbisland_generic)

	/* 0x390000 -> 0x10cfc2 ($fc2,A5) : DSWA */

	/* 0x3b0000 -> 0x10cfc4 ($fc4,A5) : DSWB */

	PORT_START("800007")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("800009")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("80000B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("80000D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( jumping )
	PORT_INCLUDE(rbisland_generic)

	/* 0x400000 -> 0x10cfc2 ($fc2,A5) */
	PORT_MODIFY("DSWA")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                        /* see notes */

	/* 0x400002 -> 0x10cfc4 ($fc4,A5) */
	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                        /* see notes */

	PORT_START("401001")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("401003")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
INPUT_PORTS_END


/**************************************************************
                         GFX DECODING
**************************************************************/

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 tiles */
	RGN_FRAC(1,1),
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 8, 12, 0, 4, 24, 28, 16, 20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 8, 12, 0, 4, 24, 28, 16, 20, 40, 44, 32, 36, 56, 60, 48, 52 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( rbisland )
	GFXDECODE_ENTRY( "gfx2", 0x000000, spritelayout, 0, 0x80 )  /* OBJ 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout,   0, 0x80 )  /* SCR 8x8 */
GFXDECODE_END


static const gfx_layout jumping_tilelayout =
{
	8,8,    /* 8*8 tiles */
	16384,  /* 16384 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 0x20000*8, 0x40000*8, 0x60000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every tile takes 8 consecutive bytes */
};

static const gfx_layout jumping_spritelayout =
{
	16,16,  /* 16*16 sprites */
	5120,   /* 5120 sprites */
	4,      /* 4 bits per pixel */
	{ 0x78000*8,0x50000*8,0x28000*8,0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8*16+0, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( jumping )
	GFXDECODE_ENTRY( "gfx2", 0, jumping_spritelayout, 0, 0x80 ) /* OBJ 16x16 */
	GFXDECODE_ENTRY( "gfx1", 0, jumping_tilelayout,   0, 0x80 ) /* SCR 8x8 */
GFXDECODE_END


/***********************************************************
                      MACHINE DRIVERS
***********************************************************/

void rbisland_state::machine_start()
{
}

static MACHINE_CONFIG_START( rbisland, rbisland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rbisland_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rbisland_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rbisland_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - enough for the sound CPU to read all commands */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(rbisland_state, screen_update_rainbow)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rbisland)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("pc080sn", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_GFXDECODE("gfxdecode")
	MCFG_PC080SN_PALETTE("palette")

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_16MHz/4) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(rbisland_state,bankswitch_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/* Jumping: The PCB has 2 Xtals, 18.432MHz and 24MHz */
static MACHINE_CONFIG_START( jumping, rbisland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_18_432MHz/2)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(jumping_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rbisland_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_24MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(jumping_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - enough unless otherwise */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(rbisland_state, screen_update_jumping)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jumping)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(rbisland_state,jumping)

	MCFG_DEVICE_ADD("pc080sn", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_YINVERT(1)
	MCFG_PC080SN_GFXDECODE("gfxdecode")
	MCFG_PC080SN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_24MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_24MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/* Imnoe PCB uses 16MHz CPU crystal instead of 18.432 for CPU */
static MACHINE_CONFIG_DERIVED( jumpingi, jumping )
	MCFG_CPU_REPLACE("maincpu", M68000, XTAL_16MHz/2)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(jumping_map)
        MCFG_CPU_VBLANK_INT_DRIVER("screen", rbisland_state,  irq4_line_hold)
MACHINE_CONFIG_END

/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( rbisland )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b22-10-1.19",   0x00000, 0x10000, CRC(e34a50ca) SHA1(17a92cd7182db1e18000b1ae689758fcfd70fe16) )
	ROM_LOAD16_BYTE( "b22-11-1.20",   0x00001, 0x10000, CRC(6a31a093) SHA1(1e99ae47811c0d3774d138dab02ac50bc1b92173) )
	ROM_LOAD16_BYTE( "b22-08-1.21",   0x20000, 0x10000, CRC(15d6e17a) SHA1(7b0339180239e75adf1437aee276b652a1bfee51) )
	ROM_LOAD16_BYTE( "b22-09-1.22",   0x20001, 0x10000, CRC(454e66bc) SHA1(54d57dee90da04101f4b1473770b38001d63230f) )
	ROM_LOAD16_BYTE( "b22-03.23",     0x40000, 0x20000, CRC(3ebb0fb8) SHA1(1b41b305623d121255eb70cb992e4d9da13abd82) )
	ROM_LOAD16_BYTE( "b22-04.24",     0x40001, 0x20000, CRC(91625e7f) SHA1(765afd973d9b82bb496b04beca284bf2769d6e6f) )

	ROM_REGION( 0x10000, "cchip", 0 )
	ROM_LOAD( "cchip_b22-15.53",            0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b22-14.43",            0x00000, 0x4000, CRC(113c1a5b) SHA1(effa2adf54a6be78b2d4baf3a47529342fb0d895) )
	ROM_CONTINUE(                     0x10000, 0xc000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b22-01.2",             0x00000, 0x80000, CRC(b76c9168) SHA1(e924be0c8294b930488bb04583784254a840a52e) )  /* tiles */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD( "b22-02.5",             0x00000, 0x80000, CRC(1b87ecf0) SHA1(37a463184f4064fe0565367236e289d57639614c) )  /* sprites */
	ROM_LOAD16_BYTE( "b22-12.7",      0x80000, 0x10000, CRC(67a76dc6) SHA1(626ee684eb3ea859c695ffe03344ccaa442da4af) )
	ROM_LOAD16_BYTE( "b22-13.6",      0x80001, 0x10000, CRC(2fda099f) SHA1(a1e27a4497f6733608be924d69d965b19f725b99) )
ROM_END

ROM_START( rbislando )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b22-10.19",     0x00000, 0x10000, CRC(3b013495) SHA1(fc89f401a80b9bde174df8a257bb7fad4937c838) )
	ROM_LOAD16_BYTE( "b22-11.20",     0x00001, 0x10000, CRC(80041a3d) SHA1(619d71a2bef5fd898a15d37d8016850f832428c3) )
	ROM_LOAD16_BYTE( "b22-08.21",     0x20000, 0x10000, CRC(962fb845) SHA1(1c5581e697902ee5cde0fb841ef05eade04a901b) )
	ROM_LOAD16_BYTE( "b22-09.22",     0x20001, 0x10000, CRC(f43efa27) SHA1(f92bcdbe9bbc1c200946c10605846bb406d8e963) )
	ROM_LOAD16_BYTE( "b22-03.23",     0x40000, 0x20000, CRC(3ebb0fb8) SHA1(1b41b305623d121255eb70cb992e4d9da13abd82) )
	ROM_LOAD16_BYTE( "b22-04.24",     0x40001, 0x20000, CRC(91625e7f) SHA1(765afd973d9b82bb496b04beca284bf2769d6e6f) )

	ROM_REGION( 0x10000, "cchip", 0 )
	ROM_LOAD( "cchip_b22-15.53",            0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b22-14.43",            0x00000, 0x4000, CRC(113c1a5b) SHA1(effa2adf54a6be78b2d4baf3a47529342fb0d895) )
	ROM_CONTINUE(                     0x10000, 0xc000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b22-01.2",             0x00000, 0x80000, CRC(b76c9168) SHA1(e924be0c8294b930488bb04583784254a840a52e) )  /* tiles */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD( "b22-02.5",             0x00000, 0x80000, CRC(1b87ecf0) SHA1(37a463184f4064fe0565367236e289d57639614c) )  /* sprites */
	ROM_LOAD16_BYTE( "b22-12.7",      0x80000, 0x10000, CRC(67a76dc6) SHA1(626ee684eb3ea859c695ffe03344ccaa442da4af) )
	ROM_LOAD16_BYTE( "b22-13.6",      0x80001, 0x10000, CRC(2fda099f) SHA1(a1e27a4497f6733608be924d69d965b19f725b99) )
ROM_END

ROM_START( rbislande )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b39-01.19",     0x00000, 0x10000, CRC(50690880) SHA1(88cd8739eaa6e4e5988be225c31d2a6605173d39) )
	ROM_LOAD16_BYTE( "b39-02.20",     0x00001, 0x10000, CRC(4dead71f) SHA1(03e9df33fc8fc64d6eeb1c3a763acac00b10c071) )
	ROM_LOAD16_BYTE( "b39-03.21",     0x20000, 0x10000, CRC(4a4cb785) SHA1(6ba7de3901a001cbb31713664ccb68b943a5578f) )
	ROM_LOAD16_BYTE( "b39-04.22",     0x20001, 0x10000, CRC(4caa53bd) SHA1(302c668b77de41ffef96a3ca9d94877b4affc466) )
	ROM_LOAD16_BYTE( "b22-03.23",     0x40000, 0x20000, CRC(3ebb0fb8) SHA1(1b41b305623d121255eb70cb992e4d9da13abd82) )
	ROM_LOAD16_BYTE( "b22-04.24",     0x40001, 0x20000, CRC(91625e7f) SHA1(765afd973d9b82bb496b04beca284bf2769d6e6f) )

	ROM_REGION( 0x10000, "cchip", 0 )
	ROM_LOAD( "cchip_b39-05.53",            0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b22-14.43",            0x00000, 0x4000, CRC(113c1a5b) SHA1(effa2adf54a6be78b2d4baf3a47529342fb0d895) )
	ROM_CONTINUE(                     0x10000, 0xc000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b22-01.2",             0x00000, 0x80000, CRC(b76c9168) SHA1(e924be0c8294b930488bb04583784254a840a52e) )  /* tiles */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD( "b22-02.5",             0x00000, 0x80000, CRC(1b87ecf0) SHA1(37a463184f4064fe0565367236e289d57639614c) )  /* sprites */
	ROM_LOAD16_BYTE( "b22-12.7",      0x80000, 0x10000, CRC(67a76dc6) SHA1(626ee684eb3ea859c695ffe03344ccaa442da4af) )
	ROM_LOAD16_BYTE( "b22-13.6",      0x80001, 0x10000, CRC(2fda099f) SHA1(a1e27a4497f6733608be924d69d965b19f725b99) )
ROM_END

ROM_START( jumping )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6.h4",          0x00000, 0x10000, CRC(3fab6b31) SHA1(57803478949cb62f7eab2ef9be08b13aa2237dbc) )
	ROM_LOAD16_BYTE( "10.h8",         0x00001, 0x10000, CRC(8c878827) SHA1(4a54a217b7c442305c3ce9298aa36ae225382444) )
	ROM_LOAD16_BYTE( "5.i4",          0x20000, 0x10000, CRC(443492cf) SHA1(fc3809d784d611df4fd446ca2443eebdf4f0bfd7) )
	ROM_LOAD16_BYTE( "9.i8",          0x20001, 0x10000, CRC(ed33bae1) SHA1(24ade29f3fdd45aa772ccf9c046294baa6eae7a2) )
	ROM_LOAD16_BYTE( "4.bin",         0x40000, 0x10000, CRC(00bf8a91) SHA1(973868eef99b1bcfa60e1b22a18946aa3edefad3) ) // 4+3 == b22-03.23
	ROM_LOAD16_BYTE( "8.bin",         0x40001, 0x10000, CRC(e3d7a844) SHA1(9559b38f2017de0c93ed82ca7dccfb046fff39f9) ) // 8+7 == b22-04.24
	ROM_LOAD16_BYTE( "3.bin",         0x60000, 0x10000, CRC(a3ab61c6) SHA1(5cf82d1aa1f548fb3c243c625e4ff52c8714bacc) ) // ^
	ROM_LOAD16_BYTE( "7.bin",         0x60001, 0x10000, CRC(c1c4c701) SHA1(4dd751418ee0f8ae766e2fe47f752d0758d7d682) ) // ^
	ROM_LOAD16_BYTE( "2.f89",         0x80001, 0x10000, CRC(0810d327) SHA1(fe91ac02e617bde413dc8a20b7cbcaf3e20aeb28) ) /* c-chip substitute */

	ROM_REGION( 0x14000, "audiocpu", 0 )
	ROM_LOAD( "jb1_cd67",             0x00000, 0x8000, CRC(8527c00e) SHA1(86e3824caca39aca4ca4df63bb4474adacfc4c53) )
	ROM_CONTINUE(                     0x10000, 0x4000 )
	ROM_CONTINUE(                     0x0c000, 0x4000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "17.ic8",              0x00000, 0x10000, CRC(65b76309) SHA1(1e345726e137f4c56d4bf239651c986fd53a16c3) )  /* tiles */
	ROM_LOAD( "18.ic7",              0x10000, 0x10000, CRC(43a94283) SHA1(d6a05cbc7b996a8e7f1520563f6fada9a59021a4) )
	ROM_LOAD( "15.ic10",             0x20000, 0x10000, CRC(e61933fb) SHA1(02bc0e1a7a3ce9e15fb83b28ce8fafb0b8d80ebd) )
	ROM_LOAD( "16.ic9",              0x30000, 0x10000, CRC(ed031eb2) SHA1(905be4d890ff7bb8a4d8ad85b2a11483fb4d67eb) )
	ROM_LOAD( "13.ic12",             0x40000, 0x10000, CRC(312700ca) SHA1(c79edc9c25f364d0afd79aaa21cfe2fe46044314) )
	ROM_LOAD( "14.ic11",             0x50000, 0x10000, CRC(de3b0b88) SHA1(14b8871821e4c0abbb9967c5aa282cf4e67884fe) )
	ROM_LOAD( "11.ic14",             0x60000, 0x10000, CRC(9fdc6c8e) SHA1(ff4e1a98dc982bce2f9d235cac62c7166f477f64) )
	ROM_LOAD( "12.ic13",             0x70000, 0x10000, CRC(06226492) SHA1(834280ec49e61a0c9c6b6fe2033e1b20bd1bffbf) )

	ROM_REGION( 0xa0000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "jb2_ic62",             0x00000, 0x10000, CRC(8548db6c) SHA1(675cd301259d5ed16098a38ac58b27b5ccd91264) )  /* sprites */
	ROM_LOAD( "jb2_ic61",             0x10000, 0x10000, CRC(37c5923b) SHA1(c83ef45564c56ef62d7019aecbd79dccc671deee) )
	ROM_LOAD( "jb2_ic60",             0x20000, 0x08000, CRC(662a2f1e) SHA1(1c5e8b1f0623e64faf9cd60f9653fc5957191a9b) )
	ROM_LOAD( "jb2_ic78",             0x28000, 0x10000, CRC(925865e1) SHA1(457de50bc03e8b949ac7d46ae4188201e87574a8) )
	ROM_LOAD( "jb2_ic77",             0x38000, 0x10000, CRC(b09695d1) SHA1(e6d315f9befb7b47f42668d573a1102e52d78aea) )
	ROM_LOAD( "jb2_ic76",             0x48000, 0x08000, CRC(41937743) SHA1(890c832a7cf87e6fe749d4824b02d57e10872bdf) )
	ROM_LOAD( "jb2_ic93",             0x50000, 0x10000, CRC(f644eeab) SHA1(9d45e9dfb08e8c90b4b10f5dc383fa4732161a81) )
	ROM_LOAD( "jb2_ic92",             0x60000, 0x10000, CRC(3fbccd33) SHA1(137880a564573db1f9e6de5c730af6709ca301f3) )
	ROM_LOAD( "jb2_ic91",             0x70000, 0x08000, CRC(d886c014) SHA1(9327c332c98a81451e9e0624344d2601ef06e490) )
	ROM_LOAD( "jb2_i121",             0x78000, 0x10000, CRC(93df1e4d) SHA1(b100d265b973254ec9cd44b6c32f62b4bac3b732) )
	ROM_LOAD( "jb2_i120",             0x88000, 0x10000, CRC(7c4e893b) SHA1(eceecb38554157ee24d228a2c722dad750a6a07d) )
	ROM_LOAD( "jb2_i119",             0x98000, 0x08000, CRC(7e1d58d8) SHA1(d586a018c3ec3e6e6a39992170d324361e03c68a) )

	ROM_REGION( 0x200, "pals", 0 )
	ROM_LOAD( "jumping-pal16r6.bin",             0x000, 0x104, CRC(12e9a7b8) SHA1(a0ce8b6083c9adfcb4bdbca87f63a01f292525f3) )
	ROM_LOAD( "jumping-pal20l8.bin",             0x000, 0x144, CRC(76944f81) SHA1(ab78e4e157ffdc13aea5dc360268b2640e60d19c) )
	ROM_LOAD( "pal16l8a.ic51.bin",               0x000, 0x104, CRC(c1e6cb8f) SHA1(9908e62bb9b806047b7a344bb62334bd696b9fc8) ) // z80 address decoder?
ROM_END

ROM_START( jumpinga )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6.h4",          0x00000, 0x10000, CRC(3fab6b31) SHA1(57803478949cb62f7eab2ef9be08b13aa2237dbc) )
	ROM_LOAD16_BYTE( "10.h8",         0x00001, 0x10000, CRC(8c878827) SHA1(4a54a217b7c442305c3ce9298aa36ae225382444) )
	ROM_LOAD16_BYTE( "seyutu_5.i4",   0x20000, 0x10000, CRC(25f19b71) SHA1(538f9a510cf73581b5ec01befdba7dfb02032c66) )
	ROM_LOAD16_BYTE( "seyutu_9.i8",   0x20001, 0x10000, CRC(9c94f260) SHA1(aa963acacbf11480d0ddc3b99fc839414970c043) )
	ROM_LOAD16_BYTE( "4.bin",         0x40000, 0x10000, CRC(00bf8a91) SHA1(973868eef99b1bcfa60e1b22a18946aa3edefad3) ) // 4+3 == b22-03.23
	ROM_LOAD16_BYTE( "8.bin",         0x40001, 0x10000, CRC(e3d7a844) SHA1(9559b38f2017de0c93ed82ca7dccfb046fff39f9) ) // 8+7 == b22-04.24
	ROM_LOAD16_BYTE( "3.bin",         0x60000, 0x10000, CRC(a3ab61c6) SHA1(5cf82d1aa1f548fb3c243c625e4ff52c8714bacc) ) // ^
	ROM_LOAD16_BYTE( "7.bin",         0x60001, 0x10000, CRC(c1c4c701) SHA1(4dd751418ee0f8ae766e2fe47f752d0758d7d682) ) // ^
	ROM_LOAD16_BYTE( "2.f89",         0x80001, 0x10000, CRC(0810d327) SHA1(fe91ac02e617bde413dc8a20b7cbcaf3e20aeb28) ) /* c-chip substitute */

	ROM_REGION( 0x14000, "audiocpu", 0 )
	ROM_LOAD( "jb1_cd67",             0x00000, 0x8000, CRC(8527c00e) SHA1(86e3824caca39aca4ca4df63bb4474adacfc4c53) )
	ROM_CONTINUE(                     0x10000, 0x4000 )
	ROM_CONTINUE(                     0x0c000, 0x4000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "17.ic8",              0x00000, 0x10000, CRC(65b76309) SHA1(1e345726e137f4c56d4bf239651c986fd53a16c3) )  /* tiles */
	ROM_LOAD( "18.ic7",              0x10000, 0x10000, CRC(43a94283) SHA1(d6a05cbc7b996a8e7f1520563f6fada9a59021a4) )
	ROM_LOAD( "15.ic10",             0x20000, 0x10000, CRC(e61933fb) SHA1(02bc0e1a7a3ce9e15fb83b28ce8fafb0b8d80ebd) )
	ROM_LOAD( "16.ic9",              0x30000, 0x10000, CRC(ed031eb2) SHA1(905be4d890ff7bb8a4d8ad85b2a11483fb4d67eb) )
	ROM_LOAD( "13.ic12",             0x40000, 0x10000, CRC(312700ca) SHA1(c79edc9c25f364d0afd79aaa21cfe2fe46044314) )
	ROM_LOAD( "14.ic11",             0x50000, 0x10000, CRC(de3b0b88) SHA1(14b8871821e4c0abbb9967c5aa282cf4e67884fe) )
	ROM_LOAD( "11.ic14",             0x60000, 0x10000, CRC(9fdc6c8e) SHA1(ff4e1a98dc982bce2f9d235cac62c7166f477f64) )
	ROM_LOAD( "12.ic13",             0x70000, 0x10000, CRC(06226492) SHA1(834280ec49e61a0c9c6b6fe2033e1b20bd1bffbf) )

	ROM_REGION( 0xa0000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "jb2_ic62",             0x00000, 0x10000, CRC(8548db6c) SHA1(675cd301259d5ed16098a38ac58b27b5ccd91264) )  /* sprites */
	ROM_LOAD( "20.bin",               0x10000, 0x10000, CRC(89b3d8ee) SHA1(8491de6e8292e58b9a8696be15827bcb1ea42845) ) // dumped multiple times, always the same
	ROM_LOAD( "jb2_ic60",             0x20000, 0x08000, CRC(662a2f1e) SHA1(1c5e8b1f0623e64faf9cd60f9653fc5957191a9b) )
	ROM_LOAD( "jb2_ic78",             0x28000, 0x10000, CRC(925865e1) SHA1(457de50bc03e8b949ac7d46ae4188201e87574a8) )
	ROM_LOAD( "jb2_ic77",             0x38000, 0x10000, CRC(b09695d1) SHA1(e6d315f9befb7b47f42668d573a1102e52d78aea) )
	ROM_LOAD( "jb2_ic76",             0x48000, 0x08000, CRC(41937743) SHA1(890c832a7cf87e6fe749d4824b02d57e10872bdf) )
	ROM_LOAD( "jb2_ic93",             0x50000, 0x10000, CRC(f644eeab) SHA1(9d45e9dfb08e8c90b4b10f5dc383fa4732161a81) )
	ROM_LOAD( "jb2_ic92",             0x60000, 0x10000, CRC(3fbccd33) SHA1(137880a564573db1f9e6de5c730af6709ca301f3) )
	ROM_LOAD( "jb2_ic91",             0x70000, 0x08000, CRC(d886c014) SHA1(9327c332c98a81451e9e0624344d2601ef06e490) )
	ROM_LOAD( "jb2_i121",             0x78000, 0x10000, CRC(93df1e4d) SHA1(b100d265b973254ec9cd44b6c32f62b4bac3b732) )
	ROM_LOAD( "jb2_i120",             0x88000, 0x10000, CRC(7c4e893b) SHA1(eceecb38554157ee24d228a2c722dad750a6a07d) )
	ROM_LOAD( "jb2_i119",             0x98000, 0x08000, CRC(7e1d58d8) SHA1(d586a018c3ec3e6e6a39992170d324361e03c68a) )

	ROM_REGION( 0x200, "pals", 0 )
	ROM_LOAD( "jumping-pal16r6.bin",             0x000, 0x104, CRC(12e9a7b8) SHA1(a0ce8b6083c9adfcb4bdbca87f63a01f292525f3) )
	ROM_LOAD( "jumping-pal20l8.bin",             0x000, 0x144, CRC(76944f81) SHA1(ab78e4e157ffdc13aea5dc360268b2640e60d19c) )
	ROM_LOAD( "pal16l8a.ic51.bin",               0x000, 0x104, CRC(c1e6cb8f) SHA1(9908e62bb9b806047b7a344bb62334bd696b9fc8) ) // z80 address decoder?
ROM_END

/* red 'Imnoe' PCB */
ROM_START( jumpingi )
        ROM_REGION( 0xa0000, "maincpu", 0 )
        ROM_LOAD16_BYTE( "05.IC3",         0x00000, 0x20000, CRC(69ac4af4) SHA1(39055573e412e2591f7a68f9fee5919528529544) )
        ROM_LOAD16_BYTE( "03.IC6",         0x00001, 0x20000, CRC(38975cdc) SHA1(23c02a4574a95904805d5f458c06c77c14d11c14) )
        ROM_LOAD16_BYTE( "06.IC2",         0x40000, 0x20000, CRC(3ebb0fb8) SHA1(1b41b305623d121255eb70cb992e4d9da13abd82) ) // b22-03.23
        ROM_LOAD16_BYTE( "04.IC5",         0x40001, 0x20000, CRC(91625e7f) SHA1(765afd973d9b82bb496b04beca284bf2769d6e6f) ) // b22-04.24
        ROM_LOAD16_BYTE( "02",             0x80001, 0x10000, CRC(0810d327) SHA1(fe91ac02e617bde413dc8a20b7cbcaf3e20aeb28) ) /* c-chip substitute */

        ROM_REGION( 0x14000, "audiocpu", 0 )
        ROM_LOAD( "01.IC53",              0x00000, 0x8000, CRC(8527c00e) SHA1(86e3824caca39aca4ca4df63bb4474adacfc4c53) )
        ROM_CONTINUE(                     0x10000, 0x4000 )
        ROM_CONTINUE(                     0x0c000, 0x4000 )

        ROM_REGION( 0x80000, "gfx1", 0 )
        ROM_LOAD( "13.IC8",              0x00000, 0x10000, CRC(65b76309) SHA1(1e345726e137f4c56d4bf239651c986fd53a16c3) )  /* tiles */
        ROM_LOAD( "14.IC7",              0x10000, 0x10000, CRC(43a94283) SHA1(d6a05cbc7b996a8e7f1520563f6fada9a59021a4) )
        ROM_LOAD( "11.IC10",             0x20000, 0x10000, CRC(e61933fb) SHA1(02bc0e1a7a3ce9e15fb83b28ce8fafb0b8d80ebd) )
        ROM_LOAD( "12.IC9",              0x30000, 0x10000, CRC(ed031eb2) SHA1(905be4d890ff7bb8a4d8ad85b2a11483fb4d67eb) )
        ROM_LOAD( "09.IC12",             0x40000, 0x10000, CRC(312700ca) SHA1(c79edc9c25f364d0afd79aaa21cfe2fe46044314) )
        ROM_LOAD( "10.IC11",             0x50000, 0x10000, CRC(de3b0b88) SHA1(14b8871821e4c0abbb9967c5aa282cf4e67884fe) )
        ROM_LOAD( "07.IC14",             0x60000, 0x10000, CRC(9fdc6c8e) SHA1(ff4e1a98dc982bce2f9d235cac62c7166f477f64) )
        ROM_LOAD( "08.IC13",             0x70000, 0x10000, CRC(06226492) SHA1(834280ec49e61a0c9c6b6fe2033e1b20bd1bffbf) )

        ROM_REGION( 0xa0000, "gfx2", ROMREGION_INVERT )
        ROM_LOAD( "15.IC62",             0x00000, 0x10000, CRC(8548db6c) SHA1(675cd301259d5ed16098a38ac58b27b5ccd91264) )  /* sprites */
        ROM_LOAD( "19.IC61",             0x10000, 0x10000, CRC(89b3d8ee) SHA1(8491de6e8292e58b9a8696be15827bcb1ea42845) ) 
        ROM_LOAD( "23.IC60",             0x20000, 0x08000, CRC(662a2f1e) SHA1(1c5e8b1f0623e64faf9cd60f9653fc5957191a9b) )
        ROM_LOAD( "16.IC78",             0x28000, 0x10000, CRC(925865e1) SHA1(457de50bc03e8b949ac7d46ae4188201e87574a8) )
        ROM_LOAD( "20.IC77",             0x38000, 0x10000, CRC(b09695d1) SHA1(e6d315f9befb7b47f42668d573a1102e52d78aea) )
        ROM_LOAD( "24.IC76",             0x48000, 0x08000, CRC(41937743) SHA1(890c832a7cf87e6fe749d4824b02d57e10872bdf) )
        ROM_LOAD( "17.IC93",             0x50000, 0x10000, CRC(f644eeab) SHA1(9d45e9dfb08e8c90b4b10f5dc383fa4732161a81) )
        ROM_LOAD( "21.IC92",             0x60000, 0x10000, CRC(16e1b0ff) SHA1(1467a317d07a447d01113e6b6b9f5aca30cb0dcb) )
        ROM_LOAD( "25.IC91",             0x70000, 0x08000, CRC(d886c014) SHA1(9327c332c98a81451e9e0624344d2601ef06e490) )
        ROM_LOAD( "18.IC121",            0x78000, 0x10000, CRC(93df1e4d) SHA1(b100d265b973254ec9cd44b6c32f62b4bac3b732) )
        ROM_LOAD( "22.IC120",            0x88000, 0x10000, CRC(7c4e893b) SHA1(eceecb38554157ee24d228a2c722dad750a6a07d) )
        ROM_LOAD( "26.IC119",            0x98000, 0x08000, CRC(7e1d58d8) SHA1(d586a018c3ec3e6e6a39992170d324361e03c68a) )

        ROM_REGION( 0x200, "pals", 0 )
        ROM_LOAD( "JP2.IC56",            0x000, 0x104, CRC(12e9a7b8) SHA1(a0ce8b6083c9adfcb4bdbca87f63a01f292525f3) ) // PAL16R6A-2CN
        ROM_LOAD( "JP1.IC13",            0x000, 0x144, CRC(76944f81) SHA1(ab78e4e157ffdc13aea5dc360268b2640e60d19c) ) // PAL20L8A-2CNS
        ROM_LOAD( "JP3.IC51",            0x000, 0x104, CRC(c1e6cb8f) SHA1(9908e62bb9b806047b7a344bb62334bd696b9fc8) ) // PAL16L8A-2CN z80 address decoder?
ROM_END

DRIVER_INIT_MEMBER(rbisland_state,rbisland)
{
	UINT8 *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0xc000], 0x4000);

	rbisland_cchip_init(0);
}

DRIVER_INIT_MEMBER(rbisland_state,rbislande)
{
	UINT8 *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0xc000], 0x4000);

	rbisland_cchip_init(1);
}

DRIVER_INIT_MEMBER(rbisland_state,jumping)
{
	m_jumping_latch = 0;
	save_item(NAME(m_jumping_latch));
}


GAME( 1987, rbisland,  0,        rbisland, rbisland, rbisland_state, rbisland,  ROT0, "Taito Corporation", "Rainbow Islands (new version)", GAME_SUPPORTS_SAVE )
GAME( 1987, rbislando, rbisland, rbisland, rbisland, rbisland_state, rbisland,  ROT0, "Taito Corporation", "Rainbow Islands (old version)", GAME_SUPPORTS_SAVE )
GAME( 1988, rbislande, rbisland, rbisland, rbisland, rbisland_state, rbislande, ROT0, "Taito Corporation", "Rainbow Islands (Extra)", GAME_SUPPORTS_SAVE )
GAME( 1989, jumping,   rbisland, jumping,  jumping, rbisland_state,  jumping,   ROT0, "bootleg",          "Jumping (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1988, jumpinga,  rbisland, jumping,  jumping, rbisland_state,  jumping,   ROT0, "bootleg (Seyutu)", "Jumping (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1988, jumpingi,  rbisland, jumpingi,  jumping, rbisland_state,  jumping,   ROT0, "bootleg (Seyutu)", "Jumping (set 3, Imnoe PCB)", GAME_SUPPORTS_SAVE )
