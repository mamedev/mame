// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

tecmo.cpp

driver by Nicola Salmoria

TODO:
- raster effect in Gemini Wing THE END ending, needs a side-by-side test with
  a real board and maybe waitstate penalties;

Notes:
- btanb: missing drums in backfirt, there isn't any ADPCM / rom that makes
  that to happen (code still tries to writes on that, but it's just nop'ed);

Silkworm memory map (preliminary)

0000-bfff ROM
c000-c1ff Background video RAM #2
c200-c3ff Background color RAM #2
c400-c5ff Background video RAM #1
c600-c7ff Background color RAM #1
c800-cbff Video RAM
cc00-cfff Color RAM
d000-dfff RAM
e000-e7ff Sprites
e800-efff Palette RAM, groups of 2 bytes, 4 bits per gun: xB RG
          e800-e9ff sprites
          ea00-ebff characters
          ec00-edff bg #1
          ee00-efff bg #2
f000-f7ff window for banked ROM

read:
f800      IN0 (heli) bit 0-3
f801      IN0 bit 4-7
f802      IN1 (jeep) bit 0-3
f803      IN1 bit 4-7
f806      DSWA bit 0-3
f807      DSWA bit 4-7
f808      DSWB bit 0-3
f809      DSWB bit 4-7
f80e      SYS_3 bit 0 (swap jeep/heli, proto sets only)
f80f      COIN

write:
f800-f801 bg #1 x scroll
f802      bg #1 y scroll
f803-f804 bg #2 x scroll
f805      bg #2 y scroll
f806      ????
f808      ROM bank selector
f809      ????
f80b      ????

***************************************************************************

Rygar, Tecmo 1986
Hardware Info By Guru

PCB Layout
----------
6002-A
|-----------------------------------------------------------------|
|DSW2               4MHz                            24MHz         |
|DSW1                    M4069                                    |
|                                                                |-|
|                   Z80B                            MN50005XTA   | |
|1                            5.5P                               | |
|8                                                               | |
|W                            4.5M                               | |
|A                  MA7053                6116                   |-|
|Y   DIP28                    6264                                |
|                   Z80A                  6.7K                    |
|                             3.5J                                |
|                   2.4H                                          |
|                                                                |-|
|                   6116                                         | |
|    1.1E                                                        | |
|         M5205     YM3526                                       | |
|         400kHz                                                 | |
|    VOL                 M4066           MBM2148                 |-|
|         M5224                          MBM2148                  |
|M51516         M5224  YM3014            MBM2148              CN4 |
|-----------------------------------------------------------------|
Notes:
      Z80B       - Clock 6.000MHz [24/4]
      Z80A       - Clock 4.000MHz
      M5205      - Clock 400kHz
      YM3526     - Clock 4.000MHz
      CN4        - RGB/Sync connector for video output to monitor
      MN50005XTA - Mitsubishi DIP28 custom chip. On the bootleg this chip
                   is replaced by a plug-in daughterboard containing a
                   few logic chips
      MA7053     - Mitsubishi SIL28 custom ceramic module
      M5224      - Equivalent to LM324 OP AMP
      DIP28      - unpopulated socket
      DSW1/2     - 8-position DIP Switch
      6116       - 2kx8 SRAM
      6264       - 8kx8 SRAM
      2148       - 1kx4 SRAM

      Measurements -
                    OSC1  - 23.99999MHz
                    XTAL1 - 3.999187MHz
                    VSync - 59.1856Hz
                    HSync - 15.1436kHz
6002B
|-----------------------------------------------------------------|
|                            6116                                 |
|                            18.6R                                |
|                            17.6P                               |-|
|                            16.6N                               | |
|4164 4164 4164 4164         15.6M                               | |
|4164 4164 4164 4164         14.6L                               | |
|4164 4164 4164 4164         13.6K                               | |
|4164 4164 4164 4164                                             |-|
|4164 4164 4164 4164         12.6J                                |
|                                                                 |
|                            11.6H                                |
|                                                                 |
|                            10.6E                               |-|
|                                                                | |
|                            9.6D                                | |
|                                                                | |
|                            8.6C                                | |
|                  M60002-0118P                                  |-|
|      6116                  7.6B                                 |
|                            6116                                 |
|-----------------------------------------------------------------|
Notes:
      M60002-0118P - Mitsubishi DIP42 custom chip. On the bootleg this chip
                     is replaced by a plug-in daughterboard containing a
                     few logic chips
      4164         - 64kx1 DRAM
      6116         - 2kx8 SRAM


Silkworm, Tecmo 1988
Hardware Info By Guru

PCB Layout
----------
6217A
|-----------------------------------------------------------------|
|CN3   M5224    1.6B                       MN50005XTA        24MHz|
| MB3731  M5205 384kHz                                            |
|  VOL                                                           |-|
|LED  M5224             D780C-2                                  | |
|LED  Y3014                                   M58725             | |
|                                                                | |
|                                                                | |
|J  J9 J8  AM2148  YM3812                                        |-|
|A         AM2148  M58725 3.5J        2.3J                        |
|M         AM2148                                                 |
|M                                                                |
|A                                                                |
|                                                                |-|
| PC847                                                          | |
| PC847                                                          | |
| PC847                                                          | |
| PC847         5.6S 6264 4.5S                                   | |
| PC847                                            J1            |-|
| PC847 SW2                                        J2             |
|       SW1    MA7053                     LH0080E  J11   8MHz     |
|-----------------------------------------------------------------|
Notes:
      D780C-2    - Z80 CPU (sound program), clock 4.000MHz [24/6]
      LH0080E    - Z80 CPU (main program), clock 8.000MHz
      M5205      - Oki M5205 ADPCM sample player driven by a 384kHz resonator. Pin 1,2=H,L so sample rate divider is /48 and sample rate is 8kHz
      YM3812     - Clock 4.000MHz [24/6]
      Y3014      - Yamaha DAC
      MB3731     - Fujitsu MB3731 audio power AMP
      CN3        - 4-pin power connector joining to bottom board
      MN50005XTA - Mitsubishi DIP28 custom chip. On the bootleg this chip
                   is replaced by a plug-in daughterboard containing a few
                   logic chips or integrated directly onto the PCB using logic chips.
      MA7053     - Mitsubishi SIL28 custom ceramic module
      M5224      - Equivalent to LM324 OP AMP
      DSW1/2     - 8-position DIP switch
      M58725     - 2kx8 SRAM equivalent to 6116
      6264       - 8kx8 SRAM
      AM2148     - 1kx4 SRAM
      PC847      - Sharp PC847 Optocoupler. Printed on the chip is PC817 x4 and it's the same on the datasheet.
      J1,J2      - Solder-blob jumper, open
      J11,J8,J9  - Solder-blob jumper, shorted

      Measurements -
                    VSync - 59.2680Hz
                    HSync - 15.1439kHz
6217B
|-----------------------------------------------------------------|
| 14.1S  16.2S         JP8         6116                           |
|   15.12S  17.3S      JP7                                        |
|                      JP6                                       |-|
|                      JP5                                       | |
| 10.1P  12.2P                                                   | |
|   11.12P  13.3P                                                | |
|                      JP4                                       | |
|                      JP3         6116                          |-|
|                      JP2                                        |
|                      JP1         6116                           |
|                                                                 |
| 9.1H                                                            |
|                                                                |-|
| 8.1F                                                           | |
|                   MN41128 MN41128 MN41128 MN41128              | |
| 7.1D              MN41128 MN41128 MN41128 MN41128              | |
|                   MN41128 MN41128 MN41128 MN41128              | |
| 6.1C              MN41128 MN41128 MN41128 MN41128              |-|
|                   MN41128 MN41128 MN41128 MN41128               |
| M60002-0118P                                                    |
|-----------------------------------------------------------------|
Notes:
      Note some ROMs are mid-way between position 1 and 2 so 12 is used as the location.
      M60002-0118P - Mitsubishi DIP42 custom chip. On the bootleg this chip
                     is replaced by a plug-in daughterboard containing a
                     few logic chips or integrated directly onto the PCB using logic chips.
                     There is a 6MHz clock input on pin 4.
      MN41128      - Panasonic MN41128 128kx1 NMOS DRAM (total=320kx8 video RAM)
      6116         - 2kx8 SRAM
      JP8,6,4,2    - Solder-blob jumper, open
      JP7,5,3,1    - Solder-blob jumper, shorted
      CN3          - 4-pin power connector joining to bottom board

***************************************************************************/

#include "emu.h"
#include "tecmo.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopl.h"
#include "speaker.h"


void tecmo_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data >> 3);
}

void tecmo_state::adpcm_start_w(uint8_t data)
{
	m_adpcm_pos = data << 8;
	m_msm->reset_w(0);
}

void tecmo_state::adpcm_end_w(uint8_t data)
{
	m_adpcm_end = (data + 1) << 8;
}

void tecmo_state::adpcm_vol_w(uint8_t data)
{
	m_msm->set_output_gain(ALL_OUTPUTS, (data & 15) / 15.0);
}

void tecmo_state::adpcm_int(int state)
{
	if (m_adpcm_pos >= m_adpcm_end ||
				m_adpcm_pos >= m_adpcm_rom.bytes())
		m_msm->reset_w(1);
	else if (m_adpcm_data != -1)
	{
		m_msm->data_w(m_adpcm_data & 0x0f);
		m_adpcm_data = -1;
	}
	else
	{
		m_adpcm_data = m_adpcm_rom[m_adpcm_pos++];
		m_msm->data_w(m_adpcm_data >> 4);
	}
}

// the 8-bit dipswitches are split across addresses
template <unsigned Which>
uint8_t tecmo_state::dsw_l_r()
{
	uint8_t port = m_dsw[Which]->read();
	port &= 0x0f;
	return port;
}

template <unsigned Which>
uint8_t tecmo_state::dsw_h_r()
{
	uint8_t port = m_dsw[Which]->read();
	port &= 0xf0;
	return port >> 4;
}

void tecmo_state::rygar_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(tecmo_state::txvideoram_w)).share(m_txvideoram);
	map(0xd800, 0xdbff).ram().w(FUNC(tecmo_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xdc00, 0xdfff).ram().w(FUNC(tecmo_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xe000, 0xe7ff).ram().share(m_spriteram);
	map(0xe800, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xf800).portr("JOY1");
	map(0xf801, 0xf801).portr("BUTTONS1");
	map(0xf802, 0xf802).portr("JOY2");
	map(0xf803, 0xf803).portr("BUTTONS2");
	map(0xf804, 0xf804).portr("SYS_0");
	map(0xf805, 0xf805).portr("SYS_1");
	map(0xf806, 0xf806).r(FUNC(tecmo_state::dsw_l_r<0>));
	map(0xf807, 0xf807).r(FUNC(tecmo_state::dsw_h_r<0>));
	map(0xf808, 0xf808).r(FUNC(tecmo_state::dsw_l_r<1>));
	map(0xf809, 0xf809).r(FUNC(tecmo_state::dsw_h_r<1>));
	map(0xf80f, 0xf80f).portr("SYS_2");
	map(0xf800, 0xf802).w(FUNC(tecmo_state::fgscroll_w)).share(m_fgscroll);
	map(0xf803, 0xf805).w(FUNC(tecmo_state::bgscroll_w)).share(m_bgscroll);
	map(0xf806, 0xf806).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xf807, 0xf807).w(FUNC(tecmo_state::flipscreen_w));
	map(0xf808, 0xf808).w(FUNC(tecmo_state::bankswitch_w));
	map(0xf80b, 0xf80b).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

void tecmo_state::gemini_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(tecmo_state::txvideoram_w)).share(m_txvideoram);
	map(0xd800, 0xdbff).ram().w(FUNC(tecmo_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xdc00, 0xdfff).ram().w(FUNC(tecmo_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xe000, 0xe7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe800, 0xefff).ram().share(m_spriteram);
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xf800).portr("JOY1");
	map(0xf801, 0xf801).portr("BUTTONS1");
	map(0xf802, 0xf802).portr("JOY2");
	map(0xf803, 0xf803).portr("BUTTONS2");
	map(0xf804, 0xf804).portr("SYS_0");
	map(0xf805, 0xf805).portr("SYS_1");
	map(0xf806, 0xf806).r(FUNC(tecmo_state::dsw_l_r<0>));
	map(0xf807, 0xf807).r(FUNC(tecmo_state::dsw_h_r<0>));
	map(0xf808, 0xf808).r(FUNC(tecmo_state::dsw_l_r<1>));
	map(0xf809, 0xf809).r(FUNC(tecmo_state::dsw_h_r<1>));
	map(0xf80f, 0xf80f).portr("SYS_2");
	map(0xf800, 0xf802).w(FUNC(tecmo_state::fgscroll_w)).share(m_fgscroll);
	map(0xf803, 0xf805).w(FUNC(tecmo_state::bgscroll_w)).share(m_bgscroll);
	map(0xf806, 0xf806).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xf807, 0xf807).w(FUNC(tecmo_state::flipscreen_w));
	map(0xf808, 0xf808).w(FUNC(tecmo_state::bankswitch_w));
	map(0xf80b, 0xf80b).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

void tecmo_state::silkworm_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram().w(FUNC(tecmo_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xc400, 0xc7ff).ram().w(FUNC(tecmo_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xc800, 0xcfff).ram().w(FUNC(tecmo_state::txvideoram_w)).share(m_txvideoram);
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().share(m_spriteram);
	map(0xe800, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xf800).portr("JOY1");
	map(0xf801, 0xf801).portr("BUTTONS1");
	map(0xf802, 0xf802).portr("JOY2");
	map(0xf803, 0xf803).portr("BUTTONS2");
	map(0xf804, 0xf804).portr("SYS_0");
	map(0xf805, 0xf805).portr("SYS_1");
	map(0xf806, 0xf806).r(FUNC(tecmo_state::dsw_l_r<0>));
	map(0xf807, 0xf807).r(FUNC(tecmo_state::dsw_h_r<0>));
	map(0xf808, 0xf808).r(FUNC(tecmo_state::dsw_l_r<1>));
	map(0xf809, 0xf809).r(FUNC(tecmo_state::dsw_h_r<1>));
	map(0xf80e, 0xf80e).portr("SYS_3");
	map(0xf80f, 0xf80f).portr("SYS_2");
	map(0xf800, 0xf802).w(FUNC(tecmo_state::fgscroll_w)).share(m_fgscroll);
	map(0xf803, 0xf805).w(FUNC(tecmo_state::bgscroll_w)).share(m_bgscroll);
	map(0xf806, 0xf806).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xf807, 0xf807).w(FUNC(tecmo_state::flipscreen_w));
	map(0xf808, 0xf808).w(FUNC(tecmo_state::bankswitch_w));
	map(0xf809, 0xf809).nopw();    // ?
	map(0xf80b, 0xf80b).nopw();    // ? if mapped to watchdog like in the others, causes reset
}

void tecmo_state::rygar_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).w("ymsnd", FUNC(ym3526_device::write));
	map(0xc000, 0xc000).r("soundlatch", FUNC(generic_latch_8_device::read)).w(FUNC(tecmo_state::adpcm_start_w));
	map(0xd000, 0xd000).w(FUNC(tecmo_state::adpcm_end_w));
	map(0xe000, 0xe000).w(FUNC(tecmo_state::adpcm_vol_w));
	map(0xf000, 0xf000).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
}

void tecmo_state::silkwormp_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).w("ymsnd", FUNC(ym3812_device::write));
	map(0xc000, 0xc000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xcc00, 0xcc00).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
}

void tecmo_state::backfirt_sound_map(address_map &map)
{
	silkwormp_sound_map(map);
	map(0x2000, 0x207f).ram(); // Silkworm set #2 has a custom CPU which writes code to this area
}

void tecmo_state::tecmo_sound_map(address_map &map)
{
	backfirt_sound_map(map);
	map(0xc000, 0xc000).w(FUNC(tecmo_state::adpcm_start_w));
	map(0xc400, 0xc400).w(FUNC(tecmo_state::adpcm_end_w));
	map(0xc800, 0xc800).w(FUNC(tecmo_state::adpcm_vol_w));
}

static INPUT_PORTS_START( tecmo_default )
	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYS_0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYS_1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYS_2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYS_3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_HIGH, "SW1:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "SW1:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW1:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW1:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW1:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW1:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW1:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW1:!8" )

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_HIGH, "SW2:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "SW2:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW2:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW2:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW2:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW2:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW2:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW2:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( rygar )
	PORT_INCLUDE(tecmo_default)

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_MODIFY("SYS_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0C, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, "50000 200000 500000" )
	PORT_DIPSETTING(    0x01, "100000 300000 600000" )
	PORT_DIPSETTING(    0x02, "200000 500000" )
	PORT_DIPSETTING(    0x03, "100000" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, "2P Can Start Anytime" )  PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gemini )
	PORT_INCLUDE(tecmo_default)

	PORT_MODIFY("SYS_1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_MODIFY("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x00, "Final Round Continuation" )  PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, "Round 6" )
	PORT_DIPSETTING(    0x08, "Round 7" )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!5,!6,!7")
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x00, "Buy in During Final Round" ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!5,!6,!7")
	PORT_DIPSETTING(    0x00, "50000 200000" )
	PORT_DIPSETTING(    0x10, "50000 300000" )
	PORT_DIPSETTING(    0x20, "100000 500000" )
	PORT_DIPSETTING(    0x30, "50000" )
	PORT_DIPSETTING(    0x40, "100000" )
	PORT_DIPSETTING(    0x50, "200000" )
	PORT_DIPSETTING(    0x60, "300000" )
	PORT_DIPSETTING(    0x70, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( backfirt )
	PORT_INCLUDE(tecmo_default)

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("SYS_1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) ) // limit of 9?
	PORT_DIPNAME( 0x0C, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "50000 200000 500000" )
	PORT_DIPSETTING(    0x01, "100000 300000 800000" )
	PORT_DIPSETTING(    0x02, "50000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "100000" )
	PORT_DIPSETTING(    0x06, "200000" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!4,!5,!6")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPNAME( 0x40, 0x40, "Continue" )          PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Invulnerability" )       PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( silkworm )
	PORT_INCLUDE(tecmo_default)

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_MODIFY("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("SYS_2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0C, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "50000 200000 500000" )
	PORT_DIPSETTING(    0x01, "100000 300000 800000" )
	PORT_DIPSETTING(    0x02, "50000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "100000" )
	PORT_DIPSETTING(    0x06, "200000" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!5,!6,!7")
//  PORT_DIPSETTING(    0x60, "0" )             // Not listed in manual
//  PORT_DIPSETTING(    0x70, "0" )             // Not listed in manual
//  PORT_DIPSETTING(    0x00, "0" )             // Not listed in manual
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!8") // Listed as "NC" in manual
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END


// Bootleg/prototype sets don't have the "disable continue" feature (dip B, sw 8) and coin inputs are switched.
// They also have a "swap vehicle" feature which allows 1P to be the jeep and 2P to be the heli.
// Setting dip B, sw 7 on enables the feature and it is performed on the start screen with an extra input @ 1P button 4 (jamma pin 25, maps to 0xf80e bit 0).
// Seems this feature was removed for the production release of the game.
static INPUT_PORTS_START( silkwormp )
	PORT_INCLUDE(tecmo_default)

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_MODIFY("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("SYS_2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )   // Coin inputs are switched
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("SYS_3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )   // Vehicle swap extra button

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0C, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Allow Vehicle Swap" )    PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "50000 200000 500000" )
	PORT_DIPSETTING(    0x01, "100000 300000 800000" )
	PORT_DIPSETTING(    0x02, "50000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "100000" )
	PORT_DIPSETTING(    0x06, "200000" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x70, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!5,!6,!7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )          // from the Tecmo US manual
	PORT_DIPSETTING(    0x60, "3" )          //  (although conflicting info exists, so unverified)
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	// no allow continue setting
INPUT_PORTS_END


static GFXDECODE_START( gfx_tecmo )
	GFXDECODE_ENTRY( "txtiles", 0, gfx_8x8x4_packed_msb,               256, 16 )   // colors 256 - 511
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_row_2x2_group_packed_msb, 512, 16 )   // colors 512 - 767
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_row_2x2_group_packed_msb, 768, 16 )   // colors 768 - 1023
GFXDECODE_END

static GFXDECODE_START( gfx_tecmo_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 0, 16 )   // colors   0 - 255
GFXDECODE_END


void tecmo_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x800);

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_data));
}

void tecmo_state::machine_reset()
{
	m_adpcm_pos = 0;
	m_adpcm_end = 0;
	m_adpcm_data = -1;
}

void tecmo_state::rygar(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(24'000'000)/4); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &tecmo_state::rygar_map);
	m_maincpu->set_vblank_int("screen", FUNC(tecmo_state::irq0_line_hold));

	Z80(config, m_soundcpu, XTAL(4'000'000)); // verified on pcb
	m_soundcpu->set_addrmap(AS_PROGRAM, &tecmo_state::rygar_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000)/4, 384,0,256,264,16,240); // 59.18 Hz
	m_screen->set_screen_update(FUNC(tecmo_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tecmo);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024).set_endianness(ENDIANNESS_BIG);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_tecmo_spr);
	m_sprgen->set_pri_callback(FUNC(tecmo_state::pri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);
	soundlatch.set_separate_acknowledge(true);

	ym3526_device &ymsnd(YM3526(config, "ymsnd", XTAL(4'000'000))); // verified on pcb
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	MSM5205(config, m_msm, XTAL(400'000)); // verified on pcb, even if schematics shows a 384khz resonator
	m_msm->vck_legacy_callback().set(FUNC(tecmo_state::adpcm_int));    // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);      // 8KHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void tecmo_state::gemini(machine_config &config)
{
	rygar(config);

	// basic machine hardware
	// xtal found on bootleg, to be confirmed on a real board
	m_maincpu->set_clock(XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tecmo_state::gemini_map);

	m_soundcpu->set_addrmap(AS_PROGRAM, &tecmo_state::tecmo_sound_map);

	ym3812_device &ymsnd(YM3812(config.replace(), "ymsnd", XTAL(4'000'000)));
	ymsnd.irq_handler().set_inputline(m_soundcpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void tecmo_state::geminib(machine_config &config)
{
	gemini(config);
	// 24.18 MHz OSC / 59.62 Hz, bootleg only?
	m_screen->set_raw(24180000/4, 384,0,256,264,16,240);
}

void tecmo_state::silkworm(machine_config &config)
{
	gemini(config);

	// basic machine hardware
	m_maincpu->set_clock(8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tecmo_state::silkworm_map);
}

void tecmo_state::backfirt(machine_config &config)
{
	gemini(config);

	// this pcb has no MSM5205
	config.device_remove("msm");
	m_soundcpu->set_addrmap(AS_PROGRAM, &tecmo_state::backfirt_sound_map);
}

void tecmo_state::silkwormp(machine_config &config)
{
	silkworm(config);

	// bootleg pcb doesn't have the MSM5205 populated
	config.device_remove("msm");
	m_soundcpu->set_addrmap(AS_PROGRAM, &tecmo_state::silkwormp_sound_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rygar )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "5.5p",         0x00000, 0x08000, CRC(062cd55d) SHA1(656e29c890f5de964920b7841b3e11469cd20051) ) // code
	ROM_LOAD( "cpu_5m.bin",   0x08000, 0x04000, CRC(7ac5191b) SHA1(305f39d974f906f9bc24e9fe2ca58e647925ab63) ) // code
	ROM_LOAD( "cpu_5j.bin",   0x10000, 0x08000, CRC(ed76d606) SHA1(39c8a07e9a1f218ad088d00a2c9dfc993efafb6b) ) // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_4h.bin",   0x0000, 0x2000, CRC(e4a2fa87) SHA1(ed58187dbbcf59358496a98ffd6c227a87d6c433) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "cpu_8k.bin",   0x00000, 0x08000, CRC(4d482fb6) SHA1(57ad838b6d30b49dbd2d0ec425f33cfb15a67918) )  // characters

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "vid_6k.bin",   0x00000, 0x08000, CRC(aba6db9e) SHA1(43eb6f4f92afb5fbc11adc7e2ab04878ab56cb17) )  // sprites
	ROM_LOAD( "vid_6j.bin",   0x08000, 0x08000, CRC(ae1f2ed6) SHA1(6e6a33e665ba0884b7f57e9ad69d3f51e41d9e7b) )  // sprites
	ROM_LOAD( "vid_6h.bin",   0x10000, 0x08000, CRC(46d9e7df) SHA1(a24e0bea310a03636af704a0ad3f1a9cc4aafe12) )  // sprites
	ROM_LOAD( "vid_6g.bin",   0x18000, 0x08000, CRC(45839c9a) SHA1(eaee5767d8b0b62b991c089ef51b922e89850b79) )  // sprites

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "vid_6p.bin",   0x00000, 0x08000, CRC(9eae5f8e) SHA1(ed83b608ca57b9bf69fa866d9b8f55d16b7cff63) )
	ROM_LOAD( "vid_6o.bin",   0x08000, 0x08000, CRC(5a10a396) SHA1(12ebed3952ff35a2c275cb27c915f82183048cd4) )
	ROM_LOAD( "vid_6n.bin",   0x10000, 0x08000, CRC(7b12cf3f) SHA1(6b9d8cad6e15317df01bab0591fab09199ca6d40) )
	ROM_LOAD( "vid_6l.bin",   0x18000, 0x08000, CRC(3cea7eaa) SHA1(1dd194d5672dfe71c2b27d2d7b76f5a611cff76f) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "vid_6f.bin",   0x00000, 0x08000, CRC(9840edd8) SHA1(f19a1a1d932214037144c533ad07ed81256c34e7) )
	ROM_LOAD( "vid_6e.bin",   0x08000, 0x08000, CRC(ff65e074) SHA1(513c1bad336ef5d871f15d6ba8943020f98d1f4a) )
	ROM_LOAD( "vid_6c.bin",   0x10000, 0x08000, CRC(89868c85) SHA1(f21550f40e7a177e95c40f2726c651f85ca8edce) )
	ROM_LOAD( "vid_6b.bin",   0x18000, 0x08000, CRC(35389a7b) SHA1(a887a89f9bbb5979bb589468d80efba1f243690b) )

	ROM_REGION( 0x4000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "cpu_1f.bin",   0x0000, 0x4000, CRC(3cc98c5a) SHA1(ea1035be939ed1a994f3273b33412c85dda0973e) )
ROM_END

ROM_START( rygar2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "5p.bin",       0x00000, 0x08000, CRC(151ffc0b) SHA1(0eb877f2c68d3d1f52d7b12d0a8ad08c9932c054) ) // code
	ROM_LOAD( "cpu_5m.bin",   0x08000, 0x04000, CRC(7ac5191b) SHA1(305f39d974f906f9bc24e9fe2ca58e647925ab63) ) // code
	ROM_LOAD( "cpu_5j.bin",   0x10000, 0x08000, CRC(ed76d606) SHA1(39c8a07e9a1f218ad088d00a2c9dfc993efafb6b) ) // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_4h.bin",   0x0000, 0x2000, CRC(e4a2fa87) SHA1(ed58187dbbcf59358496a98ffd6c227a87d6c433) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "cpu_8k.bin",   0x00000, 0x08000, CRC(4d482fb6) SHA1(57ad838b6d30b49dbd2d0ec425f33cfb15a67918) )  // characters

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "vid_6k.bin",   0x00000, 0x08000, CRC(aba6db9e) SHA1(43eb6f4f92afb5fbc11adc7e2ab04878ab56cb17) )  // sprites
	ROM_LOAD( "vid_6j.bin",   0x08000, 0x08000, CRC(ae1f2ed6) SHA1(6e6a33e665ba0884b7f57e9ad69d3f51e41d9e7b) )  // sprites
	ROM_LOAD( "vid_6h.bin",   0x10000, 0x08000, CRC(46d9e7df) SHA1(a24e0bea310a03636af704a0ad3f1a9cc4aafe12) )  // sprites
	ROM_LOAD( "vid_6g.bin",   0x18000, 0x08000, CRC(45839c9a) SHA1(eaee5767d8b0b62b991c089ef51b922e89850b79) )  // sprites

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "vid_6p.bin",   0x00000, 0x08000, CRC(9eae5f8e) SHA1(ed83b608ca57b9bf69fa866d9b8f55d16b7cff63) )
	ROM_LOAD( "vid_6o.bin",   0x08000, 0x08000, CRC(5a10a396) SHA1(12ebed3952ff35a2c275cb27c915f82183048cd4) )
	ROM_LOAD( "vid_6n.bin",   0x10000, 0x08000, CRC(7b12cf3f) SHA1(6b9d8cad6e15317df01bab0591fab09199ca6d40) )
	ROM_LOAD( "vid_6l.bin",   0x18000, 0x08000, CRC(3cea7eaa) SHA1(1dd194d5672dfe71c2b27d2d7b76f5a611cff76f) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "vid_6f.bin",   0x00000, 0x08000, CRC(9840edd8) SHA1(f19a1a1d932214037144c533ad07ed81256c34e7) )
	ROM_LOAD( "vid_6e.bin",   0x08000, 0x08000, CRC(ff65e074) SHA1(513c1bad336ef5d871f15d6ba8943020f98d1f4a) )
	ROM_LOAD( "vid_6c.bin",   0x10000, 0x08000, CRC(89868c85) SHA1(f21550f40e7a177e95c40f2726c651f85ca8edce) )
	ROM_LOAD( "vid_6b.bin",   0x18000, 0x08000, CRC(35389a7b) SHA1(a887a89f9bbb5979bb589468d80efba1f243690b) )

	ROM_REGION( 0x4000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "cpu_1f.bin",   0x0000, 0x4000, CRC(3cc98c5a) SHA1(ea1035be939ed1a994f3273b33412c85dda0973e) )
ROM_END

// There is a known bootleg board which uses U locations but without Tecmo etchings which is a match for rygar3
ROM_START( rygar3 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cpu_5p.bin",   0x00000, 0x08000, CRC(e79c054a) SHA1(1aaffa53d121d5c55899bf18e85c42333fe0df54) ) // code
	ROM_LOAD( "cpu_5m.bin",   0x08000, 0x04000, CRC(7ac5191b) SHA1(305f39d974f906f9bc24e9fe2ca58e647925ab63) ) // code
	ROM_LOAD( "cpu_5j.bin",   0x10000, 0x08000, CRC(ed76d606) SHA1(39c8a07e9a1f218ad088d00a2c9dfc993efafb6b) ) // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_4h.bin",   0x0000, 0x2000, CRC(e4a2fa87) SHA1(ed58187dbbcf59358496a98ffd6c227a87d6c433) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "cpu_8k.bin",   0x00000, 0x08000, CRC(4d482fb6) SHA1(57ad838b6d30b49dbd2d0ec425f33cfb15a67918) )  // characters

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "vid_6k.bin",   0x00000, 0x08000, CRC(aba6db9e) SHA1(43eb6f4f92afb5fbc11adc7e2ab04878ab56cb17) )  // sprites
	ROM_LOAD( "vid_6j.bin",   0x08000, 0x08000, CRC(ae1f2ed6) SHA1(6e6a33e665ba0884b7f57e9ad69d3f51e41d9e7b) )  // sprites
	ROM_LOAD( "vid_6h.bin",   0x10000, 0x08000, CRC(46d9e7df) SHA1(a24e0bea310a03636af704a0ad3f1a9cc4aafe12) )  // sprites
	ROM_LOAD( "vid_6g.bin",   0x18000, 0x08000, CRC(45839c9a) SHA1(eaee5767d8b0b62b991c089ef51b922e89850b79) )  // sprites

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "vid_6p.bin",   0x00000, 0x08000, CRC(9eae5f8e) SHA1(ed83b608ca57b9bf69fa866d9b8f55d16b7cff63) )
	ROM_LOAD( "vid_6o.bin",   0x08000, 0x08000, CRC(5a10a396) SHA1(12ebed3952ff35a2c275cb27c915f82183048cd4) )
	ROM_LOAD( "vid_6n.bin",   0x10000, 0x08000, CRC(7b12cf3f) SHA1(6b9d8cad6e15317df01bab0591fab09199ca6d40) )
	ROM_LOAD( "vid_6l.bin",   0x18000, 0x08000, CRC(3cea7eaa) SHA1(1dd194d5672dfe71c2b27d2d7b76f5a611cff76f) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "vid_6f.bin",   0x00000, 0x08000, CRC(9840edd8) SHA1(f19a1a1d932214037144c533ad07ed81256c34e7) )
	ROM_LOAD( "vid_6e.bin",   0x08000, 0x08000, CRC(ff65e074) SHA1(513c1bad336ef5d871f15d6ba8943020f98d1f4a) )
	ROM_LOAD( "vid_6c.bin",   0x10000, 0x08000, CRC(89868c85) SHA1(f21550f40e7a177e95c40f2726c651f85ca8edce) )
	ROM_LOAD( "vid_6b.bin",   0x18000, 0x08000, CRC(35389a7b) SHA1(a887a89f9bbb5979bb589468d80efba1f243690b) )

	ROM_REGION( 0x4000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "cpu_1f.bin",   0x0000, 0x4000, CRC(3cc98c5a) SHA1(ea1035be939ed1a994f3273b33412c85dda0973e) )
ROM_END

ROM_START( rygarj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cpuj_5p.bin",  0x00000, 0x08000, CRC(b39698ba) SHA1(01a5a12a71973ad117b0bbd763e470f89c439e45) ) // code
	ROM_LOAD( "cpuj_5m.bin",  0x08000, 0x04000, CRC(3f180979) SHA1(c4c2e9f83b06b8677978800bfcc39f4ba3b344ab) ) // code
	ROM_LOAD( "cpuj_5j.bin",  0x10000, 0x08000, CRC(69e44e8f) SHA1(e979760a3582e64788c043adf7e475f0e1b75033) ) // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_4h.bin",   0x0000, 0x2000, CRC(e4a2fa87) SHA1(ed58187dbbcf59358496a98ffd6c227a87d6c433) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "cpuj_8k.bin",  0x00000, 0x08000, CRC(45047707) SHA1(deb47f5ec4b22e55e0393d8108e4ffb67dd68e12) )  // characters

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "vid_6k.bin",   0x00000, 0x08000, CRC(aba6db9e) SHA1(43eb6f4f92afb5fbc11adc7e2ab04878ab56cb17) )  // sprites
	ROM_LOAD( "vid_6j.bin",   0x08000, 0x08000, CRC(ae1f2ed6) SHA1(6e6a33e665ba0884b7f57e9ad69d3f51e41d9e7b) )  // sprites
	ROM_LOAD( "vid_6h.bin",   0x10000, 0x08000, CRC(46d9e7df) SHA1(a24e0bea310a03636af704a0ad3f1a9cc4aafe12) )  // sprites
	ROM_LOAD( "vid_6g.bin",   0x18000, 0x08000, CRC(45839c9a) SHA1(eaee5767d8b0b62b991c089ef51b922e89850b79) )  // sprites

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "vid_6p.bin",   0x00000, 0x08000, CRC(9eae5f8e) SHA1(ed83b608ca57b9bf69fa866d9b8f55d16b7cff63) )
	ROM_LOAD( "vid_6o.bin",   0x08000, 0x08000, CRC(5a10a396) SHA1(12ebed3952ff35a2c275cb27c915f82183048cd4) )
	ROM_LOAD( "vid_6n.bin",   0x10000, 0x08000, CRC(7b12cf3f) SHA1(6b9d8cad6e15317df01bab0591fab09199ca6d40) )
	ROM_LOAD( "vid_6l.bin",   0x18000, 0x08000, CRC(3cea7eaa) SHA1(1dd194d5672dfe71c2b27d2d7b76f5a611cff76f) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "vid_6f.bin",   0x00000, 0x08000, CRC(9840edd8) SHA1(f19a1a1d932214037144c533ad07ed81256c34e7) )
	ROM_LOAD( "vid_6e.bin",   0x08000, 0x08000, CRC(ff65e074) SHA1(513c1bad336ef5d871f15d6ba8943020f98d1f4a) )
	ROM_LOAD( "vid_6c.bin",   0x10000, 0x08000, CRC(89868c85) SHA1(f21550f40e7a177e95c40f2726c651f85ca8edce) )
	ROM_LOAD( "vid_6b.bin",   0x18000, 0x08000, CRC(35389a7b) SHA1(a887a89f9bbb5979bb589468d80efba1f243690b) )

	ROM_REGION( 0x4000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "cpu_1f.bin",   0x0000, 0x4000, CRC(3cc98c5a) SHA1(ea1035be939ed1a994f3273b33412c85dda0973e) )
ROM_END

ROM_START( rygarj2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "5.5p",         0x00000, 0x08000, CRC(c5d9af81) SHA1(accdad907631fc3e831cf369bca84a0e25943638) ) // code
	ROM_LOAD( "4.5m",         0x08000, 0x04000, CRC(af5d4a2a) SHA1(f3644a9bbea4df099d9a9aa48ac2c41a12c473f6) ) // code
	ROM_LOAD( "cpuj_5j.bin",  0x10000, 0x08000, CRC(69e44e8f) SHA1(e979760a3582e64788c043adf7e475f0e1b75033) ) // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_4h.bin",   0x0000, 0x2000, CRC(e4a2fa87) SHA1(ed58187dbbcf59358496a98ffd6c227a87d6c433) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "cpuj_8k.bin",  0x00000, 0x08000, CRC(45047707) SHA1(deb47f5ec4b22e55e0393d8108e4ffb67dd68e12) )  // characters

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "vid_6k.bin",   0x00000, 0x08000, CRC(aba6db9e) SHA1(43eb6f4f92afb5fbc11adc7e2ab04878ab56cb17) )  // sprites
	ROM_LOAD( "vid_6j.bin",   0x08000, 0x08000, CRC(ae1f2ed6) SHA1(6e6a33e665ba0884b7f57e9ad69d3f51e41d9e7b) )  // sprites
	ROM_LOAD( "vid_6h.bin",   0x10000, 0x08000, CRC(46d9e7df) SHA1(a24e0bea310a03636af704a0ad3f1a9cc4aafe12) )  // sprites
	ROM_LOAD( "vid_6g.bin",   0x18000, 0x08000, CRC(45839c9a) SHA1(eaee5767d8b0b62b991c089ef51b922e89850b79) )  // sprites

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "vid_6p.bin",   0x00000, 0x08000, CRC(9eae5f8e) SHA1(ed83b608ca57b9bf69fa866d9b8f55d16b7cff63) )
	ROM_LOAD( "vid_6o.bin",   0x08000, 0x08000, CRC(5a10a396) SHA1(12ebed3952ff35a2c275cb27c915f82183048cd4) )
	ROM_LOAD( "vid_6n.bin",   0x10000, 0x08000, CRC(7b12cf3f) SHA1(6b9d8cad6e15317df01bab0591fab09199ca6d40) )
	ROM_LOAD( "vid_6l.bin",   0x18000, 0x08000, CRC(3cea7eaa) SHA1(1dd194d5672dfe71c2b27d2d7b76f5a611cff76f) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "vid_6f.bin",   0x00000, 0x08000, CRC(9840edd8) SHA1(f19a1a1d932214037144c533ad07ed81256c34e7) )
	ROM_LOAD( "vid_6e.bin",   0x08000, 0x08000, CRC(ff65e074) SHA1(513c1bad336ef5d871f15d6ba8943020f98d1f4a) )
	ROM_LOAD( "vid_6c.bin",   0x10000, 0x08000, CRC(89868c85) SHA1(f21550f40e7a177e95c40f2726c651f85ca8edce) )
	ROM_LOAD( "vid_6b.bin",   0x18000, 0x08000, CRC(35389a7b) SHA1(a887a89f9bbb5979bb589468d80efba1f243690b) )

	ROM_REGION( 0x4000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "cpu_1f.bin",   0x0000, 0x4000, CRC(3cc98c5a) SHA1(ea1035be939ed1a994f3273b33412c85dda0973e) )
ROM_END

ROM_START( silkworm )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "4.5s",   0x00000, 0x10000, CRC(a5277cce) SHA1(3886a3f3d1230d49d541f884c5b29938e13f98c8) )  // c000-ffff is not used
	ROM_LOAD( "5.6s",   0x10000, 0x10000, CRC(a6c7bb51) SHA1(75f6625459ab65f2d47a282c1295d4db38f5fe51) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "3.5j",   0x0000, 0x8000, CRC(b589f587) SHA1(0be5e2bf3daf3e28d63fdc8c89bb6fe7c48c6c3f) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "2.3j",   0x00000, 0x08000, CRC(e80a1cd9) SHA1(ef16feb1113acc7401f8951158b25f6f201196f2) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "6.1c",   0x00000, 0x10000, CRC(1138d159) SHA1(3b938606d448c4effdfe414bbf495b50cc3bc1c1) )  // sprites
	ROM_LOAD( "7.1d",   0x10000, 0x10000, CRC(d96214f7) SHA1(a5b2be3ae6a6eb8afef2c18c865a998fbf4adf93) )  // sprites
	ROM_LOAD( "8.1f",   0x20000, 0x10000, CRC(0494b38e) SHA1(03255f153824056e430a0b8595103f3b58b1fd97) )  // sprites
	ROM_LOAD( "9.1h",   0x30000, 0x10000, CRC(8ce3cdf5) SHA1(635248514c4e1e5aab7a2ed4d620a5b970d4a43a) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "10.1p",  0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )  // tiles #1
	ROM_LOAD( "11.12p", 0x10000, 0x10000, CRC(6c03c476) SHA1(79ad800a2f4ba6d44ba5a31210cbd8566bb357b6) )  // tiles #1
	ROM_LOAD( "12.2p",  0x20000, 0x10000, CRC(bb0f568f) SHA1(b66c6d0407ed0b068c6bf07987f1b923d4a6e4f8) )  // tiles #1
	ROM_LOAD( "13.3p",  0x30000, 0x10000, CRC(773ad0a4) SHA1(f7576e1ac8c779b33d7ec393555fd097a34257fa) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "14.1s",  0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )  // tiles #2
	ROM_LOAD( "15.12s", 0x10000, 0x10000, CRC(6e4052c9) SHA1(e2e3d7221b75cb044449a25a076a93c3def1f11b) )  // tiles #2
	ROM_LOAD( "16.2s",  0x20000, 0x10000, CRC(9292ed63) SHA1(70aa46fcc187b8200c5d246870e2e2dc4b2985cb) )  // tiles #2
	ROM_LOAD( "17.3s",  0x30000, 0x10000, CRC(3fa4563d) SHA1(46e3cc41491d63efcdda43c84c7ac1385a1926d0) )  // tiles #2

	ROM_REGION( 0x8000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "1.6b",   0x0000, 0x8000, CRC(5b553644) SHA1(5d39d2251094c17f7b732b4861401b3516fce9b1) )
ROM_END

ROM_START( silkwormj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "silkwormj.4",  0x00000, 0x10000, CRC(6df3df22) SHA1(9d6201c2df014bdb6877dfff936dddde1fe6fbd0) )  // c000-ffff is not used
	ROM_LOAD( "silkworm.5",   0x10000, 0x10000, CRC(a6c7bb51) SHA1(75f6625459ab65f2d47a282c1295d4db38f5fe51) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "silkwormj.3",    0x0000, 0x8000, CRC(b79848d0) SHA1(d8162ab847bd0768572454d9775b0e9ed92b9519) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "silkworm.2",   0x00000, 0x08000, CRC(e80a1cd9) SHA1(ef16feb1113acc7401f8951158b25f6f201196f2) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "silkworm.6",   0x00000, 0x10000, CRC(1138d159) SHA1(3b938606d448c4effdfe414bbf495b50cc3bc1c1) )  // sprites
	ROM_LOAD( "silkworm.7",   0x10000, 0x10000, CRC(d96214f7) SHA1(a5b2be3ae6a6eb8afef2c18c865a998fbf4adf93) )  // sprites
	ROM_LOAD( "silkworm.8",   0x20000, 0x10000, CRC(0494b38e) SHA1(03255f153824056e430a0b8595103f3b58b1fd97) )  // sprites
	ROM_LOAD( "silkworm.9",   0x30000, 0x10000, CRC(8ce3cdf5) SHA1(635248514c4e1e5aab7a2ed4d620a5b970d4a43a) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "silkworm.10",  0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )  // tiles #1
	ROM_LOAD( "silkworm.11",  0x10000, 0x10000, CRC(6c03c476) SHA1(79ad800a2f4ba6d44ba5a31210cbd8566bb357b6) )  // tiles #1
	ROM_LOAD( "silkworm.12",  0x20000, 0x10000, CRC(bb0f568f) SHA1(b66c6d0407ed0b068c6bf07987f1b923d4a6e4f8) )  // tiles #1
	ROM_LOAD( "silkworm.13",  0x30000, 0x10000, CRC(773ad0a4) SHA1(f7576e1ac8c779b33d7ec393555fd097a34257fa) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "silkworm.14",  0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )  // tiles #2
	ROM_LOAD( "silkworm.15",  0x10000, 0x10000, CRC(6e4052c9) SHA1(e2e3d7221b75cb044449a25a076a93c3def1f11b) )  // tiles #2
	ROM_LOAD( "silkworm.16",  0x20000, 0x10000, CRC(9292ed63) SHA1(70aa46fcc187b8200c5d246870e2e2dc4b2985cb) )  // tiles #2
	ROM_LOAD( "silkworm.17",  0x30000, 0x10000, CRC(3fa4563d) SHA1(46e3cc41491d63efcdda43c84c7ac1385a1926d0) )  // tiles #2

	ROM_REGION( 0x8000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "silkworm.1",   0x0000, 0x8000, CRC(5b553644) SHA1(5d39d2251094c17f7b732b4861401b3516fce9b1) )
ROM_END

// 6217A
// SILKWORM H T737
// board have Japanese label "ADONO"
// this set shows "AD" (revision?) on title screen
// sound cpu isn't attempting to use samples, so removed the parent rom reference, have to assume it doesn't exist like the other proto set
ROM_START( silkwormp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "silkworm_pr4ma.4",  0x00000, 0x10000, CRC(5e2a39cc) SHA1(e2fb0fa2d4e3d439935b7814c8572224eddf271e) )  // c000-ffff is not used
	ROM_LOAD( "silkworm.5",        0x10000, 0x10000, CRC(a6c7bb51) SHA1(75f6625459ab65f2d47a282c1295d4db38f5fe51) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "silkworm_sound.3",    0x0000, 0x8000, CRC(c67c5644) SHA1(0963eda467dbc18806a4f0a9525a093d2fcb82fb) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "sw.2",   0x00000, 0x08000, CRC(1acc54be) SHA1(b210e4c0753bc84171ca418f3fcf07f0e6965390) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "silkworm.6",   0x00000, 0x10000, CRC(1138d159) SHA1(3b938606d448c4effdfe414bbf495b50cc3bc1c1) )  // sprites
	ROM_LOAD( "silkworm.7",   0x10000, 0x10000, CRC(d96214f7) SHA1(a5b2be3ae6a6eb8afef2c18c865a998fbf4adf93) )  // sprites
	ROM_LOAD( "silkworm.8",   0x20000, 0x10000, CRC(0494b38e) SHA1(03255f153824056e430a0b8595103f3b58b1fd97) )  // sprites
	ROM_LOAD( "silkworm.9",   0x30000, 0x10000, CRC(8ce3cdf5) SHA1(635248514c4e1e5aab7a2ed4d620a5b970d4a43a) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "silkworm.10",  0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )  // tiles #1
	ROM_LOAD( "silkworm.11",  0x10000, 0x10000, CRC(6c03c476) SHA1(79ad800a2f4ba6d44ba5a31210cbd8566bb357b6) )  // tiles #1
	ROM_LOAD( "silkworm.12",  0x20000, 0x10000, CRC(bb0f568f) SHA1(b66c6d0407ed0b068c6bf07987f1b923d4a6e4f8) )  // tiles #1
	ROM_LOAD( "silkworm.13",  0x30000, 0x10000, CRC(773ad0a4) SHA1(f7576e1ac8c779b33d7ec393555fd097a34257fa) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "silkworm.14",  0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )  // tiles #2
	ROM_LOAD( "silkworm.15",  0x10000, 0x10000, CRC(6e4052c9) SHA1(e2e3d7221b75cb044449a25a076a93c3def1f11b) )  // tiles #2
	ROM_LOAD( "silkworm.16",  0x20000, 0x10000, CRC(9292ed63) SHA1(70aa46fcc187b8200c5d246870e2e2dc4b2985cb) )  // tiles #2
	ROM_LOAD( "silkworm.17",  0x30000, 0x10000, CRC(3fa4563d) SHA1(46e3cc41491d63efcdda43c84c7ac1385a1926d0) )  // tiles #2
ROM_END

/*
 markings:  CPU board:   HE 202 (silkscreen)  SK-50 (sticker)
            video board: HE 203 (silkscreen)
 main cpu:  zilog Z8400BPS Z80B @ 6MHz  8609
 sound cpu: sharp LH0080A  Z80A @ 4MHz  8705
 this set shows "GF" (revision?) on title screen
 this seems to be a bootleg of a prototype, not sure if it's the other proto set hacked or another rev proto
 some of the tile roms are half size but the original has a lot of unused data so nothing is missing in game.
 the various Mitsubishi custom chips present on original board are implemented with standard ttl chips:
   MN50005XTA on the main board is 11 ttl chips
   M60002-0118P on the video board is 27 ttl chips
   MA7053 sil module on the main board is 4 ttl chips
   these are all properly integrated into the design of the boards, not plug-in sub/daughter boards.
*/
ROM_START( silkwormb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "e3.4",         0x00000, 0x10000, CRC(3d86fd58) SHA1(7245186259e08bda33a7dc0d5f895f847c94519b) )
	ROM_LOAD( "silkworm.5",   0x10000, 0x10000, CRC(a6c7bb51) SHA1(75f6625459ab65f2d47a282c1295d4db38f5fe51) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "e2.3",         0x0000,  0x8000,  CRC(b7a3fb80) SHA1(de52ef3c8b22f083816a42cbf239e8f9dbee2424) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "silkworm.2",   0x00000, 0x08000, CRC(e80a1cd9) SHA1(ef16feb1113acc7401f8951158b25f6f201196f2) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "silkworm.6",   0x00000, 0x10000, CRC(1138d159) SHA1(3b938606d448c4effdfe414bbf495b50cc3bc1c1) )  // sprites
	ROM_LOAD( "silkworm.7",   0x10000, 0x10000, CRC(d96214f7) SHA1(a5b2be3ae6a6eb8afef2c18c865a998fbf4adf93) )  // sprites
	ROM_LOAD( "silkworm.8",   0x20000, 0x10000, CRC(0494b38e) SHA1(03255f153824056e430a0b8595103f3b58b1fd97) )  // sprites
	ROM_LOAD( "silkworm.9",   0x30000, 0x10000, CRC(8ce3cdf5) SHA1(635248514c4e1e5aab7a2ed4d620a5b970d4a43a) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "silkworm.10",  0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )  // fg tiles TMM24512
	ROM_LOAD( "e10.11",       0x10000, 0x08000, CRC(c0c4687d) SHA1(afe05eb7e5a65c995aeac9ea773ad79eb053303f) )  // fg tiles TMM24256
	ROM_LOAD( "silkworm.12",  0x20000, 0x10000, CRC(bb0f568f) SHA1(b66c6d0407ed0b068c6bf07987f1b923d4a6e4f8) )  // fg tiles TMM24512
	ROM_LOAD( "e12.13",       0x30000, 0x08000, CRC(fc472811) SHA1(e862ec9b38f3f3a1f4668fbc587063eee8e9e821) )  // fg tiles 27C256  

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "silkworm.14",  0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )  // bg tiles TMM24512
	ROM_LOAD( "e14.15",       0x10000, 0x08000, CRC(b02acdb6) SHA1(6be74bb89680b79b3a5d13af638ed5a0bb077dad) )  // bg tiles 27C256  
	ROM_LOAD( "e15.16",       0x20000, 0x08000, CRC(caf7b25e) SHA1(2c348af9d03efd801cbbc06deb02869bd6449518) )  // bg tiles 27C256  
	ROM_LOAD( "e16.17",       0x38000, 0x08000, CRC(7ec93873) SHA1(0993a3b3e5ca84ef0ea32159825e379ba4cc5fbb) )  // bg tiles 27C256  
ROM_END

ROM_START( silkwormb2 ) // 2-PCB stack, no markings
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "280100_pc-4.4", 0x00000, 0x10000, CRC(a10f2414) SHA1(5dbad60582d3193802c7c5e727c9c2685dec27b0) ) // unique, seems quite different wrt to the other dumped sets
	ROM_LOAD( "280100_pc-5.5", 0x10000, 0x10000, CRC(a6c7bb51) SHA1(75f6625459ab65f2d47a282c1295d4db38f5fe51) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "280100_pc-3.3", 0x00000, 0x08000, CRC(5a880df9) SHA1(fc3f78ea05571ecf127fd1a6d3c6c349e300967a) ) // unique

	ROM_REGION( 0x08000, "txtiles", 0 ) // characters
	ROM_LOAD( "280100_pc-2.2", 0x00000, 0x08000, CRC(e80a1cd9) SHA1(ef16feb1113acc7401f8951158b25f6f201196f2) )

	ROM_REGION( 0x40000, "sprites", 0 ) // sprites
	ROM_LOAD( "280100_pc-6.6", 0x00000, 0x10000, CRC(1138d159) SHA1(3b938606d448c4effdfe414bbf495b50cc3bc1c1) )
	ROM_LOAD( "280100_pc-7.7", 0x10000, 0x10000, CRC(d96214f7) SHA1(a5b2be3ae6a6eb8afef2c18c865a998fbf4adf93) )
	ROM_LOAD( "280100_pc-8.8", 0x20000, 0x10000, CRC(0494b38e) SHA1(03255f153824056e430a0b8595103f3b58b1fd97) )
	ROM_LOAD( "280100_pc-9.9", 0x30000, 0x10000, CRC(8ce3cdf5) SHA1(635248514c4e1e5aab7a2ed4d620a5b970d4a43a) )

	ROM_REGION( 0x40000, "fgtiles", 0 ) // fg tiles
	ROM_LOAD( "280100_pc-10.10", 0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )
	ROM_LOAD( "280100_pc-11.11", 0x10000, 0x08000, CRC(c0c4687d) SHA1(afe05eb7e5a65c995aeac9ea773ad79eb053303f) )
	ROM_LOAD( "280100_pc-12.12", 0x20000, 0x10000, CRC(bb0f568f) SHA1(b66c6d0407ed0b068c6bf07987f1b923d4a6e4f8) )
	ROM_LOAD( "280100_pc-13.13", 0x30000, 0x08000, CRC(fc472811) SHA1(e862ec9b38f3f3a1f4668fbc587063eee8e9e821) )

	ROM_REGION( 0x40000, "bgtiles", 0 ) // bg tiles
	ROM_LOAD( "280100_pc-14.14", 0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )
	ROM_LOAD( "280100_pc-15.15", 0x10000, 0x08000, CRC(b02acdb6) SHA1(6be74bb89680b79b3a5d13af638ed5a0bb077dad) )
	ROM_LOAD( "280100_pc-16.16", 0x20000, 0x08000, CRC(caf7b25e) SHA1(2c348af9d03efd801cbbc06deb02869bd6449518) )
	ROM_LOAD( "280100_pc-17.17", 0x38000, 0x08000, CRC(7ec93873) SHA1(0993a3b3e5ca84ef0ea32159825e379ba4cc5fbb) )
ROM_END

/*

main cpu Z80A
sound cpu Z80A* see note
sound ic ym3812 + 3014
Osc 8Mhz and 24Mhz

*Note:The sound cpu was protected inside a epoxy block fit on a 40 pin socket in reverse of cpu board (solder side).
 By dissolving resin the small sub pcb contains Z80A (identified by pins),76c28 (6116),a 74ls00 and 74ls138.

ROMs    B4,B5 main program
B2 sound program
B3 character gfx
B6 to B9 object gfx
B10 to B13 foreground gfx
B14 to B17 background gfx
All eprom/rom are 27256,27512

RAMs:
-ram (cpu board):
6264 main/work
6116 (sub pcb*),6116, 2114 x3 sound
6116 character
-ram (video board):
6116 scroll
6116 foreground
6116 background
4164 x20 object/sprites

*/

ROM_START( backfirt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b5-e3.bin",    0x00000, 0x10000, CRC(0ab3bd4d) SHA1(2653d099c894304d3f9c2b2de9a7fed67be7b6dc) )  // c000-ffff is not used
	ROM_LOAD( "b4-f3.bin",    0x10000, 0x10000, CRC(150b6949) SHA1(31870a2f471b71d79a4daa0b5baca0d941de12e4) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "b2-e10.bin",   0x00000, 0x08000, CRC(9b2ac54f) SHA1(7c10e00235dc2668dee5c97ea5c6dc7722f35f03) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "b3-c10.bin",   0x00000, 0x08000, CRC(08ce729f) SHA1(8e426251b20edfb10f0837b3106b4f333bc114a4) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "b6-c2.bin",   0x00000, 0x10000, CRC(c8c25e45) SHA1(d771d5e7d2d8082680f73b778ef2d88f2e9b8591) )   // sprites
	ROM_LOAD( "b7-d2.bin",   0x10000, 0x10000, CRC(25fb6a57) SHA1(7f411af7417fa901d65194c348ecec58c61b7cf7) )   // sprites
	ROM_LOAD( "b8-e2.bin",   0x20000, 0x10000, CRC(6bccac4e) SHA1(e042d049761affe4d3d0eac3c7a24f428643a9cf) )   // sprites
	ROM_LOAD( "b9-h2.bin",   0x30000, 0x10000, CRC(566a99b8) SHA1(a78825f0a85235399e66906cffafda98445a89a2) )   // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "b13-p1.bin",  0x00000, 0x10000, CRC(8c7138bb) SHA1(0cfd69fa77d5b546f7dad80537d8d2497ae758bc) )   // tiles #1
	ROM_LOAD( "b12-p2.bin",  0x10000, 0x10000, CRC(6c03c476) SHA1(79ad800a2f4ba6d44ba5a31210cbd8566bb357b6) )   // tiles #1
	ROM_LOAD( "b11-p2.bin",  0x20000, 0x10000, CRC(0bc84b4b) SHA1(599041108d09fd61aab2b0aeac0e07715887476c) )   // tiles #1
	ROM_LOAD( "b10-p3.bin",  0x30000, 0x10000, CRC(ec149ec3) SHA1(7817dc2659fe4ba3bb810df278378d51d97065b3) )   // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "b17-s1.bin",  0x00000, 0x10000, CRC(409df64b) SHA1(cada970bf9cc8f6522e7a71e00fe873568852873) )   // tiles #2
	ROM_LOAD( "b16-s2.bin",  0x10000, 0x10000, CRC(6e4052c9) SHA1(e2e3d7221b75cb044449a25a076a93c3def1f11b) )   // tiles #2
	ROM_LOAD( "b15-s2.bin",  0x20000, 0x10000, CRC(2b6cc20e) SHA1(4815819288753400935836cc1b0b69f4c4b43ddc) )   // tiles #2
	ROM_LOAD( "b14-s3.bin",  0x30000, 0x08000, CRC(4d29637a) SHA1(28e85925138256b8ce5a1c4a5df5b219b1b6b197) )   // tiles #2 // half size is correct, rom type 27256
ROM_END

ROM_START( gemini )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "4-5s",  0x00000, 0x10000, CRC(ce71e27a) SHA1(5aac0434edd39e444687d9988c7f49e0752a4900) )  // c000-ffff is not used
	ROM_LOAD( "5-6s",  0x10000, 0x10000, CRC(216784a9) SHA1(ec74a3753e5b6384b875125ec0db7beea0a6aa6d) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gw03-5h.rom",  0x0000, 0x8000, CRC(9bc79596) SHA1(61de9ddd45140e8ed88173294bd26147e2abfa21) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "gw02-3h.rom",  0x00000, 0x08000, CRC(7acc8d35) SHA1(05056e9f077e7571b314390b508c72d56ad0f43b) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gw06-1c.rom",  0x00000, 0x10000, CRC(4ea51631) SHA1(9aee0f1ba210ac953dc193cfc739322966b6de8a) )  // sprites
	ROM_LOAD( "gw07-1d.rom",  0x10000, 0x10000, CRC(da42637e) SHA1(9885c52823279f26871092c77bdbe027df08268f) )  // sprites
	ROM_LOAD( "gw08-1f.rom",  0x20000, 0x10000, CRC(0b4e8d70) SHA1(55069f3df1c8db83f306d46b8262fd23585e6013) )  // sprites
	ROM_LOAD( "gw09-1h.rom",  0x30000, 0x10000, CRC(b65c5e4c) SHA1(699e1a9e72b8d94edae7382ba119fe5da113514d) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "gw10-1n.rom",  0x00000, 0x10000, CRC(5e84cd4f) SHA1(e85320291027a16619c87fc2365448367bda454a) )  // tiles #1
	ROM_LOAD( "gw11-2na.rom", 0x10000, 0x10000, CRC(08b458e1) SHA1(b3426faa57dca51dc053db44fa4968425d8bf3ee) )  // tiles #1
	ROM_LOAD( "gw12-2nb.rom", 0x20000, 0x10000, CRC(229c9714) SHA1(f4f47d6b379c973c22f9ae7d7bec7041cdf3f737) )  // tiles #1
	ROM_LOAD( "gw13-3n.rom",  0x30000, 0x10000, CRC(c5dfaf47) SHA1(c3202ca8c7f3c5c7dc9acdc09c1c894e168ef9fe) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "gw14-1r.rom",  0x00000, 0x10000, CRC(9c10e5b5) SHA1(a81399b85d8f3ddca26883ec3535cb9044c35ada) )  // tiles #2
	ROM_LOAD( "gw15-2ra.rom", 0x10000, 0x10000, CRC(4cd18cfa) SHA1(c197a098a7c1e5220aad039383a40702fe7c4f21) )  // tiles #2
	ROM_LOAD( "gw16-2rb.rom", 0x20000, 0x10000, CRC(f911c7be) SHA1(3f49f6c4734f2b644d93c4a54249aae6ff080e1d) )  // tiles #2
	ROM_LOAD( "gw17-3r.rom",  0x30000, 0x10000, CRC(79a9ce25) SHA1(74e3917b8e7a920ceb2135d7ef8fb2f2c5176b21) )  // tiles #2

	ROM_REGION( 0x8000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "gw01-6a.rom",  0x0000, 0x8000, CRC(d78afa05) SHA1(b02a739b045f5cddf943ce59226ef234463eeebe) )
ROM_END

ROM_START( geminij )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gw04-5s.rom",  0x00000, 0x10000, CRC(ff9de855) SHA1(34167af8456a081f68b338f10d4319ce1e703fd4) )  // c000-ffff is not used
	ROM_LOAD( "gw05-6s.rom",  0x10000, 0x10000, CRC(5a6947a9) SHA1(18b7aeb0f0e2c396bc759118dd7c45fd6070b804) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gw03-5h.rom",  0x0000, 0x8000, CRC(9bc79596) SHA1(61de9ddd45140e8ed88173294bd26147e2abfa21) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "gw02-3h.rom",  0x00000, 0x08000, CRC(7acc8d35) SHA1(05056e9f077e7571b314390b508c72d56ad0f43b) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gw06-1c.rom",  0x00000, 0x10000, CRC(4ea51631) SHA1(9aee0f1ba210ac953dc193cfc739322966b6de8a) )  // sprites
	ROM_LOAD( "gw07-1d.rom",  0x10000, 0x10000, CRC(da42637e) SHA1(9885c52823279f26871092c77bdbe027df08268f) )  // sprites
	ROM_LOAD( "gw08-1f.rom",  0x20000, 0x10000, CRC(0b4e8d70) SHA1(55069f3df1c8db83f306d46b8262fd23585e6013) )  // sprites
	ROM_LOAD( "gw09-1h.rom",  0x30000, 0x10000, CRC(b65c5e4c) SHA1(699e1a9e72b8d94edae7382ba119fe5da113514d) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "gw10-1n.rom",  0x00000, 0x10000, CRC(5e84cd4f) SHA1(e85320291027a16619c87fc2365448367bda454a) )  // tiles #1
	ROM_LOAD( "gw11-2na.rom", 0x10000, 0x10000, CRC(08b458e1) SHA1(b3426faa57dca51dc053db44fa4968425d8bf3ee) )  // tiles #1
	ROM_LOAD( "gw12-2nb.rom", 0x20000, 0x10000, CRC(229c9714) SHA1(f4f47d6b379c973c22f9ae7d7bec7041cdf3f737) )  // tiles #1
	ROM_LOAD( "gw13-3n.rom",  0x30000, 0x10000, CRC(c5dfaf47) SHA1(c3202ca8c7f3c5c7dc9acdc09c1c894e168ef9fe) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "gw14-1r.rom",  0x00000, 0x10000, CRC(9c10e5b5) SHA1(a81399b85d8f3ddca26883ec3535cb9044c35ada) )  // tiles #2
	ROM_LOAD( "gw15-2ra.rom", 0x10000, 0x10000, CRC(4cd18cfa) SHA1(c197a098a7c1e5220aad039383a40702fe7c4f21) )  // tiles #2
	ROM_LOAD( "gw16-2rb.rom", 0x20000, 0x10000, CRC(f911c7be) SHA1(3f49f6c4734f2b644d93c4a54249aae6ff080e1d) )  // tiles #2
	ROM_LOAD( "gw17-3r.rom",  0x30000, 0x10000, CRC(79a9ce25) SHA1(74e3917b8e7a920ceb2135d7ef8fb2f2c5176b21) )  // tiles #2

	ROM_REGION( 0x8000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "gw01-6a.rom",  0x0000, 0x8000, CRC(d78afa05) SHA1(b02a739b045f5cddf943ce59226ef234463eeebe) )
ROM_END

/*
 hsync is 15.742kHz
 vsync is 59.629Hz
 hsync pulse is 5.3uS
 vsync pulse is 8 raster lines

The non matching EPROM is a modified version of gw04-5s.rom with the following changes:
- offset 0x0A4A contains 0x6F instead of 0x1F
- offset 0x0A4E contains 0xAA instead of 0xFA
 */

ROM_START( geminib )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "g-2.6d",       0x00000, 0x10000, CRC(cd79c5b3) SHA1(355aae2346d49d14a801fad05d49376581d329c6) )  // c000-ffff is not used
	ROM_LOAD( "gw05-6s.rom",  0x10000, 0x10000, CRC(5a6947a9) SHA1(18b7aeb0f0e2c396bc759118dd7c45fd6070b804) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gw03-5h.rom",  0x0000, 0x8000, CRC(9bc79596) SHA1(61de9ddd45140e8ed88173294bd26147e2abfa21) )

	ROM_REGION( 0x08000, "txtiles", 0 )
	ROM_LOAD( "gw02-3h.rom",  0x00000, 0x08000, CRC(7acc8d35) SHA1(05056e9f077e7571b314390b508c72d56ad0f43b) )  // characters

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "gw06-1c.rom",  0x00000, 0x10000, CRC(4ea51631) SHA1(9aee0f1ba210ac953dc193cfc739322966b6de8a) )  // sprites
	ROM_LOAD( "gw07-1d.rom",  0x10000, 0x10000, CRC(da42637e) SHA1(9885c52823279f26871092c77bdbe027df08268f) )  // sprites
	ROM_LOAD( "gw08-1f.rom",  0x20000, 0x10000, CRC(0b4e8d70) SHA1(55069f3df1c8db83f306d46b8262fd23585e6013) )  // sprites
	ROM_LOAD( "gw09-1h.rom",  0x30000, 0x10000, CRC(b65c5e4c) SHA1(699e1a9e72b8d94edae7382ba119fe5da113514d) )  // sprites

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "gw10-1n.rom",  0x00000, 0x10000, CRC(5e84cd4f) SHA1(e85320291027a16619c87fc2365448367bda454a) )  // tiles #1
	ROM_LOAD( "gw11-2na.rom", 0x10000, 0x10000, CRC(08b458e1) SHA1(b3426faa57dca51dc053db44fa4968425d8bf3ee) )  // tiles #1
	ROM_LOAD( "gw12-2nb.rom", 0x20000, 0x10000, CRC(229c9714) SHA1(f4f47d6b379c973c22f9ae7d7bec7041cdf3f737) )  // tiles #1
	ROM_LOAD( "gw13-3n.rom",  0x30000, 0x10000, CRC(c5dfaf47) SHA1(c3202ca8c7f3c5c7dc9acdc09c1c894e168ef9fe) )  // tiles #1

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "gw14-1r.rom",  0x00000, 0x10000, CRC(9c10e5b5) SHA1(a81399b85d8f3ddca26883ec3535cb9044c35ada) )  // tiles #2
	ROM_LOAD( "gw15-2ra.rom", 0x10000, 0x10000, CRC(4cd18cfa) SHA1(c197a098a7c1e5220aad039383a40702fe7c4f21) )  // tiles #2
	ROM_LOAD( "gw16-2rb.rom", 0x20000, 0x10000, CRC(f911c7be) SHA1(3f49f6c4734f2b644d93c4a54249aae6ff080e1d) )  // tiles #2
	ROM_LOAD( "gw17-3r.rom",  0x30000, 0x10000, CRC(79a9ce25) SHA1(74e3917b8e7a920ceb2135d7ef8fb2f2c5176b21) )  // tiles #2

	ROM_REGION( 0x8000, "adpcm", 0 )    // ADPCM samples
	ROM_LOAD( "gw01-6a.rom",  0x0000, 0x8000, CRC(d78afa05) SHA1(b02a739b045f5cddf943ce59226ef234463eeebe) )
ROM_END

/*
   video_type is used to distinguish Rygar, Silkworm and Gemini Wing.
   This is needed because there is a difference in the tile and sprite indexing.
*/
void tecmo_state::init_rygar()
{
	m_video_type = 0;
}

void tecmo_state::init_silkworm()
{
	m_video_type = 1;
}

void tecmo_state::init_gemini()
{
	m_video_type = 2;
}


GAME( 1986, rygar,      0,        rygar,     rygar,     tecmo_state, init_rygar,    ROT0,  "Tecmo",   "Rygar (US set 1)",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, rygar2,     rygar,    rygar,     rygar,     tecmo_state, init_rygar,    ROT0,  "Tecmo",   "Rygar (US set 2)",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, rygar3,     rygar,    rygar,     rygar,     tecmo_state, init_rygar,    ROT0,  "Tecmo",   "Rygar (US set 3 Old Version)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, rygarj,     rygar,    rygar,     rygar,     tecmo_state, init_rygar,    ROT0,  "Tecmo",   "Argus no Senshi (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, rygarj2,    rygar,    rygar,     rygar,     tecmo_state, init_rygar,    ROT0,  "Tecmo",   "Argus no Senshi (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, gemini,     0,        gemini,    gemini,    tecmo_state, init_gemini,   ROT90, "Tecmo",   "Gemini Wing (World)",           MACHINE_SUPPORTS_SAVE ) // No regional "Warning, if you are playing ..." screen
GAME( 1987, geminij,    gemini,   gemini,    gemini,    tecmo_state, init_gemini,   ROT90, "Tecmo",   "Gemini Wing (Japan)",           MACHINE_SUPPORTS_SAVE ) // Japan regional warning screen
GAME( 1987, geminib,    gemini,   geminib,   gemini,    tecmo_state, init_gemini,   ROT90, "bootleg", "Gemini Wing (bootleg)",         MACHINE_SUPPORTS_SAVE ) // regional warning screen is blanked (still get a delay)
GAME( 1988, silkworm,   0,        silkworm,  silkworm,  tecmo_state, init_silkworm, ROT0,  "Tecmo",   "Silk Worm (World)",             MACHINE_SUPPORTS_SAVE ) // No regional "Warning, if you are playing ..." screen
GAME( 1988, silkwormj,  silkworm, silkworm,  silkworm,  tecmo_state, init_silkworm, ROT0,  "Tecmo",   "Silk Worm (Japan)",             MACHINE_SUPPORTS_SAVE ) // Japan regional warning screen
GAME( 1988, silkwormp,  silkworm, silkwormp, silkwormp, tecmo_state, init_silkworm, ROT0,  "Tecmo",   "Silk Worm (prototype)",         MACHINE_SUPPORTS_SAVE ) // prototype
GAME( 1988, silkwormb,  silkworm, silkwormp, silkwormp, tecmo_state, init_silkworm, ROT0,  "bootleg", "Silk Worm (bootleg, set 1)",    MACHINE_SUPPORTS_SAVE ) // bootleg of (a different?) prototype
GAME( 1988, silkwormb2, silkworm, silkwormp, silkwormp, tecmo_state, init_silkworm, ROT0,  "bootleg", "Silk Worm (bootleg, set 2)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, backfirt,   0,        backfirt,  backfirt,  tecmo_state, init_gemini,   ROT0,  "Tecmo",   "Back Fire (Tecmo, bootleg)",    MACHINE_SUPPORTS_SAVE )
