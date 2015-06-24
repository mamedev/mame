// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Sega Pengo

    driver by Nicola Salmoria

    Games supported:
        * Pengo
        * Jr Pac-Man (bootleg)

    Known issues:
        * none

****************************************************************************

    Pengo memory map (preliminary)

    driver by Nicola Salmoria

    0000-7fff ROM
    8000-83ff Video RAM
    8400-87ff Color RAM
    8800-8fff RAM

    memory mapped ports:

    read:
    9000      DSW1
    9040      DSW0
    9080      IN1
    90c0      IN0

    write:
    8ff2-8ffd 6 pairs of two bytes:
              the first byte contains the sprite image number (bits 2-7), Y flip (bit 0),
              X flip (bit 1); the second byte the color
    9005      sound voice 1 waveform (nibble)
    9011-9013 sound voice 1 frequency (nibble)
    9015      sound voice 1 volume (nibble)
    900a      sound voice 2 waveform (nibble)
    9016-9018 sound voice 2 frequency (nibble)
    901a      sound voice 2 volume (nibble)
    900f      sound voice 3 waveform (nibble)
    901b-901d sound voice 3 frequency (nibble)
    901f      sound voice 3 volume (nibble)
    9022-902d Sprite coordinates, x/y pairs for 6 sprites
    9040      interrupt enable
    9041      sound enable
    9042      palette bank selector
    9043      flip screen
    9044-9045 coin counters
    9046      color lookup table bank selector
    9047      character/sprite bank selector
    9070      watchdog reset

    Main clock: XTAL = 18.432 MHz
    Z80 Clock: XTAL/6 = 3.072 MHz
    Horizontal video frequency: HSYNC = XTAL/3/192/2 = 16 kHz
    Video frequency: VSYNC = HSYNC/132/2 = 60.606060 Hz
    VBlank duration: 1/VSYNC * (20/132) = 2500 us

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/segacrpt.h"
#include "includes/pacman.h"


class pengo_state : public pacman_state
{
public:
	pengo_state(const machine_config &mconfig, device_type type, const char *tag)
		: pacman_state(mconfig, type, tag), m_decrypted_opcodes(*this, "decrypted_opcodes") { }
	DECLARE_WRITE8_MEMBER(pengo_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_DRIVER_INIT(penta);
	DECLARE_DRIVER_INIT(pengo);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	optional_shared_ptr<UINT8> m_decrypted_opcodes;
};




/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (18432000)

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 128+16=144 and ends at 128+64+32+16=240 */
#define HTOTAL              (384)
#define HBEND               (0)     /*(96+16)*/
#define HBSTART             (288)   /*(16)*/

#define VTOTAL              (264)
#define VBEND               (0)     /*(16)*/
#define VBSTART             (224)   /*(224+16)*/



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

WRITE8_MEMBER(pengo_state::pengo_coin_counter_w)
{
	coin_counter_w(machine(), offset, data & 1);
}

WRITE8_MEMBER(pengo_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

static ADDRESS_MAP_START( pengo_map, AS_PROGRAM, 8, pengo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram") /* video and color RAM, scratchpad RAM, sprite codes */
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8800, 0x8fef) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x8ff0, 0x8fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9000, 0x901f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x9020, 0x902f) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x9000, 0x903f) AM_READ_PORT("DSW1")
	AM_RANGE(0x9040, 0x907f) AM_READ_PORT("DSW0")
	AM_RANGE(0x9040, 0x9040) AM_WRITE(irq_mask_w)
	AM_RANGE(0x9041, 0x9041) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x9042, 0x9042) AM_WRITE(pengo_palettebank_w)
	AM_RANGE(0x9043, 0x9043) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x9044, 0x9045) AM_WRITE(pengo_coin_counter_w)
	AM_RANGE(0x9046, 0x9046) AM_WRITE(pengo_colortablebank_w)
	AM_RANGE(0x9047, 0x9047) AM_WRITE(pengo_gfxbank_w)
	AM_RANGE(0x9070, 0x9070) AM_WRITENOP
	AM_RANGE(0x9080, 0x90bf) AM_READ_PORT("IN1")
	AM_RANGE(0x90c0, 0x90ff) AM_READ_PORT("IN0")
ADDRESS_MAP_END


static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, pengo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8800, 0x8fef) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x8ff0, 0x8fff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( jrpacmbl_map, AS_PROGRAM, 8, pengo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(jrpacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x8fef) AM_RAM
	AM_RANGE(0x8ff0, 0x8fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9000, 0x901f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x9020, 0x902f) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x9030, 0x9030) AM_WRITE(jrpacman_scroll_w)
	AM_RANGE(0x9040, 0x904f) AM_READ_PORT("DSW")
	AM_RANGE(0x9040, 0x9040) AM_WRITE(irq_mask_w)
	AM_RANGE(0x9041, 0x9041) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x9042, 0x9042) AM_WRITE(pengo_palettebank_w)
	AM_RANGE(0x9043, 0x9043) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x9044, 0x9044) AM_WRITE(jrpacman_bgpriority_w)
	AM_RANGE(0x9045, 0x9045) AM_WRITE(jrpacman_spritebank_w)
	AM_RANGE(0x9046, 0x9046) AM_WRITE(pengo_colortablebank_w)
	AM_RANGE(0x9047, 0x9047) AM_WRITE(jrpacman_charbank_w)
	AM_RANGE(0x9070, 0x9070) AM_WRITENOP
	AM_RANGE(0x9080, 0x90bf) AM_READ_PORT("P2")
	AM_RANGE(0x90c0, 0x90ff) AM_READ_PORT("P1")
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( pengo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	/* the coin input must stay low for no less than 2 frames and no more */
	/* than 9 frames to pass the self test check. */
	/* Moreover, this way we avoid the game freezing until the user releases */
	/* the "coin" key. */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	/* Coin Aux doesn't need IMPULSE to pass the test, but it still needs it */
	/* to avoid the freeze. */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0c, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0d, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x0b, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x07, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x0f, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xd0, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0xb0, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x70, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0xf0, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jrpacmbl )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPNAME( 0x40, 0x40, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};


static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),  /* 64 sprites */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 bytes */
};


static GFXDECODE_START( pengo )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x2000, spritelayout, 0, 128 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(pengo_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


static MACHINE_CONFIG_START( pengo, pengo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(pengo_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pengo_state,  vblank_irq)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pengo)
	MCFG_PALETTE_ADD("palette", 128*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(pengo_state,pacman)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(pengo_state, screen_update_pacman)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(pengo_state,pengo)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( jrpacmbl, pengo )

	/* basic machine hardware */

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jrpacmbl_map)
	MCFG_DEVICE_REMOVE_ADDRESS_MAP(AS_DECRYPTED_OPCODES)

	MCFG_VIDEO_START_OVERRIDE(pengo_state,jrpacman)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( pengo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ep1689c.8",    0x0000, 0x1000, CRC(f37066a8) SHA1(0930de17a763a527057f60783a92662b09554426) )
	ROM_LOAD( "ep1690b.7",    0x1000, 0x1000, CRC(baf48143) SHA1(4c97529e61eeca5d94938b1dfbeac41bf8cbaf7d) )
	ROM_LOAD( "ep1691b.15",   0x2000, 0x1000, CRC(adf0eba0) SHA1(c8949fbdbfe5023ee17a789ef60205e834a76c81) )
	ROM_LOAD( "ep1692b.14",   0x3000, 0x1000, CRC(a086d60f) SHA1(7079769d14dfe3873ffe29623ba0a93413706c6d) )
	ROM_LOAD( "ep1693b.21",   0x4000, 0x1000, CRC(b72084ec) SHA1(c0508951c2ad8dc31481be8b3bfee2063e3fb0d7) )
	ROM_LOAD( "ep1694b.20",   0x5000, 0x1000, CRC(94194a89) SHA1(7b47aec61593efd758e2a031f72a854bb0ba8af1) )
	ROM_LOAD( "ep5118b.32",   0x6000, 0x1000, CRC(af7b12c4) SHA1(207ed466546f40ca60a38031b83aef61446902e2) )
	ROM_LOAD( "ep5119c.31",   0x7000, 0x1000, CRC(933950fe) SHA1(fec7236b3dee2ea6e39c68440a6d2d9e3f72675a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ep1640.92",    0x0000, 0x1000, CRC(d7eec6cd) SHA1(e542bcc28f292be9a0a29d949de726e0b55e654a) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1)*/
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( pengo2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic8.2",        0x0000, 0x1000, CRC(e4924b7b) SHA1(44297658af8f8c884eba02b792346c5008137dfe) )
	ROM_LOAD( "ic7.2",        0x1000, 0x1000, CRC(72e7775d) SHA1(49e04178ee171f727dd023c019395679cfbad452) )
	ROM_LOAD( "ic15.2",       0x2000, 0x1000, CRC(7410ef1e) SHA1(7ed8e16c6ce401904c0da9758e2a405d7b9b451b) )
	ROM_LOAD( "ic14.2",       0x3000, 0x1000, CRC(55b3f379) SHA1(bc244f97132f0514adb3d6ceda8afbd45c1c587a) )
	ROM_LOAD( "ep1693b.21",   0x4000, 0x1000, CRC(b72084ec) SHA1(c0508951c2ad8dc31481be8b3bfee2063e3fb0d7) )
	ROM_LOAD( "ic20.2",       0x5000, 0x1000, CRC(770570cf) SHA1(43ead8236f53d39041ffc21bdeef10b3a77ce7f2) )
	ROM_LOAD( "ep5118b.32",   0x6000, 0x1000, CRC(af7b12c4) SHA1(207ed466546f40ca60a38031b83aef61446902e2) )
	ROM_LOAD( "ic31.2",       0x7000, 0x1000, CRC(669555c1) SHA1(50d5cf8022af6d6bd022235ab06015cb5c8aa433) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ep1640.92",    0x0000, 0x1000, CRC(d7eec6cd) SHA1(e542bcc28f292be9a0a29d949de726e0b55e654a) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( pengo2u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5128.u8",     0x0000, 0x1000, CRC(3dfeb20e) SHA1(a387b72501da77bf38b58619d2099083a0463e1f) )
	ROM_LOAD( "epr5129.u7",     0x1000, 0x1000, CRC(1db341bd) SHA1(d1c66bb9cf479e6960dbcd35c820097a81eaa555) )
	ROM_LOAD( "epr5130.u15",    0x2000, 0x1000, CRC(7c2842d5) SHA1(a8a568da68babd0ccb9f2cee4182fc01c3138494) )
	ROM_LOAD( "epr5131.u14",    0x3000, 0x1000, CRC(6e3c1f2f) SHA1(2ee821b0f6e0f3cfeae7f5ff25a6e9bd977efce0) )
	ROM_LOAD( "ep5124.21",    0x4000, 0x1000, CRC(95f354ff) SHA1(fdebc68a6d87f8ecdf52a57a34ae5ae844a13510) )
	ROM_LOAD( "epr5133.u20",    0x5000, 0x1000, CRC(0fdb04b8) SHA1(ed814d58318c1055e475ff678609d189727bf9b4) )
	ROM_LOAD( "ep5126.32",    0x6000, 0x1000, CRC(e5920728) SHA1(0ac5ffdad7bdcb32e630b9582e1b1aaece5198c9) )
	ROM_LOAD( "epr5135.u31",    0x7000, 0x1000, CRC(13de47ed) SHA1(332b484d47c9921ed93432755bb2d7a9d4628939) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ep1640.92",    0x0000, 0x1000, CRC(d7eec6cd) SHA1(e542bcc28f292be9a0a29d949de726e0b55e654a) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( pengo3u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ep5120.8",     0x0000, 0x1000, CRC(f01afb60) SHA1(1db732a17a9f79f8f1751f80c77889142928e41b) )
	ROM_LOAD( "ep5121.7",     0x1000, 0x1000, CRC(2eb38353) SHA1(d351347f93a3ed01c8b5274ec19352dd611a8dd4) )
	ROM_LOAD( "ep5122.15",    0x2000, 0x1000, CRC(c33400d7) SHA1(7b9617d22a9de8d3658abe34b5d2171ce37acc39) )
	ROM_LOAD( "ep5123.14",    0x3000, 0x1000, CRC(6a85c6a2) SHA1(444acc08607c892bb20b3a02753169addf5b11de) )
	ROM_LOAD( "ep5124.21",    0x4000, 0x1000, CRC(95f354ff) SHA1(fdebc68a6d87f8ecdf52a57a34ae5ae844a13510) )
	ROM_LOAD( "ep5125.20",    0x5000, 0x1000, CRC(1a42310f) SHA1(fef20385299a709ee17ed16510ac5702bd5cc257) )
	ROM_LOAD( "ep5126.32",    0x6000, 0x1000, CRC(e5920728) SHA1(0ac5ffdad7bdcb32e630b9582e1b1aaece5198c9) )
	ROM_LOAD( "ep5127.31",    0x7000, 0x1000, CRC(a7d3d1d6) SHA1(20e4353208c3803d8879b25f821ea617e9a19cc4) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ep1640.92",    0x0000, 0x1000, CRC(d7eec6cd) SHA1(e542bcc28f292be9a0a29d949de726e0b55e654a) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( pengo4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ep1738.8",     0x0000, 0x1000, CRC(68ba25ea) SHA1(ce937831b7b210b4a625514bd4e6b3a7a36d008e) )
	ROM_LOAD( "ep1739.7",     0x1000, 0x1000, CRC(41e7b5b3) SHA1(d512d41ee3f5716070250e7ab63342e4fbf92875) )
	ROM_LOAD( "ep1740.15",    0x2000, 0x1000, CRC(27f05f59) SHA1(c0d40328a7dff34f6b84c991d9c88b240e55b4f3) )
	ROM_LOAD( "ep1741.14",    0x3000, 0x1000, CRC(27d93ec1) SHA1(925e59878342af58106d5b11ebb6c86cbb69ae91) )
	ROM_LOAD( "ep1742.21",    0x4000, 0x1000, CRC(b72084ec) SHA1(c0508951c2ad8dc31481be8b3bfee2063e3fb0d7) )
	ROM_LOAD( "ep1743.20",    0x5000, 0x1000, CRC(770570cf) SHA1(43ead8236f53d39041ffc21bdeef10b3a77ce7f2) )
	ROM_LOAD( "ep1744.32",    0x6000, 0x1000, CRC(af7b12c4) SHA1(207ed466546f40ca60a38031b83aef61446902e2) )
	ROM_LOAD( "ep1745.31",    0x7000, 0x1000, CRC(507e18b9) SHA1(e169e4c9c6350fb5e4020222dbcaa6f5ce41849c) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ep1640.92",    0x0000, 0x1000, CRC(d7eec6cd) SHA1(e542bcc28f292be9a0a29d949de726e0b55e654a) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( pengob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x2000, CRC(e04064db) SHA1(6689066b443807646894a357317f468bfc92368a) )
	ROM_LOAD( "2",            0x2000, 0x2000, CRC(75752424) SHA1(634e696a692c7245dfa5c5dfd4ce87755c2a90d4) )
	ROM_LOAD( "021_pn03.bin", 0x4000, 0x1000, CRC(7824e3ef) SHA1(3395bb537614de8da763d05f0d2e312145017e8f) ) // 3 (1/2)
	ROM_LOAD( "020_pn07.bin", 0x5000, 0x1000, CRC(377b9663) SHA1(35327dc0f0c19fa5a863aaf8d8f3bfcd2a5717a9) ) // 3 (2/2)
	ROM_LOAD( "032_pn04.bin", 0x6000, 0x1000, CRC(bfde44c1) SHA1(97e8a360ce09faa36d864d7020b1669a349867c6) ) // 4 (1/2)
	ROM_LOAD( "031_pn08.bin", 0x7000, 0x1000, CRC(64e8c30d) SHA1(aa50c21db2ac8361fc575f0785e2aae57f338564) ) // 4 (2/2)

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "5",            0x0000, 0x1000, CRC(1232437b) SHA1(7ec410a2a802514449ccb05684762c25f29556b0) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( penta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "008_pn01.bin", 0x0000, 0x1000, CRC(22f328df) SHA1(ba13b8d20ccde995a158cf62b4bc48cb369a0788) )
	ROM_LOAD( "007_pn05.bin", 0x1000, 0x1000, CRC(15bbc7d3) SHA1(1823e3ba7388d3f4d9262e9b9cf70f123862c546) )
	ROM_LOAD( "015_pn02.bin", 0x2000, 0x1000, CRC(de82b74a) SHA1(301c1223dd0b111f8439affcb96b6e29106364ed) )
	ROM_LOAD( "014_pn06.bin", 0x3000, 0x1000, CRC(160f3836) SHA1(ff90c82d52ed0c2c17a7aeabc9401ee9d8cf4d2d) )
	ROM_LOAD( "021_pn03.bin", 0x4000, 0x1000, CRC(7824e3ef) SHA1(3395bb537614de8da763d05f0d2e312145017e8f) )
	ROM_LOAD( "020_pn07.bin", 0x5000, 0x1000, CRC(377b9663) SHA1(35327dc0f0c19fa5a863aaf8d8f3bfcd2a5717a9) )
	ROM_LOAD( "032_pn04.bin", 0x6000, 0x1000, CRC(bfde44c1) SHA1(97e8a360ce09faa36d864d7020b1669a349867c6) )
	ROM_LOAD( "031_pn08.bin", 0x7000, 0x1000, CRC(64e8c30d) SHA1(aa50c21db2ac8361fc575f0785e2aae57f338564) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "092_pn09.bin", 0x0000, 0x1000, CRC(6afeba9d) SHA1(cd723fb94aa90dbaac9a6fe085c0f4786d2fa092) ) /* tiles (bank 1) */
	ROM_CONTINUE(             0x2000, 0x1000 ) /* sprites (bank 1) */
	ROM_LOAD( "ep1695.105",   0x1000, 0x1000, CRC(5bfd26e9) SHA1(bdec535e486b43a8f5550334beff423eeace10b2) ) /* tiles (bank 2) */
	ROM_CONTINUE(             0x3000, 0x1000 ) /* sprites (bank 2) */

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "pr1635.51",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) ) /* waveform */
	ROM_LOAD( "pr1636.70",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( jrpacmbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jrpacpe-05.ic8",     0x0000, 0x1000, CRC(98049df4) SHA1(4ea022c8664dd9ec185f9d5990a548e867e5071f) )
	ROM_LOAD( "jrpacpe-01.ic7",     0x1000, 0x1000, CRC(b7a5cef8) SHA1(c315970f0dd698a1036df12502e3fe3ec7f81d53) )
	ROM_LOAD( "jrpacpe-06.ic6",     0x2000, 0x1000, CRC(ecf39785) SHA1(9e47f29f4cadb5d8fd3790c7e16c653fc0a96a88) )
	ROM_LOAD( "jrpacpe-02.ic5",     0x3000, 0x1000, CRC(c090145c) SHA1(918d32267379b99f898fdcd987b3a65f9ec4f088) )
	ROM_LOAD( "jrpacpe-07.ic4",     0x4000, 0x1000, CRC(659b9956) SHA1(5576d4d95ced804e8abdd870662574bfdd6df18f) )
	ROM_LOAD( "jrpacpe-03.ic3",     0x5000, 0x1000, CRC(0ebcfac9) SHA1(4da01169768e35601e04df7004ae7496f08a709a) )
	ROM_LOAD( "jrpacpe-08.ic2",     0x6000, 0x1000, CRC(0624ffd6) SHA1(8209ab633242f2c8952a5afe2b7cd399bab08f0a) )
	ROM_LOAD( "jrpacpe-04.ic1",     0x7000, 0x1000, CRC(d3a8448c) SHA1(f58aed6ebdb45ed38613b336a517b87745831e24) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jrpacpe-09.ic92",    0x0000, 0x2000, CRC(2128d9b4) SHA1(b6f64423ae6ee3765050f7b85b4490b5eed95215) ) /* tiles bank 1 & 2 */
	ROM_LOAD( "jrpacpe-10.ic105",   0x2000, 0x2000, CRC(73477193) SHA1(f00a488958ea0438642d345693787bdf771219ad) ) /* sprites (bank 1) & 2 */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD_NIB_LOW ( "jrprom.9e", 0x0000, 0x0100, CRC(029d35c4) SHA1(d9aa2dc442e9ac36cf3c346b9fb1aa745eaf3cb8) ) /* color palette (low bits) */
	ROM_LOAD_NIB_HIGH( "jrprom.9f", 0x0000, 0x0100, CRC(eee34a79) SHA1(7561f8ccab2af85c111af6a02af6986eb67503e5) ) /* color palette (high bits) */
	ROM_LOAD( "jrprom.9p",          0x0020, 0x0100, CRC(9f6ea9d8) SHA1(62cf15513934d34641433c891a7f73bef82e2fb1) ) /* color lookup table */

	ROM_REGION( 0x0200, "namco", 0 )    /* waveform */
	ROM_LOAD( "jrprom.7p",          0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) /* waveform */
	ROM_LOAD( "jrprom.5s",          0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(pengo_state,pengo)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0xa0,0x80,0xa8,0x88 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...0...0...1 */
		{ 0xa0,0x80,0x20,0x00 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...0...1...0 */
		{ 0x08,0x28,0x88,0xa8 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...0...1...1 */
		{ 0x08,0x00,0x88,0x80 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...1...0...0 */
		{ 0xa0,0x80,0x20,0x00 }, { 0x08,0x00,0x88,0x80 },   /* ...0...1...0...1 */
		{ 0xa0,0x80,0x20,0x00 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...1...1...0 */
		{ 0xa0,0x80,0x20,0x00 }, { 0x00,0x08,0x20,0x28 },   /* ...0...1...1...1 */
		{ 0x88,0x80,0x08,0x00 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...0...0...0 */
		{ 0x88,0x80,0x08,0x00 }, { 0x00,0x08,0x20,0x28 },   /* ...1...0...0...1 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x08,0x28,0x88,0xa8 },   /* ...1...0...1...0 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...0...1...1 */
		{ 0x08,0x00,0x88,0x80 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...0...0 */
		{ 0x00,0x08,0x20,0x28 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...0...1 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x08,0x28,0x88,0xa8 },   /* ...1...1...1...0 */
		{ 0x08,0x00,0x88,0x80 }, { 0xa0,0x80,0x20,0x00 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);
}


DRIVER_INIT_MEMBER(pengo_state,penta)
{
/*
    the values vary, but the translation mask is always laid out like this:

      0 1 2 3 4 5 6 7 8 9 a b c d e f
    0 A A B B A A B B C C D D C C D D
    1 A A B B A A B B C C D D C C D D
    2 E E F F E E F F G G H H G G H H
    3 E E F F E E F F G G H H G G H H
    4 A A B B A A B B C C D D C C D D
    5 A A B B A A B B C C D D C C D D
    6 E E F F E E F F G G H H G G H H
    7 E E F F E E F F G G H H G G H H
    8 H H G G H H G G F F E E F F E E
    9 H H G G H H G G F F E E F F E E
    a D D C C D D C C B B A A B B A A
    b D D C C D D C C B B A A B B A A
    c H H G G H H G G F F E E F F E E
    d H H G G H H G G F F E E F F E E
    e D D C C D D C C B B A A B B A A
    f D D C C D D C C B B A A B B A A

    (e.g. 0xc0 is XORed with H)
    therefore in the following tables we only keep track of A, B, C, D, E, F, G and H.
*/
	static const UINT8 data_xortable[2][8] =
	{
		{ 0xa0,0x82,0x28,0x0a,0x82,0xa0,0x0a,0x28 },    /* ...............0 */
		{ 0x88,0x0a,0x82,0x00,0x88,0x0a,0x82,0x00 }     /* ...............1 */
	};
	static const UINT8 opcode_xortable[8][8] =
	{
		{ 0x02,0x08,0x2a,0x20,0x20,0x2a,0x08,0x02 },    /* ...0...0...0.... */
		{ 0x88,0x88,0x00,0x00,0x88,0x88,0x00,0x00 },    /* ...0...0...1.... */
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    /* ...0...1...0.... */
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    /* ...0...1...1.... */
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },    /* ...1...0...0.... */
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },    /* ...1...0...1.... */
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    /* ...1...1...0.... */
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 }     /* ...1...1...1.... */
	};

	UINT8 *rom = memregion("maincpu")->base();

	for (int A = 0x0000;A < 0x8000;A++)
	{
		int i,j;
		UINT8 src;


		src = rom[A];

		/* pick the translation table from bit 0 of the address */
		i = A & 1;

		/* pick the offset in the table from bits 1, 3 and 5 of the source data */
		j = ((src >> 1) & 1) + (((src >> 3) & 1) << 1) + (((src >> 5) & 1) << 2);
		/* the bottom half of the translation table is the mirror image of the top */
		if (src & 0x80) j = 7 - j;

		/* decode the ROM data */
		rom[A] = src ^ data_xortable[i][j];

		/* now decode the opcodes */
		/* pick the translation table from bits 4, 8 and 12 of the address */
		i = ((A >> 4) & 1) + (((A >> 8) & 1) << 1) + (((A >> 12) & 1) << 2);
		m_decrypted_opcodes[A] = src ^ opcode_xortable[i][j];
	}
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, pengo,    0,     pengo, pengo, pengo_state, pengo,    ROT90, "Sega", "Pengo (set 1 rev c)", GAME_SUPPORTS_SAVE )
GAME( 1982, pengo2,   pengo, pengo, pengo, pengo_state, pengo,    ROT90, "Sega", "Pengo (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1982, pengo2u,  pengo, pengo, pengo, driver_device, 0,        ROT90, "Sega", "Pengo (set 2 not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1982, pengo3u,  pengo, pengo, pengo, driver_device, 0,        ROT90, "Sega", "Pengo (set 3 not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1982, pengo4,   pengo, pengo, pengo, pengo_state, pengo,    ROT90, "Sega", "Pengo (set 4)", GAME_SUPPORTS_SAVE )
GAME( 1982, pengob,   pengo, pengo, pengo, pengo_state, penta,    ROT90, "bootleg", "Pengo (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1982, penta,    pengo, pengo, pengo, pengo_state, penta,    ROT90, "bootleg (Grinbee Shouji)", "Penta", GAME_SUPPORTS_SAVE ) // Grinbee Shouji was a subsidiary of Orca
GAME( 1983, jrpacmbl, jrpacman, jrpacmbl, jrpacmbl, driver_device, 0, ROT90, "bootleg", "Jr. Pac-Man (Pengo hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
