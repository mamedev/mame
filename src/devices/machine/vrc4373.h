// license:BSD-3-Clause
// copyright-holders:Ted Green
// NEC VRC 4373 System Controller

#ifndef VRC4373_H
#define VRC4373_H

#include "pci.h"
#include "cpu/mips/mips3.h"

#define MCFG_VRC4373_ADD(_tag,  _cpu_tag) \
	MCFG_PCI_HOST_ADD(_tag, VRC4373, 0x005B1033, 0x00, 0x00000000) \
	downcast<vrc4373_device *>(device)->set_cpu_tag(_cpu_tag);

#define VRC4373_PAGESHIFT 12

/* NILE 3 registers 0x000-0x0ff */
#define NREG_BMCR           (0x000/4)
#define NREG_SIMM1            (0x004/4)
#define NREG_SIMM2            (0x008/4)
#define NREG_SIMM3            (0x00C/4)
#define NREG_SIMM4            (0x010/4)
#define NREG_PCIMW1         (0x014/4)
#define NREG_PCIMW2         (0x018/4)
#define NREG_PCITW1         (0x01C/4)
#define NREG_PCITW2         (0x020/4)
#define NREG_PCIMIOW        (0x024/4)
#define NREG_PCICDR         (0x028/4)
#define NREG_PCICAR         (0x02C/4)
#define NREG_PCIMB1         (0x030/4)
#define NREG_PCIMB2         (0x034/4)
#define NREG_DMACR1         (0x038/4)
#define NREG_DMAMAR1        (0x03C/4)
#define NREG_DMAPCI1        (0x040/4)
#define NREG_DMACR2         (0x044/4)
#define NREG_DMAMAR2        (0x048/4)
#define NREG_DMAPCI2        (0x04C/4)

#define NREG_BESR           (0x050/4)
#define NREG_ICSR           (0x054/4)
#define NREG_DRAMRCR        (0x058/4)
#define NREG_BOOTWP         (0x05C/4)
#define NREG_PCIEAR         (0x060/4)
#define NREG_DMA_REM        (0x064/4)
#define NREG_DMA_CMAR       (0x068/4)
#define NREG_DMA_CPAR       (0x06C/4)
#define NREG_PCIRC          (0x070/4)
#define NREG_PCIEN          (0x074/4)
#define NREG_PMIR           (0x078/4)

#define DMA_BUSY                0x80000000
#define DMA_INT_EN          0x40000000
#define DMA_RW                  0x20000000
#define DMA_GO                  0x10000000
#define DMA_SUS                 0x08000000
#define DMA_INC                 0x04000000
#define DMA_MIO                 0x02000000
#define DMA_RST                 0x01000000
#define DMA_BLK_SIZE        0x000fffff


class vrc4373_device : public pci_host_device {
public:
	vrc4373_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

	void set_cpu_tag(const char *tag);

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;

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

	virtual DECLARE_ADDRESS_MAP(target1_map, 32);
	DECLARE_READ32_MEMBER (target1_r);
	DECLARE_WRITE32_MEMBER(target1_w);

	virtual DECLARE_ADDRESS_MAP(target2_map, 32);
	DECLARE_READ32_MEMBER (target2_r);
	DECLARE_WRITE32_MEMBER(target2_w);

protected:
	address_space *m_cpu_space;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	void dma_transfer(int which);

private:
	mips3_device *m_cpu;
	const char *cpu_tag;

	address_space_config m_mem_config, m_io_config;

	DECLARE_ADDRESS_MAP(cpu_map, 32);

	void map_cpu_space();

	UINT32 m_ram_size;
	UINT32 m_ram_base;
	std::vector<UINT32> m_ram;

	UINT32 m_simm_size;
	UINT32 m_simm_base;
	std::vector<UINT32> m_simm;

	UINT32 m_cpu_regs[0x7c];

	UINT32 m_pci1_laddr, m_pci2_laddr, m_pci_io_laddr;
	UINT32 m_target1_laddr, m_target2_laddr;

};


extern const device_type VRC4373;

#endif
