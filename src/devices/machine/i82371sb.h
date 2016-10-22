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

	DECLARE_WRITE8_MEMBER (boot_state_w);
	DECLARE_WRITE8_MEMBER (nop_w);

	DECLARE_READ8_MEMBER  (iort_r);
	DECLARE_WRITE8_MEMBER (iort_w);
	DECLARE_READ16_MEMBER (xbcs_r);
	DECLARE_WRITE16_MEMBER(xbcs_w);
	DECLARE_READ8_MEMBER  (pirqrc_r);
	DECLARE_WRITE8_MEMBER (pirqrc_w);
	DECLARE_READ8_MEMBER  (tom_r);
	DECLARE_WRITE8_MEMBER (tom_w);
	DECLARE_READ16_MEMBER (mstat_r);
	DECLARE_WRITE16_MEMBER(mstat_w);
	DECLARE_READ8_MEMBER  (mbirq0_r);
	DECLARE_WRITE8_MEMBER (mbirq0_w);
	DECLARE_READ8_MEMBER  (mbdma_r);
	DECLARE_WRITE8_MEMBER (mbdma_w);
	DECLARE_READ16_MEMBER (pcsc_r);
	DECLARE_WRITE16_MEMBER(pcsc_w);
	DECLARE_READ8_MEMBER  (apicbase_r);
	DECLARE_WRITE8_MEMBER (apicbase_w);
	DECLARE_READ8_MEMBER  (dlc_r);
	DECLARE_WRITE8_MEMBER (dlc_w);
	DECLARE_READ8_MEMBER  (smicntl_r);
	DECLARE_WRITE8_MEMBER (smicntl_w);
	DECLARE_READ16_MEMBER (smien_r);
	DECLARE_WRITE16_MEMBER(smien_w);
	DECLARE_READ32_MEMBER (see_r);
	DECLARE_WRITE32_MEMBER(see_w);
	DECLARE_READ8_MEMBER  (ftmr_r);
	DECLARE_WRITE8_MEMBER (ftmr_w);
	DECLARE_READ16_MEMBER (smireq_r);
	DECLARE_WRITE16_MEMBER(smireq_w);
	DECLARE_READ8_MEMBER  (ctltmr_r);
	DECLARE_WRITE8_MEMBER (ctltmr_w);
	DECLARE_READ8_MEMBER  (cthtmr_r);
	DECLARE_WRITE8_MEMBER (cthtmr_w);

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
