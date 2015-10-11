// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/***************************************************************************
  GOINDOL

  Driver provided by Jarek Parchanski (jpdev@friko6.onet.pl)

Notes:
- byte at 7f87 controls region:
  0 = Japan
  1 = USA
  2 = World
  Regardless of the setting of this byte, the startup notice in Korean is
  always displayed.
  After the title screen, depending on the byte you get "for use only in Japan",
  "for use only in USA", or the Korean notice again! So 2 might actually mean
  Korea instead of World... but that version surely got to Europe since Gerald
  has three boards with this ROM.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/goindol.h"


WRITE8_MEMBER(goindol_state::goindol_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);

	if (m_char_bank != ((data & 0x10) >> 4))
	{
		m_char_bank = (data & 0x10) >> 4;
		machine().tilemap().mark_all_dirty();
	}

	flip_screen_set(data & 0x20);
}



READ8_MEMBER(goindol_state::prot_f422_r)
{
	/* bit 7 = vblank? */
	m_prot_toggle ^= 0x80;

	return m_prot_toggle;
}


WRITE8_MEMBER(goindol_state::prot_fc44_w)
{
	logerror("%04x: prot_fc44_w(%02x)\n", space.device().safe_pc(), data);
	m_ram[0x0419] = 0x5b;
	m_ram[0x041a] = 0x3f;
	m_ram[0x041b] = 0x6d;
}

WRITE8_MEMBER(goindol_state::prot_fd99_w)
{
	logerror("%04x: prot_fd99_w(%02x)\n", space.device().safe_pc(), data);
	m_ram[0x0421] = 0x3f;
}

WRITE8_MEMBER(goindol_state::prot_fc66_w)
{
	logerror("%04x: prot_fc66_w(%02x)\n", space.device().safe_pc(), data);
	m_ram[0x0423] = 0x06;
}

WRITE8_MEMBER(goindol_state::prot_fcb0_w)
{
	logerror("%04x: prot_fcb0_w(%02x)\n", space.device().safe_pc(), data);
	m_ram[0x0425] = 0x06;
}



static ADDRESS_MAP_START( goindol_map, AS_PROGRAM, 8, goindol_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xc800, 0xc800) AM_READNOP AM_WRITE(soundlatch_byte_w) // watchdog?
	AM_RANGE(0xc810, 0xc810) AM_WRITE(goindol_bankswitch_w)
	AM_RANGE(0xc820, 0xc820) AM_READ_PORT("DIAL")
	AM_RANGE(0xc820, 0xd820) AM_WRITEONLY AM_SHARE("fg_scrolly")
	AM_RANGE(0xc830, 0xc830) AM_READ_PORT("P1")
	AM_RANGE(0xc830, 0xd830) AM_WRITEONLY AM_SHARE("fg_scrollx")
	AM_RANGE(0xc834, 0xc834) AM_READ_PORT("P2")
	AM_RANGE(0xd000, 0xd03f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd040, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(goindol_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xe03f) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xe040, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(goindol_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW1")
	AM_RANGE(0xf422, 0xf422) AM_READ(prot_f422_r)
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("DSW2")
	AM_RANGE(0xfc44, 0xfc44) AM_WRITE(prot_fc44_w)
	AM_RANGE(0xfc66, 0xfc66) AM_WRITE(prot_fc66_w)
	AM_RANGE(0xfcb0, 0xfcb0) AM_WRITE(prot_fcb0_w)
	AM_RANGE(0xfd99, 0xfd99) AM_WRITE(prot_fd99_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, goindol_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("ymsnd", ym2203_device, write)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd800, 0xd800) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( goindol )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("DIAL")      /* spinner */
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, "Very Very Easy" )
	PORT_DIPSETTING(    0x14, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x04, "30k and every 50k" )
	PORT_DIPSETTING(    0x05, "50k and every 100k" )
	PORT_DIPSETTING(    0x06, "50k and every 200k" )
	PORT_DIPSETTING(    0x07, "100k and every 200k" )
	PORT_DIPSETTING(    0x01, "10000 only" )
	PORT_DIPSETTING(    0x02, "30000 only" )
	PORT_DIPSETTING(    0x03, "50000 only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( homo )
	PORT_INCLUDE( goindol )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{  RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( goindol )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0, 32 )
GFXDECODE_END



void goindol_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_char_bank));
	save_item(NAME(m_prot_toggle));
}

void goindol_state::machine_reset()
{
	m_char_bank = 0;
	m_prot_toggle = 0;
}

static MACHINE_CONFIG_START( goindol, goindol_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)  /* XTAL confirmed, divisor is not */
	MCFG_CPU_PROGRAM_MAP(goindol_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", goindol_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/2) /* XTAL confirmed, divisor is not */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(goindol_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(goindol_state, screen_update_goindol)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", goindol)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8)   /* Confirmed pitch from recording */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goindol )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1w", 0x00000, 0x8000, CRC(df77c502) SHA1(15d111e38d63a8a800fbf5f15c4fb72efb0e5cf4) ) /* Code 0000-7fff */
	ROM_LOAD( "r2",  0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) /* Paged data */
	ROM_LOAD( "r3",  0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) /* Paged data */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) ) /* Characters */
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) /* palette red bits   */
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) /* palette green bits */
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) /* palette blue bits  */
ROM_END

ROM_START( goindolu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1", 0x00000, 0x8000, CRC(3111c61b) SHA1(6cc3834f946566646f06efe0b65c4704574ec6f1) ) /* Code 0000-7fff */
	ROM_LOAD( "r2", 0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) /* Paged data */
	ROM_LOAD( "r3", 0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) /* Paged data */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) ) /* Characters */
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) /* palette red bits   */
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) /* palette green bits */
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) /* palette blue bits  */
ROM_END

ROM_START( goindolk )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1j", 0x00000, 0x8000, CRC(dde33ad3) SHA1(23cdb3494f5eeaeae2657a0101d5827aa32c526d) ) /* Code 0000-7fff */
	ROM_LOAD( "r2",  0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) /* Paged data */
	ROM_LOAD( "r3",  0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) /* Paged data */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) ) /* Characters */
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) /* palette red bits   */
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) /* palette green bits */
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) /* palette blue bits  */
ROM_END

ROM_START( homo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "homo.01", 0x00000, 0x8000, CRC(28c539ad) SHA1(64e950a4238a5656a9e0d0a699a6545da8c59548) ) /* Code 0000-7fff */
	ROM_LOAD( "r2", 0x10000, 0x8000, CRC(1ff6e3a2) SHA1(321d32b5236f8fadc55b00412081cd17fbdb42bf) ) /* Paged data */
	ROM_LOAD( "r3", 0x18000, 0x8000, CRC(e9eec24a) SHA1(d193dd23b8bee3a788114e6bb86902dddf6fdd99) ) /* Paged data */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r10", 0x00000, 0x8000, CRC(72e1add1) SHA1(e8bdaffbbbf8ed22eb161cb8d7945ff09420f68f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "r4", 0x00000, 0x8000, CRC(1ab84225) SHA1(47494d03fb8d153335203155e61d90108db62961) ) /* Characters */
	ROM_LOAD( "r5", 0x08000, 0x8000, CRC(4997d469) SHA1(60c482b2408079bc8b2ffb86bc01927d5cad66ea) )
	ROM_LOAD( "r6", 0x10000, 0x8000, CRC(752904b0) SHA1(6ff44bd45b000bccae4fd67eefce936aacd971fc) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "r7", 0x00000, 0x8000, CRC(362f2a27) SHA1(9b8232a9ce7d752a749897fb2231a005c734239d) )
	ROM_LOAD( "r8", 0x08000, 0x8000, CRC(9fc7946e) SHA1(89100fae14826ad4f6735770827cbfe97562038c) )
	ROM_LOAD( "r9", 0x10000, 0x8000, CRC(e6212fe4) SHA1(f42b5ddbdb6599ba4ff5e6ef7d86e55f58a671b6) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "am27s21.pr1", 0x0000, 0x0100, CRC(361f0868) SHA1(aea681a2e168aca327a998db7b537c7b82dbc433) ) /* palette red bits   */
	ROM_LOAD( "am27s21.pr2", 0x0100, 0x0100, CRC(e355da4d) SHA1(40ebdbf6519b2817402ea716aae838c315da4fcb) ) /* palette green bits */
	ROM_LOAD( "am27s21.pr3", 0x0200, 0x0100, CRC(8534cfb5) SHA1(337b6d5e9ceb2116aea73a7a4ac7e70716460323) ) /* palette blue bits  */
ROM_END



DRIVER_INIT_MEMBER(goindol_state,goindol)
{
	UINT8 *rom = memregion("maincpu")->base();


	/* I hope that's all patches to avoid protection */

	rom[0x18e9] = 0x18; // ROM 1 check
	rom[0x1964] = 0x00; // ROM 9 error (MCU?)
	rom[0x1965] = 0x00; //
	rom[0x1966] = 0x00; //
//  rom[0x17c7] = 0x00; // c421 == 3f
//  rom[0x17c8] = 0x00; //
//  rom[0x16f0] = 0x18; // c425 == 06
//  rom[0x172c] = 0x18; // c423 == 06
//  rom[0x1779] = 0x00; // c419 == 5b 3f 6d
//  rom[0x177a] = 0x00; //
	rom[0x063f] = 0x18; //->fc55
	rom[0x0b30] = 0x00; // verify code at 0601-064b
	rom[0x1bdf] = 0x18; //->fc49

	rom[0x04a7] = 0xc9;
	rom[0x0831] = 0xc9;
	rom[0x3365] = 0x00; // verify code at 081d-0876
	rom[0x0c13] = 0xc9;
	rom[0x134e] = 0xc9;
	rom[0x333d] = 0xc9;
}



GAME( 1987, goindol,  0,       goindol, goindol, goindol_state, goindol, ROT90, "SunA",    "Goindol (World)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, goindolu, goindol, goindol, goindol, goindol_state, goindol, ROT90, "SunA",    "Goindol (US)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, goindolk, goindol, goindol, goindol, goindol_state, goindol, ROT90, "SunA",    "Goindol (Korea)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1987, homo,     goindol, goindol, homo, driver_device,    0,       ROT90, "bootleg", "Homo", MACHINE_SUPPORTS_SAVE )
