// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corefile.c

    File access functions.

***************************************************************************/

#include <assert.h>

#include "corefile.h"
#include "unicode.h"
#include <zlib.h>

#include <stdarg.h>
#include <ctype.h>



/***************************************************************************
    VALIDATION
***************************************************************************/

#if !defined(CRLF) || (CRLF < 1) || (CRLF > 3)
#error CRLF undefined: must be 1 (CR), 2 (LF) or 3 (CR/LF)
#endif



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define FILE_BUFFER_SIZE        512

#define OPEN_FLAG_HAS_CRC       0x10000



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum text_file_type
{
	TFT_OSD = 0,    /* OSD depdendent encoding format used when BOMs missing */
	TFT_UTF8,       /* UTF-8 */
	TFT_UTF16BE,    /* UTF-16 (big endian) */
	TFT_UTF16LE,    /* UTF-16 (little endian) */
	TFT_UTF32BE,    /* UTF-32 (UCS-4) (big endian) */
	TFT_UTF32LE     /* UTF-32 (UCS-4) (little endian) */
};


struct zlib_data
{
	z_stream        stream;
	UINT8           buffer[1024];
	UINT64          realoffset;
	UINT64          nextoffset;
};


struct core_file
{
	osd_file *      file;                       /* OSD file handle */
	zlib_data *     zdata;                      /* compression data */
	UINT32          openflags;                  /* flags we were opened with */
	UINT8           data_allocated;             /* was the data allocated by us? */
	UINT8 *         data;                       /* file data, if RAM-based */
	UINT64          offset;                     /* current file offset */
	UINT64          length;                     /* total file length */
	text_file_type  text_type;                  /* text output format */
	char            back_chars[UTF8_CHAR_MAX];  /* buffer to hold characters for ungetc */
	int             back_char_head;             /* head of ungetc buffer */
	int             back_char_tail;             /* tail of ungetc buffer */
	UINT64          bufferbase;                 /* base offset of internal buffer */
	UINT32          bufferbytes;                /* bytes currently loaded into buffer */
	UINT8           buffer[FILE_BUFFER_SIZE];   /* buffer data */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* misc helpers */
static UINT32 safe_buffer_copy(const void *source, UINT32 sourceoffs, UINT32 sourcelen, void *dest, UINT32 destoffs, UINT32 destlen);
static file_error osd_or_zlib_read(core_file *file, void *buffer, UINT64 offset, UINT32 length, UINT32 *actual);
static file_error osd_or_zlib_write(core_file *file, const void *buffer, UINT64 offset, UINT32 length, UINT32 *actual);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    is_directory_separator - is a given character
    a directory separator? The following logic
    works for most platforms
-------------------------------------------------*/

INLINE int is_directory_separator(char c)
{
	return (c == '\\' || c == '/' || c == ':');
}



/***************************************************************************
    FILE OPEN/CLOSE
***************************************************************************/

/*-------------------------------------------------
    core_fopen - open a file for access and
    return an error code
-------------------------------------------------*/

file_error core_fopen(const char *filename, UINT32 openflags, core_file **file)
{
	file_error filerr = FILERR_NOT_FOUND;

	/* allocate the file itself */
	*file = (core_file *)malloc(sizeof(**file));
	if (*file == NULL)
		return FILERR_OUT_OF_MEMORY;
	memset(*file, 0, sizeof(**file));

	/* attempt to open the file */
	filerr = osd_open(filename, openflags, &(*file)->file, &(*file)->length);
	(*file)->openflags = openflags;

	/* handle errors and return */
	if (filerr != FILERR_NONE)
	{
		core_fclose(*file);
		*file = NULL;
	}
	return filerr;
}


/*-------------------------------------------------
    core_fopen_ram_internal - open a RAM-based buffer
    for file-like access, possibly copying the data,
    and return an error code
-------------------------------------------------*/

static file_error core_fopen_ram_internal(const void *data, size_t length, int copy_buffer, UINT32 openflags, core_file **file)
{
	/* can only do this for read access */
	if ((openflags & OPEN_FLAG_WRITE) != 0)
		return FILERR_INVALID_ACCESS;
	if ((openflags & OPEN_FLAG_CREATE) != 0)
		return FILERR_INVALID_ACCESS;

	/* allocate the file itself */
	*file = (core_file *)malloc(sizeof(**file) + (copy_buffer ? length : 0));
	if (*file == NULL)
		return FILERR_OUT_OF_MEMORY;
	memset(*file, 0, sizeof(**file));

	/* copy the buffer, if we're asked to */
	if (copy_buffer)
	{
		void *dest = ((UINT8 *) *file) + sizeof(**file);
		memcpy(dest, data, length);
		data = dest;
	}

	/* claim the buffer */
	(*file)->data = (UINT8 *)data;
	(*file)->length = length;
	(*file)->openflags = openflags;

	return FILERR_NONE;
}


/*-------------------------------------------------
    core_fopen_ram - open a RAM-based buffer for
    file-like access and return an error code
-------------------------------------------------*/

file_error core_fopen_ram(const void *data, size_t length, UINT32 openflags, core_file **file)
{
	return core_fopen_ram_internal(data, length, FALSE, openflags, file);
}


/*-------------------------------------------------
    core_fopen_ram_copy - open a copy of a RAM-based
    buffer for file-like access and return an error code
-------------------------------------------------*/

file_error core_fopen_ram_copy(const void *data, size_t length, UINT32 openflags, core_file **file)
{
	return core_fopen_ram_internal(data, length, TRUE, openflags, file);
}


/*-------------------------------------------------
    core_fclose - closes a file
-------------------------------------------------*/

void core_fclose(core_file *file)
{
	/* close files and free memory */
	if (file->zdata != NULL)
		core_fcompress(file, FCOMPRESS_NONE);
	if (file->file != NULL)
		osd_close(file->file);
	if (file->data != NULL && file->data_allocated)
		free(file->data);
	free(file);
}


/*-------------------------------------------------
    core_fcompress - enable/disable streaming file
    compression via zlib; level is 0 to disable
    compression, or up to 9 for max compression
-------------------------------------------------*/

file_error core_fcompress(core_file *file, int level)
{
	file_error result = FILERR_NONE;

	/* can only do this for read-only and write-only cases */
	if ((file->openflags & OPEN_FLAG_WRITE) != 0 && (file->openflags & OPEN_FLAG_READ) != 0)
		return FILERR_INVALID_ACCESS;

	/* if we have been compressing, flush and free the data */
	if (file->zdata != NULL && level == FCOMPRESS_NONE)
	{
		int zerr = Z_OK;

		/* flush any remaining data if we are writing */
		while ((file->openflags & OPEN_FLAG_WRITE) != 0 && zerr != Z_STREAM_END)
		{
			UINT32 actualdata;
			file_error filerr;

			/* deflate some more */
			zerr = deflate(&file->zdata->stream, Z_FINISH);
			if (zerr != Z_STREAM_END && zerr != Z_OK)
			{
				result = FILERR_INVALID_DATA;
				break;
			}

			/* write the data */
			if (file->zdata->stream.avail_out != sizeof(file->zdata->buffer))
			{
				filerr = osd_write(file->file, file->zdata->buffer, file->zdata->realoffset, sizeof(file->zdata->buffer) - file->zdata->stream.avail_out, &actualdata);
				if (filerr != FILERR_NONE)
					break;
				file->zdata->realoffset += actualdata;
				file->zdata->stream.next_out = file->zdata->buffer;
				file->zdata->stream.avail_out = sizeof(file->zdata->buffer);
			}
		}

		/* end the appropriate operation */
		if ((file->openflags & OPEN_FLAG_WRITE) != 0)
			deflateEnd(&file->zdata->stream);
		else
			inflateEnd(&file->zdata->stream);

		/* free memory */
		free(file->zdata);
		file->zdata = NULL;
	}

	/* if we are just starting to compress, allocate a new buffer */
	if (file->zdata == NULL && level > FCOMPRESS_NONE)
	{
		int zerr;

		/* allocate memory */
		file->zdata = (zlib_data *)malloc(sizeof(*file->zdata));
		if (file->zdata == NULL)
			return FILERR_OUT_OF_MEMORY;
		memset(file->zdata, 0, sizeof(*file->zdata));

		/* initialize the stream and compressor */
		if ((file->openflags & OPEN_FLAG_WRITE) != 0)
		{
			file->zdata->stream.next_out = file->zdata->buffer;
			file->zdata->stream.avail_out = sizeof(file->zdata->buffer);
			zerr = deflateInit(&file->zdata->stream, level);
		}
		else
			zerr = inflateInit(&file->zdata->stream);

		/* on error, return an error */
		if (zerr != Z_OK)
		{
			free(file->zdata);
			file->zdata = NULL;
			return FILERR_OUT_OF_MEMORY;
		}

		/* flush buffers */
		file->bufferbytes = 0;

		/* set the initial offset */
		file->zdata->realoffset = file->offset;
		file->zdata->nextoffset = file->offset;
	}

	return result;
}



/***************************************************************************
    FILE POSITIONING
***************************************************************************/

/*-------------------------------------------------
    core_fseek - seek within a file
-------------------------------------------------*/

int core_fseek(core_file *file, INT64 offset, int whence)
{
	int err = 0;

	/* error if compressing */
	if (file->zdata != NULL)
		return 1;

	/* flush any buffered char */
	file->back_char_head = 0;
	file->back_char_tail = 0;

	/* switch off the relative location */
	switch (whence)
	{
		case SEEK_SET:
			file->offset = offset;
			break;

		case SEEK_CUR:
			file->offset += offset;
			break;

		case SEEK_END:
			file->offset = file->length + offset;
			break;
	}
	return err;
}


/*-------------------------------------------------
    core_ftell - return the current file position
-------------------------------------------------*/

UINT64 core_ftell(core_file *file)
{
	/* return the current offset */
	return file->offset;
}


/*-------------------------------------------------
    core_feof - return 1 if we're at the end
    of file
-------------------------------------------------*/

int core_feof(core_file *file)
{
	/* check for buffered chars */
	if (file->back_char_head != file->back_char_tail)
		return 0;

	/* if the offset == length, we're at EOF */
	return (file->offset >= file->length);
}


/*-------------------------------------------------
    core_fsize - returns the size of a file
-------------------------------------------------*/

UINT64 core_fsize(core_file *file)
{
	return file->length;
}



/***************************************************************************
    FILE READ
***************************************************************************/

/*-------------------------------------------------
    core_fread - read from a file
-------------------------------------------------*/

UINT32 core_fread(core_file *file, void *buffer, UINT32 length)
{
	UINT32 bytes_read = 0;

	/* flush any buffered char */
	file->back_char_head = 0;
	file->back_char_tail = 0;

	/* handle real files */
	if (file->file && file->data == NULL)
	{
		/* if we're within the buffer, consume that first */
		if (file->offset >= file->bufferbase && file->offset < file->bufferbase + file->bufferbytes)
			bytes_read += safe_buffer_copy(file->buffer, file->offset - file->bufferbase, file->bufferbytes, buffer, bytes_read, length);

		/* if we've got a small amount left, read it into the buffer first */
		if (bytes_read < length)
		{
			if (length - bytes_read < sizeof(file->buffer) / 2)
			{
				/* read as much as makes sense into the buffer */
				file->bufferbase = file->offset + bytes_read;
				file->bufferbytes = 0;
				osd_or_zlib_read(file, file->buffer, file->bufferbase, sizeof(file->buffer), &file->bufferbytes);

				/* do a bounded copy from the buffer to the destination */
				bytes_read += safe_buffer_copy(file->buffer, 0, file->bufferbytes, buffer, bytes_read, length);
			}

			/* read the remainder directly from the file */
			else
			{
				UINT32 new_bytes_read = 0;
				osd_or_zlib_read(file, (UINT8 *)buffer + bytes_read, file->offset + bytes_read, length - bytes_read, &new_bytes_read);
				bytes_read += new_bytes_read;
			}
		}
	}

	/* handle RAM-based files */
	else
		bytes_read += safe_buffer_copy(file->data, (UINT32)file->offset, file->length, buffer, bytes_read, length);

	/* return the number of bytes read */
	file->offset += bytes_read;
	return bytes_read;
}


/*-------------------------------------------------
    core_fgetc - read a character from a file
-------------------------------------------------*/

int core_fgetc(core_file *file)
{
	int result;

	/* refresh buffer, if necessary */
	if (file->back_char_head == file->back_char_tail)
	{
		utf16_char utf16_buffer[UTF16_CHAR_MAX];
		char utf8_buffer[UTF8_CHAR_MAX];
		char default_buffer[16];
		unicode_char uchar = (unicode_char) ~0;
		int readlen, charlen;

		/* do we need to check the byte order marks? */
		if (file->offset == 0)
		{
			UINT8 bom[4];
			int pos = 0;

			if (core_fread(file, bom, 4) == 4)
			{
				if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf)
				{
					file->text_type = TFT_UTF8;
					pos = 3;
				}
				else if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xfe && bom[3] == 0xff)
				{
					file->text_type = TFT_UTF32BE;
					pos = 4;
				}
				else if (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0x00 && bom[3] == 0x00)
				{
					file->text_type = TFT_UTF32LE;
					pos = 4;
				}
				else if (bom[0] == 0xfe && bom[1] == 0xff)
				{
					file->text_type = TFT_UTF16BE;
					pos = 2;
				}
				else if (bom[0] == 0xff && bom[1] == 0xfe)
				{
					file->text_type = TFT_UTF16LE;
					pos = 2;
				}
				else
				{
					file->text_type = TFT_OSD;
					pos = 0;
				}
			}
			core_fseek(file, pos, SEEK_SET);
		}

		/* fetch the next character */
		switch (file->text_type)
		{
			default:
			case TFT_OSD:
				readlen = core_fread(file, default_buffer, sizeof(default_buffer));
				if (readlen > 0)
				{
					charlen = osd_uchar_from_osdchar(&uchar, default_buffer, readlen / sizeof(default_buffer[0]));
					core_fseek(file, (INT64) (charlen * sizeof(default_buffer[0])) - readlen, SEEK_CUR);
				}
				break;

			case TFT_UTF8:
				readlen = core_fread(file, utf8_buffer, sizeof(utf8_buffer));
				if (readlen > 0)
				{
					charlen = uchar_from_utf8(&uchar, utf8_buffer, readlen / sizeof(utf8_buffer[0]));
					core_fseek(file, (INT64) (charlen * sizeof(utf8_buffer[0])) - readlen, SEEK_CUR);
				}
				break;

			case TFT_UTF16BE:
				readlen = core_fread(file, utf16_buffer, sizeof(utf16_buffer));
				if (readlen > 0)
				{
					charlen = uchar_from_utf16be(&uchar, utf16_buffer, readlen / sizeof(utf16_buffer[0]));
					core_fseek(file, (INT64) (charlen * sizeof(utf16_buffer[0])) - readlen, SEEK_CUR);
				}
				break;

			case TFT_UTF16LE:
				readlen = core_fread(file, utf16_buffer, sizeof(utf16_buffer));
				if (readlen > 0)
				{
					charlen = uchar_from_utf16le(&uchar, utf16_buffer, readlen / sizeof(utf16_buffer[0]));
					core_fseek(file, (INT64) (charlen * sizeof(utf16_buffer[0])) - readlen, SEEK_CUR);
				}
				break;

			case TFT_UTF32BE:
				if (core_fread(file, &uchar, sizeof(uchar)) == sizeof(uchar))
					uchar = BIG_ENDIANIZE_INT32(uchar);
				break;

			case TFT_UTF32LE:
				if (core_fread(file, &uchar, sizeof(uchar)) == sizeof(uchar))
					uchar = LITTLE_ENDIANIZE_INT32(uchar);
				break;
		}

		if (uchar != ~0)
		{
			/* place the new character in the ring buffer */
			file->back_char_head = 0;
			file->back_char_tail = utf8_from_uchar(file->back_chars, ARRAY_LENGTH(file->back_chars), uchar);
/*          assert(file->back_char_tail != -1);*/
		}
	}

	/* now read from the ring buffer */
	if (file->back_char_head != file->back_char_tail)
	{
		result = file->back_chars[file->back_char_head++];
		file->back_char_head %= ARRAY_LENGTH(file->back_chars);
	}
	else
		result = EOF;

	return result;
}


/*-------------------------------------------------
    core_ungetc - put back a character read from
    a file
-------------------------------------------------*/

int core_ungetc(int c, core_file *file)
{
	file->back_chars[file->back_char_tail++] = (char) c;
	file->back_char_tail %= ARRAY_LENGTH(file->back_chars);
	return c;
}


/*-------------------------------------------------
    core_fgets - read a line from a text file
-------------------------------------------------*/

char *core_fgets(char *s, int n, core_file *file)
{
	char *cur = s;

	/* loop while we have characters */
	while (n > 0)
	{
		int c = core_fgetc(file);
		if (c == EOF)
			break;

		/* if there's a CR, look for an LF afterwards */
		if (c == 0x0d)
		{
			int c2 = core_fgetc(file);
			if (c2 != 0x0a)
				core_ungetc(c2, file);
			*cur++ = 0x0d;
			n--;
			break;
		}

		/* if there's an LF, reinterp as a CR for consistency */
		else if (c == 0x0a)
		{
			*cur++ = 0x0d;
			n--;
			break;
		}

		/* otherwise, pop the character in and continue */
		*cur++ = c;
		n--;
	}

	/* if we put nothing in, return NULL */
	if (cur == s)
		return NULL;

	/* otherwise, terminate */
	if (n > 0)
		*cur++ = 0;
	return s;
}


/*-------------------------------------------------
    core_fbuffer - return a pointer to the file
    buffer; if it doesn't yet exist, load the
    file into RAM first
-------------------------------------------------*/

const void *core_fbuffer(core_file *file)
{
	file_error filerr;
	UINT32 read_length;

	/* if we already have data, just return it */
	if (file->data != NULL || !file->length)
		return file->data;

	/* allocate some memory */
	file->data = (UINT8 *)malloc(file->length);
	if (file->data == NULL)
		return NULL;
	file->data_allocated = TRUE;

	/* read the file */
	filerr = osd_or_zlib_read(file, file->data, 0, file->length, &read_length);
	if (filerr != FILERR_NONE || read_length != file->length)
	{
		free(file->data);
		file->data = NULL;
		return NULL;
	}

	/* close the file because we don't need it anymore */
	osd_close(file->file);
	file->file = NULL;
	return file->data;
}


/*-------------------------------------------------
    core_fload - open a file with the specified
    filename, read it into memory, and return a
    pointer
-------------------------------------------------*/

file_error core_fload(const char *filename, void **data, UINT32 *length)
{
	core_file *file = NULL;
	file_error err;
	UINT64 size;

	/* attempt to open the file */
	err = core_fopen(filename, OPEN_FLAG_READ, &file);
	if (err != FILERR_NONE)
		return err;

	/* get the size */
	size = core_fsize(file);
	if ((UINT32)size != size)
	{
		core_fclose(file);
		return FILERR_OUT_OF_MEMORY;
	}

	/* allocate memory */
	*data = osd_malloc(size);
	if (length != NULL)
		*length = (UINT32)size;

	/* read the data */
	if (core_fread(file, *data, size) != size)
	{
		core_fclose(file);
		free(*data);
		return FILERR_FAILURE;
	}

	/* close the file and return data */
	core_fclose(file);
	return FILERR_NONE;
}

file_error core_fload(const char *filename, dynamic_buffer &data)
{
	core_file *file = NULL;
	file_error err;
	UINT64 size;

	/* attempt to open the file */
	err = core_fopen(filename, OPEN_FLAG_READ, &file);
	if (err != FILERR_NONE)
		return err;

	/* get the size */
	size = core_fsize(file);
	if ((UINT32)size != size)
	{
		core_fclose(file);
		return FILERR_OUT_OF_MEMORY;
	}

	/* allocate memory */
	data.resize(size);

	/* read the data */
	if (core_fread(file, &data[0], size) != size)
	{
		core_fclose(file);
		data.clear();
		return FILERR_FAILURE;
	}

	/* close the file and return data */
	core_fclose(file);
	return FILERR_NONE;
}



/***************************************************************************
    FILE WRITE
***************************************************************************/

/*-------------------------------------------------
    core_fwrite - write to a file
-------------------------------------------------*/

UINT32 core_fwrite(core_file *file, const void *buffer, UINT32 length)
{
	UINT32 bytes_written = 0;

	/* can't write to RAM-based stuff */
	if (file->data != NULL)
		return 0;

	/* flush any buffered char */
	file->back_char_head = 0;
	file->back_char_tail = 0;

	/* invalidate any buffered data */
	file->bufferbytes = 0;

	/* do the write */
	osd_or_zlib_write(file, buffer, file->offset, length, &bytes_written);

	/* return the number of bytes read */
	file->offset += bytes_written;
	file->length = MAX(file->length, file->offset);
	return bytes_written;
}


/*-------------------------------------------------
    core_fputs - write a line to a text file
-------------------------------------------------*/

int core_fputs(core_file *f, const char *s)
{
	char convbuf[1024];
	char *pconvbuf = convbuf;
	int count = 0;

	/* is this the beginning of the file?  if so, write a byte order mark */
	if (f->offset == 0 && !(f->openflags & OPEN_FLAG_NO_BOM))
	{
		*pconvbuf++ = (char)0xef;
		*pconvbuf++ = (char)0xbb;
		*pconvbuf++ = (char)0xbf;
	}

	/* convert '\n' to platform dependant line endings */
	while (*s != 0)
	{
		if (*s == '\n')
		{
			if (CRLF == 1)      /* CR only */
				*pconvbuf++ = 13;
			else if (CRLF == 2) /* LF only */
				*pconvbuf++ = 10;
			else if (CRLF == 3) /* CR+LF */
			{
				*pconvbuf++ = 13;
				*pconvbuf++ = 10;
			}
		}
		else
			*pconvbuf++ = *s;
		s++;

		/* if we overflow, break into chunks */
		if (pconvbuf >= convbuf + ARRAY_LENGTH(convbuf) - 10)
		{
			count += core_fwrite(f, convbuf, pconvbuf - convbuf);
			pconvbuf = convbuf;
		}
	}

	/* final flush */
	if (pconvbuf != convbuf)
		count += core_fwrite(f, convbuf, pconvbuf - convbuf);

	return count;
}


/*-------------------------------------------------
    core_vfprintf - vfprintf to a text file
-------------------------------------------------*/

int core_vfprintf(core_file *f, const char *fmt, va_list va)
{
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, va);
	return core_fputs(f, buf);
}


/*-------------------------------------------------
    core_fprintf - vfprintf to a text file
-------------------------------------------------*/

int CLIB_DECL core_fprintf(core_file *f, const char *fmt, ...)
{
	int rc;
	va_list va;
	va_start(va, fmt);
	rc = core_vfprintf(f, fmt, va);
	va_end(va);
	return rc;
}


/*-------------------------------------------------
    core_truncate - truncate a file
-------------------------------------------------*/

file_error core_truncate(core_file *f, UINT64 offset)
{
	file_error err;

	/* truncate file */
	err = osd_truncate(f->file, offset);
	if (err != FILERR_NONE)
		return err;

	/* and adjust to new length and offset */
	f->length = offset;
	f->offset = MIN(f->offset, f->length);

	return FILERR_NONE;
}



/***************************************************************************
    FILENAME UTILITIES
***************************************************************************/

/*-------------------------------------------------
    core_filename_extract_base - extract the base
    name from a filename; note that this makes
    assumptions about path separators
-------------------------------------------------*/

std::string &core_filename_extract_base(std::string &result, const char *name, bool strip_extension)
{
	/* find the start of the name */
	const char *start = name + strlen(name);
	while (start > name && !is_directory_separator(start[-1]))
		start--;

	/* copy the rest into an astring */
	result.assign(start);

	/* chop the extension if present */
	if (strip_extension)
		result = result.substr(0, result.find_last_of('.'));
	return result;
}


/*-------------------------------------------------
    core_filename_ends_with - does the given
    filename end with the specified extension?
-------------------------------------------------*/

int core_filename_ends_with(const char *filename, const char *extension)
{
	int namelen = strlen(filename);
	int extlen = strlen(extension);
	int matches = TRUE;

	/* work backwards checking for a match */
	while (extlen > 0)
		if (tolower((UINT8)filename[--namelen]) != tolower((UINT8)extension[--extlen]))
		{
			matches = FALSE;
			break;
		}

	return matches;
}



/***************************************************************************
    MISC HELPERS
***************************************************************************/

/*-------------------------------------------------
    safe_buffer_copy - copy safely from one
    bounded buffer to another
-------------------------------------------------*/

static UINT32 safe_buffer_copy(const void *source, UINT32 sourceoffs, UINT32 sourcelen, void *dest, UINT32 destoffs, UINT32 destlen)
{
	UINT32 sourceavail = sourcelen - sourceoffs;
	UINT32 destavail = destlen - destoffs;
	UINT32 bytes_to_copy = MIN(sourceavail, destavail);
	if (bytes_to_copy > 0)
		memcpy((UINT8 *)dest + destoffs, (const UINT8 *)source + sourceoffs, bytes_to_copy);
	return bytes_to_copy;
}


/*-------------------------------------------------
    osd_or_zlib_read - wrapper for osd_read that
    handles zlib-compressed data
-------------------------------------------------*/

static file_error osd_or_zlib_read(core_file *file, void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	/* if no compression, just pass through */
	if (file->zdata == NULL)
		return osd_read(file->file, buffer, offset, length, actual);

	/* if the offset doesn't match the next offset, fail */
	if (offset != file->zdata->nextoffset)
		return FILERR_INVALID_ACCESS;

	/* set up the destination */
	file->zdata->stream.next_out = (Bytef *)buffer;
	file->zdata->stream.avail_out = length;
	while (file->zdata->stream.avail_out != 0)
	{
		file_error filerr;
		UINT32 actualdata;
		int zerr = Z_OK;

		/* if we didn't make progress, report an error or the end */
		if (file->zdata->stream.avail_in != 0)
			zerr = inflate(&file->zdata->stream, Z_SYNC_FLUSH);
		if (zerr != Z_OK)
		{
			*actual = length - file->zdata->stream.avail_out;
			file->zdata->nextoffset += *actual;
			return (zerr == Z_STREAM_END) ? FILERR_NONE : FILERR_INVALID_DATA;
		}

		/* fetch more data if needed */
		if (file->zdata->stream.avail_in == 0)
		{
			filerr = osd_read(file->file, file->zdata->buffer, file->zdata->realoffset, sizeof(file->zdata->buffer), &actualdata);
			if (filerr != FILERR_NONE)
				return filerr;
			file->zdata->realoffset += actualdata;
			file->zdata->stream.next_in = file->zdata->buffer;
			file->zdata->stream.avail_in = sizeof(file->zdata->buffer);
		}
	}

	/* we read everything */
	*actual = length;
	file->zdata->nextoffset += *actual;
	return FILERR_NONE;
}


/*-------------------------------------------------
    osd_or_zlib_write - wrapper for osd_write that
    handles zlib-compressed data
-------------------------------------------------*/

/**
 * @fn  static file_error osd_or_zlib_write(core_file *file, const void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
 *
 * @brief   OSD or zlib write.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   buffer          The buffer.
 * @param   offset          The offset.
 * @param   length          The length.
 * @param [in,out]  actual  If non-null, the actual.
 *
 * @return  A file_error.
 */

static file_error osd_or_zlib_write(core_file *file, const void *buffer, UINT64 offset, UINT32 length, UINT32 *actual)
{
	/* if no compression, just pass through */
	if (file->zdata == NULL)
		return osd_write(file->file, buffer, offset, length, actual);

	/* if the offset doesn't match the next offset, fail */
	if (offset != file->zdata->nextoffset)
		return FILERR_INVALID_ACCESS;

	/* set up the source */
	file->zdata->stream.next_in = (Bytef *)buffer;
	file->zdata->stream.avail_in = length;
	while (file->zdata->stream.avail_in != 0)
	{
		file_error filerr;
		UINT32 actualdata;
		int zerr;

		/* if we didn't make progress, report an error or the end */
		zerr = deflate(&file->zdata->stream, Z_NO_FLUSH);
		if (zerr != Z_OK)
		{
			*actual = length - file->zdata->stream.avail_in;
			file->zdata->nextoffset += *actual;
			return FILERR_INVALID_DATA;
		}

		/* write more data if we are full up */
		if (file->zdata->stream.avail_out == 0)
		{
			filerr = osd_write(file->file, file->zdata->buffer, file->zdata->realoffset, sizeof(file->zdata->buffer), &actualdata);
			if (filerr != FILERR_NONE)
				return filerr;
			file->zdata->realoffset += actualdata;
			file->zdata->stream.next_out = file->zdata->buffer;
			file->zdata->stream.avail_out = sizeof(file->zdata->buffer);
		}
	}

	/* we read everything */
	*actual = length;
	file->zdata->nextoffset += *actual;
	return FILERR_NONE;
}
