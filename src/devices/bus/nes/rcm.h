// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_RCM_H
#define MAME_BUS_NES_RCM_H

#pragma once

#include "nxrom.h"
#include "cpu/pic16c5x/pic16c5x.h"


// ======================> nes_gs2015_device

class nes_gs2015_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override { return read_m(offset); }
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_gs2004_device

class nes_gs2004_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2004_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_gs2004_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bank);

private:
	const u32 m_base;
};


// ======================> nes_gs2013_device

class nes_gs2013_device : public nes_gs2004_device
{
public:
	// construction/destruction
	nes_gs2013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_tf9_device

class nes_tf9_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tf9_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_3dblock_device

class nes_3dblock_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_3dblock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// the PIC16C54 watches the CPU address bus
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 pic_porta_r();
	u8 pic_portb_r();
	void pic_porta_w(offs_t offset, u8 data, u8 mem_mask);

	required_device<pic16c54_device> m_pic;
	offs_t m_addr;      // the CPU address of the latest cycle
	bool m_irq;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_GS2004,  nes_gs2004_device)
DECLARE_DEVICE_TYPE(NES_GS2013,  nes_gs2013_device)
DECLARE_DEVICE_TYPE(NES_GS2015,  nes_gs2015_device)
DECLARE_DEVICE_TYPE(NES_TF9IN1,  nes_tf9_device)
DECLARE_DEVICE_TYPE(NES_3DBLOCK, nes_3dblock_device)

#endif // MAME_BUS_NES_RCM_H
