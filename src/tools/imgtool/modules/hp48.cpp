// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/****************************************************************************

    hp48.cpp

    Memory cards for HP48 SX/GX

    Antoine Mine' 2014

*****************************************************************************
*/

/*
  HP memory cards can contain two kinds of objects:
  - attached libraries
  - backup objects (encapsulating any kind of object)

  Attached libraries appear with a LIB attribute in imgtool's listing, and in
  the global "LIBRARY" menu on the HP48.
  Backup objects appear with a BAK attribute in imgtool's listing, and
  in the relevant "PORTS" menu on the HP48.

  Currently, we only support storing host files into backup objects and
  retreiving backup objects into files.
  For attached library objects in the memory card, we only show their name, but
  cannot retrieve their contents.

  To install a library contained in a host file, copy it with imgtool to
  the card; it creates a backup object visible in the "PORTS" menu;
  then evalue the object, store it into some port (e.g, 0 STO),
  turn off the HP48 and back on; the library should now be attached and
  visible in the  "LIBRARY" menu.
  (The process is similar to installing a library from a file you get
  from the serial interface.)
*/


/*
  HP memory cards have size from 32 KB to 4096 KB.
  However, cards with size greater than 128 KB are seen as multiple ports of
  size 128 KB on the HP48 GX. In imgtool, we see them as distinct 128 KB
  partitions.
  Unfortunately, at the moment, partitions do not seem to be supported in the
  command-line tool...
 */


/*
  Here is the format of backup objects (addresses are in 4-bit nibbles):

  offset         field           size       value
    0            prolog            5        02B62       prolog for overall backup object
    5            size              5          len       total size in nibble without prolog
   10            name length       2        nbcar       object name, in characters
   12            name            2*nbcar      -
   12+2*nbcar    name length       2        nbcar       same value as name length at 10
   14+2*nbcar    object            -          -
   len-5         prolog            5        02911       prolog for inlined system integer
   len            -                1          0
   len+1          CRC              4          -         CRC from 5 to len

   total length: len+4

   i.e., the backup object is a container containing the object name
   and two objects: the  backuped object itself and a system integer
   that contains the CRC.

   HP48 host files start with a "HPHP48-X" header (where X can be any
   single letter, which denotes a ROM revision and, implicitly, whether
   the program is for the S/SX or the G/GX family).
   When storing a file to the memory card, we strip the header and
   embed the object into a backup object with the correct CRC.
   When copying from the memory card to a host file, we extract the object
   from the backup container, stripping its CRC, and add the HPHP48-X header.

 */





/*****************************************************************************
 Includes
*****************************************************************************/

#include "imgtool.h"

#include "opresolv.h"



/*****************************************************************************
 Data structures
*****************************************************************************/


struct hp48_card
{
	imgtool::stream *stream;
		int modified;

		/* size, in bytes of card data: from 32 KB to 4 MB */
		int size;

		/* we store each nibble (4-bit) into its own byte, for simpler addressing;
		   hence, data has 2*size
		*/
		uint8_t* data;

};

struct hp48_partition
{
		/* pointer to the beginning of the partition inside the hp48_card */
		uint8_t* data;

		/* size, in bytes (128 KB or less) */
		int size;

};

struct hp48_directory
{
		int pos;
};


#define PROLOG_BACKUP   0x02B62
#define PROLOG_LIBRARY  0x02B40
#define PROLOG_SYSINT   0x02911

/* memory cards are composed solely of libraries and backup objects */
#define IS_OBJECT(prolog) ((prolog) == PROLOG_LIBRARY || (prolog) == PROLOG_BACKUP)

/* host files begin with this prefix (up to the last letter) */
static const char* hp48_prefix = "HPHP48-R";



OPTION_GUIDE_START( hp48_create_optionguide )
	OPTION_INT('S', "size", "Size in KB" )
OPTION_GUIDE_END

/* size in KB, 128 KB being the default */
static const char hp48_create_optionspec[] = "S32/64/[128]/256/512/1024/2048/4096";



/*****************************************************************************
 Utility functions
*****************************************************************************/


static hp48_card *get_hp48_card(imgtool::image &image)
{
	return (hp48_card*) image.extra_bytes();
}

/* byes to nibbles */
static void unpack(uint8_t* dst, uint8_t* src, int nsize)
{
		int i;

		if ( nsize & 1 )
		{
				dst[nsize-1] = src[nsize/2] & 0xf;
		}

		for ( i = nsize/2-1; i >= 0; i-- )
		{
				dst[2*i+1] = src[i] >> 4;
				dst[2*i  ] = src[i] & 0xf;
		}
}

/* nibbles to bytes */
static void pack(uint8_t* dst, uint8_t* src, int nsize)
{
		int i;

		for ( i = 0 ; i  < nsize/2; i++ )
		{
				dst[i] = (src[2*i] & 0xf) | (src[2*i+1] << 4);
		}

		if ( nsize & 1 )
		{
				dst[nsize/2] = src[nsize-1] & 0xf;
		}
	}


static int read20(uint8_t* data)
{
		return data[0] | (data[1] << 4) | (data[2] << 8) | (data[3] << 12) | (data[4] << 16);
}

static int read8(uint8_t* data)
{
		return data[0] | (data[1] << 4);
}

static void readstring(char* dst, uint8_t* data, int nb)
{
		int i;
		for ( i = 0; i < nb; i++ )
		{
				dst[i] = read8( data + 2*i );
		}
		dst[nb] = 0;
}

static void write20(uint8_t* data, int v)
{
		data[0] = v & 0xf;
		data[1] = (v >> 4) & 0xf;
		data[2] = (v >> 8) & 0xf;
		data[3] = (v >> 12) & 0xf;
		data[4] = (v >> 16) & 0xf;
}

static void write8(uint8_t* data, int v)
{
		data[0] = v & 0xf;
		data[1] = (v >> 4) & 0xf;
}

static void writestring(uint8_t* data, const char* str, int nb)
{
		int i;
		for ( i = 0; i < nb; i++ )
		{
				write8( data + 2*i, str[i] );
		}
}


/* go to the end, return its offset */
static int find_end(hp48_partition* p)
{
		int pos = 0;
		while (1)
		{
				if ( pos + 10 > 2*p->size) break;

				int prolog = read20( p->data + pos );
				if ( !IS_OBJECT( prolog )) break;

				pos += read20( p->data + pos + 5 ) + 5;
		}

		if ( pos > 2*p->size ) pos = 2*p->size;
		return pos;
}


/* find the backup object with the given name, returns its offset or -1 (not found) */
static int find_file(hp48_partition* p, const char* filename, int *ptotalsize, int* pstart, int* pcontentsize)
{
		uint8_t* data = p->data;
		int pos = 0;

		/* find file */
		while (1)
		{
				if ( pos + 10 > 2*p->size) return -1;

				/* get prolog */
				int prolog = read20( data+pos );
				if ( !IS_OBJECT(prolog)) return -1;

				/* get size */
				int totalsize = read20( data+pos+5 );
				if ( totalsize < 14) return -1;
				if ( pos + 5 + totalsize > 2*p->size ) return -1;

				if ( prolog == PROLOG_BACKUP )
				{
						/* get name */
						int namelen = read8( data + pos + 10 );
						char name[257];
						if ( 9 + 2*namelen > totalsize ) return -1;
						readstring( name, data + pos + 12, namelen );

						/* check name */
						if ( !strcmp( name, filename ) )
						{
								/* found! */
								if ( ptotalsize ) *ptotalsize = totalsize;
								if ( pstart ) *pstart = pos + 14 + 2*namelen;
								if ( pcontentsize ) *pcontentsize = totalsize - (9 + 2*namelen);
								return pos;
						}
						else
						{
								/* skip */
								pos += totalsize + 5;
						}
				}
				else
				{
						/* skip */
						pos += totalsize + 5;
				}

		}

		// never executed
		//return -1;
}


/* CRC computing.
   This is the same CRC that is computed by the HP48 hardware.
 */
static uint16_t crc(uint8_t* data, int len)
{
		uint16_t crc = 0;
		int i;

		for ( i = 0; i < len; i++ )
		{
				crc = (crc >> 4) ^ (((crc ^ data[i]) & 0xf) * 0x1081);
		}

		return crc;
}



/*****************************************************************************
 Imgtool functions
*****************************************************************************/


static imgtoolerr_t hp48_open(imgtool::image &img, imgtool::stream::ptr &&stream)
{
	hp48_card* c = get_hp48_card(img);
	int size = stream->size();

	/* check that size is a power of 2 between 32 KB and 4 MG */
	if ( (size < 32 * 1024) ||
			(size > 4 * 1024 * 1024) ||
			(size & (size-1)) )
	{
		return IMGTOOLERR_CORRUPTIMAGE;
	}

	/* store info */
	c->stream = stream.get();
	c->modified = 0;
	c->size = size;
	c->data = (uint8_t*) malloc( 2 * size );
	if ( !c->data )
	{
		return IMGTOOLERR_READERROR;
	}

	/* fully load image */
	c->stream->seek(0, SEEK_SET);
	if (c->stream->read(c->data, size) < size)
	{
		return IMGTOOLERR_READERROR;
	}
	unpack( c->data, c->data, 2 * size );

	c->stream = stream.release();
	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_create(imgtool::image &img,
				imgtool::stream::ptr &&stream,
				util::option_resolution *opts)
{
	hp48_card* c = get_hp48_card(img);
	int size;

	size = opts->lookup_int('S');

	c->stream = stream.get();
	c->modified = 1;
	c->size = size * 1024;
	c->data = (uint8_t*) malloc( 2 * c->size );
	if ( !c->data )
	{
		return IMGTOOLERR_READERROR;
	}

	/* zeroing the image seems fine */
	memset( c->data, 0, 2 * c->size );

	c->stream = stream.release();
	return IMGTOOLERR_SUCCESS;
}



static void hp48_close(imgtool::image &img)
{
	hp48_card* c = get_hp48_card(img);

	if ( c->modified )
		{
				/* save image */
				pack( c->data, c->data, 2 * c->size );
				c->stream->seek(0, SEEK_SET);
				c->stream->write(c->data, c->size);
		}

		/* clean up */
		free( c->data );
	delete c->stream;
}



/* each 128 KB chunk is a distinct partition */
#define MAX_PORT_SIZE (128*1024)

void hp48_partition_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info);

static imgtoolerr_t hp48_list_partitions(imgtool::image &img, std::vector<imgtool::partition_info> &partitions)
{
	hp48_card* c = get_hp48_card(img);

	int i;
	for (i = 0; i * MAX_PORT_SIZE < c->size ; i++)
	{
		// offset and size in bytes
		uint64_t base_block = i * MAX_PORT_SIZE;
		uint64_t block_count = std::min((uint64_t)c->size - base_block, (uint64_t)MAX_PORT_SIZE);

		// append the partition
		partitions.emplace_back(hp48_partition_get_info, base_block, block_count);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp48_open_partition(imgtool::partition &part,
					uint64_t first_block, uint64_t block_count)
{
	imgtool::image &img(part.image());
	hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();

		if ( first_block + block_count > c->size )
				return IMGTOOLERR_INVALIDPARTITION;

		/* store partition position */
		p->data = c->data + first_block;
		p->size = block_count;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_beginenum(imgtool::directory &enumeration, const char *path)
{
	hp48_directory* d = (hp48_directory*) enumeration.extra_bytes();

	d->pos = 0;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_nextenum(imgtool::directory &enumeration, imgtool_dirent &ent)
{
	imgtool::partition &part(enumeration.partition());
	//imgtool::image &img(part.image());
	//hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();
	hp48_directory* d = (hp48_directory*) enumeration.extra_bytes();

		uint8_t* data = p->data;
		int pos = d->pos;

		if ( pos < 0 || pos+12 > 2*p->size )
		{
				ent.eof = 1;
				return IMGTOOLERR_SUCCESS;
		}

		int prolog = read20( data+pos );

		if ( IS_OBJECT(prolog) )
		{
				pos += 5;
				int totalsize = read20( data+pos );
				pos += 5;

				int namelen = read8( data+pos );
				pos += 2;
				if ( (pos + 2*namelen > 2*p->size) ||
						(namelen >= sizeof(ent.filename)) )
				{
						ent.eof = 1;
						return IMGTOOLERR_CORRUPTFILE;
				}
				readstring( ent.filename, data+pos, namelen );

				/* compute size in bytes, removing name, length & CRC fields */
		ent.filesize = ((totalsize - 19 - 2*namelen) + 1) / 2;

				switch (prolog)
				{
				case PROLOG_LIBRARY:  strncpy( ent.attr, "LIB", sizeof(ent.attr) ); break;
				case PROLOG_BACKUP:   strncpy( ent.attr, "BAK", sizeof(ent.attr) ); break;
				default:              strncpy( ent.attr, "?", sizeof(ent.attr) );
				}

				d->pos = d->pos + totalsize + 5;
		}
		else
		{
				/* 0 or unknown object => end */
				ent.eof = 1;
		}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_freespace(imgtool::partition &part, uint64_t *size)
{
	//imgtool::image &img(part.image());
	//hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();

		*size = p->size - (find_end(p)+1)/2;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_readfile(imgtool::partition &part,
									const char *filename,
									const char *fork,
									imgtool::stream &destf)
{
	//imgtool::image &img(part.image());
	//hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();

		/* find entry */
		int totalsize, start, size;
		int pos = find_file(p, filename, &totalsize, &start, &size);
		if ( pos == -1 )
		{
				return IMGTOOLERR_FILENOTFOUND;
		}

		/* CRC check */
		uint16_t objcrc = read20( p->data + pos + totalsize ) >> 4;
		uint16_t mycrc = crc( p->data + pos + 5, totalsize - 4);
		if ( objcrc != mycrc )
		{
				return IMGTOOLERR_CORRUPTIMAGE;
		}
		size -= 10;

		/* save header */
		destf.write(hp48_prefix, 8);

		/* save contents to host file */
		int bytesize = (size + 1) / 2;
		uint8_t* buf = (uint8_t*) malloc( bytesize );
		if (!buf)
		{
				return IMGTOOLERR_FILENOTFOUND;
		}
		pack( buf, p->data + start, size );
		destf.write(buf, bytesize);
		free( buf );

		return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_deletefile(imgtool::partition &part,
									const char *filename)
{
	imgtool::image &img(part.image());
	hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();

		/* find entry */
		int totalsize;
		int pos = find_file(p, filename, &totalsize, NULL, NULL );
		if ( pos == -1 )
		{
				return IMGTOOLERR_FILENOTFOUND;
		}

		/* move */
		totalsize += 5;
		memmove( p->data+pos, p->data+pos+totalsize, 2*p->size-(pos+totalsize) );
		memset( p->data + 2*p->size-totalsize, 0, totalsize);
		c->modified = 1;

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t hp48_writefile(imgtool::partition &part,
									const char *filename,
									const char *fork,
									imgtool::stream &sourcef,
									util::option_resolution *opts)
{
	imgtool::image &img(part.image());
	hp48_card* c = get_hp48_card(img);
	hp48_partition* p = (hp48_partition*) part.extra_bytes();

		/* check header */
		char head[8];
		sourcef.read(head, 8);
		if ( memcmp( head, hp48_prefix, 7) )
		{
				return IMGTOOLERR_READERROR;
		}

		/* ensure that the file does not exist */
		/* TODO: resize the existing file instead, to keep it in place? */
		hp48_deletefile( part, filename );

		/* goto end */
		//uint8_t* data = p->data;
		int pos = find_end(p);

		int len = strlen( filename );
		if ( len > 255 ) len = 255;

		/* check size */
		int filesize = sourcef.size() - 8;
		if ( pos + 2*filesize + 24 + 2*len > 2 * p->size )
		{
				return IMGTOOLERR_NOSPACE;
		}

		/* load file */
		uint8_t* buf = (uint8_t*) malloc( filesize );
		if ( !buf ) return IMGTOOLERR_NOSPACE;
		sourcef.read(buf, filesize);

		/* store backup object */
		int org = pos;
		int totalsize = 2*filesize + 19 + 2*len;
		write20( p->data+pos, PROLOG_BACKUP );
		pos +=5;
		write20( p->data+pos, totalsize );
		pos +=5;
		write8( p->data+pos, len );
		pos += 2;
		writestring( p->data+pos, filename, len );
		pos += 2*len;
		write8( p->data+pos, len );
		pos += 2;
		unpack( p->data+pos, buf, 2*filesize );
		pos += 2*filesize;

		/* store crc */
		write20( p->data+pos, PROLOG_SYSINT );
		pos += 5;
		p->data[pos] = 0;
		write20( p->data+pos, crc(p->data+org+5, totalsize-4) << 4 );

		free(buf);

		c->modified = 1;

	return IMGTOOLERR_SUCCESS;
}




/*****************************************************************************
 Imgtool module declaration
*****************************************************************************/



void hp48_partition_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "hp48"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "HP48 SX/GX memory card"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "crd"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_OPEN_PARTITION:                info->open_partition = hp48_open_partition; break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close       = hp48_close; break;
		case IMGTOOLINFO_PTR_BEGIN_ENUM:                    info->begin_enum  = hp48_beginenum; break;
		case IMGTOOLINFO_PTR_NEXT_ENUM:                     info->next_enum   = hp48_nextenum; break;
		case IMGTOOLINFO_PTR_FREE_SPACE:                    info->free_space  = hp48_freespace; break;
		case IMGTOOLINFO_PTR_READ_FILE:                     info->read_file   = hp48_readfile; break;
		case IMGTOOLINFO_PTR_WRITE_FILE:                    info->write_file  = hp48_writefile; break;
				case IMGTOOLINFO_PTR_DELETE_FILE:                   info->delete_file = hp48_deletefile; break;

		case IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES:         info->i = sizeof(hp48_partition); break;
		case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:         info->i = sizeof(hp48_directory); break;

	}
}

void hp48_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "hp48"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "HP48 SX/GX memory card"); break;
		case IMGTOOLINFO_STR_FILE:                          strcpy(info->s = imgtool_temp_str(), __FILE__); break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:               strcpy(info->s = imgtool_temp_str(), "crd"); break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:           strcpy(info->s = imgtool_temp_str(), hp48_create_optionspec); break;

		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:          info->createimage_optguide = &hp48_create_optionguide; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_OPEN:                          info->open        = hp48_open; break;
		case IMGTOOLINFO_PTR_CREATE:                        info->create      = hp48_create; break;
		case IMGTOOLINFO_PTR_CLOSE:                         info->close       = hp48_close; break;
		case IMGTOOLINFO_PTR_LIST_PARTITIONS:               info->list_partitions = hp48_list_partitions; break;

		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:             info->i = sizeof(hp48_card); break;
		}
}
