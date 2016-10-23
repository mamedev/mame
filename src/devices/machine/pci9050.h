// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pci9050.h - PLX PCI9050 PCI to 4x Local Bus Bridge

    by R. Belmont

*********************************************************************/

#ifndef _PCI9050_H
#define _PCI9050_H

#include "machine/pci.h"

#define MCFG_PCI9050_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI9050, 0x10b59050, 0x01, 0x06800000, 0x10b59050)

#define MCFG_PCI9050_SET_MAP(id, map) \
	downcast<pci9050_device *>(device)->set_map(id, ADDRESS_MAP_NAME(map), #map, owner);

#define MCFG_PCI9050_USER_INPUT_CALLBACK(_write) \
	devcb = &pci9050_device::set_user_input_callback(*device, DEVCB_##_write);

#define MCFG_PCI9050_USER_OUTPUT_CALLBACK(_read) \
	devcb = &pci9050_device::set_user_output_callback(*device, DEVCB_##_read);

class pci9050_device :
	public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// PCI9050 I/O register space handlers
	uint32_t lasrr_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void lasrr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t eromrr_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eromrr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t lasba_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void lasba_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t eromba_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eromba_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t lasbrd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void lasbrd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t erombrd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void erombrd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t csbase_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void csbase_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t intcsr_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void intcsr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cntrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cntrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	template<class _Object> static devcb_base &set_user_input_callback(device_t &device, _Object object) { return downcast<pci9050_device &>(device).m_user_input_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_user_output_callback(device_t &device, _Object object) { return downcast<pci9050_device &>(device).m_user_output_handler.set_callback(object); }

	void set_map(int id, address_map_constructor map, const char *name, device_t *device);

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
	DECLARE_ADDRESS_MAP(empty, 32);

	const char *m_names[4];
	device_t *m_devices[4];
	address_map_constructor m_maps[4];

	uint32_t m_lasrr[4], m_lasba[4], m_lasbrd[4], m_csbase[4];
	uint32_t m_eromrr, m_eromba, m_erombrd, m_intcsr, m_cntrl;

	void remap_local(int id);
	void remap_rom();

	template<int id> void map_trampoline(address_map &map, device_t &device) {
		m_maps[id](map, *m_devices[id]);
	}

	devcb_read32 m_user_input_handler;
	devcb_write32 m_user_output_handler;

};

extern const device_type PCI9050;

#endif
