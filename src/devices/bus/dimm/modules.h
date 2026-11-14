// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    DIMM "Dual In-line Memory Module"

***************************************************************************/

#ifndef MAME_BUS_DIMM_MODULES_H
#define MAME_BUS_DIMM_MODULES_H

#pragma once


DECLARE_DEVICE_TYPE(DIMM_VANGUARD_32MB_PC100, spd_dimm_device)
DECLARE_DEVICE_TYPE(DIMM_MICRON_128MB_PC133, spd_dimm_device)
DECLARE_DEVICE_TYPE(DIMM_INFINEON_256MB_PC333, spd_dimm_device)


void sdr_sdram_modules(device_slot_interface &device);
void ddr_sdram_modules(device_slot_interface &device);


#endif // MAME_BUS_DIMM_MODULES_H
