// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82371sb southbridge (PIIX3)

#ifndef I82371SB_H
#define I82371SB_H

#include "pci.h"

#define MCFG_I82371SB_ISA_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, I82371SB_ISA, 0x80867000, 0x03, 0x060100, 0x00000000)

#define MCFG_I82371SB_BOOT_STATE_HOOK(_devcb) \
	devcb = &i82371sb_isa_device::set_boot_state_hook(*device, DEVCB_##_devcb);

class i82371sb_isa_device : public pci_device {
public:
	i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_boot_state_hook(device_t &device, _Object object) { return downcast<i82371sb_isa_device &>(device).m_boot_state_hook.set_callback(object); }

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;
	DECLARE_ADDRESS_MAP(internal_io_map, 32);

	void boot_state_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t iort_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iort_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t xbcs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void xbcs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t pirqrc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pirqrc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t mstat_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mstat_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t mbirq0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mbirq0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mbdma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mbdma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t pcsc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pcsc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t apicbase_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void apicbase_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dlc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dlc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smicntl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smicntl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t smien_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void smien_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t see_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void see_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t ftmr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ftmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t smireq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void smireq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t ctltmr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctltmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cthtmr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cthtmr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write8 m_boot_state_hook;

	uint32_t see;
	uint16_t xbcs, mstat, pcsc, smien, smireq;
	uint8_t iort, pirqrc[4], tom, mbirq0, mbdma[2], apicbase;
	uint8_t dlc, smicntl, ftmr, ctlmtr, cthmtr;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);
};

extern const device_type I82371SB_ISA;

#endif
