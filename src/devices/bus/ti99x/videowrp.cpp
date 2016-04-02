// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4A Video subsystem
    This device actually wraps the naked video chip implementation

    EVPC (Enhanced Video Processor Card) from SNUG
    based on v9938 (may also be equipped with v9958)
    Can be used with TI-99/4A as an add-on card; internal VDP must be removed

    The SGCPU ("TI-99/4P") only runs with EVPC

    Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "videowrp.h"

/*
    Constructors
*/
ti_video_device::ti_video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
: bus8z_device(mconfig, type, name, tag, owner, clock, shortname, source),
m_tms9928a(nullptr)
{
}

ti_std_video_device::ti_std_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti_video_device(mconfig, TI99VIDEO, "TI99 STD Video subsystem", tag, owner, clock, "ti99_video", __FILE__)
{
}

ti_exp_video_device::ti_exp_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti_video_device(mconfig, V9938VIDEO, "TI99 EXP Video subsystem", tag, owner, clock, "v9938_video", __FILE__),
		m_v9938(nullptr)
{
}

/*****************************************************************************/
/*
    Accessing TMS9928A (TI-99/4A)
*/
READ8Z_MEMBER( ti_std_video_device::readz )
{
	if (space.debugger_access()) return;

	if (offset & 2)
	{       /* read VDP status */
		*value = m_tms9928a->register_read(space, 0);
	}
	else
	{       /* read VDP RAM */
		*value = m_tms9928a->vram_read(space, 0);
	}
}

WRITE8_MEMBER( ti_std_video_device::write )
{
	if (space.debugger_access()) return;

	if (offset & 2)
	{   /* write VDP address */
		m_tms9928a->register_write(space, 0, data);
	}
	else
	{   /* write VDP data */
		m_tms9928a->vram_write(space, 0, data);
	}
}

/*****************************************************************************/

/*
    Accessing v9938 via 16 bit bus (SGCPU)
*/
READ16_MEMBER( ti_exp_video_device::read16 )
{
	if (space.debugger_access()) return 0;
	return (int)(m_v9938->read(space, offset)<<8);
}

WRITE16_MEMBER( ti_exp_video_device::write16 )
{
	if (space.debugger_access()) return;
	m_v9938->write(space, offset, (data>>8)&0xff);
}


/******************************************************************************/

/*
    Accessing v9938 via 8 bit bus (EVPC)
*/
READ8Z_MEMBER( ti_exp_video_device::readz )
{
	if (space.debugger_access()) return;
	*value = m_v9938->read(space, offset>>1);
}

WRITE8_MEMBER( ti_exp_video_device::write )
{
	if (space.debugger_access()) return;
	m_v9938->write(space, offset>>1, data);
}

void ti_video_device::device_start(void)
{
	m_tms9928a = static_cast<tms9928a_device*>(machine().device(VDP_TAG));
}

void ti_exp_video_device::device_start(void)
{
	m_v9938 = static_cast<v9938_device*>(machine().device(VDP_TAG));
}

/**************************************************************************/

const device_type TI99VIDEO = &device_creator<ti_std_video_device>;
const device_type V9938VIDEO = &device_creator<ti_exp_video_device>;
