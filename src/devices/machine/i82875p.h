// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82875p northbridge

#ifndef I82875P_H
#define I82875P_H

#include "pci.h"

#define MCFG_I82875P_HOST_ADD(_tag, _subdevice_id, _cpu_tag, _ram_size)    \
	MCFG_PCI_HOST_ADD(_tag, I82875P_HOST, 0x80862578, 0x02, _subdevice_id) \
	downcast<i82875p_host_device *>(device)->set_cpu_tag(_cpu_tag);        \
	downcast<i82875p_host_device *>(device)->set_ram_size(_ram_size);

#define MCFG_I82875P_AGP_ADD(_tag) \
	MCFG_AGP_BRIDGE_ADD(_tag, I82875P_AGP, 0x80862579, 0x02)

#define MCFG_I82875P_OVERFLOW_ADD(_tag, _subdevice_id)    \
	MCFG_PCI_DEVICE_ADD(_tag, I82875P_OVERFLOW, 0x8086257e, 0x02, 0x088000, _subdevice_id)

class i82875p_host_device : public pci_host_device {
public:
	i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu_tag(const char *tag);
	void set_ram_size(int ram_size);

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;

	virtual uint8_t capptr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

	uint8_t agpm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void agpm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t csabcont_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t eap_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint8_t derrsyn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t des_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t fpllcont_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fpllcont_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pam_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pam_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t smram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void smram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t esmramc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void esmramc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t acapid_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t agpstat_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t agpcmd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t agpctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void agpctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t apsize_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void apsize_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t attbase_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void attbase_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t amtt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void amtt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lptt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lptt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t toud_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toud_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mchcfg_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mchcfg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t errsts_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t errcmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void errcmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t smicmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void smicmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t scicmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void scicmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t skpd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void skpd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t capreg1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint8_t capreg2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(agp_translation_map, 32);

	const char *cpu_tag;
	int ram_size;
	cpu_device *cpu;
	std::vector<uint32_t> ram;

	uint8_t agpm, fpllcont, pam[8], smram, esmramc;
	uint8_t apsize, amtt, lptt;
	uint16_t toud, mchcfg, errcmd, smicmd, scicmd, skpd;
	uint32_t agpctrl, attbase;
};

class i82875p_agp_device : public agp_bridge_device {
public:
	i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class i82875p_overflow_device : public pci_device {
public:
	i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	uint8_t dram_row_boundary_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dram_row_boundary_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dram_row_attribute_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dram_row_attribute_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t dram_timing_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dram_timing_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dram_controller_mode_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dram_controller_mode_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

protected:

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(overflow_map, 32);

	uint8_t dram_row_boundary[8], dram_row_attribute[4];
	uint32_t dram_timing, dram_controller_mode;
};

extern const device_type I82875P_HOST;
extern const device_type I82875P_AGP;
extern const device_type I82875P_OVERFLOW;


#endif
