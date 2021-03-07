// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include <cstdio>
#include <cstring>
#include <cassert>
#include "ioprocs.h"
#include "corefile.h"


/*********************************************************************
    ioprocs implementation on stdio
*********************************************************************/

static void stdio_closeproc(void *file)
{
	fclose((FILE*)file);
}

static int stdio_seekproc(void *file, int64_t offset, int whence)
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

static uint64_t stdio_filesizeproc(void *file)
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
	delete (util::core_file*)file;
}

static int corefile_seekproc(void *file, int64_t offset, int whence)
{
	return ((util::core_file*)file)->seek(offset, whence);
}

static size_t corefile_readproc(void *file, void *buffer, size_t length)
{
	return ((util::core_file*)file)->read(buffer, length);
}

static size_t corefile_writeproc(void *file, const void *buffer, size_t length)
{
	return ((util::core_file*)file)->write(buffer, length);
}

static uint64_t corefile_filesizeproc(void *file)
{
	const auto l = ((util::core_file*)file)->tell();
	if (((util::core_file*)file)->seek(0, SEEK_END))
		return (size_t) -1;
	const auto sz = ((util::core_file*)file)->tell();
	if (((util::core_file*)file)->seek(l, SEEK_SET))
		return uint64_t(-1);
	return uint64_t(sz);
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

static void io_generic_seek(struct io_generic *genio, uint64_t offset)
{
	genio->procs->seekproc(genio->file, offset, SEEK_SET);
}



void io_generic_close(struct io_generic *genio)
{
	if (genio->procs->closeproc)
		genio->procs->closeproc(genio->file);
}



void io_generic_read(struct io_generic *genio, void *buffer, uint64_t offset, size_t length)
{
	uint64_t size;
	size_t bytes_read;

	size = io_generic_size(genio);
	if (size <= offset)
	{
		bytes_read = 0;
	}
	else
	{
		io_generic_seek(genio, offset);
		bytes_read = genio->procs->readproc(genio->file, buffer, length);
	}
	memset(((uint8_t *) buffer) + bytes_read, genio->filler, length - bytes_read);
}



void io_generic_write(struct io_generic *genio, const void *buffer, uint64_t offset, size_t length)
{
	uint64_t filler_size = 0;
	char filler_buffer[1024];
	size_t bytes_to_write;
	uint64_t size;

	size = io_generic_size(genio);

	if (size < offset)
	{
		filler_size = offset - size;
		offset = size;
	}

	io_generic_seek(genio, offset);

	if (filler_size)
	{
		memset(filler_buffer, genio->filler, sizeof(filler_buffer));
		do
		{
			bytes_to_write = (filler_size > sizeof(filler_buffer)) ? sizeof(filler_buffer) : (size_t) filler_size;
			genio->procs->writeproc(genio->file, filler_buffer, bytes_to_write);
			filler_size -= bytes_to_write;
		}
		while(filler_size > 0);
	}

	if (length > 0)
		genio->procs->writeproc(genio->file, buffer, length);
}



void io_generic_write_filler(struct io_generic *genio, uint8_t filler, uint64_t offset, size_t length)
{
	uint8_t buffer[512];
	size_t this_length;

	memset(buffer, filler, std::min(length, sizeof(buffer)));

	while(length > 0)
	{
		this_length = std::min(length, sizeof(buffer));
		io_generic_write(genio, buffer, offset, this_length);
		offset += this_length;
		length -= this_length;
	}
}



uint64_t io_generic_size(struct io_generic *genio)
{
	return genio->procs->filesizeproc(genio->file);
}
