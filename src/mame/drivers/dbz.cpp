// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*
  Dragonball Z                  (c) 1993 Banpresto
  Dragonball Z 2 - Super Battle (c) 1994 Banpresto

  Driver by David Haywood, R. Belmont and Pierpaolo Prazzoli

  MC68000 + Konami Xexex-era video hardware and system controller ICs
  Z80 + YM2151 + OKIM6295 for sound

  Note: game has an extremely complete test mode, it's beautiful for emulation.
        flip the DIP and check it out!

  TODO:
    - Self Test Fails
    - Banpresto logo in DBZ has bad colors after 1 run of the attract mode because
      it's associated to the wrong logical tilemap and the same happens in DBZ2
      test mode. It should be a bug in K056832 emulation.

PCB Layout:

BP924-1  PWB250248D (note PCB is identical to DBZ2 also)
|-------------------------------------------------------|
| YM3014  Z80    32MHz  053252       222A05   222A07    |
|   YM2151 5168                      222A04   222A06    |
| 1.056kHz 5168                                         |
|  M6295   222A10                           5864        |
|   222A03     68000                 2018   5864 053246A|
|J          222A11  222A12           2018               |
|A 5864        *       *                                |
|M 5864     62256   62256                               |
|M                                               053247A|
|A                                                      |
|                                053936 053936     2018 |
|    053251  053251                                2018 |
|                                        CY7C128        |
|       054157  054156  5864     CY7C128 CY7C128 CY7C128|
|       222A01  222A02  5864     CY7C128                |
| DSW2  DSW1            5864     CY7C128 222A08  222A09 |
|                                           *       *   |
|-------------------------------------------------------|

Notes:
      68k clock: 16.000MHz
      Z80 clock: 4.000MHz
   YM2151 clock: 4.000MHz
    M6295 clock: 1.056MHz (sample rate = /132)
          Vsync: 55Hz
          Hsync: 15.36kHz
              *: unpopulated ROM positions on DBZ

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/dbz.h"


TIMER_DEVICE_CALLBACK_MEMBER(dbz_state::dbz_scanline)
{
	int scanline = param;

	if(scanline == 256) // vblank-out irq
		m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);

	if(scanline == 0 && m_k053246->k053246_is_irq_enabled()) // vblank-in irq
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE); //auto-acks apparently
}

#if 0
READ16_MEMBER(dbz_state::dbzcontrol_r)
{
	return m_control;
}
#endif

WRITE16_MEMBER(dbz_state::dbzcontrol_w)
{
	/* bit 10 = enable '246 readback */

	COMBINE_DATA(&m_control);

	if (data & 0x400)
		m_k053246->k053246_set_objcha_line( ASSERT_LINE);
	else
		m_k053246->k053246_set_objcha_line( CLEAR_LINE);

	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

WRITE16_MEMBER(dbz_state::dbz_sound_command_w)
{
	soundlatch_byte_w(space, 0, data >> 8);
}

WRITE16_MEMBER(dbz_state::dbz_sound_cause_nmi)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( dbz_map, AS_PROGRAM, 16, dbz_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x480000, 0x48ffff) AM_RAM
	AM_RANGE(0x490000, 0x491fff) AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)  // '157 RAM is mirrored twice
	AM_RANGE(0x492000, 0x493fff) AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)
	AM_RANGE(0x498000, 0x49ffff) AM_DEVREAD("k056832", k056832_device, rom_word_8000_r)  // code near a60 in dbz2, subroutine at 730 in dbz
	AM_RANGE(0x4a0000, 0x4a0fff) AM_DEVREADWRITE("k053246", k053247_device, k053247_word_r, k053247_word_w)
	AM_RANGE(0x4a1000, 0x4a3fff) AM_RAM
	AM_RANGE(0x4a8000, 0x4abfff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // palette
	AM_RANGE(0x4c0000, 0x4c0001) AM_DEVREAD("k053246", k053247_device, k053246_word_r)
	AM_RANGE(0x4c0000, 0x4c0007) AM_DEVWRITE("k053246", k053247_device, k053246_word_w)
	AM_RANGE(0x4c4000, 0x4c4007) AM_DEVWRITE("k053246", k053247_device, k053246_word_w)
	AM_RANGE(0x4c8000, 0x4c8007) AM_DEVWRITE("k056832", k056832_device, b_word_w)
	AM_RANGE(0x4cc000, 0x4cc03f) AM_DEVWRITE("k056832", k056832_device, word_w)
	AM_RANGE(0x4d0000, 0x4d001f) AM_DEVWRITE("k053936_1", k053936_device, ctrl_w)
	AM_RANGE(0x4d4000, 0x4d401f) AM_DEVWRITE("k053936_2", k053936_device, ctrl_w)
	AM_RANGE(0x4e0000, 0x4e0001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x4e0002, 0x4e0003) AM_READ_PORT("SYSTEM_DSW1")
	AM_RANGE(0x4e4000, 0x4e4001) AM_READ_PORT("DSW2")
	AM_RANGE(0x4e8000, 0x4e8001) AM_WRITENOP
	AM_RANGE(0x4ec000, 0x4ec001) AM_WRITE(dbzcontrol_w)
	AM_RANGE(0x4f0000, 0x4f0001) AM_WRITE(dbz_sound_command_w)
	AM_RANGE(0x4f4000, 0x4f4001) AM_WRITE(dbz_sound_cause_nmi)
	AM_RANGE(0x4f8000, 0x4f801f) AM_DEVREADWRITE8("k053252", k053252_device, read, write, 0xff00)      // 251 #1
	AM_RANGE(0x4fc000, 0x4fc01f) AM_DEVWRITE("k053251", k053251_device, lsb_w)   // 251 #2

	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(dbz_bg2_videoram_w) AM_SHARE("bg2_videoram")
	AM_RANGE(0x508000, 0x509fff) AM_RAM_WRITE(dbz_bg1_videoram_w) AM_SHARE("bg1_videoram")
	AM_RANGE(0x510000, 0x513fff) AM_DEVREADWRITE("k053936_1", k053936_device, linectrl_r, linectrl_w) // ?? guess, it might not be
	AM_RANGE(0x518000, 0x51bfff) AM_DEVREADWRITE("k053936_2", k053936_device, linectrl_r, linectrl_w) // ?? guess, it might not be
	AM_RANGE(0x600000, 0x6fffff) AM_READNOP             // PSAC 1 ROM readback window
	AM_RANGE(0x700000, 0x7fffff) AM_READNOP             // PSAC 2 ROM readback window
ADDRESS_MAP_END

/* dbz sound */
/* IRQ: from YM2151.  NMI: from 68000.  Port 0: write to ack NMI */

static ADDRESS_MAP_START( dbz_sound_map, AS_PROGRAM, 8, dbz_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xd000, 0xd002) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe000, 0xe001) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dbz_sound_io_map, AS_IO, 8, dbz_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP
ADDRESS_MAP_END

/**********************************************************************************/


static INPUT_PORTS_START( dbz )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM_DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2") // I think this is right, but can't stomach the game long enough to check
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW1:3" )                       // seems unused
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:4")   // Definitely correct
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW1:5" )
	PORT_SERVICE_DIPLOC(  0x2000, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x8000, 0x0000, "Mask ROM Test" )     PORT_DIPLOCATION("SW1:8")           //NOP'd
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "FAKE")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "FAKE")

	PORT_START("FAKE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */
INPUT_PORTS_END

static INPUT_PORTS_START( dbza )
	PORT_INCLUDE( dbz )

	PORT_MODIFY("SYSTEM_DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW1:8" )                       // tests are always performed at start
INPUT_PORTS_END

static INPUT_PORTS_START( dbz2 )
	PORT_INCLUDE( dbz )

	PORT_MODIFY("SYSTEM_DSW1")
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout bglayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4,
		9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( dbz )
	GFXDECODE_ENTRY( "gfx3", 0, bglayout, 0, 512 )
	GFXDECODE_ENTRY( "gfx4", 0, bglayout, 0, 512 )
GFXDECODE_END

/**********************************************************************************/

WRITE_LINE_MEMBER(dbz_state::dbz_irq2_ack_w)
{
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}

void dbz_state::machine_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layerpri));
	save_item(NAME(m_layer_colorbase));
}

void dbz_state::machine_reset()
{
	int i;

	for (i = 0; i < 5; i++)
		m_layerpri[i] = 0;

	for (i = 0; i < 6; i++)
		m_layer_colorbase[i] = 0;

	m_sprite_colorbase = 0;
	m_control = 0;
}

static MACHINE_CONFIG_START( dbz, dbz_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(dbz_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", dbz_state, dbz_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dbz_sound_map)
	MCFG_CPU_IO_MAP(dbz_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 40*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 48*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dbz_state, screen_update_dbz)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dbz)

	MCFG_PALETTE_ADD("palette", 0x4000/2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_DEVICE_ADD("k056832", K056832, 0)
	MCFG_K056832_CB(dbz_state, tile_callback)
	MCFG_K056832_CONFIG("gfx1", K056832_BPP_4, 1, 1, "none")
	MCFG_K056832_PALETTE("palette")

	MCFG_DEVICE_ADD("k053246", K053246, 0)
	MCFG_K053246_CB(dbz_state, sprite_callback)
	MCFG_K053246_CONFIG("gfx2", NORMAL_PLANE_ORDER, -87, 32) // or -52, 16?
	MCFG_K053246_PALETTE("palette")

	MCFG_K053251_ADD("k053251")

	MCFG_DEVICE_ADD("k053936_1", K053936, 0)
	MCFG_K053936_WRAP(1)
	MCFG_K053936_OFFSETS(-46, -16)

	MCFG_DEVICE_ADD("k053936_2", K053936, 0)
	MCFG_K053936_WRAP(1)
	MCFG_K053936_OFFSETS(-46, -16)

	MCFG_DEVICE_ADD("k053252", K053252, 16000000/2)
	MCFG_K053252_INT1_ACK_CB(WRITELINE(dbz_state, dbz_irq2_ack_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

/**********************************************************************************/

ROM_START( dbz )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "222b11.9e", 0x000000, 0x80000, CRC(4c6b75e9) SHA1(8b1d67f8c8b64bb38f824506eca4c6966215f233) )
	ROM_LOAD16_BYTE( "222b12.9f", 0x000001, 0x80000, CRC(48637fce) SHA1(d3db0d56b70b9a4b20c645dda15327ec60e69d81) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("222a10.5e", 0x000000, 0x08000, CRC(1c93e30a) SHA1(8545a0ac5126b3c855e1901b186f57820699895d) )

	/* tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD32_WORD( "222a01.27c", 0x000000, 0x200000, CRC(9fce4ed4) SHA1(81e19375b351ee247f066434dd595149333d73c5) )
	ROM_LOAD32_WORD( "222a02.27e", 0x000002, 0x200000, CRC(651acaa5) SHA1(33942a90fb294b5da6a48e5bfb741b31babca188) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", 0)
	ROM_LOAD64_WORD( "222a04.3j", 0x000000, 0x200000, CRC(2533b95a) SHA1(35910836b6030130d742eae6c4bf1cdf1ff43fa4) )
	ROM_LOAD64_WORD( "222a05.1j", 0x000002, 0x200000, CRC(731b7f93) SHA1(b676fff2ede5aa72c49fe12736cd60766462fe0b) )
	ROM_LOAD64_WORD( "222a06.3l", 0x000004, 0x200000, CRC(97b767d3) SHA1(3d879c431586da2f88c632ab1a531b4a5ec96939) )
	ROM_LOAD64_WORD( "222a07.1l", 0x000006, 0x200000, CRC(430bc873) SHA1(ea483195bb7f20ef3df7cfba153e5f6f8d53e5f9) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x200000, "gfx3", 0)
	ROM_LOAD( "222a08.25k", 0x000000, 0x200000, CRC(6410ee1b) SHA1(2296aafd3ba25f63a12130f7b58de53e88f14e92) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x200000, "gfx4", 0)
	ROM_LOAD( "222a09.25l", 0x000000, 0x200000, CRC(f7b3f070) SHA1(50ebd8cfcda292a3df5664de50f9212108d58923) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "222a03.7c", 0x000000, 0x40000, CRC(1924467b) SHA1(57922090509bcc63b4783e8f2c5e95afd2090b87) )
ROM_END

ROM_START( dbza )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "222a11.9e", 0x000000, 0x80000, CRC(60c7d9b2) SHA1(718ef89e89b3943845e91bedfc5c1d26229f9fe5) )
	ROM_LOAD16_BYTE( "222a12.9f", 0x000001, 0x80000, CRC(6ebc6853) SHA1(e9b2068246228968cc6b8554215563cacaa5ba9f) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("222a10.5e", 0x000000, 0x08000, CRC(1c93e30a) SHA1(8545a0ac5126b3c855e1901b186f57820699895d) )

	/* tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD32_WORD( "222a01.27c", 0x000000, 0x200000, CRC(9fce4ed4) SHA1(81e19375b351ee247f066434dd595149333d73c5) )
	ROM_LOAD32_WORD( "222a02.27e", 0x000002, 0x200000, CRC(651acaa5) SHA1(33942a90fb294b5da6a48e5bfb741b31babca188) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", 0)
	ROM_LOAD64_WORD( "222a04.3j", 0x000000, 0x200000, CRC(2533b95a) SHA1(35910836b6030130d742eae6c4bf1cdf1ff43fa4) )
	ROM_LOAD64_WORD( "222a05.1j", 0x000002, 0x200000, CRC(731b7f93) SHA1(b676fff2ede5aa72c49fe12736cd60766462fe0b) )
	ROM_LOAD64_WORD( "222a06.3l", 0x000004, 0x200000, CRC(97b767d3) SHA1(3d879c431586da2f88c632ab1a531b4a5ec96939) )
	ROM_LOAD64_WORD( "222a07.1l", 0x000006, 0x200000, CRC(430bc873) SHA1(ea483195bb7f20ef3df7cfba153e5f6f8d53e5f9) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x200000, "gfx3", 0)
	ROM_LOAD( "222a08.25k", 0x000000, 0x200000, CRC(6410ee1b) SHA1(2296aafd3ba25f63a12130f7b58de53e88f14e92) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x200000, "gfx4", 0)
	ROM_LOAD( "222a09.25l", 0x000000, 0x200000, CRC(f7b3f070) SHA1(50ebd8cfcda292a3df5664de50f9212108d58923) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "222a03.7c", 0x000000, 0x40000, CRC(1924467b) SHA1(57922090509bcc63b4783e8f2c5e95afd2090b87) )
ROM_END

ROM_START( dbz2 )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "a9e.9e", 0x000000, 0x80000, CRC(e6a142c9) SHA1(7951c8f7036a67a0cd3260f434654820bf3e603f) )
	ROM_LOAD16_BYTE( "a9f.9f", 0x000001, 0x80000, CRC(76cac399) SHA1(af6daa1f8b87c861dc62adef5ca029190c3cb9ae) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("s-001.5e", 0x000000, 0x08000, CRC(154e6d03) SHA1(db15c20982692271f40a733dfc3f2486221cd604) )

	/* tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD32_WORD( "ds-b01.27c", 0x000000, 0x200000, CRC(8dc39972) SHA1(c6e3d4e0ff069e08bdb68e2b0ad24cc7314e4e93) )
	ROM_LOAD32_WORD( "ds-b02.27e", 0x000002, 0x200000, CRC(7552f8cd) SHA1(1f3beffe9733b1a18d44b5e8880ff1cc97e7a8ab) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", 0)
	ROM_LOAD64_WORD( "ds-o01.3j", 0x000000, 0x200000, CRC(d018531f) SHA1(d4082fe28e9f1f3f35aa75b4be650cadf1cef192) )
	ROM_LOAD64_WORD( "ds-o02.1j", 0x000002, 0x200000, CRC(5a0f1ebe) SHA1(3bb9e1389299dc046a24740ef1a1c543e44b5c37) )
	ROM_LOAD64_WORD( "ds-o03.3l", 0x000004, 0x200000, CRC(ddc3bef1) SHA1(69638ef53f627a238a12b6c206d57faadf894893) )
	ROM_LOAD64_WORD( "ds-o04.1l", 0x000006, 0x200000, CRC(b5df6676) SHA1(194cfce460ccd29e2cceec577aae4ec936ae88e5) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x400000, "gfx3", 0)
	ROM_LOAD( "ds-p01.25k", 0x000000, 0x200000, CRC(1c7aad68) SHA1(a5296cf12cec262eede55397ea929965576fea81) )
	ROM_LOAD( "ds-p02.27k", 0x200000, 0x200000, CRC(e4c3a43b) SHA1(f327f75fe82f8aafd2cfe6bdd3a426418615974b) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x400000, "gfx4", 0)
	ROM_LOAD( "ds-p03.25l", 0x000000, 0x200000, CRC(1eaa671b) SHA1(1875eefc6f2c3fc8feada56bfa6701144e8ef64b) )
	ROM_LOAD( "ds-p04.27l", 0x200000, 0x200000, CRC(5845ff98) SHA1(73b4c3f439321ce9c462119fe933e7cbda8cd498) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "pcm.7c", 0x000000, 0x40000, CRC(b58c884a) SHA1(0e2a7267e9dff29c9af25558081ec9d56629bc43) )
ROM_END

DRIVER_INIT_MEMBER(dbz_state,dbz)
{
	UINT16 *ROM;

	ROM = (UINT16 *)memregion("maincpu")->base();

	// to avoid crash during loop at 0x00076e after D4 > 0x80 (reading tiles region out of bounds)
	ROM[0x76c/2] = 0x007f;    /* 0x00ff */
	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0x7b0/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #-$1e0d, D3 */
	ROM[0x7b2/2] = 0x4e71;    /* 0xe1f3 */
	ROM[0x7b4/2] = 0x4e71;    /* 0x6600 - bne     $7d6 */
	ROM[0x7b6/2] = 0x4e71;    /* 0x0020 */
	ROM[0x7c0/2] = 0x4e71;    /* 0x0c45 - cmpi.w  #-$7aad, D5 */
	ROM[0x7c2/2] = 0x4e71;    /* 0x8553 */
	ROM[0x7c4/2] = 0x4e71;    /* 0x6600 - bne     $7d6 */
	ROM[0x7c6/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test (A and B)
	ROM[0x9a8/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$43c0, D3 */
	ROM[0x9aa/2] = 0x4e71;    /* 0x43c0 */
	ROM[0x9ac/2] = 0x4e71;    /* 0x6600 - bne     $a00 */
	ROM[0x9ae/2] = 0x4e71;    /* 0x0052 */
	ROM[0x9ea/2] = 0x4e71;    /* 0x0c44 - cmpi.w  #-$13de, D4 */
	ROM[0x9ec/2] = 0x4e71;    /* 0xec22 */
	ROM[0x9ee/2] = 0x4e71;    /* 0x6600 - bne     $a00 */
	ROM[0x9f0/2] = 0x4e71;    /* 0x0010 */
	// prog ROM test
	ROM[0x80c/2] = 0x4e71;    /* 0xb650 - cmp.w   (A0), D3 */
	ROM[0x80e/2] = 0x4e71;    /* 0x6600 - bne     $820 */
	ROM[0x810/2] = 0x4e71;    /* 0x005e */
}

DRIVER_INIT_MEMBER(dbz_state,dbza)
{
	UINT16 *ROM;

	ROM = (UINT16 *)memregion("maincpu")->base();

	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0x78c/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #-$1236, D3 */
	ROM[0x78e/2] = 0x4e71;    /* 0x0010 */
	ROM[0x790/2] = 0x4e71;    /* 0x6600 - bne     $7a2 */
	ROM[0x792/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test
	ROM[0x982/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$437e, D3 */
	ROM[0x984/2] = 0x4e71;    /* 0x437e */
	ROM[0x986/2] = 0x4e71;    /* 0x6600 - bne     $9a0 */
	ROM[0x988/2] = 0x4e71;    /* 0x0018 */
	ROM[0x98a/2] = 0x4e71;    /* 0x0c44 - cmpi.w  #$65e8, D4 */
	ROM[0x98c/2] = 0x4e71;    /* 0x65e8 */
	ROM[0x98e/2] = 0x4e71;    /* 0x6600 - bne     $9a0 */
	ROM[0x990/2] = 0x4e71;    /* 0x0010 */
}

DRIVER_INIT_MEMBER(dbz_state,dbz2)
{
	UINT16 *ROM;

	ROM = (UINT16 *)memregion("maincpu")->base();

	// to avoid crash during loop at 0x000a4a after D4 > 0x80 (reading tiles region out of bounds)
	ROM[0xa48/2] = 0x007f;    /* 0x00ff */
	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0xa88/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$e58, D3 */
	ROM[0xa8a/2] = 0x4e71;    /* 0x0e58 */
	ROM[0xa8c/2] = 0x4e71;    /* 0x6600 - bne     $aae */
	ROM[0xa8e/2] = 0x4e71;    /* 0x0020 */
	ROM[0xa98/2] = 0x4e71;    /* 0x0c45 - cmpi.w  #-$3d20, D5 */
	ROM[0xa9a/2] = 0x4e71;    /* 0xc2e0 */
	ROM[0xa9c/2] = 0x4e71;    /* 0x6600 - bne     $aae */
	ROM[0xa9e/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test (0 to 3)
	ROM[0xc66/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xc68/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xc6a/2] = 0x4e71;    /* 0x005e */
	ROM[0xc7c/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xc7e/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xc80/2] = 0x4e71;    /* 0x0048 */
	ROM[0xc9e/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xca0/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xca2/2] = 0x4e71;    /* 0x0026 */
	ROM[0xcb4/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xcb6/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xcb8/2] = 0x4e71;    /* 0x0010 */
	// prog ROM test
	ROM[0xae4/2] = 0x4e71;    /* 0xb650 - cmp.w   (A0), D3 */
	ROM[0xae6/2] = 0x4e71;    /* 0x6600 - bne     $af8 */
	ROM[0xae8/2] = 0x4e71;    /* 0x005e */
}

GAME( 1993, dbz,  0,   dbz, dbz, dbz_state,  dbz,  ROT0, "Banpresto", "Dragonball Z (rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // crashes MAME in tile/PSAC2 ROM test
GAME( 1993, dbza, dbz, dbz, dbza, dbz_state, dbza, ROT0, "Banpresto", "Dragonball Z (rev A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, dbz2, 0,   dbz, dbz2, dbz_state, dbz2, ROT0, "Banpresto", "Dragonball Z 2 - Super Battle", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // crashes MAME in tile/PSAC2 ROM test
