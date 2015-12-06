// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Casio FP-6000

    preliminary driver by Angelo Salese

    TODO:
    - keyboard;
    - fdc / cmt;
    - gvram color pen is a rather crude guess (the layer is monochrome on
      BASIC?);
    - everything else

    Debug trick for the keyboard:
    - bp 0xfc93e, ip+=2 then define al = ASCII code

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "video/mc6845.h"


class fp6000_state : public driver_device
{
public:
	fp6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gvram(*this, "gvram"),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	UINT8 *m_char_rom;
	required_shared_ptr<UINT16> m_gvram;
	required_shared_ptr<UINT16> m_vram;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;

	struct {
		UINT16 cmd;
	}m_key;
	DECLARE_READ8_MEMBER(fp6000_pcg_r);
	DECLARE_WRITE8_MEMBER(fp6000_pcg_w);
	DECLARE_WRITE8_MEMBER(fp6000_6845_address_w);
	DECLARE_WRITE8_MEMBER(fp6000_6845_data_w);
	DECLARE_READ8_MEMBER(fp6000_key_r);
	DECLARE_WRITE8_MEMBER(fp6000_key_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_READ16_MEMBER(ex_board_r);
	DECLARE_READ16_MEMBER(pit_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_fp6000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device>m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

void fp6000_state::video_start()
{
}

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


UINT32 fp6000_state::screen_update_fp6000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int xi,yi;
	UINT8 *gfx_rom = memregion("pcg")->base();
	UINT32 count;

	count = 0;

	for(y=0;y<400;y++)
	{
		for(x=0;x<640/4;x++)
		{
			for(xi=0;xi<4;xi++)
			{
				int dot = (m_gvram[count] >> (12-xi*4)) & 0xf;

				if(y < 400 && x*4+xi < 640) /* TODO: safety check */
					bitmap.pix16(y, x*4+xi) = m_palette->pen(dot);
			}

			count++;
		}
	}

	for(y=0;y<mc6845_v_display;y++)
	{
		for(x=0;x<mc6845_h_display;x++)
		{
			int tile = m_vram[x+y*mc6845_h_display] & 0xff;
			int color = (m_vram[x+y*mc6845_h_display] & 0x700) >> 8;
			int pen;

			for(yi=0;yi<mc6845_tile_height;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					pen = (gfx_rom[tile*16+yi] >> (7-xi) & 1) ? color : -1;

					if(pen != -1)
						if(y*mc6845_tile_height < 400 && x*8+xi < 640) /* TODO: safety check */
							bitmap.pix16(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(pen);
				}
			}
		}
	}

	/* quick and dirty way to do the cursor */
	for(yi=0;yi<mc6845_tile_height;yi++)
	{
		for(xi=0;xi<8;xi++)
		{
			if(mc6845_h_display)
			{
				x = mc6845_cursor_addr % mc6845_h_display;
				y = mc6845_cursor_addr / mc6845_h_display;
				bitmap.pix16(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(7);
			}
		}
	}

	return 0;
}

READ8_MEMBER(fp6000_state::fp6000_pcg_r)
{
	return m_char_rom[offset];
}

WRITE8_MEMBER(fp6000_state::fp6000_pcg_w)
{
	m_char_rom[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
}

WRITE8_MEMBER(fp6000_state::fp6000_6845_address_w)
{
	m_crtc_index = data;
	m_crtc->address_w(space, offset, data);
}

WRITE8_MEMBER(fp6000_state::fp6000_6845_data_w)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(space, offset, data);
}

static ADDRESS_MAP_START(fp6000_map, AS_PROGRAM, 16, fp6000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xbffff) AM_RAM
	AM_RANGE(0xc0000,0xdffff) AM_RAM AM_SHARE("gvram")//gvram
	AM_RANGE(0xe0000,0xe0fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xe7000,0xe7fff) AM_READWRITE8(fp6000_pcg_r,fp6000_pcg_w,0xffff)
	AM_RANGE(0xf0000,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

/* Hack until I understand what UART is this one ... */
READ8_MEMBER(fp6000_state::fp6000_key_r)
{
	if(offset)
	{
		switch(m_key.cmd)
		{
			case 0x7e15: return 3;
			case 0x1b15: return 1;
			case 0x2415: return 0;
			default: printf("%04x\n",m_key.cmd);
		}
		return 0;
	}

	return 0x40;
}

WRITE8_MEMBER(fp6000_state::fp6000_key_w)
{
	if(offset)
		m_key.cmd = (data & 0xff) | (m_key.cmd << 8);
	else
		m_key.cmd = (data << 8) | (m_key.cmd & 0xff);
}

READ16_MEMBER(fp6000_state::unk_r)
{
	return 0x40;
}

READ16_MEMBER(fp6000_state::ex_board_r)
{
	return 0xffff;
}

READ16_MEMBER(fp6000_state::pit_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START(fp6000_io, AS_IO, 16, fp6000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x08, 0x09) AM_READ(ex_board_r) // BIOS of some sort ...
	AM_RANGE(0x0a, 0x0b) AM_READ_PORT("DSW") // installed RAM id?
	AM_RANGE(0x10, 0x11) AM_READNOP
	AM_RANGE(0x20, 0x23) AM_READWRITE8(fp6000_key_r,fp6000_key_w,0x00ff)
	AM_RANGE(0x38, 0x39) AM_READ(pit_r) // pit?
	AM_RANGE(0x70, 0x71) AM_WRITE8(fp6000_6845_address_w,0x00ff)
	AM_RANGE(0x72, 0x73) AM_WRITE8(fp6000_6845_data_w,0x00ff)
	AM_RANGE(0x74, 0x75) AM_READ(unk_r) //bit 6 busy flag
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( fp6000 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x40, "Installed RAM banks" )
	PORT_DIPSETTING(    0xe0, "0" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0xa0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x60, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "6 (INVALID)" ) //exceeds 768KB limit (writes to gvram et al)
	PORT_DIPSETTING(    0x00, "7 (INVALID)" )
INPUT_PORTS_END

static const gfx_layout fp6000_charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( fp6000 )
	GFXDECODE_ENTRY( "pcg", 0x0000, fp6000_charlayout, 0, 1 )
GFXDECODE_END


void fp6000_state::machine_start()
{
	m_char_rom = memregion("pcg")->base();
}

void fp6000_state::machine_reset()
{
}

static MACHINE_CONFIG_START( fp6000, fp6000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 16000000/2)
	MCFG_CPU_PROGRAM_MAP(fp6000_map)
	MCFG_CPU_IO_MAP(fp6000_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(fp6000_state, screen_update_fp6000)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", H46505, "screen", 16000000/5)    /* unknown clock, hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_PALETTE_ADD("palette", 8)
//  MCFG_PALETTE_INIT(black_and_white)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fp6000)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fp6000 )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x10000, CRC(c72fe40a) SHA1(0e4c60dc27f6c7f461c4bc382b81602b3327a7a4))

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP ) //unknown MCU type

	ROM_REGION( 0x10000, "pcg", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1985, fp6000,  0,      0,       fp6000,     fp6000, driver_device,    0,     "Casio",   "FP-6000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
