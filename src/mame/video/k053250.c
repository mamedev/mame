// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "k053250.h"

const device_type K053250 = &device_creator<k053250_device>;

k053250_device::k053250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053250, "K053250 LVC", tag, owner, clock, "k053250", __FILE__),
		device_gfx_interface(mconfig, *this),
		device_video_interface(mconfig, *this)
{
}

void k053250_device::static_set_offsets(device_t &device, int offx, int offy)
{
	k053250_device &dev = downcast<k053250_device &>(device);
	dev.m_offx = offx;
	dev.m_offy = offy;
}

void k053250_device::unpack_nibbles()
{
	if(!m_region)
		throw emu_fatalerror("k053250 %s: no associated region found\n", tag());

	const UINT8 *base = m_region->base();
	int size = m_region->bytes();

	m_unpacked_rom.resize(size*2);

	for(int i=0; i<size; i++)
	{
		m_unpacked_rom[2*i] = base[i] >> 4;
		m_unpacked_rom[2*i+1] = base[i] & 0xf;
	}
}

void k053250_device::device_start()
{
	m_ram.resize(0x6000/2);
	m_buffer[0] = &m_ram[0x2000];
	m_buffer[1] = &m_ram[0x2800];

	unpack_nibbles();

	save_item(NAME(m_ram));
	save_item(NAME(m_regs));
	save_item(NAME(m_page));
	save_item(NAME(m_frame));
}

void k053250_device::device_reset()
{
	m_page = 0;
	m_frame = -1;
	memset(m_regs, 0, sizeof(m_regs));
}

// utility function to render a clipped scanline vertically or horizontally
inline void k053250_device::pdraw_scanline32(bitmap_rgb32 &bitmap, const pen_t *pal_base, UINT8 *source,
										const rectangle &cliprect, int linepos, int scroll, int zoom,
										UINT32 clipmask, UINT32 wrapmask, UINT32 orientation, bitmap_ind8 &priority, UINT8 pri)
{
// a sixteen-bit fixed point resolution should be adequate to our application
#define FIXPOINT_PRECISION      16
#define FIXPOINT_PRECISION_HALF (1<<(FIXPOINT_PRECISION-1))

	int end_pixel, flip, dst_min, dst_max, dst_start, dst_length;

	UINT32 src_wrapmask;
	UINT8  *src_base;
	int src_fx, src_fdx;
	int pix_data, dst_offset;
	UINT8  *pri_base;
	UINT32 *dst_base;
	int dst_adv;

	// flip X and flip Y also switch role when the X Y coordinates are swapped
	if (!(orientation & ORIENTATION_SWAP_XY))
	{
		flip = orientation & ORIENTATION_FLIP_X;
		dst_min = cliprect.min_x;
		dst_max = cliprect.max_x;
	}
	else
	{
		flip = orientation & ORIENTATION_FLIP_Y;
		dst_min = cliprect.min_y;
		dst_max = cliprect.max_y;
	}

	if (clipmask)
	{
		// reject scanlines that are outside of the target bitmap's right(bottom) clip boundary
		dst_start = -scroll;
		if (dst_start > dst_max) return;

		// calculate target length
		dst_length = clipmask + 1;
		if (zoom) dst_length = (dst_length << 6) / zoom;

		// reject scanlines that are outside of the target bitmap's left(top) clip boundary
		end_pixel = dst_start + dst_length - 1;
		if (end_pixel < dst_min) return;

		// clip scanline tail
		if ((end_pixel -= dst_max) > 0) dst_length -= end_pixel;

		// reject 0-length scanlines
		if (dst_length <= 0) return;

		// calculate zoom factor
		src_fdx = zoom << (FIXPOINT_PRECISION-6);

		// clip scanline head
		end_pixel = dst_min;
		if ((end_pixel -= dst_start) > 0)
		{
			// chop scanline to the correct length and move target start location to the left(top) clip boundary
			dst_length -= end_pixel;
			dst_start = dst_min;

			// and skip the source for the left(top) clip region
			src_fx = end_pixel * src_fdx + FIXPOINT_PRECISION_HALF;
		}
		else
			// the point five bias is to ensure even distribution of stretched or shrinked pixels
			src_fx = FIXPOINT_PRECISION_HALF;

		// adjust flipped source
		if (flip)
		{
			// start from the target's clipped end if the scanline is flipped
			dst_start = dst_max + dst_min - dst_start - (dst_length-1);

			// and move source start location to the opposite end
			src_fx += (dst_length-1) * src_fdx - 1;
			src_fdx = -src_fdx;
		}
	}
	else
	{
		// draw wrapped scanline at virtual bitmap boundary when source clipping is off
		dst_start = dst_min;
		dst_length = dst_max - dst_min + 1; // target scanline spans the entire visible area
		src_fdx = zoom << (FIXPOINT_PRECISION-6);

		// pre-advance source for the clipped region
		if (!flip)
			src_fx = (scroll + dst_min) * src_fdx + FIXPOINT_PRECISION_HALF;
		else
		{
			src_fx = (scroll + dst_max) * src_fdx + FIXPOINT_PRECISION_HALF-1;
			src_fdx = -src_fdx;
		}
	}

	if (!(orientation & ORIENTATION_SWAP_XY))
	{
		// calculate target increment for horizontal scanlines which is exactly one
		dst_adv = 1;
		dst_offset = dst_length;
		pri_base = &priority.pix8(linepos, dst_start + dst_offset);
		dst_base = &bitmap.pix32(linepos, dst_start + dst_length);
	}
	else
	{
		// calculate target increment for vertical scanlines which is the bitmap's pitch value
		dst_adv = bitmap.rowpixels();
		dst_offset= dst_length * dst_adv;
		pri_base = &priority.pix8(dst_start, linepos + dst_offset);
		dst_base = &bitmap.pix32(dst_start, linepos + dst_offset);
	}

	// generalized
	src_base = source;

	// there is no need to wrap source offsets along with source clipping
	// so we set all bits of the wrapmask to one
	src_wrapmask = (clipmask) ? ~0 : wrapmask;

	dst_offset = -dst_offset; // negate target offset in order to terminated draw loop at 0 condition

	if (pri)
	{
		// draw scanline and update priority bitmap
		do
		{
			pix_data = src_base[(src_fx>>FIXPOINT_PRECISION) & src_wrapmask];
			src_fx += src_fdx;

			if (pix_data)
			{
				pix_data = pal_base[pix_data];
				pri_base[dst_offset] = pri;
				dst_base[dst_offset] = pix_data;
			}
		}
		while (dst_offset += dst_adv);
	}
	else
	{
		// draw scanline but do not update priority bitmap
		do
		{
			pix_data = src_base[(src_fx>>FIXPOINT_PRECISION) & src_wrapmask];
			src_fx += src_fdx;

			if (pix_data)
			{
				dst_base[dst_offset] = pal_base[pix_data];
			}
		}
		while (dst_offset += dst_adv);
	}

#undef FIXPOINT_PRECISION
#undef FIXPOINT_PRECISION_HALF
}

void k053250_device::draw( bitmap_rgb32 &bitmap, const rectangle &cliprect, int colorbase, int flags, bitmap_ind8 &priority_bitmap, int priority )
{
	UINT8 *pix_ptr;
	const pen_t *pal_base, *pal_ptr;
	UINT32 src_clipmask, src_wrapmask, dst_wrapmask;
	int linedata_offs, line_pos, line_start, line_end, scroll_corr;
	int color, offset, zoom, scroll, passes, i;
	bool wrap500 = false;

	UINT16 *line_ram = m_buffer[m_page];                // pointer to physical line RAM
	int map_scrollx = short(m_regs[0] << 8 | m_regs[1]) - m_offx; // signed horizontal scroll value
	int map_scrolly = short(m_regs[2] << 8 | m_regs[3]) - m_offy; // signed vertical scroll value
	UINT8 ctrl = m_regs[4];                                   // register four is the main control register

	// copy visible boundary values to more accessible locations
	int dst_minx  = cliprect.min_x;
	int dst_maxx  = cliprect.max_x;
	int dst_miny  = cliprect.min_y;
	int dst_maxy  = cliprect.max_y;

	int orientation  = 0;   // orientation defaults to no swapping and no flipping
	int dst_height   = 512; // virtual bitmap height defaults to 512 pixels
	int linedata_adv = 4;   // line info packets are four words(eight bytes) apart

	// switch X and Y parameters when the first bit of the control register is cleared
	if (!(ctrl & 0x01)) orientation |= ORIENTATION_SWAP_XY;

	// invert X parameters when the forth bit of the control register is set
	if   (ctrl & 0x08)  orientation |= ORIENTATION_FLIP_X;

	// invert Y parameters when the fifth bit of the control register is set
	if   (ctrl & 0x10)  orientation |= ORIENTATION_FLIP_Y;

	switch (ctrl >> 5) // the upper four bits of the control register select source and target dimensions
	{
		case 0 :
			// Xexex: L6 galaxies
			// Metam: L4 forest, L5 arena, L6 tower interior, final boss

			// crop source offset between 0 and 255 inclusive,
			// and set virtual bitmap height to 256 pixels
			src_wrapmask = src_clipmask = 0xff;
			dst_height = 0x100;
		break;
		case 1 :
			// Xexex: prologue, L7 nebulae

			// the source offset is cropped to 0 and 511 inclusive
			src_wrapmask = src_clipmask = 0x1ff;
		break;
		case 4 :
			// Xexex: L1 sky and boss, L3 planet, L5 poly-face, L7 battle ship patches
			// Metam: L1 summoning circle, L3 caves, L6 gargoyle towers

			// crop source offset between 0 and 255 inclusive,
			// and allow source offset to wrap back at 0x500 to -0x300
			src_wrapmask = src_clipmask = 0xff;
			wrap500 = true;
		break;
//      case 2 : // Xexex: title
//      case 7 : // Xexex: L4 organic stage
		default:
			// crop source offset between 0 and 1023 inclusive,
			// keep other dimensions to their defaults
			src_wrapmask = src_clipmask = 0x3ff;
		break;
	}

	// disable source clipping when the third bit of the control register is set
	if (ctrl & 0x04) src_clipmask = 0;

	if (!(orientation & ORIENTATION_SWAP_XY))   // normal orientaion with no X Y switching
	{
		line_start = dst_miny;          // the first scanline starts at the minimum Y clip location
		line_end   = dst_maxy;          // the last scanline ends at the maximum Y clip location
		scroll_corr = map_scrollx;      // concentrate global X scroll
		linedata_offs = map_scrolly;    // determine where to get info for the first line

		if (orientation & ORIENTATION_FLIP_X)
		{
			scroll_corr = -scroll_corr; // X scroll adjustment should be negated in X flipped scenarioes
		}

		if (orientation & ORIENTATION_FLIP_Y)
		{
			linedata_adv = -linedata_adv;           // traverse line RAM backward in Y flipped scenarioes
			linedata_offs += bitmap.height() - 1;   // and get info for the first line from the bottom
		}

		dst_wrapmask = ~0;  // scanlines don't seem to wrap horizontally in normal orientation
		passes = 1;         // draw scanline in a single pass
	}
	else  // orientaion with X and Y parameters switched
	{
		line_start = dst_minx;          // the first scanline starts at the minimum X clip location
		line_end   = dst_maxx;          // the last scanline ends at the maximum X clip location
		scroll_corr = map_scrolly;      // concentrate global Y scroll
		linedata_offs = map_scrollx;    // determine where to get info for the first line

		if (orientation & ORIENTATION_FLIP_Y)
		{
			scroll_corr = 0x100 - scroll_corr;  // apply common vertical correction

			// Y correction (ref: 1st and 5th boss)
			scroll_corr -= 2;   // apply unique vertical correction

			// X correction (ref: 1st boss, seems to undo non-rotated global X offset)
			linedata_offs -= 5; // apply unique horizontal correction
		}

		if (orientation & ORIENTATION_FLIP_X)
		{
			linedata_adv = -linedata_adv;       // traverse line RAM backward in X flipped scenarioes
			linedata_offs += bitmap.width() - 1;    // and get info for the first line from the bottom
		}

		if (src_clipmask)
		{
			// determine target wrap boundary and draw scanline in two passes if the source is clipped
			dst_wrapmask = dst_height - 1;
			passes = 2;
		}
		else
		{
			// otherwise disable target wraparound and draw scanline in a single pass
			dst_wrapmask = ~0;
			passes = 1;
		}
	}

	linedata_offs *= 4;                             // each line info packet has four words(eight bytes)
	linedata_offs &= 0x7ff;                         // and it should wrap at the four-kilobyte boundary
	linedata_offs += line_start * linedata_adv;     // pre-advance line info offset for the clipped region

	// load physical palette base
	pal_base = m_palette->pens() + (colorbase << 4) % m_palette->entries();

	// walk the target bitmap within the visible area vertically or horizontally, one line at a time
	for (line_pos=line_start; line_pos <= line_end; linedata_offs += linedata_adv, line_pos++)
	{
		linedata_offs &= 0x7ff;                     // line info data wraps at the four-kilobyte boundary

		color  = line_ram[linedata_offs];           // get scanline color code
		if (color == 0xffff) continue;              // reject scanline if color code equals minus one

		offset   = line_ram[linedata_offs + 1];     // get first pixel offset in ROM
		if (!(color & 0xff) && !offset) continue;   // reject scanline if both color and pixel offset are 0

		// calculate physical palette location
		// there can be thirty-two color codes and each code represents sixteen pens
		pal_ptr = pal_base + ((color & 0x1f) << 4);

		// calculate physical pixel location
		// each offset unit represents 256 pixels and should wrap at ROM boundary for safety
		pix_ptr = &m_unpacked_rom[((offset << 8) % m_unpacked_rom.size())];

		// get scanline zoom factor
		// For example, 0x20 doubles the length, 0x40 maintains a one-to-one length,
		// and 0x80 halves the length. The zoom center is at the beginning of the
		// scanline therefore it is not necessary to adjust render start position
		zoom    = line_ram[linedata_offs + 2];

		scroll  = (short)line_ram[linedata_offs + 3];   // get signed local scroll value for the current scanline

		// scavenged from old code; improves Xexex' first level sky
		if (wrap500 && scroll >= 0x500) scroll -= 0x800;

		scroll += scroll_corr;  // apply final scroll correction
		scroll &= dst_wrapmask; // wraparound scroll value if necessary

		// draw scanlines wrapped at virtual bitmap boundary in two passes
		// this should not impose too much overhead due to clipping performed by the render code
		i = passes;
		do
		{
			/*
			    Parameter descriptions:

			    bitmap       : pointer to a MAME bitmap as the render target
			    pal_ptr      : pointer to the palette's physical location relative to the scanline
			    pix_ptr      : pointer to the physical start location of source pixels in ROM
			    cliprect     : pointer to a rectangle structue which describes the visible area of the target bitmap
			    line_pos     : scanline render position relative to the target bitmap
			                   should be a Y offset to the target bitmap in normal orientaion,
			                   or an X offset to the target bitmap if X,Y are swapped
			    scroll       : source scroll value of the scanline
			    zoom         : source zoom factor of the scanline
			    src_clipmask : source offset clip mask; source pixels with offsets beyond the scope of this mask will not be drawn
			    src_wrapmask : source offset wrap mask; wraps source offset around, no effect when src_clipmask is set
			    orientation  : flags indicating whether scanlines should be drawn horizontally, vertically, forward or backward
			    priority     : value to be written to the priority bitmap, no effect when equals 0
			*/
			pdraw_scanline32(bitmap, pal_ptr, pix_ptr, cliprect,
				line_pos, scroll, zoom, src_clipmask, src_wrapmask, orientation, priority_bitmap, (UINT8)priority);

			// shift scanline position one virtual screen upward to render the wrapped end if necessary
			scroll -= dst_height;
		}
		while (--i);
	}
}

void k053250_device::dma(int limiter)
{
	int current_frame = m_screen->frame_number();

	if (limiter && current_frame == m_frame)
		return; // make sure we only do DMA transfer once per frame

	m_frame = current_frame;
	memcpy(m_buffer[m_page], &m_ram[0], 0x1000);
	m_page ^= 1;
}

READ16_MEMBER(k053250_device::reg_r)
{
	return m_regs[offset];
}

WRITE16_MEMBER(k053250_device::reg_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// start LVC DMA transfer at the falling edge of control register's bit1
		if (offset == 4 && !(data & 2) && (m_regs[4] & 2))
			dma(1);

		m_regs[offset] = data;
	}
}

READ16_MEMBER(k053250_device::ram_r)
{
	return m_ram[offset];
}

WRITE16_MEMBER(k053250_device::ram_w)
{
	COMBINE_DATA(&m_ram[offset]);
}

READ16_MEMBER(k053250_device::rom_r)
{
	return m_region->base()[0x80000 * m_regs[6] + 0x800 * m_regs[7] + offset/2];
}
