// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg_legacy.cpp

    Floppy disk image abstraction code (legacy implementation)

*********************************************************************/

#include "flopimg_legacy.h"
#include "imageutl.h"

#include "ioprocs.h"
#include "opresolv.h"

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define TRACK_LOADED        0x01
#define TRACK_DIRTY         0x02


struct floppy_image_legacy
{
	util::random_read_write::ptr io = nullptr;

	const struct FloppyFormat *floppy_option = nullptr;
	struct FloppyCallbacks format = { 0 };

	/* loaded track stuff */
	int loaded_track_head = 0;
	int loaded_track_index = 0;
	uint32_t loaded_track_size = 0;
	std::unique_ptr<uint8_t[]> loaded_track_data;
	uint8_t loaded_track_status = 0;
	uint8_t flags = 0;

	/* tagging system */
	std::unique_ptr<uint8_t[]> tag_data;
};



struct floppy_params
{
	int param;
	int value;
};



static floperr_t floppy_track_unload(floppy_image_legacy *floppy);

OPTION_GUIDE_START(floplgcy_option_guide)
	OPTION_INT('H', "heads",            "Heads")
	OPTION_INT('T', "tracks",           "Tracks")
	OPTION_INT('S', "sectors",          "Sectors")
	OPTION_INT('L', "sectorlength",     "Sector Bytes")
	OPTION_INT('I', "interleave",       "Interleave")
	OPTION_INT('F', "firstsectorid",    "First Sector")
OPTION_GUIDE_END

const util::option_guide &floppy_option_guide()
{
	return floplgcy_option_guide;
}


static void floppy_close_internal(floppy_image_legacy *floppy);

/*********************************************************************
    opening, closing and creating of floppy images
*********************************************************************/

/* basic floppy_image_legacy initialization common to floppy_open() and floppy_create() */
static floppy_image_legacy *floppy_init(util::random_read_write::ptr &&io, int flags)
{
	floppy_image_legacy *floppy;

	floppy = new floppy_image_legacy;

	floppy->io = std::move(io);
	floppy->flags = (uint8_t) flags;
	return floppy;
}



/* main code for identifying and maybe opening a disk image; not exposed
 * directly because this function is big and hideous */
static floperr_t floppy_open_internal(util::random_read_write::ptr &&io, const std::string &extension,
	const struct FloppyFormat *floppy_options, int max_options, int flags, floppy_image_legacy **outfloppy,
	int *outoption)
{
	floperr_t err;
	floppy_image_legacy *floppy;
	int best_option = -1;
	int best_vote = 0;
	int vote;
	size_t i;

	floppy = floppy_init(std::move(io), flags);
	if (!floppy)
	{
		err = FLOPPY_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* vote on the best format */
	for (i = 0; (i < max_options) && floppy_options[i].construct; i++)
	{
		if (extension.empty() || !floppy_options[i].extensions || image_find_extension(floppy_options[i].extensions, extension.c_str()))
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
		floppy_close_internal(floppy);
		floppy = nullptr;
	}

	if (outoption)
		*outoption = err ? -1 : best_option;
	if (outfloppy)
		*outfloppy = floppy;
	return err;
}



floperr_t floppy_identify(util::random_read_write::ptr &&io, const char *extension,
	const struct FloppyFormat *formats, int *identified_format)
{
	return floppy_open_internal(std::move(io), extension, formats, INT_MAX, FLOPPY_FLAGS_READONLY, nullptr, identified_format);
}



floperr_t floppy_open(util::random_read_write::ptr &&io, const std::string &extension,
	const struct FloppyFormat *format, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(std::move(io), extension, format, 1, flags, outfloppy, nullptr);
}



floperr_t floppy_open_choices(util::random_read_write::ptr &&io, const std::string &extension,
	const struct FloppyFormat *formats, int flags, floppy_image_legacy **outfloppy)
{
	return floppy_open_internal(std::move(io), extension, formats, INT_MAX, flags, outfloppy, nullptr);
}



floperr_t floppy_create(util::random_read_write::ptr &&io, const struct FloppyFormat *format, util::option_resolution *parameters, floppy_image_legacy **outfloppy)
{
	floppy_image_legacy *floppy = nullptr;
	floperr_t err;
	int heads, tracks, h, t;
	std::unique_ptr<util::option_resolution> alloc_resolution;

	assert(format);

	/* create the new image */
	floppy = floppy_init(std::move(io), 0);
	if (!floppy)
	{
		err = FLOPPY_ERROR_OUTOFMEMORY;
		goto done;
	}

	/* if this format expects creation parameters and none were specified, create some */
	if (!parameters && format->param_guidelines)
	{
		try { alloc_resolution = std::make_unique<util::option_resolution>(floplgcy_option_guide); }
		catch (...)
		{
			err = FLOPPY_ERROR_OUTOFMEMORY;
			goto done;
		}
		alloc_resolution->set_specification(format->param_guidelines);
		parameters = alloc_resolution.get();
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
		floppy_close_internal(floppy);
		floppy = nullptr;
	}

	if (outfloppy)
		*outfloppy = floppy;
	else if (floppy)
		floppy_close_internal(floppy);
	return err;
}



static void floppy_close_internal(floppy_image_legacy *floppy)
{
	if (floppy) {
		floppy_track_unload(floppy);

		if(floppy->floppy_option && floppy->floppy_option->destruct)
			floppy->floppy_option->destruct(floppy, floppy->floppy_option);

		delete floppy;
	}
}



void floppy_close(floppy_image_legacy *floppy)
{
	floppy_close_internal(floppy);
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
	return floppy->tag_data.get();
}



void *floppy_create_tag(floppy_image_legacy *floppy, size_t tagsize)
{
	floppy->tag_data = std::make_unique<uint8_t[]>(tagsize);
	return floppy->tag_data.get();
}



uint8_t floppy_get_filler(floppy_image_legacy *floppy)
{
	// FIXME: remove this function, it's here for legacy reasons
	// the caller actually sets the filler byte - in practice it's always 0xff but there's no actual guarantee
	return 0xff;
}



util::random_read_write &floppy_get_io(floppy_image_legacy *floppy)
{
	return *floppy->io;
}



/*********************************************************************
    calls for accessing the raw disk image
*********************************************************************/

void floppy_image_read(floppy_image_legacy *floppy, void *buffer, uint64_t offset, size_t length)
{
	size_t actual;
	floppy->io->read_at(offset, buffer, length, actual);
}



void floppy_image_write(floppy_image_legacy *floppy, const void *buffer, uint64_t offset, size_t length)
{
	size_t actual;
	floppy->io->write_at(offset, buffer, length, actual);
}



void floppy_image_write_filler(floppy_image_legacy *floppy, uint8_t filler, uint64_t offset, size_t length)
{
	uint8_t buffer[512];
	memset(buffer, filler, std::min(sizeof(buffer), length));

	while (length)
	{
		size_t const block = std::min(sizeof(buffer), length);
		size_t actual;
		floppy->io->write_at(offset, buffer, block, actual);
		offset += block;
		length -= block;
	}
}



uint64_t floppy_image_size(floppy_image_legacy *floppy)
{
	uint64_t result;
	floppy->io->length(result);
	return result;
}



/*********************************************************************
    calls for accessing disk image data
*********************************************************************/

static floperr_t floppy_readwrite_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset,
	void *buffer, size_t buffer_len, bool writing, bool indexed, int ddam)
{
	floperr_t err;
	const struct FloppyCallbacks *fmt;
	size_t this_buffer_len;
	std::vector<uint8_t> alloc_buf;
	uint32_t sector_length;
	uint8_t *buffer_ptr = (uint8_t *)buffer;
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

				this_buffer_len = std::min(buffer_len, size_t(sector_length - offset));

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
	return floppy_readwrite_sector(floppy, head, track, sector, offset, buffer, buffer_len, false, false, 0);
}



floperr_t floppy_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, const void *buffer, size_t buffer_len, int ddam)
{
	return floppy_readwrite_sector(floppy, head, track, sector, offset, (void *) buffer, buffer_len, true, false, ddam);
}



floperr_t floppy_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset,    void *buffer, size_t buffer_len)
{
	return floppy_readwrite_sector(floppy, head, track, sector_index, offset, buffer, buffer_len, false, true, 0);
}



floperr_t floppy_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, const void *buffer, size_t buffer_len, int ddam)
{
	return floppy_readwrite_sector(floppy, head, track, sector_index, offset, (void *) buffer, buffer_len, true, true, ddam);
}


static floperr_t floppy_get_track_data_offset(floppy_image_legacy *floppy, int head, int track, uint64_t *offset)
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



static floperr_t floppy_read_track_offset(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buffer_len)
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
	uint64_t offset;

	err = floppy_get_track_data_offset(floppy, head, track, &offset);
	if (err)
		return err;

	return floppy_read_track_offset(floppy, head, track, offset, buffer, buffer_len);
}



static floperr_t floppy_write_track_offset(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buffer_len)
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
	uint64_t offset;

	err = floppy_get_track_data_offset(floppy, head, track, &offset);
	if (err)
		return err;

	return floppy_write_track_offset(floppy, head, track, offset, buffer, buffer_len);
}



floperr_t floppy_format_track(floppy_image_legacy *floppy, int head, int track, util::option_resolution *parameters)
{
	floperr_t err;
	struct FloppyCallbacks *format;
	std::unique_ptr<util::option_resolution> alloc_resolution;

	/* supported? */
	format = floppy_callbacks(floppy);
	if (!format->format_track)
		return FLOPPY_ERROR_UNSUPPORTED;

	/* create a dummy resolution; if no parameters were specified */
	if (!parameters)
	{
		try
		{
			alloc_resolution = std::make_unique<util::option_resolution>(floplgcy_option_guide);
		}
		catch (...)
		{
			return FLOPPY_ERROR_OUTOFMEMORY;
		}
		alloc_resolution->set_specification(floppy->floppy_option->param_guidelines);

		parameters = alloc_resolution.get();
	}

	err = format->format_track(floppy, head, track, parameters);
	if (err)
		return err;

	return FLOPPY_ERROR_SUCCESS;
}



int floppy_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return floppy_callbacks(floppy)->get_tracks_per_disk(floppy);
}



int floppy_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return floppy_callbacks(floppy)->get_heads_per_disk(floppy);
}



uint32_t floppy_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	const struct FloppyCallbacks *fmt;

	fmt = floppy_callbacks(floppy);
	if (!fmt->get_track_size)
		return 0;

	return fmt->get_track_size(floppy, head, track);
}



floperr_t floppy_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
{
	const struct FloppyCallbacks *fmt;

	fmt = floppy_callbacks(floppy);
	if (!fmt->get_sector_length)
		return FLOPPY_ERROR_UNSUPPORTED;

	return fmt->get_sector_length(floppy, head, track, sector, sector_length);
}



floperr_t floppy_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
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



uint8_t floppy_random_byte(floppy_image_legacy *floppy)
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
	uint32_t track_size;

	/* have we already loaded this track? */
	if (((floppy->loaded_track_status & TRACK_LOADED) == 0) || (head != floppy->loaded_track_head) || (track != floppy->loaded_track_index))
	{
		err = floppy_track_unload(floppy);
		if (err)
			goto error;

		track_size = floppy_callbacks(floppy)->get_track_size(floppy, head, track);

		floppy->loaded_track_data = std::make_unique<uint8_t[]>(track_size);
		floppy->loaded_track_size = track_size;
		floppy->loaded_track_head = head;
		floppy->loaded_track_index = track;

		err = floppy_callbacks(floppy)->read_track(floppy, floppy->loaded_track_head, floppy->loaded_track_index, 0, floppy->loaded_track_data.get(), floppy->loaded_track_size);
		if (err)
			goto error;

		floppy->loaded_track_status |= TRACK_LOADED | (dirtify ? TRACK_DIRTY : 0);
	}
	else
		floppy->loaded_track_status |= (dirtify ? TRACK_DIRTY : 0);

	if (track_data)
		*track_data = floppy->loaded_track_data.get();
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
		err = floppy_callbacks(floppy)->write_track(floppy, floppy->loaded_track_head, floppy->loaded_track_index, 0, floppy->loaded_track_data.get(), floppy->loaded_track_size);
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

	if ((err < 0) || (err >= std::size(error_messages)))
		return nullptr;
	return error_messages[err];
}


LEGACY_FLOPPY_OPTIONS_START(default)
LEGACY_FLOPPY_OPTIONS_END
