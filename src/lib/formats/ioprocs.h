// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    ioprocs.h

    File IO abstraction layer

*********************************************************************/
#ifndef MAME_FORMATS_IOPROCS_H
#define MAME_FORMATS_IOPROCS_H

#pragma once

#include <stdint.h>
#include <stdlib.h>



/***************************************************************************

    Type definitions

***************************************************************************/

struct io_procs
{
	void (*closeproc)(void *file);
	int (*seekproc)(void *file, int64_t offset, int whence);
	size_t (*readproc)(void *file, void *buffer, size_t length);
	size_t (*writeproc)(void *file, const void *buffer, size_t length);
	uint64_t (*filesizeproc)(void *file);
};



struct io_generic
{
	const struct io_procs *procs;
	void *file;
	uint8_t filler;
};


/***************************************************************************

    Globals

***************************************************************************/

extern const struct io_procs stdio_ioprocs;
extern const struct io_procs stdio_ioprocs_noclose;
extern const struct io_procs corefile_ioprocs;
extern const struct io_procs corefile_ioprocs_noclose;



/***************************************************************************

    Prototypes

***************************************************************************/

void io_generic_close(struct io_generic *genio);
void io_generic_read(struct io_generic *genio, void *buffer, uint64_t offset, size_t length);
void io_generic_write(struct io_generic *genio, const void *buffer, uint64_t offset, size_t length);
void io_generic_write_filler(struct io_generic *genio, uint8_t filler, uint64_t offset, size_t length);
uint64_t io_generic_size(struct io_generic *genio);

#endif // MAME_FORMATS_IOPROCS_H
