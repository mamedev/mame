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
	nes_jf11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_jf13_device

class nes_jf13_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_jf16_device

class nes_jf16_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_jf17_device

class nes_jf17_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf17_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	nes_jf17_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	uint8_t m_latch;
};


// ======================> nes_jf17_adpcm_device

class nes_jf17_adpcm_device : public nes_jf17_device
{
public:
	// construction/destruction
	nes_jf17_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_jf19_device

class nes_jf19_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf19_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	nes_jf19_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_jf19_adpcm_device

class nes_jf19_adpcm_device : public nes_jf19_device
{
public:
	// construction/destruction
	nes_jf19_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_ss88006_device

class nes_ss88006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ss88006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(ss88006_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
	nes_ss88006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint16_t m_irq_count, m_irq_count_latch;
	uint8_t m_irq_mode;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	uint8_t m_mmc_prg_bank[3];
	uint8_t m_mmc_vrom_bank[8];

	uint8_t m_latch; // used for samples, in derived classes
};


// ======================> nes_ss88006_adpcm_device

class nes_ss88006_adpcm_device : public nes_ss88006_device
{
protected:
	// construction/destruction
	nes_ss88006_adpcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void ss88006_adpcm_write(address_space &space, offs_t offset, uint8_t data, samples_device *dev);
};


// ======================> nes_jf23_device

class nes_jf23_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf23_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }

	required_device<samples_device> m_samples;
};


// ======================> nes_jf24_device

class nes_jf24_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }

	required_device<samples_device> m_samples;
};


// ======================> nes_jf29_device

class nes_jf29_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf29_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }

	required_device<samples_device> m_samples;
};


// ======================> nes_jf33_device

class nes_jf33_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf33_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }

	required_device<samples_device> m_samples;
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
