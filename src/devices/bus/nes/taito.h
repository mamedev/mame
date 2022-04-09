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

	void tc0190fmc_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { tc0190fmc_write(offset, data); }

	virtual void pcb_reset() override;

protected:
	nes_tc0190fmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_tc0190fmc_pal16r4_device

class nes_tc0190fmc_pal16r4_device : public nes_tc0190fmc_device
{
public:
	// construction/destruction
	nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

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

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

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

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

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
