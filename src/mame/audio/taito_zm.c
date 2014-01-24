/***************************************************************************

    Taito Zoom ZSG-2 sound board
    Includes: MN10200 CPU, ZOOM ZSG-2 audio chip, TMS57002 DASP
    By Olivier Galibert.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

TODO:
- add TMS57002
- a lot more

***************************************************************************/

#include "emu.h"
#include "taito_zm.h"

/**************************************************************************/

const device_type TAITO_ZOOM = &device_creator<taito_zoom_device>;

taito_zoom_device::taito_zoom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TAITO_ZOOM, "Taito Zoom Sound System", tag, owner, clock, "taito_zoom", __FILE__),
	m_tms_ctrl(0)
{
	memset(m_snd_shared_ram, 0, 0x100);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito_zoom_device::device_start()
{
	// register for savestates
	save_item(NAME(m_tms_ctrl));
	save_item(NAME(m_snd_shared_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taito_zoom_device::device_reset()
{
}


/***************************************************************************

  MN10200 I/O and Memory Map

***************************************************************************/

READ8_MEMBER(taito_zoom_device::shared_ram_r)
{
	return m_snd_shared_ram[offset];
}

WRITE8_MEMBER(taito_zoom_device::shared_ram_w)
{
	m_snd_shared_ram[offset] = data;
}


READ8_MEMBER(taito_zoom_device::tms_ctrl_r)
{
	return m_tms_ctrl;
}

WRITE8_MEMBER(taito_zoom_device::tms_ctrl_w)
{
#if 0
	tms57002_reset_w(data & 4);
	tms57002_cload_w(data & 2);
	tms57002_pload_w(data & 1);
#endif

	m_tms_ctrl = data;
}


static ADDRESS_MAP_START(taitozoom_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("mn10200", 0)
	AM_RANGE(0x400000, 0x40ffff) AM_RAM
	AM_RANGE(0x800000, 0x800fff) AM_DEVREADWRITE("zsg2", zsg2_device, zsg2_r, zsg2_w)
	AM_RANGE(0xe00000, 0xe000ff) AM_DEVREADWRITE8("taito_zoom", taito_zoom_device, shared_ram_r, shared_ram_w, 0xffff) // // M66220FP for comms with maincpu
	AM_RANGE(0xc00000, 0xc00001) AM_RAM // TMS57002 comms
ADDRESS_MAP_END

static ADDRESS_MAP_START(taitozoom_io_map, AS_IO, 8, driver_device )
	AM_RANGE(MN10200_PORT1, MN10200_PORT1) AM_DEVREADWRITE("taito_zoom", taito_zoom_device, tms_ctrl_r, tms_ctrl_w)
ADDRESS_MAP_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const zsg2_interface zsg2_taito_config =
{
	"zsg2"  /* sample region */
};

MACHINE_CONFIG_FRAGMENT( taito_zoom_sound )
	MCFG_TAITO_ZOOM_ADD("taito_zoom")
	MCFG_CPU_ADD("mn10200", MN10200, XTAL_25MHz/2)
	MCFG_CPU_PROGRAM_MAP(taitozoom_map)
	MCFG_CPU_IO_MAP(taitozoom_io_map)
//  MCFG_CPU_VBLANK_INT("screen", irq0_line_pulse)

	// we assume the parent machine has created lspeaker/rspeaker
	MCFG_ZSG2_ADD("zsg2", XTAL_25MHz/2)
	MCFG_SOUND_CONFIG(zsg2_taito_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END
