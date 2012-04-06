/***************************************************************************

    Lemmings                (c) 1991 Data East USA (DE-0357)

    Prototype!  Licensed from the home computer version this game never
    made it past the arcade field test stage.  Unlike most Data East games
    this hardware features a pixel layer and a VRAM layer, probably to
    make the transition from the pixel addressable computer code to the
    arcade hardware.

    As prototype software it seems to have a couple of non-critical bugs,
    the palette ram check and vram check both overrun their actual ramsize.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/lemmings.h"
#include "video/decospr.h"

WRITE16_MEMBER(lemmings_state::lemmings_control_w)
{

	/* Offset==0 Pixel layer X scroll */
	if (offset == 4)
		return; /* Watchdog or IRQ ack */
	COMBINE_DATA(&m_control_data[offset]);
}

READ16_MEMBER(lemmings_state::lemmings_trackball_r)
{
	switch (offset)
	{
	case 0: return input_port_read(machine(), "AN0");
	case 1: return input_port_read(machine(), "AN1");
	case 4: return input_port_read(machine(), "AN2");
	case 5: return input_port_read(machine(), "AN3");
	}
	return 0;
}

/* Same as Robocop 2 protection chip */
READ16_MEMBER(lemmings_state::lemmings_prot_r)
{
	switch (offset << 1)
	{
		case 0x41a:
			return input_port_read(machine(), "BUTTONS");

		case 0x320:
			return input_port_read(machine(), "SYSTEM");

		case 0x4e6:
			return input_port_read(machine(), "DSW");
	}

	return 0;
}

WRITE16_MEMBER(lemmings_state::lemmings_palette_24bit_w)
{
	int r, g, b;

	COMBINE_DATA(&m_paletteram[offset]);
	if (offset & 1)
		offset--;

	b = (m_paletteram[offset] >> 0) & 0xff;
	g = (m_paletteram[offset + 1] >> 8) & 0xff;
	r = (m_paletteram[offset + 1] >> 0) & 0xff;

	palette_set_color(machine(), offset / 2, MAKE_RGB(r, g, b));
}

WRITE16_MEMBER(lemmings_state::lemmings_sound_w)
{
	soundlatch_w(space, 0, data & 0xff);
	device_set_input_line(m_audiocpu, 1, HOLD_LINE);
}

WRITE8_MEMBER(lemmings_state::lemmings_sound_ack_w)
{
	device_set_input_line(m_audiocpu, 1, CLEAR_LINE);
}

/******************************************************************************/

static ADDRESS_MAP_START( lemmings_map, AS_PROGRAM, 16, lemmings_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x140000, 0x1407ff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x160000, 0x160fff) AM_RAM_WRITE(lemmings_palette_24bit_w) AM_BASE(m_paletteram)
	AM_RANGE(0x170000, 0x17000f) AM_RAM_WRITE(lemmings_control_w) AM_BASE(m_control_data)
	AM_RANGE(0x190000, 0x19000f) AM_READ(lemmings_trackball_r)
	AM_RANGE(0x1a0000, 0x1a07ff) AM_READ(lemmings_prot_r)
	AM_RANGE(0x1a0064, 0x1a0065) AM_WRITE(lemmings_sound_w)
	AM_RANGE(0x1c0000, 0x1c0001) AM_DEVWRITE("spriteram", buffered_spriteram16_device, write) /* 1 written once a frame */
	AM_RANGE(0x1e0000, 0x1e0001) AM_DEVWRITE("spriteram2", buffered_spriteram16_device, write) /* 1 written once a frame */
	AM_RANGE(0x200000, 0x201fff) AM_RAM_WRITE(lemmings_vram_w) AM_BASE(m_vram_data)
	AM_RANGE(0x202000, 0x202fff) AM_RAM
	AM_RANGE(0x300000, 0x37ffff) AM_RAM_WRITE(lemmings_pixel_0_w) AM_BASE(m_pixel_0_data)
	AM_RANGE(0x380000, 0x39ffff) AM_RAM_WRITE(lemmings_pixel_1_w) AM_BASE(m_pixel_1_data)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, lemmings_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r,ym2151_w)
	AM_RANGE(0x1000, 0x1000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x1800, 0x1800) AM_READ(soundlatch_r) AM_WRITE(lemmings_sound_ack_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( lemmings )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Hurry")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Select")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Hurry")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_LOW)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Credits for 1 Player" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, "Credits for 2 Player" )
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, "Credits for Continue" )
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPUNKNOWN( 0x4000, 0x4000 )
	PORT_DIPUNKNOWN( 0x8000, 0x8000 )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	2048,
	4,
	{ 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*0, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ 0x30000*8, 0x20000*8, 0x10000*8, 0x00000*8 },
	{
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,0, 1, 2, 3, 4, 5, 6, 7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( lemmings )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,  0, 16 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout,  0, 16 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( NULL,           0, charlayout,         0, 16 ) /* Dynamically modified */
GFXDECODE_END

/******************************************************************************/

static void sound_irq( device_t *device, int state )
{
	lemmings_state *lemmings = device->machine().driver_data<lemmings_state>();
	device_set_input_line(lemmings->m_audiocpu, 0, state);
}

static const ym2151_interface ym2151_config =
{
	sound_irq
};

static MACHINE_START( lemmings )
{
	lemmings_state *state = machine.driver_data<lemmings_state>();

	state->m_audiocpu = machine.device("audiocpu");
}

static MACHINE_CONFIG_START( lemmings, lemmings_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(lemmings_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", M6809,32220000/8)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(lemmings)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram2")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(lemmings)
	MCFG_SCREEN_VBLANK_STATIC(lemmings)

	MCFG_GFXDECODE(lemmings)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(lemmings)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 1);

	MCFG_DEVICE_ADD("spritegen2", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 0);


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( lemmings )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "lemmings.5", 0x00000, 0x20000, CRC(e9a2b439) SHA1(873723a06d71bb41772951f451a75578b30267d5) )
	ROM_LOAD16_BYTE( "lemmings.1", 0x00001, 0x20000, CRC(bf52293b) SHA1(47a1ed64bf02776db086fdce80997b8a0c068791) )
	ROM_LOAD16_BYTE( "lemmings.6", 0x40000, 0x20000, CRC(0e3dc0ea) SHA1(533abf66ca4b578d03566d5de922dc5828c26eca) )
	ROM_LOAD16_BYTE( "lemmings.2", 0x40001, 0x20000, CRC(0cf3d7ce) SHA1(95dc43a8cded860fcf8743b62cbe4f2a97f43215) )
	ROM_LOAD16_BYTE( "lemmings.7", 0x80000, 0x20000, CRC(d020219c) SHA1(9678d8636798d1e528269fe2f9eb532e189c134e) )
	ROM_LOAD16_BYTE( "lemmings.3", 0x80001, 0x20000, CRC(c635494a) SHA1(e105dc79bd3c425d971629a3066c38dbf08b6428) )
	ROM_LOAD16_BYTE( "lemmings.8", 0xc0000, 0x20000, CRC(9166ce09) SHA1(7f0970cc07ebdbfc9a738342259d07d37b397161) )
	ROM_LOAD16_BYTE( "lemmings.4", 0xc0001, 0x20000, CRC(aa845488) SHA1(d17ec80f43d2a0123e93fad83d4e1319eb18d7c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound CPU */
	ROM_LOAD( "lemmings.15",    0x00000, 0x10000, CRC(f0b24a35) SHA1(1aaeb1e6faee04d2e433161fd7aa965b58e3b8a7) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "lemmings.9",  0x000000, 0x10000, CRC(e06442f5) SHA1(d9c8b681cce1d0257a0446bc820c7d679e2a1168) )
	ROM_LOAD( "lemmings.10", 0x010000, 0x10000, CRC(36398848) SHA1(6c6956607f889c35367e6df4a32359042fad695e) )
	ROM_LOAD( "lemmings.11", 0x020000, 0x10000, CRC(b46a54e5) SHA1(53b053346f80357aecff4ab888a8562f99cb318f) )
	ROM_FILL(                0x030000, 0x10000, 0 ) /* 3bpp data but sprite chip expects 4 */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "lemmings.12", 0x000000, 0x10000, CRC(dc9047ff) SHA1(1bbe573fa51127a9e8b970a353f3cceab00f486a) )
	ROM_LOAD( "lemmings.13", 0x010000, 0x10000, CRC(7cc15491) SHA1(73c1c11b2738f6679c70cae8ac4c55cdc9b8fc27) )
	ROM_LOAD( "lemmings.14", 0x020000, 0x10000, CRC(c162788f) SHA1(e1f669efa59699cd1b7da71b112701ee79240c18) )
	ROM_FILL(                0x030000, 0x10000, 0 ) /* 3bpp data but sprite chip expects 4 */

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "lemmings.16",    0x00000, 0x20000, CRC(f747847c) SHA1(00880fa6dff979e5d15daea61938bd18c768c92f) )
ROM_END

/******************************************************************************/

GAME( 1991, lemmings, 0, lemmings, lemmings, 0, ROT0, "Data East USA", "Lemmings (US prototype)", GAME_SUPPORTS_SAVE )
