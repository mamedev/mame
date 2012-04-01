/****************************************************************************

Ace by Allied Leisure

Driver by Jarek Burczynski
2002.09.19



Allied Leisure 1976
"MAJOR MFG. INC. SUNNYVALE, CA" in PCB etch

18MHz
                                                          5MHz

8080


2101
2101


A5               3106          3106         3106
A4
A3                                                      3622.K4
A2                                   2101
A1                   2101            2101

                                                         [ RANGE ] [ TIME ]
                                                        (two 0-9 thumbwheel switches)


5x2101 - SRAM 256x4
3x3106 - SRAM 256x1
1x3622 - ROM 512x4


****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"

#define MASTER_CLOCK XTAL_18MHz

// ace_state was also defined in mess/drivers/ace.c
class aceal_state : public driver_device
{
public:
	aceal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8 *  m_ram2;
	UINT8 *  m_scoreram;
	UINT8 *  m_characterram;

	/* input-related */
	int m_objpos[8];
};


static WRITE8_HANDLER( ace_objpos_w )
{
	aceal_state *state = space->machine().driver_data<aceal_state>();
	state->m_objpos[offset] = data;
}

#if 0
static READ8_HANDLER( ace_objpos_r )
{
	aceal_state *state = space->machine().driver_data<aceal_state>();
	return state->m_objpos[offset];
}
#endif

static VIDEO_START( ace )
{
	aceal_state *state = machine.driver_data<aceal_state>();
	gfx_element_set_source(machine.gfx[1], state->m_characterram);
	gfx_element_set_source(machine.gfx[2], state->m_characterram);
	gfx_element_set_source(machine.gfx[3], state->m_characterram);
	gfx_element_set_source(machine.gfx[4], state->m_scoreram);
}

static SCREEN_UPDATE_IND16( ace )
{
	aceal_state *state = screen.machine().driver_data<aceal_state>();
	int offs;

	/* first of all, fill the screen with the background color */
	bitmap.fill(0, cliprect);

	drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[1],
			0,
			0,
			0, 0,
			state->m_objpos[0], state->m_objpos[1]);

	drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[2],
			0,
			0,
			0, 0,
			state->m_objpos[2], state->m_objpos[3]);

	drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[3],
			0,
			0,
			0, 0,
			state->m_objpos[4], state->m_objpos[5]);

	for (offs = 0; offs < 8; offs++)
	{
		drawgfx_opaque(bitmap,/* ?? */
				cliprect, screen.machine().gfx[4],
				offs,
				0,
				0, 0,
				10 * 8 + offs * 16, 256 - 16);
	}
	return 0;
}


static PALETTE_INIT( ace )
{
	palette_set_color(machine, 0, MAKE_RGB(0x00,0x00,0x00)); /* black */
	palette_set_color(machine, 1, MAKE_RGB(0xff,0xff,0xff)); /* white */
}


static WRITE8_HANDLER( ace_characterram_w )
{
	aceal_state *state = space->machine().driver_data<aceal_state>();
	if (state->m_characterram[offset] != data)
	{
		if (data & ~0x07)
		{
			logerror("write to %04x data = %02x\n", 0x8000 + offset, data);
			popmessage("write to %04x data = %02x\n", 0x8000 + offset, data);
		}
		state->m_characterram[offset] = data;
		gfx_element_mark_dirty(space->machine().gfx[1], 0);
		gfx_element_mark_dirty(space->machine().gfx[2], 0);
		gfx_element_mark_dirty(space->machine().gfx[3], 0);
	}
}

static WRITE8_HANDLER( ace_scoreram_w )
{
	aceal_state *state = space->machine().driver_data<aceal_state>();
	state->m_scoreram[offset] = data;
	gfx_element_mark_dirty(space->machine().gfx[4], offset / 32);
}

static READ8_HANDLER( unk_r )
{
	return space->machine().rand() & 0xff;
}


/* 5x2101 - SRAM 256x4 */
/* 3x3106 - SRAM 256x1 */
/* 1x3622 - ROM 512x4  - doesn't seem to be used ????????????*/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, aceal_state )

	AM_RANGE(0x0000, 0x09ff) AM_ROM

	AM_RANGE(0x2000, 0x20ff) AM_RAM_WRITE_LEGACY(ace_scoreram_w) AM_BASE( m_scoreram)	/* 2x2101 */
	AM_RANGE(0x8300, 0x83ff) AM_RAM AM_BASE( m_ram2)	/* 2x2101 */
	AM_RANGE(0x8000, 0x80ff) AM_RAM_WRITE_LEGACY(ace_characterram_w) AM_BASE( m_characterram)	/* 3x3101 (3bits: 0, 1, 2) */

	AM_RANGE(0xc000, 0xc005) AM_WRITE_LEGACY(ace_objpos_w)

	/* players inputs */
	AM_RANGE(0xc008, 0xc008) AM_READ_PORT("c008")
	AM_RANGE(0xc009, 0xc009) AM_READ_PORT("c009")
	AM_RANGE(0xc00a, 0xc00a) AM_READ_PORT("c00a")
	AM_RANGE(0xc00b, 0xc00b) AM_READ_PORT("c00b")
	AM_RANGE(0xc00c, 0xc00c) AM_READ_PORT("c00c")
	AM_RANGE(0xc00d, 0xc00d) AM_READ_PORT("c00d")
	AM_RANGE(0xc00e, 0xc00e) AM_READ_PORT("c00e")
	AM_RANGE(0xc00f, 0xc00f) AM_READ_PORT("c00f")
	AM_RANGE(0xc010, 0xc010) AM_READ_PORT("c010")
	AM_RANGE(0xc011, 0xc011) AM_READ_PORT("c011")

	AM_RANGE(0xc012, 0xc012) AM_READ_LEGACY(unk_r)

	/* vblank */
	AM_RANGE(0xc014, 0xc014) AM_READ_PORT("c014")

	/* coin */
	AM_RANGE(0xc015, 0xc015) AM_READ_PORT("c015")

	/* start (must read 1 at least once to make the game run) */
	AM_RANGE(0xc016, 0xc016) AM_READ_PORT("c016")

	AM_RANGE(0xc017, 0xc017) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc018, 0xc018) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc019, 0xc019) AM_READ_LEGACY(unk_r)

	AM_RANGE(0xc020, 0xc020) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc021, 0xc021) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc022, 0xc022) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc023, 0xc023) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc024, 0xc024) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc025, 0xc025) AM_READ_LEGACY(unk_r)
	AM_RANGE(0xc026, 0xc026) AM_READ_LEGACY(unk_r)

ADDRESS_MAP_END


static INPUT_PORTS_START( ace )
	PORT_START("c008")	/* player thrust */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Thrust")

	PORT_START("c009")	/* player slowdown */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Slowdown")

	PORT_START("c00a")	/* player left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("c00b")	/* player right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("c00c")	/* player fire */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")

	PORT_START("c00d")	/* enemy thrust */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Thrust")

	PORT_START("c00e")	/* enemy slowdown */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Slowdown")

	PORT_START("c00f")	/* enemy left  */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("c010")	/* enemy right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("c011")	/* enemy fire */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")

	//c012

	PORT_START("c014")	/* VBLANK??? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("c015")	/* coin input */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("c016")	/* game start */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16,16,	/* 16*16 chars */
	8,	/* 8 characters */
	1,		/* 1 bit per pixel */
	{ 4 },	/* character rom is 512x4 bits (3622 type)*/
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3, 16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8	/* every char takes 64 consecutive bytes */
};

static const gfx_layout charlayout0 =
{
	16,16,	/* 16*16 chars */
	1,	/* 1 characters */
	1,		/* 1 bit per pixel */
	{ 7 },	/* bit 0 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8	/* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout1 =
{
	16,16,	/* 16*16 chars */
	1,	/* 1 characters */
	1,		/* 1 bit per pixel */
	{ 6 },	/* bit 1 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8	/* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	16,16,	/* 16*16 chars */
	1,	/* 1 characters */
	1,		/* 1 bit per pixel */
	{ 5 },	/* bit 2 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8	/* every char takes 256 consecutive bytes */
};

static const gfx_layout scorelayout =
{
	16,16,	/* 16*16 chars */
	8,	/* 8 characters */
	1,		/* 1 bit per pixel */
	{ 0 },	/*  */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8	/* every char takes 32 consecutive bytes */
};

static GFXDECODE_START( ace )
	GFXDECODE_ENTRY( "gfx1", 0     , charlayout,  0, 2 )
	GFXDECODE_ENTRY( NULL          , 0x8000, charlayout0, 0, 2 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL          , 0x8000, charlayout1, 0, 2 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL          , 0x8000, charlayout2, 0, 2 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL          , 0x8000, scorelayout, 0, 2 )    /* the game dynamically modifies this */
GFXDECODE_END

static void ace_postload(running_machine &machine)
{
	gfx_element_mark_dirty(machine.gfx[1], 0);
	gfx_element_mark_dirty(machine.gfx[2], 0);
	gfx_element_mark_dirty(machine.gfx[3], 0);
	gfx_element_mark_dirty(machine.gfx[4], 0);
}

static MACHINE_START( ace )
{
	aceal_state *state = machine.driver_data<aceal_state>();
	state->save_item(NAME(state->m_objpos));
	machine.save().register_postload(save_prepost_delegate(FUNC(ace_postload), &machine));
}

static MACHINE_RESET( ace )
{
	aceal_state *state = machine.driver_data<aceal_state>();
	int i;

	for (i = 0; i < 8; i++)
		state->m_objpos[i] = 0;
}

static MACHINE_CONFIG_START( ace, aceal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, MASTER_CLOCK/9)	/* 2 MHz ? */
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_MACHINE_START(ace)
	MCFG_MACHINE_RESET(ace)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 32*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(ace)

	MCFG_GFXDECODE(ace)
	MCFG_PALETTE_LENGTH(2)

	MCFG_PALETTE_INIT(ace)
	MCFG_VIDEO_START(ace)

	/* sound hardware */
	/* ???? */

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ace.a1",		0x0000, 0x0200, CRC(16811834) SHA1(5502812dd161908eea3fa8851d7e5c1e22b0f8ff) )
	ROM_LOAD( "ace.a2",		0x0200, 0x0200, CRC(f9eae80e) SHA1(8865b86c7b5d57c76312c16f8a614bf35ffaf532) )
	ROM_LOAD( "ace.a3",		0x0400, 0x0200, CRC(c5c63b8c) SHA1(2079dd12ff0c4aafec19aeb9baa70fc9b6788356) )
	ROM_LOAD( "ace.a4",		0x0600, 0x0200, CRC(ea4503aa) SHA1(fea610124b9f7ea18d29b4e4599253ba1ee067e1) )
	ROM_LOAD( "ace.a5",		0x0800, 0x0200, CRC(623c58e7) SHA1(a92418bc323a1ae76eae8e094e4d6ebd1e8da14e) )

	/* not used - I couldn't guess when this should be displayed */
	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "ace.k4",		0x0000, 0x0200, CRC(daa05ec6) SHA1(8b71ffb802293dc93f6b492ff128a704e676a5fd) )

ROM_END

GAME( 1976, ace, 0, ace, ace, 0, ROT0, "Allied Leisure", "Ace", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
