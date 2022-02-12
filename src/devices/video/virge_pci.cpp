// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"
#include "virge_pci.h"

#include "screen.h"

virge_pci_device::virge_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: virge_pci_device(mconfig, VIRGE_PCI, tag, owner, clock)
{
}

virge_pci_device::virge_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock),
	m_vga(*this, "vga"),
	m_bios(*this, "bios"),
	m_screen(*this, finder_base::DUMMY_TAG)
{
}

virgedx_pci_device::virgedx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: virge_pci_device(mconfig, VIRGEDX_PCI, tag, owner, clock)
{
}

void virge_pci_device::mmio_map(address_map& map)
{
	// image transfer ports
	map(0x1000000,0x1007fff).w(m_vga, FUNC(s3virge_vga_device::image_xfer));

	// MMIO address map
	map(0x1008504,0x1008507).rw(m_vga, FUNC(s3virge_vga_device::s3d_sub_status_r), FUNC(s3virge_vga_device::s3d_sub_control_w));
	map(0x100850c,0x100850f).r(m_vga, FUNC(s3virge_vga_device::s3d_func_ctrl_r));

	// S3D engine registers
	map(0x100a000,0x100b7ff).rw(m_vga, FUNC(s3virge_vga_device::s3d_register_r), FUNC(s3virge_vga_device::s3d_register_w));

	// alternate image transfer ports
	map(0x100d000,0x100efff).w(m_vga, FUNC(s3virge_vga_device::image_xfer));

}

void virge_pci_device::lfb_map(address_map& map)
{
	map(0x0, 0x00ffffff).rw(m_vga, FUNC(s3virge_vga_device::fb_r), FUNC(s3virge_vga_device::fb_w));
	if(downcast<s3virge_vga_device *>(m_vga.target())->is_new_mmio_active())
		mmio_map(map);
}

void virge_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x10, 0x13).rw(FUNC(virge_pci_device::base_address_r),FUNC(virge_pci_device::base_address_w));
}

void virge_pci_device::refresh_linear_window()
{
	if(downcast<s3virge_vga_device *>(m_vga.target())->is_linear_address_active())
	{
		set_map_flags(0, M_MEM);
		switch(downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address_size() & 0x03)
		{
			case LAW_64K:
				set_map_address(0,downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffff0000);
				logerror("Linear window set to 0x%08x\n",downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffff0000);
				break;
			case LAW_1MB:
				set_map_address(0,downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xfff00000);
				logerror("Linear window set to 0x%08x\n",downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xfff00000);
				break;
			case LAW_2MB:
				set_map_address(0,downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffe00000);
				logerror("Linear window set to 0x%08x\n",downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffe00000);
				break;
			case LAW_4MB:
				set_map_address(0,downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffc00000);
				logerror("Linear window set to 0x%08x\n",downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffc00000);
				break;
		}
		remap_cb();
	}
	else
	{
		set_map_flags(0, M_MEM | M_DISABLED);
		remap_cb();
	}
}

uint32_t virge_pci_device::base_address_r()
{
	return downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address();
}

void virge_pci_device::base_address_w(offs_t offset, uint32_t data)
{
	pci_device::address_base_w(offset,data);
	downcast<s3virge_vga_device *>(m_vga.target())->set_linear_address(data & 0xffff0000);
	refresh_linear_window();
}

uint32_t virge_pci_device::vga_3b0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 3) << 24;
	return result;
}

void virge_pci_device::vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		downcast<s3_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 0, data >> 0);
		if(offset == 1 && downcast<s3virge_vga_device *>(m_vga.target())->get_crtc_port() == 0x3b0)
			m_current_crtc_reg = data & 0xff;
	}
	if (ACCESSING_BITS_8_15)
	{
		downcast<s3_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 1, data >> 8);
		if(offset == 1 && downcast<s3virge_vga_device *>(m_vga.target())->get_crtc_port() == 0x3b0)
		{
			if(m_current_crtc_reg == 0x58)
			{
				refresh_linear_window();
				remap_cb();
			}
			else if(m_current_crtc_reg == 0x59 || m_current_crtc_reg == 0x5a)
			{
				refresh_linear_window();
				remap_cb();
			}
		}
	}
	if (ACCESSING_BITS_16_23)
		downcast<s3_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<s3_vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 3, data >> 24);
}

uint32_t virge_pci_device::vga_3c0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 3) << 24;
	return result;
}

void virge_pci_device::vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<s3_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<s3_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<s3_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<s3_vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 3, data >> 24);
}

uint32_t virge_pci_device::vga_3d0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<s3_vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 3) << 24;
	return result;
}

void virge_pci_device::vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		downcast<s3_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 0, data >> 0);
		if(offset == 1 && downcast<s3virge_vga_device *>(m_vga.target())->get_crtc_port() == 0x3d0)
			m_current_crtc_reg = data & 0xff;
	}
	if (ACCESSING_BITS_8_15)
	{
		downcast<s3_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 1, data >> 8);
		if(offset == 1 && downcast<s3virge_vga_device *>(m_vga.target())->get_crtc_port() == 0x3d0)
		{
			if(m_current_crtc_reg >= 0x58 && m_current_crtc_reg <= 0x5a)
			{
				refresh_linear_window();
			}
		}
	}
	if (ACCESSING_BITS_16_23)
		downcast<s3_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<s3_vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 3, data >> 24);
}

uint8_t virge_pci_device::vram_r(offs_t offset)
{
	return downcast<s3_vga_device *>(m_vga.target())->mem_r(offset);
}

void virge_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<s3_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void virge_pci_device::postload()
{
	remap_cb();
}

void virge_pci_device::device_start()
{
	set_ids(0x53335631, 0x00, 0x030000, 0x000000);
	pci_device::device_start();

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	add_map(32 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(virge_pci_device::lfb_map));
	set_map_address(0, 0x70000000);
	set_map_size(0, 0x01100000);  // Linear addressing maps to a 32MB address space

	remap_cb();
	machine().save().register_postload(save_prepost_delegate(FUNC(virge_pci_device::postload), this));
}

void virgedx_pci_device::device_start()
{
	set_ids(0x53338a01, 0x00, 0x030000, 0x000000);
	pci_device::device_start();

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	add_map(4 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(virge_pci_device::lfb_map));
	set_map_address(0, 0x70000000);
	set_map_size(0, 0x01100000);  // Linear addressing maps to a 32MB address space

	remap_cb();
	machine().save().register_postload(save_prepost_delegate(FUNC(virgedx_pci_device::postload), this));
}

void virge_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(virge_pci_device::vram_r)), write8sm_delegate(*this, FUNC(virge_pci_device::vram_w)));

	io_space->install_readwrite_handler(0x3b0, 0x3bf, read32s_delegate(*this, FUNC(virge_pci_device::vga_3b0_r)), write32s_delegate(*this, FUNC(virge_pci_device::vga_3b0_w)));
	io_space->install_readwrite_handler(0x3c0, 0x3cf, read32s_delegate(*this, FUNC(virge_pci_device::vga_3c0_r)), write32s_delegate(*this, FUNC(virge_pci_device::vga_3c0_w)));
	io_space->install_readwrite_handler(0x3d0, 0x3df, read32s_delegate(*this, FUNC(virge_pci_device::vga_3d0_r)), write32s_delegate(*this, FUNC(virge_pci_device::vga_3d0_w)));
}

void virge_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGE(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x400000);
}

void virgedx_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGEDX(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x400000);
}


ROM_START( virge_pci )
	ROM_REGION(0x8000,"bios", 0)
	ROM_DEFAULT_BIOS("virge")

	ROM_SYSTEM_BIOS( 0, "virge", "PCI S3 ViRGE v1.00-10" )
	ROMX_LOAD("pci_m-v_virge-4s3.bin", 0x00000, 0x8000, CRC(d0a0f1de) SHA1(b7b41081974762a199610219bdeab149b7c7143d), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "virgeo", "PCI S3 ViRGE v1.00-05" )
	ROMX_LOAD("s3virge.bin", 0x00000, 0x8000, CRC(a7983a85) SHA1(e885371816d3237f7badd57ccd602cd863c9c9f8), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )
ROM_END

ROM_START( virgedx_pci )
	ROM_REGION(0x8000,"bios", 0)
	ROM_LOAD("s3virgedx.bin", 0x00000, 0x8000, CRC(0da83bd3) SHA1(228a2d644e1732cb5a2eb1291608c7050cf39229) )
	//ROMX_LOAD("virgedxdiamond.bin", 0x00000, 0x8000, CRC(58b0dcda) SHA1(b13ae6b04db6fc05a76d924ddf2efe150b823029), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *virge_pci_device::device_rom_region() const
{
	return ROM_NAME( virge_pci );
}

const tiny_rom_entry *virgedx_pci_device::device_rom_region() const
{
	return ROM_NAME( virgedx_pci );
}

DEFINE_DEVICE_TYPE(VIRGE_PCI, virge_pci_device, "virge_pci", "S3 86C325 ViRGE PCI")
DEFINE_DEVICE_TYPE(VIRGEDX_PCI, virgedx_pci_device, "virgedx_pci", "S3 86C375 ViRGE/DX PCI")
