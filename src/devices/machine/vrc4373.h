// license:BSD-3-Clause
// copyright-holders:Ted Green
// NEC VRC 4373 System Controller

#ifndef MAME_MACHINE_VRC4373_H
#define MAME_MACHINE_VRC4373_H

#pragma once

#include "pci.h"
#include "cpu/mips/mips3.h"

#define MCFG_VRC4373_ADD(_tag,  _cpu_tag) \
	MCFG_PCI_HOST_ADD(_tag, VRC4373, 0x1033005B, 0x00, 0x00000000) \
	downcast<vrc4373_device *>(device)->set_cpu_tag(_cpu_tag);

#define MCFG_VRC4373_SET_RAM(_size) \
	downcast<vrc4373_device *>(device)->set_ram_size(_size);

#define MCFG_VRC4373_SET_SIMM0(_size) \
	downcast<vrc4373_device *>(device)->set_simm0_size(_size);

#define MCFG_VRC4373_IRQ_CB(_devcb) \
	devcb = &ide_pci_device::set_irq_cb(*device, DEVCB_##_devcb);


class vrc4373_device : public pci_host_device {
public:
	vrc4373_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	void postload(void);

	template <class Object> static devcb_base &set_irq_cb(device_t &device, Object &&cb) { return downcast<vrc4373_device &>(device).m_irq_cb.set_callback(std::forward<Object>(cb)); }
	void set_cpu_tag(const char *tag);
	void set_ram_size(const int size) { m_ram_size = size; };
	void set_simm0_size(const int size) { m_simm0_size = size; };

	virtual void config_map(address_map &map) override;

	DECLARE_READ32_MEMBER(  pcictrl_r);
	DECLARE_WRITE32_MEMBER( pcictrl_w);
	//cpu bus registers
	DECLARE_READ32_MEMBER (cpu_if_r);
	DECLARE_WRITE32_MEMBER(cpu_if_w);

	DECLARE_READ32_MEMBER (master1_r);
	DECLARE_WRITE32_MEMBER(master1_w);

	DECLARE_READ32_MEMBER (master2_r);
	DECLARE_WRITE32_MEMBER(master2_w);

	DECLARE_READ32_MEMBER (master_io_r);
	DECLARE_WRITE32_MEMBER(master_io_w);

	virtual void target1_map(address_map &map);
	DECLARE_READ32_MEMBER (target1_r);
	DECLARE_WRITE32_MEMBER(target1_w);

	virtual void target2_map(address_map &map);
	DECLARE_READ32_MEMBER (target2_r);
	DECLARE_WRITE32_MEMBER(target2_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(dma_transfer);

	address_space *m_cpu_space;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	void cpu_map(address_map &map);

	void map_cpu_space();

	devcb_write_line m_irq_cb;

	mips3_device *m_cpu;
	const char *cpu_tag;
	int m_ram_size;
	int m_simm0_size;

	address_space_config m_mem_config, m_io_config;

	std::vector<uint32_t> m_ram;

	std::vector<uint32_t> m_simm[4];

	uint32_t m_cpu_regs[0x7c];

	uint32_t m_pci1_laddr, m_pci2_laddr, m_pci_io_laddr;
	uint32_t m_target1_laddr, m_target2_laddr;

	required_memory_region m_romRegion;

	emu_timer* m_dma_timer;
};


DECLARE_DEVICE_TYPE(VRC4373, vrc4373_device)

#endif // MAME_MACHINE_VRC4373_H
