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
#include "includes/mac.h"
#include "machine/ram.h"

PALETTE_INIT_MEMBER(mac_state,mac)
{
	palette.set_pen_color(0, 0xff, 0xff, 0xff);
	palette.set_pen_color(1, 0x00, 0x00, 0x00);
}

// 4-level grayscale
PALETTE_INIT_MEMBER(mac_state,macgsc)
{
	palette.set_pen_color(0, 0xff, 0xff, 0xff);
	palette.set_pen_color(1, 0x7f, 0x7f, 0x7f);
	palette.set_pen_color(2, 0x3f, 0x3f, 0x3f);
	palette.set_pen_color(3, 0x00, 0x00, 0x00);
}

VIDEO_START_MEMBER(mac_state,mac)
{
}

#define MAC_MAIN_SCREEN_BUF_OFFSET  0x5900
#define MAC_ALT_SCREEN_BUF_OFFSET   0xD900

UINT32 mac_state::screen_update_mac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 video_base;
	const UINT16 *video_ram;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	video_base = m_ram->size() - (m_screen_buffer ? MAC_MAIN_SCREEN_BUF_OFFSET : MAC_ALT_SCREEN_BUF_OFFSET);
	video_ram = (const UINT16 *) (m_ram->pointer() + video_base);

	for (y = 0; y < MAC_V_VIS; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < MAC_H_VIS; x += 16)
		{
			word = *(video_ram++);
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

UINT32 mac_state::screen_update_macse30(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 video_base;
	const UINT16 *video_ram;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	video_base = m_screen_buffer ? 0x8000 : 0;
	video_base += (MAC_H_VIS/8);
	video_ram = (const UINT16 *) &m_vram[video_base/4];

	for (y = 0; y < MAC_V_VIS; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < MAC_H_VIS; x += 16)
		{
			word = video_ram[((y * MAC_H_VIS)/16) + ((x/16)^1)];
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

UINT32 mac_state::screen_update_macprtb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT16 *video_ram;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	video_ram = (const UINT16 *) m_vram16.target();

	for (y = 0; y < 400; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < 640; x += 16)
		{
			word = video_ram[((y * 640)/16) + ((x/16))];
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

UINT32 mac_state::screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT16 *video_ram;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	video_ram = (const UINT16 *) m_vram.target();

	for (y = 0; y < 400; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < 640; x += 16)
		{
			word = video_ram[((y * 640)/16) + ((x/16)^1)];
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

UINT32 mac_state::screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *line;
	int y, x;
	UINT8 pixels;
	UINT8 *vram8 = (UINT8 *)m_vram.target();

	for (y = 0; y < 400; y++)
	{
		line = &bitmap.pix16(y);

		for (x = 0; x < 640/4; x++)
		{
			pixels = vram8[(y * 160) + (BYTE4_XOR_BE(x))];

			*line++ = ((pixels>>6)&3);
			*line++ = ((pixels>>4)&3);
			*line++ = ((pixels>>2)&3);
			*line++ = (pixels&3);

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
	rectangle visarea;
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

	visarea.min_x = 0;
	visarea.min_y = 0;
	view = 0;
	if (m_montype)
	{
		m_rbv_montype = m_montype->read();
	}
	else
	{
		m_rbv_montype = 2;
	}
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			visarea.max_x = 640-1;
			visarea.max_y = 870-1;
			htotal = 832;
			vtotal = 918;
			framerate = 75.0;
			view = 1;
			break;

		case 2: // 12" RGB
			visarea.max_x = 512-1;
			visarea.max_y = 384-1;
			htotal = 640;
			vtotal = 407;
			framerate = 60.15;
			break;

		case 6: // 13" RGB
		default:
			visarea.max_x = 640-1;
			visarea.max_y = 480-1;
			htotal = 800;
			vtotal = 525;
			framerate = 59.94;
			break;
	}

//    printf("RBV reset: monitor is %dx%d @ %f Hz\n", visarea.max_x+1, visarea.max_y+1, framerate);
	machine().first_screen()->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(framerate));
	render_target *target = machine().render().first_target();
	target->set_view(view);
}

VIDEO_RESET_MEMBER(mac_state,macsonora)
{
	rectangle visarea;
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

	visarea.min_x = 0;
	visarea.min_y = 0;

	m_rbv_montype = m_montype ? m_montype->read() : 2;
	switch (m_rbv_montype)
	{
		case 1: // 15" portrait display
			visarea.max_x = 640-1;
			visarea.max_y = 870-1;
			htotal = 832;
			vtotal = 918;
			framerate = 75.0;
			view = 1;
			break;

		case 2: // 12" RGB
			visarea.max_x = 512-1;
			visarea.max_y = 384-1;
			htotal = 640;
			vtotal = 407;
			framerate = 60.15;
			break;

		case 6: // 13" RGB
		default:
			visarea.max_x = 640-1;
			visarea.max_y = 480-1;
			htotal = 800;
			vtotal = 525;
			framerate = 59.94;
			break;
	}

//    printf("Sonora reset: monitor is %dx%d @ %f Hz\n", visarea.max_x+1, visarea.max_y+1, framerate);
	machine().first_screen()->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(framerate));
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

UINT32 mac_state::screen_update_macrbv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y, hres, vres;
	UINT8 *vram8 = (UINT8 *)m_ram->pointer();

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
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres; x+=8)
				{
					pixels = vram8[(y * (hres/8)) + ((x/8)^3)];

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
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres/4; x++)
				{
					pixels = vram8[(y * (hres/4)) + (BYTE4_XOR_BE(x))];

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
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres/2; x++)
				{
					pixels = vram8[(y * (hres/2)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0xf0|(pixels>>4)];
					*scanline++ = m_rbv_palette[0xf0|(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres; x++)
				{
					pixels = vram8[(y * hres) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
	}

	return 0;
}

UINT32 mac_state::screen_update_macrbvvram(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y, hres, vres;

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres; x+=8)
				{
					pixels = vram8[(y * 0x400) + ((x/8)^3)];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres/4; x++)
				{
					pixels = vram8[(y * (hres/4)) + (BYTE4_XOR_BE(x))];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres/2; x++)
				{
					pixels = vram8[(y * (hres/2)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[0xf0|(pixels>>4)];
					*scanline++ = m_rbv_palette[0xf0|(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres; x++)
				{
					pixels = vram8[(y * 2048) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
	}

	return 0;
}

UINT32 mac_state::screen_update_macv8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y, hres, vres;

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres; x+=8)
				{
					pixels = vram8[(y * 1024) + ((x/8)^3)];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres/4; x++)
				{
					pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres/2; x++)
				{
					pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[(pixels&0xf0) | 0xf];
					*scanline++ = m_rbv_palette[((pixels&0x0f)<<4) | 0xf];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres; x++)
				{
					pixels = vram8[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;
	}

	return 0;
}

UINT32 mac_state::screen_update_macsonora(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y, hres, vres, stride;

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres; x+=8)
				{
					pixels = vram8[(y * (stride/8)) + ((x/8)^3)];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < hres/4; x++)
				{
					pixels = vram8[(y * (stride/4)) + (BYTE4_XOR_BE(x))];

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
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres/2; x++)
				{
					pixels = vram8[(y * (stride/2)) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[(pixels&0xf0) | 0xf];
					*scanline++ = m_rbv_palette[((pixels&0x0f)<<4) | 0xf];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres; x++)
				{
					pixels = vram8[(y * stride) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;

		case 4: // 16bpp
		{
			UINT16 *vram16 = (UINT16 *)m_vram.target();
			UINT16 pixels;

			for (y = 0; y < vres; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < hres; x++)
				{
					pixels = vram16[(y * stride) + (x^1)];
					*scanline++ = rgb_t(((pixels>>10) & 0x1f)<<3, ((pixels>>5) & 0x1f)<<3, (pixels & 0x1f)<<3);
				}
			}
		}
		break;
	}

	return 0;
}

// DAFB: video for Quadra 700/900

void mac_state::dafb_recalc_ints()
{
	if (m_dafb_int_status != 0)
	{
		nubus_slot_interrupt(0xf, ASSERT_LINE);
	}
	else
	{
		nubus_slot_interrupt(0xf, CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(mac_state::dafb_vbl_tick)
{
	m_dafb_int_status |= 1;
	dafb_recalc_ints();

	m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
}

TIMER_CALLBACK_MEMBER(mac_state::dafb_cursor_tick)
{
	m_dafb_int_status |= 4;
	dafb_recalc_ints();

	m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
}

VIDEO_START_MEMBER(mac_state,macdafb)
{
	m_vbl_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::dafb_vbl_tick),this));
	m_cursor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mac_state::dafb_cursor_tick),this));

	m_vbl_timer->adjust(attotime::never);
	m_cursor_timer->adjust(attotime::never);
}

VIDEO_RESET_MEMBER(mac_state,macdafb)
{
	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_montype = 6;
	m_rbv_vbltime = 0;
	m_dafb_int_status = 0;
	m_rbv_type = RBV_TYPE_DAFB;
	m_dafb_mode = 0;
	m_dafb_base = 0x1000;
	m_dafb_stride = 256*4;

	memset(m_rbv_palette, 0, sizeof(m_rbv_palette));
}

READ32_MEMBER(mac_state::dafb_r)
{
//  if (offset != 0x108/4) printf("DAFB: Read @ %x (mask %x PC=%x)\n", offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0x1c:  // inverse of monitor sense
			return 7;   // 21" color 2-page

		case 0x24: // SCSI 539x #1 status
			return m_dafb_scsi1_drq<<9;

		case 0x28: // SCSI 539x #2 status
			return m_dafb_scsi2_drq<<9;

		case 0x108: // IRQ/VBL status
			return m_dafb_int_status;

		case 0x10c: // clear cursor scanline int
			m_dafb_int_status &= ~4;
			dafb_recalc_ints();
			break;

		case 0x114: // clear VBL int
			m_dafb_int_status &= ~1;
			dafb_recalc_ints();
			break;
	}
	return 0;
}

WRITE32_MEMBER(mac_state::dafb_w)
{
//  if (offset != 0x10c/4) printf("DAFB: Write %08x @ %x (mask %x PC=%x)\n", data, offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0: // bits 20-9 of base
			m_dafb_base &= 0x1ff;
			m_dafb_base |= (data & 0xffff) << 9;
//          printf("DAFB baseH: %x\n", m_dafb_base);
			break;

		case 4: // bits 8-5 of base
			m_dafb_base &= ~0x1ff;
			m_dafb_base |= (data & 0xf) << 5;
//          printf("DAFB baseL: %x\n", m_dafb_base);
			break;

		case 8:
			m_dafb_stride = data<<2;    // stride in DWORDs
//          printf("DAFB stride: %x %x\n", m_dafb_stride, data);
			break;

		case 0x104:
			if (data & 1)   // VBL enable
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
			}
			else
			{
				m_vbl_timer->adjust(attotime::never);
				m_dafb_int_status &= ~1;
				dafb_recalc_ints();
			}

			if (data & 2)   // aux scanline interrupt enable
			{
				fatalerror("DAFB: Aux scanline interrupt enable not supported!\n");
			}

			if (data & 4)   // cursor scanline interrupt enable
			{
				m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
			}
			else
			{
				m_cursor_timer->adjust(attotime::never);
				m_dafb_int_status &= ~4;
				dafb_recalc_ints();
			}
			break;

		case 0x10c: // clear cursor scanline int
			m_dafb_int_status &= ~4;
			dafb_recalc_ints();
			break;

		case 0x114: // clear VBL int
			m_dafb_int_status &= ~1;
			dafb_recalc_ints();
			break;
	}
}

READ32_MEMBER(mac_state::dafb_dac_r)
{
//  printf("DAFB: Read DAC @ %x (mask %x PC=%x)\n", offset*4, mem_mask, m_maincpu->pc());

	return 0;
}

WRITE32_MEMBER(mac_state::dafb_dac_w)
{
//  if ((offset > 0) && (offset != 0x10/4)) printf("DAFB: Write %08x to DAC @ %x (mask %x PC=%x)\n", data, offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0:
			m_rbv_clutoffs = data & 0xff;
			m_rbv_count = 0;
			break;

		case 0x10:
			m_rbv_colors[m_rbv_count++] = data&0xff;

			if (m_rbv_count == 3)
			{
				m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]));
				m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
				m_rbv_clutoffs++;
				m_rbv_count = 0;
			}
			break;

		case 0x20:
			printf("%x to DAFB mode\n", data);
			switch (data & 0x9f)
			{
				case 0x80:
					m_dafb_mode = 0;    // 1bpp
					break;

				case 0x88:
					m_dafb_mode = 1;    // 2bpp
					break;

				case 0x90:
					m_dafb_mode = 2;    // 4bpp
					break;

				case 0x98:
					m_dafb_mode = 3;    // 8bpp
					break;

				case 0x9c:
					m_dafb_mode = 4;    // 24bpp
					break;
			}
			break;
	}
}

UINT32 mac_state::screen_update_macdafb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;

	switch (m_dafb_mode)
	{
		case 0: // 1bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;
			vram8 += m_dafb_base;

			for (y = 0; y < 870; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1152; x+=8)
				{
					pixels = vram8[(y * m_dafb_stride) + ((x/8)^3)];

					*scanline++ = m_rbv_palette[(pixels>>7)&1];
					*scanline++ = m_rbv_palette[(pixels>>6)&1];
					*scanline++ = m_rbv_palette[(pixels>>5)&1];
					*scanline++ = m_rbv_palette[(pixels>>4)&1];
					*scanline++ = m_rbv_palette[(pixels>>3)&1];
					*scanline++ = m_rbv_palette[(pixels>>2)&1];
					*scanline++ = m_rbv_palette[(pixels>>1)&1];
					*scanline++ = m_rbv_palette[(pixels&1)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;
			vram8 += m_dafb_base;

			for (y = 0; y < 870; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1152/4; x++)
				{
					pixels = vram8[(y * m_dafb_stride) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[((pixels>>6)&3)];
					*scanline++ = m_rbv_palette[((pixels>>4)&3)];
					*scanline++ = m_rbv_palette[((pixels>>2)&3)];
					*scanline++ = m_rbv_palette[(pixels&3)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;
			vram8 += m_dafb_base;

			for (y = 0; y < 870; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1152/2; x++)
				{
					pixels = vram8[(y * m_dafb_stride) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_rbv_palette[(pixels>>4)];
					*scanline++ = m_rbv_palette[(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			UINT8 *vram8 = (UINT8 *)m_vram.target();
			UINT8 pixels;
			vram8 += m_dafb_base;

			for (y = 0; y < 870; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1152; x++)
				{
					pixels = vram8[(y * m_dafb_stride) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_rbv_palette[pixels];
				}
			}
		}
		break;

		case 4: // 24 bpp
			for (y = 0; y < 480; y++)
			{
				UINT32 *base;

				scanline = &bitmap.pix32(y);
				base = (UINT32 *)&m_vram[(y * (m_dafb_stride/4)) + (m_dafb_base/4)];
				for (x = 0; x < 640; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}

	return 0;
}

UINT32 mac_state::screen_update_macpbwd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)/* Color PowerBooks using an off-the-shelf WD video chipset */
{
	UINT32 *scanline;
	int x, y;
	UINT8 *vram8 = (UINT8 *)m_vram.target();
	UINT8 pixels;

//    vram8 += 0x40000;

	for (y = 0; y < 480; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 640; x++)
		{
			pixels = vram8[(y * 640) + (BYTE4_XOR_BE(x))];
			*scanline++ = m_rbv_palette[pixels];
		}
	}

	return 0;
}

READ32_MEMBER(mac_state::macwd_r)
{
	switch (offset)
	{
		case 0xf6:
			if (m_screen->vblank())
			{
				return 0xffffffff;
			}
			else
			{
				return 0;
			}

		default:
//            printf("macwd_r: @ %x, mask %08x (PC=%x)\n", offset, mem_mask, m_maincpu->pc());
			break;
	}
	return 0;
}

WRITE32_MEMBER(mac_state::macwd_w)
{
	switch (offset)
	{
		case 0xf2:
			if (mem_mask == 0xff000000) // DAC control
			{
				m_rbv_clutoffs = data>>24;
				m_rbv_count = 0;
			}
			else if (mem_mask == 0x00ff0000)    // DAC data
			{
				m_rbv_colors[m_rbv_count++] = (data>>16)&0xff;
				if (m_rbv_count == 3)
				{
//                    printf("RAMDAC: color %d = %02x %02x %02x\n", m_rbv_clutoffs, m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
					m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
					m_rbv_clutoffs++;
					m_rbv_count = 0;
				}
			}
			else
			{
				printf("macwd: Unknown DAC write, data %08x, mask %08x\n", data, mem_mask);
			}
			break;

		default:
//            printf("macwd_w: %x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, m_maincpu->pc());
			break;
	}
}
