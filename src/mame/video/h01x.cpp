// license:BSD-3-Clause
// copyright-holders:zzemu-cn
/***************************************************************************
        NF500A (TRS80 Level II Basic)
        09/01/2019
****************************************************************************/

#include "emu.h"
#include "includes/h01x.h"

uint32_t h01x_state::screen_update_h01x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0;
	uint8_t const skip = 16;

	//screen.set_visible_area(0, 336-1, 0, 192-1);

	// 336*192 = 84*4 * 12*16
	// 12*16*84

	// uint16_t *p = &bitmap.pix(sy++);
	// _PixelType &pixt(int32_t y, int32_t x = 0) const { return *(reinterpret_cast<_PixelType *>(m_base) + y * m_rowpixels + x); }
	// set_raw(u32 pixclock, u16 htotal, u16 hbend, u16 hbstart, u16 vtotal, u16 vbend, u16 vbstart)
	// bitmap.rowpixels() == htotal
	// printf("%d ", bitmap.rowpixels());

	//uint16_t *p = &bitmap.pix(0);
	for(uint8_t ra=0; ra<12; ra++) {
		for(uint8_t y=0; y<16; y++) {
			uint16_t const ma = ra*84*16+y;
			uint16_t *p = &bitmap.pix(sy++);
			for(uint16_t x=ma; x<ma+84*skip; x+=skip) {
				uint8_t const gfx = m_vram_ptr[x];
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
	}

	return 0;
}
