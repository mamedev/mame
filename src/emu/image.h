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

void image_init(running_machine &machine);
void image_postdevice_init(running_machine &machine);

extern struct io_procs image_ioprocs;

void image_battery_load_by_name(emu_options &options, const char *filename, void *buffer, int length, int fill);
void image_battery_save_by_name(emu_options &options, const char *filename, const void *buffer, int length);

#endif /* __IMAGE_H__ */
