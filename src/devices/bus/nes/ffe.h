// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_FFE_H
#define MAME_BUS_NES_FFE_H

#pragma once

#include "nxrom.h"


// ======================> nes_ffe4_device
//
// Front Fareast (FFE) Super Magic Card (SMC-801) and its predecessors (Magic Card 1M/2M).
// Covers iNES mapper 6 directly; nes_ffe3_device (mapper 8) and nes_ffe8_device (mapper 17)
// derive from this, since all three iNES mapper numbers denote different initial register
// states of this one physical board (iNES Mapper 8 is documented as a synonym of Mapper 6
// submapper 4; Mapper 17 is the same hardware with the Super Magic Card's own default init).
//
// Register/banking source: NESdev Wiki "Super Magic Card" article (plus the companion
// "INES Mapper 006"/"INES Mapper 017" articles), cross-checked against the older FanWen Yang
// mapper6.txt where the two agree. See the file header in ffe.cpp for details and for the
// points still flagged VERIFY (here and in the .cpp). The PA12 IRQ clocking path
// (wants_a12_level()/ppu_a12_level() below) relies on ppu2c0x_device::ppu_bus() calling the
// A12 level callback once per real level transition, so counting only state 1 gives one
// increment per rising edge.

class nes_ffe4_device : public nes_nrom_device
{
public:
	// CHR data supplied by the image behaves as ROM (as in MAME's original FFE board and Mesen): writes are dropped, e.g.
	// Kaiketsu Yanchamaru 2 [hFFE] writes $00 into tile 0 at power-on, which would erase its sprite-0 graphic.
	// Images without CHR data keep fully writable CHR-RAM
	virtual void chr_w(offs_t offset, uint8_t data) override { if (!get_vrom_size()) device_nes_cart_interface::chr_w(offset, data); }
	// construction/destruction
	nes_ffe4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void ppu_latch(offs_t offset) override;

	virtual bool wants_a12_level() override { return true; }
	virtual void ppu_a12_level(int state) override; // see the note above on ppu2c0x_device::ppu_bus()

	virtual void pcb_reset() override;

protected:
	// power-on start at the trainer (mapper 17, NESdev): the reset vector in the PRG copy is pointed at $7000 until the
	// CPU fetches from there, as the copier's BIOS does on a hard reset
	void update_nt();
	void begin_trainer_boot(uint16_t entry);
	void begin_trainer_call_boot();
	void end_trainer_boot();
	bool m_power_on = true;
	bool m_trainer_boot = false;
	offs_t m_trainer_vec_ofs = 0;
	uint16_t m_trainer_entry = 0x7000;
	uint8_t m_trainer_vec_save[2] = { 0, 0 };
	uint8_t m_trainer_stub_save[6] = { 0, 0, 0, 0, 0, 0 };
	bool m_trainer_stub = false;
	nes_ffe4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted) override ATTR_COLD;

	// common init shared by mapper 6/8/17 initial states; see pcb_reset() overrides below
	// banking_2m4m_init: -1 = 2M/4M banking disabled at reset (latch-based mode active),
	//                      0 = 4M banking mode enabled at reset, 1 = 2M banking mode enabled at reset
	static constexpr int BANK_INIT_DISABLED = -1;
	static constexpr int BANK_INIT_4M       = 0;
	static constexpr int BANK_INIT_2M       = 1;
	void ffe_common_initialize(uint8_t config_4500, uint8_t reg_42ff, int banking_2m4m_init);

	void write_42fx(offs_t offset, uint8_t data);
	void write_43fx(offs_t offset, uint8_t data);
	void write_4500(uint8_t data);
	void write_8000(offs_t offset, uint8_t data);
	void write_1k_chr(offs_t offset, uint8_t data);
	void scratch_mirror(uint16_t address, uint8_t data);

	void update_prg();
	void update_chr();

	// M2-clocked IRQ counter (bit4 = 0): counted at the M2 rise, before that cycle's CPU access
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override
	{
		if (state)
		{
			irq_m2_tick();
			if (m_trainer_boot && !write && address == m_trainer_entry)
				end_trainer_boot();   // the CPU has reached the trainer: put the real reset vector back
		}
	}
	void irq_m2_tick();

	// ---- 1M banking mode register ($42FC-$42FF) ----
	uint8_t m_latch_mode;      // BBB: selects one of the 8 latch-based compatibility modes (0-7)
	bool    m_prg_wprotect;    // raw address bit "b"(A1) of the last $42Fx write: false: PRG writeable/latch disabled, true: PRG write-protected/latch enabled
	bool    m_mirror_group;    // raw address bit "M"(A0) of the last $42Fx write
	bool    m_mirror_sub;      // raw data bit D4 of the last $42Fx write
	// NOTE: A0 selects the mirroring group and A1 the PRG write-protect (NESdev), which is also the order of
	// state register 1 (bit0 = A0, bit1 = A1); read_l() reports m_mirror_group/m_prg_wprotect in that order.

	// ---- 2M/4M PRG banking mode register ($43FC-$43FF) ----
	bool    m_banking_2m4m_enable;
	bool    m_banking_is_2m;
	uint8_t m_shared_chr_bank; // CC: shared 8 KiB CHR bank, mirrored with the $8000-$FFFF latch's low 2 bits

	// ---- $8000-$FFFF latch: one shared latch for latch-based modes, 4 independent
	// per-quadrant latches for 2M PRG banking mode (same physical write path, different
	// semantics - see write_8000() in ffe.cpp) ----
	uint8_t m_quad_latch[4];

	// ---- 4M PRG banking mode registers ($4504-$4507) ----
	uint8_t m_prg_4m[4];

	// ---- Super Magic Card mode register ($4500) ----
	bool    m_chr_1k_mode;     // C: 0 = 8 KiB CHR granularity, 1 = 1 KiB CHR granularity
	bool    m_nt_from_chr;     // N: 0 = nametables from CHR memory via $4518-$451B, 1 = nametables from CIRAM
	bool    m_mmc4_enabled;    // m: 0 = enabled (only takes effect when m_chr_1k_mode is set), 1 = disabled
	bool    m_irq_source_pa12; // bit4: 0 = IRQ counter clocked by M2, 1 = clocked by PA12 (unfiltered)
	uint8_t m_wram_bank;       // 2-bit WRAM bank select for CPU $6000-$7FFF
	bool    m_play_mode;       // tracked for state-register read-back only; GUI mode is not emulated (see .cpp)
	bool    m_pass_through;    // tracked for state-register read-back only; not acted upon (see .cpp)

	// ---- 1 KiB CHR banking mode registers ($4510-$451B) ----
	uint8_t m_chr_1k[12];      // [0-7] = $4510-$4517, [8-11] = $4518-$451B (nametables)
	uint8_t m_mmc4_latch[2];   // MMC4-style $FD/$FE pattern-table latches, one per 4 KiB half

	// ---- IRQ counter ($4501 disable, $4502/$4503 LSB/MSB) ----
	uint16_t m_irq_count;
	bool     m_irq_enable;

	// ---- WRAM / scratch RAM ----
	std::vector<uint8_t> m_scratch;  // 4 KiB scratch RAM, CPU $5000-$5FFF
	static constexpr unsigned SCRATCH_SIZE = 0x1000;
	static constexpr unsigned CHRRAM_SIZE  = 0x40000; // 256 KiB CHR-RAM, 1 KiB granularity
	static constexpr unsigned WRAM_SIZE    = 0x8000;  // 32 KiB WRAM, 8 KiB granularity
};


// ======================> nes_ffe3_device  (iNES mapper 8 == mapper 6 submapper 4)

class nes_ffe3_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;
};


// ======================> nes_ffe8_device  (iNES mapper 17, "Super Magic Card" proper init)

class nes_ffe8_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FFE3, nes_ffe3_device)
DECLARE_DEVICE_TYPE(NES_FFE4, nes_ffe4_device)
DECLARE_DEVICE_TYPE(NES_FFE8, nes_ffe8_device)

#endif // MAME_BUS_NES_FFE_H
