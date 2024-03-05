// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_port.h

    SH i/o ports

***************************************************************************/

#include "emu.h"
#include "sh_intc.h"

#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH_PORT16, sh_port16_device, "sh_port16", "SH 16-bits port")
DEFINE_DEVICE_TYPE(SH_PORT32, sh_port32_device, "sh_port32", "SH 32-bits port")

sh_port16_device::sh_port16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_PORT16, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sh_port16_device::device_start()
{
	m_io = m_default_io;
	save_item(NAME(m_dr));
	save_item(NAME(m_io));
}

void sh_port16_device::device_reset()
{
}

u16 sh_port16_device::dr_r()
{
	if(~m_io & ~m_mask)
		return (m_dr & m_io) | (m_cpu->do_read_port16(m_index) & ~m_io);
	return m_dr;
}

void sh_port16_device::dr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dr);
	m_dr &= ~m_mask;
	if(m_io)
		m_cpu->do_write_port16(m_index, m_dr & m_io, m_io);
}

u16 sh_port16_device::io_r()
{
	return m_io;
}

void sh_port16_device::io_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_io);
	m_io &= ~m_mask;
	if(m_io)
		m_cpu->do_write_port16(m_index, m_dr & m_io, m_io);
}


sh_port32_device::sh_port32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_PORT32, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sh_port32_device::device_start()
{
	m_io = m_default_io;
	save_item(NAME(m_dr));
	save_item(NAME(m_io));
}

void sh_port32_device::device_reset()
{
}

u32 sh_port32_device::dr_r()
{
	if((~m_io) & (~m_mask))
		return (m_dr & m_io) | (m_cpu->do_read_port32(m_index) & ~m_io);
	return m_dr;
}

void sh_port32_device::dr_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dr);
	m_dr &= ~m_mask;
	if(m_io)
		m_cpu->do_write_port32(m_index, m_dr & m_io, m_io);
}

u32 sh_port32_device::io_r()
{
	return m_io;
}

void sh_port32_device::io_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io);
	m_io &= ~m_mask;
	if(m_io)
		m_cpu->do_write_port32(m_index, m_dr & m_io, m_io);	
}

