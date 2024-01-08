// license:BSD-3-Clause
/**********************************************************************

    Spectrum Next ULA

**********************************************************************/

#include "emu.h"
#include "screen.h"

#include "specnext_ula.h"

specnext_ula_device::specnext_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_ULA, tag, owner, clock)
    , device_gfx_interface(mconfig, *this, nullptr, ":palette")
    , m_ram(*this, "^ram")
{
}

void specnext_ula_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash)
{
	//m_ram->pointer() + 0x40000 + ((port_7ffd_shadow() ? 7 : 5) << 14);
    const u8 *screen_location = m_ram->pointer() + 0x40000 + (5 << 14);
    for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - m_offset.first;
		u16 y = vpos - m_offset.second;
		const u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		const u8 *attr = &screen_location[0x1800 + (((y & 0xf8) << 2) | (x >> 3))];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while (hpos <= cliprect.right())
		{
			u16 ink, pap;
			if (m_ports.ulap_en)
			{
				ink = ((*attr >> 3) & 0x18) | (*attr & 0x07);
				pap = ((*attr >> 3) & 0x1f) | 0x20;
				//pap = UTM_FALLBACK_PEN;
				if (m_ports.ulanext_en)
					;
			}
			else
			{
				ink = ((*attr >> 3) & 0x08) | (*attr & 0x07);
				pap = ((*attr >> 3) & 0x0f)/* | 0x10*/;
			}

			const u8 pix8 = (flash && (*attr & 0x80)) ? ~*scr : *scr;
			for (u8 b = (0x80 >> (x & 7)); b; b >>= 1, x++, hpos++, pix++)
			{
				const u16 pen = (pix8 & b) ? ink : pap;
				if (palette().pen_color(pen) != m_global_transparent)
					*pix = pen;
			}
			scr++;
			attr++;
		}
	}
}

void specnext_ula_device::device_start()
{
    //save_item(NAME(m_offset));
    /*
    save_item(NAME(m_ula_clip_x1));
    save_item(NAME(m_ula_clip_x2));
    save_item(NAME(m_ula_clip_y1));
    save_item(NAME(m_ula_clip_y2));
    */
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_ULA, specnext_ula_device, "ula", "Spectrum Next ULA")
