// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC3_H
#define MAME_BUS_NES_MMC3_H

#pragma once

#include "nxrom.h"

// ======================> nes_txrom_device
class m6502_device;
class nes_txrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void txrom_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { txrom_write(offset, data); }
	virtual void prg_cb(int start, int bank);
	virtual void chr_cb(int start, int bank, int source);

	virtual void ppu_to_mapper(int scanline, unsigned dot, int ppu_tick, uint16_t ppu_address) override; //called from ppu
	virtual void ppu_bus_address(uint16_t ppu_addr, uint64_t cpu_cycles, int ppu_tick, bool m_odd_frame) override; //called from ppu
	virtual void ppu_odd_frame_skip() override;	//called from ppu
	virtual void pcb_reset() override; 

protected:
	nes_txrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { mmc3_start(); }

	// Banking helpers. Variants override these to add board-specific behavior.
	virtual void set_prg(int prg_base, int prg_mask);
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask);
	
	// Common MMC3 startup/reset helpers.
	void mmc3_start();
	void mmc3_common_initialize(int prg_mask, int chr_mask, int nec_irq_behavior);
	void mmc3_irq_clock();
	
	// PRG bank registers.
	// MMC3 exposes two switchable 8 KiB PRG banks plus two fixed banks.
	// m_mmc_prg_bank[0] = $8000/$C000 switchable bank, depending on PRG mode.
	// m_mmc_prg_bank[1] = $A000 switchable bank.
	// m_mmc_prg_bank[2] = second-last fixed bank.
	// m_mmc_prg_bank[3] = last fixed bank.
	uint16_t m_mmc_prg_bank[4];
	
	// CHR bank registers.
	// Base MMC3 uses registers 0-5:
	//   R0/R1 are 2 KiB CHR banks, forced even/odd in set_chr().
	//   R2-R5 are 1 KiB CHR banks.
	// Entries 6-7 are kept for MMC3-like clone/variant boards that need extra CHR regs.
	uint16_t m_mmc_vrom_bank[8]; 

	// Multicart outer-bank selectors.
	// Normal MMC3 keeps these at base 0 with masks covering the full ROM.
	// Mapper 37/47-style boards change these to expose different PRG/CHR blocks.
	int m_prg_base;
	int m_prg_mask;
	int m_chr_base;
	int m_chr_mask;

	// Bank select register written at $8000-$9FFE even.
	// Bits 0-2 select the target bank register for $8001.
	// Bit 6 selects PRG bank mode.
	// Bit 7 selects CHR A12 inversion mode.
	int m_latch;
	
	// Last value written to MMC3 mirroring register $A000-$BFFE even.
	// Base MMC3 applies it immediately, but some clone boards need to read
	// the saved value later when switching between MMC3-controlled mirroring
	// and board-controlled mirroring.
	uint8_t m_mmc_mirror;
	
	// PRG RAM protect register written at $A001-$BFFF odd.
	// Bit 7 enables PRG RAM access.
	// Bit 6 write-protects PRG RAM when set.
	int m_wram_protect;
		
	// MMC3 IRQ counter state.
	// $C000 writes the reload value.
	// $C001 clears the current counter and requests reload on the next filtered A12 rising edge.
	// $E000 disables/acknowledges IRQ.
	// $E001 enables IRQ.
	uint16_t m_irq_count;
	uint16_t m_irq_count_latch;
	int m_irq_enable;
	bool m_irq_reload;

	// IRQ behavior revision.
	// true  = Sharp/new MMC3 behavior: IRQ can assert when counter is 0 after clock/reload.
	// false = NEC/old behavior: IRQ asserts on 1->0 decrement, plus explicit reload-to-0 cases.
	bool rev_b_behavior;
	
	// Delayed IRQ assertion after the MMC3 counter decides to fire.
	// This lets the mapper line up with the CPU-visible IRQ timing used by tests.
	int delay_irq;
	
	// Filtered PPU A12 tracking for MMC3 IRQ clocking.
	// The PPU calls observe_ppu_a12() with real PPU bus addresses.
	// m_prev_ppu_addr provides the previous A12 state.
	// m_last_a12_low_cpu records when A12 last transitioned high->low, in CPU cycles.
	uint64_t m_last_a12_low_cpu;
	uint16_t m_prev_ppu_addr;

	// Cached CPU pointer used for delayed IRQ queue/cancel and timing.
	m6502_device* m_maincpu6502 = nullptr;
	
	bool m_a12_low_seen = false;
	int m_ppu_tick;
	bool m_mmc3_odd_skip_a12_pending;
};


// ======================> nes_hkrom_device

class nes_hkrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_hkrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	int m_wram_enable;
	uint8_t m_mmc6_reg;

	// MMC-6 contains 1K of internal ram, battery backed up
	uint8_t m_mmc6_ram[0x400];
	
};


// ======================> nes_txsrom_device

class nes_txsrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_txsrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_txsrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_tqrom_device

class nes_tqrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_tqrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void chr_cb(int start, int bank, int source) override;

protected:
	// construction/destruction
	nes_tqrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_qj_device

class nes_qj_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_qj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_zz_device

class nes_zz_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_zz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TXROM,  nes_txrom_device)
DECLARE_DEVICE_TYPE(NES_HKROM,  nes_hkrom_device)
DECLARE_DEVICE_TYPE(NES_TXSROM, nes_txsrom_device)
DECLARE_DEVICE_TYPE(NES_TQROM,  nes_tqrom_device)
DECLARE_DEVICE_TYPE(NES_QJ_PCB, nes_qj_device)
DECLARE_DEVICE_TYPE(NES_ZZ_PCB, nes_zz_device)

#endif // MAME_BUS_NES_MMC3_H
