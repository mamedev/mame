// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "voodoo_pci.h"

#include "screen.h"


void voodoo_1_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_1(config, m_voodoo, voodoo_1_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(4);
	m_voodoo->set_tmumem(1, 0);
	m_voodoo->set_status_cycles(m_status_cycles);
	m_voodoo->set_cpu(m_cpu);
	m_voodoo->set_screen(m_screen);
}

void voodoo_2_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_2(config, m_voodoo, voodoo_2_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(4);
	m_voodoo->set_tmumem(1, 0);
	m_voodoo->set_status_cycles(m_status_cycles);
	m_voodoo->set_cpu(m_cpu);
	m_voodoo->set_screen(m_screen);
}

void voodoo_banshee_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_BANSHEE(config, m_voodoo, voodoo_banshee_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(16);
	m_voodoo->set_status_cycles(m_status_cycles);
	m_voodoo->set_cpu(m_cpu);
	m_voodoo->set_screen(m_screen);
}

void voodoo_3_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_3(config, m_voodoo, voodoo_3_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(16);
	m_voodoo->set_status_cycles(m_status_cycles);
	m_voodoo->set_cpu(m_cpu);
	m_voodoo->set_screen(m_screen);
}

DEFINE_DEVICE_TYPE(VOODOO_1_PCI, voodoo_1_pci_device, "voodoo_1_pci", "Voodoo 1 PCI")
DEFINE_DEVICE_TYPE(VOODOO_2_PCI, voodoo_2_pci_device, "voodoo_2_pci", "Voodoo 2 PCI")
DEFINE_DEVICE_TYPE(VOODOO_BANSHEE_PCI, voodoo_banshee_pci_device, "voodoo_banshee_pci", "Voodoo Banshee PCI")
DEFINE_DEVICE_TYPE(VOODOO_3_PCI, voodoo_3_pci_device, "voodoo_3_pci", "Voodoo 3 PCI")

DEFINE_DEVICE_TYPE(VOODOO_BANSHEE_X86_PCI, voodoo_banshee_x86_pci_device, "banshee_x86", "Voodoo Banshee PCI (x86)")


void voodoo_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x5f).rw(FUNC(voodoo_pci_device::pcictrl_r), FUNC(voodoo_pci_device::pcictrl_w));
}

voodoo_pci_device::voodoo_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pci_device(mconfig, type, tag, owner, clock),
	  m_generic_voodoo(*this, "voodoo"), m_cpu(*this, finder_base::DUMMY_TAG), m_screen(*this, finder_base::DUMMY_TAG), m_fbmem(2), m_tmumem0(0), m_tmumem1(0)
{
}

voodoo_1_pci_device::voodoo_1_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_pci_device(mconfig, VOODOO_1_PCI, tag, owner, clock), m_voodoo(*this, "voodoo")
{
}

voodoo_2_pci_device::voodoo_2_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_pci_device(mconfig, VOODOO_2_PCI, tag, owner, clock), m_voodoo(*this, "voodoo")
{
}

voodoo_banshee_pci_device::voodoo_banshee_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: voodoo_pci_device(mconfig, type, tag, owner, clock), m_voodoo(*this, "voodoo")
{
}

voodoo_banshee_pci_device::voodoo_banshee_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_pci_device(mconfig, VOODOO_BANSHEE_PCI, tag, owner, clock)
{
}

voodoo_3_pci_device::voodoo_3_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_pci_device(mconfig, VOODOO_3_PCI, tag, owner, clock), m_voodoo(*this, "voodoo")
{
}

void voodoo_pci_device::device_start()
{
	m_generic_voodoo->set_fbmem(m_fbmem);
	m_generic_voodoo->set_tmumem(m_tmumem0, m_tmumem1);
	m_generic_voodoo->set_status_cycles(m_status_cycles);
	pci_device::device_start();
	save_item(NAME(m_pcictrl_reg));
	machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_pci_device::postload), this));
}

void voodoo_1_pci_device::device_start()
{
	// NOTE: class code = 0 (backward compatible non-VGA device)
	set_ids(0x121a0001, 0x02, 0x000000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(16 * 1024 * 1024, M_MEM | M_PREF, *m_voodoo, FUNC(voodoo_1_device::core_map));
	bank_infos[0].adr = 0xff000000;

	command = 0;
	command_mask = 2;
	status = 0;

	// no max_gnt / max_lat (hardwired to 0, cannot bus master)

	intr_line = 5;
	// INTA#
	intr_pin = 1;
}

void voodoo_2_pci_device::device_start()
{
	// FIXME: proper PCI values (check manual)
	set_ids(0x121a0002, 0x02, 0x038000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(16 * 1024 * 1024, M_MEM | M_PREF, *m_voodoo, FUNC(voodoo_2_device::core_map));
	bank_infos[0].adr = 0xff000000;
}

void voodoo_banshee_pci_device::device_start()
{
	// FIXME: proper PCI values (check manual)
	set_ids(0x121a0003, 0x02, 0x030000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(32 * 1024 * 1024, M_MEM, *m_voodoo, FUNC(voodoo_banshee_device::core_map));
	add_map(32 * 1024 * 1024, M_MEM, *m_voodoo, FUNC(voodoo_banshee_device::lfb_map));
	add_map(256, M_IO, *m_voodoo, FUNC(voodoo_banshee_device::io_map));
	bank_infos[0].adr = 0xf8000000;
	bank_infos[1].adr = 0xf8000008;
	bank_infos[2].adr = 0xfffffff0;
}

void voodoo_3_pci_device::device_start()
{
	// FIXME: proper PCI values (check manual)
	set_ids(0x121a0005, 0x02, 0x030000, 0x000000);

	voodoo_pci_device::device_start();

	// need to downcast to voodoo_banshee_device because the map functions are shared
	// this will cleaner when the PCI interface is implemented as a mix-in
	auto &banshee = static_cast<voodoo_banshee_device &>(*m_voodoo);
	add_map(32 * 1024 * 1024, M_MEM, banshee, FUNC(voodoo_banshee_device::core_map));
	add_map(32 * 1024 * 1024, M_MEM, banshee, FUNC(voodoo_banshee_device::lfb_map));
	add_map(256, M_IO, banshee, FUNC(voodoo_banshee_device::io_map));
	bank_infos[0].adr = 0xf8000000;
	bank_infos[1].adr = 0xf8000008;
	bank_infos[2].adr = 0xfffffff0;
}

void voodoo_pci_device::postload()
{
	remap_cb();
}

void voodoo_pci_device::device_reset()
{
	memset(m_pcictrl_reg, 0, sizeof(m_pcictrl_reg));
	pci_device::device_reset();
}

void voodoo_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());
}

void voodoo_banshee_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());

	// Map VGA legacy access
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	u64 start = io_offset + 0x3b0;
	u64 end = io_offset + 0x3df;
	io_space->install_readwrite_handler(start, end, read32s_delegate(*this, FUNC(voodoo_pci_device::vga_r)), write32s_delegate(*this, FUNC(voodoo_pci_device::vga_w)));
	logerror("%s: map %s at %0*x-%0*x\n", this->tag(), "vga_r/w", 4, u32(start), 4, u32(end));
}

void voodoo_3_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());

	// Map VGA legacy access
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	u64 start = io_offset + 0x3b0;
	u64 end = io_offset + 0x3df;
	io_space->install_readwrite_handler(start, end, read32s_delegate(*this, FUNC(voodoo_pci_device::vga_r)), write32s_delegate(*this, FUNC(voodoo_pci_device::vga_w)));
	logerror("%s: map %s at %0*x-%0*x\n", this->tag(), "vga_r/w", 4, u32(start), 4, u32(end));
}

u32 voodoo_pci_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_generic_voodoo->update(bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}

// PCI bus control
u32 voodoo_pci_device::pcictrl_r(offs_t offset, u32 mem_mask)
{
	u32 result = m_pcictrl_reg[offset];
	// The address map starts at 0x40
	switch (offset + 0x40 / 4) {
	case 0x40/4:
	{
		// V1: initEnable: 11:0
		// V2: initEnable: Fab ID=19:16, Graphics Rev=15:12
		// Banshee/V3: fabID: ScratchPad=31:4 fabID=3:0
		auto model = m_generic_voodoo->model();
		if (model == voodoo::voodoo_model::VOODOO_2)
			result = (result & ~0xff000) | 0x00044000; // Vegas driver needs this value
		else if (model >= voodoo::voodoo_model::VOODOO_BANSHEE)
			result = (result & ~0xf) | 0x1;
		break;
	}
	case 0x54/4:
		// V2: SiProcess Register: Osc Force On, Osc Ring Sel, Osc Count Reset, 12 bit PCI Counter, 16 bit Oscillator Counter
		// V3: AGP Capability Register: 8 bit 0, 4 bit AGP Major, 4 bit AGP Minor, 8 bit Next Ptr, 8 bit Capability ID
		// Tenthdeg (vegas) checks this
		result = 0x00006002;
		break;
	}

	if (1)
		logerror("%s:voodoo_pci_device pcictrl_r from offset %02X = %08X & %08X\n", machine().describe_context(), offset*4 + 0x40, result, mem_mask);
	return result;
}
void voodoo_pci_device::pcictrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pcictrl_reg[offset]);
	// The address map starts at 0x40
	switch (offset + 0x40 / 4) {
		case 0x40/4:
			// HW initEnable
			m_generic_voodoo->set_init_enable(data);
			logerror("%s:voodoo_pci_device pcictrl_w (initEnable) offset %02X = %08X & %08X\n", machine().describe_context(), offset * 4 + 0x40, data, mem_mask);
			break;
		default:
			logerror("%s:voodoo_pci_device pcictrl_w to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4 + 0x40, data, mem_mask);
			break;
	}
}

// VGA legacy accesses
u32 voodoo_pci_device::vga_r(offs_t offset, u32 mem_mask)
{
	// map to I/O space at offset 0xb0
	return m_generic_voodoo->read(0xb0/4 + offset, mem_mask);
}
void voodoo_pci_device::vga_w(offs_t offset, u32 data, u32 mem_mask)
{
	// map to I/O space at offset 0xb0
	m_generic_voodoo->write(0xb0/4 + offset, data, mem_mask);
}

// x86 cards, same with additional BIOS ROM

voodoo_banshee_x86_pci_device::voodoo_banshee_x86_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_pci_device(mconfig, VOODOO_BANSHEE_X86_PCI, tag, owner, clock)
	, m_vga_rom(*this, "vga_rom")
{
}

void voodoo_banshee_x86_pci_device::device_start()
{
	voodoo_banshee_pci_device::device_start();

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;
}

ROM_START( voodoo_banshee )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "gainward", "Gainward Dragon 4000" )
	ROMX_LOAD( "gainward.bin", 0x000000, 0x008000, CRC(a53df538) SHA1(679f94619eac11c59effb89fe44bb74f589e3050), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
	ROM_SYSTEM_BIOS( 1, "atrend", "A-Trend Helios 3D" )
	ROMX_LOAD( "a-trend.vbi",  0x000000, 0x008000, CRC(117a9e6f) SHA1(48b0bc08d142be3aa0d937ee56afd299b3b20386), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *voodoo_banshee_x86_pci_device::device_rom_region() const
{
	return ROM_NAME(voodoo_banshee);
}
