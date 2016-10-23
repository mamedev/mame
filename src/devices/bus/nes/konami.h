// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_KONAMI_H
#define __NES_KONAMI_H

#include "nxrom.h"
#include "sound/vrc6.h"
#include "sound/ym2413.h"


// ======================> nes_konami_vrc1_device

class nes_konami_vrc1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_mmc_vrom_bank[2];
};


// ======================> nes_konami_vrc2_device

class nes_konami_vrc2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_mmc_vrom_bank[8];
	uint8_t m_latch;
};


// ======================> nes_konami_vrc3_device

class nes_konami_vrc3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint16_t m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_konami_vrc4_device

class nes_konami_vrc4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	void set_prg();
	uint8_t m_mmc_vrom_bank[8];
	uint8_t m_latch, m_mmc_prg_bank;

	void irq_tick();
	uint16_t m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;
	int m_irq_prescale;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_konami_vrc6_device

class nes_konami_vrc6_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	required_device<vrc6snd_device> m_vrc6snd;
};


// ======================> nes_konami_vrc7_device

class nes_konami_vrc7_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	required_device<ym2413_device> m_ym2413;
};



// device type definition
extern const device_type NES_VRC1;
extern const device_type NES_VRC2;
extern const device_type NES_VRC3;
extern const device_type NES_VRC4;
extern const device_type NES_VRC6;
extern const device_type NES_VRC7;

#endif
