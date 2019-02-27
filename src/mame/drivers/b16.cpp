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
#include "machine/am9517a.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


class b16_state : public driver_device
{
public:
	b16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_mc6845(*this, "crtc"),
		m_dma8237(*this, "8237dma"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_char_rom(*this, "pcg")
	{ }

	void b16(machine_config &config);

protected:
	virtual void video_start() override;

private:
	uint8_t m_crtc_vreg[0x100], m_crtc_index;

	required_shared_ptr<uint16_t> m_vram;
	required_device<mc6845_device> m_mc6845;
	required_device<am9517a_device> m_dma8237;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_char_rom;

	DECLARE_READ16_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(b16_pcg_w);
	DECLARE_WRITE8_MEMBER(b16_6845_address_w);
	DECLARE_WRITE8_MEMBER(b16_6845_data_w);
	DECLARE_READ8_MEMBER(unk_dev_r);
	DECLARE_WRITE8_MEMBER(unk_dev_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void b16_io(address_map &map);
	void b16_map(address_map &map);
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
	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_crtc_index));
}


uint32_t b16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y=0;y<mc6845_v_display;y++)
	{
		for(int x=0;x<mc6845_h_display;x++)
		{
			int tile = m_vram[x+y*mc6845_h_display] & 0xff;
			int color = (m_vram[x+y*mc6845_h_display] & 0x700) >> 8;
			int pen;

			for(int yi=0;yi<mc6845_tile_height;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					pen = (m_char_rom[tile*16+yi] >> (7-xi) & 1) ? color : 0;

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

void b16_state::b16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x9ffff).ram(); // probably not all of it.
	map(0xa0000, 0xaffff).ram(); // bitmap?
	map(0xb0000, 0xb7fff).ram().share("vram"); // tvram
	map(0xb8000, 0xbbfff).w(FUNC(b16_state::b16_pcg_w)).umask16(0x00ff); // pcg
	map(0xfc000, 0xfffff).rom().region("ipl", 0);
}

READ16_MEMBER( b16_state::vblank_r )
{
	return ioport("SYSTEM")->read();
}

WRITE8_MEMBER( b16_state::b16_6845_address_w )
{
	m_crtc_index = data;
	m_mc6845->address_w(data);
}

WRITE8_MEMBER( b16_state::b16_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_mc6845->register_w(data);
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

void b16_state::b16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x0f).rw(FUNC(b16_state::unk_dev_r), FUNC(b16_state::unk_dev_w)).umask16(0x00ff); // DMA device?
	map(0x20, 0x20).w(FUNC(b16_state::b16_6845_address_w));
	map(0x22, 0x22).w(FUNC(b16_state::b16_6845_data_w));
	//0x79 bit 0 DSW?
	map(0x80, 0x81).r(FUNC(b16_state::vblank_r)); // TODO
}


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

static GFXDECODE_START( gfx_b16 )
	GFXDECODE_ENTRY( "pcg", 0x0000, b16_charlayout, 0, 1 )
GFXDECODE_END

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


MACHINE_CONFIG_START(b16_state::b16)
	/* basic machine hardware */
	MCFG_DEVICE_ADD(m_maincpu, I8086, XTAL(14'318'181)/2) //unknown xtal
	MCFG_DEVICE_PROGRAM_MAP(b16_map)
	MCFG_DEVICE_IO_MAP(b16_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(b16_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_PALETTE("palette")

	H46505(config, m_mc6845, XTAL(14'318'181)/5);    /* unknown clock, hand tuned to get ~60 fps */
	m_mc6845->set_screen("screen");
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);

	AM9517A(config, m_dma8237, XTAL(14'318'181)/2);
	m_dma8237->in_memr_callback().set(FUNC(b16_state::memory_read_byte));
	m_dma8237->out_memw_callback().set(FUNC(b16_state::memory_write_byte));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_b16);
	MCFG_PALETTE_ADD(m_palette, 8)
//  MCFG_PALETTE_INIT_STANDARD(black_and_white) // TODO

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( b16 )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(7c1c93d5) SHA1(2a1e63a689c316ff836f21646166b38714a18e03) )

	ROM_REGION( 0x4000/2, "pcg", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY    FULLNAME  FLAGS */
COMP( 1983, b16,  0,      0,      b16,     b16,   b16_state, empty_init, "Hitachi", "B16",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
