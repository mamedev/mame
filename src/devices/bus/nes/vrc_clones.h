// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
#ifndef MAME_BUS_NES_VRC_CLONES_H
#define MAME_BUS_NES_VRC_CLONES_H

#pragma once

#include "konami.h"


// ======================> nes_900218_device

class nes_900218_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_900218_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	u16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_ax40g_device

class nes_ax40g_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_ax40g_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_ax5705_device

class nes_ax5705_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_cityfight_device

class nes_cityfight_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_shuiguan_device

class nes_shuiguan_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_reg;
};


// ======================> nes_tf1201_device

class nes_tf1201_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_th21311_device

class nes_th21311_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_th21311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	u16 m_irq_count;
	u8 m_irq_latch;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_900218,    nes_900218_device)
DECLARE_DEVICE_TYPE(NES_AX40G,     nes_ax40g_device)
DECLARE_DEVICE_TYPE(NES_AX5705,    nes_ax5705_device)
DECLARE_DEVICE_TYPE(NES_CITYFIGHT, nes_cityfight_device)
DECLARE_DEVICE_TYPE(NES_SHUIGUAN,  nes_shuiguan_device)
DECLARE_DEVICE_TYPE(NES_TF1201,    nes_tf1201_device)
DECLARE_DEVICE_TYPE(NES_TH21311,   nes_th21311_device)

#endif // MAME_BUS_NES_VRC_CLONES_H
