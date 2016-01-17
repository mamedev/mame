// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Angelo Salese, hap
/* Cross Shooter (c) 1987 Seibu

 TS 01.05.2006:

 - added sprites, bgmap reading and few fixes here and there
   airraid and cshootere are a bit "playable" ;) without gfx


Haze's notes :

  - interrupts are probably wrong .. it ends up writing to rom etc.
  - how do the sprites / bg's work .. these big black unknown things on the pcb
    also sound a bit disturbing, what are they?
  - i can't decode the other gfx? rom
  - there don't seem to be any sprites / bg's in ram, interrupts?
  - palette? format isn't understood
  - the other sets ('cshootere' and 'airraid') need decrypting ..
    is the main one protected ? theres a 68705 on it


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


  - Interrupts notes :

      * I think that they aren't handled correctly : after a few frames,
        the number of lives are reset to 0, causing a "GAME OVER" 8(
            * - or is this protection from the 68705, haze


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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"


class cshooter_state : public driver_device
{
public:
	cshooter_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_txram(*this, "txram"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_generic_paletteram2_8(*this, "paletteram2"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_shared_ptr<UINT8> m_txram;
	optional_shared_ptr<UINT8> m_mainram;
	optional_shared_ptr<UINT8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_generic_paletteram_8;
	required_shared_ptr<UINT8> m_generic_paletteram2_8;
	required_shared_ptr<UINT8> m_decrypted_opcodes;

	tilemap_t *m_txtilemap;
	int m_coin_stat;
	int m_counter;

	DECLARE_WRITE8_MEMBER(cshooter_txram_w);
	DECLARE_READ8_MEMBER(cshooter_coin_r);
	DECLARE_WRITE8_MEMBER(cshooter_c500_w);
	DECLARE_WRITE8_MEMBER(cshooter_c700_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(seibu_sound_comms_r);
	DECLARE_WRITE8_MEMBER(seibu_sound_comms_w);
	DECLARE_DRIVER_INIT(cshootere);
	DECLARE_DRIVER_INIT(cshooter);
	TILE_GET_INFO_MEMBER(get_cstx_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(cshooter);
	DECLARE_MACHINE_RESET(cshooter);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_airraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(cshooter_scanline);
};


PALETTE_INIT_MEMBER(cshooter_state, cshooter)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	// text uses colors 0xc0-0xdf
	for (i = 0; i < 0x40; i++)
		palette.set_pen_indirect(i, (color_prom[i] & 0x1f) | 0xc0);

	// rest is still unknown..
	for (i = 0x40; i < 0x100; i++)
		palette.set_pen_indirect(i, color_prom[i]);
}

TILE_GET_INFO_MEMBER(cshooter_state::get_cstx_tile_info)
{
	int code = (m_txram[tile_index*2]);
	int attr = (m_txram[tile_index*2+1]);
	int color = attr & 0xf;

	SET_TILE_INFO_MEMBER(0, (code << 1) | ((attr & 0x20) >> 5), color, 0);
}

WRITE8_MEMBER(cshooter_state::cshooter_txram_w)
{
	m_txram[offset] = data;
	m_txtilemap->mark_tile_dirty(offset/2);
}

void cshooter_state::video_start()
{
	m_txtilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cshooter_state::get_cstx_tile_info),this),TILEMAP_SCAN_ROWS, 8,8,32,32);
	m_txtilemap->set_transparent_pen(0);
}

void cshooter_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = m_spriteram.bytes() - 4; i >= 0 ; i -= 4)
	{
		if (m_spriteram[i+1]&0x80)
			continue;

		/* BCD debug code, to be removed in the end */
		UINT8 tile_low = (m_spriteram[i]&0x0f);
		UINT8 tile_high = ((m_spriteram[i]&0xf0)>>4);

		tile_low += (tile_low > 0x9) ? 0x37 : 0x30;
		tile_high += (tile_high > 0x9) ? 0x37 : 0x30;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tile_high << 1, m_spriteram[i+1], 0, 0, m_spriteram[i+3],m_spriteram[i+2],0);
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tile_high << 1, m_spriteram[i+1], 0, 0, m_spriteram[i+3]+8,m_spriteram[i+2],0);
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tile_low << 1, m_spriteram[i+1], 0, 0, m_spriteram[i+3]+8,m_spriteram[i+2]+8,0);
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tile_low << 1, m_spriteram[i+1], 0, 0, m_spriteram[i+3],m_spriteram[i+2]+8,0);
	}
}

UINT32 cshooter_state::screen_update_airraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set palette (compared to cshooter, r and g are swapped)
	for (int i = 0; i < 0x100; i++)
	{
		int r = m_generic_paletteram_8[i] & 0xf;
		int g = m_generic_paletteram_8[i] >> 4;
		int b = m_generic_paletteram2_8[i] & 0xf;

		rgb_t color = rgb_t(pal4bit(r), pal4bit(g), pal4bit(b));
		m_palette->set_indirect_color(i, color);
	}

	bitmap.fill(0x80, cliprect); // temp

	draw_sprites(bitmap, cliprect);

	m_txtilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}


/* main cpu */

TIMER_DEVICE_CALLBACK_MEMBER(cshooter_state::cshooter_scanline)
{
	int scanline = param;

//  if(scanline == 240) // presumably a SW trap, not an irq
//      m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* RST 10h */

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* RST 08h */
}


MACHINE_RESET_MEMBER(cshooter_state,cshooter)
{
	m_counter = 0;
}

READ8_MEMBER(cshooter_state::cshooter_coin_r)
{
	/* Even reads must return 0xff - Odd reads must return the contents of input port 5.
	   Code at 0x5061 is executed once during P.O.S.T. where there is one read.
	   Code at 0x50b4 is then executed each frame (not sure) where there are 2 reads. */
	return ( (m_counter++ & 1) ? 0xff : ioport("COIN")->read() );
}

WRITE8_MEMBER(cshooter_state::cshooter_c500_w)
{
}

WRITE8_MEMBER(cshooter_state::cshooter_c700_w)
{
}

WRITE8_MEMBER(cshooter_state::bank_w)
{
	membank("bank1")->set_entry((data>>4)&3);
}


READ8_MEMBER(cshooter_state::seibu_sound_comms_r)
{
	return m_seibu_sound->main_word_r(space,offset,0x00ff);
}

WRITE8_MEMBER(cshooter_state::seibu_sound_comms_w)
{
	m_seibu_sound->main_word_w(space,offset,data,0x00ff);
}

#if 0
static ADDRESS_MAP_START( cshooter_map, AS_PROGRAM, 8, cshooter_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xafff) AM_READ_BANK("bank1") AM_WRITEONLY
	AM_RANGE(0xb000, 0xb0ff) AM_READONLY            // sound related ?
	AM_RANGE(0xc000, 0xc0ff) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xc100, 0xc1ff) AM_RAM AM_SHARE("paletteram2")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("IN0")
	AM_RANGE(0xc201, 0xc201) AM_READ_PORT("IN1")
	AM_RANGE(0xc202, 0xc202) AM_READ_PORT("IN2")
	AM_RANGE(0xc203, 0xc203) AM_READ_PORT("DSW2")
	AM_RANGE(0xc204, 0xc204) AM_READ_PORT("DSW1")
	AM_RANGE(0xc205, 0xc205) AM_READ(cshooter_coin_r)   // hack until I understand
	AM_RANGE(0xc500, 0xc500) AM_WRITE(cshooter_c500_w)
	AM_RANGE(0xc600, 0xc600) AM_WRITENOP            // see notes
	AM_RANGE(0xc700, 0xc700) AM_WRITE(cshooter_c700_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITENOP            // see notes
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(cshooter_txram_w) AM_SHARE("txram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

#endif

static ADDRESS_MAP_START( airraid_map, AS_PROGRAM, 8, cshooter_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1") AM_WRITENOP // rld result write-back
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("IN1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("IN2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW2")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW1")
	AM_RANGE(0xc500, 0xc500) AM_WRITE(cshooter_c500_w)
	AM_RANGE(0xc600, 0xc600) AM_WRITENOP            // see notes
	AM_RANGE(0xc700, 0xc700) AM_WRITE(cshooter_c700_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITENOP            // see notes
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(cshooter_txram_w) AM_SHARE("txram")
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xda00, 0xdaff) AM_RAM AM_SHARE("paletteram2")
	AM_RANGE(0xdc11, 0xdc11) AM_WRITE(bank_w)
	AM_RANGE(0xdc00, 0xdc1f) AM_RAM //video registers
	AM_RANGE(0xde00, 0xde0f) AM_READWRITE(seibu_sound_comms_r,seibu_sound_comms_w)
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, cshooter_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

#if 0
/* Sound CPU */

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cshooter_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc001) AM_WRITENOP // AM_DEVWRITE("ym1", ym2203_device, write) ?
	AM_RANGE(0xc800, 0xc801) AM_WRITENOP // AM_DEVWRITE("ym2", ym2203_device, write) ?
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END
#endif

static INPUT_PORTS_START( cshooter )
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( airraid )
	PORT_INCLUDE( cshooter )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout cshooter_charlayout =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),      /* 512 characters */
	2,          /* 4 bits per pixel */
	{ 0,4 },
	{ 8,9,10,11,0,1,2,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128*1
};


static GFXDECODE_START( cshooter )
	GFXDECODE_ENTRY( "gfx1", 0,     cshooter_charlayout, 0, 16  )
GFXDECODE_END

static MACHINE_CONFIG_START( airraid, cshooter_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_12MHz/2)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(airraid_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", cshooter_state, cshooter_scanline, "screen", 0, 1)

	SEIBU2_AIRRAID_SOUND_SYSTEM_CPU(XTAL_14_31818MHz/4)      /* verified on pcb */
	SEIBU_SOUND_SYSTEM_ENCRYPTED_LOW()

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1-16)
	MCFG_SCREEN_UPDATE_DRIVER(cshooter_state, screen_update_airraid)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cshooter)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x100)
	MCFG_PALETTE_INIT_OWNER(cshooter_state, cshooter)

	/* sound hardware */
	SEIBU_AIRRAID_SOUND_SYSTEM_YM2151_INTERFACE(XTAL_14_31818MHz/4)
MACHINE_CONFIG_END






/*

Cross Shooter
(C) J K H Corp  (Seibu?)

Seibu Hardware
PCB is coloured black and supposed to be proto, but mask roms are present......?

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

ROM_START( cshootere )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Main CPU
	ROM_LOAD( "1.k19",   0x00000, 0x08000, CRC(71418952) SHA1(9745ca006576381c9e9595d8e42ab276bab80a41) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sub/Sound CPU
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) ) // 5.g6
	ROM_LOAD( "4.7f",    0x08000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) ) // 4.g8

	ROM_REGION( 0x02000, "gfx1",  0 ) // TX Layer
	ROM_LOAD( "3.f11",   0x00000, 0x02000, CRC(67b50a47) SHA1(b1f4aefc9437edbeefba5371149cc08c0b55c741) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // tiles
	ROM_LOAD( "graphics1.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "graphics2.bin", 0x10000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "graphics3.bin", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "user1", 0 ) // Tilemaps
	ROM_LOAD( "2.k20",   0x00000, 0x10000, CRC(5812fe72) SHA1(3b28bff6b62a411d2195bb228952db62ad32ef3d) )

	ROM_REGION( 0x320, "proms", 0 ) // taken from parent set
	ROM_LOAD( "63s281.16a", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut
	ROM_LOAD( "82s123.7a",  0x0100, 0x0020, CRC(93e2d292) SHA1(af8edd0cfe85f28ede9604cfaf4516d54e5277c9) ) // sprite color related? (not used)
	ROM_LOAD( "82s129.9s",  0x0120, 0x0100, CRC(cf14ba30) SHA1(3284b6809075756b3c8e07d9705fc7eacb7556f1) ) // timing? (not used)
	ROM_LOAD( "82s129.4e",  0x0220, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) ) // timing? (not used)
ROM_END

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

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sub/Sound CPU
	ROM_LOAD( "5.6f",    0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) )
	ROM_LOAD( "4.7f",    0x08000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) )

	ROM_REGION( 0x02000, "gfx1", 0 ) // TX Layer
	ROM_LOAD( "3.13e",   0x00000, 0x02000, CRC(672ec0e8) SHA1(a11cd90d6494251ceee3bc7c72f4e7b1580b77e2) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // tiles
	ROM_LOAD( "graphics1.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "graphics2.bin", 0x10000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "graphics3.bin", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "user1", 0 ) // bg maps
	ROM_LOAD( "2.19j",   0x00000, 0x10000, CRC(842ae6c2) SHA1(0468445e4ab6f42bac786f9a258df3972fd1fde9) )

	ROM_REGION( 0x320, "proms", 0 ) // taken from parent set
	ROM_LOAD( "63s281.16a", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) ) // clut
	ROM_LOAD( "82s123.7a",  0x0100, 0x0020, CRC(93e2d292) SHA1(af8edd0cfe85f28ede9604cfaf4516d54e5277c9) ) // sprite color related? (not used)
	ROM_LOAD( "82s129.9s",  0x0120, 0x0100, CRC(cf14ba30) SHA1(3284b6809075756b3c8e07d9705fc7eacb7556f1) ) // timing? (not used)
	ROM_LOAD( "82s129.4e",  0x0220, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) ) // timing? (not used)
ROM_END


#if 0
DRIVER_INIT_MEMBER(cshooter_state,cshooter)
{
	/* temp so it boots */
	UINT8 *rom = memregion("maincpu")->base();

	rom[0xa2] = 0x00;
	rom[0xa3] = 0x00;
	rom[0xa4] = 0x00;
	membank("bank1")->set_base(&memregion("user1")->base()[0]);
}
#endif

DRIVER_INIT_MEMBER(cshooter_state,cshootere)
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

	membank("bank1")->configure_entries(0, 4, memregion("user1")->base(), 0x4000);
}



GAME( 1987, cshootere, cshooter,  airraid,  airraid,  cshooter_state, cshootere, ROT270, "Seibu Kaihatsu (J.K.H. license)", "Cross Shooter (encrypted)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1987, airraid,   cshooter,  airraid,  airraid,  cshooter_state, cshootere, ROT270, "Seibu Kaihatsu", "Air Raid (encrypted)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
// There's also an undumped International Games version
