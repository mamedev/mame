/***************************************************************************

    Taito Zoom ZSG-1 sound board
    Includes: MN10200 CPU, ZOOM ZSG-1 audio chip, TMS57002 DASP
    By Olivier Galibert.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "cpu/mn10200/mn10200.h"
#include "cpu/tms57002/tms57002.h"
#include "audio/taito_zm.h"

static ADDRESS_MAP_START(taitozoom_map, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("mn10200", 0)
	AM_RANGE(0x400000, 0x40ffff) AM_RAM
	AM_RANGE(0x800000, 0x800fff) AM_RAM	// Zoom ZSG-1
	AM_RANGE(0xe00000, 0xe000ff) AM_RAM	// main CPU comms?
	AM_RANGE(0xc00000, 0xc00001) AM_RAM	// TMS57002 comms
ADDRESS_MAP_END

static UINT8 tms_ctrl;

static READ8_HANDLER(tms_ctrl_r)
{
	return tms_ctrl;
}

static WRITE8_HANDLER(tms_ctrl_w)
{
#if 0
	tms57002_reset_w(data & 4);
	tms57002_cload_w(data & 2);
	tms57002_pload_w(data & 1);
#endif

	tms_ctrl = data;
}

static ADDRESS_MAP_START(taitozoom_io_map, ADDRESS_SPACE_IO, 8)
	AM_RANGE(MN10200_PORT1, MN10200_PORT1) AM_READWRITE(tms_ctrl_r, tms_ctrl_w)
ADDRESS_MAP_END

MACHINE_DRIVER_START( taito_zoom_sound )
	MDRV_CPU_ADD("mn10200", MN10200, 25000000/2)
	MDRV_CPU_PROGRAM_MAP(taitozoom_map)
	MDRV_CPU_IO_MAP(taitozoom_io_map)
MACHINE_DRIVER_END
