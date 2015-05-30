// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Cybiko Classic File System

    (c) 2007 Tim Schuerewegen

*/

#include "imgtool.h"

#include <zlib.h>

struct cybiko_file_system
{
	imgtool_stream *stream;
	UINT32 page_count, page_size, block_count_boot, block_count_file;
	UINT16 write_count;
};

struct cybiko_iter
{
	UINT16 block;
};

struct cfs_file
{
	char name[64]; // name of the file
	UINT32 date;   // date/time of the file (seconds since 1900/01/01)
	UINT32 size;   // size of the file
	UINT32 blocks; // number of blocks occupied by the file
};

enum
{
	BLOCK_TYPE_INVALID,
	BLOCK_TYPE_BOOT,
	BLOCK_TYPE_FILE
};

#define MAX_PAGE_SIZE (264 * 2)

#define INVALID_FILE_ID  0xFFFF

enum
{
	FLASH_TYPE_INVALID,
	FLASH_TYPE_AT45DB041,
	FLASH_TYPE_AT45DB081,
	FLASH_TYPE_AT45DB161
};

#define BLOCK_USED(x)      (x[0] & 0x80)
#define BLOCK_FILE_ID(x)   buffer_read_16_be( x + 2)
#define BLOCK_PART_ID(x)   buffer_read_16_be( x + 4)
#define BLOCK_FILENAME(x)  (char*)(x + 7)

#define FILE_HEADER_SIZE  0x48

// 2208988800 is the number of seconds between 1900/01/01 and 1970/01/01

static time_t time_crack( UINT32 cfs_time)
{
	return (time_t)(cfs_time - 2208988800UL);
}

static UINT32 time_setup( time_t ansi_time)
{
	return (UINT32)(ansi_time + 2208988800UL);
}

static UINT32 buffer_read_32_be( UINT8 *buffer)
{
	return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] << 0);
}

static UINT16 buffer_read_16_be( UINT8 *buffer)
{
	return (buffer[0] << 8) | (buffer[1] << 0);
}

static void buffer_write_32_be( UINT8 *buffer, UINT32 data)
{
	buffer[0] = (data >> 24) & 0xFF;
	buffer[1] = (data >> 16) & 0xFF;
	buffer[2] = (data >>  8) & 0xFF;
	buffer[3] = (data >>  0) & 0xFF;
}

static void buffer_write_16_be( UINT8 *buffer, UINT16 data)
{
	buffer[0] = (data >> 8) & 0xFF;
	buffer[1] = (data >> 0) & 0xFF;
}

// page = crc1 (4) + wcnt (2) + crc2 (2) + data (x) + unk (2)

static UINT32 page_buffer_calc_checksum_1( UINT8 *buffer, UINT32 size, int block_type)
{
	return crc32( 0, buffer + 8, (block_type == BLOCK_TYPE_BOOT) ? 250 : size - 10);
}

static UINT16 page_buffer_calc_checksum_2( UINT8 *buffer)
{
	UINT16 val = 0xAF17;
	val ^= buffer_read_16_be( buffer + 0);
	val ^= buffer_read_16_be( buffer + 2);
	val ^= buffer_read_16_be( buffer + 4);
	return FLIPENDIAN_INT16(val);
}

static int page_buffer_verify( UINT8 *buffer, UINT32 size, int block_type)
{
	UINT32 checksum_page, checksum_calc;
	// checksum 1
	checksum_calc = page_buffer_calc_checksum_1( buffer, size, block_type);
	checksum_page = buffer_read_32_be( buffer + 0);
	if (checksum_calc != checksum_page) return FALSE;
	// checksum 2
	checksum_calc = page_buffer_calc_checksum_2( buffer);
	checksum_page = buffer_read_16_be( buffer + 6);
	if (checksum_calc != checksum_page) return FALSE;
	// ok
	return TRUE;
}

static int cfs_block_to_page( cybiko_file_system *cfs, int block_type, UINT32 block, UINT32 *page)
{
	switch (block_type)
	{
		case BLOCK_TYPE_BOOT : if (page) *page = block; return TRUE;
		case BLOCK_TYPE_FILE : if (page) *page = block + cfs->block_count_boot; return TRUE;
		default              : return FALSE;
	}
}

static int cfs_page_to_block( cybiko_file_system *cfs, UINT32 page, int *block_type, UINT32 *block)
{
	UINT32 tmp = page;
	// boot block
	if (tmp < cfs->block_count_boot)
	{
		if (block_type) *block_type = BLOCK_TYPE_BOOT;
		if (block) *block = tmp;
		return TRUE;
	}
	tmp -= cfs->block_count_boot;
	// file block
	if (tmp < cfs->block_count_file)
	{
		if (block_type) *block_type = BLOCK_TYPE_FILE;
		if (block) *block = tmp;
		return TRUE;
	}
	tmp -= cfs->block_count_file;
	// error
	return FALSE;
}

static int cfs_page_read( cybiko_file_system *cfs, UINT8 *buffer, UINT32 page)
{
	if (page >= cfs->page_count) return FALSE;
	stream_seek( cfs->stream, page * cfs->page_size, SEEK_SET);
	stream_read( cfs->stream, buffer, cfs->page_size);
	return TRUE;
}

static int cfs_page_write( cybiko_file_system *cfs, UINT8 *buffer, UINT32 page)
{
	if (page >= cfs->page_count) return FALSE;
	stream_seek( cfs->stream, page * cfs->page_size, SEEK_SET);
	stream_write( cfs->stream, buffer, cfs->page_size);
	return TRUE;
}

static int cfs_block_read( cybiko_file_system *cfs, UINT8 *buffer, int block_type, UINT32 block)
{
	UINT8 buffer_page[MAX_PAGE_SIZE];
	UINT32 page;
	if (!cfs_block_to_page( cfs, block_type, block, &page)) return FALSE;
	if (!cfs_page_read( cfs, buffer_page, page)) return FALSE;
	memcpy( buffer, buffer_page + 8, cfs->page_size - 10);
	return TRUE;
}

static int cfs_block_write( cybiko_file_system *cfs, UINT8 *buffer, int block_type, UINT32 block)
{
	UINT8 buffer_page[MAX_PAGE_SIZE];
	UINT32 page;
	memcpy( buffer_page + 8, buffer, cfs->page_size - 10);
	buffer_write_32_be( buffer_page + 0, page_buffer_calc_checksum_1( buffer_page, cfs->page_size, block_type));
	buffer_write_16_be( buffer_page + 4, cfs->write_count++);
	buffer_write_16_be( buffer_page + 6, page_buffer_calc_checksum_2( buffer_page));
	buffer_write_16_be( buffer_page + cfs->page_size - 2, 0xFFFF);
	if (!cfs_block_to_page( cfs, block_type, block, &page)) return FALSE;
	if (!cfs_page_write( cfs, buffer_page, page)) return FALSE;
	return TRUE;
}

static int cfs_file_delete( cybiko_file_system *cfs, UINT16 file_id)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i;
	for (i=0;i<cfs->block_count_file;i++)
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return FALSE;
		if (BLOCK_USED(buffer) && (BLOCK_FILE_ID(buffer) == file_id))
		{
			buffer[0] &= ~0x80;
			if (!cfs_block_write( cfs, buffer, BLOCK_TYPE_FILE, i)) return FALSE;
		}
	}
	return TRUE;
}

static int cfs_file_info( cybiko_file_system *cfs, UINT16 file_id, cfs_file *file)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i;
	file->blocks = file->size = 0;
	for (i=0;i<cfs->block_count_file;i++)
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return FALSE;
		if (BLOCK_USED(buffer) && (BLOCK_FILE_ID(buffer) == file_id))
		{
			if (BLOCK_PART_ID(buffer) == 0)
			{
				strcpy( file->name, BLOCK_FILENAME(buffer));
				file->date = buffer_read_32_be( buffer + 6 + FILE_HEADER_SIZE - 4);
			}
			file->size += buffer[1];
			file->blocks++;
		}
	}
	return (file->blocks > 0) ? TRUE : FALSE;
}

static int cfs_file_find( cybiko_file_system *cfs, const char *filename, UINT16 *file_id)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i;
	for (i=0;i<cfs->block_count_file;i++)
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return FALSE;
		if (BLOCK_USED(buffer) && (strncmp( filename, BLOCK_FILENAME(buffer), 40) == 0))
		{
			*file_id = i;
			return TRUE;
		}
	}
	return FALSE;
}

static int cfs_verify( cybiko_file_system *cfs)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i, block_type;
	for (i=0;i<cfs->page_count;i++)
	{
		if (!cfs_page_read( cfs, buffer, i)) return FALSE;
		if (!cfs_page_to_block( cfs, i, &block_type, NULL)) return FALSE;
		if (!page_buffer_verify( buffer, cfs->page_size, block_type)) return FALSE;
	}
	return TRUE;
}

static int cfs_init( cybiko_file_system *cfs, imgtool_stream *stream, int flash_type)
{
	cfs->stream = stream;
	switch (flash_type)
	{
		case FLASH_TYPE_AT45DB041 : cfs->page_count = 2048; cfs->page_size = 264; break;
		case FLASH_TYPE_AT45DB081 : cfs->page_count = 4096; cfs->page_size = 264; break;
		case FLASH_TYPE_AT45DB161 : cfs->page_count = 4096; cfs->page_size = 528; break;
		default                   : return FALSE;
	}
	cfs->block_count_boot = 5;
	cfs->block_count_file = cfs->page_count - cfs->block_count_boot;
	cfs->write_count = 0;
	return TRUE;
}

static int cfs_format( cybiko_file_system *cfs)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i;
	// boot blocks
	memset( buffer, 0xFF, sizeof( buffer));
	for (i=0;i<cfs->block_count_boot;i++)
	{
		if (!cfs_block_write( cfs, buffer, BLOCK_TYPE_BOOT, i)) return FALSE;
	}
	// file blocks
	memset( buffer, 0xFF, sizeof( buffer));
	buffer[0] &= ~0x80;
	for (i=0;i<cfs->block_count_file;i++)
	{
		if (!cfs_block_write( cfs, buffer, BLOCK_TYPE_FILE, i)) return FALSE;
	}
	// ok
	return TRUE;
}

static UINT16 cfs_calc_free_blocks( cybiko_file_system *cfs)
{
	UINT8 buffer[MAX_PAGE_SIZE];
	int i;
	UINT16 blocks = 0;
	for (i=0;i<cfs->block_count_file;i++)
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return 0;
		if (!BLOCK_USED(buffer)) blocks++;
	}
	return blocks;
}

static UINT32 cfs_calc_free_space( cybiko_file_system *cfs, UINT16 blocks)
{
	UINT32 free_space;
	free_space = blocks * (cfs->page_size - 0x10);
	if (free_space > 0) free_space -= FILE_HEADER_SIZE;
	return free_space;
}

static int flash_size_to_flash_type( size_t size)
{
	switch (size)
	{
		case 0x084000 : return FLASH_TYPE_AT45DB041;
		case 0x108000 : return FLASH_TYPE_AT45DB081;
		case 0x210000 : return FLASH_TYPE_AT45DB161;
		default       : return FLASH_TYPE_INVALID;
	}
}

static int flash_option_to_flash_type( int option)
{
	switch (option)
	{
		case 0  : return FLASH_TYPE_AT45DB041;
		case 1  : return FLASH_TYPE_AT45DB081;
		case 2  : return FLASH_TYPE_AT45DB161;
		default : return FLASH_TYPE_INVALID;
	}
}

static imgtoolerr_t cybiko_image_open( imgtool_image *image, imgtool_stream *stream)
{
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	int flash_type;
	// init
	flash_type = flash_size_to_flash_type( stream_size( stream));
	if (!cfs_init( cfs, stream, flash_type)) return IMGTOOLERR_CORRUPTIMAGE;
	// verify
	if (!cfs_verify( cfs)) return IMGTOOLERR_CORRUPTIMAGE;
	// ok
	return IMGTOOLERR_SUCCESS;
}

static void cybiko_image_close( imgtool_image *image)
{
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	stream_close( cfs->stream);
}

static imgtoolerr_t cybiko_image_create( imgtool_image *image, imgtool_stream *stream, option_resolution *opts)
{
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	int flash_type;
	// init
	flash_type = flash_option_to_flash_type( option_resolution_lookup_int( opts, 'F'));
	if (!cfs_init( cfs, stream, flash_type)) return IMGTOOLERR_CORRUPTIMAGE;
	// format
	if (!cfs_format( cfs)) return IMGTOOLERR_CORRUPTIMAGE;
	// ok
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t cybiko_image_begin_enum( imgtool_directory *enumeration, const char *path)
{
	cybiko_iter *iter = (cybiko_iter*)imgtool_directory_extrabytes( enumeration);
	iter->block = 0;
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t cybiko_image_next_enum( imgtool_directory *enumeration, imgtool_dirent *ent)
{
	imgtool_image *image = imgtool_directory_image( enumeration);
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	cybiko_iter *iter = (cybiko_iter*)imgtool_directory_extrabytes( enumeration);
	UINT8 buffer[MAX_PAGE_SIZE];
	UINT16 file_id = INVALID_FILE_ID;
	cfs_file file;
	// find next file
	while (iter->block < cfs->block_count_file)
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, iter->block++)) return IMGTOOLERR_READERROR;
		if (BLOCK_USED(buffer) && (BLOCK_PART_ID(buffer) == 0))
		{
			file_id = BLOCK_FILE_ID(buffer);
			break;
		}
	}
	// get file information
	if ((file_id != INVALID_FILE_ID) && cfs_file_info( cfs, file_id, &file))
	{
		strcpy( ent->filename, file.name);
		ent->filesize = file.size;
		ent->lastmodified_time = time_crack( file.date);
		ent->filesize = file.size;
	}
	else
	{
		ent->eof = 1;
	}
	// ok
	return IMGTOOLERR_SUCCESS;
}

static void cybiko_image_close_enum( imgtool_directory *enumeration)
{
	// nothing
}

static imgtoolerr_t cybiko_image_free_space( imgtool_partition *partition, UINT64 *size)
{
	imgtool_image *image = imgtool_partition_image( partition);
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	if (size) *size = cfs_calc_free_space( cfs, cfs_calc_free_blocks( cfs));
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t cybiko_image_read_file( imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf)
{
	imgtool_image *image = imgtool_partition_image( partition);
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	UINT8 buffer[MAX_PAGE_SIZE];
	UINT16 file_id, part_id = 0, old_part_id;
	int i;
	// find file
	if (!cfs_file_find( cfs, filename, &file_id)) return IMGTOOLERR_FILENOTFOUND;
	// read file
	do
	{
		old_part_id = part_id;
		for (i=0;i<cfs->block_count_file;i++)
		{
			if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return IMGTOOLERR_READERROR;
			if (BLOCK_USED(buffer) && (BLOCK_FILE_ID(buffer) == file_id) && (BLOCK_PART_ID(buffer) == part_id))
			{
				stream_write( destf, buffer + 6 + ((part_id == 0) ? FILE_HEADER_SIZE : 0), buffer[1]);
				part_id++;
			}
		}
	} while (old_part_id != part_id);
	// ok
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t cybiko_image_write_file( imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts)
{
	imgtool_image *image = imgtool_partition_image( partition);
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	UINT8 buffer[MAX_PAGE_SIZE];
	UINT16 file_id, part_id = 0, free_blocks;
	UINT64 bytes_left;
	cfs_file file;
	int i;
	// find file
	if (!cfs_file_find( cfs, filename, &file_id)) file_id = INVALID_FILE_ID;
	// check free space
	free_blocks = cfs_calc_free_blocks( cfs);
	if (file_id != INVALID_FILE_ID)
	{
		if (!cfs_file_info( cfs, file_id, &file)) return IMGTOOLERR_UNEXPECTED;
		free_blocks += file.blocks;
	}
	if (cfs_calc_free_space( cfs, free_blocks) < stream_size( sourcef)) return IMGTOOLERR_NOSPACE;
	// delete file
	if (file_id != INVALID_FILE_ID)
	{
		if (!cfs_file_delete( cfs, file_id)) return IMGTOOLERR_UNEXPECTED;
	}
	// create/write destination file
	bytes_left = stream_size( sourcef);
	i = 0;
	while ((bytes_left > 0) && (i < cfs->block_count_file))
	{
		if (!cfs_block_read( cfs, buffer, BLOCK_TYPE_FILE, i)) return IMGTOOLERR_READERROR;
		if (!BLOCK_USED(buffer))
		{
			if (part_id == 0) file_id = i;
			memset( buffer, 0xFF, cfs->page_size - 0x10);
			buffer[0] = 0x80;
			buffer[1] = cfs->page_size - 0x10 - ((part_id == 0) ? FILE_HEADER_SIZE : 0);
			if (bytes_left < buffer[1]) buffer[1] = bytes_left;
			buffer_write_16_be( buffer + 2, file_id);
			buffer_write_16_be( buffer + 4, part_id);
			if (part_id == 0)
			{
				buffer[6] = 0;
				strcpy( BLOCK_FILENAME(buffer), filename);
				buffer_write_32_be( buffer + 6 + FILE_HEADER_SIZE - 4, time_setup( time( NULL)));
				stream_read( sourcef, buffer + 6 + FILE_HEADER_SIZE, buffer[1]);
			}
			else
			{
				stream_read( sourcef, buffer + 6, buffer[1]);
			}
			if (!cfs_block_write( cfs, buffer, BLOCK_TYPE_FILE, i)) return IMGTOOLERR_WRITEERROR;
			bytes_left -= buffer[1];
			part_id++;
		}
		i++;
	}
	// ok
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t cybiko_image_delete_file( imgtool_partition *partition, const char *filename)
{
	imgtool_image *image = imgtool_partition_image( partition);
	cybiko_file_system *cfs = (cybiko_file_system*)imgtool_image_extra_bytes( image);
	UINT16 file_id;
	// find file
	if (!cfs_file_find( cfs, filename, &file_id)) return IMGTOOLERR_FILENOTFOUND;
	// delete file
	if (!cfs_file_delete( cfs, file_id)) return IMGTOOLERR_UNEXPECTED;
	// ok
	return IMGTOOLERR_SUCCESS;
}

static OPTION_GUIDE_START( cybiko_image_createimage_optguide )
	OPTION_ENUM_START( 'F', "flash", "Flash Type" )
		OPTION_ENUM( 0, "AT45DB041", "AT45DB041 (528 KByte)" )
		OPTION_ENUM( 1, "AT45DB081", "AT45DB081 (1056 KByte)" )
		OPTION_ENUM( 2, "AT45DB161", "AT45DB161 (2112 KByte)" )
	OPTION_ENUM_END
OPTION_GUIDE_END

//OPTION_GUIDE_START( cybiko_image_writefile_optguide )
//  OPTION_INT( 'B', "boot", "Boot Flag" )
//OPTION_GUIDE_END

void cybiko_get_info( const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES          : info->i = sizeof( cybiko_file_system); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES      : info->i = sizeof( cybiko_iter); break;
//      case IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME     : info->i = 1; break;
		case IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME : info->i = 1; break;
//      case IMGTOOLINFO_INT_BLOCK_SIZE                 : info->i = 264; break;
		// --- the following bits of info are returned as pointers to data or functions ---
		case IMGTOOLINFO_PTR_OPEN        : info->open        = cybiko_image_open; break;
		case IMGTOOLINFO_PTR_CREATE      : info->create      = cybiko_image_create; break;
		case IMGTOOLINFO_PTR_CLOSE       : info->close       = cybiko_image_close; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM  : info->begin_enum  = cybiko_image_begin_enum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM   : info->next_enum   = cybiko_image_next_enum; break;
		case IMGTOOLINFO_PTR_CLOSE_ENUM  : info->close_enum  = cybiko_image_close_enum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE  : info->free_space  = cybiko_image_free_space; break;
		case IMGTOOLINFO_PTR_READ_FILE   : info->read_file   = cybiko_image_read_file; break;
		case IMGTOOLINFO_PTR_WRITE_FILE  : info->write_file  = cybiko_image_write_file; break;
		case IMGTOOLINFO_PTR_DELETE_FILE : info->delete_file = cybiko_image_delete_file; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE : info->createimage_optguide = cybiko_image_createimage_optguide; break;
//      case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE   : info->writefile_optguide   = cybiko_image_writefile_optguide; break;
		// --- the following bits of info are returned as NULL-terminated strings ---
		case IMGTOOLINFO_STR_NAME            : strcpy( info->s = imgtool_temp_str(), "cybiko"); break;
		case IMGTOOLINFO_STR_DESCRIPTION     : strcpy( info->s = imgtool_temp_str(), "Cybiko Classic File System"); break;
		case IMGTOOLINFO_STR_FILE            : strcpy( info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS : strcpy( info->s = imgtool_temp_str(), "bin,nv"); break;
		case IMGTOOLINFO_STR_EOLN            : strcpy( info->s = imgtool_temp_str(), "\r\n"); break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC : strcpy( info->s = imgtool_temp_str(), "F[0]-2"); break;
//      case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC   : strcpy( info->s = imgtool_temp_str(), "B[0]-1"); break;
	}
}
