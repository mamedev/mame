// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet
/*
    Code to interface the MESS image code with MAME's harddisk core.

    We do not support diff files as it will involve some changes in the MESS
    image code.

    Raphael Nabet 2003
*/

#include "imgtool.h"
#include "harddisk.h"
#include "imghd.h"


static imgtoolerr_t map_chd_error(chd_error chderr)
{
	imgtoolerr_t err;

	switch(chderr)
	{
		case CHDERR_NONE:
			err = IMGTOOLERR_SUCCESS;
			break;
		case CHDERR_OUT_OF_MEMORY:
			err = IMGTOOLERR_OUTOFMEMORY;
			break;
		case CHDERR_FILE_NOT_WRITEABLE:
			err = IMGTOOLERR_READONLY;
			break;
		case CHDERR_NOT_SUPPORTED:
			err = IMGTOOLERR_UNIMPLEMENTED;
			break;
		default:
			err = IMGTOOLERR_UNEXPECTED;
			break;
	}
	return err;
}



/*
    imghd_create()

    Create a MAME HD image
*/
imgtoolerr_t imghd_create(imgtool::stream &stream, uint32_t hunksize, uint32_t cylinders, uint32_t heads, uint32_t sectors, uint32_t seclen)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	chd_file chd;
	chd_error rc;
	chd_codec_type compression[4] = { CHD_CODEC_NONE };

	/* sanity check args -- see parse_hunk_size() in src/lib/util/chd.cpp */
	if (hunksize > (1024 * 1024))
	{
		err = IMGTOOLERR_PARAMCORRUPT;
		return err;
	}
	if (hunksize <= 0)
		hunksize = 4096;    /* default value */

	/* bail if we are read only */
	if (stream.is_read_only())
	{
		err = IMGTOOLERR_READONLY;
		return err;
	}

	/* calculations */
	const uint64_t logicalbytes = (uint64_t)cylinders * heads * sectors * seclen;

	/* create the new hard drive */
	rc = chd.create(*stream.core_file(), logicalbytes, hunksize, seclen, compression);
	if (rc != CHDERR_NONE)
	{
		err = map_chd_error(rc);
		return err;
	}

	/* write the metadata */
	const std::string metadata = util::string_format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, seclen);
	err = (imgtoolerr_t)chd.write_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	if (rc != CHDERR_NONE)
	{
		err = map_chd_error(rc);
		return err;
	}

	/* alloc and zero buffer */
	std::vector<uint8_t> cache;
	cache.resize(hunksize);
	memset(&cache[0], 0, hunksize);

	/* zero out every hunk */
	const int totalhunks = (logicalbytes + hunksize - 1) / hunksize;
	for (int hunknum = 0; hunknum < totalhunks; hunknum++)
	{
		rc = chd.write_units(hunknum, &cache[0]);
		if (rc)
		{
			err = IMGTOOLERR_WRITEERROR;
			return err;
		}
	}

	return err;
}



/*
    imghd_open()

    Open stream as a MAME HD image
*/
imgtoolerr_t imghd_open(imgtool::stream &stream, struct mess_hard_disk_file *hard_disk)
{
	chd_error chderr;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;

	hard_disk->hard_disk = nullptr;

	chderr = hard_disk->chd.open(*stream.core_file(), !stream.is_read_only());
	if (chderr)
	{
		err = map_chd_error(chderr);
		goto done;
	}

	hard_disk->hard_disk = hard_disk_open(&hard_disk->chd);
	if (!hard_disk->hard_disk)
	{
		err = IMGTOOLERR_UNEXPECTED;
		goto done;
	}
	hard_disk->stream = &stream;

done:
	if (err)
		imghd_close(hard_disk);
	return err;
}



/*
    imghd_close()

    Close MAME HD image
*/
void imghd_close(struct mess_hard_disk_file *disk)
{
	if (disk->hard_disk)
	{
		hard_disk_close(disk->hard_disk);
		disk->hard_disk = nullptr;
	}
	if (disk->stream)
	{
		delete disk->stream;
		disk->stream = nullptr;
	}
}



/*
    imghd_read()

    Read sector(s) from MAME HD image
*/
imgtoolerr_t imghd_read(struct mess_hard_disk_file *disk, uint32_t lbasector, void *buffer)
{
	uint32_t reply;
	reply = hard_disk_read(disk->hard_disk, lbasector, buffer);
	return (imgtoolerr_t)(reply ? IMGTOOLERR_SUCCESS : map_chd_error((chd_error)reply));
}



/*
    imghd_write()

    Write sector(s) from MAME HD image
*/
imgtoolerr_t imghd_write(struct mess_hard_disk_file *disk, uint32_t lbasector, const void *buffer)
{
	uint32_t reply;
	reply = hard_disk_write(disk->hard_disk, lbasector, buffer);
	return (imgtoolerr_t)(reply ? IMGTOOLERR_SUCCESS : map_chd_error((chd_error)reply));
}



/*
    imghd_get_header()

    Return pointer to the header of MAME HD image
*/
const hard_disk_info *imghd_get_header(struct mess_hard_disk_file *disk)
{
	const hard_disk_info *reply;
	reply = hard_disk_get_info(disk->hard_disk);
	return reply;
}


static imgtoolerr_t mess_hd_image_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *createoptions);

enum
{
	mess_hd_createopts_blocksize = 'B',
	mess_hd_createopts_cylinders = 'C',
	mess_hd_createopts_heads     = 'D',
	mess_hd_createopts_sectors   = 'E',
	mess_hd_createopts_seclen    = 'F'
};

OPTION_GUIDE_START( mess_hd_create_optionguide )
	OPTION_INT(mess_hd_createopts_blocksize, "blocksize", "Sectors Per Block" )
	OPTION_INT(mess_hd_createopts_cylinders, "cylinders", "Cylinders" )
	OPTION_INT(mess_hd_createopts_heads, "heads",   "Heads" )
	OPTION_INT(mess_hd_createopts_sectors, "sectors", "Total Sectors" )
	OPTION_INT(mess_hd_createopts_seclen, "seclen", "Sector Bytes" )
OPTION_GUIDE_END

#define mess_hd_create_optionspecs "B1-[4]-2048;C1-[32]-65536;D1-[8]-64;E1-[128]-4096;F128/256/[512]/1024/2048/4096/8192/16384/32768/65536"


void hd_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "mess_hd"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "MESS hard disk image"); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "hd"); break;

		case IMGTOOLINFO_PTR_CREATE:                        info->create = mess_hd_image_create; break;

		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:          info->createimage_optguide = &mess_hd_create_optionguide; break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:           strcpy(info->s = imgtool_temp_str(), mess_hd_create_optionspecs); break;
	}
}



static imgtoolerr_t mess_hd_image_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *createoptions)
{
	uint32_t  blocksize, cylinders, heads, sectors, seclen;

	/* read options */
	blocksize = createoptions->lookup_int(mess_hd_createopts_blocksize);
	cylinders = createoptions->lookup_int(mess_hd_createopts_cylinders);
	heads = createoptions->lookup_int(mess_hd_createopts_heads);
	sectors = createoptions->lookup_int(mess_hd_createopts_sectors);
	seclen = createoptions->lookup_int(mess_hd_createopts_seclen);

	return imghd_create(*stream.get(), blocksize * seclen, cylinders, heads, sectors, seclen);
}
