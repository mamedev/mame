// license:BSD-3-Clause
// copyright-holders:Barry Rodewald


#include "emu.h"
#include "virge_pci.h"

#include "screen.h"

DEFINE_DEVICE_TYPE(VIRGE_PCI, virge_pci_device, "virge_pci", "S3 86C325 ViRGE")
DEFINE_DEVICE_TYPE(VIRGEVX_PCI, virgevx_pci_device, "virgevx_pci", "S3 86C988 ViRGE/VX")
DEFINE_DEVICE_TYPE(VIRGEDX_PCI, virgedx_pci_device, "virgedx_pci", "S3 86C375 ViRGE/DX")
//VIRGEGX_PCI, /DX with SGRAM/SDRAM support

virge_pci_device::virge_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: virge_pci_device(mconfig, VIRGE_PCI, tag, owner, clock)
{
}

virge_pci_device::virge_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock),
	m_vga(*this, "vga"),
	m_bios(*this, "bios"),
	m_screen(*this, finder_base::DUMMY_TAG)
{
}

virgevx_pci_device::virgevx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: virge_pci_device(mconfig, VIRGEVX_PCI, tag, owner, clock)
{
}

virgedx_pci_device::virgedx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: virge_pci_device(mconfig, VIRGEDX_PCI, tag, owner, clock)
{
}

// TODO: this is repeated in old MMIO mode inside the VGA legacy memory space
void virge_pci_device::mmio_map(address_map& map)
{
	// image transfer ports
	map(0x0000,0x7fff).w(m_vga, FUNC(s3virge_vga_device::image_xfer));

	//map(0x8000,0x8043) PCI Configuration Space regs
	map(0x8180,0x81ff).m(m_vga, FUNC(s3virge_vga_device::streams_control_map));

	//map(0x8200,0x821f) memory port controller
	//map(0x8220,0x8227) DMA control

	// MMIO address map
	map(0x83b0,0x83df).m(m_vga, FUNC(s3virge_vga_device::io_map));
	map(0x8504,0x8507).rw(m_vga, FUNC(s3virge_vga_device::s3d_sub_status_r), FUNC(s3virge_vga_device::s3d_sub_control_w));
	// TODO: writeable
	map(0x850c,0x850f).r(m_vga, FUNC(s3virge_vga_device::s3d_func_ctrl_r));

	//map(0x8580,0x858b) video DMA
	//map(0x8590,0x859f) command DMA

	// S3D engine registers
	map(0xa000,0xb7ff).m(m_vga, FUNC(s3virge_vga_device::s3d_register_map));

	// alternate image transfer ports
	map(0xd000,0xefff).w(m_vga, FUNC(s3virge_vga_device::image_xfer));

	//map(0xff00, 0xff5f) LPB Local Peripheral Bus control
	//map(0xff1c, 0xff1f) LPB GIP/GOP General Input/Output Port (for OEM implementations)
	map(0xff20, 0xff23).rw(m_vga, FUNC(s3virge_vga_device::serial_port_r), FUNC(s3virge_vga_device::serial_port_w));
	//map(0xff24, 0xff27) LPB Video Input Window Size
	//map(0xff28, 0xff2b) LPB Video Data Offsets
	//map(0xff2c, 0xff2f) LPB Horizontal Decimation Control Register
	//map(0xff30, 0xff33) LPB Vertical Decimation Control Register
	//map(0xff34, 0xff37) LPB Line Stride
	//map(0xff40, 0xff5f) LPB Output FIFO Register
}

void virge_pci_device::lfb_map(address_map& map)
{
	// NOTE: upper mirror is i/f for BE hosts.
	map(0x0, 0x00ffffff).mirror(0x02000000).rw(m_vga, FUNC(s3virge_vga_device::fb_r), FUNC(s3virge_vga_device::fb_w));
}

void virge_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x10, 0x13).rw(FUNC(virge_pci_device::base_address_r),FUNC(virge_pci_device::base_address_w));
}

void virge_pci_device::legacy_io_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(s3virge_vga_device::io_map));
}

// FIXME: belongs to s3virge_vga_device.
u32 virge_pci_device::get_vga_linear_address()
{
	switch(downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address_size() & 0x03)
	{
		case LAW_64K:
			return downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffff0000;

		case LAW_1MB:
			return downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xfff00000;

		case LAW_2MB:
			return downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffe00000;

		default:
		case LAW_4MB:
			return downcast<s3virge_vga_device *>(m_vga.target())->get_linear_address() & 0xffc00000;
	}
}

void virge_pci_device::refresh_linear_window()
{
	if(downcast<s3virge_vga_device *>(m_vga.target())->is_linear_address_active())
	{
		set_map_flags(0, M_MEM);
		const u32 get_bar_address = get_vga_linear_address();
		logerror("Linear window set to 0x%08x\n", get_bar_address);
		set_map_address(0, get_bar_address);
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
	// only bits 31-26 are changed here, cfr. page 25-4
	const u32 new_address = (data & 0xfc000000) | (base_address_r() & 0x3ffffff);
	downcast<s3virge_vga_device *>(m_vga.target())->set_linear_address(new_address);
	refresh_linear_window();
}

void virge_pci_device::linear_config_changed_w(int state)
{
	refresh_linear_window();
	remap_cb();
}

uint8_t virge_pci_device::vram_r(offs_t offset)
{
	return downcast<s3virge_vga_device *>(m_vga.target())->mem_r(offset);
}

void virge_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<s3virge_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void virge_pci_device::postload()
{
	remap_cb();
}

void virge_pci_device::device_start()
{
	set_ids(0x53335631, 0x00, 0x030000, 0x000000);
	pci_card_device::device_start();

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	add_map(64 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(virge_pci_device::lfb_map));
	set_map_address(0, 0x70000000);

	command = 0x0000;
	// DAC SNP / BME / MEM / I/O
	command_mask = 0x27;
	// medium DEVSELB
	status = 0x0200;

	remap_cb();
	machine().save().register_postload(save_prepost_delegate(FUNC(virge_pci_device::postload), this));
}

void virgevx_pci_device::device_start()
{
	set_ids(0x5333883d, 0x00, 0x030000, 0x000000);
	pci_card_device::device_start();

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	add_map(64 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(virgevx_pci_device::lfb_map));
	set_map_address(0, 0x70000000);

	command = 0x0000;
	// DAC SNP / BME / MEM / I/O
	command_mask = 0x27;
	// medium DEVSELB
	status = 0x0200;

	remap_cb();
	machine().save().register_postload(save_prepost_delegate(FUNC(virgevx_pci_device::postload), this));
}

void virgedx_pci_device::device_start()
{
	set_ids(0x53338a01, 0x00, 0x030000, 0x000000);
	pci_card_device::device_start();

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	add_map(64 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(virge_pci_device::lfb_map));
	set_map_address(0, 0x70000000);

	command = 0x0000;
	// DAC SNP / BME / MEM / I/O
	command_mask = 0x27;
	// medium DEVSELB
	status = 0x0200;

	remap_cb();
	machine().save().register_postload(save_prepost_delegate(FUNC(virgedx_pci_device::postload), this));
}

void virge_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(virge_pci_device::vram_r)), write8sm_delegate(*this, FUNC(virge_pci_device::vram_w)));
		// NOTE: this looks unrelated to the linear addressing
		// xubuntu 6.10 and BeOS 3.x definitely wants it this way.
		if(downcast<s3virge_vga_device *>(m_vga.target())->is_new_mmio_active())
		{
			u32 get_bar_address = get_vga_linear_address() | 0x01000000;
			memory_space->install_device(get_bar_address, get_bar_address | 0xffff, *this, &virge_pci_device::mmio_map);

			get_bar_address |= 0x02000000;
			memory_space->install_device(get_bar_address, get_bar_address | 0xffff, *this, &virge_pci_device::mmio_map);
		}
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &virge_pci_device::legacy_io_map);

		// Available at MMFF20 only if true
		if (m_vga->read_pd26_strapping() == false)
		{
			const u16 port_offset = m_vga->read_pd25_strapping() ? 0xe2 : 0xe8;
			io_space->install_readwrite_handler(port_offset, port_offset, read8sm_delegate(m_vga, FUNC(s3virge_vga_device::serial_port_r)), write8sm_delegate(m_vga, FUNC(s3virge_vga_device::serial_port_w)));
		}
	}
}

void virge_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGE(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x400000);
	m_vga->linear_config_changed().set(FUNC(virge_pci_device::linear_config_changed_w));
}

void virgevx_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGEVX(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 2MB, 4MB and 8MB, EDO RAM
	m_vga->set_vram_size(0x400000);
	m_vga->linear_config_changed().set(FUNC(virgevx_pci_device::linear_config_changed_w));
}

void virgedx_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGEDX(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x400000);
	m_vga->linear_config_changed().set(FUNC(virgedx_pci_device::linear_config_changed_w));
}


ROM_START( virge_pci )
	ROM_REGION(0x8000,"bios", 0)
	ROM_DEFAULT_BIOS("virge")

	ROM_SYSTEM_BIOS( 0, "virge", "S3 ViRGE v1.00-10" )
	ROMX_LOAD("pci_m-v_virge-4s3.bin", 0x00000, 0x8000, CRC(d0a0f1de) SHA1(b7b41081974762a199610219bdeab149b7c7143d), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "virgeo", "S3 ViRGE v1.00-05" )
	ROMX_LOAD("s3virge.bin", 0x00000, 0x8000, CRC(a7983a85) SHA1(e885371816d3237f7badd57ccd602cd863c9c9f8), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *virge_pci_device::device_rom_region() const
{
	return ROM_NAME( virge_pci );
}

ROM_START( virgevx_pci )
	ROM_REGION(0x8000,"bios", 0)
	ROM_DEFAULT_BIOS("dms3d3k")

	// Vlask dump
	// v2.xx BIOSes known to exist
	ROM_SYSTEM_BIOS( 0, "dms3d3k", "Diamond Stealth 3D 3000 HQ v1.00" )
	ROMX_LOAD("diamondstealth3000.vbi", 0x0000, 0x8000, CRC(c2423896) SHA1(fdab43f15da1d66bca286583f9da86873f3e22de), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *virgevx_pci_device::device_rom_region() const
{
	return ROM_NAME( virgevx_pci );
}


ROM_START( virgedx_pci )
	ROM_REGION(0x8000,"bios", 0)
	ROM_DEFAULT_BIOS("virgedx")

	ROM_SYSTEM_BIOS( 0, "virgedx", "S3 ViRGE/DX v2.01.13" )
	ROMX_LOAD("s3virgedx.bin", 0x00000, 0x8000, CRC(0da83bd3) SHA1(228a2d644e1732cb5a2eb1291608c7050cf39229), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "dms3d2kp", "Diamond Stealth 3D 2000 Pro v3.04" )
	ROMX_LOAD("virgedxdiamond.bin", 0x00000, 0x8000, CRC(58b0dcda) SHA1(b13ae6b04db6fc05a76d924ddf2efe150b823029), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "s600dx", "Leadtek WinFast 3D S600DX V1.01.03" )
	ROMX_LOAD("winfast_3d_s600dx.bin", 0x00000, 0x8000, CRC(d68db9f4) SHA1(9a7f58fab7811342a00bbc76837b4f9015913ddb), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *virgedx_pci_device::device_rom_region() const
{
	return ROM_NAME( virgedx_pci );
}
