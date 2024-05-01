// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

1942

driver by Paul Leaman


MAIN CPU:

0000-bfff ROM (8000-bfff banked)
cc00-cc7f Sprites
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff Background RAM (groups of 32 bytes, 16 code, 16 color/attribute)
e000-efff RAM

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW0
c004      DSW1

write:
c800      command for the audio CPU
c802-c803 background scroll
c804      bit 7: flip screen
          bit 4: cpu B reset
          bit 0: coin counter
c805      background palette bank selector
c806      bit 0-1 ROM bank selector 00=1-N5.BIN
                                    01=1-N6.BIN
                                    10=1-N7.BIN



SOUND CPU:

0000-3fff ROM
4000-47ff RAM
6000      command from the main CPU
8000      8910 #1 control
8001      8910 #1 write
c000      8910 #2 control
c001      8910 #2 write



Game runs in interrupt mode 0 (the devices supply the interrupt number).

Two interrupts must be triggered per refresh for the game to function
correctly.

0x10 is the video retrace. This controls the speed of the game and generally
     drives the code. This must be triggered for each video retrace.
0x08 is the sound card service interrupt. The game uses this to throw sounds
     at the sound CPU.


***************************************************************************

1942, Capcom 1984
Hardware info by Guru

PCB layout
----------
(Note the two boards are mounted solder side to solder side with components facing outwards)

Top board:

84100-01A-03
       |-------------|                |---------|
|------|-------------|----------------|---------|-------|
|HA1368  VOL             82S129.F1                      |
|                                                       |
|              M58725(3)  SR02.F2                       |
|                                              SR03.N3  |
|                                                       |
|                                 J1           SR04.N4  |
|2                                                      |
|8                                             SR05.N5  |
|W                                                      |
|A  SWA                           82S129.K6    SR06.N6  |
|Y                                                      |
|   SWB                                  J2    SR07.N7  |
|                                              X        |
|AY-3-8910   M58725(2)                         M58725(1)|
|                                                       |
|AY-3-8910   SR01.C11  Z80A(2) 12MHz   Z80A(1) M58725(1)|
|-------------------------------------------------------|
Notes:
       Z80A(1) - Z80A CPU. Clock 3.000MHz [12/4] (Main CPU)
       Z80A(2) - Z80A CPU. Clock 3.000MHz [12/4] (Sound CPU)
     AY-3-8910 - General Instrument AY-3-8910 Programmable Sound Generator (PSG). Clock 1.500MHz [12/8, both chips)
         SWA/B - 8-position DIP switch
            J1 - Wire link set to lower position marked '100'
            J2 - Wire link set to right position marked '100'
         28WAY - Uses unique Capcom 28-way pinout for 1942. This is not JAMMA
           VOL - 2k-ohm Volume Pot
        HA1368 - Hitachi HA1368 5.3W Audio Power Amplifier
     M58725(1) - Mitsubishi M58725 2kBx8-bit SRAM (main program RAM. 'W-RAM' on the test screen)
     M58725(2) - Mitsubishi M58725 2kBx8-bit SRAM (sound program RAM. 'S-RAM' on the test screen)
     M58725(3) - Mitsubishi M58725 2kBx8-bit SRAM (character RAM. 'V-RAM' on the test screen)
             X - Empty DIP28 location (no ROM or socket)
      SR01.C11 - 27C128 16kBx8-bit EPROM (sound program)
       SR02.F2 - 27C64 8kBx8-bit EPROM (characters)
       SR03.N3 - \
       SR04.N4 -  |
       SR05.N5 -  | 27C128 16kBx8-bit EPROM (main program)
       SR07.N7 - /
       SR06.N6 - 27C64 8kBx8-bit EPROM (part of main program)
     82S129.F1 - Signetics 82S129 256x4-bit Bipolar PROM (character lookup table)
     82S129.K6 - Signetics 82S129 256x4-bit Bipolar PROM (interrupt timing)
         HSync - 15.6173kHz
         VSync - 59.6079Hz


Bottom board:

84100-02A-3
|-------------------------------------------------------|
| SR08.A1    82S129.D1                  SR14.L1  SR16.N1|
| SR09.A2    82S129.D2    6148          SR15.L2  SR17.N2|
|                                                       |
| SR10.A3                           82S129.K3           |
| SR11.A4                        6148                   |
|                                                       |
| SR12.A5                                               |
| SR13.A6    82S129.D6                                  |
|                                                       |
|  MB8128                                               |
|                  82S129.E8  5114                      |
|                                                       |
|                  82S129.E9  5114                      |
|                                                       |
|                  82S129.E10                           |
|                                          82S129.M11   |
|------|-------------|----------------|---------|-------|
       |-------------|                |---------|
Notes:
      MB8128 - Fujitsu MB8128 2kBx8-bit SRAM (background tile RAM)
        6148 - Hitachi HM6148 1kBx4-bit SRAM (sprite generator RAM)
        5114 - Sharp LH5114 1kBx4-bit SRAM (sprite display RAM)
     SR14.L1 \
     SR16.N1  |
     SR15.L2  | 27C128 16kBx8-bit EPROM (sprites)
     SR17.N2 /
     SR08.A1 \
     SR09.A2  |
     SR10.A3  | 27C64 8kBx8-bit EPROM (background tiles)
     SR11.A4  |
     SR12.A5  |
     SR13.A6 /
   82S129.D1 - Signetics 82S129 256x4-bit Bipolar PROM (tile palette selector)
   82S129.D2 - Signetics 82S129 256x4-bit Bipolar PROM (tile palette selector)
   82S129.K3 - Signetics 82S129 256x4-bit Bipolar PROM (sprite lookup table)
   82S129.D6 - Signetics 82S129 256x4-bit Bipolar PROM (tile lookup table)
   82S129.E8 - Signetics 82S129 256x4-bit Bipolar PROM (red color PROM)
   82S129.E9 - Signetics 82S129 256x4-bit Bipolar PROM (green color PROM)
  82S129.E10 - Signetics 82S129 256x4-bit Bipolar PROM (blue color PROM)
  82S129.M11 - Signetics 82S129 256x4-bit Bipolar PROM (video timing)

***************************************************************************/

#include "emu.h"
#include "1942.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/netlist.h"
#include "speaker.h"

#include "nl_1942.h"

namespace {

/* 12mhz OSC */
constexpr XTAL MASTER_CLOCK(12_MHz_XTAL);
constexpr XTAL MAIN_CPU_CLOCK(MASTER_CLOCK/4);
constexpr XTAL SOUND_CPU_CLOCK(MASTER_CLOCK/4);
constexpr XTAL AUDIO_CLOCK(MASTER_CLOCK/8);

/* 20mhz OSC - both Z80s are 4 MHz */
constexpr XTAL MASTER_CLOCK_1942P(20_MHz_XTAL);
constexpr XTAL MAIN_CPU_CLOCK_1942P(MASTER_CLOCK_1942P/5);
constexpr XTAL SOUND_CPU_CLOCK_1942P(MASTER_CLOCK_1942P/5);
constexpr XTAL AUDIO_CLOCK_1942P(MASTER_CLOCK_1942P/16);

} // anonymous namespace


void _1942_state::_1942_bankswitch_w(uint8_t data)
{
	membank("bank1")->set_entry(data & 0x03);
}

TIMER_DEVICE_CALLBACK_MEMBER(_1942_state::_1942_scanline)
{
	int scanline = param;

	if (scanline == 0x2c) // audio irq point 1
		m_audiocpu->set_input_line(0, HOLD_LINE);

	if (scanline == 0x6d) // periodic irq (writes to the soundlatch and drives freeze dip-switch), + audio irq point 2
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   /* Z80 - RST 08h */
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}

	if (scanline == 0xaf) // audio irq point 3
		m_audiocpu->set_input_line(0, HOLD_LINE);

	if (scanline == 0xf0) // vblank-out irq, audio irq point 4
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* Z80 - RST 10h - vblank */
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}


void _1942_state::_1942_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSWA");
	map(0xc004, 0xc004).portr("DSWB");
	map(0xc800, 0xc800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc802, 0xc803).w(FUNC(_1942_state::_1942_scroll_w));
	map(0xc804, 0xc804).w(FUNC(_1942_state::_1942_c804_w));
	map(0xc805, 0xc805).w(FUNC(_1942_state::_1942_palette_bank_w));
	map(0xc806, 0xc806).w(FUNC(_1942_state::_1942_bankswitch_w));
	map(0xcc00, 0xcc7f).ram().share("spriteram");
	map(0xd000, 0xd7ff).ram().w(FUNC(_1942_state::_1942_fgvideoram_w)).share("fg_videoram");
	map(0xd800, 0xdbff).ram().w(FUNC(_1942_state::_1942_bgvideoram_w)).share("bg_videoram");
	map(0xe000, 0xefff).ram();
}

void _1942p_state::_1942p_f600_w(uint8_t data)
{
//  printf("_1942p_f600_w %02x\n", data);
}

void _1942p_state::_1942p_palette_w(offs_t offset, uint8_t data)
{
	m_protopal[offset] = data;

	int r = (data & 0x07) >> 0;
	int g = (data & 0x38) >> 3;
	int b = (data & 0xc0) >> 6;

	m_palette->set_indirect_color(offset, rgb_t(r<<5,g<<5,b<<6));
}

void _1942p_state::_1942p_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");

	map(0xd000, 0xd7ff).ram().w(FUNC(_1942p_state::_1942_fgvideoram_w)).share("fg_videoram");
	map(0xd800, 0xdbff).ram().w(FUNC(_1942p_state::_1942_bgvideoram_w)).share("bg_videoram");

	map(0xe000, 0xefff).ram();

	map(0xce00, 0xcfff).ram().share("spriteram");

	map(0xdc02, 0xdc03).w(FUNC(_1942p_state::_1942_scroll_w));
	map(0xc804, 0xc804).w(FUNC(_1942p_state::_1942_c804_w));
	map(0xc805, 0xc805).w(FUNC(_1942p_state::_1942_palette_bank_w));

	map(0xf000, 0xf3ff).ram().w(FUNC(_1942p_state::_1942p_palette_w)).share("protopal");

	map(0xf400, 0xf400).w(FUNC(_1942p_state::_1942_bankswitch_w));
	map(0xf500, 0xf500).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf600, 0xf600).w(FUNC(_1942p_state::_1942p_f600_w));

	map(0xf700, 0xf700).portr("DSWA");
	map(0xf701, 0xf701).portr("SYSTEM");
	map(0xf702, 0xf702).portr("DSWB");
	map(0xf703, 0xf703).portr("P1");
	map(0xf704, 0xf704).portr("P2");
}


void _1942p_state::_1942p_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void _1942p_state::_1942p_sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).nopw();
	map(0x0014, 0x0015).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x0018, 0x0019).w("ay2", FUNC(ay8910_device::address_data_w));
}



void _1942_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xc000, 0xc001).w("ay2", FUNC(ay8910_device::address_data_w));
}


static INPUT_PORTS_START( 1942 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x30, "20K 80K 80K+" )
	PORT_DIPSETTING(    0x20, "20K 100K 100K+" )
	PORT_DIPSETTING(    0x10, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 100K+" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_SERVICE_DIPLOC(0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop" )           PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( 1942p )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "B" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x30, "20K 80K 80K+" )
	PORT_DIPSETTING(    0x20, "20K 100K 100K+" )
	PORT_DIPSETTING(    0x10, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 100K+" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_SERVICE_DIPLOC(0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop" )           PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( gfx_1942 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,             0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,          64*4, 4*32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 64*4+4*32*8, 16 )
GFXDECODE_END


static const gfx_layout charlayout_p =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout_p =
{
	16,16,
	RGN_FRAC(1,12),
	3,
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			0x2000*8, 0x2000*8 +1, 0x2000*8 +2, 0x2000*8 +3, 0x2000*8 +4, 0x2000*8 +5, 0x2000*8 + 6, 0x2000*8 +7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,    0x1000*8 + 0*8, 0x1000*8 + 1*8, 0x1000*8 + 2*8,  0x1000*8 + 3*8,  0x1000*8 + 4*8,  0x1000*8 + 5*8,  0x1000*8 + 6*8,  0x1000*8 + 7*8     },
	8*8
};

static const gfx_layout spritelayout_p =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*16+0, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8 , 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( gfx_1942p )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_p,             0x000, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_p,          0x300, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_p, 0x400, 16 )
GFXDECODE_END



void _1942_state::machine_start()
{
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_scroll));
}

void _1942_state::machine_reset()
{
	m_palette_bank = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
}

void _1942_state::_1942(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CPU_CLOCK);    /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &_1942_state::_1942_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(_1942_state::_1942_scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, SOUND_CPU_CLOCK);  /* 3 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &_1942_state::sound_map);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_1942);

	PALETTE(config, m_palette, FUNC(_1942_state::_1942_palette), 64*4+4*32*8+16*16, 256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/2, 384, 128, 0, 262, 22, 246);   // hsync is 50..77, vsync is 257..259
	m_screen->set_screen_update(FUNC(_1942_state::screen_update));
	m_screen->set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &ay1(AY8910(config, "ay1", AUDIO_CLOCK));  /* 1.5 MHz */
	ay1.set_flags(AY8910_RESISTOR_OUTPUT);
	ay1.set_resistors_load(10000.0, 10000.0, 10000.0);
	ay1.add_route(0, "snd_nl", 1.0, 0);
	ay1.add_route(1, "snd_nl", 1.0, 1);
	ay1.add_route(2, "snd_nl", 1.0, 2);

	ay8910_device &ay2(AY8910(config, "ay2", AUDIO_CLOCK));  /* 1.5 MHz */
	ay2.set_flags(AY8910_RESISTOR_OUTPUT);
	ay2.set_resistors_load(10000.0, 10000.0, 10000.0);
	ay2.add_route(0, "snd_nl", 1.0, 3);
	ay2.add_route(1, "snd_nl", 1.0, 4);
	ay2.add_route(2, "snd_nl", 1.0, 5);

	/* NETLIST configuration using internal AY8910 resistor values */

	/* Minimize resampling between ay8910 and netlist */
	NETLIST_SOUND(config, "snd_nl", AUDIO_CLOCK / 8 / 2)
		.set_source(NETLIST_NAME(1942))
		.add_route(ALL_OUTPUTS, "mono", 5.0);
	NETLIST_STREAM_INPUT(config, "snd_nl:cin0", 0, "R_AY1_1.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin1", 1, "R_AY1_2.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin2", 2, "R_AY1_3.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin3", 3, "R_AY2_1.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin4", 4, "R_AY2_2.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin5", 5, "R_AY2_3.R");

	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "R1.1").set_mult_offset(70000.0 / 32768.0, 0.0);
	//NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "VR.2");
}


void _1942p_state::_1942p(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CPU_CLOCK_1942P);    /* 4 MHz - verified on PCB */
	m_maincpu->set_addrmap(AS_PROGRAM, &_1942p_state::_1942p_map);
	m_maincpu->set_vblank_int("screen", FUNC(_1942p_state::irq0_line_hold)); // note, powerups won't move down the screen with the original '1942' logic.

	Z80(config, m_audiocpu, SOUND_CPU_CLOCK_1942P);  /* 4 MHz - verified on PCB */
	m_audiocpu->set_addrmap(AS_PROGRAM, &_1942p_state::_1942p_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &_1942p_state::_1942p_sound_io);
	m_audiocpu->set_periodic_int(FUNC(_1942p_state::irq0_line_hold), attotime::from_hz(4*60));


	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_1942p);

	PALETTE(config, m_palette, FUNC(_1942p_state::_1942p_palette), 0x500, 0x400);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/2, 384, 128, 0, 262, 22, 246);   // hsync is 50..77, vsync is 257..259
	m_screen->set_screen_update(FUNC(_1942p_state::screen_update));
	m_screen->set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	AY8910(config, "ay1", AUDIO_CLOCK_1942P).add_route(ALL_OUTPUTS, "mono", 0.25); // 1.25 MHz - verified on PCB
	AY8910(config, "ay2", AUDIO_CLOCK_1942P).add_route(ALL_OUTPUTS, "mono", 0.25); // 1.25 MHz - verified on PCB
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 1942 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "srb-03.m3", 0x00000, 0x4000, CRC(d9dafcc3) SHA1(a089a9bc55fb7d6d0ac53f91b258396d5d62677a) )
	ROM_LOAD( "srb-04.m4", 0x04000, 0x4000, CRC(da0cf924) SHA1(856fbb302c9a4ec7850a26ab23dab8467f79bba4) )
	ROM_LOAD( "srb-05.m5", 0x10000, 0x4000, CRC(d102911c) SHA1(35ba1d82bd901940f61d8619273463d02fc0a952) )
	ROM_LOAD( "srb-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )
	ROM_LOAD( "srb-07.m7", 0x18000, 0x4000, CRC(0d31038c) SHA1(b588eaf6fddd66ecb2d9832dc197f286f1ccd846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sra-03.m3", 0x00000, 0x4000, CRC(40201bab) SHA1(4886c07a4602223c21419118e10aadce9c99fa5a) )
	ROM_LOAD( "sr-04.m4",  0x04000, 0x4000, CRC(a60ac644) SHA1(f37862db3cf5e6cc9ab3276f3bc45fd629fd70dd) )
	ROM_LOAD( "sr-05.m5",  0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "sr-06.m6",  0x14000, 0x2000, CRC(821c6481) SHA1(06becb6bf8b4bde3a458098498eecad566a87711) )
	ROM_LOAD( "sr-07.m7",  0x18000, 0x4000, CRC(5df525e1) SHA1(70cd2910e2945db76bd6ebfa0ff09a5efadc2d0b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

/* this is the same as the 1942a set, but with a different rom arrangement (larger roms), it appears to be a common bootleg */
ROM_START( 1942abl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "3.bin", 0x00000, 0x8000, CRC(f3184f5a) SHA1(a566c344ee1f63580d41aca95ece9ad1f7a135d2) )
	ROM_LOAD( "5.bin", 0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "7.bin", 0x14000, 0x8000, CRC(2f456c6e) SHA1(b728c72f97ccdb57a4aac53ef7ca3f4516fc2ecb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )   /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(60329fa4) SHA1(8f66c283c992a6bc676f5f0f739b7e9d07bbf9ee) )  /* tiles */
	ROM_LOAD( "11.bin", 0x4000, 0x4000, CRC(66bac116) SHA1(ce21a693ad8d7592d21e05d0cb9eabb36e7e8fef) )
	ROM_LOAD( "13.bin", 0x8000, 0x4000, CRC(623fcec1) SHA1(b3eea37d705e3871dc94e4cf6f2aacc6fbd09216) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "14.bin", 0x04000, 0x4000, CRC(df2345ef) SHA1(3776edebda7bc9c72117f4b764f3bdaec0a632b4) ) /* sprites */
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "16.bin", 0x0c000, 0x4000, CRC(c106b1ed) SHA1(a16520752fb02e403c93975ecf12b75854d58d69) )
	ROM_CONTINUE(0x8000,0x4000)

	// proms not in the set, assumed to be the same
	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

/* set contained only three program ROMs, other ROMs should be checked against a real PCB */
ROM_START( 1942h )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "supercharger_1942_@3.m3",  0x00000, 0x4000, CRC(ec70785f) SHA1(2010a945e1d5c984a14cf7f47a883d04bd71567d) ) /* labeled as SuperCharger 1942 #3 (c)1991 TBS - handwritten 1.28A */
	ROM_LOAD( "supercharger_1942_@4.m4",  0x04000, 0x4000, CRC(cc11355f) SHA1(44fceb449f406f657494eeee4e6b43bf063f2013) ) /* labeled as SuperCharger 1942 #4 (c)1991 TBS - handwritten 1.28  */
	ROM_LOAD( "supercharger_1942_@5.m5",  0x10000, 0x4000, CRC(42746d75) SHA1(ede6919b84653b94fddeb40b3004e44336880ba2) ) /* labeled as SuperCharger 1942 #5 (c)1991 TBS - handwritten 1.28  */
	ROM_LOAD( "srb-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )
	ROM_LOAD( "srb-07.m7", 0x18000, 0x4000, CRC(0d31038c) SHA1(b588eaf6fddd66ecb2d9832dc197f286f1ccd846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942b )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sr-03.m3", 0x00000, 0x4000, CRC(612975f2) SHA1(f3744335862dd4c53925cc32792badd4a378c837) )
	ROM_LOAD( "sr-04.m4", 0x04000, 0x4000, CRC(a60ac644) SHA1(f37862db3cf5e6cc9ab3276f3bc45fd629fd70dd) )
	ROM_LOAD( "sr-05.m5", 0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "sr-06.m6", 0x14000, 0x2000, CRC(821c6481) SHA1(06becb6bf8b4bde3a458098498eecad566a87711) )
	ROM_LOAD( "sr-07.m7", 0x18000, 0x4000, CRC(5df525e1) SHA1(70cd2910e2945db76bd6ebfa0ff09a5efadc2d0b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942w )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sw-03.m3", 0x00000, 0x4000, CRC(afd79770) SHA1(74c7a887fe3d4abfce1dcfec4c75b21ab81adc8c) )
	ROM_LOAD( "sw-04.m4", 0x04000, 0x4000, CRC(933d9910) SHA1(9c73ef880f56e30a865be959f8bbdbe79c7ef8e2) )
	ROM_LOAD( "sw-05.m5", 0x10000, 0x4000, CRC(e9a71bb6) SHA1(1f0d52c9282d15f9e4898b3b144ece25d345b71f) )
	ROM_LOAD( "sw-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )   /* matches srb-06.m6 from 1942 (Revision B) */
	ROM_LOAD( "sw-07.m7", 0x18000, 0x4000, CRC(ec41655e) SHA1(dbe4bb11f2e88574cb43ba5cd216354c3b7f69a6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sw-02.f2", 0x0000, 0x2000, CRC(f8e9ada2) SHA1(028f554e70425c53faa30a6fe1c45cc16724560a) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0300, "palproms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */

	ROM_REGION( 0x0100, "charprom", 0 )
	ROM_LOAD( "sb-0.f1",  0x0000, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */

	ROM_REGION( 0x0100, "tileprom", 0 )
	ROM_LOAD( "sb-4.d6",  0x0000, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */

	ROM_REGION( 0x0100, "sprprom", 0 )
	ROM_LOAD( "sb-8.k3",  0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "sb-2.d1",  0x0000, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0100, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0200, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0300, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END


ROM_START( 1942p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "1.bin",    0x00000, 0x8000, CRC(d8506aee) SHA1(aebdce3203e7743d70a8465a5e5766f9f47cb33f) ) // sldh
	ROM_LOAD( "2.bin",    0x10000, 0x4000, CRC(793a8fbc) SHA1(57f27a2b59cbc7e82e41683ddfd58055350f80bc) ) // sldh
	ROM_CONTINUE(         0x18000, 0x4000)
	ROM_LOAD( "3.bin",    0x14000, 0x4000, CRC(108fda63) SHA1(6ffdf57a04bcfae9fdb2343f30cff50926188cbf) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "04.bin",   0x0000, 0x4000,  CRC(b4efd1af) SHA1(015b687b1714f892c3b2528bceb2df8ca48b6b8e) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "8.bin",    0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) ) /* characters */ // sldh

	ROM_REGION( 0xc000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "5.bin",    0x0000, 0x4000, CRC(1081b88c) SHA1(f3026e72206c96573fd6ba28d15e865b51735004) ) /* tiles */ // sldh
	ROM_LOAD( "6.bin",    0x4000, 0x4000, CRC(2d6acd8c) SHA1(914bb971c8f1364d0c44bd11f5f7e8da1f4953bb) ) // sldh
	ROM_LOAD( "7.bin",    0x8000, 0x4000, CRC(30f13e78) SHA1(51b9c0dfc53db705b75dd7ce643cec807533af5a) ) // sldh

	ROM_REGION( 0x10000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "9.bin",    0x0000, 0x4000, CRC(755a4762) SHA1(b8747e02854a2dd8fa1251e206dbf0a0fc017b38) ) /* tiles */ // sldh
	ROM_LOAD( "10.bin",   0x4000, 0x4000, CRC(4a5a9084) SHA1(dcf9834e58324f9c94206728a055083e335bc862) ) // sldh
	ROM_LOAD( "11.bin",   0x8000, 0x4000, CRC(d2ce3eb6) SHA1(ebe71bd413b169ff2cea6973faf48527a8283eef) ) // sldh
	ROM_LOAD( "12.bin",   0xc000, 0x4000, CRC(aaa86493) SHA1(b0f6c59b5369b565bf863544a26cde2105aa35be) ) // sldh

	ROM_REGION( 0x0100, "proms", 0 ) // only one prom was in the dump - uses paletteram instead of proms
	ROM_LOAD( "ic22.bin", 0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) ) /* sprite lookup table */ // sldh
ROM_END


void _1942_state::driver_init()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
}


//    YEAR  NAME     PARENT  MACHINE  INPUT  CLASS         INIT         ROT     COMPANY, FULLNAME, FLAGS
GAME( 1984, 1942,    0,      _1942,   1942,  _1942_state,  driver_init, ROT270, "Capcom", "1942 (Revision B)", MACHINE_SUPPORTS_SAVE)
GAME( 1984, 1942a,   1942,   _1942,   1942,  _1942_state,  driver_init, ROT270, "Capcom", "1942 (Revision A)", MACHINE_SUPPORTS_SAVE)
GAME( 1984, 1942abl, 1942,   _1942,   1942,  _1942_state,  driver_init, ROT270, "bootleg", "1942 (Revision A, bootleg)", MACHINE_SUPPORTS_SAVE) // data is the same as 1942a set, different rom format
GAME( 1991, 1942h,   1942,   _1942,   1942,  _1942_state,  driver_init, ROT270, "hack (Two Bit Score)", "Supercharger 1942", MACHINE_SUPPORTS_SAVE) // v1.28A of hack
GAME( 1984, 1942b,   1942,   _1942,   1942,  _1942_state,  driver_init, ROT270, "Capcom", "1942 (First Version)", MACHINE_SUPPORTS_SAVE)
GAME( 1985, 1942w,   1942,   _1942,   1942,  _1942_state,  driver_init, ROT270, "Capcom (Williams Electronics license)", "1942 (Williams Electronics license)", MACHINE_SUPPORTS_SAVE) // based on 1942 (Revision B)
GAME( 1984, 1942p,   1942,   _1942p,  1942p, _1942p_state, driver_init, ROT270, "bootleg", "1942 (Tecfri PCB, bootleg?)", MACHINE_SUPPORTS_SAVE )
