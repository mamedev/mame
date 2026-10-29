// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_JY_H
#define MAME_BUS_NES_JY_H

#pragma once

#include "nxrom.h"


// ======================> nes_jy_typea_device (iNES mapper 90)
//
// J.Y. Company ASIC. Mapper 90 is the board with the solder jumper that inhibits ROM nametables and extended mirroring,
// mappers 209 and 211 the ones without it (NESdev: 211 is a duplicate of 209 with correct emulation)

class nes_jy_typea_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;

	// IRQ sources: CPU M2 rise and CPU writes (M2), unfiltered PPU A12 rise, every PPU read
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;
	virtual bool wants_a12_level() override { return true; }
	virtual void ppu_a12_level(int state) override;
	// IRQ source 2 counts every PPU read strobe (/RD)
	virtual bool wants_ppu_strobes() override { return true; }
	virtual void ppu_rd(offs_t addr) override;

	// MMC4-like CHR latches
	virtual bool wants_read_latch() override { return true; }
	virtual void ppu_read_latch(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool nt_jumper);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// A board built around the ASIC with outer banking of its own replaces the final bank numbers the ASIC forms:
	// PRG in 8K units, CHR and ROM nametables in 1K units. The default leaves them alone.
	virtual uint32_t prg_bank_filter(uint32_t bank) const { return bank; }
	virtual uint32_t chr_bank_filter(uint32_t bank) const { return bank; }
	void refresh_banks() { update_prg(); update_chr(); update_nt(); }

	// The outer bank register ($D003) differs by PCB (NESdev: "PCB/Mapper-specific"). The defaults are the standard
	// board's (mappers 35, 90, 209, 211): the $8000-$8003 banks are masked to 512K, PRG A19-A20 from bits 2-1, CHR
	// masked to 256K (A18 from bit 0) or 512K (bit 5 set), CHR A19-A20 from bits 4-3. PRG in 8K units, CHR in 1K units.
	virtual uint32_t asic_prg_mask() const { return 0x3f; }
	virtual uint32_t asic_prg_outer() const { return uint32_t(BIT(m_outer, 1, 2)) << 6; }
	virtual uint32_t asic_chr_mask() const { return BIT(m_outer, 5) ? 0x1ff : 0xff; }
	virtual uint32_t asic_chr_outer() const { return (uint32_t(BIT(m_outer, 3, 2)) << 9) | (BIT(m_outer, 5) ? 0 : (uint32_t(BIT(m_outer, 0)) << 8)); }
	// a ROM nametable's bank number; the standard board applies no outer bits to it
	virtual uint32_t asic_nt_bank(uint32_t bank) const { return bank; }
	uint8_t asic_outer_reg() const { return m_outer; }

private:
	void update_prg();
	void update_chr();
	void update_nt();
	void irq_tick();
	void irq_disable();
	uint32_t chr_window(int reg, int shift) const;

	required_ioport m_jumper;
	const bool m_nt_jumper;     // mapper 90: ROM nametables and extended mirroring are inhibited

	// $5800-$5803
	uint8_t m_mul[2];
	uint8_t m_acc;
	uint8_t m_test;

	// $8000-$B007
	uint8_t m_prg_reg[4];
	uint8_t m_chr_lo[8];
	uint8_t m_chr_hi[8];
	uint8_t m_nt_lo[4];
	uint8_t m_nt_hi[4];

	// $D000-$D003
	uint8_t m_mode;
	uint8_t m_mirror;
	uint8_t m_ppucfg;
	uint8_t m_outer;

	uint8_t m_chr_latch[2];     // MMC4-like mode: selects the second register of each 4K half
	uint32_t m_bank_6000;

	// $C000-$C007
	uint8_t m_irq_enable;
	uint8_t m_irq_source;
	uint8_t m_irq_dir;
	uint8_t m_irq_small;
	uint8_t m_irq_funky;
	uint8_t m_irq_prescaler;
	uint8_t m_irq_counter;
	uint8_t m_irq_xor;
	uint8_t m_irq_c007;
	uint8_t m_a12;
};


// ======================> nes_jy_typeb_device (iNES mapper 211)

class nes_jy_typeb_device : public nes_jy_typea_device
{
public:
	// construction/destruction
	nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> nes_jy_typec_device (iNES mappers 209 and 35)

class nes_jy_typec_device : public nes_jy_typeb_device
{
public:
	// construction/destruction
	nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_jy_mc_device
//
// J.Y. Company multicart PCBs (NES 2.0 mappers 281, 282, 295, 358, 386, 387, 388, 397): the ASIC with the outer
// bank register ($D003) wired per PCB

class nes_jy_mc_device : public nes_jy_typea_device
{
protected:
	nes_jy_mc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int pcb, bool nt_jumper);

	virtual uint32_t asic_prg_mask() const override;
	virtual uint32_t asic_prg_outer() const override;
	virtual uint32_t asic_chr_mask() const override;
	virtual uint32_t asic_chr_outer() const override;
	virtual uint32_t asic_nt_bank(uint32_t bank) const override;

private:
	const int m_pcb;            // NES 2.0 mapper number
};

#define NES_JY_MC_CLASS(num) \
class nes_jy_##num##_device : public nes_jy_mc_device \
{ \
public: \
	nes_jy_##num##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); \
};

NES_JY_MC_CLASS(281)
NES_JY_MC_CLASS(282)
NES_JY_MC_CLASS(295)
NES_JY_MC_CLASS(358)
NES_JY_MC_CLASS(386)
NES_JY_MC_CLASS(387)
NES_JY_MC_CLASS(388)
NES_JY_MC_CLASS(397)

#undef NES_JY_MC_CLASS


// ======================> nes_jy_hsk007_device
//
// Realtec HSK007 multicart board (NES 2.0 mapper 394): a mode register selects BNROM, an MMC3 clone or the J.Y. Company
// ASIC, and an outer bank register and the mode register add the outer PRG and CHR bits in every mode

class nes_jy_hsk007_device : public nes_jy_typeb_device
{
public:
	// construction/destruction
	nes_jy_hsk007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void set_hsk007_submapper(u8 val) override { m_submapper = val; }

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;

	virtual void m2_w(int state, offs_t address, bool write) override;
	virtual void ppu_a12_level(int state) override;
	virtual void ppu_read_latch(offs_t offset) override;

	// the MMC3 mode counts filtered A12 rises
	virtual bool wants_a12() override { return true; }
	virtual bool wants_a12_filter() override { return true; }
	virtual void ppu_a12_rise() override;
	virtual bool a12_rise_accepted(const attotime &low_since, const attotime &when) override;

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual uint32_t prg_bank_filter(uint32_t bank) const override;
	virtual uint32_t chr_bank_filter(uint32_t bank) const override;

private:
	bool jy_mode() const { return BIT(m_reg[1], 4); }
	uint32_t prg_outer_ab() const { return (m_reg[1] << 5) & 0x60; }
	uint32_t prg_outer() const { return ((m_reg[3] << 1) & 0x10) | prg_outer_ab(); }
	uint32_t chr_outer_ab() const;
	uint32_t chr_outer() const { return ((m_reg[3] << 1) & 0x80) | chr_outer_ab(); }
	uint32_t mmc3_prg(uint32_t bank) const;
	uint32_t mmc3_chr(uint32_t bank) const;

	void mmc3_power();
	void update_banks();
	void update_mmc3();

	uint8_t m_submapper;
	uint8_t m_reg[4];       // $5000-$5FFF with A3 set, decoded on A1-A0: 1 = mode, 3 = outer bank

	// the MMC3 mode
	uint8_t m_mmc3_cmd;
	uint8_t m_mmc3_bank[8];
	uint8_t m_mmc3_irq_latch;
	uint8_t m_mmc3_irq_count;
	bool m_mmc3_irq_reload;
	bool m_mmc3_irq_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JY_TYPEA, nes_jy_typea_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEB, nes_jy_typeb_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEC, nes_jy_typec_device)
DECLARE_DEVICE_TYPE(NES_JY_281, nes_jy_281_device)
DECLARE_DEVICE_TYPE(NES_JY_282, nes_jy_282_device)
DECLARE_DEVICE_TYPE(NES_JY_295, nes_jy_295_device)
DECLARE_DEVICE_TYPE(NES_JY_358, nes_jy_358_device)
DECLARE_DEVICE_TYPE(NES_JY_386, nes_jy_386_device)
DECLARE_DEVICE_TYPE(NES_JY_387, nes_jy_387_device)
DECLARE_DEVICE_TYPE(NES_JY_388, nes_jy_388_device)
DECLARE_DEVICE_TYPE(NES_JY_397, nes_jy_397_device)
DECLARE_DEVICE_TYPE(NES_JY_HSK007, nes_jy_hsk007_device)

#endif // MAME_BUS_NES_JY_H
