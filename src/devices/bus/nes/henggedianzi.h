// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_HENGGEDIANZI_H
#define __NES_HENGGEDIANZI_H

#include "nxrom.h"


// ======================> nes_hengg_srich_device

class nes_hengg_srich_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_srich_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_hengg_xhzs_device

class nes_hengg_xhzs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_xhzs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_hengg_shjy3_device

class nes_hengg_shjy3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_hengg_shjy3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	void update_banks();

	UINT16 m_irq_count, m_irq_count_latch;
	int m_irq_enable;

	int m_chr_mode;
	UINT8 m_mmc_prg_bank[2];
	UINT8 m_mmc_vrom_bank[8];
	UINT8 m_mmc_extra_bank[8];
};





// device type definition
extern const device_type NES_HENGG_SRICH;
extern const device_type NES_HENGG_XHZS;
extern const device_type NES_HENGG_SHJY3;

#endif
