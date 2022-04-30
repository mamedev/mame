// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Atari Jaguar "Blitter" device

    TODO:
    - Stub device, port/rewrite from jagblit;
    - actual codename/chip part number;
    - has different revs, encapsulate;

***************************************************************************/

#include "emu.h"
#include "jag_blitter.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(JAG_BLITTER, jag_blitter_device, "jag_blitter", "Atari Jaguar Blitter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jag_blitter_device - constructor
//-------------------------------------------------

jag_blitter_device::jag_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JAG_BLITTER, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_BIG, 32, 8, 0, address_map_constructor(FUNC(jag_blitter_device::regs_map), this))
	, m_command_latch(0)
	, m_status_idle(true)
	, m_count_lines(0)
	, m_count_pixels(0)
{
	m_a1.base = 0;
	m_a1.xstep = 0;
	m_a1.ystep = 0;
}

device_memory_interface::space_config_vector jag_blitter_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jag_blitter_device::device_start()
{
	save_item(NAME(m_status_idle));
	save_item(NAME(m_command_latch));
	save_item(NAME(m_count_lines));
	save_item(NAME(m_count_pixels));
	save_item(NAME(m_a1.base));
	save_item(NAME(m_a1.xstep));
	save_item(NAME(m_a1.ystep));

	m_command_timer = timer_alloc(0);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jag_blitter_device::device_reset()
{
	m_command_latch = 0;
	m_status_idle = true;

	m_command_timer->adjust(attotime::never);
}

//--------------------------------------------------
// core workings
//--------------------------------------------------

/***************************************************************************
    FUNCTION TABLES
***************************************************************************/

const jag_blitter_device::op_func jag_blitter_device::upda_ops[8] =
{
	&jag_blitter_device::op_nop,
	&jag_blitter_device::op_unemulated, // upda1f
	&jag_blitter_device::op_upda1,
	&jag_blitter_device::op_unemulated, // upda1 + upda1f

	&jag_blitter_device::op_unemulated, // upda2
	&jag_blitter_device::op_unemulated, // upda1f + upda2
	&jag_blitter_device::op_unemulated, // upda1 + upda2
	&jag_blitter_device::op_unemulated  // upda1 + upda1f + upda2
};

void jag_blitter_device::op_nop()
{
	// do nothing for this step
}

// common cases for unhandled, will be removed once everything is set
void jag_blitter_device::op_unemulated()
{
	throw emu_fatalerror("%s: unhandled step with command latch %08x",m_command_latch);
}

void jag_blitter_device::op_upda1()
{
	// ...

}

inline void jag_blitter_device::command_start()
{
	if (m_status_idle == false)
		throw emu_fatalerror("%s: inflight blitter trigger %08x", this->tag(), m_command_latch);
	m_status_idle = false;
	// TODO: to be removed, see below
	m_command_timer->adjust(attotime::from_ticks(m_count_lines * m_count_pixels, this->clock()));
}

inline void jag_blitter_device::command_run()
{
	// TODO: need to single step, have different timings between pixel and phrase modes,
	// calculate collision detection, delay a bit the kickoff due of bus chain requests,
	// take more time by virtue of using additional steps, be a civilian or not depending
	// of BUSHI setting

	printf("%08x\n",m_command_latch);
	// init
	m_a1.ptr = m_a1.base;

	// ...

	command_done();
}

inline void jag_blitter_device::command_done()
{
	// m_status_idle = true;
	// ...
}

void jag_blitter_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	command_run();
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void jag_blitter_device::regs_map(address_map &map)
{
	// $f02200
	map(0x00, 0x03).w(FUNC(jag_blitter_device::a1_base_w));
	map(0x10, 0x13).w(FUNC(jag_blitter_device::a1_ystep_w)).umask32(0xffff0000);
	map(0x10, 0x13).w(FUNC(jag_blitter_device::a1_xstep_w)).umask32(0x0000ffff);
	map(0x38, 0x3b).rw(FUNC(jag_blitter_device::status_r), FUNC(jag_blitter_device::command_w));
	map(0x3c, 0x3f).w(FUNC(jag_blitter_device::count_outer_w)).umask32(0xffff0000);
	map(0x3c, 0x3f).w(FUNC(jag_blitter_device::count_inner_w)).umask32(0x0000ffff);
}

void jag_blitter_device::a1_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_a1.base);
	// align to phrase
	m_a1.base &= ~7;
}

void jag_blitter_device::a1_xstep_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_a1.xstep);
}

void jag_blitter_device::a1_ystep_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_a1.ystep);
}

u32 jag_blitter_device::status_r()
{
	// TODO: stopped bit
	// TODO: diag bits 2-31
	return m_status_idle;
}

void jag_blitter_device::command_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_command_latch);
	// TODO: is it possible from 68k to write on this in byte units?
	// We may just do so in order to take endianness into account, or just delegate to the overlying bus framework,
	// may be a common problem with ALL video regs for that matter.
	if (ACCESSING_BITS_0_15)
		command_start();
}

void jag_blitter_device::count_outer_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_count_lines);
	if (m_count_lines == 0)
	{
		// according to documentation, log it out
		m_count_lines = 0x10000;
		popmessage("Blitter: line count set to max, contact MAMEdev");
	}
}

void jag_blitter_device::count_inner_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_count_pixels);
	if (m_count_pixels == 0)
	{
		// according to documentation, log it out
		m_count_pixels = 0x10000;
		popmessage("Blitter: pixel count set to max, contact MAMEdev");
	}
}

u32 jag_blitter_device::iobus_r(offs_t offset, u32 mem_mask)
{
	return space().read_dword(offset*4, mem_mask);
}

void jag_blitter_device::iobus_w(offs_t offset, u32 data, u32 mem_mask)
{
	space().write_dword(offset*4, data, mem_mask);
}
