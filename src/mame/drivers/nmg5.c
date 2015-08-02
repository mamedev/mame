// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Title                          Copyright            PCB - ID
 ----------------------------------------------------------------------------
 Multi 5 / New Multi Game 5     (c) 1997 Yun Sung    YS-1300
 Multi 5 / New Multi Game 5     (c) 1998 Yun Sung    YS-1300 / YS-1301
 Search Eye                     (c) 1999 Yun Sung    YS-1905
 Search Eye Plus V2.0           (c) 1999 Yun Sung    YS-1905
 Puzzle Club (set 1)            (c) 2000 Yun Sung    YS-2113
 Puzzle Club (set 2)            (c) 2000 Yun Sung    YS-2111
 Garogun Seroyang               (c) 2000 Yun Sung    YS-2111
 7 Ordi                         (c) 2002 Yun Sung    YS-2118B
 Wonder Stick                   (c) ???? Yun Sung    YS-2320

 driver by Pierpaolo Prazzoli


Stephh's notes (based on the games M68000 code and some tests) :

1) 'nmg5'

  - On title screen, there's a 1998 release date.
    However, ending "Tong Boy" shows a 1997 copyright,
    and there's even a 1996 date in the PRG ROMS :
    look at addresses 0x0565bc to 0x0565bf to see it.
  - There's sort of "protection" routine at address 0x000366.
    Its effect is unknown but prevents the game to boot.
  - Here are what the inputs which are read in "Test Mode" do :
      * COIN1 : adds 1 coin (you can hear the sound)
      * COIN2 : adds 1 coin (you can hear the sound)
      * Player 2 BUTTON3 :
          . if SW2-1 is ON, it goes to "Test Mode" routine
          . if SW2-1 is OFF, it goes back to the game
            WITHOUT resetting the number of coins / credits
      * Player 2 BUTTON2 : toggles "Slideshow Mode" ON/OFF
  - Here are what the additional inputs which are read in "Slideshow Mode" do :
      * Player 1 BUTTON1 : (picture++)%0x1c  (next)
      * Player 1 BUTTON2 : (picture--)%0x1c  (previous)
  - Here are what the buttons do for each sub-game :
      * Rocktris :
          . BUTTON1 : Rotate counterclockwise
          . BUTTON2 : Rotate counterclockwise (same as BUTTON1)
          . BUTTON3 : UNUSED
      * Bubble Gum :
          . BUTTON1 : Fire
          . BUTTON2 : UNUSED
          . BUTTON3 : UNUSED
      * Tong Boy :
          . BUTTON1 : Jump / Fire (depending on stages)
          . BUTTON2 : UNUSED
          . BUTTON3 : UNUSED
      * Cross Point :
          . BUTTON1 : Rotate counterclockwise
          . BUTTON2 : Help
          . BUTTON3 : UNUSED
      * Box Logic :
          . BUTTON1 : Tag as good
          . BUTTON2 : Tag as bad / Untag (switch)
          . BUTTON3 : Help
  - It is difficult to find a name for DSW1-6 and DSW1-7.
    All I can tell is that it seems to affects the crocodiles
    patterns in "Tong Boy". Watch the demo and see what you find.
  - Unused DSW2-4 is told to be "Flip Screen" in "Test Mode",
    but is in fact not read in-game. Leftover from another game ?

2) 'nmg5e'

  - On title screen, there's a 1997 release date.
    However, there's a 1996 date in the PRG ROMS :
    look at addresses 0x0564a4 to 0x0564a7 to see it.
  - There are 2 main differences with the other set :
      * The code for the "protection" routine doesn't exist
      * Player 2 BUTTON2 is tested but has no visible effect
        (there is no "Slideshow Mode" and code which tests
        Player 1 BUTTON1 and Player 1 BUTTON2 doesn't exist)

3) 'searchey'

  - Here are what the buttons do :
      * BUTTON1 : spot
      * BUTTON2 : help
  - It's difficult to find a good description for DSW2-1 !
    All I can tell is that it affects the number of items to find :
      * OFF : table at 0x024cae (4 items for level 1)
      * ON  : table at 0x024d3c (3 items for level 1)

4) 'searchp2'

  - Here are what the buttons do :
      * BUTTON1 : spot
      * BUTTON2 : help
  - The "Test Mode" Dip Switch points only to a black screen.
    I can't tell if it's a bug, but there is no text for it as in 'searchey'.
  - There is no "Demo Sounds" Dip Switch : sound is ALWAYS OFF in "Demo Mode".
  - It is difficult to find a name for DSW2-6 and DSW2-7.
    All I can tell is that it affects the rolls in "Lucky Chance".
    Look at code from 0x006c08 to 0x006de8 and tables from 0x01db10 to 0x01db27
    (4 tables - each table is 3 * 3 words = 6 bytes wide).
  - The "Language" Dip Switch only affects the "how to play" instructions
    in "Demo Mode" as well as the text which tells how many items to find.
  - It's difficult to find a good description for DSW2-2 !
    All I can tell is that it affects the "Lucky" timer :
    when ON, you have 5 "seconds" less than when OFF.
  - It's difficult to find a good description for DSW2-1 !
    All I can tell is that it affects the number of items to find :
    when ON, you have 1 item less than when OFF.

5) 'psclubys*'

  - Here are what the buttons do for each sub-game :
      * Magic Eye :
          . BUTTON1 : Fire
          . BUTTON2 : Fire (same as BUTTON1)
          . BUTTON3 : UNUSED
      * Box Logic :
          . BUTTON1 : Tag as good
          . BUTTON2 : Tag as bad / Untag (switch)
          . BUTTON3 : Help
      * Mad Ball :
          . BUTTON1 : Jump / Launch ball
          . BUTTON2 : Fire
          . BUTTON3 : UNUSED
      * Magic Bubble :
          . BUTTON1 : Fire
          . BUTTON2 : UNUSED
          . BUTTON3 : UNUSED
      * Bogle Puyo :
          . BUTTON1 : Rotate counterclockwise
          . BUTTON2 : Rotate clockwise
          . BUTTON3 : UNUSED
      * Rocktris :
          . BUTTON1 : Rotate counterclockwise
          . BUTTON2 : Rotate counterclockwise (same as BUTTON1)
          . BUTTON3 : UNUSED
  - DSW1-6 only affects "Magic Bubble", "Bogle Puyo" and "Rocktris" sub-games.
  - There are 72 gals pics, 36 of them are "soft" (sort girls are even not nude)
    and the 36 others are "hard".
    When nudity set to "Soft only", the game will only display "soft" gals pics.
    When nudity set to "Soft and High", the game will alternate "soft" and "hard" gals pics.
    There is no gals pics (thus no nudity) in "Magic Eye" and "Bogle Puyo" sub-games.
    "Girl view" choice in "Test Mode" is also affected by DSW1-5.
  - As there is no nudity in "Magic Eye" sub-game, I haven't been able
    to determine effect of DSW1-5. Have a look at routine at 0x006dc8 ('pclubys')
    or 0x006dc4 ('pclubysa') and see what you find.

6) 'garogun'

  - Here are what the buttons do :
      * BUTTON1 : spot
      * BUTTON2 : help
  - The game starts as soon as you have inserted enough coins.
    However, the START button is needed when you want to continue play.
  - There are 12 main levels and 3 bonus levels :
      * levels  1 to 6  : 3 sub-levels each - 1 bonus level after 2 full main levels
      * levels  7 to 9  : 5 sub-levels each - 1 bonus level after 1 full main level
      * levels 10 to 12 : 5 sub-levels each - 1 bonus level after 1 full main level
        (there is no bonus game when you end full main level 12)
  - There's an ingame bug when timer for bonus game is displayed :
      * Level is stored at address 0x2056c8 :
          . 0x00 to 0x0b : main levels 1 to 12
          . 0x0c : bonus level 3
          . 0x0d : bonus level 2
          . 0x0e : bonus level 1
      * Routine that handles starting time begins at address 0x010a22.
      * The problem is that when time is displayed in bonus game screen,
        the value stored at 0x2056c8 is still the one from the main level
        instead of being the one from the bonus game. So the time which
        is displayed is incorrect and it sometimes causes graphics glitches.
        However, when bonus game starts, value stored at 0x2056c8 is good.
   - Depending on "Helps" Dip Switch, you get 1 to 4 helps at start.
     Then, depending on your starting level, you get additional helps :
       * level 1 : +0
       * level 3 : +1
       * level 5 : +2

7) '7ordi'

  - Here are what the buttons do in "Test Mode" :
      * START1   : back
      * BUTTON1  : left
      * BUTTON2  : right
      * BUTTON3  : fire
      * BUTTON4  : down
      * BUTTON5  : up
  - Here are what the buttons do outside "Test Mode" :
      * COIN1    : adds 50 to 1000 credits depending on "Coinage" settings
      * SERVICE1 : adds 10000 credits
      * SERVICE2 : reset game AND number of credits
                   (but Dip Switches aren't read again)
  - Here are what the buttons do when the cards are just drawn :
      * BUTTON1  : select card 1
      * BUTTON2  : select card 2
      * BUTTON3  : select card 3
  - Here are what the buttons do when in-game :
      * BUTTON1  : die
      * BUTTON2  : check/call
      * BUTTON3  : bet/raise
  - Here are what the buttons do when you win a hand :
      * BUTTON2  : bonus game
      * BUTTON3  : take
  - Here are what the buttons do when you play bonus game :
      * START1   : pool
      * BUTTON1  : low (Ace, 2, 3, 4, 5, 6)
      * BUTTON6  : high (8, 9, 10, Jack, Queen, King)
  - Here are what the buttons do when you collect and
    "Winnings" Dip Switch set to "Medals and Credits"
    ("do you want to draw a medal ?" question) :
      * BUTTON5 : Yes
      * BUTTON6 : No
  - As hopper and other mecanisms aren't emulated, you can't
    draw a medal (winnings are always converted to credits).
    I hope that someone will be able to fix that as I can't do it.

8) 'wondstck'

  - Here are what the buttons do :
      * BUTTON1 : trace
      * BUTTON2 : help

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "video/decospr.h"

class nmg5_state : public driver_device
{
public:
	nmg5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bitmap(*this, "bitmap"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scroll_ram;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_fg_videoram;
	required_shared_ptr<UINT16> m_bitmap;
	optional_device<decospr_device> m_sprgen;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	UINT8 m_prot_val;
	UINT8 m_input_data;
	UINT8 m_priority_reg;
	UINT8 m_gfx_bank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_WRITE16_MEMBER(fg_videoram_w);
	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(nmg5_soundlatch_w);
	DECLARE_READ16_MEMBER(prot_r);
	DECLARE_WRITE16_MEMBER(prot_w);
	DECLARE_WRITE16_MEMBER(gfx_bank_w);
	DECLARE_WRITE16_MEMBER(priority_reg_w);
	DECLARE_WRITE8_MEMBER(oki_banking_w);
	DECLARE_DRIVER_INIT(prot_val_00);
	DECLARE_DRIVER_INIT(prot_val_10);
	DECLARE_DRIVER_INIT(prot_val_20);
	DECLARE_DRIVER_INIT(prot_val_40);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_nmg5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap( bitmap_ind16 &bitmap );
};



WRITE16_MEMBER(nmg5_state::fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmg5_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(nmg5_state::nmg5_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

READ16_MEMBER(nmg5_state::prot_r)
{
	return m_prot_val | m_input_data;
}

WRITE16_MEMBER(nmg5_state::prot_w)
{
	m_input_data = data & 0x0f;
}

WRITE16_MEMBER(nmg5_state::gfx_bank_w)
{
	if (m_gfx_bank != (data & 3))
	{
		m_gfx_bank = data & 3;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE16_MEMBER(nmg5_state::priority_reg_w)
{
	m_priority_reg = data & 7;

	if (m_priority_reg == 4 || m_priority_reg == 5 || m_priority_reg == 6)
		popmessage("unknown priority_reg value = %d\n", m_priority_reg);
}

WRITE8_MEMBER(nmg5_state::oki_banking_w)
{
	m_oki->set_bank_base((data & 1) ? 0x40000 : 0);
}

/*******************************************************************

                            Main Cpu

********************************************************************/

static ADDRESS_MAP_START( nmg5_map, AS_PROGRAM, 16, nmg5_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x120000, 0x12ffff) AM_RAM
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x180000, 0x180001) AM_WRITE(nmg5_soundlatch_w)
	AM_RANGE(0x180002, 0x180003) AM_WRITENOP
	AM_RANGE(0x180004, 0x180005) AM_READWRITE(prot_r, prot_w)
	AM_RANGE(0x180006, 0x180007) AM_WRITE(gfx_bank_w)
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("DSW")
	AM_RANGE(0x18000a, 0x18000b) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x18000c, 0x18000d) AM_READ_PORT("INPUTS")
	AM_RANGE(0x18000e, 0x18000f) AM_WRITE(priority_reg_w)
	AM_RANGE(0x300002, 0x300009) AM_WRITEONLY AM_SHARE("scroll_ram")
	AM_RANGE(0x30000a, 0x30000f) AM_WRITENOP
	AM_RANGE(0x320000, 0x321fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x322000, 0x323fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("bitmap")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pclubys_map, AS_PROGRAM, 16, nmg5_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x440000, 0x4407ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x460000, 0x4607ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x480000, 0x480001) AM_WRITE(nmg5_soundlatch_w)
	AM_RANGE(0x480002, 0x480003) AM_WRITENOP
	AM_RANGE(0x480004, 0x480005) AM_READWRITE(prot_r, prot_w)
	AM_RANGE(0x480006, 0x480007) AM_WRITE(gfx_bank_w)
	AM_RANGE(0x480008, 0x480009) AM_READ_PORT("DSW")
	AM_RANGE(0x48000a, 0x48000b) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x48000c, 0x48000d) AM_READ_PORT("INPUTS")
	AM_RANGE(0x48000e, 0x48000f) AM_WRITE(priority_reg_w)
	AM_RANGE(0x500002, 0x500009) AM_WRITEONLY AM_SHARE("scroll_ram")
	AM_RANGE(0x520000, 0x521fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x522000, 0x523fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("bitmap")
ADDRESS_MAP_END

/*******************************************************************

                            Sound Cpu

********************************************************************/

static ADDRESS_MAP_START( nmg5_sound_map, AS_PROGRAM, 8, nmg5_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pclubys_sound_map, AS_PROGRAM, 8, nmg5_state )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, nmg5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(oki_banking_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x18, 0x18) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1c, 0x1c) AM_DEVREADWRITE("oki", okim6295_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( nmg5 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Game Title" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, "Multi 5" )
	PORT_DIPSETTING(      0x0000, "New Multi Game 5" )
	PORT_DIPNAME( 0x0006, 0x0006, "Crocodiles (Tong Boy)" ) PORT_DIPLOCATION("SW1:7,6")   // See notes
	PORT_DIPSETTING(      0x0006, "Pattern 1" )
	PORT_DIPSETTING(      0x0002, "Pattern 2" )
	PORT_DIPSETTING(      0x0004, "Pattern 3" )
	PORT_DIPSETTING(      0x0000, "Pattern 4" )
	PORT_DIPNAME( 0x0018, 0x0018, "License" )               PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0000, "New Impeuropex Corp. S.R.L." )
	PORT_DIPSETTING(      0x0008, "BNS Enterprises" )
	PORT_DIPSETTING(      0x0010, "Nova Games" )
	PORT_DIPSETTING(      0x0018, DEF_STR( None ) )
	PORT_DIPNAME( 0x0020, 0x0020, "1P Vs 2P Rounds (Bubble Gum)" )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, "Best of 1" )             /* 1 winning round needed */
	PORT_DIPSETTING(      0x0000, "Best of 3" )             /* 2 winning rounds needed */
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x4000,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x4000,EQUALS,0x00)
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )                               // See notes
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Coin Type" )             PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("SYSTEM")    /* Coins */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0050, IP_ACTIVE_HIGH, IPT_SPECIAL )         // otherwise it doesn't boot (unneeded for 'nmg5e' - see notes)
	PORT_BIT( 0xffac, IP_ACTIVE_LOW,  IPT_UNUSED )          // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( searchey )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Timer Speed" )           PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0003, "Slowest" )
	PORT_DIPSETTING(      0x0002, "Slow" )
	PORT_DIPSETTING(      0x0001, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x000c, 0x0000, "Helps" )                 PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0030, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x3800, 0x3000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x0000, "Korean Duplicate 1" )
	PORT_DIPSETTING(      0x0800, "Korean Duplicate 2" )
	PORT_DIPSETTING(      0x1000, "Korean Duplicate 3" )
	PORT_DIPSETTING(      0x1800, DEF_STR( Italian ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x3800, "Korean Duplicate 4" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Items to find" )         PORT_DIPLOCATION("SW2:1")     // See notes
	PORT_DIPSETTING(      0x0000, "Less" )
	PORT_DIPSETTING(      0x8000, "More" )

	PORT_START("SYSTEM")    /* Coins */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW,  IPT_UNUSED )          // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Spot" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Help" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           // tested in service mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "Spot" )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME( "Help" )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           // tested in service mode
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( searcheya )
	PORT_INCLUDE(searchey)

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Korean ) )
INPUT_PORTS_END

static INPUT_PORTS_START( searchp2 )
	PORT_INCLUDE(searchey)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0600, 0x0600, "Lucky Chance" )          PORT_DIPLOCATION("SW2:7,6")   // See notes
	PORT_DIPSETTING(      0x0600, "Table 1" )
	PORT_DIPSETTING(      0x0400, "Table 2" )
	PORT_DIPSETTING(      0x0200, "Table 3" )
	PORT_DIPSETTING(      0x0000, "Table 4" )
	PORT_DIPNAME( 0x3800, 0x3000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:5,4,3") // See notes
	PORT_DIPSETTING(      0x0000, "Korean Duplicate 1" )
	PORT_DIPSETTING(      0x0800, "Korean Duplicate 2" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Italian ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Chinese ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x3800, "Korean Duplicate 3" )
	PORT_DIPNAME( 0x4000, 0x4000, "Lucky Timer" )           PORT_DIPLOCATION("SW2:2")     // See notes
	PORT_DIPSETTING(      0x0000, "Less" )
	PORT_DIPSETTING(      0x4000, "More" )
	PORT_DIPNAME( 0x8000, 0x8000, "Items to find" )         PORT_DIPLOCATION("SW2:1")     // See notes
	PORT_DIPSETTING(      0x0000, "Less" )
	PORT_DIPSETTING(      0x8000, "More" )
INPUT_PORTS_END

static INPUT_PORTS_START( pclubys )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "Lives (Mad Ball)" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "1P Vs 2P Rounds" )       PORT_DIPLOCATION("SW1:6")     // See notes
	PORT_DIPSETTING(      0x0000, "Best of 1" )             /* 1 winning round needed */
	PORT_DIPSETTING(      0x0004, "Best of 3" )             /* 2 winning rounds needed */
	PORT_DIPNAME( 0x0008, 0x0000, "Nudity" )                PORT_DIPLOCATION("SW1:5")     // See notes
	PORT_DIPSETTING(      0x0008, "Soft only" )
	PORT_DIPSETTING(      0x0000, "Soft and Hard" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x0030, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x3000, 0x3000, "Timer Speed (Magic Eye)" )    PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x3000, "Slowest" )
	PORT_DIPSETTING(      0x2000, "Slow" )
	PORT_DIPSETTING(      0x1000, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x4000, 0x0000, "Lives (Magic Eye)" )     PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPNAME( 0x8000, 0x8000, "Timer Speed (Box Logic)" )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW,  IPT_UNUSED )          // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( garogun )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Helps" )                 PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0003, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, "Timer Speed (Bonus Levels)" ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x000c, "Slowest" )
	PORT_DIPSETTING(      0x0008, "Slow" )
	PORT_DIPSETTING(      0x0004, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x0030, 0x0030, "Timer Speed (Main Levels)" )  PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x0030, "Slowest" )
	PORT_DIPSETTING(      0x0020, "Slow" )
	PORT_DIPSETTING(      0x0010, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("SYSTEM")    /* Coins */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )           // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff40, IP_ACTIVE_LOW, IPT_UNUSED )           // tested in service mode
INPUT_PORTS_END


static INPUT_PORTS_START( 7ordi )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "High-Low Error" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, "-500" )
	PORT_DIPSETTING(      0x0000, "Lose All" )
	PORT_DIPNAME( 0x0002, 0x0002, "Minimum Credits" )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0002, "300" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0x000c, 0x0000, "Credit Limit" )          PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x000c, "10000" )
	PORT_DIPSETTING(      0x0008, "30000" )
	PORT_DIPSETTING(      0x0004, "50000" )
	PORT_DIPSETTING(      0x0000, "100000" )
	PORT_DIPNAME( 0x0030, 0x0030, "Bet" )                   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x0030, "50 Credits" )
	PORT_DIPSETTING(      0x0020, "100 Credits" )
	PORT_DIPSETTING(      0x0010, "150 Credits" )
	PORT_DIPSETTING(      0x0000, "200 Credits" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x00c0, "1 Coin 50 Credits" )
	PORT_DIPSETTING(      0x0080, "1 Coin 100 Credits" )
	PORT_DIPSETTING(      0x0040, "1 Coin 500 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Coin 1000 Credits" )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")     // Not even sure it is used
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, "Winnings" )              PORT_DIPLOCATION("SW2:6")     // See notes
	PORT_DIPSETTING(      0x0400, "Medals and Credits" )
	PORT_DIPSETTING(      0x0000, "Credits only" )
	PORT_DIPNAME( 0x1800, 0x1800, "Medals Table" )          PORT_DIPLOCATION("SW2:5,4")   // To be confirmed
	PORT_DIPSETTING(      0x1800, "x1.0" )                  /*  50  35  25  15   5 */
	PORT_DIPSETTING(      0x1000, "x1.4" )                  /*  70  50  35  20   7 */
	PORT_DIPSETTING(      0x0800, "x2.0" )                  /* 100  70  50  30  10 */
	PORT_DIPSETTING(      0x0000, "x3.0" )                  /* 150 100  75  45  15 */
	PORT_DIPNAME( 0xe000, 0xe000, "Payout %" )              PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(      0xe000, "90%" )
	PORT_DIPSETTING(      0xc000, "85%" )
	PORT_DIPSETTING(      0xa000, "80%" )
	PORT_DIPSETTING(      0x8000, "75%" )
	PORT_DIPSETTING(      0x6000, "70%" )
	PORT_DIPSETTING(      0x4000, "65%" )
	PORT_DIPSETTING(      0x2000, "60%" )
	PORT_DIPSETTING(      0x0000, "55%" )

	PORT_START("SYSTEM")    /* Coins */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW,  IPT_UNUSED )          // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME( "Reset Credits" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( wondstck )
	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0010, 0x0010, "Helps" )                 PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	PORT_START("SYSTEM")    /* Coins */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW,  IPT_UNUSED )          // tested in service mode

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

TILE_GET_INFO_MEMBER(nmg5_state::fg_get_tile_info){ SET_TILE_INFO_MEMBER(0, m_fg_videoram[tile_index] | (m_gfx_bank << 16), 0, 0);}
TILE_GET_INFO_MEMBER(nmg5_state::bg_get_tile_info){ SET_TILE_INFO_MEMBER(0, m_bg_videoram[tile_index] | (m_gfx_bank << 16), 1, 0);}

void nmg5_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmg5_state::bg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nmg5_state::fg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap->set_transparent_pen(0);
}



void nmg5_state::draw_bitmap( bitmap_ind16 &bitmap )
{
	int yyy = 256;
	int xxx = 512 / 4;
	UINT16 x, y, count;
	int xoff = -12;
	int yoff = -9;
	int pix;

	count = 0;
	for (y = 0; y < yyy; y++)
	{
		for (x = 0; x < xxx; x++)
		{
			pix = (m_bitmap[count] & 0xf000) >> 12;
			if (pix) bitmap.pix16(y + yoff, x * 4 + 0 + xoff) = pix + 0x300;
			pix = (m_bitmap[count] & 0x0f00) >> 8;
			if (pix) bitmap.pix16(y + yoff, x * 4 + 1 + xoff) = pix + 0x300;
			pix = (m_bitmap[count] & 0x00f0) >> 4;
			if (pix) bitmap.pix16(y + yoff, x * 4 + 2 + xoff) = pix + 0x300;
			pix = (m_bitmap[count] & 0x000f) >> 0;
			if (pix) bitmap.pix16(y + yoff, x * 4 + 3 + xoff) = pix + 0x300;

			count++;
		}
	}
}


UINT32 nmg5_state::screen_update_nmg5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, m_scroll_ram[3] + 9);
	m_bg_tilemap->set_scrollx(0, m_scroll_ram[2] + 3);
	m_fg_tilemap->set_scrolly(0, m_scroll_ram[1] + 9);
	m_fg_tilemap->set_scrollx(0, m_scroll_ram[0] - 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_priority_reg == 0)
	{
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_bitmap(bitmap);
	}
	else if (m_priority_reg == 1)
	{
		draw_bitmap(bitmap);
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (m_priority_reg == 2)
	{
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
		draw_bitmap(bitmap);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (m_priority_reg == 3)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
		draw_bitmap(bitmap);
	}
	else if (m_priority_reg == 7)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_bitmap(bitmap);
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	}
	return 0;
}


static const gfx_layout nmg5_layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(7,8),RGN_FRAC(6,8),RGN_FRAC(5,8),RGN_FRAC(4,8),RGN_FRAC(3,8),RGN_FRAC(2,8),RGN_FRAC(1,8),RGN_FRAC(0,8) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout pclubys_layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(1,4)+8,RGN_FRAC(2,4)+0,RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0,RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,RGN_FRAC(2,4)+8,RGN_FRAC(1,4)+0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16 },
	8*16
};

static const gfx_layout layout_16x16x5 =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(1,5),RGN_FRAC(4,5),RGN_FRAC(0,5) },
	{ 128,129,130,131,132,133,134,135, 0,1,2,3,4,5,6,7, },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( nmg5 )
	GFXDECODE_ENTRY( "gfx1", 0, nmg5_layout_8x8x8, 0x000,  2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x5,   0x200, 16 )
GFXDECODE_END

static GFXDECODE_START( pclubys )
	GFXDECODE_ENTRY( "gfx1", 0, pclubys_layout_8x8x8, 0x000,  2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x5,      0x200, 16 )
GFXDECODE_END


void nmg5_state::machine_start()
{
	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_priority_reg));
	save_item(NAME(m_input_data));
}

void nmg5_state::machine_reset()
{
	/* some games don't set the priority register so it should be hard-coded to a normal layout */
	m_priority_reg = 7;

	m_gfx_bank = 0;
	m_input_data = 0;
}

static MACHINE_CONFIG_START( nmg5, nmg5_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)   /* 16 MHz */
	MCFG_CPU_PROGRAM_MAP(nmg5_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nmg5_state,  irq6_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, 4000000)      /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(nmg5_sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(nmg5_state, screen_update_nmg5)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nmg5)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(1)
	MCFG_DECO_SPRITE_ISBOOTLEG(true)
	MCFG_DECO_SPRITE_FLIPALLX(1)
	MCFG_DECO_SPRITE_OFFSETS(0, 8)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 4000000) /* 4MHz */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 1000000 , OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( garogun, nmg5 )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pclubys_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(pclubys_sound_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pclubys, nmg5 )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pclubys_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(pclubys_sound_map)

	MCFG_GFXDECODE_MODIFY("gfxdecode", pclubys)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( searchp2, nmg5 )

	/* basic machine hardware */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(55) // !

	MCFG_GFXDECODE_MODIFY("gfxdecode", pclubys)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( 7ordi, nmg5 )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(pclubys_sound_map)
MACHINE_CONFIG_END


/*

Multi 5 / New Multi Game 5
Yunsung, 1997/1998

PCB Layout
----------

YS-1300
+------------------------------------------+
| 4.000MHz 6116  xra1           ufa1       |
|          xh15   M6295         uj1        |
| YM3014 YM3812 Z8400AB1        uf1        |
|                               uh1        |
|J                              uf2        |
|A DIP1        62256                       |
|M             62256    Actel   srom5 srom6|
|M                      A1020B  srom7 srom8|
|A     ub15             Actel   srom2 srom1|
|      ub16             A1020B  srom4 srom3|
|DIP2 MC68HC000P16                     6264|
|                                      6264|
| 16.00MHz 14.31818MHz                     |
+------------------------------------------+

Notes:
  68000 16MHz
  Z80A 4MHz
  K-664/K-666 & AD-65 (YM3014/YM3812 & OKI M6295)
  Actel A1020B PL84C (x2)
  4.000MHz, 16.000MHz & 14.31818MHz OSCs

*/

ROM_START( nmg5 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "ub15.bin", 0x000000, 0x80000, CRC(36af3e2f) SHA1(735aaa901290b1d921242869e81e59649905eb30) )
	ROM_LOAD16_BYTE( "ub16.bin", 0x000001, 0x80000, CRC(2d9923d4) SHA1(e27549da311244db14ae1d3ad5e814a731a0f440) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "xh15.bin", 0x00000, 0x10000, CRC(12d047c4) SHA1(3123b1856219380ff598a2fad97a66863e30d80f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "srom1.bin", 0x000000, 0x80000, CRC(6771b694) SHA1(115e5eb45bb05f7a8021b3af3b8e709bbdcae55e) )
	ROM_LOAD( "srom2.bin", 0x080000, 0x80000, CRC(362d33af) SHA1(abf66ab9eabdd40fcd47bc291d60e7e4903cde74) )
	ROM_LOAD( "srom3.bin", 0x100000, 0x80000, CRC(8bad69d1) SHA1(c68d6b318e86b6deb64cc0cd5b51a2ea3ce04fb8) )
	ROM_LOAD( "srom4.bin", 0x180000, 0x80000, CRC(e73a7fcb) SHA1(b6213c0da61ba1c6dbe975365bcde17c71ea3388) )
	ROM_LOAD( "srom5.bin", 0x200000, 0x80000, CRC(7300494e) SHA1(031a74d7a82d23cdd5976b88379b9119322da1a0) )
	ROM_LOAD( "srom6.bin", 0x280000, 0x80000, CRC(74b5fdf9) SHA1(1c0e82a0e3cc1006b902e509076bbea04618320b) )
	ROM_LOAD( "srom7.bin", 0x300000, 0x80000, CRC(bd2b9036) SHA1(28c2d86c9645e6738811f3ece7c2fa02cd6ae4a1) )
	ROM_LOAD( "srom8.bin", 0x380000, 0x80000, CRC(dd38360e) SHA1(be7cb62369513b972c4370adf78df6fcf8caea0a) )

	ROM_REGION( 0x140000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "uf1.bin", 0x000000, 0x40000, CRC(9a9fb6f4) SHA1(4541d33493b9bba11b8e5ed35431271790763db4) )
	ROM_LOAD( "uf2.bin", 0x040000, 0x40000, CRC(66954d63) SHA1(62a315640beb8b063886ea6ed1433a18f75e8d0d) )
	ROM_LOAD( "ufa1.bin",0x080000, 0x40000, CRC(ba73ed2d) SHA1(efd2548fb0ada11ff98b73335689d2394cbf42a4) )
	ROM_LOAD( "uh1.bin", 0x0c0000, 0x40000, CRC(f7726e8e) SHA1(f28669725609ffab7c6c3bfddbe293c55ddd0155) )
	ROM_LOAD( "uj1.bin", 0x100000, 0x40000, CRC(54f7486e) SHA1(88a237a1005b1fd70b6d8544ef60a0d16cb38e6f) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "xra1.bin", 0x00000, 0x20000, CRC(c74a4f3e) SHA1(2f6165c1d5bdd3e816b95ffd9303dd4bd07f7ac8) )
ROM_END

ROM_START( nmg5a )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "m5_p1.ub15", 0x000000, 0x80000, CRC(0d63a21d) SHA1(e669d0d280573a4e05ee7dbacb4e0bf70880af6e) )
	ROM_LOAD16_BYTE( "m5_p2.ub16", 0x000001, 0x80000, CRC(230438db) SHA1(8ccd6a225a37b02afdcc987168f74b9fa568c71b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "m5_sndcpu.xh15", 0x00000, 0x10000, CRC(12d047c4) SHA1(3123b1856219380ff598a2fad97a66863e30d80f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "m5_12.srom1", 0x000000, 0x80000, CRC(3adff261) SHA1(c72d45a91cc03872fd3c5e1c7328097a48aac115) )
	ROM_LOAD( "m5_8.srom2",  0x080000, 0x80000, CRC(b0736b66) SHA1(c8223aca5ef03348c132ff0625a43a7e56eccaee) )
	ROM_LOAD( "m5_13.srom3", 0x100000, 0x80000, CRC(8e904919) SHA1(f6b0b92ccfaaf1b1129c433f6a54399fc0c0ad44) )
	ROM_LOAD( "m5_9.srom4",  0x180000, 0x80000, CRC(779e0e30) SHA1(bb703ebe1bb48c4a1ae1c3e86e18db853d5f1816) )
	ROM_LOAD( "m5_6.srom5",  0x200000, 0x80000, CRC(41061258) SHA1(85f55e9e8c67e514c890e88e5719b88399d9ce99) )
	ROM_LOAD( "m5_10.srom6", 0x280000, 0x80000, CRC(8147d8ef) SHA1(d07300fc6b2bedbe8f5d09f8bb9becb14dd61a7c) )
	ROM_LOAD( "m5_7.srom7",  0x300000, 0x80000, CRC(acb00d15) SHA1(b03d5f960ed527ac1132dbaa01539462cb325aa6) )
	ROM_LOAD( "m5_11.srom8", 0x380000, 0x80000, CRC(0ba74fce) SHA1(62215eee4eb7100029ae3344e5a6d03da523eede))

	ROM_REGION( 0x140000, "gfx2", 0 )   /* 16x16x5 */ // same as parent set
	ROM_LOAD( "m5_3.uf1", 0x000000, 0x40000, CRC(9a9fb6f4) SHA1(4541d33493b9bba11b8e5ed35431271790763db4) )
	ROM_LOAD( "m5_5.uf2", 0x040000, 0x40000, CRC(66954d63) SHA1(62a315640beb8b063886ea6ed1433a18f75e8d0d) )
	ROM_LOAD( "m5_1.ufa1",0x080000, 0x40000, CRC(ba73ed2d) SHA1(efd2548fb0ada11ff98b73335689d2394cbf42a4) )
	ROM_LOAD( "m5_4.uh1", 0x0c0000, 0x40000, CRC(f7726e8e) SHA1(f28669725609ffab7c6c3bfddbe293c55ddd0155) )
	ROM_LOAD( "m5_2.uj1", 0x100000, 0x40000, CRC(54f7486e) SHA1(88a237a1005b1fd70b6d8544ef60a0d16cb38e6f) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "m5_oki.xra1", 0x00000, 0x20000, CRC(c74a4f3e) SHA1(2f6165c1d5bdd3e816b95ffd9303dd4bd07f7ac8) )
ROM_END

ROM_START( nmg5e )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "ub15.rom", 0x000000, 0x80000, CRC(578516e2) SHA1(87785e0071c62f17664e875d95cd6124984b8080) )
	ROM_LOAD16_BYTE( "ub16.rom", 0x000001, 0x80000, CRC(12fab483) SHA1(3b6a410b730d8bf5a81470ec9cdc46c05da0721b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "xh15.bin", 0x00000, 0x10000, CRC(12d047c4) SHA1(3123b1856219380ff598a2fad97a66863e30d80f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "srom1.rom", 0x000000, 0x80000, CRC(6df3e0c2) SHA1(855e7d3a75d18c92cfb18ddbe7d110ae6879870d) )
	ROM_LOAD( "srom2.rom", 0x080000, 0x80000, CRC(3caf65f9) SHA1(da0d417ab6b57fb33e7ed62d4e00b47698764e11) )
	ROM_LOAD( "srom3.rom", 0x100000, 0x80000, CRC(812f3f87) SHA1(157270343674265ef7d6415a970084ba05daf061) )
	ROM_LOAD( "srom4.rom", 0x180000, 0x80000, CRC(edb8299d) SHA1(ab5c1195bf2229b50992994031eb8e9a6847b652) )
	ROM_LOAD( "srom5.rom", 0x200000, 0x80000, CRC(79821f18) SHA1(228de566f47339d30caf5908f78368ed45ba8da3) )
	ROM_LOAD( "srom6.rom", 0x280000, 0x80000, CRC(7e37abaf) SHA1(16361d08f6da49b5ee57185febd1dc14a609f415) )
	ROM_LOAD( "srom7.rom", 0x300000, 0x80000, CRC(b7a9c660) SHA1(d4f39e99813cd2635a95bd05a3776587a2f9351b) )
	ROM_LOAD( "srom8.rom", 0x380000, 0x80000, CRC(d7ba6058) SHA1(31739fc233ad9b565631b72aab7f9f5a70eca15f) )

	ROM_REGION( 0x140000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "uf1.rom", 0x000000, 0x40000, CRC(502dbd65) SHA1(b44c7fa61180c807b55f5fd12ef9cba82b0fe18b) )
	ROM_LOAD( "uf2.rom", 0x040000, 0x40000, CRC(6744cca0) SHA1(f1a45c9b5cd6f4131511910199f71e98a79a4d97) )
	ROM_LOAD( "ufa1.rom",0x080000, 0x40000, CRC(7110677f) SHA1(8a644223bbf87af446796347d6b0309b439b43dc) )
	ROM_LOAD( "uh1.rom", 0x0c0000, 0x40000, CRC(f6a3ef4d) SHA1(9d5c011ec5eb69822905d0872d5baf55dc76ca39) )
	ROM_LOAD( "uj1.rom", 0x100000, 0x40000, CRC(0595d8ef) SHA1(b2f2fbd8cf6dce6358db9cd976aa7d117cafb5ae) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "xra1.bin", 0x00000, 0x20000, CRC(c74a4f3e) SHA1(2f6165c1d5bdd3e816b95ffd9303dd4bd07f7ac8) )
ROM_END

ROM_START( searchey )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "se.u7", 0x000000, 0x40000, CRC(332b0d83) SHA1(8b79f792a0e4bfd0c64744cdcf3be1daf29910d3) ) /* World set?? Supports 4 languages */
	ROM_LOAD16_BYTE( "se.u2", 0x000001, 0x40000, CRC(bd16114e) SHA1(596fb8841168af88a02dc5f028f5731be5fa08a6) ) /* English, Korean, Japanese & Italian */

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "u128.bin", 0x00000, 0x10000, CRC(85bae10c) SHA1(a1e58d8b8c8718cc346aae400bb4eadf6873b86d) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "u63.bin", 0x000000, 0x80000, CRC(1b0b7b7d) SHA1(092855407fef95da69fcd6e608b8b3aa720d8bcd) )
	ROM_LOAD( "u68.bin", 0x080000, 0x80000, CRC(ae18b2aa) SHA1(f1b2d3c1bafe99ec8b7e8e587ae0a0f9fa410a5a) )
	ROM_LOAD( "u73.bin", 0x100000, 0x80000, CRC(ab7f8716) SHA1(c8cc3c1e9c37add31af28c43130b66f7fdd28042) )
	ROM_LOAD( "u79.bin", 0x180000, 0x80000, CRC(7f2c8b83) SHA1(4f25bf5652ad3327efe63d960b987e581d20afbb) )
	ROM_LOAD( "se.u64",  0x200000, 0x80000, CRC(32b7e4f3) SHA1(76a01b802bd4f13926cc0c5f8388962a89e45c6e) )
	ROM_LOAD( "u69.bin", 0x280000, 0x80000, CRC(d546eaf8) SHA1(de6c80b733c31ef2c0c64d25d46f1cff9a262c42) )
	ROM_LOAD( "u74.bin", 0x300000, 0x80000, CRC(e6134d84) SHA1(d639f44ef404e206b25a0b4f71ded3854836c60f) )
	ROM_LOAD( "u80.bin", 0x380000, 0x80000, CRC(9a160918) SHA1(aac63dcb6005eaad7088d4e4e584825a6e232764) )

	ROM_REGION( 0x0a0000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "u83.bin", 0x000000, 0x20000, CRC(c5a1c647) SHA1(c8d1cc631b0286a4caa35dce6552c4206e58b620) )
	ROM_LOAD( "u82.bin", 0x020000, 0x20000, CRC(25b2ae62) SHA1(02a1bd8719ca1792c2e4ff52fd5d4845e19fedb7) )
	ROM_LOAD( "u105.bin",0x040000, 0x20000, CRC(b4207ef0) SHA1(e70a73b98e5399221208d81a324fab6b942470c8) )
	ROM_LOAD( "u96.bin", 0x060000, 0x20000, CRC(8c40818a) SHA1(fe2c0da42154261ae1734ddb6cb9ddf33dd58510) )
	ROM_LOAD( "u97.bin", 0x080000, 0x20000, CRC(5dc7f231) SHA1(5e57e436a24dfa14228bac7b8ae5f000393926b9) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u137.bin", 0x00000, 0x40000,  CRC(49105e23) SHA1(99543fbbccf5df5b15a0470eac641b4158024c6a) )
ROM_END

ROM_START( searcheya )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u7.bin", 0x000000, 0x40000, CRC(287ce3dd) SHA1(32305f7b09c58b7f126d41b5b1991e349884cc02) ) /* Korean set?? only supports English & Korean */
	ROM_LOAD16_BYTE( "u2.bin", 0x000001, 0x40000, CRC(b574f033) SHA1(8603926cef9df2495e97a071f08bbf418b9e01a8) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "u128.bin", 0x00000, 0x10000, CRC(85bae10c) SHA1(a1e58d8b8c8718cc346aae400bb4eadf6873b86d) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "u63.bin", 0x000000, 0x80000, CRC(1b0b7b7d) SHA1(092855407fef95da69fcd6e608b8b3aa720d8bcd) )
	ROM_LOAD( "u68.bin", 0x080000, 0x80000, CRC(ae18b2aa) SHA1(f1b2d3c1bafe99ec8b7e8e587ae0a0f9fa410a5a) )
	ROM_LOAD( "u73.bin", 0x100000, 0x80000, CRC(ab7f8716) SHA1(c8cc3c1e9c37add31af28c43130b66f7fdd28042) )
	ROM_LOAD( "u79.bin", 0x180000, 0x80000, CRC(7f2c8b83) SHA1(4f25bf5652ad3327efe63d960b987e581d20afbb) )
	ROM_LOAD( "u64.bin", 0x200000, 0x80000, CRC(322a903c) SHA1(e3b00576edf58c7743854de95102d3d36ce3b775) )
	ROM_LOAD( "u69.bin", 0x280000, 0x80000, CRC(d546eaf8) SHA1(de6c80b733c31ef2c0c64d25d46f1cff9a262c42) )
	ROM_LOAD( "u74.bin", 0x300000, 0x80000, CRC(e6134d84) SHA1(d639f44ef404e206b25a0b4f71ded3854836c60f) )
	ROM_LOAD( "u80.bin", 0x380000, 0x80000, CRC(9a160918) SHA1(aac63dcb6005eaad7088d4e4e584825a6e232764) )

	ROM_REGION( 0x0a0000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "u83.bin", 0x000000, 0x20000, CRC(c5a1c647) SHA1(c8d1cc631b0286a4caa35dce6552c4206e58b620) )
	ROM_LOAD( "u82.bin", 0x020000, 0x20000, CRC(25b2ae62) SHA1(02a1bd8719ca1792c2e4ff52fd5d4845e19fedb7) )
	ROM_LOAD( "u105.bin",0x040000, 0x20000, CRC(b4207ef0) SHA1(e70a73b98e5399221208d81a324fab6b942470c8) )
	ROM_LOAD( "u96.bin", 0x060000, 0x20000, CRC(8c40818a) SHA1(fe2c0da42154261ae1734ddb6cb9ddf33dd58510) )
	ROM_LOAD( "u97.bin", 0x080000, 0x20000, CRC(5dc7f231) SHA1(5e57e436a24dfa14228bac7b8ae5f000393926b9) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u137.bin", 0x00000, 0x40000,  CRC(49105e23) SHA1(99543fbbccf5df5b15a0470eac641b4158024c6a) )
ROM_END

/*

Search Eye Plus V2.0
Yunsung, 1999

PCB Layout
----------

YS-1905
|----------------------------------------------------|
|  U128       U137      U7       62256        U97    |
|  6116   Z80  M6295    U2       62256               |
|  YM3812             PAL                     U96    |
| YM3014       16MHz  PAL   PAL                      |
| 324                  |-----|                U105   |
|         62256        |68000|                       |
|J        62256 6116   |     |                U83    |
|A              6116   |-----|                       |
|M              6116  |------|  6116          U82    |
|M              6116  |QL2003|            14.31818MHz|
|A              62256 |      |  6116   PAL   PAL     |
|               62256 |------|         PAL           |
|            |------|                  PAL           |
|            |QL2003| |--------|       PAL           |
|DSW1  DSW2  |      | |QL12X16B|       PAL           |
|            |------| |        |                     |
|                     |        |                     |
|0.U1   2.U3 |------| |--------|                     |
|            |QL2003|                       PAL      |
|1.U2   3.U4 |      |                       PAL      |
|            |------|             PAL                |
|----------------------------------------------------|

Notes:
      68000  @ 16MHz
      Z80    @ 4.0MHz [16/4]
      YM3812 @ 4.0MHz [16/4] (badged as U6612 & U6614 for YM3014)
      M6295  @ 1.0MHz [16/16]. Pin 7 HIGH
      VSync    55Hz
      HSync    25.0kHz (!)

*/

ROM_START( searchp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u7", 0x000000, 0x80000, CRC(37fe9e18) SHA1(ddb5c8d7cc68823850af8a186a4500688115b00f) )
	ROM_LOAD16_BYTE( "u2", 0x000001, 0x80000, CRC(8278513b) SHA1(a48870dc27147e0e9d9d76286028fab1088fa57a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "u128", 0x00000, 0x10000, CRC(85bae10c) SHA1(a1e58d8b8c8718cc346aae400bb4eadf6873b86d) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* 8x8x8 */
	ROM_LOAD( "0.u1", 0x000000, 0x400000, CRC(28a50dcf) SHA1(d00992675115e0187a9ca2193edfccdc6e03a0ed) )
	ROM_LOAD( "2.u3", 0x400000, 0x400000, CRC(30d46e19) SHA1(e3b3d0c5eed29e104f2ecf89541fb74da4f2980f) )
	ROM_LOAD( "1.u2", 0x800000, 0x400000, CRC(f9c4e824) SHA1(2b3216054060bc349dba87401ce9d8c5e0b60101) )
	ROM_LOAD( "3.u4", 0xc00000, 0x400000, CRC(619f142f) SHA1(6568e8b2d103bf6aded1314f270c2d288700815e) )

	ROM_REGION( 0x140000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "u83", 0x000000, 0x40000, CRC(2bae34cb) SHA1(8d18ca44033e064dbf20381158f5bd53931a7362) )
	ROM_LOAD( "u82", 0x040000, 0x40000, CRC(5cb773f0) SHA1(f3b69073998c9521661c3cffa5d6d022172f30e6) )
	ROM_LOAD( "u105",0x080000, 0x40000, CRC(e8adb15e) SHA1(f80a030c394fa4e48d569bbcfe945bb22551d542) )
	ROM_LOAD( "u96", 0x0c0000, 0x40000, CRC(67efb536) SHA1(fb7a829245f1418865637329cbb78acf63e5640b) )
	ROM_LOAD( "u97", 0x100000, 0x40000, CRC(f7b63826) SHA1(457b836b765c31dc7a162e0d072524de4bebba31) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u137", 0x00000, 0x40000,  CRC(cd037524) SHA1(1f4accc909d73068fe16feb27dfe0c6f19234af1) )
ROM_END

/*

Puzzle Club
Yunsung, 2000

PCB Layout
----------

YS-2113
|----------------------------------------------|
|ROM1.128 ROM2.137 ROM3.7  62256  ROM5.97      |
|  6116 Z80  6295  ROM4.2  62256  ROM6.95      |
|  YM3812   16MHz     PAL         ROM7.105     |
|  YM3014             PAL  PAL    ROM8.83      |
|          62256    68000         ROM9.82      |
|          62256           6116     14.38383MHz|
|                          6116   PAL     PAL  |
|J           6116   QL2003        PAL          |
|A           6116                 PAL          |
|M           6116   QL12X16B      PAL          |
|M DIP1      6116                 PAL          |
|A DIP2                                        |
|            62256  QL2003                     |
|            62256                             |
|                                              |
|  ROM11.166  ROM10.167                        |
|                   QL2003        PAL          |
|  ROM13.164  ROM12.165           PAL          |
|----------------------------------------------|

Notes:
      68000 clock : 16.000MHz
      Z80 clock   : 4.000MHz (16/4)
      M6295 clock : 1.000MHz (16/16). Sample Rate = 1000000 / 132
      YM3812 clock: 4.000MHz (16/4).
      VSync       : 60Hz

*/

ROM_START( pclubys )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rom3.7", 0x000000, 0x80000, CRC(62e28e6d) SHA1(30307dfbb6bd02d78fb06d3c3522b41115f1c27a) )
	ROM_LOAD16_BYTE( "rom4.2", 0x000001, 0x80000, CRC(b51dab41) SHA1(2ad3929c8cf2b66c36289c2c851769190916b718) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "rom1.128", 0x00000, 0x10000, CRC(25cd27f8) SHA1(97af1368381234361bbd97f4552209c435652372) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* 8x8x8 */
	ROM_LOAD( "rom10.167", 0x000000, 0x400000, CRC(d67e8e84) SHA1(197d1bdb321cf7ac986b5dbfa061494ffd4db6e4) )
	ROM_LOAD( "rom12.165", 0x400000, 0x400000, CRC(6be8b733) SHA1(bdbbec77938828ac9831537c00abd5c31dc56284) )
	ROM_LOAD( "rom11.166", 0x800000, 0x400000, CRC(672501a4) SHA1(193e1965c2f21f469e5c6c514d3cdcab3ffdf629) )
	ROM_LOAD( "rom13.164", 0xc00000, 0x400000, CRC(fc725ce7) SHA1(d997a31a975ae67efa071720c58235c738ebbbe3) )

	ROM_REGION( 0x280000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "rom8.83", 0x000000, 0x80000, CRC(651af101) SHA1(350698bd7ee65fc1ed084382db7f66ffb83c23c6) )
	ROM_LOAD( "rom9.82", 0x080000, 0x80000, CRC(2535b4d6) SHA1(85af6a042e83f8a7abb78c5edfd4497a9018ed68) )
	ROM_LOAD( "rom7.105",0x100000, 0x80000, CRC(f7536c52) SHA1(546976f52c6f064f5172736988ada053c1c1183f) )
	ROM_LOAD( "rom6.95", 0x180000, 0x80000, CRC(3c078a52) SHA1(be8936bcafbfd77e491c81a3d215a53dad78d652) )
	ROM_LOAD( "rom5.97", 0x200000, 0x80000, CRC(20eae2f8) SHA1(ad9ac6e5e0331fb19652f6578dbb2f532bb22b3d) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rom2.137", 0x00000, 0x80000, CRC(4ff97ad1) SHA1(d472c8298e428cb9659ce90a8ce9402c119bdb0f) )
ROM_END

ROM_START( pclubysa )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rom3a.7", 0x000000, 0x80000, CRC(885aa07a) SHA1(a0af5b0704f7fb18ed21f42979a40a8b419377b1) )
	ROM_LOAD16_BYTE( "rom4a.2", 0x000001, 0x80000, CRC(9bfbdeac) SHA1(263341b05883d4a9125da69d9d8d6f4d654f3475) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "rom1.128", 0x00000, 0x10000, CRC(25cd27f8) SHA1(97af1368381234361bbd97f4552209c435652372) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* 8x8x8 */
	ROM_LOAD( "rom10.167", 0x000000, 0x400000, CRC(d67e8e84) SHA1(197d1bdb321cf7ac986b5dbfa061494ffd4db6e4) )
	ROM_LOAD( "rom12.165", 0x400000, 0x400000, CRC(6be8b733) SHA1(bdbbec77938828ac9831537c00abd5c31dc56284) )
	ROM_LOAD( "rom11.166", 0x800000, 0x400000, CRC(672501a4) SHA1(193e1965c2f21f469e5c6c514d3cdcab3ffdf629) )
	ROM_LOAD( "rom13.164", 0xc00000, 0x400000, CRC(fc725ce7) SHA1(d997a31a975ae67efa071720c58235c738ebbbe3) )

	ROM_REGION( 0x280000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "rom8.83", 0x000000, 0x80000, CRC(651af101) SHA1(350698bd7ee65fc1ed084382db7f66ffb83c23c6) )
	ROM_LOAD( "rom9.82", 0x080000, 0x80000, CRC(2535b4d6) SHA1(85af6a042e83f8a7abb78c5edfd4497a9018ed68) )
	ROM_LOAD( "rom7.105",0x100000, 0x80000, CRC(f7536c52) SHA1(546976f52c6f064f5172736988ada053c1c1183f) )
	ROM_LOAD( "rom6.95", 0x180000, 0x80000, CRC(3c078a52) SHA1(be8936bcafbfd77e491c81a3d215a53dad78d652) )
	ROM_LOAD( "rom5.97", 0x200000, 0x80000, CRC(20eae2f8) SHA1(ad9ac6e5e0331fb19652f6578dbb2f532bb22b3d) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rom2.137", 0x00000, 0x80000, CRC(4ff97ad1) SHA1(d472c8298e428cb9659ce90a8ce9402c119bdb0f) )
ROM_END

ROM_START( wondstck )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u2.bin", 0x000001, 0x20000, CRC(9995b743) SHA1(178afd9c54758dd4fb4fb7debe4da2af5c10410a) )
	ROM_LOAD16_BYTE( "u4.bin", 0x000000, 0x20000, CRC(46a3e9f6) SHA1(f39b6457b2c5772db16a5ba29d9114671e3d9749) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "u128.bin", 0x00000, 0x10000, CRC(86dba085) SHA1(6dedfb4bcf890490848409b6d9bce69e72bf1bba) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "u63.bin", 0x000000, 0x80000, CRC(c6cf09b4) SHA1(d3341c1effa7874b0554e5c79985b32198571767) )
	ROM_LOAD( "u68.bin", 0x080000, 0x80000, CRC(2e9e9a5e) SHA1(27bae2d913ad4c569f4c476d954c47665456e818) )
	ROM_LOAD( "u73.bin", 0x100000, 0x80000, CRC(3a828604) SHA1(562ecfc1b485218b861512c62225e7d0e148a6df) )
	ROM_LOAD( "u79.bin", 0x180000, 0x80000, CRC(0cca46af) SHA1(a22dcc9f59953cc0c75048b0fa3d7834eea76432) )
	ROM_LOAD( "u64.bin", 0x200000, 0x80000, CRC(dcec9ac5) SHA1(3619d7643932264eac2fbbf95581f6ff3e2829b1) )
	ROM_LOAD( "u69.bin", 0x280000, 0x80000, CRC(27b9d708) SHA1(930f6b742b45b09c5cba80c78bf64eb2b01243e0) )
	ROM_LOAD( "u74.bin", 0x300000, 0x80000, CRC(7eff8e2f) SHA1(a08d188fc1a549ba684e69adb277ef684c6d875c) )
	ROM_LOAD( "u80.bin", 0x380000, 0x80000, CRC(1160a0c2) SHA1(b23f6fb256b927a5606a1aacf004727b984807de) )

	ROM_REGION( 0x280000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "u83.bin", 0x000000, 0x80000, CRC(f51cf9c6) SHA1(6d0fc749bab918ff6a9d7fae8be7c65823349283) )
	ROM_LOAD( "u82.bin", 0x080000, 0x80000, CRC(ddd3c60c) SHA1(19b68a44c877d0bf630d07b18541ef9636f5adac) )
	ROM_LOAD( "u105.bin",0x100000, 0x80000, CRC(a7fc624d) SHA1(b336ab6e16555db30f9366bf5b797b5ba3ea767c) )
	ROM_LOAD( "u96.bin", 0x180000, 0x80000, CRC(2369d8a3) SHA1(4224f50c9c31624dfcac6215d60a2acdd39bb477) )
	ROM_LOAD( "u97.bin", 0x200000, 0x80000, CRC(aba1bd94) SHA1(28dce35ad92547a54912c5645e9979c0876d6fe8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u137.bin", 0x000000, 0x40000, CRC(294b6cbd) SHA1(1498a3298c53d62f56f9c85c82035d09a5bb8b4a) )
ROM_END

ROM_START( garogun )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "p1.u7", 0x000000, 0x80000, CRC(9b5627f8) SHA1(d336d4f34de7fdf5ba16bc76223e701369d24a5e) )
	ROM_LOAD16_BYTE( "p2.u2", 0x000001, 0x80000, CRC(1d2ff271) SHA1(6b875be42f945b5793ba41ff20e23dacf8eb6a9a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "rom.u128", 0x00000, 0x10000,  CRC(117b31ce) SHA1(1681aea60111274599c86b7050d46ea497878f9e) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* 8x8x8 */
	ROM_LOAD( "8.u63",  0x000000, 0x80000, CRC(2d152d32) SHA1(7dd3b0bb9db8cec8ff8a75099375fbad51ee5676) )
	ROM_LOAD( "11.u68", 0x080000, 0x80000, CRC(60ec7f67) SHA1(9c6c3f5a5be244fe5ac24da3642fd666def11120) )
	ROM_LOAD( "9.u73",  0x100000, 0x80000, CRC(a4b16319) SHA1(8f7976f58ecbccd728cf1a01d0fcd1cef6b90d47) )
	ROM_LOAD( "13.u79", 0x180000, 0x80000, CRC(2dc14fb6) SHA1(d6a01e8bb0ce2f94c1562d8302f62fbbb86e5fa0) )
	ROM_LOAD( "6.u64",  0x200000, 0x80000, CRC(a0fc7547) SHA1(91249226b9491085d15216c11e00f39b03f9128a) )
	ROM_LOAD( "10.u69", 0x280000, 0x80000, CRC(e5dc36c3) SHA1(370b5f93d2f425fe59a519dafce9cb859bd7b609) )
	ROM_LOAD( "7.u74",  0x300000, 0x80000, CRC(a0574f8d) SHA1(3016dd5ee2c78eb2e16b396cedcdc69151312a06) )
	ROM_LOAD( "12.u80", 0x380000, 0x80000, CRC(94d66169) SHA1(4a84d46caa7da98ac376965d6e1ebe1d26fda542) )

	ROM_REGION( 0x280000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "4.u83", 0x000000, 0x40000, CRC(3d1d46ff) SHA1(713beb8cb5b105900d59380f760d759e94f4b0b2) )
	ROM_LOAD( "5.u82", 0x080000, 0x40000, CRC(2a7b2fb5) SHA1(f860047d78f625592605f425e9e066c3e595be62) )
	ROM_LOAD( "3.u105",0x100000, 0x40000, CRC(cd20e39c) SHA1(beb129a44223cc542906f96e5bb17aabfe4c4c49) )
	ROM_LOAD( "2.u96", 0x180000, 0x40000, CRC(4df3b502) SHA1(638138e09d69390c899f48ae59dcd116c1b338c7) )
	ROM_LOAD( "1.u97", 0x200000, 0x40000, CRC(591b3efe) SHA1(ea7d2f2802effa6895e02f50cc9f7c189a720ef5) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s.u137", 0x00000, 0x80000, CRC(3eadc21a) SHA1(b1c131c3f59adbc370696b277f8f04681212761d) )
ROM_END

/*

7 Ordi
Yunsung, 2002 (Sticker is printed 2002. 1. 05)

PCB Layout
----------

YS-2118B
+----------------------------------------------+
|  5.u128   3.u137   p1.u7  62256   1.u97      |
|  6116   Z80  6295  p2.u2  62256   2.u95      |
|  YM3812   16MHz     PAL           3.u105     |
|  YM3014             PAL  PAL      4.u83      |
|          62256    68000 BAT       5.u82      |
|          62256                    14.38383MHz|
|                           6116  PAL     PAL  |
|J           6116           6116  PAL          |
|A           6116  A40MX04-F      PAL          |
|M           6116                 PAL          |
|M           6116                 PAL          |
|A DIP1 DIP2                                   |
|            62256  QL12X16B                   |
|            62256                             |
|                                              |
|     6.u64 10.u69  A40MX04-F                  |
|     7.u74 11.u68                             |
| SW1 8.u63 12.u80  A40MX04-F    PAL           |
|     9.u73 13.u79               PAL           |
+----------------------------------------------+

Notes (originaly for YS-2113, but should apply):
      68000 clock : 16.000MHz
      Z80 clock   : 4.000MHz (16/4)
      M6295 clock : 1.000MHz (16/16). Sample Rate = 1000000 / 132
      YM3812 clock: 4.000MHz (16/4). (badged as U6612 and U6614 for the YM3014)
      VSync       : 60Hz

*/

ROM_START( 7ordi )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "p1.u7", 0x000000, 0x20000, CRC(ebf21862) SHA1(ffbea41adb3f2ab276b2785bd6f98bb6ac622edd) )
	ROM_LOAD16_BYTE( "p2.u2", 0x000001, 0x20000, CRC(f7943a6a) SHA1(1d36d92c0d349394ba71929215b704d34e5be87e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )        /* Z80 Code */
	ROM_LOAD( "4.u128", 0x00000, 0x10000, CRC(ed73b565) SHA1(cb473b2b4ca9b9facf3bcb033f1ca9667bb5c587) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* 8x8x8 */
	ROM_LOAD( "8.u63",  0x000000, 0x80000, CRC(ed8dfe5d) SHA1(a4ac6bf80682b978158c44a62c8abb117f25c4db) )
	ROM_LOAD( "11.u68", 0x080000, 0x80000, CRC(742764a7) SHA1(c38a7ea76034d5c34d30f6caa6dc12b1253de2a8) )
	ROM_LOAD( "9.u73",  0x100000, 0x80000, CRC(2b76efd0) SHA1(3ae0b708a94ed04516f451be833ea02d9ee8d645) )
	ROM_LOAD( "13.u79", 0x180000, 0x80000, CRC(3892b356) SHA1(2be4479bdce2cbff6cfa26ab1d58aef2923614cd) )
	ROM_LOAD( "6.u64",  0x200000, 0x80000, CRC(5c0b0838) SHA1(f91be789ea90c64871a09ca246dcd680fe986ef1) )
	ROM_LOAD( "10.u69", 0x280000, 0x80000, CRC(c15db1a4) SHA1(3377c2ffd46ac5e3b4e7852a43aa9eaad3b0b8f8) )
	ROM_LOAD( "7.u74",  0x300000, 0x80000, CRC(6910f754) SHA1(b0c29a205e66f21ff1bebac79505d1d3170d923f) )
	ROM_LOAD( "12.u80", 0x380000, 0x80000, CRC(4c5dd9ef) SHA1(e22b2d5652ad97c8d84168f8c4851437e3f07c97) )

	ROM_REGION( 0x280000, "gfx2", 0 )   /* 16x16x5 */
	ROM_LOAD( "4.u83",  0x000000, 0x80000, CRC(a2569cf4) SHA1(7f65a0ea79c38aa89d06466b0ce5e3846073c676) )
	ROM_LOAD( "5.u82",  0x080000, 0x80000, CRC(045e548e) SHA1(7135ce56c3987f3d7e0514670836603fc95dfc84) )
	ROM_LOAD( "3.u105", 0x100000, 0x80000, CRC(04c1dbf9) SHA1(afca98aeac6095992611eaaa958952a40a0ffd23) )
	ROM_LOAD( "2.u96",  0x180000, 0x80000, CRC(11fa7de8) SHA1(a5a29589f37899720901c6f802390f91ce308d87) )
	ROM_LOAD( "1.u97",  0x200000, 0x80000, CRC(cd1ffe88) SHA1(f5dfc119f2811e7cae920637a24c25f9d1f7e8df) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "3.u137", 0x00000, 0x20000, CRC(669ed310) SHA1(f93dc3c20f86bd6a0bff9947d805358c82a04c97) )
	ROM_RELOAD(0x20000,0x20000)
	ROM_RELOAD(0x40000,0x20000) // it attempts to bank the sample rom even tho there is only 1 bank
	ROM_RELOAD(0x60000,0x20000)
ROM_END

DRIVER_INIT_MEMBER(nmg5_state,prot_val_00)
{
	m_prot_val = 0x00;
}

DRIVER_INIT_MEMBER(nmg5_state,prot_val_10)
{
	m_prot_val = 0x10;
}

DRIVER_INIT_MEMBER(nmg5_state,prot_val_20)
{
	m_prot_val = 0x20;
}

DRIVER_INIT_MEMBER(nmg5_state,prot_val_40)
{
	m_prot_val = 0x40;
}

GAME( 1998, nmg5,      0,        nmg5,     nmg5,      nmg5_state, prot_val_10, ROT0, "Yun Sung", "Multi 5 / New Multi Game 5 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, nmg5a,     nmg5,     nmg5,     nmg5,      nmg5_state, prot_val_10, ROT0, "Yun Sung", "Multi 5 / New Multi Game 5 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, nmg5e,     nmg5,     nmg5,     nmg5,      nmg5_state, prot_val_10, ROT0, "Yun Sung", "Multi 5 / New Multi Game 5 (set 3, earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, searchey,  0,        nmg5,     searchey,  nmg5_state, prot_val_10, ROT0, "Yun Sung", "Search Eye (English / Korean / Japanese / Italian)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, searcheya, searchey, nmg5,     searcheya, nmg5_state, prot_val_10, ROT0, "Yun Sung", "Search Eye (English / Korean)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, searchp2,  0,        searchp2, searchp2,  nmg5_state, prot_val_10, ROT0, "Yun Sung", "Search Eye Plus V2.0", MACHINE_SUPPORTS_SAVE )
GAME( 2000, pclubys,   0,        pclubys,  pclubys,   nmg5_state, prot_val_10, ROT0, "Yun Sung", "Puzzle Club (Yun Sung, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, pclubysa,  pclubys,  pclubys,  pclubys,   nmg5_state, prot_val_10, ROT0, "Yun Sung", "Puzzle Club (Yun Sung, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, garogun,   0,        garogun,  garogun,   nmg5_state, prot_val_40, ROT0, "Yun Sung", "Garogun Seroyang (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, 7ordi,     0,        7ordi,    7ordi,     nmg5_state, prot_val_20, ROT0, "Yun Sung", "7 Ordi (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( ????, wondstck,  0,        nmg5,     wondstck,  nmg5_state, prot_val_00, ROT0, "Yun Sung", "Wonder Stick", MACHINE_SUPPORTS_SAVE )
