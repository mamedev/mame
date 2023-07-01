// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    machine/z80bin.h

    Quickload code for Z80 bin format

*********************************************************************/

#ifndef MAME_SHARED_Z80BIN_H
#define MAME_SHARED_Z80BIN_H

#pragma once

#include <cstdint>
#include <string>
#include <system_error>
#include <utility>


class snapshot_image_device;

std::pair<std::error_condition, std::string> z80bin_load_file(snapshot_image_device &image, address_space &space, uint16_t &exec_addr, uint16_t &start_addr, uint16_t &end_addr);

#endif // MAME_SHARED_Z80BIN_H
