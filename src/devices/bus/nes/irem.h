// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_IREM_H
#define __NES_IREM_H

#include "nxrom.h"


// ======================> nes_lrog017_device

class nes_lrog017_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_lrog017_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_holydivr_device

class nes_holydivr_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_holydivr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_tam_s1_device

class nes_tam_s1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tam_s1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_g101_device

class nes_g101_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_g101_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

protected:
	UINT8     m_latch;
};


// ======================> nes_h3001_device

class nes_h3001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_h3001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

protected:
	UINT16     m_irq_count, m_irq_count_latch;
	int        m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};



// device type definition
extern const device_type NES_LROG017;
extern const device_type NES_HOLYDIVR;
extern const device_type NES_TAM_S1;
extern const device_type NES_G101;
extern const device_type NES_H3001;

#endif
