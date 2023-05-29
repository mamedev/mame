// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputfwd.h

    Forward declarations for OSD input interface types

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_INPUTFWD_H
#define MAME_OSD_INTERFACE_INPUTFWD_H

#include <cstdint>

#pragma once


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

enum ioport_type : std::uint32_t;
enum input_seq_type : int;

class input_code;


namespace osd {

class input_device;
class input_manager;
class input_seq;

} // namespace osd

#endif // MAME_OSD_INTERFACE_INPUTFWD_H
