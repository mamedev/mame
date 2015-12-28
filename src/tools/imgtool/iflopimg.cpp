// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    iflopimg.c

    Bridge code for Imgtool into the standard floppy code

*********************************************************************/

#include "imgtool.h"
#include "formats/flopimg.h"
#include "library.h"
#include "iflopimg.h"

imgtoolerr_t imgtool_floppy_error(floperr_t err)
{
	switch(err)
	{
		case FLOPPY_ERROR_SUCCESS:
			return IMGTOOLERR_SUCCESS;

		case FLOPPY_ERROR_OUTOFMEMORY:
			return IMGTOOLERR_OUTOFMEMORY;

		case FLOPPY_ERROR_INVALIDIMAGE:
			return IMGTOOLERR_CORRUPTIMAGE;

		case FLOPPY_ERROR_SEEKERROR:
			return IMGTOOLERR_SEEKERROR;

		case FLOPPY_ERROR_UNSUPPORTED:
			return IMGTOOLERR_UNIMPLEMENTED;

		default:
			return IMGTOOLERR_UNEXPECTED;
	}
}



/*********************************************************************
    Imgtool ioprocs
*********************************************************************/

static void imgtool_floppy_closeproc(void *file)
{
	stream_close((imgtool_stream *) file);
}

static int imgtool_floppy_seekproc(void *file, INT64 offset, int whence)
{
	stream_seek((imgtool_stream *) file, offset, whence);
	return 0;
}

static size_t imgtool_floppy_readproc(void *file, void *buffer, size_t length)
{
	return stream_read((imgtool_stream *) file, buffer, length);
}

static size_t imgtool_floppy_writeproc(void *file, const void *buffer, size_t length)
{
	stream_write((imgtool_stream *) file, buffer, length);
	return length;
}

static UINT64 imgtool_floppy_filesizeproc(void *file)
{
	return stream_size((imgtool_stream *) file);
}

static const struct io_procs imgtool_ioprocs =
{
	imgtool_floppy_closeproc,
	imgtool_floppy_seekproc,
	imgtool_floppy_readproc,
	imgtool_floppy_writeproc,
	imgtool_floppy_filesizeproc
};

static const struct io_procs imgtool_noclose_ioprocs =
{
	nullptr,
	imgtool_floppy_seekproc,
	imgtool_floppy_readproc,
	imgtool_floppy_writeproc,
	imgtool_floppy_filesizeproc
};



/*********************************************************************
    Imgtool handlers
*********************************************************************/

struct imgtool_floppy_image
{
	floppy_image_legacy *floppy;
};




static imgtoolerr_t imgtool_floppy_open_internal(imgtool_image *image, imgtool_stream *f, int noclose)
{
	floperr_t ferr;
	imgtoolerr_t err;
	struct imgtool_floppy_image *fimg;
	const imgtool_class *imgclass;
	const struct FloppyFormat *format;
	imgtoolerr_t (*open)(imgtool_image *image, imgtool_stream *f);

	fimg = (struct imgtool_floppy_image *) imgtool_image_extra_bytes(image);
	imgclass = &imgtool_image_module(image)->imgclass;
	format = (const struct FloppyFormat *) imgclass->derived_param;
	open = (imgtoolerr_t (*)(imgtool_image *, imgtool_stream *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_OPEN);

	/* open up the floppy */
	ferr = floppy_open(f, noclose ? &imgtool_noclose_ioprocs : &imgtool_ioprocs,
		nullptr, format, FLOPPY_FLAGS_READWRITE, &fimg->floppy);
	if (ferr)
	{
		err = imgtool_floppy_error(ferr);
		return err;
	}

	if (open)
	{
		err = open(image, nullptr);
		if (err)
			return err;
	}

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t imgtool_floppy_open(imgtool_image *image, imgtool_stream *f)
{
	return imgtool_floppy_open_internal(image, f, FALSE);
}



static imgtoolerr_t imgtool_floppy_create(imgtool_image *image, imgtool_stream *f, option_resolution *opts)
{
	floperr_t ferr;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	struct imgtool_floppy_image *fimg;
	const imgtool_class *imgclass;
	const struct FloppyFormat *format;
	imgtoolerr_t (*create)(imgtool_image *, imgtool_stream *, option_resolution *);
	imgtoolerr_t (*open)(imgtool_image *image, imgtool_stream *f);

	fimg = (struct imgtool_floppy_image *) imgtool_image_extra_bytes(image);
	imgclass = &imgtool_image_module(image)->imgclass;
	format = (const struct FloppyFormat *) imgclass->derived_param;
	create = (imgtoolerr_t (*)(imgtool_image *, imgtool_stream *, option_resolution *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_CREATE);
	open = (imgtoolerr_t (*)(imgtool_image *, imgtool_stream *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_OPEN);

	/* open up the floppy */
	ferr = floppy_create(f, &imgtool_ioprocs, format, opts, &fimg->floppy);
	if (ferr)
	{
		err = imgtool_floppy_error(ferr);
		goto done;
	}

	/* do we have to do extra stuff when creating the image? */
	if (create)
	{
		err = create(image, nullptr, opts);
		if (err)
			goto done;
	}

	/* do we have to do extra stuff when opening the image? */
	if (open)
	{
		err = open(image, nullptr);
		if (err)
			goto done;
	}

done:
	return err;
}



static void imgtool_floppy_close(imgtool_image *img)
{
	floppy_close(imgtool_floppy(img));
}



static imgtoolerr_t imgtool_floppy_get_sector_size(imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, UINT32 *sector_size)
{
	floperr_t ferr;

	ferr = floppy_get_sector_length(imgtool_floppy(image), head, track, sector, sector_size);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t imgtool_floppy_read_sector(imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, void *buffer, size_t len)
{
	floperr_t ferr;

	ferr = floppy_read_sector(imgtool_floppy(image), head, track, sector, 0, buffer, len);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t imgtool_floppy_write_sector(imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, const void *buffer, size_t len, int ddam)
{
	floperr_t ferr;

	ferr = floppy_write_sector(imgtool_floppy(image), head, track, sector, 0, buffer, len, ddam);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static void imgtool_floppy_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	const struct FloppyFormat *format;
	imgtool_class derived_class;

	format = (const struct FloppyFormat *) imgclass->derived_param;
	memset(&derived_class, 0, sizeof(derived_class));
	derived_class.get_info = imgclass->derived_get_info;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:
			info->i = sizeof(struct imgtool_floppy_image) +
				imgtool_get_info_int(&derived_class, IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES);
			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:
			sprintf(info->s = imgtool_temp_str(), "%s_%s", format->name,
				imgtool_get_info_string(&derived_class, IMGTOOLINFO_STR_NAME));
			break;
		case IMGTOOLINFO_STR_DESCRIPTION:
			sprintf(info->s = imgtool_temp_str(), "%s (%s)", format->description,
				imgtool_get_info_string(&derived_class, IMGTOOLINFO_STR_DESCRIPTION));
			break;
		case IMGTOOLINFO_STR_FILE_EXTENSIONS:       strcpy(info->s = imgtool_temp_str(), format->extensions); break;
		case IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC:   info->p = (void*)format->param_guidelines; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case IMGTOOLINFO_PTR_OPEN:                  info->open = imgtool_floppy_open; break;
		case IMGTOOLINFO_PTR_CREATE:                info->create = imgtool_floppy_create; break;
		case IMGTOOLINFO_PTR_CLOSE:                 info->close = imgtool_floppy_close; break;
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:  info->createimage_optguide = format->param_guidelines ? floppy_option_guide : nullptr; break;
		case IMGTOOLINFO_PTR_GET_SECTOR_SIZE:       info->get_sector_size = imgtool_floppy_get_sector_size; break;
		case IMGTOOLINFO_PTR_READ_SECTOR:           info->read_sector = imgtool_floppy_read_sector; break;
		case IMGTOOLINFO_PTR_WRITE_SECTOR:          info->write_sector = imgtool_floppy_write_sector; break;

		default:    imgclass->derived_get_info(imgclass, state, info); break;
	}
}



int imgtool_floppy_make_class(int index, imgtool_class *imgclass)
{
	const struct FloppyFormat *format;

	/* get the format */
	format = (const struct FloppyFormat *)
		imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_FORMAT);
	assert(format);
	if (!format[index].construct)
		return FALSE;

	imgclass->derived_get_info = imgclass->get_info;
	imgclass->get_info = imgtool_floppy_get_info;
	imgclass->derived_param = (void *) &format[index];
	return TRUE;
}



floppy_image_legacy *imgtool_floppy(imgtool_image *img)
{
	struct imgtool_floppy_image *fimg;
	fimg = (struct imgtool_floppy_image *) imgtool_image_extra_bytes(img);
	return fimg->floppy;
}



static imgtoolerr_t imgtool_floppy_transfer_sector_tofrom_stream(imgtool_image *img, int head, int track, int sector, int offset, size_t length, imgtool_stream *f, int direction)
{
	floperr_t err;
	floppy_image_legacy *floppy;
	dynamic_buffer buffer;

	floppy = imgtool_floppy(img);

	buffer.resize(length);

	if (direction)
	{
		err = floppy_read_sector(floppy, head, track, sector, offset, &buffer[0], length);
		if (err)
			goto done;
		stream_write(f, &buffer[0], length);
	}
	else
	{
		stream_read(f, &buffer[0], length);
		err = floppy_write_sector(floppy, head, track, sector, offset, &buffer[0], length, 0);  /* TODO: pass ddam argument from imgtool */
		if (err)
			goto done;
	}

	err = FLOPPY_ERROR_SUCCESS;

done:
	return imgtool_floppy_error(err);
}



imgtoolerr_t imgtool_floppy_read_sector_to_stream(imgtool_image *img, int head, int track, int sector, int offset, size_t length, imgtool_stream *f)
{
	return imgtool_floppy_transfer_sector_tofrom_stream(img, head, track, sector, offset, length, f, 1);
}



imgtoolerr_t imgtool_floppy_write_sector_from_stream(imgtool_image *img, int head, int track, int sector, int offset, size_t length, imgtool_stream *f)
{
	return imgtool_floppy_transfer_sector_tofrom_stream(img, head, track, sector, offset, length, f, 0);
}



void *imgtool_floppy_extrabytes(imgtool_image *img)
{
	struct imgtool_floppy_image *fimg;
	fimg = (struct imgtool_floppy_image *) imgtool_image_extra_bytes(img);
	return fimg + 1;
}
