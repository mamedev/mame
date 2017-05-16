// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    machine/z80bin.h

    Quickload code for Z80 bin format

*********************************************************************/

#ifndef MAME_MACHINE_Z80BIN_H
#define MAME_MACHINE_Z80BIN_H

#pragma once

image_init_result z80bin_load_file(device_image_interface *image, address_space &space, const char *file_type, uint16_t *exec_addr, uint16_t *start_addr, uint16_t *end_addr);

#endif // MAME_MACHINE_Z80BIN_H
