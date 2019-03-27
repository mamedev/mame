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
#include "emupal.h"
#include "screen.h"


class fp6000_state : public driver_device
{
public:
	fp6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gvram(*this, "gvram"),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void fp6000(machine_config &config);

private:
	uint8_t *m_char_rom;
	required_shared_ptr<uint16_t> m_gvram;
	required_shared_ptr<uint16_t> m_vram;
	uint8_t m_crtc_vreg[0x100],m_crtc_index;

	struct {
		uint16_t cmd;
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
	uint32_t screen_update_fp6000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device>m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void fp6000_io(address_map &map);
	void fp6000_map(address_map &map);
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


uint32_t fp6000_state::screen_update_fp6000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int xi,yi;
	uint8_t *gfx_rom = memregion("pcg")->base();
	uint32_t count;

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
	m_crtc->address_w(data);
}

WRITE8_MEMBER(fp6000_state::fp6000_6845_data_w)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

void fp6000_state::fp6000_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xbffff).ram();
	map(0xc0000, 0xdffff).ram().share("gvram");//gvram
	map(0xe0000, 0xe0fff).ram().share("vram");
	map(0xe7000, 0xe7fff).rw(FUNC(fp6000_state::fp6000_pcg_r), FUNC(fp6000_state::fp6000_pcg_w));
	map(0xf0000, 0xfffff).rom().region("ipl", 0);
}

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

void fp6000_state::fp6000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x08, 0x09).r(FUNC(fp6000_state::ex_board_r)); // BIOS of some sort ...
	map(0x0a, 0x0b).portr("DSW"); // installed RAM id?
	map(0x10, 0x11).nopr();
	map(0x20, 0x23).rw(FUNC(fp6000_state::fp6000_key_r), FUNC(fp6000_state::fp6000_key_w)).umask16(0x00ff);
	map(0x38, 0x39).r(FUNC(fp6000_state::pit_r)); // pit?
	map(0x70, 0x70).w(FUNC(fp6000_state::fp6000_6845_address_w));
	map(0x72, 0x72).w(FUNC(fp6000_state::fp6000_6845_data_w));
	map(0x74, 0x75).r(FUNC(fp6000_state::unk_r)); //bit 6 busy flag
}

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

static GFXDECODE_START( gfx_fp6000 )
	GFXDECODE_ENTRY( "pcg", 0x0000, fp6000_charlayout, 0, 1 )
GFXDECODE_END


void fp6000_state::machine_start()
{
	m_char_rom = memregion("pcg")->base();
}

void fp6000_state::machine_reset()
{
}

void fp6000_state::fp6000(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 16000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp6000_state::fp6000_map);
	m_maincpu->set_addrmap(AS_IO, &fp6000_state::fp6000_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(fp6000_state::screen_update_fp6000));
	screen.set_palette(m_palette);

	H46505(config, m_crtc, 16000000/5);    /* unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	PALETTE(config, m_palette).set_entries(8);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fp6000);
}

/* ROM definition */
ROM_START( fp6000 )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x10000, CRC(c72fe40a) SHA1(0e4c60dc27f6c7f461c4bc382b81602b3327a7a4))

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP ) //unknown MCU type

	ROM_REGION( 0x10000, "pcg", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS */
COMP( 1985, fp6000, 0,      0,      fp6000,  fp6000, fp6000_state, empty_init, "Casio", "FP-6000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
