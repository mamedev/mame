/***************************************************************************

    image.h

    Core image interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "ioprocs.h"

void image_init(running_machine *machine);
void image_postdevice_init(running_machine *machine);

extern struct io_procs image_ioprocs;

void image_battery_load_by_name(const char *filename, void *buffer, int length, int fill);
void image_battery_save_by_name(const char *filename, const void *buffer, int length);

astring *image_info_astring(running_machine *machine, astring *string);

device_image_interface *image_from_absolute_index(running_machine *machine, int absolute_index);

/* extension list handling */
int image_find_extension(const char *extensions, const char *ext);
void image_specify_extension(char *buffer, size_t buffer_len, const char *extension);

#endif /* __IMAGE_H__ */
