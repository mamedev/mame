// license:???
// copyright-holders:???
#include "voodoo_pci.h"

static MACHINE_CONFIG_FRAGMENT( voodoo_pci )
	MCFG_DEVICE_ADD("voodoo", VOODOO_3, STD_VOODOO_3_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
MACHINE_CONFIG_END

machine_config_constructor voodoo_pci_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( voodoo_pci );
}

const device_type VOODOO_PCI = &device_creator<voodoo_pci_device>;

DEVICE_ADDRESS_MAP_START(reg_map, 32, voodoo_pci_device)
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
		m_voodoo(*this, "voodoo")
{
}

void voodoo_pci_device::set_cpu_tag(const char *_cpu_tag)
{
	m_cpu_tag = _cpu_tag;
}

void voodoo_pci_device::device_start()
{
	voodoo_device::static_set_cpu_tag(m_voodoo, m_cpu_tag);
	pci_device::device_start();
	add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::reg_map));
	add_map(32*1024*1024, M_MEM, FUNC(voodoo_pci_device::lfb_map));
	add_map(256, M_IO, FUNC(voodoo_pci_device::io_map));
}

void voodoo_pci_device::device_reset()
{
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
	return voodoo_update(m_voodoo, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}
