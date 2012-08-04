/****************************************************************************************

18 Holes Pro Golf (c) 1981 Data East

driver by Angelo Salese and Roberto Zandona',
based on early work by Pierpaolo Prazzoli and David Haywood

TODO:
- Map display is (almost) correct but color pens aren't;
- Flip screen support;

=========================================================================================

Pro Golf
Data East 1981
PCB version (not cassette)
--------------

All eproms 2732

g0-m through g6-m on top pcb.
g7-m through g9-m on bottom pcb.

Three Proms are 82S123 or equivalents.

gam.k11
gbm.k4
gcm.a14

Top pcb contains:
-----------------
One 6502 CPU

Two AY-3-8910

One 6116 ram.

Two 8 dip banks
------------------

Bottom pcb contains:
--------------------
Epoxy CPU module.

MC6845P CRT controller.

Two 6116 rams.

Twenty four 8116 rams.

****************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"


class progolf_state : public driver_device
{
public:
	progolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fbram(*this, "fbram"){ }

	UINT8 *m_videoram;
	UINT8 m_char_pen;
	UINT8 m_char_pen_vreg;
	UINT8 *m_fg_fb;
	required_shared_ptr<UINT8> m_fbram;
	UINT8 m_scrollx_hi;
	UINT8 m_scrollx_lo;
	UINT8 m_gfx_switch;
	UINT8 m_sound_cmd;
	DECLARE_WRITE8_MEMBER(progolf_charram_w);
	DECLARE_WRITE8_MEMBER(progolf_char_vregs_w);
	DECLARE_WRITE8_MEMBER(progolf_scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(progolf_scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(progolf_flip_screen_w);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_READ8_MEMBER(progolf_videoram_r);
	DECLARE_WRITE8_MEMBER(progolf_videoram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
};




static VIDEO_START( progolf )
{
	progolf_state *state = machine.driver_data<progolf_state>();
	state->m_scrollx_hi = 0;
	state->m_scrollx_lo = 0;

	state->m_fg_fb = auto_alloc_array(machine, UINT8, 0x2000*8);
	state->m_videoram = auto_alloc_array(machine, UINT8, 0x1000);
}


static SCREEN_UPDATE_IND16( progolf )
{
	progolf_state *state = screen.machine().driver_data<progolf_state>();
	UINT8 *videoram = state->m_videoram;
	int count,color,x,y,xi,yi;

	{
		int scroll = (state->m_scrollx_lo | ((state->m_scrollx_hi & 0x03) << 8));

		count = 0;

		for(x=0;x<128;x++)
		{
			for(y=0;y<32;y++)
			{
				int tile = videoram[count];

				drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],tile,1,0,0,(256-x*8)+scroll,y*8);
				/* wrap-around */
				drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],tile,1,0,0,(256-x*8)+scroll-1024,y*8);

				count++;
			}
		}
	}

	/* framebuffer is 8x8 chars arranged like a bitmap + a register that controls the pen handling. */
	{
		count = 0;

		for(y=0;y<256;y+=8)
		{
			for(x=0;x<256;x+=8)
			{
				for (yi=0;yi<8;yi++)
				{
					for (xi=0;xi<8;xi++)
					{
						color = state->m_fg_fb[(xi+yi*8)+count*0x40];

						if(color != 0 && cliprect.contains(x+yi, 256-y+xi))
							bitmap.pix16(x+yi, 256-y+xi) = screen.machine().pens[(color & 0x7)];
					}
				}

				count++;
			}
		}
	}

	return 0;
}

WRITE8_MEMBER(progolf_state::progolf_charram_w)
{
	int i;
	m_fbram[offset] = data;

	if(m_char_pen == 7)
	{
		for(i=0;i<8;i++)
			m_fg_fb[offset*8+i] = 0;
	}
	else
	{
		for(i=0;i<8;i++)
		{
			if(m_fg_fb[offset*8+i] == m_char_pen)
				m_fg_fb[offset*8+i] = data & (1<<(7-i)) ? m_char_pen : 0;
			else
				m_fg_fb[offset*8+i] = data & (1<<(7-i)) ? m_fg_fb[offset*8+i]|m_char_pen : m_fg_fb[offset*8+i];
		}
	}
}

WRITE8_MEMBER(progolf_state::progolf_char_vregs_w)
{
	m_char_pen = data & 0x07;
	m_gfx_switch = data & 0xf0;
	m_char_pen_vreg = data & 0x30;
}

WRITE8_MEMBER(progolf_state::progolf_scrollx_lo_w)
{
	m_scrollx_lo = data;
}

WRITE8_MEMBER(progolf_state::progolf_scrollx_hi_w)
{
	m_scrollx_hi = data;
}

WRITE8_MEMBER(progolf_state::progolf_flip_screen_w)
{
	flip_screen_set(data & 1);
	if(data & 0xfe)
		printf("$9600 with data = %02x used\n",data);
}

WRITE8_MEMBER(progolf_state::audio_command_w)
{
	m_sound_cmd = data;
	cputag_set_input_line(machine(), "audiocpu", 0, ASSERT_LINE);
}

READ8_MEMBER(progolf_state::audio_command_r)
{
	cputag_set_input_line(machine(), "audiocpu", 0, CLEAR_LINE);
	return m_sound_cmd;
}

READ8_MEMBER(progolf_state::progolf_videoram_r)
{
	UINT8 *videoram = m_videoram;
	UINT8 *gfx_rom = memregion("gfx1")->base();

	if (offset >= 0x0800)
	{
		if      (m_gfx_switch == 0x50)
			return gfx_rom[offset];
		else if (m_gfx_switch == 0x60)
			return gfx_rom[offset + 0x1000];
		else if (m_gfx_switch == 0x70)
			return gfx_rom[offset + 0x2000];
		else
			return videoram[offset];
	} else {
		if      (m_gfx_switch == 0x10)
			return gfx_rom[offset];
		else if (m_gfx_switch == 0x20)
			return gfx_rom[offset + 0x1000];
		else if (m_gfx_switch == 0x30)
			return gfx_rom[offset + 0x2000];
		else
			return videoram[offset];
	}
}

WRITE8_MEMBER(progolf_state::progolf_videoram_w)
{
	UINT8 *videoram = m_videoram;
	//if(m_gfx_switch & 0x40)
	videoram[offset] = data;
}

static ADDRESS_MAP_START( main_cpu, AS_PROGRAM, 8, progolf_state )
	AM_RANGE(0x0000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_RAM_WRITE(progolf_charram_w) AM_SHARE("fbram")
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE(progolf_videoram_r,progolf_videoram_w)
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("IN2") AM_WRITE(progolf_char_vregs_w)
	AM_RANGE(0x9200, 0x9200) AM_READ_PORT("P1") AM_WRITE(progolf_scrollx_hi_w) //p1 inputs
	AM_RANGE(0x9400, 0x9400) AM_READ_PORT("P2") AM_WRITE(progolf_scrollx_lo_w) //p2 inputs
	AM_RANGE(0x9600, 0x9600) AM_READ_PORT("IN0") AM_WRITE(progolf_flip_screen_w)   /* VBLANK */
	AM_RANGE(0x9800, 0x9800) AM_READ_PORT("DSW1")
	AM_RANGE(0x9800, 0x9800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x9801, 0x9801) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x9a00, 0x9a00) AM_READ_PORT("DSW2") AM_WRITE(audio_command_w)
//  AM_RANGE(0x9e00, 0x9e00) AM_WRITENOP
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu, AS_PROGRAM, 8, progolf_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x4000, 0x4fff) AM_DEVREADWRITE_LEGACY("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x5000, 0x5fff) AM_DEVWRITE_LEGACY("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6fff) AM_DEVREADWRITE_LEGACY("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x7000, 0x7fff) AM_DEVWRITE_LEGACY("ay2", ay8910_address_w)
	AM_RANGE(0x8000, 0x8fff) AM_READ(audio_command_r) AM_WRITENOP //volume control?
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(progolf_state::coin_inserted)
{
	cputag_set_input_line(machine(), "maincpu", INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

/* verified from M6502 code */
static INPUT_PORTS_START( progolf )
	PORT_START("IN0")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state,coin_inserted, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, progolf_state,coin_inserted, 0)    /* same coinage as COIN1 */
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0xd16e (4 * 3 bytes, LSB first) - no multiple bonus lives */
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )       /* code at 0xd188 */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Display Strength and Position" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Force Coinage = A 1C/3C - B 1C/8C" )   /* SERVICE1 = 2C/1C */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Coin Mode" )
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x40, "Mode 2" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END

static const gfx_layout progolf_charlayout =
{
	8,8,			/* 8*8 characters */
	RGN_FRAC(1,3),  /* 512 characters */
	3,				/* 3 bits per pixel */
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },  /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( progolf )
	GFXDECODE_ENTRY( "gfx1", 0x0000, progolf_charlayout, 0, 8 ) /* sprites */
GFXDECODE_END


//#ifdef UNUSED_FUNCTION
static INTERRUPT_GEN( progolf_interrupt )
{
}
//#endif

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */

};

static PALETTE_INIT( progolf )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	int i;

	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static MACHINE_CONFIG_START( progolf, progolf_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3000000/2) /* guess, 3 Mhz makes the game to behave worse? */
	MCFG_CPU_PROGRAM_MAP(main_cpu)
	MCFG_CPU_VBLANK_INT("screen", progolf_interrupt)

	MCFG_CPU_ADD("audiocpu", M6502, 500000)
	MCFG_CPU_PROGRAM_MAP(sound_cpu)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3072))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(progolf)

	MCFG_GFXDECODE(progolf)
	MCFG_PALETTE_LENGTH(32*3)
	MCFG_PALETTE_INIT(progolf)

	MCFG_MC6845_ADD("crtc", MC6845, 3000000/4, mc6845_intf)	/* hand tuned to get ~57 fps */
	MCFG_VIDEO_START(progolf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 12000000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)

	MCFG_SOUND_ADD("ay2", AY8910, 12000000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_CONFIG_END


ROM_START( progolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g4-m.2a",      0xb000, 0x1000, CRC(8f06ebc0) SHA1(c012dcaf06cbd9e49f3ae819d9cbed4df8751cec) )
	ROM_LOAD( "g3-m.4a",      0xc000, 0x1000, CRC(8101b231) SHA1(d933992c93b3cd9a052ac40ec1fa92a181b28691) )
	ROM_LOAD( "g2-m.6a",      0xd000, 0x1000, CRC(a4a0d8dc) SHA1(04db60d5cfca4834ac2cc7661f772704489cb329) )
	ROM_LOAD( "g1-m.8a",      0xe000, 0x1000, CRC(749032eb) SHA1(daa356b2c70bcd8cdd0c4df4268b6158bc8aae8e) )
	ROM_LOAD( "g0-m.9a",      0xf000, 0x1000, CRC(8f8b1e8e) SHA1(fc877a8f2b26ea48c5ba2324678d6077f3432a79) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g6-m.1b",      0xf000, 0x1000, CRC(0c6fadf5) SHA1(9af2c2152b339cadab7aff0b0164d4431d2558bd) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g7-m.7a",      0x0000, 0x1000, CRC(16b42975) SHA1(29268a8a660781ff0de77b3b1bfc16edff7be134) )
	ROM_LOAD( "g8-m.9a",      0x1000, 0x1000, CRC(cf3f35da) SHA1(06acc29a5e282b5a9960eabebdb1a529910286b6) )
	ROM_LOAD( "g9-m.10a",     0x2000, 0x1000, CRC(7712e248) SHA1(4e7dd12d323cf8378adb1e32a763a1799e2b4bdc) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "gcm.a14",      0x0000, 0x0020, CRC(8259e7db) SHA1(f98db5ebf8182eb0359fa372fa664cb6d3b09437) )
	ROM_LOAD( "gbm.k4",       0x0020, 0x0020, CRC(1ea3319f) SHA1(809af38e73fa1f30410e7d6b4504fe360ee9b091) )
	ROM_LOAD( "gam.k11",      0x0040, 0x0020, CRC(b9665de3) SHA1(4c5aba5f6589f4bce4692c0d5bb2811ab8e14aed) )
ROM_END

ROM_START( progolfa )
	ROM_REGION( 0x10000, "maincpu", 0 ) // custom DECO CPU-6 module
	ROM_LOAD( "g4-m.a3",      0xb000, 0x1000, CRC(015a08d9) SHA1(671d5cd708e098dbda3e495a8b4ce3393c6971da) )
	ROM_LOAD( "g3-m.a4",      0xc000, 0x1000, CRC(c1339da5) SHA1(e9728dcc5f67fbe79eea818ba48421c46d9e63e9) )
	ROM_LOAD( "g2-m.a6",      0xd000, 0x1000, CRC(fafec36e) SHA1(70880d6f9b11505d466f36c12a43361ee2639fed) )
	ROM_LOAD( "g1-m.a8",      0xe000, 0x1000, CRC(749032eb) SHA1(daa356b2c70bcd8cdd0c4df4268b6158bc8aae8e) )
	ROM_LOAD( "g0-m.a9",      0xf000, 0x1000, CRC(a03c533f) SHA1(2e0006be40e32b64b1490bd339d9fc9302eee7c4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "g5-m.b1",      0xf000, 0x1000, CRC(0c6fadf5) SHA1(9af2c2152b339cadab7aff0b0164d4431d2558bd) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g7-m.a8",      0x0000, 0x1000, CRC(16b42975) SHA1(29268a8a660781ff0de77b3b1bfc16edff7be134) )
	ROM_LOAD( "g8-m.a9",      0x1000, 0x1000, CRC(cf3f35da) SHA1(06acc29a5e282b5a9960eabebdb1a529910286b6) )
	ROM_LOAD( "g9-m.a10",     0x2000, 0x1000, CRC(7712e248) SHA1(4e7dd12d323cf8378adb1e32a763a1799e2b4bdc) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "gcm.a14",      0x0000, 0x0020, CRC(8259e7db) SHA1(f98db5ebf8182eb0359fa372fa664cb6d3b09437) )
	ROM_LOAD( "gbm.k4",       0x0020, 0x0020, CRC(1ea3319f) SHA1(809af38e73fa1f30410e7d6b4504fe360ee9b091) )
	ROM_LOAD( "gam.k11",      0x0040, 0x0020, CRC(b9665de3) SHA1(4c5aba5f6589f4bce4692c0d5bb2811ab8e14aed) )
ROM_END


static DRIVER_INIT( progolf )
{
	int A;
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *rom = machine.root_device().memregion("maincpu")->base();
	UINT8* decrypted = auto_alloc_array(machine, UINT8, 0x10000);

	space->set_decrypted_region(0x0000,0xffff, decrypted);

	/* Swap bits 5 & 6 for opcodes */
	for (A = 0xb000 ; A < 0x10000 ; A++)
		decrypted[A] = BITSWAP8(rom[A],7,5,6,4,3,2,1,0);
}

static DRIVER_INIT( progolfa )
{
	int A;
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *rom = machine.root_device().memregion("maincpu")->base();
	UINT8* decrypted = auto_alloc_array(machine, UINT8, 0x10000);

	space->set_decrypted_region(0x0000,0xffff, decrypted);

	/* data is likely to not be encrypted, just the opcodes are. */
	for (A = 0x0000 ; A < 0x10000 ; A++)
	{
		if (A & 1)
			decrypted[A] = BITSWAP8(rom[A],6,4,7,5,3,2,1,0);
		else
			decrypted[A] = rom[A];
	}
}

/* Maybe progolf is a bootleg? progolfa uses DECO CPU-6 as custom module CPU (the same as Zoar) */
GAME( 1981, progolf,  0,       progolf, progolf, progolf_state, progolf,  ROT270, "Data East Corporation", "18 Holes Pro Golf (set 1)", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
GAME( 1981, progolfa, progolf, progolf, progolf, progolf_state, progolfa, ROT270, "Data East Corporation", "18 Holes Pro Golf (set 2)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
