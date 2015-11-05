// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/***************************************************************************

    image.h

    Core image interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef __IMAGE_H__
#define __IMAGE_H__

void image_init(running_machine &machine);
void image_postdevice_init(running_machine &machine);
std::string &image_mandatory_scan(running_machine &machine, std::string &mandatory);

extern struct io_procs image_ioprocs;

void image_battery_load_by_name(emu_options &options, const char *filename, void *buffer, int length, int fill);
void image_battery_load_by_name(emu_options &options, const char *filename, void *buffer, int length, void *def_buffer);
void image_battery_save_by_name(emu_options &options, const char *filename, const void *buffer, int length);

#endif /* __IMAGE_H__ */
