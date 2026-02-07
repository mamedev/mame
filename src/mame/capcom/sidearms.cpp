// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Curt Coder
/*******************************************************************************

Sidearms
Driver provided by Paul Leaman

Notes:

The main board of Side Arms has an unpopulated position reserved for a 8751
protection MCU.

Unknown PROMs are mostly used for timing. Only the first four sprite encoding
parameters have been identified, the other 28(!) are believed to be line-buffer
controls.

A bootleg has been found that matches "sidearmsj" but with the starfield data
ROM being half the size of the original one and containing its second half.
Also, it seems that, as the original game it's currently emulated, it uses just
the first half of the starfield ROM, so it's something worth checking.

TODO:
- writing to the palette outside of the allowed time specified in the 16H PROM
  will trigger a write error on SYSTEM port bit 2 (cleared by writing to 0xc803)

********************************************************************************

Turtle Ship (Philko / Pacific Games license (Japan region), 1988)
Hardware info by Guru

Philko's hardware is very bootleg-like and most probably
made in the same factory that made other bootlegs.
The sound circuit looks very close to the sound circuit
on Sidearms and 1943. You could say the entire Turtle
Ship board is a bootleg of Side Arms with different
graphics. They disguised the board by shuffling parts
around and putting the video board on top to make it
look like different hardware. However a dump of the
bi-polar PROMs reveals they are the same as Side Arms
which makes this just another bootleg ;-)

PCB Layout
----------

CPU Board

20-40-2 PHILKO
|---------------------------------------------------|
|LA4460 VOL YM3014     YM2203    T-4.8A    LC3517(1)|
|  VOL LM324   YM3014  YM2203                       |
|                                Z80A               |
|ULN2003                                       16MHz|
|     Z80B   T-1.3E    HY6264           82S123.9E   |
|            T-2.3G    T-3.5G                       |
|                                                   |
|J                      HY6264                      |
|A           LC3517(2)                              |
|M                               T-5.8K             |
|M           LC3517(2)                              |
|A                                                  |
|  DIP-SW1                                          |
|  DIP-SW2         PAL16L8                82S129.11P|
|                                         82S129.11R|
|--------------------|---------|-----|---------|----|
                     |---------|     |---------|
Notes:
        Z80A - ZILOG Z80A CPU. Clock 4.000MHz [16/4] (sound CPU)
        Z80B - ZILOG Z80B CPU. Clock 8.000MHz [16/2] (main CPU)
      YM2203 - Yamaha YM2203 FM Operator Type-N(OPN) sound chip. Clock 4.000MHz [16/4]
      YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.3333MHz [16/4/3]
   HY6264(1) - Hyundai HY6264 8kBx8-bit SRAM (main program RAM)
   HY6264(2) - Hyundai HY6264 8kBx8-bit SRAM (character/text layer RAM)
   LC3517(1) - Sanyo LC3517 2kBx8-bit SRAM (sound CPU RAM)
   LC3517(2) - Sanyo LC3517 2kBx8-bit SRAM (color RAM)
      LA4460 - Sanyo LA4460 12W AF Audio Power Amplifier
       LM324 - Texas Instruments LM324 Quad Operational Amplifier
       SW1/2 - 8-position DIP switch labelled on the board as 'DIP-SW1' and 'DIP-SW2'
       HSync - 15.6246kHz. Measured on horizontal timing PROM at 11R
       VSync - 61.0338Hz. Measured on vertical timing PROM at 11P
      T-1.3E \
      T-2.3G | 27256 OTP EPROM (main program)
      T-3.5G /
      T-4.8A - 27256 OTP EPROM (sound program)
      T-5.8K - 27256 OTP EPROM (characters / text layer). A14 is tied high on the PCB
               effectively making this chip a 27128
   82S123.9E - Signetics 82S123 32x8-bit Bi-Polar PROM (sound CPU-related)
               The address lines are tied to the sound CPU and when removed there is no sound.
  82S129.11P - Signetics 82S129 256x4-bit Bi-Polar PROM (vertical timing PROM)
  82S129.11R - Signetics 82S129 256x4-bit Bi-Polar PROM (horizontal timing PROM)


Top Board

20-41-1
|-----------------------------------|
|T-6.1A T-8.1D       T-12.1G T-13.1I|
|  T-7.1B            T-14.3G T-15.3I|
|T-9.3A T-11.3D                     |
|  T-10.3B                          |
|                                   |
|                                   |
|                  T-16.9F          |
|                         2018 2018 |
|                                   |
|                                   |
|                                   |
|                |------|           |
|    LC3517      |PK8808|           |
|    LC3517      |      |           |
|    LC3517      |------|           |
|----|---------|-----|---------|----|
     |---------|     |---------|
Notes:
      PK8808 - Altera EP1800LC Erasable Programmable Logic Device (EPLD in PLCC68 package) acting as the sprite generator chip.
               This is clearly a clone of Capcom's custom 86S105 sprite chip but without the internal RAM.
      LC3517 - Sanyo LC3517 2kBx8-bit SRAM (sprite RAM for the PK8808 i.e. the internal RAM on the 86S105)
        2018 - Toshiba TMM2018 2kBx8-bit SRAM (sprite RAM)
T-12 to T-15 - 27512 OTP EPROM (sprites)
   T6 to T11 - 27512 OTP EPROM (tiles)
         T16 - 27256 OTP EPROM (background tiles)

*******************************************************************************/

#include "emu.h"
#include "sidearms.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "speaker.h"

void sidearms_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x8000, 0x4000);
}

void sidearms_state::bankswitch_w(uint8_t data)
{
	membank("bank1")->set_entry(data & 0x07);
}

void sidearms_state::whizz_bankswitch_w(uint8_t data)
{
	membank("bank1")->set_entry(bitswap<2>(data,6,7));
}


TIMER_DEVICE_CALLBACK_MEMBER(sidearms_state::scanline)
{
	const int scanline = param;

	// 2 interrupts per frame, every 128 scanlines
	if (scanline == 112 || scanline == 240)
		m_maincpu->set_input_line(0, HOLD_LINE);
}


uint8_t sidearms_state::turtship_ports_r(offs_t offset)
{
	int res = 0;

	// Turtle Ship input ports are rotated 90 degrees
	for (int i = 0; i < 5; i++)
		res |= ((m_ports[i].read_safe(0) >> offset) & 1) << i;

	return res;
}


void sidearms_state::sidearms_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc3ff).writeonly().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc400, 0xc7ff).writeonly().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xc800, 0xc800).portr("SYSTEM").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc801, 0xc801).portr("P1").w(FUNC(sidearms_state::bankswitch_w));
	map(0xc802, 0xc802).portr("P2").w(m_spriteram, FUNC(buffered_spriteram8_device::write)); // 86S105 DMA transfer request
	map(0xc803, 0xc803).portr("DSW0");
	map(0xc804, 0xc804).portr("DSW1").w(FUNC(sidearms_state::control_w));
	map(0xc805, 0xc805).portr("DSW2").w(FUNC(sidearms_state::star_scrollx_w));
	map(0xc806, 0xc806).w(FUNC(sidearms_state::star_scrolly_w));
	map(0xc808, 0xc809).writeonly().share("bg_scrollx");
	map(0xc80a, 0xc80b).writeonly().share("bg_scrolly");
	map(0xc80c, 0xc80c).w(FUNC(sidearms_state::gfxctrl_w)); // background and sprite enable
	map(0xd000, 0xd7ff).ram().w(FUNC(sidearms_state::videoram_w)).share("videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(sidearms_state::colorram_w)).share("colorram");
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share("spriteram");
}

void sidearms_state::turtship_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xdfff).ram().share("spriteram");
	map(0xe000, 0xe3ff).writeonly().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe400, 0xe7ff).writeonly().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xe800, 0xe807).r(FUNC(sidearms_state::turtship_ports_r));
	map(0xe800, 0xe800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xe801, 0xe801).w(FUNC(sidearms_state::bankswitch_w));
	map(0xe802, 0xe802).w(m_spriteram, FUNC(buffered_spriteram8_device::write)); // 86S105 DMA transfer request
	map(0xe804, 0xe804).w(FUNC(sidearms_state::control_w));
	map(0xe805, 0xe805).w(FUNC(sidearms_state::star_scrollx_w));
	map(0xe806, 0xe806).w(FUNC(sidearms_state::star_scrolly_w));
	map(0xe808, 0xe809).writeonly().share("bg_scrollx");
	map(0xe80a, 0xe80b).writeonly().share("bg_scrolly");
	map(0xe80c, 0xe80c).w(FUNC(sidearms_state::gfxctrl_w)); // background and sprite enable
	map(0xf000, 0xf7ff).ram().w(FUNC(sidearms_state::videoram_w)).share("videoram");
	map(0xf800, 0xffff).ram().w(FUNC(sidearms_state::colorram_w)).share("colorram");
}

void sidearms_state::sidearms_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xf000, 0xf001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf002, 0xf003).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

/* Whizz */

void sidearms_state::whizz_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc3ff).writeonly().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc400, 0xc7ff).writeonly().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xc800, 0xc800).portr("DSW0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc801, 0xc801).portr("DSW1").w(FUNC(sidearms_state::whizz_bankswitch_w));
	map(0xc802, 0xc802).portr("DSW2").w(m_spriteram, FUNC(buffered_spriteram8_device::write)); // 86S105 DMA transfer request
	map(0xc803, 0xc803).portr("IN0").nopw();
	map(0xc804, 0xc804).portr("IN1").w(FUNC(sidearms_state::control_w));
	map(0xc805, 0xc805).portr("IN2").nopw();
	map(0xc806, 0xc806).portr("IN3");
	map(0xc807, 0xc807).portr("IN4");
	map(0xc808, 0xc809).writeonly().share("bg_scrollx");
	map(0xc80a, 0xc80b).writeonly().share("bg_scrolly");
	map(0xc80c, 0xc80c).w(FUNC(sidearms_state::gfxctrl_w));
	map(0xd000, 0xd7ff).ram().w(FUNC(sidearms_state::videoram_w)).share("videoram");
	map(0xd800, 0xdfff).ram().w(FUNC(sidearms_state::colorram_w)).share("colorram");
	map(0xe000, 0xefff).ram();
	map(0xe805, 0xe805).w(FUNC(sidearms_state::star_scrollx_w));
	map(0xe806, 0xe806).w(FUNC(sidearms_state::star_scrolly_w));
	map(0xf000, 0xffff).ram().share("spriteram");
}

void sidearms_state::whizz_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).ram();
}

void sidearms_state::whizz_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x40, 0x40).nopw();
	map(0xc0, 0xc0).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( sidearms )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3 (Normal)" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "100000" )
	PORT_DIPSETTING(    0x20, "100000 100000" )
	PORT_DIPSETTING(    0x10, "150000 150000" )
	PORT_DIPSETTING(    0x00, "200000 200000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( turtship )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xe0, 0xa0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(   0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_DIPSETTING(    0x0c, "150000 only" )
	PORT_DIPSETTING(    0x04, "200000 only" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	/* 0xc0 1 Coin/1 Credit */
INPUT_PORTS_END

static INPUT_PORTS_START( dyger )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* seems to be 1-player only */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* seems to be 1-player only */

	PORT_START("DSW0")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xe0, 0xa0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(   0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x04, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_DIPSETTING(    0x0c, "150000 only" )
	PORT_DIPSETTING(    0x08, "200000 only" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	/* 0xc0 1 Coin/1 Credit */
INPUT_PORTS_END

static INPUT_PORTS_START( whizz )
	PORT_START("DSW0")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3 (Normal)" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x18, "100000 Only" )
	PORT_DIPSETTING(    0x10, "Every 100000" )
	PORT_DIPSETTING(    0x08, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 200000" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 8-bit */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* 8-bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* 8-bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	32,32,  /* 32*32 tiles */
	RGN_FRAC(1,2),    /* 512 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0,       1,       2,       3,       8+0,       8+1,       8+2,       8+3,
		32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+8+0, 32*16+8+1, 32*16+8+2, 32*16+8+3,
		64*16+0, 64*16+1, 64*16+2, 64*16+3, 64*16+8+0, 64*16+8+1, 64*16+8+2, 64*16+8+3,
		96*16+0, 96*16+1, 96*16+2, 96*16+3, 96*16+8+0, 96*16+8+1, 96*16+8+2, 96*16+8+3,
	},
	{
		0*16,  1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16,  9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8   /* every tile takes 256 consecutive bytes */
};

static GFXDECODE_START( gfx_sidearms )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   768, 64 ) /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0, 32 ) /* colors   0-511 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 ) /* colors 512-767 */
GFXDECODE_END



static const gfx_layout turtship_tilelayout =
{
	32,32,  /* 32*32 tiles */
	RGN_FRAC(1,2),    /* 768 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0,       1,       2,       3,       8+0,       8+1,       8+2,       8+3,
		32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+8+0, 32*16+8+1, 32*16+8+2, 32*16+8+3,
		64*16+0, 64*16+1, 64*16+2, 64*16+3, 64*16+8+0, 64*16+8+1, 64*16+8+2, 64*16+8+3,
		96*16+0, 96*16+1, 96*16+2, 96*16+3, 96*16+8+0, 96*16+8+1, 96*16+8+2, 96*16+8+3,
	},
	{
		0*16,  1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16,  9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8   /* every tile takes 256 consecutive bytes */
};

static GFXDECODE_START( gfx_turtship )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          768, 64 )  /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, turtship_tilelayout,   0, 32 )  /* colors   0-511 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,        512, 16 )  /* colors 512-767 */
GFXDECODE_END


void sidearms_state::sidearms(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sidearms_state::sidearms_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sidearms_state::sidearms_sound_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(sidearms_state::scanline), "screen", 112, 128);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL / 2, 64*8, 8*8, (64-8)*8, 32*8, 2*8, 30*8);
	m_screen->set_screen_update(FUNC(sidearms_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sidearms);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 16_MHz_XTAL / 4));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.25);

	ym2203_device &ym2(YM2203(config, "ym2", 16_MHz_XTAL / 4));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.25);
}

void sidearms_state::turtship(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sidearms_state::turtship_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sidearms_state::sidearms_sound_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(sidearms_state::scanline), "screen", 112, 128);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL / 2, 64*8, 8*8, (64-8)*8, 32*8, 2*8, 30*8); // 61.0338 Hz measured
	m_screen->set_screen_update(FUNC(sidearms_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turtship);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 16_MHz_XTAL / 4));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.25);

	ym2203_device &ym2(YM2203(config, "ym2", 16_MHz_XTAL / 4));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.25);
}

void sidearms_state::whizz(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sidearms_state::whizz_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sidearms_state::whizz_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &sidearms_state::whizz_io_map);

	config.set_maximum_quantum(attotime::from_hz(60000));

	TIMER(config, "scantimer").configure_scanline(FUNC(sidearms_state::scanline), "screen", 112, 128);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL / 2, 64*8, 8*8, (64-8)*8, 32*8, 2*8, 30*8);
	m_screen->set_screen_update(FUNC(sidearms_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_turtship);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.add_route(0, "mono", 1.0);
	ymsnd.add_route(1, "mono", 1.0);
}



ROM_START( sidearms )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "sa03.bin",     0x00000, 0x08000, CRC(e10fe6a0) SHA1(ae59461768d044f14b9aac3e4e491c76cec7adac) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsu )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "saa_03.15e",   0x00000, 0x08000, CRC(32ef2739) SHA1(15e0535a6e3508c0d1ed73157a052c3716571000) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsur1 )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "03",           0x00000, 0x08000, CRC(9a799c45) SHA1(cf6836108506929ee2449546a4867a7cbf00bcc8) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( sidearmsj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "a_15e.rom",    0x00000, 0x08000, CRC(61ceb0cc) SHA1(bacf28e5e02b90a9d404c3ade0267e0a7cd73cd8) )        /* CODE */
	ROM_LOAD( "a_14e.rom",    0x08000, 0x08000, CRC(4925ed03) SHA1(b11dbd9889db89cff008ca21beb6b1b70d983e16) )        /* 0+1 */
	ROM_LOAD( "a_12e.rom",    0x10000, 0x08000, CRC(81d0ece7) SHA1(5c1d154f9c1de6b5f5d7abf5d413e9c493461e6f) )        /* 2+3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a_04k.rom",    0x0000, 0x8000, CRC(34efe2d2) SHA1(e1d8895c113e4dee1a132e2471d75dfa6c36b620) )

	ROM_REGION( 0x08000, "user1", 0 )    /* starfield data */
	ROM_LOAD( "b_11j.rom",    0x0000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a_10j.rom",    0x00000, 0x4000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b_13d.rom",    0x00000, 0x8000, CRC(3c59afe1) SHA1(5459a5795cf13012674993aa55bbd39e9a5c2f1b) ) /* tiles */
	ROM_LOAD( "b_13e.rom",    0x08000, 0x8000, CRC(64bc3b77) SHA1(54fe6f258fda509a92eb0f5aa238102efce729e0) )
	ROM_LOAD( "b_13f.rom",    0x10000, 0x8000, CRC(e6bcea6f) SHA1(19477e284967beafc4e7cd0d0da3534eb6dec388) )
	ROM_LOAD( "b_13g.rom",    0x18000, 0x8000, CRC(c71a3053) SHA1(963e105aa0b0174e8aa5e1f7676c5c604ca72d1c) )
	ROM_LOAD( "b_14d.rom",    0x20000, 0x8000, CRC(826e8a97) SHA1(ad5ed9a81805dde54fb2703345b2ab7b56853ec6) )
	ROM_LOAD( "b_14e.rom",    0x28000, 0x8000, CRC(6cfc02a4) SHA1(491e880e85d5256fa2eea6d0fb402f0a1176b675) )
	ROM_LOAD( "b_14f.rom",    0x30000, 0x8000, CRC(9b9f6730) SHA1(0f8fe5dc32ee50ebb2051c0c0c4d635582416317) )
	ROM_LOAD( "b_14g.rom",    0x38000, 0x8000, CRC(ef6af630) SHA1(499b17eeb5e7256ede477510b0547df520316996) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "b_11b.rom",    0x00000, 0x8000, CRC(eb6f278c) SHA1(15e250aa98ee69ac3983d4511976c35833b37cab) ) /* sprites */
	ROM_LOAD( "b_13b.rom",    0x08000, 0x8000, CRC(e91b4014) SHA1(6557344ce8bc05309ab8ebe846871ed554b256b8) )
	ROM_LOAD( "b_11a.rom",    0x10000, 0x8000, CRC(2822c522) SHA1(00b3cab899e5ac1af6300f2ec2a54303df9ab014) )
	ROM_LOAD( "b_13a.rom",    0x18000, 0x8000, CRC(3e8a9f75) SHA1(b1bfb7604791950aa0454b68b24f6ad3b9131be8) )
	ROM_LOAD( "b_12b.rom",    0x20000, 0x8000, CRC(86e43eda) SHA1(c33b0ab6f7f0f886410a3943988b737d175635be) )
	ROM_LOAD( "b_14b.rom",    0x28000, 0x8000, CRC(076e92d1) SHA1(27144834b5b2849be8c46e97aaaeaa8b304ea810) )
	ROM_LOAD( "b_12a.rom",    0x30000, 0x8000, CRC(ce107f3c) SHA1(2235281449247cb2446b008b36077788c5b15026) )
	ROM_LOAD( "b_14a.rom",    0x38000, 0x8000, CRC(dba06076) SHA1(87b3b3437bc4bd727ce7e34dd914e6fe23bcac3d) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "b_03d.rom",    0x0000, 0x8000, CRC(6f348008) SHA1(b500bc32ba47e9cc9dcf2254b9455ac4d61992db) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "63s141.16h",   0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) )    // timing
	ROM_LOAD( "63s141.11h",   0x0100, 0x0100, CRC(a6e4d68f) SHA1(b9367e0c959cdf0397d33a49d778a66a407572b7) )    // color mixing
	ROM_LOAD( "63s141.15h",   0x0200, 0x0100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) )    // timing
	ROM_LOAD( "63s081.3j",    0x0300, 0x0020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) )    // unknown
ROM_END

ROM_START( turtship )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.5g",   0x00000, 0x08000, CRC(b73ed7f2) SHA1(bb98fe41b989d6568fe8cf1900a0d15c176b61a0) )
	ROM_LOAD( "t-2.3g",   0x08000, 0x08000, CRC(2327b35a) SHA1(bf7b5e11c3f75aff7d09c0fc4ad61fb4bcb38100) )
	ROM_LOAD( "t-1.3e",   0x10000, 0x08000, CRC(a258ffec) SHA1(caa689607ebe450a68736933dbfaf6bf9b6d3487) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",    0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.8k",    0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
	ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.1d",    0x00000, 0x10000, CRC(30a857f0) SHA1(a2d261e8104d0459067bdbdd71662fe8d6917da1) ) /* tiles */
	ROM_LOAD( "t-10.3c",   0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,   0x10000)
	ROM_LOAD( "t-11.3d",   0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.1a",    0x40000, 0x10000, CRC(45ce41ad) SHA1(6e2f559adc4aee80326b3ae5ae6c6688a3491962) )
	ROM_LOAD( "t-7.1c",    0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,   0x10000)
	ROM_LOAD( "t-9.3a",    0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",  0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "t-15.3i",  0x10000, 0x10000, CRC(6489b7b4) SHA1(438d088db131f5bb4ef2124eee814b25c92115e3) )
	ROM_LOAD( "t-12.1g",  0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "t-14.3g",  0x30000, 0x10000, CRC(1b67b674) SHA1(a77ef1b4ba4d544aa230acf779f9c339d0fc55db) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.9f",   0x00000, 0x08000, CRC(1a5a45d7) SHA1(51ceeae938fbda207c3f8ce65593d271dc8c4a41) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "82s129.11p", 0x000, 0x100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) // vertical timing
	ROM_LOAD( "82s129.11r", 0x100, 0x100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) ) // horizontal timing
	ROM_LOAD( "82s123.9e",  0x200, 0x020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) ) // tied to the sound Z80 and without it there's no sound on the real PCB
ROM_END

ROM_START( turtshipj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.5g",    0x00000, 0x08000, CRC(0863fc1c) SHA1(b583e06e05e466c2344a4a420a47227c9ab8705c) )
	ROM_LOAD( "t-2.3g",    0x08000, 0x08000, CRC(2327b35a) SHA1(bf7b5e11c3f75aff7d09c0fc4ad61fb4bcb38100) )
	ROM_LOAD( "t-1.3e",    0x10000, 0x08000, CRC(845a9ab0) SHA1(f1455aeca92d129c7ed145d76e5093f41ce62ccb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",    0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.8k",    0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
	ROM_CONTINUE(          0x00000, 0x04000 )   /* A14 tied high, only upper half is used */


	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.1d",    0x00000, 0x10000, CRC(30a857f0) SHA1(a2d261e8104d0459067bdbdd71662fe8d6917da1) ) /* tiles */
	ROM_LOAD( "t-10.3c",   0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,   0x10000)
	ROM_LOAD( "t-11.3d",   0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.1a",    0x40000, 0x10000, CRC(45ce41ad) SHA1(6e2f559adc4aee80326b3ae5ae6c6688a3491962) )
	ROM_LOAD( "t-7.1c",    0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,   0x10000)
	ROM_LOAD( "t-9.3a",    0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",   0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "t-15.3i",   0x10000, 0x10000, CRC(f30cfa90) SHA1(0e4ecea069df6a6bb6ec03eff51c0f37e7531aa8) )
	ROM_LOAD( "t-12.1g",   0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "t-14.3g",   0x30000, 0x10000, CRC(d636873c) SHA1(6edf01d0bd6d085eda491c600b1f4b4cbede5a74) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.9f",   0x00000, 0x08000, CRC(1a5a45d7) SHA1(51ceeae938fbda207c3f8ce65593d271dc8c4a41) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "82s129.11p", 0x000, 0x100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) // vertical timing
	ROM_LOAD( "82s129.11r", 0x100, 0x100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) ) // horizontal timing
	ROM_LOAD( "82s123.9e",  0x200, 0x020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) ) // tied to the sound Z80 and without it there's no sound on the real PCB
ROM_END

ROM_START( turtshipk )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "turtship.003",  0x00000, 0x08000, CRC(e7a7fc2e) SHA1(1a9147e82a5e56e8e5b68bbce144f96261e88669) )
	ROM_LOAD( "turtship.002",  0x08000, 0x08000, CRC(e576f482) SHA1(3be3792cb437bff0345681a3a2fdefefa3439357) )
	ROM_LOAD( "turtship.001",  0x10000, 0x08000, CRC(a9b64240) SHA1(38c59877de6055230c3250ef74abc97e4ed88cb6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.8a",        0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 ) /* Really a 27128? */
	ROM_LOAD( "turtship.005",  0x00000, 0x04000, CRC(651fef75) SHA1(9c821a2ee30c222987f0d4192133776490d6a4e0) ) /* characters */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "turtship.008",  0x00000, 0x10000, CRC(e0658469) SHA1(931c41cd6af759b30f6018248c3bab4d544acb98) ) /* tiles */
	ROM_LOAD( "t-10.3c",       0x10000, 0x10000, CRC(76bb73bb) SHA1(4c4acd205421674878948a0d2bed6032bde3f97f) )
	ROM_RELOAD( 0x30000,       0x10000)
	ROM_LOAD( "t-11.3d",       0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "turtship.006",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
	ROM_LOAD( "t-7.1c",        0x50000, 0x10000, CRC(3ccf11b9) SHA1(777cc853bfcf2db4027b35d516fa5bef8b010e63) )
	ROM_RELOAD( 0x70000,       0x10000)
	ROM_LOAD( "t-9.3a",        0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.1i",       0x00000, 0x10000, CRC(599f5246) SHA1(b7e5bbff3b6117613744970c8680b7bc171516bd) ) /* sprites */
	ROM_LOAD( "turtship.015",  0x10000, 0x10000, CRC(69fd202f) SHA1(67d7d6d08f5daa0460ce51516f1d27dfd6aef297) )
	ROM_LOAD( "t-12.1g",       0x20000, 0x10000, CRC(fb54cd33) SHA1(49f7b728a4de8b93f5fd929f59a65509e4556161) )
	ROM_LOAD( "turtship.014",  0x30000, 0x10000, CRC(b3ea74a3) SHA1(aa347a6cd75408a3ba4ce26d3e1015a1be1faa64) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "turtship.016",  0x00000, 0x08000, CRC(affd51dd) SHA1(3338aa1fdd6b9926acc215f7f3656d70803f1832) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "82s129.11p", 0x000, 0x100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) // vertical timing
	ROM_LOAD( "82s129.11r", 0x100, 0x100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) ) // horizontal timing
	ROM_LOAD( "82s123.9e",  0x200, 0x020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) ) // tied to the sound Z80 and without it there's no sound on the real PCB
ROM_END

ROM_START( turtshipko )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.g5",  0x00000, 0x08000, CRC(cd789535) SHA1(3c4f94c751645b61066177fbf3157924ad177c32) )
	ROM_LOAD( "t-2.g3",  0x08000, 0x08000, CRC(253678c0) SHA1(1470fd936003462d480c759658628ea085d4bd71) )
	ROM_LOAD( "t-1.e3",  0x10000, 0x08000, CRC(d6fdc376) SHA1(3f4e1fde8b83e3762f9499dfe291309efe940093) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.a8",  0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.k8",  0x00000, 0x04000, CRC(35c3dbc5) SHA1(6700c72e5e0f7bd1429d342cb5d3daccd6b1b70f) ) /* characters */
	ROM_CONTINUE(        0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.d1",  0x00000, 0x10000, CRC(2f0b2336) SHA1(a869e0a50aab7d29afbca46fa04bd470488a8eeb) ) /* tiles */
	ROM_LOAD( "t-10.c3", 0x10000, 0x10000, CRC(6a0072f4) SHA1(d74b53ed90a4d01020a179f263a39b7547b8f82e) )
	ROM_RELOAD(          0x30000, 0x10000)
	ROM_LOAD( "t-11.d3", 0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.a1",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
	ROM_LOAD( "t-7.c1",  0x50000, 0x10000, CRC(90dd8415) SHA1(8e9d43ff9164fb287ab82df7da8890976b9d21c7) )
	ROM_RELOAD(          0x70000, 0x10000)
	ROM_LOAD( "t-9.a3",  0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.i1", 0x00000, 0x10000, CRC(1cc87f50) SHA1(d7d8a4376b556675dafa0a407bb34b6017f17e7d) ) /* sprites */
	ROM_LOAD( "t-15.i3", 0x10000, 0x10000, CRC(775ee5d9) SHA1(e39eb558cc2d5cdf4c87b96f85af72e5600b995e) )
	ROM_LOAD( "t-12.g1", 0x20000, 0x10000, CRC(57783312) SHA1(57942e8c3b7be63ea62bae3c104cb2842eb6b755) )
	ROM_LOAD( "t-14.g3", 0x30000, 0x10000, CRC(a30e3346) SHA1(150a837fb5d4705df9e8e9a94f78cff0e1c57d64) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.f9", 0x00000, 0x08000, CRC(9b377277) SHA1(4858560e35144727aea958023f3df785baa994a8) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "82s129.11p", 0x000, 0x100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) // vertical timing
	ROM_LOAD( "82s129.11r", 0x100, 0x100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) ) // horizontal timing
	ROM_LOAD( "82s123.9e",  0x200, 0x020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) ) // tied to the sound Z80 and without it there's no sound on the real PCB
ROM_END

ROM_START( turtshipkn )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-3.g5",  0x00000, 0x08000, CRC(529b091c) SHA1(9a3a885dbf1f9d3c3c326418efdcb4f6f96eb4ae) ) // sldh
	ROM_LOAD( "t-2.g3",  0x08000, 0x08000, CRC(d2f30195) SHA1(d64f088ed776658563943e8cde086842d0d899f8) ) // sldh
	ROM_LOAD( "t-1.e3",  0x10000, 0x08000, CRC(2d02da90) SHA1(5cf059e04e145861f9877cefa2c7168e6ded19ac) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-4.a8",  0x00000, 0x08000, CRC(1cbe48e8) SHA1(6ac5981d36a44595bb8dc847c54c7be7b374f82c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "t-5.k8",  0x00000, 0x04000, CRC(5c2ee02d) SHA1(c8d3dbdaab943c1639795915cf275951501a2a77) ) // sldh
	ROM_CONTINUE(        0x00000, 0x04000 )   /* A14 tied high, only upper half is used */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-8.d1",  0x00000, 0x10000, CRC(2f0b2336) SHA1(a869e0a50aab7d29afbca46fa04bd470488a8eeb) ) /* tiles */
	ROM_LOAD( "t-10.c3", 0x10000, 0x10000, CRC(6a0072f4) SHA1(d74b53ed90a4d01020a179f263a39b7547b8f82e) )
	ROM_RELOAD( 0x30000, 0x10000)
	ROM_LOAD( "t-11.d3", 0x20000, 0x10000, CRC(53da6cb1) SHA1(52720746298adb01828f959f81b385d268c94343) )
	ROM_LOAD( "t-6.a1",  0x40000, 0x10000, CRC(a7cce654) SHA1(f6c99622dcacc1d76021ca29b0bbceefbb75c499) )
	ROM_LOAD( "t-7.c1",  0x50000, 0x10000, CRC(90dd8415) SHA1(8e9d43ff9164fb287ab82df7da8890976b9d21c7) )
	ROM_RELOAD( 0x70000, 0x10000)
	ROM_LOAD( "t-9.a3",  0x60000, 0x10000, CRC(44762916) SHA1(3427066fc02d1b9b71a59ac41d3332d5cd8d1423) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-13.i1", 0x00000, 0x10000, CRC(1cc87f50) SHA1(d7d8a4376b556675dafa0a407bb34b6017f17e7d) ) /* sprites */
	ROM_LOAD( "t-15.i3", 0x10000, 0x10000, CRC(3bf91fb8) SHA1(1c8368dc8d52c3c48a85391f00c91a80fa5d781d) ) // sldh
	ROM_LOAD( "t-12.g1", 0x20000, 0x10000, CRC(57783312) SHA1(57942e8c3b7be63ea62bae3c104cb2842eb6b755) )
	ROM_LOAD( "t-14.g3", 0x30000, 0x10000, CRC(ee162dc0) SHA1(127b3cb3ddd47aa8ee70cad2d54b1306ad8f10e8) ) // sldh

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-16.f9", 0x00000, 0x08000, CRC(9b377277) SHA1(4858560e35144727aea958023f3df785baa994a8) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "82s129.11p", 0x000, 0x100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) // vertical timing
	ROM_LOAD( "82s129.11r", 0x100, 0x100, CRC(c47c182a) SHA1(47d6139256e6838f633a04084bd0a7a84912f7fb) ) // horizontal timing
	ROM_LOAD( "82s123.9e",  0x200, 0x020, CRC(c5817816) SHA1(cc642daafa0bcb160ee04e74e2d168fd44087608) ) // tied to the sound Z80 and without it there's no sound on the real PCB
ROM_END


ROM_START( dyger )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "d-3.5g",  0x00000, 0x08000, CRC(bae9882e) SHA1(88194e58673ebd0841e9e07482842f6dbb823afc) )
	ROM_LOAD( "d-2.3g",  0x08000, 0x08000, CRC(059ac4dc) SHA1(fe46d819946e168b4a8188302737fdde957743ea) )
	ROM_LOAD( "d-1.3e",  0x10000, 0x08000, CRC(d8440f66) SHA1(3b2ee8c09d40edbe76d5004ed9074add0d4e4fd0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d-4.8a",  0x0000, 0x8000, CRC(8a256c09) SHA1(2c692af62da7c12b7d4f3f79264ee045a2cfa39f) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "d-5.8k",  0x00000, 0x04000, CRC(c4bc72a5) SHA1(ee4ac5cbc9e97dd6fd0c9f507ee22a3eb36ba1b2) )   /* characters */
	ROM_CONTINUE(        0x00000, 0x04000 ) /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "d-10.1d", 0x00000, 0x10000, CRC(9715880d) SHA1(a6a400a0f4a80f3d151851a8ed182a6695a468b7) )   /* tiles */
	ROM_LOAD( "d-9.3c",  0x10000, 0x10000, CRC(628dae72) SHA1(5cfd5b87f702650afaf0999a45670f956b8254b2) )
	ROM_RELOAD( 0x30000, 0x10000)
	ROM_LOAD( "d-11.3d", 0x20000, 0x10000, CRC(23248db1) SHA1(47c5ef86e74be142faa0b896749d964ea1adc958) )
	ROM_LOAD( "d-6.1a",  0x40000, 0x10000, CRC(4ba7a437) SHA1(14bd939e3c5c28c5c7379e57832a0d3d707984f7) )
	ROM_LOAD( "d-8.1c",  0x50000, 0x10000, CRC(6c0f0e0c) SHA1(aac2b31346ebc6f2fb664faca732cd3738efcbab) )
	ROM_RELOAD( 0x70000, 0x10000)
	ROM_LOAD( "d-7.3a",  0x60000, 0x10000, CRC(2c50a229) SHA1(14498a06ec7c683c161f46633b270548ca8a9b85) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d-14.1i", 0x00000, 0x10000, CRC(99c60b26) SHA1(bcd56df5ef93c6133b61bce6472a708e340fbaaf) )   /* sprites */
	ROM_LOAD( "d-15.3i", 0x10000, 0x10000, CRC(d6475ecc) SHA1(61f6a9b443810742a2d39e61d14b92924cc27da7) )
	ROM_LOAD( "d-12.1g", 0x20000, 0x10000, CRC(e345705f) SHA1(0c51c0c598c0f51268108c7351b1b24977ae2b9f) )
	ROM_LOAD( "d-13.3g", 0x30000, 0x10000, CRC(faf4be3a) SHA1(dcf1958a17b587845174374f9598d0a979d7a6d5) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "d-16.9f", 0x0000, 0x8000, CRC(0792e8f2) SHA1(3716839502679ecc973571d824065b40771d5bfa) )
ROM_END

ROM_START( dygera )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "d-3.bin", 0x00000, 0x08000, CRC(fc63da8b) SHA1(f324a314cda167ae05e2eb017da355709489a7a3) )
	ROM_LOAD( "d-2.3g",  0x08000, 0x08000, CRC(059ac4dc) SHA1(fe46d819946e168b4a8188302737fdde957743ea) )
	ROM_LOAD( "d-1.3e",  0x10000, 0x08000, CRC(d8440f66) SHA1(3b2ee8c09d40edbe76d5004ed9074add0d4e4fd0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d-4.8a",  0x0000, 0x8000, CRC(8a256c09) SHA1(2c692af62da7c12b7d4f3f79264ee045a2cfa39f) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "d-5.8k",  0x00000, 0x04000, CRC(c4bc72a5) SHA1(ee4ac5cbc9e97dd6fd0c9f507ee22a3eb36ba1b2) )   /* characters */
	ROM_CONTINUE(        0x00000, 0x04000 ) /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "d-10.1d", 0x00000, 0x10000, CRC(9715880d) SHA1(a6a400a0f4a80f3d151851a8ed182a6695a468b7) )   /* tiles */
	ROM_LOAD( "d-9.3c",  0x10000, 0x10000, CRC(628dae72) SHA1(5cfd5b87f702650afaf0999a45670f956b8254b2) )
	ROM_RELOAD( 0x30000, 0x10000)
	ROM_LOAD( "d-11.3d", 0x20000, 0x10000, CRC(23248db1) SHA1(47c5ef86e74be142faa0b896749d964ea1adc958) )
	ROM_LOAD( "d-6.1a",  0x40000, 0x10000, CRC(4ba7a437) SHA1(14bd939e3c5c28c5c7379e57832a0d3d707984f7) )
	ROM_LOAD( "d-8.1c",  0x50000, 0x10000, CRC(6c0f0e0c) SHA1(aac2b31346ebc6f2fb664faca732cd3738efcbab) )
	ROM_RELOAD( 0x70000, 0x10000)
	ROM_LOAD( "d-7.3a",  0x60000, 0x10000, CRC(2c50a229) SHA1(14498a06ec7c683c161f46633b270548ca8a9b85) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d-14.1i", 0x00000, 0x10000, CRC(99c60b26) SHA1(bcd56df5ef93c6133b61bce6472a708e340fbaaf) )   /* sprites */
	ROM_LOAD( "d-15.3i", 0x10000, 0x10000, CRC(d6475ecc) SHA1(61f6a9b443810742a2d39e61d14b92924cc27da7) )
	ROM_LOAD( "d-12.1g", 0x20000, 0x10000, CRC(e345705f) SHA1(0c51c0c598c0f51268108c7351b1b24977ae2b9f) )
	ROM_LOAD( "d-13.3g", 0x30000, 0x10000, CRC(faf4be3a) SHA1(dcf1958a17b587845174374f9598d0a979d7a6d5) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "d-16.9f", 0x0000, 0x8000, CRC(0792e8f2) SHA1(3716839502679ecc973571d824065b40771d5bfa) )
ROM_END

ROM_START( twinfalc )   /* Shows "Notice  This game is for use in Korea only..." The real PCB displays the same :-) */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-15.bin",    0x00000, 0x08000, CRC(e1f20144) SHA1(911781232fc1a7d6e36abb1c45e68a4398d8deac) )
	ROM_LOAD( "t-14.bin",    0x08000, 0x10000, CRC(c499ff83) SHA1(d99bb8cb04485638c5f05584cffdd2fbbe061af7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-1.b4",     0x0000, 0x8000, CRC(b84bc980) SHA1(d2d302a96a9e3197f27144e525a901cfb9da09e4) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "t-6.r6",     0x04000, 0x04000, CRC(8e4ca776) SHA1(412a47f030e3b491e23e5696ef88d065f9de0220) ) /* characters */
	ROM_CONTINUE(           0x00000, 0x04000 )  /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-10.y10",    0x00000, 0x10000, CRC(b678ef5b) SHA1(cdddd2a033291585e25839e864e898ef36f4d287) )
	ROM_LOAD( "t-9.w10",     0x10000, 0x10000, CRC(d7345fb9) SHA1(9da907c2bcacc750426a2989bae3c3e5fcc3e3ab) )
	ROM_RELOAD( 0x30000,     0x10000)
	ROM_LOAD( "t-8.u10",     0x20000, 0x10000, CRC(41428dac) SHA1(16ae6c178b91e5cd859deb13176b7333f05c378a) )
	ROM_LOAD( "t-13.y11",    0x40000, 0x10000, CRC(0eba10bd) SHA1(e2504a5576c6af6c5bdb0263e1d3cb9ccabde3f8) )
	ROM_LOAD( "t-12.w11",    0x50000, 0x10000, CRC(c65050ce) SHA1(f90616aa4e1f80d8d7fccf5748f564cb7bc2d83a) )
	ROM_RELOAD( 0x70000,     0x10000)
	ROM_LOAD( "t-11.u11",    0x60000, 0x10000, CRC(51a2c65d) SHA1(a89f46d581d2907b7813454925ce690af007997d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-2.a5",    0x00000, 0x10000, CRC(9c106835) SHA1(7e032e65e78c380b5f03a4febd6dcd3f0bdb642b) ) /* sprites */
	ROM_LOAD( "t-3.b5",    0x10000, 0x10000, CRC(9b421ccf) SHA1(0365d48437da0f90c1c146da0605139a3da0b03b) )
	ROM_LOAD( "t-4.a7",    0x20000, 0x10000, CRC(3a1db986) SHA1(5435e891eebe5b95a5a97ee8743a8a10282e4d19) )
	ROM_LOAD( "t-5.b7",    0x30000, 0x10000, CRC(9bd22190) SHA1(7a571becde02ea4b64db4138f00408f312bf54c0) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-7.y8",    0x0000, 0x8000, CRC(a8b5f750) SHA1(94eb7af3cb8bee87ce3d31260e3bde062ebbc8f0) )
ROM_END

ROM_START( whizz )  /* Whizz Philko 1989. Original pcb. Boardnumber: 01-90 / Serial: WZ-089-00845 */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + banked ROMs images */
	ROM_LOAD( "t-15.l11",    0x00000, 0x08000, CRC(73161302) SHA1(de815bba66c376cea775139f4285de0b1a589d88) )
	ROM_LOAD( "t-14.k11",    0x08000, 0x10000, CRC(bf248879) SHA1(f46f15e3949221e59d8c37de9c23473a74c2927e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "t-1.b4",     0x0000, 0x8000, CRC(b84bc980) SHA1(d2d302a96a9e3197f27144e525a901cfb9da09e4) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "t-6.r6",     0x04000, 0x04000, CRC(8e4ca776) SHA1(412a47f030e3b491e23e5696ef88d065f9de0220) ) /* characters */
	ROM_CONTINUE(           0x00000, 0x04000 )  /* is the first half used? */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "t-10.y10",    0x00000, 0x10000, CRC(b678ef5b) SHA1(cdddd2a033291585e25839e864e898ef36f4d287) )
	ROM_LOAD( "t-9.w10",     0x10000, 0x10000, CRC(d7345fb9) SHA1(9da907c2bcacc750426a2989bae3c3e5fcc3e3ab) )
	ROM_RELOAD( 0x30000,     0x10000)
	ROM_LOAD( "t-8.u10",     0x20000, 0x10000, CRC(41428dac) SHA1(16ae6c178b91e5cd859deb13176b7333f05c378a) )
	ROM_LOAD( "t-13.y11",    0x40000, 0x10000, CRC(0eba10bd) SHA1(e2504a5576c6af6c5bdb0263e1d3cb9ccabde3f8) )
	ROM_LOAD( "t-12.w11",    0x50000, 0x10000, CRC(c65050ce) SHA1(f90616aa4e1f80d8d7fccf5748f564cb7bc2d83a) )
	ROM_RELOAD( 0x70000,     0x10000)
	ROM_LOAD( "t-11.u11",    0x60000, 0x10000, CRC(51a2c65d) SHA1(a89f46d581d2907b7813454925ce690af007997d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "t-2.a5",    0x00000, 0x10000, CRC(9c106835) SHA1(7e032e65e78c380b5f03a4febd6dcd3f0bdb642b) ) /* sprites */
	ROM_LOAD( "t-3.b5",    0x10000, 0x10000, CRC(9b421ccf) SHA1(0365d48437da0f90c1c146da0605139a3da0b03b) )
	ROM_LOAD( "t-4.a7",    0x20000, 0x10000, CRC(3a1db986) SHA1(5435e891eebe5b95a5a97ee8743a8a10282e4d19) )
	ROM_LOAD( "t-5.b7",    0x30000, 0x10000, CRC(9bd22190) SHA1(7a571becde02ea4b64db4138f00408f312bf54c0) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* background tilemaps */
	ROM_LOAD( "t-7.y8",    0x0000, 0x8000, CRC(a8b5f750) SHA1(94eb7af3cb8bee87ce3d31260e3bde062ebbc8f0) )
ROM_END


// date string is at 0xaa2 in 'rom 03' it does not appear to be displayed

GAME( 1986, sidearms,   0,        sidearms, sidearms, sidearms_state, init_sidearms, ROT0,   "Capcom",                   "Hyper Dyne Side Arms (World, 861129)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsu,  sidearms, sidearms, sidearms, sidearms_state, init_sidearms, ROT0,   "Capcom (Romstar license)", "Hyper Dyne Side Arms (US, 861202)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsur1,sidearms, sidearms, sidearms, sidearms_state, init_sidearms, ROT0,   "Capcom (Romstar license)", "Hyper Dyne Side Arms (US, 861128)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidearmsj,  sidearms, sidearms, sidearms, sidearms_state, init_sidearms, ROT0,   "Capcom",                   "Hyper Dyne Side Arms (Japan, 861128)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1988, turtship,   0,        turtship, turtship, sidearms_state, init_turtship, ROT0,   "Philko (Sharp Image license)",   "Turtle Ship (North America)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipj,  turtship, turtship, turtship, sidearms_state, init_turtship, ROT0,   "Philko (Pacific Games license)", "Turtle Ship (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipk,  turtship, turtship, turtship, sidearms_state, init_turtship, ROT0,   "Philko",                         "Turtle Ship (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipko, turtship, turtship, turtship, sidearms_state, init_turtship, ROT0,   "Philko",                         "Turtle Ship (Korea, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, turtshipkn, turtship, turtship, turtship, sidearms_state, init_turtship, ROT0,   "Philko",                         "Turtle Ship (Korea, 88/9)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, dyger,      0,        turtship, dyger,    sidearms_state, init_dyger,    ROT270, "Philko", "Dyger (Korea set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, dygera,     dyger,    turtship, dyger,    sidearms_state, init_dyger,    ROT270, "Philko", "Dyger (Korea set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, twinfalc,   0,        whizz,    whizz,    sidearms_state, init_whizz,    ROT0,   "Philko (Poara Enterprises license)", "Twin Falcons", MACHINE_SUPPORTS_SAVE )
GAME( 1989, whizz,      twinfalc, whizz,    whizz,    sidearms_state, init_whizz,    ROT0,   "Philko",                             "Whizz", MACHINE_SUPPORTS_SAVE )
