// license:BSD-3-Clause
/**********************************************************************

	Spectrum Next ULA

**********************************************************************/

#include "emu.h"
#include "specnext_ula.h"

#include "screen.h"


specnext_ula_device::specnext_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_ULA, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
{
}

void specnext_ula_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash)
{
	//m_host_ram_ptr + ((port_7ffd_shadow() ? 7 : 5) << 14);
	const u8 *screen_location = m_host_ram_ptr + (5 << 14); // TODO +shadow
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - m_offset_h;
		u16 y = vpos - m_offset_v;
		const u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		const u8 *attr = &screen_location[0x1800 + (((y & 0xf8) << 2) | (x >> 3))];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while (hpos <= cliprect.right())
		{
			u16 ink, pap;
			if (m_ulap_en)
			{
				ink = ((*attr >> 3) & 0x18) | (*attr & 0x07);
				pap = ((*attr >> 3) & 0x1f) | 0x20;
				//pap = UTM_FALLBACK_PEN;
				if (m_ulanext_en)
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

void specnext_ula_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
}

void specnext_ula_device::device_start()
{
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_ulanext_en));
	save_item(NAME(m_ulanext_format));
	save_item(NAME(m_ulap_en));
	save_item(NAME(m_ula_clip_x1));
	save_item(NAME(m_ula_clip_x2));
	save_item(NAME(m_ula_clip_y1));
	save_item(NAME(m_ula_clip_y2));
	save_item(NAME(m_ula_scroll_x));
	save_item(NAME(m_ula_scroll_y));
	save_item(NAME(m_ula_fine_scroll_x));
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_ULA, specnext_ula_device, "ula", "Spectrum Next ULA")
