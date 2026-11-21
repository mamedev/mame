// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_PIRATE_H
#define MAME_BUS_NES_PIRATE_H

#pragma once

#include "nxrom.h"
#include "sound/ymopl.h"


// ======================> nes_agci_device

class nes_agci_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_agci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_dreamtech_device

class nes_dreamtech_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dreamtech_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_kkachi_device

class nes_kkachi_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_kkachi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_noraebang_device

class nes_noraebang_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_noraebang_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ym2413_device> m_ym2413;
	u8 m_reg;                   // $8000: R BBBBBB, R = 1 selects the internal ROM
	u8 m_spi_cs, m_spi_clk;     // microphone ADC lines ($C002 bit 7, active low, and $C003 bit 7)
	u8 m_spi_count;             // falling clock edges since the chip was selected
	u8 m_spi_out;               // the ADC's data out line, read at $6003 bit 7
};


// ======================> nes_fukutake_device

class nes_fukutake_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fukutake_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_latch;
	uint8_t m_ram[0xb00];
};


// ======================> nes_futuremedia_device

class nes_futuremedia_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_futuremedia_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void hblank_irq(int scanline, bool vblank, bool blanked) override;
	// Mesen2 and Nestopia clock the counter from PPU A12 like an MMC3 (NESdev only says "scanline minus $14 (?)"); the PPU's
	// own A12 filter is used, as Mesen2's A12 watcher does, and the hblank callback is only the fallback
	virtual bool wants_a12() override { return true; }
	virtual void ppu_a12_rise() override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_irq_count, m_irq_count_latch;
	int m_irq_enable;
};


// ======================> nes_magseries_device

class nes_magseries_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_magseries_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_daou306_device

class nes_daou306_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_daou306_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[16];
};


// ======================> nes_cc21_device

class nes_cc21_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cc21_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_xiaozy_device

class nes_xiaozy_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_xiaozy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_edu2k_device

class nes_edu2k_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_edu2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_latch;
};


// ======================> nes_jy830623c_device

class nes_jy830623c_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy830623c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
	virtual void set_m91_submapper(u8 val) override { m_submapper = val; }

	// submapper 0 counts unfiltered PPU A12 rises (the raw level, not the filtered ppu_a12_rise), submapper 1 counts M2
	virtual bool wants_a12_level() override { return m_submapper == 0; }
	virtual void ppu_a12_level(int state) override;
	virtual bool wants_m2() override { return m_submapper == 1; }
	virtual void m2_w(int state, offs_t address, bool write) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();
	u8 m_submapper;
	u8 m_latch;
	u8 m_reg[6];

	int m_irq_enable;
	u8 m_a12_count;         // submapper 0: unfiltered PPU A12 rises since the IRQ was started
	u8 m_a12_last;          // last A12 level seen
	s16 m_m2_count;         // submapper 1: counts down by 5 every fourth M2 cycle
	u8 m_m2_prescaler;
};


// ======================> nes_43272_device

class nes_43272_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_43272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint16_t m_latch;
};


// ======================> nes_eh8813a_device

class nes_eh8813a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_eh8813a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_jumper;
	u16 m_latch;
	u8 m_reg;
};



// ======================> nes_fujiya_device

class nes_fujiya_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fujiya_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_latch;
};


// ======================> nes_bj56_device

class nes_bj56_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bj56_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the IRQ counter counts M2 cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	u16 m_irq_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_AGCI_50282,  nes_agci_device)
DECLARE_DEVICE_TYPE(NES_DREAMTECH01, nes_dreamtech_device)
DECLARE_DEVICE_TYPE(NES_FUKUTAKE,    nes_fukutake_device)
DECLARE_DEVICE_TYPE(NES_KKACHI,      nes_kkachi_device)
DECLARE_DEVICE_TYPE(NES_NORAEBANG,   nes_noraebang_device)
DECLARE_DEVICE_TYPE(NES_FUTUREMEDIA, nes_futuremedia_device)
DECLARE_DEVICE_TYPE(NES_MAGSERIES,   nes_magseries_device)
DECLARE_DEVICE_TYPE(NES_DAOU306,     nes_daou306_device)
DECLARE_DEVICE_TYPE(NES_CC21,        nes_cc21_device)
DECLARE_DEVICE_TYPE(NES_XIAOZY,      nes_xiaozy_device)
DECLARE_DEVICE_TYPE(NES_EDU2K,       nes_edu2k_device)
DECLARE_DEVICE_TYPE(NES_JY830623C,   nes_jy830623c_device)
DECLARE_DEVICE_TYPE(NES_43272,       nes_43272_device)
DECLARE_DEVICE_TYPE(NES_EH8813A,     nes_eh8813a_device)
DECLARE_DEVICE_TYPE(NES_BJ56,        nes_bj56_device)
DECLARE_DEVICE_TYPE(NES_FUJIYA,      nes_fujiya_device)

#endif // MAME_BUS_NES_PIRATE_H
