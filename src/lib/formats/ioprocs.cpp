// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "osdcore.h"
#include "ioprocs.h"
#include "corefile.h"


/*********************************************************************
    ioprocs implementation on stdio
*********************************************************************/

static void stdio_closeproc(void *file)
{
	fclose((FILE*)file);
}

static int stdio_seekproc(void *file, INT64 offset, int whence)
{
	return fseek((FILE*)file, (long) offset, whence);
}

static size_t stdio_readproc(void *file, void *buffer, size_t length)
{
	return fread(buffer, 1, length, (FILE*)file);
}

static size_t stdio_writeproc(void *file, const void *buffer, size_t length)
{
	return fwrite(buffer, 1, length, (FILE*)file);
}

static UINT64 stdio_filesizeproc(void *file)
{
	long l, sz;
	l = ftell((FILE*)file);
	if (fseek((FILE*)file, 0, SEEK_END))
		return (size_t) -1;
	sz = ftell((FILE*)file);
	if (fseek((FILE*)file, l, SEEK_SET))
		return (size_t) -1;
	return (size_t) sz;
}

const struct io_procs stdio_ioprocs =
{
	stdio_closeproc,
	stdio_seekproc,
	stdio_readproc,
	stdio_writeproc,
	stdio_filesizeproc
};

const struct io_procs stdio_ioprocs_noclose =
{
	nullptr,
	stdio_seekproc,
	stdio_readproc,
	stdio_writeproc,
	stdio_filesizeproc
};

/*********************************************************************
    ioprocs implementation on corefile
*********************************************************************/

static void corefile_closeproc(void *file)
{
	core_fclose((core_file*)file);
}

static int corefile_seekproc(void *file, INT64 offset, int whence)
{
	return core_fseek((core_file*)file, (long) offset, whence);
}

static size_t corefile_readproc(void *file, void *buffer, size_t length)
{
	return core_fread((core_file*)file, buffer, length);
}

static size_t corefile_writeproc(void *file, const void *buffer, size_t length)
{
	return core_fwrite((core_file*)file, buffer, length);
}

static UINT64 corefile_filesizeproc(void *file)
{
	long l, sz;
	l = core_ftell((core_file*)file);
	if (core_fseek((core_file*)file, 0, SEEK_END))
		return (size_t) -1;
	sz = core_ftell((core_file*)file);
	if (core_fseek((core_file*)file, l, SEEK_SET))
		return (size_t) -1;
	return (size_t) sz;
}

const struct io_procs corefile_ioprocs =
{
	corefile_closeproc,
	corefile_seekproc,
	corefile_readproc,
	corefile_writeproc,
	corefile_filesizeproc
};

const struct io_procs corefile_ioprocs_noclose =
{
	nullptr,
	corefile_seekproc,
	corefile_readproc,
	corefile_writeproc,
	corefile_filesizeproc
};



/*********************************************************************
    calls for accessing generic IO
*********************************************************************/

static void io_generic_seek(struct io_generic *generic, UINT64 offset)
{
	generic->procs->seekproc(generic->file, offset, SEEK_SET);
}



void io_generic_close(struct io_generic *generic)
{
	if (generic->procs->closeproc)
		generic->procs->closeproc(generic->file);
}



void io_generic_read(struct io_generic *generic, void *buffer, UINT64 offset, size_t length)
{
	UINT64 size;
	size_t bytes_read;

	size = io_generic_size(generic);
	if (size <= offset)
	{
		bytes_read = 0;
	}
	else
	{
		io_generic_seek(generic, offset);
		bytes_read = generic->procs->readproc(generic->file, buffer, length);
	}
	memset(((UINT8 *) buffer) + bytes_read, generic->filler, length - bytes_read);
}



void io_generic_write(struct io_generic *generic, const void *buffer, UINT64 offset, size_t length)
{
	UINT64 filler_size = 0;
	char filler_buffer[1024];
	size_t bytes_to_write;
	UINT64 size;

	size = io_generic_size(generic);

	if (size < offset)
	{
		filler_size = offset - size;
		offset = size;
	}

	io_generic_seek(generic, offset);

	if (filler_size)
	{
		memset(filler_buffer, generic->filler, sizeof(filler_buffer));
		do
		{
			bytes_to_write = (filler_size > sizeof(filler_buffer)) ? sizeof(filler_buffer) : (size_t) filler_size;
			generic->procs->writeproc(generic->file, filler_buffer, bytes_to_write);
			filler_size -= bytes_to_write;
		}
		while(filler_size > 0);
	}

	if (length > 0)
		generic->procs->writeproc(generic->file, buffer, length);
}



void io_generic_write_filler(struct io_generic *generic, UINT8 filler, UINT64 offset, size_t length)
{
	UINT8 buffer[512];
	size_t this_length;

	memset(buffer, filler, MIN(length, sizeof(buffer)));

	while(length > 0)
	{
		this_length = MIN(length, sizeof(buffer));
		io_generic_write(generic, buffer, offset, this_length);
		offset += this_length;
		length -= this_length;
	}
}



UINT64 io_generic_size(struct io_generic *generic)
{
	return generic->procs->filesizeproc(generic->file);
}
