#ifndef __NES_LEGACY_H
#define __NES_LEGACY_H

#include "machine/nes_nxrom.h"


// ======================> nes_ffe3_device

class nes_ffe3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};


// ======================> nes_ffe4_device

class nes_ffe4_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ffe4_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_ffe4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

protected:
	UINT16 m_irq_count;
	int m_irq_enable;

	UINT8 m_latch;
};


// ======================> nes_ffe8_device

class nes_ffe8_device : public nes_ffe4_device
{
public:
	// construction/destruction
	nes_ffe8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h) {}

	virtual void pcb_reset();
};



// device type definition
extern const device_type NES_FFE3;
extern const device_type NES_FFE4;
extern const device_type NES_FFE8;


#endif
