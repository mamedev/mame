// license:BSD-3-Clause
// copyright-holders:Ted Green
// 3dfx Voodoo Graphics SST-1/2 emulator.

#ifndef VOODOO_PCI_H
#define VOODOO_PCI_H

#include "machine/pci.h"
#include "voodoo.h"

#define MCFG_VOODOO_PCI_ADD(_tag,  _type, _cpu_tag) \
	voodoo_pci_device::set_type(_type); \
	MCFG_PCI_DEVICE_ADD(_tag, VOODOO_PCI, 0, 0, 0, 0) \
	downcast<voodoo_pci_device *>(device)->set_cpu_tag(_cpu_tag);

#define MCFG_VOODOO_PCI_FBMEM(_value) \
	downcast<voodoo_pci_device *>(device)->set_fbmem(_value);

#define MCFG_VOODOO_PCI_TMUMEM(_value1, _value2) \
	downcast<voodoo_pci_device *>(device)->set_tmumem(_value1, _value2);

class voodoo_pci_device : public pci_device {
public:
	voodoo_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;

	void set_cpu_tag(const char *tag);
	static void set_type(const int type) {m_type = type;}
	void set_fbmem(const int fbmem) {m_fbmem = fbmem;}
	void set_tmumem(const int tmumem0, const int tmumem1) {m_tmumem0 = tmumem0; m_tmumem1 = tmumem1;}

	DECLARE_READ32_MEMBER(  pcictrl_r);
	DECLARE_WRITE32_MEMBER( pcictrl_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<voodoo_device> m_voodoo;
	static int m_type;
	int m_fbmem, m_tmumem0, m_tmumem1;
	const char *m_cpu_tag;

	UINT32 m_pcictrl_reg[0x10];
	DECLARE_ADDRESS_MAP(voodoo_reg_map, 32);
	DECLARE_ADDRESS_MAP(banshee_reg_map, 32);
	DECLARE_ADDRESS_MAP(lfb_map, 32);
	DECLARE_ADDRESS_MAP(io_map, 32);
};

extern const device_type VOODOO_PCI;

#endif
