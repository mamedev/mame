// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4(A) and /8 Video subsystem
    This device actually wraps the naked video chip implementation

    EVPC (Enhanced Video Processor Card) from SNUG
    based on v9938 (may also be equipped with v9958)
    Can be used with TI-99/4A as an add-on card; internal VDP must be removed

    The SGCPU ("TI-99/4P") only runs with EVPC

    We also include a class wrapper for the sound chip here.

    Michael Zapf

    October 2010
    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "videowrp.h"
#include "sound/sn76496.h"

/*
    Constructors
*/
ti_video_device::ti_video_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
: bus8z_device(mconfig, type, name, tag, owner, clock, shortname, source),
m_tms9928a(nullptr)
{
}

ti_std_video_device::ti_std_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ti_video_device(mconfig, TI99VIDEO, "TI99 STD Video subsystem", tag, owner, clock, "ti99_video", __FILE__)
{
}

ti_exp_video_device::ti_exp_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ti_video_device(mconfig, V9938VIDEO, "TI99 EXP Video subsystem", tag, owner, clock, "v9938_video", __FILE__), m_v9938(nullptr)
{
}

ti_sound_sn94624_device::ti_sound_sn94624_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ti_sound_system_device(mconfig, TISOUND_94624, "Onboard sound (SN94624)", tag, owner, clock, "ti_sound_sn94624", __FILE__)
{
}

ti_sound_sn76496_device::ti_sound_sn76496_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ti_sound_system_device(mconfig, TISOUND_76496, "Onboard sound (SN76496)", tag, owner, clock, "ti_sound_sn76496", __FILE__)
{
}

/*****************************************************************************/
/*
    Memory access (TI-99/4A and TI-99/8)
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
    Memory access (EVPC) via 16 bit bus
*/
READ16_MEMBER( ti_exp_video_device::read16 )
{
	if (space.debugger_access()) return 0;

	if (offset & 1)
	{   /* read VDP status */
		return ((int) m_v9938->status_r()) << 8;
	}
	else
	{   /* read VDP RAM */
		return ((int) m_v9938->vram_r()) << 8;
	}
}

WRITE16_MEMBER( ti_exp_video_device::write16 )
{
	if (space.debugger_access()) return;

	switch (offset & 3)
	{
	case 0:
		/* write VDP data */
		m_v9938->vram_w((data >> 8) & 0xff);
		break;
	case 1:
		/* write VDP address */
		m_v9938->command_w((data >> 8) & 0xff);
		break;
	case 2:
		/* write VDP palette */
		m_v9938->palette_w((data >> 8) & 0xff);
		break;
	case 3:
		/* write VDP register pointer (indirect access) */
		m_v9938->register_w((data >> 8) & 0xff);
		break;
	}
}

/******************************************************************************/

/*
    Video read (Geneve) via 8 bit bus
*/
READ8Z_MEMBER( ti_exp_video_device::readz )
{
	if (space.debugger_access()) return;

	if (offset & 2)
	{   /* read VDP status */
		*value = m_v9938->status_r();
	}
	else
	{   /* read VDP RAM */
		*value = m_v9938->vram_r();
	}
}

/*
    Video write (Geneve)
*/
WRITE8_MEMBER( ti_exp_video_device::write )
{
	if (space.debugger_access()) return;

	switch (offset & 6)
	{
	case 0:
		/* write VDP data */
		m_v9938->vram_w(data);
		break;
	case 2:
		/* write VDP address */
		m_v9938->command_w(data);
		break;
	case 4:
		/* write VDP palette */
		m_v9938->palette_w(data);
		break;
	case 6:
		/* write VDP register pointer (indirect access) */
		m_v9938->register_w(data);
		break;
	}
}

/**************************************************************************/
// Interfacing to mouse attached to v9938

void ti_exp_video_device::video_update_mouse(int delta_x, int delta_y, int buttons)
{
	m_v9938->update_mouse_state(delta_x, delta_y, buttons & 3);
}

/**************************************************************************/

void ti_video_device::device_start(void)
{
	m_tms9928a = static_cast<tms9928a_device*>(machine().device(VDP_TAG));
}

void ti_exp_video_device::device_start(void)
{
	m_v9938 = static_cast<v9938_device*>(machine().device(VDP_TAG));
}

void ti_video_device::device_reset(void)
{
}

/**************************************************************************/

/*
    Sound subsystem.
    TODO: Seriously consider to simplify this by connecting to the datamux
    directly. We don't do anything reasonable here.
*/

WRITE8_MEMBER( ti_sound_system_device::write )
{
	if (space.debugger_access()) return;
	m_sound_chip->write(space, 0, data);
}

void ti_sound_system_device::device_start(void)
{
	m_console_ready.resolve();
	m_sound_chip = subdevice<sn76496_base_device>(TISOUNDCHIP_TAG);
}

WRITE_LINE_MEMBER( ti_sound_system_device::sound_ready )
{
	m_console_ready(state);
}

MACHINE_CONFIG_FRAGMENT( sn94624 )
	MCFG_SPEAKER_STANDARD_MONO("sound_out")

	MCFG_SOUND_ADD(TISOUNDCHIP_TAG, SN94624, 3579545/8) /* 3.579545 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sound_out", 0.75)
	MCFG_SN76496_READY_HANDLER(WRITELINE(ti_sound_system_device, sound_ready))
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( sn76496 )
	MCFG_SPEAKER_STANDARD_MONO("sound_out")

	MCFG_SOUND_ADD(TISOUNDCHIP_TAG, SN76496, 3579545)   /* 3.579545 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sound_out", 0.75)
	MCFG_SN76496_READY_HANDLER(WRITELINE(ti_sound_system_device, sound_ready))
MACHINE_CONFIG_END

machine_config_constructor ti_sound_sn94624_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sn94624 );
}

machine_config_constructor ti_sound_sn76496_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sn76496 );
}

/**************************************************************************/

const device_type TI99VIDEO = &device_creator<ti_std_video_device>;
const device_type V9938VIDEO = &device_creator<ti_exp_video_device>;
const device_type TISOUND_94624 = &device_creator<ti_sound_sn94624_device>;
const device_type TISOUND_76496 = &device_creator<ti_sound_sn76496_device>;
