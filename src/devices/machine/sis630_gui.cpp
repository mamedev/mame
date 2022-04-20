// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    SiS 630 Video GUI portion (SVGA-based) & 301 video bridge

	- 630 core is SVGA based:
	\- has two sets of extended CRTC ($3c4) regs;
	\- a dedicated MPEG-2 video playback interface;
	\- a digital video interface to 301;
	- 301 draws to a separate monitor, and it was originally tied to a SiS300 AGP card,
	  (which we don't have a dump of at the time of this writing):
	\- can select VGA, NTSC, PAL or LCD sources;
	\- Has separate set of VGA and RAMDAC regs;
	\- Has TV encoder;
	\- Has macrovision regs;
	- GUI is the 630 PCI/AGP i/f
	\- it's actually internal to the rest of 630;
	\- 301 is external but closely tied to it;

	TODO:
	- Very preliminary, enough to make it to draw basic VGA primary screen and not much else;
	- Legacy BIOS (disable shadow RAM in host) draws in MDA mode, which doesn't work with this
	  implementation;
	- Understand how exactly 630 selects between the SVGA and extended register sets;
	- Backward port 630 GUI/PCI implementation to 300;
	- Confirm PCI IDs (they aren't well formed);

**************************************************************************************************/

#include "emu.h"
#include "sis630_gui.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

/**************************
 *
 * SVGA implementation
 *
 *************************/

DEFINE_DEVICE_TYPE(SIS630_SVGA, sis630_svga_device, "sis630_svga", "SiS 630 SVGA")

sis630_svga_device::sis630_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, SIS630_SVGA, tag, owner, clock)
{
	
}

void sis630_svga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.read_dipswitch.set(nullptr); //read_dipswitch;
	vga.svga_intf.seq_regcount = 0x05;
	vga.svga_intf.crtc_regcount = 0x27;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

// Page 144
uint8_t sis630_svga_device::crtc_reg_read(uint8_t index)
{
	if (index < 0x19)
		return svga_device::crtc_reg_read(index);

	// TODO: if one of these is 0xff then it enables a single port transfer to $b8000
	return m_crtc_ext_regs[index];
} 

void sis630_svga_device::crtc_reg_write(uint8_t index, uint8_t data)
{
	if (index < 0x19)
		svga_device::crtc_reg_write(index, data);
	else
	{
		//printf("%02x %02x\n", index, data);
		m_crtc_ext_regs[index] = data;
	}
}


uint8_t sis630_svga_device::mem_r(offs_t offset)
{
//	printf("%08x %08llx\n", offset, vga.svga_intf.vram_size);

	return svga_device::mem_r(offset);
}

void sis630_svga_device::mem_w(offs_t offset, uint8_t data)
{
//	printf("%08x %02x %08llx\n", offset, data, vga.svga_intf.vram_size);
//	printf("%d %d %d %d\n",svga.rgb8_en, svga.rgb15_en, svga.rgb16_en, svga.rgb24_en);
	svga_device::mem_w(offset, data);
}

/*****************************
 *
 * 630 GUI PCI implementation
 *
 ****************************/

DEFINE_DEVICE_TYPE(SIS630_GUI, sis630_gui_device, "sis630_gui", "SiS 630 GUI")

sis630_gui_device::sis630_gui_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS630_GUI, tag, owner, clock)
	, m_biosrom(*this, "biosrom")
	, m_svga(*this, "svga")
{
	set_ids(0x10396300, 0x00, 0x030000, 0x00);
}

ROM_START( sis630gui )
	ROM_REGION32_LE( 0x10000, "biosrom", ROMREGION_ERASEFF )
	// "SiS 630 (Ver. 2.02.1c) [AGP VGA] (Silicon Integrated Systems Corp.).bin"
	ROM_LOAD( "sis630.bin", 0x0000, 0x8000, CRC(f04ef9b0) SHA1(2396a79cd4045362bfc511090b146daa85902b4d) )

	// TODO: why the OEM ROM is 0xc000 in size?
	// 0x8000-0xbfff mostly contains a charset, may not be visible by HW or that part is programmable
	// via a dedicated interface.
	// gamecstl dump ver. 2.06.50
	// (which actually writes to VRAM with the actual expansion ROM enabled, uh?)
	//ROM_LOAD( "oemrom.bin", 0x0000, 0xc000, CRC(03d8df9d) SHA1(8fb80a2bf4067d9bebc90fb498448869ae795b2b) )
ROM_END

const tiny_rom_entry *sis630_gui_device::device_rom_region() const
{
	return ROM_NAME(sis630gui);
}

void sis630_gui_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(sis630_svga_device::screen_update));

	SIS630_SVGA(config, m_svga, 0);
	m_svga->set_screen("screen");
	m_svga->set_vram_size(0x400000); // 64MB according to POST
}

void sis630_gui_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x10, 0x4f).unmaprw();
	map(0x10, 0x5c).rw(FUNC(sis630_gui_device::unmap_log_r), FUNC(sis630_gui_device::unmap_log_w));
	map(0x10, 0x13).rw(FUNC(sis630_gui_device::base_fb_r), FUNC(sis630_gui_device::base_fb_w));
	map(0x14, 0x17).rw(FUNC(sis630_gui_device::base_io_r), FUNC(sis630_gui_device::base_io_w));
	map(0x18, 0x1b).rw(FUNC(sis630_gui_device::space_io_r), FUNC(sis630_gui_device::space_io_w));
	map(0x30, 0x33).rw(FUNC(sis630_gui_device::exp_rom_r), FUNC(sis630_gui_device::exp_rom_w));

//	map(0x3c, 0x3d) irq line/pin

	// TODO: AGP regs, available only when it's enabled
//	map(0x34, 0x34) capabilities list offset pointer (0x50)
//	map(0x50, 0x50) AGP config 0x00105c02
//	map(0x54, 0x54) AGP id 0x01000003
//	map(0x58, 0x5b) AGP control (bit 8 AGP enable)
}

// TODO: debugging, to be removed
u8 sis630_gui_device::unmap_log_r(offs_t offset)
{
	LOGTODO("GUI Unemulated [%02x] R\n", offset + 0x10);
	return 0;
}

void sis630_gui_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("GUI Unemulated [%02x] %02x W\n", offset + 0x10, data);
}

void sis630_gui_device::memory_map(address_map &map)
{

}

void sis630_gui_device::io_map(address_map &map)
{
	
}



void sis630_gui_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: understand how the full VRAM memory really maps up
//	memory_space->install_device(0, 0xffffffff, *this, &sis630_gui_device::memory_map);

	memory_space->install_readwrite_handler(
		0xa0000, 0xbffff, 
		read8sm_delegate(*this, FUNC(sis630_gui_device::vram_r)),
		write8sm_delegate(*this, FUNC(sis630_gui_device::vram_w))
	);
	
	// TODO: expansion ROM doesn't work properly
	

	//printf("%08x\n", m_exp_rom_reg);
	if (m_exp_rom_reg & 1)
		memory_space->install_rom(0x000d0000, 0x000dffff, m_biosrom);

#if 0
	if (m_exp_rom_reg & 1)
	{
		const u32 start_offs = m_exp_rom_reg & ~1;
		const u32 end_offs = start_offs + m_biosrom.bytes() - 1;
		
		if (start_offs < end_offs)
			memory_space->install_rom(start_offs, end_offs, m_biosrom);
	}
#endif

	// TODO: convert to io_map
	io_space->install_device(0, 0xffff, *this, &sis630_gui_device::io_map);
	
	io_space->install_readwrite_handler(0x03b0, 0x03bf, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3b0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3b0_w)));
	io_space->install_readwrite_handler(0x03c0, 0x03cf, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3c0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3c0_w)));
	io_space->install_readwrite_handler(0x03d0, 0x03df, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3d0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3d0_w)));

	// "128 I/O space" probably means the RIO "Relocate I/O" base
	// at some point I've got extensive checks with 0xff84 (digital video interface regs),
	// TODO: pinpoint effective value base
	// (doc mentions being 16-bit wide, should be 7-bit by logic)
	if (m_space_io_base == 0xffffffff)
	{
		// all regs follow index/data pattern space
		// RIO + 0x00: video capture regs on 300, omitted or missing on 630 
		// RIO + 0x02: MPEG-2 video playback
		// RIO + 0x04: digital video interface (to 301 only?)
		// RIO + 0x10: 301 TV encoder
		// RIO + 0x12: 301 macrovision regs
		// RIO + 0x14: 301 VGA regs
		// RIO + 0x16: 301 RAMDAC
		// RIO + 0x30/+0x40/+0x50: omitted, assume 300/630 VGA regs
		io_space->install_readwrite_handler(0xffb0, 0xffbf, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3b0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3b0_w)));
		io_space->install_readwrite_handler(0xffc0, 0xffcf, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3c0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3c0_w)));
		io_space->install_readwrite_handler(0xffd0, 0xffdf, read32s_delegate(*this, FUNC(sis630_gui_device::vga_3d0_r)), write32s_delegate(*this, FUNC(sis630_gui_device::vga_3d0_w)));
	}
}

void sis630_gui_device::device_start()
{
	pci_device::device_start();

#if 0
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
#endif
}

void sis630_gui_device::device_reset()
{
	pci_device::device_reset();
	
	command = 0x0004;
	status = 0x0220;
	m_exp_rom_reg =   0x000c0001;
	m_fb_base =       0x00000008;
	m_io_base =       0x00000000;
	m_space_io_base = 0x00000001;
}

// TODO: remove these trampolines
uint8_t sis630_gui_device::vram_r(offs_t offset)
{
	return downcast<sis630_svga_device *>(m_svga.target())->mem_r(offset);
}

void sis630_gui_device::vram_w(offs_t offset, uint8_t data)
{
//	printf("%08x %02x\n", offset, data);
	downcast<sis630_svga_device *>(m_svga.target())->mem_w(offset, data);
}

u32 sis630_gui_device::vga_3b0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03b0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03b0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03b0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03b0_r(offset * 4 + 3) << 24;
	return result;
}

void sis630_gui_device::vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<sis630_svga_device *>(m_svga.target())->port_03b0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<sis630_svga_device *>(m_svga.target())->port_03b0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<sis630_svga_device *>(m_svga.target())->port_03b0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<sis630_svga_device *>(m_svga.target())->port_03b0_w(offset * 4 + 3, data >> 24);
}


u32 sis630_gui_device::vga_3c0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03c0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03c0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03c0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03c0_r(offset * 4 + 3) << 24;
	return result;
}

void sis630_gui_device::vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<sis630_svga_device *>(m_svga.target())->port_03c0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<sis630_svga_device *>(m_svga.target())->port_03c0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<sis630_svga_device *>(m_svga.target())->port_03c0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<sis630_svga_device *>(m_svga.target())->port_03c0_w(offset * 4 + 3, data >> 24);
}

u32 sis630_gui_device::vga_3d0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03d0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03d0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03d0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<sis630_svga_device *>(m_svga.target())->port_03d0_r(offset * 4 + 3) << 24;
	return result;
}

void sis630_gui_device::vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<sis630_svga_device *>(m_svga.target())->port_03d0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<sis630_svga_device *>(m_svga.target())->port_03d0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<sis630_svga_device *>(m_svga.target())->port_03d0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<sis630_svga_device *>(m_svga.target())->port_03d0_w(offset * 4 + 3, data >> 24);
}

u32 sis630_gui_device::base_fb_r(uint32_t mem_mask)
{
	LOGIO("Read FB base [$10] %08x & %08x\n", m_fb_base, mem_mask);
	return m_fb_base;
}

void sis630_gui_device::base_fb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_fb_base);	
	LOGIO("Write FB base [$10] %08x & %08x (%08x)\n", data, mem_mask, m_fb_base);
	remap_cb();
}

u32 sis630_gui_device::base_io_r(uint32_t mem_mask)
{
	LOGIO("Read I/O base [$14] %08x & %08x\n", m_io_base, mem_mask);
	return m_io_base;
}

void sis630_gui_device::base_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_io_base);
	
	LOGIO("Write I/O base [$14] %08x & %08x\n", m_io_base, mem_mask);
	remap_cb();
}

u32 sis630_gui_device::space_io_r(uint32_t mem_mask)
{
	LOGIO("Read RIO base [$18] %08x & %08x\n", m_space_io_base, mem_mask);
	return m_space_io_base;
}

void sis630_gui_device::space_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_space_io_base);
	LOGIO("Write RIO base [$18] %08x & %08x (%08x)\n", data, mem_mask, m_space_io_base);
	remap_cb();
}

u32 sis630_gui_device::exp_rom_r(uint32_t mem_mask)
{
	LOGIO("Read expansion ROM base [$30] %08x & %08x\n", m_exp_rom_reg, mem_mask);
	return m_exp_rom_reg;
}

// TODO: remove, use the actual pci_device implementation
void sis630_gui_device::exp_rom_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_exp_rom_reg);
//	m_exp_rom_reg &= 0xfffff801;
	m_exp_rom_reg &= 0xfffff801 & (1-0x10000);
	LOGIO("Write expansion ROM base [$30] %08x & %08x (%08x)\n", data, mem_mask, m_exp_rom_reg);
	remap_cb();
}

/*************************
*
* SiS 301 Virtual Bridge
* 
*************************/

DEFINE_DEVICE_TYPE(SIS301_VIDEO_BRIDGE, sis301_video_bridge_device, "sis630_bridge", "SiS 301 Virtual PCI-to-PCI Video Bridge")

sis301_video_bridge_device::sis301_video_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_bridge_device(mconfig, SIS301_VIDEO_BRIDGE, tag, owner, clock)
{
	set_ids(0x10390001, 0x00, 0x060400, 0x00);
}

void sis301_video_bridge_device::device_add_mconfig(machine_config &config)
{
	// ...
}

void sis301_video_bridge_device::config_map(address_map &map)
{
	pci_bridge_device::config_map(map);
	// shouldn't have any programming interface
//	map(0x10, 0x4f).unmaprw();
//	map(0x10, 0x3e).rw(FUNC(sis301_video_bridge_device::unmap_log_r), FUNC(sis301_video_bridge_device::unmap_log_w));
}

void sis301_video_bridge_device::memory_map(address_map &map)
{
	// TODO: how it access shared VRAM from GUI?
}

void sis301_video_bridge_device::io_map(address_map &map)
{
	
}


void sis301_video_bridge_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: installs from GUI i/f via RIO
	io_space->install_device(0, 0xffff, *this, &sis301_video_bridge_device::io_map);
}

void sis301_video_bridge_device::device_start()
{
	pci_device::device_start();

#if 0
	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;
#endif
}


void sis301_video_bridge_device::device_reset()
{
	pci_device::device_reset();
	
	command = 0x0000;
	status = 0x0000;
}
