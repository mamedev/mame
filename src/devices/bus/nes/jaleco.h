// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_JALECO_H
#define MAME_BUS_NES_JALECO_H

#pragma once

#include "nxrom.h"
#include "sound/samples.h"


// ======================> nes_jf11_device

class nes_jf11_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
};


// ======================> nes_jf13_device

class nes_jf13_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf13_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_jf16_device

class nes_jf16_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_jf17_device

class nes_jf17_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf17_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_jf17_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool m_prg_flip);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	optional_device<samples_device> m_samples;

private:
	u8 m_latch;
	const bool m_prg_flip;
};


// ======================> nes_jf17_adpcm_device

class nes_jf17_adpcm_device : public nes_jf17_device
{
public:
	// construction/destruction
	nes_jf17_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_jf19_device

class nes_jf19_device : public nes_jf17_device
{
public:
	// construction/destruction
	nes_jf19_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_jf19_adpcm_device

class nes_jf19_adpcm_device : public nes_jf17_device
{
public:
	// construction/destruction
	nes_jf19_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_ss88006_device

class nes_ss88006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ss88006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	nes_ss88006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	optional_device<samples_device> m_samples;

private:
	u16 m_irq_count, m_irq_count_latch;
	u8 m_irq_mode;
	u8 m_irq_enable;

	emu_timer *irq_timer;

	u8 m_mmc_prg_bank[3];
	u8 m_mmc_vrom_bank[8];
	u8 m_wram_protect;
};


// ======================> nes_jf23_device

class nes_jf23_device : public nes_ss88006_device
{
public:
	// construction/destruction
	nes_jf23_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_jf24_device

class nes_jf24_device : public nes_ss88006_device
{
public:
	// construction/destruction
	nes_jf24_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_jf29_device

class nes_jf29_device : public nes_ss88006_device
{
public:
	// construction/destruction
	nes_jf29_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> nes_jf33_device

class nes_jf33_device : public nes_ss88006_device
{
public:
	// construction/destruction
	nes_jf33_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JF11,       nes_jf11_device)
DECLARE_DEVICE_TYPE(NES_JF13,       nes_jf13_device)
DECLARE_DEVICE_TYPE(NES_JF16,       nes_jf16_device)
DECLARE_DEVICE_TYPE(NES_JF17,       nes_jf17_device)
DECLARE_DEVICE_TYPE(NES_JF17_ADPCM, nes_jf17_adpcm_device)
DECLARE_DEVICE_TYPE(NES_JF19,       nes_jf19_device)
DECLARE_DEVICE_TYPE(NES_JF19_ADPCM, nes_jf19_adpcm_device)
DECLARE_DEVICE_TYPE(NES_SS88006,    nes_ss88006_device)
DECLARE_DEVICE_TYPE(NES_JF23,       nes_jf23_device)
DECLARE_DEVICE_TYPE(NES_JF24,       nes_jf24_device)
DECLARE_DEVICE_TYPE(NES_JF29,       nes_jf29_device)
DECLARE_DEVICE_TYPE(NES_JF33,       nes_jf33_device)

#endif // MAME_BUS_NES_JALECO_H
