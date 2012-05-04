/***************************************************************************


***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_8MHz

class bikkuric_state : public driver_device
{
public:
	bikkuric_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_vram(*this, "vram"),
		  m_cram(*this, "cram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_cram;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(nmi_mask_w);

	UINT8 m_nmi_enable;

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void bikkuric_state::video_start()
{

}

UINT32 bikkuric_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const gfx_element *gfx = screen.machine().gfx[0];
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = m_vram[count];

			tile += ((m_cram[count] & 1) << 8);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,0/*col*/,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

WRITE8_MEMBER(bikkuric_state::nmi_mask_w)
{
	m_nmi_enable = data & 1;
}

static ADDRESS_MAP_START( bikkuric_map, AS_PROGRAM, 8, bikkuric_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_SHARE("cram")
	AM_RANGE(0xa400, 0xa7ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xa800, 0xbfff) AM_RAM // TODO
	//AM_RANGE(0xc200, 0xc200) AM_WRITENOP //watchdog?
	AM_RANGE(0xc300, 0xc300) AM_WRITE(nmi_mask_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bikkuric_io, AS_IO, 8, bikkuric_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( bikkuric )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	8*16
};


static GFXDECODE_START( bikkuric )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0, 1 )
GFXDECODE_END


void bikkuric_state::machine_start()
{
}

void bikkuric_state::machine_reset()
{
}


static PALETTE_INIT( bikkuric )
{
}

static INTERRUPT_GEN( bikkuric_irq )
{
	bikkuric_state *state = device->machine().driver_data<bikkuric_state>();
	if(state->m_nmi_enable)
		cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( bikkuric, bikkuric_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(bikkuric_map)
	MCFG_CPU_IO_MAP(bikkuric_io)
	MCFG_CPU_VBLANK_INT("screen", bikkuric_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(bikkuric_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_GFXDECODE(bikkuric)

	MCFG_PALETTE_INIT(bikkuric)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bikkuric )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "1.a16",        0x00000, 0x04000, CRC(e8d595ab) SHA1(01f6a5321274befcd03a0ec18ed9770aca4527b6) )
	ROM_LOAD( "2.a14",        0x04000, 0x02000, CRC(63fd7d53) SHA1(b1ef666453c5c9e344bee544a0673068d60158fa) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "5.l3",         0x00000, 0x02000, CRC(bc438531) SHA1(e19badc417b0538010cf535d3f733acc54b0cd96) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "3.d4",         0x00000, 0x08000, BAD_DUMP CRC(74e8a64b) SHA1(b2542e1f6f4b54d8f7aec8f673cedcf5bff5e429) ) // must be half size, 1st and 2nd identical

	ROM_REGION( 0x2000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "4.h8",         0x00000, 0x02000, CRC(d303942d) SHA1(688d43e6dbe505d44fc41fdde74858a02910080d) )
ROM_END

GAME( 198?, bikkuric,  0,   bikkuric,  bikkuric,  0,       ROT90, "<unknown>",      "Bikkuri Card (Japan)", GAME_NOT_WORKING | GAME_NO_SOUND )
