// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    pc_flop.c

    PC floppies

****************************************************************************/

#include "imgtool.h"
#include "formats/imageutl.h"
#include "formats/pc_dsk.h"
#include "fat.h"
#include "iflopimg.h"

#define FAT_SECLEN              512


static imgtoolerr_t fat_image_create(imgtool_image *image, imgtool_stream *stream, option_resolution *opts)
{
	imgtoolerr_t err;
	UINT32 tracks, heads, sectors;
	UINT8 buffer[FAT_SECLEN];
	imgtool_class imgclass = { fat_get_info };
	imgtoolerr_t (*fat_partition_create)(imgtool_image *image, UINT64 first_block, UINT64 block_count);

	tracks = option_resolution_lookup_int(opts, 'T');
	heads = option_resolution_lookup_int(opts, 'H');
	sectors = option_resolution_lookup_int(opts, 'S');

	/* set up just enough of a boot sector to specify geometry */
	memset(buffer, 0, sizeof(buffer));
	place_integer_le(buffer, 24, 2, sectors);
	place_integer_le(buffer, 26, 2, heads);
	place_integer_le(buffer, 19, 2, (UINT16) (((UINT64) tracks * heads * sectors) >> 0));
	place_integer_le(buffer, 32, 4, (UINT16) (((UINT64) tracks * heads * sectors) >> 16));
	err = imgtool_image_write_block(image, 0, buffer);
	if (err)
		goto done;

	/* load fat_partition_create */
	fat_partition_create = (imgtoolerr_t (*)(imgtool_image *, UINT64, UINT64))
		imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_CREATE_PARTITION);

	/* actually create the partition */
	err = fat_partition_create(image, 0, ((UINT64) tracks) * heads * sectors);
	if (err)
		goto done;

done:
	return err;
}



static imgtoolerr_t fat_image_get_geometry(imgtool_image *image, UINT32 *tracks, UINT32 *heads, UINT32 *sectors)
{
	imgtoolerr_t err;
	UINT64 total_sectors;
	UINT8 buffer[FAT_SECLEN];

	err = imgtool_image_read_block(image, 0, buffer);
	if (err)
		return err;

	total_sectors = pick_integer_le(buffer, 19, 2)
		| (pick_integer_le(buffer, 32, 4) << 16);

	*sectors = pick_integer_le(buffer, 24, 2);
	*heads = pick_integer_le(buffer, 26, 2);
	*tracks = total_sectors / *heads / *sectors;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_get_sector_position(imgtool_image *image, UINT32 sector_index,
	UINT32 *track, UINT32 *head, UINT32 *sector)
{
	imgtoolerr_t err;
	UINT32 tracks, heads, sectors;

	if (sector_index == 0)
	{
		/* special case */
		*head = 0;
		*track = 0;
		*sector = 1;
	}
	else
	{
		err = imgtool_image_get_geometry(image, &tracks, &heads, &sectors);
		if (err)
			return err;

		*track = sector_index / sectors / heads;
		*head = (sector_index / sectors) % heads;
		*sector = 1 + (sector_index % sectors);
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_image_readblock(imgtool_image *image, void *buffer, UINT64 block)
{
	imgtoolerr_t err;
	floperr_t ferr;
	UINT32 track, head, sector;
	UINT32 block_size;

	err = imgtool_image_get_block_size(image, &block_size);
	if (err)
		return err;

	err = fat_get_sector_position(image, block, &track, &head, &sector);
	if (err)
		return err;

	ferr = floppy_read_sector(imgtool_floppy(image), head, track, sector, 0, buffer, block_size);
	if (ferr)
		return imgtool_floppy_error(ferr);
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_image_writeblock(imgtool_image *image, const void *buffer, UINT64 block)
{
	imgtoolerr_t err;
	floperr_t ferr;
	UINT32 track, head, sector;
	UINT32 block_size;

	err = imgtool_image_get_block_size(image, &block_size);
	if (err)
		return err;

	err = fat_get_sector_position(image, block, &track, &head, &sector);
	if (err)
		return err;

	ferr = floppy_write_sector(imgtool_floppy(image), head, track, sector, 0, buffer, block_size, 0);   /* TODO: pass ddam argument from imgtool */
	if (ferr)
		return imgtool_floppy_error(ferr);
	return IMGTOOLERR_SUCCESS;
}



void pc_floppy_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_BLOCK_SIZE:                    info->i = FAT_SECLEN; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "fat"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "FAT format"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_FLOPPY_CREATE:                 info->create = fat_image_create; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_pc; break;
		case IMGTOOLINFO_PTR_READ_BLOCK:                    info->read_block = fat_image_readblock; break;
		case IMGTOOLINFO_PTR_WRITE_BLOCK:                   info->write_block = fat_image_writeblock; break;
		case IMGTOOLINFO_PTR_GET_GEOMETRY:                  info->get_geometry = fat_image_get_geometry; break;

		default: fat_get_info(imgclass, state, info); break;
	}
}
