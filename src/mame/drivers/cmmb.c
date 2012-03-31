/***************************************************************************

Centipede, Millipede, Missile Command, Let's Go Bowling "Multipede"
(c) 1980-2 / 2002 - Infogrames / CosmoDog

preliminary driver by Angelo Salese

Earlier revisions of this cabinet did not include the bowling game.
 Known to exist "CMM Rev 1.03" (without Let's Go Bowling)

TODO:
- program banking;
- finish video emulation;
- inputs;
- sound;
- NVRAM (EEPROM) at U8 or U11 on PCB
- driver probably needs rewriting, at least the i/o part;
- Is the W65C02S the same as the 65SC02 core or are there any
  extra Op-codes & addressing modes?

Probably on the CPLD (CY39100V208B) - Quoted from Cosmodog's website:
 "Instead, we used a programmable chip that we could reconfigure very
 quickly while the game is running. So, during that 1/8th of a second
 or so when the screen goes black while it switches games, it's actually
 reloading the hardware with a whole new design to run the next game."

============================================================================

Centipede, Millipede, Missile Command, Let's Go Bowling.
Team Play

Multipede 1.00 PCB by CosmoDog

Flash ROM AT29C020

Cypress CY39100V208B
CPU WDC 65C02S8P-14
CY37128-P100
CYC1399
OSC @ 72.576MHz

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"


class cmmb_state : public driver_device
{
public:
	cmmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_irq_mask;
};


static VIDEO_START( cmmb )
{

}

static SCREEN_UPDATE_IND16( cmmb )
{
	cmmb_state *state = screen.machine().driver_data<cmmb_state>();
	UINT8 *videoram = state->m_videoram;
	const gfx_element *gfx = screen.machine().gfx[0];
	int count = 0x00000;

	int y,x;


	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = videoram[count] & 0x3f;
			int colour = (videoram[count] & 0xc0)>>6;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,colour,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

static READ8_HANDLER( cmmb_charram_r )
{
	UINT8 *GFX = space->machine().region("gfx")->base();

	return GFX[offset];
}

static WRITE8_HANDLER( cmmb_charram_w )
{
	UINT8 *GFX = space->machine().region("gfx")->base();

	GFX[offset] = data;

	offset&=0xfff;

	/* dirty char */
	gfx_element_mark_dirty(space->machine().gfx[0], offset >> 4);
    gfx_element_mark_dirty(space->machine().gfx[1], offset >> 5);
}


static WRITE8_HANDLER( cmmb_paletteram_w )
{
    /* RGB output is inverted */
    paletteram_RRRGGGBB_w(space,offset,~data);
}

static READ8_HANDLER( cmmb_input_r )
{
	//printf("%02x R\n",offset);
	switch(offset)
	{
		case 0x00: return input_port_read(space->machine(), "IN2");
		case 0x03: return 4; //eeprom?
		case 0x0e: return input_port_read(space->machine(), "IN0");
		case 0x0f: return input_port_read(space->machine(), "IN1");
	}

	return 0xff;
}


/*
    {
        UINT8 *ROM = space->machine().region("maincpu")->base();
        UINT32 bankaddress;

        bankaddress = 0x10000 + (0x10000 * (data & 0x03));
        memory_set_bankptr(space->machine(), "bank1", &ROM[bankaddress]);
    }
*/

static WRITE8_HANDLER( cmmb_output_w )
{
	cmmb_state *state = space->machine().driver_data<cmmb_state>();
	//printf("%02x -> [%02x] W\n",data,offset);
	switch(offset)
	{
		case 0x01:
			{
				UINT8 *ROM = space->machine().region("maincpu")->base();
				UINT32 bankaddress;

				bankaddress = 0x1c000 + (0x10000 * (data & 0x03));
				memory_set_bankptr(space->machine(), "bank1", &ROM[bankaddress]);
			}
			break;
		case 0x03:
			state->m_irq_mask = data & 0x80;
			break;
		case 0x07:
			break;
	}
}

static READ8_HANDLER( kludge_r )
{
	return space->machine().rand();
}

/* overlap empty addresses */
static ADDRESS_MAP_START( cmmb_map, AS_PROGRAM, 8, cmmb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM /* zero page address */
//  AM_RANGE(0x13c0, 0x13ff) AM_RAM //spriteram
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_BASE_MEMBER(cmmb_state, m_videoram)
	AM_RANGE(0x2480, 0x249f) AM_RAM_WRITE(cmmb_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x4000, 0x400f) AM_READWRITE(cmmb_input_r,cmmb_output_w) //i/o
	AM_RANGE(0x4900, 0x4900) AM_READ(kludge_r)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(cmmb_charram_r,cmmb_charram_w)
	AM_RANGE(0xc000, 0xc00f) AM_READWRITE(cmmb_input_r,cmmb_output_w) //i/o
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( cmmb )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout spritelayout =
{
	8,16,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	8*32
};


static GFXDECODE_START( cmmb )
	GFXDECODE_ENTRY( "gfx", 0, charlayout,     0x00, 4 )
	GFXDECODE_ENTRY( "gfx", 0, spritelayout,   0x10, 4 )
GFXDECODE_END

static INTERRUPT_GEN( cmmb_irq )
{
	//if(device->machine().input().code_pressed_once(KEYCODE_Z))
	//  device_set_input_line(device, 0, HOLD_LINE);
}

static MACHINE_RESET( cmmb )
{
}

static MACHINE_CONFIG_START( cmmb, cmmb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M65SC02,XTAL_72_576MHz/5) // Unknown clock, but chip rated for 14MHz
	MCFG_CPU_PROGRAM_MAP(cmmb_map)
	MCFG_CPU_VBLANK_INT("screen",cmmb_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // unknown
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(cmmb)

	MCFG_GFXDECODE(cmmb)
	MCFG_PALETTE_LENGTH(512)

	MCFG_VIDEO_START(cmmb)

	MCFG_MACHINE_RESET(cmmb)

	/* sound hardware */
//  MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, 8000000/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cmmb162 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cmmb162.u2",   0x10000, 0x40000, CRC(71a5a75d) SHA1(0ad7b97580082cda98cb1e8aab8efcf491d0ed25) )
	ROM_COPY( "maincpu",	  0x18000, 0x08000, 0x08000 )

	ROM_REGION( 0x1000, "gfx", ROMREGION_ERASE00 )
ROM_END

GAME( 2002, cmmb162,  0,       cmmb,  cmmb,  0, ROT270, "Infogrames / Cosmodog", "Multipede (rev 1.62)", GAME_NO_SOUND|GAME_NOT_WORKING )
