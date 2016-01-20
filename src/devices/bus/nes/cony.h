// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_CONY_H
#define __NES_CONY_H

#include "nxrom.h"


// ======================> nes_cony_device

class nes_cony_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cony_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_cony_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	virtual void set_prg();
	virtual void set_chr();

	UINT16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	UINT8 m_latch1, m_latch2;
	UINT8 m_low_reg[4];
	UINT8 m_reg[10];
	UINT8 m_extra1;
};


// ======================> nes_yoko_device

class nes_yoko_device : public nes_cony_device
{
public:
	// construction/destruction
	nes_yoko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

private:
	virtual void set_prg() override;
	virtual void set_chr() override;

	UINT8 m_extra2;
};





// device type definition
extern const device_type NES_CONY;
extern const device_type NES_YOKO;

#endif
