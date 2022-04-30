// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Sharp LZ8420M Z80 with Built-in RAM

    (TODO: everything;
        8 Bit IO, 4 Bit Input, 4 Bit Output, 512 Byte RAM)

***************************************************************************/

#include "emu.h"
#include "lz8420m.h"

DEFINE_DEVICE_TYPE(LZ8420M, lz8420m_device, "lz8420m", "Sharp LZ8420M")


lz8420m_device::lz8420m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80_device(mconfig, LZ8420M, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lz8420m_device::device_start()
{
	z80_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lz8420m_device::device_reset()
{
	z80_device::device_reset();
}


/* CPU interface */
void lz8420m_device::device_add_mconfig(machine_config &config)
{
}
