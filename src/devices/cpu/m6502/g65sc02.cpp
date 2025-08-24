// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    g65sc02.cpp

    65c02 with internal static registers, making clock stoppable?

    TODO:
    - none of the CPU type differences are currently emulated (eg. BE pin, ML pin),
      are any of them meaningful to MAME?

***************************************************************************/

#include "emu.h"
#include "g65sc02.h"

DEFINE_DEVICE_TYPE(G65SC02, g65sc02_device, "g65sc02", "GTE G65SC02")
DEFINE_DEVICE_TYPE(G65SC12, g65sc12_device, "g65sc12", "GTE G65SC12")
DEFINE_DEVICE_TYPE(G65SC102, g65sc102_device, "g65sc102", "GTE G65SC102")
DEFINE_DEVICE_TYPE(G65SC112, g65sc112_device, "g65sc112", "GTE G65SC112")

g65sc02_device::g65sc02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	w65c02_device(mconfig, type, tag, owner, clock)
{
}

g65sc02_device::g65sc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	g65sc02_device(mconfig, G65SC02, tag, owner, clock)
{
}

g65sc12_device::g65sc12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	g65sc02_device(mconfig, G65SC12, tag, owner, clock)
{
}

g65sc102_device::g65sc102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	g65sc02_device(mconfig, G65SC102, tag, owner, clock)
{
}

g65sc112_device::g65sc112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	g65sc02_device(mconfig, G65SC112, tag, owner, clock)
{
}
