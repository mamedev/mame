/*********************************************************************

    flopimg.c

    Floppy disk image abstraction code

*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#include "osdcore.h"
#include "ioprocs.h"
#include "flopimg.h"
#include "pool.h"
#include "imageutl.h"

#define TRACK_LOADED		0x01
#define TRACK_DIRTY			0x02


struct _floppy_image
{
	struct io_generic io;

	const struct FloppyFormat *floppy_option;
	struct FloppyCallbacks format;

	/* loaded track stuff */
	int loaded_track_head;
	int loaded_track_index;
	UINT32 loaded_track_size;
	void *loaded_track_data;
	UINT8 loaded_track_status;
	UINT8 flags;

	/* tagging system */
	object_pool *tags;
	void *tag_data;
};



struct _floppy_params
{
	int param;
	int value;
};



static floperr_t floppy_track_unload(floppy_image_legacy *floppy);

OPTION_GUIDE_START(floppy_option_guide)
	OPTION_INT('H', "heads",			"Heads")
	OPTION_INT('T', "tracks",			"Tracks")
	OPTION_INT('S', "sectors",			"Sectors")
	OPTION_INT('L', "sectorlength",		"Sector Bytes")
	OPTION_INT('I', "interleave",		"Interleave")
	OPTION_INT('F', "firstsectorid",	"First Sector")
OPTION_GUIDE_END


static void floppy_close_internal(floppy_image_legacy *floppy, int close_file);

/*********************************************************************
    opening, closing and creating of floppy images
*********************************************************************/

/* basic floppy_image_legacy initialization common to floppy_open() and floppy_create() */
static floppy_image_legacy *floppy_init(void *fp, const struct io_procs *procs, int flags)
{
	floppy_image_legacy *floppy;

	floppy = (floppy_image_legacy *)malloc(sizeof(struct _floppy_image));
	if (!floppy)
		return NULL;

	memset(floppy, 0, sizeof(*floppy));
	floppy->tags = pool_alloc_lib(NULL);
	floppy->tag_data = NULL;
	floppy->io.file = fp;
	floppy->io.procs = procs;
	floppy->io.filler = 0xFF;
	floppy->flags = (UINT8) flags;
	return floppy;
}



/* main code for identifying and maybe opening a disk image; not exposed
 * directly because this function is big and hideous */
static floperr_t floppy_open_internal(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *floppy_options, int max_options, int flags, floppy_image_legacy **outfloppy,
	int *outoption)
{
	floperr_t err;
	floppy_image_legacy *floppy;
	int best_option = -1;
	int best_vote = 0;
	int vote;
	size_t i;

	floppy = floppy_init(fp, procs, flags);
	if (!floppy)
	{
		err = FLOPPY_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* vote on the best format */
	for (i = 0; (i < max_options) && floppy_options[i].construct; i++)
	{
		if (!extension || !floppy_options[i].extensions || image_find_extension(floppy_options[i].extensions, extension))
		{
			if (floppy_options[i].identify)
			{
				vote = 0;
				err = floppy_options[i].identify(floppy, &floppy_options[i], &vote);
				if (err)
					goto done;
			}
			else
			{
				vote = 1;
			}

			/* is this option a better one? */
			if (vote > best_vote)
			{
				best_vote = vote;
				best_option = i;
			}
		}
	}

	/* did we find a format? */
	if (best_option == -1)
	{
		err = FLOPPY_ERROR_INVALIDIMAGE;
		goto done;
	}

	if (outfloppy)
	{
		/* call the format constructor */
		err = floppy_options[best_option].construct(floppy, &floppy_options[best_option], NULL);
		if (err)
			goto done;

		floppy->floppy_option = &floppy_options[best_option];
	}
	if (best_vote != 100)
	{
		printf("Loading image that is not 100%% recognized\n");
	}
	err = FLOPPY_ERROR_SUCCESS;

done:
	/* if we have a floppy disk and we either errored or are not keeping it, close it */
	if (floppy && (!outfloppy || err))
	{
		floppy_close_internal(floppy, FALSE);
		floppy = NULL;
	}

	if (outoption)
		*outoption = err ? -1 : best_option;
	if (outfloppy)
		*outfloppy = floppy;
	return err;
}



floperr_t floppy_identify(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *formats, int *identified_format)
{
	return floppy_open_internal(fp, procs, extension, formats, INT_MAX, FLOPPY_FLAGS_READONLY, NULL, identified_format);
}



floperr_t floppy_open(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *format, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(fp, procs, extension, format, 1, flags, outfloppy, NULL);
}



floperr_t floppy_open_choices(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *formats, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(fp, procs, extension, formats, INT_MAX, flags, outfloppy, NULL);
}



static floperr_t option_to_floppy_error(optreserr_t oerr)
{
	floperr_t err;
	switch(oerr) {
	case OPTIONRESOLUTION_ERROR_SUCCESS:
		err = FLOPPY_ERROR_SUCCESS;
		break;
	case OPTIONRESOLUTION_ERROR_OUTOFMEMORY:
		err = FLOPPY_ERROR_OUTOFMEMORY;
		break;
	case OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE:
	case OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED:
	case OPTIONRESOLUTION_ERROR_PARAMNOTFOUND:
	case OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED:
	case OPTIONRESOLUTION_ERROR_BADPARAM:
	case OPTIONRESOLUTION_ERROR_SYNTAX:
	default:
		err = FLOPPY_ERROR_INTERNAL;
		break;
	};
	return err;
}



floperr_t floppy_create(void *fp, const struct io_procs *procs, const struct FloppyFormat *format, option_resolution *parameters, floppy_image_legacy **outfloppy)
{
	floppy_image_legacy *floppy = NULL;
	optreserr_t oerr;
	floperr_t err;
	int heads, tracks, h, t;
	option_resolution *alloc_resolution = NULL;

	assert(format);

	/* create the new image */
	floppy = floppy_init(fp, procs, 0);
	if (!floppy)
	{
		err = FLOPPY_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* if this format expects creation parameters and none were specified, create some */
	if (!parameters && format->param_guidelines)
	{
		alloc_resolution = option_resolution_create(floppy_option_guide, format->param_guidelines);
		if (!alloc_resolution)
		{
			err = FLOPPY_ERROR_OUTOFMEMORY;
			goto done;
		}
		parameters = alloc_resolution;
	}

	/* finish the parameters, if specified */
	if (parameters)
	{
		oerr = option_resolution_finish(parameters);
		if (oerr)
		{
			err = option_to_floppy_error(oerr);
			goto done;
		}
	}

	/* call the format constructor */
	err = format->construct(floppy, format, parameters);
	if (err)
		goto done;

	/* format the disk, ignoring if formatting not implemented */
	if (floppy->format.format_track)
	{
		heads = floppy_get_heads_per_disk(floppy);
		tracks = floppy_get_tracks_per_disk(floppy);

		for (h = 0; h < heads; h++)
		{
			for (t = 0; t < tracks; t++)
			{
				err = floppy->format.format_track(floppy, h, t, parameters);
				if (err)
					goto done;
			}
		}
	}

	/* call the post_format function, if present */
	if (floppy->format.post_format)
	{
		err = floppy->format.post_format(floppy, parameters);
		if (err)
			goto done;
	}

	floppy->floppy_option = format;
	err = FLOPPY_ERROR_SUCCESS;

done:
	if (err && floppy)
	{
		floppy_close_internal(floppy, FALSE);
		floppy = NULL;
	}

	if (outfloppy)
		*outfloppy = floppy;
	else if (floppy)
		floppy_close_internal(floppy, FALSE);

	if (alloc_resolution)
		option_resolution_close(alloc_resolution);
	return err;
}



static void floppy_close_internal(floppy_image_legacy *floppy, int close_file)
{
	if (floppy) {
		floppy_track_unload(floppy);

		if(floppy->floppy_option && floppy->floppy_option->destruct)
			floppy->floppy_option->destruct(floppy, floppy->floppy_option);
		if (close_file)
			io_generic_close(&floppy->io);
		if (floppy->loaded_track_data)
			free(floppy->loaded_track_data);
		pool_free_lib(floppy->tags);

		free(floppy);
	}
}



void floppy_close(floppy_image_legacy *floppy)
{
	floppy_close_internal(floppy, TRUE);
}



/*********************************************************************
    functions useful in format constructors
*********************************************************************/

struct FloppyCallbacks *floppy_callbacks(floppy_image_legacy *floppy)
{
	assert(floppy);
	return &floppy->format;
}



void *floppy_tag(floppy_image_legacy *floppy)
{
	assert(floppy);
	return floppy->tag_data;
}



void *floppy_create_tag(floppy_image_legacy *floppy, size_t tagsize)
{
	floppy->tag_data = pool_malloc_lib(floppy->tags,tagsize);
	return floppy->tag_data;
}



UINT8 floppy_get_filler(floppy_image_legacy *floppy)
{
	return floppy->io.filler;
}



void floppy_set_filler(floppy_image_legacy *floppy, UINT8 filler)
{
	floppy->io.filler = filler;
}



/*********************************************************************
    calls for accessing the raw disk image
*********************************************************************/

void floppy_image_read(floppy_image_legacy *floppy, void *buffer, UINT64 offset, size_t length)
{
	io_generic_read(&floppy->io, buffer, offset, length);
}



void floppy_image_write(floppy_image_legacy *floppy, const void *buffer, UINT64 offset, size_t length)
{
	io_generic_write(&floppy->io, buffer, offset, length);
}



void floppy_image_write_filler(floppy_image_legacy *floppy, UINT8 filler, UINT64 offset, size_t length)
{
	io_generic_write_filler(&floppy->io, filler, offset, length);
}



UINT64 floppy_image_size(floppy_image_legacy *floppy)
{
	return io_generic_size(&floppy->io);
}



/*********************************************************************
    calls for accessing disk image data
*********************************************************************/

static floperr_t floppy_readwrite_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset,
	void *buffer, size_t buffer_len, int writing, int indexed, int ddam)
{
	floperr_t err;
	const struct FloppyCallbacks *fmt;
	size_t this_buffer_len;
	UINT8 *alloc_buf = NULL;
	UINT32 sector_length;
	UINT8 *buffer_ptr = (UINT8 *)buffer;
	floperr_t (*read_sector)(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
	floperr_t (*write_sector)(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);

	fmt = floppy_callbacks(floppy);

	/* choose proper calls for indexed vs non-indexed */
	if (indexed)
	{
		read_sector = fmt->read_indexed_sector;
		write_sector = fmt->write_indexed_sector;
		if (!fmt->get_indexed_sector_info)
		{
			err = FLOPPY_ERROR_UNSUPPORTED;
			goto done;
		}
	}
	else
	{
		read_sector = fmt->read_sector;
		write_sector = fmt->write_sector;
		if (!fmt->get_sector_length)
		{
			err = FLOPPY_ERROR_UNSUPPORTED;
			goto done;
		}
	}

	/* check to make sure that the operation is supported */
	if (!read_sector || (writing && !write_sector))
	{
		err = FLOPPY_ERROR_UNSUPPORTED;
		goto done;
	}

	/* main loop */
	while(buffer_len > 0)
	{
		/* find out the size of this sector */
		if (indexed)
			err = fmt->get_indexed_sector_info(floppy, head, track, sector, NULL, NULL, NULL, &sector_length, NULL);
		else
			err = fmt->get_sector_length(floppy, head, track, sector, &sector_length);
		if (err)
			goto done;

		/* do we even do anything with this sector? */
		if (offset < sector_length)
		{
			/* ok we will be doing something */
			if ((offset > 0) || (buffer_len < sector_length))
			{
				/* we will be doing an partial read/write; in other words we
                 * will not be reading/writing a full sector */
				if (alloc_buf) free(alloc_buf);
				alloc_buf = (UINT8*)malloc(sector_length);
				if (!alloc_buf)
				{
					err = FLOPPY_ERROR_OUTOFMEMORY;
					goto done;
				}

				/* read the sector (we need to do this even when writing */
				err = read_sector(floppy, head, track, sector, alloc_buf, sector_length);
				if (err)
					goto done;

				this_buffer_len = MIN(buffer_len, sector_length - offset);

				if (writing)
				{
					memcpy(alloc_buf + offset, buffer_ptr, this_buffer_len);

					err = write_sector(floppy, head, track, sector, alloc_buf, sector_length, ddam);
					if (err)
						goto done;
				}
				else
				{
					memcpy(buffer_ptr, alloc_buf + offset, this_buffer_len);
				}
				offset += this_buffer_len;
				offset %= sector_length;
			}
			else
			{
				this_buffer_len = sector_length;

				if (writing)
					err = write_sector(floppy, head, track, sector, buffer_ptr, sector_length, ddam);
				else
					err = read_sector(floppy, head, track, sector, buffer_ptr, sector_length);
				if (err)
					goto done;
			}
		}
		else
		{
			/* skip this sector */
			offset -= sector_length;
			this_buffer_len = 0;
		}

		buffer_ptr += this_buffer_len;
		buffer_len -= this_buffer_len;
		sector++;
	}

	err = FLOPPY_ERROR_SUCCESS;

done:
	if (alloc_buf)
		free(alloc_buf);
	return err;
}



floperr_t floppy_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset,	void *buffer, size_t buffer_len)
{
	return floppy_readwrite_sector(floppy, head, track, sector, offset, buffer, buffer_len, FALSE, FALSE, 0);
}



floperr_t floppy_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, const void *buffer, size_t buffer_len, int ddam)
{
	return floppy_readwrite_sector(floppy, head, track, sector, offset, (void *) buffer, buffer_len, TRUE, FALSE, ddam);
}



floperr_t floppy_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset,	void *buffer, size_t buffer_len)
{
	return floppy_readwrite_sector(floppy, head, track, sector_index, offset, buffer, buffer_len, FALSE, TRUE, 0);
}



floperr_t floppy_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, const void *buffer, size_t buffer_len, int ddam)
{
	return floppy_readwrite_sector(floppy, head, track, sector_index, offset, (void *) buffer, buffer_len, TRUE, TRUE, ddam);
}


static floperr_t floppy_get_track_data_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	floperr_t err;
	const struct FloppyCallbacks *callbacks;

	*offset = 0;
	callbacks = floppy_callbacks(floppy);
	if (callbacks->get_track_data_offset)
	{
		err = callbacks->get_track_data_offset(floppy, head, track, offset);
		if (err)
			return err;
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t floppy_read_track_offset(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buffer_len)
{
	floperr_t err;
	const struct FloppyCallbacks *format;

	format = floppy_callbacks(floppy);

	if (!format->read_track)
		return FLOPPY_ERROR_UNSUPPORTED;

	err = floppy_track_unload(floppy);
	if (err)
		return err;

	err = format->read_track(floppy, head, track, offset, buffer, buffer_len);
	if (err)
		return err;

	return FLOPPY_ERROR_SUCCESS;
}



floperr_t floppy_read_track(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len)
{
	return floppy_read_track_offset(floppy, head, track, 0, buffer, buffer_len);
}



floperr_t floppy_read_track_data(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len)
{
	floperr_t err;
	UINT64 offset;

	err = floppy_get_track_data_offset(floppy, head, track, &offset);
	if (err)
		return err;

	return floppy_read_track_offset(floppy, head, track, offset, buffer, buffer_len);
}



static floperr_t floppy_write_track_offset(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buffer_len)
{
	floperr_t err;

	/* track writing supported? */
	if (!floppy_callbacks(floppy)->write_track)
		return FLOPPY_ERROR_UNSUPPORTED;

	/* read only? */
	if (floppy->flags & FLOPPY_FLAGS_READONLY)
		return FLOPPY_ERROR_READONLY;

	err = floppy_track_unload(floppy);
	if (err)
		return err;

	err = floppy_callbacks(floppy)->write_track(floppy, head, track, offset, buffer, buffer_len);
	if (err)
		return err;

	return FLOPPY_ERROR_SUCCESS;
}



floperr_t floppy_write_track(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len)
{
	return floppy_write_track_offset(floppy, head, track, 0, buffer, buffer_len);
}



floperr_t floppy_write_track_data(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len)
{
	floperr_t err;
	UINT64 offset;

	err = floppy_get_track_data_offset(floppy, head, track, &offset);
	if (err)
		return err;

	return floppy_write_track_offset(floppy, head, track, offset, buffer, buffer_len);
}



floperr_t floppy_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *parameters)
{
	floperr_t err;
	struct FloppyCallbacks *format;
	option_resolution *alloc_resolution = NULL;
	optreserr_t oerr;

	/* supported? */
	format = floppy_callbacks(floppy);
	if (!format->format_track)
	{
		err = FLOPPY_ERROR_UNSUPPORTED;
		goto done;
	}

	/* create a dummy resolution; if no parameters were specified */
	if (!parameters)
	{
		alloc_resolution = option_resolution_create(floppy_option_guide, floppy->floppy_option->param_guidelines);
		if (!alloc_resolution)
		{
			err = FLOPPY_ERROR_OUTOFMEMORY;
			goto done;
		}
		parameters = alloc_resolution;
	}

	oerr = option_resolution_finish(parameters);
	if (oerr)
	{
		err = option_to_floppy_error(oerr);
		goto done;
	}

	err = format->format_track(floppy, head, track, parameters);
	if (err)
		goto done;

done:
	if (alloc_resolution)
		option_resolution_close(alloc_resolution);
	return err;
}



int floppy_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return floppy_callbacks(floppy)->get_tracks_per_disk(floppy);
}



int floppy_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return floppy_callbacks(floppy)->get_heads_per_disk(floppy);
}



UINT32 floppy_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	const struct FloppyCallbacks *fmt;

	fmt = floppy_callbacks(floppy);
	if (!fmt->get_track_size)
		return 0;

	return fmt->get_track_size(floppy, head, track);
}



floperr_t floppy_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	const struct FloppyCallbacks *fmt;

	fmt = floppy_callbacks(floppy);
	if (!fmt->get_sector_length)
		return FLOPPY_ERROR_UNSUPPORTED;

	return fmt->get_sector_length(floppy, head, track, sector, sector_length);
}



floperr_t floppy_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	const struct FloppyCallbacks *fmt;

	fmt = floppy_callbacks(floppy);
	if (!fmt->get_indexed_sector_info)
		return FLOPPY_ERROR_UNSUPPORTED;

	return fmt->get_indexed_sector_info(floppy, head, track, sector_index, cylinder, side, sector, sector_length, flags);
}



floperr_t floppy_get_sector_count(floppy_image_legacy *floppy, int head, int track, int *sector_count)
{
	floperr_t err;
	int sector_index = 0;

	do
	{
		err = floppy_get_indexed_sector_info(floppy, head, track, sector_index, NULL, NULL, NULL, NULL, NULL);
		if (!err)
			sector_index++;
	}
	while(!err);

	if (sector_index && (err == FLOPPY_ERROR_SEEKERROR))
		err = FLOPPY_ERROR_SUCCESS;
	if (sector_count)
		*sector_count = err ? 0 : sector_index;
	return err;
}



int floppy_is_read_only(floppy_image_legacy *floppy)
{
	return floppy->flags & FLOPPY_FLAGS_READONLY;
}



UINT8 floppy_random_byte(floppy_image_legacy *floppy)
{
	/* can't use mame_rand(); this might not be in the core */
#ifdef rand
#undef rand
#endif
	return rand();
}



/*********************************************************************
    calls for track based IO
*********************************************************************/

floperr_t floppy_load_track(floppy_image_legacy *floppy, int head, int track, int dirtify, void **track_data, size_t *track_length)
{
	floperr_t err;
	void *new_loaded_track_data;
	UINT32 track_size;

	/* have we already loaded this track? */
	if (((floppy->loaded_track_status & TRACK_LOADED) == 0) || (head != floppy->loaded_track_head) || (track != floppy->loaded_track_index))
	{
		err = floppy_track_unload(floppy);
		if (err)
			goto error;

		track_size = floppy_callbacks(floppy)->get_track_size(floppy, head, track);

		if (floppy->loaded_track_data) free(floppy->loaded_track_data);
		new_loaded_track_data = malloc(track_size);
		if (!new_loaded_track_data)
		{
			err = FLOPPY_ERROR_OUTOFMEMORY;
			goto error;
		}

		floppy->loaded_track_data = new_loaded_track_data;
		floppy->loaded_track_size = track_size;
		floppy->loaded_track_head = head;
		floppy->loaded_track_index = track;

		err = floppy_callbacks(floppy)->read_track(floppy, floppy->loaded_track_head, floppy->loaded_track_index, 0, floppy->loaded_track_data, floppy->loaded_track_size);
		if (err)
			goto error;

		floppy->loaded_track_status |= TRACK_LOADED | (dirtify ? TRACK_DIRTY : 0);
	}
	else
		floppy->loaded_track_status |= (dirtify ? TRACK_DIRTY : 0);

	if (track_data)
		*track_data = floppy->loaded_track_data;
	if (track_length)
		*track_length = floppy->loaded_track_size;
	return FLOPPY_ERROR_SUCCESS;

error:
	if (track_data)
		*track_data = NULL;
	if (track_length)
		*track_length = 0;
	return err;
}



static floperr_t floppy_track_unload(floppy_image_legacy *floppy)
{
	int err;
	if (floppy->loaded_track_status & TRACK_DIRTY)
	{
		err = floppy_callbacks(floppy)->write_track(floppy, floppy->loaded_track_head, floppy->loaded_track_index, 0, floppy->loaded_track_data, floppy->loaded_track_size);
		if (err)
			return (floperr_t)err;
	}

	floppy->loaded_track_status &= ~(TRACK_LOADED | TRACK_DIRTY);
	return FLOPPY_ERROR_SUCCESS;
}



/*********************************************************************
    accessors for meta information about the image
*********************************************************************/

const char *floppy_format_description(floppy_image_legacy *floppy)
{
	return floppy->floppy_option->description;
}



/*********************************************************************
    misc calls
*********************************************************************/

const char *floppy_error(floperr_t err)
{
	static const char *const error_messages[] =
	{
		"The operation completed successfully",
		"Fatal internal error",
		"This operation is unsupported",
		"Out of memory",
		"Seek error",
		"Invalid image",
		"Attempted to write to read only image",
		"No space left on image",
		"Parameter out of range",
		"Required parameter not specified"
	};

	if ((err < 0) || (err >= ARRAY_LENGTH(error_messages)))
		return NULL;
	return error_messages[err];
}


LEGACY_FLOPPY_OPTIONS_START(default)
LEGACY_FLOPPY_OPTIONS_END


//////////////////////////////////////////////////////////
/// New implementation
//////////////////////////////////////////////////////////

floppy_image::floppy_image(void *fp, const struct io_procs *procs,const struct floppy_format_def *formats)
{
	m_io.file = fp;
	m_io.procs = procs;
	m_io.filler = 0xFF;
	m_formats = formats;
}

floppy_image::~floppy_image()
{
	close();
}

void floppy_image::image_read(void *buffer, UINT64 offset, size_t length)
{
	io_generic_read(&m_io, buffer, offset, length);
}

void floppy_image::image_write(const void *buffer, UINT64 offset, size_t length)
{
	io_generic_write(&m_io, buffer, offset, length);
}

void floppy_image::image_write_filler(UINT8 filler, UINT64 offset, size_t length)
{
	io_generic_write_filler(&m_io, filler, offset, length);
}

UINT64 floppy_image::image_size()
{
	return io_generic_size(&m_io);
}

void floppy_image::close_internal(bool close_file)
{
	if (close_file)
		io_generic_close(&m_io);
}

void floppy_image::close()
{
	close_internal(TRUE);
}

void floppy_image::set_meta_data(UINT16 tracks, UINT8 sides, UINT16 rpm, UINT16 bitrate)
{
	m_tracks = tracks;
	m_sides  = sides;
	m_rpm    = rpm;
	m_bitrate= bitrate;
}

const struct floppy_format_def *floppy_image::identify(int *best)
{
	const struct floppy_format_def *retVal = NULL;
	int best_vote = 0;
	*best = -1;
	for (int i = 0; m_formats[i].type; i++)
	{
		floppy_image_format_t *t = (m_formats[i].type)(m_formats[i].name,m_formats[i].extensions,m_formats[i].description,m_formats[i].param_guidelines);
		int vote = t->identify(this);
		/* is this option a better one? */
		if (vote > best_vote)
		{
			best_vote = vote;
			*best = i;
			retVal = &m_formats[i];
		}
	}
	return retVal;
}

bool floppy_image::load(int num)
{
	floppy_image_format_t *t = (m_formats[num].type)(m_formats[num].name,m_formats[num].extensions,m_formats[num].description,m_formats[num].param_guidelines);
	return t->load(this);
}

floppy_image_format_t::floppy_image_format_t(const char *name,const char *extensions,const char *description,const char *param_guidelines)
{
	m_name = name;
	m_extensions = extensions;
	m_description = description;
	m_param_guidelines = param_guidelines;
}
floppy_image_format_t::~floppy_image_format_t()
{
}

bool floppy_image_format_t::type_no_data(int type) const
{
	return type == CRC_CCITT_START ||
		type == CRC_AMIGA_START ||
		type == CRC_END ||
		type == SECTOR_LOOP_START ||
		type == SECTOR_LOOP_END ||
		type == END;
}

bool floppy_image_format_t::type_data_mfm(int type, int p1, const gen_crc_info *crcs) const
{
	return !type_no_data(type) && 
		type != RAW &&
		type != RAWBITS &&
		(type != CRC || (crcs[p1].type != CRC_CCITT && crcs[p1].type != CRC_AMIGA));
}

void floppy_image_format_t::collect_crcs(const desc_e *desc, gen_crc_info *crcs)
{
	memset(crcs, 0, MAX_CRC_COUNT * sizeof(*crcs));
	for(int i=0; i != MAX_CRC_COUNT; i++)
		crcs[i].write = -1;

	for(int i=0; desc[i].type != END; i++)
		switch(desc[i].type) {
		case CRC_CCITT_START:
			crcs[desc[i].p1].type = CRC_CCITT;
			break;
		case CRC_AMIGA_START:
			crcs[desc[i].p1].type = CRC_AMIGA;
			break;
		}

	for(int i=0; desc[i].type != END; i++)
		if(desc[i].type == CRC) {
			int j;
			for(j = i+1; desc[j].type != END && type_no_data(desc[j].type); j++);
			crcs[desc[i].p1].fixup_mfm_clock = type_data_mfm(desc[j].type, desc[j].p1, crcs);
		}
}

int floppy_image_format_t::crc_cells_size(int type) const
{
	switch(type) {
	case CRC_CCITT: return 32;
	case CRC_AMIGA: return 64;
	default: return 0;
	}
}

bool floppy_image_format_t::bit_r(UINT8 *buffer, int offset)
{
	return (buffer[offset >> 3] >> ((offset & 7) ^ 7)) & 1;
}

void floppy_image_format_t::bit_w(UINT8 *buffer, int offset, bool val)
{
	if(val)
		buffer[offset >> 3] |= 0x80 >> (offset & 7);
	else
		buffer[offset >> 3] &= ~(0x80 >> (offset & 7));
}

void floppy_image_format_t::raw_w(UINT8 *buffer, int &offset, int n, UINT32 val)
{
	for(int i=n-1; i>=0; i--)
		bit_w(buffer, offset++, (val >> i) & 1);
}

void floppy_image_format_t::mfm_w(UINT8 *buffer, int &offset, int n, UINT32 val)
{
	int prec = offset ? bit_r(buffer, offset-1) : 0;
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, offset++, !(prec || bit));
		bit_w(buffer, offset++, bit);
		prec = bit;
	}
}

void floppy_image_format_t::mfm_half_w(UINT8 *buffer, int &offset, int start_bit, UINT32 val)
{
	int prec = offset ? bit_r(buffer, offset-1) : 0;
	for(int i=start_bit; i>=0; i-=2) {
		int bit = (val >> i) & 1;
		bit_w(buffer, offset++, !(prec || bit));
		bit_w(buffer, offset++, bit);
		prec = bit;
	}
}

void floppy_image_format_t::fixup_crc_amiga(UINT8 *buffer, const gen_crc_info *crc)
{
	UINT16 res = 0;
	int size = crc->end - crc->start;
	for(int i=1; i<size; i+=2)
		if(bit_r(buffer, crc->start + i))
			res = res ^ (0x8000 >> ((i >> 1) & 15));
	int offset = crc->write;
	mfm_w(buffer, offset, 16, 0);
	mfm_w(buffer, offset, 16, res);
}

void floppy_image_format_t::fixup_crc_ccitt(UINT8 *buffer, const gen_crc_info *crc)
{
	UINT32 res = 0xffff;
	int size = crc->end - crc->start;
	for(int i=1; i<size; i+=2) {
		res <<= 1;
		if(bit_r(buffer, crc->start + i))
			res ^= 0x10000;
		if(res & 0x10000)
			res ^= 0x11021;
	}
	int offset = crc->write;
	mfm_w(buffer, offset, 16, res);
}

void floppy_image_format_t::fixup_crcs(UINT8 *buffer, gen_crc_info *crcs)
{
	for(int i=0; i != MAX_CRC_COUNT; i++)
		if(crcs[i].write != -1) {
			switch(crcs[i].type) {
			case CRC_AMIGA: fixup_crc_amiga(buffer, crcs+i); break;
			case CRC_CCITT: fixup_crc_ccitt(buffer, crcs+i); break;
			}
			if(crcs[i].fixup_mfm_clock) {
				int offset = crcs[i].write + crc_cells_size(crcs[i].type);
				bit_w(buffer, offset, !((offset ? bit_r(buffer, offset-1) : false) || bit_r(buffer, offset+1)));
			}
			crcs[i].write = -1;
		}
}

void floppy_image_format_t::generate_track(const desc_e *desc, UINT8 track, UINT8 head, const desc_s *sect, int sect_count, int track_size, UINT8 *buffer)
{
	memset(buffer, 0, (track_size+7)/8);

	gen_crc_info crcs[MAX_CRC_COUNT];
	collect_crcs(desc, crcs);

	int offset = 0;
	int index = 0;
	int sector_loop_start = 0;
	int sector_id = 0;
	int sector_limit = 0;

	while(desc[index].type != END) {
		//		printf("%d.%d.%d (%d) - %d %d\n", desc[index].type, desc[index].p1, desc[index].p2, index, offset, offset/8);
		switch(desc[index].type) {
		case MFM:
			for(int i=0; i<desc[index].p2; i++)
				mfm_w(buffer, offset, 8, desc[index].p1);
			break;

		case MFMBITS:
			mfm_w(buffer, offset, desc[index].p2, desc[index].p1);
			break;

		case RAW:
			for(int i=0; i<desc[index].p2; i++)
				raw_w(buffer, offset, 16, desc[index].p1);
			break;

		case RAWBITS:
			raw_w(buffer, offset, desc[index].p2, desc[index].p1);
			break;

		case TRACK_ID:
			mfm_w(buffer, offset, 8, track);
			break;

		case HEAD_ID:
			mfm_w(buffer, offset, 8, head);
			break;

		case SECTOR_ID:
			mfm_w(buffer, offset, 8, sector_id);
			break;

		case SIZE_ID: {
			int size = sect[sector_id].size;
			int id;
			for(id = 0; size > 128; size >>=1, id++);
			mfm_w(buffer, offset, 8, id);
			break;
		}

		case OFFSET_ID_O:
			mfm_half_w(buffer, offset, 7, track*2+head);
			break;

		case OFFSET_ID_E:
			mfm_half_w(buffer, offset, 6, track*2+head);
			break;

		case SECTOR_ID_O:
			mfm_half_w(buffer, offset, 7, sector_id);
			break;

		case SECTOR_ID_E:
			mfm_half_w(buffer, offset, 6, sector_id);
			break;

		case REMAIN_O:
			mfm_half_w(buffer, offset, 7, desc[index].p1 - sector_id);
			break;

		case REMAIN_E:
			mfm_half_w(buffer, offset, 6, desc[index].p1 - sector_id);
			break;

		case SECTOR_LOOP_START:
			fixup_crcs(buffer, crcs);
			sector_loop_start = index;
			sector_id = desc[index].p1;
			sector_limit = desc[index].p2;
			break;

		case SECTOR_LOOP_END:
			fixup_crcs(buffer, crcs);
			if(sector_id < sector_limit) {
				sector_id++;
				index = sector_loop_start;
			}
			break;

		case CRC_AMIGA_START:
		case CRC_CCITT_START:
			crcs[desc[index].p1].start = offset;
			break;

		case CRC_END:
			crcs[desc[index].p1].end = offset;
			break;

		case CRC:
			crcs[desc[index].p1].write = offset;
			offset += crc_cells_size(crcs[desc[index].p1].type);
			break;

		case SECTOR_DATA: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_id);
			for(int i=0; i != csect->size; i++)
				mfm_w(buffer, offset, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_O: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_id);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, offset, 7, csect->data[i]);
			break;
		}

		case SECTOR_DATA_E: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_id);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, offset, 6, csect->data[i]);
			break;
		}

		default:
			printf("%d.%d.%d (%d) unhandled\n", desc[index].type, desc[index].p1, desc[index].p2, index);
			break;
		}
		index++;
	}

	fixup_crcs(buffer, crcs);
}

