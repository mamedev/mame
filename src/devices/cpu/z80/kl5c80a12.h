// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Kawasaki LSI
    KL5C80A12 CPU (KL5C80A12CFP on hng64.c)

    Binary compatible with Z80, significantly faster opcode timings, operating at up to 10Mhz
    Timers / Counters, Parrallel / Serial ports/ MMU, Interrupt Controller

    (is this different enough to need it's own core?)
    (todo: everything, some code currently lives in machine/hng64_net.c but not much)

***************************************************************************/

#pragma once

#ifndef __KL5C80A12__
#define __KL5C80A12__

#include "emu.h"
#include "z80.h"
#include "machine/z80ctc.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class kl5c80a12_device : public z80_device
{
public:
	kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
};


// device type definition
extern const device_type KL5C80A12;


#endif /// __KL5C80A12__
