// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    macbin.cpp

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

#include "imgtool.h"
#include "filter.h"

#include "macutil.h"

#include "formats/imageutl.h"

#include <cstring>



static uint32_t pad128(uint32_t length)
{
	if (length % 128)
		length += 128 - (length % 128);
	return length;
}



static imgtoolerr_t macbinary_readfile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	static const uint32_t attrs[] =
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
	uint8_t header[128];
	const char *basename;

	uint32_t type_code = 0x3F3F3F3F;
	uint32_t creator_code = 0x3F3F3F3F;
	uint16_t finder_flags = 0;
	uint16_t coord_x = 0;
	uint16_t coord_y = 0;
	uint16_t finder_folder = 0;
	uint8_t script_code = 0;
	uint8_t extended_flags = 0;

	uint32_t creation_time = 0;
	uint32_t lastmodified_time = 0;
	imgtool_attribute attr_values[10];

	// get the forks
	std::vector<imgtool::fork_entry> fork_entries;
	err = partition.list_file_forks(filename, fork_entries);
	if (err)
		return err;

	const imgtool::fork_entry *data_fork = nullptr;
	const imgtool::fork_entry *resource_fork = nullptr;
	for (const auto &entry : fork_entries)
	{
		switch (entry.type())
		{
		case imgtool::fork_entry::type_t::DATA:
			data_fork = &entry;
			break;
		case imgtool::fork_entry::type_t::RESOURCE:
			resource_fork = &entry;
			break;
		default:
			// do nothing
			break;
		}
	}

	/* get the attributes */
	err = partition.get_file_attributes(filename, attrs, attr_values);
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
	place_integer_be(header,  83, 4, data_fork ? data_fork->size() : 0);
	place_integer_be(header,  87, 4, resource_fork ? resource_fork->size() : 0);
	place_integer_be(header,  91, 4, creation_time);
	place_integer_be(header,  95, 4, lastmodified_time);
	place_integer_be(header, 101, 1, (finder_flags >> 0) & 0xFF);
	place_integer_be(header, 102, 4, 0x6D42494E);
	place_integer_be(header, 106, 1, script_code);
	place_integer_be(header, 107, 1, extended_flags);
	place_integer_be(header, 122, 1, 0x82);
	place_integer_be(header, 123, 1, 0x81);
	place_integer_be(header, 124, 2, ccitt_crc16(0, header, 124));

	destf.write(header, sizeof(header));

	if (data_fork)
	{
		err = partition.read_file(filename, "", destf, NULL);
		if (err)
			return err;

		destf.fill(0, pad128(data_fork->size()));
	}

	if (resource_fork)
	{
		err = partition.read_file(filename, "RESOURCE_FORK", destf, NULL);
		if (err)
			return err;

		destf.fill(0, pad128(resource_fork->size()));
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t write_fork(imgtool::partition &partition, const char *filename, const char *fork,
	imgtool::stream &sourcef, uint64_t pos, uint64_t fork_len, util::option_resolution *opts)
{
	imgtoolerr_t err;
	imgtool::stream::ptr mem_stream;
	size_t len;

	if (fork_len > 0)
	{
		mem_stream = imgtool::stream::open_mem(nullptr, 0);
		if (!mem_stream)
			return IMGTOOLERR_OUTOFMEMORY;

		sourcef.seek(pos, SEEK_SET);
		len = imgtool::stream::transfer(*mem_stream, sourcef, fork_len);
		if (len < fork_len)
			mem_stream->fill(0, fork_len);

		mem_stream->seek(0, SEEK_SET);
		err = partition.write_file(filename, fork, *mem_stream, opts, NULL);
		if (err)
			return err;
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t macbinary_writefile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	static const uint32_t attrs[] =
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
	imgtool::image *image = &partition.image();
	uint8_t header[128];
	uint32_t datafork_size;
	uint32_t resourcefork_size;
	uint64_t total_size;
	uint32_t creation_time;
	uint32_t lastmodified_time;
	//int version;
	imgtool_attribute attr_values[10];

	uint32_t type_code;
	uint32_t creator_code;
	uint16_t finder_flags;
	uint16_t coord_x;
	uint16_t coord_y;
	uint16_t finder_folder;
	uint8_t script_code = 0;
	uint8_t extended_flags = 0;

	/* read in the header */
	memset(header, 0, sizeof(header));
	sourcef.read(header, sizeof(header));

	/* check magic and zero fill bytes */
	if (header[0] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;
	if (header[74] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;
	if (header[82] != 0x00)
		return IMGTOOLERR_CORRUPTFILE;

	datafork_size = pick_integer_be(header, 83, 4);
	resourcefork_size = pick_integer_be(header, 87, 4);
	total_size = sourcef.size();

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
		attr_values[0].t = mac_crack_time(creation_time).to_time_t();
		attr_values[1].t = mac_crack_time(lastmodified_time).to_time_t();
		attr_values[2].i = type_code;
		attr_values[3].i = creator_code;
		attr_values[4].i = finder_flags;
		attr_values[5].i = coord_x;
		attr_values[6].i = coord_y;
		attr_values[7].i = finder_folder;
		attr_values[8].i = script_code;
		attr_values[9].i = extended_flags;

		err = partition.put_file_attributes(filename, attrs, attr_values);
		if (err)
			return err;
	}

	return IMGTOOLERR_SUCCESS;
}


// this was completely broken - it was calling macbinary_writefile() with a nullptr partition
#if 0
static imgtoolerr_t macbinary_checkstream(imgtool::stream &stream, imgtool_suggestion_viability_t *viability)
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
#endif



void filter_macbinary_getinfo(uint32_t state, union filterinfo *info)
{
	switch(state)
	{
		case FILTINFO_STR_NAME:         info->s = "macbinary"; break;
		case FILTINFO_STR_HUMANNAME:    info->s = "MacBinary"; break;
		case FILTINFO_STR_EXTENSION:    info->s = "bin"; break;
		case FILTINFO_PTR_READFILE:     info->read_file = macbinary_readfile; break;
		case FILTINFO_PTR_WRITEFILE:    info->write_file = macbinary_writefile; break;
		//case FILTINFO_PTR_CHECKSTREAM:  info->check_stream = macbinary_checkstream; break;
	}
}
