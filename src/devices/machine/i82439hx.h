// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82439hx northbridge (440hx)

#ifndef MAME_MACHINE_I82439HX_H
#define MAME_MACHINE_I82439HX_H

#pragma once

#include "pci.h"

class i82439hx_host_device : public pci_host_device {
public:
	template <typename T>
	i82439hx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int ram_size)
		: i82439hx_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x80861250, 0x03, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}
	i82439hx_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size);

	DECLARE_WRITE_LINE_MEMBER(smi_act_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	int ram_size;
	required_device<device_memory_interface> cpu;
	std::vector<uint32_t> ram;

	uint8_t pcon, cc, dramec, dramc, dramt;
	uint8_t pam[7], drb[8];
	uint8_t drt, drat, smram, errcmd, errsts, errsyn;
	int smiact_n;

	virtual DECLARE_READ8_MEMBER(header_type_r) override;
	DECLARE_READ8_MEMBER (pcon_r);
	DECLARE_WRITE8_MEMBER(pcon_w);
	DECLARE_READ8_MEMBER (cc_r);
	DECLARE_WRITE8_MEMBER(cc_w);
	DECLARE_READ8_MEMBER (dramec_r);
	DECLARE_WRITE8_MEMBER(dramec_w);
	DECLARE_READ8_MEMBER (dramc_r);
	DECLARE_WRITE8_MEMBER(dramc_w);
	DECLARE_READ8_MEMBER (dramt_r);
	DECLARE_WRITE8_MEMBER(dramt_w);
	DECLARE_READ8_MEMBER (pam_r);
	DECLARE_WRITE8_MEMBER(pam_w);
	DECLARE_READ8_MEMBER (drb_r);
	DECLARE_WRITE8_MEMBER(drb_w);
	DECLARE_READ8_MEMBER (drt_r);
	DECLARE_WRITE8_MEMBER(drt_w);
	DECLARE_READ8_MEMBER (drat_r);
	DECLARE_WRITE8_MEMBER(drat_w);
	DECLARE_READ8_MEMBER (smram_r);
	DECLARE_WRITE8_MEMBER(smram_w);
	DECLARE_READ8_MEMBER (errcmd_r);
	DECLARE_WRITE8_MEMBER(errcmd_w);
	DECLARE_READ8_MEMBER (errsts_r);
	DECLARE_WRITE8_MEMBER(errsts_w);
	DECLARE_READ8_MEMBER (errsyn_r);
};

DECLARE_DEVICE_TYPE(I82439HX, i82439hx_host_device)

#endif // MAME_MACHINE_I82439HX_H
