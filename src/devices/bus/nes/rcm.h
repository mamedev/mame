// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_RCM_H
#define __NES_RCM_H

#include "nxrom.h"


// ======================> nes_gs2015_device

class nes_gs2015_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override { return read_m(space, offset, mem_mask); }
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gs2004_device

class nes_gs2004_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gs2013_device

class nes_gs2013_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gs2013_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_tf9_device

class nes_tf9_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tf9_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_3dblock_device

class nes_3dblock_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_3dblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	UINT8 m_reg[4];
	UINT8 m_irq_count;
};




// device type definition
extern const device_type NES_GS2015;
extern const device_type NES_GS2004;
extern const device_type NES_GS2013;
extern const device_type NES_TF9IN1;
extern const device_type NES_3DBLOCK;

#endif
