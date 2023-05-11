// license:BSD-3-Clause
// copyright-holders:Eric Anderson
#include "emu.h"

#include "sbcvideo.h"

#include "video/cgapal.h"

#include "screen.h"

vector_sbc_video_device::vector_sbc_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBC_VIDEO, tag, owner, clock)
	, m_io_sbc_video_conf(*this, "SBC_VIDEO_CONF")
	, m_buffer(*this, finder_base::DUMMY_TAG)
	, m_chrroml(*this, finder_base::DUMMY_TAG)
	, m_chrromr(*this, finder_base::DUMMY_TAG)
	, m_res320_ram{0}
{
}

void vector_sbc_video_device::spr_w(uint8_t data)
{
	m_spr = data;
}

void vector_sbc_video_device::res320_mapping_ram_w(offs_t offset, uint8_t data)
{
	// 7200-0001 page 209 (VI A-6) C6
	m_res320_ram[offset & 0x3] = data & 0x0f;
}

static inline rgb_t raw_4bit_to_rgb(uint8_t enc, bool color)
{
	if (color) {
		// 7200-0001 page 83. Ordering: IBGR
		enc = bitswap<4>(enc, 3, 0, 1, 2);
		return rgb_t(cga_palette[enc][0], cga_palette[enc][1], cga_palette[enc][2]);
	} else {
		// 7200-0001 page 209 C11
		uint8_t px = enc | (enc << 4);
		return rgb_t(0, px, 0);
	}
}

MC6845_UPDATE_ROW(vector_sbc_video_device::update_row)
{
	uint32_t monochrome_color = rgb_t(0x00, 0xff, 0x00);
	bool graph = BIT(m_spr, 1); // ALPHA|/GRAPH
	bool gry = BIT(m_spr, 2); // DIG|/GRY
	bool res320 = BIT(m_spr, 3); // 320/160|
	bool chr1 = BIT(m_spr, 4); // CHR0|/CHR1
	// Color is available only on an external/second monitor, via a CGA
	// connector. But the color monitor is not provided output for
	// monochrome modes (only if GRAPH and GRY). The same pixel value is used
	// by the builtin monitor and RGBI monitor, but they interpret the value
	// differently.
	bool color = (m_io_sbc_video_conf->read() & 0x1) == 0x1;
	// video never uses MA17. 7200-0001 209 B5
	uint32_t addr = BIT(ma, 0, 10); // U82 U100
	uint8_t jumper_c = (m_io_sbc_video_conf->read() >> 1) & 0x7;
	if (graph)
		addr |= (ra | (BIT(ma, 11, 3) << 4)) << 10; // U81
	else
		addr |= (BIT(ma, 10, 4) | (jumper_c << 4)) << 10; // U99
	addr <<= 1;

	// The display is always 312 scanlines.
	// The horizonal resolution is 1280 in alpha mode, 640 in high res mode
	// (monochrome), and 320 or 160 in gray/color scale mode. 320 and 160 modes
	// have the same color space, but the 320 mode uses a palette.
	// Buffer is 1920 bytes for Alpha mode, and 24960 bytes for Graphic mode.

	uint32_t *p = &bitmap.pix(y);

	if (graph) {
		if (gry) {
			// 7200-0001 page 83 (II 3-20)
			if (res320) {
				for (uint16_t x = 0; x < x_count*2; x++) {
					uint8_t cell = m_buffer->read(x+addr);
					rgb_t px1 = raw_4bit_to_rgb(m_res320_ram[BIT(cell, 0, 2)], color);
					for (int i = 0; i < 4; i++)
						*p++ = px1;
					rgb_t px2 = raw_4bit_to_rgb(m_res320_ram[BIT(cell, 2, 2)], color);
					for (int i = 0; i < 4; i++)
						*p++ = px2;
					rgb_t px3 = raw_4bit_to_rgb(m_res320_ram[BIT(cell, 4, 2)], color);
					for (int i = 0; i < 4; i++)
						*p++ = px3;
					rgb_t px4 = raw_4bit_to_rgb(m_res320_ram[BIT(cell, 6, 2)], color);
					for (int i = 0; i < 4; i++)
						*p++ = px4;
				}
			} else { // 160
				for (uint16_t x = 0; x < x_count*2; x++) {
					uint8_t cell = m_buffer->read(x+addr);
					rgb_t px1 = raw_4bit_to_rgb(BIT(cell, 0, 4), color);
					for (int i = 0; i < 8; i++)
						*p++ = px1;
					rgb_t px2 = raw_4bit_to_rgb(BIT(cell, 4, 4), color);
					for (int i = 0; i < 8; i++)
						*p++ = px2;
				}
			}
		} else { // DIG
			for (uint16_t x = 0; x < x_count*2; x++) {
				uint8_t cell = m_buffer->read(x+addr);
				for (int i = 0; i < 8; i++) {
					*p++ = BIT(cell, i) ? monochrome_color : 0;
					*p++ = BIT(cell, i) ? monochrome_color : 0;
				}
			}
		}
	} else { // ALPHA
		for (uint16_t x = 0; x < x_count*2; x++) {
			// Character Generators. 7200-0001 page 69 (II 3-6) C4
			uint8_t cell = m_buffer->read(x+addr);
			uint8_t chr = cell & 0x7f;
			uint8_t invert = BIT(cell, 7);
			uint8_t gfxl = m_chrroml[chr | (ra << 7) | (chr1 << 11)];
			uint8_t gfxr = m_chrromr[chr | (ra << 7) | (chr1 << 11)];

			if (!invert) {
				gfxl ^= 0xff;
				gfxr ^= 0xff;
			}

			// Alpha Mode Shift Registers. C3
			for (int i = 0; i < 8; i++)
				*p++ = BIT(gfxr, i) ? monochrome_color : 0;
			for (int i = 0; i < 8; i++)
				*p++ = BIT(gfxl, i) ? monochrome_color : 0;
		}
	}
}

void vector_sbc_video_device::device_add_mconfig(machine_config &config)
{
	// CPU|/VID used as a clock
	c6545_1_device &crtc(C6545_1(config, "crtc", DERIVED_CLOCK(1, 32)));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(16*2);
	crtc.set_update_row_callback(FUNC(vector_sbc_video_device::update_row));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(32'640'000, 1600, 0, 1280, 340, 0, 312);
	screen.set_screen_update(crtc, FUNC(c6545_1_device::screen_update));
}

void vector_sbc_video_device::device_start()
{
	save_item(NAME(m_spr));
	save_item(NAME(m_res320_ram));
}

void vector_sbc_video_device::device_reset()
{
	m_spr = 0;
}

INPUT_PORTS_START( sbc_video )
	PORT_START("SBC_VIDEO_CONF")
	PORT_CONFNAME(0x001, 0x000, "Color Monitor")
	PORT_CONFSETTING(0x000, DEF_STR(No))
	PORT_CONFSETTING(0x001, DEF_STR(Yes))

	PORT_CONFNAME(0x00e, 0x002, "Alpha Graphic Memory Block")
	PORT_CONFSETTING(0x000, "0x00000-0x07FFF")
	PORT_CONFSETTING(0x002, "0x08000-0x0FFFF")
	PORT_CONFSETTING(0x004, "0x10000-0x17FFF")
	PORT_CONFSETTING(0x006, "0x18000-0x1FFFF")
	PORT_CONFSETTING(0x008, "0x20000-0x27FFF")
	PORT_CONFSETTING(0x00a, "0x28000-0x2FFFF")
	PORT_CONFSETTING(0x00c, "0x30000-0x37FFF")
	PORT_CONFSETTING(0x00e, "0x38000-0x3FFFF")
INPUT_PORTS_END

ioport_constructor vector_sbc_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sbc_video);
}

DEFINE_DEVICE_TYPE(SBC_VIDEO, vector_sbc_video_device, "vectorsbcvideo", "Vector SBC Video Output")
