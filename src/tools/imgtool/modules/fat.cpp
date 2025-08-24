// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    fat.cpp

    PC FAT disk images

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
       0       1  Active byte (0x80=active 0x00=inactive)
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


  Boot sector format:

  Offset  Length  Description
  ------  ------  -----------
       0       3  Jump instruction (to skip over header on boot)
       3       8  OEM Name
      11       2  Bytes per sector
      13       1  Sectors per cluster
      14       2  Reserved sector count (including boot sector)
      16       1  Number of FATs (file allocation tables)
      17       2  Number of root directory entries
      19       2  Total sectors (bits 0-15)
      21       1  Media descriptor
      22       2  Sectors per FAT
      24       2  Sectors per track
      26       2  Number of heads
      28       4  Hidden sectors
      32       4  Total sectors (bits 16-47)
      36       1  Physical drive number
      37       1  Current head
      38       1  Signature
      39       4  ID
      43      11  Volume Label
      54       8  FAT file system type
      62     448  Boot machine code
     510       2  Magic bytes (0x55 0xAA)

  For more information:
    http://support.microsoft.com/kb/q140418/


  Directory Entry Format:

  Offset  Length  Description
  ------  ------  -----------
       0       8  DOS File Name (padded with spaces)
       8       3  DOS File Extension (padded with spaces)
      11       1  File Attributes
      12       2  Unknown
      14       4  Time of Creation
      18       2  Last Access Time
      20       2  EA-Index (OS/2 stuff)
      22       4  Last Modified Time
      26       2  First Cluster
      28       4  File Size


  Dates and times are stored in separate words; when together, the time is
  first and the date is second.

    Time:
        bits 15-11      Hour
        bits 10- 5      Minute
        bits  4- 0      Second / 2

    Date:
        bits 15- 9      Year - 1980
        bits  8- 5      Month
        bits  4- 0      Day

  LFN Entry Format:

  Offset  Length  Description
  ------  ------  -----------
       0       1  Sequence Number (bit 6 is set on highest sequence)
       1      10  Name characters (five UTF-16LE chars)
      11       1  Attributes (always 0x0F)
      12       1  Reserved (always 0x00)
      13       1  Checksum of short filename entry
      14      12  Name characters (six UTF-16LE chars)
      26       2  Entry Cluster (always 0x00)
      28       4  Name characters (two UTF-16LE chars)

  Valid characters in DOS file names:
    - Upper case letters A-Z
    - Numbers 0-9
    - Space (though there is no way to identify a trailing space)
    - ! # $ % & ( ) - @ ^ _ ` { } ~
    - Characters 128-255 (though the code page is indeterminate)

  For more information:
    http://en.wikipedia.org/wiki/File_Allocation_Table

****************************************************************************/

#include "fat.h"

#include "corestr.h"
#include "multibyte.h"
#include "unicode.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>

#define FAT_DIRENT_SIZE         32
#define FAT_SECLEN              512

#define LOG(x)

struct fat_partition_info
{
	uint32_t fat_bits;
	uint32_t sectors_per_cluster;
	uint32_t cluster_size;
	uint32_t reserved_sectors;
	uint32_t fat_count;
	uint32_t root_entries;
	uint32_t sectors_per_fat;
	uint64_t total_sectors;
	uint32_t total_clusters;
};

struct fat_file
{
	unsigned int root : 1;
	unsigned int directory : 1;
	unsigned int eof : 1;
	uint32_t index;
	uint32_t filesize;
	uint32_t first_cluster;
	uint32_t parent_first_cluster;
	uint32_t cluster;
	uint32_t cluster_index;
	uint32_t dirent_sector_index;
	uint32_t dirent_sector_offset;
};

struct fat_dirent
{
	fat_dirent() :
		directory(0),
		eof(0),
		filesize(0),
		first_cluster(0),
		dirent_sector_index(0),
		dirent_sector_offset(0)
	{
		std::fill(std::begin(long_filename), std::end(long_filename), 0);
		std::fill(std::begin(short_filename), std::end(short_filename), 0);
	}

	char long_filename[512];
	char short_filename[13];
	unsigned int directory : 1;
	unsigned int eof : 1;
	uint32_t filesize;
	uint32_t first_cluster;
	uint32_t dirent_sector_index;
	uint32_t dirent_sector_offset;
	imgtool::datetime creation_time;
	imgtool::datetime lastmodified_time;
};

struct fat_freeentry_info
{
	uint32_t required_size;
	uint32_t candidate_position;
	uint32_t position;
};

struct fat_mediatype
{
	uint8_t media_descriptor;
	uint8_t heads;
	uint8_t tracks;
	uint8_t sectors;
};

enum creation_policy_t
{
	CREATE_NONE,
	CREATE_FILE,
	CREATE_DIR
};



static const fat_mediatype known_media[] =
{
	{ 0xF0, 2, 80, 36 },
	{ 0xF0, 2, 80, 18 },
	{ 0xF9, 2, 80,  9 },
	{ 0xF9, 2, 80, 15 },
	{ 0xFD, 2, 40,  9 },
	{ 0xFF, 2, 40,  8 },
	{ 0xFC, 1, 40,  9 },
	{ 0xFE, 1, 40,  8 },
	{ 0xF8, 0,  0,  0 }
};

/* boot sector code taken from FreeDOS */
static const uint8_t boot_sector_code[] =
{
	0xfa, 0xfc, 0x31, 0xc0, 0x8e, 0xd8, 0xbd, 0x00, 0x7c, 0xb8, 0xe0, 0x1f,
	0x8e, 0xc0, 0x89, 0xee, 0x89, 0xef, 0xb9, 0x00, 0x01, 0xf3, 0xa5, 0xea,
	0x5e, 0x7c, 0xe0, 0x1f, 0x00, 0x00, 0x60, 0x00, 0x8e, 0xd8, 0x8e, 0xd0,
	0x8d, 0x66, 0xa0, 0xfb, 0x80, 0x7e, 0x24, 0xff, 0x75, 0x03, 0x88, 0x56,
	0x24, 0xc7, 0x46, 0xc0, 0x10, 0x00, 0xc7, 0x46, 0xc2, 0x01, 0x00, 0x8b,
	0x76, 0x1c, 0x8b, 0x7e, 0x1e, 0x03, 0x76, 0x0e, 0x83, 0xd7, 0x00, 0x89,
	0x76, 0xd2, 0x89, 0x7e, 0xd4, 0x8a, 0x46, 0x10, 0x98, 0xf7, 0x66, 0x16,
	0x01, 0xc6, 0x11, 0xd7, 0x89, 0x76, 0xd6, 0x89, 0x7e, 0xd8, 0x8b, 0x5e,
	0x0b, 0xb1, 0x05, 0xd3, 0xeb, 0x8b, 0x46, 0x11, 0x31, 0xd2, 0xf7, 0xf3,
	0x89, 0x46, 0xd0, 0x01, 0xc6, 0x83, 0xd7, 0x00, 0x89, 0x76, 0xda, 0x89,
	0x7e, 0xdc, 0x8b, 0x46, 0xd6, 0x8b, 0x56, 0xd8, 0x8b, 0x7e, 0xd0, 0xc4,
	0x5e, 0x5a, 0xe8, 0xac, 0x00, 0xc4, 0x7e, 0x5a, 0xb9, 0x0b, 0x00, 0xbe,
	0xf1, 0x7d, 0x57, 0xf3, 0xa6, 0x5f, 0x26, 0x8b, 0x45, 0x1a, 0x74, 0x0b,
	0x83, 0xc7, 0x20, 0x26, 0x80, 0x3d, 0x00, 0x75, 0xe7, 0x72, 0x68, 0x50,
	0xc4, 0x5e, 0x5a, 0x8b, 0x7e, 0x16, 0x8b, 0x46, 0xd2, 0x8b, 0x56, 0xd4,
	0xe8, 0x7e, 0x00, 0x58, 0x1e, 0x07, 0x8e, 0x5e, 0x5c, 0xbf, 0x00, 0x20,
	0xab, 0x89, 0xc6, 0x01, 0xf6, 0x01, 0xc6, 0xd1, 0xee, 0xad, 0x73, 0x04,
	0xb1, 0x04, 0xd3, 0xe8, 0x80, 0xe4, 0x0f, 0x3d, 0xf8, 0x0f, 0x72, 0xe8,
	0x31, 0xc0, 0xab, 0x0e, 0x1f, 0xc4, 0x5e, 0x5a, 0xbe, 0x00, 0x20, 0xad,
	0x09, 0xc0, 0x75, 0x05, 0x88, 0xd3, 0xff, 0x6e, 0x5a, 0x48, 0x48, 0x8b,
	0x7e, 0x0d, 0x81, 0xe7, 0xff, 0x00, 0xf7, 0xe7, 0x03, 0x46, 0xda, 0x13,
	0x56, 0xdc, 0xe8, 0x34, 0x00, 0xeb, 0xe0, 0x5e, 0xac, 0x56, 0xb4, 0x0e,
	0xcd, 0x10, 0x3c, 0x2e, 0x75, 0xf5, 0xc3, 0xe8, 0xf1, 0xff, 0x45, 0x72,
	0x72, 0x6f, 0x72, 0x21, 0x20, 0x48, 0x69, 0x74, 0x20, 0x61, 0x20, 0x6b,
	0x65, 0x79, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x65, 0x62, 0x6f, 0x6f, 0x74,
	0x2e, 0x30, 0xe4, 0xcd, 0x13, 0xcd, 0x16, 0xcd, 0x19, 0x56, 0x89, 0x46,
	0xc8, 0x89, 0x56, 0xca, 0x8c, 0x46, 0xc6, 0x89, 0x5e, 0xc4, 0xe8, 0xbe,
	0xff, 0x2e, 0xb4, 0x41, 0xbb, 0xaa, 0x55, 0x8a, 0x56, 0x24, 0x84, 0xd2,
	0x74, 0x19, 0xcd, 0x13, 0x72, 0x15, 0xd1, 0xe9, 0x81, 0xdb, 0x54, 0xaa,
	0x75, 0x0d, 0x8d, 0x76, 0xc0, 0x89, 0x5e, 0xcc, 0x89, 0x5e, 0xce, 0xb4,
	0x42, 0xeb, 0x26, 0x8b, 0x4e, 0xc8, 0x8b, 0x56, 0xca, 0x8a, 0x46, 0x18,
	0xf6, 0x66, 0x1a, 0x91, 0xf7, 0xf1, 0x92, 0xf6, 0x76, 0x18, 0x89, 0xd1,
	0x88, 0xc6, 0x86, 0xe9, 0xd0, 0xc9, 0xd0, 0xc9, 0x08, 0xe1, 0x41, 0xc4,
	0x5e, 0xc4, 0xb8, 0x01, 0x02, 0x8a, 0x56, 0x24, 0xcd, 0x13, 0x0f, 0x82,
	0x75, 0xff, 0x8b, 0x46, 0x0b, 0xf6, 0x76, 0xc0, 0x01, 0x46, 0xc6, 0x83,
	0x46, 0xc8, 0x01, 0x83, 0x56, 0xca, 0x00, 0x4f, 0x75, 0x98, 0x8e, 0x46,
	0xc6, 0x5e, 0xc3, 0x4d, 0x45, 0x54, 0x41, 0x4b, 0x45, 0x52, 0x4e, 0x53,
	0x59, 0x53, 0x00, 0x00, 0x55, 0xaa
};

static const char fat8_string[8]  = { 'F', 'A', 'T', ' ', ' ', ' ', ' ', ' ' };
static const char fat12_string[8] = { 'F', 'A', 'T', '1', '2', ' ', ' ', ' ' };
static const char fat16_string[8] = { 'F', 'A', 'T', '1', '6', ' ', ' ', ' ' };
static const char fat32_string[8] = { 'F', 'A', 'T', '3', '2', ' ', ' ', ' ' };



static fat_partition_info *fat_get_partition_info(imgtool::partition &partition)
{
	return (fat_partition_info *)partition.extra_bytes();
}



static imgtoolerr_t fat_read_sector(imgtool::partition &partition, uint32_t sector_index,
	int offset, void *buffer, size_t buffer_len)
{
	//const fat_partition_info *disk_info;
	imgtoolerr_t err;
	uint8_t data[FAT_SECLEN];
	uint32_t block_size;
	size_t len;

	//disk_info = fat_get_partition_info(partition);

	/* sanity check */
	err = partition.get_block_size(block_size);
	if (err)
		return err;
	assert(block_size == sizeof(data));

	while(buffer_len > 0)
	{
		err = partition.read_block(sector_index++, data);
		if (err)
			return err;

		len = std::min(buffer_len, sizeof(data) - offset);
		memcpy(buffer, data + offset, len);

		buffer = ((uint8_t *) buffer) + len;
		buffer_len -= len;
		offset = 0;
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_write_sector(imgtool::partition &partition, uint32_t sector_index,
	int offset, const void *buffer, size_t buffer_len)
{
	//const fat_partition_info *disk_info;
	imgtoolerr_t err;
	uint8_t data[FAT_SECLEN];
	const void *write_data;
	uint32_t block_size;
	size_t len;

	//disk_info = fat_get_partition_info(partition);

	/* sanity check */
	err = partition.get_block_size(block_size);
	if (err)
		return err;
	assert(block_size == sizeof(data));

	while(buffer_len > 0)
	{
		len = std::min(buffer_len, sizeof(data) - offset);

		if ((offset != 0) || (buffer_len < sizeof(data)))
		{
			err = partition.read_block(sector_index, data);
			if (err)
				return err;
			memcpy(data + offset, buffer, len);
			write_data = data;
		}
		else
		{
			write_data = buffer;
		}

		err = partition.write_block(sector_index++, write_data);
		if (err)
			return err;

		buffer = ((uint8_t *) buffer) + len;
		buffer_len -= len;
		offset = 0;
	}
	return IMGTOOLERR_SUCCESS;
}


[[maybe_unused]] static imgtoolerr_t fat_clear_sector(imgtool::partition &partition, uint32_t sector_index, uint8_t data)
{
	char buf[FAT_SECLEN];
	memset(buf, data, sizeof(buf));
	return fat_write_sector(partition, sector_index, 0, buf, sizeof(buf));
}


static imgtoolerr_t fat_partition_open(imgtool::partition &partition, uint64_t first_block, uint64_t block_count)
{
	uint8_t header[FAT_SECLEN];
	imgtoolerr_t err;
	fat_partition_info *info;
	uint32_t fat_bits, total_sectors_l, total_sectors_h, sector_size;
	uint64_t available_sectors;
	//int has_extended_bios_param_block = true;

	info = fat_get_partition_info(partition);

	/* read the boot/root sector */
	err = fat_read_sector(partition, 0, 0, header, sizeof(header));
	if (err)
		return err;

	/* magic bytes present? */
	if ((header[510] != 0x55) || (header[511] != 0xAA))
		return IMGTOOLERR_CORRUPTIMAGE;

	/* determine which type of FAT is on this disk */
	if (!memcmp(&header[54], fat8_string, sizeof(fat8_string)))
		fat_bits = 8;
	else if (!memcmp(&header[54], fat12_string, sizeof(fat12_string)))
		fat_bits = 12;
	else if (!memcmp(&header[54], fat16_string, sizeof(fat16_string)))
		fat_bits = 16;
	else if (!memcmp(&header[54], fat32_string, sizeof(fat32_string)))
		fat_bits = 32;
	else
	{
		fat_bits = 8;
		//has_extended_bios_param_block = false;
	}

	info->fat_bits              = fat_bits;
	sector_size                 = get_u16le(&header[11]);
	info->sectors_per_cluster   = header[13];
	info->reserved_sectors      = get_u16le(&header[14]);
	info->fat_count             = header[16];
	info->root_entries          = get_u16le(&header[17]);
	total_sectors_l             = get_u16le(&header[19]);
	info->sectors_per_fat       = get_u16le(&header[22]);
	total_sectors_h             = get_u32le(&header[32]);

	if (info->sectors_per_cluster == 0)
		return IMGTOOLERR_CORRUPTIMAGE;

	info->total_sectors = total_sectors_l + (uint64_t(total_sectors_h) << 16);
	available_sectors = info->total_sectors - info->reserved_sectors
		- (info->sectors_per_fat * info->fat_count)
		- (info->root_entries * FAT_DIRENT_SIZE + FAT_SECLEN - 1) / FAT_SECLEN;
	info->total_clusters = (available_sectors + info->sectors_per_cluster - 1) / info->sectors_per_cluster;
	info->cluster_size = FAT_SECLEN * info->sectors_per_cluster;

	if (info->fat_count == 0)
		return IMGTOOLERR_CORRUPTIMAGE;
	if (sector_size != FAT_SECLEN)
		return IMGTOOLERR_CORRUPTIMAGE;
	if (info->sectors_per_fat == 0)
		return IMGTOOLERR_CORRUPTIMAGE;
	if (info->sectors_per_cluster == 0)
		return IMGTOOLERR_CORRUPTIMAGE;
	if (info->reserved_sectors == 0)
		return IMGTOOLERR_CORRUPTIMAGE;
	if (info->total_clusters * info->fat_bits > info->sectors_per_fat * FAT_SECLEN * 8)
		return IMGTOOLERR_CORRUPTIMAGE;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_create(imgtool::image &image, uint64_t first_block, uint64_t block_count)
{
	imgtoolerr_t err;
	uint32_t heads, tracks, sectors_per_track;
	uint32_t fat_bits, sectors_per_cluster, reserved_sectors, hidden_sectors;
	uint32_t root_dir_count, root_dir_sectors;
	uint32_t sectors_per_fat, fat_count, i;
	uint32_t boot_sector_offset;
	uint64_t total_clusters;
	uint8_t media_descriptor;
	//const char *title;
	const char *fat_bits_string;
	uint8_t header[FAT_SECLEN];
#if 0
	uint64_t first_fat_entries;
#endif

	/* check for limits */
	if (block_count > 0xFFFFFFFFFFFFU)
		return IMGTOOLERR_PARAMTOOLARGE;

	/* get the geometry */
	err = image.get_geometry(&tracks, &heads, &sectors_per_track);
	if (err)
		return err;

	/* cap our sector count so that we only use FAT12/16 */
	sectors_per_cluster = (block_count + 65524 - 1) / 65524;

	/* compute the FAT file system type */
	if ((block_count / sectors_per_cluster) <= 4084)
	{
		fat_bits = 12;
		fat_bits_string = fat12_string;
	}
	else if ((block_count / sectors_per_cluster) <= 65524)
	{
		fat_bits = 16;
		fat_bits_string = fat16_string;
	}
	else
	{
		fat_bits = 32;
		fat_bits_string = fat32_string;
	}

	/* figure out media type */
	i = 0;
	while((known_media[i].heads > 0) && ((known_media[i].heads != heads)
		|| (known_media[i].tracks != tracks)
		|| (known_media[i].sectors != sectors_per_track)))
	{
		i++;
	}
	media_descriptor = known_media[i].media_descriptor;

	/* other miscellaneous settings */
	//title = "";
	fat_count = 2;
	root_dir_count = 512;
	hidden_sectors = 0;
	reserved_sectors = 1;

	/* calculated settings */
	root_dir_sectors = (root_dir_count * FAT_DIRENT_SIZE + FAT_SECLEN - 1) / FAT_SECLEN;
	total_clusters = (block_count - reserved_sectors - hidden_sectors - root_dir_sectors)
		/ sectors_per_cluster;
	sectors_per_fat = (total_clusters * fat_bits + (FAT_SECLEN * 8) - 1)
		/ (FAT_SECLEN * 8);

	/* prepare the header */
	memset(header, 0, sizeof(header));
	memcpy(&header[3], "IMGTOOL ", 8);
	put_u16le(&header[11], FAT_SECLEN);
	header[13] = sectors_per_cluster;
	header[14] = reserved_sectors;
	header[16] = fat_count;
	put_u16le(&header[17], root_dir_count);
	put_u16le(&header[19], uint16_t(block_count >> 0));
	header[21] = media_descriptor;
	put_u16le(&header[22], sectors_per_fat);
	put_u16le(&header[24], sectors_per_track);
	put_u16le(&header[26], heads);
	put_u32le(&header[28], hidden_sectors);
	put_u32le(&header[32], uint32_t(block_count >> 16));
	header[36] = 0xFF;
	header[38] = 0x28;
	header[39] = std::rand();
	header[40] = std::rand();
	header[41] = std::rand();
	header[42] = std::rand();
	memcpy(&header[43], "           ", 11);
	memcpy(&header[54], fat_bits_string, 8);

	/* store boot code */
	boot_sector_offset = sizeof(header) - sizeof(boot_sector_code);
	if (boot_sector_offset < 62)
		return IMGTOOLERR_UNEXPECTED;   /* sanity check */
	if (boot_sector_offset > 510)
		return IMGTOOLERR_UNEXPECTED;   /* sanity check */
	memcpy(&header[boot_sector_offset], boot_sector_code, sizeof(boot_sector_code));

	/* specify jump instruction */
	if (boot_sector_offset <= 129)
	{
		header[0] = 0xEB;                                    /* JMP rel8 */
		header[1] = uint8_t(boot_sector_offset - 2);         /* (offset) */
		header[2] = 0x90;                                    /* NOP */
	}
	else
	{
		header[0] = 0xE9;                                    /* JMP rel16 */
		header[1] = uint8_t((boot_sector_offset - 2) >> 0);  /* (offset) */
		header[2] = uint8_t((boot_sector_offset - 2) >> 8);  /* (offset) */
	}

	err = image.write_block(first_block, header);
	if (err)
		return err;

	/* clear out file allocation table */
	for (i = reserved_sectors; i < (reserved_sectors + sectors_per_fat * fat_count + root_dir_sectors); i++)
	{
		err = image.clear_block(first_block + i, 0);
		if (err)
			return err;
	}

	// FIXME: this causes a corrupt PC floppy image since it doubles the FAT partition header - works without it though
#if 0
	/* set first two FAT entries */
	first_fat_entries = uint64_t(media_descriptor) | 0xFFFFFF00;
	first_fat_entries &= (uint64_t(1) << fat_bits) - 1;
	first_fat_entries |= ((uint64_t(1) << fat_bits) - 1) << fat_bits;
	first_fat_entries = little_endianize_int64(first_fat_entries);

	for (i = 0; i < fat_count; i++)
	{
		err = image->write_block(first_block + 1 + (i * sectors_per_fat), &first_fat_entries);
		if (err)
			return err;
	}
#endif

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_load_fat(imgtool::partition &partition, uint8_t **fat_table)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	const fat_partition_info *disk_info;
	uint8_t *table;
	uint32_t table_size;
	uint32_t pos, len;
	uint32_t sector_index;

	disk_info = fat_get_partition_info(partition);

	table_size = disk_info->sectors_per_fat * disk_info->fat_count * FAT_SECLEN;

	/* allocate the table with extra bytes, in case we "overextend" our reads */
	table = (uint8_t*)malloc(table_size + sizeof(uint64_t));
	if (!table)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(table, 0, table_size + sizeof(uint64_t));

	pos = 0;
	sector_index = disk_info->reserved_sectors;

	while(pos < table_size)
	{
		len = std::min(table_size - pos, uint32_t(FAT_SECLEN));

		err = fat_read_sector(partition, sector_index++, 0, &table[pos], len);
		if (err)
			goto done;

		pos += FAT_SECLEN;
	}

done:
	if (err && table)
	{
		free(table);
		table = NULL;
	}
	*fat_table = table;
	return err;
}



static imgtoolerr_t fat_save_fat(imgtool::partition &partition, const uint8_t *fat_table)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	const fat_partition_info *disk_info;
	uint32_t table_size;
	uint32_t pos, len;
	uint32_t sector_index;

	disk_info = fat_get_partition_info(partition);

	table_size = disk_info->sectors_per_fat * disk_info->fat_count * FAT_SECLEN;

	pos = 0;
	sector_index = disk_info->reserved_sectors;

	while(pos < table_size)
	{
		len = std::min(table_size - pos, uint32_t(FAT_SECLEN));

		err = fat_write_sector(partition, sector_index++, 0, &fat_table[pos], len);
		if (err)
			goto done;

		pos += FAT_SECLEN;
	}

done:
	return err;
}



static uint32_t fat_get_fat_entry(imgtool::partition &partition, const uint8_t *fat_table, uint32_t fat_entry)
{
	const fat_partition_info *disk_info;
	uint64_t entry;
	uint32_t bit_index, i;
	uint32_t last_entry = 0;
	uint32_t bit_mask;

	disk_info = fat_get_partition_info(partition);
	bit_index = fat_entry * disk_info->fat_bits;
	bit_mask = 0xFFFFFFFF >> (32 - disk_info->fat_bits);

	if (fat_entry >= disk_info->total_clusters)
	{
		assert(0);
		return 1;
	}

	/* make sure that the cluster is free in all fats */
	for (i = 0; i < disk_info->fat_count; i++)
	{
		memcpy(&entry, fat_table + (i * FAT_SECLEN
			* disk_info->sectors_per_fat) + (bit_index / 8), sizeof(entry));

		/* we've extracted the bytes; we now need to normalize it */
		entry = little_endianize_int64(entry);
		entry >>= bit_index % 8;
		entry &= bit_mask;

		if (i == 0)
			last_entry = uint32_t(entry);
		else if (last_entry != uint32_t(entry))
			return 1;   /* if the FATs disagree; mark this as reserved */
	}

	/* normalize special clusters */
	if (last_entry >= (0xFFFFFFF0 & bit_mask))
	{
		last_entry |= 0xFFFFFFF0;
		if (last_entry >= 0xFFFFFFF8)
			last_entry = 0xFFFFFFFF;
	}
	return last_entry;
}



static void fat_set_fat_entry(imgtool::partition &partition, uint8_t *fat_table, uint32_t fat_entry, uint32_t value)
{
	const fat_partition_info *disk_info;
	uint64_t entry;
	uint32_t bit_index, i;

	disk_info = fat_get_partition_info(partition);
	bit_index = fat_entry * disk_info->fat_bits;
	value &= 0xFFFFFFFF >> (32 - disk_info->fat_bits);

	for (i = 0; i < disk_info->fat_count; i++)
	{
		memcpy(&entry, fat_table + (i * FAT_SECLEN
			* disk_info->sectors_per_fat) + (bit_index / 8), sizeof(entry));

		entry = little_endianize_int64(entry);
		entry &= (~(uint64_t(0xFFFFFFFF) >> (32 - disk_info->fat_bits)) << (bit_index % 8)) | ((1 << (bit_index % 8)) - 1);
		entry |= uint64_t(value) << (bit_index % 8);
		entry = little_endianize_int64(entry);

		memcpy(fat_table + (i * FAT_SECLEN
			* disk_info->sectors_per_fat) + (bit_index / 8), &entry, sizeof(entry));
	}
}



static void fat_debug_integrity_check(imgtool::partition &partition, const uint8_t *fat_table, const fat_file *file)
{
#ifdef MAME_DEBUG
	/* debug function to test the integrity of a file */
	uint32_t cluster;
	const fat_partition_info *disk_info;

	disk_info = fat_get_partition_info(partition);
	cluster = file->first_cluster ? file->first_cluster : 0xFFFFFFFF;

	if (!file->root)
	{
		while(cluster != 0xFFFFFFFF)
		{
			assert((cluster >= 2) && (cluster < disk_info->total_clusters));
			cluster = fat_get_fat_entry(partition, fat_table, cluster);
		}
	}
#endif
}



static imgtoolerr_t fat_seek_file(imgtool::partition &partition, fat_file *file, uint32_t pos)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	const fat_partition_info *disk_info;
	uint32_t new_cluster;
	uint8_t *fat_table = NULL;

	disk_info = fat_get_partition_info(partition);

	/* can't seek past end of file */
	if (!file->directory && (pos > file->filesize))
		pos = file->filesize;

	if (file->first_cluster == 0)
	{
		/* special case; the root directory */
		file->index = pos;
	}
	else
	{
		/* first, we need to check to see if we have to go back to the beginning */
		if (pos < file->index)
		{
			file->cluster = file->first_cluster;
			file->cluster_index = 0;
			file->eof = 0;
		}

		/* skip ahead clusters */
		while((file->cluster_index + disk_info->cluster_size) <= pos)
		{
			if (!fat_table)
			{
				err = fat_load_fat(partition, &fat_table);
				if (err)
					goto done;
			}

			new_cluster = fat_get_fat_entry(partition, fat_table, file->cluster);

			file->cluster = new_cluster;
			file->cluster_index += disk_info->cluster_size;

			/* are we at the end of the file? */
			if (new_cluster == 0xFFFFFFFF)
			{
				pos = file->cluster_index;
				file->eof = 1;
			}
		}
		file->index = pos;
	}

done:
	if (fat_table)
		free(fat_table);
	return err;
}



static uint32_t fat_get_filepos_sector_index(imgtool::partition &partition, fat_file *file)
{
	uint32_t sector_index;
	const fat_partition_info *disk_info;

	disk_info = fat_get_partition_info(partition);

	sector_index = disk_info->reserved_sectors + (disk_info->sectors_per_fat * disk_info->fat_count);
	if (file->root)
	{
		/* special case for the root file */
		sector_index += file->index / FAT_SECLEN;
	}
	else
	{
		/* cluster out of range? */
		if ((file->cluster < 2) || (file->cluster >= disk_info->total_clusters))
			return 0;

		sector_index += (disk_info->root_entries * FAT_DIRENT_SIZE + FAT_SECLEN - 1) / FAT_SECLEN;
		sector_index += (file->cluster - 2) * disk_info->sectors_per_cluster;
		sector_index += (file->index / FAT_SECLEN) % disk_info->sectors_per_cluster;
	}
	return sector_index;
}



static imgtoolerr_t fat_corrupt_file_error(const fat_file *file)
{
	imgtoolerr_t err;
	if (file->root)
		err = IMGTOOLERR_CORRUPTIMAGE;
	else if (file->directory)
		err = IMGTOOLERR_CORRUPTDIR;
	else
		err = IMGTOOLERR_CORRUPTFILE;
	return err;
}



static imgtoolerr_t fat_readwrite_file(imgtool::partition &partition, fat_file *file,
	void *buffer, size_t buffer_len, size_t *bytes_read, int read_or_write)
{
	imgtoolerr_t err;
//  const fat_partition_info *disk_info;
	uint32_t sector_index;
	int offset;
	size_t len;

//  disk_info =
		fat_get_partition_info(partition);
	if (bytes_read)
		*bytes_read = 0;
	if (!file->directory)
		buffer_len = std::min(buffer_len, size_t(file->filesize - file->index));

	while(!file->eof && (buffer_len > 0))
	{
		sector_index = fat_get_filepos_sector_index(partition, file);
		if (sector_index == 0)
			return fat_corrupt_file_error(file);

		offset = file->index % FAT_SECLEN;
		len = std::min(buffer_len, size_t(FAT_SECLEN - offset));

		/* read or write the data from the disk */
		if (read_or_write)
			err = fat_write_sector(partition, sector_index, offset, buffer, len);
		else
			err = fat_read_sector(partition, sector_index, offset, buffer, len);
		if (err)
			return err;

		/* and move the file pointer ahead */
		err = fat_seek_file(partition, file, file->index + len);
		if (err)
			return err;

		buffer = ((uint8_t *) buffer) + len;
		buffer_len -= len;
		if (bytes_read)
			*bytes_read += len;
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_read_file(imgtool::partition &partition, fat_file *file,
	void *buffer, size_t buffer_len, size_t *bytes_read)
{
	return fat_readwrite_file(partition, file, buffer, buffer_len, bytes_read, 0);
}



static imgtoolerr_t fat_write_file(imgtool::partition &partition, fat_file *file,
	const void *buffer, size_t buffer_len, size_t *bytes_read)
{
	return fat_readwrite_file(partition, file, (void *) buffer, buffer_len, bytes_read, 1);
}



static uint32_t fat_allocate_cluster(imgtool::partition &partition, uint8_t *fat_table)
{
	const fat_partition_info *disk_info;
	uint32_t i, val;

	disk_info = fat_get_partition_info(partition);

	for (i = 2; i < disk_info->total_clusters; i++)
	{
		val = fat_get_fat_entry(partition, fat_table, i);
		if (val == 0)
		{
			fat_set_fat_entry(partition, fat_table, i, 1);
			return i;
		}
	}
	return 0;
}



/* sets the size of a file; ~0 means 'delete' */
static imgtoolerr_t fat_set_file_size(imgtool::partition &partition, fat_file *file,
	uint32_t new_size)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	const fat_partition_info *disk_info;
	uint32_t new_cluster_count;
	uint32_t old_cluster_count;
	uint32_t cluster, write_cluster, last_cluster, new_pos, i;
	uint8_t *fat_table = NULL;
	uint8_t dirent[32];
	size_t clear_size;
	void *clear_buffer = NULL;
	int delete_file = false;
	int rest_free = false;

	disk_info = fat_get_partition_info(partition);

	LOG(("fat_set_file_size(): file->first_cluster=%d new_size=0x%08x\n", file->first_cluster, new_size));

	/* special case */
	if (new_size == ~0)
	{
		delete_file = true;
		new_size = 0;
	}

	/* if this is the trivial case (not changing the size), succeed */
	if (!delete_file && (file->filesize == new_size))
	{
		err = IMGTOOLERR_SUCCESS;
		goto done;
	}

	/* what is the new position? */
	new_pos = std::min(file->index, new_size);

	if (file->root)
	{
		/* this is the root directory; this is a special case */
		if (new_size > (disk_info->root_entries * FAT_DIRENT_SIZE))
		{
			err = IMGTOOLERR_NOSPACE;
			goto done;
		}
	}
	else
	{
		old_cluster_count = (file->filesize + disk_info->cluster_size - 1) / disk_info->cluster_size;
		new_cluster_count = (new_size + disk_info->cluster_size - 1) / disk_info->cluster_size;
		cluster = 0;

		/* load the dirent */
		err = fat_read_sector(partition, file->dirent_sector_index, file->dirent_sector_offset, dirent, sizeof(dirent));
		if (err)
			goto done;

		/* need to load the FAT whether we are growing or shrinking the file */
		if (old_cluster_count != new_cluster_count)
		{
			err = fat_load_fat(partition, &fat_table);
			if (err)
				goto done;

			cluster = 0;
			i = 0;
			do
			{
				last_cluster = cluster;
				write_cluster = 0;

				/* identify the next cluster */
				if (cluster != 0)
					cluster = fat_get_fat_entry(partition, fat_table, cluster);
				else
					cluster = file->first_cluster ? file->first_cluster : 0xFFFFFFFF;

				/* do we need to grow the file by a cluster? */
				if (i < new_cluster_count && ((cluster < 2) || (cluster >= disk_info->total_clusters)))
				{
					/* grow this file by a cluster */
					cluster = fat_allocate_cluster(partition, fat_table);
					if (cluster == 0)
					{
						err = IMGTOOLERR_NOSPACE;
						goto done;
					}

					write_cluster = cluster;
				}
				else if (i >= new_cluster_count)
				{
					/* we are shrinking the file; we need to unlink this node */
					if ((cluster < 2) || (cluster >= disk_info->total_clusters))
						cluster = 0xFFFFFFFF; /* ack file is corrupt! recover */
					write_cluster = 0xFFFFFFFF;
				}

				/* write out the entry, if appropriate */
				if (write_cluster != 0)
				{
					/* are we tieing up loose ends? */
					if (rest_free && (write_cluster == 0xFFFFFFFF))
						write_cluster = 0;

					if (last_cluster == 0)
						file->first_cluster = (write_cluster != 0xFFFFFFFF) ? write_cluster : 0;
					else
						fat_set_fat_entry(partition, fat_table, last_cluster, write_cluster);

					/* did we write the last cluster?  if so, the rest (if any) are free */
					if (write_cluster == 0xFFFFFFFF)
						rest_free = true;
				}
			}
			while((++i < new_cluster_count) || (cluster != 0xFFFFFFFF));
		}

		/* record the new file size */
		put_u16le(&dirent[26], file->first_cluster);
		put_u32le(&dirent[28], new_size);

		/* delete the file, if appropriate */
		if (delete_file)
			dirent[0] = 0xE5;

		/* save the dirent */
		err = fat_write_sector(partition, file->dirent_sector_index, file->dirent_sector_offset, dirent, sizeof(dirent));
		if (err)
			goto done;

		/* if we've modified the FAT, save it out */
		if (fat_table)
		{
			err = fat_save_fat(partition, fat_table);
			if (err)
				goto done;
		}

		/* update the file structure */
		if (!file->directory)
			file->filesize = new_size;
		file->cluster = file->first_cluster;
		file->index = 0;
		file->cluster_index = 0;
		file->eof = (new_cluster_count == 0);
	}

	/* special case; clear out stale bytes on non-root directories */
	if (file->directory && !delete_file)
	{
		if (file->root)
			clear_size = std::min(file->filesize - new_size, uint32_t(FAT_DIRENT_SIZE));
		else
			clear_size = (disk_info->cluster_size - (new_size % disk_info->cluster_size)) % disk_info->cluster_size;

		if (clear_size > 0)
		{
			err = fat_seek_file(partition, file, new_size);
			if (err)
				goto done;

			clear_buffer = malloc(clear_size);
			if (!clear_buffer)
			{
				err = IMGTOOLERR_OUTOFMEMORY;
				goto done;
			}
			memset(clear_buffer, '\0', clear_size);

			err = fat_write_file(partition, file, clear_buffer, clear_size, NULL);
			if (err)
				goto done;
		}
	}

	/* seek back to original pos */
	err = fat_seek_file(partition, file, new_pos);
	if (err)
		goto done;

	if (fat_table)
		fat_debug_integrity_check(partition, fat_table, file);

done:
	if (fat_table)
		free(fat_table);
	if (clear_buffer)
		free(clear_buffer);
	return err;
}



static void prepend_lfn_bytes(char16_t *lfn_buf, size_t lfn_buflen, size_t *lfn_len,
	const uint8_t *entry, int offset, int chars)
{
	uint16_t w;
	int i;
	size_t move_len;

	move_len = std::min(*lfn_len + 1, lfn_buflen - chars - 1);
	memmove(&lfn_buf[chars], &lfn_buf[0], move_len * sizeof(*lfn_buf));

	for (i = 0; i < chars; i++)
	{
		/* read the character */
		memcpy(&w, &entry[offset + i * 2], 2);
		w = little_endianize_int16(w);

		/* append to buffer */
		lfn_buf[i] = (w != 0xFFFF) ? w : 0;
	}
	*lfn_len += chars;
}



static uint8_t fat_calc_filename_checksum(const uint8_t *short_filename)
{
	uint8_t checksum;
	int i, j;

	checksum = 0;
	for (i = 0; i < 11; i++)
	{
		j = checksum & 1;
		checksum >>= 1;
		if (j)
			checksum |= 0x80;
		checksum += short_filename[i];
	}
	return checksum;
}



static void fat_calc_dirent_lfnchecksum(uint8_t *entry, size_t entry_len)
{
	uint8_t checksum;
	int i;

	checksum = fat_calc_filename_checksum(entry + entry_len - FAT_DIRENT_SIZE);

	for (i = 0; i < (entry_len / FAT_DIRENT_SIZE - 1); i++)
		entry[i * FAT_DIRENT_SIZE + 13] = checksum;
}



static char fat_canonicalize_sfn_char(char ch)
{
	/* return the display version of this short file name character */
	return tolower(ch);
}



static void fat_canonicalize_sfn(char *sfn, const uint8_t *sfn_bytes)
{
	/* return the display version of this short file name */
	int i;

	memset(sfn, '\0', 13);
	memcpy(sfn, sfn_bytes, 8);
	rtrim(sfn);
	if (sfn[0] == 0x05)
		sfn[0] = char(0xE5);
	if ((sfn_bytes[8] != ' ') || (sfn_bytes[9] != ' ') || (sfn_bytes[10] != ' '))
	{
		strcat(sfn, ".");
		memcpy(sfn + strlen(sfn), &sfn_bytes[8], 3);
		rtrim(sfn);
	}
	for (i = 0; sfn[i]; i++)
		sfn[i] = fat_canonicalize_sfn_char(sfn[i]);
}



static imgtool::datetime fat_crack_time(uint32_t fat_time)
{
	util::arbitrary_datetime dt;
	dt.second       = ((fat_time >>  0) & 0x001F) * 2;
	dt.minute       = ((fat_time >>  5) & 0x003F);
	dt.hour         = ((fat_time >> 11) & 0x001F);
	dt.day_of_month = ((fat_time >> 16) & 0x001F);
	dt.month        = ((fat_time >> 21) & 0x000F);
	dt.year         = ((fat_time >> 25) & 0x007F) + 1980;
	return imgtool::datetime(imgtool::datetime::LOCAL, dt);
}



static uint32_t fat_setup_time(time_t ansi_time)
{
	std::tm t = *localtime(&ansi_time);

	uint32_t result = 0;
	result |= (((uint32_t) (t.tm_sec / 2))            & 0x001F) <<  0;
	result |= (((uint32_t) t.tm_min)                  & 0x003F) <<  5;
	result |= (((uint32_t) t.tm_hour)                 & 0x001F) << 11;
	result |= (((uint32_t) t.tm_mday)                 & 0x001F) << 16;
	result |= (((uint32_t) t.tm_mon)                  & 0x000F) << 21;
	result |= (((uint32_t) (t.tm_year + 1900 - 1980)) & 0x007F) << 25;

	return result;
}



static imgtoolerr_t fat_read_dirent(
		imgtool::partition &partition,
		fat_file *file,
		fat_dirent &ent,
		fat_freeentry_info *freeent)
{
	imgtoolerr_t err;
	//const fat_partition_info *disk_info;
	uint8_t entry[FAT_DIRENT_SIZE];
	size_t bytes_read;
	char16_t lfn_buf[512];
	size_t lfn_len = 0;
	int lfn_lastentry = 0;
	uint8_t lfn_checksum = 0;
	uint32_t entry_index, entry_sector_index, entry_sector_offset;

	assert(file->directory);
	lfn_buf[0] = '\0';
	ent = fat_dirent();
	//disk_info = fat_get_partition_info(partition);

	/* The first eight bytes of a FAT directory entry is a blank padded name
	 *
	 * The first byte can be special:
	 *  0x00 - entry is available and no further entry is used
	 *  0x05 - first character is actually 0xe5
	 *  0x2E - dot entry; either '.' or '..'
	 *  0xE5 - entry has been erased and is available
	 *
	 * Byte 11 is the attributes; and 0x0F denotes a LFN entry
	 */
	do
	{
		entry_index = file->index;
		entry_sector_index = fat_get_filepos_sector_index(partition, file);
		entry_sector_offset = file->index % FAT_SECLEN;

		err = fat_read_file(partition, file, entry, sizeof(entry), &bytes_read);
		if (err)
			return err;
		if (bytes_read < sizeof(entry))
			memset(entry, 0, sizeof(entry));

		if (entry[11] == 0x0F)
		{
			/* this is an LFN entry */
			if ((lfn_lastentry == 0)
				|| ((entry[0] & 0x3F) != (lfn_lastentry - 1))
				|| (lfn_checksum != entry[13]))
			{
				lfn_buf[0] = 0;
				lfn_len = 0;
				lfn_checksum = entry[13];
			}
			lfn_lastentry = entry[0] & 0x3F;
			prepend_lfn_bytes(lfn_buf, std::size(lfn_buf),
				&lfn_len, entry, 28, 2);
			prepend_lfn_bytes(lfn_buf, std::size(lfn_buf),
				&lfn_len, entry, 14, 6);
			prepend_lfn_bytes(lfn_buf, std::size(lfn_buf),
				&lfn_len, entry,  1, 5);
		}
		else if (freeent && (freeent->position == ~0))
		{
			/* do a quick check to find out if we found space */
			if ((entry[0] == '\0') || (entry[0] == 0xE5))
			{
				if (freeent->candidate_position > entry_index)
					freeent->candidate_position = entry_index;

				if ((entry[0] == '\0') || (freeent->candidate_position + freeent->required_size < file->index))
					freeent->position = freeent->candidate_position;
			}
			else
			{
				freeent->candidate_position = ~0;
			}
		}
	}
	while ((entry[0] == 0x2E) || (entry[0] == 0xE5) || (entry[11] == 0x0F));

	/* no more directory entries? */
	if (entry[0] == '\0')
	{
		ent.eof = 1;
		return IMGTOOLERR_SUCCESS;
	}

	/* pick apart short filename */
	fat_canonicalize_sfn(ent.short_filename, entry);

	/* and the long filename */
	if (lfn_lastentry == 1)
	{
		/* only use the LFN if the checksum passes */
		if (lfn_checksum == fat_calc_filename_checksum(entry))
		{
			int i = 0, j = 0;
			char32_t ch;
			do
			{
				i += uchar_from_utf16(&ch, &lfn_buf[i], std::size(lfn_buf) - i);
				j += utf8_from_uchar(&ent.long_filename[j], std::size(ent.long_filename) - j, ch);
			}
			while (ch != 0);
		}
	}

	/* other attributes */
	ent.filesize               = get_u32le(&entry[28]);
	ent.directory              = (entry[11] & 0x10) ? 1 : 0;
	ent.first_cluster          = get_u16le(&entry[26]);
	ent.dirent_sector_index    = entry_sector_index;
	ent.dirent_sector_offset   = entry_sector_offset;
	ent.creation_time          = fat_crack_time(get_u32le(&entry[14]));
	ent.lastmodified_time      = fat_crack_time(get_u32le(&entry[22]));
	return IMGTOOLERR_SUCCESS;
}



enum sfn_disposition_t
{
	SFN_SUFFICIENT, /* name fully representable in short file name */
	SFN_DERIVATIVE, /* name not fully representable in short file name, but no need to tildize */
	SFN_MANGLED     /* name not representable in short file name; must tildize */
};

static imgtoolerr_t fat_construct_dirent(const char *filename, creation_policy_t create,
	uint8_t **entry, size_t *entry_len)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	uint8_t *created_entry = NULL;
	uint8_t *new_created_entry;
	uint32_t now;
	size_t created_entry_len = FAT_DIRENT_SIZE;
	size_t created_entry_pos = 0;
	char32_t ch;
	char last_short_char = ' ';
	char short_char = '\0';
	char canonical_short_char;
	char16_t buf[UTF16_CHAR_MAX];
	int i, len;
	int sfn_pos = 0;
	sfn_disposition_t sfn_disposition = SFN_SUFFICIENT;
	int sfn_in_extension = 0;

	/* sanity check */
	if (*filename == '\0')
	{
		err = IMGTOOLERR_BADFILENAME;
		goto done;
	}

	/* construct intial entry */
	created_entry = (uint8_t *) malloc(FAT_DIRENT_SIZE);
	if (!created_entry)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}

	/* set up the basics for the new dirent */
	memset(created_entry +  0, ' ', 11);
	memset(created_entry + 12, '\0', FAT_DIRENT_SIZE - 12);
	created_entry[11] = (create == CREATE_DIR) ? 0x10 : 0x00;

	/* set up file dates in the new dirent */
	now = fat_setup_time(time(NULL));
	put_u32le(&created_entry[14], now);
	put_u16le(&created_entry[18], now);
	put_u32le(&created_entry[22], now);

	while(*filename)
	{
		filename += uchar_from_utf8(&ch, filename, UTF8_CHAR_MAX);

		/* append to short filename, if possible */
		if ((ch < 32) || (ch > 128))
			short_char = '\0';
		else if (isalnum(char(ch)))
			short_char = toupper(char(ch));
		else if (strchr(".!#$%^()-@^_`{}~", char(ch)))
			short_char = char(ch);
		else
			short_char = '\0';  /* illegal SFN char */
		canonical_short_char = fat_canonicalize_sfn_char(char(ch));
		if (!short_char || (short_char != canonical_short_char))
		{
			if (toupper(short_char) == toupper(canonical_short_char))
				sfn_disposition = std::max(sfn_disposition, SFN_DERIVATIVE);
			else
				sfn_disposition = SFN_MANGLED;
		}

		/* append the short filename char */
		if (short_char == '.')
		{
			/* multiple extensions or trailing spaces? */
			if (sfn_in_extension)
				sfn_disposition = SFN_MANGLED;
			else if (last_short_char == ' ')
				sfn_disposition = std::max(sfn_disposition, SFN_DERIVATIVE);

			sfn_in_extension = 1;
			sfn_pos = 8;
			created_entry[created_entry_len - FAT_DIRENT_SIZE + 8] = ' ';
			created_entry[created_entry_len - FAT_DIRENT_SIZE + 9] = ' ';
			created_entry[created_entry_len - FAT_DIRENT_SIZE + 10] = ' ';
		}
		else if (sfn_pos == (sfn_in_extension ? 11 : 8))
		{
			/* ran out of characters for short filename */
			sfn_disposition = SFN_MANGLED;
		}
		else if (short_char != '\0')
		{
			created_entry[created_entry_len - FAT_DIRENT_SIZE + sfn_pos++] = short_char;
		}
		last_short_char = short_char;


		/* convert to UTF-16 and add a long filename entry */
		len = utf16le_from_uchar(buf, UTF16_CHAR_MAX, ch);
		for (i = 0; i < len; i++)
		{
			switch(created_entry_pos)
			{
				case 0:
				case 32:
					/* need to grow */
					new_created_entry = (uint8_t *) malloc(created_entry_len + FAT_DIRENT_SIZE);
					if (!new_created_entry)
					{
						err = IMGTOOLERR_OUTOFMEMORY;
						goto done;
					}

					/* move existing entries forward */
					memmove(new_created_entry + 32, created_entry, created_entry_len);

					if (created_entry) free(created_entry);
					created_entry = new_created_entry;
					created_entry_len += 32;

					/* set up this LFN */
					memset(created_entry, '\0', 32);
					memset(&created_entry[1], '\xFF', 10);
					memset(&created_entry[14], '\xFF', 12);
					memset(&created_entry[28], '\xFF', 4);
					created_entry[11] = 0x0F;

					/* specify entry index */
					created_entry[0] = (created_entry_len / 32) - 1;
					if (created_entry[0] >= 0x40)
					{
						err = IMGTOOLERR_BADFILENAME;
						goto done;
					}
					created_entry_pos = 1;
					break;

				case 11:
					created_entry_pos = 14;
					break;

				case 26:
					created_entry_pos = 28;
					break;
			}

			memcpy(&created_entry[created_entry_pos], &buf[i], 2);
			created_entry_pos += 2;
		}
	}

	/* trailing spaces? */
	if (short_char == ' ')
		sfn_disposition = std::max(sfn_disposition, SFN_DERIVATIVE);

	if (sfn_disposition == SFN_SUFFICIENT)
	{
		/* the short filename suffices; remove the LFN stuff */
		memcpy(created_entry, created_entry + created_entry_len - FAT_DIRENT_SIZE, FAT_DIRENT_SIZE);
		created_entry_len = FAT_DIRENT_SIZE;
		free(created_entry);
		created_entry = NULL;
		new_created_entry = (uint8_t *) malloc(created_entry_len);
		if (!new_created_entry)
		{
			err = IMGTOOLERR_OUTOFMEMORY;
			goto done;
		}
		created_entry = new_created_entry;
	}
	else
	{
		/* need to do finishing touches on the LFN */
		created_entry[0] |= 0x40;

		/* if necessary, mangle the name */
		if (sfn_disposition == SFN_MANGLED)
		{
			i = 6;
			while((i > 0) && isspace(created_entry[created_entry_len - FAT_DIRENT_SIZE + i - 1]))
				i--;
			created_entry[created_entry_len - FAT_DIRENT_SIZE + i + 0] = '~';
			created_entry[created_entry_len - FAT_DIRENT_SIZE + i + 1] = '1';
		}
		fat_calc_dirent_lfnchecksum(created_entry, created_entry_len);
	}

done:
	if (err && created_entry)
	{
		free(created_entry);
		created_entry = NULL;
	}
	*entry = created_entry;
	*entry_len = created_entry_len;
	return err;
}



static void fat_bump_dirent(imgtool::partition &partition, uint8_t *entry, size_t entry_len)
{
	uint8_t *sfn_entry;
	int pos, digit_count, i;
	uint32_t digit_place, val = 0;

	sfn_entry = &entry[entry_len - FAT_DIRENT_SIZE];

	digit_place = 1;
	for (pos = 7; (pos >= 0) && isdigit(char(sfn_entry[pos])); pos--)
	{
		val += (sfn_entry[pos] - '0') * digit_place;
		digit_place *= 10;
	}
	val++;
	pos++;

	/* count the digits */
	digit_place = 1;
	digit_count = 1;
	while(val >= digit_place * 10)
	{
		digit_count++;
		digit_place *= 10;
	}

	/* give us some more space, if necessary */
	while ((pos > 0) && ((pos + digit_count) > 8))
		pos--;

	/* have we actually ran out of digits */
	if ((pos + digit_count) > 8)
	{
		/* extreme degenerate case; simply randomize the filename */
		for (i = 0; i < 6; i++)
			sfn_entry[i] = 'A' + (std::rand() % 26);
		sfn_entry[6] = '~';
		sfn_entry[7] = '0';
	}
	else
	{
		/* write the tilde, if possible */
		if (pos > 0)
			sfn_entry[pos - 1] = '~';

		/* write out the number */
		while(digit_place > 0)
		{
			sfn_entry[pos++] = (val / digit_place) + '0';
			val %= digit_place;
			digit_place /= 10;
		}
	}

	/* since we changed the short file name, we need to recalc the checksums
	 * in the LFN entries */
	fat_calc_dirent_lfnchecksum(entry, entry_len);
}



static imgtoolerr_t fat_lookup_path(imgtool::partition &partition, const char *path,
	creation_policy_t create, fat_file *file)
{
	imgtoolerr_t err;
	const fat_partition_info *disk_info;
	fat_dirent ent;
	fat_freeentry_info freeent = { 0, };
	const char *next_path_part;
	uint8_t *created_entry = NULL;
	size_t created_entry_len = 0;
	uint32_t entry_sector_index, entry_sector_offset;
	uint32_t parent_first_cluster;
	int bumped_sfn;
	char sfn[13];

	disk_info = fat_get_partition_info(partition);

	memset(file, 0, sizeof(*file));
	file->root = 1;
	file->directory = 1;
	file->filesize = disk_info->root_entries * FAT_DIRENT_SIZE;

	while(*path)
	{
		if (!file->directory)
		{
			err = IMGTOOLERR_PATHNOTFOUND;
			goto done;
		}

		next_path_part = path + strlen(path) + 1;
		if (create && (*next_path_part == '\0'))
		{
			/* this is the last entry, and we are creating a file */
			err = fat_construct_dirent(path, create, &created_entry, &created_entry_len);
			if (err)
				goto done;

			freeent.required_size = created_entry_len;
			freeent.candidate_position = ~0;
			freeent.position = ~0;
		}

		do
		{
			err = fat_read_dirent(partition, file, ent, created_entry ? &freeent : NULL);
			if (err)
				goto done;

			LOG(("fat_lookup_path(): %s/%s: %d\n", ent.short_filename, ent.long_filename, ent.dirent_sector_offset));
		}
		while(!ent.eof && core_stricmp(path, ent.short_filename) && core_stricmp(path, ent.long_filename));

		parent_first_cluster = file->first_cluster;

		if (ent.eof)
		{
			/* it seems that we have reached the end of this directory */
			if (!created_entry)
			{
				err = IMGTOOLERR_FILENOTFOUND;
				goto done;
			}

			if (created_entry_len > FAT_DIRENT_SIZE)
			{
				/* must ensure uniqueness of the short filename */
				do
				{
					/* rewind to the beginning of the directory */
					err = fat_seek_file(partition, file, 0);
					if (err)
						goto done;

					bumped_sfn = false;
					fat_canonicalize_sfn(sfn, &created_entry[created_entry_len - FAT_DIRENT_SIZE]);

					do
					{
						err = fat_read_dirent(partition, file, ent, NULL);
						if (err)
							goto done;

						if (!core_stricmp(sfn, ent.short_filename))
						{
							bumped_sfn = true;
							fat_bump_dirent(partition, created_entry, created_entry_len);
							fat_canonicalize_sfn(sfn, &created_entry[created_entry_len - FAT_DIRENT_SIZE]);
						}
					}
					while(!ent.eof);
				}
				while(bumped_sfn);
			}

			LOG(("fat_lookup_path(): creating entry; pos=%u length=%u\n", freeent.position, freeent.required_size));

			err = fat_set_file_size(partition, file, std::max(file->filesize, uint32_t(freeent.position + created_entry_len)));
			if (err)
				goto done;

			err = fat_seek_file(partition, file, freeent.position);
			if (err)
				goto done;

			err = fat_write_file(partition, file, created_entry, created_entry_len, NULL);
			if (err)
				goto done;

			/* we have to do a special seek operation to get the main dirent */
			err = fat_seek_file(partition, file, freeent.position + created_entry_len - FAT_DIRENT_SIZE);
			if (err)
				goto done;
			entry_sector_index = fat_get_filepos_sector_index(partition, file);
			entry_sector_offset = file->index % FAT_SECLEN;

			/* build the file struct for the newly created file/directory */
			memset(file, 0, sizeof(*file));
			file->directory = (created_entry[created_entry_len - FAT_DIRENT_SIZE + 11] & 0x10) ? 1 : 0;
			file->dirent_sector_index = entry_sector_index;
			file->dirent_sector_offset = entry_sector_offset;
		}
		else
		{
			/* update the current file */
			memset(file, 0, sizeof(*file));
			file->directory = ent.directory;
			file->filesize = ent.filesize;
			file->cluster = ent.first_cluster;
			file->first_cluster = ent.first_cluster;
			file->dirent_sector_index = ent.dirent_sector_index;
			file->dirent_sector_offset = ent.dirent_sector_offset;
		}

		path = next_path_part;
		file->parent_first_cluster = parent_first_cluster;
	}

	err = IMGTOOLERR_SUCCESS;

done:
	if (created_entry)
		free(created_entry);
	return err;
}



static imgtoolerr_t fat_partition_beginenum(imgtool::directory &enumeration, const char *path)
{
	imgtoolerr_t err;
	fat_file *file;

	file = (fat_file *) enumeration.extra_bytes();

	err = fat_lookup_path(enumeration.partition(), path, CREATE_NONE, file);
	if (err)
		return err;
	if (!file->directory)
		return IMGTOOLERR_PATHNOTFOUND;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	imgtoolerr_t err;
	fat_file *file;
	fat_dirent fatent;

	file = (fat_file *) enumeration.extra_bytes();
	err = fat_read_dirent(enumeration.partition(), file, fatent, NULL);
	if (err)
		return err;

	/* copy stuff from the FAT dirent to the Imgtool dirent */
	snprintf(ent.filename, std::size(ent.filename), "%s", fatent.long_filename[0]
		? fatent.long_filename : fatent.short_filename);
	ent.filesize = fatent.filesize;
	ent.directory = fatent.directory;
	ent.eof = fatent.eof;
	ent.creation_time = fatent.creation_time;
	ent.lastmodified_time = fatent.lastmodified_time;
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_read_bootblock(imgtool::partition &partition, imgtool::stream &stream)
{
	imgtoolerr_t err;
	uint8_t block[FAT_SECLEN];

	err = fat_read_sector(partition, 0, 0, block, sizeof(block));
	if (err)
		return err;

	stream.write(block, sizeof(block));
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_write_bootblock(imgtool::partition &partition, imgtool::stream &stream)
{
	imgtoolerr_t err;
	uint8_t block[FAT_SECLEN];
	uint8_t new_block[FAT_SECLEN];

	if (stream.size() != sizeof(new_block))
		return IMGTOOLERR_UNEXPECTED;
	stream.read(new_block, sizeof(new_block));

	if (new_block[510] != 0x55)
		return IMGTOOLERR_UNEXPECTED;
	if (new_block[511] != 0xAA)
		return IMGTOOLERR_UNEXPECTED;

	/* read current boot sector */
	err = fat_read_sector(partition, 0, 0, block, sizeof(block));
	if (err)
		return err;

	/* merge in the new stuff */
	memcpy(&block[ 0], &new_block[ 0],   3);
	memcpy(&block[62], &new_block[62], 448);

	/* and write it out */
	err = fat_write_sector(partition, 0, 0, block, sizeof(block));
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_readfile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	imgtoolerr_t err;
	fat_file file;
	size_t bytes_read;
	char buffer[1024];

	/* special case for bootblock */
	if (filename == FILENAME_BOOTBLOCK)
		return fat_read_bootblock(partition, destf);

	err = fat_lookup_path(partition, filename, CREATE_NONE, &file);
	if (err)
		return err;

	if (file.directory)
		return IMGTOOLERR_FILENOTFOUND;

	do
	{
		err = fat_read_file(partition, &file, buffer, sizeof(buffer), &bytes_read);
		if (err)
			return err;

		destf.write(buffer, bytes_read);
	}
	while(bytes_read > 0);
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_writefile(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	imgtoolerr_t err;
	fat_file file;
	uint32_t bytes_left, len;
	char buffer[1024];

	/* special case for bootblock */
	if (filename == FILENAME_BOOTBLOCK)
		return fat_write_bootblock(partition, sourcef);

	err = fat_lookup_path(partition, filename, CREATE_FILE, &file);
	if (err)
		return err;

	if (file.directory)
		return IMGTOOLERR_FILENOTFOUND;

	bytes_left = uint32_t(sourcef.size());

	err = fat_set_file_size(partition, &file, bytes_left);
	if (err)
		return err;

	while(bytes_left > 0)
	{
		len = (std::min<size_t>)(bytes_left, sizeof(buffer));
		sourcef.read(buffer, len);

		err = fat_write_file(partition, &file, buffer, len, NULL);
		if (err)
			return err;

		bytes_left -= len;
	}
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_delete(imgtool::partition &partition, const char *filename, unsigned int dir)
{
	imgtoolerr_t err;
	fat_file file;
	fat_dirent ent;

	err = fat_lookup_path(partition, filename, CREATE_NONE, &file);
	if (err)
		return err;
	if (file.directory != dir)
		return IMGTOOLERR_FILENOTFOUND;

	if (dir)
	{
		err = fat_read_dirent(partition, &file, ent, NULL);
		if (err)
			return err;
		if (!ent.eof)
			return IMGTOOLERR_DIRNOTEMPTY;
	}

	err = fat_set_file_size(partition, &file, ~0);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_deletefile(imgtool::partition &partition, const char *filename)
{
	return fat_partition_delete(partition, filename, 0);
}



static imgtoolerr_t fat_partition_freespace(imgtool::partition &partition, uint64_t *size)
{
	imgtoolerr_t err;
	const fat_partition_info *disk_info;
	uint8_t *fat_table;
	uint32_t i;

	disk_info = fat_get_partition_info(partition);

	err = fat_load_fat(partition, &fat_table);
	if (err)
		goto done;

	*size = 0;
	for (i = 2; i < disk_info->total_clusters; i++)
	{
		if (fat_get_fat_entry(partition, fat_table, i) == 0)
			*size += disk_info->cluster_size;
	}

done:
	if (fat_table)
		free(fat_table);
	return err;
}



static imgtoolerr_t fat_partition_createdir(imgtool::partition &partition, const char *path)
{
	imgtoolerr_t err;
	fat_file file;
	uint8_t initial_data[64];

	err = fat_lookup_path(partition, path, CREATE_DIR, &file);
	if (err)
		return err;
	if (!file.directory)
		return IMGTOOLERR_FILENOTFOUND;

	err = fat_set_file_size(partition, &file, sizeof(initial_data));
	if (err)
		return err;

	/* set up the two directory entries in all directories */
	memset(initial_data, 0, sizeof(initial_data));
	memcpy(&initial_data[0], ".          ", 11);
	initial_data[11] = 0x10;
	put_u16le(&initial_data[26], file.first_cluster);
	memcpy(&initial_data[32], ".          ", 11);
	initial_data[43] = 0x10;
	put_u16le(&initial_data[58], file.parent_first_cluster);

	err = fat_write_file(partition, &file, initial_data, sizeof(initial_data), NULL);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t fat_partition_deletedir(imgtool::partition &partition, const char *path)
{
	return fat_partition_delete(partition, path, 1);
}



void fat_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR:        info->i = 1; break;
		case IMGTOOLINFO_INT_OPEN_IS_STRICT:                info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME:        info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME:    info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK:            info->i = 1; break;
		case IMGTOOLINFO_INT_PATH_SEPARATOR:                info->i = '\\'; break;
		case IMGTOOLINFO_INT_ALTERNATE_PATH_SEPARATOR:      info->i = '/'; break;
		case IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES:         info->i = sizeof(fat_partition_info); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:         info->i = sizeof(fat_file); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r\n"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_CREATE_PARTITION:              info->create_partition = fat_partition_create; break;
		case IMGTOOLINFO_PTR_OPEN_PARTITION:                info->open_partition = fat_partition_open; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum = fat_partition_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = fat_partition_nextenum; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = fat_partition_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = fat_partition_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = fat_partition_deletefile; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = fat_partition_freespace; break;
		case IMGTOOLINFO_PTR_CREATE_DIR:                    info->create_dir = fat_partition_createdir; break;
		case IMGTOOLINFO_PTR_DELETE_DIR:                    info->delete_dir = fat_partition_deletedir; break;
	}
}
