// license:BSD-3-Clause
// copyright-holders: Luca Elia, Paul Priest

/***************************************************************************

                          -= Fuuki 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU   :   M68000
Sound Chips :   YM2203  +  YM3812  +  M6295
Video Chips :   FI-002K (208pin PQFP, GA2)
                FI-003K (208pin PQFP, GA3)
Other       :   Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)


---------------------------------------------------------------------------
Year + Game
---------------------------------------------------------------------------
95  Susume! Mile Smile / Go Go! Mile Smile
96  Gyakuten!! Puzzle Bancho
---------------------------------------------------------------------------

Do NOT trust the Service Mode for dipswitch settings for Go Go! Mile Smile:
  Service Mode shows Coin A as SW2:3-5 & Coin B as SW2:6-8, but the game ignores the
  setting of Coin B and only uses the settings for Coin A, except for Coin B "Free Play"
  The game says Press 1 or 2, and will start the game, but jumps right to the Game Over
  and "Continue" countdown.
The Service Mode is WAY off on effects of dipswitches for the earlier set!!! It reports
  the effects of MAME's SW1:3-8 have been moved to SW1:2-7 and Demo Sound has moved to
  SW2:7. What MAME shows as settings are according to actual game effect and reflect what
  the manual states.


To Do:

- Raster effects (level 5 interrupt is used for that). In pbancho
  they involve changing the *vertical* scroll value of the layers
  each scanline (when you are about to die, in the solo game).
  In gogomile they weave the water backgrounds and do some
  parallax scrolling on later levels. *partly done, could do with
  some tweaking

- The scroll values are generally wrong when flip screen is on and rasters are often incorrect

***************************************************************************/

#include "emu.h"

#include "fuukispr.h"
#include "fuukitmap.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class fuuki16_state : public driver_device
{
public:
	fuuki16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_fuukispr(*this, "fuukispr")
		, m_fuukitmap(*this, "fuukitmap")
		, m_soundlatch(*this, "soundlatch")
		, m_spriteram(*this, "spriteram")
		, m_soundbank(*this, "soundbank")
	{ }

	void fuuki16(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukispr_device> m_fuukispr;
	required_device<fuukitmap_device> m_fuukitmap;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<u16> m_spriteram;

	required_memory_bank m_soundbank;

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void sound_command_w(u8 data);
	void sound_rombank_w(u8 data);
	void oki_banking_w(u8 data);

	void colpri_cb(u32 &colour, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/***************************************************************************

    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layer 2 (double-buffered) ]

    Tile Size:              16 x 16 x 4     16 x 16 x 8     8 x 8 x 4
    Layer Size (tiles):     64 x 32         64 x 32         64 x 32

    [ 1024? Zooming Sprites ]

    Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
    tiles both horizontally and vertically.
    There is zooming (from full size to half size) and 4 levels of
    priority (wrt layers)

    * Note: the game does hardware assisted raster effects *

***************************************************************************/

/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void fuuki16_state::video_start()
{
	m_fuukitmap->set_transparent_pen(0, 0x0f);    // 4 bits
	m_fuukitmap->set_transparent_pen(1, 0xff);    // 8 bits
	m_fuukitmap->set_transparent_pen(2, 0x0f);    // 4 bits

	m_fuukitmap->gfx(1)->set_granularity(16); // 256 colour tiles with palette selectable on 16 colour boundaries
}


void fuuki16_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	const u8 priority = (colour >> 6) & 3;
	switch (priority)
	{
		case 3:  pri_mask = 0xf0 | 0xcc | 0xaa;  break;  // behind all layers
		case 2:  pri_mask = 0xf0 | 0xcc;         break;  // behind fg + middle layer
		case 1:  pri_mask = 0xf0;                break;  // behind fg layer
		case 0:
		default: pri_mask = 0;                       // above all
	}
	colour &= 0x3f;
}


u32 fuuki16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fuukitmap->prepare();
	flip_screen_set(m_fuukitmap->flip_screen());

	/* The backmost tilemap decides the background color(s) but sprites can
	   go below the opaque pixels of that tilemap. We thus need to mark the
	   transparent pixels of this layer with a different priority value */
//  m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_back(), TILEMAP_DRAW_OPAQUE, 0);

	/* Actually, bg colour is simply the last pen i.e. 0x1fff -pjp */
	bitmap.fill((0x800 * 4) - 1, cliprect);
	screen.priority().fill(0, cliprect);

	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_back(),   0, 1);
	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_middle(), 0, 2);
	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_front(),  0, 4);

	m_fuukispr->draw_sprites(screen, bitmap, cliprect, flip_screen(), m_spriteram, m_spriteram.bytes() / 2);

	return 0;
}


//-------------------------------------------------
//  memory - main CPU
//-------------------------------------------------

void fuuki16_state::sound_command_w(u8 data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
//      m_maincpu->spin_until_time(attotime::from_usec(50));   // Allow the other CPU to reply
	machine().scheduler().perfect_quantum(attotime::from_usec(50)); // Fixes glitching in rasters
}

void fuuki16_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram();
	map(0x500000, 0x507fff).m(m_fuukitmap, FUNC(fuukitmap_device::vram_map));
	map(0x600000, 0x601fff).mirror(0x008000).ram().share(m_spriteram);   // mirrored?
	map(0x700000, 0x703fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x800001).portr("SYSTEM");
	map(0x810000, 0x810001).portr("P1_P2");
	map(0x880000, 0x880001).portr("DSW");
	map(0x8a0001, 0x8a0001).w(FUNC(fuuki16_state::sound_command_w));
	map(0x8c0000, 0x8effff).m(m_fuukitmap, FUNC(fuukitmap_device::vregs_map));
}


//-------------------------------------------------
//  memory - sound CPU
//-------------------------------------------------

void fuuki16_state::sound_rombank_w(u8 data)
{
	if (data <= 2)
		m_soundbank->set_entry(data);
	else
		logerror("CPU #1 - PC %04X: unknown bank bits: %02X\n", m_audiocpu->pc(), data);
}

void fuuki16_state::oki_banking_w(u8 data)
{
	/*
	    data & 0x06 is always equals to data & 0x60
	    data & 0x10 is always set
	*/

	m_oki->set_rom_bank((data & 6) >> 1);
}

void fuuki16_state::sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();         // ROM
	map(0x6000, 0x7fff).ram();         // RAM
	map(0x8000, 0xffff).bankr(m_soundbank);    // Banked ROM
}

void fuuki16_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(fuuki16_state::sound_rombank_w));
	map(0x11, 0x11).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw(); // To Main CPU ?
	map(0x20, 0x20).w(FUNC(fuuki16_state::oki_banking_w));
	map(0x30, 0x30).nopw();    // ? In the NMI routine
	map(0x40, 0x41).w("ym1", FUNC(ym2203_device::write));
	map(0x50, 0x51).rw("ym2", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x60, 0x60).r(m_oki, FUNC(okim6295_device::read));
	map(0x61, 0x61).w(m_oki, FUNC(okim6295_device::write));
}


//-------------------------------------------------
//  input ports
//-------------------------------------------------

static INPUT_PORTS_START( gogomile )
	PORT_START("SYSTEM")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")     // $810000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )              // There's code that uses
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )              // these unknown bits
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")       // $880000.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Demo Music" )        PORT_DIPLOCATION("SW1:2") // Game play sounds still play, only effects Music
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( Chinese ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Japanese ) ) // Only setting to give a "For use only in...." Copyright Notice
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0040, "5" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )    // Manual states this dip is "Unused"
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )
/*
    PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
    PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
*/
INPUT_PORTS_END


static INPUT_PORTS_START( gogomileo )   // The earlier version has different coinage settings.
	PORT_INCLUDE( gogomile )

	PORT_MODIFY("DSW")      // $880000.w
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
/*
    PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
    PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(      0x6000, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
*/
INPUT_PORTS_END

static INPUT_PORTS_START( pbancho )
	PORT_INCLUDE( gogomile )

	PORT_MODIFY("SYSTEM")   // $800000.w
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("DSW")      // $880000.w
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easiest ) )  // 1
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy )    )  // 2
	PORT_DIPSETTING(      0x001c, DEF_STR( Normal )  )  // 3
	PORT_DIPSETTING(      0x0000, "Normal, duplicate" ) // 3
	PORT_DIPSETTING(      0x000c, "Normal, duplicate" ) // 3
	PORT_DIPSETTING(      0x0014, "Normal, duplicate" ) // 3
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard )    )  // 4
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )  // 5
	PORT_DIPNAME( 0x0060, 0x0060, "Lives (Vs Mode)" )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, "1" ) // 1 1
	PORT_DIPSETTING(      0x0060, "2" ) // 2 3
	PORT_DIPSETTING(      0x0020, "2, duplicate" ) // 2 3
	PORT_DIPSETTING(      0x0040, "3" ) // 3 5
	PORT_DIPNAME( 0x0080, 0x0080, "? Senin Mode ?" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Allow Versus Mode" ) PORT_DIPLOCATION("SW2:2") // "unused" in the manual?
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0x6000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END



//-------------------------------------------------
//  graphics layouts
//-------------------------------------------------

// 16x16x8
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP4(0,1), STEP4(16,1) },
	{ STEP4(0,4), STEP4(16*2,4), STEP4(16*4,4), STEP4(16*6,4) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( gfx_fuuki16 )
	GFXDECODE_ENTRY( "tiles_l0", 0, gfx_16x16x4_packed_msb, 0x400*0, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "tiles_l1", 0, layout_16x16x8,         0x400*1, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "tiles_l2", 0, gfx_8x8x4_packed_msb,   0x400*3, 0x40 ) // [2] Layer 2
GFXDECODE_END


//-------------------------------------------------
//  driver functions
//-------------------------------------------------

/*
    - Interrupts (pbancho) -

    Lev 1:  Sets bit 5 of $400010. Prints "credit .." with sprites.
    Lev 2:  Sets bit 7 of $400010. Clears $8c0012.
            It seems unused by the game.
    Lev 3:  VBlank.
    Lev 5:  Programmable to happen on a raster line. Used to do raster
            effects when you die and its clearing the blocks
            also used for water effects and titlescreen linescroll on gogomile
*/

void fuuki16_state::machine_start()
{
	u8 *rom = memregion("audiocpu")->base();

	m_soundbank->configure_entries(0, 3, &rom[0x8000], 0x8000);
}


void fuuki16_state::fuuki16(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(32'000'000) / 2);    // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &fuuki16_state::main_map);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);      // 6 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &fuuki16_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fuuki16_state::sound_io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 256);
	m_screen->set_visarea(0, 320-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(fuuki16_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fuuki16);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xRGB_555, 0x4000 / 2);

	FUUKI_SPRITE(config, m_fuukispr, 0);
	m_fuukispr->set_palette(m_palette);
	m_fuukispr->set_color_base(0x400*2);
	m_fuukispr->set_color_num(0x40);
	m_fuukispr->set_colpri_callback(FUNC(fuuki16_state::colpri_cb));

	FUUKI_TILEMAP(config, m_fuukitmap, 0, m_palette, gfx_fuuki16);
	m_fuukitmap->set_screen(m_screen);
	m_fuukitmap->level_1_irq_callback().set_inputline(m_maincpu, 1, HOLD_LINE);
	m_fuukitmap->vblank_irq_callback().set_inputline(m_maincpu, 3, HOLD_LINE);
	m_fuukitmap->raster_irq_callback().set_inputline(m_maincpu, 5, HOLD_LINE);
	m_fuukitmap->set_xoffs(0x1f3, 0x103);
	m_fuukitmap->set_yoffs(0x3f6, 0x2a7);
	m_fuukitmap->set_layer2_xoffs(0x10);
	//m_fuukitmap->set_layer2_yoffs(0x02);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(28'640'000) / 8)); // 3.58 MHz
	ym1.add_route(ALL_OUTPUTS, "mono", 0.15);

	ym3812_device &ym2(YM3812(config, "ym2", XTAL(28'640'000) / 8)); // 3.58 MHz
	ym2.irq_handler().set_inputline(m_audiocpu, 0);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, m_oki, XTAL(32'000'000) / 32, okim6295_device::PIN7_HIGH); // 1 MHz
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.85);
}


//-------------------------------------------------
//  ROM loading
//-------------------------------------------------

/***************************************************************************

                    Go! Go! Mile Smile / Susume! Mile Smile

(c)1995 Fuuki
FG-1C AI AM-2 (same board as Gyakuten Puzzle Banchou)

CPU  : TMP68HC000P-16
Sound: Z80 YM2203C YM3812 M6295 Y3014Bx2
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
fp2.rom2 - Main programs (27c4000)
fp1.rom1 /

lh538n1d.rom25 - Samples (Sharp mask, read as 27c8001)
fs1.rom24 - Sound program (27c010)

lh5370h8.rom11 - Sprites? (Sharp Mask, read as 27c160)
lh5370ha.rom12 |
lh5370h7.rom15 |
lh5370h9.rom16 /

lh537k2r.rom20 - Tiles? (Sharp Mask, read as 27c160)
lh5370hb.rom19 |
lh5370h6.rom3  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)


Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)
5 GALs (GAL16V8B, not dumped)

Measured clocks:

    68k - 16mhz, OSC1 / 2
    Z80 - 6mhz, Xtal1 / 2
  M6295 - 1mhz,  OSC1 / 32
YM2203C - 3.58mhz, OSC2 / 8
 YM3812 - 3.58mhz, OSC2 / 8

***************************************************************************/

ROM_START( gogomile )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "fp2n.rom2", 0x000000, 0x080000, CRC(e73583a0) SHA1(05c6ee5cb2c151b32c462e8b920f9a57fb6cce5b) )
	ROM_LOAD16_BYTE( "fp1n.rom1", 0x000001, 0x080000, CRC(7b110824) SHA1(980e326d3b9e113ed522be3076663a249da4e739) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "fs1.rom24", 0x00000, 0x20000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )

	ROM_REGION( 0x200000, "fuukispr", 0 )   // 16x16x4 sprites
	ROM_LOAD16_WORD_SWAP( "lh537k2r.rom20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "tiles_l0", 0 )   // 16x16x4 tiles
	ROM_LOAD16_WORD_SWAP( "lh5370h6.rom3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )  // x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "tiles_l1", 0 )   // 16x16x8 tiles
	ROM_LOAD32_WORD_SWAP( "lh5370h7.rom15", 0x000000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD32_WORD_SWAP( "lh5370h8.rom11", 0x000002, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD32_WORD_SWAP( "lh5370h9.rom16", 0x400000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )
	ROM_LOAD32_WORD_SWAP( "lh5370ha.rom12", 0x400002, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )

	ROM_REGION( 0x200000, "tiles_l2", 0 )   // 8x8x4 tiles
	ROM_LOAD16_WORD_SWAP( "lh5370hb.rom19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) ) // FIRST AND SECOND HALF IDENTICAL

	// 0x40000 * 4: sounds+speech (Japanese), sounds+speech (English)
	ROM_REGION( 0x100000, "oki", 0 )    // samples
	ROM_LOAD( "lh538n1d.rom25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) ) // 0x40000 * 4
ROM_END

ROM_START( gogomileo )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "fp2.rom2", 0x000000, 0x080000, CRC(28fd3e4e) SHA1(3303e5759c0781035c74354587e1916719695754) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "fp1.rom1", 0x000001, 0x080000, CRC(35a5fc45) SHA1(307207791cee7f40e88feffc5805ac25008a8566) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "fs1.rom24", 0x00000, 0x20000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )

	ROM_REGION( 0x200000, "fuukispr", 0 )   // 16x16x4 sprites
	ROM_LOAD16_WORD_SWAP( "lh537k2r.rom20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "tiles_l0", 0 )   // 16x16x4 tiles
	ROM_LOAD16_WORD_SWAP( "lh5370h6.rom3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )  // x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "tiles_l1", 0 )   // 16x16x8 tiles
	ROM_LOAD32_WORD_SWAP( "lh5370h7.rom15", 0x000000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD32_WORD_SWAP( "lh5370h8.rom11", 0x000002, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD32_WORD_SWAP( "lh5370h9.rom16", 0x400000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )
	ROM_LOAD32_WORD_SWAP( "lh5370ha.rom12", 0x400002, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )

	ROM_REGION( 0x200000, "tiles_l2", 0 )   // 8x8x4 tiles
	ROM_LOAD16_WORD_SWAP( "lh5370hb.rom19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) ) // FIRST AND SECOND HALF IDENTICAL

	// 0x40000 * 4: sounds+speech (Japanese), sounds+speech (English)
	ROM_REGION( 0x100000, "oki", 0 )    // samples
	ROM_LOAD( "lh538n1d.rom25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) ) // 0x40000 * 4
ROM_END



/***************************************************************************

                            Gyakuten!! Puzzle Bancho

(c)1996 Fuuki
FG-1C AI AM-2

CPU  : TMP68HC000P-16
Sound: Z80 YM2203 YM3812 M6295
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
no1.rom2 - Main program (even)(27c4000)
no2.rom1 - Main program (odd) (27c4000)

no3.rom25 - Samples (27c2001)
no4.rom24 - Sound program (27c010)

61.rom11 - Graphics (Mask, read as 27c160)
59.rom15 |
58.rom20 |
60.rom3  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)

Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)

***************************************************************************/

ROM_START( pbancho ) // ROMs NO1 & NO2 had an addition block dot on labels
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "no1..rom2", 0x000000, 0x080000, CRC(e607eca6) SHA1(be9156d2a336a04fb9ff147b0d0287d8ff2ccfc5) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "no2..rom1", 0x000001, 0x080000, CRC(ee15b423) SHA1(6da7ba9dd785dfcf919c030e126daf8d6750d072) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "no4.rom23", 0x00000, 0x20000, CRC(dfbfdb81) SHA1(84b0cbe843a9bbae43975afdbd029a9b76fd488b) )

	ROM_REGION( 0x200000, "fuukispr", 0 )   // 16x16x4 sprites
	ROM_LOAD16_WORD_SWAP( "58.rom20", 0x000000, 0x200000, CRC(4dad0a2e) SHA1(a4f70557503110a5457b9096a79a5f249095fa55) )

	ROM_REGION( 0x200000, "tiles_l0", 0 )   // 16x16x4 tiles
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )

	ROM_REGION( 0x400000, "tiles_l1", 0 )   // 16x16x8 tiles
	ROM_LOAD32_WORD_SWAP( "59.rom15", 0x000000, 0x200000, CRC(b83dcb70) SHA1(b0b9df451535d85612fa095b4f694cf2e7930bca) )
	ROM_LOAD32_WORD_SWAP( "61.rom11", 0x000002, 0x200000, CRC(7f1213b9) SHA1(f8d6432b270c4d0954602e430ddd26841eb05656) )

	ROM_REGION( 0x200000, "tiles_l2", 0 )   // 8x8x4 tiles
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )    // ?maybe?

	ROM_REGION( 0x040000, "oki", 0 )    // samples
	ROM_LOAD( "n03.rom25", 0x000000, 0x040000, CRC(a7bfb5ea) SHA1(61937eae4f8855bc09c494aff52d76d41dc3b76a) )
ROM_END

ROM_START( pbanchoa )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "no1.rom2", 0x000000, 0x080000, CRC(1b4fd178) SHA1(02cf3d2554b29cd253470d68ea959738f3b98dbe) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "no2.rom1", 0x000001, 0x080000, CRC(9cf510a5) SHA1(08e79b5bbd1c011c32f82dd15fba42d7898861be) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "no4.rom23", 0x00000, 0x20000, CRC(dfbfdb81) SHA1(84b0cbe843a9bbae43975afdbd029a9b76fd488b) )

	ROM_REGION( 0x200000, "fuukispr", 0 )   // 16x16x4 sprites
	ROM_LOAD16_WORD_SWAP( "58.rom20", 0x000000, 0x200000, CRC(4dad0a2e) SHA1(a4f70557503110a5457b9096a79a5f249095fa55) )

	ROM_REGION( 0x200000, "tiles_l0", 0 )   // 16x16x4 tiles
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )

	ROM_REGION( 0x400000, "tiles_l1", 0 )   // 16x16x8 tiles
	ROM_LOAD32_WORD_SWAP( "59.rom15", 0x000000, 0x200000, CRC(b83dcb70) SHA1(b0b9df451535d85612fa095b4f694cf2e7930bca) )
	ROM_LOAD32_WORD_SWAP( "61.rom11", 0x000002, 0x200000, CRC(7f1213b9) SHA1(f8d6432b270c4d0954602e430ddd26841eb05656) )

	ROM_REGION( 0x200000, "tiles_l2", 0 )   // 8x8x4 tiles
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )    // ?maybe?

	ROM_REGION( 0x040000, "oki", 0 )    // samples
	ROM_LOAD( "n03.rom25", 0x000000, 0x040000, CRC(a7bfb5ea) SHA1(61937eae4f8855bc09c494aff52d76d41dc3b76a) )
ROM_END

} // anonymous namespace


//-------------------------------------------------
//  game drivers
//-------------------------------------------------

GAME( 1995, gogomile,  0,        fuuki16, gogomile,  fuuki16_state, empty_init, ROT0, "Fuuki", "Susume! Mile Smile / Go Go! Mile Smile (newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, gogomileo, gogomile, fuuki16, gogomileo, fuuki16_state, empty_init, ROT0, "Fuuki", "Susume! Mile Smile / Go Go! Mile Smile (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, pbancho,   0,        fuuki16, pbancho,   fuuki16_state, empty_init, ROT0, "Fuuki", "Gyakuten!! Puzzle Bancho (Japan, set 1)",        MACHINE_SUPPORTS_SAVE ) // program ROMs had extra black dot on labels
GAME( 1996, pbanchoa,  pbancho,  fuuki16, pbancho,   fuuki16_state, empty_init, ROT0, "Fuuki", "Gyakuten!! Puzzle Bancho (Japan, set 2)",        MACHINE_SUPPORTS_SAVE )
