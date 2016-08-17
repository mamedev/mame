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

#include "kl5c80a12.h"

const device_type KL5C80A12 = &device_creator<kl5c80a12_device>;


kl5c80a12_device::kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, KL5C80A12, "KL5C80A12", tag, owner, clock, "kl5c80a12", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kl5c80a12_device::device_start()
{
	z80_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kl5c80a12_device::device_reset()
{
	z80_device::device_reset();
}


/* CPU interface */
static MACHINE_CONFIG_FRAGMENT( kl5c80a12 )
MACHINE_CONFIG_END

machine_config_constructor kl5c80a12_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kl5c80a12 );
}
