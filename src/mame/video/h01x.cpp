// license:BSD-3-Clause
/***************************************************************************
        NF500A (TRS80 Level II Basic)
        09/01/2019
****************************************************************************/

#include "emu.h"
#include "includes/h01x.h"

uint32_t h01x_state::screen_update_h01x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,gfx;
	uint16_t sy=0,ma=0,x;
	uint8_t skip = 16;

	//screen.set_visible_area(0, 336-1, 0, 192-1);

	// 336*192 = 84*4 * 12*16
	// 12*16*84

	// uint16_t *p = &bitmap.pix16(sy++);
	// _PixelType &pixt(int32_t y, int32_t x = 0) const { return *(reinterpret_cast<_PixelType *>(m_base) + y * m_rowpixels + x); }
	// set_raw(u32 pixclock, u16 htotal, u16 hbend, u16 hbstart, u16 vtotal, u16 vbend, u16 vbstart)
	// bitmap.rowpixels() == htotal
	// printf("%d ", bitmap.rowpixels());

	//uint16_t *p = &bitmap.pix16(0);
	for(ra=0; ra<12; ra++) {
		for(y=0; y<16; y++) {
			ma = ra*84*16+y;
			uint16_t *p = &bitmap.pix16(sy++);
			for(x=ma; x<ma+84*skip; x+=skip) {
				gfx = m_vram_ptr[x];
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
	}

	return 0;
}
