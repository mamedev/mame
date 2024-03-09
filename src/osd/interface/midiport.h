// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    midiport.h

    OSD interface to midi ports

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_MIDIPORT_H
#define MAME_OSD_INTERFACE_MIDIPORT_H

#pragma once

#include <cstdint>


namespace osd {

class midi_input_port
{
public:
	virtual ~midi_input_port() = default;

	virtual bool poll() = 0;
	virtual int read(uint8_t *pOut) = 0;
};


class midi_output_port
{
public:
	virtual ~midi_output_port() = default;

	virtual void write(uint8_t data) = 0;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_MIDIPORT_H
