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

// Supports R4600/4650/4700/R5000 CPUs
#define MCFG_GT64010_ADD(_tag,  _cpu_tag, _clock, _irq_num) \
	MCFG_PCI_HOST_ADD(_tag, GT64XXX, 0x11ab0146, 0x03, 0x00000000) \
	downcast<gt64xxx_device *>(device)->set_cpu_tag(_cpu_tag); \
	downcast<gt64xxx_device *>(device)->set_clock(_clock); \
	downcast<gt64xxx_device *>(device)->set_irq_num(_irq_num);

// Supports the following 32-bit bus CPUs:
// IDT RC4640 and RC4650 (in 32-bit mode)
// QED RM523X
// NEC/Toshiba VR4300
#define MCFG_GT64111_ADD(_tag,  _cpu_tag, _clock, _irq_num) \
	MCFG_PCI_DEVICE_ADD(_tag, GT64XXX, 0x414611ab, 0x10, 0x058000, 0x00000000) \
	downcast<gt64xxx_device *>(device)->set_cpu_tag(_cpu_tag); \
	downcast<gt64xxx_device *>(device)->set_clock(_clock); \
	downcast<gt64xxx_device *>(device)->set_irq_num(_irq_num);

#define MCFG_GT64XXX_SET_BE_CPU(_be) \
	downcast<gt64xxx_device *>(device)->set_be(_be);

#define MCFG_GT64XXX_IRQ_ADD(_irq_num) \
	downcast<gt64xxx_device *>(device)->set_irq_info(_irq_num);

#define MCFG_GT64XXX_SET_CS(_cs_num, _map) \
	downcast<gt64xxx_device *>(device)->set_map(_cs_num, address_map_constructor(&_map, #_map, this), this);

#define MCFG_GT64XX_SET_SIMM(_index, _size) \
	downcast<gt64xxx_device *>(device)->set_simm_size(_index, _size);

#define MCFG_GT64XX_SET_SIMM0(_size) \
	downcast<gt64xxx_device *>(device)->set_simm0_size(_size);

#define MCFG_GT64XX_SET_SIMM1(_size) \
	downcast<gt64xxx_device *>(device)->set_simm1_size(_size);

/*************************************
 *  Structures
 *************************************/
class gt64xxx_device : public pci_host_device {
public:
	gt64xxx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	void set_cpu_tag(const char *tag) { cpu_tag = tag;}
	void set_clock(const uint32_t clock) {m_clock = clock;}
	void set_be(const int be) {m_be = be;}
	void set_autoconfig(const int autoconfig) {m_autoconfig = autoconfig;}
	void set_irq_num(const int irq_num) {m_irq_num = irq_num;}
	virtual void config_map(address_map &map) override;
	void set_simm_size(const int index, const int size) { m_simm_size[index] = size; };
	void set_simm0_size(const int size) { m_simm_size[0] = size; };
	void set_simm1_size(const int size) { m_simm_size[1] = size; };

	DECLARE_WRITE_LINE_MEMBER(pci_stall);

	// pci bus
	DECLARE_READ32_MEMBER(  pci_config_r);
	DECLARE_WRITE32_MEMBER( pci_config_w);

	// cpu bus
	DECLARE_READ32_MEMBER (cpu_if_r);
	DECLARE_WRITE32_MEMBER(cpu_if_w);

	DECLARE_READ32_MEMBER (master_mem0_r);
	DECLARE_WRITE32_MEMBER(master_mem0_w);

	DECLARE_READ32_MEMBER (master_mem1_r);
	DECLARE_WRITE32_MEMBER(master_mem1_w);

	DECLARE_READ32_MEMBER (master_io_r);
	DECLARE_WRITE32_MEMBER(master_io_w);

	// devices
	DECLARE_READ32_MEMBER (ras_0_r);
	DECLARE_WRITE32_MEMBER(ras_0_w);
	DECLARE_READ32_MEMBER(ras_1_r);
	DECLARE_WRITE32_MEMBER(ras_1_w);
	DECLARE_READ32_MEMBER(ras_2_r);
	DECLARE_WRITE32_MEMBER(ras_2_w);
	DECLARE_READ32_MEMBER(ras_3_r);
	DECLARE_WRITE32_MEMBER(ras_3_w);
	DECLARE_READ32_MEMBER (cs_0_r);
	DECLARE_WRITE32_MEMBER(cs_0_w);

	// Enums
	enum proc_addr_bank {ADDR_RAS1_0, ADDR_RAS3_2, ADDR_CS2_0, ADDR_CS3_BCS, ADDR_PCI_IO, ADDR_PCI_MEM0, ADDR_PCI_MEM1, ADDR_NUM};

	void set_map(int id, const address_map_constructor &map, device_t *device);
	void postload();

protected:
	address_space *m_cpu_space;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override;
	virtual void device_reset() override;


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

	mips3_device *m_cpu;
	const char *cpu_tag;
	uint32_t m_clock;
	int m_be, m_autoconfig;
	int m_irq_num;
	int m_simm_size[4];

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

	void cpu_map(address_map &map);

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

DECLARE_DEVICE_TYPE(GT64XXX, gt64xxx_device)

#endif // MAME_MACHINE_GT64XXX_H
