// 3dfx Voodoo Graphics SST-1/2 emulator.

#ifndef VOODOO_PCI_H
#define VOODOO_PCI_H

#include "machine/pci.h"
#include "voodoo.h"

#define MCFG_VOODOO_ADD(_tag,  _cpu_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, VOODOO_PCI, 0x121a0005, 0x02, 0x000003, 0x000000) \
	downcast<voodoo_pci_device *>(device)->set_cpu_tag(_cpu_tag);

class voodoo_pci_device : public pci_device {
public:
	voodoo_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void set_cpu_tag(const char *tag);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	required_device<voodoo_banshee_device> m_voodoo;
	const char *m_cpu_tag;

	DECLARE_ADDRESS_MAP(reg_map, 32);
	DECLARE_ADDRESS_MAP(lfb_map, 32);
	DECLARE_ADDRESS_MAP(io_map, 32);
};

extern const device_type VOODOO_PCI;

#endif
