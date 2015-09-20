// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia

/* IGS017 / IGS031 video device */

/*

what's the difference between IGS017 and IGS031? encryption?

all the known IGS017 / IGS031 games use the following memory map, is the IGS017 / IGS031 providing the interface to the 8255, or is it coincidence?

	AM_RANGE( 0x1000, 0x17ff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x1800, 0x1bff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x1c00, 0x1fff ) AM_RAM
	AM_RANGE( 0x2010, 0x2013 ) AM_DEVREAD("ppi8255", i8255_device, read)
	AM_RANGE( 0x2012, 0x2012 ) AM_WRITE(video_disable_w )
	AM_RANGE( 0x2014, 0x2014 ) AM_WRITE(nmi_enable_w )
	AM_RANGE( 0x2015, 0x2015 ) AM_WRITE(irq_enable_w )
	AM_RANGE( 0x4000, 0x5fff ) AM_RAM_WRITE(fg_w ) AM_SHARE("fg_videoram")
	AM_RANGE( 0x6000, 0x7fff ) AM_RAM_WRITE(bg_w ) AM_SHARE("bg_videoram")

*/



#include "emu.h"
#include "igs017_igs031.h"



DEVICE_ADDRESS_MAP_START( map, 8, igs017_igs031_device )
	AM_RANGE( 0x1000, 0x17ff ) AM_RAM AM_SHARE("spriteram")
//	AM_RANGE( 0x1800, 0x1bff ) AM_RAM //_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x1800, 0x1bff ) AM_RAM_WRITE(palram_w) AM_SHARE("palram")
	AM_RANGE( 0x1c00, 0x1fff ) AM_RAM

	AM_RANGE( 0x2010, 0x2013 ) AM_READ(i8255_r)
	AM_RANGE( 0x2012, 0x2012 ) AM_WRITE(video_disable_w )

	AM_RANGE( 0x2014, 0x2014 ) AM_WRITE(nmi_enable_w )
	AM_RANGE( 0x2015, 0x2015 ) AM_WRITE(irq_enable_w )

	AM_RANGE( 0x4000, 0x5fff ) AM_RAM_WRITE(fg_w ) AM_SHARE("fg_videoram")
	AM_RANGE( 0x6000, 0x7fff ) AM_RAM_WRITE(bg_w ) AM_SHARE("bg_videoram")

ADDRESS_MAP_END

READ8_MEMBER(igs017_igs031_device::i8255_r)
{
	if (m_i8255)
		return m_i8255->read(space, offset);

	logerror("igs017_igs031_device::i8255_r with no 8255 device %02x\n", offset);

	return 0;
}


static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8*3,8*2,8*1,8*0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*4) },
	8*8*4
};

GFXDECODE_MEMBER( igs017_igs031_device::gfxinfo )
	GFXDECODE_DEVICE( "^tilemaps", 0, layout_8x8x4,   0, 16 )
//	GFXDECODE_DEVICE( DEVICE_SELF, 0, spritelayout, 0, 0x1000 )
GFXDECODE_END


const device_type IGS017_IGS031 = &device_creator<igs017_igs031_device>;

igs017_igs031_device::igs017_igs031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IGS017_IGS031, "IGS017_IGS031", tag, owner, clock, "igs017_igs031", __FILE__),
		device_gfx_interface(mconfig, *this, gfxinfo),
		device_video_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("igs017_igs031", ENDIANNESS_BIG, 8,15, 0, address_map_delegate(FUNC(igs017_igs031_device::map), this)),
		m_spriteram(*this, "spriteram", 0),
		m_fg_videoram(*this, "fg_videoram", 0),
		m_bg_videoram(*this, "bg_videoram", 0),
		m_palram(*this, "palram", 0),
		m_i8255(*this, "^ppi8255"),
		m_palette(*this, "^palette")
{
	m_palette_scramble_cb =  igs017_igs031_palette_scramble_delegate(FUNC(igs017_igs031_device::palette_callback_straight), this);
	m_revbits = 0;
}

const address_space_config *igs017_igs031_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

UINT16 igs017_igs031_device::palette_callback_straight(UINT16 bgr)
{
	return bgr;
}


// static
void igs017_igs031_device::set_palette_scramble_cb(device_t &device,igs017_igs031_palette_scramble_delegate newtilecb)
{
	igs017_igs031_device &dev = downcast<igs017_igs031_device &>(device);
	dev.m_palette_scramble_cb = newtilecb;
}


void igs017_igs031_device::device_start()
{
	m_palette_scramble_cb.bind_relative_to(*owner());

	m_fg_tilemap = &machine().tilemap().create(*this,  tilemap_get_info_delegate(FUNC(igs017_igs031_device::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap = &machine().tilemap().create(*this,  tilemap_get_info_delegate(FUNC(igs017_igs031_device::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);



	m_fg_tilemap->set_transparent_pen(0xf);
	m_bg_tilemap->set_transparent_pen(0xf);

	m_toggle = 0;
	m_debug_addr = 0;
	m_debug_width = 512;

	
}

void igs017_igs031_device::video_start()
{
	// make sure thie happens AFTER driver init, or things won't work
	expand_sprites();

	if (m_revbits)
	{
		UINT8 *rom  =   memregion("^tilemaps")->base();
		int size    =   memregion("^tilemaps")->bytes();
		int i;

		for (i = 0; i < size ; i++)
		{
			rom[i] = BITSWAP8(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);
//			rom[i^1] = BITSWAP8(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);
		}
	}
}


void igs017_igs031_device::device_reset()
{
	m_video_disable = 0;
	m_nmi_enable = 0;
	m_irq_enable = 0;
}

READ8_MEMBER(igs017_igs031_device::read)
{
	return space_r(offset);
}

WRITE8_MEMBER(igs017_igs031_device::write)
{
	space_w(offset, data);
}


void igs017_igs031_device::space_w(int offset, UINT8 data)
{
	space().write_byte(offset, data);
}

UINT8 igs017_igs031_device::space_r(int offset)
{
	return space().read_byte(offset);
}


WRITE8_MEMBER(igs017_igs031_device::video_disable_w)
{
	m_video_disable = data & 1;
	if (data & (~1))
		logerror("%s: unknown bits of video_disable written = %02x\n", machine().describe_context(), data);
//  popmessage("VIDEO %02X",data);
}

WRITE8_MEMBER(igs017_igs031_device::palram_w)
{
	m_palram[offset] = data;

	offset &= ~1;

	int bgr = (m_palram[offset+1] << 8) | (m_palram[offset]);

	// bitswap (some games)
	bgr = m_palette_scramble_cb(bgr);

	m_palette->set_pen_color(offset/2, pal5bit(bgr >> 0), pal5bit(bgr >> 5), pal5bit(bgr >> 10));

}


#define COLOR(_X)   (((_X)>>2)&7)

TILE_GET_INFO_MEMBER(igs017_igs031_device::get_fg_tile_info)
{
	int code = m_fg_videoram[tile_index*4+0] + (m_fg_videoram[tile_index*4+1] << 8);
	int attr = m_fg_videoram[tile_index*4+2] + (m_fg_videoram[tile_index*4+3] << 8);
	SET_TILE_INFO_MEMBER(0, code, COLOR(attr), TILE_FLIPXY( attr >> 5 ));
}
TILE_GET_INFO_MEMBER(igs017_igs031_device::get_bg_tile_info)
{
	int code = m_bg_videoram[tile_index*4+0] + (m_bg_videoram[tile_index*4+1] << 8);
	int attr = m_bg_videoram[tile_index*4+2] + (m_bg_videoram[tile_index*4+3] << 8);
	SET_TILE_INFO_MEMBER(0, code, COLOR(attr)+8, TILE_FLIPXY( attr >> 5 ));
}

WRITE8_MEMBER(igs017_igs031_device::fg_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset/4);
}

WRITE8_MEMBER(igs017_igs031_device::bg_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset/4);
}




// Eeach 16 bit word in the sprites gfx roms contains three 5 bit pens: x-22222-11111-00000 (little endian!).
// This routine expands each word into three bytes.
void igs017_igs031_device::expand_sprites()
{
	UINT8 *rom  =   memregion("^sprites")->base();
	int size    =   memregion("^sprites")->bytes();
	int i;

	m_sprites_gfx_size   =   size / 2 * 3;
	m_sprites_gfx        =   auto_alloc_array(machine(), UINT8, m_sprites_gfx_size);

	for (i = 0; i < size / 2 ; i++)
	{
		UINT16 pens = (rom[i*2+1] << 8) | rom[i*2];

		m_sprites_gfx[i * 3 + 0] = (pens >>  0) & 0x1f;
		m_sprites_gfx[i * 3 + 1] = (pens >>  5) & 0x1f;
		m_sprites_gfx[i * 3 + 2] = (pens >> 10) & 0x1f;
	}
}


/***************************************************************************
                              Sprites Format

    Offset:         Bits:               Value:

        0.b                             Y (low)

        1.b         7654 32--           Size Y (low)
                    ---- --10           Y (high)

        2.b         7654 3---           X (low)
                    ---- -210           Size Y (high)

        3.b         76-- ----           Size X (low)
                    --5- ----
                    ---4 3210           X (high)

        4.b         76-- ----           Code (low)
                    --54 3210           Size X (high)

        5.b                             Code (mid low)

        6.b                             Code (mid high)

        7.b         765- ----           Color
                    ---4 ----           Flip X
                    ---- 3---
                    ---- -210           Code (high)

    Code = ROM Address / 2 = Pixel / 3

***************************************************************************/

void igs017_igs031_device::draw_sprite(bitmap_ind16 &bitmap,const rectangle &cliprect, int sx, int sy, int dimx, int dimy, int flipx, int flipy, int color, int addr)
{
	// prepare GfxElement on the fly

	// Bounds checking
	if ( addr + dimx * dimy >= m_sprites_gfx_size )
		return;

	gfx_element gfx(m_palette, m_sprites_gfx + addr, dimx, dimy, dimx, m_palette->entries(), 0x100, 32);

	gfx.transpen(bitmap,cliprect,
				0, color,
				flipx, flipy,
				sx, sy, 0x1f    );
}

void igs017_igs031_device::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT8 *s    =   m_spriteram;
	UINT8 *end  =   m_spriteram + 0x800;

	for ( ; s < end; s += 8 )
	{
		int x,y, sx,sy, dimx,dimy, flipx,flipy, addr,color;

		y       =   s[0] + (s[1] << 8);
		x       =   s[2] + (s[3] << 8);
		addr    =   (s[4] >> 6) | (s[5] << 2) | (s[6] << 10) | ((s[7] & 0x07) << 18);
		addr    *=  3;

		flipx   =   s[7] & 0x10;
		flipy   =   0;

		dimx    =   ((((s[4] & 0x3f)<<2) | ((s[3] & 0xc0)>>6))+1) * 3;
		dimy    =   ((y >> 10) | ((x & 0x03)<<6))+1;

		x       >>= 3;
		sx      =   (x & 0x1ff) - (x & 0x200);
		sy      =   (y & 0x1ff) - (y & 0x200);

		// sprites list stop (used by mgdh & sdmg2 during don den)
		if (sy == -0x200)
			break;

		color = (s[7] & 0xe0) >> 5;

		draw_sprite(bitmap, cliprect, sx, sy, dimx, dimy, flipx, flipy, color, addr);
	}
}

// A simple gfx viewer (toggle with T)
int igs017_igs031_device::debug_viewer(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_T))   m_toggle = 1-m_toggle;
	if (m_toggle)    {
		int h = 256, w = m_debug_width, a = m_debug_addr;

		if (machine().input().code_pressed(KEYCODE_O))        w += 1;
		if (machine().input().code_pressed(KEYCODE_I))        w -= 1;

		if (machine().input().code_pressed(KEYCODE_U))        w += 8;
		if (machine().input().code_pressed(KEYCODE_Y))        w -= 8;

		if (machine().input().code_pressed(KEYCODE_RIGHT))    a += 1;
		if (machine().input().code_pressed(KEYCODE_LEFT))     a -= 1;

		if (machine().input().code_pressed(KEYCODE_DOWN))     a += w;
		if (machine().input().code_pressed(KEYCODE_UP))       a -= w;

		if (machine().input().code_pressed(KEYCODE_PGDN))     a += w * h;
		if (machine().input().code_pressed(KEYCODE_PGUP))     a -= w * h;

		if (a < 0)      a = 0;
		if (a > m_sprites_gfx_size)  a = m_sprites_gfx_size;

		if (w <= 0)     w = 0;
		if (w > 1024)   w = 1024;

		bitmap.fill(0, cliprect);

		draw_sprite(bitmap, cliprect, 0,0, w,h, 0,0, 0, a);

		popmessage("a: %08X w: %03X p: %02x-%02x-%02x",a,w,m_sprites_gfx[a/3*3+0],m_sprites_gfx[a/3*3+1],m_sprites_gfx[a/3*3+2]);
		m_debug_addr = a;
		m_debug_width = w;
		osd_sleep(200000);
		return 1;
	}
#endif
	return 0;
}

UINT32 igs017_igs031_device::screen_update_igs017(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (debug_viewer(bitmap,cliprect))
		return 0;

	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_video_disable)
		return 0;

	if (layers_ctrl & 1)    m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	if (layers_ctrl & 4)    draw_sprites(bitmap, cliprect);

	if (layers_ctrl & 2)    m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

#if 0
WRITE16_MEMBER(igs017_igs031_device::irq_enable_w)
{
	if (ACCESSING_BITS_0_7)
		m_irq_enable = data & 1;

	if (data != 0 && data != 1 && data != 0xff)
		logerror("%s: irq_enable = %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(igs017_igs031_device::nmi_enable_w)
{
	if (ACCESSING_BITS_0_7)
		m_nmi_enable = data & 1;

	if (data != 0 && data != 1 && data != 0xff)
		logerror("%s: nmi_enable = %04x\n", machine().describe_context(), data);
}
#endif

WRITE8_MEMBER(igs017_igs031_device::nmi_enable_w)
{
	m_nmi_enable = data & 1;
	if (data & (~1))
		logerror("%s: nmi_enable = %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(igs017_igs031_device::irq_enable_w)
{
	m_irq_enable = data & 1;
	if (data & (~1))
		logerror("%s: irq_enable = %02x\n", machine().describe_context(), data);
}


