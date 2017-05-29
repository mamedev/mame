// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_RCM_H
#define MAME_BUS_NES_RCM_H

#pragma once

#include "nxrom.h"


// ======================> nes_gs2015_device

class nes_gs2015_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_l) override { return read_m(space, offset, mem_mask); }
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_gs2004_device

class nes_gs2004_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_gs2013_device

class nes_gs2013_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_tf9_device

class nes_tf9_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tf9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_3dblock_device

class nes_3dblock_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_3dblock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[4];
	uint8_t m_irq_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_GS2015,  nes_gs2015_device)
DECLARE_DEVICE_TYPE(NES_GS2004,  nes_gs2004_device)
DECLARE_DEVICE_TYPE(NES_GS2013,  nes_gs2013_device)
DECLARE_DEVICE_TYPE(NES_TF9IN1,  nes_tf9_device)
DECLARE_DEVICE_TYPE(NES_3DBLOCK, nes_3dblock_device)

#endif // MAME_BUS_NES_RCM_H
