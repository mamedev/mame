// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "voodoo_pci.h"

int voodoo_pci_device::m_type = 0;

static MACHINE_CONFIG_FRAGMENT( voodoo_1_pci )
	MCFG_DEVICE_ADD("voodoo", VOODOO_1, STD_VOODOO_1_CLOCK)
	MCFG_VOODOO_FBMEM(4)
	MCFG_VOODOO_TMUMEM(1, 0)
	MCFG_VOODOO_SCREEN_TAG("screen")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( voodoo_2_pci )
	MCFG_DEVICE_ADD("voodoo", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(4)
	MCFG_VOODOO_TMUMEM(1, 0)
	MCFG_VOODOO_SCREEN_TAG("screen")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( voodoo_banshee_pci )
	MCFG_DEVICE_ADD("voodoo", VOODOO_BANSHEE, STD_VOODOO_BANSHEE_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( voodoo_3_pci )
	MCFG_DEVICE_ADD("voodoo", VOODOO_3, STD_VOODOO_3_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
MACHINE_CONFIG_END

machine_config_constructor voodoo_pci_device::device_mconfig_additions() const
{
	switch (m_type) {
		case TYPE_VOODOO_1:
			return MACHINE_CONFIG_NAME( voodoo_1_pci );
			break;
		case TYPE_VOODOO_2:
			return MACHINE_CONFIG_NAME( voodoo_2_pci );
			break;
		case TYPE_VOODOO_BANSHEE:
			return MACHINE_CONFIG_NAME( voodoo_banshee_pci );
			break;
		//case TYPE_VOODOO_3
		default:
			return MACHINE_CONFIG_NAME( voodoo_3_pci );
			break;
	}
}

const device_type VOODOO_PCI = &device_creator<voodoo_pci_device>;

DEVICE_ADDRESS_MAP_START(config_map, 32, voodoo_pci_device)
	AM_RANGE(0x40, 0x4f) AM_READWRITE  (pcictrl_r,  pcictrl_w)
	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(voodoo_reg_map, 32, voodoo_pci_device)
	AM_RANGE(0x0, 0x00ffffff) AM_DEVREADWRITE("voodoo", voodoo_device, voodoo_r, voodoo_w)
ADDRESS_MAP_END


DEVICE_ADDRESS_MAP_START(banshee_reg_map, 32, voodoo_pci_device)
	AM_RANGE(0x0, 0x01ffffff) AM_DEVREADWRITE("voodoo", voodoo_banshee_device, banshee_r, banshee_w)
ADDRESS_MAP_END
DEVICE_ADDRESS_MAP_START(lfb_map, 32, voodoo_pci_device)
	AM_RANGE(0x0, 0x01ffffff) AM_DEVREADWRITE("voodoo", voodoo_banshee_device, banshee_fb_r, banshee_fb_w)
ADDRESS_MAP_END
DEVICE_ADDRESS_MAP_START(io_map, 32, voodoo_pci_device)
	AM_RANGE(0x000, 0x0ff) AM_DEVREADWRITE("voodoo", voodoo_banshee_device, banshee_io_r, banshee_io_w)
ADDRESS_MAP_END

voodoo_pci_device::voodoo_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, VOODOO_PCI, "Voodoo PCI", tag, owner, clock, "voodoo_pci", __FILE__),
		m_voodoo(*this, "voodoo"), m_fbmem(2), m_tmumem0(0), m_tmumem1(0)
{
}

void voodoo_pci_device::set_cpu_tag(const char *_cpu_tag)
{
	m_cpu_tag = _cpu_tag;
}

void voodoo_pci_device::device_start()
{
	voodoo_device::static_set_cpu_tag(m_voodoo, m_cpu_tag);
	voodoo_device::static_set_fbmem(m_voodoo, m_fbmem);
	voodoo_device::static_set_tmumem(m_voodoo, m_tmumem0, m_tmumem1);
	switch (m_type) {
		//void set_ids(UINT32 main_id, UINT8 revision, UINT32 pclass, UINT32 subsystem_id);
		case TYPE_VOODOO_1:
			set_ids(0x121a0001, 0x02, 0x000000, 0x000000);
			break;
		case TYPE_VOODOO_2:
			set_ids(0x121a0002, 0x02, 0x040000, 0x000000);
			break;
		case TYPE_VOODOO_BANSHEE:
			set_ids(0x121a0003, 0x02, 0x000003, 0x000000);
			break;
		//case TYPE_VOODOO_3
		default:
			set_ids(0x121a0005, 0x02, 0x000003, 0x000000);
			break;
	}
	pci_device::device_start();
	if (m_type<=TYPE_VOODOO_2) {
		add_map(16*1024*1024, M_MEM | M_PREF, FUNC(voodoo_pci_device::voodoo_reg_map));
	} else {
		add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::banshee_reg_map));
		add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::lfb_map));
		add_map(256, M_IO, FUNC(voodoo_pci_device::io_map));
	}
}

void voodoo_pci_device::device_reset()
{
	memset(m_pcictrl_reg, 0, sizeof(m_pcictrl_reg));
	pci_device::device_reset();
}

void voodoo_pci_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	logerror("%s: map_extra\n", this->tag());
	// Really awkward way of getting vga address space mapped
	// Should really be dependent on voodoo VGAINIT0 bit 8 and IO base + 0xc3 bit 0
	if (1) {
		// io map is on bank_infos[2]
		bank_info &bi = bank_infos[2];
		if(bi.adr==-1)
				return;
		if(UINT32(bi.adr) == UINT32(~(bi.size - 1)))
			return;

		UINT64 start;
		address_space *space;
		if(bi.flags & M_IO) {
			space = io_space;
			start = bi.adr + io_offset;
		} else {
			space = memory_space;
			start = bi.adr + memory_offset;
		}
		// The mapping needs to only check high address bits
		start = (start & 0xFFFF0000) + 0x300;
		UINT64 end = (start & 0xFFFF0000) + 0x3ef;
		space->install_device_delegate(start, end, *this, bi.map);
		logerror("%s: map %s at %0*x-%0*x\n", this->tag(), bi.map.name(), bi.flags & M_IO ? 4 : 8, UINT32(start), bi.flags & M_IO ? 4 : 8, UINT32(end));
	}

}

UINT32 voodoo_pci_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_voodoo->voodoo_update(bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}

// PCI bus control
READ32_MEMBER (voodoo_pci_device::pcictrl_r)
{
	UINT32 result = m_pcictrl_reg[offset];
	if (1)
		logerror("%06X:voodoo_pci_device pcictrl_r from offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, result, mem_mask);
	return result;
}
WRITE32_MEMBER (voodoo_pci_device::pcictrl_w)
{
	COMBINE_DATA(&m_pcictrl_reg[offset]);
	switch (offset) {
		case 0x0/4:  // The address map starts at 0x40
			// HW initEnable
			m_voodoo->voodoo_set_init_enable(data);
			logerror("%06X:voodoo_pci_device pcictrl_w to offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;
		default:
			logerror("%06X:voodoo_pci_device pcictrl_w to offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
			break;
	}
}
