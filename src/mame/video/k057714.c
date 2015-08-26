// license:BSD-3-Clause
// copyright-holders:Ville Linde

// Konami 0000057714 "GCU" 2D Graphics Chip

#include "emu.h"
#include "k057714.h"


#define DUMP_VRAM 0
#define PRINT_GCU 0


const device_type K057714 = &device_creator<k057714_device>;

k057714_device::k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K057714, "K057714 GCU", tag, owner, clock, "k057714", __FILE__),
	m_irq(*this)
{
}

void k057714_device::device_start()
{
	m_irq.resolve_safe();

	m_vram = auto_alloc_array(machine(), UINT32, 0x2000000/4);
	memset(m_vram, 0, 0x2000000);
}

void k057714_device::device_reset()
{
	m_vram_read_addr = 0;
	m_command_fifo0_ptr = 0;
	m_command_fifo1_ptr = 0;
	m_vram_fifo0_addr = 0;
	m_vram_fifo1_addr = 0;

	for (int i=0; i < 4; i++)
	{
		m_frame[i].base = 0;
		m_frame[i].width = 0;
		m_frame[i].height = 0;
	}
}

void k057714_device::device_stop()
{
#if DUMP_VRAM
	char filename[200];
	sprintf(filename, "%s_vram.bin", basetag());
	printf("dumping %s\n", filename);
	FILE *file = fopen(filename, "wb");
	int i;

	for (i=0; i < 0x2000000/4; i++)
	{
		fputc((m_vram[i] >> 24) & 0xff, file);
		fputc((m_vram[i] >> 16) & 0xff, file);
		fputc((m_vram[i] >> 8) & 0xff, file);
		fputc((m_vram[i] >> 0) & 0xff, file);
	}

	fclose(file);
#endif
}


READ32_MEMBER(k057714_device::read)
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

WRITE32_MEMBER(k057714_device::write)
{
	int reg = offset * 4;

	switch (reg)
	{
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
#if PRINT_GCU
				printf("%s_w: %02X, %08X, %08X\n", basetag(), reg, data, mem_mask);
#endif
			break;

		case 0x14:      // ?
			break;

		case 0x18:      // ?
			break;

		case 0x1c:      // set to 1 on "media bus" access
			if ((data >> 16) == 1)
			{
				m_ext_fifo_count = 0;
				m_ext_fifo_line = 0;
			}
			break;

		case 0x20:      // Framebuffer 0 Origin(?)
			break;

		case 0x24:      // Framebuffer 1 Origin(?)
			break;

		case 0x28:      // Framebuffer 2 Origin(?)
			break;

		case 0x2c:      // Framebuffer 3 Origin(?)
			break;

		case 0x30:      // Framebuffer 0 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[0].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[0].width = data & 0xffff;
			break;

		case 0x34:      // Framebuffer 1 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[1].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[1].width = data & 0xffff;
			break;

		case 0x38:      // Framebuffer 2 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[2].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[2].width = data & 0xffff;
			break;

		case 0x3c:      // Framebuffer 3 Dimensions
			if (ACCESSING_BITS_16_31)
				m_frame[3].height = (data >> 16) & 0xffff;
			if (ACCESSING_BITS_0_15)
				m_frame[3].width = data & 0xffff;
			break;

		case 0x40:      // Framebuffer 0 Base
			m_frame[0].base = data;
#if PRINT_GCU
			printf("%s FB0 Base: %08X\n", basetag(), data);
#endif
			break;

		case 0x44:      // Framebuffer 1 Base
			m_frame[1].base = data;
#if PRINT_GCU
			printf("%s FB1 Base: %08X\n", basetag(), data);
#endif
			break;

		case 0x48:      // Framebuffer 2 Base
			m_frame[2].base = data;
#if PRINT_GCU
			printf("%s FB2 Base: %08X\n", basetag(), data);
#endif
			break;

		case 0x4c:      // Framebuffer 3 Base
			m_frame[3].base = data;
#if PRINT_GCU
			printf("%s FB3 Base: %08X\n", basetag(), data);
#endif
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
					//printf("GCU FIFO0 exec: %08X %08X %08X %08X\n", m_command_fifo0[0], m_command_fifo0[1], m_command_fifo0[2], m_command_fifo0[3]);
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
					//printf("GCU FIFO1 exec: %08X %08X %08X %08X\n", m_command_fifo1[0], m_command_fifo1[1], m_command_fifo1[2], m_command_fifo1[3]);
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

		default:
			//printf("%s_w: %02X, %08X, %08X at %08X\n", basetag(), reg, data, mem_mask, space.device().safe_pc());
			break;
	}
}

WRITE32_MEMBER(k057714_device::fifo_w)
{
	if (ACCESSING_BITS_16_31)
	{
		if (m_ext_fifo_count != 0)      // first access is a dummy write
		{
			int count = m_ext_fifo_count - 1;
			UINT32 addr = (((m_ext_fifo_addr >> 10) + m_ext_fifo_line) * 1024) + count;

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

int k057714_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *vram16 = (UINT16*)m_vram;

	int x = 0;
	int y = 0;
	int width = m_frame[0].width;
	int height = m_frame[0].height;

	if (width != 0 && height != 0)
	{
		rectangle visarea = screen.visible_area();
		if ((visarea.max_x+1) != width || (visarea.max_y+1) != height)
		{
			visarea.max_x = width-1;
			visarea.max_y = height-1;
			screen.configure(width, height, visarea, screen.frame_period().attoseconds());
		}
	}

	int fb_pitch = 1024;

	for (int j=0; j < height; j++)
	{
		UINT16 *d = &bitmap.pix16(j, x);
		int li = ((j+y) * fb_pitch) + x;
		UINT32 fbaddr0 = m_frame[0].base + li;
		UINT32 fbaddr1 = m_frame[1].base + li;
//      UINT32 fbaddr2 = m_frame[2].base + li;
//      UINT32 fbaddr3 = m_frame[3].base + li;

		for (int i=0; i < width; i++)
		{
			UINT16 pix0 = vram16[fbaddr0 ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];
			UINT16 pix1 = vram16[fbaddr1 ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];
//          UINT16 pix2 = vram16[fbaddr2 ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];
//          UINT16 pix3 = vram16[fbaddr3 ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];

			if (pix0 & 0x8000)
			{
				d[i] = pix0 & 0x7fff;
			}
			else
			{
				d[i] = pix1 & 0x7fff;
			}

			fbaddr0++;
			fbaddr1++;
//          fbaddr2++;
//          fbaddr3++;
		}
	}

	return 0;
}

void k057714_device::draw_object(UINT32 *cmd)
{
	// 0x00: xxx----- -------- -------- --------   command (5)
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer origin
	// 0x00: ----xx-- -------- -------- --------   ?
	// 0x00: -------- xxxxxxxx xxxxxxxx xxxxxxxx   object data address in vram

	// 0x01: -------- -------- ------xx xxxxxxxx   object x
	// 0x01: -------- xxxxxxxx xxxxxx-- --------   object y
	// 0x01: -----x-- -------- -------- --------   object x flip
	// 0x01: ----x--- -------- -------- --------   object y flip
	// 0x01: --xx---- -------- -------- --------   object alpha enable (different blend modes?)
	// 0x01: -x------ -------- -------- --------   object transparency enable (?)

	// 0x02: -------- -------- ------xx xxxxxxxx   object width
	// 0x02: -------- -----xxx xxxxxx-- --------   object x scale

	// 0x03: -------- -------- ------xx xxxxxxxx   object height
	// 0x03: -------- -----xxx xxxxxx-- --------   object y scale

	int x = cmd[1] & 0x3ff;
	int y = (cmd[1] >> 10) & 0x3fff;
	int width = (cmd[2] & 0x3ff) + 1;
	int height = (cmd[3] & 0x3ff)  + 1;
	int xscale = (cmd[2] >> 10) & 0x1ff;
	int yscale = (cmd[3] >> 10) & 0x1ff;
	bool xflip = (cmd[1] & 0x04000000) ? true : false;
	bool yflip = (cmd[1] & 0x08000000) ? true : false;
	bool alpha_enable = (cmd[1] & 0x30000000) ? true : false;
	bool trans_enable = (cmd[1] & 0x40000000) ? true : false;
	UINT32 address = cmd[0] & 0xffffff;
	int alpha_level = (cmd[2] >> 27) & 0x1f;
	bool relative_coords = (cmd[0] & 0x10000000) ? true : false;

	if (relative_coords)
	{
		x += m_fb_origin_x;
		y += m_fb_origin_y;
	}

	UINT16 *vram16 = (UINT16*)m_vram;

	if (xscale == 0 || yscale == 0)
	{
		return;
	}

#if PRINT_GCU
	printf("%s Draw Object %08X, x %d, y %d, w %d, h %d [%08X %08X %08X %08X]\n", basetag(), address, x, y, width, height, cmd[0], cmd[1], cmd[2], cmd[3]);
#endif

	width = (((width * 65536) / xscale) * 64) / 65536;
	height = (((height * 65536) / yscale) * 64) / 65536;

	int fb_pitch = 1024;

	int v = 0;
	for (int j=0; j < height; j++)
	{
		int index;
		int xinc;
		UINT32 fbaddr = ((j+y) * fb_pitch) + x;

		if (yflip)
		{
			index = address + ((height - 1 - (v >> 6)) * 1024);
		}
		else
		{
			index = address + ((v >> 6) * 1024);
		}

		if (xflip)
		{
			fbaddr += width;
			xinc = -1;
		}
		else
		{
			xinc = 1;
		}

		int u = 0;
		for (int i=0; i < width; i++)
		{
			UINT16 pix = vram16[((index + (u >> 6)) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)) & 0xffffff];
			bool draw = !trans_enable || (trans_enable && (pix & 0x8000));
			if (alpha_enable)
			{
				if (draw)
				{
					if ((pix & 0x7fff) != 0)
					{
						UINT16 srcpix = vram16[fbaddr ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];

						UINT32 sr = (srcpix >> 10) & 0x1f;
						UINT32 sg = (srcpix >>  5) & 0x1f;
						UINT32 sb = (srcpix >>  0) & 0x1f;
						UINT32 r = (pix >> 10) & 0x1f;
						UINT32 g = (pix >>  5) & 0x1f;
						UINT32 b = (pix >>  0) & 0x1f;

						sr += (r * alpha_level) >> 4;
						sg += (g * alpha_level) >> 4;
						sb += (b * alpha_level) >> 4;

						if (sr > 0x1f) sr = 0x1f;
						if (sg > 0x1f) sg = 0x1f;
						if (sb > 0x1f) sb = 0x1f;

						vram16[fbaddr ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = (sr << 10) | (sg << 5) | sb | 0x8000;
					}
				}
			}
			else
			{
				if (draw)
				{
					vram16[fbaddr ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = pix | 0x8000;
				}
			}

			fbaddr += xinc;
			u += xscale;
		}

		v += yscale;
	}
}

void k057714_device::fill_rect(UINT32 *cmd)
{
	// 0x00: xxx----- -------- -------- --------       command (4)
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer origin
	// 0x00: ----xx-- -------- -------- --------   ?
	// 0x00: -------- -------- ------xx xxxxxxxx   width
	// 0x00: -------- ----xxxx xxxxxx-- --------   height

	// 0x01: -------- -------- ------xx xxxxxxxx   x
	// 0x01: -------- xxxxxxxx xxxxxx-- --------   y

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

	UINT16 color[4];
	color[0] = (cmd[2] >> 16);
	color[1] = (cmd[2] & 0xffff);
	color[2] = (cmd[3] >> 16);
	color[3] = (cmd[3] & 0xffff);

#if PRINT_GCU
	printf("%s Fill Rect x %d, y %d, w %d, h %d, %08X %08X [%08X %08X %08X %08X]\n", basetag(), x, y, width, height, cmd[2], cmd[3], cmd[0], cmd[1], cmd[2], cmd[3]);
#endif

	int x1 = x;
	int x2 = x + width;
	int y1 = y;
	int y2 = y + height;

	UINT16 *vram16 = (UINT16*)m_vram;

	int fb_pitch = 1024;

	for (int j=y1; j < y2; j++)
	{
		UINT32 fbaddr = j * fb_pitch;
		for (int i=x1; i < x2; i++)
		{
			vram16[(fbaddr+i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = color[i&3];
		}
	}
}

void k057714_device::draw_character(UINT32 *cmd)
{
	// 0x00: xxx----- -------- -------- --------   command (7)
	// 0x00: ---x---- -------- -------- --------   0: absolute coordinates
	//                                             1: relative coordinates from framebuffer base (unverified, should be same as other operations)
	// 0x00: -------- xxxxxxxx xxxxxxxx xxxxxxxx   character data address in vram

	// 0x01: -------- -------- ------xx xxxxxxxx   character x
	// 0x01: -------- ----xxxx xxxxxx-- --------   character y
	// 0x01: -------x -------- -------- --------   double height

	// 0x02: xxxxxxxx xxxxxxxx -------- --------   color 0
	// 0x02: -------- -------- xxxxxxxx xxxxxxxx   color 1

	// 0x03: xxxxxxxx xxxxxxxx -------- --------   color 2
	// 0x03: -------- -------- xxxxxxxx xxxxxxxx   color 3

	int x = cmd[1] & 0x3ff;
	int y = (cmd[1] >> 10) & 0x3ff;
	UINT32 address = cmd[0] & 0xffffff;
	UINT16 color[4];
	bool relative_coords = (cmd[0] & 0x10000000) ? true : false;
	bool double_height = (cmd[1] & 0x01000000) ? true : false;

	if (relative_coords)
	{
		x += m_fb_origin_x;
		y += m_fb_origin_y;
	}

	color[0] = cmd[2] >> 16;
	color[1] = cmd[2] & 0xffff;
	color[2] = cmd[3] >> 16;
	color[3] = cmd[3] & 0xffff;

#if PRINT_GCU
	printf("%s Draw Char %08X, x %d, y %d\n", basetag(), address, x, y);
#endif

	UINT16 *vram16 = (UINT16*)m_vram;
	int fb_pitch = 1024;
	int height = double_height ? 16 : 8;

	for (int j=0; j < height; j++)
	{
		UINT32 fbaddr = (y+j) * fb_pitch;
		UINT16 line = vram16[address ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)];

		address += 4;

		for (int i=0; i < 8; i++)
		{
			int p = (line >> ((7-i) * 2)) & 3;
			vram16[(fbaddr+x+i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = color[p] | 0x8000;
		}
	}
}

void k057714_device::fb_config(UINT32 *cmd)
{
	// 0x00: xxx----- -------- -------- --------   command (3)

	// 0x01: -------- -------- -------- --------   unused?

	// 0x02: -------- -------- ------xx xxxxxxxx   Framebuffer Origin X

	// 0x03: -------- -------- --xxxxxx xxxxxxxx   Framebuffer Origin Y

#if PRINT_GCU
	printf("%s FB Config %08X %08X %08X %08X\n", basetag(), cmd[0], cmd[1], cmd[2], cmd[3]);
#endif

	m_fb_origin_x = cmd[2] & 0x3ff;
	m_fb_origin_y = cmd[3] & 0x3fff;
}

void k057714_device::execute_display_list(UINT32 addr)
{
	bool end = false;

	int counter = 0;

#if PRINT_GCU
	printf("%s Exec Display List %08X\n", basetag(), addr);
#endif

	addr /= 2;
	while (!end && counter < 0x1000 && addr < (0x2000000/4))
	{
		UINT32 *cmd = &m_vram[addr];
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
				printf("GCU Unknown command %08X %08X %08X %08X\n", cmd[0], cmd[1], cmd[2], cmd[3]);
				break;
		}
		counter++;
	};
}

void k057714_device::execute_command(UINT32* cmd)
{
	int command = (cmd[0] >> 29) & 0x7;

#if PRINT_GCU
	printf("%s Exec Command %08X, %08X, %08X, %08X\n", basetag(), cmd[0], cmd[1], cmd[2], cmd[3]);
#endif

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
			printf("GCU Unknown command %08X %08X %08X %08X\n", cmd[0], cmd[1], cmd[2], cmd[3]);
			break;
	}
}
