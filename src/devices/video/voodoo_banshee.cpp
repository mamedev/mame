// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_banshee.cpp

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

    Specs:

    Voodoo Banshee (h3):
        Integrated VGA support
        2,4,8MB frame buffer RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    Voodoo 3 ("Avenger"/h4):
        Integrated VGA support
        4,8,16MB frame buffer RAM
        143MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

**************************************************************************/

#include "emu.h"
#include "voodoo_banshee.h"

#include "screen.h"

using namespace voodoo;


//**************************************************************************
//  DEBUGGING
//**************************************************************************



DEFINE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device, "voodoo_banshee", "3dfx Voodoo Banshee")

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_BANSHEE, tag, owner, clock, MODEL_VOODOO_BANSHEE)
{
}

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model)
	: voodoo_device_base(mconfig, type, tag, owner, clock, model)
{
}


int voodoo_banshee_device::update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if bypassing the clut, don't worry about the rest
	if (BIT(m_banshee.io[io_vidProcCfg], 11))
		return update_common(bitmap, cliprect, m_shared->rgb565);

	// if the CLUT is dirty, recompute the pens array
	if (m_fbi.m_clut_dirty)
	{
		rgb_t const *clutbase = &m_fbi.m_clut[256 * BIT(m_banshee.io[io_vidProcCfg], 13)];

		// compute R/G/B pens first
		u8 rtable[32], gtable[64], btable[32];
		for (u32 rawcolor = 0; rawcolor < 32; rawcolor++)
		{
			// treat X as a 5-bit value, scale up to 8 bits
			u32 color = pal5bit(rawcolor);
			rtable[rawcolor] = clutbase[color].r();
			btable[rawcolor] = clutbase[color].b();

			// treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 0);
			gtable[rawcolor * 2 + 0] = clutbase[color].g();

			// treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 1);
			gtable[rawcolor * 2 + 1] = clutbase[color].g();
		}

		// now compute the actual pens array
		for (u32 pen = 0; pen < 65536; pen++)
			m_fbi.m_pen[pen] = rgb_t(rtable[BIT(pen, 11, 5)], gtable[BIT(pen, 5, 6)], btable[BIT(pen, 0, 5)]);

		// no longer dirty
		m_fbi.m_clut_dirty = false;
		m_fbi.m_video_changed = true;
	}
	return update_common(bitmap, cliprect, &m_fbi.m_pen[0]);
}


void voodoo_banshee_device::device_start()
{
	// initialize banshee registers
	memset(m_banshee.io, 0, sizeof(m_banshee.io));
	m_banshee.io[io_pciInit0] = 0x01800040;
	m_banshee.io[io_sipMonitor] = 0x40000000;
	m_banshee.io[io_lfbMemoryConfig] = 0x000a2200;
	m_banshee.io[io_dramInit0] = 0x00579d29;
	if (m_fbmem_in_mb == 16)
		m_banshee.io[io_dramInit0] |= 0x0c000000;      // Midway Vegas (denver) expects 2 banks of 16MBit SGRAMs
	else
		m_banshee.io[io_dramInit0] |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	m_banshee.io[io_dramInit1] = 0x00f02200;
	m_banshee.io[io_tmuGbeInit] = 0x00000bfb;

	// call our parent
	voodoo_device_base::device_start();

	/* register states: banshee */
	save_item(NAME(m_banshee.io));
	save_item(NAME(m_banshee.agp));
	save_item(NAME(m_banshee.vga));
	save_item(NAME(m_banshee.crtc));
	save_item(NAME(m_banshee.seq));
	save_item(NAME(m_banshee.gc));
	save_item(NAME(m_banshee.att));
	save_item(NAME(m_banshee.attff));
}


s32 voodoo_banshee_device::lfb_direct_w(offs_t offset, u32 data, u32 mem_mask)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.lfb_writes++;

	// byte swizzling
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.byte_swizzle_writes())
	{
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	// word swapping
	if (lfbmode.word_swap_writes())
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// TODO: This direct write is not verified.
	// For direct lfb access just write the data
	offset <<= 1;
	int const x = offset & ((1 << m_fbi.m_lfb_stride) - 1);
	int const y = offset >> m_fbi.m_lfb_stride;
	u16 *const dest = (u16 *)&m_fbi.m_ram[m_fbi.m_lfb_base * 4];
	u32 const destmax = m_fbi.ram_end() - dest;
	u32 const bufoffs = y * m_fbi.m_rowpixels + x;
	if (bufoffs >= destmax)
	{
		logerror("lfb_direct_w: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X data=%08X\n", x, y, offset, bufoffs, data);
		return 0;
	}
	if (ACCESSING_BITS_0_15)
		dest[bufoffs + 0] = data & 0xffff;
	if (ACCESSING_BITS_16_31)
		dest[bufoffs + 1] = data >> 16;

	// Need to notify that frame buffer has changed
	m_fbi.m_video_changed = true;
	if (LOG_LFB)
		logerror("VOODOO.LFB:write direct (%d,%d) = %08X & %08X\n", x, y, data, mem_mask);
	return 0;
}


/*************************************
 *
 *  Handle a read from the Banshee
 *  I/O space
 *
 *************************************/

u32 voodoo_banshee_device::banshee_agp_r(offs_t offset)
{
	u32 result;

	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdRdPtrL0:
			result = m_fbi.m_cmdfifo[0].read_pointer();
			break;

		case cmdAMin0:
			result = m_fbi.m_cmdfifo[0].address_min();
			break;

		case cmdAMax0:
			result = m_fbi.m_cmdfifo[0].address_max();
			break;

		case cmdFifoDepth0:
			result = m_fbi.m_cmdfifo[0].depth();
			break;

		case cmdHoleCnt0:
			result = m_fbi.m_cmdfifo[0].holes();
			break;

		case cmdRdPtrL1:
			result = m_fbi.m_cmdfifo[1].read_pointer();
			break;

		case cmdAMin1:
			result = m_fbi.m_cmdfifo[1].address_min();
			break;

		case cmdAMax1:
			result = m_fbi.m_cmdfifo[1].address_max();
			break;

		case cmdFifoDepth1:
			result = m_fbi.m_cmdfifo[1].depth();
			break;

		case cmdHoleCnt1:
			result = m_fbi.m_cmdfifo[1].holes();
			break;

		default:
			result = m_banshee.agp[offset];
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_r(AGP:%s)\n", machine().describe_context(), voodoo_regs::s_banshee_agp_reg_name[offset]);
	return result;
}


u32 voodoo_banshee_device::banshee_r(offs_t offset, u32 mem_mask)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (operation_pending())
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		result = banshee_io_r(offset, mem_mask);
	else if (offset < 0x100000/4)
		result = banshee_agp_r(offset);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_r(2D:%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	else if (offset < 0x600000/4)
		result = register_r(offset & 0x1fffff/4);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_r(TEX0:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_r(TEX1:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_r(YUV:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x2000000/4)
	{
		result = lfb_r(offset & 0xffffff/4, true);
	} else {
			logerror("%s:banshee_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	return result;
}


u32 voodoo_banshee_device::banshee_fb_r(offs_t offset)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (operation_pending())
		flush_fifos(machine().time());

	if (offset < m_fbi.m_lfb_base)
	{
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X)\n", machine().describe_context(), offset*4);
		if (offset*4 <= m_fbi.m_mask)
			result = ((u32 *)m_fbi.m_ram)[offset];
		else
			logerror("%s:banshee_fb_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	else {
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X) to lfb_r: %08X lfb_base=%08X\n", machine().describe_context(), offset*4, offset - m_fbi.m_lfb_base, m_fbi.m_lfb_base);
		result = lfb_r(offset - m_fbi.m_lfb_base, false);
	}
	return result;
}


u8 voodoo_banshee_device::banshee_vga_r(offs_t offset)
{
	u8 result = 0xff;

	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
			if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
				result = m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_att_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f]);
			break;

		/* Input status 0 */
		case 0x3c2:
			/*
			    bit 7 = Interrupt Status. When its value is ?1?, denotes that an interrupt is pending.
			    bit 6:5 = Feature Connector. These 2 bits are readable bits from the feature connector.
			    bit 4 = Sense. This bit reflects the state of the DAC monitor sense logic.
			    bit 3:0 = Reserved. Read back as 0.
			*/
			result = 0x00;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				result = m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f]);
			break;

		/* Feature control */
		case 0x3ca:
			result = m_banshee.vga[0x3da & 0x1f];
			m_banshee.attff = 0;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Miscellaneous output */
		case 0x3cc:
			result = m_banshee.vga[0x3c2 & 0x1f];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				result = m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f]);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				result = m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f]);
			break;

		/* Input status 1 */
		case 0x3da:
			/*
			    bit 7:6 = Reserved. These bits read back 0.
			    bit 5:4 = Display Status. These 2 bits reflect 2 of the 8 pixel data outputs from the Attribute
			                controller, as determined by the Attribute controller index 0x12 bits 4 and 5.
			    bit 3 = Vertical sync Status. A ?1? indicates vertical retrace is in progress.
			    bit 2:1 = Reserved. These bits read back 0x2.
			    bit 0 = Display Disable. When this bit is 1, either horizontal or vertical display end has occurred,
			                otherwise video data is being displayed.
			*/
			result = 0x04;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		default:
			result = m_banshee.vga[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;
	}
	return result;
}


u32 voodoo_banshee_device::banshee_io_r(offs_t offset, u32 mem_mask)
{
	u32 result;

	offset &= 0xff/4;

	/* switch off the offset */
	switch (offset)
	{
		case io_status:
			result = register_r(0);
			break;

		case io_dacData:
			result = m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_r(%X)\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			result = 0;
			if (ACCESSING_BITS_0_7)
				result |= banshee_vga_r(offset*4+0) << 0;
			if (ACCESSING_BITS_8_15)
				result |= banshee_vga_r(offset*4+1) << 8;
			if (ACCESSING_BITS_16_23)
				result |= banshee_vga_r(offset*4+2) << 16;
			if (ACCESSING_BITS_24_31)
				result |= banshee_vga_r(offset*4+3) << 24;
			break;

		default:
			result = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_r(%s)\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset]);
			break;
	}

	return result;
}


u32 voodoo_banshee_device::banshee_rom_r(offs_t offset)
{
	logerror("%s:banshee_rom_r(%X)\n", machine().describe_context(), offset*4);
	return 0xffffffff;
}

void voodoo_banshee_device::banshee_blit_2d(u32 data)
{
	switch (m_banshee.blt_cmd)
	{
		case 0:         // NOP - wait for idle
		{
			break;
		}

		case 1:         // Screen-to-screen blit
		{
			// TODO
			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:screen_to_screen: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
			break;
		}

		case 2:         // Screen-to-screen stretch blit
		{
			fatalerror("   blit_2d:screen_to_screen_stretch: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 3:         // Host-to-screen blit
		{
			u32 addr = m_banshee.blt_dst_base;

			addr += (m_banshee.blt_dst_y * m_banshee.blt_dst_stride) + (m_banshee.blt_dst_x * m_banshee.blt_dst_bpp);

			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, m_banshee.blt_dst_x, m_banshee.blt_dst_y);

			switch (m_banshee.blt_dst_bpp)
			{
				case 1:
					m_fbi.m_ram[addr+0] = data & 0xff;
					m_fbi.m_ram[addr+1] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+3] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 4;
					break;
				case 2:
					m_fbi.m_ram[addr+1] = data & 0xff;
					m_fbi.m_ram[addr+0] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+3] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 2;
					break;
				case 3:
					m_banshee.blt_dst_x += 1;
					break;
				case 4:
					m_fbi.m_ram[addr+3] = data & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+1] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+0] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 1;
					break;
			}

			if (m_banshee.blt_dst_x >= m_banshee.blt_dst_width)
			{
				m_banshee.blt_dst_x = 0;
				m_banshee.blt_dst_y++;
			}
			break;
		}

		case 5:         // Rectangle fill
		{
			fatalerror("blit_2d:rectangle_fill: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 6:         // Line
		{
			fatalerror("blit_2d:line: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 7:         // Polyline
		{
			fatalerror("blit_2d:polyline: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 8:         // Polygon fill
		{
			fatalerror("blit_2d:polygon_fill\n");
		}

		default:
		{
			fatalerror("blit_2d: unknown command %d\n", m_banshee.blt_cmd);
		}
	}
}

s32 voodoo_banshee_device::banshee_2d_w(offs_t offset, u32 data)
{
	switch (offset)
	{
		case banshee2D_command:
			if (LOG_BANSHEE_2D)
				logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);

			m_banshee.blt_src_x        = m_banshee.blt_regs[banshee2D_srcXY] & 0xfff;
			m_banshee.blt_src_y        = (m_banshee.blt_regs[banshee2D_srcXY] >> 16) & 0xfff;
			m_banshee.blt_src_base     = m_banshee.blt_regs[banshee2D_srcBaseAddr] & 0xffffff;
			m_banshee.blt_src_stride   = m_banshee.blt_regs[banshee2D_srcFormat] & 0x3fff;
			m_banshee.blt_src_width    = m_banshee.blt_regs[banshee2D_srcSize] & 0xfff;
			m_banshee.blt_src_height   = (m_banshee.blt_regs[banshee2D_srcSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_srcFormat] >> 16) & 0xf)
			{
				case 1: m_banshee.blt_src_bpp = 1; break;
				case 3: m_banshee.blt_src_bpp = 2; break;
				case 4: m_banshee.blt_src_bpp = 3; break;
				case 5: m_banshee.blt_src_bpp = 4; break;
				case 8: m_banshee.blt_src_bpp = 2; break;
				case 9: m_banshee.blt_src_bpp = 2; break;
				default: m_banshee.blt_src_bpp = 1; break;
			}

			m_banshee.blt_dst_x        = m_banshee.blt_regs[banshee2D_dstXY] & 0xfff;
			m_banshee.blt_dst_y        = (m_banshee.blt_regs[banshee2D_dstXY] >> 16) & 0xfff;
			m_banshee.blt_dst_base     = m_banshee.blt_regs[banshee2D_dstBaseAddr] & 0xffffff;
			m_banshee.blt_dst_stride   = m_banshee.blt_regs[banshee2D_dstFormat] & 0x3fff;
			m_banshee.blt_dst_width    = m_banshee.blt_regs[banshee2D_dstSize] & 0xfff;
			m_banshee.blt_dst_height   = (m_banshee.blt_regs[banshee2D_dstSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_dstFormat] >> 16) & 0x7)
			{
				case 1: m_banshee.blt_dst_bpp = 1; break;
				case 3: m_banshee.blt_dst_bpp = 2; break;
				case 4: m_banshee.blt_dst_bpp = 3; break;
				case 5: m_banshee.blt_dst_bpp = 4; break;
				default: m_banshee.blt_dst_bpp = 1; break;
			}

			m_banshee.blt_cmd = data & 0xf;
			break;

		case banshee2D_colorBack:
			if (LOG_BANSHEE_2D)
				logerror("   2D:colorBack: %08X\n", data);
			m_banshee.blt_regs[banshee2D_colorBack] = data;
			break;

		case banshee2D_colorFore:
			if (LOG_BANSHEE_2D)
				logerror("   2D:colorFore: %08X\n", data);
			m_banshee.blt_regs[banshee2D_colorFore] = data;
			break;

		case banshee2D_srcBaseAddr:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
			m_banshee.blt_regs[banshee2D_srcBaseAddr] = data;
			break;

		case banshee2D_dstBaseAddr:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
			m_banshee.blt_regs[banshee2D_dstBaseAddr] = data;
			break;

		case banshee2D_srcSize:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_srcSize] = data;
			break;

		case banshee2D_dstSize:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_dstSize] = data;
			break;

		case banshee2D_srcXY:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_srcXY] = data;
			break;

		case banshee2D_dstXY:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_dstXY] = data;
			break;

		case banshee2D_srcFormat:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcFormat: str %d, fmt %d, packing %d\n", data & 0x3fff, (data >> 16) & 0xf, (data >> 22) & 0x3);
			m_banshee.blt_regs[banshee2D_srcFormat] = data;
			break;

		case banshee2D_dstFormat:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstFormat: str %d, fmt %d\n", data & 0x3fff, (data >> 16) & 0xf);
			m_banshee.blt_regs[banshee2D_dstFormat] = data;
			break;

		case banshee2D_clip0Min:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip0Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip0Min] = data;
			break;

		case banshee2D_clip0Max:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip0Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip0Max] = data;
			break;

		case banshee2D_clip1Min:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip1Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip1Min] = data;
			break;

		case banshee2D_clip1Max:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip1Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip1Max] = data;
			break;

		case banshee2D_rop:
			if (LOG_BANSHEE_2D)
				logerror("   2D:rop: %d, %d, %d\n",  data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
			m_banshee.blt_regs[banshee2D_rop] = data;
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
			{
				banshee_blit_2d(data);
			}
			else if (offset >= 0x40 && offset < 0x80)
			{
				// TODO: colorPattern
			}
			break;
	}


	return 1;
}




void voodoo_banshee_device::banshee_agp_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdBaseAddr0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[0].set_base(BIT(data, 0, 24) << 12);
			m_fbi.m_cmdfifo[0].set_size((BIT(m_banshee.agp[cmdBaseSize0], 0, 8) + 1) << 12);
			break;

		case cmdBaseSize0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[0].set_size((BIT(m_banshee.agp[cmdBaseSize0], 0, 8) + 1) << 12);
			m_fbi.m_cmdfifo[0].set_enable(BIT(data, 8));
			m_fbi.m_cmdfifo[0].set_count_holes(!BIT(data, 10));
			break;

		case cmdBump0:
			fatalerror("cmdBump0\n");

		case cmdRdPtrL0:
			m_fbi.m_cmdfifo[0].set_read_pointer(data);
			break;

		case cmdAMin0:
			m_fbi.m_cmdfifo[0].set_address_min(data);
			break;

		case cmdAMax0:
			m_fbi.m_cmdfifo[0].set_address_max(data);
			break;

		case cmdFifoDepth0:
			m_fbi.m_cmdfifo[0].set_depth(data);
			break;

		case cmdHoleCnt0:
			m_fbi.m_cmdfifo[0].set_holes(data);
			break;

		case cmdBaseAddr1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[1].set_base(BIT(data, 0, 24) << 12);
			m_fbi.m_cmdfifo[1].set_size((BIT(m_banshee.agp[cmdBaseSize1], 0, 8) + 1) << 12);
			break;

		case cmdBaseSize1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[1].set_size((BIT(m_banshee.agp[cmdBaseSize1], 0, 8) + 1) << 12);
			m_fbi.m_cmdfifo[1].set_enable(BIT(data, 8));
			m_fbi.m_cmdfifo[1].set_count_holes(!BIT(data, 10));
			break;

		case cmdBump1:
			fatalerror("cmdBump1\n");

		case cmdRdPtrL1:
			m_fbi.m_cmdfifo[1].set_read_pointer(data);
			break;

		case cmdAMin1:
			m_fbi.m_cmdfifo[1].set_address_min(data);
			break;

		case cmdAMax1:
			m_fbi.m_cmdfifo[1].set_address_max(data);
			break;

		case cmdFifoDepth1:
			m_fbi.m_cmdfifo[1].set_depth(data);
			break;

		case cmdHoleCnt1:
			m_fbi.m_cmdfifo[1].set_holes(data);
			break;

		default:
			COMBINE_DATA(&m_banshee.agp[offset]);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_w(AGP:%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_agp_reg_name[offset], data, mem_mask);
}


void voodoo_banshee_device::banshee_w(offs_t offset, u32 data, u32 mem_mask)
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (operation_pending())
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		banshee_io_w(offset, data, mem_mask);
	else if (offset < 0x100000/4)
		banshee_agp_w(offset, data, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_w(2D:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0xfffff, data, mem_mask);
	else if (offset < 0x600000/4)
		register_w(offset & 0x1fffff/4, data);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_w(TEX0:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_w(TEX1:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_w(YUV:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x3fffff, data, mem_mask);
	else if (offset < 0x2000000/4)
	{
		lfb_w(offset & 0xffffff/4, data, mem_mask);
	} else {
		logerror("%s:banshee_w Address out of range %08X = %08X & %08X\n", machine().describe_context(), (offset*4), data, mem_mask);
	}
}


void voodoo_banshee_device::banshee_fb_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 addr = offset*4;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (operation_pending())
		flush_fifos(machine().time());

	if (offset < m_fbi.m_lfb_base)
	{
		if (!m_fbi.m_cmdfifo[0].write_if_in_range(addr, data) && !m_fbi.m_cmdfifo[1].write_if_in_range(addr, data))
		{
			if (offset*4 <= m_fbi.m_mask)
				COMBINE_DATA(&((u32 *)m_fbi.m_ram)[offset]);
			else
				logerror("%s:banshee_fb_w Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			if (LOG_LFB)
				logerror("%s:banshee_fb_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
		}
	}
	else
		lfb_direct_w(offset - m_fbi.m_lfb_base, data, mem_mask);
}


void voodoo_banshee_device::banshee_vga_w(offs_t offset, u8 data)
{
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
		case 0x3c1:
			if (m_banshee.attff == 0)
			{
				m_banshee.vga[0x3c1 & 0x1f] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			}
			else
			{
				if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
					m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_att_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f], data);
			}
			m_banshee.attff ^= 1;
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f], data);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f], data);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f], data);
			break;

		default:
			m_banshee.vga[offset] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			break;
	}
}


void voodoo_banshee_device::banshee_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 old;

	offset &= 0xff/4;
	old = m_banshee.io[offset];

	/* switch off the offset */
	switch (offset)
	{
		case io_vidProcCfg:
			COMBINE_DATA(&m_banshee.io[offset]);
			if ((m_banshee.io[offset] ^ old) & 0x2000)
				m_fbi.m_clut_dirty = true;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_dacData:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (m_banshee.io[offset] != m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff])
			{
				m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
				m_fbi.m_clut_dirty = true;
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_w(%X) = %08X & %08X\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff, data, mem_mask);
			break;

		case io_miscInit0:
			m_renderer->wait(voodoo_regs::s_banshee_io_reg_name[offset]);
			COMBINE_DATA(&m_banshee.io[offset]);
			m_renderer->set_yorigin(m_fbi.m_yorigin = (data >> 18) & 0xfff);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vidScreenSize:
			if (data & 0xfff)
				m_fbi.m_width = data & 0xfff;
			if (data & 0xfff000)
				m_fbi.m_height = (data >> 12) & 0xfff;
			[[fallthrough]];
		case io_vidOverlayDudx:
		case io_vidOverlayDvdy:
		{
			COMBINE_DATA(&m_banshee.io[offset]);

			// Get horizontal total and vertical total from CRTC registers
			int htotal = (m_banshee.crtc[0] + 5) * 8;
			int vtotal = m_banshee.crtc[6];
			vtotal |= ((m_banshee.crtc[7] >> 0) & 0x1) << 8;
			vtotal |= ((m_banshee.crtc[7] >> 5) & 0x1) << 9;
			vtotal += 2;

			int vstart = m_banshee.crtc[0x10];
			vstart |= ((m_banshee.crtc[7] >> 2) & 0x1) << 8;
			vstart |= ((m_banshee.crtc[7] >> 7) & 0x1) << 9;

			int vstop = m_banshee.crtc[0x11] & 0xf;
			// Compare to see if vstop is before or after low 4 bits of vstart
			if (vstop < (vstart & 0xf))
				vstop |= (vstart + 0x10) & ~0xf;
			else
				vstop |= vstart & ~0xf;

			// Get pll k, m and n from pllCtrl0
			const u32 k = (m_banshee.io[io_pllCtrl0] >> 0) & 0x3;
			const u32 m = (m_banshee.io[io_pllCtrl0] >> 2) & 0x3f;
			const u32 n = (m_banshee.io[io_pllCtrl0] >> 8) & 0xff;
			const double video_clock = (XTAL(14'318'181) * (n + 2) / ((m + 2) << k)).dvalue();
			const double frame_period = vtotal * htotal / video_clock;
			//osd_printf_info("k: %d m: %d n: %d clock: %f period: %f rate: %.2f\n", k, m, n, video_clock, frame_period, 1.0 / frame_period);

			int width = m_fbi.m_width;
			int height = m_fbi.m_height;
			//m_fbi.m_xoffs = hbp;
			//m_fbi.m_yoffs = vbp;

			if (m_banshee.io[io_vidOverlayDudx] != 0)
				width = (m_fbi.m_width * m_banshee.io[io_vidOverlayDudx]) / 1048576;
			if (m_banshee.io[io_vidOverlayDvdy] != 0)
				height = (m_fbi.m_height * m_banshee.io[io_vidOverlayDvdy]) / 1048576;
			if (LOG_REGISTERS)
				logerror("configure screen: htotal: %d vtotal: %d vstart: %d vstop: %d width: %d height: %d refresh: %f\n",
					htotal, vtotal, vstart, vstop, width, height, 1.0 / frame_period);
			if (htotal > 0 && vtotal > 0) {
				rectangle visarea(0, width - 1, 0, height - 1);
				screen().configure(htotal, vtotal, visarea, DOUBLE_TO_ATTOSECONDS(frame_period));

				// Set the vsync start and stop
				m_fbi.m_vsyncstart = vstart;
				m_fbi.m_vsyncstop = vstop;
				adjust_vblank_timer();
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;
		}

		case io_lfbMemoryConfig:
			m_fbi.m_lfb_base = (data & 0x1fff) << (12-2);
			m_fbi.m_lfb_stride = ((data >> 13) & 7) + 9;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			if (ACCESSING_BITS_0_7)
				banshee_vga_w(offset*4+0, data >> 0);
			if (ACCESSING_BITS_8_15)
				banshee_vga_w(offset*4+1, data >> 8);
			if (ACCESSING_BITS_16_23)
				banshee_vga_w(offset*4+2, data >> 16);
			if (ACCESSING_BITS_24_31)
				banshee_vga_w(offset*4+3, data >> 24);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		default:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;
	}
}


DEFINE_DEVICE_TYPE(VOODOO_3, voodoo_3_device, "voodoo_3", "3dfx Voodoo 3")

voodoo_3_device::voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_3, tag, owner, clock, MODEL_VOODOO_3)
{
}
