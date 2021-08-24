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

using namespace voodoo;



//**************************************************************************
//  INTERNAL CLASSES
//**************************************************************************

//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void banshee_2d_regs::register_save(save_proxy &save)
{
	save.save_item(NAME(m_regs));
}


//-------------------------------------------------
//  s_names - table of register names
//-------------------------------------------------

char const *const banshee_2d_regs::s_names[0x20] =
{
	"reserved00",       "reserved04",       "clip0Min",         "clip0Max",
	"dstBaseAddr",      "dstFormat",        "srcColorkeyMin",   "srcColorkeyMax",
	"dstColorkeyMin",   "dstColorkeyMax",   "bresError0",       "bresError1",
	"rop",              "srcBaseAddr",      "commandExtra",     "lineStipple",
	"lineStyle",        "pattern0Alias",    "pattern1Alias",    "clip1Min",
	"clip1Max",         "srcFormat",        "srcSize",          "srcXY",
	"colorBack",        "colorFore",        "dstSize",          "dstXY",
	"command",          "reserved74",       "reserved78",       "reserved7c"
};


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void banshee_io_regs::register_save(save_proxy &save)
{
	save.save_item(NAME(m_regs));
}


//-------------------------------------------------
//  s_names - table of register names
//-------------------------------------------------

char const *const banshee_io_regs::s_names[0x40] =
{
	"status",                   "pciInit0",            "sipMonitor",                   "lfbMemoryConfig",
	"miscInit0",                "miscInit1",           "dramInit0",                    "dramInit1",
	"agpInit",                  "tmuGbeInit",          "vgaInit0",                     "vgaInit1",
	"dramCommand",              "dramData",            "reserved38",                   "reserved3c",
	"pllCtrl0",                 "pllCtrl1",            "pllCtrl2",                     "dacMode",
	"dacAddr",                  "dacData",             "rgbMaxDelta",                  "vidProcCfg",
	"hwCurPatAddr",             "hwCurLoc",            "hwCurC0",                      "hwCurC1",
	"vidInFormat",              "vidInStatus",         "vidSerialParallelPort",        "vidInXDecimDeltas",
	"vidInDecimInitErrs",       "vidInYDecimDeltas",   "vidPixelBufThold",             "vidChromaMin",
	"vidChromaMax",             "vidCurrentLine",      "vidScreenSize",                "vidOverlayStartCoords",
	"vidOverlayEndScreenCoord", "vidOverlayDudx",      "vidOverlayDudxOffsetSrcWidth", "vidOverlayDvdy",
	"vga[b0]",                  "vga[b4]",             "vga[b8]",                      "vga[bc]",
	"vga[c0]",                  "vga[c4]",             "vga[c8]",                      "vga[cc]",
	"vga[d0]",                  "vga[d4]",             "vga[d8]",                      "vga[dc]",
	"vidOverlayDvdyOffset",     "vidDesktopStartAddr", "vidDesktopOverlayStride",      "vidInAddr0",
	"vidInAddr1",               "vidInAddr2",          "vidInStride",                  "vidCurrOverlayStartAddr"
};


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void banshee_cmd_agp_regs::register_save(save_proxy &save)
{
	save.save_item(NAME(m_regs));
}


//-------------------------------------------------
//  s_names - table of register names
//-------------------------------------------------

char const *const banshee_cmd_agp_regs::s_names[0x80] =
{
	"agpReqSize",        "agpHostAddressLow", "agpHostAddressHigh", "agpGraphicsAddress",
	"agpGraphicsStride", "agpMoveCMD",        "reserved18",         "reserved1c",
	"cmdBaseAddr0",      "cmdBaseSize0",      "cmdBump0",           "cmdRdPtrL0",
	"cmdRdPtrH0",        "cmdAMin0",          "reserved38",         "cmdAMax0",
	"reserved40",        "cmdFifoDepth0",     "cmdHoleCnt0",        "reserved4c",
	"cmdBaseAddr1",      "cmdBaseSize1",      "cmdBump1",           "cmdRdPtrL1",
	"cmdRdPtrH1",        "cmdAMin1",          "reserved68",         "cmdAMax1",
	"reserved70",        "cmdFifoDepth1",     "cmdHoleCnt1",        "reserved7c",
	"cmdFifoThresh",     "cmdHoleInt",        "reserved88",         "reserved8c",
	"reserved90",        "reserved94",        "reserved98",         "reserved9c",
	"reserveda0",        "reserveda4",        "reserveda8",         "reservedac",
	"reservedb0",        "reservedb4",        "reservedb8",         "reservedbc",
	"reservedc0",        "reservedc4",        "reservedc8",         "reservedcc",
	"reservedd0",        "reservedd4",        "reservedd8",         "reserveddc",
	"reservede0",        "reservede4",        "reservede8",         "reservedec",
	"reservedf0",        "reservedf4",        "reservedf8",         "reservedfc",
	"yuvBaseAddress",    "yuvStride",         "reserved108",        "reserved10c",
	"reserved110",       "reserved114",       "reserved118",        "reserved11c",
	"crc1",              "reserved124",       "reserved128",        "reserved12c",
	"crc2",              "reserved134",       "reserved138",        "reserved13c",
	"reserved140",       "reserved144",       "reserved148",        "reserved14c",
	"reserved150",       "reserved154",       "reserved158",        "reserved15c",
	"reserved160",       "reserved164",       "reserved168",        "reserved16c",
	"reserved170",       "reserved174",       "reserved178",        "reserved17c",
	"reserved180",       "reserved184",       "reserved188",        "reserved18c",
	"reserved190",       "reserved194",       "reserved198",        "reserved19c",
	"reserved1a0",       "reserved1a4",       "reserved1a8",        "reserved1ac",
	"reserved1b0",       "reserved1b4",       "reserved1b8",        "reserved1bc",
	"reserved1c0",       "reserved1c4",       "reserved1c8",        "reserved1cc",
	"reserved1d0",       "reserved1d4",       "reserved1d8",        "reserved1dc",
	"reserved1e0",       "reserved1e4",       "reserved1e8",        "reserved1ec",
	"reserved1f0",       "reserved1f4",       "reserved1f8",        "reserved1fc"
};


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void banshee_vga_regs::register_save(save_proxy &save)
{
	save.save_item(NAME(m_regs));
	save.save_item(NAME(m_crtc));
	save.save_item(NAME(m_seq));
	save.save_item(NAME(m_gc));
	save.save_item(NAME(m_attr));
	save.save_item(NAME(m_attr_flip_flop));
}



//**************************************************************************
//  VOODOO BANSHEE DEVICE
//**************************************************************************

//-------------------------------------------------
//  voodoo_banshee_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device, "voodoo_banshee", "3dfx Voodoo Banshee")

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	voodoo_2_device(mconfig, type, tag, owner, clock, model),
	m_cmdfifo2(*this)
{
	for (int index = 0; index < std::size(m_regtable); index++)
		m_regtable[index].unpack(s_register_table[index], *this);
}


//-------------------------------------------------
//  core_map - device map for core memory access
//-------------------------------------------------

void voodoo_banshee_device::core_map(address_map &map)
{
	// Voodoo Banshee/Voodoo 3 memory map:
	//
	//   0`00000xxx`xxxxxxxx`xxxxxxxx I/O register remap
	//   0`00001xxx`xxxxxxxx`xxxxxxxx CMD/AGP transfer/Misc registers
	//   0`0001xxxx`xxxxxxxx`xxxxxxxx 2D registers
	//   0`001xxxxx`xxxxxxxx`xxxxxxxx 3D registers
	//   0`010xxxxx`xxxxxxxx`xxxxxxxx 3D registers (cont'd)
	//   0`011xxxxx`xxxxxxxx`xxxxxxxx Texture TMU 0 download
	//   0`100xxxxx`xxxxxxxx`xxxxxxxx Texture TMU 1 download (Voodoo 3 only)
	//   0`101xxxxx`xxxxxxxx`xxxxxxxx Reserved
	//   0`11xxxxxx`xxxxxxxx`xxxxxxxx YUV planar space
	//   1`xxxxxxxx`xxxxxxxx`xxxxxxxx 3D LFB space
	//
	map(0x0000000, 0x007ffff).rw(FUNC(voodoo_banshee_device::map_io_r), FUNC(voodoo_banshee_device::map_io_w));
	map(0x0080000, 0x00fffff).rw(FUNC(voodoo_banshee_device::map_cmd_agp_r), FUNC(voodoo_banshee_device::map_cmd_agp_w));
	map(0x0100000, 0x01fffff).rw(FUNC(voodoo_banshee_device::map_2d_r), FUNC(voodoo_banshee_device::map_2d_w));
	map(0x0200000, 0x05fffff).rw(FUNC(voodoo_banshee_device::map_register_r), FUNC(voodoo_banshee_device::map_register_w));
	map(0x0600000, 0x07fffff).w(FUNC(voodoo_banshee_device::map_texture_w<0>));
	if (BIT(m_chipmask, 2))
		map(0x0800000, 0x09fffff).w(FUNC(voodoo_banshee_device::map_texture_w<1>));
	map(0x0c00000, 0x0ffffff).w(FUNC(voodoo_banshee_device::map_yuv_w));
	map(0x1000000, 0x1ffffff).w(FUNC(voodoo_banshee_device::map_lfb_w));
}


//-------------------------------------------------
//  read - generic read handler until everyone is
//  using the memory map
//-------------------------------------------------

u32 voodoo_banshee_device::read(offs_t offset, u32 mem_mask)
{
	switch (offset >> (19-2))
	{
	case 0x0000000 >> 19:
		return map_io_r(offset - 0x0000000/4, mem_mask);

	case 0x0080000 >> 19:
		return map_cmd_agp_r(offset - 0x0080000/4);

	case 0x0100000 >> 19:   case 0x0180000 >> 19:
		return map_2d_r(offset - 0x0100000/4);

	case 0x0200000 >> 19:   case 0x0280000 >> 19:   case 0x0300000 >> 19:   case 0x0380000 >> 19:
	case 0x0400000 >> 19:   case 0x0480000 >> 19:   case 0x0500000 >> 19:   case 0x0580000 >> 19:
		return map_register_r(offset - 0x0200000/4);

	default:
		logerror("%s:voodoo_banshee_device::read Address out of range %08X & %08X\n", machine().describe_context(), offset*4, mem_mask);
		return 0xffffffff;
	}
}


//-------------------------------------------------
//  write - generic write handler until everyone is
//  using the memory map
//-------------------------------------------------

void voodoo_banshee_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> (19-2))
	{
	case 0x0000000 >> 19:
		map_io_w(offset - 0x0000000/4, data, mem_mask);
		break;

	case 0x0080000 >> 19:
		map_cmd_agp_w(offset - 0x0080000/4, data, mem_mask);
		break;

	case 0x0100000 >> 19:   case 0x0180000 >> 19:
		map_2d_w(offset - 0x0100000/4, data, mem_mask);
		break;

	case 0x0200000 >> 19:   case 0x0280000 >> 19:   case 0x0300000 >> 19:   case 0x0380000 >> 19:
	case 0x0400000 >> 19:   case 0x0480000 >> 19:   case 0x0500000 >> 19:   case 0x0580000 >> 19:
		map_register_w(offset - 0x0200000/4, data, mem_mask);
		break;

	case 0x0600000 >> 19:   case 0x0680000 >> 19:   case 0x0700000 >> 19:   case 0x0780000 >> 19:
		map_texture_w<0>(offset - 0x0600000, data, mem_mask);
		break;

	case 0x0800000 >> 19:   case 0x0880000 >> 19:   case 0x0900000 >> 19:   case 0x0980000 >> 19:
		if (BIT(m_chipmask, 2))
			map_texture_w<1>(offset - 0x0800000, data, mem_mask);
		break;

	case 0xc000000 >> 19:   case 0xc800000 >> 19:   case 0xd000000 >> 19:   case 0xd800000 >> 19:
	case 0xe000000 >> 19:   case 0xe800000 >> 19:   case 0xf000000 >> 19:   case 0xf800000 >> 19:
		map_yuv_w(offset - 0xc000000/4, data, mem_mask);
		break;

	case 0x1000000 >> 19:   case 0x1080000 >> 19:   case 0x1100000 >> 19:   case 0x1180000 >> 19:
	case 0x1200000 >> 19:   case 0x1280000 >> 19:   case 0x1300000 >> 19:   case 0x1380000 >> 19:
	case 0x1400000 >> 19:   case 0x1480000 >> 19:   case 0x1500000 >> 19:   case 0x1580000 >> 19:
	case 0x1600000 >> 19:   case 0x1680000 >> 19:   case 0x1700000 >> 19:   case 0x1780000 >> 19:
	case 0x1800000 >> 19:   case 0x1880000 >> 19:   case 0x1900000 >> 19:   case 0x1980000 >> 19:
	case 0x1a00000 >> 19:   case 0x1a80000 >> 19:   case 0x1b00000 >> 19:   case 0x1b80000 >> 19:
	case 0x1c00000 >> 19:   case 0x1c80000 >> 19:   case 0x1d00000 >> 19:   case 0x1d80000 >> 19:
	case 0x1e00000 >> 19:   case 0x1e80000 >> 19:   case 0x1f00000 >> 19:   case 0x1f80000 >> 19:
		map_lfb_w(offset - 0x1000000/4, data, mem_mask);
		break;

	default:
		logerror("%s:voodoo_banshee_device::write Address out of range %08X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
		break;
	}
}


//-------------------------------------------------
//  lfb_map - device map for LFB space
//-------------------------------------------------

void voodoo_banshee_device::lfb_map(address_map &map)
{
	map(0x0000000, 0x1ffffff).rw(FUNC(voodoo_banshee_device::read_lfb), FUNC(voodoo_banshee_device::write_lfb));
}


//-------------------------------------------------
//  read_lfb - generic LFB space read handler
//-------------------------------------------------

u32 voodoo_banshee_device::read_lfb(offs_t offset, u32 mem_mask)
{
	prepare_for_read();

	// above LFB base goes the traditional route
	if (offset >= m_lfb_base)
		return internal_lfb_r(offset - m_lfb_base);

	// reads below the LFB base are direct?
	u32 addr = offset * 4;
	if (addr <= m_fbmask)
	{
		m_renderer->wait("read_lfb");
		u32 result = *(u32 *)&m_fbram[addr];
		if (LOG_LFB)
			logerror("%s:read_lfb(%X) = %08X\n", machine().describe_context(), addr, result);
		return result;
	}

	// log out-of-bounds accesses
	logerror("%s:read_lfb(%X) Access out of bounds\n", machine().describe_context(), addr);
	return 0xffffffff;
}


//-------------------------------------------------
//  write_lfb - generic LFB space write handler
//-------------------------------------------------

void voodoo_banshee_device::write_lfb(offs_t offset, u32 data, u32 mem_mask)
{
	prepare_for_write();

	// above LFB base goes the traditional route
	if (offset >= m_lfb_base)
		return internal_lfb_direct_w(offset - m_lfb_base, data, mem_mask);

	// could be writing to cmdfifo space
	u32 addr = offset * 4;
	if (m_cmdfifo.write_if_in_range(addr, data) || m_cmdfifo2.write_if_in_range(addr, data))
		return;

	// writes below the LFB base are direct?
	if (addr <= m_fbmask)
	{
		m_renderer->wait("write_lfb");
		if (LOG_LFB)
			logerror("%s:write_lfb(%X) = %08X & %08X\n", machine().describe_context(), addr, data, mem_mask);
		COMBINE_DATA((u32 *)&m_fbram[addr]);
	}
	else
		logerror("%s:write_lfb Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), addr, data, mem_mask);
}


//-------------------------------------------------
//  io_map - device map for I/O space
//-------------------------------------------------

void voodoo_banshee_device::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(voodoo_banshee_device::map_io_r), FUNC(voodoo_banshee_device::map_io_w));
}


//-------------------------------------------------
//  update - video update
//-------------------------------------------------

int voodoo_banshee_device::update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if bypassing the clut, don't worry about the rest
	u32 config = m_io_regs.read(banshee_io_regs::vidProcCfg);
	if (BIT(config, 11))
		return update_common(bitmap, cliprect, m_shared->rgb565);

	// if the CLUT is dirty, recompute the pens array
	if (m_clut_dirty)
	{
		rgb_t const *clutbase = &m_clut[256 * BIT(config, 13)];

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
			m_pen[pen] = rgb_t(rtable[BIT(pen, 11, 5)], gtable[BIT(pen, 5, 6)], btable[BIT(pen, 0, 5)]);

		// no longer dirty
		m_clut_dirty = false;
		m_video_changed = true;
	}
	return update_common(bitmap, cliprect, &m_pen[0]);
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void voodoo_banshee_device::device_start()
{
	// expand CLUT to 512 entries
	m_clut.resize(512);

	// pre-set the chipmask and clear the TMU memory to indicate we're shared
	if (m_chipmask == 0x01)
		m_chipmask = 0x03;
	m_tmumem0_in_mb = m_tmumem1_in_mb = 0;

	// start like a Voodoo-2
	voodoo_2_device::device_start();

	// texture base address/shift is different compared to previous versions
	m_tmu[0].set_baseaddr_mask_shift(0xfffff0, 0);
	m_tmu[1].set_baseaddr_mask_shift(0xfffff0, 0);

	// initialize the second cmdfifo
	m_cmdfifo2.init(m_fbram, m_fbmask + 1);

	// LFB stride defaualts to 11 on Banshee and later
	m_lfb_stride = 11;

	// 512-entry CLUT on Banshee and later
	for (int pen = 0; pen < 512; pen++)
		m_clut[pen] = rgb_t(pen, pen, pen);

	// initialize banshee registers
	m_io_regs.reset();
	m_io_regs.write(banshee_io_regs::pciInit0, 0x01800040);
	m_io_regs.write(banshee_io_regs::sipMonitor, 0x40000000);
	m_io_regs.write(banshee_io_regs::lfbMemoryConfig, 0x000a2200);
	u32 dram0 = 0x00579d29;
	if (m_fbmem_in_mb == 16)
		dram0 |= 0x0c000000;      // Midway Vegas (denver) expects 2 banks of 16MBit SGRAMs
	else
		dram0 |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	m_io_regs.write(banshee_io_regs::dramInit0, dram0);
	m_io_regs.write(banshee_io_regs::dramInit1, 0x00f02200);
	m_io_regs.write(banshee_io_regs::tmuGbeInit, 0x00000bfb);
}


//-------------------------------------------------
//  soft_reset - handle reset when initiated by
//  a register write
//-------------------------------------------------

void voodoo_banshee_device::soft_reset()
{
	voodoo_2_device::soft_reset();
	m_cmdfifo2.set_enable(0);
}


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void voodoo_banshee_device::register_save(save_proxy &save, u32 total_allocation)
{
	voodoo_2_device::register_save(save, total_allocation);

	// Voodoo Banshee stuff
	save.save_class(NAME(m_cmdfifo2));
	save.save_class(NAME(m_io_regs));
	save.save_class(NAME(m_cmd_agp_regs));
	save.save_class(NAME(m_vga_regs));
	save.save_class(NAME(m_2d_regs));
	save.save_item(NAME(m_blt_dst_base));
	save.save_item(NAME(m_blt_dst_x));
	save.save_item(NAME(m_blt_dst_y));
	save.save_item(NAME(m_blt_dst_width));
	save.save_item(NAME(m_blt_dst_height));
	save.save_item(NAME(m_blt_dst_stride));
	save.save_item(NAME(m_blt_dst_bpp));
	save.save_item(NAME(m_blt_cmd));
	save.save_item(NAME(m_blt_src_base));
	save.save_item(NAME(m_blt_src_x));
	save.save_item(NAME(m_blt_src_y));
	save.save_item(NAME(m_blt_src_width));
	save.save_item(NAME(m_blt_src_height));
	save.save_item(NAME(m_blt_src_stride));
	save.save_item(NAME(m_blt_src_bpp));
}


//-------------------------------------------------
//  lfb_buffer_indirect - return the buffer base
//  for LFB accesses via the classic 3D means
//-------------------------------------------------

u16 *voodoo_banshee_device::lfb_buffer_indirect(int index)
{
	// LFB access is configured via I/O registers
	return (u16 *)(m_fbram + m_lfb_base * 4);
}


//-------------------------------------------------
//  draw_buffer_indirect - return the buffer base
//  for drawing; on Banshee this is always the
//  back buffer
//-------------------------------------------------

u16 *voodoo_banshee_device::draw_buffer_indirect(int index)
{
	// drawing is confined to the back buffer
	return back_buffer();
}


//-------------------------------------------------
//  map_io_r - handle a mapped read from I/O
//  space
//-------------------------------------------------

u32 voodoo_banshee_device::map_io_r(offs_t offset, u32 mem_mask)
{
	prepare_for_read();
	return internal_io_r(offset, mem_mask);
}


//-------------------------------------------------
//  map_cmd_agp_r - handle a mapped read from CMD/
//  AGP space
//-------------------------------------------------

u32 voodoo_banshee_device::map_cmd_agp_r(offs_t offset)
{
	prepare_for_read();
	return internal_cmd_agp_r(offset);
}


//-------------------------------------------------
//  map_2d_r - handle a mapped read from 2D space
//-------------------------------------------------

u32 voodoo_banshee_device::map_2d_r(offs_t offset)
{
	prepare_for_read();
	logerror("%s:map_2d_r(%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	return 0xffffffff;
}


//-------------------------------------------------
//  map_register_r - handle a mapped read from
//  regular register space
//-------------------------------------------------

u32 voodoo_banshee_device::map_register_r(offs_t offset)
{
	prepare_for_read();

	// extract chipmask and register
	u32 chipmask = BIT(offset, 8, 4);
	if (chipmask == 0)
		chipmask = 0xf;
	chipmask &= m_chipmask;
	u32 regnum = BIT(offset, 0, 8);

	// look up the register
	return m_regtable[regnum].read(*this, chipmask, regnum);
}


//-------------------------------------------------
//  map_io_w - handle a mapped write to I/O space
//-------------------------------------------------

void voodoo_banshee_device::map_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	// no mechanism to stall I/O writes
	prepare_for_write();
	internal_io_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  map_agp_cmd_w - handle a mapped write to CMD/
//  AGP space
//-------------------------------------------------

void voodoo_banshee_device::map_cmd_agp_w(offs_t offset, u32 data, u32 mem_mask)
{
	// no mechanism to stall AGP/CMD writes
	prepare_for_write();
	internal_cmd_agp_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  map_2d_w - handle a mapped write to 2D space
//-------------------------------------------------

void voodoo_banshee_device::map_2d_w(offs_t offset, u32 data, u32 mem_mask)
{
	// no mechanism to stall 2D writes
	prepare_for_write();
	logerror("%s:map_2d_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}


//-------------------------------------------------
//  map_register_w - handle a mapped write to
//  regular register space
//-------------------------------------------------

void voodoo_banshee_device::map_register_w(offs_t offset, u32 data, u32 mem_mask)
{
	bool pending = prepare_for_write();

	// extract chipmask and register
	u32 chipmask = BIT(offset, 8, 4);
	if (chipmask == 0)
		chipmask = 0xf;
	chipmask &= m_chipmask;
	u32 regnum = BIT(offset, 0, 8);

	// handle register swizzling
	if (BIT(offset, 20-2))
		data = swapendian_int32(data);

	// handle aliasing
	if (BIT(offset, 21-2))
		regnum = voodoo_regs::alias(regnum);

	// look up the register
	auto const &regentry = m_regtable[regnum];

	// if this is non-FIFO command, execute immediately
	if (!regentry.is_fifo())
		return void(regentry.write(*this, chipmask, regnum, data));

	// track swap buffers
	if (regnum == voodoo_regs::reg_swapbufferCMD)
		m_swaps_pending++;

	// if we're busy add to the fifo
	if (pending && m_init_enable.enable_pci_fifo())
		return add_to_fifo((chipmask << 4) | regnum | memory_fifo::TYPE_REGISTER, data, mem_mask);

	// if we get a non-zero number of cycles back, mark things pending
	int cycles = regentry.write(*this, chipmask, regnum, data);
	if (cycles > 0)
	{
		// if we ended up with cycles, mark the operation pending
		m_operation_end = machine().time() + clocks_to_attotime(cycles);
		if (LOG_FIFO_VERBOSE)
			logerror("VOODOO.FIFO:direct write start at %s end at %s\n", machine().time().as_string(18), m_operation_end.as_string(18));
	}
}


//-------------------------------------------------
//  map_texture_w - handle a mapped write to
//  texture download space
//-------------------------------------------------

template<int Which>
void voodoo_banshee_device::map_texture_w(offs_t offset, u32 data, u32 mem_mask)
{
	prepare_for_write();
	logerror("%s:map_texture_w<%d>(%X) = %08X & %08X\n", machine().describe_context(), Which, offset*4, data, mem_mask);
}


//-------------------------------------------------
//  map_yuv_w - handle a mapped write to YUV space
//-------------------------------------------------

void voodoo_banshee_device::map_yuv_w(offs_t offset, u32 data, u32 mem_mask)
{
	prepare_for_write();
	logerror("%s:map_yuv_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}


//-------------------------------------------------
//  map_lfb_w - handle a mapped write to LFB space
//-------------------------------------------------

void voodoo_banshee_device::map_lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if we're busy add to the fifo, else just execute immediately
	if (prepare_for_write() && m_init_enable.enable_pci_fifo())
		add_to_fifo(offset | (0x400000/4), data, mem_mask);
	else
		internal_lfb_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  internal_io_r - handle reads from the I/O
//  registers
//-------------------------------------------------

u32 voodoo_banshee_device::internal_io_r(offs_t offset, u32 mem_mask)
{
	// by default just return regular data
	offset &= 0x3f;
	u32 result = m_io_regs.read(offset);

	// handle special offsets
	switch (offset)
	{
		// status reflects the voodoo status
		case banshee_io_regs::status:
			result = reg_status_r(0, 0);
			break;

		// DAC data reads fetch data from the CLUT
		case banshee_io_regs::dacData:
			result = m_clut[BIT(m_io_regs.read(banshee_io_regs::dacAddr), 0, 9)];
			break;

		// VGA reads are special and byte-wise
		case banshee_io_regs::vgab0: case banshee_io_regs::vgab4: case banshee_io_regs::vgab8: case banshee_io_regs::vgabc:
		case banshee_io_regs::vgac0: case banshee_io_regs::vgac4: case banshee_io_regs::vgac8: case banshee_io_regs::vgacc:
		case banshee_io_regs::vgad0: case banshee_io_regs::vgad4: case banshee_io_regs::vgad8: case banshee_io_regs::vgadc:
			result = 0;
			if (ACCESSING_BITS_0_7)
				result |= internal_vga_r(offset * 4 + 0) << 0;
			if (ACCESSING_BITS_8_15)
				result |= internal_vga_r(offset * 4 + 1) << 8;
			if (ACCESSING_BITS_16_23)
				result |= internal_vga_r(offset * 4 + 2) << 16;
			if (ACCESSING_BITS_24_31)
				result |= internal_vga_r(offset * 4 + 3) << 24;

			// early out to skip extra logging
			return result;
	}

	if (LOG_REGISTERS)
		logerror("%s:internal_io_r(%s) = %08X\n", machine().describe_context(), m_io_regs.name(offset), result);
	return result;
}


//-------------------------------------------------
//  internal_io_w - handle writes to the I/O
//  registers
//-------------------------------------------------

void voodoo_banshee_device::internal_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	// need to block if Y origin is changing; everything else can proceed
	offset &= 0x3f;
	if (offset == banshee_io_regs::miscInit0)
		m_renderer->wait(m_io_regs.name(offset));

	// fetch original value and compute new value
	u32 oldval = m_io_regs.read(offset);
	u32 newval = m_io_regs.write(offset, data, mem_mask);

	// handle special cases
	switch (offset)
	{
		// dacData writes indirectly to the CLUT registers; mark dirty if changing
		case banshee_io_regs::dacData:
		{
			u32 dacaddr = BIT(m_io_regs.read(banshee_io_regs::dacAddr), 0, 9);
			if (newval != m_clut[dacaddr])
			{
				screen().update_partial(screen().vpos());
				m_clut[dacaddr] = newval;
				m_clut_dirty = true;
			}
			break;
		}

		// vidProcCfg can change the CLUT index
		case banshee_io_regs::vidProcCfg:
			if (BIT(oldval ^ newval, 13))
				m_clut_dirty = true;
			break;

		// miscInit0 can alter the rendering Y origin
		case banshee_io_regs::miscInit0:
			m_renderer->set_yorigin(BIT(newval, 18, 12));
			break;

		// vidScreenSize causes the video to be reconfigured
		case banshee_io_regs::vidScreenSize:
			if (BIT(newval, 0, 12) != 0)
				m_width = BIT(data, 0, 12);
			if (BIT(newval, 12, 12) != 0)
				m_height = BIT(data, 12, 12);
			if (m_width != 0 && m_height != 0 && newval != oldval)
				recompute_video();
			break;

		// vidOverlayDudx/vidOverlayDudy impact how we configure the video
		case banshee_io_regs::vidOverlayDudx:
		case banshee_io_regs::vidOverlayDvdy:
			recompute_video();
			break;

		// lfbMemoryConfig affects LFB reads
		case banshee_io_regs::lfbMemoryConfig:
			m_lfb_base = BIT(newval, 0, 13) << (12-2);
			m_lfb_stride = BIT(newval, 13, 3) + 9;
			break;

		// VGA is special and byte-wise
		case banshee_io_regs::vgab0: case banshee_io_regs::vgab4: case banshee_io_regs::vgab8: case banshee_io_regs::vgabc:
		case banshee_io_regs::vgac0: case banshee_io_regs::vgac4: case banshee_io_regs::vgac8: case banshee_io_regs::vgacc:
		case banshee_io_regs::vgad0: case banshee_io_regs::vgad4: case banshee_io_regs::vgad8: case banshee_io_regs::vgadc:
			if (ACCESSING_BITS_0_7)
				internal_vga_w(offset * 4 + 0, BIT(data, 0, 8));
			if (ACCESSING_BITS_8_15)
				internal_vga_w(offset * 4 + 1, BIT(data, 8, 8));
			if (ACCESSING_BITS_16_23)
				internal_vga_w(offset * 4 + 2, BIT(data, 16, 8));
			if (ACCESSING_BITS_24_31)
				internal_vga_w(offset * 4 + 3, BIT(data, 24, 8));

			// early out to skip extra logging
			return;
	}

	if (LOG_REGISTERS)
		logerror("%s:internal_io_w(%s) = %08X & %08X\n", machine().describe_context(), m_io_regs.name(offset), data, mem_mask);
}


//-------------------------------------------------
//  internal_cmd_agp_r - handle reads from the
//  CMD/AGP registers
//-------------------------------------------------

u32 voodoo_banshee_device::internal_cmd_agp_r(offs_t offset)
{
	// by default just return regular data
	offset &= 0x7f;
	u32 result = m_cmd_agp_regs.read(offset);

	// check for special cases
	switch (offset)
	{
		case banshee_cmd_agp_regs::cmdRdPtrL0:
			result = m_cmdfifo.read_pointer();
			break;

		case banshee_cmd_agp_regs::cmdFifoDepth0:
			result = m_cmdfifo.depth();
			break;

		case banshee_cmd_agp_regs::cmdHoleCnt0:
			result = m_cmdfifo.holes();
			break;

		case banshee_cmd_agp_regs::cmdRdPtrL1:
			result = m_cmdfifo2.read_pointer();
			break;

		case banshee_cmd_agp_regs::cmdFifoDepth1:
			result = m_cmdfifo2.depth();
			break;

		case banshee_cmd_agp_regs::cmdHoleCnt1:
			result = m_cmdfifo2.holes();
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:internal_cmd_agp_r(%s) = %08X\n", machine().describe_context(), m_cmd_agp_regs.name(offset), result);
	return result;
}


//-------------------------------------------------
//  internal_cmd_agp_w - handle writes to the
//  CMD/AGP registers
//-------------------------------------------------

void voodoo_banshee_device::internal_cmd_agp_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x7f;

	// fetch original value and compute new value
	u32 newval = m_cmd_agp_regs.write(offset, data, mem_mask);

	// handle special cases
	switch (offset)
	{
		case banshee_cmd_agp_regs::cmdBaseAddr0:
			m_cmdfifo.set_base(BIT(newval, 0, 24) << 12);
			m_cmdfifo.set_size((BIT(m_cmd_agp_regs.read(banshee_cmd_agp_regs::cmdBaseSize0), 0, 8) + 1) << 12);
			break;

		case banshee_cmd_agp_regs::cmdBaseSize0:
			m_cmdfifo.set_size((BIT(newval, 0, 8) + 1) << 12);
			m_cmdfifo.set_enable(BIT(newval, 8));
			m_cmdfifo.set_count_holes(!BIT(newval, 10));
			break;

		case banshee_cmd_agp_regs::cmdBump0:
			fatalerror("%s: Unsupported write to cmdBump0", tag());

		case banshee_cmd_agp_regs::cmdRdPtrL0:
			m_cmdfifo.set_read_pointer(newval);
			break;

		case banshee_cmd_agp_regs::cmdAMin0:
			m_cmdfifo.set_address_min(newval);
			break;

		case banshee_cmd_agp_regs::cmdAMax0:
			m_cmdfifo.set_address_max(newval);
			break;

		case banshee_cmd_agp_regs::cmdFifoDepth0:
			m_cmdfifo.set_depth(newval);
			break;

		case banshee_cmd_agp_regs::cmdHoleCnt0:
			m_cmdfifo.set_holes(newval);
			break;

		case banshee_cmd_agp_regs::cmdBaseAddr1:
			m_cmdfifo2.set_base(BIT(newval, 0, 24) << 12);
			m_cmdfifo2.set_size((BIT(m_cmd_agp_regs.read(banshee_cmd_agp_regs::cmdBaseSize1), 0, 8) + 1) << 12);
			break;

		case banshee_cmd_agp_regs::cmdBaseSize1:
			m_cmdfifo2.set_size((BIT(newval, 0, 8) + 1) << 12);
			m_cmdfifo2.set_enable(BIT(newval, 8));
			m_cmdfifo2.set_count_holes(!BIT(newval, 10));
			break;

		case banshee_cmd_agp_regs::cmdBump1:
			fatalerror("%s: Unsupported write to cmdBump1", tag());

		case banshee_cmd_agp_regs::cmdRdPtrL1:
			m_cmdfifo2.set_read_pointer(newval);
			break;

		case banshee_cmd_agp_regs::cmdAMin1:
			m_cmdfifo2.set_address_min(newval);
			break;

		case banshee_cmd_agp_regs::cmdAMax1:
			m_cmdfifo2.set_address_max(newval);
			break;

		case banshee_cmd_agp_regs::cmdFifoDepth1:
			m_cmdfifo2.set_depth(newval);
			break;

		case banshee_cmd_agp_regs::cmdHoleCnt1:
			m_cmdfifo2.set_holes(newval);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:internal_cmd_agp_w(%s) = %08X & %08X\n", machine().describe_context(), m_io_regs.name(offset), data, mem_mask);
}


//-------------------------------------------------
//  internal_2d_w - handle writes to the 2D
//  registers
//-------------------------------------------------

s32 voodoo_banshee_device::internal_2d_w(offs_t offset, u32 data)
{
	static u8 const s_format_bpp[16] = { 1,1,1,2,3,4,1,1,2,2,1,1,1,1,1,1 };

	// by default write through to the register
	offset &= 0x3f;
	m_2d_regs.write(offset, data);

	// handle special cases
	switch (offset)
	{
		case banshee_2d_regs::command:
			if (LOG_BANSHEE_2D)
				logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);

			m_blt_src_x      = BIT(m_2d_regs.read(banshee_2d_regs::srcXY), 0, 12);
			m_blt_src_y      = BIT(m_2d_regs.read(banshee_2d_regs::srcXY), 16, 12);
			m_blt_src_base   = BIT(m_2d_regs.read(banshee_2d_regs::srcBaseAddr), 0, 24);
			m_blt_src_stride = BIT(m_2d_regs.read(banshee_2d_regs::srcFormat), 0, 14);
			m_blt_src_width  = BIT(m_2d_regs.read(banshee_2d_regs::srcSize), 0, 12);
			m_blt_src_height = BIT(m_2d_regs.read(banshee_2d_regs::srcSize), 16, 12);
			m_blt_src_bpp    = s_format_bpp[BIT(m_2d_regs.read(banshee_2d_regs::srcFormat), 16, 4)];

			m_blt_dst_x      = BIT(m_2d_regs.read(banshee_2d_regs::dstXY), 0, 12);
			m_blt_dst_y      = BIT(m_2d_regs.read(banshee_2d_regs::dstXY), 16, 12);
			m_blt_dst_base   = BIT(m_2d_regs.read(banshee_2d_regs::dstBaseAddr), 0, 24);
			m_blt_dst_stride = BIT(m_2d_regs.read(banshee_2d_regs::dstFormat), 0, 14);
			m_blt_dst_width  = BIT(m_2d_regs.read(banshee_2d_regs::dstSize), 0, 12);
			m_blt_dst_height = BIT(m_2d_regs.read(banshee_2d_regs::dstSize), 16, 12);
			m_blt_dst_bpp    = s_format_bpp[BIT(m_2d_regs.read(banshee_2d_regs::dstFormat), 16, 3)];

			m_blt_cmd = BIT(data, 0, 4);
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
				execute_blit(data);
			else if (offset >= 0x40 && offset < 0x80)
				{} // TODO: colorPattern
			break;
	}

	return 1;
}


//-------------------------------------------------
//  internal_texture_w - handle writes to texture
//  space
//-------------------------------------------------

void voodoo_banshee_device::internal_texture_w(offs_t offset, u32 data)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.m_tex_writes++;

	// point to the right TMU
	int tmunum = BIT(offset, 19, 2);
	if (!BIT(m_chipmask, 1 + tmunum))
		return;

	// pull out modes from the TMU and update state
	auto &regs = m_tmu[tmunum].regs();
	auto const texlod = regs.texture_lod();
	auto const texmode = regs.texture_mode();
	auto &texture = m_tmu[tmunum].prepare_texture(*m_renderer.get());

	// swizzle the data
	if (texlod.tdata_swizzle())
		data = swapendian_int32(data);
	if (texlod.tdata_swap())
		data = (data >> 16) | (data << 16);

	// determine destination pointer
	u8 *dest = texture.write_ptr(0, offset * 4, 0, 1);

	// wait for any outstanding work to finish
	m_renderer->wait("internal_texture_w");

	// write the four bytes in little-endian order
	u32 bytes_per_texel = (texmode.format() < 8) ? 1 : 2;
	if (bytes_per_texel == 1)
	{
		dest[BYTE4_XOR_LE(0)] = (data >> 0) & 0xff;
		dest[BYTE4_XOR_LE(1)] = (data >> 8) & 0xff;
		dest[BYTE4_XOR_LE(2)] = (data >> 16) & 0xff;
		dest[BYTE4_XOR_LE(3)] = (data >> 24) & 0xff;
	}
	else
	{
		u16 *dest16 = reinterpret_cast<u16 *>(dest);
		dest16[BYTE_XOR_LE(0)] = (data >> 0) & 0xffff;
		dest16[BYTE_XOR_LE(1)] = (data >> 16) & 0xffff;
	}
}


//-------------------------------------------------
//  internal_lfb_direct_w - handle "direct" writes
//  to LFB space; this is like previous generations
//  but ignores the LFB mode and treats everything
//  as 16-bit pixel data
//-------------------------------------------------

void voodoo_banshee_device::internal_lfb_direct_w(offs_t offset, u32 data, u32 mem_mask)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.m_lfb_writes++;

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

	// no data expansion or pixel pipelines, just raw write of 2 16-bit pixels?
	offset <<= 1;

	// compute X,Y
	s32 x = offset & ((1 << m_lfb_stride) - 1);
	s32 y = offset >> m_lfb_stride;

	// select the target buffers
	u16 *dest = lfb_buffer_indirect(0);
	u16 *end = ram_end();

	// advance pointers to the proper row
	dest += y * m_renderer->rowpixels() + x;

	// wait for any outstanding work to finish
	m_renderer->wait("internal_lfb_direct_w");

	// write to the RGB buffer
	if (ACCESSING_BITS_0_15 && dest < end)
		dest[0] = BIT(data, 0, 16);
	if (ACCESSING_BITS_16_31 && dest + 1 < end)
		dest[1] = BIT(data, 16, 16);

	// notify that frame buffer has changed
	m_video_changed = true;
	if (LOG_LFB)
		logerror("VOODOO.LFB:write direct (%d,%d) = %08X & %08X\n", x, y, data, mem_mask);
}


//-------------------------------------------------
//  internal_vga_r - handle reads from VGA register
//  space
//-------------------------------------------------

u8 voodoo_banshee_device::internal_vga_r(offs_t offset)
{
	// by default just return regular data
	offset &= 0x1f;
	u32 result = m_vga_regs.read(offset);

	// handle special offsets
	char const *logtype = "internal_vga_r";
	u32 logoffs = offset + 0x3c0;
	switch (offset)
	{
		// attribute access
		case banshee_vga_regs::attributeData:
			result = m_vga_regs.read_attr(logoffs = m_vga_regs.attribute_index());
			logtype = "vga_attr_r";
			break;

		// input status 0
		case banshee_vga_regs::inputStatus0:
			//  bit 7 = Interrupt Status. When its value is ?1?, denotes that an interrupt is pending.
			//  bit 6:5 = Feature Connector. These 2 bits are readable bits from the feature connector.
			//  bit 4 = Sense. This bit reflects the state of the DAC monitor sense logic.
			//  bit 3:0 = Reserved. Read back as 0.
			result = 0x00;
			break;

		// sequencer access
		case banshee_vga_regs::sequencerData:
			result = m_vga_regs.read_seq(logoffs = m_vga_regs.sequencer_index());
			logtype = "vga_seq_r";
			break;

		// feature control
		case banshee_vga_regs::featureControlR:
			result = m_vga_regs.read(banshee_vga_regs::featureControlW);
			m_vga_regs.clear_flip_flop();
			break;

		// miscellaneous output
		case banshee_vga_regs::miscOutputR:
			result = m_vga_regs.read(banshee_vga_regs::miscOutputW);
			break;

		// graphics controller access
		case banshee_vga_regs::gfxControllerData:
			result = m_vga_regs.read_gc(logoffs = m_vga_regs.gfx_controller_index());
			logtype = "vga_gc_r";
			break;

		// CRTC access
		case banshee_vga_regs::crtcData:
			result = m_vga_regs.read_crtc(logoffs = m_vga_regs.crtc_index());
			logtype = "vga_crtc_r";
			break;

		// input status 1
		case banshee_vga_regs::inputStatus1:
			// bit 7:6 = Reserved. These bits read back 0.
			// bit 5:4 = Display Status. These 2 bits reflect 2 of the 8 pixel data outputs from the Attribute
			//             controller, as determined by the Attribute controller index 0x12 bits 4 and 5.
			// bit 3 = Vertical sync Status. A ?1? indicates vertical retrace is in progress.
			// bit 2:1 = Reserved. These bits read back 0x2.
			// bit 0 = Display Disable. When this bit is 1, either horizontal or vertical display end has occurred,
			//             otherwise video data is being displayed.
			result = 0x04;
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:%s(%X) = %02X\n", machine().describe_context(), logtype, logoffs, result);
	return result;
}


//-------------------------------------------------
//  internal_vga_w - handle writes to VGA register
//  space
//-------------------------------------------------

void voodoo_banshee_device::internal_vga_w(offs_t offset, u8 data)
{
	offset &= 0x1f;

	// fetch original value and compute new value
	m_vga_regs.write(offset, data);

	// handle special cases
	char const *logtype = "internal_vga_w";
	u32 logoffs = offset + 0x3c0;
	switch (offset)
	{
		// attribute access
		case banshee_vga_regs::attributeData:
		case banshee_vga_regs::attributeIndex:
			if (m_vga_regs.toggle_flip_flop() == 0)
				m_vga_regs.write(banshee_vga_regs::attributeIndex, data);
			else
			{
				m_vga_regs.write_attr(logoffs = m_vga_regs.attribute_index(), data);
				logtype = "vga_crtc_w";
			}
			break;

		// sequencer access
		case banshee_vga_regs::sequencerData:
			m_vga_regs.write_seq(logoffs = m_vga_regs.sequencer_index(), data);
			logtype = "vga_seq_w";
			break;

		// graphics controller access
		case banshee_vga_regs::gfxControllerData:
			m_vga_regs.write_gc(logoffs = m_vga_regs.gfx_controller_index(), data);
			logtype = "vga_gc_w";
			break;

		// CRTC access
		case banshee_vga_regs::crtcData:
			m_vga_regs.write_crtc(logoffs = m_vga_regs.crtc_index(), data);
			logtype = "vga_crtc_w";
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:%s(%X) = %02X\n", machine().describe_context(), logtype, logoffs, data);
}


//-------------------------------------------------
//  execute_fifos - execute commands from the FIFOs
//  until a non-zero cycle count operation is run
//-------------------------------------------------

u32 voodoo_banshee_device::execute_fifos()
{
	// we might be in CMDFIFO mode
	if (m_cmdfifo.enabled())
		return m_cmdfifo.execute_if_ready();
	if (m_cmdfifo2.enabled())
		return m_cmdfifo2.execute_if_ready();

	// otherwise, run the traditional memory FIFOs
	return voodoo_1_device::execute_fifos();
}


//-------------------------------------------------
//  reg_status_r - status register read
//-------------------------------------------------

u32 voodoo_banshee_device::reg_status_r(u32 chipmask, u32 offset)
{
	u32 result = 0;

	// bits 5:0 are the PCI FIFO free space
	result |= std::min(m_pci_fifo.space() / 2, 0x3f) << 0;

	// bit 6 is the vertical retrace
	result |= m_vblank << 6;

	// bit 7 is FBI graphics engine busy
	// bit 8 is TREX busy
	// bit 9 is overall busy */
	if (operation_pending())
		result |= (1 << 7) | (1 << 8) | (1 << 9);

	// bit 10 is 2D busy

	// bit 11 is cmd FIFO 0 busy
	if (m_cmdfifo.enabled() && m_cmdfifo.depth() > 0)
		result |= 1 << 11;

	// bit 12 is cmd FIFO 1 busy
	if (m_cmdfifo2.enabled() && m_cmdfifo2.depth() > 0)
		result |= 1 << 12;

	// bits 30:28 are the number of pending swaps
	result |= std::min<u32>(m_swaps_pending, 7) << 28;

	// eat some cycles since people like polling here
	if (m_status_cycles != 0)
		m_cpu->eat_cycles(m_status_cycles);

	// bit 31 is PCI interrupt pending (not implemented)
	return result;
}


//-------------------------------------------------
//  reg_colbufbase_w - colBufferAddr register write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_colbufbase_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		m_rgboffs[1] = data & m_fbmask & ~0x0f;
	}
	return 0;
}


//-------------------------------------------------
//  reg_colbufstride_w - colBufferStride register
//  write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_colbufstride_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		u32 newpix = BIT(data, 15) ? (BIT(data, 0, 7) << 6) : (BIT(data, 0, 14) >> 1);
		if (newpix != m_renderer->rowpixels())
		{
			m_renderer->set_rowpixels(newpix);
			m_video_changed = true;
		}
	}
	return 0;
}


//-------------------------------------------------
//  reg_auxbufbase_w - auxBufferAddr register write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_auxbufbase_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		m_auxoffs = data & m_fbmask & ~0x0f;
	}
	return 0;
}


//-------------------------------------------------
//  reg_auxbufstride_w - auxBufferStride register
//  write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_auxbufstride_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		u32 newpix = BIT(data, 15) ? (BIT(data, 0, 7) << 6) : (BIT(data, 0, 14) >> 1);
		if (newpix != m_renderer->rowpixels())
			fatalerror("%s: Unsupported aux buffer stride (%d) differs from color buffer stride (%d)", tag(), newpix, m_renderer->rowpixels());
	}
	return 0;
}


//-------------------------------------------------
//  reg_swappending_w - swapPending register write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_swappending_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0)) m_swaps_pending++;
	return 0;
}


//-------------------------------------------------
//  reg_overbuffer_w - leftOverlayBuf/
//  rightOverlayBuf register write
//-------------------------------------------------

u32 voodoo_banshee_device::reg_overbuffer_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0)) m_reg.write(regnum, data);
	return 0;
}


//-------------------------------------------------
//  rotate_buffers -- rotate the buffers according
//  to the current buffer config; just update the
//  base to the left overlay buffer
//-------------------------------------------------

void voodoo_banshee_device::rotate_buffers()
{
	m_rgboffs[0] = m_reg.read(voodoo_regs::reg_leftOverlayBuf) & m_fbmask & ~0x0f;
}


//-------------------------------------------------
//  recompute_video -- determine the video size
//  and configuration based on current registers
//-------------------------------------------------

void voodoo_banshee_device::recompute_video()
{
	u8 crtc7 = m_vga_regs.read_crtc(7);

	// get horizontal total and vertical total from CRTC registers
	u32 htotal = (m_vga_regs.read_crtc(0) + 5) * 8;
	u32 vtotal = (m_vga_regs.read_crtc(6) | (BIT(crtc7, 0) << 8) | (BIT(crtc7, 5) << 9)) + 2;
	if (htotal == 0 || vtotal == 0)
		return;

	// compute start/stop for vertical retrace
	u32 vstart = m_vga_regs.read_crtc(16) | (BIT(crtc7, 2) << 8) | (BIT(crtc7, 7) << 9);
	u32 vstop = m_vga_regs.read_crtc(17) & 0xf;

	// compare to see if vstop is before or after low 4 bits of vstart
	if (vstop < (vstart & 0xf))
		vstop |= (vstart + 0x10) & ~0xf;
	else
		vstop |= vstart & ~0xf;

	// get pll k, m and n from pllCtrl0
	const u32 pll0 = m_io_regs.read(banshee_io_regs::pllCtrl0);
	const u32 k = BIT(pll0, 0, 2);
	const u32 m = BIT(pll0, 2, 6);
	const u32 n = BIT(pll0, 8, 8);
	const double video_clock = (XTAL(14'318'181) * (n + 2) / ((m + 2) << k)).dvalue();
	const double frame_period = vtotal * htotal / video_clock;

	// compute scaled screen size, using the overlay fraction if specified
	u32 dudx = m_io_regs.read(banshee_io_regs::vidOverlayDudx);
	u32 dvdy = m_io_regs.read(banshee_io_regs::vidOverlayDvdy);
	u32 width = (dudx != 0) ? ((m_width * dudx) >> 20) : m_width;
	u32 height = (dvdy != 0) ? ((m_height * dvdy) >> 20) : m_height;
	if (LOG_REGISTERS)
		logerror("configure screen: htotal: %d vtotal: %d vstart: %d vstop: %d width: %d height: %d refresh: %f\n", htotal, vtotal, vstart, vstop, width, height, 1.0 / frame_period);

	// configure the screen
	rectangle visarea(0, width - 1, 0, height - 1);
	screen().configure(htotal, vtotal, visarea, DOUBLE_TO_ATTOSECONDS(frame_period));

	// set the vsync start and stop
	m_vsyncstart = vstart;
	m_vsyncstop = vstop;
	adjust_vblank_start_timer();
}


//-------------------------------------------------
//  cmdfifo_register_w -- handle a register write
//  from the cmdfifo
//-------------------------------------------------

u32 voodoo_banshee_device::cmdfifo_register_w(u32 offset, u32 data)
{
	// bit 11 indicates a write to 2D register space
	u32 regnum = BIT(offset, 0, 8);
	if (BIT(offset, 11, 1))
		return internal_2d_w(regnum, data);

	// otherwise, just a normal register write
	u32 chipmask = chipmask_from_offset(offset);
	return m_regtable[regnum].write(*this, chipmask, regnum, data);
}


//-------------------------------------------------
//  cmdfifo_2d_w -- handle a 2D register write
//  from the cmdfifo
//-------------------------------------------------

u32 voodoo_banshee_device::cmdfifo_2d_w(u32 offset, u32 data)
{
	u32 regnum = banshee_2d_regs::clip0Min + offset;
	return internal_2d_w(regnum, data);
}


//-------------------------------------------------
//  execute_blit -- perform a 2D blitting operation
//-------------------------------------------------

void voodoo_banshee_device::execute_blit(u32 data)
{
	switch (m_blt_cmd)
	{
		case 0:         // NOP - wait for idle
			break;

		case 1:         // Screen-to-screen blit
			// TODO
			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:screen_to_screen: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
			break;

		case 2:         // Screen-to-screen stretch blit
			fatalerror("%s: Unsupported blit_2d:screen_to_screen_stretch: src X %d, src Y %d\n", tag(), data & 0xfff, (data >> 16) & 0xfff);

		case 3:         // Host-to-screen blit
		{
			u32 addr = m_blt_dst_base + m_blt_dst_y * m_blt_dst_stride + m_blt_dst_x * m_blt_dst_bpp;

			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, m_blt_dst_x, m_blt_dst_y);

			m_renderer->wait("execute_blit(3)");

			switch (m_blt_dst_bpp)
			{
				case 1:
					m_fbram[addr + 0] = BIT(data, 0, 8);
					m_fbram[addr + 1] = BIT(data, 8, 8);
					m_fbram[addr + 2] = BIT(data, 16, 8);
					m_fbram[addr + 3] = BIT(data, 24, 8);
					m_blt_dst_x += 4;
					break;

				case 2:
					m_fbram[addr + 1] = BIT(data, 0, 8);
					m_fbram[addr + 0] = BIT(data, 8, 8);
					m_fbram[addr + 3] = BIT(data, 16, 8);
					m_fbram[addr + 2] = BIT(data, 24, 8);
					m_blt_dst_x += 2;
					break;

				case 3:
					m_blt_dst_x += 1;
					break;

				case 4:
					m_fbram[addr + 3] = BIT(data, 0, 8);
					m_fbram[addr + 2] = BIT(data, 8, 8);
					m_fbram[addr + 1] = BIT(data, 16, 8);
					m_fbram[addr + 0] = BIT(data, 24, 8);
					m_blt_dst_x += 1;
					break;
			}

			if (m_blt_dst_x >= m_blt_dst_width)
			{
				m_blt_dst_x = 0;
				m_blt_dst_y++;
			}
			break;
		}

		case 5:         // Rectangle fill
			fatalerror("%s: Unsupported 2D rectangle_fill: src X %d, src Y %d", tag(), BIT(data, 0, 12), BIT(data, 16, 12));

		case 6:         // Line
			fatalerror("%s: Unsupported 2D line: end X %d, end Y %d", tag(), BIT(data, 0, 12), BIT(data, 16, 12));

		case 7:         // Polyline
			fatalerror("%s: Unsupported 2D polyline: end X %d, end Y %d", tag(), BIT(data, 0, 12), BIT(data, 16, 12));

		case 8:         // Polygon fill
			fatalerror("%s: Unsupported 2D polygon_fill", tag());

		default:
			fatalerror("%s: Unsupported 2D unknown command %d", tag(), m_blt_cmd);
	}
}


//**************************************************************************
//  VOODOO 3 DEVICE
//**************************************************************************

//-------------------------------------------------
//  voodoo_3_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(VOODOO_3, voodoo_3_device, "voodoo_3", "3dfx Voodoo 3")

voodoo_3_device::voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	voodoo_banshee_device(mconfig, VOODOO_3, tag, owner, clock, voodoo_model::VOODOO_3)
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void voodoo_3_device::device_start()
{
	// Voodoo-3 has 2 TMUs standard
	if (m_chipmask == 0x01)
		m_chipmask = 0x07;

	// start like a Banshee
	voodoo_banshee_device::device_start();
}


//**************************************************************************
//  VOODOO BANSHEE REGISTER MAP
//**************************************************************************

#define REGISTER_ENTRY(name, reader, writer, bits, chips, sync, fifo) \
	{ static_register_table_entry<voodoo_banshee_device>::make_mask(bits), register_table_entry::CHIPMASK_##chips | register_table_entry::SYNC_##sync | register_table_entry::FIFO_##fifo, #name, &voodoo_banshee_device::reg_##writer##_w, &voodoo_banshee_device::reg_##reader##_r },

#define RESERVED_ENTRY REGISTER_ENTRY(reserved, invalid, invalid, 32, FBI, NOSYNC, FIFO)

#define RESERVED_ENTRY_x8 RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY

static_register_table_entry<voodoo_banshee_device> const voodoo_banshee_device::s_register_table[256] =
{
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(status,          status,      invalid,     32, FBI,      NOSYNC,   FIFO)    // 000
	REGISTER_ENTRY(intrCtrl,        passive,     intrctrl,    32, FBI,      NOSYNC, NOFIFO)    // 004 - cmdFIFO mode
	REGISTER_ENTRY(vertexAx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 008
	REGISTER_ENTRY(vertexAy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 00c
	REGISTER_ENTRY(vertexBx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 010
	REGISTER_ENTRY(vertexBy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 014
	REGISTER_ENTRY(vertexCx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 018
	REGISTER_ENTRY(vertexCy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 01c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(startR,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 020
	REGISTER_ENTRY(startG,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 024
	REGISTER_ENTRY(startB,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 028
	REGISTER_ENTRY(startZ,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 02c
	REGISTER_ENTRY(startA,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 030
	REGISTER_ENTRY(startS,          invalid,     starts,      32, TREX,     NOSYNC,   FIFO)    // 034
	REGISTER_ENTRY(startT,          invalid,     startt,      32, TREX,     NOSYNC,   FIFO)    // 038
	REGISTER_ENTRY(startW,          invalid,     startw,      32, FBI_TREX, NOSYNC,   FIFO)    // 03c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 040
	REGISTER_ENTRY(dGdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 044
	REGISTER_ENTRY(dBdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 048
	REGISTER_ENTRY(dZdX,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 04c
	REGISTER_ENTRY(dAdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 050
	REGISTER_ENTRY(dSdX,            invalid,     dsdx,        32, TREX,     NOSYNC,   FIFO)    // 054
	REGISTER_ENTRY(dTdX,            invalid,     dtdx,        32, TREX,     NOSYNC,   FIFO)    // 058
	REGISTER_ENTRY(dWdX,            invalid,     dwdx,        32, FBI_TREX, NOSYNC,   FIFO)    // 05c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 060
	REGISTER_ENTRY(dGdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 064
	REGISTER_ENTRY(dBdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 068
	REGISTER_ENTRY(dZdY,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 06c
	REGISTER_ENTRY(dAdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 070
	REGISTER_ENTRY(dSdY,            invalid,     dsdy,        32, TREX,     NOSYNC,   FIFO)    // 074
	REGISTER_ENTRY(dTdY,            invalid,     dtdy,        32, TREX,     NOSYNC,   FIFO)    // 078
	REGISTER_ENTRY(dWdY,            invalid,     dwdy,        32, FBI_TREX, NOSYNC,   FIFO)    // 07c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(triangleCMD,     invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 080
	RESERVED_ENTRY                                                                             // 084
	REGISTER_ENTRY(fvertexAx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 088
	REGISTER_ENTRY(fvertexAy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 08c
	REGISTER_ENTRY(fvertexBx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 090
	REGISTER_ENTRY(fvertexBy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 094
	REGISTER_ENTRY(fvertexCx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 098
	REGISTER_ENTRY(fvertexCy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 09c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fstartR,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a0
	REGISTER_ENTRY(fstartG,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a4
	REGISTER_ENTRY(fstartB,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a8
	REGISTER_ENTRY(fstartZ,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ac
	REGISTER_ENTRY(fstartA,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0b0
	REGISTER_ENTRY(fstartS,         invalid,     fstarts,     32, TREX,     NOSYNC,   FIFO)    // 0b4
	REGISTER_ENTRY(fstartT,         invalid,     fstartt,     32, TREX,     NOSYNC,   FIFO)    // 0b8
	REGISTER_ENTRY(fstartW,         invalid,     fstartw,     32, FBI_TREX, NOSYNC,   FIFO)    // 0bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c0
	REGISTER_ENTRY(fdGdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c4
	REGISTER_ENTRY(fdBdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c8
	REGISTER_ENTRY(fdZdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0cc
	REGISTER_ENTRY(fdAdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0d0
	REGISTER_ENTRY(fdSdX,           invalid,     fdsdx,       32, TREX,     NOSYNC,   FIFO)    // 0d4
	REGISTER_ENTRY(fdTdX,           invalid,     fdtdx,       32, TREX,     NOSYNC,   FIFO)    // 0d8
	REGISTER_ENTRY(fdWdX,           invalid,     fdwdx,       32, FBI_TREX, NOSYNC,   FIFO)    // 0dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e0
	REGISTER_ENTRY(fdGdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e4
	REGISTER_ENTRY(fdBdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e8
	REGISTER_ENTRY(fdZdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ec
	REGISTER_ENTRY(fdAdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0f0
	REGISTER_ENTRY(fdSdY,           invalid,     fdsdy,       32, TREX,     NOSYNC,   FIFO)    // 0f4
	REGISTER_ENTRY(fdTdY,           invalid,     fdtdy,       32, TREX,     NOSYNC,   FIFO)    // 0f8
	REGISTER_ENTRY(fdWdY,           invalid,     fdwdy,       32, FBI_TREX, NOSYNC,   FIFO)    // 0fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(ftriangleCMD,    invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 100
	REGISTER_ENTRY(fbzColorPath,    passive,     passive,     30, FBI_TREX, NOSYNC,   FIFO)    // 104
	REGISTER_ENTRY(fogMode,         passive,     passive,      8, FBI_TREX, NOSYNC,   FIFO)    // 108
	REGISTER_ENTRY(alphaMode,       passive,     passive,     32, FBI_TREX, NOSYNC,   FIFO)    // 10c
	REGISTER_ENTRY(fbzMode,         passive,     passive,     22, FBI_TREX,   SYNC,   FIFO)    // 110
	REGISTER_ENTRY(lfbMode,         passive,     passive,     17, FBI_TREX,   SYNC,   FIFO)    // 114
	REGISTER_ENTRY(clipLeftRight,   passive,     passive,     32, FBI_TREX,   SYNC,   FIFO)    // 118
	REGISTER_ENTRY(clipLowYHighY,   passive,     passive,     32, FBI_TREX,   SYNC,   FIFO)    // 11c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nopCMD,          invalid,     nop,          2, FBI_TREX,   SYNC,   FIFO)    // 120
	REGISTER_ENTRY(fastfillCMD,     invalid,     fastfill,     0, FBI,        SYNC,   FIFO)    // 124
	REGISTER_ENTRY(swapbufferCMD,   invalid,     swapbuffer,  10, FBI,        SYNC,   FIFO)    // 128
	REGISTER_ENTRY(fogColor,        invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 12c
	REGISTER_ENTRY(zaColor,         invalid,     passive,     32, FBI,        SYNC,   FIFO)    // 130
	REGISTER_ENTRY(chromaKey,       invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 134
	REGISTER_ENTRY(chromaRange,     invalid,     passive,     28, FBI,        SYNC,   FIFO)    // 138
	REGISTER_ENTRY(userIntrCMD,     invalid,     userintr,    10, FBI,        SYNC,   FIFO)    // 13c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(stipple,         passive,     passive,     32, FBI,        SYNC,   FIFO)    // 140
	REGISTER_ENTRY(color0,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 144
	REGISTER_ENTRY(color1,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 148
	REGISTER_ENTRY(fbiPixelsIn,     stats,       invalid,     24, FBI,          NA,     NA)    // 14c
	REGISTER_ENTRY(fbiChromaFail,   stats,       invalid,     24, FBI,          NA,     NA)    // 150
	REGISTER_ENTRY(fbiZfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 154
	REGISTER_ENTRY(fbiAfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 158
	REGISTER_ENTRY(fbiPixelsOut,    stats,       invalid,     24, FBI,          NA,     NA)    // 15c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[0],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 160
	REGISTER_ENTRY(fogTable[1],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 164
	REGISTER_ENTRY(fogTable[2],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 168
	REGISTER_ENTRY(fogTable[3],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 16c
	REGISTER_ENTRY(fogTable[4],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 170
	REGISTER_ENTRY(fogTable[5],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 174
	REGISTER_ENTRY(fogTable[6],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 178
	REGISTER_ENTRY(fogTable[7],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 17c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[8],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 180
	REGISTER_ENTRY(fogTable[9],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 184
	REGISTER_ENTRY(fogTable[10],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 188
	REGISTER_ENTRY(fogTable[11],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 18c
	REGISTER_ENTRY(fogTable[12],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 190
	REGISTER_ENTRY(fogTable[13],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 194
	REGISTER_ENTRY(fogTable[14],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 198
	REGISTER_ENTRY(fogTable[15],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 19c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[16],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a0
	REGISTER_ENTRY(fogTable[17],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a4
	REGISTER_ENTRY(fogTable[18],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a8
	REGISTER_ENTRY(fogTable[19],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1ac
	REGISTER_ENTRY(fogTable[20],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b0
	REGISTER_ENTRY(fogTable[21],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b4
	REGISTER_ENTRY(fogTable[22],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b8
	REGISTER_ENTRY(fogTable[23],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[24],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c0
	REGISTER_ENTRY(fogTable[25],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c4
	REGISTER_ENTRY(fogTable[26],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c8
	REGISTER_ENTRY(fogTable[27],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1cc
	REGISTER_ENTRY(fogTable[28],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d0
	REGISTER_ENTRY(fogTable[29],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d4
	REGISTER_ENTRY(fogTable[30],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d8
	REGISTER_ENTRY(fogTable[31],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY                                                                             // 1e0
	RESERVED_ENTRY                                                                             // 1e4
	RESERVED_ENTRY                                                                             // 1e8
	REGISTER_ENTRY(colBufferAddr,   passive,     colbufbase,  24, FBI,        SYNC,   FIFO)    // 1ec
	REGISTER_ENTRY(colBufferStride, passive,     colbufstride,16, FBI,        SYNC,   FIFO)    // 1f0
	REGISTER_ENTRY(auxBufferAddr,   passive,     auxbufbase,  24, FBI,        SYNC,   FIFO)    // 1f4
	REGISTER_ENTRY(auxBufferStride, passive,     auxbufstride,16, FBI,        SYNC,   FIFO)    // 1f8
	RESERVED_ENTRY                                                                             // 1fc

	REGISTER_ENTRY(clipLeftRight1,  passive,     passive,     32, FBI,        SYNC,   FIFO)    // 200
	REGISTER_ENTRY(clipTopBottom1,  passive,     passive,     32, FBI,        SYNC,   FIFO)    // 204
	RESERVED_ENTRY                                                                             // 208
	RESERVED_ENTRY                                                                             // 20c
	RESERVED_ENTRY                                                                             // 210
	RESERVED_ENTRY                                                                             // 214
	RESERVED_ENTRY                                                                             // 218
	RESERVED_ENTRY                                                                             // 21c

	RESERVED_ENTRY_x8                                                                          // 220-23c

	RESERVED_ENTRY                                                                             // 240
	RESERVED_ENTRY                                                                             // 244
	RESERVED_ENTRY                                                                             // 248
	REGISTER_ENTRY(swapPending,     invalid,     swappending, 32, FBI,      NOSYNC, NOFIFO)    // 24c
	REGISTER_ENTRY(leftOverlayBuf,  invalid,     overbuffer,  32, FBI,      NOSYNC,   FIFO)    // 250
	REGISTER_ENTRY(rightOverlayBuf, invalid,     overbuffer,  32, FBI,      NOSYNC,   FIFO)    // 254
	REGISTER_ENTRY(fbiSwapHistory,  passive,     invalid,     32, FBI,          NA,     NA)    // 258
	REGISTER_ENTRY(fbiTrianglesOut, passive,     invalid,     24, FBI,          NA,     NA)    // 25c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sSetupMode,      invalid,     passive,     20, FBI,      NOSYNC,   FIFO)    // 260
	REGISTER_ENTRY(sVx,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 264
	REGISTER_ENTRY(sVy,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 268
	REGISTER_ENTRY(sARGB,           invalid,     sargb,       32, FBI,      NOSYNC,   FIFO)    // 26c
	REGISTER_ENTRY(sRed,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 270
	REGISTER_ENTRY(sGreen,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 274
	REGISTER_ENTRY(sBlue,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 278
	REGISTER_ENTRY(sAlpha,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 27c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sVz,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 280
	REGISTER_ENTRY(sWb,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 284
	REGISTER_ENTRY(sWtmu0,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 288
	REGISTER_ENTRY(sS_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 28c
	REGISTER_ENTRY(sT_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 290
	REGISTER_ENTRY(sWtmu1,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 294
	REGISTER_ENTRY(sS_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 298
	REGISTER_ENTRY(sT_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 29c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sDrawTriCMD,     invalid,     draw_tri,    32, FBI,      NOSYNC,   FIFO)    // 2a0
	REGISTER_ENTRY(sBeginTriCMD,    invalid,     begin_tri,   32, FBI,      NOSYNC,   FIFO)    // 2a4
	RESERVED_ENTRY                                                                             // 2a8
	RESERVED_ENTRY                                                                             // 2ac
	RESERVED_ENTRY                                                                             // 2b0
	RESERVED_ENTRY                                                                             // 2b4
	RESERVED_ENTRY                                                                             // 2b8
	RESERVED_ENTRY                                                                             // 2bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 2c0-2dc
	RESERVED_ENTRY_x8                                                                          // 2e0-2fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(textureMode,     invalid,     texture,     31, TREX,     NOSYNC,   FIFO)    // 300
	REGISTER_ENTRY(tLOD,            invalid,     texture,     28, TREX,     NOSYNC,   FIFO)    // 304
	REGISTER_ENTRY(tDetail,         invalid,     texture,     22, TREX,     NOSYNC,   FIFO)    // 308
	REGISTER_ENTRY(texBaseAddr,     invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 30c
	REGISTER_ENTRY(texBaseAddr_1,   invalid,     texture,     24, TREX,     NOSYNC,   FIFO)    // 310
	REGISTER_ENTRY(texBaseAddr_2,   invalid,     texture,     24, TREX,     NOSYNC,   FIFO)    // 314
	REGISTER_ENTRY(texBaseAddr_3_8, invalid,     texture,     24, TREX,     NOSYNC,   FIFO)    // 318
	REGISTER_ENTRY(trexInit0,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 31c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(trexInit1,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 320
	REGISTER_ENTRY(nccTable0[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 324
	REGISTER_ENTRY(nccTable0[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 328
	REGISTER_ENTRY(nccTable0[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 32c
	REGISTER_ENTRY(nccTable0[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 330
	REGISTER_ENTRY(nccTable0[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 334
	REGISTER_ENTRY(nccTable0[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 338
	REGISTER_ENTRY(nccTable0[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 33c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable0[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 340
	REGISTER_ENTRY(nccTable0[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 344
	REGISTER_ENTRY(nccTable0[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 348
	REGISTER_ENTRY(nccTable0[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 34c
	REGISTER_ENTRY(nccTable0[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 350
	REGISTER_ENTRY(nccTable1[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 354
	REGISTER_ENTRY(nccTable1[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 358
	REGISTER_ENTRY(nccTable1[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 35c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 360
	REGISTER_ENTRY(nccTable1[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 364
	REGISTER_ENTRY(nccTable1[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 368
	REGISTER_ENTRY(nccTable1[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 36c
	REGISTER_ENTRY(nccTable1[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 370
	REGISTER_ENTRY(nccTable1[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 374
	REGISTER_ENTRY(nccTable1[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 378
	REGISTER_ENTRY(nccTable1[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 37c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 380
	RESERVED_ENTRY                                                                             // 384
	RESERVED_ENTRY                                                                             // 388
	RESERVED_ENTRY                                                                             // 38c
	RESERVED_ENTRY                                                                             // 390
	RESERVED_ENTRY                                                                             // 394
	RESERVED_ENTRY                                                                             // 398
	RESERVED_ENTRY                                                                             // 39c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	RESERVED_ENTRY_x8                                                                          // 3a0-3bc
	RESERVED_ENTRY_x8                                                                          // 3c0-3dc
	RESERVED_ENTRY_x8                                                                          // 3e0-3fc
};
