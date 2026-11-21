// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82439hx northbridge (440hx)

#ifndef MAME_MACHINE_I82439TX_H
#define MAME_MACHINE_I82439TX_H

#pragma once

#include "pci.h"
#include "ram.h"

class i82439tx_host_device : public pci_host_device {
public:
	template <typename T>
	i82439tx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, int ram_size)
		: i82439tx_host_device(mconfig, tag, owner)
	{
		set_ids_host(0x80867100, 0x01, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}
	i82439tx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size);
	// take size and memory from a RAM device instead of set_ram_size
	template <typename T> void set_ram_tag(T &&tag) { m_ram_dev.set_tag(std::forward<T>(tag)); }

	void smi_act_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	int ram_size;
	required_device<device_memory_interface> cpu;
	optional_device<ram_device> m_ram_dev;
	std::vector<uint32_t> ram;
	uint32_t *m_ram;

	uint8_t acon, pcon, cc, cec, dramec, dramc, dramt;
	uint16_t sdramc;
	uint8_t pam[7], drb[6];
	uint8_t drt, drth, esmramc, smram, mctl;
	bool smiact_n;

	uint8_t acon_r();
	void acon_w(uint8_t data);
	uint8_t pcon_r();
	void pcon_w(uint8_t data);
	uint8_t cc_r();
	void cc_w(uint8_t data);
	uint8_t cec_r();
	void cec_w(uint8_t data);
	uint16_t sdramc_r();
	void sdramc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t dramec_r();
	void dramec_w(uint8_t data);
	uint8_t dramc_r();
	void dramc_w(uint8_t data);
	uint8_t dramt_r();
	void dramt_w(uint8_t data);
	uint8_t pam_r(offs_t offset);
	void pam_w(offs_t offset, uint8_t data);
	uint8_t drb_r(offs_t offset);
	void drb_w(offs_t offset, uint8_t data);
	uint8_t drth_r();
	void drth_w(uint8_t data);
	uint8_t drtl_r();
	void drtl_w(uint8_t data);
	uint8_t esmramc_r();
	void esmramc_w(uint8_t data);
	uint8_t smram_r();
	void smram_w(uint8_t data);
	uint8_t mctl_r();
	void mctl_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(I82439TX, i82439tx_host_device)

#endif // MAME_MACHINE_I82439TX_H
