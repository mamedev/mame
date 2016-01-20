// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_LEGACY_H
#define __NES_LEGACY_H

#include "nxrom.h"


// ======================> nes_ffe3_device

class nes_ffe3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ffe4_device

class nes_ffe4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe4_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_ffe4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_WRITE8_MEMBER(chr_w) override;

	virtual void pcb_reset() override;

protected:
	UINT16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	UINT8 m_latch;
	int m_exram_enabled;
	int m_exram_bank;
	UINT8 m_exram[0x8000];
};


// ======================> nes_ffe8_device

class nes_ffe8_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe8_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override {}

	virtual void pcb_reset() override;
};



// device type definition
extern const device_type NES_FFE3;
extern const device_type NES_FFE4;
extern const device_type NES_FFE8;


#endif
