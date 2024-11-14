// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
// Galileo GT-64xxx System Controller
// Skeleton code based off seattle machine driver.
// TODO:
// Need PCI to be able to have a target delay (pci bus stall) a dma transfer
// Configurable byte swapping on cpu and pci busses.

#ifndef MAME_MACHINE_GT64XXX_H
#define MAME_MACHINE_GT64XXX_H

#include "pci.h"
#include "cpu/mips/mips3.h"

DECLARE_DEVICE_TYPE(GT64010, gt64010_device)
DECLARE_DEVICE_TYPE(GT64111, gt64111_device)

/*************************************
 *  Structures
 *************************************/
class gt64xxx_device : public pci_host_device {
public:
	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_be(int be) { m_be = be; }
	void set_autoconfig(int autoconfig) { m_autoconfig = autoconfig; }
	void set_irq_num(int irq_num) { m_irq_num = irq_num; }
	virtual void config_map(address_map &map) override ATTR_COLD;
	void set_simm_size(int index, int size) { m_simm_size[index] = size; }
	void set_simm0_size(int size) { m_simm_size[0] = size; }
	void set_simm1_size(int size) { m_simm_size[1] = size; }

	void pci_stall(int state);

	// pci bus
	uint32_t pci_config_r(offs_t offset, uint32_t mem_mask = ~0);
	void pci_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// cpu bus
	uint32_t cpu_if_r(offs_t offset);
	void cpu_if_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master_mem0_r(offs_t offset, uint32_t mem_mask = ~0);
	void master_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master_mem1_r(offs_t offset, uint32_t mem_mask = ~0);
	void master_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master_io_r(offs_t offset, uint32_t mem_mask = ~0);
	void master_io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// devices
	uint32_t ras_0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ras_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ras_1_r(offs_t offset, uint32_t mem_mask = ~0);
	void ras_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ras_2_r(offs_t offset, uint32_t mem_mask = ~0);
	void ras_2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ras_3_r(offs_t offset, uint32_t mem_mask = ~0);
	void ras_3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// Enums
	enum proc_addr_bank {ADDR_RAS1_0, ADDR_RAS3_2, ADDR_CS2_0, ADDR_CS3_BCS, ADDR_PCI_IO, ADDR_PCI_MEM0, ADDR_PCI_MEM1, ADDR_NUM};

	void set_map(int id, const address_map_constructor &map, device_t *device);
	virtual void device_post_load() override;

protected:
	gt64xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	address_space *m_cpu_space;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;


private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	struct galileo_timer
	{
		emu_timer *     timer;
		uint32_t          count;
		uint8_t           active;
	};

	struct galileo_addr_map
	{
		uint32_t low_addr;
		uint32_t high_addr;
		address_space* space;
		galileo_addr_map() : low_addr(0xffffffff), high_addr(0x0) {}
	};

	required_device<mips3_device> m_cpu;
	int m_be, m_autoconfig;
	int m_irq_num;
	int m_simm_size[4];

	int m_irq_state;
	uint32_t m_irq_pending;
	int m_pci_stall_state;
	int m_retry_count;
	int m_pci_cpu_stalled;
	uint32_t m_stall_windex;
	uint32_t m_cpu_stalled_offset[2];
	uint32_t m_cpu_stalled_data[2];
	uint32_t m_cpu_stalled_mem_mask[2];

	address_space_config m_mem_config, m_io_config;

	required_memory_region m_romRegion;
	optional_memory_region m_updateRegion;

	void cpu_map(address_map &map) ATTR_COLD;

	void map_cpu_space();

	uint32_t m_prev_addr;
	/* raw register data */
	uint32_t          m_reg[0xd00/4];

	/* timer info */
	galileo_timer   m_timer[4];
	TIMER_CALLBACK_MEMBER(timer_callback);

	/* DMA info */
	int8_t            m_dma_active;

	// Ram
	std::vector<uint32_t> m_ram[4];

	// Chip Select
	device_t *m_cs_devices[4];
	address_map_constructor m_cs_maps[4];

	void update_irqs();

	int m_last_dma;
	emu_timer* m_dma_timer;
	galileo_addr_map dma_addr_map[proc_addr_bank::ADDR_NUM];
	int dma_fetch_next(address_space &space, int which);
	TIMER_CALLBACK_MEMBER(perform_dma);
	address_space* dma_decode_address(uint32_t &addr);
};

// Supports R4600/4650/4700/R5000 CPUs
class gt64010_device : public gt64xxx_device {
public:
	template <typename T>
	gt64010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int irq_num)
		: gt64010_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x11ab0146, 0x03, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_irq_num(irq_num);
	}
	gt64010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: gt64xxx_device(mconfig, GT64010, tag, owner, clock) {}
};

// Supports the following 32-bit bus CPUs:
// IDT RC4640 and RC4650 (in 32-bit mode)
// QED RM523X
// NEC/Toshiba VR4300
class gt64111_device : public gt64xxx_device {
public:
	template <typename T>
	gt64111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int irq_num)
		: gt64111_device(mconfig, tag, owner, clock)
	{
		set_ids(0x414611ab, 0x10, 0x058000, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_irq_num(irq_num);
	}
	gt64111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: gt64xxx_device(mconfig, GT64111, tag, owner, clock) {}
};

#endif // MAME_MACHINE_GT64XXX_H
