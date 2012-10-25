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

extern const device_type PSX_IRQ;

class psxirq_device : public device_t
{
public:
	psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( intin0 );
	DECLARE_WRITE_LINE_MEMBER( intin1 );
	DECLARE_WRITE_LINE_MEMBER( intin2 );
	DECLARE_WRITE_LINE_MEMBER( intin3 );
	DECLARE_WRITE_LINE_MEMBER( intin4 );
	DECLARE_WRITE_LINE_MEMBER( intin5 );
	DECLARE_WRITE_LINE_MEMBER( intin6 );
	DECLARE_WRITE_LINE_MEMBER( intin7 );
	DECLARE_WRITE_LINE_MEMBER( intin8 );
	DECLARE_WRITE_LINE_MEMBER( intin9 );
	DECLARE_WRITE_LINE_MEMBER( intin10 );

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
