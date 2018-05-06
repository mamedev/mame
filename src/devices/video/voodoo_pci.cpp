// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "voodoo_pci.h"

#include "screen.h"


int voodoo_pci_device::m_type = 0;

MACHINE_CONFIG_START(voodoo_pci_device::device_add_mconfig)
	switch (m_type) {
		case TYPE_VOODOO_1:
				MCFG_DEVICE_ADD("voodoo", VOODOO_1, STD_VOODOO_1_CLOCK)
				MCFG_VOODOO_FBMEM(4)
				MCFG_VOODOO_TMUMEM(1, 0)
				MCFG_VOODOO_SCREEN_TAG("screen")
			break;
		case TYPE_VOODOO_2:
				MCFG_DEVICE_ADD("voodoo", VOODOO_2, STD_VOODOO_2_CLOCK)
				MCFG_VOODOO_FBMEM(4)
				MCFG_VOODOO_TMUMEM(1, 0)
				MCFG_VOODOO_SCREEN_TAG("screen")
			break;
		case TYPE_VOODOO_BANSHEE:
				MCFG_DEVICE_ADD("voodoo", VOODOO_BANSHEE, STD_VOODOO_BANSHEE_CLOCK)
				MCFG_VOODOO_FBMEM(16)
				MCFG_VOODOO_SCREEN_TAG("screen")
			break;
		//case TYPE_VOODOO_3
		default:
				MCFG_DEVICE_ADD("voodoo", VOODOO_3, STD_VOODOO_3_CLOCK)
				MCFG_VOODOO_FBMEM(16)
				MCFG_VOODOO_SCREEN_TAG("screen")
			break;}
MACHINE_CONFIG_END


DEFINE_DEVICE_TYPE(VOODOO_PCI, voodoo_pci_device, "voodoo_pci", "Voodoo PCI")

void voodoo_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x5f).rw(this, FUNC(voodoo_pci_device::pcictrl_r), FUNC(voodoo_pci_device::pcictrl_w));
}

// VOODOO_1 & VOODOO_2 map
void voodoo_pci_device::voodoo_reg_map(address_map &map)
{
	map(0x0, 0x00ffffff).rw("voodoo", FUNC(voodoo_device::voodoo_r), FUNC(voodoo_device::voodoo_w));
}
// VOODOO_BANSHEE and VOODOO_3 maps
void voodoo_pci_device::banshee_reg_map(address_map &map)
{
	map(0x0, 0x01ffffff).rw("voodoo", FUNC(voodoo_banshee_device::banshee_r), FUNC(voodoo_banshee_device::banshee_w));
}

void voodoo_pci_device::lfb_map(address_map &map)
{
	map(0x0, 0x01ffffff).rw("voodoo", FUNC(voodoo_banshee_device::banshee_fb_r), FUNC(voodoo_banshee_device::banshee_fb_w));
}

void voodoo_pci_device::io_map(address_map &map)
{
	map(0x000, 0x0ff).rw("voodoo", FUNC(voodoo_banshee_device::banshee_io_r), FUNC(voodoo_banshee_device::banshee_io_w));
}

voodoo_pci_device::voodoo_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, VOODOO_PCI, tag, owner, clock),
		m_voodoo(*this, "voodoo"), m_fbmem(2), m_tmumem0(0), m_tmumem1(0)
{
}

void voodoo_pci_device::set_cpu_tag(const char *_cpu_tag)
{
	m_cpu_tag = _cpu_tag;
}

void voodoo_pci_device::device_start()
{
	m_voodoo->set_cpu_tag(m_cpu_tag);
	m_voodoo->set_fbmem(m_fbmem);
	m_voodoo->set_tmumem(m_tmumem0, m_tmumem1);
	switch (m_type) {
		//void set_ids(uint32_t main_id, uint8_t revision, uint32_t pclass, uint32_t subsystem_id);
		case TYPE_VOODOO_1:
			set_ids(0x121a0001, 0x02, 0x030000, 0x000000);
			break;
		case TYPE_VOODOO_2:
			set_ids(0x121a0002, 0x02, 0x038000, 0x000000);
			break;
		case TYPE_VOODOO_BANSHEE:
			set_ids(0x121a0003, 0x02, 0x030000, 0x000000);
			break;
		//case TYPE_VOODOO_3
		default:
			set_ids(0x121a0005, 0x02, 0x030000, 0x000000);
			break;
	}
	pci_device::device_start();
	if (m_type<=TYPE_VOODOO_2) {
		add_map(16*1024*1024, M_MEM | M_PREF, FUNC(voodoo_pci_device::voodoo_reg_map));
		bank_infos[0].adr = 0xff000000;
	} else {
		add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::banshee_reg_map));
		add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::lfb_map));
		add_map(256, M_IO, FUNC(voodoo_pci_device::io_map));
		bank_infos[0].adr = 0xf8000000;
		bank_infos[1].adr = 0xf8000008;
		bank_infos[2].adr = 0xfffffff0;
	}
	save_item(NAME(m_pcictrl_reg));
	machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_pci_device::postload), this));
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
	// Map VGA legacy access
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	if (m_type>=TYPE_VOODOO_BANSHEE) {
		uint64_t start = io_offset + 0x3b0;
		uint64_t end = io_offset + 0x3df;
		io_space->install_readwrite_handler(start, end, read32_delegate(FUNC(voodoo_pci_device::vga_r), this), write32_delegate(FUNC(voodoo_pci_device::vga_w), this));
		logerror("%s: map %s at %0*x-%0*x\n", this->tag(), "vga_r/w", 4, uint32_t(start), 4, uint32_t(end));
	}
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
		// V2: Init Enable: 19:16=Fab ID, 15:12=Graphics Rev
		// Vegas driver needs this value at PCI 0x40
		result = 0x00044000;
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
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(space, offset * 4 + 0 + 0xb0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(space, offset * 4 + 1 + 0xb0, mem_mask >> 8) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(space, offset * 4 + 2 + 0xb0, mem_mask >> 16) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_r(space, offset * 4 + 3 + 0xb0, mem_mask >> 24) << 24;
	if (0)
		logerror("%s voodoo_pci_device vga_r from offset %02X = %08X & %08X\n", machine().describe_context(), offset * 4, result, mem_mask);
	return result;
}
WRITE32_MEMBER(voodoo_pci_device::vga_w)
{
	if (ACCESSING_BITS_0_7)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(space, offset * 4 + 0 + 0xb0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(space, offset * 4 + 1 + 0xb0, data >> 8, mem_mask >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(space, offset * 4 + 2 + 0xb0, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<voodoo_banshee_device *>(m_voodoo.target())->banshee_vga_w(space, offset * 4 + 3 + 0xb0, data >> 24, mem_mask >> 24);

	if (0)
		logerror("%s voodoo_pci_device vga_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}
