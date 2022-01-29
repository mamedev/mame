// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    papertape.cpp

    Base classes for paper tape reader and punch devices.

    TODO: move some actual implementation here

*********************************************************************/

#include "emu.h"
#include "papertape.h"

#include "softlist_dev.h"


paper_tape_image_device::paper_tape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

paper_tape_reader_device::paper_tape_reader_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: paper_tape_image_device(mconfig, type, tag, owner, clock)
{
}

paper_tape_punch_device::paper_tape_punch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: paper_tape_image_device(mconfig, type, tag, owner, clock)
{
}

const software_list_loader &paper_tape_reader_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}
