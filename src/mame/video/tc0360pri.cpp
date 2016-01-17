// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0360PRI
---------
Priority manager
A higher priority value means higher priority. 0 could mean disable but
I'm not sure. If two inputs have the same priority value, I think the first
one has priority, but I'm not sure of that either.
It seems the chip accepts three inputs from three different sources, and
each one of them can declare to have four different priority levels.

000 Top two bits indicate special blend mode (see taito_f2.c).  Other bits unused?
001 in games with a roz layer, this is the roz palette bank (bottom 6 bits
    affect roz color, top 2 bits affect priority)
002 unknown
003 unknown

004 ----xxxx \       priority level 0 (usually FG1 if present)
    xxxx---- | Input priority level 1 (usually FG0)
005 ----xxxx |   #1  priority level 2 (usually BG0)
    xxxx---- /       priority level 3 (usually BG1)

006 ----xxxx \       priority level 0 (usually sprites with top color bits 00)
    xxxx---- | Input priority level 1 (usually sprites with top color bits 01)
007 ----xxxx |   #2  priority level 2 (usually sprites with top color bits 10)
    xxxx---- /       priority level 3 (usually sprites with top color bits 11)

008 ----xxxx \       priority level 0 (e.g. roz layer if top bits of register 001 are 00)
    xxxx---- | Input priority level 1 (e.g. roz layer if top bits of register 001 are 01)
009 ----xxxx |   #3  priority level 2 (e.g. roz layer if top bits of register 001 are 10)
    xxxx---- /       priority level 3 (e.g. roz layer if top bits of register 001 are 11)

00a unused
00b unused
00c unused
00d unused
00e unused
00f unused
*/

#include "emu.h"
#include "tc0360pri.h"


const device_type TC0360PRI = &device_creator<tc0360pri_device>;

tc0360pri_device::tc0360pri_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0360PRI, "Taito TC0360PRI", tag, owner, clock, "tc0360pri", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0360pri_device::device_start()
{
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0360pri_device::device_reset()
{
	for (auto & elem : m_regs)
		elem = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_MEMBER( tc0360pri_device::write )
{
	m_regs[offset] = data;

if (offset >= 0x0a)
	popmessage("write %02x to unused TC0360PRI reg %x", data, offset);
#if 0
#define regs m_regs
	popmessage("%02x %02x  %02x %02x  %02x %02x %02x %02x %02x %02x",
		regs[0x00], regs[0x01], regs[0x02], regs[0x03],
		regs[0x04], regs[0x05], regs[0x06], regs[0x07],
		regs[0x08], regs[0x09]);
#endif
}

READ8_MEMBER( tc0360pri_device::read )
{
	return m_regs[offset];
}
