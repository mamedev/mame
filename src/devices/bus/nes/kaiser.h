// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_KAISER_H
#define MAME_BUS_NES_KAISER_H

#pragma once

#include "nxrom.h"


// ======================> nes_ks106c_device

class nes_ks106c_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks106c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_ks7058_device

class nes_ks7058_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_ks7022_device

class nes_ks7022_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_ks7032_device

class nes_ks7032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_ks7032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

	u8 m_reg[8];

private:
	void prg_update();
	u8 m_latch;

	u16 m_irq_count, m_irq_count_latch;
	u8 m_irq_enable;

	emu_timer *irq_timer;
};


// ======================> nes_ks202_device

class nes_ks202_device : public nes_ks7032_device
{
public:
	// construction/destruction
	nes_ks202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_ks7016_device

class nes_ks7016_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_ks7016_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 a15_flip);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
	const u8 m_a15_flip;
};


// ======================> nes_ks7016b_device

class nes_ks7016b_device : public nes_ks7016_device
{
public:
	// construction/destruction
	nes_ks7016b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_ks7017_device

class nes_ks7017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_ex(offs_t offset) override;
	virtual void write_ex(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	u8 m_latch;

	u16 m_irq_count;
	u8 m_irq_status;
	u8 m_irq_enable;

	emu_timer *irq_timer;
};


// ======================> nes_ks7021a_device

class nes_ks7021a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7021a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ks7010_device

class nes_ks7010_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_ks7012_device

class nes_ks7012_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7012_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ks7013b_device

class nes_ks7013b_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7013b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ks7030_device

class nes_ks7030_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[2];
};


// ======================> nes_ks7031_device

class nes_ks7031_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[4];
};


// ======================> nes_ks7037_device

class nes_ks7037_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7037_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg;
};


// ======================> nes_ks7057_device

class nes_ks7057_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7057_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[8];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_KS106C,  nes_ks106c_device)
DECLARE_DEVICE_TYPE(NES_KS202,   nes_ks202_device)
DECLARE_DEVICE_TYPE(NES_KS7010,  nes_ks7010_device)
DECLARE_DEVICE_TYPE(NES_KS7012,  nes_ks7012_device)
DECLARE_DEVICE_TYPE(NES_KS7013B, nes_ks7013b_device)
DECLARE_DEVICE_TYPE(NES_KS7016,  nes_ks7016_device)
DECLARE_DEVICE_TYPE(NES_KS7016B, nes_ks7016b_device)
DECLARE_DEVICE_TYPE(NES_KS7017,  nes_ks7017_device)
DECLARE_DEVICE_TYPE(NES_KS7021A, nes_ks7021a_device)
DECLARE_DEVICE_TYPE(NES_KS7022,  nes_ks7022_device)
DECLARE_DEVICE_TYPE(NES_KS7030,  nes_ks7030_device)
DECLARE_DEVICE_TYPE(NES_KS7031,  nes_ks7031_device)
DECLARE_DEVICE_TYPE(NES_KS7032,  nes_ks7032_device)
DECLARE_DEVICE_TYPE(NES_KS7037,  nes_ks7037_device)
DECLARE_DEVICE_TYPE(NES_KS7057,  nes_ks7057_device)
DECLARE_DEVICE_TYPE(NES_KS7058,  nes_ks7058_device)

#endif // MAME_BUS_NES_KAISER_H
