// license:BSD-3-Clause
// copyright-holders:David Haywood, Olivier Galibert

#include "k054338.h"

/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

// The 054338 "CLTC" is a final blender/modifier.  It is connected to
// a series of 54573 dacs (on per color channel) or one integrated
// 56766 which also handles brightness.

// Input:
// - two pixels composed each of:
//   - 13-bit color index
//   - 2-bit shadow information
//   - 2-bit mixing information
//   - 2-bit brightness information
// - one bit of "is a pixel present?"

// Output:
//   - 8-bit R, G and B
//   - 8-bit intensity (may be unused if not connected to a 56766)

// The chip is connected to the palette ram.  It is designed to get
// its inputs from a 55555 mixer chip, but it has been used with a
// 53251 with some hacking (fixed second pixel connected to a specific
// tilemap layer, fixed mixing/brigthness info).

// Theory of operation:
// - lookup the pixel colors in the colormap
// - blend the two pixels together, either with an average or additively, saturate
// - add shadow-controlled value to each channel, saturate or wrap
// - output the result conjointly with a brightness value

// If there's no pixel, a background color is output instead

// Register map:
//     f  e  d  c  b  a  9  8  7  6  5  4  3  2  1  0
// 00  .  .  .  .  .  .  .  .  --------back R--------
// 02  --------back G--------  --------back B--------
// 04  .  .  .  .  .  .  .  --------shadow 1 R-------
// 06  .  .  .  .  .  .  .  --------shadow 1 G-------
// 08  .  .  .  .  .  .  .  --------shadow 1 B-------
// 0a  .  .  .  .  .  .  .  --------shadow 2 R-------
// 0c  .  .  .  .  .  .  .  --------shadow 2 G-------
// 0e  .  .  .  .  .  .  .  --------shadow 2 B-------
// 10  .  .  .  .  .  .  .  --------shadow 3 R-------
// 12  .  .  .  .  .  .  .  --------shadow 3 G-------
// 14  .  .  .  .  .  .  .  --------shadow 3 B-------
// 16  .  .  .  .  .  .  .  .  -------briset 1-------
// 18  -------briset 2-------  -------briset 3-------
// 1a  .  .  .  .  .  .  .  .  .  . x1 -----mv 1-----
// 1c  .  . x2 -----mv 2-----  .  . x3 -----mv 3-----
// 1e  .  .  .  .  .  .  .  .  .  . CL WA BR SH MI KI


// MI (mixpri): select the second (0) or the first (1) pixel as
//              primary in the blending and as source of the 2-bit
//              mixing mode.  Mixing mode 0 is no mixing, pass the
//              primary pixel color as-is.
// x<n>: mixing mode n is average (0) or additive (1)
// mv<n>: mixing mode n level (0-31)

// mixing formula for each channel:
// - average:  result = (primary * (32-level) + secondary * level) / 32
// - additive: result = min(0xff, primary * (32-level)/32 + secondary)

// SH (shdpri): select the second (0) or the first (1) pixel as source
//              of the 2-bit shadow mode. Shadow mode 0 is no shadow,
//              output the color unchanged.
// CL (clipsl): select whether to saturate (0) or wrap (1) the result
// shadow <n><c>: value to add to channel c in shadow mode n. 9-bits
//                signed (-255..+255, -256 is not supposed to be used)
//
// clipping formula:
// - saturate: result = min(0xff, max(0, input + value))
// - wrap:     r1 = abs(input + value), result = r1 > 255 ? 510 - r1 : r1
// Hardware-wise, wrapping is doing a 9-bit signed add and taking the
// absolute value

// BR (brtpri): select the second (0) or the first (1) pixel as source
//              of the 2-bit brightness mode. Brightness mode 0 is no
//              brightness, outputs 0xff.
// briset<n>: brigthness value to output in mode n

// KI (kill): output background color instead of computed pixels (0)
// back<c>: background color channel c
// Background color is also output when the input says there's no pixel present

// WA (wait): when actually mixing two colors, the palette ram access
//            is saturated.  If the cpu tries to access the palette ram at that
//            time, it will either wait (0) or take priority (1) but then the
//            output pixel will be incorrect.


const device_type K054338 = device_creator<k054338_device>;

DEVICE_ADDRESS_MAP_START(map, 16, k054338_device)
	AM_RANGE(0x00, 0x01) AM_WRITE(backr_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(backgb_w)
	AM_RANGE(0x04, 0x13) AM_WRITE(shadow_w)
	AM_RANGE(0x14, 0x15) AM_WRITE(shadow2_w) // Memory subsystem limitation
	AM_RANGE(0x16, 0x17) AM_WRITE(bri1_w)
	AM_RANGE(0x18, 0x19) AM_WRITE(bri23_w)
	AM_RANGE(0x1a, 0x1b) AM_WRITE(mix1_w)
	AM_RANGE(0x1c, 0x1d) AM_WRITE(mix23_w)
	AM_RANGE(0x1e, 0x1f) AM_WRITE(system_w)
ADDRESS_MAP_END

k054338_device::k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054338, "K054338 Mixer", tag, owner, clock, "k054338", __FILE__)
{
	m_palette_tag = nullptr;
	m_palette = nullptr;
	memset(m_bitmaps, 0, sizeof(m_bitmaps));
}

k054338_device::~k054338_device()
{
	if(m_bitmaps[0])
		for(int i=0; i<4; i++)
			delete m_bitmaps[i];
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k054338_device::device_start()
{
	m_palette = siblingdevice<palette_device>(m_palette_tag);

	m_init_cb.bind_relative_to(*owner());
	m_update_cb.bind_relative_to(*owner());

	save_item(NAME(m_back));
	save_item(NAME(m_shadow));
	save_item(NAME(m_brightness));
	save_item(NAME(m_mix_level));
	save_item(NAME(m_mix_add));
	save_item(NAME(m_system));

	for(int i=-0x100; i<0x200; i++) {
		m_clip_shadow_table[i+0x100] = i < 0 ? 0 : i > 0xff ? 0xff : i;
		// Unclear whether that's correct, but that's the simplest hardware-wise
		m_through_shadow_table[i+0x100] = i & 0x100 ? i : ~i;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k054338_device::device_reset()
{
	m_back = 0x000000;
	memset(m_shadow, 0, sizeof(m_shadow));
	memset(m_brightness, 0, sizeof(m_brightness));
	memset(m_mix_level, 0, sizeof(m_mix_level));
	memset(m_mix_add, 0, sizeof(m_mix_add));
	m_system = 0x00;
}


WRITE16_MEMBER( k054338_device::backr_w)
{
	if(ACCESSING_BITS_0_7) {
		if((data & 0xff) != ((m_back >> 16) & 0xff))
			logerror("back_r %02x\n", data & 0xff);
		m_back = (m_back & 0x00ffff) | ((data & 0xff) << 16);
	}
}

WRITE16_MEMBER( k054338_device::backgb_w)
{
	if(ACCESSING_BITS_8_15) {
		if((data >> 8) != ((m_back >> 8) & 0xff))
			logerror("back_g %02x\n", data >> 8);
		m_back = (m_back & 0xff00ff) | (data & 0xff00);
	}
	if(ACCESSING_BITS_0_7) {
		if((data & 0xff) != (m_back & 0xff))
			logerror("back_b %02x\n", data & 0xff);
		m_back = (m_back & 0xffff00) | (data & 0xff);
	}
}

WRITE16_MEMBER( k054338_device::shadow2_w)
{
	shadow_w(space, 8, data, mem_mask);
}

WRITE16_MEMBER( k054338_device::shadow_w)
{
	uint32_t sentry = offset / 3;
	uint32_t scolor = offset % 3;
	int32_t old = m_shadow[sentry][scolor];
	int32_t shadow = (m_shadow[sentry][scolor] & ~mem_mask) | (data & mem_mask);
	shadow = shadow & 0x100 ? shadow | 0xffffff00 : shadow & 0x000000ff;
	m_shadow[sentry][scolor] = shadow;
	if(old != shadow)
		logerror("shadow %d%c %c%02x\n",
				 sentry+1,
				 "rgb"[scolor],
				 shadow < 0 ? '-' : ' ',
				 shadow < 0 ? -shadow : shadow);
}


WRITE16_MEMBER( k054338_device::bri1_w)
{
	if(ACCESSING_BITS_0_7) {
		if((data & 0xff) != m_brightness[0])
			logerror("brightness 1 %02x\n", data & 0xff);
		m_brightness[0] = data & 0xff;
	}
}

WRITE16_MEMBER( k054338_device::bri23_w)
{
	if(ACCESSING_BITS_8_15) {
		if((data >> 8) != m_brightness[1])
			logerror("brightness 2 %02x\n", data >> 8);
		m_brightness[1] = data >> 8;
	}
	if(ACCESSING_BITS_0_7) {
		if((data & 0xff) != m_brightness[2])
			logerror("brightness 3 %02x\n", data & 0xff);
		m_brightness[2] = data & 0xff;
	}
}

WRITE16_MEMBER( k054338_device::mix1_w)
{
	if(ACCESSING_BITS_0_7) {
		if((data & 0x3f) != (m_mix_level[0] | (m_mix_add[0] ? 0x20 : 0x00)))
			logerror("mix 1 %c%02x\n", data & 0x20 ? '+' : '~', data & 0x1f);
		m_mix_level[0] = data & 0x1f;
		m_mix_add[0] = data & 0x20;
	}
}

WRITE16_MEMBER( k054338_device::mix23_w)
{
	if(ACCESSING_BITS_8_15) {
		if(((data >> 8) & 0x3f) != (m_mix_level[1] | (m_mix_add[1] ? 0x20 : 0x00)))
			logerror("mix 2 %c%02x\n", data & 0x2000 ? '+' : '~', (data >> 8) & 0x1f);
		m_mix_level[1] = (data >> 8) & 0x1f;
		m_mix_add[1] = data & 0x2000;
	}
	if(ACCESSING_BITS_0_7) {
		if((data & 0x3f) != (m_mix_level[2] | (m_mix_add[2] ? 0x20 : 0x00)))
			logerror("mix 3 %c%02x\n", data & 0x20 ? '+' : '~', data & 0x1f);
		m_mix_level[2] = data & 0x1f;
		m_mix_add[2] = data & 0x20;
	}
}

WRITE16_MEMBER( k054338_device::system_w)
{
	if(ACCESSING_BITS_0_7) {
		if((data & 0xff) != m_system)
			logerror("clipsl=%s waitsl=%s brtpri=%c shdpri=%c mixpri=%c kill=%s\n", data & 0x20 ? "wrap" : "sat", data & 0x10 ? "normal" : "wait", data & 0x08 ? '1' : '2', data & 0x04 ? '1' : '2', data & 0x02 ? '1' : '2', data & 0x01 ? "normal" : "background");
		m_system = data & 0xff;
	}
}

void k054338_device::bitmap_update(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	if(!m_bitmaps[0] || m_bitmaps[0]->width() != bitmap->width() || m_bitmaps[0]->height() != bitmap->height()) {
		if(m_bitmaps[0])
			for(int i=0; i<4; i++)
				delete m_bitmaps[i];
		for(int i=0; i<4; i++)
			m_bitmaps[i] = new bitmap_ind16(bitmap->width(), bitmap->height());
		if(!m_init_cb.isnull())
			m_init_cb(m_bitmaps);
	}

	m_update_cb(m_bitmaps, cliprect);
	const uint8_t *shadow_wrap = (m_system & 0x20 ? m_through_shadow_table : m_clip_shadow_table) + 0x100;
	const uint32_t *pens = m_palette->pens();
	uint16_t mask = m_palette->entries() - 1;
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		const uint16_t *b0color = &m_bitmaps[BITMAP_FRONT_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *b0attr  = &m_bitmaps[BITMAP_FRONT_ATTRIBUTES]->pix16(y, cliprect.min_x);
		const uint16_t *b1color = &m_bitmaps[BITMAP_LAYER2_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *b1attr  = &m_bitmaps[BITMAP_LAYER2_ATTRIBUTES]->pix16(y, cliprect.min_x);
		uint32_t *dest = &bitmap->pix32(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			uint16_t b0c = *b0color++;
			uint16_t b0a = *b0attr++;
			uint16_t b1c = *b1color++;
			uint16_t b1a = *b1attr++;
			uint16_t ma  = m_system & 2 ? b1a : b0a;
			uint16_t sa  = m_system & 4 ? b1a : b0a;
			uint16_t ba  = m_system & 8 ? b1a : b0a;

			uint32_t col0 = b0a & 0x8000 ? pens[b0c & mask] : m_back;
			if(ma & 0x30) {
				uint32_t col1 = b1a & 0x8000 ? pens[b1c & mask] : m_back;
				int code = ((ma >> 4) & 3) - 1;
				uint32_t r, g, b;
				if(m_mix_add[code]) {
					if(m_system & 2) {
						r = ((col0 >> 16) & 0xff) + ((((col1 >> 16) & 0xff) * (32 - m_mix_level[code])) >> 5);
						g = ((col0 >>  8) & 0xff) + ((((col1 >>  8) & 0xff) * (32 - m_mix_level[code])) >> 5);
						b = ((col0 >>  0) & 0xff) + ((((col1 >>  0) & 0xff) * (32 - m_mix_level[code])) >> 5);
					} else {
						r = ((col1 >> 16) & 0xff) + ((((col0 >> 16) & 0xff) * (32 - m_mix_level[code])) >> 5);
						g = ((col1 >>  8) & 0xff) + ((((col0 >>  8) & 0xff) * (32 - m_mix_level[code])) >> 5);
						b = ((col1 >>  0) & 0xff) + ((((col0 >>  0) & 0xff) * (32 - m_mix_level[code])) >> 5);
					}
					if(r > 0xff)
						r = 0xff;
					if(g > 0xff)
						g = 0xff;
					if(b > 0xff)
						b = 0xff;
				} else {
					r = (((col0 >> 16) & 0xff) * (32 - m_mix_level[code]) + ((col1 >> 16) & 0xff) * m_mix_level[code]) >> 5;
					g = (((col0 >>  8) & 0xff) * (32 - m_mix_level[code]) + ((col1 >>  8) & 0xff) * m_mix_level[code]) >> 5;
					b = (((col0 >>  0) & 0xff) * (32 - m_mix_level[code]) + ((col1 >>  0) & 0xff) * m_mix_level[code]) >> 5;
				}
				col0 = (r << 16) | (g << 8) | b;
			}

			if(sa & 0x03) {
				int code = ((sa >> 0) & 3) - 1;
				int rr = ((col0 >> 16) & 0xff) + m_shadow[code][0];
				uint8_t r = shadow_wrap[rr];
				int gg = ((col0 >>  8) & 0xff) + m_shadow[code][1];
				uint8_t g = shadow_wrap[gg];
				int bb = ((col0 >>  0) & 0xff) + m_shadow[code][2];
				uint8_t b = shadow_wrap[bb];
				col0 = (r << 16) | (g << 8) | b;
			}

			if(ba & 0x0c) {
				int code = ((ba >> 2) & 3) - 1;
				uint32_t bri = m_brightness[code] + 1;
				uint8_t r = (((col0 >> 16) & 0xff) * bri) >> 8;
				uint8_t g = (((col0 >>  8) & 0xff) * bri) >> 8;
				uint8_t b = (((col0 >>  0) & 0xff) * bri) >> 8;
				col0 = (r << 16) | (g << 8) | b;
			}
			*dest++ = col0;
		}
	}
}

uint32_t k054338_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_update(&bitmap, cliprect);
	return 0;
}

