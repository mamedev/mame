/*******************************************************************************

    Irem M107 games:

    Fire Barrel                             (c) 1993 Irem Corporation
    Dream Soccer '94                        (c) 1994 Data East Corporation
    Kick for the Goal                       (c) 1994 Jaleco
    World PK Soccer                         (c) 1995 Jaleco


    Graphics glitches in both games.

    Emulation by Bryan McPhail, mish@tendril.co.uk


To Do:
  Hook up EEPROM in Kick for the Goal / World PK Soccer

2008-08
Dip locations have been added assuming that the layout is the same as the
m92 boards (and earlier Irem boards). However, it would be nice to have them
confirmed for m107 games as well.

*******************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "includes/m107.h"
#include "includes/iremipt.h"
#include "machine/irem_cpu.h"
#include "sound/2151intf.h"
#include "sound/iremga20.h"


#define M107_IRQ_0 ((m107_irq_vectorbase+0)/4) /* VBL interrupt*/
#define M107_IRQ_1 ((m107_irq_vectorbase+4)/4) /* ??? */
#define M107_IRQ_2 ((m107_irq_vectorbase+8)/4) /* Raster interrupt */
#define M107_IRQ_3 ((m107_irq_vectorbase+12)/4) /* ??? */



static emu_timer *scanline_timer;
static UINT8 m107_irq_vectorbase;

static TIMER_CALLBACK( m107_scanline_interrupt );

/*****************************************************************************/

static WRITE16_HANDLER( bankswitch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 *RAM = memory_region(space->machine, "maincpu");
		memory_set_bankptr(space->machine, "bank1",&RAM[0x100000 + ((data&0x7)*0x10000)]);
	}
}

static MACHINE_START( m107 )
{
	scanline_timer = timer_alloc(machine, m107_scanline_interrupt, NULL);
}

static MACHINE_RESET( m107 )
{
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), 0);
}

/*****************************************************************************/

static TIMER_CALLBACK( m107_scanline_interrupt )
{
	int scanline = param;

	/* raster interrupt */
	if (scanline == m107_raster_irq_position)
	{
		video_screen_update_partial(machine->primary_screen, scanline);
		cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, M107_IRQ_2);
	}

	/* VBLANK interrupt */
	else if (scanline == video_screen_get_visible_area(machine->primary_screen)->max_y + 1)
	{
		video_screen_update_partial(machine->primary_screen, scanline);
		cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, M107_IRQ_0);
	}

	/* adjust for next scanline */
	if (++scanline >= video_screen_get_height(machine->primary_screen))
		scanline = 0;
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), scanline);
}


static WRITE16_HANDLER( m107_coincounter_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0,data & 0x01);
		coin_counter_w(space->machine, 1,data & 0x02);
	}
}



enum { VECTOR_INIT, YM2151_ASSERT, YM2151_CLEAR, V30_ASSERT, V30_CLEAR };

static TIMER_CALLBACK( setvector_callback )
{
	static int irqvector;

	switch(param)
	{
		case VECTOR_INIT:	irqvector = 0;		break;
		case YM2151_ASSERT:	irqvector |= 0x2;	break;
		case YM2151_CLEAR:	irqvector &= ~0x2;	break;
		case V30_ASSERT:	irqvector |= 0x1;	break;
		case V30_CLEAR:		irqvector &= ~0x1;	break;
	}

	if (irqvector & 0x2)		/* YM2151 has precedence */
		cpu_set_input_line_vector(devtag_get_device(machine, "soundcpu"), 0, 0x18);
	else if (irqvector & 0x1)	/* V30 */
		cpu_set_input_line_vector(devtag_get_device(machine, "soundcpu"), 0, 0x19);

	if (irqvector == 0)	/* no IRQs pending */
		cputag_set_input_line(machine, "soundcpu", 0, CLEAR_LINE);
	else	/* IRQ pending */
		cputag_set_input_line(machine, "soundcpu", 0, ASSERT_LINE);
}

static WRITE16_HANDLER( m107_soundlatch_w )
{
	timer_call_after_resynch(space->machine, NULL, V30_ASSERT, setvector_callback);
	soundlatch_w(space, 0, data & 0xff);
//      logerror("soundlatch_w %02x\n",data);
}

static int sound_status;

static READ16_HANDLER( m107_sound_status_r )
{
	return sound_status;
}

static READ16_HANDLER( m107_soundlatch_r )
{
	return soundlatch_r(space, offset) | 0xff00;
}

static WRITE16_HANDLER( m107_sound_irq_ack_w )
{
	timer_call_after_resynch(space->machine, NULL, V30_CLEAR,setvector_callback);
}

static WRITE16_HANDLER( m107_sound_status_w )
{
	COMBINE_DATA(&sound_status);
	cputag_set_input_line_and_vector(space->machine, "maincpu", 0, HOLD_LINE, M107_IRQ_3);
}

/*****************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x9ffff) AM_ROM
	AM_RANGE(0xa0000, 0xbffff) AM_ROMBANK("bank1")
	AM_RANGE(0xd0000, 0xdffff) AM_RAM_WRITE(m107_vram_w) AM_BASE(&m107_vram_data)
	AM_RANGE(0xe0000, 0xeffff) AM_RAM /* System ram */
	AM_RANGE(0xf8000, 0xf8fff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0xf9000, 0xf9fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("P1_P2")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("COINS_DSW3")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x06, 0x07) AM_READ_PORT("P3_P4")
	AM_RANGE(0x08, 0x09) AM_READ(m107_sound_status_r)	/* answer from sound CPU */
	AM_RANGE(0x00, 0x01) AM_WRITE(m107_soundlatch_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(m107_coincounter_w)
	AM_RANGE(0x04, 0x05) AM_WRITENOP /* ??? 0008 */
	AM_RANGE(0x06, 0x07) AM_WRITE(bankswitch_w)
	AM_RANGE(0x80, 0x9f) AM_WRITE(m107_control_w)
	AM_RANGE(0xa0, 0xaf) AM_WRITENOP /* Written with 0's in interrupt */
	AM_RANGE(0xb0, 0xb1) AM_WRITE(m107_spritebuffer_w)
	AM_RANGE(0xc0, 0xc3) AM_READNOP /* Only wpksoc: ticket related? */
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x9ff00, 0x9ffff) AM_WRITENOP /* Irq controller? */
	AM_RANGE(0xa0000, 0xa3fff) AM_RAM
	AM_RANGE(0xa8000, 0xa803f) AM_DEVREADWRITE8("irem", irem_ga20_r, irem_ga20_w, 0x00ff)
	AM_RANGE(0xa8040, 0xa8043) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x00ff)
	AM_RANGE(0xa8044, 0xa8045) AM_READWRITE(m107_soundlatch_r, m107_sound_irq_ack_w)
	AM_RANGE(0xa8046, 0xa8047) AM_WRITE(m107_sound_status_w)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( m107_2player )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS(1, 2)

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_VBLANK )

	/* DIP switch bank 3 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW3:8" )

	PORT_START("DSW")
	/* Dip switch bank 1 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Slots" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Common" )
	PORT_DIPSETTING(      0x0000, "Separate" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m107_3player )
	PORT_INCLUDE(m107_2player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "3 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_3
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m107_4player )
	PORT_INCLUDE(m107_3player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "4 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_4
INPUT_PORTS_END

/******************************************************************************/

static INPUT_PORTS_START( firebarr )
	PORT_INCLUDE(m107_2player)

	PORT_MODIFY("COINS_DSW3")
	PORT_DIPNAME( 0x1000, 0x0000, "Continuous Play" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( dsoccr94 )
	PORT_INCLUDE(m107_4player)

	PORT_MODIFY("COINS_DSW3")
	PORT_DIPNAME( 0x0300, 0x0300, "Player Power" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x0300, "1000" )
	PORT_DIPSETTING(      0x0100, "1500" )
	PORT_DIPSETTING(      0x0200, "2000" )

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, "Time" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1:30" )
	PORT_DIPSETTING(      0x0003, "2:00" )
	PORT_DIPSETTING(      0x0002, "2:30" )
	PORT_DIPSETTING(      0x0001, "3:00" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Mode" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Match Mode" )
	PORT_DIPSETTING(      0x0000, "Power Mode" )
/*
   Match Mode: Winner advances to the next game.  Game Over for the loser
   Power Mode: The Players can play the game until their respective powers run
               out, reguardless of whether they win or lose the game.
               Player 2 can join in any time during the game
               Player power (time) can be adjusted by dip switch #3
*/
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Button" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "Button 1" )
	PORT_DIPSETTING(      0x0020, "Start Button" )

	/* Manual says not to use these SW3:3-8 */
	PORT_START("DSW3")	/* Dip switch bank 3 */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wpksoc )
	PORT_INCLUDE(m107_2player)
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8, 0, 24, 16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout2 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8
};

static GFXDECODE_START( m107 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( firebarr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2,0, 128 )
GFXDECODE_END

/***************************************************************************/

static void sound_irq(const device_config *device, int state)
{
	if (state)
		timer_call_after_resynch(device->machine, NULL, YM2151_ASSERT,setvector_callback);
	else
		timer_call_after_resynch(device->machine, NULL, YM2151_CLEAR,setvector_callback);
}

static const ym2151_interface ym2151_config =
{
	sound_irq
};

/***************************************************************************/

static const nec_config firebarr_config ={ rtypeleo_decryption_table, };

static MACHINE_DRIVER_START( firebarr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V33, 28000000/2)	/* NEC V33, 28MHz clock */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)

	MDRV_CPU_ADD("soundcpu", V30, 14318000/2)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_CONFIG(firebarr_config)

	MDRV_MACHINE_START(m107)
	MDRV_MACHINE_RESET(m107)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(80, 511-112, 8, 247) /* 320 x 240 */

	MDRV_GFXDECODE(firebarr)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(m107)
	MDRV_VIDEO_UPDATE(m107)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 14318180/4)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.40)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.40)

	MDRV_SOUND_ADD("irem", IREMGA20, 14318180/4)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static const nec_config dsoccr94_config ={ dsoccr94_decryption_table, };
static MACHINE_DRIVER_START( dsoccr94 )
	MDRV_IMPORT_FROM(firebarr)

	/* basic machine hardware */
	MDRV_CPU_REPLACE("maincpu", V33, 20000000/2)	/* NEC V33, Could be 28MHz clock? */

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_CONFIG(dsoccr94_config)

	/* video hardware */
	MDRV_GFXDECODE(m107)
MACHINE_DRIVER_END


static const nec_config wpksoc_config ={ leagueman_decryption_table, };
static MACHINE_DRIVER_START( wpksoc )
	MDRV_IMPORT_FROM(firebarr)
	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_CONFIG(wpksoc_config)
MACHINE_DRIVER_END

/***************************************************************************/

ROM_START( firebarr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f4-a-h0-c.9d", 0x000001, 0x40000, CRC(2aa5676e) SHA1(7f51c462c58b63fa4f34ec9dd2ee69c932ebf718) )
	ROM_LOAD16_BYTE( "f4-a-l0-c.9f", 0x000000, 0x40000, CRC(42f75d59) SHA1(eba3a02874d608ecb8c93160c8f0b4c8bb8061d2) )
	ROM_LOAD16_BYTE( "f4-a-h1-c.9e", 0x080001, 0x20000, CRC(bb7f6968) SHA1(366747672aac939454d9915cda5277b0438f063b) )
	ROM_LOAD16_BYTE( "f4-a-l1-c.9h", 0x080000, 0x20000, CRC(9d57edd6) SHA1(16122829b61aa3aee88aeb6634831e8cf95eaee0) )

	ROM_REGION( 0x100000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "f4-b-sh0-b", 0x000001, 0x10000, CRC(30a8e232) SHA1(d4695aed35a1aa796b2872e58a6014e8b28bc154) )
	ROM_LOAD16_BYTE( "f4-b-sl0-b", 0x000000, 0x10000, CRC(204b5f1f) SHA1(f0386500773cd7cca93f0e8e740db29182320c70) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* chars */
	ROM_LOAD16_BYTE( "f4-c00", 0x000000, 0x80000, CRC(50cab384) SHA1(66e88a1dfa943e0d49c2e186ac2f6cbf5cfe0864) )
	ROM_LOAD16_BYTE( "f4-c10", 0x000001, 0x80000, CRC(330c6df2) SHA1(f199d959385398adb6b86ec8ec5de8b40899597c) )
	ROM_LOAD16_BYTE( "f4-c01", 0x100000, 0x80000, CRC(12a698c8) SHA1(74d21768bac70e8cb7e1a6737f758f33869b6af9) )
	ROM_LOAD16_BYTE( "f4-c11", 0x100001, 0x80000, CRC(3f9add18) SHA1(840339a1f33d68c555e42618dd436788639b1edf) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* sprites */
	ROM_LOAD16_BYTE( "f4-000", 0x000000, 0x80000, CRC(920deee9) SHA1(6341eeccdad97fde5337f32f317ddc94f6b8d07a) )
	ROM_LOAD16_BYTE( "f4-001", 0x000001, 0x80000, CRC(e5725eaf) SHA1(c884d69742484a7c07eb0c7882a33d90b240529e) )
	ROM_LOAD16_BYTE( "f4-010", 0x100000, 0x80000, CRC(3505d185) SHA1(1330c18eaadb3e23d6205f3912015cb9ca5f3590) )
	ROM_LOAD16_BYTE( "f4-011", 0x100001, 0x80000, CRC(1912682f) SHA1(d0234877aabf94df7f6a6091e38247954725e1f3) )
	ROM_LOAD16_BYTE( "f4-020", 0x200000, 0x80000, CRC(ec130b8e) SHA1(6a4562f3e39d02f97f3b917e4a51f48b6f43a4c8) )
	ROM_LOAD16_BYTE( "f4-021", 0x200001, 0x80000, CRC(8dd384dc) SHA1(dee79d0d48762b98c20c88ba6617de5e939f596d) )
	ROM_LOAD16_BYTE( "f4-030", 0x300000, 0x80000, CRC(7e7b30cd) SHA1(eca9d2a5d9f9deebb565456018126bc37a1de1d8) )
	ROM_LOAD16_BYTE( "f4-031", 0x300001, 0x80000, CRC(83ac56c5) SHA1(47e1063c71d5570fecf8591c2cb7c74fd45199f5) )

	ROM_REGION( 0x40000, "user1", 0 )	/* sprite tables */
	ROM_LOAD16_BYTE( "f4-b-drh", 0x000001, 0x20000, CRC(12001372) SHA1(a5346d8a741cd1a93aa289562bb56d2fc40c1bbb) )
	ROM_LOAD16_BYTE( "f4-b-drl", 0x000000, 0x20000, CRC(08cb7533) SHA1(9e0d8f8498bddfa1c6135abbab4465e9eeb033fe) )

	ROM_REGION( 0x80000, "irem", 0 )	/* ADPCM samples */
	ROM_LOAD( "f4-b-da0", 0x000000, 0x80000, CRC(7a493e2e) SHA1(f6a8bacbe25760c86bdd8e8bb6d052ff15718eef) )
ROM_END

ROM_START( dsoccr94 )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ds_h0-c.rom", 0x000001, 0x040000, CRC(d01d3fd7) SHA1(925dff999252bf3b920bc0f427744e1464620fe8) )
	ROM_LOAD16_BYTE( "ds_l0-c.rom", 0x000000, 0x040000, CRC(8af0afe2) SHA1(423c77d392a79cdaed66ad8c13039450d34d3f6d) )
	ROM_LOAD16_BYTE( "ds_h1-c.rom", 0x100001, 0x040000, CRC(6109041b) SHA1(063898a88f8a6a9f1510aa55e53a39f037b02903) )
	ROM_LOAD16_BYTE( "ds_l1-c.rom", 0x100000, 0x040000, CRC(97a01f6b) SHA1(e188e28f880f5f3f4d7b49eca639d643989b1468) )

	ROM_REGION( 0x100000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "ds_sh0.rom", 0x000001, 0x010000, CRC(23fe6ffc) SHA1(896377961cafc19e44d9d889f9fbfdbaedd556da) )
	ROM_LOAD16_BYTE( "ds_sl0.rom", 0x000000, 0x010000, CRC(768132e5) SHA1(1bb64516eb58d3b246f08e1c07f091e78085689f) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* chars */
	ROM_LOAD16_BYTE( "ds_c00.rom", 0x000000, 0x100000, CRC(2d31d418) SHA1(6cd0e362bc2e3f2b20d96ee97a04bff46ee3016a) )
	ROM_LOAD16_BYTE( "ds_c10.rom", 0x000001, 0x100000, CRC(57f7bcd3) SHA1(a38e7cdfdea72d882fba414cae391ba09443e73c) )
	ROM_LOAD16_BYTE( "ds_c01.rom", 0x200000, 0x100000, CRC(9d31a464) SHA1(1e38ac296f64d77fabfc0d5f7921a9b7a8424875) )
	ROM_LOAD16_BYTE( "ds_c11.rom", 0x200001, 0x100000, CRC(a372e79f) SHA1(6b0889cfc2970028832566e25257927ddc461ea6) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* sprites */
	ROM_LOAD( "ds_000.rom", 0x000000, 0x100000, CRC(366b3e29) SHA1(cb016dcbdc6e8ea56c28c00135263666b07df991) )
	ROM_LOAD( "ds_010.rom", 0x100000, 0x100000, CRC(28a4cc40) SHA1(7f4e1ef995eaadf1945ee22ab3270cb8a21c601d) )
	ROM_LOAD( "ds_020.rom", 0x200000, 0x100000, CRC(5a310f7f) SHA1(21969e4247c8328d27118d00604096deaf6700af) )
	ROM_LOAD( "ds_030.rom", 0x300000, 0x100000, CRC(328b1f45) SHA1(4cbbd4d9be4fc151d426175bdbd35d8481bf2966) )

	ROM_REGION( 0x100000, "irem", 0 )	 /* ADPCM samples */
	ROM_LOAD( "ds_da0.rom", 0x000000, 0x100000, CRC(67fc52fd) SHA1(5771e948115af8fe4a6d3f448c03a2a9b42b6f20) )
ROM_END

ROM_START( wpksoc )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "pk-h0-eur-d.h0", 0x000001, 0x040000, CRC(b4917788) SHA1(673294c518eaf28354fa6a3058f9325c6d9ddde6) )
	ROM_LOAD16_BYTE( "pk-l0-eur-d.l0", 0x000000, 0x040000, CRC(03816bae) SHA1(832e2ec722b41d41626fec583fc11e9ff62cdaa0) )

	ROM_REGION( 0x100000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "pk-sh0.sh0", 0x000001, 0x010000, CRC(1145998c) SHA1(cdb2a428e0f35302b81696dab02d3dd2c433f6e5) )
	ROM_LOAD16_BYTE( "pk-sl0.sl0", 0x000000, 0x010000, CRC(542ee1c7) SHA1(b934adeecbba17cf96b06a2b1dc1ceaebdf9ad10) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* chars */
	ROM_LOAD16_BYTE( "pk-c00-os.c00", 0x000000, 0x80000, CRC(42ae3d73) SHA1(e4777066155c9882695ebff0412bd879b8d6f716) )
	ROM_LOAD16_BYTE( "pk-c10-os.c10", 0x000001, 0x80000, CRC(86acf45c) SHA1(3b3d2abcf8000161a37d5e2619df529533aea47d) )
	ROM_LOAD16_BYTE( "pk-c01-os.c01", 0x100000, 0x80000, CRC(b0d33f87) SHA1(f2c0e3a10615c6861a3f6fd82a3f066e8e264233) )
	ROM_LOAD16_BYTE( "pk-c11-os.c11", 0x100001, 0x80000, CRC(19de7d63) SHA1(6d0633e412b47accaecc887a5c39f542eda49e81) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* sprites */
	ROM_LOAD16_BYTE( "pk-000.000", 0x000000, 0x80000, CRC(165ce027) SHA1(3510b323c683ade4dd7307b539072bb342b6796d) )
	ROM_LOAD16_BYTE( "pk-001.001", 0x000001, 0x80000, CRC(e2745147) SHA1(99026525449c2ca84e054a7d633c400e0e836461) )
	ROM_LOAD16_BYTE( "pk-010.010", 0x100000, 0x80000, CRC(6c171b73) SHA1(a99c9f012f21373daea08d28554cc36170f4e1fa) )
	ROM_LOAD16_BYTE( "pk-011.011", 0x100001, 0x80000, CRC(471c0bf4) SHA1(1cace5ffd5db91850662de929cb9086dc154d662) )
	ROM_LOAD16_BYTE( "pk-020.020", 0x200000, 0x80000, CRC(c886dad1) SHA1(9b58a2f108547c3f55399932a7e56031c5658737) )
	ROM_LOAD16_BYTE( "pk-021.021", 0x200001, 0x80000, CRC(91e877ff) SHA1(3df095632728ab16ab229d592ab12d3df44b2629) )
	ROM_LOAD16_BYTE( "pk-030.030", 0x300000, 0x80000, CRC(3390621c) SHA1(4138c690666f78b1c5cf83d815ed6b37239a94b4) )
	ROM_LOAD16_BYTE( "pk-031.031", 0x300001, 0x80000, CRC(4d322804) SHA1(b5e2b40e3ce83b6f97b2b57edaa79df6968d0997) )

	ROM_REGION( 0x100000, "irem", 0 )	 /* ADPCM samples */
	ROM_LOAD( "pk-da0.da0", 0x000000, 0x80000, CRC(26a34cf4) SHA1(a8a7cd91cdc6d644ee02ca16e7fdc8debf8f3a5f) )
ROM_END

ROM_START( kftgoal )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "pk-h0-usa-d.h0", 0x000001, 0x040000, CRC(aed4cde0) SHA1(2fe04bf93c353108b82a0b6017229e9b0f451b06) )
	ROM_LOAD16_BYTE( "pk-l0-usa-d.l0", 0x000000, 0x040000, CRC(39fe30d2) SHA1(e0c117da4fe9c779dd534ee0d09685aeb5f579c6) )

	ROM_REGION( 0x100000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "pk-sh0.sh0", 0x000001, 0x010000, CRC(1145998c) SHA1(cdb2a428e0f35302b81696dab02d3dd2c433f6e5) )
	ROM_LOAD16_BYTE( "pk-sl0.sl0", 0x000000, 0x010000, CRC(542ee1c7) SHA1(b934adeecbba17cf96b06a2b1dc1ceaebdf9ad10) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* chars */
	ROM_LOAD16_BYTE( "pk-c00-os.c00", 0x000000, 0x80000, CRC(42ae3d73) SHA1(e4777066155c9882695ebff0412bd879b8d6f716) )
	ROM_LOAD16_BYTE( "pk-c10-os.c10", 0x000001, 0x80000, CRC(86acf45c) SHA1(3b3d2abcf8000161a37d5e2619df529533aea47d) )
	ROM_LOAD16_BYTE( "pk-c01-os.c01", 0x100000, 0x80000, CRC(b0d33f87) SHA1(f2c0e3a10615c6861a3f6fd82a3f066e8e264233) )
	ROM_LOAD16_BYTE( "pk-c11-os.c11", 0x100001, 0x80000, CRC(19de7d63) SHA1(6d0633e412b47accaecc887a5c39f542eda49e81) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* sprites */
	ROM_LOAD16_BYTE( "pk-000-usa.000", 0x000000, 0x80000, CRC(72e905ab) SHA1(5f47f0db0a19311cb74c39ea9d86f6909b926aa8) )
	ROM_LOAD16_BYTE( "pk-001-usa.001", 0x000001, 0x80000, CRC(eec4f43c) SHA1(93133389701c8752fc288f3f82da2646446804ca) )
	ROM_LOAD16_BYTE( "pk-010-usa.010", 0x100000, 0x80000, CRC(b3339d73) SHA1(1f59591a24434cf7d754d211c1a20591c1d7234c) )
	ROM_LOAD16_BYTE( "pk-011-usa.011", 0x100001, 0x80000, CRC(bab2b7cf) SHA1(53997c5dc204a4f510624dcdef949e859ad79d23) )
	ROM_LOAD16_BYTE( "pk-020-usa.020", 0x200000, 0x80000, CRC(740a0bef) SHA1(89782d6d76e0cbd99047dc9a4d3c00bbab0d6bce) )
	ROM_LOAD16_BYTE( "pk-021-usa.021", 0x200001, 0x80000, CRC(f44208a6) SHA1(e6436bacebca786de1521ce7a207aca686e312a0) )
	ROM_LOAD16_BYTE( "pk-030-usa.030", 0x300000, 0x80000, CRC(8eceef50) SHA1(e39a2420a6259a8571a71fd3f9b003b0e0abea3b) )
	ROM_LOAD16_BYTE( "pk-031-usa.031", 0x300001, 0x80000, CRC(8aa7dc04) SHA1(8aebdf50a832acf00fcfebb35ab49a06d13bc444) )

	ROM_REGION( 0x100000, "irem", 0 )	 /* ADPCM samples */
	ROM_LOAD( "pk-da0.da0", 0x000000, 0x80000, CRC(26a34cf4) SHA1(a8a7cd91cdc6d644ee02ca16e7fdc8debf8f3a5f) )

	ROM_REGION( 0x2000, "user1", 0 ) /* ST M28C64C-20PI Eeprom */
	ROM_LOAD( "st-m28c64c.eeprom", 0x000, 0x2000, CRC(8e0c8b7c) SHA1(0b57290d709e6d54ce1bb3a5c01b80590203c1dd) )
ROM_END

/***************************************************************************/

static DRIVER_INIT( firebarr )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	memcpy(RAM + 0xffff0, RAM + 0x7fff0, 0x10); /* Start vector */
	memory_set_bankptr(machine, "bank1", &RAM[0xa0000]); /* Initial bank */

	RAM = memory_region(machine, "soundcpu");
	memcpy(RAM + 0xffff0,RAM + 0x1fff0, 0x10); /* Sound cpu Start vector */

	m107_irq_vectorbase = 0x20;
	m107_spritesystem = 1;
}

static DRIVER_INIT( dsoccr94 )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	memcpy(RAM + 0xffff0, RAM + 0x7fff0, 0x10); /* Start vector */
	memory_set_bankptr(machine, "bank1", &RAM[0xa0000]); /* Initial bank */

	RAM = memory_region(machine, "soundcpu");
	memcpy(RAM + 0xffff0, RAM + 0x1fff0, 0x10); /* Sound cpu Start vector */

	m107_irq_vectorbase = 0x80;
	m107_spritesystem = 0;
}

static DRIVER_INIT( wpksoc )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	memcpy(RAM + 0xffff0, RAM + 0x7fff0, 0x10); /* Start vector */
	memory_set_bankptr(machine, "bank1", &RAM[0xa0000]); /* Initial bank */

	RAM = memory_region(machine, "soundcpu");
	memcpy(RAM + 0xffff0, RAM + 0x1fff0, 0x10); /* Sound cpu Start vector */


	m107_irq_vectorbase = 0x80;
	m107_spritesystem = 0;
}

/***************************************************************************/

GAME( 1993, firebarr,      0, firebarr, firebarr, firebarr, ROT270, "Irem", "Fire Barrel (Japan)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, dsoccr94,      0, dsoccr94, dsoccr94, dsoccr94, ROT0,   "Irem (Data East Corporation license)", "Dream Soccer '94", 0 )
GAME( 1995, wpksoc,        0, wpksoc,   wpksoc,   wpksoc,   ROT0,   "Jaleco", "World PK Soccer", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, kftgoal,  wpksoc, wpksoc,   wpksoc,   wpksoc,   ROT0,   "Jaleco", "Kick for the Goal", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
