// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg.c

    Floppy disk image abstraction code

*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

#include "emu.h" // emu_fatalerror
#include "osdcore.h"
#include "ioprocs.h"
#include "flopimg.h"
#include "pool.h"
#include "imageutl.h"

#define TRACK_LOADED        0x01
#define TRACK_DIRTY         0x02


struct floppy_image_legacy
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



struct floppy_params
{
	int param;
	int value;
};



static floperr_t floppy_track_unload(floppy_image_legacy *floppy);

OPTION_GUIDE_START(floppy_option_guide)
	OPTION_INT('H', "heads",            "Heads")
	OPTION_INT('T', "tracks",           "Tracks")
	OPTION_INT('S', "sectors",          "Sectors")
	OPTION_INT('L', "sectorlength",     "Sector Bytes")
	OPTION_INT('I', "interleave",       "Interleave")
	OPTION_INT('F', "firstsectorid",    "First Sector")
OPTION_GUIDE_END


static void floppy_close_internal(floppy_image_legacy *floppy, int close_file);

/*********************************************************************
    opening, closing and creating of floppy images
*********************************************************************/

/* basic floppy_image_legacy initialization common to floppy_open() and floppy_create() */
static floppy_image_legacy *floppy_init(void *fp, const struct io_procs *procs, int flags)
{
	floppy_image_legacy *floppy;

	floppy = (floppy_image_legacy *)malloc(sizeof(floppy_image_legacy));
	if (!floppy)
		return nullptr;

	memset(floppy, 0, sizeof(*floppy));
	floppy->tags = pool_alloc_lib(nullptr);
	floppy->tag_data = nullptr;
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
		err = floppy_options[best_option].construct(floppy, &floppy_options[best_option], nullptr);
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
		floppy = nullptr;
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
	return floppy_open_internal(fp, procs, extension, formats, INT_MAX, FLOPPY_FLAGS_READONLY, nullptr, identified_format);
}



floperr_t floppy_open(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *format, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(fp, procs, extension, format, 1, flags, outfloppy, nullptr);
}



floperr_t floppy_open_choices(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *formats, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(fp, procs, extension, formats, INT_MAX, flags, outfloppy, nullptr);
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
	floppy_image_legacy *floppy = nullptr;
	optreserr_t oerr;
	floperr_t err;
	int heads, tracks, h, t;
	option_resolution *alloc_resolution = nullptr;

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
		floppy = nullptr;
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
	dynamic_buffer alloc_buf;
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
			err = fmt->get_indexed_sector_info(floppy, head, track, sector, nullptr, nullptr, nullptr, &sector_length, nullptr);
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
				alloc_buf.resize(sector_length);

				/* read the sector (we need to do this even when writing */
				err = read_sector(floppy, head, track, sector, &alloc_buf[0], sector_length);
				if (err)
					goto done;

				this_buffer_len = MIN(buffer_len, sector_length - offset);

				if (writing)
				{
					memcpy(&alloc_buf[offset], buffer_ptr, this_buffer_len);

					err = write_sector(floppy, head, track, sector, &alloc_buf[0], sector_length, ddam);
					if (err)
						goto done;
				}
				else
				{
					memcpy(buffer_ptr, &alloc_buf[offset], this_buffer_len);
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
	return err;
}



floperr_t floppy_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset,  void *buffer, size_t buffer_len)
{
	return floppy_readwrite_sector(floppy, head, track, sector, offset, buffer, buffer_len, FALSE, FALSE, 0);
}



floperr_t floppy_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, const void *buffer, size_t buffer_len, int ddam)
{
	return floppy_readwrite_sector(floppy, head, track, sector, offset, (void *) buffer, buffer_len, TRUE, FALSE, ddam);
}



floperr_t floppy_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset,    void *buffer, size_t buffer_len)
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
	option_resolution *alloc_resolution = nullptr;
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
		err = floppy_get_indexed_sector_info(floppy, head, track, sector_index, nullptr, nullptr, nullptr, nullptr, nullptr);
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
		*track_data = nullptr;
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
		return nullptr;
	return error_messages[err];
}


LEGACY_FLOPPY_OPTIONS_START(default)
LEGACY_FLOPPY_OPTIONS_END


// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    New implementation

****************************************************************************/


floppy_image::floppy_image(int _tracks, int _heads, UINT32 _form_factor)
{
	tracks = _tracks;
	heads = _heads;

	form_factor = _form_factor;
	variant = 0;

	track_array.resize(tracks*4+1);
	for(int i=0; i<tracks*4+1; i++)
		track_array[i].resize(heads);
}

floppy_image::~floppy_image()
{
}

void floppy_image::get_maximal_geometry(int &_tracks, int &_heads) const
{
	_tracks = tracks;
	_heads = heads;
}

void floppy_image::get_actual_geometry(int &_tracks, int &_heads)
{
	int maxt = (tracks-1)*4, maxh = heads-1;

	while(maxt >= 0) {
		for(int i=0; i<=maxh; i++)
			if(!track_array[maxt][i].cell_data.empty())
				goto track_done;
		maxt--;
	}
	track_done:
	if(maxt >= 0)
		while(maxh >= 0) {
			for(int i=0; i<=maxt; i++)
				if(!track_array[i][maxh].cell_data.empty())
					goto head_done;
			maxh--;
		}
	head_done:
	_tracks = (maxt+4)/4;
	_heads = maxh+1;
}

int floppy_image::get_resolution() const
{
	int mask = 0;
	for(int i=0; i<=(tracks-1)*4; i++)
		for(int j=0; j<heads; j++)
			if(!track_array[i][j].cell_data.empty())
				mask |= 1 << (i & 3);
	if(mask & 0xa)
		return 2;
	if(mask & 0x4)
		return 1;
	return 0;
}

const char *floppy_image::get_variant_name(UINT32 form_factor, UINT32 variant)
{
	switch(variant) {
	case SSSD: return "Single side, single density";
	case SSDD: return "Single side, double density";
	case SSQD: return "Single side, quad density";
	case DSDD: return "Double side, double density";
	case DSQD: return "Double side, quad density";
	case DSHD: return "Double side, high density";
	case DSED: return "Double side, extended density";
	}
	return "Unknown";
}

floppy_image_format_t::floppy_image_format_t()
{
	next = nullptr;
}

floppy_image_format_t::~floppy_image_format_t()
{
}

void floppy_image_format_t::append(floppy_image_format_t *_next)
{
	if(next)
		next->append(_next);
	else
		next = _next;
}

bool floppy_image_format_t::save(io_generic *, floppy_image *)
{
	return false;
}

bool floppy_image_format_t::extension_matches(const char *file_name) const
{
	const char *ext = strrchr(file_name, '.');
	if(!ext)
		return false;
	ext++;
	int elen = strlen(ext);
	const char *rext = extensions();
	for(;;) {
		const char *next_ext = strchr(rext, ',');
		int rlen = next_ext ? next_ext - rext : strlen(rext);
		if(rlen == elen && !memcmp(ext, rext, rlen))
			return true;
		if(next_ext)
			next_ext = next_ext +1;
		else
			break;
	}
	return false;
}

bool floppy_image_format_t::type_no_data(int type) const
{
	return
		type == CRC_CCITT_START ||
		type == CRC_CCITT_FM_START ||
		type == CRC_AMIGA_START ||
		type == CRC_CBM_START ||
		type == CRC_MACHEAD_START ||
		type == CRC_FCS_START ||
		type == CRC_VICTOR_HDR_START ||
		type == CRC_VICTOR_DATA_START ||
		type == CRC_END ||
		type == SECTOR_LOOP_START ||
		type == SECTOR_LOOP_END ||
		type == END;
}

bool floppy_image_format_t::type_data_mfm(int type, int p1, const gen_crc_info *crcs) const
{
	return
		type == MFM ||
		type == MFMBITS ||
		type == TRACK_ID ||
		type == HEAD_ID ||
		type == HEAD_ID_SWAP ||
		type == SECTOR_ID ||
		type == SIZE_ID ||
		type == OFFSET_ID_O ||
		type == OFFSET_ID_E ||
		type == SECTOR_ID_O ||
		type == SECTOR_ID_E ||
		type == REMAIN_O ||
		type == REMAIN_E ||
		type == SECTOR_DATA ||
		type == SECTOR_DATA_O ||
		type == SECTOR_DATA_E ||
		(type == CRC && (crcs[p1].type == CRC_CCITT || crcs[p1].type == CRC_AMIGA));
}

void floppy_image_format_t::collect_crcs(const desc_e *desc, gen_crc_info *crcs) const
{
	memset(crcs, 0, MAX_CRC_COUNT * sizeof(*crcs));
	for(int i=0; i != MAX_CRC_COUNT; i++)
		crcs[i].write = -1;

	for(int i=0; desc[i].type != END; i++)
		switch(desc[i].type) {
		case CRC_CCITT_START:
			crcs[desc[i].p1].type = CRC_CCITT;
			break;
		case CRC_CCITT_FM_START:
			crcs[desc[i].p1].type = CRC_CCITT_FM;
			break;
		case CRC_AMIGA_START:
			crcs[desc[i].p1].type = CRC_AMIGA;
			break;
		case CRC_CBM_START:
			crcs[desc[i].p1].type = CRC_CBM;
			break;
		case CRC_MACHEAD_START:
			crcs[desc[i].p1].type = CRC_MACHEAD;
			break;
		case CRC_FCS_START:
			crcs[desc[i].p1].type = CRC_FCS;
			break;
		case CRC_VICTOR_HDR_START:
			crcs[desc[i].p1].type = CRC_VICTOR_HDR;
			break;
		case CRC_VICTOR_DATA_START:
			crcs[desc[i].p1].type = CRC_VICTOR_DATA;
			break;
		}

	for(int i=0; desc[i].type != END; i++)
		if(desc[i].type == CRC) {
			int j;
			for(j = i+1; desc[j].type != END && type_no_data(desc[j].type); j++) {};
			crcs[desc[i].p1].fixup_mfm_clock = type_data_mfm(desc[j].type, desc[j].p1, crcs);
		}
}

int floppy_image_format_t::crc_cells_size(int type) const
{
	switch(type) {
	case CRC_CCITT: return 32;
	case CRC_CCITT_FM: return 32;
	case CRC_AMIGA: return 64;
	case CRC_CBM: return 10;
	case CRC_MACHEAD: return 8;
	case CRC_FCS: return 20;
	case CRC_VICTOR_HDR: return 10;
	case CRC_VICTOR_DATA: return 20;
	default: return 0;
	}
}

bool floppy_image_format_t::bit_r(const std::vector<UINT32> &buffer, int offset)
{
	return (buffer[offset] & floppy_image::MG_MASK) == MG_1;
}

UINT32 floppy_image_format_t::bitn_r(const std::vector<UINT32> &buffer, int offset, int count)
{
	UINT32 r = 0;
	for(int i=0; i<count; i++)
		r = (r << 1) | (UINT32) bit_r(buffer, offset+i);
	return r;
}

void floppy_image_format_t::bit_w(std::vector<UINT32> &buffer, bool val, UINT32 size, int offset)
{
	buffer[offset] = (val ? MG_1 : MG_0) | size;
}

void floppy_image_format_t::bit_w(std::vector<UINT32> &buffer, bool val, UINT32 size)
{
	buffer.push_back((val ? MG_1 : MG_0) | size);
}

void floppy_image_format_t::raw_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size)
{
	for(int i=n-1; i>=0; i--)
		bit_w(buffer, (val >> i) & 1, size);
}

void floppy_image_format_t::raw_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset)
{
	for(int i=n-1; i>=0; i--)
		bit_w(buffer, (val >> i) & 1, size, offset++);
}

void floppy_image_format_t::mfm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size)
{
	int prec = buffer.empty() ? 0 : bit_r(buffer, buffer.size()-1);
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size);
		bit_w(buffer, bit, size);
		prec = bit;
	}
}

void floppy_image_format_t::mfm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset)
{
	int prec = offset ? bit_r(buffer, offset-1) : 0;
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size, offset++);
		bit_w(buffer, bit,            size, offset++);
		prec = bit;
	}
}

void floppy_image_format_t::fm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size)
{
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, true, size);
		bit_w(buffer, bit,  size);
	}
}

void floppy_image_format_t::fm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset)
{
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, true, size, offset++);
		bit_w(buffer, bit,  size, offset++);
	}
}

void floppy_image_format_t::mfm_half_w(std::vector<UINT32> &buffer, int start_bit, UINT32 val, UINT32 size)
{
	int prec = buffer.empty() ? 0 : bit_r(buffer, buffer.size()-1);
	for(int i=start_bit; i>=0; i-=2) {
		int bit = (val >> i) & 1;
		bit_w(buffer, !(prec || bit), size);
		bit_w(buffer, bit,            size);
		prec = bit;
	}
}

void floppy_image_format_t::gcr5_w(std::vector<UINT32> &buffer, UINT8 val, UINT32 size)
{
	UINT32 e0 = gcr5fw_tb[val >> 4];
	UINT32 e1 = gcr5fw_tb[val & 0x0f];
	raw_w(buffer, 5, e0, size);
	raw_w(buffer, 5, e1, size);
}

void floppy_image_format_t::gcr5_w(std::vector<UINT32> &buffer, UINT8 val, UINT32 size, int offset)
{
	UINT32 e0 = gcr5fw_tb[val >> 4];
	UINT32 e1 = gcr5fw_tb[val & 0x0f];
	raw_w(buffer, 5, e0, size, offset);
	raw_w(buffer, 5, e1, size, offset+5);
}

void floppy_image_format_t::_8n1_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size)
{
	bit_w(buffer, 0, size);
	for(int i=n-1; i>=0; i--) {
		int bit = (val >> i) & 1;
		bit_w(buffer, bit, size);
	}
	bit_w(buffer, 1, size);
}

void floppy_image_format_t::fixup_crc_amiga(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	UINT16 res = 0;
	int size = crc->end - crc->start;
	for(int i=1; i<size; i+=2)
		if(bit_r(buffer, crc->start + i))
			res = res ^ (0x8000 >> ((i >> 1) & 15));
	mfm_w(buffer, 16,   0, 1000, crc->write);
	mfm_w(buffer, 16, res, 1000, crc->write+32);
}

void floppy_image_format_t::fixup_crc_cbm(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	UINT8 v = 0;
	for(int o = crc->start; o < crc->end; o+=10) {
		v = v ^ (gcr5bw_tb[bitn_r(buffer, o, 5)] << 4);
		v = v ^ gcr5bw_tb[bitn_r(buffer, o+5, 5)];
	}
	gcr5_w(buffer, v, 1000, crc->write);
}

UINT16 floppy_image_format_t::calc_crc_ccitt(const std::vector<UINT32> &buffer, int start, int end)
{
	UINT32 res = 0xffff;
	int size = end - start;
	for(int i=1; i<size; i+=2) {
		res <<= 1;
		if(bit_r(buffer, start + i))
			res ^= 0x10000;
		if(res & 0x10000)
			res ^= 0x11021;
	}
	return res;
}

void floppy_image_format_t::fixup_crc_ccitt(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	mfm_w(buffer, 16, calc_crc_ccitt(buffer, crc->start, crc->end), 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_ccitt_fm(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	fm_w(buffer, 16, calc_crc_ccitt(buffer, crc->start, crc->end), 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_machead(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	UINT8 v = 0;
	for(int o = crc->start; o < crc->end; o+=8)
		v = v ^ gcr6bw_tb[bitn_r(buffer, o, 8)];
	raw_w(buffer, 8, gcr6fw_tb[v], 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_fcs(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	// TODO
}

void floppy_image_format_t::fixup_crc_victor_header(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	UINT8 v = 0;
	for(int o = crc->start; o < crc->end; o+=10)
		v += ((gcr5bw_tb[bitn_r(buffer, o, 5)] << 4) | gcr5bw_tb[bitn_r(buffer, o+5, 5)]);
	gcr5_w(buffer, v, 1000, crc->write);
}

void floppy_image_format_t::fixup_crc_victor_data(std::vector<UINT32> &buffer, const gen_crc_info *crc)
{
	UINT16 v = 0;
	for(int o = crc->start; o < crc->end; o+=10)
		v += ((gcr5bw_tb[bitn_r(buffer, o, 5)] << 4) | gcr5bw_tb[bitn_r(buffer, o+5, 5)]);
	gcr5_w(buffer, v & 0xff, 1000, crc->write);
	gcr5_w(buffer, v >> 8, 1000, crc->write+10);
}

void floppy_image_format_t::fixup_crcs(std::vector<UINT32> &buffer, gen_crc_info *crcs)
{
	for(int i=0; i != MAX_CRC_COUNT; i++)
		if(crcs[i].write != -1) {
			switch(crcs[i].type) {
			case CRC_AMIGA:         fixup_crc_amiga(buffer, crcs+i); break;
			case CRC_CBM:           fixup_crc_cbm(buffer, crcs+i); break;
			case CRC_CCITT:         fixup_crc_ccitt(buffer, crcs+i); break;
			case CRC_CCITT_FM:      fixup_crc_ccitt_fm(buffer, crcs+i); break;
			case CRC_MACHEAD:       fixup_crc_machead(buffer, crcs+i); break;
			case CRC_FCS:           fixup_crc_fcs(buffer, crcs+i); break;
			case CRC_VICTOR_HDR:    fixup_crc_victor_header(buffer, crcs+i); break;
			case CRC_VICTOR_DATA:   fixup_crc_victor_data(buffer, crcs+i); break;
			}
			if(crcs[i].fixup_mfm_clock) {
				int offset = crcs[i].write + crc_cells_size(crcs[i].type);
				bit_w(buffer, !((offset ? bit_r(buffer, offset-1) : false) || bit_r(buffer, offset+1)), 1000, offset);
			}
			crcs[i].write = -1;
		}
}

UINT32 floppy_image_format_t::gcr6_encode(UINT8 va, UINT8 vb, UINT8 vc)
{
	UINT32 r;
	r = gcr6fw_tb[((va >> 2) & 0x30) | ((vb >> 4) & 0x0c) | ((vc >> 6) & 0x03)] << 24;
	r |= gcr6fw_tb[va & 0x3f] << 16;
	r |= gcr6fw_tb[vb & 0x3f] << 8;
	r |= gcr6fw_tb[vc & 0x3f];
	return r;
}

void floppy_image_format_t::gcr6_decode(UINT8 e0, UINT8 e1, UINT8 e2, UINT8 e3, UINT8 &va, UINT8 &vb, UINT8 &vc)
{
	e0 = gcr6bw_tb[e0];
	e1 = gcr6bw_tb[e1];
	e2 = gcr6bw_tb[e2];
	e3 = gcr6bw_tb[e3];

	va = ((e0 << 2) & 0xc0) | e1;
	vb = ((e0 << 4) & 0xc0) | e2;
	vc = ((e0 << 6) & 0xc0) | e3;
}

UINT16 floppy_image_format_t::gcr4_encode(UINT8 va)
{
	return (va << 7) | va | 0xaaaa;
}

UINT8 floppy_image_format_t::gcr4_decode(UINT8 e0, UINT8 e1)
{
	return ((e0 << 1) & 0xaa) | (e1 & 0x55);
}


int floppy_image_format_t::calc_sector_index(int num, int interleave, int skew, int total_sectors, int track_head)
{
	int i = 0;
	int sec = 0;
	// use interleave
	while (i != num)
	{
		i++;
		i += interleave;
		i %= total_sectors;
		sec++;
		// This line prevents lock-ups of the emulator when the interleave is not appropriate
		if (sec > total_sectors)
			throw emu_fatalerror("Format error: interleave %d not appropriate for %d sectors per track\n", interleave, total_sectors);
	}
	// use skew param
	sec -= track_head * skew;
	sec %= total_sectors;
	if (sec < 0) sec += total_sectors;
	return sec;
}

void floppy_image_format_t::generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image *image)
{
	std::vector<UINT32> buffer;

	gen_crc_info crcs[MAX_CRC_COUNT];
	collect_crcs(desc, crcs);

	int index = 0;
	int sector_loop_start = 0;
	int sector_idx = 0;
	int sector_cnt = 0;
	int sector_limit = 0;
	int sector_interleave = 0;
	int sector_skew = 0;

	while(desc[index].type != END) {
		switch(desc[index].type) {
		case FM:
			for(int i=0; i<desc[index].p2; i++)
				fm_w(buffer, 8, desc[index].p1);
			break;

		case MFM:
			for(int i=0; i<desc[index].p2; i++)
				mfm_w(buffer, 8, desc[index].p1);
			break;

		case MFMBITS:
			mfm_w(buffer, desc[index].p2, desc[index].p1);
			break;

		case GCR5:
			for(int i=0; i<desc[index].p2; i++)
				gcr5_w(buffer, desc[index].p1);
			break;

		case _8N1:
			for(int i=0; i<desc[index].p2; i++)
				_8n1_w(buffer, 8, desc[index].p1);
			break;

		case RAW:
			for(int i=0; i<desc[index].p2; i++)
				raw_w(buffer, 16, desc[index].p1);
			break;

		case RAWBYTE:
			for(int i=0; i<desc[index].p2; i++)
				raw_w(buffer, 8, desc[index].p1);
			break;

		case RAWBITS:
			raw_w(buffer, desc[index].p2, desc[index].p1);
			break;

		case SYNC_GCR5:
			for(int i=0; i<desc[index].p1; i++)
				raw_w(buffer, 10, 0xffff);
			break;

		case TRACK_ID:
			mfm_w(buffer, 8, track);
			break;

		case TRACK_ID_FM:
			fm_w(buffer, 8, track);
			break;

		case TRACK_ID_DOS2_GCR5:
			gcr5_w(buffer, 1 + (track >> 1) + (head * 35));
			break;

		case TRACK_ID_DOS25_GCR5:
			gcr5_w(buffer, 1 + track + (head * 77));
			break;

		case TRACK_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[track & 0x3f]);
			break;

		case TRACK_ID_8N1:
			_8n1_w(buffer, 8, track);
			break;

		case TRACK_ID_VICTOR_GCR5:
			gcr5_w(buffer, track + (head * 0x80));
			break;

		case HEAD_ID:
			mfm_w(buffer, 8, head);
			break;

		case HEAD_ID_FM:
			fm_w(buffer, 8, head);
			break;

		case HEAD_ID_SWAP:
			mfm_w(buffer, 8, !head);
			break;

		case TRACK_HEAD_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[(track & 0x40 ? 1 : 0) | (head ? 0x20 : 0)]);
			break;

		case SECTOR_ID:
			mfm_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_FM:
			fm_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_GCR5:
			gcr5_w(buffer, sect[sector_idx].sector_id);
			break;

		case SECTOR_ID_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[sect[sector_idx].sector_id]);
			break;

		case SECTOR_ID_8N1:
			_8n1_w(buffer, 8, sect[sector_idx].sector_id);
			break;

		case SIZE_ID: {
			int size = sect[sector_idx].size;
			int id;
			for(id = 0; size > 128; size >>=1, id++) {};
			mfm_w(buffer, 8, id);
			break;
		}

		case SIZE_ID_FM: {
			int size = sect[sector_idx].size;
			int id;
			for(id = 0; size > 128; size >>=1, id++) {};
			fm_w(buffer, 8, id);
			break;
		}

		case SECTOR_INFO_GCR6:
			raw_w(buffer, 8, gcr6fw_tb[sect[sector_idx].sector_info]);
			break;

		case OFFSET_ID_O:
			mfm_half_w(buffer, 7, track*2+head);
			break;

		case OFFSET_ID_E:
			mfm_half_w(buffer, 6, track*2+head);
			break;

		case SECTOR_ID_O:
			mfm_half_w(buffer, 7, sector_idx);
			break;

		case SECTOR_ID_E:
			mfm_half_w(buffer, 6, sector_idx);
			break;

		case REMAIN_O:
			mfm_half_w(buffer, 7, desc[index].p1 - sector_idx);
			break;

		case REMAIN_E:
			mfm_half_w(buffer, 6, desc[index].p1 - sector_idx);
			break;

		case SECTOR_LOOP_START:
			fixup_crcs(buffer, crcs);
			sector_loop_start = index;
			sector_idx = desc[index].p1;
			sector_cnt = sector_idx;
			sector_limit = desc[index].p2 == -1 ? sector_idx+sect_count-1 : desc[index].p2;
			sector_idx = calc_sector_index(sector_cnt,sector_interleave,sector_skew,sector_limit+1,track*2 + head);
			break;

		case SECTOR_LOOP_END:
			fixup_crcs(buffer, crcs);
			if(sector_cnt < sector_limit) {
				sector_cnt++;
				sector_idx = calc_sector_index(sector_cnt,sector_interleave,sector_skew,sector_limit+1,track*2 + head);
				index = sector_loop_start;
			}
			break;

		case SECTOR_INTERLEAVE_SKEW:
			sector_interleave = desc[index].p1;
			sector_skew = desc[index].p2;
			break;

		case CRC_AMIGA_START:
		case CRC_CBM_START:
		case CRC_CCITT_START:
		case CRC_CCITT_FM_START:
		case CRC_MACHEAD_START:
		case CRC_FCS_START:
		case CRC_VICTOR_HDR_START:
		case CRC_VICTOR_DATA_START:
			crcs[desc[index].p1].start = buffer.size();
			break;

		case CRC_END:
			crcs[desc[index].p1].end = buffer.size();
			break;

		case CRC:
			crcs[desc[index].p1].write = buffer.size();
			buffer.resize(buffer.size() + crc_cells_size(crcs[desc[index].p1].type));
			break;

		case SECTOR_DATA: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_w(buffer, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_FM: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				fm_w(buffer, 8, csect->data[i]);
			break;
		}

		case SECTOR_DATA_O: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, 7, csect->data[i]);
			break;
		}

		case SECTOR_DATA_E: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				mfm_half_w(buffer, 6, csect->data[i]);
			break;
		}

		case SECTOR_DATA_GCR5: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				gcr5_w(buffer, csect->data[i]);
			break;
		}

		case SECTOR_DATA_MAC: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			const UINT8 *data = csect->data;
			int size = csect->size;
			UINT8 ca = 0, cb = 0, cc = 0;
			for(int i=0; i < size; i+=3) {
				int dt = size-i;
				UINT8 va = data[i];
				UINT8 vb = dt > 1 ? data[i+1] : 0;
				UINT8 vc = dt > 2 ? data[i+2] : 0;

				cc = (cc << 1) | (cc >> 7);
				int suma = ca + va + (cc & 1);
				ca = suma;
				va = va ^ cc;
				int sumb = cb + vb + (suma >> 8);
				cb = sumb;
				vb = vb ^ ca;
				cc = cc + vc + (sumb >> 8);
				vc = vc ^ cb;

				int nb = dt > 2 ? 32 : dt > 1 ? 24 : 16;
				raw_w(buffer, nb, gcr6_encode(va, vb, vc) >> (32-nb));
			}
			raw_w(buffer, 32, gcr6_encode(ca, cb, cc));
			break;
		}

		case SECTOR_DATA_8N1: {
			const desc_s *csect = sect + (desc[index].p1 >= 0 ? desc[index].p1 : sector_idx);
			for(int i=0; i != csect->size; i++)
				_8n1_w(buffer, 8, csect->data[i]);
			break;
		}

		default:
			printf("%d.%d.%d (%d) unhandled\n", desc[index].type, desc[index].p1, desc[index].p2, index);
			break;
		}
		index++;
	}

	if(int(buffer.size()) != track_size)
		throw emu_fatalerror("Wrong track size in generate_track, expected %d, got %d\n", track_size, int(buffer.size()));

	fixup_crcs(buffer, crcs);

	generate_track_from_levels(track, head, buffer, 0, image);
}

void floppy_image_format_t::normalize_times(std::vector<UINT32> &buffer)
{
	unsigned int total_sum = 0;
	for(unsigned int i=0; i != buffer.size(); i++)
		total_sum += buffer[i] & floppy_image::TIME_MASK;

	unsigned int current_sum = 0;
	for(unsigned int i=0; i != buffer.size(); i++) {
		UINT32 time = buffer[i] & floppy_image::TIME_MASK;
		buffer[i] = (buffer[i] & floppy_image::MG_MASK) | (200000000ULL * current_sum / total_sum);
		current_sum += time;
	}
}

void floppy_image_format_t::generate_track_from_bitstream(int track, int head, const UINT8 *trackbuf, int track_size, floppy_image *image, int subtrack)
{
	// Maximal number of cells which happens when the buffer is all 1
	std::vector<UINT32> &dest = image->get_buffer(track, head, subtrack);
	dest.clear();

	UINT32 cbit = floppy_image::MG_A;
	UINT32 count = 0;
	for(int i=0; i != track_size; i++)
		if(trackbuf[i >> 3] & (0x80 >> (i & 7))) {
			dest.push_back(cbit | (count+1));
			cbit = cbit == floppy_image::MG_A ? floppy_image::MG_B : floppy_image::MG_A;
			count = 1;
		} else
			count += 2;

	if(count)
		dest.push_back(cbit | count);

	normalize_times(dest);
	image->set_write_splice_position(track, head, 0, subtrack);
}

void floppy_image_format_t::generate_track_from_levels(int track, int head, std::vector<UINT32> &trackbuf, int splice_pos, floppy_image *image)
{
	// Retrieve the angular splice pos before messing with the data
	splice_pos = splice_pos % trackbuf.size();
	UINT32 splice_angular_pos = trackbuf[splice_pos] & floppy_image::TIME_MASK;

	// Check if we need to invert a cell to get an even number of
	// transitions on the whole track
	//
	// Also check if all MG values are valid

	int transition_count = 0;
	for(auto & elem : trackbuf) {
		switch(elem & floppy_image::MG_MASK) {
		case MG_1:
			transition_count++;
			break;

		case MG_W:
			throw emu_fatalerror("Weak bits not yet handled, track %d head %d\n", track, head);

		case MG_0:
		case floppy_image::MG_N:
		case floppy_image::MG_D:
			break;

		case floppy_image::MG_A:
		case floppy_image::MG_B:
		default:
			throw emu_fatalerror("Incorrect MG information in generate_track_from_levels, track %d head %d\n", track, head);
		}
	}

	if(transition_count & 1) {
		int pos = splice_pos;
		while((trackbuf[pos] & floppy_image::MG_MASK) != MG_0 && (trackbuf[pos] & floppy_image::MG_MASK) != MG_1) {
			pos++;
			if(pos == int(trackbuf.size()))
				pos = 0;
			if(pos == splice_pos)
				goto meh;
		}
		if((trackbuf[pos] & floppy_image::MG_MASK) == MG_0)
			trackbuf[pos] = (trackbuf[pos] & floppy_image::TIME_MASK) | MG_1;
		else
			trackbuf[pos] = (trackbuf[pos] & floppy_image::TIME_MASK) | MG_0;

	meh:
		;

	}

	// Maximal number of cells which happens when the buffer is all MG_1/MG_N alternated, which would be 3/2
	std::vector<UINT32> &dest = image->get_buffer(track, head);
	dest.clear();

	UINT32 cbit = floppy_image::MG_A;
	UINT32 count = 0;
	for(auto & elem : trackbuf) {
		UINT32 bit = elem & floppy_image::MG_MASK;
		UINT32 time = elem & floppy_image::TIME_MASK;
		if(bit == MG_0) {
			count += time;
			continue;
		}
		if(bit == MG_1) {
			count += time >> 1;
			dest.push_back(cbit | count);
			cbit = cbit == floppy_image::MG_A ? floppy_image::MG_B : floppy_image::MG_A;
			count = time - (time >> 1);
			continue;
		}
		dest.push_back(cbit | count);
		dest.push_back(elem);
		count = 0;
	}

	if(count)
		dest.push_back(cbit | count);

	normalize_times(dest);
	image->set_write_splice_position(track, head, splice_angular_pos);
}

const UINT8 floppy_image_format_t::gcr5fw_tb[0x10] =
{
	0x0a, 0x0b, 0x12, 0x13, 0x0e, 0x0f, 0x16, 0x17,
	0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15
};

const UINT8 floppy_image_format_t::gcr5bw_tb[0x20] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x00, 0x01, 0x00, 0x0c, 0x04, 0x05,
	0x00, 0x00, 0x02, 0x03, 0x00, 0x0f, 0x06, 0x07,
	0x00, 0x09, 0x0a, 0x0b, 0x00, 0x0d, 0x0e, 0x00
};

const UINT8 floppy_image_format_t::gcr6fw_tb[0x40] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

const UINT8 floppy_image_format_t::gcr6bw_tb[0x100] =
{
	// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x03, 0x00, 0x04, 0x05, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0x00, 0x00, 0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0x00, 0x00, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x00, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x1c, 0x1d, 0x1e,
	0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x20, 0x21, 0x00, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x2a, 0x2b, 0x00, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
	0x00, 0x00, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x00, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
};

//  Atari ST Fastcopy Pro layouts

#define SECTOR_42_HEADER(cid)   \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   MFM, 0x42, 1 },         \
	{   MFM, 0x02, 1 },         \
	{ CRC_END, cid },           \
	{ CRC, cid }

#define NORMAL_SECTOR(cid)      \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   SECTOR_ID },            \
	{   SIZE_ID },              \
	{ CRC_END, cid },           \
	{ CRC, cid },               \
	{ MFM, 0x4e, 22 },          \
	{ MFM, 0x00, 12 },          \
	{ CRC_CCITT_START, cid+1 }, \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfb, 1 },         \
	{   SECTOR_DATA, -1 },      \
	{ CRC_END, cid+1 },         \
	{ CRC, cid+1 }

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_9[] = {
	{ MFM, 0x4e, 501 },
	{ MFM, 0x00, 12 },

	SECTOR_42_HEADER(1),

	{ MFM, 0x4e, 22 },
	{ MFM, 0x00, 12 },

	{ SECTOR_LOOP_START, 0, 8 },
	NORMAL_SECTOR(2),
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },

	SECTOR_42_HEADER(4),

	{ MFM, 0x4e, 157 },

	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_0[] = {
	{ MFM, 0x4e, 46 },
	SECTOR_42_HEADER(1),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 9 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_1[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 9, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 8 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_2[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 8, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 7 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_3[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 7, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 6 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_4[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 6, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 5 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_5[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 5, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 4 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_6[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 4, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 3 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_7[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 3, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 2 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_8[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 2, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 1 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_10_9[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 1, 9 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 0, 0 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e *const floppy_image_format_t::atari_st_fcp_10[10] = {
	atari_st_fcp_10_0,
	atari_st_fcp_10_1,
	atari_st_fcp_10_2,
	atari_st_fcp_10_3,
	atari_st_fcp_10_4,
	atari_st_fcp_10_5,
	atari_st_fcp_10_6,
	atari_st_fcp_10_7,
	atari_st_fcp_10_8,
	atari_st_fcp_10_9,
};


const floppy_image_format_t::desc_e floppy_image_format_t::atari_st_fcp_11[] = {
	{ MFM, 0x4e, 1 },
	{ SECTOR_INTERLEAVE_SKEW, 1, 1},
	{ SECTOR_LOOP_START,  0,  10 }, { MFM, 0x4e, 2 }, { MFM, 0x00, 2 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

#undef SECTOR_42_HEADER
#undef NORMAL_SECTOR

const floppy_image_format_t::desc_e *floppy_image_format_t::atari_st_fcp_get_desc(int track, int head, int head_count, int sect_count)
{
	switch(sect_count) {
	case 9:
		return atari_st_fcp_9;
	case 10:
		return atari_st_fcp_10[(track*head_count + head) % 10];
	case 11:
		return atari_st_fcp_11;
	}
	return nullptr;
}

//  Amiga layouts

const floppy_image_format_t::desc_e floppy_image_format_t::amiga_11[] = {
	{ SECTOR_LOOP_START, 0, 10 },
	{   MFM, 0x00, 2 },
	{   RAW, 0x4489, 2 },
	{   CRC_AMIGA_START, 1 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_O },
	{     SECTOR_ID_O },
	{     REMAIN_O, 11 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_E },
	{     SECTOR_ID_E },
	{     REMAIN_E, 11 },
	{     MFM, 0x00, 16 },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   CRC, 2 },
	{   CRC_AMIGA_START, 2 },
	{     SECTOR_DATA_O, -1 },
	{     SECTOR_DATA_E, -1 },
	{   CRC_END, 2 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x00, 266 },
	{ END }
};

const floppy_image_format_t::desc_e floppy_image_format_t::amiga_22[] = {
	{ SECTOR_LOOP_START, 0, 21 },
	{   MFM, 0x00, 2 },
	{   RAW, 0x4489, 2 },
	{   CRC_AMIGA_START, 1 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_O },
	{     SECTOR_ID_O },
	{     REMAIN_O, 11 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_E },
	{     SECTOR_ID_E },
	{     REMAIN_E, 11 },
	{     MFM, 0x00, 16 },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   CRC, 2 },
	{   CRC_AMIGA_START, 2 },
	{     SECTOR_DATA_O, -1 },
	{     SECTOR_DATA_E, -1 },
	{   CRC_END, 2 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x00, 532 },
	{ END }
};

void floppy_image_format_t::generate_bitstream_from_track(int track, int head, int cell_size, UINT8 *trackbuf, int &track_size, floppy_image *image, int subtrack)
{
	std::vector<UINT32> &tbuf = image->get_buffer(track, head, subtrack);
	if(tbuf.size() <= 1) {
		// Unformatted track
		track_size = 200000000/cell_size;
		memset(trackbuf, 0, (track_size+7)/8);
		return;
	}

	// Start at the write splice
	UINT32 splice = image->get_write_splice_position(track, head, subtrack);
	int cur_pos = splice;
	int cur_entry = 0;
	while(cur_entry < int(tbuf.size())-1 && (tbuf[cur_entry+1] & floppy_image::TIME_MASK) < cur_pos)
		cur_entry++;

	int cur_bit = 0;

	int period = cell_size;
	int period_adjust_base = period * 0.05;

	int min_period = int(cell_size*0.75);
	int max_period = int(cell_size*1.25);
	int phase_adjust = 0;
	int freq_hist = 0;

	UINT32 scanned = 0;
	while(scanned < 200000000) {
		// Note that all magnetic cell type changes are considered
		// edges.  No randomness added for neutral/damaged cells
		int edge = tbuf[cur_entry] & floppy_image::TIME_MASK;
		if(edge < cur_pos)
			edge += 200000000;
		int next = cur_pos + period + phase_adjust;
		scanned += period + phase_adjust;

		if(edge >= next) {
			// No transition in the window means 0 and pll in free run mode
			trackbuf[cur_bit >> 3] &= ~(0x80 >> (cur_bit & 7));
			cur_bit++;
			phase_adjust = 0;

		} else {
			// Transition in the window means 1, and the pll is adjusted
			trackbuf[cur_bit >> 3] |= 0x80 >> (cur_bit & 7);
			cur_bit++;

			int delta = edge - (next - period/2);

			phase_adjust = 0.65*delta;

			if(delta < 0) {
				if(freq_hist < 0)
					freq_hist--;
				else
					freq_hist = -1;
			} else if(delta > 0) {
				if(freq_hist > 0)
					freq_hist++;
				else
					freq_hist = 1;
			} else
				freq_hist = 0;

			if(freq_hist) {
				int afh = freq_hist < 0 ? -freq_hist : freq_hist;
				if(afh > 1) {
					int aper = period_adjust_base*delta/period;
					if(!aper)
						aper = freq_hist < 0 ? -1 : 1;
					period += aper;

					if(period < min_period)
						period = min_period;
					else if(period > max_period)
						period = max_period;
				}
			}
		}

		cur_pos = next;
		if(cur_pos >= 200000000) {
			cur_pos -= 200000000;
			cur_entry = 0;
		}
		while(cur_entry < int(tbuf.size())-1 && (tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos)
			cur_entry++;

		// Wrap around
		if(cur_entry == int(tbuf.size())-1 &&
			(tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos) {
			// Wrap to index 0 or 1 depending on whether there is a transition exactly at the index hole
			cur_entry = (tbuf[int(tbuf.size())-1] & floppy_image::MG_MASK) != (tbuf[0] & floppy_image::MG_MASK) ?
				0 : 1;
		}
	}
	// Clean the leftover bottom bits just in case
	trackbuf[cur_bit >> 3] &= ~(0x7f >> (cur_bit & 7));
	track_size = cur_bit;
}

int floppy_image_format_t::sbit_r(const UINT8 *bitstream, int pos)
{
	return (bitstream[pos >> 3] & (0x80 >> (pos & 7))) != 0;
}

int floppy_image_format_t::sbit_rp(const UINT8 *bitstream, int &pos, int track_size)
{
	int res = sbit_r(bitstream, pos);
	pos ++;
	if(pos == track_size)
		pos = 0;
	return res;
}

UINT8 floppy_image_format_t::sbyte_mfm_r(const UINT8 *bitstream, int &pos, int track_size)
{
	UINT8 res = 0;
	for(int i=0; i<8; i++) {
		sbit_rp(bitstream, pos, track_size);
		if(sbit_rp(bitstream, pos, track_size))
			res |= 0x80 >> i;
	}
	return res;
}

UINT8 floppy_image_format_t::sbyte_gcr5_r(const UINT8 *bitstream, int &pos, int track_size)
{
	UINT16 gcr = 0;
	for(int i=0; i<10; i++) {
		if(sbit_rp(bitstream, pos, track_size))
			gcr |= 0x200 >> i;
	}

	return (gcr5bw_tb[gcr >> 5] << 4) | gcr5bw_tb[gcr & 0x1f];
}

void floppy_image_format_t::extract_sectors_from_bitstream_mfm_pc(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size)
{
	memset(sectors, 0, 256*sizeof(desc_xs));

	// Don't bother if it's just too small
	if(track_size < 100)
		return;

	// Start by detecting all id and data blocks

	// If 100 is not enough, that track is too funky to be worth
	// bothering anyway

	int idblk[100], dblk[100];
	int idblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	UINT16 shift_reg = 0;
	for(int i=0; i<16; i++)
		if(sbit_r(bitstream, track_size-16+i))
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for
	// blocks
	for(int i=0; i<track_size; i++) {
		shift_reg = (shift_reg << 1) | sbit_r(bitstream, i);
		if(shift_reg == 0x4489) {
			UINT16 header;
			int pos = i+1;
			do {
				header = 0;
				for(int j=0; j<16; j++)
					if(sbit_rp(bitstream, pos, track_size))
						header |= 0x8000 >> j;
				// Accept strings of sync marks as long and they're not wrapping

				// Wrapping ones have already been take into account
				// thanks to the precharging
			} while(header == 0x4489 && pos > i);

			// fe, ff
			if(header == 0x5554 || header == 0x5555) {
				if(idblk_count < 100)
					idblk[idblk_count++] = pos;
				i = pos-1;
			}
			// f8, f9, fa, fb
			if(header == 0x554a || header == 0x5549 || header == 0x5544 || header == 0x5545) {
				if(dblk_count < 100)
					dblk[dblk_count++] = pos;
				i = pos-1;
			}
		}
	}

	// Then extract the sectors
	int sectdata_pos = 0;
	for(int i=0; i<idblk_count; i++) {
		int pos = idblk[i];
		UINT8 track = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 head = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 sector = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 size = sbyte_mfm_r(bitstream, pos, track_size);

		if(size >= 8)
			continue;
		int ssize = 128 << size;

		// If we don't have enough space for a sector's data, skip it
		if(ssize + sectdata_pos > sectdata_size)
			continue;

		// Start of IDAM and DAM are supposed to be exactly 704 cells
		// apart in normal format or 1008 cells apart in perpendicular
		// format.  Of course the hardware is tolerant.  Accept +/-
		// 128 cells of shift.

		int d_index;
		for(d_index = 0; d_index < dblk_count; d_index++) {
			int delta = dblk[d_index] - idblk[i];
			if(delta >= 704-128 && delta <= 1008+128)
				break;
		}
		if(d_index == dblk_count)
			continue;

		pos = dblk[d_index];

		sectors[sector].track = track;
		sectors[sector].head = head;
		sectors[sector].size = ssize;
		sectors[sector].data = sectdata + sectdata_pos;
		for(int j=0; j<ssize; j++)
			sectdata[sectdata_pos++] = sbyte_mfm_r(bitstream, pos, track_size);
	}
}

void floppy_image_format_t::get_geometry_mfm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count)
{
	image->get_actual_geometry(track_count, head_count);

	if(!track_count) {
		sector_count = 0;
		return;
	}

	UINT8 bitstream[500000/8];
	UINT8 sectdata[50000];
	desc_xs sectors[256];
	int track_size;

	// Extract an arbitrary track to get an idea of the number of
	// sectors

	// 20 was rarely used for protections, not near the start like
	// 0-10, not near the end like 70+, no special effects on sync
	// like 33

	generate_bitstream_from_track(track_count > 20 ? 20 : 0, 0, cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));

	for(sector_count = 44; sector_count > 0 && !sectors[sector_count].data; sector_count--);
}


void floppy_image_format_t::get_track_data_mfm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, UINT8 *sectdata)
{
	UINT8 bitstream[500000/8];
	UINT8 sectbuf[50000];
	desc_xs sectors[256];
	int track_size;

	generate_bitstream_from_track(track, head, cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_mfm_pc(bitstream, track_size, sectors, sectbuf, sizeof(sectbuf));
	for(int sector=1; sector <= sector_count; sector++) {
		UINT8 *sd = sectdata + (sector-1)*sector_size;
		if(sectors[sector].data && sectors[sector].track == track && sectors[sector].head == head) {
			int asize = sectors[sector].size;
			if(asize > sector_size)
				asize = sector_size;
			memcpy(sd, sectors[sector].data, asize);
			if(asize < sector_size)
				memset(sd+asize, 0, sector_size-asize);
		} else
			memset(sd, 0, sector_size);
	}
}


void floppy_image_format_t::extract_sectors_from_bitstream_fm_pc(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size)
{
	memset(sectors, 0, 256*sizeof(desc_xs));

	// Don't bother if it's just too small
	if(track_size < 100)
		return;

	// Start by detecting all id and data blocks

	// If 100 is not enough, that track is too funky to be worth
	// bothering anyway

	int idblk[100], dblk[100];
	int idblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	UINT16 shift_reg = 0;
	for(int i=0; i<16; i++)
		if(sbit_r(bitstream, track_size-16+i))
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for
	// blocks
	// We scan for address marks only, as index marks are not mandatory,
	// and many formats actually do not use them

	for(int i=0; i<track_size; i++) {
		shift_reg = (shift_reg << 1) | sbit_r(bitstream, i);

		// fe
		if(shift_reg == 0xf57e) {       // address mark
			if(idblk_count < 100)
				idblk[idblk_count++] = i+1;
		}
		// f8, f9, fa, fb
		if(shift_reg == 0xf56a || shift_reg == 0xf56b ||
			shift_reg == 0xf56e || shift_reg == 0xf56f) {       // data mark
			if(dblk_count < 100)
				dblk[dblk_count++] = i+1;
		}
	}

	// Then extract the sectors
	int sectdata_pos = 0;
	for(int i=0; i<idblk_count; i++) {
		int pos = idblk[i];
		UINT8 track = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 head = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 sector = sbyte_mfm_r(bitstream, pos, track_size);
		UINT8 size = sbyte_mfm_r(bitstream, pos, track_size);
		if(size >= 8)
			continue;
		int ssize = 128 << size;

		// If we don't have enough space for a sector's data, skip it
		if(ssize + sectdata_pos > sectdata_size)
			continue;

		// Start of IDAM and DAM are supposed to be exactly 384 cells
		// apart.  Of course the hardware is tolerant.  Accept +/- 128
		// cells of shift.

		int d_index;
		for(d_index = 0; d_index < dblk_count; d_index++) {
			int delta = dblk[d_index] - idblk[i];
			if(delta >= 384-128 && delta <= 384+128)
				break;
		}
		if(d_index == dblk_count)
			continue;

		pos = dblk[d_index];

		sectors[sector].track = track;
		sectors[sector].head = head;
		sectors[sector].size = ssize;
		sectors[sector].data = sectdata + sectdata_pos;
		for(int j=0; j<ssize; j++)
			sectdata[sectdata_pos++] = sbyte_mfm_r(bitstream, pos, track_size);
	}
}

void floppy_image_format_t::get_geometry_fm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count)
{
	image->get_actual_geometry(track_count, head_count);

	if(!track_count) {
		sector_count = 0;
		return;
	}

	UINT8 bitstream[500000/8];
	UINT8 sectdata[50000];
	desc_xs sectors[256];
	int track_size;

	// Extract an arbitrary track to get an idea of the number of
	// sectors

	// 20 was rarely used for protections, not near the start like
	// 0-10, not near the end like 70+, no special effects on sync
	// like 33

	generate_bitstream_from_track(track_count > 20 ? 20 : 0, 0, cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_fm_pc(bitstream, track_size, sectors, sectdata, sizeof(sectdata));

	for(sector_count = 44; sector_count > 0 && !sectors[sector_count].data; sector_count--);
}


void floppy_image_format_t::get_track_data_fm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, UINT8 *sectdata)
{
	UINT8 bitstream[500000/8];
	UINT8 sectbuf[50000];
	desc_xs sectors[256];
	int track_size;

	generate_bitstream_from_track(track, head, cell_size, bitstream, track_size, image);
	extract_sectors_from_bitstream_fm_pc(bitstream, track_size, sectors, sectbuf, sizeof(sectbuf));
	for(int sector=1; sector <= sector_count; sector++) {
		UINT8 *sd = sectdata + (sector-1)*sector_size;
		if(sectors[sector].data && sectors[sector].track == track && sectors[sector].head == head) {
			int asize = sectors[sector].size;
			if(asize > sector_size)
				asize = sector_size;
			memcpy(sd, sectors[sector].data, asize);
			if(asize < sector_size)
				memset(sd+asize, 0, sector_size-asize);
		} else
			memset(sd, 0, sector_size);
	}
}

int floppy_image_format_t::calc_default_pc_gap3_size(UINT32 form_factor, int sector_size)
{
	return
		form_factor == floppy_image::FF_8 ? 25 :
		sector_size < 512 ?
		(form_factor == floppy_image::FF_35 ? 54 : 50) :
		(form_factor == floppy_image::FF_35 ? 84 : 80);
}

void floppy_image_format_t::build_wd_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2)
{
	build_pc_track_fm(track, head, image, cell_count, sector_count, sects, gap_3, -1, gap_1, gap_2);
}

void floppy_image_format_t::build_wd_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2)
{
	build_pc_track_mfm(track, head, image, cell_count, sector_count, sects, gap_3, -1, gap_1, gap_2);
}

void floppy_image_format_t::build_pc_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a, int gap_1, int gap_2)
{
	std::vector<UINT32> track_data;

	// gap 4a , IAM and gap 1
	if(gap_4a != -1) {
		for(int i=0; i<gap_4a; i++) fm_w(track_data, 8, 0xff);
		for(int i=0; i< 6;     i++) fm_w(track_data, 8, 0x00);
		raw_w(track_data, 16, 0xf77a);
	}
	for(int i=0; i<gap_1; i++) fm_w(track_data, 8, 0xff);

	int total_size = 0;
	for(int i=0; i<sector_count; i++)
		total_size += sects[i].actual_size;

	unsigned int etpos = track_data.size() + (sector_count*(6+5+2+gap_2+6+1+2) + total_size)*16;

	if(etpos > cell_count)
		throw emu_fatalerror("Incorrect layout on track %d head %d, expected_size=%d, current_size=%d", track, head, cell_count, etpos);

	if(etpos + gap_3*16*(sector_count-1) > cell_count)
		gap_3 = (cell_count - etpos) / 16 / (sector_count-1);

	// Build the track
	for(int i=0; i<sector_count; i++) {
		UINT16 crc;
		// sync and IDAM and gap 2
		for(int j=0; j< 6; j++) fm_w(track_data, 8, 0x00);

		unsigned int cpos = track_data.size();
		raw_w(track_data, 16, 0xf57e);
		fm_w (track_data, 8, sects[i].track);
		fm_w (track_data, 8, sects[i].head);
		fm_w (track_data, 8, sects[i].sector);
		fm_w (track_data, 8, sects[i].size);
		crc = calc_crc_ccitt(track_data, cpos, track_data.size());
		fm_w (track_data, 16, crc);
		for(int j=0; j<gap_2; j++) fm_w(track_data, 8, 0xff);

		if(!sects[i].data)
			for(int j=0; j<6+1+sects[i].actual_size+2+(i != sector_count-1 ? gap_3 : 0); j++) fm_w(track_data, 8, 0xff);

		else {
			// sync, DAM, data and gap 3
			for(int j=0; j< 6; j++) fm_w(track_data, 8, 0x00);
			cpos = track_data.size();
			raw_w(track_data, 16, sects[i].deleted ? 0xf56a : 0xf56f);
			for(int j=0; j<sects[i].actual_size; j++) fm_w(track_data, 8, sects[i].data[j]);
			crc = calc_crc_ccitt(track_data, cpos, track_data.size());
			if(sects[i].bad_crc)
				crc = 0xffff^crc;
			fm_w(track_data, 16, crc);
			if(i != sector_count-1)
				for(int j=0; j<gap_3; j++) fm_w(track_data, 8, 0xff);
		}
	}

	// Gap 4b

	while(int(track_data.size()) < cell_count-15) fm_w(track_data, 8, 0xff);
	raw_w(track_data, cell_count-int(track_data.size()), 0xffff >> (16+int(track_data.size())-cell_count));

	generate_track_from_levels(track, head, track_data, 0, image);
}

void floppy_image_format_t::build_pc_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a, int gap_1, int gap_2)
{
	std::vector<UINT32> track_data;

	// gap 4a , IAM and gap 1
	if(gap_4a != -1) {
		for(int i=0; i<gap_4a; i++) mfm_w(track_data, 8, 0x4e);
		for(int i=0; i<12;     i++) mfm_w(track_data, 8, 0x00);
		for(int i=0; i< 3;     i++) raw_w(track_data, 16, 0x5224);
		mfm_w(track_data, 8, 0xfc);
	}
	for(int i=0; i<gap_1; i++) mfm_w(track_data, 8, 0x4e);

	int total_size = 0;
	for(int i=0; i<sector_count; i++)
		total_size += sects[i].actual_size;

	int etpos = int(track_data.size()) + (sector_count*(12+3+5+2+gap_2+12+3+1+2) + total_size)*16;

	if(etpos > cell_count)
		throw emu_fatalerror("Incorrect layout on track %d head %d, expected_size=%d, current_size=%d", track, head, cell_count, etpos);

	if(etpos + gap_3*16*(sector_count-1) > cell_count)
		gap_3 = (cell_count - etpos) / 16 / (sector_count-1);

	// Build the track
	for(int i=0; i<sector_count; i++) {
		UINT16 crc;
		// sync and IDAM and gap 2
		for(int j=0; j<12; j++) mfm_w(track_data, 8, 0x00);
		unsigned int cpos = track_data.size();
		for(int j=0; j< 3; j++) raw_w(track_data, 16, 0x4489);
		mfm_w(track_data, 8, 0xfe);
		mfm_w(track_data, 8, sects[i].track);
		mfm_w(track_data, 8, sects[i].head);
		mfm_w(track_data, 8, sects[i].sector);
		mfm_w(track_data, 8, sects[i].size);
		crc = calc_crc_ccitt(track_data, cpos, track_data.size());
		mfm_w(track_data, 16, crc);
		for(int j=0; j<gap_2; j++) mfm_w(track_data, 8, 0x4e);

		if(!sects[i].data)
			for(int j=0; j<12+4+sects[i].actual_size+2+(i != sector_count-1 ? gap_3 : 0); j++) mfm_w(track_data, 8, 0x4e);

		else {
			// sync, DAM, data and gap 3
			for(int j=0; j<12; j++) mfm_w(track_data, 8, 0x00);
			cpos = track_data.size();
			for(int j=0; j< 3; j++) raw_w(track_data, 16, 0x4489);
			mfm_w(track_data, 8, sects[i].deleted ? 0xf8 : 0xfb);
			for(int j=0; j<sects[i].actual_size; j++) mfm_w(track_data, 8, sects[i].data[j]);
			crc = calc_crc_ccitt(track_data, cpos, track_data.size());
			if(sects[i].bad_crc)
				crc = 0xffff^crc;
			mfm_w(track_data, 16, crc);
			if(i != sector_count-1)
				for(int j=0; j<gap_3; j++) mfm_w(track_data, 8, 0x4e);
		}
	}

	// Gap 4b

	while(int(track_data.size()) < cell_count-15) mfm_w(track_data, 8, 0x4e);
	raw_w(track_data, cell_count-int(track_data.size()), 0x9254 >> (16+int(track_data.size())-cell_count));

	generate_track_from_levels(track, head, track_data, 0, image);
}

void floppy_image_format_t::extract_sectors_from_bitstream_gcr5(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size, int head, int tracks)
{
	memset(sectors, 0, 256*sizeof(desc_xs));

	// Don't bother if it's just too small
	if(track_size < 100)
		return;

	// Start by detecting all id and data blocks
	int hblk[100], dblk[100];
	int hblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	UINT16 shift_reg = 0;
	for(int i=0; i<16; i++)
		if(sbit_r(bitstream, track_size-16+i))
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for blocks
	bool sync = false;
	for(int i=0; i<track_size; i++) {
		int bit = sbit_r(bitstream, i);
		shift_reg = ((shift_reg << 1) | bit) & 0x3ff;

		if (sync && !bit) {
			UINT8 id = sbyte_gcr5_r(bitstream, i, track_size);

			switch (id) {
			case 0x08:
				if(hblk_count < 100)
					hblk[hblk_count++] = i-10;
				break;

			case 0x07:
				if(dblk_count < 100)
					dblk[dblk_count++] = i-10;
				break;
			}
		}

		sync = (shift_reg == 0x3ff);
	}

	// Then extract the sectors
	int sectdata_pos = 0;
	for(int i=0; i<hblk_count; i++) {
		int pos = hblk[i];
		ATTR_UNUSED UINT8 block_id = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 crc = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 sector = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 track = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 id2 = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 id1 = sbyte_gcr5_r(bitstream, pos, track_size);

		if (crc ^ sector ^ track ^ id2 ^ id1) {
			// header crc mismatch
		}

		pos = dblk[i];
		block_id = sbyte_gcr5_r(bitstream, pos, track_size);

		if (track > tracks) track -= tracks;
		sectors[sector].track = track;
		sectors[sector].head = head;
		sectors[sector].size = 256;
		sectors[sector].data = sectdata + sectdata_pos;
		UINT8 data_crc = 0;
		for(int j=0; j<sectors[sector].size; j++) {
			UINT8 data = sbyte_gcr5_r(bitstream, pos, track_size);
			data_crc ^= data;
			sectdata[sectdata_pos++] = data;
		}
		data_crc ^= sbyte_gcr5_r(bitstream, pos, track_size);
		if (data_crc) {
			// data crc mismatch
		}
	}
}

void floppy_image_format_t::extract_sectors_from_bitstream_victor_gcr5(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size)
{
	memset(sectors, 0, 256*sizeof(desc_xs));

	// Don't bother if it's just too small
	if(track_size < 100)
		return;

	// Start by detecting all id and data blocks
	int hblk[100], dblk[100];
	int hblk_count = 0, dblk_count = 0;

	// Precharge the shift register to detect over-the-index stuff
	UINT16 shift_reg = 0;
	for(int i=0; i<16; i++)
		if(sbit_r(bitstream, track_size-16+i))
			shift_reg |= 0x8000 >> i;

	// Scan the bitstream for sync marks and follow them to check for blocks
	bool sync = false;
	for(int i=0; i<track_size; i++) {
		int bit = sbit_r(bitstream, i);
		shift_reg = ((shift_reg << 1) | bit) & 0x3ff;

		if (sync && !bit) {
			UINT8 id = sbyte_gcr5_r(bitstream, i, track_size);

			switch (id) {
			case 0x07:
				if(hblk_count < 100)
					hblk[hblk_count++] = i-10;
				break;

			case 0x08:
				if(dblk_count < 100)
					dblk[dblk_count++] = i-10;
				break;
			}
		}

		sync = (shift_reg == 0x3ff);
	}

	// Then extract the sectors
	int sectdata_pos = 0;
	for(int i=0; i<hblk_count; i++) {
		int pos = hblk[i];
		ATTR_UNUSED UINT8 block_id = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 track = sbyte_gcr5_r(bitstream, pos, track_size);
		UINT8 sector = sbyte_gcr5_r(bitstream, pos, track_size);

		pos = dblk[i];
		block_id = sbyte_gcr5_r(bitstream, pos, track_size);

		sectors[sector].track = track & 0x7f;
		sectors[sector].head = BIT(track, 7);
		sectors[sector].size = 512;
		sectors[sector].data = sectdata + sectdata_pos;
		for(int j=0; j<sectors[sector].size; j++) {
			UINT8 data = sbyte_gcr5_r(bitstream, pos, track_size);
			sectdata[sectdata_pos++] = data;
		}
	}
}
