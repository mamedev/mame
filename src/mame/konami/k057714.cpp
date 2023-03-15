// license:BSD-3-Clause
// copyright-holders:Ville Linde

// Konami 0000057714 "GCU" 2D Graphics Chip

#include "emu.h"
#include "k057714.h"
#include "screen.h"


#define DUMP_VRAM 0

#define LOG_GENERAL  (1 << 0)
#define LOG_REGISTER (1 << 1)
#define LOG_FIFO     (1 << 2)
#define LOG_CMDEXEC  (1 << 3)
#define LOG_DRAW     (1 << 4)
// #define VERBOSE      (LOG_GENERAL | LOG_REGISTER | LOG_FIFO | LOG_CMDEXEC | LOG_DRAW)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGREGISTER(...) LOGMASKED(LOG_REGISTER, __VA_ARGS__)
#define LOGFIFO(...) LOGMASKED(LOG_FIFO, __VA_ARGS__)
#define LOGCMDEXEC(...) LOGMASKED(LOG_CMDEXEC, __VA_ARGS__)
#define LOGDRAW(...) LOGMASKED(LOG_DRAW, __VA_ARGS__)

DEFINE_DEVICE_TYPE(K057714, k057714_device, "k057714", "k057714_device GCU")

k057714_device::k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K057714, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_irq(*this)
{
}

void k057714_device::device_start()
{
	m_irq.resolve_safe();

	m_vram = std::make_unique<uint32_t[]>(VRAM_SIZE/4);

	save_pointer(NAME(m_vram), VRAM_SIZE/4);
	save_item(NAME(m_vram_read_addr));
	save_item(NAME(m_vram_fifo0_addr));
	save_item(NAME(m_vram_fifo1_addr));
	save_item(NAME(m_vram_fifo0_mode));
	save_item(NAME(m_vram_fifo1_mode));
	save_item(NAME(m_command_fifo0));
	save_item(NAME(m_command_fifo0_ptr));
	save_item(NAME(m_command_fifo1));
	save_item(NAME(m_command_fifo1_ptr));
	save_item(NAME(m_ext_fifo_addr));
	save_item(NAME(m_ext_fifo_count));
	save_item(NAME(m_ext_fifo_line));
	save_item(NAME(m_ext_fifo_num_lines));
	save_item(NAME(m_ext_fifo_width));
	save_item(STRUCT_MEMBER(m_frame, base));
	save_item(STRUCT_MEMBER(m_frame, width));
	save_item(STRUCT_MEMBER(m_frame, height));
	save_item(STRUCT_MEMBER(m_frame, x));
	save_item(STRUCT_MEMBER(m_frame, y));
	save_item(STRUCT_MEMBER(m_frame, alpha));
	save_item(NAME(m_fb_origin_x));
	save_item(NAME(m_fb_origin_y));
	save_item(NAME(m_layer_select));
	save_item(NAME(m_reg_6c));
	save_item(NAME(m_display_h_visarea));
	save_item(NAME(m_display_h_frontporch));
	save_item(NAME(m_display_h_backporch));
	save_item(NAME(m_display_h_syncpulse));
	save_item(NAME(m_display_v_visarea));
	save_item(NAME(m_display_v_frontporch));
	save_item(NAME(m_display_v_backporch));
	save_item(NAME(m_display_v_syncpulse));
	save_item(NAME(m_pixclock));
}

void k057714_device::device_reset()
{
	// Default display width/height are a guess.
	// All Firebeat games except beatmania III, which uses 640x480, will set the
	// display width/height through registers.
	// The assumption here is that since beatmania III doesn't set the display width/height
	// then the game is assuming that it's already at the correct settings upon boot.

	// Timing information taken from table found in all Firebeat games.
	// table idx (h vis area, front porch, sync pulse, back porch, h total) (v vis area, front porch, sync pulse, back porch, v total)
	// 0 (640, 16, 96, 48 = 800) (480, 10, 2, 33 = 525)
	// 1 (512, 5, 96, 72 = 685) (384, 6, 4, 22 = 416)
	// 2 (800, 40, 128, 88 = 1056) (600, 1, 4, 23 = 628)
	// 3 (640, 20, 23, 165 = 848) (384, 6, 1, 27 = 418)
	// 4 (640, 10, 21, 10 = 681) (480, 10, 2, 33 = 525)
	m_display_h_visarea = 640;
	m_display_h_frontporch = 16;
	m_display_h_backporch = 48;
	m_display_h_syncpulse = 96;
	m_display_v_visarea = 480;
	m_display_v_frontporch = 10;
	m_display_v_backporch = 33;
	m_display_v_syncpulse = 2;
	m_pixclock = 25'175'000; // 25.175_MHz_XTAL, default for Firebeat but maybe not other machiness. The value can be changed externally
	crtc_set_screen_params();

	m_vram_read_addr = 0;
	m_command_fifo0_ptr = 0;
	m_command_fifo1_ptr = 0;
	m_vram_fifo0_addr = 0;
	m_vram_fifo1_addr = 0;

	m_fb_origin_x = 0;
	m_fb_origin_y = 0;

	m_reg_6c = 0;

	for (auto & elem : m_frame)
	{
		elem.base = 0;
		elem.width = 0;
		elem.height = 0;
		elem.alpha = (16 << 7) | (16 << 2); // Set alpha 1 and 2 to 16 (100%) and blend mode to 0
	}

	memset(m_vram.get(), 0, VRAM_SIZE);
}

void k057714_device::device_stop()
{
#if DUMP_VRAM
	char filename[200];
	sprintf(filename, "%s_vram.bin", basetag());
	printf("dumping %s\n", filename);
	FILE *file = fopen(filename, "wb");
	int i;

	for (i=0; i < VRAM_SIZE/4; i++)
	{
		fputc((m_vram[i] >> 24) & 0xff, file);
		fputc((m_vram[i] >> 16) & 0xff, file);
		fputc((m_vram[i] >> 8) & 0xff, file);
		fputc((m_vram[i] >> 0) & 0xff, file);
	}

	fclose(file);
#endif
}

void k057714_device::set_pixclock(const XTAL &xtal)
{
	xtal.validate(std::string("Setting pixel clock for ") + tag());
	m_pixclock = xtal.value();
	crtc_set_screen_params();
}

inline void k057714_device::crtc_set_screen_params()
{
	auto htotal = m_display_h_visarea + m_display_h_frontporch + m_display_h_backporch + m_display_h_syncpulse;
	auto vtotal = m_display_v_visarea + m_display_v_frontporch + m_display_v_backporch + m_display_v_syncpulse;

	rectangle visarea(0, m_display_h_visarea - 1, 0, m_display_v_visarea - 1);
	screen().configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(m_pixclock) * htotal * vtotal);
}

uint32_t k057714_device::read(offs_t offset)
{
	int reg = offset * 4;

	// VRAM Read
	if (reg >= 0x80 && reg < 0x100)
	{
		return m_vram[m_vram_read_addr + offset - 0x20];
	}

	switch (reg)
	{
		case 0x78:      // GCU Status
			/* ppd checks bits 0x0041 of the upper halfword on interrupt */
			return 0xffff0005;

		default:
			break;
	}

	return 0xffffffff;
}

void k057714_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int reg = offset * 4;

	switch (reg)
	{
		case 0x00:
			if (ACCESSING_BITS_16_31)
			{
				m_display_h_visarea = ((data >> 16) & 0xffff) + 1;
			}
			if (ACCESSING_BITS_0_15)
			{
				m_display_h_frontporch = ((data >> 8) & 0xff) + 1;
				m_display_h_backporch = (data & 0xff) + 1;
			}
			crtc_set_screen_params();
			break;

		case 0x04:
			if (ACCESSING_BITS_16_31)
			{
				m_display_v_visarea = ((data >> 16) & 0xffff) + 1;
			}
			if (ACCESSING_BITS_0_15)
			{
				m_display_v_frontporch = ((data >> 8) & 0xff) + 1;
				m_display_v_backporch = (data & 0xff) + 1;
			}
			crtc_set_screen_params();
			break;

		case 0x08:
			if (ACCESSING_BITS_16_31)
			{
				m_display_h_syncpulse = ((data >> 24) & 0xff) + 1;
				m_display_v_syncpulse = ((data >> 16) & 0xff) + 1;
				crtc_set_screen_params();
			}
			break;

		case 0x10:
			/* IRQ clear/enable; ppd writes bit off then on in response to interrupt */
			/* it enables bits 0x41, but 0x01 seems to be the one it cares about */
			if (ACCESSING_BITS_16_31 && (data & 0x00010000) == 0)
			{
				if (!m_irq.isnull())
				{
					m_irq(CLEAR_LINE);
				}
			}
			if (ACCESSING_BITS_0_15)
			{
				m_layer_select = data;
				LOGREGISTER("%s_w: %02X, %08X, %08X\n", basetag(), reg, data, mem_mask);
			}
			break;

		case 0x14:      // Framebuffer 0/1 alpha values
			if (ACCESSING_BITS_16_31)
				m_frame[0].alpha = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[1].alpha = data & 0xffff;
			break;

		case 0x18:      // Framebuffer 0/1 alpha values
			if (ACCESSING_BITS_16_31)
				m_frame[2].alpha = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[3].alpha = data & 0xffff;
			break;

		case 0x1c:      // set to 1 on "media bus" access
			if ((data >> 16) == 1)
			{
				m_ext_fifo_count = 0;
				m_ext_fifo_line = 0;
			}
			break;

		case 0x20:      // Framebuffer 0 Origin(?)
			if (ACCESSING_BITS_16_31)
				m_frame[0].y = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[0].x = data & 0xffff;
			break;

		case 0x24:      // Framebuffer 1 Origin(?)
			if (ACCESSING_BITS_16_31)
				m_frame[1].y = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[1].x = data & 0xffff;
			break;

		case 0x28:      // Framebuffer 2 Origin(?)
			if (ACCESSING_BITS_16_31)
				m_frame[2].y = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[2].x = data & 0xffff;
			break;

		case 0x2c:      // Framebuffer 3 Origin(?)
			if (ACCESSING_BITS_16_31)
				m_frame[3].y = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[3].x = data & 0xffff;
			break;

		case 0x30:      // Framebuffer 0 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[0].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[0].width = data & 0xffff;
			LOGREGISTER("%s FB0 Dimensions: W %04X, H %04X\n", basetag(), data & 0xffff, (data >> 16) & 0xffff);
			break;

		case 0x34:      // Framebuffer 1 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[1].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[1].width = data & 0xffff;
			LOGREGISTER("%s FB1 Dimensions: W %04X, H %04X\n", basetag(), data & 0xffff, (data >> 16) & 0xffff);
			break;

		case 0x38:      // Framebuffer 2 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[2].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[2].width = data & 0xffff;
			LOGREGISTER("%s FB2 Dimensions: W %04X, H %04X\n", basetag(), data & 0xffff, (data >> 16) & 0xffff);
			break;

		case 0x3c:      // Framebuffer 3 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[3].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[3].width = data & 0xffff;
			LOGREGISTER("%s FB3 Dimensions: W %04X, H %04X\n", basetag(), data & 0xffff, (data >> 16) & 0xffff);
			break;

		case 0x40:      // Framebuffer 0 Base
			m_frame[0].base = data;
			LOGREGISTER("%s FB0 Base: %08X\n", basetag(), data);
			break;

		case 0x44:      // Framebuffer 1 Base
			m_frame[1].base = data;
			LOGREGISTER("%s FB1 Base: %08X\n", basetag(), data);
			break;

		case 0x48:      // Framebuffer 2 Base
			m_frame[2].base = data;
			LOGREGISTER("%s FB2 Base: %08X\n", basetag(), data);
			break;

		case 0x4c:      // Framebuffer 3 Base
			m_frame[3].base = data;
			LOGREGISTER("%s FB3 Base: %08X\n", basetag(), data);
			break;

		case 0x54:
			if (ACCESSING_BITS_16_31)
				m_ext_fifo_num_lines = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_ext_fifo_width = data & 0xffff;
			break;

		case 0x58:
			m_ext_fifo_addr = (data & 0xffffff);
			break;

		case 0x5c:      // VRAM Read Address
			m_vram_read_addr = (data & 0xffffff) / 2;
			break;

		case 0x60:      // VRAM Port 0 Write Address
			m_vram_fifo0_addr = (data & 0xffffff) / 2;
			break;

		case 0x68:      // VRAM Port 0/1 Mode
			if (ACCESSING_BITS_16_31)
				m_vram_fifo0_mode = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_vram_fifo1_mode = data & 0xffff;
			break;

		case 0x70:      // VRAM Port 0 Write FIFO
			if (m_vram_fifo0_mode & 0x100)
			{
				// write to command fifo
				m_command_fifo0[m_command_fifo0_ptr] = data;
				m_command_fifo0_ptr++;

				// execute when filled
				if (m_command_fifo0_ptr >= 4)
				{
					LOGFIFO("GCU FIFO0 exec: %08X %08X %08X %08X\n", m_command_fifo0[0], m_command_fifo0[1], m_command_fifo0[2], m_command_fifo0[3]);
					execute_command(m_command_fifo0);
					m_command_fifo0_ptr = 0;
				}
			}
			else
			{
				// write to VRAM fifo
				m_vram[m_vram_fifo0_addr] = data;
				m_vram_fifo0_addr++;
			}
			break;

		case 0x64:      // VRAM Port 1 Write Address
			m_vram_fifo1_addr = (data & 0xffffff) / 2;
			break;

		case 0x74:      // VRAM Port 1 Write FIFO
			if (m_vram_fifo1_mode & 0x100)
			{
				// write to command fifo
				m_command_fifo1[m_command_fifo1_ptr] = data;
				m_command_fifo1_ptr++;

				// execute when filled
				if (m_command_fifo1_ptr >= 4)
				{
					LOGFIFO("GCU FIFO1 exec: %08X %08X %08X %08X\n", m_command_fifo1[0], m_command_fifo1[1], m_command_fifo1[2], m_command_fifo1[3]);
					execute_command(m_command_fifo1);
					m_command_fifo1_ptr = 0;
				}
			}
			else
			{
				// write to VRAM fifo
				m_vram[m_vram_fifo1_addr] = data;
				m_vram_fifo1_addr++;
			}
			break;

		case 0x6c:
			if (ACCESSING_BITS_0_15)
			{
				m_reg_6c = data & 0xffff;
			}
			break;

		default:
			LOGREGISTER("%s_w: %02X, %08X, %08X\n", basetag(), reg, data, mem_mask);
			break;
	}
}

void k057714_device::fifo_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		if (m_ext_fifo_count != 0)      // first access is a dummy write
		{
			int count = m_ext_fifo_count - 1;
			uint32_t addr = (((m_ext_fifo_addr >> 10) + m_ext_fifo_line) * 1024) + count;

			if ((count & 1) == 0)
			{
				m_vram[addr >> 1] &= 0x0000ffff;
				m_vram[addr >> 1] |= (data & 0xffff0000);
			}
			else
			{
				m_vram[addr >> 1] &= 0xffff0000;
				m_vram[addr >> 1] |= (data >> 16);
			}
		}
		m_ext_fifo_count++;

		if (m_ext_fifo_count > m_ext_fifo_width+1)
		{
			m_ext_fifo_line++;
			m_ext_fifo_count = 0;
		}
	}
}

void k057714_device::draw_frame(int frame, bitmap_ind16 &bitmap, const rectangle &cliprect, bool inverse_trans)
{
	if (m_frame[frame].height == 0 || m_frame[frame].width == 0)
		return;

	int height = m_frame[frame].height + 1;
	int width = m_frame[frame].width + 1;
	int alpha = m_frame[frame].alpha;
	int blend_mode = alpha & 3;
	int alpha1 = (alpha >> 7) & 0x1f; // beatmania III uses this for blend mode 1
	int alpha2 = (alpha >> 2) & 0x1f; // But pop'n music has alpha 1 and 2 the same for blend mode 1

	if (blend_mode == 2)
	{
		alpha1 = (alpha2 * 16) / alpha1;
	}

	uint16_t *vram16 = (uint16_t*)m_vram.get();

	int fb_pitch = 1024;

	uint16_t trans_value = inverse_trans ? 0x8000 : 0x0000;

	if (m_frame[frame].y + height > cliprect.max_y)
		height = cliprect.max_y - m_frame[frame].y;
	if (m_frame[frame].x + width > cliprect.max_x)
		width = cliprect.max_x - m_frame[frame].x;

	for (int j = 0; j <= height; j++)
	{
		uint16_t *const d = &bitmap.pix(j + m_frame[frame].y, m_frame[frame].x);
		int li = (j * fb_pitch);
		for (int i = 0; i <= width; i++)
		{
			uint16_t pix = vram16[(m_frame[frame].base + li + i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1, 0)];
			if ((pix & 0x8000) != trans_value)
			{
				uint32_t r = (pix >> 10) & 0x1f;
				uint32_t g = (pix >> 5) & 0x1f;
				uint32_t b = (pix >> 0) & 0x1f;

				r = (r * alpha1) >> 4;
				g = (g * alpha1) >> 4;
				b = (b * alpha1) >> 4;

				if (r > 0x1f) r = 0x1f;
				if (g > 0x1f) g = 0x1f;
				if (b > 0x1f) b = 0x1f;

				d[i] = (r << 10) | (g << 5) | b;
			}
		}
	}
}

int k057714_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	bool inverse_trans = false;

	// most likely wrong, inverse transparency is only used by kbm
	// beatmania III sets m_reg_6c to 0xfff but it doesn't use inverse transparency
	if ((m_reg_6c & 0xf) != 0 && m_reg_6c != 0xfff)
		inverse_trans = true;

	draw_frame((m_layer_select >> 8) & 3, bitmap, cliprect, inverse_trans);
	draw_frame((m_layer_select >> 10) & 3, bitmap, cliprect, inverse_trans);
	draw_frame((m_layer_select >> 12) & 3, bitmap, cliprect, inverse_trans);
	draw_frame((m_layer_select >> 14) & 3, bitmap, cliprect, inverse_trans);

	return 0;
}

void k057714_device::draw_object(uint32_t *cmd)
{
	// 0x00: -------- -------- ------xx xxxxxxxx   ram x
	// 0x00: -------- xxxxxxxx xxxxxx-- --------   ram y
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer origin
	// 0x00: xxx----- -------- -------- --------   command (always 5)

	// 0x01: -------- -------- ------xx xxxxxxxx   object x
	// 0x01: -------- xxxxxxxx xxxxxx-- --------   object y
	// 0x01: -----x-- -------- -------- --------   object x flip
	// 0x01: ----x--- -------- -------- --------   object y flip
	// 0x01: --xx---- -------- -------- --------   blend mode
	// 0x01: -x------ -------- -------- --------   object transparency enable
	// 0x01: x------- -------- -------- --------   inverse transparency? (used by kbm)

	// 0x02: -------- -------- -------x xxxxxxxx   object width
	// 0x02: -------- --xxxxxx xxxxxx-- --------   object x scale
	// 0x02: -----xxx xx------ -------- --------   alpha1_1 (blend mode 2)
	// 0x02: xxxxx--- -------- -------- --------   alpha1_2 (blend mode 1)

	// 0x03: -------- -------- ------xx xxxxxxxx   object height
	// 0x03: -------- --xxxxxx xxxxxx-- --------   object y scale
	// 0x03: -----xxx xx------ -------- --------   alpha2_1 (blend mode 2)
	// 0x03: xxxxx--- -------- -------- --------   alpha2_2 (blend mode 1)

	uint32_t address_x = cmd[0] & 0x3ff;
	uint32_t address_y = (cmd[0] >> 10) & 0x3fff;
	bool relative_coords = (cmd[0] & 0x10000000) ? true : false;

	int x = cmd[1] & 0x3ff;
	int y = (cmd[1] >> 10) & 0x3fff;
	bool xflip = (cmd[1] & 0x04000000) ? true : false;
	bool yflip = (cmd[1] & 0x08000000) ? true : false;
	int blend_mode = (cmd[1] >> 28) & 3;
	bool trans_enable = (cmd[1] & 0xc0000000) ? true : false;
	uint16_t trans_value = (cmd[1] & 0x80000000) ? 0x0000 : 0x8000;

	int width = (cmd[2] & 0x1ff) + 1;
	int xscale = ((cmd[2] >> 10) & 0x7ff) * (((cmd[2] >> 10) & 0x800) ? -1 : 1);
	int alpha1_1 = (cmd[2] >> 22) & 0x1f;
	int alpha1_2 = (cmd[2] >> 27) & 0x1f;

	int height = (cmd[3] & 0x3ff) + 1;
	int yscale = ((cmd[3] >> 10) & 0x7ff) * (((cmd[3] >> 10) & 0x800) ? -1 : 1);
	int alpha2_1 = (cmd[3] >> 22) & 0x1f;
	int alpha2_2 = (cmd[3] >> 27) & 0x1f;

	if (xscale == 0 || yscale == 0)
	{
		return;
	}

	if (xflip && ((4 - ((width - 1) % 4)) <= (address_x % 4)))
	{
		// Based on logic from pop'n music 8 @ 0x800b30d0
		address_x -= 4;
	}

	if (yflip)
	{
		// Based on logic from pop'n music 8 @ 0x800b3140
		y -= (((height * 64) - 1) / yscale) - (((height - 1) * 64) / yscale);
	}

	if (relative_coords)
	{
		x += m_fb_origin_x;
		y += m_fb_origin_y;
	}

	uint32_t address = (address_y << 10) | address_x;

	LOGDRAW("%s Draw Object %08X (%d, %d), x %d, y %d, w %d, h %d, sx: %f, sy: %f [%08X %08X %08X %08X]\n", basetag(), address, address_x, address_y, x, y, width, height, 64.0f / (float)xscale, 64.0f / (float)yscale, cmd[0], cmd[1], cmd[2], cmd[3]);

	int orig_height = height;

	width = (((width * 65536) / xscale) * 64) / 65536;
	height = (((height * 65536) / yscale) * 64) / 65536;

	if (height <= 0 || width <= 0)
	{
		return;
	}

	int fb_width = m_frame[0].width + 1;
	int fb_height = m_frame[0].height + 1;

	if (width > fb_width)
		width = fb_width;
	if (height > fb_height)
		height = fb_height;

	int fb_pitch = 1024;

	int v = 0;
	int xinc = xflip ? -1 : 1;
	uint16_t *vram16 = (uint16_t*)m_vram.get();
	for (int j=0; j < height; j++)
	{
		int index;
		uint32_t fbaddr = ((j+y) * fb_pitch) + x;

		if (yflip)
		{
			index = address + ((orig_height - 1 - (v >> 6)) * fb_pitch);
		}
		else
		{
			index = address + ((v >> 6) * fb_pitch);
		}

		if (xflip)
		{
			fbaddr += width - 1;
		}

		int u = 0;
		for (int i=0; i < width; i++)
		{
			uint16_t pix = vram16[((index + (u >> 6)) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)) & 0xffffff];
			bool draw = !trans_enable || (trans_enable && ((pix & 0x8000) == trans_value));

			if (fbaddr < VRAM_SIZE_HALF && draw)
			{
				if (blend_mode)
				{
					uint16_t srcpix = vram16[fbaddr ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];

					uint32_t sr = (srcpix >> 10) & 0x1f;
					uint32_t sg = (srcpix >>  5) & 0x1f;
					uint32_t sb = (srcpix >>  0) & 0x1f;

					uint32_t r = (pix >> 10) & 0x1f;
					uint32_t g = (pix >>  5) & 0x1f;
					uint32_t b = (pix >>  0) & 0x1f;

					if (blend_mode == 1)
					{
						sr = ((sr * alpha2_2) + (r * alpha1_2)) >> 4;
						sg = ((sg * alpha2_2) + (g * alpha1_2)) >> 4;
						sb = ((sb * alpha2_2) + (b * alpha1_2)) >> 4;
					}
					else if (blend_mode == 2)
					{
						// Used by Keyboardmania for pulsating glow effects
						sr = ((sr * alpha2_1) + (r * alpha1_1)) >> 4;
						sg = ((sg * alpha2_1) + (g * alpha1_1)) >> 4;
						sb = ((sb * alpha2_1) + (b * alpha1_1)) >> 4;
					}

					if (sr > 0x1f) sr = 0x1f;
					if (sg > 0x1f) sg = 0x1f;
					if (sb > 0x1f) sb = 0x1f;

					pix = (sr << 10) | (sg << 5) | sb | (pix & 0x8000);
				}

				vram16[fbaddr ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = pix;
			}

			fbaddr += xinc;
			u += xscale;
		}

		v += yscale;
	}
}

void k057714_device::fill_rect(uint32_t *cmd)
{
	// 0x00: xxx----- -------- -------- --------       command (4)
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer origin
	// 0x00: ----xx-- -------- -------- --------   ?
	// 0x00: -------- -------- ------xx xxxxxxxx   width
	// 0x00: -------- ----xxxx xxxxxx-- --------   height

	// 0x01: -------- -------- ------xx xxxxxxxx   x
	// 0x01: -------- xxxxxxxx xxxxxx-- --------   y
	// 0x01: ---x---- -------- -------- --------   ?

	// 0x02: xxxxxxxx xxxxxxxx -------- --------   fill pattern pixel 0
	// 0x02: -------- -------- xxxxxxxx xxxxxxxx   fill pattern pixel 1

	// 0x03: xxxxxxxx xxxxxxxx -------- --------   fill pattern pixel 2
	// 0x03: -------- -------- xxxxxxxx xxxxxxxx   fill pattern pixel 3

	int x = cmd[1] & 0x3ff;
	int y = (cmd[1] >> 10) & 0x3fff;
	int width = (cmd[0] & 0x3ff) + 1;
	int height = ((cmd[0] >> 10) & 0x3ff) + 1;
	bool relative_coords = (cmd[0] & 0x10000000) ? true : false;

	if (relative_coords)
	{
		x += m_fb_origin_x;
		y += m_fb_origin_y;
	}

	uint16_t color[4];
	color[0] = (cmd[2] >> 16);
	color[1] = (cmd[2] & 0xffff);
	color[2] = (cmd[3] >> 16);
	color[3] = (cmd[3] & 0xffff);

	LOGCMDEXEC("%s Fill Rect x %d, y %d, w %d, h %d, %08X %08X [%08X %08X %08X %08X]\n", basetag(), x, y, width, height, cmd[2], cmd[3], cmd[0], cmd[1], cmd[2], cmd[3]);

	int x1 = x;
	int x2 = x + width;
	int y1 = y;
	int y2 = y + height;

	uint16_t *vram16 = (uint16_t*)m_vram.get();

	int fb_pitch = 1024;

	for (int j=y1; j < y2; j++)
	{
		uint32_t fbaddr = j * fb_pitch;
		for (int i=x1; i < x2; i++)
		{
			vram16[(fbaddr+i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = color[i&3];
		}
	}
}

void k057714_device::draw_character(uint32_t *cmd)
{
	// 0x00: xxx----- -------- -------- --------   command (7)
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer base (unverified, should be same as other operations)
	// 0x00: -------- xxxxxxxx xxxxxxxx xxxxxxxx   character data address in vram

	// 0x01: -------- -------- ------xx xxxxxxxx   character x
	// 0x01: -------- xxxxxxxx xxxxxx-- --------   character y
	// 0x01: -------x -------- -------- --------   double height
	// 0x01: --x----- -------- -------- --------   ?
	// 0x01: -x------ -------- -------- --------   transparency enable

	// 0x02: xxxxxxxx xxxxxxxx -------- --------   color 0
	// 0x02: -------- -------- xxxxxxxx xxxxxxxx   color 1

	// 0x03: xxxxxxxx xxxxxxxx -------- --------   color 2
	// 0x03: -------- -------- xxxxxxxx xxxxxxxx   color 3

	int x = cmd[1] & 0x3ff;
	int y = (cmd[1] >> 10) & 0x3fff;
	uint32_t address = cmd[0] & 0xffffff;
	uint16_t color[4];
	bool relative_coords = (cmd[0] & 0x10000000) ? true : false;
	bool double_height = (cmd[1] & 0x01000000) ? true : false;
	bool trans_enable = (cmd[1] & 0x40000000) ? true : false;

	if (relative_coords)
	{
		x += m_fb_origin_x;
		y += m_fb_origin_y;
	}

	color[0] = cmd[2] >> 16;
	color[1] = cmd[2] & 0xffff;
	color[2] = cmd[3] >> 16;
	color[3] = cmd[3] & 0xffff;

	LOGCMDEXEC("%s Draw Char %08X, x %d, y %d [%08X %08X %08X %08X]\n", basetag(), address, x, y, cmd[0], cmd[1], cmd[2], cmd[3]);

	uint16_t *vram16 = (uint16_t*)m_vram.get();
	int fb_pitch = 1024;
	int height = double_height ? 16 : 8;

	for (int j=0; j < height; j++)
	{
		uint32_t fbaddr = (y+j) * fb_pitch;
		uint16_t line = vram16[address ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];

		address += 4;

		for (int i=0; i < 8; i++)
		{
			int p = (line >> ((7-i) * 2)) & 3;
			bool draw = !trans_enable || (trans_enable && (color[p] & 0x8000));
			if (draw)
				vram16[(fbaddr+x+i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = color[p];
		}
	}
}

void k057714_device::fb_config(uint32_t *cmd)
{
	// 0x00: xxx----- -------- -------- --------   command (3)

	// 0x01: -------- -------- -------- --------   unused?

	// 0x02: -------- -------- ------xx xxxxxxxx   Framebuffer Origin X

	// 0x03: -------- -------- --xxxxxx xxxxxxxx   Framebuffer Origin Y

	LOGCMDEXEC("%s FB Config %08X %08X %08X %08X\n", basetag(), cmd[0], cmd[1], cmd[2], cmd[3]);

	m_fb_origin_x = cmd[2] & 0x3ff;
	m_fb_origin_y = cmd[3] & 0x3fff;
}

void k057714_device::execute_display_list(uint32_t addr)
{
	bool end = false;

	int counter = 0;

	LOGCMDEXEC("%s Exec Display List %08X\n", basetag(), addr);

	addr /= 2;
	while (!end && counter < 0x1000 && addr < (VRAM_SIZE/4))
	{
		uint32_t *cmd = &m_vram[addr];
		addr += 4;

		int command = (cmd[0] >> 29) & 0x7;

		switch (command)
		{
			case 0:     // NOP?
				break;

			case 1:     // Execute display list
				execute_display_list(cmd[0] & 0xffffff);
				break;

			case 2:     // End of display list
				end = true;
				break;

			case 3:     // Framebuffer config
				fb_config(cmd);
				break;

			case 4:     // Fill rectangle
				fill_rect(cmd);
				break;

			case 5:     // Draw object
				draw_object(cmd);
				break;

			case 6:
			case 7:     // Draw 8x8 character (2 bits per pixel)
				draw_character(cmd);
				break;

			default:
				LOGCMDEXEC("GCU Unknown command %08X %08X %08X %08X\n", cmd[0], cmd[1], cmd[2], cmd[3]);
				break;
		}
		counter++;
	};
}

void k057714_device::execute_command(uint32_t* cmd)
{
	int command = (cmd[0] >> 29) & 0x7;

	LOGCMDEXEC("%s Exec Command %08X, %08X, %08X, %08X\n", basetag(), cmd[0], cmd[1], cmd[2], cmd[3]);

	switch (command)
	{
		case 0:     // NOP?
			break;

		case 1:     // Execute display list
			execute_display_list(cmd[0] & 0xffffff);
			break;

		case 2:     // End of display list
			break;

		case 3:     // Framebuffer config
			fb_config(cmd);
			break;

		case 4:     // Fill rectangle
			fill_rect(cmd);
			break;

		case 5:     // Draw object
			draw_object(cmd);
			break;

		case 6:
		case 7:     // Draw 8x8 character (2 bits per pixel)
			draw_character(cmd);
			break;

		default:
			LOGCMDEXEC("GCU Unknown command %08X %08X %08X %08X\n", cmd[0], cmd[1], cmd[2], cmd[3]);
			break;
	}
}
