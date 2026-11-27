// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_KONAMI_H
#define MAME_BUS_NES_KONAMI_H

#pragma once

#include "nxrom.h"
#include "sound/vrc6.h"
#include "sound/ymopl.h"


// ======================> nes_konami_vrc1_device

class nes_konami_vrc1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_mmc_vrom_bank[2];
};


// ======================> nes_konami_vrc2_device

class nes_konami_vrc2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 m_mmc_vrom_bank[8];

private:
	u8 m_latch;
};


// ======================> nes_konami_vrc3_device

class nes_konami_vrc3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter is clocked by M2: it is counted at the M2 rise, before that cycle's CPU access
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_clock(); }
	void irq_clock();

private:
	u16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;
};


// ======================> nes_konami_vrc4_device

class nes_konami_vrc4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter is clocked by M2: it is counted at the M2 rise, before that cycle's CPU access
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_clock(); }
	void irq_clock();

	void set_mirror(u8 data);
	void set_prg(int prg_base, int prg_mask);
	void set_chr(int chr_base, int chr_mask);
	virtual void set_prg() { set_prg(0x00, 0x1f); }
	virtual void set_chr() { set_chr(0x00, 0x1ff); }

	u16 m_mmc_vrom_bank[8];
	u8 m_mmc_prg_bank[2];
	u8 m_prg_flip;
	u8 m_wram_enable;

	void irq_tick();
	virtual void irq_ack_w();
	void irq_ctrl_w(u8 data);
	// the VRC4's /IRQ output; boards that delay it between the VRC4 and the cartridge edge override this
	virtual void vrc_irq_w(int state) { set_irq_line(state); }
	u8 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;
	int m_irq_prescale;
};


// ======================> nes_konami_vrc6_device

class nes_konami_vrc6_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void vrc6_update_ppu();

	required_device<vrc6snd_device> m_vrc6snd;
	u8 m_vrc6_chr[8];   // $D000-$E003 CHR registers
	u8 m_vrc6_mode;     // $B003 PPU banking style
};


// ======================> nes_konami_vrc7_device

class nes_konami_vrc7_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ds1001_device> m_vrc7snd;
};


// ======================> nes_konami_vrc5_device

class ppu2c0x_device;

class nes_konami_vrc5_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual u8 chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, u8 data) override;
	virtual u8 nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter counts M2 cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	void update_prg();
	bool background_fetch();
	u16 qtram_index(offs_t offset) const;
	u8 translated_tile() const;
	u8 translated_bank() const;

	required_device<ppu2c0x_device> m_ppu;
	u8 m_reg[16];           // $D000-$DF00
	u8 m_qtram[0x800];      // the adapter's shadow nametable RAM
	u8 m_qt_byte;           // QTRAM byte fetched with the last background nametable byte
	u16 m_irq_latch, m_irq_count;
	bool m_irq_enable, m_irq_enable_ack;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_VRC1, nes_konami_vrc1_device)
DECLARE_DEVICE_TYPE(NES_VRC2, nes_konami_vrc2_device)
DECLARE_DEVICE_TYPE(NES_VRC3, nes_konami_vrc3_device)
DECLARE_DEVICE_TYPE(NES_VRC4, nes_konami_vrc4_device)
DECLARE_DEVICE_TYPE(NES_VRC5, nes_konami_vrc5_device)
DECLARE_DEVICE_TYPE(NES_VRC6, nes_konami_vrc6_device)
DECLARE_DEVICE_TYPE(NES_VRC7, nes_konami_vrc7_device)

#endif // MAME_BUS_NES_KONAMI_H
