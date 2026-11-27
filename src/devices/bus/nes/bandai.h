// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_BANDAI_H
#define MAME_BUS_NES_BANDAI_H

#pragma once

#include "nxrom.h"
#include "machine/i2cmem.h"


// ======================> nes_oekakids_device

class nes_oekakids_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_oekakids_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual bool wants_ppu_addr_bus() override { return true; }
	virtual void ppu_addr_bus(u16 addr) override;

	virtual void pcb_reset() override;

	// TODO: add oeka kids controller emulation

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void update_chr();

	uint8_t m_reg, m_latch;
};


// ======================> nes_fcg_device

class nes_fcg_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fcg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// latched: LZ93D50 ($8000-$FFFF) shadows the counter value in a latch; the FCG-1/2 ($6000-$7FFF) writes the counter itself
	void fcg_write(offs_t offset, uint8_t data, bool latched = true);
	virtual void set_bandai_submapper(u8 val) override { m_regs_at_6000 = (val != 5); }
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	bool m_regs_at_6000 = true;    // FCG-1/2 registers at $6000-$7FFF (not on LZ93D50-only boards)
	nes_fcg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter is clocked by M2: it is counted at the M2 rise, before that cycle's CPU access
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_clock(); }
	void irq_clock();

	uint16_t   m_irq_count;
	int        m_irq_enable;
	uint16_t   m_irq_latch;
};


// ======================> nes_lz93d50_device

class nes_lz93d50_device : public nes_fcg_device
{
public:
	// construction/destruction
	nes_lz93d50_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override { fcg_write(offset, data); }

protected:
	nes_lz93d50_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_lz93d50_24c01_device

class nes_lz93d50_24c01_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_lz93d50_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<i2cmem_device> m_i2cmem;
	uint8_t m_i2c_dir;
};


// ======================> nes_lz93d50_24c02_device

class nes_lz93d50_24c02_device : public nes_lz93d50_24c01_device
{
public:
	// construction/destruction
	nes_lz93d50_24c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_fjump2_device

class nes_fjump2_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_fjump2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void set_prg();
	uint8_t m_reg[5];
	uint8_t m_wram_enable;   // $800D bit 5: PRG-RAM chip enable (NESdev INES Mapper 153)
};


// device type definition
DECLARE_DEVICE_TYPE(NES_OEKAKIDS,      nes_oekakids_device)
DECLARE_DEVICE_TYPE(NES_FCG,           nes_fcg_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50,       nes_lz93d50_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50_24C01, nes_lz93d50_24c01_device)
DECLARE_DEVICE_TYPE(NES_LZ93D50_24C02, nes_lz93d50_24c02_device)
DECLARE_DEVICE_TYPE(NES_FJUMP2,        nes_fjump2_device)

#endif // MAME_BUS_NES_BANDAI_H
