// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88_SDIP_H
#define MAME_NEC_PC88_SDIP_H

#pragma once

#include "machine/eepromser.h"


class pc88_sdip_device : public eeprom_serial_93c06_16bit_device
{
public:
	pc88_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	ioport_value dsw1_r();
	ioport_value dsw2_r();
	ioport_value auto_boot_floppy_r();
//	ioport_value built_in_fdd_r();
	ioport_value memory_weight_r();
};


// device type definition
DECLARE_DEVICE_TYPE(PC88_SDIP, pc88_sdip_device)

#endif // MAME_NEC_PC88_SDIP_H
