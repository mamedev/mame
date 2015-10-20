// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hitachi B(asic Master?) 16

    very preliminary driver by Angelo Salese

    TODO:
    - Driver is all made up of educated guesses (no documentation available)

    0xfcc67 after the ROM checksum to zero (bp 0xfc153 -> SI = 0) -> system boots

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "video/mc6845.h"
#include "machine/am9517a.h"



class b16_state : public driver_device
{
public:
	b16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_dma8237(*this, "8237dma"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 *m_char_rom;
	required_shared_ptr<UINT16> m_vram;
	UINT8 m_crtc_vreg[0x100],m_crtc_index;

	DECLARE_READ16_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(b16_pcg_w);
	DECLARE_WRITE8_MEMBER(b16_6845_address_w);
	DECLARE_WRITE8_MEMBER(b16_6845_data_w);
	DECLARE_READ8_MEMBER(unk_dev_r);
	DECLARE_WRITE8_MEMBER(unk_dev_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	mc6845_device *m_mc6845;
	required_device<am9517a_device> m_dma8237;
	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

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


void b16_state::video_start()
{
	// find memory regions
	m_char_rom = memregion("pcg")->base();
}


UINT32 b16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int xi,yi;
	UINT8 *gfx_rom = memregion("pcg")->base();

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
					pen = (gfx_rom[tile*16+yi] >> (7-xi) & 1) ? color : 0;

					if(y*mc6845_tile_height < 400 && x*8+xi < 640) /* TODO: safety check */
						bitmap.pix16(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(pen);
				}
			}
		}
	}

	return 0;
}

WRITE8_MEMBER( b16_state::b16_pcg_w )
{
	m_char_rom[offset] = data;

	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
}

static ADDRESS_MAP_START( b16_map, AS_PROGRAM, 16, b16_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00000, 0x9ffff ) AM_RAM // probably not all of it.
	AM_RANGE( 0xa0000, 0xaffff ) AM_RAM // bitmap?
	AM_RANGE( 0xb0000, 0xb7fff ) AM_RAM AM_SHARE("vram") // tvram
	AM_RANGE( 0xb8000, 0xbbfff ) AM_WRITE8(b16_pcg_w,0x00ff) // pcg
	AM_RANGE( 0xfc000, 0xfffff ) AM_ROM AM_REGION("ipl",0)
ADDRESS_MAP_END

READ16_MEMBER( b16_state::vblank_r )
{
	return ioport("SYSTEM")->read();
}

WRITE8_MEMBER( b16_state::b16_6845_address_w )
{
	m_crtc_index = data;
	m_mc6845->address_w(space,offset, data);
}

WRITE8_MEMBER( b16_state::b16_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_mc6845->register_w(space, offset, data);
}

/*
Pretty weird protocol, dunno what it is ...

8a (06) W
(04) R
ff (04) W
(04) R
ff (04) W
36 (0e) W
ff (08) W
ff (08) W
06 (0e) W
(08) R
(08) R
36 (0e) W
40 (08) W
9c (08) W
76 (0e) W
ff (0a) W
ff (0a) W
46 (0e) W
(0a) R
(0a) R
76 (0e) W
1a (0a) W
00 (0a) W
b6 (0e) W
ff (0c) W
ff (0c) W
86 (0e) W
(0c) R
(0c) R
b6 (0e) W
34 (0c) W
00 (0c) W
36 (0e) W
b6 (0e) W
40 (08) W
9c (08) W
34 (0c) W
00 (0c) W
8a (06) W
06 (06) W
05 (06) W
*/

READ8_MEMBER( b16_state::unk_dev_r )
{
	static int test;

	printf("(%02x) R\n",offset << 1);

	if(offset == 0x8/2 || offset == 0x0a/2 || offset == 0x0c/2) // quick hack
	{
		test^=1;
		return test ? 0x9f : 0x92;
	}

	return 0xff;
}

WRITE8_MEMBER( b16_state::unk_dev_w )
{
	printf("%02x (%02x) W\n",data,offset << 1);

}

static ADDRESS_MAP_START( b16_io, AS_IO, 16, b16_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x0f) AM_READWRITE8(unk_dev_r,unk_dev_w,0x00ff) // DMA device?
	AM_RANGE(0x20, 0x21) AM_WRITE8(b16_6845_address_w,0x00ff)
	AM_RANGE(0x22, 0x23) AM_WRITE8(b16_6845_data_w,0x00ff)
	//0x79 bit 0 DSW?
	AM_RANGE(0x80, 0x81) AM_READ(vblank_r) // TODO
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( b16 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END




static const gfx_layout b16_charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( b16 )
	GFXDECODE_ENTRY( "pcg", 0x0000, b16_charlayout, 0, 1 )
GFXDECODE_END

void b16_state::machine_start()
{
	m_mc6845 = machine().device<mc6845_device>("crtc");
}

void b16_state::machine_reset()
{
}


READ8_MEMBER(b16_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(b16_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}


static MACHINE_CONFIG_START( b16, b16_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086, XTAL_14_31818MHz/2) //unknown xtal
	MCFG_CPU_PROGRAM_MAP(b16_map)
	MCFG_CPU_IO_MAP(b16_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(b16_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_14_31818MHz/5)    /* unknown clock, hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_DEVICE_ADD("8237dma", AM9517A, XTAL_14_31818MHz/2)
	MCFG_I8237_IN_MEMR_CB(READ8(b16_state, memory_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(b16_state, memory_write_byte))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", b16)
	MCFG_PALETTE_ADD("palette", 8)
//  MCFG_PALETTE_INIT_STANDARD(black_and_white) // TODO

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( b16 )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(7c1c93d5) SHA1(2a1e63a689c316ff836f21646166b38714a18e03) )

	ROM_REGION( 0x4000/2, "pcg", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE     INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1983, b16,  0,      0,       b16,      b16, driver_device,   0,      "Hitachi",   "B16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
