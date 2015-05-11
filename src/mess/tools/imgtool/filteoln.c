// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    filteoln.c

    Native end-of-line filter

*****************************************************************************/

#include <string.h>

#include "imgtool.h"

#define EOLN (CRLF == 1 ? "\r" : (CRLF == 2 ? "\n" : (CRLF == 3 ? "\r\n" : NULL)))



static imgtoolerr_t convert_stream_eolns(imgtool_stream *source, imgtool_stream *dest, const char *eoln)
{
	size_t len, i, pos;
	char buffer[2000];
	int hit_cr = FALSE;

	while((len = stream_read(source, buffer, sizeof(buffer))) > 0)
	{
		pos = 0;

		for (i = 0; i < len; i++)
		{
			switch(buffer[i])
			{
				case '\r':
				case '\n':
					if (!hit_cr || (buffer[i] != '\n'))
					{
						if (i > pos)
							stream_write(dest, buffer + pos, i - pos);
						stream_write(dest, eoln, strlen(eoln));
					}
					pos = i + 1;
					break;
			}
			hit_cr = (buffer[i] == '\r');
		}

		if (i > pos)
			stream_write(dest, buffer + pos, i - pos);
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t ascii_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf)
{
	imgtoolerr_t err;
	imgtool_stream *mem_stream;

	mem_stream = stream_open_mem(NULL, 0);
	if (!mem_stream)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}

	err = imgtool_partition_read_file(partition, filename, fork, mem_stream, NULL);
	if (err)
		goto done;

	stream_seek(mem_stream, SEEK_SET, 0);
	err = convert_stream_eolns(mem_stream, destf, EOLN);
	if (err)
		goto done;

done:
	if (mem_stream)
		stream_close(mem_stream);
	return err;
}



static imgtoolerr_t ascii_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts)
{
	imgtoolerr_t err;
	imgtool_stream *mem_stream = NULL;
	const char *eoln;

	/* create a stream */
	mem_stream = stream_open_mem(NULL, 0);
	if (!mem_stream)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}

	eoln = imgtool_partition_get_info_string(partition, IMGTOOLINFO_STR_EOLN);

	err = convert_stream_eolns(sourcef, mem_stream, eoln);
	if (err)
		goto done;
	stream_seek(mem_stream, SEEK_SET, 0);

	err = imgtool_partition_write_file(partition, filename, fork, mem_stream, opts, NULL);
	if (err)
		goto done;

done:
	if (mem_stream)
		stream_close(mem_stream);
	return err;
}



void filter_eoln_getinfo(UINT32 state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "ascii"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "Ascii"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = ascii_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = ascii_writefile; break;
	}
}
