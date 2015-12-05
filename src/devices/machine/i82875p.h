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
	i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_cpu_tag(const char *tag);
	void set_ram_size(int ram_size);

	virtual void reset_all_mappings() override;

	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

	virtual DECLARE_ADDRESS_MAP(config_map, 32);

	virtual DECLARE_READ8_MEMBER(capptr_r);

	DECLARE_READ8_MEMBER(  agpm_r);
	DECLARE_WRITE8_MEMBER( agpm_w);
	DECLARE_READ8_MEMBER(  gc_r);
	DECLARE_READ8_MEMBER(  csabcont_r);
	DECLARE_READ32_MEMBER( eap_r);
	DECLARE_READ8_MEMBER(  derrsyn_r);
	DECLARE_READ8_MEMBER(  des_r);
	DECLARE_READ8_MEMBER(  fpllcont_r);
	DECLARE_WRITE8_MEMBER( fpllcont_w);
	DECLARE_READ8_MEMBER(  pam_r);
	DECLARE_WRITE8_MEMBER( pam_w);
	DECLARE_READ8_MEMBER(  smram_r);
	DECLARE_WRITE8_MEMBER( smram_w);
	DECLARE_READ8_MEMBER(  esmramc_r);
	DECLARE_WRITE8_MEMBER( esmramc_w);
	DECLARE_READ32_MEMBER( acapid_r);
	DECLARE_READ32_MEMBER( agpstat_r);
	DECLARE_READ32_MEMBER( agpcmd_r);
	DECLARE_READ32_MEMBER( agpctrl_r);
	DECLARE_WRITE32_MEMBER(agpctrl_w);
	DECLARE_READ8_MEMBER(  apsize_r);
	DECLARE_WRITE8_MEMBER( apsize_w);
	DECLARE_READ32_MEMBER( attbase_r);
	DECLARE_WRITE32_MEMBER(attbase_w);
	DECLARE_READ8_MEMBER(  amtt_r);
	DECLARE_WRITE8_MEMBER( amtt_w);
	DECLARE_READ8_MEMBER(  lptt_r);
	DECLARE_WRITE8_MEMBER( lptt_w);
	DECLARE_READ16_MEMBER( toud_r);
	DECLARE_WRITE16_MEMBER(toud_w);
	DECLARE_READ16_MEMBER( mchcfg_r);
	DECLARE_WRITE16_MEMBER(mchcfg_w);
	DECLARE_READ16_MEMBER( errsts_r);
	DECLARE_READ16_MEMBER( errcmd_r);
	DECLARE_WRITE16_MEMBER(errcmd_w);
	DECLARE_READ16_MEMBER( smicmd_r);
	DECLARE_WRITE16_MEMBER(smicmd_w);
	DECLARE_READ16_MEMBER( scicmd_r);
	DECLARE_WRITE16_MEMBER(scicmd_w);
	DECLARE_READ16_MEMBER( skpd_r);
	DECLARE_WRITE16_MEMBER(skpd_w);
	DECLARE_READ32_MEMBER( capreg1_r);
	DECLARE_READ8_MEMBER(  capreg2_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(agp_translation_map, 32);

	const char *cpu_tag;
	int ram_size;
	cpu_device *cpu;
	std::vector<UINT32> ram;

	UINT8 agpm, fpllcont, pam[8], smram, esmramc;
	UINT8 apsize, amtt, lptt;
	UINT16 toud, mchcfg, errcmd, smicmd, scicmd, skpd;
	UINT32 agpctrl, attbase;
};

class i82875p_agp_device : public agp_bridge_device {
public:
	i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class i82875p_overflow_device : public pci_device {
public:
	i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	DECLARE_READ8_MEMBER  (dram_row_boundary_r);
	DECLARE_WRITE8_MEMBER (dram_row_boundary_w);
	DECLARE_READ8_MEMBER  (dram_row_attribute_r);
	DECLARE_WRITE8_MEMBER (dram_row_attribute_w);
	DECLARE_READ32_MEMBER (dram_timing_r);
	DECLARE_WRITE32_MEMBER(dram_timing_w);
	DECLARE_READ32_MEMBER (dram_controller_mode_r);
	DECLARE_WRITE32_MEMBER(dram_controller_mode_w);

protected:

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(overflow_map, 32);

	UINT8 dram_row_boundary[8], dram_row_attribute[4];
	UINT32 dram_timing, dram_controller_mode;
};

extern const device_type I82875P_HOST;
extern const device_type I82875P_AGP;
extern const device_type I82875P_OVERFLOW;


#endif
