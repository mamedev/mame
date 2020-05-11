// license:BSD-3-Clause
// copyright-holders:tim lindner
/****************************************************************************

    dgndos.cpp

    Dragon DOS disk images

****************************************************************************/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "imgtool.h"
#include "formats/coco_dsk.h"
#include "iflopimg.h"

typedef struct __attribute__((packed)) dngdos_sector_allocation_format {
	uint16_t lsn;
	uint8_t count;
} dngdos_sector_allocation_format;

typedef struct __attribute__((packed)) dngdos_file_header_block {
	char filename[11];
	dngdos_sector_allocation_format block[4];
} dngdos_file_header_block;

typedef struct __attribute__((packed)) dngdos_continuation_block {
	dngdos_sector_allocation_format block[7];
	uint16_t unused;
} dngdos_continuation_block;

struct __attribute__((packed)) dgndos_dirent
{
	unsigned char flag_byte;
	union {
		dngdos_file_header_block header_block;
		dngdos_continuation_block continuation_block;
	};
	uint8_t dngdos_last_or_next;
};

struct dgndos_direnum
{
	int index;
	bool eof;
};

/*********************************************************************
    Imgtool module code
*********************************************************************/


#define DGNDOS_OPTIONS_PROTECT 'P'
#define MAX_DIRENTS 160
#define HEADER_EXTENTS_COUNT 4
#define CONT_EXTENTS_COUNT 7
#define MAX_BITMAP_SIZE 512

#define DGNDOS_DELETED_BIT	0x80
#define DGNDOS_CONT_BIT 0x20
#define DGNDOS_END_BIT 0x08
#define DGNDOS_PROTECT_BIT 0x02
#define DGNDOS_ISCONT_BIT 0x01
//-------------------------------------------------
//  get_dgndos_dirent
//-------------------------------------------------

static floperr_t get_dgndos_dirent(imgtool::image &f, int index_loc, dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return (floperr_t)IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	return floppy_read_sector(imgtool_floppy(f), 0, 20, sector, offset, (void *) &ent, sizeof(ent));
}

//-------------------------------------------------
//  put_dgndos_dirent
//-------------------------------------------------

static floperr_t put_dgndos_dirent(imgtool::image &f, int index_loc, const dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return (floperr_t)IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	return floppy_write_sector(imgtool_floppy(f), 0, 20, sector, offset, (void *) &ent, sizeof(ent), 0);
}

//-------------------------------------------------
//  get_dirent_fname
//-------------------------------------------------

static std::string get_dirent_fname(const dgndos_dirent &ent)
{
	return extract_padded_filename(ent.header_block.filename, 8, 3, '\0');
}

static bool dgndos_real_file( dgndos_dirent &ent )
{
	if( ent.flag_byte & DGNDOS_DELETED_BIT) return false;
	if( ent.flag_byte & DGNDOS_ISCONT_BIT) return false;

	return true;
}

//-------------------------------------------------
//  lookup_dgndos_file
//-------------------------------------------------

static imgtoolerr_t lookup_dgndos_file(imgtool::image &f, const char *fname, dgndos_dirent &ent, int *position = nullptr)
{
	int i = 0;
	floperr_t ferr;
	std::string fnamebuf;

	do
	{
		do
		{
			ferr = get_dgndos_dirent( f, i++, ent );

			if( ferr )
				return imgtool_floppy_error(ferr);
		}
		while( ! dgndos_real_file(ent) );

		if( ent.flag_byte & DGNDOS_END_BIT ) return IMGTOOLERR_FILENOTFOUND;

		fnamebuf = get_dirent_fname(ent);
	}
	while(core_stricmp(fnamebuf.c_str(), fname));

	if (position)
		*position = i - 1;

	return (imgtoolerr_t)0;
}

//-------------------------------------------------
//  get_bit_map
//-------------------------------------------------

static floperr_t get_bitmap(imgtool::image &img, uint8_t *bitmap, int *bitmap_count, int *heads, int *tracks_on_disk, int *sectors_per_track)
{
	unsigned int tod, spt, sides;
	floperr_t ferr;

	ferr = floppy_read_sector(imgtool_floppy(img), 0, 20, 1, 0, bitmap, 256);
	ferr = floppy_read_sector(imgtool_floppy(img), 0, 20, 2, 0, bitmap + 256, 256);
	if(ferr) return ferr;

	tod = bitmap[0xfc];
	spt = bitmap[0xfd];

	if( (~tod & 0xff) != bitmap[0xfe])
	{
		fprintf( stderr, "tracks_on_disk check failed: %u == %u\n", (~tod & 0xff), bitmap[0xfe] );
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	if( (~spt & 0xff) != bitmap[0xff])
	{
		fprintf( stderr, "sectors_per_track check failed: %u == %u\n", (~spt & 0xff), bitmap[0xff] );
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	if(spt == 36)
	{
		sides = 1;
	}
	else if(spt == 18 )
	{
		sides = 0;
	}
	else
	{
		fprintf( stderr, "sides check failed\n" );
		return FLOPPY_ERROR_INVALIDIMAGE;
	}

	if( heads ) *heads = sides;
	if( tracks_on_disk) *tracks_on_disk = tod;
	if( sectors_per_track ) *sectors_per_track = spt;
	if( bitmap_count ) *bitmap_count = (tod * sides * 18) + (tod * 18);

	return FLOPPY_ERROR_SUCCESS;
}

//-------------------------------------------------
//  put_bitmap
//-------------------------------------------------

static floperr_t put_bitmap(imgtool::image &img, const uint8_t *bitmap)
{
	floperr_t ferr;

	ferr = floppy_write_sector(imgtool_floppy(img), 0, 20, 1, 0, bitmap, 256, 0);
	ferr = floppy_write_sector(imgtool_floppy(img), 0, 20, 2, 0, bitmap + 256, 256, 0);

	return ferr;
}

// -------------------------------------------------
//  dgndos_convert_side_track_sector
// -------------------------------------------------
//
// static floperr_t dgndos_convert_side_track_sector(imgtool::image &img, int head, int track, int sector, int *lsn )
// {
// 	*lsn = (track * 18 * head) + (track * 18) + sector;
//
// 	return FLOPPY_ERROR_SUCCESS;
// }

static floperr_t dgndos_convert_lsn(imgtool::image &img, int lsn, int *head, int *track, int *sector )
{
	uint8_t buffer[MAX_BITMAP_SIZE];
	int tracks_on_disk;
	int sectors_per_track;
	int sides;

	floperr_t ferr = get_bitmap(img, buffer, nullptr, &sides, &tracks_on_disk, &sectors_per_track);
	if(ferr) return ferr;

	if( sides == 0 )
	{
		*head = 0;
		*track = lsn / 18;
		*sector = (lsn % 18) + 1;
	}
	else
	{
		*head = (lsn / 18) % 2;
		*track = lsn / 36;
		*sector = (lsn % 18) + 1;
	}

	return FLOPPY_ERROR_SUCCESS;
}

static int dgndos_is_sector_avaiable(imgtool::image &img, int lsn)
{
	int startbyte, startbit;
	uint8_t bitmap[MAX_BITMAP_SIZE];

	floperr_t ferr = get_bitmap(img, bitmap, nullptr, nullptr, nullptr, nullptr);
	if(ferr) return ferr;

	startbyte = lsn / 8;
	startbit = lsn % 8;

	if( lsn > 1439 )
	{
		startbyte += 76;
	}

	return bitmap[startbyte] & (1 << (7 - startbit));
}

static floperr_t dgndos_fat_allocate_sector(imgtool::image &img, int lsn)
{
	int startbyte, startbit;
	uint8_t bitmap[MAX_BITMAP_SIZE];

	floperr_t ferr = get_bitmap(img, bitmap, nullptr, nullptr, nullptr, nullptr);
	if(ferr) return ferr;

	startbyte = lsn / 8;
	startbit = lsn % 8;

	if( lsn > 1439 )
	{
		startbyte += 76;
	}

	bitmap[startbyte] |= (1 << (7 - startbit++));

	ferr = put_bitmap(img, bitmap);
	return ferr;
}

static floperr_t dgndos_fat_deallocate_sector(imgtool::image &img, int lsn)
{
	int startbyte, startbit;
	uint8_t bitmap[MAX_BITMAP_SIZE];

	floperr_t ferr = get_bitmap(img, bitmap, nullptr, nullptr, nullptr, nullptr);
	if(ferr) return ferr;

	startbyte = lsn / 8;
	startbit = lsn % 8;

	if( lsn > 1439 )
	{
		startbyte += 76;
	}

	bitmap[startbyte] &= ~(1 << (7 - startbit));

	ferr = put_bitmap(img, bitmap);

	return ferr;
}

static floperr_t dgndos_fat_deallocate_span(imgtool::image &img, int lsn, int count)
{
	floperr_t ferr;

	for( int i=lsn; i<lsn+count; i++)
	{
		ferr = dgndos_fat_deallocate_sector(img, i);
	}

	return ferr;
}

static floperr_t dgndos_get_avaiable_dirent(imgtool::image img, dgndos_dirent &ent, int *position )
{
	int i;
	floperr_t ferr;

	for( i=0; i<MAX_DIRENTS; i++ )
	{

		ferr = get_dgndos_dirent(img, i, ent);
		if( ferr ) return ferr;

		if( ent.flag_byte & DGNDOS_DELETED_BIT)
		{
			break;
		}

		if( ent.flag_byte & DGNDOS_END_BIT)
		{
			if( i<MAX_DIRENTS-1)
			{
				dgndos_dirent next_ent;
				ferr = get_dgndos_dirent(img, i+1, next_ent);
				if( ferr ) return ferr;
				next_ent.flag_byte |= DGNDOS_END_BIT;
				put_dgndos_dirent(img, i+1, next_ent);
				if( ferr ) return ferr;
			}

			ent.flag_byte &= ~DGNDOS_END_BIT;
			break;
		}
	}

	if( i == MAX_DIRENTS )
	{
		return FLOPPY_ERROR_NOSPACE;
	}

	if( position ) *position = i;
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t dgndos_get_avaiable_sector( imgtool::image img, int *lsn )
{
	int bitmap_count, i;
	uint8_t bitmap[MAX_BITMAP_SIZE];


	floperr_t ferr = get_bitmap(img, bitmap, &bitmap_count, nullptr, nullptr, nullptr);
	if(ferr) return ferr;

	for( i=1; i < bitmap_count; i++ )
	{
		if( dgndos_is_sector_avaiable(img, i) )
		{
			*lsn = i;
			break;
		}
	}

	if( i == bitmap_count) return FLOPPY_ERROR_NOSPACE;

	return FLOPPY_ERROR_SUCCESS;
}


imgtoolerr_t dgndos_get_file_size(imgtool::image &image, dgndos_dirent *dgnent, size_t &filesize)
{
	floperr_t ferr;
	dgndos_dirent cont_ent;
	filesize = 0;
	int i = 0;
	int directory_entry_count = 0;

	do
	{
		if( i==HEADER_EXTENTS_COUNT) break;

		filesize += dgnent->header_block.block[i].count * 256;
	}
	while (big_endianize_int16(dgnent->header_block.block[i++].lsn) != 0 );

	if(i == HEADER_EXTENTS_COUNT && (dgnent->flag_byte & DGNDOS_CONT_BIT))
	{
		ferr = get_dgndos_dirent(image, dgnent->dngdos_last_or_next, cont_ent );
		return imgtool_floppy_error(ferr);

		i = 0;

		do
		{
			if( directory_entry_count > MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;

			if( i == CONT_EXTENTS_COUNT ) break;

			filesize += cont_ent.continuation_block.block[i].count * 256;

			if( i == CONT_EXTENTS_COUNT-1 )
			{
				if( cont_ent.flag_byte & DGNDOS_CONT_BIT)
				{
					ferr = get_dgndos_dirent(image, cont_ent.dngdos_last_or_next, cont_ent );
					return imgtool_floppy_error(ferr);
					i = -1;
					directory_entry_count++;
				}
			}
		}
		while (big_endianize_int16(cont_ent.continuation_block.block[i++].lsn) != 0 );

		filesize -= 256;
		if( cont_ent.dngdos_last_or_next == 0 ) filesize += 256;
		else filesize += cont_ent.dngdos_last_or_next;
	}
	else
	{
		filesize -= 256;
		if( dgnent->dngdos_last_or_next == 0 ) filesize += 256;
		else filesize += dgnent->dngdos_last_or_next;
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  dgndos_diskimage_nextenum
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	floperr_t ferr;
	imgtoolerr_t err;
	size_t filesize;
	dgndos_direnum *dgnenum;
	dgndos_dirent dgnent;

	imgtool::image &image(enumeration.image());
	dgnenum = (dgndos_direnum *) enumeration.extra_bytes();

	/* Did we hit the end of file before? */
	if (dgnenum->eof)
		goto eof;

	do
	{
		if (dgnenum->index >= MAX_DIRENTS)
			goto eof;

		ferr = get_dgndos_dirent(image, dgnenum->index++, dgnent);
		if (ferr)
			return imgtool_floppy_error(ferr);
	}
	while((dgnent.flag_byte & DGNDOS_DELETED_BIT) == DGNDOS_DELETED_BIT);

	// now are we at the eof point?
	if (dgnent.flag_byte & DGNDOS_END_BIT)
	{
		dgnenum->eof = 1;
eof:
		ent.eof = 1;
	}
	else
	{
 		err = dgndos_get_file_size(image, &dgnent, filesize);
		if (filesize == ((size_t) -1))
		{
			/* corrupt! */
			ent.filesize = 0;
			ent.corrupt = 1;
		}
		else
		{
			ent.filesize = filesize;
			ent.corrupt = 0;
		}
		ent.eof = 0;

		std::string fname = get_dirent_fname(dgnent);

		snprintf(ent.filename, ARRAY_LENGTH(ent.filename), "%s", fname.c_str());
		snprintf(ent.attr, ARRAY_LENGTH(ent.attr), "%c", (char) (dgnent.flag_byte & DGNDOS_PROTECT_BIT ? 'P' : '.'));
	}
	return IMGTOOLERR_SUCCESS;
}

//-------------------------------------------------
//  dgndos_diskimage_freespace
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_freespace(imgtool::partition &partition, uint64_t *size)
{
	uint8_t bitmap[MAX_BITMAP_SIZE];
	imgtool::image &img(partition.image());
	int bitmap_count;

	*size = 0;

	get_bitmap(img, bitmap, &bitmap_count, nullptr, nullptr, nullptr);

	fprintf( stderr,"bitmap cont: %d\n", bitmap_count);

	for(int i=0; i<bitmap_count; i++)
	{
		if (dgndos_is_sector_avaiable(img, i))
		{
			*size += 256;
		}
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  dgndos_diskimage_readfile
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_readfile(imgtool::partition &partition, const char *fname, const char *fork, imgtool::stream &destf)
{
	floperr_t ferr;
	imgtoolerr_t err;
	dgndos_dirent cont_ent;
	size_t block_size;
	int i = 0;
	int directory_entry_count = 0;
	imgtool::image &img(partition.image());
	int head, track, sector;

	err = lookup_dgndos_file(img, fname, cont_ent);
	if (err) return err;

	do
	{
		if( i==HEADER_EXTENTS_COUNT ) break;

		for( int j=big_endianize_int16(cont_ent.header_block.block[i].lsn); j<big_endianize_int16(cont_ent.header_block.block[i].lsn) + cont_ent.header_block.block[i].count; j++ )
		{
			block_size = 256;
			ferr = dgndos_convert_lsn(img, j, &head, &track, &sector );
			if(ferr) return imgtool_floppy_error(ferr);

			// special case last sector in extent
			if( j == big_endianize_int16(cont_ent.header_block.block[i].lsn) + cont_ent.header_block.block[i].count - 1)
			{
				// is this the last sector of the file
				if( i==3 && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
				{
					// There is another block
				}
				else if( big_endianize_int16(cont_ent.header_block.block[i+1].lsn) != 0 )
				{
					// There is another block
				}
				else
				{
					// This is the last block
					if( cont_ent.dngdos_last_or_next == 0 ) block_size = 256;
					else block_size = cont_ent.dngdos_last_or_next;
				}
			}

			err = imgtool_floppy_read_sector_to_stream(img, head, track, sector, 0, block_size, destf);
			if (err) return err;
		}
	}
	while (big_endianize_int16(cont_ent.header_block.block[i++].lsn) != 0 );

	if(i == HEADER_EXTENTS_COUNT && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
	{
		ferr = get_dgndos_dirent(img, cont_ent.dngdos_last_or_next, cont_ent );
		if(ferr) return imgtool_floppy_error(ferr);

		i = 0;

		do
		{
			if( directory_entry_count > MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;

			if( i == CONT_EXTENTS_COUNT ) break;

			for( int j=big_endianize_int16(cont_ent.continuation_block.block[i].lsn); j<big_endianize_int16(cont_ent.continuation_block.block[i].lsn) + cont_ent.continuation_block.block[i].count; j++ )
			{
				block_size = 256;
				ferr = dgndos_convert_lsn(img, j, &head, &track, &sector );
				if(ferr) return imgtool_floppy_error(ferr);

				// special case last sector in extent
				if( j == big_endianize_int16(cont_ent.continuation_block.block[i].lsn) + cont_ent.continuation_block.block[i].count - 1)
				{
					// is this the last sector of the file
					if( i==6 && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
					{
						// There is another block
					}
					else if( big_endianize_int16(cont_ent.continuation_block.block[i+1].lsn) != 0 )
					{
						// There is another block
					}
					else
					{
						// This is the last block
						if( cont_ent.dngdos_last_or_next == 0 ) block_size = 256;
						else block_size = cont_ent.dngdos_last_or_next;
					}
				}

				err = imgtool_floppy_read_sector_to_stream(img, head, track, sector, 0, block_size, destf);
			}

			if( i == 6 )
			{
				if( cont_ent.flag_byte & DGNDOS_CONT_BIT)
				{
					ferr = get_dgndos_dirent(img, cont_ent.dngdos_last_or_next, cont_ent );
					if(ferr) return imgtool_floppy_error(ferr);
					i = -1;
					directory_entry_count++;
				}
			}
		}
		while (big_endianize_int16(cont_ent.continuation_block.block[i++].lsn) != 0 );
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  dgndos_diskimage_deletefile
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_deletefile(imgtool::partition &partition, const char *fname)
{
	imgtoolerr_t err;
	floperr_t ferr;
	imgtool::image &image(partition.image());
	int pos;
	struct dgndos_dirent ent;
	int directory_entry_count = 0;

	err = lookup_dgndos_file(image, fname, ent, &pos);
	if (err) return err;

	ent.flag_byte |= DGNDOS_DELETED_BIT;

	for( int i=0; i<HEADER_EXTENTS_COUNT; i++)
	{
		if( big_endianize_int16(ent.header_block.block[i].lsn) != 0)
		{
			ferr = dgndos_fat_deallocate_span(image, big_endianize_int16(ent.header_block.block[i].lsn), ent.header_block.block[i].count);
			if (ferr) return imgtool_floppy_error(ferr);
			ent.header_block.block[i].lsn = 0;
			ent.header_block.block[i].count = 0;
		}
	}

	int continue_flag = ent.flag_byte & DGNDOS_ISCONT_BIT;
	int continue_dirent_index = ent.dngdos_last_or_next;

	ent.dngdos_last_or_next = 0;
	ferr = put_dgndos_dirent(image, pos, ent);
	if (ferr) return imgtool_floppy_error(ferr);

	while( continue_flag )
	{
		if( directory_entry_count > MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;

		int dirent_index = continue_dirent_index;
		ferr = get_dgndos_dirent(image, dirent_index, ent);
		if (ferr) return imgtool_floppy_error(ferr);

		for( int i=0; i<CONT_EXTENTS_COUNT; i++)
		{
			if(big_endianize_int16(ent.continuation_block.block[i].lsn) !=0)
			{
				ferr = dgndos_fat_deallocate_span(image, big_endianize_int16(ent.continuation_block.block[i].lsn), ent.continuation_block.block[i].count);
				if (ferr) return imgtool_floppy_error(ferr);

				ent.continuation_block.block[i].lsn = 0;
				ent.continuation_block.block[i].count = 0;
			}
		}

		ent.flag_byte |= DGNDOS_DELETED_BIT;
		continue_flag = ent.flag_byte & DGNDOS_ISCONT_BIT;
		continue_dirent_index = ent.dngdos_last_or_next;

		ent.dngdos_last_or_next = 0;
		ferr = put_dgndos_dirent(image, dirent_index, ent);
		if (ferr) return imgtool_floppy_error(ferr);

		directory_entry_count++;
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  dgndos_diskimage_writefile
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_writefile(imgtool::partition &partition, const char *fname, const char *fork, imgtool::stream &sourcef, util::option_resolution *writeoptions)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool::image &img(partition.image());
	dgndos_dirent ent;
	int position;
	uint64_t written = 0;
	int fat_block, block_index, first_lsn, sectors_avaiable, current_sector_index;
	int last_sector_size = 0;

	// find directory entry with same file name
	err = lookup_dgndos_file(img, fname, ent, &position);

	if( err == IMGTOOLERR_FILENOTFOUND )
	{
		/* todo create new file */
	}

	if( writeoptions->lookup_int(DGNDOS_OPTIONS_PROTECT))
	{
		ent.flag_byte |= DGNDOS_PROTECT_BIT;
	}
	else
	{
		ent.flag_byte &= ~DGNDOS_PROTECT_BIT;
	}

	fat_block = 0;
	block_index = 0;

	/* get next available fat entry */
	first_lsn = big_endianize_int16(ent.header_block.block[fat_block].lsn);
	sectors_avaiable = ent.header_block.block[fat_block].count;
	current_sector_index = 0;

	do
	{
		while ((current_sector_index < sectors_avaiable) && (written < sourcef.size()))
		{
			int head, track, sector, write_count;

			ferr = dgndos_convert_lsn(img, first_lsn + current_sector_index, &head, &track, &sector );
			if (ferr) return imgtool_floppy_error(ferr);

			if( sourcef.size() - written < 256 )
			{
				write_count = sourcef.size() - written;
				last_sector_size = write_count;
			}
			else
			{
				write_count = 256;
			}

			err = imgtool_floppy_write_sector_from_stream(img, head, track, sector, 0, write_count, sourcef);
			if (ferr) return err;

			current_sector_index++;
			written += write_count;
		}

		if( written == sourcef.size()) // are we are done writing file
		{
			int next_de = 0;

			// yes, truncate this allocation block if necessary
			for( int i = current_sector_index; i < ent.header_block.block[fat_block].count; i++)
			{
				ferr = dgndos_fat_deallocate_sector(img, first_lsn + current_sector_index);
				if (ferr) return imgtool_floppy_error(ferr);
			}

			ent.header_block.block[fat_block].count = current_sector_index - 1;

			// flag if there are more DEs
			if( ent.flag_byte & DGNDOS_CONT_BIT)
			{
				next_de = ent.dngdos_last_or_next;
			}

			do // check remaining allocation blocks and zero them
			{
				int first_fat_block;
				fat_block++;
				first_fat_block = fat_block;

				if( ent.flag_byte & DGNDOS_ISCONT_BIT)
				{
					for( int i = fat_block; i < CONT_EXTENTS_COUNT; i++ )
					{
						ferr = dgndos_fat_deallocate_span(img, big_endianize_int16(ent.continuation_block.block[i].lsn), ent.continuation_block.block[i].count);
						if (ferr) return imgtool_floppy_error(ferr);
						ent.continuation_block.block[i].lsn = big_endianize_int16(0);
						ent.continuation_block.block[i].count = 0;
					}
				}
				else
				{
					for( int i = fat_block; i < HEADER_EXTENTS_COUNT; i++ )
					{
						ferr = dgndos_fat_deallocate_span(img, big_endianize_int16(ent.header_block.block[i].lsn), ent.header_block.block[i].count);
						if (ferr) return imgtool_floppy_error(ferr);
						ent.header_block.block[i].lsn = big_endianize_int16(0);
						ent.header_block.block[i].count = 0;
					}
				}

				// delete DE if all blocks were cleared and it is a continued DE
				if( (first_fat_block == 0) && (ent.flag_byte & DGNDOS_ISCONT_BIT) )
				{
					ent.flag_byte |= DGNDOS_DELETED_BIT;
				}

				ferr = put_dgndos_dirent(img, position, ent);
				if (ferr) return imgtool_floppy_error(ferr);

				if( next_de != 0 )
				{
					ferr = get_dgndos_dirent(img, next_de, ent);
					if (ferr) return imgtool_floppy_error(ferr);

					// flag if there are more DEs
					if( ent.flag_byte & DGNDOS_CONT_BIT)
					{
						next_de = ent.dngdos_last_or_next;
						fat_block = -1;
					}
				}
			}
			while( next_de != 0 );
		}
		else // more to write - check if I can extend the allocation count
		{
			if( (current_sector_index < 256) && (dgndos_is_sector_avaiable( img, first_lsn + current_sector_index )) )
			{
				sectors_avaiable++;
				ent.header_block.block[fat_block].count = sectors_avaiable;
				ferr = dgndos_fat_allocate_sector( img, first_lsn + current_sector_index );
				if (ferr) return imgtool_floppy_error(ferr);
			}
			else // this allocation list is full - check next allocation list
			{
				fat_block++;

				if( ((ent.flag_byte & DGNDOS_ISCONT_BIT) && fat_block == CONT_EXTENTS_COUNT) || (!(ent.flag_byte & DGNDOS_ISCONT_BIT) && fat_block == HEADER_EXTENTS_COUNT))
				{
					// need a or another continuation directory entry
					ferr = put_dgndos_dirent(img, position, ent);
					if (ferr) return imgtool_floppy_error(ferr);

					ferr = dgndos_get_avaiable_dirent( img, ent, &position );
					if (ferr)
					{
						fprintf( stderr, "to do: handle disk full condition gracefully\n" );
						return imgtool_floppy_error(ferr);
					}

					memset( (void *)&ent, 0, sizeof(dgndos_dirent) );
					ent.flag_byte |= DGNDOS_ISCONT_BIT;
					fat_block = 0;
				}

				first_lsn = big_endianize_int16(ent.header_block.block[fat_block].lsn);

				// check if this block is empty
				if( first_lsn == 0)
				{
					int lsn;

					ferr = dgndos_get_avaiable_sector( img, &lsn );
					if (ferr)
					{
						fprintf( stderr, "to do: handle disk full condition gracefully\n" );
						return imgtool_floppy_error(ferr);
					}

					ent.header_block.block[fat_block].lsn = big_endianize_int16(lsn);
					ent.header_block.block[fat_block].count = 1;
				}

				sectors_avaiable = ent.header_block.block[fat_block].count;
				current_sector_index = 0;
			}
		}
	}
	while( written < sourcef.size() );

	return IMGTOOLERR_SUCCESS;
}



//-------------------------------------------------
//  dgndos_diskimage_suggesttransfer
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_suggesttransfer(imgtool::partition &partition, const char *fname, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	imgtool::image &image(partition.image());
	struct dgndos_dirent ent;
	int pos;

	if (fname)
	{
		err = lookup_dgndos_file(image, fname, ent, &pos);
		if (err)
			return err;

		if (strcmp(ent.header_block.filename+8,"DAT") == 0)
		{
			/* ASCII file */
			suggestions[0].viability = SUGGESTION_RECOMMENDED;
			suggestions[0].filter = filter_eoln_getinfo;
			suggestions[1].viability = SUGGESTION_POSSIBLE;
			suggestions[1].filter = NULL;
		}
		else if (strcmp(ent.header_block.filename+8,"BAS") == 0)
		{
			/* tokenized BASIC file */
			suggestions[0].viability = SUGGESTION_RECOMMENDED;
			suggestions[0].filter = NULL;
			suggestions[1].viability = SUGGESTION_POSSIBLE;
			suggestions[1].filter = filter_dragonbas_getinfo;
		}
	}
	else
	{
		suggestions[0].viability = SUGGESTION_RECOMMENDED;
		suggestions[0].filter = NULL;
		suggestions[1].viability = SUGGESTION_POSSIBLE;
		suggestions[1].filter = filter_eoln_getinfo;
		suggestions[2].viability = SUGGESTION_POSSIBLE;
		suggestions[2].filter = filter_dragonbas_getinfo;
	}

	return IMGTOOLERR_SUCCESS;
}

/*********************************************************************
    Imgtool module declaration
*********************************************************************/

OPTION_GUIDE_START( dragon_dgndos_writefile_optionguide )
	OPTION_ENUM_START(  DGNDOS_OPTIONS_PROTECT, "prot", "Protect file" )
		OPTION_ENUM(    0,      "no",        "No" )
		OPTION_ENUM(    1,      "yes",         "Yes" )
	OPTION_ENUM_END
OPTION_GUIDE_END

void dgndos_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_PREFER_UCASE:                  info->i = 1; break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:         info->i = sizeof(struct dgndos_direnum); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "dgndos"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Dragon DOS format"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_EOLN:                          strcpy(info->s = imgtool_temp_str(), "\r"); break;
		case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:             strcpy(info->s = imgtool_temp_str(), "P[0]-1"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_MAKE_CLASS:                    info->make_class = imgtool_floppy_make_class; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum = dgndos_diskimage_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space = dgndos_diskimage_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file = dgndos_diskimage_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file = dgndos_diskimage_writefile; break;
		case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = dgndos_diskimage_deletefile; break;
		case IMGTOOLINFO_PTR_SUGGEST_TRANSFER:              info->suggest_transfer = dgndos_diskimage_suggesttransfer; break;
		case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:            info->writefile_optguide = &dragon_dgndos_writefile_optionguide; break;
		case IMGTOOLINFO_PTR_FLOPPY_FORMAT:                 info->p = (void *) floppyoptions_coco; break;
	}
}






