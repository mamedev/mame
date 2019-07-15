// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "voodoo_pci.h"

#include "screen.h"


void voodoo_1_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_1(config, m_voodoo, STD_VOODOO_1_CLOCK);
	m_voodoo->set_fbmem(4);
	m_voodoo->set_tmumem(1, 0);
}

void voodoo_2_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_2(config, m_voodoo, STD_VOODOO_2_CLOCK);
	m_voodoo->set_fbmem(4);
	m_voodoo->set_tmumem(1, 0);
}

void voodoo_banshee_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_BANSHEE(config, m_voodoo, STD_VOODOO_BANSHEE_CLOCK);
	m_voodoo->set_fbmem(16);
}

void voodoo_3_pci_device::device_add_mconfig(machine_config &config)
{
	VOODOO_3(config, m_voodoo, STD_VOODOO_3_CLOCK);
	m_voodoo->set_fbmem(16);
}

DEFINE_DEVICE_TYPE(VOODOO_1_PCI, voodoo_1_pci_device, "voodoo_1_pci", "Voodoo 1 PCI")
DEFINE_DEVICE_TYPE(VOODOO_2_PCI, voodoo_2_pci_device, "voodoo_2_pci", "Voodoo 2 PCI")
DEFINE_DEVICE_TYPE(VOODOO_BANSHEE_PCI, voodoo_banshee_pci_device, "voodoo_banshee_pci", "Voodoo Banshee PCI")
DEFINE_DEVICE_TYPE(VOODOO_3_PCI, voodoo_3_pci_device, "voodoo_3_pci", "Voodoo 3 PCI")

void voodoo_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x5f).rw(FUNC(voodoo_pci_device::pcictrl_r), FUNC(voodoo_pci_device::pcictrl_w));
}

// VOODOO_1 & VOODOO_2 map
void voodoo_pci_device::voodoo_reg_map(address_map &map)
{
	map(0x0, 0x00ffffff).rw(m_voodoo, FUNC(voodoo_device::voodoo_r), FUNC(voodoo_device::voodoo_w));
}
// VOODOO_BANSHEE and VOODOO_3 maps
void voodoo_pci_device::banshee_reg_map(address_map &map)
{
	map(0x0, 0x01ffffff).rw(m_voodoo, FUNC(voodoo_banshee_device::banshee_r), FUNC(voodoo_banshee_device::banshee_w));
}

void voodoo_pci_device::lfb_map(address_map &map)
{
	map(0x0, 0x01ffffff).rw(m_voodoo, FUNC(voodoo_banshee_device::banshee_fb_r), FUNC(voodoo_banshee_device::banshee_fb_w));
}

void voodoo_pci_device::io_map(address_map &map)
{
	map(0x000, 0x0ff).rw(m_voodoo, FUNC(voodoo_banshee_device::banshee_io_r), FUNC(voodoo_banshee_device::banshee_io_w));
}

voodoo_pci_device::voodoo_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock),
	  m_voodoo(*this, "voodoo"), m_cpu(*this, finder_base::DUMMY_TAG), m_screen(*this, finder_base::DUMMY_TAG), m_fbmem(2), m_tmumem0(0), m_tmumem1(0)
{
}

voodoo_1_pci_device::voodoo_1_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: voodoo_pci_device(mconfig, VOODOO_1_PCI, tag, owner, clock)
{
}

voodoo_2_pci_device::voodoo_2_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: voodoo_pci_device(mconfig, VOODOO_2_PCI, tag, owner, clock)
{
}

voodoo_banshee_pci_device::voodoo_banshee_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: voodoo_pci_device(mconfig, VOODOO_BANSHEE_PCI, tag, owner, clock)
{
}

voodoo_3_pci_device::voodoo_3_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: voodoo_pci_device(mconfig, VOODOO_3_PCI, tag, owner, clock)
{
}

void voodoo_pci_device::device_start()
{
	if (m_cpu)
		m_voodoo->set_cpu(*m_cpu);
	if (m_screen)
		m_voodoo->set_screen(*m_screen);
	m_voodoo->set_fbmem(m_fbmem);
	m_voodoo->set_tmumem(m_tmumem0, m_tmumem1);
	pci_device::device_start();
	save_item(NAME(m_pcictrl_reg));
	machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_pci_device::postload), this));
}

//void set_ids(uint32_t main_id, uint8_t revision, uint32_t pclass, uint32_t subsystem_id);
void voodoo_1_pci_device::device_start()
{
	set_ids(0x121a0001, 0x02, 0x030000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(16 * 1024 * 1024, M_MEM | M_PREF, FUNC(voodoo_pci_device::voodoo_reg_map));
	bank_infos[0].adr = 0xff000000;
}

void voodoo_2_pci_device::device_start()
{
	set_ids(0x121a0002, 0x02, 0x038000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(16 * 1024 * 1024, M_MEM | M_PREF, FUNC(voodoo_pci_device::voodoo_reg_map));
	bank_infos[0].adr = 0xff000000;
}

void voodoo_banshee_pci_device::device_start()
{
	set_ids(0x121a0003, 0x02, 0x030000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(32 * 1024 * 1024, M_MEM, FUNC(voodoo_pci_device::banshee_reg_map));
	add_map(32 * 1024 * 1024, M_MEM, FUNC(voodoo_pci_device::lfb_map));
	add_map(256, M_IO, FUNC(voodoo_pci_device::io_map));
	bank_infos[0].adr = 0xf8000000;
	bank_infos[1].adr = 0xf8000008;
	bank_infos[2].adr = 0xfffffff0;
}

void voodoo_3_pci_device::device_start()
{
	set_ids(0x121a0005, 0x02, 0x030000, 0x000000);

	voodoo_pci_device::device_start();

	add_map(32 * 1024 * 1024, M_MEM, FUNC(voodoo_pci_device::banshee_reg_map));
	add_map(32 * 1024 * 1024, M_MEM, FUNC(voodoo_pci_device::lfb_map));
	add_map(256, M_IO, FUNC(voodoo_pci_device::io_map));
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

void voodoo_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());
}

void voodoo_banshee_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());

	// Map VGA legacy access
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	uint64_t start = io_offset + 0x3b0;
	uint64_t end = io_offset + 0x3df;
	io_space->install_readwrite_handler(start, end, read32_delegate(FUNC(voodoo_pci_device::vga_r), this), write32_delegate(FUNC(voodoo_pci_device::vga_w), this));
	logerror("%s: map %s at %0*x-%0*x\n", this->tag(), "vga_r/w", 4, uint32_t(start), 4, uint32_t(end));
}

void voodoo_3_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());

	// Map VGA legacy access
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	uint64_t start = io_offset + 0x3b0;
	uint64_t end = io_offset + 0x3df;
	io_space->install_readwrite_handler(start, end, read32_delegate(FUNC(voodoo_pci_device::vga_r), this), write32_delegate(FUNC(voodoo_pci_device::vga_w), this));
	logerror("%s: map %s at %0*x-%0*x\n", this->tag(), "vga_r/w", 4, uint32_t(start), 4, uint32_t(end));
}

uint32_t voodoo_pci_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_voodoo->voodoo_update(bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}

// PCI bus control
READ32_MEMBER (voodoo_pci_device::pcictrl_r)
{
	uint32_t result = m_pcictrl_reg[offset];
	// The address map starts at 0x40
	switch (offset + 0x40 / 4) {
	case 0x40/4:
		// V1: initEnable: 11:0
		// V2: initEnable: Fab ID=19:16, Graphics Rev=15:12
		// Banshee/V3: fabID: ScratchPad=31:4 fabID=3:0
		if (m_voodoo->vd_type== TYPE_VOODOO_2)
			result = (result & ~0xff000) | 0x00044000; // Vegas driver needs this value
		else if (m_voodoo->vd_type >= TYPE_VOODOO_BANSHEE)
			result = (result & ~0xf) | 0x1;
		break;
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
WRITE32_MEMBER (voodoo_pci_device::pcictrl_w)
{
	COMBINE_DATA(&m_pcictrl_reg[offset]);
	// The address map starts at 0x40
	switch (offset + 0x40 / 4) {
		case 0x40/4:
			// HW initEnable
			m_voodoo->voodoo_set_init_enable(data);
			logerror("%s:voodoo_pci_device pcictrl_w (initEnable) offset %02X = %08X & %08X\n", machine().describe_context(), offset * 4 + 0x40, data, mem_mask);
			break;
		default:
			logerror("%s:voodoo_pci_device pcictrl_w to offset %02X = %08X & %08X\n", machine().describe_context(), offset*4 + 0x40, data, mem_mask);
			break;
	}
}

// VGA legacy accesses
READ32_MEMBER(voodoo_pci_device::vga_r)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(offset * 4 + 0 + 0xb0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(offset * 4 + 1 + 0xb0) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(offset * 4 + 2 + 0xb0) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(offset * 4 + 3 + 0xb0) << 24;
	if (0)
		logerror("%s voodoo_pci_device vga_r from offset %02X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}
WRITE32_MEMBER(voodoo_pci_device::vga_w)
{
	if (ACCESSING_BITS_0_7)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(offset * 4 + 0 + 0xb0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(offset * 4 + 1 + 0xb0, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(offset * 4 + 2 + 0xb0, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(offset * 4 + 3 + 0xb0, data >> 24);

	if (0)
		logerror("%s voodoo_pci_device vga_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}
