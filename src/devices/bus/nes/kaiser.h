// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_KAISER_H
#define __NES_KAISER_H

#include "nxrom.h"


// ======================> nes_ks7058_device

class nes_ks7058_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ks7022_device

class nes_ks7022_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
};


// ======================> nes_ks7032_device

class nes_ks7032_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7032_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void ks7032_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { ks7032_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
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
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// ======================> nes_ks7017_device

class nes_ks7017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual uint8_t read_ex(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ex(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

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

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};

// ======================> nes_ks7013b_device

class nes_ks7013b_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7013b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ks7031_device

class nes_ks7031_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_reg[4];
};


// ======================> nes_ks7016_device

class nes_ks7016_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_reg;
};

// ======================> nes_ks7037_device

class nes_ks7037_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ks7037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	void update_prg();
	uint8_t m_latch;
	uint8_t m_reg[8];
};




// device type definition
extern const device_type NES_KS7058;
extern const device_type NES_KS7022;
extern const device_type NES_KS7032;
extern const device_type NES_KS202;
extern const device_type NES_KS7017;
extern const device_type NES_KS7012;
extern const device_type NES_KS7013B;
extern const device_type NES_KS7031;
extern const device_type NES_KS7016;
extern const device_type NES_KS7037;

#endif
