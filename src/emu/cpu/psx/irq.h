/*
 * PlayStation IRQ emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXIRQ_H__
#define __PSXIRQ_H__

#include "emu.h"

#define PSX_IRQ_VBLANK			0x0001
#define PSX_IRQ_CDROM			0x0004
#define PSX_IRQ_DMA				0x0008
#define PSX_IRQ_ROOTCOUNTER0	0x0010
#define PSX_IRQ_ROOTCOUNTER1	0x0020
#define PSX_IRQ_ROOTCOUNTER2	0x0040
#define PSX_IRQ_SIO0			0x0080
#define PSX_IRQ_SIO1			0x0100
#define PSX_IRQ_SPU				0x0200
#define PSX_IRQ_EXTCD			0x0400
#define PSX_IRQ_MASK			(PSX_IRQ_VBLANK | PSX_IRQ_CDROM | PSX_IRQ_DMA | PSX_IRQ_ROOTCOUNTER2 | PSX_IRQ_ROOTCOUNTER1 | PSX_IRQ_ROOTCOUNTER0 | PSX_IRQ_SIO0 | PSX_IRQ_SIO1 | PSX_IRQ_SPU | PSX_IRQ_EXTCD)

extern const device_type PSX_IRQ;

class psxirq_device : public device_t
{
public:
	psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	void set( UINT32 bitmask );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

private:
	void psx_irq_update( void );

	UINT32 n_irqdata;
	UINT32 n_irqmask;
};

#endif
