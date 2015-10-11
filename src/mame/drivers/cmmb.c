// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Centipede / Millipede / Missile Command / Let's Go Bowling
(c) 1980-2 / 2002 - Infogrames / CosmoDog

preliminary driver by Angelo Salese

Earlier revisions of this cabinet did not include the bowling game.
 Known to exist "CMM Rev 1.03" (without Let's Go Bowling)
 Let's Go Bowling is actually a completely new game by Cosmodog, not
 a port or prototype of an old Atari game.

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

Centipede / Millipede / Missile Command / Let's Go Bowling.
Team Play

1.00 PCB by CosmoDog + sticker "Multipede"

U1  = WDC 65C02S8P-14
U2  = Flash ROM AT29C020 (256KB)
U4  = ISSI IS62LV256-45J (32KB)
U5  = CY39100V208B (Cypress CPLD)
U9  = CY37128-P100 (Cypress CPLD)
U10 = CYC1399 (?)

OSC @ 72.576MHz

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65sc02.h"


class cmmb_state : public driver_device
{
public:
	cmmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 m_irq_mask;

	DECLARE_READ8_MEMBER(cmmb_charram_r);
	DECLARE_WRITE8_MEMBER(cmmb_charram_w);
	DECLARE_READ8_MEMBER(cmmb_input_r);
	DECLARE_WRITE8_MEMBER(cmmb_output_w);
	DECLARE_READ8_MEMBER(kludge_r);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_cmmb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cmmb_irq);
};


void cmmb_state::video_start()
{
}

UINT32 cmmb_state::screen_update_cmmb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0x00000;

	int y,x;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = videoram[count] & 0x3f;
			int colour = (videoram[count] & 0xc0)>>6;
			gfx->opaque(bitmap,cliprect,tile,colour,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

READ8_MEMBER(cmmb_state::cmmb_charram_r)
{
	UINT8 *GFX = memregion("gfx")->base();

	return GFX[offset];
}

WRITE8_MEMBER(cmmb_state::cmmb_charram_w)
{
	UINT8 *GFX = memregion("gfx")->base();

	GFX[offset] = data;

	offset&=0xfff;

	/* dirty char */
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 5);
}

READ8_MEMBER(cmmb_state::cmmb_input_r)
{
	//printf("%02x R\n",offset);
	switch(offset)
	{
		case 0x00: return ioport("IN2")->read();
		case 0x03: return 4; //eeprom?
		case 0x0e: return ioport("IN0")->read();
		case 0x0f: return ioport("IN1")->read();
	}

	return 0xff;
}


/*
    {
        UINT8 *ROM = space.memregion("maincpu")->base();
        UINT32 bankaddress;

        bankaddress = 0x10000 + (0x10000 * (data & 0x03));
        space.membank("bank1")->set_base(&ROM[bankaddress]);
    }
*/

WRITE8_MEMBER(cmmb_state::cmmb_output_w)
{
	//printf("%02x -> [%02x] W\n",data,offset);
	switch(offset)
	{
		case 0x01:
			{
				UINT8 *ROM = memregion("maincpu")->base();
				UINT32 bankaddress;

				bankaddress = 0x1c000 + (0x10000 * (data & 0x03));
				membank("bank1")->set_base(&ROM[bankaddress]);
			}
			break;
		case 0x03:
			m_irq_mask = data & 0x80;
			break;
		case 0x07:
			break;
	}
}

READ8_MEMBER(cmmb_state::kludge_r)
{
	return machine().rand();
}

/* overlap empty addresses */
static ADDRESS_MAP_START( cmmb_map, AS_PROGRAM, 8, cmmb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM /* zero page address */
//  AM_RANGE(0x13c0, 0x13ff) AM_RAM //spriteram
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2480, 0x249f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
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

INTERRUPT_GEN_MEMBER(cmmb_state::cmmb_irq)
{
	//if(machine().input().code_pressed_once(KEYCODE_Z))
	//if(machine().input().code_pressed(KEYCODE_Z))
//      device.execute().set_input_line(0, HOLD_LINE);
}

void cmmb_state::machine_reset()
{
}

static MACHINE_CONFIG_START( cmmb, cmmb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, XTAL_72_576MHz/5) // Unknown clock, but chip rated for 14MHz
	MCFG_CPU_PROGRAM_MAP(cmmb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cmmb_state, cmmb_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // unknown
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cmmb_state, screen_update_cmmb)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cmmb)

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRGGGBB_inverted)

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
	ROM_COPY( "maincpu",      0x18000, 0x08000, 0x08000 )

	ROM_REGION( 0x1000, "gfx", ROMREGION_ERASE00 )
ROM_END

GAME( 2002, cmmb162,  0,       cmmb,  cmmb, driver_device,  0, ROT270, "Cosmodog / Team Play (Licensed from Infogrames via Midway Games West)", "Centipede / Millipede / Missile Command / Let's Go Bowling (rev 1.62)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
