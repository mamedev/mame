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

static imgtoolerr_t get_dgndos_dirent(uint8_t *track, int index_loc, dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	memcpy( (void *)&ent, track + (256 * (sector-1)) + offset, sizeof(ent));

	return IMGTOOLERR_SUCCESS;
}

//-------------------------------------------------
//  put_dgndos_dirent
//-------------------------------------------------

static imgtoolerr_t put_dgndos_dirent(uint8_t *track, int index_loc, const dgndos_dirent &ent)
{
	if (index_loc >= MAX_DIRENTS)
		return IMGTOOLERR_FILENOTFOUND;

	int sector = 3 + (index_loc / 10);
	int offset = (index_loc * 25) % 250;

	memcpy( track + (256 * (sector-1)) + offset, (void *)&ent, sizeof(ent));

	return IMGTOOLERR_SUCCESS;
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
		}
		while( ! dgndos_real_file(ent) );

		if( ent.flag_byte & DGNDOS_END_BIT ) return IMGTOOLERR_FILENOTFOUND;

		fnamebuf = get_dirent_fname(ent);
	}
	while(core_stricmp(fnamebuf.c_str(), fname));

	if (position)
		*position = i - 1;

	return IMGTOOLERR_SUCCESS;
}

//-------------------------------------------------
//  dgndos_get_geometry
//-------------------------------------------------

static imgtoolerr_t dgndos_get_geometry(uint8_t *entire_track, int *bitmap_count, int *heads, int *tracks_on_disk, int *sectors_per_track)
{
	unsigned int tod, spt, sides;

	tod = entire_track[0xfc];
	spt = entire_track[0xfd];

	if( (~tod & 0xff) != entire_track[0xfe])
	{
		fprintf( stderr, "tracks_on_disk check failed: %u == %u\n", (~tod & 0xff), entire_track[0xfe] );
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	if( (~spt & 0xff) != entire_track[0xff])
	{
		fprintf( stderr, "sectors_per_track check failed: %u == %u\n", (~spt & 0xff), entire_track[0xff] );
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
		fprintf( stderr, "sides check failed\n" );
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

static int dgndos_is_sector_avaiable(uint8_t *entire_track, int lsn)
{
	int startbyte, startbit;

	startbyte = lsn / 8;
	startbit = lsn % 8;

	if( lsn > 1439 )
	{
		startbyte += 76;
	}

	return (entire_track[startbyte] & (1 << startbit)) == (1 << startbit);
}

#define ALLOCATE_BIT true
#define DEALLOCATE_BIT false

static imgtoolerr_t dgndos_set_reset_bitmap( uint8_t *entire_track, int lsn, bool set )
{
	int startbyte  = lsn / 8;
	int startbit = lsn % 8;

	if( lsn > 1439 ) startbyte += 76;

	if(set)
	{
		entire_track[startbyte] &= ~(1 << startbit);
	}
	else
	{
		entire_track[startbyte] |= (1 << startbit);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t dgndos_fat_allocate_sector(uint8_t *entire_track, int lsn)
{
	return dgndos_set_reset_bitmap( entire_track, lsn, ALLOCATE_BIT );
}

static imgtoolerr_t dgndos_fat_deallocate_sector(uint8_t *entire_track, int lsn)
{
	return dgndos_set_reset_bitmap( entire_track, lsn, DEALLOCATE_BIT );
}

static imgtoolerr_t dgndos_fat_deallocate_span(uint8_t *entire_track, int lsn, int count)
{
	imgtoolerr_t err;

	for( int i=lsn; i<lsn+count; i++)
	{
		err = dgndos_fat_deallocate_sector(entire_track, i);
	}

	return err;
}

//-------------------------------------------------
//  dgndos_get_avaiable_dirent_position
//-------------------------------------------------

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

		if( ent.flag_byte & DGNDOS_END_BIT)
		{
			if( i >= MAX_DIRENTS)
			{
				return IMGTOOLERR_NOSPACE;
			}

			break;
		}
	}

	if( position ) *position = i;

	return IMGTOOLERR_SUCCESS;
}

//-------------------------------------------------
//  dgndos_get_avaiable_dirent
//-------------------------------------------------

static imgtoolerr_t dgndos_get_avaiable_dirent(uint8_t *entire_track, dgndos_dirent &ent, int *position )
{
	int i;
	imgtoolerr_t err;

	for( i=0; i<MAX_DIRENTS; i++ )
	{
		err = get_dgndos_dirent(entire_track, i, ent);
		if( err ) return err;

		if( ent.flag_byte & DGNDOS_DELETED_BIT) break;

		if( ent.flag_byte & DGNDOS_END_BIT)
		{
			if( i >= MAX_DIRENTS)
			{
				return IMGTOOLERR_NOSPACE;
			}

			break;
		}
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

static imgtoolerr_t dgndos_get_file_size(uint8_t *entire_track, dgndos_dirent *dgnent, size_t &filesize)
{
	imgtoolerr_t err;
	dgndos_dirent cont_ent;
	filesize = 0;
	int i = 0;
	int directory_entry_count = 0;

	do
	{
		if( i==HEADER_EXTENTS_COUNT) break;

		filesize += dgnent->header_block.block[i].count * 256;
	}
	while (dgnent->header_block.block[++i].count != 0 );

	if(i == HEADER_EXTENTS_COUNT && (dgnent->flag_byte & DGNDOS_CONT_BIT))
	{
		err = get_dgndos_dirent(entire_track, dgnent->dngdos_last_or_next, cont_ent );
		return err;

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
					err = get_dgndos_dirent(entire_track, cont_ent.dngdos_last_or_next, cont_ent );
					return err;
					i = -1;
					directory_entry_count++;
				}
			}
		}
		while (cont_ent.continuation_block.block[++i].count != 0 );

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
 		err = dgndos_get_file_size(entire_track20, &dgnent, filesize);
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
	floperr_t ferr;
	imgtool::image &image(partition.image());
	int bitmap_count;

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
	imgtool::image &image(partition.image());
	int head, track, sector;

	uint8_t entire_track20[18*256];

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	err = lookup_dgndos_file(entire_track20, fname, cont_ent);
	if (err) return err;

	do
	{
		if( i==HEADER_EXTENTS_COUNT ) break;

		for( int j=big_endianize_int16(cont_ent.header_block.block[i].lsn); j<big_endianize_int16(cont_ent.header_block.block[i].lsn) + cont_ent.header_block.block[i].count; j++ )
		{
			block_size = 256;
			err = dgndos_convert_lsn(entire_track20, j, &head, &track, &sector );
			if(err) return err;

			// special case last sector in extent
			if( j == big_endianize_int16(cont_ent.header_block.block[i].lsn) + cont_ent.header_block.block[i].count - 1)
			{
				// is this the last sector of the file
				if( i==3 && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
				{
					// There is another block
				}
				else if( cont_ent.header_block.block[i+1].count != 0 )
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

			err = imgtool_floppy_read_sector_to_stream(image, head, track, sector, 0, block_size, destf);
			if (err) return err;
		}
	}
	while (cont_ent.header_block.block[++i].count != 0 );

	if(i == HEADER_EXTENTS_COUNT && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
	{
		err = get_dgndos_dirent(entire_track20, cont_ent.dngdos_last_or_next, cont_ent );
		if(err) return err;

		i = 0;

		do
		{
			if( directory_entry_count > MAX_DIRENTS ) return IMGTOOLERR_CORRUPTDIR;

			if( i == CONT_EXTENTS_COUNT ) break;

			for( int j=big_endianize_int16(cont_ent.continuation_block.block[i].lsn); j<big_endianize_int16(cont_ent.continuation_block.block[i].lsn) + cont_ent.continuation_block.block[i].count; j++ )
			{
				block_size = 256;
				err = dgndos_convert_lsn(entire_track20, j, &head, &track, &sector );
				if(err) return err;

				// special case last sector in extent
				if( j == big_endianize_int16(cont_ent.continuation_block.block[i].lsn) + cont_ent.continuation_block.block[i].count - 1)
				{
					// is this the last sector of the file
					if( i==6 && (cont_ent.flag_byte & DGNDOS_CONT_BIT))
					{
						// There is another block
					}
					else if( cont_ent.continuation_block.block[i+1].count != 0 )
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

				err = imgtool_floppy_read_sector_to_stream(image, head, track, sector, 0, block_size, destf);
			}

			if( i == CONT_EXTENTS_COUNT-1 )
			{
				if( cont_ent.flag_byte & DGNDOS_CONT_BIT)
				{
					err = get_dgndos_dirent(entire_track20, cont_ent.dngdos_last_or_next, cont_ent );
					if(err) return err;

					i = -1;
					directory_entry_count++;
				}
			}
		}
		while (cont_ent.continuation_block.block[++i].count != 0 );
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
		if( ent.header_block.block[i].count != 0)
		{
			err = dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.header_block.block[i].lsn), ent.header_block.block[i].count);
			if (err) return err;

			ent.header_block.block[i].lsn = 0;
			ent.header_block.block[i].count = 0;
		}
		else break;
	}

	int continue_flag = ent.flag_byte & DGNDOS_ISCONT_BIT;
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
			if(ent.continuation_block.block[i].count !=0)
			{
				err = dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.continuation_block.block[i].lsn), ent.continuation_block.block[i].count);
				if (err) return err;

				ent.continuation_block.block[i].lsn = 0;
				ent.continuation_block.block[i].count = 0;
			}
			else break;
		}

		ent.flag_byte |= DGNDOS_DELETED_BIT;
		continue_flag = ent.flag_byte & DGNDOS_ISCONT_BIT;
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

//-------------------------------------------------
//  dgndos_prepare_dirent - create a new directory
//  entry with a specified name
//-------------------------------------------------

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

	memcpy(&ent.header_block.filename[0], fname, fname_end - fname);
	memcpy(&ent.header_block.filename[8], fname_ext, fname_ext_len);

	return IMGTOOLERR_SUCCESS;
}

//-------------------------------------------------
//  dgndos_diskimage_writefile
//-------------------------------------------------

static imgtoolerr_t dgndos_diskimage_writefile(imgtool::partition &partition, const char *fname, const char *fork, imgtool::stream &sourcef, util::option_resolution *writeoptions)
{
	floperr_t ferr;
	imgtoolerr_t err;
	imgtool::image &image(partition.image());
	dgndos_dirent ent;
	int position;
	uint64_t written = 0;
	int fat_block, block_index, first_lsn, sectors_avaiable, current_sector_index;
	int last_sector_size = 0;
	uint64_t freespace = 0;

	err = dgndos_diskimage_freespace(partition, &freespace);
	if (err) return err;

	if( sourcef.size() == 0 ) return IMGTOOLERR_BUFFERTOOSMALL;

	if (sourcef.size() > freespace) return IMGTOOLERR_NOSPACE;

	uint8_t entire_track20[18*256];
	uint8_t entire_track16[18*256];
	bool write_20_to_16;

	ferr = dgndos_get_directory_track( image, 20, entire_track20 );
	if (ferr) return imgtool_floppy_error(ferr);

	ferr = dgndos_get_directory_track( image, 16, entire_track16 );
	if (ferr) return imgtool_floppy_error(ferr);

	write_20_to_16 = memcmp(entire_track20, entire_track16, 18*256 );

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
		if(err) return err;

		err = dgndos_fat_allocate_sector(entire_track20, new_lsn);
		if(err) return err;

		ent.header_block.block[0].lsn = big_endianize_int16(new_lsn);
		ent.header_block.block[0].count = 1;
	}
	else
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

			err = dgndos_convert_lsn(entire_track20, first_lsn + current_sector_index, &head, &track, &sector );
			if (err) return err;

			if( sourcef.size() - written < 256 )
			{
				write_count = sourcef.size() - written;
				last_sector_size = write_count;
			}
			else
			{
				write_count = 256;
			}

			err = imgtool_floppy_write_sector_from_stream(image, head, track, sector, 0, write_count, sourcef);
			if (err) return err;

			current_sector_index++;
			written += write_count;
		}

		if( written == sourcef.size()) // are we are done writing file
		{
			int next_de = 0;

			// yes, truncate this allocation block if necessary
			for( int i = current_sector_index; i < ent.header_block.block[fat_block].count; i++)
			{
				err = dgndos_fat_deallocate_sector(entire_track20, first_lsn + i);
				if (err) return err;
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
						err = dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.continuation_block.block[i].lsn), ent.continuation_block.block[i].count);
						if (err) return err;
						ent.continuation_block.block[i].lsn = 0;
						ent.continuation_block.block[i].count = 0;
					}
				}
				else
				{
					for( int i = fat_block; i < HEADER_EXTENTS_COUNT; i++ )
					{
						err = dgndos_fat_deallocate_span(entire_track20, big_endianize_int16(ent.header_block.block[i].lsn), ent.header_block.block[i].count);
						if (err) return err;
						ent.header_block.block[i].lsn = big_endianize_int16(0);
						ent.header_block.block[i].count = 0;
					}
				}

				// delete DE if all blocks were cleared and it is a continued DE
				if( (first_fat_block == 0) && (ent.flag_byte & DGNDOS_ISCONT_BIT) )
				{
					ent.flag_byte |= DGNDOS_DELETED_BIT;
				}

				err = put_dgndos_dirent(entire_track20, position, ent);
				if (err) return err;

				if( next_de != 0 )
				{
					err = get_dgndos_dirent(entire_track20, next_de, ent);
					if (err) return err;

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
			if( (current_sector_index < 255) && (dgndos_is_sector_avaiable( entire_track20, first_lsn + current_sector_index )) )
			{
				sectors_avaiable++;

				if( ent.flag_byte & DGNDOS_CONT_BIT )
				{
					ent.header_block.block[fat_block].count = sectors_avaiable;
				}
				else
				{
					ent.continuation_block.block[fat_block].count = sectors_avaiable;
				}

				err = dgndos_fat_allocate_sector( entire_track20, first_lsn + current_sector_index );
				if (err) return err;
			}
			else // this allocation list is full - check next allocation list
			{
				fat_block++;

				if( fat_block == (ent.flag_byte & DGNDOS_ISCONT_BIT ? CONT_EXTENTS_COUNT : HEADER_EXTENTS_COUNT))
				{
					// need a or another continuation directory entry
					int save_position = position;

					err = dgndos_get_avaiable_dirent_position( entire_track20, &position );
					if (err) return err;

					ent.flag_byte |= DGNDOS_ISCONT_BIT;
					ent.dngdos_last_or_next = position;
					err = put_dgndos_dirent(entire_track20, save_position, ent);
					if (err) return err;

					err = get_dgndos_dirent(entire_track20, position, ent);
					if (err) return err;

					memset( (void *)&ent, 0, sizeof(dgndos_dirent) );
					ent.flag_byte |= DGNDOS_CONT_BIT;
					fat_block = 0;
				}

				first_lsn = ent.flag_byte & DGNDOS_CONT_BIT ? big_endianize_int16(ent.continuation_block.block[fat_block].lsn) : big_endianize_int16(ent.header_block.block[fat_block].lsn);
				sectors_avaiable = ent.flag_byte & DGNDOS_CONT_BIT ? ent.continuation_block.block[fat_block].count : ent.header_block.block[fat_block].count;

				// check if this block is empty
				if( sectors_avaiable == 0)
				{
					int lsn;

					err = dgndos_get_avaiable_sector( entire_track20, &lsn );
					if (err)
					{
						// directory track is cached, and un-written. So aborting leaves disk image in correct state
						return err;
					}

					if( ent.flag_byte & DGNDOS_CONT_BIT )
					{
						ent.continuation_block.block[fat_block].lsn = big_endianize_int16(lsn);
						ent.continuation_block.block[fat_block].count = 1;
					}
					else
					{
						ent.header_block.block[fat_block].lsn = big_endianize_int16(lsn);
						ent.header_block.block[fat_block].count = 1;
					}
				}

				first_lsn = ent.flag_byte & DGNDOS_CONT_BIT ? big_endianize_int16(ent.continuation_block.block[fat_block].lsn) : big_endianize_int16(ent.header_block.block[fat_block].lsn);
				sectors_avaiable = ent.flag_byte & DGNDOS_CONT_BIT ? ent.continuation_block.block[fat_block].count : ent.header_block.block[fat_block].count;
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

//-------------------------------------------------
//  dgndos_diskimage_suggesttransfer
//-------------------------------------------------

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


static imgtoolerr_t dgndos_diskimage_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
{
	imgtoolerr_t err;
	uint32_t heads, tracks, sectors, first_sector_id, sector_bytes, total_sector_count;
	uint8_t sector[512];
	struct dgndos_dirent *ents;

	heads = opts->lookup_int('H')-1;
	tracks = opts->lookup_int('T');
	sectors = opts->lookup_int('S');
	first_sector_id = opts->lookup_int('F');
	sector_bytes = opts->lookup_int('L');

	if(sector_bytes!=256) return IMGTOOLERR_UNIMPLEMENTED;

	// create FAT sectors
	memset( sector, 0, 512 );

	total_sector_count = (tracks * sectors) + (heads * tracks * sectors);

	for( int i=0; i<total_sector_count; i++ )
	{
		dgndos_set_reset_bitmap( sector, i, DEALLOCATE_BIT );
	}

	sector[0xfc] = tracks;
	sector[0xfd] = heads ? (sectors) * 2 : (sectors);
	sector[0xfe] = ~sector[0xfc];
	sector[0xff] = ~sector[0xfd];

	// write fat sectors
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 20, first_sector_id, 0, &sector[0], sector_bytes, 0);
	if (err) return err;
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 20, first_sector_id+1, 0, &sector[256], sector_bytes, 0);
	if (err) return err;

	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 16, first_sector_id, 0, &sector[0], sector_bytes, 0);
	if (err) return err;
	err = (imgtoolerr_t)floppy_write_sector(imgtool_floppy(image), 0, 16, first_sector_id+1, 0, &sector[256], sector_bytes, 0);
	if (err) return err;

	// allocate sectors
	for( int i=0; i<18; i++ )
	{
		dgndos_set_reset_bitmap( sector, (20 * (sectors) * heads) + (20 * (sectors)) + i, ALLOCATE_BIT );
		dgndos_set_reset_bitmap( sector, (16 * (sectors) * heads) + (16 * (sectors)) + i, ALLOCATE_BIT );
	}

	// create empty directory entries
	memset( sector, 0, 512 );
	ents = (struct dgndos_dirent *)sector;

	for( int i=0; i<9; i++ )
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

