// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/***************************************************************************

    video/mac.c

    Macintosh video hardware

    Emulates the video hardware for compact Macintosh series (original
    Macintosh (128k, 512k, 512ke), Macintosh Plus, Macintosh SE, Macintosh
    Classic)

    Also emulates on-board video for systems with the
    RBV, V8, Eagle, Sonora, and DAFB chips.

    ----------------------------------------------------------------------
    Monitor sense codes

    Apple assigns 3 pins for monitor IDs.  These allow 8 possible codes:

    000 - color 2-Page Display (21")
    001 - monochrome Full Page display (15")
    010 - color 512x384 (12")
    011 - monochrome 2 Page display (21")
    100 - NTSC
    101 - color Full Page display (15")
    110 - High-Resolution Color (13" 640x480) or use "type 6" extended codes
    111 - No monitor connected or use "type 7" extended codes

    For extended codes, you drive one of the 3 pins at a time and read the 2
    undriven pins.  See http://support.apple.com/kb/TA21618?viewlocale=en_US
    for details.

Extended codes:

                    Sense 2 Low  Sense 1 Low   Sense 0 Low
                    1 & 0        2 & 0         2 & 1

Multiple Scan 14"    00           00           11
Multiple Scan 16"    00           10           11
Multiple Scan 21"    10           00           11
PAL Encoder          00           00           00
NTSC Encoder         01           01           00
VGA/Super VGA        01           01           11
RGB 16"              10           11           01
PAL Monitor          11           00           00
RGB 19"              11           10           10
Radius color TPD     11           00           01   (TPD = Two Page Display)
Radius mono TPD      11           01           00
Apple TPD            11           01           01
Apple color FPD      01           11           10   (FPD = Full Page Display)

***************************************************************************/


#include "emu.h"
#include "sound/asc.h"
#include "mac.h"
#include "machine/ram.h"
#include "render.h"

VIDEO_START_MEMBER(mac_state,mac)
{
}

uint32_t mac_state::screen_update_macse30(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t const video_base = (m_screen_buffer ? 0x8000 : 0) + (MAC_H_VIS/8);
	uint16_t const *const video_ram = (const uint16_t *) &m_vram[video_base/4];

	for (int y = 0; y < MAC_V_VIS; y++)
	{
		uint16_t *const line = &bitmap.pix(y);

		for (int x = 0; x < MAC_H_VIS; x += 16)
		{
			uint16_t const word = video_ram[((y * MAC_H_VIS)/16) + ((x/16)^1)];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

// IIci/IIsi RAM-Based Video (RBV) and children: V8, Eagle, Spice, VASP, Sonora

VIDEO_START_MEMBER(mac_state,macrbv)
{
}

VIDEO_RESET_MEMBER(mac_state,maceagle)
{
	m_rbv_montype = 32;
	m_rbv_palette[0xfe] = 0xffffff;
	m_rbv_palette[0xff] = 0;
}

VIDEO_RESET_MEMBER(mac_state,macrbv)
{
	int htotal, vtotal;
	double framerate;
	int view;

	memset(m_rbv_regs, 0, sizeof(m_rbv_regs));

	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_immed10wr = 0;

	m_rbv_regs[2] = 0x7f;
	m_rbv_regs[3] = 0;

	m_rbv_type = RBV_TYPE_RBV;

	view = 0;

	m_rbv_montype = m_montype.read_safe(2);
	rectangle visarea;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			visarea.set(0, 640-1, 0, 870-1);
			htotal = 832;
			vtotal = 918;
			framerate = 75.0;
			view = 1;
			break;

		case 2: // 12" RGB
			visarea.set(0, 512-1, 0, 384-1);
			htotal = 640;
			vtotal = 407;
			framerate = 60.15;
			break;

		case 6: // 13" RGB
		default:
			visarea.set(0, 640-1, 0, 480-1);
			htotal = 800;
			vtotal = 525;
			framerate = 59.94;
			break;
	}

//    logerror("RBV reset: monitor is %dx%d @ %f Hz\n", visarea.width(), visarea.height(), framerate);
	m_screen->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(framerate));
	render_target *target = machine().render().first_target();
	target->set_view(view);
}

VIDEO_RESET_MEMBER(mac_state,macsonora)
{
	int htotal, vtotal;
	double framerate;
	int view = 0;

	memset(m_rbv_regs, 0, sizeof(m_rbv_regs));

	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_immed10wr = 0;

	m_rbv_regs[2] = 0x7f;
	m_rbv_regs[3] = 0;

	m_rbv_type = RBV_TYPE_SONORA;

	m_rbv_montype = m_montype.read_safe(2);
	rectangle visarea;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			visarea.set(0, 640-1, 0, 870-1);
			htotal = 832;
			vtotal = 918;
			framerate = 75.0;
			view = 1;
			break;

		case 2: // 12" RGB
			visarea.set(0, 512-1, 0, 384-1);
			htotal = 640;
			vtotal = 407;
			framerate = 60.15;
			break;

		case 6: // 13" RGB
		default:
			visarea.set(0, 640-1, 0, 480-1);
			htotal = 800;
			vtotal = 525;
			framerate = 59.94;
			break;
	}

//    logerror("Sonora reset: monitor is %dx%d @ %f Hz\n", visarea.width(), visarea.height(), framerate);
	m_screen->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(framerate));
	render_target *target = machine().render().first_target();
	target->set_view(view);
}

VIDEO_START_MEMBER(mac_state,macsonora)
{
	memset(m_rbv_regs, 0, sizeof(m_rbv_regs));

	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_immed10wr = 0;

	m_rbv_regs[2] = 0x7f;
	m_rbv_regs[3] = 0;
	m_rbv_regs[4] = 0x6;
	m_rbv_regs[5] = 0x3;

	m_sonora_vctl[0] = 0x9f;
	m_sonora_vctl[1] = 0;
	m_sonora_vctl[2] = 0;

	m_rbv_type = RBV_TYPE_SONORA;
}

VIDEO_START_MEMBER(mac_state,macv8)
{
	memset(m_rbv_regs, 0, sizeof(m_rbv_regs));

	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_immed10wr = 0;

	m_rbv_regs[0] = 0x4f;
	m_rbv_regs[1] = 0x06;
	m_rbv_regs[2] = 0x7f;

	m_rbv_type = RBV_TYPE_V8;
}

uint32_t mac_state::screen_update_macrbv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *vram8 = (uint8_t *)m_ram->pointer();
	int hres, vres;

	switch (m_rbv_montype)
	{
		case 32: // classic II built-in display
			hres = MAC_H_VIS;
			vres = MAC_V_VIS;
			vram8 += 0x1f9a80;  // Classic II apparently doesn't use VRAM?
			break;

		case 1: // 15" portrait display
			hres = 640;
			vres = 870;
			break;

		case 2: // 12" RGB
			hres = 512;
			vres = 384;
			break;

		case 6: // 13" RGB
		default:
			hres = 640;
			vres = 480;
			break;
	}

	switch (m_rbv_regs[0x10] & 7)
	{
		case 0: // 1bpp
		{
			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x+=8)
				{
					uint8_t const pixels = vram8[(y * (hres/8)) + ((x/8)^3)];

					*scanline++ = m_rbv_palette[0xfe|(pixels>>7)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>6)&1)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>5)&1)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>4)&1)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>3)&1)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>2)&1)];
					*scanline++ = m_rbv_palette[0xfe|((pixels>>1)&1)];
					*scanline++ = m_rbv_palette[0xfe|(pixels&1)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres/4; x++)
				{
					uint8_t const pixels = vram8[(y * (hres/4)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0xfc|((pixels>>6)&3)];
					*scanline++ = m_rbv_palette[0xfc|((pixels>>4)&3)];
					*scanline++ = m_rbv_palette[0xfc|((pixels>>2)&3)];
					*scanline++ = m_rbv_palette[0xfc|(pixels&3)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres/2; x++)
				{
					uint8_t const pixels = vram8[(y * (hres/2)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0xf0|(pixels>>4)];
					*scanline++ = m_rbv_palette[0xf0|(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres; x++)
				{
					uint8_t const pixels = vram8[(y * hres) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
	}

	return 0;
}

uint32_t mac_state::screen_update_macrbvvram(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			hres = 640;
			vres = 870;
			break;

		case 2: // 12" RGB
			hres = 512;
			vres = 384;
			break;

		case 6: // 13" RGB
		default:
			hres = 640;
			vres = 480;
			break;
	}

	switch (m_rbv_regs[0x10] & 7)
	{
		case 0: // 1bpp
		{
			auto const vram8 = util::big_endian_cast<uint8_t const>(m_vram.target());

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x+=8)
				{
					uint8_t const pixels = vram8[(y * 2048) + (x / 8)];

					*scanline++ = m_rbv_palette[0x7f|(pixels&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<1)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<2)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<3)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<4)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<5)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<6)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<7)&0x80)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			auto const vram8 = util::big_endian_cast<uint8_t const>(m_vram.target());

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres/4; x++)
				{
					uint8_t const pixels = vram8[(y * 2048) + x];

					*scanline++ = m_rbv_palette[0x3f|(pixels&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<2)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<4)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<6)&0xc0)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			auto const vram8 = util::big_endian_cast<uint8_t const>(m_vram.target());

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres/2; x++)
				{
					uint8_t const pixels = vram8[(y * 2048) + x];

					*scanline++ = m_rbv_palette[0x0f|(pixels&0xf0)];
					*scanline++ = m_rbv_palette[0x0f|((pixels<<4)&0xf0)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			auto const vram8 = util::big_endian_cast<uint8_t const>(m_vram.target());

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres; x++)
				{
					uint8_t const pixels = vram8[(y * 2048) + x];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;

		case 4: // 16bpp
		{
			auto const vram16 = util::big_endian_cast<uint16_t const>(m_vram.target());

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x++)
				{
					uint16_t const pixels = vram16[(y * 1024) + x];
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
		}
		break;
	}

	return 0;
}

uint32_t mac_state::screen_update_macv8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			hres = 640;
			vres = 870;
			break;

		case 2: // 12" RGB
			hres = 512;
			vres = 384;
			break;

		case 6: // 13" RGB
		default:
			hres = 640;
			vres = 480;
			break;
	}

	switch (m_rbv_regs[0x10] & 7)
	{
		case 0: // 1bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x+=8)
				{
					uint8_t const pixels = vram8[(y * 1024) + ((x/8)^3)];

					*scanline++ = m_rbv_palette[0x7f|(pixels&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<1)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<2)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<3)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<4)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<5)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<6)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<7)&0x80)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres/4; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0x3f|(pixels&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<2)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<4)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<6)&0xc0)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres/2; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[(pixels&0xf0) | 0xf];
					*scanline++ = m_rbv_palette[((pixels&0x0f)<<4) | 0xf];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);

				for (int x = 0; x < hres; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;
	}

	return 0;
}

uint32_t mac_state::screen_update_macsonora(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres, stride;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			stride = hres = 640;
			vres = 870;
			break;

		case 2: // 12" RGB
			stride = hres = 512;
			vres = 384;
			break;

		case 6: // 13" RGB
		default:
			stride = hres = 640;
			vres = 480;
			break;
	}

	// forced blank?
	if (m_sonora_vctl[0] & 0x80)
	{
		return 0;
	}

	switch (m_sonora_vctl[1] & 7)
	{
		case 0: // 1bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x+=8)
				{
					uint8_t const pixels = vram8[(y * (stride/8)) + ((x/8)^3)];

					*scanline++ = m_rbv_palette[0x7f|(pixels&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<1)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<2)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<3)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<4)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<5)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<6)&0x80)];
					*scanline++ = m_rbv_palette[0x7f|((pixels<<7)&0x80)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres/4; x++)
				{
					uint8_t const pixels = vram8[(y * (stride/4)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0x3f|(pixels&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<2)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<4)&0xc0)];
					*scanline++ = m_rbv_palette[0x3f|((pixels<<6)&0xc0)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres/2; x++)
				{
					uint8_t const pixels = vram8[(y * (stride/2)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[(pixels&0xf0) | 0xf];
					*scanline++ = m_rbv_palette[((pixels&0x0f)<<4) | 0xf];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			uint8_t const *const vram8 = (uint8_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x++)
				{
					uint8_t const pixels = vram8[(y * stride) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;

		case 4: // 16bpp
		{
			uint16_t const *const vram16 = (uint16_t *)m_vram.target();

			for (int y = 0; y < vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < hres; x++)
				{
					uint16_t const pixels = vram16[(y * stride) + (x^1)];
					*scanline++ = rgb_t(((pixels>>10) & 0x1f)<<3, ((pixels>>5) & 0x1f)<<3, (pixels & 0x1f)<<3);
				}
			}
		}
		break;
	}

	return 0;
}

