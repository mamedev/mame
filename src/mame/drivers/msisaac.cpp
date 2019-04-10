// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/****************************************************************************

    Metal Soldier Isaac II  (c) Taito 1985

    driver by Jarek Burczynski

****************************************************************************/

#include "emu.h"
#include "includes/msisaac.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


/*
TO DO:
  - sprites are probably banked differently (no way to be sure until MCU dump is available)
  - TA7630 emulation needs filter support (characteristics depend on the frequency)
  - TA7630 volume table is hand tuned to match the sample, but still slightly off.
*/


TIMER_CALLBACK_MEMBER(msisaac_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(msisaac_state::sound_command_w)
{
	m_soundlatch->write(data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(msisaac_state::nmi_callback),this), data);
}

WRITE8_MEMBER(msisaac_state::nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(msisaac_state::nmi_enable_w)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_pending_nmi = 0;
	}
}

#if 0
WRITE8_MEMBER(msisaac_state::flip_screen_w)
{
	flip_screen_set(data);
}

WRITE8_MEMBER(msisaac_state::msisaac_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset,data);
}
#endif

WRITE8_MEMBER(msisaac_state::ms_unknown_w)
{
	if (data != 0x08)
		popmessage("CPU #0 write to 0xf0a3 data=%2x", data);
}





/* If good MCU dump will be available, it should be fully working game */
/* To test the game without the MCU simply comment out #define USE_MCU */



READ8_MEMBER(msisaac_state::msisaac_mcu_r)
{
#ifdef USE_MCU
	return m_bmcu->buggychl_mcu_r(offset);
#else
/*
MCU simulation TODO:
\-Find the command which allows player-2 to play.
\-Fix some graphics imperfections(*not* confirmed if they are caused by unhandled
  commands or imperfect video emulation).
*/

	switch (m_mcu_val)
	{
		/*Start-up check*/
		case 0x5f:  return (m_mcu_val + 0x6b);
		/*These interferes with RAM operations(setting them to non-zero you  *
		 * will have unexpected results,such as infinite lives or score not  *
		 * incremented properly).*/
		case 0x40:
		case 0x41:
		case 0x42:
			return 0;

		/*With this command the MCU controls body direction  */
		case 0x02:
		{
			//direction:
			//0-left
			//1-leftup
			//2-up
			//3-rigtup
			//4-right
			//5-rightdwn
			//6-down
			//7-leftdwn

			uint8_t val= (ioport("IN1")->read() >> 2) & 0x0f;
			/* bit0 = left
			   bit1 = right
			   bit2 = down
			   bit3 = up
			*/
			/* direction is encoded as:
			                   4
			                 3   5
			                2     6
			                 1   7
			                   0
			*/
			/*       0000   0001   0010   0011      0100   0101   0110   0111     1000   1001   1010   1011   1100   1101   1110   1111 */
			/*      nochange left  right nochange   down downlft dwnrght down     up     upleft uprgt  up    nochnge left   right  nochange */

			static const int8_t table[16] = { -1,    2,    6,     -1,       0,   1,      7,      0,       4,     3,     5,    4,     -1,     2,     6,    -1 };

			if (table[val] >= 0)
				m_direction = table[val];

			return m_direction;
		}

		/*This controls the arms when they return to the player.            */
		case 0x07:
			return 0x45;

		default:
			logerror("CPU#0 read from MCU pc=%4x, mcu_val=%2x\n", m_maincpu->pc(), m_mcu_val);
			return m_mcu_val;
	}
#endif
}

READ8_MEMBER(msisaac_state::msisaac_mcu_status_r)
{
#ifdef USE_MCU
	return m_bmcu->buggychl_mcu_status_r(offset);
#else
	return 3;   //mcu ready / cpu data ready
#endif
}

WRITE8_MEMBER(msisaac_state::msisaac_mcu_w)
{
#ifdef USE_MCU
	m_bmcu->buggychl_mcu_w(offset,data);
#else
	//if(data != 0x0a && data != 0x42 && data != 0x02)
	//  popmessage("PC = %04x %02x", m_maincpu->pc(), data);
	m_mcu_val = data;
#endif
}

void msisaac_state::msisaac_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf000).w(FUNC(msisaac_state::msisaac_bg2_textbank_w));
	map(0xf001, 0xf001).nopw();                    //???
	map(0xf002, 0xf002).nopw();                    //???

	map(0xf060, 0xf060).w(FUNC(msisaac_state::sound_command_w));      //sound command
	map(0xf061, 0xf061).nopw(); /*sound_reset*/    //????

	map(0xf0a3, 0xf0a3).w(FUNC(msisaac_state::ms_unknown_w));         //???? written in interrupt routine

	map(0xf0c0, 0xf0c0).w(FUNC(msisaac_state::msisaac_fg_scrollx_w));
	map(0xf0c1, 0xf0c1).w(FUNC(msisaac_state::msisaac_fg_scrolly_w));
	map(0xf0c2, 0xf0c2).w(FUNC(msisaac_state::msisaac_bg2_scrollx_w));
	map(0xf0c3, 0xf0c3).w(FUNC(msisaac_state::msisaac_bg2_scrolly_w));
	map(0xf0c4, 0xf0c4).w(FUNC(msisaac_state::msisaac_bg_scrollx_w));
	map(0xf0c5, 0xf0c5).w(FUNC(msisaac_state::msisaac_bg_scrolly_w));

	map(0xf0e0, 0xf0e0).rw(FUNC(msisaac_state::msisaac_mcu_r), FUNC(msisaac_state::msisaac_mcu_w));
	map(0xf0e1, 0xf0e1).r(FUNC(msisaac_state::msisaac_mcu_status_r));

	map(0xf080, 0xf080).portr("DSW1");
	map(0xf081, 0xf081).portr("DSW2");
	map(0xf082, 0xf082).portr("DSW3");
	map(0xf083, 0xf083).portr("IN0");
	map(0xf084, 0xf084).portr("IN1");
//  AM_RANGE(0xf086, 0xf086) AM_READ_PORT("IN2")

	map(0xf100, 0xf17f).ram().share("spriteram");   //sprites
	map(0xf400, 0xf7ff).ram().w(FUNC(msisaac_state::msisaac_fg_videoram_w)).share("videoram");
	map(0xf800, 0xfbff).ram().w(FUNC(msisaac_state::msisaac_bg2_videoram_w)).share("videoram3");
	map(0xfc00, 0xffff).ram().w(FUNC(msisaac_state::msisaac_bg_videoram_w)).share("videoram2");
//  AM_RANGE(0xf801, 0xf801) AM_WRITE(msisaac_bgcolor_w)
//  AM_RANGE(0xfc00, 0xfc00) AM_WRITE(flip_screen_w)
//  AM_RANGE(0xfc03, 0xfc04) AM_WRITE(msisaac_coin_counter_w)
}

WRITE8_MEMBER(msisaac_state::sound_control_0_w)
{
	m_snd_ctrl0 = data & 0xff;
	//popmessage("SND0 0=%2x 1=%2x", m_snd_ctrl0, m_snd_ctrl1);

	for(int i=0;i<4;i++)
	{
		// group1
		m_ta7630->set_channel_volume(m_msm,i,   m_snd_ctrl0 & 0xf);
		// group2
		m_ta7630->set_channel_volume(m_msm,i+4, m_snd_ctrl0 >> 4);
	}
//  m_msm->set_output_gain(0, m_vol_ctrl[m_snd_ctrl0 & 15] / 100.0);    /* group1 from msm5232 */
//  m_msm->set_output_gain(1, m_vol_ctrl[m_snd_ctrl0 & 15] / 100.0);    /* group1 from msm5232 */
//  m_msm->set_output_gain(2, m_vol_ctrl[m_snd_ctrl0 & 15] / 100.0);    /* group1 from msm5232 */
//  m_msm->set_output_gain(3, m_vol_ctrl[m_snd_ctrl0 & 15] / 100.0);    /* group1 from msm5232 */
//  m_msm->set_output_gain(4, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group2 from msm5232 */
//  m_msm->set_output_gain(5, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group2 from msm5232 */
//  m_msm->set_output_gain(6, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group2 from msm5232 */
//  m_msm->set_output_gain(7, m_vol_ctrl[(m_snd_ctrl0 >> 4) & 15] / 100.0); /* group2 from msm5232 */
}
WRITE8_MEMBER(msisaac_state::sound_control_1_w)
{
	m_snd_ctrl1 = data & 0xff;
	//popmessage("SND1 0=%2x 1=%2x", m_snd_ctrl0, m_snd_ctrl1);
}

void msisaac_state::msisaac_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x8002, 0x8003).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x8010, 0x801d).w(m_msm, FUNC(msm5232_device::write));
	map(0x8020, 0x8020).w(FUNC(msisaac_state::sound_control_0_w));
	map(0x8030, 0x8030).w(FUNC(msisaac_state::sound_control_1_w));
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc001, 0xc001).w(FUNC(msisaac_state::nmi_enable_w));
	map(0xc002, 0xc002).w(FUNC(msisaac_state::nmi_disable_w));
	map(0xc003, 0xc003).nopw(); /*???*/ /* this is NOT mixer_enable */
	map(0xe000, 0xffff).nopr(); /*space for diagnostic ROM (not dumped, not reachable) */
}

static INPUT_PORTS_START( msisaac )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1 Unknown 0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW1 Unknown 1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPNAME( 0x20, 0x00, "DSW1 Unknown 5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW1 Unknown 6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1 Unknown 7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW3 Unknown 1" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x02, "02" )
	PORT_DIPNAME( 0x04, 0x04, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW3 Unknown 3" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x08, "08" )
	PORT_DIPNAME( 0x30, 0x00, "Copyright Notice" )
	PORT_DIPSETTING(    0x00, "(C) 1985 Taito Corporation" )
	PORT_DIPSETTING(    0x10, "(C) Taito Corporation" )
	PORT_DIPSETTING(    0x20, "(C) Taito Corp. MCMLXXXV" )
	PORT_DIPSETTING(    0x30, "(C) Taito Corporation" )
	PORT_DIPNAME( 0x40, 0x00, "Coinage Display" )
	PORT_DIPSETTING(    0x40, "Insert Coin" )
	PORT_DIPSETTING(    0x00, "Coins/Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )   //??
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   //??
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   //??

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


static const gfx_layout char_layout =
{
	8,8,
	0x400,
	4,
	{ 0*0x2000*8, 1*0x2000*8, 2*0x2000*8, 3*0x2000*8 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tile_layout =
{
	16,16,
	0x100,
	4,
	{ 0*0x2000*8, 1*0x2000*8, 2*0x2000*8, 3*0x2000*8 },
	{ 7,6,5,4,3,2,1,0,  64+7,64+6,64+5,64+4,64+3,64+2,64+1,64+0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_msisaac )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0, 64 )
GFXDECODE_END


/*******************************************************************************/

void msisaac_state::machine_start()
{
	/* video */
	save_item(NAME(m_bg2_textbank));
	/* sound */
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_snd_ctrl0));
	save_item(NAME(m_snd_ctrl1));

#ifdef USE_MCU
#else
	save_item(NAME(m_mcu_val));
	save_item(NAME(m_direction));
#endif
}

void msisaac_state::machine_reset()
{
	//MACHINE_RESET_CALL_MEMBER(ta7630);

	/* video */
	m_bg2_textbank = 0;
	/* sound */
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_snd_ctrl0 = 0;
	m_snd_ctrl1 = 0;

#ifdef USE_MCU
	m_mcu->set_input_line(0, CLEAR_LINE);
#else
	m_mcu_val = 0;
	m_direction = 0;
#endif
}

void msisaac_state::msisaac(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &msisaac_state::msisaac_map);
	m_maincpu->set_vblank_int("screen", FUNC(msisaac_state::irq0_line_hold));

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &msisaac_state::msisaac_sound_map);
	m_audiocpu->set_vblank_int("screen", FUNC(msisaac_state::irq0_line_hold));    /* source of IRQs is unknown */

#ifdef USE_MCU
	M68705(config, "mcu", 8000000/2).set_addrmap(AS_PROGRAM, &msisaac_state::buggychl_mcu_map);  /* 4 MHz */
	BUGGYCHL_MCU(config, m_bmcu, 0);
#endif

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(msisaac_state::screen_update_msisaac));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_msisaac);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	TA7630(config, m_ta7630);

	AY8910(config, "ay1", 2000000).add_route(ALL_OUTPUTS, "mono", 0.15);
	// port A/B likely to be TA7630 filters

	AY8910(config, "ay2", 2000000).add_route(ALL_OUTPUTS, "mono", 0.15);
	// port A/B likely to be TA7630 filters

	MSM5232(config, m_msm, 2000000);
	m_msm->set_capacitors(0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6); /* 0.65 (???) uF capacitors (match the sample, not verified) */
	m_msm->add_route(0, "mono", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "mono", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "mono", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "mono", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "mono", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "mono", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "mono", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "mono", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
}


/*******************************************************************************/

ROM_START( msisaac )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 main CPU */
	ROM_LOAD( "a34_11.bin", 0x0000, 0x4000, CRC(40819334) SHA1(65352607165043909a09e96c07f7060f6ce087e6) )
	ROM_LOAD( "a34_12.bin", 0x4000, 0x4000, CRC(4c50b298) SHA1(5962882ad37ba6990ba2a6312b570f214cd4c103) )
	ROM_LOAD( "a34_13.bin", 0x8000, 0x4000, CRC(2e2b09b3) SHA1(daa715282ed9ef2e519e252a684ef28085becabd) )
	ROM_LOAD( "a34_10.bin", 0xc000, 0x2000, CRC(a2c53dc1) SHA1(14f23511f92bcfc94447dabe2826555d68bc1caa) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a34_01.bin", 0x0000, 0x4000, CRC(545e45e7) SHA1(18ddb1ec8809bb62ae1c1068cd16cd3c933bf6ba) )

	ROM_REGION( 0x0800,  "cpu2", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "a34.mcu"       , 0x0000, 0x0800, NO_DUMP )

// I tried following MCUs; none of them work with this game:
//  ROM_LOAD( "a30-14"    , 0x0000, 0x0800, CRC(c4690279) ) //40love
//  ROM_LOAD( "a22-19.31",  0x0000, 0x0800, CRC(06a71df0) )     //buggy challenge
//  ROM_LOAD( "a45-19",     0x0000, 0x0800, CRC(5378253c) )     //flstory
//  ROM_LOAD( "a54-19",     0x0000, 0x0800, CRC(e08b8846) )     //lkage

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a34_02.bin", 0x0000, 0x2000, CRC(50da1a81) SHA1(8aa5a896f3e1173155d4574f5e1c2703e334cf44) )
	ROM_LOAD( "a34_03.bin", 0x2000, 0x2000, CRC(728a549e) SHA1(8969569d4b7a3ba7b740dbd236c047a46b723617) )
	ROM_LOAD( "a34_04.bin", 0x4000, 0x2000, CRC(e7d19f1c) SHA1(d55ee8085256c1f6a254d3249997326eebba7d88) )
	ROM_LOAD( "a34_05.bin", 0x6000, 0x2000, CRC(bed2107d) SHA1(83b16ca8a1b131aa6a2976cdbe907109750eaf71) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a34_06.bin", 0x0000, 0x2000, CRC(4ec71687) SHA1(e88f0c61a172fbca1784c95246776bf64c071bf7) )
	ROM_LOAD( "a34_07.bin", 0x2000, 0x2000, CRC(24922abf) SHA1(e42b4947b8c84bdf62990205308b8c187352d001) )
	ROM_LOAD( "a34_08.bin", 0x4000, 0x2000, CRC(3ddbf4c0) SHA1(7dd82aba661addd0a905bc185c1a6d7f2e21e0c6) )
	ROM_LOAD( "a34_09.bin", 0x6000, 0x2000, CRC(23eb089d) SHA1(fcf48862825bf09ba3718cbade0e163a660e1a68) )

ROM_END


GAME( 1985, msisaac, 0,     msisaac, msisaac, msisaac_state, empty_init, ROT270, "Taito Corporation", "Metal Soldier Isaac II", MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
