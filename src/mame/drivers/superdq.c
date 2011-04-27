/*************************************************************************

    Universal System 1

    Games Supported:
    - Universal Super Don Quix-ote

**************************************************************************

    ROM Revisions
    -------------


    Laserdisc Players Used
    ----------------------
    Pioneer LD-V1000

*************************************************************************/

#define MASTER_CLOCK	20000000


#include "emu.h"
#include "cpu/z80/z80.h"
#include "render.h"
#include "sound/sn76496.h"
#include "machine/laserdsc.h"
#include "video/resnet.h"

class superdq_state : public driver_device
{
public:
	superdq_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	device_t *m_laserdisc;
	UINT8 m_ld_in_latch;
	UINT8 m_ld_out_latch;

	UINT8 *m_videoram;
	tilemap_t *m_tilemap;
	int m_color_bank;
};

static TILE_GET_INFO( get_tile_info )
{
	superdq_state *state = machine.driver_data<superdq_state>();
	int tile = state->m_videoram[tile_index];

	SET_TILE_INFO(0, tile, state->m_color_bank, 0);
}

static VIDEO_START( superdq )
{
	superdq_state *state = machine.driver_data<superdq_state>();

	state->m_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static SCREEN_UPDATE( superdq )
{
	superdq_state *state = screen->machine().driver_data<superdq_state>();

	tilemap_draw(bitmap, cliprect, state->m_tilemap, 0, 0);

	return 0;
}



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

static PALETTE_INIT( superdq )
{
	int i;
	static const int resistances[3] = { 820, 390, 200 };
	double rweights[3], gweights[3], bweights[2];

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], rweights, 220, 0,
			3,	&resistances[0], gweights, 220, 0,
			2,	&resistances[1], bweights, 220, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 7) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		r = combine_3_weights(rweights, bit2, bit1, bit0);

		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		g = combine_3_weights(gweights, bit2, bit1, bit0);

		/* blue component */
		bit0 = (color_prom[i] >> 1) & 0x01;
		bit1 = (color_prom[i] >> 0) & 0x01;
		b = combine_2_weights(bweights, bit1, bit0);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static MACHINE_RESET( superdq )
{
	superdq_state *state = machine.driver_data<superdq_state>();

	state->m_ld_in_latch = 0;
	state->m_ld_out_latch = 0xff;
	state->m_color_bank = 0;
}

static INTERRUPT_GEN( superdq_vblank )
{
	superdq_state *state = device->machine().driver_data<superdq_state>();

	/* status is read when the STATUS line from the laserdisc
       toggles (600usec after the vblank). We could set up a
       timer to do that, but this works as well */
	state->m_ld_in_latch = laserdisc_data_r(state->m_laserdisc);

	/* command is written when the COMMAND line from the laserdisc
       toggles (680usec after the vblank). We could set up a
       timer to do that, but this works as well */
	laserdisc_data_w(state->m_laserdisc, state->m_ld_out_latch);
	device_set_input_line(device, 0, ASSERT_LINE);
}

static WRITE8_HANDLER( superdq_videoram_w )
{
	superdq_state *state = space->machine().driver_data<superdq_state>();

	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tilemap,offset);
}

static WRITE8_HANDLER( superdq_io_w )
{
	superdq_state *state = space->machine().driver_data<superdq_state>();
	int 			i;
	static const UINT8 black_color_entries[] = {7,15,16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

	if ( data & 0x40 ) /* bit 6 = irqack */
		cputag_set_input_line(space->machine(), "maincpu", 0, CLEAR_LINE);

	coin_counter_w( space->machine(), 0, data & 0x08 );
	coin_counter_w( space->machine(), 1, data & 0x04 );

	state->m_color_bank = ( data & 2 ) ? 1 : 0;

	for( i = 0; i < ARRAY_LENGTH( black_color_entries ); i++ )
	{
		int index = black_color_entries[i];
		if (data & 0x80)
			palette_set_color(space->machine(), index, palette_get_color(space->machine(), index) & MAKE_ARGB(0,255,255,255));
		else
			palette_set_color(space->machine(), index, palette_get_color(space->machine(), index) | MAKE_ARGB(255,0,0,0));
	}

	/*
        bit 5 = DISP1?
        bit 4 = DISP2?
        bit 0 = unused
    */
}

static READ8_HANDLER( superdq_ld_r )
{
	superdq_state *state = space->machine().driver_data<superdq_state>();

	return state->m_ld_in_latch;
}

static WRITE8_HANDLER( superdq_ld_w )
{
	superdq_state *state = space->machine().driver_data<superdq_state>();

	state->m_ld_out_latch = data;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( superdq_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x5c00, 0x5fff) AM_RAM_WRITE(superdq_videoram_w) AM_BASE_MEMBER(superdq_state,m_videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( superdq_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(superdq_ld_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW2")
	AM_RANGE(0x04, 0x04) AM_READ(superdq_ld_r) AM_DEVWRITE("snsnd", sn76496_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(superdq_io_w)
	AM_RANGE(0x0c, 0x0d) AM_NOP /* HD46505S */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( superdq )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )	/* Service button */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )		/* TEST button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Lives" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( superdq )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_START( superdq )
{
	superdq_state *state = machine.driver_data<superdq_state>();

	state->m_laserdisc = machine.device("laserdisc");
}


static MACHINE_CONFIG_START( superdq, superdq_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(superdq_map)
	MCFG_CPU_IO_MAP(superdq_io)
	MCFG_CPU_VBLANK_INT("screen", superdq_vblank)

	MCFG_MACHINE_START(superdq)
	MCFG_MACHINE_RESET(superdq)

	MCFG_LASERDISC_ADD("laserdisc", PIONEER_LDV1000, "screen", "ldsound")
	MCFG_LASERDISC_OVERLAY(superdq, 256, 256, BITMAP_FORMAT_INDEXED16)

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_INDEXED16)

	MCFG_GFXDECODE(superdq)
	MCFG_PALETTE_LENGTH(32)

	MCFG_PALETTE_INIT(superdq)
	MCFG_VIDEO_START(superdq)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("snsnd", SN76496, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.8)

	MCFG_SOUND_ADD("ldsound", LASERDISC_SOUND, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( superdq )		/* long scenes */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq-prog.bin", 0x0000, 0x4000, CRC(96b931e2) SHA1(a2408272e19b02755368a6d7e526eec15896e586) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq-char.bin", 0x0000, 0x2000, CRC(5fb0e440) SHA1(267413aeb36b661458b7229d65d7b1d03562a1d3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END

ROM_START( superdqs )		/* short scenes */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq_c45.rom", 0x0000, 0x4000, CRC(0f4d4832) SHA1(c6db63721f0c73151eb9a678ceafd0e7d6121fd3) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq_a8.rom", 0x0000, 0x2000, CRC(7d981a14) SHA1(0a0949113b80c30adbb5bdb108d396993225be5b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END

ROM_START( superdqa )		/* short scenes, alternate */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdq_c45a.rom", 0x0000, 0x4000, CRC(b12ce1f8) SHA1(3f0238ea73a6d3e1fe62f83ed3343ca4c268bdd6) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sdq_a8.rom", 0x0000, 0x2000, CRC(7d981a14) SHA1(0a0949113b80c30adbb5bdb108d396993225be5b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdq-cprm.bin", 0x0000, 0x0020, CRC(96701569) SHA1(b0f40373735d1af0c62e5ab06045a064b4eb1794) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "superdq", 0, NO_DUMP )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, superdq,  0,		superdq, superdq, 0, ROT0, "Universal", "Super Don Quix-ote (Long Scenes)", GAME_NOT_WORKING )
GAME( 1984, superdqs, superdq,  superdq, superdq, 0, ROT0, "Universal", "Super Don Quix-ote (Short Scenes)", GAME_NOT_WORKING )
GAME( 1984, superdqa, superdq,  superdq, superdq, 0, ROT0, "Universal", "Super Don Quix-ote (Short Scenes, Alt)", GAME_NOT_WORKING )
