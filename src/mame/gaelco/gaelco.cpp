// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

    Driver by Manuel Abadia

Games supported:

Year   Game                PCB                       NOTES
------------------------------------------------------------------------
1990   Xor World           ?                         Prototype
1991   Big Karnak          REF 901112-1              Unprotected
1992   Maniac Square       REF 922804/2              Prototype
1992   Squash              REF 922804/2 or 922804/1  Encrypted Video RAM
1992   Thunder Hoop        REF 922804/2 or 922804/1  Encrypted Video RAM
1995   Biomechanical Toy   REF 922804/2 or 922804/1  Unprotected
1995   Last Km             REF 922804/2              Prototype

      Priorities for all games - the games don't make extensive
      enough use of the priority scheme to properly draw any
      conclusions.

-------------------------------------------------------------
      Note about 57.42 'FRAMERATE_922804' screen refresh
      frequency and protection checks.

      In thoop there's a timing loop at 0x49e-4ac.  It's
      counting frames between interrupt-triggers.

      0x49e writes the count to 0xffdb62.

      While fighting the second-stage boss, when the pink
      things fly out,  0x8970 is called.  0x8988 fetches
      from 0xffdb62.  If the value is > 0xdd1 (via 0x898a)
      or < 0xdb1 (via 0x8992), then 0x89ac sets 0xffdc45
      to 5.

      At 60hz the value returned is 0xd29, which causes
      the fail condition to trigger. Values >=57.3 or
      <=57.7 give a result within the required range. The
      failure is not obvious at this point.

      While fighting the third boss, 0xc2e8 is called.
      After passing checks to know exactly when to trigger
      (specifically, after the boss is defeated and the
      power-up animation is finishes), 0xc350 checks if
      0xffdc45 is 5.  If it is, then we reach 0xc368, which
      0xc368 sets 0xffe08e to 0x27.  Again the failure is
      not obvious at this point.

      0xffe08e is checked during player respawn after
      losing a life or continuing at 0x16d00, with an
      explicit compare against 0x27, if this condition is
      met, then the game will intentionally corrupt memory
      and crash.

      Many of these checks are done with obfuscated code
      to hide the target addresses eg.

      writing 0x27 to 0xffe08e
      00C35C: lea     $ffc92b.l, A4
      00C362: adda.l  #$1763, A4
      00C368: move.b  #$27, (A4)

      This makes it more difficult to find where the checks
      are being performed as an additional layer of
      security

      Squash has a similar timing loop, but with the
      expected values adjusted due to the different 68000
      clock on the otherwise identical Squash PCB (10Mhz on
      Squash vs. 12Mhz on Thunder Hoop)  In the case of
      Squash the most obvious sign of failure is bad
      'Insert Coin' sprites at the bottom of the screen
      after a continue.

      A refresh rate of 57.42, while not yet accurately
      measured, allows a video of thoop to stay in sync with
      MAME over a 10 minute period.

      No checks have been observed in Biomechanical Toy,
      the Maniac Square prototype, or the Last KM prototype.

      Big Karnak runs on a different board type and does fail
      if the CPU clock is set to 10Mhz rather than 12Mhz, it
      also has additional checks which may still fail and
      need more extensive research to determine exactly what
      is being timed.

***************************************************************************/

#include "emu.h"
#include "gaelco.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

template <unsigned Which>
void gaelco_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, state);
}

template <unsigned Which>
void gaelco_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void gaelco_state::oki_bankswitch_w(u8 data)
{
	m_okibank->set_entry(data & 0x0f);
}

void gaelco_state::irqack_w(u16 data)
{
	// INT 6 ACK or Watchdog timer - written at the end of an IRQ
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

/*********** Squash Encryption Related Code ******************/

void squash_state::vram_encrypted_w(offs_t offset, u16 data, u16 mem_mask)
{
	// osd_printf_debug("vram_encrypted_w!!\n");
	data = m_vramcrypt->gaelco_decrypt(*m_maincpu, offset, data);
	vram_w(offset, data, mem_mask);
}


void squash_state::encrypted_w(offs_t offset, u16 data, u16 mem_mask)
{
	// osd_printf_debug("encrypted_w!!\n");
	data = m_vramcrypt->gaelco_decrypt(*m_maincpu, offset, data);
	COMBINE_DATA(&m_screenram[offset]);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void bigkarnk_state::bigkarnk_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                              // ROM
	map(0x100000, 0x101fff).ram().w(FUNC(bigkarnk_state::vram_w)).share(m_videoram);              // Video RAM
	map(0x102000, 0x103fff).ram();                                                              // Screen RAM
	map(0x108000, 0x108007).writeonly().share(m_vregs);                                         // Video Registers
	map(0x10800c, 0x10800d).w(FUNC(bigkarnk_state::irqack_w));                                    // INT 6 ACK/Watchdog timer
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // Palette
	map(0x440000, 0x440fff).ram().share(m_spriteram);                                           // Sprite RAM
	map(0x700000, 0x700001).portr("DSW1");
	map(0x700002, 0x700003).portr("DSW2");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x700008, 0x700009).portr("SERVICE");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000f, 0x70000f).w(m_soundlatch, FUNC(generic_latch_8_device::write));               // Triggers a FIRQ on the sound CPU
	map(0xff8000, 0xffffff).ram();                                                              // Work RAM
}

void bigkarnk_state::bigkarnk_snd_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();                                                                // RAM
	map(0x0800, 0x0801).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // OKI6295
//  map(0x0900, 0x0900).nopw();                                                                   // Enable sound output?
	map(0x0a00, 0x0a01).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));   // YM3812
	map(0x0b00, 0x0b00).r(m_soundlatch, FUNC(generic_latch_8_device::read));                  // Sound latch
	map(0x0c00, 0xffff).rom();                                                                // ROM
}

void gaelco_state::maniacsq_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                // ROM
	map(0x100000, 0x101fff).ram().w(FUNC(gaelco_state::vram_w)).share(m_videoram);                // Video RAM
	map(0x102000, 0x103fff).ram();                                                                // Screen RAM
	map(0x108000, 0x108007).writeonly().share(m_vregs);                                           // Video Registers
	map(0x10800c, 0x10800d).w(FUNC(gaelco_state::irqack_w));                                      // INT 6 ACK/Watchdog timer
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");   // Palette
	map(0x440000, 0x440fff).ram().share(m_spriteram);                                             // Sprite RAM
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000d, 0x70000d).w(FUNC(gaelco_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // OKI6295 status register
	map(0xff0000, 0xffffff).ram();                                                                // Work RAM
}

void xorwflat_state::xorwflat_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x101fff).ram().w(FUNC(xorwflat_state::vram_w)).share(m_videoram);
	map(0x102000, 0x103fff).ram();
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x440000, 0x440fff).ram().share(m_spriteram);
	map(0x441000, 0x441007).ram();  // writes a few things to 0x44100x on startup

	map(0x700000, 0x700001).portr("DSW1");
	map(0x700002, 0x700003).portr("DSW2");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");

	map(0x700010, 0x700017).ram().share(m_vregs);
	map(0x70001b, 0x70001b).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x70001c, 0x70001d).nopr().w(FUNC(xorwflat_state::irqack_w));

	map(0xff0000, 0xffffff).ram();
}


void squash_state::squash_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                               // ROM
	map(0x100000, 0x101fff).ram().w(FUNC(squash_state::vram_encrypted_w)).share(m_videoram);     // Video RAM
	map(0x102000, 0x103fff).ram().w(FUNC(squash_state::encrypted_w)).share(m_screenram);         // Screen RAM
	map(0x108000, 0x108007).writeonly().share(m_vregs);                                          // Video Registers
	map(0x10800c, 0x10800d).w(FUNC(squash_state::irqack_w));                                     // INT 6 ACK/Watchdog timer
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  // Palette
	map(0x440000, 0x440fff).ram().share(m_spriteram);                                            // Sprite RAM
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(squash_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // OKI6295 status register
	map(0xff0000, 0xffffff).ram();                                                                // Work RAM
}

void thoop_state::thoop_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM
	map(0x100000, 0x101fff).ram().w(FUNC(thoop_state::vram_encrypted_w)).share(m_videoram); // Video RAM
	map(0x102000, 0x103fff).ram().w(FUNC(thoop_state::encrypted_w)).share(m_screenram);     // Screen RAM
	map(0x108000, 0x108007).writeonly().share(m_vregs);                                            // Video Registers
	map(0x10800c, 0x10800d).w(FUNC(thoop_state::irqack_w));                                       // INT 6 ACK/Watchdog timer
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x440000, 0x440fff).ram().share(m_spriteram);                                              // Sprite RAM
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(thoop_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // OKI6295 status register
	map(0xff0000, 0xffffff).ram();                                                                 // Work RAM
}


void gaelco_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}

void xorwflat_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xf800, 0xffff).ram();
}

void xorwflat_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x88, 0x89).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xc0, 0xc0).r("soundlatch", FUNC(generic_latch_8_device::read));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

// Common Inputs used by all the games
static INPUT_PORTS_START( gaelco )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( bigkarnk )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Impact" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x02, 0x02, "Go to test mode now" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( maniacsq )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Mono ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( xorwflat )
	PORT_INCLUDE( gaelco )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off) ) // it emits the coin in sound, but it doesn't coin up
	PORT_DIPSETTING(    0x08, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END


static INPUT_PORTS_START( sltpcycld )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// TODO
	PORT_START("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static INPUT_PORTS_START( biomtoy )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") // Not Listed/shown in test mode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") // Not Listed/shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3") // Test mode doesn't show the value of the lives given, but of the ones after you die the first time
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( biomtoyc )
	PORT_INCLUDE( biomtoy )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x0c, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0xc0, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bioplayc )
	PORT_INCLUDE( biomtoy )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x0c, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0xc0, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )

	PORT_MODIFY("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5") // Not Listed/shown in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( lastkm )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Pedal
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Gear Up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gear Down
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0xd0, "Free Play (duplicate 1)" )
	PORT_DIPSETTING(    0x00, "Free Play (duplicate 2)" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Master" ) PORT_DIPLOCATION("SW2:8") // presumably has a link feature?
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( squash )
	PORT_INCLUDE( gaelco )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, "2 Player Continue" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "2 Credits / 5 Games" )
	PORT_DIPSETTING(    0x00, "1 Credit / 3 Games" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Number of Faults" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4") // Not Listed/shown in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2") // Listed as "Unused" in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( thoop )
	PORT_INCLUDE( squash )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, "Player Controls" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, "2 Joysticks" )
	PORT_DIPSETTING(    0x00, "1 Joystick" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout8 =
{
	8,8,                                                // 8x8 tiles
	RGN_FRAC(1,4),                                      // number of tiles
	4,                                                  // bitplanes
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, // plane offsets
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout tilelayout16 =
{
	16,16,                                              // 16x16 tiles
	RGN_FRAC(1,4),                                      // number of tiles
	4,                                                  // bitplanes
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, // plane offsets
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_gaelco )
	GFXDECODE_ENTRY( "gfx", 0, tilelayout8,  0, 64 )
	GFXDECODE_ENTRY( "gfx", 0, tilelayout16, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void gaelco_state::machine_start()
{
	if (m_okibank) //bigkarnk oki isn't banked
		m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

// TODO: verify all clocks (XTALs are 8.0MHz & 24.000 MHz) - One PCB reported to have 8867.23 kHz instead of 8MHz
void bigkarnk_state::bigkarnk(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P10, 12 MHz (verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &bigkarnk_state::bigkarnk_map);
	m_maincpu->set_vblank_int("screen", FUNC(bigkarnk_state::irq6_line_assert));

	MC6809E(config, m_audiocpu, XTAL(8'000'000)/4);  // 68B09EP, 2 MHz (verified)
	m_audiocpu->set_addrmap(AS_PROGRAM, &bigkarnk_state::bigkarnk_snd_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(bigkarnk_state::coin_lockout_w<0>)).invert();
	m_outlatch->q_out_cb<1>().set(FUNC(bigkarnk_state::coin_lockout_w<1>)).invert();
	m_outlatch->q_out_cb<2>().set(FUNC(bigkarnk_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(bigkarnk_state::coin_counter_w<1>));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(bigkarnk_state::screen_update_bigkarnk));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	YM3812(config, "ymsnd", XTAL(8'000'000)/2).add_route(ALL_OUTPUTS, "mono", 1.0); // 4 MHz matches PCB recording

	OKIM6295(config, "oki", XTAL(8'000'000)/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void gaelco_state::maniacsq(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000)/2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco_state::maniacsq_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco_state::irq6_line_assert));

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(FRAMERATE_922804);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco_state::screen_update_maniacsq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // pin 7 not verified
	oki.set_addrmap(0, &gaelco_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void xorwflat_state::xorwflat(machine_config &config)
{
	// Basic machine hardware - guessed, no PCB available
	M68000(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &xorwflat_state::xorwflat_map);
	m_maincpu->set_vblank_int("screen", FUNC(xorwflat_state::irq6_line_assert));

	Z80(config, m_audiocpu, 4'000'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &xorwflat_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &xorwflat_state::sound_io_map);

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline("audiocpu", 0);

	// Video hardware - guessed, no PCB available
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(FRAMERATE_922804);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 256-1, 16, 256-1);
	screen.set_screen_update(FUNC(xorwflat_state::screen_update_maniacsq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", 4'000'000).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void squash_state::squash(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2); // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &squash_state::squash_map);
	m_maincpu->set_vblank_int("screen", FUNC(squash_state::irq6_line_assert));

	config.set_maximum_quantum(attotime::from_hz(600));

	GAELCO_VRAM_ENCRYPTION(config, m_vramcrypt);
	m_vramcrypt->set_params(0x0f, 0x4228);

	LS259(config, m_outlatch); // B8
	m_outlatch->q_out_cb<0>().set(FUNC(squash_state::coin_lockout_w<0>)).invert();
	m_outlatch->q_out_cb<1>().set(FUNC(squash_state::coin_lockout_w<1>)).invert();
	m_outlatch->q_out_cb<2>().set(FUNC(squash_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(squash_state::coin_counter_w<1>));
	m_outlatch->q_out_cb<4>().set_nop(); // used

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(FRAMERATE_922804);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(squash_state::screen_update_squash));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // verified on PCB
	oki.set_addrmap(0, &squash_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void thoop_state::thoop(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000)/2); // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &thoop_state::thoop_map);
	m_maincpu->set_vblank_int("screen", FUNC(thoop_state::irq6_line_assert));

	config.set_maximum_quantum(attotime::from_hz(600));

	GAELCO_VRAM_ENCRYPTION(config, m_vramcrypt);
	m_vramcrypt->set_params(0x0e, 0x4228);

	LS259(config, m_outlatch); // B8
	m_outlatch->q_out_cb<0>().set(FUNC(thoop_state::coin_lockout_w<0>)); // not inverted
	m_outlatch->q_out_cb<1>().set(FUNC(thoop_state::coin_lockout_w<1>)); // not inverted
	m_outlatch->q_out_cb<2>().set(FUNC(thoop_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(thoop_state::coin_counter_w<1>));
	m_outlatch->q_out_cb<4>().set_nop(); // used

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(FRAMERATE_922804);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(thoop_state::screen_update_squash));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // pin 7 not verified
	oki.set_addrmap(0, &thoop_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( bigkarnk ) // PCB silkscreened REF.901112
	ROM_REGION( 0x080000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "d16", 0x000000, 0x040000, CRC(44fb9c73) SHA1(c33852b37afea15482f4a43cb045434660e7a056) )
	ROM_LOAD16_BYTE( "d19", 0x000001, 0x040000, CRC(ff79dfdd) SHA1(2bfa440299317967ba2018d3a148291ae0c144ae) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   // 6809 code
	ROM_LOAD( "d5", 0x000000, 0x010000, CRC(3b73b9c5) SHA1(1b1c5545609a695dab87d611bd53e0c3dd91e6b7) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "h5", 0x000000, 0x080000, CRC(20e239ff) SHA1(685059340f0f3a8e3c98702bd760dae685a58ddb) )
	ROM_LOAD( "h10",0x080000, 0x080000, CRC(ab442855) SHA1(bcd69d4908ff8dc1b2215d2c2d2e54b950e0c015) )
	ROM_LOAD( "h8", 0x100000, 0x080000, CRC(83dce5a3) SHA1(b4f9473e93c96f4b86c446e89d13fd3ef2b03996) )
	ROM_LOAD( "h6", 0x180000, 0x080000, CRC(24e84b24) SHA1(c0ad6ce1e4b8aa7b9c9a3db8bb0165e90f4b48ed) )

	ROM_REGION( 0x040000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "d1", 0x000000, 0x040000, CRC(26444ad1) SHA1(804101b9bbb6e1b6d43a1e9d91737f9c3b27802a) )

	ROM_REGION( 0x26e, "plds", 0 )
	ROM_LOAD( "bigkarnak_gal16v8.d6",  0x000, 0x117, BAD_DUMP CRC(587fe895) SHA1(bc244a50820ceff9bfbdc7f04bcac030198371ec) ) // Bruteforced but verified
	ROM_LOAD( "bigkarnak_gal20v8.d21", 0x117, 0x157, BAD_DUMP CRC(0dcb286e) SHA1(4071af05ba0fd17446691e53bdbba736e416bf4a) ) // Bruteforced but verified
ROM_END

ROM_START( bigkarnka )
	ROM_REGION( 0x080000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "16_20_27c020.bin", 0x000000, 0x040000, CRC(3e5d33d9) SHA1(9a1a82db2359eebb504fd10490762b8c34fa2e33) )
	ROM_LOAD16_BYTE( "20_19_27c020.bin", 0x000001, 0x040000, CRC(508fd6a1) SHA1(69bc515106ed32e5f85ca17467bd175af1c70b44) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   // 6809 code
	ROM_LOAD( "gaelco_2_karnak_27512.bin", 0x000000, 0x010000, CRC(3b73b9c5) SHA1(1b1c5545609a695dab87d611bd53e0c3dd91e6b7) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "karnak_h5_23c4000.bin", 0x000000, 0x080000, CRC(20e239ff) SHA1(685059340f0f3a8e3c98702bd760dae685a58ddb) )
	ROM_LOAD( "h10",                   0x080000, 0x080000, CRC(ab442855) SHA1(bcd69d4908ff8dc1b2215d2c2d2e54b950e0c015) )
	ROM_LOAD( "h8",                    0x100000, 0x080000, CRC(83dce5a3) SHA1(b4f9473e93c96f4b86c446e89d13fd3ef2b03996) )
	ROM_LOAD( "h6",                    0x180000, 0x080000, CRC(24e84b24) SHA1(c0ad6ce1e4b8aa7b9c9a3db8bb0165e90f4b48ed) )

	ROM_REGION( 0x040000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "gaelco_1_karnak_27c020.bin", 0x000000, 0x040000, CRC(26444ad1) SHA1(804101b9bbb6e1b6d43a1e9d91737f9c3b27802a) )

	ROM_REGION( 0x26e, "plds", 0 )
	ROM_LOAD( "gal16v8.bin",    0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD( "f0_gal16v8.bin", 0x000, 0x117, CRC(a733f0de) SHA1(6eec26043cedb3cae4efe93faa84a07327be468b) )
ROM_END

ROM_START( maniacsp ) // PCB - REF 922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE(    "d18",  0x000000, 0x020000, CRC(740ecab2) SHA1(8d8583364cc6aeea58ea2b9cb9a2aab2a43a44df) )
	ROM_LOAD16_BYTE(    "d16",  0x000001, 0x020000, CRC(c6c42729) SHA1(1aac9f93d47a4eb57e06e206e9f50e349b1817da) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "f3", 0x000000, 0x040000, CRC(e7f6582b) SHA1(9e352edf2f71d0edecb54a11ab3fd0e3ec867d42) )
	// 0x040000-0x07ffff empty
	ROM_LOAD( "f2", 0x080000, 0x040000, CRC(ca43a5ae) SHA1(8d2ed537be1dee60096a58b68b735fb50cab3285) )
	// 0x0c0000-0x0fffff empty
	ROM_LOAD( "f1", 0x100000, 0x040000, CRC(fca112e8) SHA1(2a1412f8f1c856b18b6cc7794191d327a415266f) )
	// 0x140000-0x17ffff empty
	ROM_LOAD( "f0", 0x180000, 0x040000, CRC(6e829ee8) SHA1(b602da8d987c1bafa41baf5d5e5d753e29ff5403) )
	// 0x1c0000-0x1fffff empty

	ROM_REGION( 0x080000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(2557f2d6) SHA1(3a99388f2d845281f73a427d6dc797dce87b2f82) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END


/***************************************************************************

Biomechanical Toy

REF.922804/2
+-------------------------------------------------+
| 2     6      PAL                     6116       |
| 4     8    65728                     6116       |
| M SW1 0   18.D18                                |
| H     0    65728                                |
| z SW2 0   16.D16                                |
|                   +----------+                  |
|J                  |TMS       |        65764     |
|A                  |TPC1020AFN|        65764     |
|M      65764       |   -084C  |          PAL     |
|M      65764       +----------+           H10 J10|
|A                  +----------+           H9 J9  |
|                   |TMS       |           H7 J7  |
|                   |TPC1020AFN|           H6 J6  |
|  1MHz  M6295      |   -084C  |           65728  |
|                   +----------+           65728  |
|  VR1   C3     PAL                        65728  |
|        C1    26MHz                       65728  |
+-------------------------------------------------+

  CPU: MC68000P12
Sound: OKI M6295
Video: TMS TCP1020AFN-084C (x2)
  OSC: 26MHz, 24MHz & 1MHz resonator
  RAM: MHS HM3-65756K-5  32K x 8 SRAM (x2)
       MHS HM3-65728B-5  2K x 8 SRAM (x6)
  PAL: TI F20L8-25CNT DIP24 (x3)
  VR1: Volume pot
   SW: Two 8 switch dipswitches

NOTE: Sadly Gaelco didn't differentiate between versions on the ROM labels.
      Just a version number on the initial boot up screen and ROM checksum
      on the calibration screen which followed.

***************************************************************************/


ROM_START( biomtoy ) // PCB - REF.922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "18.d18",  0x000000, 0x080000, CRC(4569ce64) SHA1(96557aca55779c23f7c2c11fddc618823c04ead0) ) // v1.0.1885
	ROM_LOAD16_BYTE( "16.d16",  0x000001, 0x080000, CRC(739449bd) SHA1(711a8ea5081f15dea6067577516c9296239c4145) ) // v1.0.1885

	ROM_REGION( 0x400000, "gfx", 0 )
	// weird gfx ordering
	ROM_LOAD( "h6",  0x040000, 0x040000, CRC(9416a729) SHA1(425149b3041554579791fc23c09fda6be054e89d) )
	ROM_CONTINUE(    0x0c0000, 0x040000 )
	ROM_LOAD( "j6",  0x000000, 0x040000, CRC(e923728b) SHA1(113eac1de73c74ef7c9d3e2e72599a1ff775176d) )
	ROM_CONTINUE(    0x080000, 0x040000 )
	ROM_LOAD( "h7",  0x140000, 0x040000, CRC(9c984d7b) SHA1(98d43a9c3fa93c9ea55f41475ecab6ca25713087) )
	ROM_CONTINUE(    0x1c0000, 0x040000 )
	ROM_LOAD( "j7",  0x100000, 0x040000, CRC(0e18fac2) SHA1(acb0a3699395a6c68cacdeadda42a785aa4020f5) )
	ROM_CONTINUE(    0x180000, 0x040000 )
	ROM_LOAD( "h9",  0x240000, 0x040000, CRC(8c1f6718) SHA1(9377e838ebb1e16d24072b9b4ed278408d7a808f) )
	ROM_CONTINUE(    0x2c0000, 0x040000 )
	ROM_LOAD( "j9",  0x200000, 0x040000, CRC(1c93f050) SHA1(fabeffa05dae7a83a199a57022bd318d6ad02c4d) )
	ROM_CONTINUE(    0x280000, 0x040000 )
	ROM_LOAD( "h10", 0x340000, 0x040000, CRC(aca1702b) SHA1(6b36b230722270dbfc2f69bd7eb07b9e718db089) )
	ROM_CONTINUE(    0x3c0000, 0x040000 )
	ROM_LOAD( "j10", 0x300000, 0x040000, CRC(8e3e96cc) SHA1(761009f3f32b18139e98f20a22c433b6a49d9168) )
	ROM_CONTINUE(    0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(0f02de7e) SHA1(a8779370cc36290616794ff11eb3eebfdea5b1a9) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(914e4bbc) SHA1(ca82b7481621a119f05992ed093b963da70d748a) )
ROM_END


ROM_START( biomtoya ) // PCB - REF.922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "18.d18", 0x000000, 0x080000, CRC(39b6cdbd) SHA1(3a22eb2e304d85ecafff677d83c3c4fca3f869d5) ) // v1.0.1884 - sldh
	ROM_LOAD16_BYTE( "16.d16", 0x000001, 0x080000, CRC(ab340671) SHA1(83f708a535048e927fd1c7de85a65282e460f98a) ) // v1.0.1884 - sldh

	ROM_REGION( 0x400000, "gfx", 0 )
	// weird gfx ordering
	ROM_LOAD( "h6",  0x040000, 0x040000, CRC(9416a729) SHA1(425149b3041554579791fc23c09fda6be054e89d) )
	ROM_CONTINUE(    0x0c0000, 0x040000 )
	ROM_LOAD( "j6",  0x000000, 0x040000, CRC(e923728b) SHA1(113eac1de73c74ef7c9d3e2e72599a1ff775176d) )
	ROM_CONTINUE(    0x080000, 0x040000 )
	ROM_LOAD( "h7",  0x140000, 0x040000, CRC(9c984d7b) SHA1(98d43a9c3fa93c9ea55f41475ecab6ca25713087) )
	ROM_CONTINUE(    0x1c0000, 0x040000 )
	ROM_LOAD( "j7",  0x100000, 0x040000, CRC(0e18fac2) SHA1(acb0a3699395a6c68cacdeadda42a785aa4020f5) )
	ROM_CONTINUE(    0x180000, 0x040000 )
	ROM_LOAD( "h9",  0x240000, 0x040000, CRC(8c1f6718) SHA1(9377e838ebb1e16d24072b9b4ed278408d7a808f) )
	ROM_CONTINUE(    0x2c0000, 0x040000 )
	ROM_LOAD( "j9",  0x200000, 0x040000, CRC(1c93f050) SHA1(fabeffa05dae7a83a199a57022bd318d6ad02c4d) )
	ROM_CONTINUE(    0x280000, 0x040000 )
	ROM_LOAD( "h10", 0x340000, 0x040000, CRC(aca1702b) SHA1(6b36b230722270dbfc2f69bd7eb07b9e718db089) )
	ROM_CONTINUE(    0x3c0000, 0x040000 )
	ROM_LOAD( "j10", 0x300000, 0x040000, CRC(8e3e96cc) SHA1(761009f3f32b18139e98f20a22c433b6a49d9168) )
	ROM_CONTINUE(    0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END


ROM_START( biomtoyb ) // PCB - REF.922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "18.d18", 0x000000, 0x080000, CRC(2dfadee3) SHA1(55ab563a9a69da940ca015f292476068cf21b01c) ) // v1.0.1878 - sldh
	ROM_LOAD16_BYTE( "16.d16", 0x000001, 0x080000, CRC(b35e3ca6) SHA1(b323fcca99d088e6fbf6a1d660ef860987af77e4) ) // v1.0.1878 - sldh

	ROM_REGION( 0x400000, "gfx", 0 ) // Graphics & Sound ROMs soldered in, not verified 100% correct for this set
	// weird gfx ordering
	ROM_LOAD( "h6",  0x040000, 0x040000, CRC(9416a729) SHA1(425149b3041554579791fc23c09fda6be054e89d) )
	ROM_CONTINUE(    0x0c0000, 0x040000 )
	ROM_LOAD( "j6",  0x000000, 0x040000, CRC(e923728b) SHA1(113eac1de73c74ef7c9d3e2e72599a1ff775176d) )
	ROM_CONTINUE(    0x080000, 0x040000 )
	ROM_LOAD( "h7",  0x140000, 0x040000, CRC(9c984d7b) SHA1(98d43a9c3fa93c9ea55f41475ecab6ca25713087) )
	ROM_CONTINUE(    0x1c0000, 0x040000 )
	ROM_LOAD( "j7",  0x100000, 0x040000, CRC(0e18fac2) SHA1(acb0a3699395a6c68cacdeadda42a785aa4020f5) )
	ROM_CONTINUE(    0x180000, 0x040000 )
	ROM_LOAD( "h9",  0x240000, 0x040000, CRC(8c1f6718) SHA1(9377e838ebb1e16d24072b9b4ed278408d7a808f) )
	ROM_CONTINUE(    0x2c0000, 0x040000 )
	ROM_LOAD( "j9",  0x200000, 0x040000, CRC(1c93f050) SHA1(fabeffa05dae7a83a199a57022bd318d6ad02c4d) )
	ROM_CONTINUE(    0x280000, 0x040000 )
	ROM_LOAD( "h10", 0x340000, 0x040000, CRC(aca1702b) SHA1(6b36b230722270dbfc2f69bd7eb07b9e718db089) )
	ROM_CONTINUE(    0x3c0000, 0x040000 )
	ROM_LOAD( "j10", 0x300000, 0x040000, CRC(8e3e96cc) SHA1(761009f3f32b18139e98f20a22c433b6a49d9168) )
	ROM_CONTINUE(    0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END


ROM_START( biomtoyc ) // PCB - REF.922804/1 or REF.922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "program18.d18", 0x000000, 0x080000, CRC(05ad7d30) SHA1(4b2596d225bf9b314db5a150921d7d6c99096ddb) ) // v1.0.1870 - sldh
	ROM_LOAD16_BYTE( "program16.d16", 0x000001, 0x080000, CRC(a288e73f) SHA1(13a53981e3fe6961494013e7466badae56481958) ) // v1.0.1870 - sldh

	ROM_REGION( 0x400000, "gfx", 0 ) // Graphics & Sound ROMs redumped from a REF.922804/1 PCB
	// weird gfx ordering
	ROM_LOAD( "gfx6.h6",   0x040000, 0x040000, CRC(ab19a1ce) SHA1(3cc896f8c20f692b02d43db8c30f410bd93fe3ca) )
	ROM_CONTINUE(          0x0c0000, 0x040000 )
	ROM_LOAD( "gfx10.j6",  0x000000, 0x040000, CRC(7b2dc36c) SHA1(2b227c24b26505148d304bb0a6cca1091ec077c8) )
	ROM_CONTINUE(          0x080000, 0x040000 )
	ROM_LOAD( "gfx7.h7",   0x140000, 0x040000, CRC(4bc82598) SHA1(8247d9003ef815f35991cfb17068c87333120b85) )
	ROM_CONTINUE(          0x1c0000, 0x040000 )
	ROM_LOAD( "gfx11.j7",  0x100000, 0x040000, CRC(aff7fd0e) SHA1(41d57b6e5b29f1d4ac9fbd2268e8239ca342ae2c) )
	ROM_CONTINUE(          0x180000, 0x040000 )
	ROM_LOAD( "gfx8.h9",   0x240000, 0x040000, CRC(09de4799) SHA1(120b7bd8e20288c3aec62d3b2bf3f87e251c3eea) )
	ROM_CONTINUE(          0x2c0000, 0x040000 )
	ROM_LOAD( "gfx12.j9",  0x200000, 0x040000, CRC(7b27b2a9) SHA1(3d9d52266d2422dafa5a33a43d90fe32d7e18c47) )
	ROM_CONTINUE(          0x280000, 0x040000 )
	ROM_LOAD( "gfx9.h10",  0x340000, 0x040000, CRC(38bcd72d) SHA1(ced1aee3841cddefb3ad4b3b0bf90a067278e76c) )
	ROM_CONTINUE(          0x3c0000, 0x040000 )
	ROM_LOAD( "gfx13.j10", 0x300000, 0x040000, CRC(52c984df) SHA1(138f434119e84d5efb3d42ebaafe6c2cdc1a06aa) )
	ROM_CONTINUE(          0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sound1.c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "sound2.c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END

ROM_START( bioplayc ) // PCB - REF.922804/2?? - Spanish version
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "t.d18", 0x000000, 0x080000, CRC(ec518c6c) SHA1(8b96313582d252bebb4bcce8f2d993f751ad0a74) ) // v1.0.1823
	ROM_LOAD16_BYTE( "t.d16", 0x000001, 0x080000, CRC(de4b031d) SHA1(d4bcdfedab1d48df0c48ffc775731a4981342c7a) ) // v1.0.1823

	ROM_REGION( 0x400000, "gfx", 0 )
	// weird gfx ordering
	ROM_LOAD( "toy-high-3.h6",  0x040000, 0x040000, CRC(ab19a1ce) SHA1(3cc896f8c20f692b02d43db8c30f410bd93fe3ca) )
	ROM_CONTINUE(               0x0c0000, 0x040000 )
	ROM_LOAD( "toy-low-3.j6",   0x000000, 0x040000, CRC(927f5cd7) SHA1(ad5e75091146ca7935a18e5dd045410e28d8b170) )
	ROM_CONTINUE(               0x080000, 0x040000 )
	ROM_LOAD( "toy-high-2.h7",  0x140000, 0x040000, CRC(fd975d89) SHA1(89bb85ccb1ba0bb82f393ef27757c0778dd696b3) )
	ROM_CONTINUE(               0x1c0000, 0x040000 )
	ROM_LOAD( "toy-low-2.j7",   0x100000, 0x040000, CRC(6cbf9937) SHA1(77123a8afea3108df54f45033dfb7f86c1d0d1b8) )
	ROM_CONTINUE(               0x180000, 0x040000 )
	ROM_LOAD( "toy-high-1.h9",  0x240000, 0x040000, CRC(09de4799) SHA1(120b7bd8e20288c3aec62d3b2bf3f87e251c3eea) )
	ROM_CONTINUE(               0x2c0000, 0x040000 )
	ROM_LOAD( "toy-low-1.j9",   0x200000, 0x040000, CRC(57922c41) SHA1(ffbe5b418ed93e8705a7aabe69d3fad2919a160f) )
	ROM_CONTINUE(               0x280000, 0x040000 )
	ROM_LOAD( "toy-high-0.h10", 0x340000, 0x040000, CRC(5bee6df7) SHA1(ecf759de2f0909f793c84c71feb08801896e2474) )
	ROM_CONTINUE(               0x3c0000, 0x040000 )
	ROM_LOAD( "toy-low-0.j10",  0x300000, 0x040000, CRC(26c49ca2) SHA1(82079eaa2c9523c9acb72fccfbbe9493bc62e84f) )
	ROM_CONTINUE(               0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	// Missing the audio ROM, the board didn't have it populated. The programmer said it was not there because the audio was ripped from other games.
	ROM_LOAD( "c1", 0x000000, 0x080000, BAD_DUMP CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "c3", 0x080000, 0x080000, BAD_DUMP CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END

ROM_START( lastkm ) // PCB - REF 922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "prog-bici-e-8.11.95.d18", 0x000000, 0x080000, CRC(1fc5fba0) SHA1(1f954fca9f25df7379eff4ea905810fa06fcebb0) ) // 1.0.0275
	ROM_LOAD16_BYTE( "prog-bici-o-8.11.95.d16", 0x000001, 0x080000, CRC(b93e57e3) SHA1(df307191a214a32a26018ca2a9200742e39939d2) ) // 1.0.0275

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "bici-f3.h6",  0x000000, 0x080000, CRC(0bf9f213) SHA1(052abef60df419d32bf8a86c89d87e5bb281b4eb) )
	ROM_LOAD( "bici-f2.h7",  0x080000, 0x080000, CRC(c48d5376) SHA1(8e987839e7254e0fa631802733482726a289439c) )
	ROM_LOAD( "bici-f1.h9",  0x100000, 0x080000, CRC(e7958070) SHA1(7f065b429a500b714dfbf497b1353e90137abbd7) )
	ROM_LOAD( "bici-f0.h10", 0x180000, 0x080000, CRC(73d4b29f) SHA1(e2563562cb5fbaba7e0517ec9811645aca56f374) )

	ROM_REGION( 0x80000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sonido-bici-0-8.11.95.c1", 0x000000, 0x080000, CRC(7380c963) SHA1(d1d90912f986b944cd95bd773c9f5502d837ce3e) )
ROM_END

ROM_START( lastkma )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "bici_zeus_0_20-2_27c040.bin", 0x000000, 0x080000, CRC(6a4a982c) SHA1(d7804b0100445740001d2938f6536fc5e857691c) )
	ROM_LOAD16_BYTE( "bici_zeus_1_20-2_27c040.bin", 0x000001, 0x080000, CRC(b63eb33e) SHA1(64bda2bd14b2ff27ed291a6367ad82c2b781c4e6) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "bici-f3.h6",  0x000000, 0x080000, CRC(0bf9f213) SHA1(052abef60df419d32bf8a86c89d87e5bb281b4eb) )
	ROM_LOAD( "bici-f2.h7",  0x080000, 0x080000, CRC(c48d5376) SHA1(8e987839e7254e0fa631802733482726a289439c) )
	ROM_LOAD( "bici-f1.h9",  0x100000, 0x080000, CRC(e7958070) SHA1(7f065b429a500b714dfbf497b1353e90137abbd7) )
	ROM_LOAD( "bici-f0.h10", 0x180000, 0x080000, CRC(73d4b29f) SHA1(e2563562cb5fbaba7e0517ec9811645aca56f374) )

	ROM_REGION( 0x80000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sonido-bici-0-8.11.95.c1", 0x000000, 0x080000, CRC(7380c963) SHA1(d1d90912f986b944cd95bd773c9f5502d837ce3e) )
ROM_END

/*
Squash (version 1.0)
Gaelco, 1992

PCB Layout
----------

REF.922804/1
|---------------------------------------------|
|   LM358  SOUND.1D    26MHz     6116         |
|   VOL                PAL       6116         |
|          M6295                 6116         |
|  1MHz            |-------|     6116         |
|                  |ACTEL  |                  |
|J                 |A1020A |    C12.6H        |
|A         6116    |PL84C  |    C11.7H        |
|M                 |-------|    C10.8H        |
|M         6116                 C09.10H       |
|A                 |-------|     PAL          |
|                  |ACTEL  |    6264          |
|                  |A1020A |    6264          |
|         D16.E16  |PL84C  |             PAL  |
| SW1     62256    |-------|                  |
|    68000                                    |
| SW2     D18.E18               6116          |
|20MHz    62256                 6116          |
|         PAL                                 |
|---------------------------------------------|
Notes:
      68000 CPU running at 10.000MHz
      OKI M6295 running at 1.000MHz. Sample Rate = 1000000 / 132
      62256 - 32k x8 SRAM (x2, DIP28)
      6264  - 8k x8 SRAM  (x2, DIP28)
      6116  - 2k x8 SRAM  (x8, DIP24)
      VSync - 58Hz

      ROMs:
           SQUASH_D16.E16    27C010   \
           SQUASH_D18.E18    27C010   /  68K Program

           SQUASH_C09.10H    27C040   \
           SQUASH_C10.8H     27C040   |
           SQUASH_C11.7H     27C040   |  GFX
           SQUASH_C12.6H     27C040   /

           SQUASH_SOUND.1D   27C040      Sound
*/

ROM_START( squash ) // PCB - REF.922804/1 or REF.922804/2
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "squash.d18", 0x000000, 0x20000, CRC(ce7aae96) SHA1(4fe8666ae571bffc5a08fa68346c0623282989eb) )
	ROM_LOAD16_BYTE( "squash.d16", 0x000001, 0x20000, CRC(8ffaedd7) SHA1(f4aada17ba67dd8b6c5a395e832bcbba2764c59d) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "squash.c09", 0x180000, 0x80000, CRC(0bb91c69) SHA1(8be945049ab411a4d49bd64bd3937542ec9ef9fb) ) // Encrypted video RAM
	ROM_LOAD( "squash.c10", 0x100000, 0x80000, CRC(892a035c) SHA1(d0156ceb9aa6639a1124c17fb12389be319bb51f) ) // Encrypted video RAM
	ROM_LOAD( "squash.c11", 0x080000, 0x80000, CRC(9e19694d) SHA1(1df4646f3147719fef516a37aa361ae26d9b23a2) ) // Encrypted video RAM
	ROM_LOAD( "squash.c12", 0x000000, 0x80000, CRC(5c440645) SHA1(4f2fc1647ffc549fa079f2dc0aaaceb447afdf44) ) // Encrypted video RAM

	ROM_REGION( 0x80000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "squash.d01", 0x000000, 0x80000, CRC(a1b9651b) SHA1(a396ba94889f70ea06d6330e3606b0f2497ff6ce) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "squashv1_gal16v8.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) ) // Unprotected
	ROM_LOAD ( "squashv1_gal16v8.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) ) // Unprotected
	ROM_LOAD ( "squashv1_gal20v8.d21", 0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) ) // Unprotected
	ROM_LOAD ( "squashv1_gal20v8.h11", 0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) ) // Unprotected
ROM_END

ROM_START( squasha )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "sq_p_0_27c010.bin", 0x000000, 0x20000, CRC(275e20e7) SHA1(52a3e4b7a8e9c23f22f4382458c731f283ad68ee) )
	ROM_LOAD16_BYTE( "sq_p_1_27c010.bin", 0x000001, 0x20000, CRC(077cc291) SHA1(fd63b1e030c1219a4c156609e9f2a9a075345b15) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "squash_c-9.c9",   0x180000, 0x80000, CRC(0bb91c69) SHA1(8be945049ab411a4d49bd64bd3937542ec9ef9fb) ) // Encrypted video RAM
	ROM_LOAD( "squash_c-10.c10", 0x100000, 0x80000, CRC(892a035c) SHA1(d0156ceb9aa6639a1124c17fb12389be319bb51f) ) // Encrypted video RAM
	ROM_LOAD( "squash.c11",      0x080000, 0x80000, CRC(9e19694d) SHA1(1df4646f3147719fef516a37aa361ae26d9b23a2) ) // Encrypted video RAM
	ROM_LOAD( "squash_c-12.c12", 0x000000, 0x80000, CRC(5c440645) SHA1(4f2fc1647ffc549fa079f2dc0aaaceb447afdf44) ) // Encrypted video RAM

	ROM_REGION( 0x80000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "squash_sound.bin", 0x000000, 0x80000, CRC(a1b9651b) SHA1(a396ba94889f70ea06d6330e3606b0f2497ff6ce) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "squashv1_gal16v8.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) ) // Unprotected
	ROM_LOAD ( "squashv1_gal16v8.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) ) // Unprotected
	ROM_LOAD ( "squashv1_gal20v8.d21", 0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) ) // Unprotected
	ROM_LOAD ( "squashv1_gal20v8.h11", 0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) ) // Unprotected
ROM_END

// Different PCB. Missing one program ROM
ROM_START( squashb )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "sq_d18_27c020.d18", 0x000000, 0x040000, CRC(10613743) SHA1(7699fd7da0c6bc98c55f10c03084aa382ca16bb1) )
	ROM_LOAD16_BYTE( "sq_d16_27c020.d16", 0x000001, 0x040000, NO_DUMP ) // ROM damaged

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sq_h6_27c040.h6",   0x040000, 0x040000, CRC(3fbb85b1) SHA1(b3436cd3120abcff953feb7ce990cbe433db46c7) )
	ROM_CONTINUE(                  0x0c0000, 0x040000 )
	ROM_LOAD( "sq_j6_27c040.j6",   0x000000, 0x040000, CRC(f48f26f2) SHA1(8d30324fdccdaa368efadba94ade72574d554f17) )
	ROM_CONTINUE(                  0x080000, 0x040000 )
	ROM_LOAD( "sq_h7_27c040.h7",   0x140000, 0x040000, CRC(bbe97cf7) SHA1(1ae4f4b2ff7b1761873f61eac17e86ec50dddd56) )
	ROM_CONTINUE(                  0x1c0000, 0x040000 )
	ROM_LOAD( "sq_j7_27c040.j7",   0x100000, 0x040000, CRC(24984ed5) SHA1(37ac01f4f9659ec8903c88a7165c15ff9fc01b5c) )
	ROM_CONTINUE(                  0x180000, 0x040000 )
	ROM_LOAD( "sq_h9_27c040.h9",   0x240000, 0x040000, CRC(5a65c21b) SHA1(97268f3e77a2b7246631776303842569bb36540a) )
	ROM_CONTINUE(                  0x2c0000, 0x040000 )
	ROM_LOAD( "sq_j9_27c040.j9",   0x200000, 0x040000, CRC(6401ff69) SHA1(bda0d8803802376cc804913e11657e8fb41403de) )
	ROM_CONTINUE(                  0x280000, 0x040000 )
	ROM_LOAD( "sq_h10_27c040.h10", 0x340000, 0x040000, CRC(9c6bf858) SHA1(c1694abdfca92b6513515ce5ba6158e300755867) )
	ROM_CONTINUE(                  0x3c0000, 0x040000 )
	ROM_LOAD( "sq_j10_27c040.j10", 0x300000, 0x040000, CRC(f8cf30e8) SHA1(403bd36261dd8dbb61678120210c82082ea94122) )
	ROM_CONTINUE(                  0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "sq_c1_27c040.c1", 0x000000, 0x080000, CRC(a6ee6670) SHA1(707fc288228cb2b77c95cbf02f86882594383c29) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "sq_c3_27c020.c3", 0x080000, 0x080000, CRC(80eca530) SHA1(0b30df014ee95ddc8fb363639fc7c81774c11f5f) )
ROM_END

/*
There is a Thunder Hoop on a REF.922804/2 PCB, with exactly the same ROM contents,
but on a different chips layout / capacity:
   program.d16             th161eb4.020            IDENTICAL
   program.d18             th18dea1.040            IDENTICAL
   gfx.j10                 c09          [1/2]      IDENTICAL
   gfx.j6                  c12          [1/2]      IDENTICAL
   gfx.j7                  c11          [1/2]      IDENTICAL
   gfx.j9                  c10          [1/2]      IDENTICAL
   sound.c1                sound        [1/2]      IDENTICAL
   gfx.h10                 c09          [2/2]      IDENTICAL
   gfx.h6                  c12          [2/2]      IDENTICAL
   gfx.h7                  c11          [2/2]      IDENTICAL
   gfx.h9                  c10          [2/2]      IDENTICAL
   sound.c3                sound        [2/2]      IDENTICAL
*/
ROM_START( thoop ) // PCB - REF.922804/1
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "th18dea1.040", 0x000000, 0x80000, CRC(59bad625) SHA1(28e058b2290bc5f7130b801014d026432f9e7fd5) )
	ROM_LOAD16_BYTE( "th161eb4.020", 0x000001, 0x40000, CRC(6add61ed) SHA1(0e789d9a0ac19b6143044fbc04ab2227735b2a8f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "c09", 0x300000, 0x040000, CRC(06f0edbf) SHA1(3cf2e5c29cd00b43d49a106084076f2ac0dbad98) ) // Encrypted video RAM
	ROM_CONTINUE(    0x380000, 0x040000 )
	ROM_CONTINUE(    0x340000, 0x040000 )
	ROM_CONTINUE(    0x3c0000, 0x040000 )
	ROM_LOAD( "c10", 0x200000, 0x040000, CRC(2d227085) SHA1(b224efd59ec83bb786fa92a23ef2d27ed36cab6c) ) // Encrypted video RAM
	ROM_CONTINUE(    0x280000, 0x040000 )
	ROM_CONTINUE(    0x240000, 0x040000 )
	ROM_CONTINUE(    0x2c0000, 0x040000 )
	ROM_LOAD( "c11", 0x100000, 0x040000, CRC(7403ef7e) SHA1(52a737816e25a07ada070ed3a5f40bbbd22ac8e0) ) // Encrypted video RAM
	ROM_CONTINUE(    0x180000, 0x040000 )
	ROM_CONTINUE(    0x140000, 0x040000 )
	ROM_CONTINUE(    0x1c0000, 0x040000 )
	ROM_LOAD( "c12", 0x000000, 0x040000, CRC(29a5ca36) SHA1(fdcfdefb3b02bfe34781fdd0295640caabe2a5fb) ) // Encrypted video RAM
	ROM_CONTINUE(    0x080000, 0x040000 )
	ROM_CONTINUE(    0x040000, 0x040000 )
	ROM_CONTINUE(    0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "sound", 0x000000, 0x100000, CRC(99f80961) SHA1(de3a514a8f46dffd5f762e52aac1f4c3b08e2e18) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

ROM_START( thoopa )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "tachado_th_4_europa_deba_22-7-92_27c040.bin", 0x000000, 0x80000, CRC(68d52248) SHA1(302ca927c7fd1a42d70f8b2166951e2de2f0ea82) )
	ROM_LOAD16_BYTE( "tachado_th_3_1eeb_22-7-92_27c020.bin",        0x000001, 0x40000, CRC(84e6c202) SHA1(16838846b7a71c6291c96107b0d903435f81add7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "th_12_3455_27c4001.bin", 0x300000, 0x040000, CRC(c1bacaef) SHA1(9aeac8f386a2547815e2bf77af6cf4c1e4bbf9bd) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x380000, 0x040000 )
	ROM_LOAD( "th_8_9565_27c4001.bin",  0x340000, 0x040000, CRC(4e989ca5) SHA1(e4b77560b7ee7a8f8663415ec527e51630c1cd23) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x3c0000, 0x040000 )
	ROM_LOAD( "th_11_f8cd_27c4001.bin", 0x200000, 0x040000, CRC(0805a690) SHA1(21ddf5c9f94483631acefea56cba75646851d98a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x280000, 0x040000 )
	ROM_LOAD( "th_7_dd91_27c4001.bin",  0x240000, 0x040000, CRC(3af227c5) SHA1(7ecc8311204f2eb2878267e95d9f01a4cf96fbe8) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x2c0000, 0x040000 )
	ROM_LOAD( "th_10_250d_27c4001.bin", 0x100000, 0x040000, CRC(1f7778d3) SHA1(9d2554f2ed275a53b9cd5a774a8dca2f9713989f) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x180000, 0x040000 )
	ROM_LOAD( "th_6_54b2_27c4001.bin",  0x140000, 0x040000, CRC(817618e0) SHA1(df2bd5fca260c765225ef03f97c774d45578c07a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x1c0000, 0x040000 )
	ROM_LOAD( "th_9_6f76_27c4001.bin",  0x000000, 0x040000, CRC(7a396cb4) SHA1(4680f774248c3a568ede9158e8dccd5403707360) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x080000, 0x040000 )
	ROM_LOAD( "th_5_1dca_27c4001.bin",  0x040000, 0x040000, CRC(8db86f56) SHA1(96a8103fc263b625aa46a7d008596ac7a502fbbe) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "th_1_a0b1_27c4001.bin", 0x000000, 0x080000, CRC(a7ba136d) SHA1(291f796104cd76ce9846c792e85cc717ca8a6eab) )
	ROM_LOAD( "th_2_655e_27c4001.bin", 0x080000, 0x080000, CRC(c9e0a14a) SHA1(a4ccfeb7fefd87cfb0adf133bf47c369b2e9b0b4) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

ROM_START( thoopb )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "tachado_th_4_sin_titol_2-7-92_27c040.bin", 0x000000, 0x80000, CRC(a562028f) SHA1(0890ae7caf5ff8ace3db16cd5f641e483415b44d) )
	ROM_LOAD16_BYTE( "tachado_th_3_sin_titol_2-7-92_27c040.bin", 0x000001, 0x80000, CRC(9768c962) SHA1(c75f7b21448f90623236983f9b858b18f3105659) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "th_12_3455_27c4001.bin", 0x300000, 0x040000, CRC(c1bacaef) SHA1(9aeac8f386a2547815e2bf77af6cf4c1e4bbf9bd) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x380000, 0x040000 )
	ROM_LOAD( "th_8_9565_27c4001.bin",  0x340000, 0x040000, CRC(4e989ca5) SHA1(e4b77560b7ee7a8f8663415ec527e51630c1cd23) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x3c0000, 0x040000 )
	ROM_LOAD( "th_11_f8cd_27c4001.bin", 0x200000, 0x040000, CRC(0805a690) SHA1(21ddf5c9f94483631acefea56cba75646851d98a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x280000, 0x040000 )
	ROM_LOAD( "th_7_dd91_27c4001.bin",  0x240000, 0x040000, CRC(3af227c5) SHA1(7ecc8311204f2eb2878267e95d9f01a4cf96fbe8) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x2c0000, 0x040000 )
	ROM_LOAD( "th_10_250d_27c4001.bin", 0x100000, 0x040000, CRC(1f7778d3) SHA1(9d2554f2ed275a53b9cd5a774a8dca2f9713989f) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x180000, 0x040000 )
	ROM_LOAD( "th_6_54b2_27c4001.bin",  0x140000, 0x040000, CRC(817618e0) SHA1(df2bd5fca260c765225ef03f97c774d45578c07a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x1c0000, 0x040000 )
	ROM_LOAD( "th_9_6f76_27c4001.bin",  0x000000, 0x040000, CRC(7a396cb4) SHA1(4680f774248c3a568ede9158e8dccd5403707360) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x080000, 0x040000 )
	ROM_LOAD( "th_5_1dca_27c4001.bin",  0x040000, 0x040000, CRC(8db86f56) SHA1(96a8103fc263b625aa46a7d008596ac7a502fbbe) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "th_1_a0b1_27c4001.bin", 0x000000, 0x080000, CRC(a7ba136d) SHA1(291f796104cd76ce9846c792e85cc717ca8a6eab) )
	ROM_LOAD( "th_2_655e_27c4001.bin", 0x080000, 0x080000, CRC(c9e0a14a) SHA1(a4ccfeb7fefd87cfb0adf133bf47c369b2e9b0b4) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

ROM_START( thoopna )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "th_4_america_de9e_24-8-92_27c040.bin", 0x000000, 0x80000, CRC(f13a6ed5) SHA1(f5c99ee5754e608e38bbc07b0b9f377cdc158deb) )
	ROM_LOAD16_BYTE( "th_3_test_24-8-92_27c020.bin",         0x000001, 0x40000, CRC(23abbb5f) SHA1(021e23c3c860194925139178540639ec1899217d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "th_12_3455_27c4001.bin", 0x300000, 0x040000, CRC(c1bacaef) SHA1(9aeac8f386a2547815e2bf77af6cf4c1e4bbf9bd) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x380000, 0x040000 )
	ROM_LOAD( "th_8_9565_27c4001.bin",  0x340000, 0x040000, CRC(4e989ca5) SHA1(e4b77560b7ee7a8f8663415ec527e51630c1cd23) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x3c0000, 0x040000 )
	ROM_LOAD( "th_11_f8cd_27c4001.bin", 0x200000, 0x040000, CRC(0805a690) SHA1(21ddf5c9f94483631acefea56cba75646851d98a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x280000, 0x040000 )
	ROM_LOAD( "th_7_dd91_27c4001.bin",  0x240000, 0x040000, CRC(3af227c5) SHA1(7ecc8311204f2eb2878267e95d9f01a4cf96fbe8) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x2c0000, 0x040000 )
	ROM_LOAD( "th_10_250d_27c4001.bin", 0x100000, 0x040000, CRC(1f7778d3) SHA1(9d2554f2ed275a53b9cd5a774a8dca2f9713989f) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x180000, 0x040000 )
	ROM_LOAD( "th_6_54b2_27c4001.bin",  0x140000, 0x040000, CRC(817618e0) SHA1(df2bd5fca260c765225ef03f97c774d45578c07a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x1c0000, 0x040000 )
	ROM_LOAD( "th_9_6f76_27c4001.bin",  0x000000, 0x040000, CRC(7a396cb4) SHA1(4680f774248c3a568ede9158e8dccd5403707360) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x080000, 0x040000 )
	ROM_LOAD( "th_5_1dca_27c4001.bin",  0x040000, 0x040000, CRC(8db86f56) SHA1(96a8103fc263b625aa46a7d008596ac7a502fbbe) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "th_1_a0b1_27c4001.bin", 0x000000, 0x080000, CRC(a7ba136d) SHA1(291f796104cd76ce9846c792e85cc717ca8a6eab) )
	ROM_LOAD( "th_2_655e_27c4001.bin", 0x080000, 0x080000, CRC(c9e0a14a) SHA1(a4ccfeb7fefd87cfb0adf133bf47c369b2e9b0b4) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

ROM_START( thoopnna )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "th_4_9-6-92_27c040.bin", 0x000000, 0x80000, CRC(52c8cb1a) SHA1(07d000443a2ba6c43c348833dfe4ff1be9a20b22) )
	ROM_LOAD16_BYTE( "th_3_9-6-92_27c040.bin", 0x000001, 0x80000, CRC(4cc24f6a) SHA1(59e242d1e8b6aa5196716911874168b8ec6e694d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "th_12_3455_27c4001.bin", 0x300000, 0x040000, CRC(c1bacaef) SHA1(9aeac8f386a2547815e2bf77af6cf4c1e4bbf9bd) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x380000, 0x040000 )
	ROM_LOAD( "th_8_9565_27c4001.bin",  0x340000, 0x040000, CRC(4e989ca5) SHA1(e4b77560b7ee7a8f8663415ec527e51630c1cd23) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x3c0000, 0x040000 )
	ROM_LOAD( "th_11_f8cd_27c4001.bin", 0x200000, 0x040000, CRC(0805a690) SHA1(21ddf5c9f94483631acefea56cba75646851d98a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x280000, 0x040000 )
	ROM_LOAD( "th_7_dd91_27c4001.bin",  0x240000, 0x040000, CRC(3af227c5) SHA1(7ecc8311204f2eb2878267e95d9f01a4cf96fbe8) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x2c0000, 0x040000 )
	ROM_LOAD( "th_10_250d_27c4001.bin", 0x100000, 0x040000, CRC(1f7778d3) SHA1(9d2554f2ed275a53b9cd5a774a8dca2f9713989f) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x180000, 0x040000 )
	ROM_LOAD( "th_6_54b2_27c4001.bin",  0x140000, 0x040000, CRC(817618e0) SHA1(df2bd5fca260c765225ef03f97c774d45578c07a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x1c0000, 0x040000 )
	ROM_LOAD( "th_9_6f76_27c4001.bin",  0x000000, 0x040000, CRC(7a396cb4) SHA1(4680f774248c3a568ede9158e8dccd5403707360) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x080000, 0x040000 )
	ROM_LOAD( "th_5_1dca_27c4001.bin",  0x040000, 0x040000, CRC(8db86f56) SHA1(96a8103fc263b625aa46a7d008596ac7a502fbbe) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM6295
	ROM_LOAD( "th_1_a0b1_27c4001.bin", 0x000000, 0x080000, CRC(a7ba136d) SHA1(291f796104cd76ce9846c792e85cc717ca8a6eab) )
	ROM_LOAD( "th_2_655e_27c4001.bin", 0x080000, 0x080000, CRC(c9e0a14a) SHA1(a4ccfeb7fefd87cfb0adf133bf47c369b2e9b0b4) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

ROM_START( thoopnnaa )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "th_4_1fb6_27c4001.bin", 0x000000, 0x80000, CRC(4dcba336) SHA1(13667bd1950f934bedc38478a85a8041f5faf9ba) )
	ROM_LOAD16_BYTE( "th_3_0c6c_27c4001.bin", 0x000001, 0x80000, CRC(9e813244) SHA1(2df853c0bf18d8fe9110c8bca00799af01f5d4ea) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "th_12_3455_27c4001.bin", 0x300000, 0x040000, CRC(c1bacaef) SHA1(9aeac8f386a2547815e2bf77af6cf4c1e4bbf9bd) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x380000, 0x040000 )
	ROM_LOAD( "th_8_9565_27c4001.bin",  0x340000, 0x040000, CRC(4e989ca5) SHA1(e4b77560b7ee7a8f8663415ec527e51630c1cd23) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x3c0000, 0x040000 )
	ROM_LOAD( "th_11_f8cd_27c4001.bin", 0x200000, 0x040000, CRC(0805a690) SHA1(21ddf5c9f94483631acefea56cba75646851d98a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x280000, 0x040000 )
	ROM_LOAD( "th_7_dd91_27c4001.bin",  0x240000, 0x040000, CRC(3af227c5) SHA1(7ecc8311204f2eb2878267e95d9f01a4cf96fbe8) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x2c0000, 0x040000 )
	ROM_LOAD( "th_10_250d_27c4001.bin", 0x100000, 0x040000, CRC(1f7778d3) SHA1(9d2554f2ed275a53b9cd5a774a8dca2f9713989f) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x180000, 0x040000 )
	ROM_LOAD( "th_6_54b2_27c4001.bin",  0x140000, 0x040000, CRC(817618e0) SHA1(df2bd5fca260c765225ef03f97c774d45578c07a) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x1c0000, 0x040000 )
	ROM_LOAD( "th_9_6f76_27c4001.bin",  0x000000, 0x040000, CRC(7a396cb4) SHA1(4680f774248c3a568ede9158e8dccd5403707360) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x080000, 0x040000 )
	ROM_LOAD( "th_5_1dca_27c4001.bin",  0x040000, 0x040000, CRC(8db86f56) SHA1(96a8103fc263b625aa46a7d008596ac7a502fbbe) ) // Encrypted video RAM
	ROM_CONTINUE(                       0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM samples - sound chip is OKIM62914:11 23/05/202514:11 23/05/20255
	ROM_LOAD( "th_1_a0b1_27c4001.bin", 0x000000, 0x080000, CRC(a7ba136d) SHA1(291f796104cd76ce9846c792e85cc717ca8a6eab) )
	ROM_LOAD( "th_2_655e_27c4001.bin", 0x080000, 0x080000, CRC(c9e0a14a) SHA1(a4ccfeb7fefd87cfb0adf133bf47c369b2e9b0b4) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x4dc, "plds", 0)
	ROM_LOAD ( "mu_mu-1_6541_gal16v8as.f2",  0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD ( "mu_mu-4_664c_gal16v8as.j16", 0x117, 0x117, CRC(fe78b903) SHA1(c806e63ce56a77f631043c184a42bf77ebda8a09) )
	ROM_LOAD ( "mu_mu-2_gal20v8.d21",        0x22e, 0x157, CRC(a715e392) SHA1(31ebc78b084d49cc2f6479cbd42738e6bfbfb46a) )
	ROM_LOAD ( "mu_mu-3_gal20v8as.h11",      0x385, 0x157, CRC(51e34bc2) SHA1(381a898b3afb709e7d8e0f87df106f23aec2ccbe) )
ROM_END

// Older main PCB, unknown I/O PCB
ROM_START( sltpcycld )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "bici_06.11_4f4e_0_d18_27c512.bin", 0x000000, 0x010000, CRC(62f25e57) SHA1(b49973bc8d59cf658371c9a5e0a7395a65a5539e) )
	ROM_LOAD16_BYTE( "bici_06.11_6ce7_1_d16_27c512.bin", 0x000001, 0x010000, CRC(b3d8327b) SHA1(bc992d93e31d1442be3170c4b27950459ff5bd23) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "bici_f3_l_j6_d1a2_27c010a.bin",  0x00000, 0x20000, CRC(8bc28070) SHA1(2338b077630b5982a044e2d042d94450f720dc54) )
	ROM_LOAD( "bici_f3_h_h7_f314_27c010a.u41",  0x20000, 0x20000, CRC(a206aa3f) SHA1(775b03f2a9a623190a5392ceadecd9fbf57cce80) )
	ROM_LOAD( "bici_f2_l_j7_326c_27c010a.bin",  0x40000, 0x20000, CRC(398db663) SHA1(30ffde3735500268dfe1b1524044ccbd16864709) )
	ROM_LOAD( "bici_f2_h_h7_3418_27c010a.bin",  0x60000, 0x20000, CRC(1cc875ec) SHA1(5fad36b4aaf148bc3e9ea11e171d3730a0e7deba) )
	ROM_LOAD( "bici_f1_l_j9_5c66_27c010a.bin",  0x80000, 0x20000, CRC(5de80a28) SHA1(86d20c53d21132cc802f9ae227ad32ec1da1c115) )
	ROM_LOAD( "bici_f1_h_h9_7e79_27c010a.bin",  0xa0000, 0x20000, CRC(c1ecab70) SHA1(7faefc09227b7449d27ad5899de134db761fbfb7) )
	ROM_LOAD( "bici_f0_l_j10_e71f_27c010a.bin", 0xc0000, 0x20000, CRC(ba239de5) SHA1(f5ed168d383488a393cdc71a5ec27723aa85d433) )
	ROM_LOAD( "bici_f0_h_h10_39da_27c010a.bin", 0xe0000, 0x20000, CRC(f8dd0e83) SHA1(8c8f6523ef40b8000bdceb67e6cc27a397e49bed) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END

// Older main PCB, unknown I/O PCB
ROM_START( sltpcycle )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "tachado_bici_22.10_4eea_d18_0_27c512.bin", 0x000000, 0x010000, CRC(d186773f) SHA1(2025597eaf2d4c461f3201be99a1d94be714c1e2) )
	ROM_LOAD16_BYTE( "tachado_bici_22.10_6c78_d16_1_27c512.bin", 0x000001, 0x010000, CRC(1afd8cec) SHA1(14aef060d2573bc54cf0efd3690f177af9475b27) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "bici_f3_l_j6_d1a2_27c010a.bin",  0x00000, 0x20000, CRC(8bc28070) SHA1(2338b077630b5982a044e2d042d94450f720dc54) )
	ROM_LOAD( "bici_f3_h_h7_f314_27c010a.u41",  0x20000, 0x20000, CRC(a206aa3f) SHA1(775b03f2a9a623190a5392ceadecd9fbf57cce80) )
	ROM_LOAD( "bici_f2_l_j7_326c_27c010a.bin",  0x40000, 0x20000, CRC(398db663) SHA1(30ffde3735500268dfe1b1524044ccbd16864709) )
	ROM_LOAD( "bici_f2_h_h7_3418_27c010a.bin",  0x60000, 0x20000, CRC(1cc875ec) SHA1(5fad36b4aaf148bc3e9ea11e171d3730a0e7deba) )
	ROM_LOAD( "bici_f1_l_j9_5c66_27c010a.bin",  0x80000, 0x20000, CRC(5de80a28) SHA1(86d20c53d21132cc802f9ae227ad32ec1da1c115) )
	ROM_LOAD( "bici_f1_h_h9_7e79_27c010a.bin",  0xa0000, 0x20000, CRC(c1ecab70) SHA1(7faefc09227b7449d27ad5899de134db761fbfb7) )
	ROM_LOAD( "bici_f0_l_j10_e71f_27c010a.bin", 0xc0000, 0x20000, CRC(ba239de5) SHA1(f5ed168d383488a393cdc71a5ec27723aa85d433) )
	ROM_LOAD( "bici_f0_h_h10_39da_27c010a.bin", 0xe0000, 0x20000, CRC(f8dd0e83) SHA1(8c8f6523ef40b8000bdceb67e6cc27a397e49bed) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END

// Older main PCB, unknown I/O PCB
ROM_START( sltpcyclf )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "tachado_bici_13.07_4caf_d18_0_27c512.bin", 0x000000, 0x010000, CRC(8aea93cb) SHA1(0352f9c1fe2a8cfa456c661605250afa384a9af9) )
	ROM_LOAD16_BYTE( "tachado_bici_13.07_6b79_d16_1_27c512.bin", 0x000001, 0x010000, CRC(7327f0f8) SHA1(032cfa43d7052bd0a224ebd13191123ab06c37b0) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "bici_f3_l_j6_d1a2_27c010a.bin",  0x00000, 0x20000, CRC(8bc28070) SHA1(2338b077630b5982a044e2d042d94450f720dc54) )
	ROM_LOAD( "bici_f3_h_h7_f314_27c010a.u41",  0x20000, 0x20000, CRC(a206aa3f) SHA1(775b03f2a9a623190a5392ceadecd9fbf57cce80) )
	ROM_LOAD( "bici_f2_l_j7_326c_27c010a.bin",  0x40000, 0x20000, CRC(398db663) SHA1(30ffde3735500268dfe1b1524044ccbd16864709) )
	ROM_LOAD( "bici_f2_h_h7_3418_27c010a.bin",  0x60000, 0x20000, CRC(1cc875ec) SHA1(5fad36b4aaf148bc3e9ea11e171d3730a0e7deba) )
	ROM_LOAD( "bici_f1_l_j9_5c66_27c010a.bin",  0x80000, 0x20000, CRC(5de80a28) SHA1(86d20c53d21132cc802f9ae227ad32ec1da1c115) )
	ROM_LOAD( "bici_f1_h_h9_7e79_27c010a.bin",  0xa0000, 0x20000, CRC(c1ecab70) SHA1(7faefc09227b7449d27ad5899de134db761fbfb7) )
	ROM_LOAD( "bici_f0_l_j10_e71f_27c010a.bin", 0xc0000, 0x20000, CRC(ba239de5) SHA1(f5ed168d383488a393cdc71a5ec27723aa85d433) )
	ROM_LOAD( "bici_f0_h_h10_39da_27c010a.bin", 0xe0000, 0x20000, CRC(f8dd0e83) SHA1(8c8f6523ef40b8000bdceb67e6cc27a397e49bed) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END


// Older main PCB, unknown I/O PCB
ROM_START( sltpstepb )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "step_0_14.06_18_ef72_27c512.bin", 0x000000, 0x010000, CRC(d91f9e0e) SHA1(7099fc468c5aab207baa9f7653805a8a08299ca6) )
	ROM_LOAD16_BYTE( "step_1_14.06_16_1c5a_27c512.bin", 0x000001, 0x010000, CRC(d22fc8e0) SHA1(e7602562434bb00fe1bab2451630f59ff293ecfe) )

	ROM_REGION( 0x0200000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "st_17-10_f3_h6_56ff8_27c040.bin", 0x0000000, 0x0080000, CRC(47d6926a) SHA1(1d939f33c3e646c9d1e36875ae8dfc30ba800c20) )
	ROM_LOAD( "st_17-10_f2_h7_021e_27c040.bin",  0x0080000, 0x0080000, CRC(c0100462) SHA1(453242183f35c30eb437d46ad6aa6f8124b64a71) )
	ROM_LOAD( "st_17-10_f1_h9_ddb8_27c040.bin",  0x0100000, 0x0080000, CRC(595173a3) SHA1(fa879a0167ff3ef25dafcc45bbedfe5a8eb50353) )
	ROM_LOAD( "st_17-10_f0_h10_234a_27c040.bin", 0x0180000, 0x0080000, CRC(082d0581) SHA1(5ddeb1ead89f9a287b58869fdfcfd2aff0cad9f2) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END

// Older main PCB, unknown I/O PCB
ROM_START( sltpstepc )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "step_10.06_0_18_27c512.bin", 0x000000, 0x010000, CRC(1c84579c) SHA1(defd2754e1ba86df703cff63d8f065ceae257ecd) )
	ROM_LOAD16_BYTE( "step_10.06_1_16_27c512.bin", 0x000001, 0x010000, CRC(ea160066) SHA1(d487c2b09bac849f7ebca5ec9a80a9eddbaf95bf) )

	ROM_REGION( 0x0200000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "st_17-10_f3_h6_56ff8_27c040.bin", 0x0000000, 0x0080000, CRC(47d6926a) SHA1(1d939f33c3e646c9d1e36875ae8dfc30ba800c20) )
	ROM_LOAD( "st_17-10_f2_h7_021e_27c040.bin",  0x0080000, 0x0080000, CRC(c0100462) SHA1(453242183f35c30eb437d46ad6aa6f8124b64a71) )
	ROM_LOAD( "st_17-10_f1_h9_ddb8_27c040.bin",  0x0100000, 0x0080000, CRC(595173a3) SHA1(fa879a0167ff3ef25dafcc45bbedfe5a8eb50353) )
	ROM_LOAD( "st_17-10_f0_h10_234a_27c040.bin", 0x0180000, 0x0080000, CRC(082d0581) SHA1(5ddeb1ead89f9a287b58869fdfcfd2aff0cad9f2) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END

// Older main PCB, unknown I/O PCB
ROM_START( sltpstepd )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )    // 68000 code
	ROM_LOAD16_BYTE( "step_0_2.07.96_18_d460_27c512.bin", 0x000000, 0x010000, CRC(ad8872c7) SHA1(1eb651417f15dabe27b5d0c3dd530c0229108c17) )
	ROM_LOAD16_BYTE( "step_1_2.07.96_16_0d94_27c512.bin", 0x000001, 0x010000, CRC(c5cb51a0) SHA1(4826bcdf3a8e0bb79b9005835ab76c7a2994029f) )

	ROM_REGION( 0x0200000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "st_17-10_f3_h6_56ff8_27c040.bin", 0x0000000, 0x0080000, CRC(47d6926a) SHA1(1d939f33c3e646c9d1e36875ae8dfc30ba800c20) )
	ROM_LOAD( "st_17-10_f2_h7_021e_27c040.bin",  0x0080000, 0x0080000, CRC(c0100462) SHA1(453242183f35c30eb437d46ad6aa6f8124b64a71) )
	ROM_LOAD( "st_17-10_f1_h9_ddb8_27c040.bin",  0x0100000, 0x0080000, CRC(595173a3) SHA1(fa879a0167ff3ef25dafcc45bbedfe5a8eb50353) )
	ROM_LOAD( "st_17-10_f0_h10_234a_27c040.bin", 0x0180000, 0x0080000, CRC(082d0581) SHA1(5ddeb1ead89f9a287b58869fdfcfd2aff0cad9f2) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// unclear if the PCB had a sound chip or not (if it was a conversion of another PCB it may have) but the machine doesn't attempt to use any sounds anyway
ROM_END

ROM_START( xorwflat )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "x_p1_27c010.bin", 0x000000, 0x020000, CRC(70a4d3cf) SHA1(3913567c2325dec53ababb8b7a3b99809a2650b2) )
	ROM_LOAD16_BYTE( "x_p2_27c010.bin", 0x000001, 0x020000, CRC(9ecbd93c) SHA1(14f4f26fc30e58e5a600d39ced94ac38baf71bbb) )

	ROM_REGION( 0x80000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "x_f4_27c010.bin", 0x000000, 0x020000, CRC(c71087b3) SHA1(447560f0ee510e46d8f92ec56f85a07cb0e6d533) )
	ROM_LOAD( "x_f3_27c010.bin", 0x020000, 0x020000, CRC(e1f9ad2f) SHA1(6598deba095afa48784726bd120d0b9ea6deec15) )
	ROM_LOAD( "x_f2_27c010.bin", 0x040000, 0x020000, CRC(5cae5ada) SHA1(276bdbd79a0e8f134dee5527a44d1261df658f1d) )
	ROM_LOAD( "x_f1_27c010.bin", 0x060000, 0x020000, CRC(3cd4101e) SHA1(e8ed0394afb2e682185fca6433409023dc41a9d6) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 ) // Z80 code
	ROM_LOAD( "x_s1_27c512.bin", 0x000000, 0x10000, CRC(46f66072) SHA1(f7eb20222f48f5e5dc1b5c5fb609105a3e1c79c1) )

	// no samples?
ROM_END

ROM_START( xorwflata )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "x_0_14-11_27c1001.bin", 0x000000, 0x020000, CRC(f3fad238) SHA1(10b76bce8e8114ecf3ba4705eb064fe91fcb654f) )
	ROM_LOAD16_BYTE( "x_1_14-11_27c1001.bin", 0x000001, 0x020000, CRC(977f1225) SHA1(a84e7a5873e4f07fdefb2d2ba716fd636c2c89d6) )

	ROM_REGION( 0x80000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "x_f4_27c010.bin", 0x000000, 0x020000, CRC(c71087b3) SHA1(447560f0ee510e46d8f92ec56f85a07cb0e6d533) )
	ROM_LOAD( "x_f3_27c010.bin", 0x020000, 0x020000, CRC(e1f9ad2f) SHA1(6598deba095afa48784726bd120d0b9ea6deec15) )
	ROM_LOAD( "x_f2_27c010.bin", 0x040000, 0x020000, CRC(5cae5ada) SHA1(276bdbd79a0e8f134dee5527a44d1261df658f1d) )
	ROM_LOAD( "x_f1_27c010.bin", 0x060000, 0x020000, CRC(3cd4101e) SHA1(e8ed0394afb2e682185fca6433409023dc41a9d6) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 ) // Z80 code
	ROM_LOAD( "x_s1_27c512.bin", 0x000000, 0x10000, CRC(46f66072) SHA1(f7eb20222f48f5e5dc1b5c5fb609105a3e1c79c1) )

	// no samples?
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// Years and regions on comments came from Gaelco stickers on program ROMs

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    CLASS,          INIT,       ROT,  COMPANY,         FULLNAME

GAME( 1991, bigkarnk,  0,        bigkarnk, bigkarnk, bigkarnk_state, empty_init, ROT0, "Gaelco",        "Big Karnak (ver. 1.0, checksum 1e38c94)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1991, bigkarnka, bigkarnk, bigkarnk, bigkarnk, bigkarnk_state, empty_init, ROT0, "Gaelco",        "Big Karnak (ver. 1.0, checksum 1e38b94)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoy,   0,        maniacsq, biomtoy,  gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Biomechanical Toy (ver. 1.0.1885, checksum 69f5e032)",               MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoya,  biomtoy,  maniacsq, biomtoy,  gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Biomechanical Toy (ver. 1.0.1884, checksum 3f316c70)",               MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoyb,  biomtoy,  maniacsq, biomtoy,  gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Biomechanical Toy (ver. 1.0.1878, checksum d84b28ff)",               MACHINE_SUPPORTS_SAVE )
GAME( 1994, biomtoyc,  biomtoy,  maniacsq, biomtoyc, gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Biomechanical Toy (ver. 1.0.1870, checksum ba682195)",               MACHINE_SUPPORTS_SAVE )
GAME( 1994, bioplayc,  biomtoy,  maniacsq, bioplayc, gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Bioplaything Cop (ver. 1.0.1823, checksum cd960fc9, prototype)",     MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // copyright based on Ver. 1.0.1870
GAME( 1992, maniacsp,  0,        maniacsq, maniacsq, gaelco_state,   empty_init, ROT0, "Gaelco",        "Maniac Square (ver 1.0, checksum b602, prototype)",                  MACHINE_SUPPORTS_SAVE ) // The prototype version was an earlier project, said to be from 1992, game was rewritten in 1996
GAME( 1995, lastkm,    0,        maniacsq, lastkm,   gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Last KM (ver 1.0.0275, checksum 13bff751, prototype)",               MACHINE_SUPPORTS_SAVE ) // Similar 'bike controller' idea to the Salter gym equipment Gaelco developed, but in game form
GAME( 1995, lastkma,   lastkm,   maniacsq, lastkm,   gaelco_state,   empty_init, ROT0, "Gaelco / Zeus", "Last KM (ver 1.0.0227, checksum 747a7443, prototype)",               MACHINE_SUPPORTS_SAVE )
GAME( 1992, squash,    0,        squash,   squash,   squash_state,   empty_init, ROT0, "Gaelco",        "Squash (World, ver. 1.0, checksum 015aef61)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1992, squasha,   squash,   squash,   squash,   squash_state,   empty_init, ROT0, "Gaelco",        "Squash (USA, ver. 1.1, checksum 015b6f8a)",                          MACHINE_SUPPORTS_SAVE )
GAME( 199?, squashb,   squash,   squash,   squash,   squash_state,   empty_init, ROT0, "Gaelco",        "Squash (newer PCB)",                                                 MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // Missing one program ROM
GAME( 1992, thoop,     0,        thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (ver. 1, checksum 02a09f7d)",                           MACHINE_SUPPORTS_SAVE ) // 24/Aug/1992, Europe
GAME( 1992, thoopa,    thoop,    thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (ver. 1, checksum 02a09fcd)",                           MACHINE_SUPPORTS_SAVE ) // 22/Jul/1992, Europe
GAME( 1992, thoopb,    thoop,    thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (ver. X, checksum 00000020, without title)",            MACHINE_SUPPORTS_SAVE ) // 02/Jul/1992
GAME( 1992, thoopna,   thoop,    thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (North America, ver. C4, checksum 02A0A008)",           MACHINE_SUPPORTS_SAVE ) // 24/Aug/1992, America, test
GAME( 1992, thoopnna,  thoop,    thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (non North America, ver. X, checksum 00000020, set 1)", MACHINE_SUPPORTS_SAVE ) // 09/Jun/1992
GAME( 1992, thoopnnaa, thoop,    thoop,    thoop,    thoop_state,    empty_init, ROT0, "Gaelco",        "Thunder Hoop (non North America, ver. X, checksum 00000020, set 2)", MACHINE_SUPPORTS_SAVE ) // Non North America but with FBI screen ??

GAME( 199?, sltpcycld, sltpcycl, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Cycle Tele Cardioline (Salter fitness bike, older hardware, ver. 1.0, checksum BAE7)",      MACHINE_NOT_WORKING )
GAME( 199?, sltpcycle, sltpcycl, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Cycle Tele Cardioline (Salter fitness bike, older hardware, ver. 1.0, checksum 5678)",      MACHINE_NOT_WORKING )
GAME( 199?, sltpcyclf, sltpcycl, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Cycle Tele Cardioline (Salter fitness bike, older hardware, ver. 1.0, checksum 1AF9)",      MACHINE_NOT_WORKING )

GAME( 199?, sltpstepb, sltpstep, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Stepper Tele Cardioline (Salter fitness stepper, older hardware, ver. 1.0, checksum 8E5A)", MACHINE_NOT_WORKING )
GAME( 199?, sltpstepc, sltpstep, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Stepper Tele Cardioline (Salter fitness stepper, older hardware, ver. 1.0, checksum 8BF3)", MACHINE_NOT_WORKING )
GAME( 1996, sltpstepd, sltpstep, maniacsq, sltpcycld, gaelco_state,  empty_init, ROT0, "Salter Fitness / Gaelco", "Pro Stepper Tele Cardioline (Salter fitness stepper, older hardware, ver. 1.0, checksum 6D94)", MACHINE_NOT_WORKING ) // 2/Jul/1996  .

/* Not 100% sure it belongs here, but fairly close. Boots at least but video regs seem incorrect, and sound hardware is different.
   Set was being marked as from a 'flat' PCB, which is how the gaelco.cpp family of boards is referred to.
   Intentionally not set as a clone due to it being a significantly different codebase / hardware type. */
GAME( 1990, xorwflat,  0,        xorwflat, xorwflat, xorwflat_state, empty_init, ROT0, "Gaelco", "Xor World (different hardware, ver 1.1, checksum 3333BA, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1990, xorwflata, xorwflat, xorwflat, xorwflat, xorwflat_state, empty_init, ROT0, "Gaelco", "Xor World (different hardware, ver 1.1, checksum 333462, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 14/Nov/1990
