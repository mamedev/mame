// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_KAISER_H
#define MAME_BUS_NES_KAISER_H

#pragma once

#include "nxrom.h"


// ======================> nes_ks7058_device

class nes_ks7058_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_ks7022_device

class nes_ks7022_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_ks7032_device

class nes_ks7032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(ks7032_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ks7032_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
	nes_ks7032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void prg_update();

	uint8_t m_latch;
	uint8_t m_reg[8];

	uint16_t m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_ks202_device

class nes_ks202_device : public nes_ks7032_device
{
public:
	// construction/destruction
	nes_ks202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
};


// ======================> nes_ks7017_device

class nes_ks7017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_ex) override;
	virtual DECLARE_WRITE8_MEMBER(write_ex) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t m_latch;

	uint16_t m_irq_count;
	uint8_t m_irq_status;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_ks7012_device

class nes_ks7012_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};

// ======================> nes_ks7013b_device

class nes_ks7013b_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7013b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_ks7031_device

class nes_ks7031_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[4];
};


// ======================> nes_ks7016_device

class nes_ks7016_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg;
};

// ======================> nes_ks7037_device

class nes_ks7037_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	uint8_t m_latch;
	uint8_t m_reg[8];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_KS7058,  nes_ks7058_device)
DECLARE_DEVICE_TYPE(NES_KS7022,  nes_ks7022_device)
DECLARE_DEVICE_TYPE(NES_KS7032,  nes_ks7032_device)
DECLARE_DEVICE_TYPE(NES_KS202,   nes_ks202_device)
DECLARE_DEVICE_TYPE(NES_KS7017,  nes_ks7017_device)
DECLARE_DEVICE_TYPE(NES_KS7012,  nes_ks7012_device)
DECLARE_DEVICE_TYPE(NES_KS7013B, nes_ks7013b_device)
DECLARE_DEVICE_TYPE(NES_KS7031,  nes_ks7031_device)
DECLARE_DEVICE_TYPE(NES_KS7016,  nes_ks7016_device)
DECLARE_DEVICE_TYPE(NES_KS7037,  nes_ks7037_device)

#endif // MAME_BUS_NES_KAISER_H
