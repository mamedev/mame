// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_TAITO_H
#define MAME_BUS_NES_TAITO_H

#pragma once

#include "nxrom.h"


// ======================> nes_tc0190fmc_device

class nes_tc0190fmc_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tc0190fmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(tc0190fmc_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { tc0190fmc_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
	nes_tc0190fmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_tc0190fmc_pal16r4_device

class nes_tc0190fmc_pal16r4_device : public nes_tc0190fmc_device
{
public:
	// construction/destruction
	nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint16_t     m_irq_count, m_irq_count_latch;
	int        m_irq_enable;
};


// ======================> nes_x1_005_device

class nes_x1_005_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
	// Taito X1-005 chip contains 80 bytes of internal ram, possibly battery backed up
	uint8_t m_x1_005_ram[0x80];
};


// ======================> nes_x1_017_device

class nes_x1_017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void set_chr();
	uint8_t m_latch;
	uint8_t m_reg[3]; //mapper ram protect
	uint8_t m_mmc_vrom_bank[6];
	// Taito X1-017 chip contains 5K of internal ram, battery backed up
	uint8_t m_x1_017_ram[0x1400];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TC0190FMC,         nes_tc0190fmc_device)
DECLARE_DEVICE_TYPE(NES_TC0190FMC_PAL16R4, nes_tc0190fmc_pal16r4_device)
DECLARE_DEVICE_TYPE(NES_X1_005,            nes_x1_005_device)
DECLARE_DEVICE_TYPE(NES_X1_017,            nes_x1_017_device)


#endif // MAME_BUS_NES_TAITO_H
