// license:BSD-3-Clause
// copyright-holders:tim lindner
/****************************************************************************

    dgndos.cpp

    Dragon DOS disk images

    I am not happy with the sector allocation algorithm

****************************************************************************/

#include "imgtool.h"
#include "filter.h"
#include "iflopimg.h"

#include "formats/coco_dsk.h"
#include "corestr.h"
#include "opresolv.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _MSC_VER
#pragma pack(push,1)
#define A_PACKED
#else
#define A_PACKED __attribute__((packed))
#endif

typedef struct A_PACKED dngdos_sector_allocation_format {
	uint16_t lsn;
	uint8_t count;
} dngdos_sector_allocation_format;

typedef struct A_PACKED dngdos_file_header_block {
	char filename[11];
	dngdos_sector_allocation_format block[4];
} dngdos_file_header_block;

typedef struct A_PACKED dngdos_file_continuation_block {
	dngdos_sector_allocation_format block[7];
	uint16_t unused;
} dngdos_file_continuation_block;

struct A_PACKED dgndos_dirent
{
	unsigned char flag_byte;
	union block {
		dngdos_file_header_block header;
		dngdos_file_continuation_block continuation;
	} block;
	uint8_t dngdos_last_or_next;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

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

#define DGNDOS_DELETED_BIT  0x80       // deleted entry
#define DGNDOS_CONTINUED_BIT 0x20      // byte at offset 0x18 give next entry number
#define DGNDOS_END_BIT 0x08            // end of directory
#define DGNDOS_PROTECT_BIT 0x02        // ignored
#define DGNDOS_CONTINUATION_BIT 0x01   // this is a continuation block

static imgtoolerr_t get_dgndos_dirent(uint8_t *track, int index_loc, dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	memcpy( (void *)&ent, track + (256 * (sector-1)) + offset, sizeof(ent));

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t put_dgndos_dirent(uint8_t *track, int index_loc, const dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	memcpy( track + (256 * (sector-1)) + offset, (void *)&ent, sizeof(ent));

	return IMGTOOLERR_SUCCESS;
}

static std::string get_dirent_fname(const dgndos_dirent &ent)
{
	return extract_padded_filename(ent.block.header.filename, 8, 3, '\0');
}

static bool dgndos_real_file( dgndos_dirent &ent )
{
	if( ent.flag_byte & DGNDOS_DELETED_BIT) return false;
	if( ent.flag_byte & DGNDOS_CONTINUATION_BIT) return false;

	return true;
}

static imgtoolerr_t lookup_dgndos_file(uint8_t *entire_track, const char *fname, dgndos_dirent &ent, int *position = nullptr)
{
	int i = 0;
	imgtoolerr_t err;
	std::string fnamebuf;

	do
	{
		do
		{
			err = get_dgndos_dirent( entire_track, i++, ent );
			if( err ) return err;

			if( ent.flag_byte & DGNDOS_END_BIT ) return IMGTOOLERR_FILENOTFOUND;
		}
		while( ! dgndos_real_file(ent) );

		fnamebuf = get_dirent_fname(ent);
	}
	while(core_stricmp(fnamebuf.c_str(), fname));

	if (position)
		*position = i - 1;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_get_geometry(uint8_t *entire_track, int *bitmap_count, int *heads, int *tracks_on_disk, int *sectors_per_track)
{
	unsigned int tod, spt, sides;

	tod = entire_track[0xfc];
	spt = entire_track[0xfd];

	if( (~tod & 0xff) != entire_track[0xfe])
	{
//      fprintf( stderr, "tracks_on_disk check failed: %u == %u\n", (~tod & 0xff), entire_track[0xfe] );
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	if( (~spt & 0xff) != entire_track[0xff])
	{
//      fprintf( stderr, "sectors_per_track check failed: %u == %u\n", (~spt & 0xff), entire_track[0xff] );
		return IMGTOOLERR_CORRUPTIMAGE;
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
//      fprintf( stderr, "sides check failed\n" );
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	if( heads ) *heads = sides;
	if( tracks_on_disk) *tracks_on_disk = tod;
	if( sectors_per_track ) *sectors_per_track = spt;
	if( bitmap_count ) *bitmap_count = (tod * sides * 18) + (tod * 18);

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_convert_lsn(uint8_t *entire_track, int lsn, int *head, int *track, int *sector )
{
	int tracks_on_disk;
	int sectors_per_track;
	int sides;

	imgtoolerr_t err = dgndos_get_geometry(entire_track, nullptr, &sides, &tracks_on_disk, &sectors_per_track);
	if(err) return err;

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

	return IMGTOOLERR_SUCCESS;
}

#define ALLOCATE_BIT 1
#define QUERY_BIT 0
#define DEALLOCATE_BIT -1

static int dgndos_set_reset_bitmap( uint8_t *entire_track, int lsn, int set )
{
	int startbyte  = lsn / 8;
	int startbit = lsn % 8;

	if( lsn > 1439 ) startbyte += 76;

	if(set > QUERY_BIT) // ALLOCATE_BIT
	{
		// 0 = used, 1 = free
		entire_track[startbyte] &= ~(1 << startbit);
	}
	else if( set < QUERY_BIT ) // DEALLOCATE_BIT
	{
		// 0 = used, 1 = free
		entire_track[startbyte] |= (1 << startbit);
	}

	// 0 = used, 1 = free
	return (entire_track[startbyte] & (1 << startbit)) != 0;
}

static int dgndos_is_sector_avaiable(uint8_t *entire_track, int lsn)
{
	return dgndos_set_reset_bitmap( entire_track, lsn, QUERY_BIT );
}

static int dgndos_fat_allocate_sector(uint8_t *entire_track, int lsn)
{
	return dgndos_set_reset_bitmap( entire_track, lsn, ALLOCATE_BIT );
}

static int dgndos_fat_deallocate_sector(uint8_t *entire_track, int lsn)
{
	return dgndos_set_reset_bitmap( entire_track, lsn, DEALLOCATE_BIT );
}

static int dgndos_fat_deallocate_span(uint8_t *entire_track, int lsn, int count)
{
	int value = 0;

	for( int i=lsn; i<lsn+count; i++)
	{
		value = dgndos_fat_deallocate_sector(entire_track, i);
	}

	return value;
}

static imgtoolerr_t dgndos_get_avaiable_dirent_position(uint8_t *entire_track, int *position )
{
	int i;
	imgtoolerr_t err;
	dgndos_dirent ent;

	for( i=0; i<MAX_DIRENTS; i++ )
	{
		err = get_dgndos_dirent(entire_track, i, ent);
		if( err ) return err;

		if( ent.flag_byte & DGNDOS_DELETED_BIT) break;

		if( ent.flag_byte & DGNDOS_END_BIT) break;
	}

	if( i == MAX_DIRENTS )
	{
		return IMGTOOLERR_NOSPACE;
	}

	if( position ) *position = i;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_get_avaiable_dirent(uint8_t *entire_track, dgndos_dirent &ent, int *position )
{
	int i;
	imgtoolerr_t err;

	for( i=0; i<MAX_DIRENTS; i++ )
	{
		err = get_dgndos_dirent(entire_track, i, ent);
		if( err ) return err;

		if( ent.flag_byte & DGNDOS_DELETED_BIT) break;

		if( ent.flag_byte & DGNDOS_END_BIT) break;
	}

	if( i == MAX_DIRENTS )
	{
		return IMGTOOLERR_NOSPACE;
	}

	if( position ) *position = i;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_get_avaiable_sector( uint8_t *entire_track, int *lsn )
{
	int bitmap_count, i;
	int pos_diff = 18;
	int neg_diff = 36;
	int start = 360;

	imgtoolerr_t err = dgndos_get_geometry(entire_track, &bitmap_count, nullptr, nullptr, nullptr);
	if(err) return err;

	do
	{
		i = start;
		while ( i >= 0 )
		{
			if( dgndos_is_sector_avaiable(entire_track, i) )
			{
				*lsn = i;
				return IMGTOOLERR_SUCCESS;
			}

			i -= neg_diff;
		}

		i = start;
		while( i < bitmap_count )
		{
			if( dgndos_is_sector_avaiable(entire_track, i) )
			{
				*lsn = i;
				return IMGTOOLERR_SUCCESS;
			}

			i += pos_diff;
		}

		start--;
	}
	while( start >= 335 );

	return IMGTOOLERR_NOSPACE;
}

static imgtoolerr_t dgndos_count_dirents(uint8_t *entire_track, dgndos_dirent dgnent, int *result)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	*result = 1;

	while( (dgnent.flag_byte & DGNDOS_CONTINUED_BIT) && *result < MAX_DIRENTS )
	{
		(*result)++;
		err = get_dgndos_dirent(entire_track, dgnent.dngdos_last_or_next, dgnent);
	}

	return err;
}

static imgtoolerr_t dgndos_get_file_size(uint8_t *entire_track, dgndos_dirent dgnent, size_t &filesize)
{
	imgtoolerr_t err;
	int directory_entry_count = 0;
	int extent = 0;
	int block_size;
	int done;
	int count;
	int i;

	filesize = 0;

	do
	{
		if(directory_entry_count>MAX_DIRENTS) return IMGTOOLERR_CORRUPTDIR;

		count = dgnent.flag_byte & DGNDOS_CONTINUATION_BIT ? dgnent.block.continuation.block[extent].count : dgnent.block.header.block[extent].count;

		if( count == 0 ) break;

		i = 0;

		// count extents except last sector
		while (i < count - 1)
		{
			filesize += 256;
			i++;
		}

		block_size = 256;
		done = false;

		if( extent < (dgnent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) - 1 )
		{
			if( (dgnent.flag_byte & DGNDOS_CONTINUATION_BIT ? dgnent.block.continuation.block[extent+1].count : dgnent.block.header.block[extent+1].count) == 0 )
			{
				// not last extent, and yes continuation
				if( dgnent.dngdos_last_or_next == 0 )
					block_size = 256;
				else
					block_size = dgnent.dngdos_last_or_next;

				done = true;
			}
		}
		else if( extent == (dgnent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) - 1)
		{
			if( !(dgnent.flag_byte & DGNDOS_CONTINUED_BIT) )
			{
				// is last extent, and no continuation
				if( dgnent.dngdos_last_or_next == 0 )
					block_size = 256;
				else
					block_size = dgnent.dngdos_last_or_next;

				done = true;
			}
		}

		filesize += block_size;

		extent++;

		if( extent == (dgnent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) )
		{
			if( dgnent.flag_byte & DGNDOS_CONTINUED_BIT)
			{
				err = get_dgndos_dirent(entire_track, dgnent.dngdos_last_or_next, *(&dgnent) );
				if(err) return err;

				extent = 0;
				directory_entry_count++;
			}
		}
	}
	while( !done );

	return IMGTOOLERR_SUCCESS;
}

static floperr_t dgndos_get_directory_track( imgtool::image &image, int track, uint8_t *buffer )
{
	floperr_t ferr;

	for(int i=1; i<=18; i++ )
	{
		ferr = floppy_read_sector(imgtool_floppy(image), 0, track, i, 0, &(buffer[(i-1)*256]), 256);
		if(ferr) break;
	}

	return ferr;
}

static floperr_t dgndos_put_directory_track( imgtool::image &image, int track, uint8_t *buffer )
{
	floperr_t ferr;

	for(int i=1; i<=18; i++ )
	{
		ferr = floppy_write_sector(imgtool_floppy(image), 0, track, i, 0, &(buffer[(i-1)*256]), 256, 0);
		if(ferr) break;
	}

	return ferr;
}

static imgtoolerr_t dgndos_diskimage_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	floperr_t ferr;
	imgtoolerr_t err;
	size_t filesize;
	dgndos_direnum *dgnenum;
	dgndos_dirent dgnent;
	int dir_ent_count;

	imgtool::image &image(enumeration.image());

	uint8_t entire_track20[18*256];

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	dgnenum = (dgndos_direnum *) enumeration.extra_bytes();

	/* Did we hit the end of file before? */
	if (dgnenum->eof) return IMGTOOLERR_SUCCESS;

	do
	{
		if (dgnenum->index >= MAX_DIRENTS)
		{
			dgnenum->eof = 1;
			ent.eof = 1;
			return IMGTOOLERR_SUCCESS;
		}

		err = get_dgndos_dirent(entire_track20, dgnenum->index++, dgnent);
		if (err) return err;

		if( dgndos_real_file( dgnent ) ) break;
	}
	while( ! (dgnent.flag_byte & DGNDOS_END_BIT) );

	// now are we at the eof point?
	if (dgnent.flag_byte & DGNDOS_END_BIT)
	{
		dgnenum->eof = 1;
		ent.eof = 1;
	}
	else
	{
		err = dgndos_get_file_size(entire_track20, dgnent, filesize);
		if (err) return err;

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

		err = dgndos_count_dirents(entire_track20, dgnent, &dir_ent_count);
		if (err) return err;

		snprintf(ent.filename, std::size(ent.filename), "%s", fname.c_str());
		snprintf(ent.attr, std::size(ent.attr), "%c (%03d)",
			(char) (dgnent.flag_byte & DGNDOS_PROTECT_BIT ? 'P' : '.'),
			dir_ent_count);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_diskimage_freespace(imgtool::partition &partition, uint64_t *size)
{
	floperr_t ferr;
	imgtool::image &image(partition.image());
	int bitmap_count(0);

	uint8_t entire_track20[18*256];

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	*size = 0;

	dgndos_get_geometry(entire_track20, &bitmap_count, nullptr, nullptr, nullptr);

	for( int i=0; i<bitmap_count; i++ )
	{
		if (dgndos_is_sector_avaiable(entire_track20, i))
		{
			*size += 256;
		}
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_diskimage_readfile(imgtool::partition &partition, const char *fname, const char *fork, imgtool::stream &destf)
{
	floperr_t ferr;
	imgtoolerr_t err;
	dgndos_dirent ent;
	imgtool::image &image(partition.image());
	int directory_entry_count = 0;
	int head, track, sector;
	int position;
	int extent = 0;
	int block_size;
	int done;
	int lsn;
	int count;
	int i;

	uint8_t entire_track20[18*256];

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	err = lookup_dgndos_file(entire_track20, fname, ent, &position);
	if (err) return err;

	do
	{
		if(directory_entry_count>MAX_DIRENTS) return IMGTOOLERR_CORRUPTDIR;

		lsn = big_endianize_int16( ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[extent].lsn : ent.block.header.block[extent].lsn );
		count = ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[extent].count : ent.block.header.block[extent].count;

		if( count == 0 ) break;

		i = 0;

		// get extents except last sector
		while (i < count - 1)
		{
			err = dgndos_convert_lsn(entire_track20, lsn + i, &head, &track, &sector );
			if(err) return err;

			err = imgtool_floppy_read_sector_to_stream(image, head, track, sector, 0, 256, destf);
			if (err) return err;

			i++;
		}

		block_size = 256;
		done = false;

		if( extent < (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) - 1 )
		{
			if( (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[extent+1].count : ent.block.header.block[extent+1].count) == 0 )
			{
				// not last extent, and yes continuation
				if( ent.dngdos_last_or_next == 0 )
					block_size = 256;
				else
					block_size = ent.dngdos_last_or_next;

				done = true;
			}
		}
		else if( extent == (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) - 1)
		{
			if( !(ent.flag_byte & DGNDOS_CONTINUED_BIT) )
			{
				// is last extent, and no continuation
				if( ent.dngdos_last_or_next == 0 )
					block_size = 256;
				else
					block_size = ent.dngdos_last_or_next;

				done = true;
			}
		}

		err = dgndos_convert_lsn(entire_track20, lsn + i, &head, &track, &sector );
		if(err) return err;

		err = imgtool_floppy_read_sector_to_stream(image, head, track, sector, 0, block_size, destf);
		if (err) return err;

		extent++;

		if( extent == (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT) )
		{
			if( ent.flag_byte & DGNDOS_CONTINUED_BIT)
			{
				position = ent.dngdos_last_or_next;
				err = get_dgndos_dirent(entire_track20, ent.dngdos_last_or_next, ent );
				if(err) return err;

				extent = 0;
				directory_entry_count++;
			}
		}
	}
	while( !done );

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_diskimage_deletefile(imgtool::partition &partition, const char *fname)
{
	imgtoolerr_t err;
	floperr_t ferr;
	imgtool::image &image(partition.image());
	int pos;
	struct dgndos_dirent ent;
	int directory_entry_count = 0;

	uint8_t entire_track20[18*256];
	uint8_t entire_track16[18*256];
	bool write_20_to_16;

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	ferr = dgndos_get_directory_track( image, 16, entire_track16 );
	if (ferr) return imgtool_floppy_error(ferr);

	write_20_to_16 = memcmp(entire_track20, entire_track16, 256*18 );

	err = lookup_dgndos_file(entire_track20, fname, ent, &pos);
	if (err) return err;

	ent.flag_byte |= DGNDOS_DELETED_BIT;
	ent.flag_byte &= ~DGNDOS_PROTECT_BIT;

	for( int i=0; i<HEADER_EXTENTS_COUNT; i++)
	{
		if( ent.block.header.block[i].count != 0)
		{
			dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.block.header.block[i].lsn), ent.block.header.block[i].count);

			ent.block.header.block[i].lsn = 0;
			ent.block.header.block[i].count = 0;
		}
		else break;
	}

	int continue_flag = ent.flag_byte & DGNDOS_CONTINUED_BIT;
	int continue_dirent_index = ent.dngdos_last_or_next;

	ent.dngdos_last_or_next = 0;
	err = put_dgndos_dirent(entire_track20, pos, ent);
	if (err) return err;

	while( continue_flag )
	{
		if( directory_entry_count > MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;

		int dirent_index = continue_dirent_index;
		err = get_dgndos_dirent(entire_track20, dirent_index, ent);
		if (err) return err;

		for( int i=0; i<CONT_EXTENTS_COUNT; i++)
		{
			if(ent.block.continuation.block[i].count !=0)
			{
				dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.block.continuation.block[i].lsn), ent.block.continuation.block[i].count);

				ent.block.continuation.block[i].lsn = 0;
				ent.block.continuation.block[i].count = 0;
			}
			else break;
		}

		ent.flag_byte |= DGNDOS_DELETED_BIT;
		continue_flag = ent.flag_byte & DGNDOS_CONTINUED_BIT;
		continue_dirent_index = ent.dngdos_last_or_next;

		ent.dngdos_last_or_next = 0;
		err = put_dgndos_dirent(entire_track20, dirent_index, ent);
		if (err) return err;

		directory_entry_count++;
	}

	ferr = dgndos_put_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	if( write_20_to_16 )
	{
		ferr = dgndos_put_directory_track( image, 16, entire_track20 );
		if (ferr) return imgtool_floppy_error(ferr);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_prepare_dirent(uint8_t *track, dgndos_dirent &ent, const char *fname)
{
	const char *fname_end;
	const char *fname_ext;
	int fname_ext_len;

	memset(&ent, '\0', sizeof(ent));

	fname_end = strchr(fname, '.');
	if (fname_end)
		fname_ext = fname_end + 1;
	else
		fname_end = fname_ext = fname + strlen(fname);

	fname_ext_len = strlen(fname_ext);

	// we had better be an 8.3 filename
	if (((fname_end - fname) > 8) || (fname_ext_len > 3))
		return IMGTOOLERR_BADFILENAME;

	memcpy(&ent.block.header.filename[0], fname, fname_end - fname);
	memcpy(&ent.block.header.filename[8], fname_ext, fname_ext_len);

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_diskimage_writefile(imgtool::partition &partition, const char *fname, const char *fork, imgtool::stream &sourcef, util::option_resolution *writeoptions)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool::image &image(partition.image());
	dgndos_dirent ent;
	int position;
	uint64_t written = 0;
	int fat_block, first_lsn, sectors_avaiable, current_sector_index;
	int last_sector_size = 0;
	int bitmap_count;

	if( sourcef.size() == 0 ) return IMGTOOLERR_BUFFERTOOSMALL;

	uint8_t entire_track20[18*256];
	uint8_t entire_track16[18*256];
	bool write_20_to_16;

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	ferr = dgndos_get_directory_track( image, 16, entire_track16 );
	if (ferr) return imgtool_floppy_error(ferr);

	write_20_to_16 = memcmp(entire_track20, entire_track16, 18*256 );

	err =  dgndos_get_geometry(entire_track20, &bitmap_count, nullptr, nullptr, nullptr);
	if(err) return err;

	// find directory entry with same file name
	err = lookup_dgndos_file(entire_track20, fname, ent, &position);

	if( err == IMGTOOLERR_FILENOTFOUND )
	{
		int new_lsn;

		// get new directory entry
		err = dgndos_get_avaiable_dirent(entire_track20, ent, &position);
		if (err) return err;

		err = dgndos_prepare_dirent(entire_track20, ent, fname);
		if(err) return err;

		err = dgndos_get_avaiable_sector( entire_track20, &new_lsn );
		if (err)
		{
			return err;
		}

		dgndos_fat_allocate_sector(entire_track20, new_lsn);

		ent.block.header.block[0].lsn = big_endianize_int16(new_lsn);
		ent.block.header.block[0].count = 1;

		err = put_dgndos_dirent(entire_track20, position, ent);
		if(err) return err;
	}
	else if(err != IMGTOOLERR_SUCCESS )
	{
		return err;
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

	/* get next available fat entry */
	first_lsn = big_endianize_int16(ent.block.header.block[fat_block].lsn);
	sectors_avaiable = ent.block.header.block[fat_block].count;
	current_sector_index = 0;

	do
	{
		while ((current_sector_index < sectors_avaiable) && (written < sourcef.size()))
		{
			int head, track, sector, write_count;

			err = dgndos_convert_lsn(entire_track20, first_lsn + current_sector_index, &head, &track, &sector );
			if (err) return err;

			if( sourcef.size() - written > 256 )
			{
				write_count = 256;
			}
			else if( sourcef.size() - written == 256 )
			{
				write_count = 256;
				last_sector_size = 0;
			}
			else
			{
				write_count = sourcef.size() - written;
				last_sector_size = write_count;
			}

			err = imgtool_floppy_write_sector_from_stream(image, head, track, sector, 0, write_count, sourcef);
			if (err) return err;

			current_sector_index++;
			written += write_count;
		}

		if( written == sourcef.size()) // are we are done writing file
		{
			int de_count = 0;
			int de_dont_delete = position;
			int lsn, count;
			int save_next_de;

			save_next_de = ent.dngdos_last_or_next;
			ent.dngdos_last_or_next = last_sector_size;

			// yes, truncate this allocation block if necessary
			if( ent.flag_byte & DGNDOS_CONTINUATION_BIT)
			{
				for( int i = current_sector_index; i < ent.block.continuation.block[fat_block].count; i++)
				{
					dgndos_fat_deallocate_sector(entire_track20, first_lsn + i);
				}

				ent.block.continuation.block[fat_block].count = current_sector_index;
			}
			else
			{
				for( int i = current_sector_index; i < ent.block.header.block[fat_block].count; i++)
				{
					dgndos_fat_deallocate_sector(entire_track20, first_lsn + i);
				}

				ent.block.header.block[fat_block].count = current_sector_index;
			}

			do // check remaining allocation blocks and zero them
			{
				fat_block++;

				if( (fat_block < (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT)) )
				{
					lsn = big_endianize_int16( ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].lsn : ent.block.header.block[fat_block].lsn );
					count = ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count;

					if( count > 0 )
					{
						dgndos_fat_deallocate_span(entire_track20, lsn, count);

						(ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].lsn : ent.block.header.block[fat_block].lsn) = 0;
						(ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count) = 0;
					}
				}
				else
				{
					uint8_t save_flag = ent.flag_byte;

					ent.flag_byte &= ~DGNDOS_CONTINUED_BIT;

					if( ent.flag_byte & DGNDOS_CONTINUATION_BIT )
					{
						if( de_dont_delete != position )
						{
							ent.flag_byte |= DGNDOS_DELETED_BIT;
						}
					}

					err = put_dgndos_dirent(entire_track20, position, ent);
					if (err) return err;

					if( save_flag & DGNDOS_CONTINUED_BIT )
					{
						position = save_next_de;

						err = get_dgndos_dirent(entire_track20, position, ent);
						if (err) return err;

						save_next_de = ent.dngdos_last_or_next;
						fat_block = -1;
					}
					else
					{
						break;
					}
				}

				de_count++;
			}
			while( de_count < MAX_DIRENTS );

			if( de_count == MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;
		}
		else // more to write
		{
			// check if I can extend the allocation count
			if( (current_sector_index < 254) && (first_lsn + current_sector_index < bitmap_count) && (dgndos_is_sector_avaiable( entire_track20, first_lsn + current_sector_index )) )
			{
				sectors_avaiable++;

				(ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count) = sectors_avaiable;

				dgndos_fat_allocate_sector( entire_track20, first_lsn + current_sector_index );
			}
			else // this allocation list is full - check next allocation list
			{
				fat_block++;

				if( fat_block == (ent.flag_byte & DGNDOS_CONTINUATION_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT))
				{
					if( ent.flag_byte & DGNDOS_CONTINUED_BIT)
					{
						// go to next directory entry.
						err = put_dgndos_dirent(entire_track20, position, ent);
						if (err) return err;

						position = ent.dngdos_last_or_next;
						err = get_dgndos_dirent(entire_track20, position, ent);
						if (err)
						{
							return err;
						}

						fat_block = 0;
					}
					else
					{
						// need a or another continuation directory entry
						int save_position = position, new_lsn;

						err = put_dgndos_dirent(entire_track20, position, ent);
						if (err) return err;

						err = dgndos_get_avaiable_dirent_position( entire_track20, &position );
						if (err)
						{
							return err;
						}

						ent.flag_byte |= DGNDOS_CONTINUED_BIT;
						ent.dngdos_last_or_next = position;
						err = put_dgndos_dirent(entire_track20, save_position, ent);
						if (err) return err;

						err = get_dgndos_dirent(entire_track20, position, ent);
						if (err)
						{
							return err;
						}

						memset( (void *)&ent, 0, sizeof(dgndos_dirent) );
						ent.flag_byte |= DGNDOS_CONTINUATION_BIT;
						fat_block = 0;

						err = dgndos_get_avaiable_sector( entire_track20, &new_lsn );
						if (err)
						{
							return err;
						}
					}
				}

				sectors_avaiable = ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count;

				// check if this block is empty
				if( sectors_avaiable == 0)
				{
					int new_lsn;

					err = dgndos_get_avaiable_sector( entire_track20, &new_lsn );
					if (err) return err;

					(ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].lsn : ent.block.header.block[fat_block].lsn) = big_endianize_int16(new_lsn);
					(ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count) = 1;

					dgndos_fat_allocate_sector(entire_track20, new_lsn);
				}

				first_lsn = ent.flag_byte & DGNDOS_CONTINUATION_BIT ? big_endianize_int16(ent.block.continuation.block[fat_block].lsn) : big_endianize_int16(ent.block.header.block[fat_block].lsn);
				sectors_avaiable = ent.flag_byte & DGNDOS_CONTINUATION_BIT ? ent.block.continuation.block[fat_block].count : ent.block.header.block[fat_block].count;
				current_sector_index = 0;
			}
		}
	}
	while( written < sourcef.size() );

	ferr = dgndos_put_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	if( write_20_to_16 )
	{
		ferr = dgndos_put_directory_track( image, 16, entire_track20 );
		if (ferr) return imgtool_floppy_error(ferr);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_diskimage_suggesttransfer(imgtool::partition &partition, const char *fname, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool::image &image(partition.image());
	struct dgndos_dirent ent;
	int pos;

	uint8_t entire_track20[18*256];

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	if (fname)
	{
		err = lookup_dgndos_file(entire_track20, fname, ent, &pos);
		if (err)
			return err;

		if (strcmp(ent.block.header.filename+8,"DAT") == 0)
		{
			/* ASCII file */
			suggestions[0].viability = SUGGESTION_RECOMMENDED;
			suggestions[0].filter = filter_eoln_getinfo;
			suggestions[1].viability = SUGGESTION_POSSIBLE;
			suggestions[1].filter = NULL;
		}
		else if (strcmp(ent.block.header.filename+8,"BAS") == 0)
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

static imgtoolerr_t dgndos_diskimage_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
{
	imgtoolerr_t err;
	uint32_t heads, tracks, sectors, first_sector_id, sector_bytes, total_sector_count;
	uint8_t sector[512];
	struct dgndos_dirent *ents;

	heads = opts->lookup_int('H');
	tracks = opts->lookup_int('T');
	sectors = opts->lookup_int('S');
	first_sector_id = opts->lookup_int('F');
	sector_bytes = opts->lookup_int('L');

	if(sector_bytes!=256) return IMGTOOLERR_UNIMPLEMENTED;

	// create FAT sectors
	memset( sector, 0, 512 );

	total_sector_count = (heads * tracks * sectors);

	for( int i=0; i<total_sector_count; i++ )
	{
		dgndos_fat_deallocate_sector(sector, i);
	}

	sector[0xfc] = tracks;
	sector[0xfd] = heads == 1 ? (sectors) : (sectors * 2);
	sector[0xfe] = ~sector[0xfc];
	sector[0xff] = ~sector[0xfd];

	// allocate sectors
	for( int i=0; i<18; i++ )
	{
		dgndos_fat_allocate_sector(sector, (20 * 18 * heads) + i);
		dgndos_fat_allocate_sector(sector, (16 * 18 * heads) + i);
	}

	// write fat sectors
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 20, first_sector_id, 0, &sector[0], sector_bytes, 0);
	if (err) return err;
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 20, first_sector_id+1, 0, &sector[256], sector_bytes, 0);
	if (err) return err;

	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 16, first_sector_id, 0, &sector[0], sector_bytes, 0);
	if (err) return err;
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 16, first_sector_id+1, 0, &sector[256], sector_bytes, 0);
	if (err) return err;

	// create empty directory entries
	memset( sector, 0, 512 );
	ents = (struct dgndos_dirent *)sector;

	for( int i=0; i<10; i++ )
	{
		ents[i].flag_byte = 0x89;
	}

	// write directory entires to disk
	for( int i=3; i<19; i++)
	{
		err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 20, i, 0, &sector[0], sector_bytes, 0);
		if (err) return err;
		err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 16, i, 0, &sector[0], sector_bytes, 0);
		if (err) return err;
	}

	return IMGTOOLERR_SUCCESS;
}

/*********************************************************************
    Imgtool module declaration
*********************************************************************/

OPTION_GUIDE_START( dragon_dgndos_writefile_optionguide )
	OPTION_ENUM_START(  DGNDOS_OPTIONS_PROTECT, "prot", "Protect file" )
		OPTION_ENUM( 0, "no",  "No" )
		OPTION_ENUM( 1, "yes", "Yes" )
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
		case IMGTOOLINFO_PTR_FLOPPY_CREATE:                 info->create = dgndos_diskimage_create; break;
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
