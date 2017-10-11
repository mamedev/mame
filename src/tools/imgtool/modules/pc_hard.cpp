// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    pc_hard.c

    PC hard drives

*****************************************************************************

  Master boot record format:

  Offset  Length  Description
  ------  ------  -----------
       0     446  Boot machine code
     446      16  Partion #1 info
     462      16  Partion #2 info
     478      16  Partion #3 info
     494      16  Partion #4 info
     510       2  Magic bytes (0x55 0xAA)


  Partition info format:

  Offset  Length  Description
  ------  ------  -----------
       0       1  Bootable (0x80=bootable 0x00=not bootable)
       1       1  Starting head
       2       1  Starting sector (bits 5-0) and high bits of starting track (bits 6-5)
       3       1  Low bits of starting track
       4       1  Partition type:
                       0x00     Unused
                       0x?1     FAT12   (0-15 MB)
                       0x?2     XENIX
                       0x?4     FAT16   (16-32 MB)
                       0x?6     FAT16`  (32 MB-2 GB)
                       0x?7     HPFS or NTFS
                       0x?A     Boot Manager
                       0x?B     FAT32   (512 MB-2 TB)
                       0x?C     FAT32   (512 MB-2 TB LBA)
                       0x1?     OS/2 Boot manager/Win95 hidden
                       0xC?     DR-DOS secured partition
                       0xD?     Multiuser DOS secured partition
                       0xE?     SpeedStor extended partition
       5       1  Ending head
       6       1  Ending sector (bits 5-0) and high bits of ending track (bits 6-5)
       7       1  Low bits of ending track
       8       4  Sector index of beginning of partition
      12       4  Total sectors in partition

****************************************************************************/
#include "imgtool.h"
#include "formats/imageutl.h"
#include "imghd.h"

#define FAT_SECLEN  512

OPTION_GUIDE_START( pc_chd_create_optionguide )
	OPTION_INT('T', "cylinders",    "Cylinders" )
	OPTION_INT('H', "heads",        "Heads" )
	OPTION_INT('S', "sectors",      "Sectors" )
OPTION_GUIDE_END

static const char pc_chd_create_optionspec[] = "H1-[16]S1-[32]-63T10/20/30/40/50/60/70/80/90/[100]/110/120/130/140/150/160/170/180/190/200";

static const char fat8_string[8]  = { 'F', 'A', 'T', ' ', ' ', ' ', ' ', ' ' };
static const char fat12_string[8] = { 'F', 'A', 'T', '1', '2', ' ', ' ', ' ' };
static const char fat16_string[8] = { 'F', 'A', 'T', '1', '6', ' ', ' ', ' ' };
//static const char fat32_string[8] = { 'F', 'A', 'T', '3', '2', ' ', ' ', ' ' };

/* imports from fat.c */
extern void fat_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info);


struct pc_chd_image_info
{
	struct mess_hard_disk_file hard_disk;

	struct
	{
		unsigned int corrupt : 1;
		uint8_t partition_type;
		uint32_t fat_bits;
		uint32_t starting_track;
		uint32_t starting_head;
		uint32_t starting_sector;
		uint32_t ending_track;
		uint32_t ending_head;
		uint32_t ending_sector;
		uint32_t sector_index;
		uint32_t total_sectors;
	} partitions[4];
};



static pc_chd_image_info *pc_chd_get_image_info(imgtool::image &image)
{
	return (pc_chd_image_info *) image.extra_bytes();
}



static void pc_chd_locate_block(imgtool::image &image, uint64_t block, uint32_t *cylinder, uint32_t *head, uint32_t *sector)
{
	pc_chd_image_info *info;
	const hard_disk_info *hd_info;

	info = pc_chd_get_image_info(image);
	hd_info = imghd_get_header(&info->hard_disk);

	*sector = block % hd_info->sectors;
	*head = (block / hd_info->sectors) % hd_info->heads;
	*cylinder = block / hd_info->sectors / hd_info->heads;
}



static imgtoolerr_t pc_chd_partition_create(imgtool::image &image, int partition_index, uint64_t first_block, uint64_t block_count)
{
	imgtoolerr_t err;
	uint8_t header_block[FAT_SECLEN];
	uint8_t partition_block[FAT_SECLEN];
	uint8_t partition_type;
	uint8_t *fat_type;
	uint8_t *partition_entry;
	uint32_t first_cylinder, first_head, first_sector;
	uint32_t last_cylinder, last_head, last_sector;
	imgtool_class imgclass = { fat_get_info };
	imgtoolerr_t (*fat_partition_create)(imgtool::image &image, uint64_t first_block, uint64_t block_count);

	/* sanity checks */
	assert((partition_index >= 0) && (partition_index <= 3));

	/* compute geometry */
	pc_chd_locate_block(image, first_block, &first_cylinder, &first_head, &first_sector);
	pc_chd_locate_block(image, first_block + block_count - 1, &last_cylinder, &last_head, &last_sector);

	/* load fat_partition_create */
	fat_partition_create = (imgtoolerr_t (*)(imgtool::image &, uint64_t, uint64_t))
		imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_CREATE_PARTITION);

	/* first create the actual partition */
	err = fat_partition_create(image, first_block, block_count);
	if (err)
		goto done;

	/* read the first block of the partition, to determine the type of FAT */
	err = image.read_block(first_block, partition_block);
	if (err)
		goto done;
	fat_type = &partition_block[54];
	if (!memcmp(fat_type, fat8_string, sizeof(fat8_string)))
		partition_type = 0x01;
	else if (!memcmp(fat_type, fat12_string, sizeof(fat12_string)))
		partition_type = 0x01;
	else if ((!memcmp(fat_type, fat16_string, sizeof(fat16_string))) && (block_count < 32*1024*1024/FAT_SECLEN))
		partition_type = 0x04;
	else if ((!memcmp(fat_type, fat16_string, sizeof(fat16_string))) && (block_count >= 32*1024*1024/FAT_SECLEN))
		partition_type = 0x06;
	else
		partition_type = 0x0B;

	/* read the partition header */
	err = image.read_block(0, header_block);
	if (err)
		goto done;

	/* fill out the partition entry */
	partition_entry = &header_block[446 + (partition_index * 16)];
	place_integer_le(partition_entry,  0, 1, 0x80);
	place_integer_le(partition_entry,  1, 1, first_head);
	place_integer_le(partition_entry,  2, 1, ((first_sector & 0x3F) | (first_cylinder >> 8 << 2)));
	place_integer_le(partition_entry,  3, 1, first_cylinder);
	place_integer_le(partition_entry,  4, 1, partition_type);
	place_integer_le(partition_entry,  5, 1, last_head);
	place_integer_le(partition_entry,  6, 1, ((last_sector & 0x3F) | (last_cylinder >> 8 << 2)));
	place_integer_le(partition_entry,  7, 1, last_cylinder);
	place_integer_le(partition_entry,  8, 4, first_block);
	place_integer_le(partition_entry, 12, 4, block_count);

	/* write the partition header */
	err = image.write_block(0, header_block);
	if (err)
		goto done;

done:
	return err;
}



static imgtoolerr_t pc_chd_read_partition_header(imgtool::image &image)
{
	imgtoolerr_t err;
	int i;
	const uint8_t *partition_info;
	pc_chd_image_info *info;
	uint8_t buffer[FAT_SECLEN];

	info = pc_chd_get_image_info(image);

	/* read the initial block */
	err = image.read_block(0, buffer);
	if (err)
		return err;

	/* magic bytes present? */
	if ((buffer[510] != 0x55) || (buffer[511] != 0xAA))
		return IMGTOOLERR_CORRUPTIMAGE;

	for (i = 0; i < ARRAY_LENGTH(info->partitions); i++)
	{
		partition_info = &buffer[446 + i * 16];

		info->partitions[i].partition_type  = partition_info[4];
		info->partitions[i].starting_head   = partition_info[1];
		info->partitions[i].starting_track  = ((partition_info[2] << 2) & 0xFF00) | partition_info[3];
		info->partitions[i].starting_sector = partition_info[2] & 0x3F;
		info->partitions[i].ending_head     = partition_info[5];
		info->partitions[i].ending_track    = ((partition_info[6] << 2) & 0xFF00) | partition_info[7];
		info->partitions[i].ending_sector   = partition_info[6] & 0x3F;

		info->partitions[i].sector_index    = pick_integer_le(partition_info,  8, 4);
		info->partitions[i].total_sectors   = pick_integer_le(partition_info, 12, 4);

		if (info->partitions[i].starting_track > info->partitions[i].ending_track)
			return IMGTOOLERR_CORRUPTIMAGE;
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t pc_chd_image_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
{
	imgtoolerr_t err;
	uint32_t cylinders, heads, sectors;
	pc_chd_image_info *info;
	uint8_t header_block[FAT_SECLEN];

	cylinders = opts->lookup_int('T');
	heads = opts->lookup_int('H');
	sectors = opts->lookup_int('S');

	info = pc_chd_get_image_info(image);

	/* create the hard disk image */
	err = imghd_create(*stream, 0, cylinders, heads, sectors, FAT_SECLEN);
	if (err)
		goto done;

	err = imghd_open(*stream, &info->hard_disk);
	if (err)
		goto done;

	/* set up partition header block */
	memset(header_block, 0, sizeof(header_block));
	header_block[510] = 0x55;
	header_block[511] = 0xAA;
	err = image.write_block(0, header_block);
	if (err)
		goto done;

	err = pc_chd_partition_create(image, 0, 1, cylinders * heads * sectors - 1);
	if (err)
		goto done;

	err = pc_chd_read_partition_header(image);
	if (err)
		goto done;

done:
	if (err)
		imghd_close(&info->hard_disk);
	return err;
}



static imgtoolerr_t pc_chd_image_open(imgtool::image &image, imgtool::stream::ptr &&stream)
{
	imgtoolerr_t err;
	pc_chd_image_info *info;

	info = pc_chd_get_image_info(image);

	/* open the hard drive */
	err = imghd_open(*stream, &info->hard_disk);
	if (err)
		return err;

	err = pc_chd_read_partition_header(image);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



static void pc_chd_image_close(imgtool::image &image)
{
	pc_chd_image_info *info;
	info = pc_chd_get_image_info(image);
	imghd_close(&info->hard_disk);
}



static imgtoolerr_t pc_chd_image_get_geometry(imgtool::image &image, uint32_t *tracks, uint32_t *heads, uint32_t *sectors)
{
	pc_chd_image_info *info;
	const hard_disk_info *hd_info;

	info = pc_chd_get_image_info(image);
	hd_info = imghd_get_header(&info->hard_disk);

	*tracks = hd_info->cylinders;
	*heads = hd_info->heads;
	*sectors = hd_info->sectors;
	return IMGTOOLERR_SUCCESS;
}



static uint32_t pc_chd_calc_lbasector(pc_chd_image_info &info, uint32_t track, uint32_t head, uint32_t sector)
{
	uint32_t lbasector;
	const hard_disk_info *hd_info;

	hd_info = imghd_get_header(&info.hard_disk);
	lbasector = track;
	lbasector *= hd_info->heads;
	lbasector += head;
	lbasector *= hd_info->sectors;
	lbasector += sector;
	return lbasector;
}



static imgtoolerr_t pc_chd_image_readsector(imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, std::vector<uint8_t> &buffer)
{
	pc_chd_image_info *info = pc_chd_get_image_info(image);

	// get the sector size and resize the buffer
	uint32_t sector_size = imghd_get_header(&info->hard_disk)->sectorbytes;
	try { buffer.resize(sector_size); }
	catch (std::bad_alloc const &) { return IMGTOOLERR_OUTOFMEMORY; }

	// read the data
	return imghd_read(&info->hard_disk,
		pc_chd_calc_lbasector(*info, track, head, sector),
		&buffer[0]);
}



static imgtoolerr_t pc_chd_image_writesector(imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, const void *buffer, size_t len, int ddam)
{
	pc_chd_image_info *info;
	info = pc_chd_get_image_info(image);
	return imghd_write(&info->hard_disk,
		pc_chd_calc_lbasector(*info, track, head, sector),
		buffer);
}



static imgtoolerr_t pc_chd_image_readblock(imgtool::image &image, void *buffer, uint64_t block)
{
	pc_chd_image_info *info;
	info = pc_chd_get_image_info(image);
	return imghd_read(&info->hard_disk, block, buffer);
}



static imgtoolerr_t pc_chd_image_writeblock(imgtool::image &image, const void *buffer, uint64_t block)
{
	pc_chd_image_info *info;
	info = pc_chd_get_image_info(image);
	return imghd_write(&info->hard_disk, block, buffer);
}



static imgtoolerr_t pc_chd_list_partitions(imgtool::image &image, std::vector<imgtool::partition_info> &partitions)
{
	pc_chd_image_info *info;
	size_t i;

	info = pc_chd_get_image_info(image);

	for (i = 0; i < 4; i++)
	{
		// what type of partition is this?
		imgtool_get_info partition_get_info;
		switch(info->partitions[i].partition_type)
		{
			case 0x00:  /* Empty Partition */
				partition_get_info = nullptr;
				break;

			case 0x01:  /* FAT12 */
			case 0x04:  /* FAT16 (-32 MB) */
			case 0x06:  /* FAT16 (32+ MB) */
			case 0x0B:  /* FAT32 */
			case 0x0C:  /* FAT32 (LBA Mapped) */
			case 0x0E:  /* FAT16 (LBA Mapped) */
			case 0x11:  /* OS/2 FAT12 */
			case 0x14:  /* OS/2 FAT16 (-32 MB) */
			case 0x16:  /* OS/2 FAT16 (32+ MB) */
			case 0x1B:  /* Hidden Win95 FAT32 */
			case 0x1C:  /* Hidden Win95 FAT32 (LBA Mapped) */
			case 0x1D:  /* Hidden Win95 FAT16 (LBA Mapped) */
			case 0xC1:  /* DR-DOS FAT12 */
			case 0xC4:  /* DR-DOS FAT16 (-32 MB) */
			case 0xC6:  /* DR-DOS FAT16 (32+ MB) */
			case 0xD1:  /* Old Multiuser DOS FAT12 */
			case 0xD4:  /* Old Multiuser DOS FAT16 (-32 MB) */
			case 0xD6:  /* Old Multiuser DOS FAT16 (32+ MB) */
				partition_get_info = fat_get_info;
				break;

			default:
				partition_get_info = unknown_partition_get_info;
				break;
		}

		partitions.emplace_back(
			partition_get_info,
			info->partitions[i].sector_index,
			info->partitions[i].total_sectors);
	}
	return IMGTOOLERR_SUCCESS;
}



void pc_chd_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_BLOCK_SIZE:                    info->i = FAT_SECLEN; break;
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(pc_chd_image_info); break;
		case IMGTOOLINFO_INT_TRACKS_ARE_CALLED_CYLINDERS:   info->i = 1; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "pc_chd"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "PC CHD disk image"); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "chd"); break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:           strcpy(info->s = imgtool_temp_str(), pc_chd_create_optionspec); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_CREATE:                        info->create = pc_chd_image_create; break;
		case IMGTOOLINFO_PTR_OPEN:                          info->open = pc_chd_image_open; break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close = pc_chd_image_close; break;
		case IMGTOOLINFO_PTR_READ_SECTOR:                   info->read_sector = pc_chd_image_readsector; break;
		case IMGTOOLINFO_PTR_WRITE_SECTOR:                  info->write_sector = pc_chd_image_writesector; break;
		case IMGTOOLINFO_PTR_READ_BLOCK:                    info->read_block = pc_chd_image_readblock; break;
		case IMGTOOLINFO_PTR_WRITE_BLOCK:                   info->write_block = pc_chd_image_writeblock; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:          info->createimage_optguide = &pc_chd_create_optionguide; break;
		case IMGTOOLINFO_PTR_GET_GEOMETRY:                  info->get_geometry = pc_chd_image_get_geometry; break;
		case IMGTOOLINFO_PTR_LIST_PARTITIONS:               info->list_partitions = pc_chd_list_partitions; break;
	}
}
