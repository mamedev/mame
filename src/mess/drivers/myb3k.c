/***************************************************************************

    Matsushita / Panasonic My Brain 3000 / JB-3000

    preliminary driver by Angelo Salese

    TODO:
    - needs a working floppy image, fdc probably needs IRQ / DRQ lines
      hooked up.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "video/mc6845.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"

class myb3k_state : public driver_device
{
public:
	myb3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_fdc(*this, "fdc"),
	m_crtc(*this, "crtc")
	,
		m_p_vram(*this, "p_vram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_fdc;
	required_device<mc6845_device> m_crtc;
	DECLARE_WRITE8_MEMBER(myb3k_6845_address_w);
	DECLARE_WRITE8_MEMBER(myb3k_6845_data_w);
	DECLARE_WRITE8_MEMBER(myb3k_video_mode_w);
	DECLARE_WRITE8_MEMBER(myb3k_fdc_output_w);
	required_shared_ptr<UINT8> m_p_vram;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;
	UINT8 m_vmode;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};

void myb3k_state::video_start()
{
}

#define mc6845_h_char_total 	(state->m_crtc_vreg[0])
#define mc6845_h_display		(state->m_crtc_vreg[1])
#define mc6845_h_sync_pos		(state->m_crtc_vreg[2])
#define mc6845_sync_width		(state->m_crtc_vreg[3])
#define mc6845_v_char_total		(state->m_crtc_vreg[4])
#define mc6845_v_total_adj		(state->m_crtc_vreg[5])
#define mc6845_v_display		(state->m_crtc_vreg[6])
#define mc6845_v_sync_pos		(state->m_crtc_vreg[7])
#define mc6845_mode_ctrl		(state->m_crtc_vreg[8])
#define mc6845_tile_height		(state->m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start	(state->m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end 	(state->m_crtc_vreg[0x0b])
#define mc6845_start_addr		(((state->m_crtc_vreg[0x0c]<<8) & 0x3f00) | (state->m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr  	(((state->m_crtc_vreg[0x0e]<<8) & 0x3f00) | (state->m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr	(((state->m_crtc_vreg[0x10]<<8) & 0x3f00) | (state->m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr  	(((state->m_crtc_vreg[0x12]<<8) & 0x3f00) | (state->m_crtc_vreg[0x13] & 0xff))


static SCREEN_UPDATE_IND16( myb3k )
{
	myb3k_state *state = screen.machine().driver_data<myb3k_state>();
	int x,y;
	int xi,yi;
	int dot;
	int h_step;

	h_step = 64 >> (state->m_vmode & 3);

	//popmessage("%02x %d",state->m_vmode,h_step);

	for(y=0;y<mc6845_v_display;y++)
	{
		for(x=0;x<mc6845_h_display;x++)
		{
			/* 8x8 grid gfxs, weird format too ... */
			for(yi=0;yi<mc6845_tile_height;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					dot = (state->m_p_vram[(x+y*mc6845_h_display)*h_step+yi+0x8000] >> (7-xi)) & 1;
					if((yi & ~7 && (!(state->m_vmode & 4))) || (yi & ~0xf && (state->m_vmode & 4)))
						dot = 0;

					if(y*mc6845_tile_height+yi < 200 && x*8+xi < 320) /* TODO: safety check */
						bitmap.pix16(y*mc6845_tile_height+yi, x*8+xi) = screen.machine().pens[dot];
				}
			}
		}
	}

	return 0;
}

WRITE8_MEMBER( myb3k_state::myb3k_6845_address_w )
{
	m_crtc_index = data;
	m_crtc->address_w(space, offset, data);
}

WRITE8_MEMBER( myb3k_state::myb3k_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(space, offset, data);
}

WRITE8_MEMBER( myb3k_state::myb3k_video_mode_w )
{
	/* ---- -x-- interlace mode */
	/* ---- --xx horizontal step count (number of offsets of vram RAM data to skip, 64 >> n) */

	m_vmode = data;
}

WRITE8_MEMBER( myb3k_state::myb3k_fdc_output_w )
{
	/* TODO: complete guesswork! (it just does a 0x24 -> 0x20 in there) */
	wd17xx_set_drive(m_fdc, data & 3);
	floppy_mon_w(floppy_get_device(machine(), data & 3), !(data & 4) ? 1: 0);
	floppy_drive_set_ready_state(floppy_get_device(machine(), data & 3), data & 0x4,0);
	//wd17xx_set_side(m_fdc, (data & 0x10)>>4);
}

static ADDRESS_MAP_START(myb3k_map, AS_PROGRAM, 8, myb3k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x7ffff) AM_RAM
	AM_RANGE(0x80000,0x8ffff) AM_NOP
	AM_RANGE(0xd0000,0xdffff) AM_RAM AM_SHARE("p_vram")
//  AM_RANGE(0xe0000,0xexxxx) option ROM board
	AM_RANGE(0xfc000,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(myb3k_io, AS_IO, 8, myb3k_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x01, 0x01) AM_READ_PORT("DSW1")
	AM_RANGE(0x03, 0x03) AM_WRITENOP
	AM_RANGE(0x04, 0x04) AM_WRITE(myb3k_video_mode_w)
	AM_RANGE(0x06, 0x06) AM_READ_PORT("DSW2")
	AM_RANGE(0x1c, 0x1c) AM_WRITE(myb3k_6845_address_w)
	AM_RANGE(0x1d, 0x1d) AM_WRITE(myb3k_6845_data_w)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE_LEGACY("fdc",wd17xx_r,wd17xx_w) //FDC, almost likely wd17xx
	AM_RANGE(0x24, 0x24) AM_WRITE(myb3k_fdc_output_w)
//  AM_RANGE(0x520,0x524) mirror of above
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( myb3k )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) // - these two plays with the video modes
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) // /
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
	PORT_DIPNAME( 0x80, 0x00, "FDC mapping" )
	PORT_DIPSETTING(    0x80, "0x520-0x524 range" )
	PORT_DIPSETTING(    0x00, "0x20-0x24 range" )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "ROM information" )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void myb3k_state::machine_start()
{
}

void myb3k_state::machine_reset()
{
}


static const gfx_layout myb3k_charlayout =
{
	8, 8,
	0x400,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( myb3k )
	GFXDECODE_ENTRY( "ipl", 0x0000, myb3k_charlayout, 0, 1 )
GFXDECODE_END

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


static const wd17xx_interface myb3k_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};


static const floppy_interface myb3k_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD_40, //todo
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( myb3k, myb3k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 4000000) /* unknown clock*/
	MCFG_CPU_PROGRAM_MAP(myb3k_map)
	MCFG_CPU_IO_MAP(myb3k_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_STATIC(myb3k)
	MCFG_GFXDECODE(myb3k)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, XTAL_3_579545MHz/4, mc6845_intf)	/* unknown clock, hand tuned to get ~60 fps */
	MCFG_MB8877_ADD("fdc", myb3k_wd17xx_interface ) //unknown type
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(myb3k_floppy_interface)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( myb3k )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(64a864a1) SHA1(d3ccfd28f2938e71a26ae5a0085439a3265f60bf))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1982, myb3k,  0,      0,       myb3k,     myb3k, driver_device,    0,     "Panasonic",   "MyBrain 3000", GAME_NOT_WORKING | GAME_NO_SOUND)
