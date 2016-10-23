// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82439hx northbridge (440hx)

#ifndef I82439HX_H
#define I82439HX_H

#include "pci.h"

#define MCFG_I82439HX_ADD(_tag, _cpu_tag, _ram_size)    \
	MCFG_PCI_HOST_ADD(_tag, I82439HX, 0x80861250, 0x03, 0x00000000) \
	downcast<i82439hx_host_device *>(device)->set_cpu_tag(_cpu_tag); \
	downcast<i82439hx_host_device *>(device)->set_ram_size(_ram_size);

class i82439hx_host_device : public pci_host_device {
public:
	i82439hx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu_tag(const char *tag);
	void set_ram_size(int ram_size);

	uint8_t pcon_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pcon_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dramec_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dramec_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dramc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dramc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dramt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dramt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pam0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pam0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pam3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pam3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t drb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void drb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t drt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void drt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t drat_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void drat_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t errcmd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void errcmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t errsts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void errsts_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t errsyn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	const char *cpu_tag;
	int ram_size;
	cpu_device *cpu;
	std::vector<uint32_t> ram;

	uint8_t pcon, cc, dramec, dramc, dramt;
	uint8_t pam[7], drb[8];
	uint8_t drt, drat, smram, errcmd, errsts, errsyn;
};

extern const device_type I82439HX;

#endif
