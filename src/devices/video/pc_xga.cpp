// license:BSD-3-Clause
// copyright-holders: Carl

#include "emu.h"
#include "pc_xga.h"


#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XGA_COPRO,  xga_copro_device,  "xga_copro",  "IBM XGA Coprocessor")

xga_copro_device::xga_copro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XGA_COPRO, tag, owner, clock)
	, m_var(TYPE::XGA)
	, m_mem_read_cb(*this, 0)
	, m_mem_write_cb(*this)
{
}

void xga_copro_device::start_command()
{
	switch((m_pelop >> 24) & 15)
	{
		case 11:
			if(m_var == TYPE::OTI111)
			{
				LOG("oak specific textblt\n");
				break;
			}
			[[fallthrough]];
		case 12:
			if(m_var == TYPE::OTI111)
			{
				LOG("oak specific fast pattern copy\n");
				break;
			}
			[[fallthrough]];
		default:
			LOG("invalid pel op step func %d\n", (m_pelop >> 24) & 15);
			break;
		case 2:
			LOG("draw and step read\n");
			break;
		case 3:
			LOG("line draw read\n");
			break;
		case 4:
			LOG("draw and step write\n");
			break;
		case 5:
			LOG("line draw write\n");
			break;
		case 8:
			do_pxblt();
			break;
		case 9:
			LOG("inverting pxblt\n");
			break;
		case 10:
			LOG("area fill pxblt\n");
			break;
	}
}

// Can maps be not byte aligned? XGA manual says they must be 32bit aligned in at least some cases.
u32 xga_copro_device::read_map_pixel(int x, int y, int map)
{
	offs_t addr = m_pelmap_base[map];
	int width = m_pelmap_width[map] + 1;
	int height = m_pelmap_height[map] + 1;
	int endian = m_pelmap_format[map] & 8;
	int wbytes, bits;
	u8 byte;
	if((x > width) || (y > height) || (x < 0) || (y < 0))
		return 0;
	switch(m_pelmap_format[map] & 7)
	{
		case 0:
			wbytes = width / 8;
			addr += y * wbytes;
			addr += x / 8;
			byte = m_mem_read_cb(addr);
			bits = (x % 8) - (endian ? 8 : 0);
			return (byte >> bits) & 1;
		case 1:
			wbytes = width / 4;
			addr += y * wbytes;
			addr += x / 4;
			byte = m_mem_read_cb(addr);
			bits = (x % 4) - (endian ? 4 : 0);
			return (byte >> (bits * 2)) & 3;
		case 2:
			wbytes = width / 2;
			addr += y * wbytes;
			addr += x / 2;
			byte = m_mem_read_cb(addr);
			bits = (x % 2) - (endian ? 2 : 0);
			return (byte >> (bits * 4)) & 0xf;
		case 3:
			wbytes = width;
			addr += y * wbytes;
			addr += x;
			//LOG("r %d %d %d %d %d %x\n",map,width, height,x,y, addr);
			return m_mem_read_cb(addr);
		case 4:
			wbytes = width * 2;
			addr += y * wbytes;
			addr += x * 2;
			if(endian)
				return m_mem_read_cb(addr + 1) | (m_mem_read_cb(addr) << 8);
			return m_mem_read_cb(addr) | (m_mem_read_cb(addr + 1) << 8);
		case 5:
			wbytes = width * 4;
			addr += y * wbytes;
			addr += x * 4;
			if(endian)
				return m_mem_read_cb(addr + 3) | (m_mem_read_cb(addr + 2) << 8) |
						(m_mem_read_cb(addr + 1) << 16) | (m_mem_read_cb(addr) << 24);
			return m_mem_read_cb(addr) | (m_mem_read_cb(addr + 1) << 8) |
					(m_mem_read_cb(addr + 2) << 16) | (m_mem_read_cb(addr + 3) << 24);
	}
	LOG("invalid pixel map mode %d %d\n", map, m_pelmap_format[map] & 7);
	return 0;
}

void xga_copro_device::write_map_pixel(int x, int y, int map, u32 pixel)
{
	offs_t addr = m_pelmap_base[map];
	int width = m_pelmap_width[map] + 1;
	int height = m_pelmap_height[map] + 1;
	int endian = m_pelmap_format[map] & 8;
	int wbytes;
	u8 byte, mask;
	if((x > width) || (y > height) || (x < 0) || (y < 0))
		return;
	switch(m_pelmap_format[map] & 7)
	{
		case 0:
			wbytes = width / 8;
			addr += y * wbytes;
			addr += x / 8;
			byte = m_mem_read_cb(addr);
			mask = 1 << ((x % 8) - (endian ? 8 : 0));
			byte = (byte & ~mask) | ((pixel ? 0xff : 0) & mask);
			m_mem_write_cb(addr, byte);
			break;
		case 1:
			wbytes = width / 4;
			addr += y * wbytes;
			addr += x / 4;
			byte = m_mem_read_cb(addr);
			mask = 3 << (((x % 4) - (endian ? 4 : 0)) * 2);
			byte = (byte & ~mask) | ((pixel ? 0xff : 0) & mask);
			m_mem_write_cb(addr, byte);
			break;
		case 2:
			wbytes = width / 2;
			addr += y * wbytes;
			addr += x / 2;
			byte = m_mem_read_cb(addr);
			mask = 0xf << (((x % 2) - (endian ? 2 : 0)) * 4);
			byte = (byte & ~mask) | ((pixel ? 0xff : 0) & mask);
			m_mem_write_cb(addr, byte);
			break;
		case 3:
			wbytes = width;
			addr += y * wbytes;
			addr += x;
			//LOG("w %d %d %d %d %d %x %x\n",map,width, height,x,y, addr, pixel);
			m_mem_write_cb(addr, (u8)pixel);
			break;
		case 4:
			wbytes = width * 2;
			addr += y * wbytes;
			addr += x * 2;
			if(endian)
			{
				m_mem_write_cb(addr + 1, pixel & 0xff);
				m_mem_write_cb(addr, pixel >> 8);
			}
			else
			{
				m_mem_write_cb(addr, pixel & 0xff);
				m_mem_write_cb(addr + 1, pixel >> 8);
			}
			break;
		case 5:
			wbytes = width * 4;
			addr += y * wbytes;
			addr += x * 4;
			if(endian)
			{
				m_mem_write_cb(addr + 3, pixel & 0xff);
				m_mem_write_cb(addr + 2, pixel >> 8);
				m_mem_write_cb(addr + 1, pixel >> 16);
				m_mem_write_cb(addr, pixel >> 24);
			}
			else
			{
				m_mem_write_cb(addr, pixel & 0xff);
				m_mem_write_cb(addr + 1, pixel >> 8);
				m_mem_write_cb(addr + 2, pixel >> 16);
				m_mem_write_cb(addr + 3, pixel >> 24);
			}
			break;
		default:
			LOG("invalid pixel map mode %d %d\n", map, m_pelmap_format[map] & 7);
			break;
	}
}

u32 xga_copro_device::rop(u32 s, u32 d, u8 op)
{
	if(m_var == TYPE::OTI111)
	{
		switch(op)
		{
			default:
			case 0:
				return 0;
			case 1:
				return ~s & ~d;
			case 2:
				return ~s & d;
			case 3:
				return ~s;
			case 4:
				return s & ~d;
			case 5:
				return ~d;
			case 6:
				return s ^ d;
			case 7:
				return ~s | ~d;
			case 8:
				return s & d;
			case 9:
				return s ^ ~d;
			case 10:
				return d;
			case 11:
				return ~s | d;
			case 12:
				return s;
			case 13:
				return s | ~d;
			case 14:
				return s | d;
			case 15:
				return -1;
		}
	}
	switch(op)
	{
		default:
		case 0:
			return 0;
		case 1:
			return s & d;
		case 2:
			return s & ~d;
		case 3:
			return s;
		case 4:
			return ~s & d;
		case 5:
			return d;
		case 6:
			return s ^ d;
		case 7:
			return s | d;
		case 8:
			return ~s & ~d;
		case 9:
			return s ^ ~d;
		case 10:
			return ~d;
		case 11:
			return s | ~d;
		case 12:
			return ~s;
		case 13:
			return ~s | d;
		case 14:
			return ~s | ~d;
		case 15:
			return -1;
		case 16:
			return std::max(s, d);
		case 17:
			return std::min(s, d);
		case 18:
			return 0; // saturate add
		case 19:
			return 0; // saturate sub d - s
		case 20:
			return 0; // saturate sub s - d
		case 21:
			return 0; // avg
	}
}

void xga_copro_device::do_pxblt()
{
	u8 dir = (m_pelop >> 25) & 2;
	int xstart, xend, xdir, ystart, yend, ydir;
	u8 srcmap = ((m_pelop >> 20) & 0xf) - 1;
	u8 dstmap = ((m_pelop >> 16) & 0xf) - 1;
	u8 patmap = ((m_pelop >> 12) & 0xf) - 1;
	LOG("pxblt src %d pat %d dst %d dim1 %d dim2 %d srcbase %x dstbase %x\n", srcmap+1, dstmap+1, patmap+1, m_opdim1 & 0xfff, m_opdim2 & 0xfff, m_pelmap_base[srcmap+1], m_pelmap_base[dstmap+1]);
	LOG("%d %d %d %d\n", m_srcxaddr & 0xfff, m_srcyaddr & 0xfff, m_dstxaddr & 0xfff, m_dstyaddr & 0xfff);
	if((srcmap > 2) || (dstmap > 2) || ((patmap > 2) && (patmap != 7) && (patmap != 8)))
	{
		LOG("invalid pelmap\n");
		return;
	}
	if(dir & 1)
	{
		ystart = (m_opdim2 & 0xfff) + 1;
		yend = 0;
		ydir = -1;
	}
	else
	{
		ystart = 0;
		yend = (m_opdim2 & 0xfff) + 1;
		ydir = 1;
	}
	if(dir & 2)
	{
		xstart = (m_opdim1 & 0xfff) + 1;
		xend = 0;
		xdir = -1;
	}
	else
	{
		xstart = 0;
		xend = (m_opdim1 & 0xfff) + 1;
		xdir = 1;
	}

	std::function<s16(s16)> dstwrap;
	if(m_var == TYPE::OTI111)
		dstwrap = [](s16 addr) { return addr & 0xfff; };
	else
	{
		dstwrap = [](s16 addr)
		{
			addr = addr & 0x1fff;
			return (addr & 0x1800) == 0x1800 ? addr | 0xf800 : addr;
		};
	}

	for(int y = ystart; y != yend; y += ydir)
	{
		u16 patxaddr = m_patxaddr & 0xfff;
		u16 srcxaddr = m_srcxaddr & 0xfff;
		s16 dstxaddr = dstwrap(m_dstxaddr);
		s16 dstyaddr = dstwrap(m_dstyaddr);
		for(int x = xstart; x != xend; x += xdir)
		{
			u32 src, dst, pat;
			if(patmap < 3)
			{
				pat = read_map_pixel(patxaddr, m_patyaddr & 0xfff, patmap + 1);
				patxaddr += xdir;
			}
			else
				pat = 1; //TODO: generate from source mode
			if(pat)
				src = (((m_pelop >> 28) & 3) == 2) ? read_map_pixel(srcxaddr, m_srcyaddr & 0xfff, srcmap + 1) : m_fcolor;
			else
				src = (((m_pelop >> 30) & 3) == 2) ? read_map_pixel(srcxaddr, m_srcyaddr & 0xfff, srcmap + 1) : m_bcolor;
			srcxaddr += xdir;
			dst = read_map_pixel(dstxaddr, dstyaddr, dstmap + 1);
			dst = (dst & ~m_pelbmask) | (rop(src, dst, pat ? m_fmix : m_bmix) & m_pelbmask);
			write_map_pixel(dstxaddr, dstyaddr, dstmap + 1, dst); // TODO: color compare
			dstxaddr = dstwrap(dstxaddr + xdir);
		}
		m_patyaddr += ydir;
		m_srcyaddr += ydir;
		m_dstyaddr += ydir;
	}
}

u8 xga_copro_device::xga_read(offs_t offset)
{
	switch(offset)
	{
		case 0x12:
			return m_pelmap;
		case 0x14:
			return m_pelmap_base[m_pelmap];
		case 0x15:
			return m_pelmap_base[m_pelmap] >> 8;
		case 0x16:
			return m_pelmap_base[m_pelmap] >> 16;
		case 0x17:
			return m_pelmap_base[m_pelmap] >> 24;
		case 0x18:
			return m_pelmap_width[m_pelmap];
		case 0x19:
			return m_pelmap_width[m_pelmap] >> 8;
		case 0x1a:
			return m_pelmap_height[m_pelmap];
		case 0x1b:
			return m_pelmap_height[m_pelmap] >> 8;
		case 0x1c:
			return m_pelmap_format[m_pelmap];
		case 0x20:
			return m_bresh_err;
		case 0x21:
			return m_bresh_err >> 8;
		case 0x24:
			return m_bresh_k1;
		case 0x25:
			return m_bresh_k1 >> 8;
		case 0x28:
			return m_bresh_k2;
		case 0x29:
			return m_bresh_k2 >> 8;
		case 0x2c:
			return m_dir;
		case 0x2d:
			return m_dir >> 8;
		case 0x2e:
			return m_dir >> 16;
		case 0x2f:
			return m_dir >> 24;
		case 0x48:
			if(m_var == TYPE::OTI111)
				return m_fmix << 4 | m_bmix;
			return m_fmix;
		case 0x49:
			if(m_var == TYPE::OTI111)
				return 0;
			return m_bmix;
		case 0x4a:
			return m_destccc;
		case 0x4c:
			return m_destccv;
		case 0x4d:
			return m_destccv >> 8;
		case 0x4e:
			return m_destccv >> 16;
		case 0x4f:
			return m_destccv >> 24;
		case 0x50:
			return m_pelbmask;
		case 0x51:
			return m_pelbmask >> 8;
		case 0x52:
			return m_pelbmask >> 16;
		case 0x53:
			return m_pelbmask >> 24;
		case 0x54:
			return m_carrychain;
		case 0x55:
			return m_carrychain >> 8;
		case 0x56:
			return m_carrychain >> 16;
		case 0x57:
			return m_carrychain >> 24;
		case 0x58:
			return m_fcolor;
		case 0x59:
			return m_fcolor >> 8;
		case 0x5a:
			return m_fcolor >> 16;
		case 0x5b:
			return m_fcolor >> 24;
		case 0x5c:
			return m_bcolor;
		case 0x5d:
			return m_bcolor >> 8;
		case 0x5e:
			return m_bcolor >> 16;
		case 0x5f:
			return m_bcolor >> 24;
		case 0x60:
			return m_opdim1;
		case 0x61:
			return m_opdim1 >> 8;
		case 0x62:
			return m_opdim2;
		case 0x63:
			return m_opdim2 >> 8;
		case 0x6c:
			return m_maskorigx;
		case 0x6d:
			return m_maskorigx >> 8;
		case 0x6e:
			return m_maskorigy;
		case 0x6f:
			return m_maskorigy >> 8;
		case 0x70:
			return m_srcxaddr;
		case 0x71:
			return m_srcxaddr >> 8;
		case 0x72:
			return m_srcyaddr;
		case 0x73:
			return m_srcyaddr >> 8;
		case 0x74:
			return m_patxaddr;
		case 0x75:
			return m_patxaddr >> 8;
		case 0x76:
			return m_patyaddr;
		case 0x77:
			return m_patyaddr >> 8;
		case 0x78:
			return m_dstxaddr;
		case 0x79:
			return m_dstxaddr >> 8;
		case 0x7a:
			return m_dstyaddr;
		case 0x7b:
			return m_dstyaddr >> 8;
		case 0x7c:
			return m_pelop;
		case 0x7d:
			return m_pelop >> 8;
		case 0x7e:
			return m_pelop >> 16;
		case 0x7f:
			return m_pelop >> 24;
	}
	return 0;
}

void xga_copro_device::xga_write(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0x12:
			m_pelmap = data & 3;
			break;
		case 0x14:
			m_pelmap_base[m_pelmap] = (m_pelmap_base[m_pelmap] & ~0xff) | data;
			break;
		case 0x15:
			m_pelmap_base[m_pelmap] = (m_pelmap_base[m_pelmap] & ~0xff00) | (data << 8);
			break;
		case 0x16:
			m_pelmap_base[m_pelmap] = (m_pelmap_base[m_pelmap] & ~0xff0000) | (data << 16);
			break;
		case 0x17:
			m_pelmap_base[m_pelmap] = (m_pelmap_base[m_pelmap] & ~0xff000000) | (data << 24);
			break;
		case 0x18:
			m_pelmap_width[m_pelmap] = (m_pelmap_width[m_pelmap] & ~0xff) | data;
			break;
		case 0x19:
			m_pelmap_width[m_pelmap] = (m_pelmap_width[m_pelmap] & ~0xff00) | (data << 8);
			break;
		case 0x1a:
			m_pelmap_height[m_pelmap] = (m_pelmap_height[m_pelmap] & ~0xff) | data;
			break;
		case 0x1b:
			m_pelmap_height[m_pelmap] = (m_pelmap_height[m_pelmap] & ~0xff00) | (data << 8);
			break;
		case 0x1c:
			m_pelmap_format[m_pelmap] = data;
			break;
		case 0x20:
			m_bresh_err = (m_bresh_err & ~0xff) | data;
			break;
		case 0x21:
			m_bresh_err = (m_bresh_err & ~0xff00) | (data << 8);
			break;
		case 0x24:
			m_bresh_k1 = (m_bresh_k1 & ~0xff) | data;
			break;
		case 0x25:
			m_bresh_k1 = (m_bresh_k1 & ~0xff00) | (data << 8);
			break;
		case 0x28:
			m_bresh_k2 = (m_bresh_k2 & ~0xff) | data;
			break;
		case 0x29:
			m_bresh_k2 = (m_bresh_k2 & ~0xff00) | (data << 8);
			break;
		case 0x2c:
			m_dir = (m_dir & ~0xff) | data;
			break;
		case 0x2d:
			m_dir = (m_dir & ~0xff00) | (data << 8);
			break;
		case 0x2e:
			m_dir = (m_dir & ~0xff0000) | (data << 16);
			break;
		case 0x2f:
			m_dir = (m_dir & ~0xff000000) | (data << 24);
			break;
		case 0x48:
			if(m_var == TYPE::OTI111)
			{
				m_fmix = data >> 4;
				m_bmix = data & 0xf;
				break;
			}
			m_fmix = data;
			break;
		case 0x49:
			if(m_var == TYPE::OTI111)
				break;
			m_bmix = data;
			break;
		case 0x4a:
			m_destccc = data;
			break;
		case 0x4c:
			m_destccv = (m_destccv & ~0xff) | data;
			break;
		case 0x4d:
			m_destccv = (m_destccv & ~0xff00) | (data << 8);
			break;
		case 0x4e:
			m_destccv = (m_destccv & ~0xff0000) | (data << 16);
			break;
		case 0x4f:
			m_destccv = (m_destccv & ~0xff000000) | (data << 24);
			break;
		case 0x50:
			m_pelbmask = (m_pelbmask & ~0xff) | data;
			break;
		case 0x51:
			m_pelbmask = (m_pelbmask & ~0xff00) | (data << 8);
			break;
		case 0x52:
			m_pelbmask = (m_pelbmask & ~0xff0000) | (data << 16);
			break;
		case 0x53:
			m_pelbmask = (m_pelbmask & ~0xff000000) | (data << 24);
			break;
		case 0x54:
			m_carrychain = (m_carrychain & ~0xff) | data;
			break;
		case 0x55:
			m_carrychain = (m_carrychain & ~0xff00) | (data << 8);
			break;
		case 0x56:
			m_carrychain = (m_carrychain & ~0xff0000) | (data << 16);
			break;
		case 0x57:
			m_carrychain = (m_carrychain & ~0xff000000) | (data << 24);
			break;
		case 0x58:
			m_fcolor = (m_fcolor & ~0xff) | data;
			break;
		case 0x59:
			m_fcolor = (m_fcolor & ~0xff00) | (data << 8);
			break;
		case 0x5a:
			m_fcolor = (m_fcolor & ~0xff0000) | (data << 16);
			break;
		case 0x5b:
			m_fcolor = (m_fcolor & ~0xff000000) | (data << 24);
			break;
		case 0x5c:
			m_bcolor = (m_bcolor & ~0xff) | data;
			break;
		case 0x5d:
			m_bcolor = (m_bcolor & ~0xff00) | (data << 8);
			break;
		case 0x5e:
			m_bcolor = (m_bcolor & ~0xff0000) | (data << 16);
			break;
		case 0x5f:
			m_bcolor = (m_bcolor & ~0xff000000) | (data << 24);
			break;
		case 0x60:
			m_opdim1 = (m_opdim1 & ~0xff) | data;
			break;
		case 0x61:
			m_opdim1 = (m_opdim1 & ~0xff00) | (data << 8);
			break;
		case 0x62:
			m_opdim2 = (m_opdim2 & ~0xff) | data;
			break;
		case 0x63:
			m_opdim2 = (m_opdim2 & ~0xff00) | (data << 8);
			break;
		case 0x6c:
			m_maskorigx = (m_maskorigx & ~0xff) | data;
			break;
		case 0x6d:
			m_maskorigx = (m_maskorigx & ~0xff00) | (data << 8);
			break;
		case 0x6e:
			m_maskorigy = (m_maskorigy & ~0xff) | data;
			break;
		case 0x6f:
			m_maskorigy = (m_maskorigy & ~0xff00) | (data << 8);
			break;
		case 0x70:
			m_srcxaddr = (m_srcxaddr & ~0xff) | data;
			break;
		case 0x71:
			m_srcxaddr = (m_srcxaddr & ~0xff00) | (data << 8);
			break;
		case 0x72:
			m_srcyaddr = (m_srcyaddr & ~0xff) | data;
			break;
		case 0x73:
			m_srcyaddr = (m_srcyaddr & ~0xff00) | (data << 8);
			break;
		case 0x74:
			m_patxaddr = (m_patxaddr & ~0xff) | data;
			break;
		case 0x75:
			m_patxaddr = (m_patxaddr & ~0xff00) | (data << 8);
			break;
		case 0x76:
			m_patyaddr = (m_patyaddr & ~0xff) | data;
			break;
		case 0x77:
			m_patyaddr = (m_patyaddr & ~0xff00) | (data << 8);
			break;
		case 0x78:
			m_dstxaddr = (m_dstxaddr & ~0xff) | data;
			break;
		case 0x79:
			m_dstxaddr = (m_dstxaddr & ~0xff00) | (data << 8);
			break;
		case 0x7a:
			m_dstyaddr = (m_dstyaddr & ~0xff) | data;
			break;
		case 0x7b:
			m_dstyaddr = (m_dstyaddr & ~0xff00) | (data << 8);
			break;
		case 0x7c:
			m_pelop = (m_pelop & ~0xff) | data;
			break;
		case 0x7d:
			m_pelop = (m_pelop & ~0xff00) | (data << 8);
			break;
		case 0x7e:
			m_pelop = (m_pelop & ~0xff0000) | (data << 16);
			break;
		case 0x7f:
			m_pelop = (m_pelop & ~0xff000000) | (data << 24);
			start_command();
			break;
	}
}

void xga_copro_device::device_start()
{
}

void xga_copro_device::device_reset()
{
	m_pelmap = 0;
}
