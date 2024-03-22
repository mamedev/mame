// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

MediaGX host implementation (northbridge)

TODO:
- Currently cheat around software VGA, MediaGX notoriously triggers SMI for every access to
  VGA legacy ranges, which is horrible both for emulation purposes and for performance.

**************************************************************************************************/

#include "emu.h"
#include "mediagx_host.h"

#define LOG_MAP    (1U << 1) // log full remaps

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(MEDIAGX_HOST, mediagx_host_device, "mediagx_host", "MediaGX X-Bus Host PCI")

mediagx_host_device::mediagx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, MEDIAGX_HOST, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_vga(*this, "vga")
{
	m_superio_space_config = address_space_config("superio_space", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(mediagx_host_device::superio_map), this));
}

u8 mediagx_host_device::superio_if_r(offs_t offset)
{
	if (!offset || m_superio_lock)
	{
		LOG("Super I/O: $%02x read while locked %02x\n", offset + 0x22, m_superio_index);
		return space().unmap();
	}

	return space(AS_PCI_IO).read_byte(m_superio_index);
}

void mediagx_host_device::superio_if_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		m_superio_index = data;
		m_superio_lock = false;
		return;
	}

	if (!m_superio_lock)
	{
		m_superio_lock = true;
		space(AS_PCI_IO).write_byte(m_superio_index, data);
	}
	else
		LOG("Super I/O: $23 write while locked %02x %02x\n", m_superio_index, data);
}

void mediagx_host_device::superio_map(address_map &map)
{
//  map(0x20, 0x20) PCR
//  map(0xb0, 0xb0) SMHR0
//  map(0xb1, 0xb1) SMHR1
//  map(0xb2, 0xb2) SMHR2
//  map(0xb3, 0xb3) SMHR3
	// GCR
	map(0xb8, 0xb8).lrw8(
		NAME([this] () {
			return m_superio.gcr;
		}),
		NAME([this] (u8 data) {
			m_superio.gcr = data;
			if (data & 0xc)
				LOG("GCR scratchpad setting %02x\n", data);
			remap_cb();
		})
	);
//  map(0xb9, 0xb9) VGACTL
//  map(0xba, 0xbd) VGAM0
//  map(0xc1, 0xc1) CCR1
//  map(0xc2, 0xc2) CCR2
//  map(0xc3, 0xc3) CCR3
//  map(0xe8, 0xe8) CCR4
//  map(0xeb, 0xeb) CCR7
	// DIR0
	map(0xfe, 0xfe).lr8(
		NAME([] () {
			// xxxx ---- Device ID
			// 0100 ---- MediaGX MMX
			// ---- xxxx Core Multiplier (depends on DIR1)
			return 0x40 | 0x05;
		})
	);
//  map(0xff, 0xff) DIR1
}

device_memory_interface::space_config_vector mediagx_host_device::memory_space_config() const
{
	auto r = pci_host_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_IO,     &m_superio_space_config));
	return r;
}

void mediagx_host_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&m_host_cpu->space(AS_PROGRAM), &m_host_cpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end   = 0xffffffff;
	memory_offset       = 0;
	io_window_start = 0;
	io_window_end   = 0xffff;
	io_offset       = 0;

	m_ram.resize(m_ram_size/4);
}

void mediagx_host_device::device_reset()
{
	pci_host_device::device_reset();

	command = 0x0007;
	status = 0x0280;

	m_pci_control[0] = 0x00;
	m_pci_control[1] = 0x96;

	m_pci_arbitration[0] = 0x80;
	m_pci_arbitration[1] = 0x00;

	m_superio_lock = true;

	remap_cb();
}

void mediagx_host_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	// HACK: needs an interruptible x86 core to even try.
	VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(16*1024*1024);
}

void mediagx_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x40, 0x41).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PCI Control Function %d read\n", offset + 1);
			return m_pci_control[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("PCI Control Function %d write %02x\n", offset + 1, data);
			m_pci_control[offset] = data;
		})
	);

	map(0x43, 0x44).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PCI Arbitration Control %d read\n", offset + 1);
			return m_pci_arbitration[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("PCI Arbitration Control %d write %02x\n", offset + 1, data);
			m_pci_arbitration[offset] = data;
		})
	);
}

void mediagx_host_device::map_shadowram(address_space *memory_space, uint32_t start_offs, uint32_t end_offs, bool read_enable, bool write_enable)
{
	LOGMAP("- 0x%08x-0x%08x ", start_offs, end_offs);

	switch(write_enable << 1 | read_enable)
	{
		case 0:
			LOGMAP("shadow RAM off\n");
			//memory_space->unmap_write(start_offs, end_offs);
			break;
		case 1:
			LOGMAP("shadow RAM r/o\n");
			memory_space->install_rom(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 2:
			LOGMAP("shadow RAM w/o\n");
			//memory_space->install_rom(start_offs, end_offs, m_region->base() + bios_rom_offset);
			memory_space->install_writeonly(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
		case 3:
			LOGMAP("shadow RAM r/w\n");
			memory_space->install_ram(start_offs, end_offs, &m_ram[start_offs/4]);
			break;
	}
}

void mediagx_host_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
) {
	io_space->install_device(0, 0xffff,  *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	regenerate_config_mapping();

	io_space->install_readwrite_handler(0x22, 0x23,
		read8sm_delegate(*this, FUNC(mediagx_host_device::superio_if_r)),
		write8sm_delegate(*this, FUNC(mediagx_host_device::superio_if_w))
	);

	memory_space->install_ram(0x00000000, 0x0009ffff, &m_ram[0x00000000/4]);
//  memory_space->install_ram(0x000a0000, 0x000bffff, &m_ram[0x000a0000/4]);
	memory_space->install_device(0x000a0000, 0x000bffff, *this, &mediagx_host_device::legacy_memory_map);
	io_space->install_device(0x03b0, 0x03df, *this, &mediagx_host_device::legacy_io_map);

	LOGMAP("Host Remapping table (BC_XMAP_1 %08x BC_XMAP_2 %08x BC_XMAP_3):\n", m_bc_xmap[0], m_bc_xmap[1], m_bc_xmap[2]);

	// BC_XMAP_2 & BC_XMAP_3 bits remaps with this arrangement:
	// x--- PCI accessible
	// -x-- Cache Enable
	// --x- Write Enable
	// ---x Read Enable
	for (int i = 0; i < 8; i ++)
	{
		u32 start_offs = 0x000c0000 + i * 0x4000;
		u32 end_offs = start_offs + 0x3fff;

		map_shadowram(
			memory_space,
			start_offs, end_offs,
			bool(BIT(m_bc_xmap[1], i * 4)), bool(BIT(m_bc_xmap[1], i * 4 + 1))
		);
	}
	for (int i = 0; i < 8; i ++)
	{
		u32 start_offs = 0x000e0000 + i * 0x4000;
		u32 end_offs = start_offs + 0x3fff;

		map_shadowram(
			memory_space,
			start_offs, end_offs,
			bool(BIT(m_bc_xmap[2], i * 4)), bool(BIT(m_bc_xmap[2], i * 4 + 1))
		);
	}
	memory_space->install_ram          (0x00100000, 0x00efffff, &m_ram[0x00100000/4]);
	// TODO: verify if there's a memory hole 15M-16M like other x86 PCI hosts
	//if(memory_hole_upper)
		memory_space->install_ram      (0x00f00000, 0x00ffffff, &m_ram[0x00f00000/4]);

	memory_space->install_ram          (0x01000000, m_ram_size-1, &m_ram[0x01000000/4]);

	const u32 gx_base = (m_superio.gcr & 3) << 30;

	if (gx_base)
	{
		LOG("gxbase mapped at %08x\n", gx_base);
		memory_space->install_device(gx_base, (gx_base) | 0xffffff, *this, &mediagx_host_device::gxbase_map);
	}
}

void mediagx_host_device::gxbase_map(address_map &map)
{
//  0x001000 scratchpad
//  0x008000 Internal bus I/F Unit
	map(0x008004, 0x00800f).lrw32(
		NAME([this] (offs_t offset) {
			return m_bc_xmap[offset];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_bc_xmap[offset]);
			remap_cb();
		})
	);
//  0x008100 GFX pipeline
//  0x008300 Display controller
//  0x008400 Memory controller
//  0x008500 Power Management
//  0x400000 SMM System Code
//  0x800000 GFX memory
}

void mediagx_host_device::legacy_memory_map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(mediagx_host_device::vram_r), FUNC(mediagx_host_device::vram_w));
}

void mediagx_host_device::legacy_io_map(address_map &map)
{
	map(0x000, 0x02f).m(m_vga, FUNC(vga_device::io_map));
}

uint8_t mediagx_host_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_vga.target())->mem_r(offset);
}

void mediagx_host_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_vga.target())->mem_w(offset, data);
}
