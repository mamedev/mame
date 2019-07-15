// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
	/*
	emulation of Seta sprite chips
	X1-001A  X1-002A (SDIP64)

	these always seem to be used as a pair, some board have been seen without the
	'A', so it's probably a chip revision / bugfix.

	used by:

	seta.cpp
	taito_x.cpp
	tnzs.cpp
	srmp2.cpp
	champbwl.cpp
	cchance.cpp
	thedealr.cpp

	note: the data bus is almost certainly 8-bit, dating back to the earliest
	    hardware the games were used on.  the RAM arrangements changes
	    slightly between games depending on how the RAM is hooked up to the
	    main cpu.

	'y' low bits are NEVER buffered?

	TODO:
	use getter callbacks in drivers for spriterams rather than allocating memory
	here, then we won't have to allocate extra memory for seta.cpp games which
	appear to have 0x800 bytes of RAM that isn't connceted to the sprite chip
	between the banks.

	*/

#include "emu.h"
#include "seta001.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(SETA001_SPRITE, seta001_device, "seta001", "Seta SETA001 Sprites")

seta001_device::seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SETA001_SPRITE, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

void seta001_device::device_start()
{
	m_spriteylow = std::make_unique<uint8_t[]>(0x300); // 0x200 low y + 0x100 bg stuff
	m_spritecodelow = std::make_unique<uint8_t[]>(0x2000);
	m_spritecodehigh = std::make_unique<uint8_t[]>(0x2000);

	/* chukatai draws a column on the left from uninitialized RAM which causes garbage

	   the reason for this is because the m_spriteylow only gets populated from 0x000 to 0x17d
	   with the remainder of the ram (0x17e - 0x1ff) never being written to.

	   If we initialize ram to 0xff it has a single line in the top left instead, but
	   the PCB has no error at all, initializing to 0xf8 is better, but unlikely from a hardware
	   perspective.

	   jjsquawk has a garbage tile in the bottom left instead, but in this case the entire
	   m_spriteylow is initalized (to 0), but sprite entry 0 for the other tables is never
	   written to, again meaning it picks up whatever we clear the RAM to as positional
	   values.

	   we can't simply skip sprite 0 as other games (mostly tnzs.cpp ones, eg. kabukiz) rely
	   on it being rendered, even if a number of seta.cpp games specifically avoid it.

	   maybe other components on the board determine the sprite start / end limits on a per
	   game basis?

	*/

	memset(m_spritectrl,0xff,4);
	memset(m_spriteylow.get(),0xff,0x300);
	memset(m_spritecodelow.get(),0xff,0x2000);
	memset(m_spritecodehigh.get(),0xff,0x2000);

	m_fg_flipxoffs = 0;
	m_fg_noflipxoffs = 0;

	m_fg_flipyoffs = 0;
	m_fg_noflipyoffs = 0;

	m_bg_flipyoffs = 0;
	m_bg_noflipyoffs = 0;

	m_bg_flipxoffs = 0;
	m_bg_noflipxoffs = 0;

	m_transpen = 0;
	m_colorbase = 0;
	m_spritelimit = 0x1ff;

	m_bgflag = 0x00;

	m_gfxbank_cb.bind_relative_to(*owner());

	save_item(NAME(m_bgflag));
	save_item(NAME(m_spritectrl));
	save_pointer(NAME(m_spriteylow),0x300);
	save_pointer(NAME(m_spritecodelow),0x2000);
	save_pointer(NAME(m_spritecodehigh),0x2000);
}

void seta001_device::device_reset()
{
}

READ16_MEMBER( seta001_device::spritectrl_r16 )
{
	return m_spritectrl[offset];
}

WRITE16_MEMBER( seta001_device::spritectrl_w16 )
{
	if (ACCESSING_BITS_0_7)
	{
		m_spritectrl[offset] = data;
	}
}

READ8_MEMBER( seta001_device::spritectrl_r8 )
{
	return m_spritectrl[offset];
}

WRITE8_MEMBER( seta001_device::spritectrl_w8 )
{
	m_spritectrl[offset] = data;
}

READ16_MEMBER( seta001_device::spriteylow_r16 )
{
	return m_spriteylow[offset];
}

WRITE16_MEMBER( seta001_device::spriteylow_w16 )
{
	if (ACCESSING_BITS_0_7)
	{
		m_spriteylow[offset] = data;
	}
}

READ8_MEMBER( seta001_device::spriteylow_r8 )
{
	return m_spriteylow[offset];
}

WRITE8_MEMBER( seta001_device::spriteylow_w8 )
{
	m_spriteylow[offset] = data;
}


READ8_MEMBER( seta001_device::spritecodelow_r8 )
{
	return m_spritecodelow[offset];
}

WRITE8_MEMBER( seta001_device::spritecodelow_w8 )
{
	m_spritecodelow[offset] = data;
}

READ8_MEMBER( seta001_device::spritecodehigh_r8 )
{
	return m_spritecodehigh[offset];
}

WRITE8_MEMBER( seta001_device::spritecodehigh_w8 )
{
	m_spritecodehigh[offset] = data;
}

READ16_MEMBER( seta001_device::spritecode_r16 )
{
	uint16_t ret = m_spritecodelow[offset];
	ret |= m_spritecodehigh[offset] << 8;
	return ret;
}

WRITE16_MEMBER( seta001_device::spritecode_w16 )
{
	if (ACCESSING_BITS_0_7) m_spritecodelow[offset] = data & 0x00ff;
	if (ACCESSING_BITS_8_15)  m_spritecodehigh[offset] = (data & 0xff00)>>8;
}

WRITE8_MEMBER( seta001_device::spritebgflag_w8 )
{
	m_bgflag = data;
}

/***************************************************************************


                                Sprites Drawing


***************************************************************************/

/*

these mostly seem to be controlling the tilemap part of the chip

twineagl:   10 2d 0f 10   (ship)
tndrcade:   58 2d 07 18   (start of game - yes, flip on!)
arbalest:   18 2d 0f 10   (logo)
metafox :   18 21 0f f0   (bomb)
zingzip :   10 2c 00 0f   (bomb)
wrofaero:   10 21 00 ff   (test mode)
thunderl:   10 6c 00 ff   (always?)
krzybowl:   11 28 c0 ff   (game)
kiwame  :   16 21 7f 00   (logo)
oisipuzl:   59 20 00 00   (game - yes, flip on!)
superman:   10 21 07 38   (game)
twineagl:   00 27 00 0f   (test mode)
doraemon:   19 2a 00 03   (always)
srmp2:      1e 20 xx xx
tnzs:       1a 20 xx xx

format
 byte 0     byte 1     byte 2     byte3
 -f-e uUss  -Bb- cccc  ---- ----  ---- ----

 f = flipy
 e = possible enable (only disabled in twineagl test mode? check if it needs sprites)
 u = sometimes set
 U = sometimes set (srmp2)
 s = start column
 B = buffering control - toggles often (kabukiz doesn't seem to use these like the other games)
 b = buffering control
 c = number of columns, 0x0 = disabled, 0x01 = 16 columns 0x2 = 2 coluns ... 0xf = 15 columns

 bytes 2 and 3 get set in a bitwize way in the tnzs driver games while horizontal scrolling happens, dependong on how
 far we've scrolled, what they actually do is unclear.

*/




void seta001_device::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	int transpen;

	int offs, col;
	int xoffs, yoffs;

	int const total_color_codes   =   m_gfxdecode->gfx(0)->colors();

	int const ctrl    =   m_spritectrl[0];
	int const ctrl2   =   m_spritectrl[1];

	int const flip    =   ctrl & 0x40;
	int numcol  =   ctrl2 & 0x0f;

	int scrollx, scrolly;

	uint32_t upper;

	const uint8_t* scrollram = &m_spriteylow[0x200];

	/* Sprites Banking and/or Sprites Buffering */
	uint16_t bank = ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? bank_size : 0 );

	int const max_y   =   0xf0;

	int startcol = 0;

	if (ctrl & 0x01)
		startcol += 0x4;

	if (ctrl & 0x02)
		startcol += 0x8;

	xoffs = flip ? m_bg_flipxoffs : m_bg_noflipxoffs;
	yoffs = flip ? m_bg_flipyoffs : m_bg_noflipyoffs;

	if (m_bgflag & 0x80)
		transpen = -1;
	else
		transpen = m_transpen;

	if (numcol == 1)
		numcol = 16;

	upper = m_spritectrl[2] + m_spritectrl[3] * 256;

	for (col = 0; col < numcol; col++)
	{
		scrollx = scrollram[col * 0x10 + 4];
		scrolly = scrollram[col * 0x10];

		/* draw this column */
		for ( offs = 0 ; offs < 0x20; offs += 1 )
		{
			int i = ((col+startcol)&0xf) * 32 + offs;

			int code = ((m_spritecodehigh[i+0x400+bank]) << 8) | m_spritecodelow[i+0x400+bank];
			int color =((m_spritecodehigh[i+0x600+bank]) << 8) | m_spritecodelow[i+0x600+bank];

			int flipx   =   code & 0x8000;
			int flipy   =   code & 0x4000;

			int sx      =     scrollx + xoffs  + (offs & 1) * 16;
			int sy      =   -(scrolly + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col)) sx -= 256;

			if (flip)
			{
				sy = max_y - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color   =   ( color >> (16-5) ) % total_color_codes;
			code &= 0x3fff;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff),
					transpen);

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff),
					transpen);

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff)-256,
					transpen);

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff)-256,
					transpen);

		}
	}
}


void seta001_device::draw_foreground( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	int const screenflip = (m_spritectrl[0] & 0x40) >> 6;
	int i;
	int const ctrl2 = m_spritectrl[1];
	int xoffs, yoffs;

	int const total_color_codes   =   m_gfxdecode->gfx(0)->colors();

	uint8_t *char_pointer = &m_spritecodelow[0x0000];
	uint8_t *x_pointer = &m_spritecodelow[0x0200];
	uint8_t *ctrl_pointer = &m_spritecodehigh[0x0000];
	uint8_t *color_pointer = &m_spritecodehigh[0x0200];

	xoffs   =   screenflip ? m_fg_flipxoffs : m_fg_noflipxoffs;
	yoffs   =   screenflip ? m_fg_flipyoffs : m_fg_noflipyoffs;

	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
	{
		char_pointer += bank_size;
		x_pointer += bank_size;
		ctrl_pointer += bank_size;
		color_pointer += bank_size;
	}

	int const max_y = screen.height();

	/* Draw up to 512 sprites, mjyuugi has glitches if you draw them all.. */
	for (i = m_spritelimit; i >= 0; i--)
	{
		int code, color, sx, sy, flipx, flipy;

		code = char_pointer[i] + ((ctrl_pointer[i] & 0x3f) << 8);
		color = (color_pointer[i] & 0xf8) >> 3;

		sx = x_pointer[i] - ((color_pointer[i] & 1) << 8);
		sy =  (m_spriteylow[i] & 0xff);
		flipx = ctrl_pointer[i] & 0x80;
		flipy = ctrl_pointer[i] & 0x40;

		if (!m_gfxbank_cb.isnull()) code = m_gfxbank_cb(code, color_pointer[i]);

		color %= total_color_codes;

		color += m_colorbase;

		if (screenflip)
		{
			sy = max_y - sy
				+(screen.height() - (screen.visible_area().max_y + 1));
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);

		/* wrap around x */
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);


		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

		/* wrap around x */
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

	}
}



void seta001_device::setac_eof()
{
	// is this handling right?
	// it differs to tnzs, and thundercade has sprite flickering issues (not related to the devicification)

	int const ctrl2 = m_spritectrl[1];

	if (~ctrl2 & 0x20)
	{
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000], &m_spritecodelow[0x1000],0x800);
			memcpy(&m_spritecodehigh[0x0000],&m_spritecodehigh[0x1000],0x800);
		}
		else
		{
			memcpy( &m_spritecodelow[0x1000], &m_spritecodelow[0x0000],0x800);
			memcpy(&m_spritecodehigh[0x1000],&m_spritecodehigh[0x0000],0x800);
		}
	}
}

void seta001_device::tnzs_eof( void )
{
	int const ctrl2 = m_spritectrl[1];
	if (~ctrl2 & 0x20)
	{
		// note I copy sprites only. setac implementation also copies the "floating tilemap"
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000],  &m_spritecodelow[0x0800], 0x0400);
			memcpy(&m_spritecodehigh[0x0000], &m_spritecodehigh[0x0800], 0x0400);
		}
		else
		{
			memcpy( &m_spritecodelow[0x0800],  &m_spritecodelow[0x0000], 0x0400);
			memcpy(&m_spritecodehigh[0x0800], &m_spritecodehigh[0x0000], 0x0400);
		}

		// and I copy the "floating tilemap" BACKWARDS - this fixes kabukiz
		memcpy( &m_spritecodelow[0x0400],  &m_spritecodelow[0x0c00], 0x0400);
		memcpy(&m_spritecodehigh[0x0400], &m_spritecodehigh[0x0c00], 0x0400);
	}

}

void seta001_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	draw_background(bitmap, cliprect, bank_size);
	draw_foreground(screen, bitmap, cliprect, bank_size);
}
