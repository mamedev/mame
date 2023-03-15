// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    iflopimg.c

    Bridge code for Imgtool into the standard floppy code

*********************************************************************/

#include "iflopimg.h"

#include "imgtool.h"
#include "library.h"

#include "ioprocs.h"

#include <cstdio>


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
    Imgtool handlers
*********************************************************************/

struct imgtool_floppy_image
{
	floppy_image_legacy *floppy;
};




static imgtoolerr_t imgtool_floppy_open_internal(imgtool::image &image, imgtool::stream::ptr &&stream)
{
	floperr_t ferr;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	struct imgtool_floppy_image *fimg;
	const imgtool_class *imgclass;
	const struct FloppyFormat *format;
	imgtoolerr_t (*open)(imgtool::image &image, imgtool::stream *f);

	fimg = (struct imgtool_floppy_image *) image.extra_bytes();
	imgclass = &image.module().imgclass;
	format = (const struct FloppyFormat *) imgclass->derived_param;
	open = (imgtoolerr_t (*)(imgtool::image &, imgtool::stream *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_OPEN);

	// open up the floppy
	ferr = floppy_open(imgtool::stream_read_write(std::move(stream), 0xff), "", format, FLOPPY_FLAGS_READWRITE, &fimg->floppy);
	if (ferr)
	{
		err = imgtool_floppy_error(ferr);
		goto done;
	}

	if (open)
	{
		err = open(image, nullptr);
		if (err)
			goto done;
	}

done:
	if (err && fimg->floppy)
	{
		floppy_close(fimg->floppy);
		fimg->floppy = nullptr;
	}
	return err;
}



static imgtoolerr_t imgtool_floppy_open(imgtool::image &image, imgtool::stream::ptr &&stream)
{
	return imgtool_floppy_open_internal(image, std::move(stream));
}



static imgtoolerr_t imgtool_floppy_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
{
	floperr_t ferr;
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	struct imgtool_floppy_image *fimg;
	const imgtool_class *imgclass;
	const struct FloppyFormat *format;
	imgtoolerr_t (*create)(imgtool::image &, imgtool::stream *, util::option_resolution *);
	imgtoolerr_t (*open)(imgtool::image &, imgtool::stream *f);

	fimg = (struct imgtool_floppy_image *) image.extra_bytes();
	imgclass = &image.module().imgclass;
	format = (const struct FloppyFormat *) imgclass->derived_param;
	create = (imgtoolerr_t (*)(imgtool::image &, imgtool::stream *, util::option_resolution *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_CREATE);
	open = (imgtoolerr_t (*)(imgtool::image &, imgtool::stream *)) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_FLOPPY_OPEN);

	// open up the floppy
	ferr = floppy_create(imgtool::stream_read_write(std::move(stream), 0xff), format, opts, &fimg->floppy);
	if (ferr)
	{
		err = imgtool_floppy_error(ferr);
		goto done;
	}

	// do we have to do extra stuff when creating the image?
	if (create)
	{
		err = create(image, nullptr, opts);
		if (err)
			goto done;
	}

	// do we have to do extra stuff when opening the image?
	if (open)
	{
		err = open(image, nullptr);
		if (err)
			goto done;
	}

done:
	if (err && fimg->floppy)
	{
		floppy_close(fimg->floppy);
		fimg->floppy = nullptr;
	}
	return err;
}



static void imgtool_floppy_close(imgtool::image &img)
{
	floppy_close(imgtool_floppy(img));
}



static imgtoolerr_t imgtool_floppy_read_sector(imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, std::vector<uint8_t> &buffer)
{
	floperr_t ferr;
	uint32_t sector_size;

	// get the sector length
	ferr = floppy_get_sector_length(imgtool_floppy(image), head, track, sector, &sector_size);
	if (ferr)
		return imgtool_floppy_error(ferr);

	// resize the buffer accordingly
	try { buffer.resize(sector_size); }
	catch (std::bad_alloc const &) { return IMGTOOLERR_OUTOFMEMORY; }

	// and read the sector
	ferr = floppy_read_sector(imgtool_floppy(image), head, track, sector, 0, &buffer[0], sector_size);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static imgtoolerr_t imgtool_floppy_write_sector(imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, const void *buffer, size_t len, int ddam)
{
	floperr_t ferr;

	ferr = floppy_write_sector(imgtool_floppy(image), head, track, sector, 0, buffer, len, ddam);
	if (ferr)
		return imgtool_floppy_error(ferr);

	return IMGTOOLERR_SUCCESS;
}



static void imgtool_floppy_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
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
		case IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE:  info->createimage_optguide = format->param_guidelines ? &floppy_option_guide() : nullptr; break;
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
		return false;

	imgclass->derived_get_info = imgclass->get_info;
	imgclass->get_info = imgtool_floppy_get_info;
	imgclass->derived_param = (void *) &format[index];
	return true;
}



floppy_image_legacy *imgtool_floppy(imgtool::image &img)
{
	struct imgtool_floppy_image *fimg;
	fimg = (struct imgtool_floppy_image *) img.extra_bytes();
	return fimg->floppy;
}



static imgtoolerr_t imgtool_floppy_transfer_sector_tofrom_stream(imgtool::image &img, int head, int track, int sector, int offset, size_t length, imgtool::stream &f, int direction)
{
	floperr_t err;
	floppy_image_legacy *floppy;
	std::vector<uint8_t> buffer;

	floppy = imgtool_floppy(img);

	buffer.resize(length);

	if (direction)
	{
		err = floppy_read_sector(floppy, head, track, sector, offset, &buffer[0], length);
		if (err)
			goto done;
		f.write(&buffer[0], length);
	}
	else
	{
		f.read(&buffer[0], length);
		err = floppy_write_sector(floppy, head, track, sector, offset, &buffer[0], length, 0);  /* TODO: pass ddam argument from imgtool */
		if (err)
			goto done;
	}

	err = FLOPPY_ERROR_SUCCESS;

done:
	return imgtool_floppy_error(err);
}



imgtoolerr_t imgtool_floppy_read_sector_to_stream(imgtool::image &img, int head, int track, int sector, int offset, size_t length, imgtool::stream &f)
{
	return imgtool_floppy_transfer_sector_tofrom_stream(img, head, track, sector, offset, length, f, 1);
}



imgtoolerr_t imgtool_floppy_write_sector_from_stream(imgtool::image &img, int head, int track, int sector, int offset, size_t length, imgtool::stream &f)
{
	return imgtool_floppy_transfer_sector_tofrom_stream(img, head, track, sector, offset, length, f, 0);
}



void *imgtool_floppy_extrabytes(imgtool::image &img)
{
	struct imgtool_floppy_image *fimg;
	fimg = (struct imgtool_floppy_image *) img.extra_bytes();
	return fimg + 1;
}
