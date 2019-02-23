// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Kawasaki LSI
    KL5C80A12 CPU (KL5C80A12CFP on hng64.c)

    Binary compatible with Z80, significantly faster opcode timings, operating at up to 10Mhz
    Timers / Counters, Parallel / Serial ports/ MMU, Interrupt Controller

    (is this different enough to need it's own core?)
    (todo: everything, some code currently lives in machine/hng64_net.c but not much)

***************************************************************************/

#include "emu.h"
#include "kl5c80a12.h"

DEFINE_DEVICE_TYPE(KL5C80A12, kl5c80a12_device, "kl5c80a12", "Kawasaki LSI KL5C80A12")


kl5c80a12_device::kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80_device(mconfig, KL5C80A12, tag, owner, clock)
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
void kl5c80a12_device::device_add_mconfig(machine_config &config)
{
}
