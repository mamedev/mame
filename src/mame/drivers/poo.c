/*******************************************************************************************

Uncle Poo (c) 1983 Diatec

actually an hack of Joinem? Or there's something even more similar?

driver by David Haywood and Angelo Salese

TODO:
-merge with jack.c driver, this is a modification of that HW (with colscroll)
-accurate game speed (controlled by an irq)
-a bunch of unmapped read / writes
-writes to ROM regions

*******************************************************************************************/

/* Stephh's Notes:

'unclepoo'

SYSTEM bit 7 is sort of "freeze", but it doesn't seem to have any effect when playing
(only during boot up sequence - unsure about attract mode)

DSW1 bit 5 is "Bonus Lives" :
  - when Off (0x00), you get an extra life EVERY 30000 points
  - When On  (0x20), you get an extra life at 30000 points ONLY

DSW1 bits 6 and 7 might be used for difficulty (to be confirmed)

DSW2 bit 0 is the "Cabinet" Dip Switch :
  - when Off (0x00), cabinet is cocktail
  - When On  (0x01), cabinet is upright
This affects write to 0xb700 (bit 7) and reads from 0xb506 and 0xb507 ...
BTW, remove PORT_PLAYER(1) and change PORT_PLAYER(2) to PORT_COCKTAIL ...

DSW2 bit 7 overwrites the number of lives :
  - When Off (0x00), lives are based on DSW1 bit 4
  - When On  (0x80), lives are set to 255 (0xff) but they are NOT infinite

Other bits from DSW2 (but bit 5) don't seem to be read / tested at all ...
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


class poo_state : public driver_device
{
public:
	poo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sprites(*this, "sprites"),
		m_scrolly(*this, "scrolly"),
		m_vram(*this, "vram"){ }

	required_shared_ptr<UINT8> m_sprites;
	required_shared_ptr<UINT8> m_scrolly;
	required_shared_ptr<UINT8> m_vram;
	UINT8 m_vram_colbank;
	DECLARE_READ8_MEMBER(unk_inp_r);
	DECLARE_READ8_MEMBER(unk_inp2_r);
	DECLARE_READ8_MEMBER(unk_inp3_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(poo_vregs_w);
	DECLARE_READ8_MEMBER(timer_r);
};


static VIDEO_START(unclepoo)
{
}

static SCREEN_UPDATE_IND16(unclepoo)
{
	poo_state *state = screen.machine().driver_data<poo_state>();
	int y,x;
	int count;
	const gfx_element *gfx = screen.machine().gfx[0];

	count = 0;

	for (x=0;x<32;x++)
	{
		for (y=0;y<32;y++)
		{
			int tile = state->m_vram[count+0x000] | ((state->m_vram[count+0x400] & 3) <<8);
			int color = (state->m_vram[count+0x400] & 0x38) >> 3;
			int scrolly = (state->m_scrolly[x*4]);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,color+state->m_vram_colbank,0,0,x*8,256-(y*8)+scrolly);
			drawgfx_opaque(bitmap,cliprect,gfx,tile,color+state->m_vram_colbank,0,0,x*8,0-(y*8)+scrolly);

			count++;
		}
	}

	{
		int spr_offs,x,y,col,fx,fy,i;

		for(i=0;i<0x80;i+=4)
		{
			spr_offs = state->m_sprites[i+2] | (state->m_sprites[i+3] & 3) << 8;
			y = state->m_sprites[i+0]+8;
			x = state->m_sprites[i+1];
			col = (state->m_sprites[i+3] & 0xf8) >> 3;
			fx = 0;
			fy = 0;

			drawgfx_transpen(bitmap,cliprect,gfx,spr_offs,col,fx,fy,x,y,0);
		}
	}

	return 0;
}

READ8_MEMBER(poo_state::unk_inp_r)
{
	return 0x00;//machine().rand();
}

#if 0

READ8_MEMBER(poo_state::unk_inp2_r)
{
	return 0xff;
}

READ8_MEMBER(poo_state::unk_inp3_r)
{
	return machine().rand();
}
#endif


#if 0
WRITE8_MEMBER(poo_state::unk_w)
{
	printf("%02x %02x\n",data,offset);
}
#endif

/* soundlatch write */
WRITE8_MEMBER(poo_state::sound_cmd_w)
{
	soundlatch_byte_w(space, 0, (data & 0xff));
	cputag_set_input_line(machine(), "subcpu", 0, HOLD_LINE);
}

WRITE8_MEMBER(poo_state::poo_vregs_w)
{
	// bit 2 used, unknown purpose
	m_vram_colbank = data & 0x18;
}

static ADDRESS_MAP_START( unclepoo_main_map, AS_PROGRAM, 8, poo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x97ff) AM_RAM
	AM_RANGE(0x9800, 0x9801) AM_READ(unk_inp_r) //AM_WRITE(unk_w )

	AM_RANGE(0xb000, 0xb07f) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0xb080, 0xb0ff) AM_RAM AM_SHARE("scrolly")

	AM_RANGE(0xb400, 0xb400) AM_WRITE(sound_cmd_w)

	AM_RANGE(0xb500, 0xb500) AM_READ_PORT("DSW1")
	AM_RANGE(0xb501, 0xb501) AM_READ_PORT("DSW2")
	AM_RANGE(0xb502, 0xb502) AM_READ_PORT("P1")
	AM_RANGE(0xb503, 0xb503) AM_READ_PORT("P2")
	AM_RANGE(0xb504, 0xb504) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xb700, 0xb700) AM_WRITE(poo_vregs_w)

	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_SHARE("vram")

ADDRESS_MAP_END

static ADDRESS_MAP_START( unclepoo_main_portmap, AS_IO, 8, poo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END



static ADDRESS_MAP_START( unclepoo_sub_map, AS_PROGRAM, 8, poo_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_WRITENOP  /* R/C filter ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( unclepoo_sub_portmap, AS_IO, 8, poo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE_LEGACY("ay", ay8910_r, ay8910_data_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE_LEGACY("ay", ay8910_address_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( unclepoo )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "30000 Only" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Test Mode" )			PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Start with 255 Lives (Cheat)" )	PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( unclepoo )
	GFXDECODE_ENTRY( "gfx", 0, tiles8x8_layout, 0, 0x20 )
GFXDECODE_END

static PALETTE_INIT( unclepoo )
{
	const UINT8 *color_prom = machine.region("proms")->base();
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 0x100; i++)
	{
		val = (color_prom[i+0x100]) | (color_prom[i+0x000]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

READ8_MEMBER(poo_state::timer_r)
{
	return downcast<cpu_device *>(&space.device())->total_cycles() / 16;
}


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(driver_device, soundlatch_byte_r),
	DEVCB_DRIVER_MEMBER(poo_state, timer_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( unclepoo, poo_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,18000000/6)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(unclepoo_main_map)
	MCFG_CPU_IO_MAP(unclepoo_main_portmap)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold,256) // ??? controls game speed

	MCFG_CPU_ADD("subcpu", Z80,18000000/12)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(unclepoo_sub_map)
	MCFG_CPU_IO_MAP(unclepoo_sub_portmap)
//  MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(unclepoo)

	MCFG_GFXDECODE(unclepoo)
	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT(unclepoo)

	MCFG_VIDEO_START(unclepoo)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay", AY8910, 18000000/12) /* ? Mhz */
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



ROM_START( unclepoo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "01.f17", 0x00000, 0x2000, CRC(92fb238c) SHA1(e9476c5c1a0bf9e8c6c364ac022ed1d97ae66d2e) )
	ROM_LOAD( "02.f14", 0x02000, 0x2000, CRC(b99214ef) SHA1(c8e4af0efbc5ea543277b2764dc6f119aae477ca) )
	ROM_LOAD( "03.f11", 0x04000, 0x2000, CRC(a136af97) SHA1(cfa610bf357870053617fed8aef6bb30bd996422) )
	ROM_LOAD( "04.f09", 0x06000, 0x2000, CRC(c4bcd414) SHA1(df3125358530f5fb8d202bddcb0ef5e322fabb7b) )

	ROM_REGION( 0x1000, "subcpu", 0 )
	ROM_LOAD( "08.c15", 0x00000, 0x1000, CRC(fd84106b) SHA1(891853d2b39850a981016108b74ca20337d2cdd8) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "05.k04", 0x0000, 0x2000, CRC(64026934) SHA1(a5342335d02d34fa6ba2b29484ed71ecc96292f2) )
	ROM_LOAD( "06.j04", 0x2000, 0x2000, CRC(94b5f676) SHA1(32c27854726636c4ce03bb6a83b32d04ed6c42af) )
	ROM_LOAD( "07.h04", 0x4000, 0x2000, CRC(e2f73e99) SHA1(61cb09ff424ba63b892b4822e7ed916af73412f1) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "diatec_h.bin", 0x000, 0x100, CRC(938601b1) SHA1(8213284989bebb5f7375878181840de8079dc1f3) )
	ROM_LOAD( "diatec_l.bin", 0x100, 0x100, CRC(b04d466a) SHA1(1438abeae76ef807ba34bd6d3e4c44f707dbde6e) )
ROM_END

GAME( 1983, unclepoo, 0, unclepoo, unclepoo, 0, ROT90, "Diatec",         "Uncle Poo", GAME_NO_COCKTAIL )
