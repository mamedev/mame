// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
// NEC VRC 5074 System Controller

#ifndef VRC5074_H
#define VRC5074_H

#include "pci.h"
#include "cpu/mips/mips3.h"

#define MCFG_VRC5074_ADD(_tag, _cpu_tag) \
	MCFG_PCI_HOST_ADD(_tag, VRC5074, 0x1033005a, 0x04, 0x00000000) \
	downcast<vrc5074_device *>(device)->set_cpu_tag(_cpu_tag);

#define MCFG_VRC5074_SET_SDRAM(_index, _size) \
	downcast<vrc5074_device *>(device)->set_sdram_size(_index, _size);

#define MCFG_VRC5074_SET_CS(_cs_num, _map) \
	downcast<vrc5074_device *>(device)->set_map(_cs_num, address_map_delegate(ADDRESS_MAP_NAME(_map), #_map), owner);

/* NILE 4 registers 0x000-0x0ff */
#define NREG_SDRAM0         (0x000/4)
#define NREG_SDRAM1         (0x008/4)
#define NREG_DCS2           (0x010/4)   /* SIO misc */
#define NREG_DCS3           (0x018/4)   /* ADC */
#define NREG_DCS4           (0x020/4)   /* CMOS */
#define NREG_DCS5           (0x028/4)   /* SIO */
#define NREG_DCS6           (0x030/4)   /* IOASIC */
#define NREG_DCS7           (0x038/4)   /* ethernet */
#define NREG_DCS8           (0x040/4)
#define NREG_PCIW0          (0x060/4)
#define NREG_PCIW1          (0x068/4)
#define NREG_INTCS          (0x070/4)
#define NREG_BOOTCS         (0x078/4)
#define NREG_CPUSTAT        (0x080/4)
#define NREG_INTCTRL        (0x088/4)
#define NREG_INTSTAT0       (0x090/4)
#define NREG_INTSTAT1       (0x098/4)
#define NREG_INTCLR         (0x0A0/4)
#define NREG_INTPPES        (0x0A8/4)
#define NREG_PCIERR         (0x0B8/4)
#define NREG_MEMCTRL        (0x0C0/4)
#define NREG_ACSTIME        (0x0C8/4)
#define NREG_CHKERR         (0x0D0/4)
#define NREG_PCICTRL        (0x0E0/4)
#define NREG_PCIARB         (0x0E8/4)
#define NREG_PCIINIT0       (0x0F0/4)
#define NREG_PCIINIT1       (0x0F8/4)

/* NILE 4 registers 0x100-0x1ff */
#define NREG_LCNFG          (0x100/4)
#define NREG_LCST2          (0x110/4)
#define NREG_LCST3          (0x118/4)
#define NREG_LCST4          (0x120/4)
#define NREG_LCST5          (0x128/4)
#define NREG_LCST6          (0x130/4)
#define NREG_LCST7          (0x138/4)
#define NREG_LCST8          (0x140/4)
#define NREG_DCSFN          (0x150/4)
#define NREG_DCSIO          (0x158/4)
#define NREG_BCST           (0x178/4)
#define NREG_DMACTRL0       (0x180/4)
#define NREG_DMASRCA0       (0x188/4)
#define NREG_DMADESA0       (0x190/4)
#define NREG_DMACTRL1       (0x198/4)
#define NREG_DMASRCA1       (0x1A0/4)
#define NREG_DMADESA1       (0x1A8/4)
#define NREG_T0CTRL         (0x1C0/4)
#define NREG_T0CNTR         (0x1C8/4)
#define NREG_T1CTRL         (0x1D0/4)
#define NREG_T1CNTR         (0x1D8/4)
#define NREG_T2CTRL         (0x1E0/4)
#define NREG_T2CNTR         (0x1E8/4)
#define NREG_T3CTRL         (0x1F0/4)
#define NREG_T3CNTR         (0x1F8/4)

/* NILE 4 registers 0x300-0x3ff */
#define NREG_UARTRBR        (0x300/4)
#define NREG_UARTTHR        (0x300/4)
#define NREG_UARTIER        (0x308/4)
#define NREG_UARTDLL        (0x300/4)
#define NREG_UARTDLM        (0x308/4)
#define NREG_UARTIIR        (0x310/4)
#define NREG_UARTFCR        (0x310/4)
#define NREG_UARTLCR        (0x318/4)
#define NREG_UARTMCR        (0x320/4)
#define NREG_UARTLSR        (0x328/4)
#define NREG_UARTMSR        (0x330/4)
#define NREG_UARTSCR        (0x338/4)

/* NILE 4 interrupts */
#define NINT_CPCE           (0)
#define NINT_CNTD           (1)
#define NINT_MCE            (2)
#define NINT_DMA            (3)
#define NINT_UART           (4)
#define NINT_WDOG           (5)
#define NINT_GPT            (6)
#define NINT_LBRTD          (7)
#define NINT_INTA           (8)
#define NINT_INTB           (9)
#define NINT_INTC           (10)
#define NINT_INTD           (11)
#define NINT_INTE           (12)
#define NINT_RESV           (13)
#define NINT_PCIS           (14)
#define NINT_PCIE           (15)

#define SYSTEM_CLOCK        100000000
#define TIMER_PERIOD        attotime::from_hz(SYSTEM_CLOCK)

#define PCI_BUS_CLOCK        33000000
// Number of dma words to transfer at a time, real hardware bursts 8
#define DMA_BURST_SIZE       128
#define DMA_TIMER_PERIOD     attotime::from_hz(PCI_BUS_CLOCK / 32)

#define DMA_BUSY                0x80000000
#define DMA_INTEN               0x40000000
#define DMA_INTVLD              0x20000000
#define DMA_GO                  0x10000000
#define DMA_SUS                 0x08000000
#define DMA_DSTINC              0x04000000
#define DMA_SRCINC              0x02000000
#define DMA_RST                 0x01000000
#define DMA_BLK_SIZE            0x000fffff


class vrc5074_device : public pci_host_device {
public:
	vrc5074_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	void postload(void);

	void set_cpu_tag(const char *tag);
	void set_sdram_size(const int index, const int size) { m_sdram_size[index] = size; };

	void set_map(int id, const address_map_delegate &map, device_t *device);

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;
	DECLARE_READ32_MEMBER(sdram_addr_r);
	DECLARE_WRITE32_MEMBER(sdram_addr_w);

	// PCI interrupts
	DECLARE_WRITE_LINE_MEMBER(pci_intr_a);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_b);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_c);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_d);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_e);
	void update_pci_irq(const int index, const int state);

	//cpu bus registers
	DECLARE_READ32_MEMBER (cpu_reg_r);
	DECLARE_WRITE32_MEMBER(cpu_reg_w);
	DECLARE_READ32_MEMBER(serial_r);
	DECLARE_WRITE32_MEMBER(serial_w);
	void update_nile_irqs();

	DECLARE_READ32_MEMBER (pci0_r);
	DECLARE_WRITE32_MEMBER(pci0_w);

	DECLARE_READ32_MEMBER (pci1_r);
	DECLARE_WRITE32_MEMBER(pci1_w);

	virtual DECLARE_ADDRESS_MAP(target1_map, 32);
	DECLARE_READ32_MEMBER (target1_r);
	DECLARE_WRITE32_MEMBER(target1_w);

protected:
	address_space *m_cpu_space;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	mips3_device *m_cpu;
	const char *cpu_tag;
	int m_sdram_size[2];

	address_space_config m_mem_config, m_io_config;

	DECLARE_ADDRESS_MAP(cpu_map, 32);
	DECLARE_ADDRESS_MAP(serial_map, 32);

	void map_cpu_space();

	emu_timer* m_dma_timer;
	TIMER_CALLBACK_MEMBER(dma_transfer);
	emu_timer *m_timer[4];
	TIMER_CALLBACK_MEMBER(nile_timer_callback);

	required_memory_region m_romRegion;
	optional_memory_region m_updateRegion;
	std::vector<uint32_t> m_sdram[2];

	// Chip Select
	device_t *m_cs_devices[7];
	address_map_delegate m_cs_maps[7];

	uint32_t m_cpu_regs[0x1ff / 4];
	uint32_t m_serial_regs[0x40 / 4];
	uint16_t m_nile_irq_state;

	void setup_pci_space(void);
	uint32_t m_pci_laddr[2], m_pci_mask[2], m_pci_type[2];
	uint32_t m_sdram_addr[2];

};


extern const device_type VRC5074;

#endif
