// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    macbin.c

    MacBinary filter for use with Mac and ProDOS drivers

*****************************************************************************

  MacBinary file format

  Offset  Length  Description
  ------  ------  -----------
       0       1  [I]   Magic byte (0x00)
       1      64  [I]   File name (Pascal String)
      65       4  [I]   File Type Code
      69       4  [I]   File Creator Code
      73       1  [I]   Finder Flags (bits 15-8)
      74       1  [I]   Magic byte (0x00)
      75       2  [I]   File Vertical Position
      77       2  [I]   File Horizontal Position
      79       2  [I]   Window/Folder ID
      81       1  [I]   Protected (bit 0)
      82       1  [I]   Magic byte (0x00)
      83       4  [I]   Data Fork Length
      87       4  [I]   Resource Fork Length
      91       4  [I]   Creation Date
      95       4  [I]   Last Modified Date
      99       2  [I]   "Get Info" comment length
     101       1  [II]  Finder Flags (bits 7-0)
     102       4  [III] MacBinary III Signature 'mBIN'
     106       1  [III] Script of Filename
     107       1  [III] Extended Finder Flags
     116       4  [II]  Unpacked Length
     120       2  [II]  Secondary Header Length
     122       1  [II]  MacBinary II Version Number (II: 0x81, III: 0x82)
     123       1  [II]  Minimum Compatible MacBinary II Version Number (0x81)
     124       2  [II]  CRC of previous 124 bytes

    For more information, consult http://www.lazerware.com/formats/macbinary.html

    TODO: I believe that the script code is some sort of identifier identifying
    the character set used for the filename.  If this is true, we are not
    handling that properly

*****************************************************************************/

#include <string.h>

#include "imgtool.h"
#include "formats/imageutl.h"
#include "macutil.h"



static UINT32 pad128(UINT32 length)
{
	if (length % 128)
		length += 128 - (length % 128);
	return length;
}



static imgtoolerr_t macbinary_readfile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf)
{
	static const UINT32 attrs[] =
	{
		IMGTOOLATTR_TIME_CREATED,
		IMGTOOLATTR_TIME_LASTMODIFIED,
		IMGTOOLATTR_INT_MAC_TYPE,
		IMGTOOLATTR_INT_MAC_CREATOR,
		IMGTOOLATTR_INT_MAC_FINDERFLAGS,
		IMGTOOLATTR_INT_MAC_COORDX,
		IMGTOOLATTR_INT_MAC_COORDY,
		IMGTOOLATTR_INT_MAC_FINDERFOLDER,
		IMGTOOLATTR_INT_MAC_SCRIPTCODE,
		IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS,
		0
	};
	imgtoolerr_t err;
	UINT8 header[128];
	const char *basename;
	int i;

	UINT32 type_code = 0x3F3F3F3F;
	UINT32 creator_code = 0x3F3F3F3F;
	UINT16 finder_flags = 0;
	UINT16 coord_x = 0;
	UINT16 coord_y = 0;
	UINT16 finder_folder = 0;
	UINT8 script_code = 0;
	UINT8 extended_flags = 0;

	imgtool_forkent fork_entries[4];
	const imgtool_forkent *data_fork = NULL;
	const imgtool_forkent *resource_fork = NULL;
	UINT32 creation_time = 0;
	UINT32 lastmodified_time = 0;
	imgtool_attribute attr_values[10];

	/* get the forks */
	err = imgtool_partition_list_file_forks(partition, filename, fork_entries, sizeof(fork_entries));
	if (err)
		return err;
	for (i = 0; fork_entries[i].type != FORK_END; i++)
	{
		if (fork_entries[i].type == FORK_DATA)
			data_fork = &fork_entries[i];
		else if (fork_entries[i].type == FORK_RESOURCE)
			resource_fork = &fork_entries[i];
	}

	/* get the attributes */
	err = imgtool_partition_get_file_attributes(partition, filename, attrs, attr_values);
	if (err && (ERRORCODE(err) != IMGTOOLERR_UNIMPLEMENTED))
		return err;
	if (err == IMGTOOLERR_SUCCESS)
	{
		creation_time     = mac_setup_time(attr_values[0].t);
		lastmodified_time = mac_setup_time(attr_values[1].t);
		type_code         = attr_values[2].i;
		creator_code      = attr_values[3].i;
		finder_flags      = attr_values[4].i;
		coord_x           = attr_values[5].i;
		coord_y           = attr_values[6].i;
		finder_folder     = attr_values[7].i;
		script_code       = attr_values[8].i;
		extended_flags    = attr_values[9].i;
	}

	memset(header, 0, sizeof(header));

	/* place filename */
	basename = filename;
	while(basename[strlen(basename) + 1])
		basename += strlen(basename) + 1;
	pascal_from_c_string((unsigned char *) &header[1], 64, basename);

	place_integer_be(header,  65, 4, type_code);
	place_integer_be(header,  69, 4, creator_code);
	place_integer_be(header,  73, 1, (finder_flags >> 8) & 0xFF);
	place_integer_be(header,  75, 2, coord_x);
	place_integer_be(header,  77, 2, coord_y);
	place_integer_be(header,  79, 2, finder_folder);
	place_integer_be(header,  83, 4, data_fork ? data_fork->size : 0);
	place_integer_be(header,  87, 4, resource_fork ? resource_fork->size : 0);
	place_integer_be(header,  91, 4, creation_time);
	place_integer_be(header,  95, 4, lastmodified_time);
	place_integer_be(header, 101, 1, (finder_flags >> 0) & 0xFF);
	place_integer_be(header, 102, 4, 0x6D42494E);
	place_integer_be(header, 106, 1, script_code);
	place_integer_be(header, 107, 1, extended_flags);
	place_integer_be(header, 122, 1, 0x82);
	place_integer_be(header, 123, 1, 0x81);
	place_integer_be(header, 124, 2, ccitt_crc16(0, header, 124));

	stream_write(destf, header, sizeof(header));

	if (data_fork)
	{
		err = imgtool_partition_read_file(partition, filename, "", destf, NULL);
		if (err)
			return err;

		stream_fill(destf, 0, pad128(data_fork->size));
	}

	if (resource_fork)
	{
		err = imgtool_partition_read_file(partition, filename, "RESOURCE_FORK", destf, NULL);
		if (err)
			return err;

		stream_fill(destf, 0, pad128(resource_fork->size));
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t write_fork(imgtool_partition *partition, const char *filename, const char *fork,
	imgtool_stream *sourcef, UINT64 pos, UINT64 fork_len, option_resolution *opts)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	imgtool_stream *mem_stream = NULL;
	size_t len;

	if (fork_len > 0)
	{
		mem_stream = stream_open_mem(NULL, 0);
		if (!mem_stream)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto done;
		}

		stream_seek(sourcef, pos, SEEK_SET);
		len = stream_transfer(mem_stream, sourcef, fork_len);
		if (len < fork_len)
			stream_fill(mem_stream, 0, fork_len);

		stream_seek(mem_stream, 0, SEEK_SET);
		err = imgtool_partition_write_file(partition, filename, fork, mem_stream, opts, NULL);
		if (err)
			goto done;
	}

done:
	if (mem_stream)
		stream_close(mem_stream);
	return err;
}



static imgtoolerr_t macbinary_writefile(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts)
{
	static const UINT32 attrs[] =
	{
		IMGTOOLATTR_TIME_CREATED,
		IMGTOOLATTR_TIME_LASTMODIFIED,
		IMGTOOLATTR_INT_MAC_TYPE,
		IMGTOOLATTR_INT_MAC_CREATOR,
		IMGTOOLATTR_INT_MAC_FINDERFLAGS,
		IMGTOOLATTR_INT_MAC_COORDX,
		IMGTOOLATTR_INT_MAC_COORDY,
		IMGTOOLATTR_INT_MAC_FINDERFOLDER,
		IMGTOOLATTR_INT_MAC_SCRIPTCODE,
		IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS,
		0
	};
	imgtoolerr_t err;
	imgtool_image *image = imgtool_partition_image(partition);
	UINT8 header[128];
	UINT32 datafork_size;
	UINT32 resourcefork_size;
	UINT64 total_size;
	UINT32 creation_time;
	UINT32 lastmodified_time;
	//int version;
	imgtool_attribute attr_values[10];

	UINT32 type_code;
	UINT32 creator_code;
	UINT16 finder_flags;
	UINT16 coord_x;
	UINT16 coord_y;
	UINT16 finder_folder;
	UINT8 script_code = 0;
	UINT8 extended_flags = 0;

	/* read in the header */
	memset(header, 0, sizeof(header));
	stream_read(sourcef, header, sizeof(header));

	/* check magic and zero fill bytes */
	if (header[0] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;
	if (header[74] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;
	if (header[82] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;

	datafork_size = pick_integer_be(header, 83, 4);
	resourcefork_size = pick_integer_be(header, 87, 4);
	total_size = stream_size(sourcef);

	/* size of a MacBinary header is always 128 bytes */
	if (total_size - pad128(datafork_size) - pad128(resourcefork_size) != 128)
		return IMGTOOLERR_CORRUPTFILE;

	/* check filename length byte */
	if ((header[1] <= 0x00) || (header[1] > 0x3F))
		return IMGTOOLERR_CORRUPTFILE;

	/* check the CRC */
	if (pick_integer_be(header, 124, 2) != ccitt_crc16(0, header, 124))
	{
		/* the CRC does not match; this file is MacBinary I */
		//version = 1;
	}
	else if (pick_integer_be(header, 102, 4) != 0x6D42494E)
	{
		/* did not see 'mBIN'; this file is MacBinary II */
		if (header[122] < 0x81)
			return IMGTOOLERR_CORRUPTFILE;
		if (header[123] < 0x81)
			return IMGTOOLERR_CORRUPTFILE;
		//version = 2;
	}
	else
	{
		/* we did see 'mBIN'; this file is MacBinary III */
		if (header[122] < 0x82)
			return IMGTOOLERR_CORRUPTFILE;
		if (header[123] < 0x81)
			return IMGTOOLERR_CORRUPTFILE;
		//version = 3;
	}

	type_code         = pick_integer_be(header, 65, 4);
	creator_code      = pick_integer_be(header, 69, 4);
	finder_flags      = pick_integer_be(header, 73, 1) << 8;
	coord_x           = pick_integer_be(header, 75, 2);
	coord_y           = pick_integer_be(header, 77, 2);
	finder_folder     = pick_integer_be(header, 79, 2);
	creation_time     = pick_integer_be(header, 91, 4);
	lastmodified_time = pick_integer_be(header, 95, 4);

	if (image)
	{
		/* write out both forks */
		err = write_fork(partition, filename, "", sourcef, sizeof(header), datafork_size, opts);
		if (err)
			return err;
		err = write_fork(partition, filename, "RESOURCE_FORK", sourcef, sizeof(header) + pad128(datafork_size), resourcefork_size, opts);
		if (err)
			return err;

		/* set up attributes */
		attr_values[0].t = mac_crack_time(creation_time);
		attr_values[1].t = mac_crack_time(lastmodified_time);
		attr_values[2].i = type_code;
		attr_values[3].i = creator_code;
		attr_values[4].i = finder_flags;
		attr_values[5].i = coord_x;
		attr_values[6].i = coord_y;
		attr_values[7].i = finder_folder;
		attr_values[8].i = script_code;
		attr_values[9].i = extended_flags;

		err = imgtool_partition_put_file_attributes(partition, filename, attrs, attr_values);
		if (err)
			return err;
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t macbinary_checkstream(imgtool_stream *stream, imgtool_suggestion_viability_t *viability)
{
	imgtoolerr_t err;

	err = macbinary_writefile(NULL, NULL, NULL, stream, NULL);
	if (err == IMGTOOLERR_CORRUPTFILE)
	{
		/* the filter returned corrupt; this is not a valid file */
		*viability = SUGGESTION_END;
		err = IMGTOOLERR_SUCCESS;
	}
	else if (err == IMGTOOLERR_SUCCESS)
	{
		/* success; lets recommend this filter */
		*viability = SUGGESTION_RECOMMENDED;
	}
	return err;
}



void filter_macbinary_getinfo(UINT32 state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "macbinary"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "MacBinary"; break;
		case FILTINFO_STR_EXTENSION:    info->s = "bin"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = macbinary_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = macbinary_writefile; break;
		case FILTINFO_PTR_CHECKSTREAM:  info->check_stream = macbinary_checkstream; break;
	}
}
