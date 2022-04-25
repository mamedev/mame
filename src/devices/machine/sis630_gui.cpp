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
    - Understand how exactly 630 selects between the SVGA and extended register sets;
    - Backward port 630 GUI/PCI implementation to 300;
    - Confirm PCI IDs (they aren't well formed);
    - With current config it claims memory size to be 3MB instead of 64
      (regressed during development)
    - Hardware cursor is supposed to be extended reg $6 bit 6, but it's apparently not the
      right trigger by gamecstl Windows non-safe mode;

**************************************************************************************************/

#include "emu.h"
#include "sis630_gui.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps
#define LOG_AGP    (1U << 4) // log AGP
#define LOG_SVGA   (1U << 5) // log SVGA

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP | LOG_AGP | LOG_SVGA)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGAGP(...)    LOGMASKED(LOG_AGP,  __VA_ARGS__)
#define LOGSVGA(...)   LOGMASKED(LOG_SVGA, __VA_ARGS__)

/**************************
 *
 * SVGA implementation
 *
 *************************/

// TODO: later variant of 5598
// (definitely doesn't have dual segment mode for instance)
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
	vga.svga_intf.vram_size = 64*1024*1024;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

void sis630_svga_device::device_reset()
{
	svga_device::device_reset();

	m_svga_bank_reg_w = m_svga_bank_reg_r = 0;
	m_unlock_reg = false;
	//m_dual_seg_mode = false;
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
		m_crtc_ext_regs[index] = data;
	}
}

uint8_t sis630_svga_device::seq_reg_read(uint8_t index)
{
	// extended id register
	if (index == 0x05)
		return m_unlock_reg ? 0xa1 : 0x21;
	//LOGSVGA("%02x\n", index);
	if (index == 0x06)
		return m_ramdac_mode;
	return m_seq_ext_regs[index];
}

void sis630_svga_device::seq_reg_write(uint8_t index, uint8_t data)
{
	if (index < vga.svga_intf.seq_regcount)
		svga_device::seq_reg_write(index, data);
	else
	{
		if (index == 0x05)
		{
			m_unlock_reg = (data == 0x86);
			LOGSVGA("Unlock register write %02x (%s)\n", data, m_unlock_reg ? "unlocked" : "locked");
		}
		else
		{
			if (!m_unlock_reg)
			{
				LOGSVGA("Attempt to write to extended SVGA while locked [$%02x] -> %02x\n", index, data);
				return;
			}

			m_seq_ext_regs[index] = data;

			switch(index)
			{
				/*
				 * x--- ---- GFX mode linear addressing enable
				 * -x-- ---- GFX hardware cursor display
				 * --x- ---- GFX mode interlace
				 * ---x ---- True Color enable (ties with index 0x07 bit 2)
				 * ---- x--- RGB16 enable
				 * ---- -x-- RGB15 enable
				 * ---- --x- enhanced GFX mode enable
				 * ---- ---x enhanced text mode enable
				 */
				case 0x06:
					m_ramdac_mode = data;
					LOGSVGA("RAMDAC mode %02x\n", data);

					if (!BIT(data, 1))
					{
						svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;
					}
					else
					{
						if (BIT(data, 2))
							svga.rgb15_en = 1;
					}
					break;
				case 0x0b:
					//m_dual_seg_mode = bool(BIT(data, 3));
					LOGSVGA("Extended Misc. Control register (%02x) %02x\n", index, data);
					break;
				default:
					LOGSVGA("Extended write %02x %02x\n", index, data);
					break;
			}
		}
	}
}

uint16_t sis630_svga_device::offset()
{
	// TODO: extended CRT pitch register $0e
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

void sis630_svga_device::port_03c0_w(offs_t offset, uint8_t data)
{
	// TODO: for '630 it's always with dual segment enabled?

	if (offset == 0xd)
	{
		//if (m_dual_seg_mode)
			m_svga_bank_reg_w = (data & 0x3f) * 0x10000;
		//else
		{
		//  m_svga_bank_reg_w = (data >> 4) * 0x10000;
		//  m_svga_bank_reg_r = (data & 0xf) * 0x10000;
		}
		return;
	}

	if (offset == 0xb)
	{
		//if (m_dual_seg_mode)
			m_svga_bank_reg_r = (data & 0x3f) * 0x10000;
		// otherwise ignored if dual segment mode disabled
		return;
	}

	svga_device::port_03c0_w(offset, data);
}

uint8_t sis630_svga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return svga_device::mem_linear_r(offset + m_svga_bank_reg_r);
	return svga_device::mem_r(offset);
}

void sis630_svga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		svga_device::mem_linear_w(offset + m_svga_bank_reg_w, data);
		return;
	}
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
	, m_svga(*this, "svga")
	, m_gui_rom(*this, "gui_rom")
{
	set_ids(0x10396300, 0x00, 0x030000, 0x00);
}

ROM_START( sis630gui )
	ROM_REGION32_LE( 0xc000, "gui_rom", ROMREGION_ERASEFF )
	// TODO: why the OEM ROM is 0xc000 in size?
	// 0x8000-0xbfff mostly contains a charset, may be either programmable via a dedicated interface
	// or the dump above is half size.
	// gamecstl dump ver. 2.06.50
	// (which actually writes to VRAM with the actual expansion ROM enabled, uh?)
	ROM_SYSTEM_BIOS( 0, "2.06.50", "Ver. 2.06.50 OEM" )
	ROMX_LOAD( "oemrom.bin", 0x0000, 0xc000, BAD_DUMP CRC(03d8df9d) SHA1(8fb80a2bf4067d9bebc90fb498448869ae795b2b), ROM_BIOS(0) )

	// "SiS 630 (Ver. 2.02.1c) [AGP VGA] (Silicon Integrated Systems Corp.).bin"
	ROM_SYSTEM_BIOS( 1, "2.02.1c", "Ver. 2.02.1c" )
	ROMX_LOAD( "sis630.bin", 0x0000, 0x8000, BAD_DUMP CRC(f04ef9b0) SHA1(2396a79cd4045362bfc511090b146daa85902b4d), ROM_BIOS(1) )
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
	// 64MB according to POST
	// documentation claims 128MB, assume being wrong
	m_svga->set_vram_size(64*1024*1024);
}

void sis630_gui_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x2c, 0x2d).r(FUNC(sis630_gui_device::subvendor_r));
	map(0x2e, 0x2f).r(FUNC(sis630_gui_device::subsystem_r));
	map(0x2c, 0x2f).w(FUNC(sis630_gui_device::subvendor_w));

//  map(0x3c, 0x3d) irq line/pin

	map(0x34, 0x34).r(FUNC(sis630_gui_device::capptr_r));

	map(0x50, 0x53).r(FUNC(sis630_gui_device::agp_id_r));
	map(0x54, 0x57).r(FUNC(sis630_gui_device::agp_status_r));
	map(0x58, 0x5b).rw(FUNC(sis630_gui_device::agp_command_r), FUNC(sis630_gui_device::agp_command_w));
	map(0x5c, 0x5c).lr8(NAME([] () { return 0; })); // NULL terminator
}

u8 sis630_gui_device::capptr_r()
{
	return 0x50;
}

// TODO: move to specific interface
u32 sis630_gui_device::agp_id_r()
{
	LOGAGP("Read AGP ID [$50]\n");
	// bits 23-16 AGP v1.0
	// bits 15-8 0x5c NEXT_PTR (which goes to NULL terminator, heh)
	// bits 7-0 CAP_ID (0x02 for AGP)
	return 0x00105c02;
}

u32 sis630_gui_device::agp_status_r()
{
	LOGAGP("Read AGP status [$54]\n");
	// RQ (1 + 1), 2X and 1X capable
	return 0x01000003;
}

u32 sis630_gui_device::agp_command_r(offs_t offset, uint32_t mem_mask)
{
	LOGAGP("Read AGP command [$58] %d %d %08x\n", m_agp.enable, m_agp.data_rate, mem_mask);
	// TODO: enable gets cleared by AGP_RESET, or even from PCI RST#
	return m_agp.enable << 8 | (m_agp.data_rate & 7);
}

void sis630_gui_device::agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGAGP("Write AGP command [$c8] %08x & %08x\n", data, mem_mask);

	if (ACCESSING_BITS_8_15)
	{
		m_agp.enable = bool(BIT(m_agp.enable, 8));
		LOGAGP("- AGP_ENABLE = %d\n", m_agp.enable);
	}

	if (ACCESSING_BITS_0_7)
	{
		std::map<u8, std::string> agp_transfer_rates = {
			{ 1, "1X" },
			{ 2, "2X" },
		};

		// make sure the AGP DATA_RATE specs are honored
		try {
			const u8 data_rate = data & 3;
			LOGAGP("- DATA_RATE = %s\n", agp_transfer_rates.at(data_rate));
			m_agp.data_rate = data_rate;
		}
		catch (std::out_of_range& err) {
			LOG("Warning: AGP illegal DATA_RATE set = %d enabled=%d\n", data & 3, m_agp.enable);
		}
	}
}

// TODO: may be common to PCI base interface, verify
void sis630_gui_device::subvendor_w(offs_t offset, u32 data, u32 mem_mask)
{
	// write once
	if (m_subsystem_logger_mask & mem_mask)
	{
		LOG("Warning: subvendor ID possible rewrite! old=%08x & %08x data=%08x & %08x\n"
			, subsystem_id
			, m_subsystem_logger_mask
			, data
			, mem_mask
		);
	}
	m_subsystem_logger_mask |= mem_mask;

	COMBINE_DATA(&subsystem_id);
	LOGIO("subsystem ID write [$2c] %08x & %08x (%08x)\n", data, mem_mask, subsystem_id);
}

void sis630_gui_device::memory_map(address_map &map)
{
}

void sis630_gui_device::io_map(address_map &map)
{

}

// "Relocate I/O" -> RIO
void sis630_gui_device::space_io_map(address_map &map)
{
	// RIO + 0x00: video capture regs on '300, omitted or missing on '630
	// RIO + 0x02: MPEG-2 video playback
	// RIO + 0x04: digital video interface (to '301 only?)
	// RIO + 0x10: 301 TV encoder
	// RIO + 0x12: 301 macrovision regs
	// RIO + 0x14: 301 VGA regs
	// RIO + 0x16: 301 RAMDAC
	// RIO + 0x30/+0x40/+0x50: omitted, legacy '300/'630 VGA regs?
}

void sis630_gui_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(sis630_gui_device::vram_r), FUNC(sis630_gui_device::vram_w));
}

void sis630_gui_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03bf).rw(FUNC(sis630_gui_device::vga_3b0_r), FUNC(sis630_gui_device::vga_3b0_w));
	map(0x03c0, 0x03cf).rw(FUNC(sis630_gui_device::vga_3c0_r), FUNC(sis630_gui_device::vga_3c0_w));
	map(0x03d0, 0x03df).rw(FUNC(sis630_gui_device::vga_3d0_r), FUNC(sis630_gui_device::vga_3d0_w));
}

void sis630_gui_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{

}

void sis630_gui_device::device_start()
{
	pci_device::device_start();

	add_map(64*1024*1024, M_MEM, FUNC(sis630_gui_device::memory_map));
	// claims 128KB, which goes outside the pentium range.
	// Assume memory mapped, given the size should be yet another VGA memory compatible window.
	add_map(128*1024, M_MEM, FUNC(sis630_gui_device::io_map));
	add_map(128, M_IO, FUNC(sis630_gui_device::space_io_map));

	add_rom((u8 *)m_gui_rom->base(), m_gui_rom->bytes());

	// TODO: INTA, correct as 0?
	intr_pin = 0;
}

void sis630_gui_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0004;
	status = 0x0220;

	m_subsystem_logger_mask = 0;
}

// TODO: remove these trampolines
uint8_t sis630_gui_device::vram_r(offs_t offset)
{
	return downcast<sis630_svga_device *>(m_svga.target())->mem_r(offset);
}

void sis630_gui_device::vram_w(offs_t offset, uint8_t data)
{
//  printf("%08x %02x\n", offset, data);
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
//  map(0x10, 0x4f).unmaprw();
//  map(0x10, 0x3e).rw(FUNC(sis301_video_bridge_device::unmap_log_r), FUNC(sis301_video_bridge_device::unmap_log_w));
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
