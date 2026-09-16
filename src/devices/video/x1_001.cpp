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
	appear to have 0x800 bytes of RAM that isn't connected to the sprite chip
	between the banks.

	This should implement device_video_interface, since it generates the vertical
	and horizontal blanking and sync signals from a master clock.

	understand sprite limits / how sprite 0 sometimes must be skipped, maybe
	layers being enabled counts against limits? maybe some chip revisions skip
	the first sprite? see note in device_start

	understand why we need offset kludges for some games, they can even differ
	from layer to layer

	*/

#include "emu.h"
#include "x1_001.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(X1_001, x1_001_device, "x1_001", "Seta X1-001 Sprites")

x1_001_device::x1_001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X1_001, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_gfxbank_cb(*this)
	, m_fg_flipxoffs(0)
	, m_fg_noflipxoffs(0)
	, m_fg_flipyoffs(0)
	, m_fg_noflipyoffs(0)
	, m_bg_flipyoffs(0)
	, m_bg_noflipyoffs(0)
	, m_bg_flipxoffs(0)
	, m_bg_noflipxoffs(0)
	, m_colorbase(0)
	, m_spritelimit(0x1ff)
	, m_transpen(0)
{
}

void x1_001_device::device_start()
{
	m_spriteylow = std::make_unique<uint8_t[]>(0x300); // 0x200 low y + 0x100 bg stuff
	m_spritecode = std::make_unique<uint16_t[]>(0x2000);

	/* chukatai draws a column on the left from uninitialized RAM which causes garbage

	   the reason for this is because the m_spriteylow only gets populated from 0x000 to 0x17d
	   with the remainder of the ram (0x17e - 0x1ff) never being written to.

	   If we initialize ram to 0xff it has a single line in the top left instead, but
	   the PCB has no error at all, initializing to 0xf8 is better, but unlikely from a hardware
	   perspective.

	   jjsquawk has a garbage tile in the bottom left instead, but in this case the entire
	   m_spriteylow is initialized (to 0), but sprite entry 0 for the other tables is never
	   written to, again meaning it picks up whatever we clear the RAM to as positional
	   values.

	   we can't simply skip sprite 0 as other games (mostly tnzs.cpp ones, eg. kabukiz) rely
	   on it being rendered, even if a number of seta.cpp games specifically avoid it.

	   maybe other components on the board determine the sprite start / end limits on a per
	   game basis?

	*/

	std::fill_n(m_spritectrl, 4, 0xff);
	std::fill_n(m_spriteylow.get(), 0x300, 0xff);
	std::fill_n(m_spritecode.get(), 0x2000, 0xffff);

	m_bgflag = 0x00;

	m_gfxbank_cb.resolve();

	save_item(NAME(m_bgflag));
	save_item(NAME(m_spritectrl));
	save_pointer(NAME(m_spriteylow),0x300);
	save_pointer(NAME(m_spritecode),0x2000);
}

void x1_001_device::device_reset()
{
}

uint16_t x1_001_device::spritectrl_r16(offs_t offset)
{
	return m_spritectrl[offset];
}

void x1_001_device::spritectrl_w16(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_spritectrl[offset] = data;
	}
}

uint8_t x1_001_device::spritectrl_r8(offs_t offset)
{
	return m_spritectrl[offset];
}

void x1_001_device::spritectrl_w8(offs_t offset, uint8_t data)
{
	m_spritectrl[offset] = data;
}

uint16_t x1_001_device::spriteylow_r16(offs_t offset)
{
	return m_spriteylow[offset];
}

void x1_001_device::spriteylow_w16(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_spriteylow[offset] = data;
	}
}

uint8_t x1_001_device::spriteylow_r8(offs_t offset)
{
	return m_spriteylow[offset];
}

void x1_001_device::spriteylow_w8(offs_t offset, uint8_t data)
{
	m_spriteylow[offset] = data;
}


uint8_t x1_001_device::spritecodelow_r8(offs_t offset)
{
	return m_spritecode[offset] & 0xff;
}

void x1_001_device::spritecodelow_w8(offs_t offset, uint8_t data)
{
	m_spritecode[offset] = (m_spritecode[offset] & 0xff00) | data;
}

uint8_t x1_001_device::spritecodehigh_r8(offs_t offset)
{
	return (m_spritecode[offset] >> 8) & 0xff;
}

void x1_001_device::spritecodehigh_w8(offs_t offset, uint8_t data)
{
	m_spritecode[offset] = (m_spritecode[offset] & 0x00ff) | uint16_t(data) << 8;
}

uint16_t x1_001_device::spritecode_r16(offs_t offset)
{
	return m_spritecode[offset];
}

void x1_001_device::spritecode_w16(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spritecode[offset]);
}

void x1_001_device::spritebgflag_w8(uint8_t data)
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




void x1_001_device::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	int transpen;

	int offs, col;
	int xoffs, yoffs;

	int const total_color_codes   =   gfx(0)->colors();

	int const ctrl    =   m_spritectrl[0];
	int const ctrl2   =   m_spritectrl[1];

	bool const flip   =   BIT(ctrl, 6);
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

			uint16_t code = m_spritecode[i+0x400+bank];
			uint16_t color = m_spritecode[i+0x600+bank];

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

			gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff),
					transpen);

			gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff),
					transpen);

			gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff)-256,
					transpen);

			gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff)-256,
					transpen);

		}
	}
}


void x1_001_device::draw_foreground( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	int const screenflip = (m_spritectrl[0] & 0x40) >> 6;
	int const ctrl2 = m_spritectrl[1];

	int const total_color_codes   =   gfx(0)->colors();

	uint16_t *char_pointer = &m_spritecode[0x0000];
	uint16_t *x_pointer = &m_spritecode[0x0200];

	// note that drgnunit, stg and qzkklogy run on the same board, yet they need different alignment
	int xoffs = screenflip ? m_fg_flipxoffs : m_fg_noflipxoffs;
	int yoffs = screenflip ? m_fg_flipyoffs : m_fg_noflipyoffs;

	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
	{
		char_pointer += bank_size;
		x_pointer += bank_size;
	}

	int const max_y = screen.height();

	/* Draw up to 512 sprites, mjyuugi has glitches if you draw them all.. */
	for (int i = m_spritelimit; i >= 0; i--)
	{
		int code, color, sx, sy, flipx, flipy;

		code = char_pointer[i] & 0x3fff;
		color = (x_pointer[i] & 0xf800) >> 11;

		sx = (x_pointer[i] & 0x00ff) - (x_pointer[i] & 0x0100);
		sy =  (m_spriteylow[i] & 0xff);
		flipx = char_pointer[i] & 0x8000;
		flipy = char_pointer[i] & 0x4000;

		if (!m_gfxbank_cb.isnull()) code = m_gfxbank_cb(code, x_pointer[i] >> 8);

		color %= total_color_codes;

		color += m_colorbase;

		if (screenflip)
		{
			sy = max_y - sy
				+(screen.height() - (screen.visible_area().max_y + 1));
			flipx = !flipx;
			flipy = !flipy;
		}

		gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);

		/* wrap around x */
		gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);


		gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

		/* wrap around x */
		gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

	}
}



void x1_001_device::setac_eof()
{
	// is this handling right?
	// it differs to tnzs, and thundercade has sprite flickering issues (not related to the devicification)

	int const ctrl2 = m_spritectrl[1];

	if (~ctrl2 & 0x20)
	{
		if (ctrl2 & 0x40)
			std::copy_n(&m_spritecode[0x1000], 0x800, &m_spritecode[0x0000]);
		else
			std::copy_n(&m_spritecode[0x0000], 0x800, &m_spritecode[0x1000]);
	}
}

void x1_001_device::tnzs_eof()
{
	int const ctrl2 = m_spritectrl[1];
	if (~ctrl2 & 0x20)
	{
		// note I copy sprites only. setac implementation also copies the "floating tilemap"
		if (ctrl2 & 0x40)
			std::copy_n(&m_spritecode[0x0800], 0x400, &m_spritecode[0x0000]);
		else
			std::copy_n(&m_spritecode[0x0000], 0x400, &m_spritecode[0x0800]);

		// and I copy the "floating tilemap" BACKWARDS - this fixes kabukiz
		std::copy_n(&m_spritecode[0x0c00], 0x0400, &m_spritecode[0x0400]);
	}
}

void x1_001_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	draw_background(bitmap, cliprect, bank_size);
	draw_foreground(screen, bitmap, cliprect, bank_size);
}
