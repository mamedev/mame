// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_TAITO_H
#define __NES_TAITO_H

#include "nxrom.h"


// ======================> nes_tc0190fmc_device

class nes_tc0190fmc_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tc0190fmc_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_tc0190fmc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(tc0190fmc_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { tc0190fmc_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;
};


// ======================> nes_tc0190fmc_pal16r4_device

class nes_tc0190fmc_pal16r4_device : public nes_tc0190fmc_device
{
public:
	// construction/destruction
	nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	UINT16     m_irq_count, m_irq_count_latch;
	int        m_irq_enable;
};


// ======================> nes_x1_005_device

class nes_x1_005_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_005_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_latch;
	// Taito X1-005 chip contains 80 bytes of internal ram, possibly battery backed up
	UINT8 m_x1_005_ram[0x80];
};


// ======================> nes_x1_017_device

class nes_x1_017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_x1_017_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

private:
	void set_chr();
	UINT8 m_latch;
	UINT8 m_reg[3]; //mapper ram protect
	UINT8 m_mmc_vrom_bank[6];
	// Taito X1-017 chip contains 5K of internal ram, battery backed up
	UINT8 m_x1_017_ram[0x1400];
};




// device type definition
extern const device_type NES_TC0190FMC;
extern const device_type NES_TC0190FMC_PAL16R4;
extern const device_type NES_X1_005;
extern const device_type NES_X1_017;


#endif
