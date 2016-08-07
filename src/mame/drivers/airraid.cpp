// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Angelo Salese, hap
/* Air Raid (aka Cross Shooter) (c) 1987 Seibu
  
  this driver is for the single board version on the S-0087-011A-0 PCB
  for the version using a S-0086-002-B0 base PCB and separate video board see stfight.cpp

  Custom Modules note:

  The Air Raid / Cross Shooter PCB contains 3 custom modules.
  These modules have not been dumped.

  2 of these are responsible for background layer generation.
  the 3rd is responsible for sprites.

  Both the tile gfx data, and tilemap layout data are
  contained within the background modules, none of this is
  accessible by the CPU (the CPU simply provides a scroll
  position)

  The sprite modules contain the sprite gfx data.

  The modules also contain the CLUT PROM data for their layer
  which controls colours and transparency.  It is likely
  each module contains the equivalent logic of a SEI0010BU
  in order to handle this.  This means there is a possibility
  that the raw data from the ROMs is not exposed on the edge
  of the modules, only post-lookup data.

  The above information is based off a development board
  which was unfortunately stripped of all ROMs.

  ---

 TS 01.05.2006:

 - added sprites, bgmap reading and few fixes here and there
   airraid and cshootere are a bit "playable" ;) without gfx

Haze's notes
 - video system is very similar to darkmist.cpp
 

Stephh's notes (based on the game Z80 code and some tests) :

  - Memory notes (100% guess) :

      * There are still some writes to the ROM area, but I think it's
        related to wrong interrupts and/or incomplete memory mapping.
      * Reads from 0xb0?? seem to be related to sound
      * Write to 0xc500 happens LOTS of time - related to scanlines ?
      * Write to 0xc600 might be used to disable the interrupts and
        the possible communication between CPUs (if they are 2)
      * Write to 0xc700 seems to be done when a coin is inserted
        (also done once during P.O.S.T. - unknown purpose here).
      * Write to 0xc801 might be sort of watchdog as it "pollutes"
        the error.log file.


  - Inputs notes :

      * COINx don't work correcly : see "cshooter_coin_r" read handler.
    * In game, bits 3 and 4 of 0xc202 ("START") are tested,
        while bits 4 and 5 are tested in the "test mode".
      * Pressing STARTx while in game adds lives (depending on the
        "Lives" Dip Switch) for player x.


  - Other notes :

      * 0x0006 contains the "region" stuff (value in 'cshooter' = 0xc4) :
          . bits 2 and 3 determine the manufacturer :
              0x00 : "J.K.H. Corp."         (no logo)
              0x04 : "Taito Corporation."   (+ logo)
              0x08 : "International Games"  (+ logo)
              0x0c : "Seibu Kaihatsu,Inc."  (+ logo)
          . bits 6 and 7 determine the title screen :
              0x00 : unknown - scrambled GFX *probably air raid, haze
              0x40 : unknown - scrambled GFX (alternate entry) *probably air raid, haze
              0x80 : "Cross Shooter"
              0xc0 : "Cross Shooter" (same as 0x80)


  - Addresses :

      * 0xe222 : contents of DSW1 (0xc204)
      * 0xe223 : contents of DSW2 (0xc203)
      * 0xe228 : difficulty (from DSW2)
      * 0xe229 : bonus life (from DSW2 - table at 0x6264)
      * 0xe22b : lives      (from DSW2 - table at 0x7546)
      * 0xe234 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , total credits
          . if 2 coin slots, credits for player 1
      * 0xe235 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , always 0x00 !
          . if 2 coin slots, credits for player 2
      * 0xe237 : lives for player 1
      * 0xe238 : lives for player 2

*/



/*

    Custom SIMM module placement

    The Air Raid PCB has 3 custom modules shown
    in the rough diagram of the PCB below.

    The modules do not appear to be 100%
    identical based on external shapes visible.

    The data they contain is as follows
    MODULE1
    Background tilemap graphic data
    Background tilemap layout data

    MODULE2
    Foreground tilemap graphic data
    Foreground tilemap layout data

    MODULE3
    Sprite graphic data

    it's also possible each module contains a
    0x100 colour look up table for that layer.


   |-------------------------------------------------|
   |    A   B   C   D   E   F   G   H   I   J   K    |
 20|            ##MOD1#######                        |
   | #                            Z80                |
   | M                                              -|
   | O                                              |
   | D                                              |=J
   | 2                                              |=A
   | #                                              |=M
 11|                                                |=M
   |                                                |=A
   |                                                |
   |                                                -|
   |                                  Z80            |
   |                                                 |
   |        ##MOD3#######                            |
   |-------------------------------------------------|

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "video/airraid_dev.h"

class airraid_state : public driver_device
{
public:
	airraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_mainram(*this, "mainram"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_airraid_video(*this,"airraid_vid")
		{ }


	required_device<cpu_device> m_maincpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	optional_shared_ptr<UINT8> m_mainram;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	required_device<airraid_video_device> m_airraid_video;

	DECLARE_READ8_MEMBER(cshooter_coin_r);
	DECLARE_WRITE8_MEMBER(cshooter_c500_w);
	DECLARE_WRITE8_MEMBER(cshooter_c700_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(seibu_sound_comms_r);
	DECLARE_WRITE8_MEMBER(seibu_sound_comms_w);
	DECLARE_DRIVER_INIT(cshootere);
	DECLARE_DRIVER_INIT(cshooter);
	DECLARE_MACHINE_RESET(cshooter);
	TIMER_DEVICE_CALLBACK_MEMBER(cshooter_scanline);
};



/* main cpu */

TIMER_DEVICE_CALLBACK_MEMBER(airraid_state::cshooter_scanline)
{
	int scanline = param;

	if(scanline == 240) // updates scroll resgiters
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* RST 10h */

	if(scanline == 250) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* RST 08h */
}


MACHINE_RESET_MEMBER(airraid_state,cshooter)
{
}

WRITE8_MEMBER(airraid_state::cshooter_c500_w)
{
}

WRITE8_MEMBER(airraid_state::cshooter_c700_w)
{
}

WRITE8_MEMBER(airraid_state::bank_w)
{
	// format of this address is TTBB tbfs

	// TT = bg tilemap upper scroll bits (probably bg tilemap bank select?)
	// BB = prg ROM bank select
	// t = text layer enable
	// b = bg layer disable
	// f = fg layer disable
	// s = sprite layer enable

	membank("bank1")->set_entry((data>>4)&3);

	m_airraid_video->layer_enable_w(data & 0xcf);

}


READ8_MEMBER(airraid_state::seibu_sound_comms_r)
{
	return m_seibu_sound->main_word_r(space,offset,0x00ff);
}

WRITE8_MEMBER(airraid_state::seibu_sound_comms_w)
{
	m_seibu_sound->main_word_w(space,offset,data,0x00ff);
}




static ADDRESS_MAP_START( airraid_map, AS_PROGRAM, 8, airraid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1") AM_WRITENOP // rld result write-back
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("IN1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("IN2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW2")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW1")
	AM_RANGE(0xc500, 0xc500) AM_WRITE(cshooter_c500_w)
//  AM_RANGE(0xc600, 0xc600) AM_WRITE(cshooter_c600_w)            // see notes
	AM_RANGE(0xc700, 0xc700) AM_WRITE(cshooter_c700_w)
//  AM_RANGE(0xc801, 0xc801) AM_WRITE(cshooter_c801_w)            // see notes
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_DEVWRITE("airraid_vid", airraid_video_device, txram_w) AM_SHARE("txram")
	AM_RANGE(0xd800, 0xd8ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xda00, 0xdaff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xdc00, 0xdc0f) AM_RAM_DEVWRITE("airraid_vid", airraid_video_device,  vregs_w) AM_SHARE("vregs")
//  AM_RANGE(0xdc10, 0xdc10) AM_RAM
	AM_RANGE(0xdc11, 0xdc11) AM_WRITE(bank_w)
//  AM_RANGE(0xdc19, 0xdc19) AM_RAM
//  AM_RANGE(0xdc1e, 0xdc1e) AM_RAM
//  AM_RANGE(0xdc1f, 0xdc1f) AM_RAM

	AM_RANGE(0xde00, 0xde0f) AM_READWRITE(seibu_sound_comms_r,seibu_sound_comms_w)
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("sprite_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, airraid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END


static INPUT_PORTS_START( airraid )
	PORT_START("IN0")   /* IN0  (0xc200) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* IN1  (0xc201) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* START    (0xc202) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  /* DSW2 (0xc203) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2k 10k 20k" )
	PORT_DIPSETTING(    0x08, "5k 20k 40k" )
	PORT_DIPSETTING(    0x04, "6k 30k 60k" )
	PORT_DIPSETTING(    0x00, "7k 40k 80k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW1")  /* DSW1 (0xc204) */
	PORT_DIPNAME( 0x01, 0x01, "Coin Slots" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC(  0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("COIN")  /* COIN (0xc205) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static MACHINE_CONFIG_START( airraid, airraid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_12MHz/2)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(airraid_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", airraid_state, cshooter_scanline, "airraid_vid:screen", 0, 1)

	SEIBU2_AIRRAID_SOUND_SYSTEM_CPU(XTAL_14_31818MHz/4)      /* verified on pcb */
	SEIBU_SOUND_SYSTEM_ENCRYPTED_LOW()

	MCFG_QUANTUM_PERFECT_CPU("maincpu")	

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_AIRRAID_VIDEO_ADD("airraid_vid")

	/* sound hardware */
	SEIBU_AIRRAID_SOUND_SYSTEM_YM2151_INTERFACE(XTAL_14_31818MHz/4)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( airraid_crypt, airraid )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END





/*

Air Raid (Seibu 1987)
S-0087-011A-0

            82S129        TMM2015      Z80B  2.19J
SEI0020BU                 TMM2015            1.18J
SEI0020BU         63S281                    TMM2063
SEI0020BU         TMM2015
63S281
SEI0050BU                 3.13F

SEI0040BU                                   TMM2015          on
                                     4.7F    YM2151          x x
TMM2015                TMM2015       5.6F    Z80         sw2  x xxxxx
TMM2015               TMM2015                                  x
SEI0030BU          SEI0060BU                             sw1 xx xxxxx
                                   SEI80BU
                                   SEI0100BU(YM3931) YM3012

*/

ROM_START( airraid )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main CPU
	ROM_LOAD( "1.16j",   0x00000, 0x08000, CRC(7ac2cedf) SHA1(272831f51a2731e067b5aec6dba6bddd3c5350c9) )

	ROM_REGION( 0x10000, "maindata", 0 ) // cpu data
	ROM_LOAD( "2.19j",   0x00000, 0x10000, CRC(842ae6c2) SHA1(0468445e4ab6f42bac786f9a258df3972fd1fde9) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) // Sub/Sound CPU
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) )
	ROM_LOAD( "4.7f",    0x10000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) )

	ROM_REGION( 0x0200, "proms", 0 ) // this PCB type has different proms when compared to the cshootert hardware PCB where they were dumped
	ROM_LOAD( "pr.c19",  0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "6308.a13",  0x0000, 0x0100, NO_DUMP )

	ROM_REGION( 0x02000, "airraid_vid:tx_gfx", 0 ) // TX Layer
	ROM_LOAD( "3.13e",   0x00000, 0x02000, CRC(672ec0e8) SHA1(a11cd90d6494251ceee3bc7c72f4e7b1580b77e2) )

	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 ) // taken from cshootert, not verified for this PCB
	ROM_LOAD( "63s281.d16", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut

	/* ### MODULE 1 ### Background generation / graphics  */
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD16_BYTE( "bg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD16_BYTE( "bg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "bg_clut",   0x000, 0x100, NO_DUMP )

	/* ### MODULE 2 ### Foreground generation / graphics  */
	ROM_REGION( 0x40000, "airraid_vid:fg_map", 0 )
	ROM_LOAD16_BYTE( "fg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD16_BYTE( "fg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "fg_clut",   0x000, 0x100, NO_DUMP )

	/* ### MODULE 3 ### Sprite graphics  */
	ROM_REGION( 0x40000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD16_BYTE( "sprite_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "sprite_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "spr_clut",   0x000, 0x100, NO_DUMP )
ROM_END



/*

Cross Shooter
(C) J K H Corp  (Seibu?)

Seibu Hardware
PCB is coloured black

PCB No. S-0087-011A-0
CPU: SHARP LH0080B (Z80B)
SND: YM2151, Z80A, SEI80BU 611 787, YM3012, SEI0100BU YM3931
RAM: TMM2015 x 7, TMM2063 x 1
DIPs: 2 x 8 position
CMOS Gate Arrays: SEI0020BU TC17G008AN-0015 (x 3), SEI10040BU TC15G008AP-0048,
                  SEI0030BU TC17G005AN-0026, SEI0060BU TC17G008AN-0024
OTHER: SEI0050BU M  6 4 0 00
XTAL: 14.318 MHz (near SEI80BU), xx.000 MHz (cant read speed, near SEI0040BU)

There are 3 BIG custom black packs on the PCB.

ROMS:
Note, all ROMs have official sticker, "(C) SEIBU KAIHATSU INC." and a number.

1.k19  TMM27256      \
2.k20  TMM27512      / Program
3.f11  TMM2764         Gfx?
4.g8   TMM24256 Mask   Sound (Samples?)
5.g6   TMM2764         Sound program


*/

ROM_START( cshooter )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main CPU
	ROM_LOAD( "1.k19",   0x00000, 0x08000, CRC(71418952) SHA1(9745ca006576381c9e9595d8e42ab276bab80a41) )

	ROM_REGION( 0x10000, "maindata", 0 ) // cpu data
	ROM_LOAD( "2.k20",   0x00000, 0x10000, CRC(5812fe72) SHA1(3b28bff6b62a411d2195bb228952db62ad32ef3d) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) // Sub/Sound CPU
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) ) // 5.g6
	ROM_LOAD( "4.7f",    0x10000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) ) // 4.g8

	ROM_REGION( 0x0200, "proms", 0 ) // this PCB type has different proms when compared to the cshootert hardware PCB where they were dumped
	ROM_LOAD( "pr.c19",  0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "6308.a13",  0x0000, 0x0100, NO_DUMP )


	ROM_REGION( 0x02000, "airraid_vid:tx_gfx",  0 ) // TX Layer
	ROM_LOAD( "3.f11",   0x00000, 0x02000, CRC(67b50a47) SHA1(b1f4aefc9437edbeefba5371149cc08c0b55c741) )

	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 ) // taken from cshootert, not verified for this PCB
	ROM_LOAD( "63s281.d16", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut

	/* ### MODULE 1 ### Background generation / graphics  */
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD16_BYTE( "bg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD16_BYTE( "bg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "bg_clut",   0x000, 0x100, NO_DUMP )

	/* ### MODULE 2 ### Foreground generation / graphics  */
	ROM_REGION( 0x40000, "airraid_vid:fg_map", 0 )
	ROM_LOAD16_BYTE( "fg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD16_BYTE( "fg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "fg_clut",   0x000, 0x100, NO_DUMP )

	/* ### MODULE 3 ### Sprite graphics  */
	ROM_REGION( 0x40000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD16_BYTE( "sprite_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "sprite_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "spr_clut",   0x000, 0x100, NO_DUMP )
ROM_END



DRIVER_INIT_MEMBER(airraid_state, cshooter)
{
	membank("bank1")->configure_entries(0, 4, memregion("maindata")->base(), 0x4000);
}

DRIVER_INIT_MEMBER(airraid_state,cshootere)
{
	UINT8 *rom = memregion("maincpu")->base();

	for (int A = 0x0000;A < 0x8000;A++)
	{
		/* decode the opcodes */
		m_decrypted_opcodes[A] = rom[A];

		if (BIT(A,5) && !BIT(A,3))
			m_decrypted_opcodes[A] ^= 0x40;

		if (BIT(A,10) && !BIT(A,9) && BIT(A,3))
			m_decrypted_opcodes[A] ^= 0x20;

		if ((BIT(A,10) ^ BIT(A,9)) && BIT(A,1))
			m_decrypted_opcodes[A] ^= 0x02;

		if (BIT(A,9) || !BIT(A,5) || BIT(A,3))
			m_decrypted_opcodes[A] = BITSWAP8(m_decrypted_opcodes[A],7,6,1,4,3,2,5,0);

		/* decode the data */
		if (BIT(A,5))
			rom[A] ^= 0x40;

		if (BIT(A,9) || !BIT(A,5))
			rom[A] = BITSWAP8(rom[A],7,6,1,4,3,2,5,0);
	}

	DRIVER_INIT_CALL(cshooter);

}

// There's also an undumped International Games version
GAME( 1987, cshooter,  airraid,      airraid_crypt, airraid, airraid_state, cshootere, ROT270, "Seibu Kaihatsu (J.K.H. license)",           "Cross Shooter (Single PCB)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, airraid,   0,         airraid_crypt,  airraid,  airraid_state, cshootere, ROT270, "Seibu Kaihatsu",                  "Air Raid (Single PCB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )

