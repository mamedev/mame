// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    machine/z80bin.h

    Quickload code for Z80 bin format

*********************************************************************/

#pragma once

#ifndef __Z80_BIN__
#define __Z80_BIN__

int z80bin_load_file(device_image_interface *image, address_space &space, const char *file_type, UINT16 *exec_addr, UINT16 *start_addr, UINT16 *end_addr );

#endif
