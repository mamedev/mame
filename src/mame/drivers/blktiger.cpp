// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Black Tiger

    Driver provided by Paul Leaman

    Thanks to Ishmair for providing information about the screen
    layout on level 3.

    Notes:
    - sprites/tile priority is a guess. I didn't find a PROM that would simply
      translate to the scheme I implemented.

***************************************************************************

Black Tiger / Black Dragon (Capcom, 1987)
Hardware info by Guru

Note boards connect solder side to solder side and parts face outwards
on both boards.

Top Board
---------

CAPCOM
87118-A-X1
|-------------------------------------------------------------------------|
|            VOL                          |------------------| BD_06.1L   |
|  HA13001         YM3014   YM2203        |138  138   21  08 |            |
|                                         |2kB  174 367 74 74| 3.579545MHz|
|        M5224     YM3014   YM2203        |Z80           139 |    JP7..12 |
|                                         |---SOUND_MODULE---|            |
|                                                                         |
|                 5814             4364                                   |
|J                                                       BD.6K            |
|A                               BD_U_01A.5E                              |
|M                5814                                             24MHz  |
|M                               BD_U_02A.6E                              |
|A                                                                        |
|                                                 BD01.8J                 |
|                                BD_U_03A.8E                              |
|                 JP1..6                          BD02.9J                 |
|         SW2                    BD_U_04A.9E                              |
|                                                                         |
|                                BD_U_05A.10E            BD03.11K         |
|         SW1                                               BD04.11L      |
|                                  Z80                                    |
|--------|--------------|-------------------------|--------------|--------|
         |--------------|                         |--------------|
Notes:
               Z80 - Zilog Z084006PSC CPU (main CPU). Clock input 6MHz [24/4]
Z80 (sound module) - Zilog Z084004PSC CPU (sound CPU). Clock input 3.579545MHz
             BD.6K - Intel D8751H-8 Microcontroller with internal 4kBx8-bit EPROM and 128 bytes internal RAM. Clock input 8.000MHz [24/3]
            YM2203 - Yamaha YM2203 FM Operator Type-N (OPN) sound chip. Clock input 3.579545MHz (both)
            YM3014 - Yamaha YM3014B Serial Input Floating D/A Converter. Clock input 1.193181667MHz [3.579545/3] (both)
      SOUND_MODULE - Contains commonly available parts.... logic, Z80A and 2kBx8-bit SRAM
            JP1..6 - 0-ohm jumpers. JP1 & JP6 are shorted, others are open
           JP7..12 - 0-ohm jumpers. JP10 & JP11 are shorted, others are open
           HA13001 - Hitachi HA13001 5.5W Dual / 17.5W BTL Audio Power Amplifier
             SW1/2 - 8-position DIP switch
             M5224 - Mitsubishi M5224 Quad Operational Amplifier (compatible with LM324)
               2kB - 2kBx8-bit SRAM (inside sound module). Compatible with 6116, 2018, 5814, 4016 etc.
              5814 - Sony CXK5814 2kBx8-bit SRAM (color RAM)
              4364 - NEC D4364 8kBx8-bit SRAM (main program RAM)
           BD01.8J - Signetics 82S129 bipolar PROM. When removed: No backgrounds and no sprites. Text layer/characters ok.
           BD02.9J - Signetics 82S129 bipolar PROM. When removed: Affects sprites, sometimes they are partial or disappear under tiles.
          BD03.11K - Signetics 82S129 bipolar PROM. Timing. When removed: Black screen, no syncs, game plays blind.
          BD04.11L - Signetics 82S129 bipolar PROM. Timing. When removed: No bootup, shows only static garbage
          BD_06.1L - Hitachi HN27256 32kBx8-bit OTP EPROM (sound program)
       BD_U_01A.5E -  \
       BD_U_02A.6E -   \ 27C512 EPROM or OTP EPROM (main program)
       BD_U_03A.8E -   /
       BD_U_04A.9E -  /
      BD_U_05A.10E - /
             VSync - 59.6373Hz
             HSync - 15.6250kHz


Bottom Board
------------

CAPCOM
87118-B-X1
         |--------------|                         |--------------|
|--------|--------------|-------------------------|--------------|--------|
|                                                                86S100   |
|         |-------|                                                       |
|         |CAPCOM |                                           BD_U_15.2N  |
|         |86S105 |                                                       |
|         |RJ5C39 |             2018           2018                       |
|         |-------|                                                       |
|                                                                         |
|                                                                         |
|  BD_07.4A    BD_U_11A.4B                                                |
|                                                                         |
|  BD_08.5A    BD_U_12A.5B                                                |
|                                                                         |
|                                                                 4016    |
|  86S100        86S100                                                   |
|                                                                         |
|  BD_09.8A    BD_U_13A.8B                                                |
|                                                                         |
|  BD_10.9A    BD_U_14A.9B   43256                                        |
|            JP7..8  JP9..12                                              |
|  JP1..6                                                                 |
|-------------------------------------------------------------------------|
Notes:
      86S105 - Custom PLCC84 chip marked 'CAPCOM 86S105 RJ5C39 863 32 JAPAN'. Manufactured by Ricoh
               This chip is used to generate sprites. Note the chip has 3x 2kBx8-bit (6kB) internal SRAM.
               This chip fails often so when a game that uses this chip has vertical jailbars in the sprites
               most likely this chip has failed.
      86S100 - Custom chip marked 'CAPCOM 86S100 M ^ 73100'. ^ = triangle. Manufactured by Texas Instruments
               This chip is similar to 2x 74LS165 8-bit parallel-to-serial shift register chips in one package
               but with direction control and two data modes (4-bit/8-bit).
      JP1..6 - 0-ohm jumpers. JP1 & JP5 are shorted, others are open
      JP7..8 - 0-ohm jumpers. JP7 shorted, JP8 open
     JP9..12 - 0-ohm jumpers. JP12 shorted, others are open
       43256 - NEC D43256 32kBx8-bit SRAM (background tile RAM)
        2018 - Toshiba TMM2018 2kBx8-bit SRAM (additional sprite RAM.... see note above re: 86S105)
        4016 - NEC D4016 2kBx8-bit SRAM (text layer / character RAM)
    BD_07.4A \
    BD_08.5A  \ 27C512 EPROM or OTP EPROM
    BD_09.8A  /
    BD_10.9A /
 BD_U_11A.4B \
 BD_U_12A.5B  \
 BD_U_13A.8B  / 27512 OTP EPROM
 BD_U_14A.9B /
  BD_U_15.2N - 27256 OTP EPROM

***************************************************************************/


#include "emu.h"
#include "includes/blktiger.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"
#include "screen.h"
#include "speaker.h"


/**************************************************

Protection comms between main cpu and i8751

**************************************************/

uint8_t blktiger_state::blktiger_from_mcu_r()
{
	return m_i8751_latch;
}

void blktiger_state::blktiger_to_mcu_w(uint8_t data)
{
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_z80_latch = data;
}

uint8_t blktiger_state::blktiger_from_main_r()
{
	m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	//printf("%02x read\n",latch);
	return m_z80_latch;
}

void blktiger_state::blktiger_to_main_w(uint8_t data)
{
	//printf("%02x write\n",data);
	m_i8751_latch = data;
}



void blktiger_state::blktiger_bankswitch_w(uint8_t data)
{
	membank("bank1")->set_entry(data & 0x0f);
}

void blktiger_state::blktiger_coinlockout_w(uint8_t data)
{
	if (ioport("COIN_LOCKOUT")->read() & 0x01)
	{
		machine().bookkeeping().coin_lockout_w(0,~data & 0x01);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x02);
	}
}


void blktiger_state::blktiger_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).rw(FUNC(blktiger_state::blktiger_bgvideoram_r), FUNC(blktiger_state::blktiger_bgvideoram_w));
	map(0xd000, 0xd7ff).ram().w(FUNC(blktiger_state::blktiger_txvideoram_w)).share("txvideoram");
	map(0xd800, 0xdbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xdc00, 0xdfff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xe000, 0xfdff).ram();
	map(0xfe00, 0xffff).ram().share("spriteram");
}

void blktiger_state::blktiger_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x01, 0x01).portr("IN1").w(FUNC(blktiger_state::blktiger_bankswitch_w));
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("DSW0").w(FUNC(blktiger_state::blktiger_coinlockout_w));
	map(0x04, 0x04).portr("DSW1").w(FUNC(blktiger_state::blktiger_video_control_w));
	map(0x05, 0x05).portr("FREEZE");
	map(0x06, 0x06).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x07, 0x07).rw(FUNC(blktiger_state::blktiger_from_mcu_r), FUNC(blktiger_state::blktiger_to_mcu_w));     /* Software protection (7) */
	map(0x08, 0x09).w(FUNC(blktiger_state::blktiger_scrollx_w));
	map(0x0a, 0x0b).w(FUNC(blktiger_state::blktiger_scrolly_w));
	map(0x0c, 0x0c).w(FUNC(blktiger_state::blktiger_video_enable_w));
	map(0x0d, 0x0d).w(FUNC(blktiger_state::blktiger_bgvideoram_bank_w));
	map(0x0e, 0x0e).w(FUNC(blktiger_state::blktiger_screen_layout_w));
}

void blktiger_state::blktigerbl_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x01, 0x01).portr("IN1").w(FUNC(blktiger_state::blktiger_bankswitch_w));
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("DSW0").w(FUNC(blktiger_state::blktiger_coinlockout_w));
	map(0x04, 0x04).portr("DSW1").w(FUNC(blktiger_state::blktiger_video_control_w));
	map(0x05, 0x05).portr("FREEZE");
	map(0x06, 0x06).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x07, 0x07).noprw();  /* Software protection (7) */
	map(0x08, 0x09).w(FUNC(blktiger_state::blktiger_scrollx_w));
	map(0x0a, 0x0b).w(FUNC(blktiger_state::blktiger_scrolly_w));
	map(0x0c, 0x0c).w(FUNC(blktiger_state::blktiger_video_enable_w));
	map(0x0d, 0x0d).w(FUNC(blktiger_state::blktiger_bgvideoram_bank_w));
	map(0x0e, 0x0e).w(FUNC(blktiger_state::blktiger_screen_layout_w));
}

void blktiger_state::blktiger_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc800).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xe002, 0xe003).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}



static INPUT_PORTS_START( blktiger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION( "SW1:1,2,3" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION( "SW1:4,5,6" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )         PORT_DIPLOCATION( "SW1:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7")
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "SW2:3,4,5" )
	PORT_DIPSETTING(    0x1c, "1 (Easiest)")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x14, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x0c, "5 (Normal)" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x04, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "SW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("FREEZE")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )    /* could be VBLANK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN_LOCKOUT")
	PORT_CONFNAME( 0x01, 0x01, "Coin Lockout Hardware Present" )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
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
	32*16
};

static GFXDECODE_START( gfx_blktiger )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x300, 32 )   /* colors 0x300-0x37f */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x000, 16 )   /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x200,  8 )   /* colors 0x200-0x27f */
GFXDECODE_END


void blktiger_state::machine_start()
{
	/* configure bankswitching */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_scroll_bank));
	save_item(NAME(m_screen_layout));
	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_bgon));
	save_item(NAME(m_z80_latch));
	save_item(NAME(m_i8751_latch));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

void blktiger_state::machine_reset()
{
	/* configure bankswitching */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);

	m_scroll_x[0] = 0;
	m_scroll_x[1] = 0;
	m_scroll_y[0] = 0;
	m_scroll_y[1] = 0;
	m_scroll_bank = 0;
	m_screen_layout = 0;
	m_z80_latch = 0;
	m_i8751_latch = 0;
}

void blktiger_state::blktiger(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000)/4); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &blktiger_state::blktiger_map);
	m_maincpu->set_addrmap(AS_IO, &blktiger_state::blktiger_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(blktiger_state::irq0_line_hold));

	Z80(config, m_audiocpu, XTAL(3'579'545));   /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &blktiger_state::blktiger_sound_map);

	I8751(config, m_mcu, XTAL(24'000'000)/3);   /* verified on pcb */
	m_mcu->port_in_cb<0>().set(FUNC(blktiger_state::blktiger_from_main_r));
	m_mcu->port_out_cb<0>().set(FUNC(blktiger_state::blktiger_to_main_w));
	// other ports unknown
	//m_mcu->set_vblank_int("screen", FUNC(blktiger_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 4, 384, 128, 0, 262, 22, 246); // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(blktiger_state::screen_update_blktiger));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram8_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_blktiger);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024);

	BUFFERED_SPRITERAM8(config, m_spriteram);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(3'579'545))); /* verified on pcb */
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(ALL_OUTPUTS, "mono", 0.15);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(3'579'545))); /* verified on pcb */
	ym2.add_route(ALL_OUTPUTS, "mono", 0.15);
}

void blktiger_state::blktigerbl(machine_config &config)
{
	blktiger(config);
	m_maincpu->set_addrmap(AS_IO, &blktiger_state::blktigerbl_io_map);

	config.device_remove("mcu");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( blktiger )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bdu-01a.5e",  0x00000, 0x08000, CRC(a8f98f22) SHA1(f77c0d0ebf3e52a21d2c0c5004350a408b8e6d24) )   /* CODE */
	ROM_LOAD( "bdu-02a.6e",  0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )   /* 0+1 */
	ROM_LOAD( "bdu-03a.8e",  0x20000, 0x10000, CRC(4089e157) SHA1(7972b1c745057802d4fd66d88b0101eb3c03e701) )   /* 2+3 */
	ROM_LOAD( "bd-04.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "bd-05.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "bd-07.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",   0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigera )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bdu-01.5e",  0x00000, 0x08000, CRC(47b13922) SHA1(a722048d48171b68119b7ef1af6e06953c238ad6) )    /* CODE */
	ROM_LOAD( "bdu-02.6e",  0x10000, 0x10000, CRC(2e0daf1b) SHA1(dbcaf2bb1b2c9cd4b2ca1d52b81d6e33b5c7eee9) )    /* 0+1 */
	ROM_LOAD( "bdu-03.8e",  0x20000, 0x10000, CRC(3b67dfec) SHA1(f9d83f2bb1fbf05d80f6870d91411e9b7bbea917) )    /* 2+3 */
	ROM_LOAD( "bd-04.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "bd-05.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "bd-07.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",   0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigerb1 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "btiger1.f6",   0x00000, 0x08000, CRC(9d8464e8) SHA1(c847ee9a22b8b636e85427214747e6bd779023e8) )  /* CODE */
	ROM_LOAD( "bdu-02a.6e",   0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )  /* 0+1 */
	ROM_LOAD( "btiger3.j6",   0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )  /* 2+3 */
	ROM_LOAD( "bd-04.9e",     0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )  /* 4+5 */
	ROM_LOAD( "bd-05.10e",    0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigerb2 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(47e2b21e) SHA1(3f03543ace435239978a95f569ac89f6762253c0) )  /* CODE */
	ROM_LOAD( "bdu-02a.6e",   0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )  /* 0+1 */
	ROM_LOAD( "3.bin",        0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )  /* 2+3 : same crc of btiger3.j6 from bktigerb */
	ROM_LOAD( "bd-04.9e",     0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )  /* 4+5 */
	ROM_LOAD( "bd-05.10e",    0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blkdrgon )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bd_01.5e",  0x00000, 0x08000, CRC(27ccdfbc) SHA1(3caafe00735ba9b24d870ee61ad2cae541551024) ) /* CODE */
	ROM_LOAD( "bd_02.6e",  0x10000, 0x10000, CRC(7d39c26f) SHA1(562a3f578e109ae020f65e341c876ad7e510a311) ) /* 0+1 */
	ROM_LOAD( "bd_03.8e",  0x20000, 0x10000, CRC(d1bf3757) SHA1(b19f8b986406bde65ac7f0d55d54f87b37f5e42f) ) /* 2+3 */
	ROM_LOAD( "bd_04.9e",  0x30000, 0x10000, CRC(4d1d6680) SHA1(e137624c59392de6aaffeded99b024938360bd25) ) /* 4+5 */
	ROM_LOAD( "bd_05.10e", 0x40000, 0x10000, CRC(c8d0c45e) SHA1(66c2e5a74c5875a2c8e28740fe944bd943246ce5) ) /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd_06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd_15.2n",  0x00000, 0x08000, CRC(3821ab29) SHA1(576f1839f63b0cad6b851d6e6a3e9dec21ac811d) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd_12.5b",  0x00000, 0x10000, CRC(22d0a4b0) SHA1(f9402ea9ffedcb280497a63c5eb352de9d4ca3fd) ) /* tiles */
	ROM_LOAD( "bd_11.4b",  0x10000, 0x10000, CRC(c8b5fc52) SHA1(621e899285ce6302e5b25d133d9cd52c09b7b202) )
	ROM_LOAD( "bd_14.9b",  0x20000, 0x10000, CRC(9498c378) SHA1(841934ddef724faf04162c4be4aea1684d8d8e0f) )
	ROM_LOAD( "bd_13.8b",  0x30000, 0x10000, CRC(5b0df8ce) SHA1(57d10b48bd61b0224ce21b36bde8d2479e8e5df4) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd_08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd_07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd_10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd_09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END


ROM_START( blkdrgonb )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "a1",           0x00000, 0x08000, CRC(7caf2ba0) SHA1(57b17caff67d36b24075f5865d433bfc8bcc9bc2) )  /* CODE */
	ROM_LOAD( "blkdrgon.6e",  0x10000, 0x10000, CRC(7d39c26f) SHA1(562a3f578e109ae020f65e341c876ad7e510a311) )  /* 0+1 */
	ROM_LOAD( "a3",           0x20000, 0x10000, CRC(f4cd0f39) SHA1(9efc5161c861c7ec8ae72509e71c6d7b71b22fc6) )  /* 2+3 */
	ROM_LOAD( "blkdrgon.9e",  0x30000, 0x10000, CRC(4d1d6680) SHA1(e137624c59392de6aaffeded99b024938360bd25) )  /* 4+5 */
	ROM_LOAD( "blkdrgon.10e", 0x40000, 0x10000, CRC(c8d0c45e) SHA1(66c2e5a74c5875a2c8e28740fe944bd943246ce5) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b5",           0x00000, 0x08000, CRC(852ad2b7) SHA1(9f30c0d7e1127589b03d8f45ea50e0f907181a4b) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "blkdrgon.5b",  0x00000, 0x10000, CRC(22d0a4b0) SHA1(f9402ea9ffedcb280497a63c5eb352de9d4ca3fd) )  /* tiles */
	ROM_LOAD( "b1",           0x10000, 0x10000, CRC(053ab15c) SHA1(f0ddc71009ab5dd69ae463c3636ec2332c0556f8) )
	ROM_LOAD( "blkdrgon.9b",  0x20000, 0x10000, CRC(9498c378) SHA1(841934ddef724faf04162c4be4aea1684d8d8e0f) )
	ROM_LOAD( "b3",           0x30000, 0x10000, CRC(9dc6e943) SHA1(0818c1fb2cc8ff403a479457b268bba6ec0730bc) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END


ROM_START( blktigerb3 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */ // == same as blktigerb2 maincpu
	ROM_LOAD( "1.5e",  0x00000, 0x08000, CRC(47e2b21e) SHA1(3f03543ace435239978a95f569ac89f6762253c0) )   /* CODE */
	ROM_LOAD( "2.6e",  0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )   /* 0+1 */
	ROM_LOAD( "3.8e",  0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )   /* 2+3 */
	ROM_LOAD( "4.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "5.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) // == same as other sets but with an address swap
	ROM_LOAD( "6.1l",  0x0000, 0x8000, CRC(6dfab115) SHA1(05f10bdfa4dff50ccc7707a7dbd8eab1680e09b9) )

	ROM_REGION( 0x08000, "gfx1", 0 ) // == same as blkdrgon
	ROM_LOAD( "15.2n",  0x00000, 0x08000, CRC(3821ab29) SHA1(576f1839f63b0cad6b851d6e6a3e9dec21ac811d) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) // == same as other sets
	ROM_LOAD( "12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 ) // == same as other sets
	ROM_LOAD( "8.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "7.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "9.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    // PROMs (function unknown), missing in this dump
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

void blktiger_state::init_blktigerb3()
{
	uint8_t *src = memregion("audiocpu")->base();
	int len = 0x8000;
	std::vector<uint8_t> buffer(len);

	for (int i = 0; i < len; i++)
	{
		int addr = bitswap<16>(i, 15,14,13,12,11,10,9,8, 3,4,5,6, 7,2,1,0);
		buffer[i] = src[addr];

	}

	memcpy(src, &buffer[0], len);
}
GAME( 1987, blktiger,   0,        blktiger,   blktiger, blktiger_state, empty_init,      ROT0, "Capcom",  "Black Tiger",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigera,  blktiger, blktiger,   blktiger, blktiger_state, empty_init,      ROT0, "Capcom",  "Black Tiger (older)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigerb1, blktiger, blktigerbl, blktiger, blktiger_state, empty_init,      ROT0, "bootleg", "Black Tiger (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigerb2, blktiger, blktigerbl, blktiger, blktiger_state, empty_init,      ROT0, "bootleg", "Black Tiger (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blkdrgon,   blktiger, blktiger,   blktiger, blktiger_state, empty_init,      ROT0, "Capcom",  "Black Dragon (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, blkdrgonb,  blktiger, blktigerbl, blktiger, blktiger_state, empty_init,      ROT0, "bootleg", "Black Dragon (bootleg)",      MACHINE_SUPPORTS_SAVE )
// this board has Capcom markings (boards 87118-A-X1 / 87118-B-X1, but no MCU, a mix of bootleg Black Tiger and Black Dragon roms, and an address swapped sound rom? is the latter an alternative security measure?
GAME( 1987, blktigerb3, blktiger, blktigerbl, blktiger, blktiger_state, init_blktigerb3, ROT0, "bootleg", "Black Tiger / Black Dragon (mixed bootleg?)", MACHINE_SUPPORTS_SAVE )
