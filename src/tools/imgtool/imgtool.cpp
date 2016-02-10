// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    imgtool.c

    Core code for Imgtool

***************************************************************************/

#include <string.h>
#include <ctype.h>

#include "imgtool.h"
#include "formats/imageutl.h"
#include "library.h"
#include "modules.h"
#include "pool.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct imgtool_image
{
	const imgtool_module *module;
	object_pool *pool;
};

struct imgtool_partition
{
	imgtool_image *image;
	object_pool *pool;
	int partition_index;
	UINT64 base_block;
	UINT64 block_count;

	imgtool_class imgclass;
	size_t partition_extra_bytes;
	size_t directory_extra_bytes;

	char path_separator;
	char alternate_path_separator;

	unsigned int prefer_ucase : 1;
	unsigned int supports_creation_time : 1;
	unsigned int supports_lastmodified_time : 1;
	unsigned int supports_bootblock : 1;            /* this module supports loading/storing the boot block */

	imgtoolerr_t    (*begin_enum)   (imgtool_directory *enumeration, const char *path);
	imgtoolerr_t    (*next_enum)    (imgtool_directory *enumeration, imgtool_dirent *ent);
	void            (*close_enum)   (imgtool_directory *enumeration);
	imgtoolerr_t    (*free_space)   (imgtool_partition *partition, UINT64 *size);
	imgtoolerr_t    (*read_file)    (imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf);
	imgtoolerr_t    (*write_file)   (imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts);
	imgtoolerr_t    (*delete_file)  (imgtool_partition *partition, const char *filename);
	imgtoolerr_t    (*list_forks)   (imgtool_partition *partition, const char *path, imgtool_forkent *ents, size_t len);
	imgtoolerr_t    (*create_dir)   (imgtool_partition *partition, const char *path);
	imgtoolerr_t    (*delete_dir)   (imgtool_partition *partition, const char *path);
	imgtoolerr_t    (*list_attrs)   (imgtool_partition *partition, const char *path, UINT32 *attrs, size_t len);
	imgtoolerr_t    (*get_attrs)    (imgtool_partition *partition, const char *path, const UINT32 *attrs, imgtool_attribute *values);
	imgtoolerr_t    (*set_attrs)    (imgtool_partition *partition, const char *path, const UINT32 *attrs, const imgtool_attribute *values);
	imgtoolerr_t    (*attr_name)    (UINT32 attribute, const imgtool_attribute *attr, char *buffer, size_t buffer_len);
	imgtoolerr_t    (*get_iconinfo) (imgtool_partition *partition, const char *path, imgtool_iconinfo *iconinfo);
	imgtoolerr_t    (*suggest_transfer)(imgtool_partition *partition, const char *path, imgtool_transfer_suggestion *suggestions, size_t suggestions_length);
	imgtoolerr_t    (*get_chain)    (imgtool_partition *partition, const char *path, imgtool_chainent *chain, size_t chain_size);

	const option_guide *writefile_optguide;
	const char *writefile_optspec;
};

struct imgtool_directory
{
	imgtool_partition *partition;
};



/***************************************************************************
    GLOBALS
***************************************************************************/

static imgtool_library *global_imgtool_library;

static int global_omit_untested;
static void (*global_warn)(const char *message);


void CLIB_DECL ATTR_PRINTF(1,2) logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}

/***************************************************************************

    Imgtool initialization and basics

***************************************************************************/
void rtrim(char *buf)
{
	size_t buflen;
	char *s;

	buflen = strlen(buf);
	if (buflen)
	{
		for (s = &buf[buflen-1]; s >= buf && (*s >= '\0') && isspace(*s); s--)
			*s = '\0';
	}
}

char *strncpyz(char *dest, const char *source, size_t len)
{
	char *s;
	if (len) {
		s = strncpy(dest, source, len - 1);
		dest[len-1] = '\0';
	}
	else {
		s = dest;
	}
	return s;
}

char *strncatz(char *dest, const char *source, size_t len)
{
	size_t l;
	l = strlen(dest);
	dest += l;
	if (len > l)
		len -= l;
	else
		len = 0;
	return strncpyz(dest, source, len);
}

/*-------------------------------------------------
    markerrorsource - marks where an error source
-------------------------------------------------*/

static imgtoolerr_t markerrorsource(imgtoolerr_t err)
{
	switch(err)
	{
		case IMGTOOLERR_OUTOFMEMORY:
		case IMGTOOLERR_UNEXPECTED:
		case IMGTOOLERR_BUFFERTOOSMALL:
			/* Do nothing */
			break;

		case IMGTOOLERR_FILENOTFOUND:
		case IMGTOOLERR_BADFILENAME:
			err = (imgtoolerr_t)(err | IMGTOOLERR_SRC_FILEONIMAGE);
			break;

		default:
			err = (imgtoolerr_t)(err | IMGTOOLERR_SRC_IMAGEFILE);
			break;
	}
	return err;
}

char *imgtool_basename(char *filename)
{
	char *c;

	// NULL begets NULL
	if (!filename)
		return nullptr;

	// start at the end and return when we hit a slash or colon
	for (c = filename + strlen(filename) - 1; c >= filename; c--)
		if (*c == '\\' || *c == '/' || *c == ':')
			return c + 1;

	// otherwise, return the whole thing
	return filename;
}

/*-------------------------------------------------
    internal_error - debug function for raising
    internal errors
-------------------------------------------------*/

static void internal_error(const imgtool_module *module, const char *message)
{
#ifdef MAME_DEBUG
	printf("%s: %s\n", module->name, message);
#endif
}


/*-------------------------------------------------
    normalize_filename - convert a filename to the
    native format used by the module
-------------------------------------------------*/

static char *normalize_filename(imgtool_partition *partition, const char *src)
{
	imgtool_charset charset;

	/* get charset from module */
	charset = (imgtool_charset) (int) imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_CHARSET);

	/* and dupe it */
	return native_from_utf8(charset, src);
}



/*-------------------------------------------------
    imgtool_init - initializes the imgtool core
-------------------------------------------------*/

void imgtool_init(int omit_untested, void (*warn)(const char *message))
{
	imgtoolerr_t err;
	err = imgtool_create_cannonical_library(omit_untested, &global_imgtool_library);
	assert(err == IMGTOOLERR_SUCCESS);
	if (err == IMGTOOLERR_SUCCESS)
	{
		imgtool_library_sort(global_imgtool_library, ITLS_DESCRIPTION);
	}
	global_omit_untested = omit_untested;
	global_warn = warn;
}



/*-------------------------------------------------
    imgtool_exit - closes out the imgtool core
-------------------------------------------------*/

void imgtool_exit(void)
{
	if (global_imgtool_library)
	{
		imgtool_library_close(global_imgtool_library);
		global_imgtool_library = nullptr;
	}
	global_warn = nullptr;
}



/*-------------------------------------------------
    imgtool_find_module - looks up a module
-------------------------------------------------*/

const imgtool_module *imgtool_find_module(const char *modulename)
{
	return imgtool_library_findmodule(global_imgtool_library, modulename);
}



/*-------------------------------------------------
    imgtool_get_module_features - retrieves a
    structure identifying this module's features
    associated with an image
-------------------------------------------------*/

imgtool_module_features imgtool_get_module_features(const imgtool_module *module)
{
	imgtool_module_features features;
	memset(&features, 0, sizeof(features));

	if (module->create)
		features.supports_create = 1;
	if (module->open)
		features.supports_open = 1;
	if (module->read_sector)
		features.supports_readsector = 1;
	if (module->write_sector)
		features.supports_writesector = 1;
	return features;
}



/*-------------------------------------------------
    imgtool_warn - issues a warning
-------------------------------------------------*/

void imgtool_warn(const char *format, ...)
{
	va_list va;
	char buffer[2000];

	if (global_warn)
	{
		va_start(va, format);
		vsprintf(buffer, format, va);
		va_end(va);
		global_warn(buffer);
	}
}



/*-------------------------------------------------
    evaluate_module - evaluates a single file to
    determine what module can best handle a file
-------------------------------------------------*/

static imgtoolerr_t evaluate_module(const char *fname,
	const imgtool_module *module, float *result)
{
	imgtoolerr_t err;
	imgtool_image *image = nullptr;
	imgtool_partition *partition = nullptr;
	imgtool_directory *imageenum = nullptr;
	imgtool_dirent ent;
	float current_result;

	*result = 0.0;

	err = imgtool_image_open(module, fname, OSD_FOPEN_READ, &image);
	if (err)
		goto done;

	if (image)
	{
		current_result = module->open_is_strict ? 0.9 : 0.5;

		err = imgtool_partition_open(image, 0, &partition);
		if (err)
			goto done;

		err = imgtool_directory_open(partition, nullptr, &imageenum);
		if (err)
			goto done;

		memset(&ent, 0, sizeof(ent));
		do
		{
			err = imgtool_directory_get_next(imageenum, &ent);
			if (err)
				goto done;

			if (ent.corrupt)
				current_result = (current_result * 99 + 1.00f) / 100;
			else
				current_result = (current_result + 1.00f) / 2;
		}
		while(!ent.eof);

		*result = current_result;
	}

done:
	if (ERRORCODE(err) == IMGTOOLERR_CORRUPTIMAGE)
		err = IMGTOOLERR_SUCCESS;
	if (imageenum)
		imgtool_directory_close(imageenum);
	if (partition)
		imgtool_partition_close(partition);
	if (image)
		imgtool_image_close(image);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_image - retrieves the image
    associated with this partition
-------------------------------------------------*/

imgtool_image *imgtool_partition_image(imgtool_partition *partition)
{
	return partition->image;
}



/*-------------------------------------------------
    imgtool_identify_file - attempts to determine the module
    for any given image
-------------------------------------------------*/

imgtoolerr_t imgtool_identify_file(const char *fname, imgtool_module **modules, size_t count)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	imgtool_library *library = global_imgtool_library;
	imgtool_module *module = nullptr;
	imgtool_module *insert_module;
	imgtool_module *temp_module;
	size_t i = 0;
	const char *extension;
	float val, temp_val, *values = nullptr;

	if (count <= 0)
	{
		err = IMGTOOLERR_UNEXPECTED;
		goto done;
	}

	for (i = 0; i < count; i++)
		modules[i] = nullptr;
	if (count > 1)
		count--;        /* null terminate */

	values = (float *) malloc(count * sizeof(*values));
	if (!values)
	{
		err = IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	for (i = 0; i < count; i++)
		values[i] = 0.0;

	/* figure out the file extension, if any */
	extension = strrchr(fname, '.');
	if (extension)
		extension++;

	/* iterate through all modules */
	while((module = imgtool_library_iterate(library, module)) != nullptr)
	{
		if (!extension || image_find_extension(module->extensions, extension))
		{
			err = evaluate_module(fname, module, &val);
			if (err)
				goto done;

			insert_module = module;
			for (i = 0; (val > 0.0f) && (i < count); i++)
			{
				if (val > values[i])
				{
					temp_val = values[i];
					temp_module = modules[i];
					values[i] = val;
					modules[i] = insert_module;
					val = temp_val;
					insert_module = temp_module;
				}
			}
		}
	}

	if (!modules[0])
		err = (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_IMAGEFILE);

done:
	if (values)
		free(values);
	return err;
}



/*-------------------------------------------------
    imgtool_image_get_sector_size - gets the size
    of a particular sector on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_get_sector_size(imgtool_image *image, UINT32 track, UINT32 head,
	UINT32 sector, UINT32 *length)
{
	/* implemented? */
	if (!image->module->get_sector_size)
		return (imgtoolerr_t )(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->get_sector_size(image, track, head, sector, length);
}



/*-------------------------------------------------
    imgtool_image_get_geometry - gets the geometry
    of an image; note that this may disagree with
    particular sectors; this is a common copy
    protection scheme
-------------------------------------------------*/

imgtoolerr_t imgtool_image_get_geometry(imgtool_image *image, UINT32 *tracks, UINT32 *heads, UINT32 *sectors)
{
	UINT32 dummy;

	/* some sanitization, to make the callbacks easier to implement */
	if (!tracks)
		tracks = &dummy;
	if (!heads)
		heads = &dummy;
	if (!sectors)
		sectors = &dummy;
	*tracks = 0;
	*heads = 0;
	*sectors = 0;

	/* implemented? */
	if (!image->module->get_geometry)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->get_geometry(image, tracks, heads, sectors);
}



/*-------------------------------------------------
    imgtool_image_read_sector - reads a sector on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_read_sector(imgtool_image *image, UINT32 track, UINT32 head,
	UINT32 sector, void *buffer, size_t len)
{
	/* implemented? */
	if (!image->module->read_sector)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->read_sector(image, track, head, sector, buffer, len);
}



/*-------------------------------------------------
    imgtool_image_write_sector - writes a sector on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_write_sector(imgtool_image *image, UINT32 track, UINT32 head,
	UINT32 sector, const void *buffer, size_t len)
{
	/* implemented? */
	if (!image->module->write_sector)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->write_sector(image, track, head, sector, buffer, len);
}



/*-------------------------------------------------
    imgtool_image_get_block_size - gets the size of
    a standard block on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_get_block_size(imgtool_image *image, UINT32 *length)
{
	/* implemented? */
	if (image->module->block_size == 0)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	*length = image->module->block_size;
	return IMGTOOLERR_SUCCESS;
}



/*-------------------------------------------------
    imgtool_image_read_block - reads a standard
    block on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_read_block(imgtool_image *image, UINT64 block, void *buffer)
{
	/* implemented? */
	if (!image->module->read_block)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->read_block(image, buffer, block);
}



/*-------------------------------------------------
    imgtool_image_write_block - writes a standard
    block on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_write_block(imgtool_image *image, UINT64 block, const void *buffer)
{
	/* implemented? */
	if (!image->module->write_block)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return image->module->write_block(image, buffer, block);
}



/*-------------------------------------------------
    imgtool_image_clear_block - clears a standard
    block on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_clear_block(imgtool_image *image, UINT64 block, UINT8 data)
{
	imgtoolerr_t err;
	UINT8 *block_data = nullptr;
	UINT32 length;

	err = imgtool_image_get_block_size(image, &length);
	if (err)
		goto done;

	block_data = (UINT8*)malloc(length);
	if (!block_data)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(block_data, data, length);

	err = imgtool_image_write_block(image, block, block_data);
	if (err)
		goto done;

done:
	if (block_data)
		free(block_data);
	return err;
}



/*-------------------------------------------------
    imgtool_image_list_partitions - lists the
    partitions on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_list_partitions(imgtool_image *image, imgtool_partition_info *partitions, size_t len)
{
	/* implemented? */
	if (!image->module->list_partitions)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	memset(partitions, '\0', sizeof(*partitions) * len);
	return image->module->list_partitions(image, partitions, len);
}



/*-------------------------------------------------
    imgtool_image_malloc - allocates memory associated with an
    image
-------------------------------------------------*/

void *imgtool_image_malloc(imgtool_image *image, size_t size)
{
	return pool_malloc_lib(image->pool, size);
}



/*-------------------------------------------------
    imgtool_image_module - returns the module associated with
    this image
-------------------------------------------------*/

const imgtool_module *imgtool_image_module(imgtool_image *img)
{
	return img->module;
}



/*-------------------------------------------------
    imgtool_image_extra_bytes - returns extra bytes on an image
-------------------------------------------------*/

void *imgtool_image_extra_bytes(imgtool_image *image)
{
	void *ptr = nullptr;
	if (image->module->image_extra_bytes > 0)
		ptr = ((UINT8 *) image) + sizeof(*image);
	assert(ptr);
	return ptr;
}



/*-------------------------------------------------
    imgtool_image_rand - returns a random number
-------------------------------------------------*/

UINT64 imgtool_image_rand(imgtool_image *image)
{
	/* we can't use mame_rand() here */
#ifdef rand
#undef rand
#endif
	return ((UINT64) rand()) << 32
		| ((UINT64) rand()) << 0;
}



/***************************************************************************

    Imgtool partition management

***************************************************************************/

static char *pool_strdup_allow_null(object_pool *pool, char *s)
{
	return s ? pool_strdup_lib(pool, s) : nullptr;
}



imgtoolerr_t imgtool_partition_open(imgtool_image *image, int partition_index, imgtool_partition **partition)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	imgtool_partition *p = nullptr;
	imgtool_class imgclass;
	imgtool_partition_info partition_info[32];
	UINT64 base_block, block_count;
	size_t partition_extra_bytes;
	object_pool *pool;
	imgtoolerr_t (*open_partition)(imgtool_partition *partition, UINT64 first_block, UINT64 block_count);

	if (image->module->list_partitions)
	{
		/* this image supports partitions */
		if ((partition_index < 0) || (partition_index >= ARRAY_LENGTH(partition_info)))
			return IMGTOOLERR_INVALIDPARTITION;

		/* retrieve the info on the partitions */
		memset(partition_info, '\0', sizeof(partition_info));
		err = image->module->list_partitions(image, partition_info, ARRAY_LENGTH(partition_info));
		if (err)
			return err;

		/* is this a valid partition */
		if (!partition_info[partition_index].get_info)
			return IMGTOOLERR_INVALIDPARTITION;

		/* use this partition */
		memset(&imgclass, 0, sizeof(imgclass));
		imgclass.get_info = partition_info[partition_index].get_info;
		base_block = partition_info[partition_index].base_block;
		block_count = partition_info[partition_index].block_count;
	}
	else
	{
		/* this image does not support partitions */
		if (partition_index != 0)
			return IMGTOOLERR_INVALIDPARTITION;

		/* identify the image class */
		imgclass = imgtool_image_module(image)->imgclass;
		base_block = 0;
		block_count = ~0;
	}

	/* does this partition type have extra bytes? */
	partition_extra_bytes = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES);

	/* allocate the new memory pool */
	pool = pool_alloc_lib(nullptr);
	if (!pool)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}

	/* allocate the new partition object */
	p = (imgtool_partition *) pool_malloc_lib(pool, sizeof(*p) + partition_extra_bytes);
	if (!p)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(p, 0, sizeof(*p) + partition_extra_bytes);
	p->pool = pool;

	/* fill out the structure */
	p->image                        = image;
	p->partition_index              = partition_index;
	p->base_block                   = base_block;
	p->block_count                  = block_count;
	p->imgclass                     = imgclass;
	p->partition_extra_bytes        = partition_extra_bytes;
	p->directory_extra_bytes        = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES);
	p->path_separator               = (char) imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PATH_SEPARATOR);
	p->alternate_path_separator     = (char) imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_ALTERNATE_PATH_SEPARATOR);
	p->prefer_ucase                 = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PREFER_UCASE) ? 1 : 0;
	p->supports_creation_time       = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME) ? 1 : 0;
	p->supports_lastmodified_time   = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME) ? 1 : 0;
	p->supports_bootblock           = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK) ? 1 : 0;
	p->begin_enum                   = (imgtoolerr_t (*)(imgtool_directory *, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_BEGIN_ENUM);
	p->next_enum                    = (imgtoolerr_t (*)(imgtool_directory *, imgtool_dirent *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_NEXT_ENUM);
	p->free_space                   = (imgtoolerr_t (*)(imgtool_partition *, UINT64 *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_FREE_SPACE);
	p->read_file                    = (imgtoolerr_t (*)(imgtool_partition *, const char *, const char *, imgtool_stream *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_READ_FILE);
	p->write_file                   = (imgtoolerr_t (*)(imgtool_partition *, const char *, const char *, imgtool_stream *, option_resolution *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_WRITE_FILE);
	p->delete_file                  = (imgtoolerr_t (*)(imgtool_partition *, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_DELETE_FILE);
	p->list_forks                   = (imgtoolerr_t (*)(imgtool_partition *, const char *, imgtool_forkent *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_LIST_FORKS);
	p->create_dir                   = (imgtoolerr_t (*)(imgtool_partition *, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_CREATE_DIR);
	p->delete_dir                   = (imgtoolerr_t (*)(imgtool_partition *, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_DELETE_DIR);
	p->list_attrs                   = (imgtoolerr_t (*)(imgtool_partition *, const char *, UINT32 *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_LIST_ATTRS);
	p->get_attrs                    = (imgtoolerr_t (*)(imgtool_partition *, const char *, const UINT32 *, imgtool_attribute *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_ATTRS);
	p->set_attrs                    = (imgtoolerr_t (*)(imgtool_partition *, const char *, const UINT32 *, const imgtool_attribute *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_SET_ATTRS);
	p->attr_name                    = (imgtoolerr_t (*)(UINT32, const imgtool_attribute *, char *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_ATTR_NAME);
	p->get_iconinfo                 = (imgtoolerr_t (*)(imgtool_partition *, const char *, imgtool_iconinfo *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_ICON_INFO);
	p->suggest_transfer             = (imgtoolerr_t (*)(imgtool_partition *, const char *, imgtool_transfer_suggestion *, size_t))  imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_SUGGEST_TRANSFER);
	p->get_chain                    = (imgtoolerr_t (*)(imgtool_partition *, const char *, imgtool_chainent *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_CHAIN);
	p->writefile_optguide           = (const option_guide *) imgtool_get_info_ptr(&imgclass, IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);
	p->writefile_optspec            = pool_strdup_allow_null(p->pool, (char*)imgtool_get_info_ptr(&imgclass, IMGTOOLINFO_STR_WRITEFILE_OPTSPEC));

	/* mask out if writing is untested */
	if (global_omit_untested && imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_WRITING_UNTESTED))
	{
		p->write_file = nullptr;
		p->delete_file = nullptr;
		p->create_dir = nullptr;
		p->delete_dir = nullptr;
		p->writefile_optguide = nullptr;
		p->writefile_optspec = nullptr;
	}

	/* call the partition open function, if present */
	open_partition = (imgtoolerr_t (*)(imgtool_partition *, UINT64, UINT64)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_OPEN_PARTITION);
	if (open_partition)
	{
		/* we have an open partition function */
		err = (*open_partition)(p, base_block, block_count);
		if (err)
			goto done;
	}

done:
	*partition = err ? nullptr : p;
	return err;
}



void imgtool_partition_close(imgtool_partition *partition)
{
	pool_free_lib(partition->pool);
}



/***************************************************************************

    Imgtool partition operations

***************************************************************************/

/*-------------------------------------------------
    imgtool_partition_get_attribute_name - retrieves the human readable
    name for an attribute
-------------------------------------------------*/

void imgtool_partition_get_attribute_name(imgtool_partition *partition, UINT32 attribute, const imgtool_attribute *attr_value,
	char *buffer, size_t buffer_len)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_UNIMPLEMENTED;

	buffer[0] = '\0';

	if (attr_value)
	{
		if (partition->attr_name)
			err = partition->attr_name(attribute, attr_value, buffer, buffer_len);

		if (err == (imgtoolerr_t)IMGTOOLERR_UNIMPLEMENTED)
		{
			switch(attribute & 0xF0000)
			{
				case IMGTOOLATTR_INT_FIRST:
					snprintf(buffer, buffer_len, "%d", (int) attr_value->i);
					break;
			}
		}
	}
	else
	{
		switch(attribute)
		{
			case IMGTOOLATTR_INT_MAC_TYPE:
				snprintf(buffer, buffer_len, "File type");
				break;
			case IMGTOOLATTR_INT_MAC_CREATOR:
				snprintf(buffer, buffer_len, "File creator");
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFLAGS:
				snprintf(buffer, buffer_len, "Finder flags");
				break;
			case IMGTOOLATTR_INT_MAC_COORDX:
				snprintf(buffer, buffer_len, "X coordinate");
				break;
			case IMGTOOLATTR_INT_MAC_COORDY:
				snprintf(buffer, buffer_len, "Y coordinate");
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFOLDER:
				snprintf(buffer, buffer_len, "Finder folder");
				break;
			case IMGTOOLATTR_INT_MAC_ICONID:
				snprintf(buffer, buffer_len, "Icon ID");
				break;
			case IMGTOOLATTR_INT_MAC_SCRIPTCODE:
				snprintf(buffer, buffer_len, "Script code");
				break;
			case IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS:
				snprintf(buffer, buffer_len, "Extended flags");
				break;
			case IMGTOOLATTR_INT_MAC_COMMENTID:
				snprintf(buffer, buffer_len, "Comment ID");
				break;
			case IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY:
				snprintf(buffer, buffer_len, "Putaway directory");
				break;
			case IMGTOOLATTR_TIME_CREATED:
				snprintf(buffer, buffer_len, "Creation time");
				break;
			case IMGTOOLATTR_TIME_LASTMODIFIED:
				snprintf(buffer, buffer_len, "Last modified time");
				break;
		}
	}
}



/*-------------------------------------------------
    imgtool_validitychecks - checks the validity
    of the imgtool modules
-------------------------------------------------*/

int imgtool_validitychecks(void)
{
	int error = 0;
	int val;
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	const imgtool_module *module = nullptr;
	const option_guide *guide_entry;
	imgtool_module_features features;
	int created_library = FALSE;

	if (!global_imgtool_library)
	{
		imgtool_init(FALSE, nullptr);
		created_library = TRUE;
	}

	while((module = imgtool_library_iterate(global_imgtool_library, module)) != nullptr)
	{
		features = imgtool_get_module_features(module);

		if (!module->name)
		{
			printf("imgtool module %s has null 'name'\n", module->name);
			error = 1;
		}
		if (!module->description)
		{
			printf("imgtool module %s has null 'description'\n", module->name);
			error = 1;
		}
		if (!module->extensions)
		{
			printf("imgtool module %s has null 'extensions'\n", module->extensions);
			error = 1;
		}

#if 0
		/* sanity checks on modules that do not support directories */
		if (!module->path_separator)
		{
			if (module->alternate_path_separator)
			{
				printf("imgtool module %s specified alternate_path_separator but not path_separator\n", module->name);
				error = 1;
			}
			if (module->initial_path_separator)
			{
				printf("imgtool module %s specified initial_path_separator without directory support\n", module->name);
				error = 1;
			}
			if (module->create_dir)
			{
				printf("imgtool module %s implements create_dir without directory support\n", module->name);
				error = 1;
			}
			if (module->delete_dir)
			{
				printf("imgtool module %s implements delete_dir without directory support\n", module->name);
				error = 1;
			}
		}
#endif

		/* sanity checks on sector operations */
		if (module->read_sector && !module->get_sector_size)
		{
			printf("imgtool module %s implements read_sector without supporting get_sector_size\n", module->name);
			error = 1;
		}
		if (module->write_sector && !module->get_sector_size)
		{
			printf("imgtool module %s implements write_sector without supporting get_sector_size\n", module->name);
			error = 1;
		}

		/* sanity checks on creation options */
		if (module->createimage_optguide || module->createimage_optspec)
		{
			if (!module->create)
			{
				printf("imgtool module %s has creation options without supporting create\n", module->name);
				error = 1;
			}
			if ((!module->createimage_optguide && module->createimage_optspec)
				|| (module->createimage_optguide && !module->createimage_optspec))
			{
				printf("imgtool module %s does has partially incomplete creation options\n", module->name);
				error = 1;
			}

			if (module->createimage_optguide && module->createimage_optspec)
			{
				guide_entry = module->createimage_optguide;
				while(guide_entry->option_type != OPTIONTYPE_END)
				{
					if (option_resolution_contains(module->createimage_optspec, guide_entry->parameter))
					{
						switch(guide_entry->option_type)
						{
							case OPTIONTYPE_INT:
							case OPTIONTYPE_ENUM_BEGIN:
								err = (imgtoolerr_t)option_resolution_getdefault(module->createimage_optspec,
									guide_entry->parameter, &val);
								if (err)
									goto done;
								break;

							default:
								break;
						}
						if (!guide_entry->identifier)
						{
							printf("imgtool module %s creation option %d has null identifier\n",
								module->name, (int) (guide_entry - module->createimage_optguide));
							error = 1;
						}
						if (!guide_entry->display_name)
						{
							printf("imgtool module %s creation option %d has null display_name\n",
								module->name, (int) (guide_entry - module->createimage_optguide));
							error = 1;
						}
					}
					guide_entry++;
				}
			}
		}
	}

done:
	if (created_library)
		imgtool_exit();
	if (err)
	{
		printf("imgtool: %s\n", imgtool_error(err));
		error = 1;
	}
	return error;
}



/*-------------------------------------------------
    imgtool_temp_str - provides a temporary string
    buffer used for string passing
-------------------------------------------------*/

char *imgtool_temp_str(void)
{
	static int index;
	static char temp_string_pool[32][256];
	return temp_string_pool[index++ % ARRAY_LENGTH(temp_string_pool)];
}



/***************************************************************************

    Image handling functions

***************************************************************************/

static imgtoolerr_t internal_open(const imgtool_module *module, const char *fname,
	int read_or_write, option_resolution *createopts, imgtool_image **outimg)
{
	imgtoolerr_t err;
	imgtool_stream *f = nullptr;
	imgtool_image *image = nullptr;
	object_pool *pool = nullptr;
	size_t size;

	if (outimg)
		*outimg = nullptr;

	/* is the requested functionality implemented? */
	if ((read_or_write == OSD_FOPEN_RW_CREATE) ? !module->create : !module->open)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	/* create a memory pool */
	pool = pool_alloc_lib(nullptr);
	if (!pool)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_OUTOFMEMORY);
		goto done;
	}

	/* open the stream */
	f = stream_open(fname, read_or_write);
	if (!f)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_IMAGEFILE);
		goto done;
	}

	/* setup the image structure */
	size = sizeof(imgtool_image) + module->image_extra_bytes;
	image = (imgtool_image *) pool_malloc_lib(pool, size);
	if (!image)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(image, '\0', size);
	image->pool = pool;
	image->module = module;

	/* actually call create or open */
	if (read_or_write == OSD_FOPEN_RW_CREATE)
		err = module->create(image, f, createopts);
	else
		err = module->open(image, f);
	if (err)
	{
		err = markerrorsource(err);
		goto done;
	}

done:
	if (err)
	{
		if (f)
			stream_close(f);
		if (pool)
			pool_free_lib(pool);
		image = nullptr;
	}

	if (outimg)
		*outimg = image;
	else if (image)
		imgtool_image_close(image);
	return err;
}



/*-------------------------------------------------
    imgtool_image_open - open an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_open(const imgtool_module *module, const char *fname, int read_or_write, imgtool_image **outimg)
{
	read_or_write = read_or_write ? OSD_FOPEN_RW : OSD_FOPEN_READ;
	return internal_open(module, fname, read_or_write, nullptr, outimg);
}



/*-------------------------------------------------
    imgtool_image_open_byname - open an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_open_byname(const char *modulename, const char *fname, int read_or_write, imgtool_image **outimg)
{
	const imgtool_module *module;

	module = imgtool_find_module(modulename);
	if (!module)
		return (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);

	return imgtool_image_open(module, fname, read_or_write, outimg);
}



/*-------------------------------------------------
    imgtool_image_close - close an image
-------------------------------------------------*/

void imgtool_image_close(imgtool_image *image)
{
	if (image->module->close)
		image->module->close(image);
	pool_free_lib(image->pool);
}



/*-------------------------------------------------
    imgtool_image_create - creates an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_create(const imgtool_module *module, const char *fname,
	option_resolution *opts, imgtool_image **image)
{
	imgtoolerr_t err;
	option_resolution *alloc_resolution = nullptr;

	/* allocate dummy options if necessary */
	if (!opts && module->createimage_optguide)
	{
		alloc_resolution = option_resolution_create(module->createimage_optguide, module->createimage_optspec);
		if (!alloc_resolution)
		{
			err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
			goto done;
		}
		opts = alloc_resolution;
	}
	if (opts)
		option_resolution_finish(opts);

	err = internal_open(module, fname, OSD_FOPEN_RW_CREATE, opts, image);
	if (err)
		goto done;

done:
	if (alloc_resolution)
		option_resolution_close(alloc_resolution);
	return err;
}



/*-------------------------------------------------
    imgtool_image_create_byname - creates an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_create_byname(const char *modulename, const char *fname,
	option_resolution *opts, imgtool_image **image)
{
	const imgtool_module *module;

	module = imgtool_find_module(modulename);
	if (!module)
		return (imgtoolerr_t)(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);

	return imgtool_image_create(module, fname, opts, image);
}



/*-------------------------------------------------
    imgtool_image_info - returns format specific information
    about an image
-------------------------------------------------*/

imgtoolerr_t imgtool_image_info(imgtool_image *image, char *string, size_t len)
{
	if (len > 0)
	{
		string[0] = '\0';
		if (image->module->info)
			image->module->info(image, string, len);
	}
	return IMGTOOLERR_SUCCESS;
}



#define PATH_MUSTBEDIR      0x00000001
#define PATH_LEAVENULLALONE 0x00000002
#define PATH_CANBEBOOTBLOCK 0x00000004

/*-------------------------------------------------
    cannonicalize_path - normalizes a path string
    into a NUL delimited list
-------------------------------------------------*/

static imgtoolerr_t cannonicalize_path(imgtool_partition *partition, UINT32 flags,
	const char **path, char **alloc_path)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	char *new_path = nullptr;
	char path_separator, alt_path_separator;
	const char *s;
	int in_path_separator, i, j;

	path_separator = partition->path_separator;
	alt_path_separator = partition->alternate_path_separator;

	/* is this path NULL?  if so, is that ignored? */
	if (!*path && (flags & PATH_LEAVENULLALONE))
		goto done;

	/* is this the special filename for bootblocks? */
	if (*path == FILENAME_BOOTBLOCK)
	{
		if (!(flags & PATH_CANBEBOOTBLOCK))
			err = (imgtoolerr_t)IMGTOOLERR_UNEXPECTED;
		else if (!partition->supports_bootblock)
			err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;
		goto done;
	}

	if (path_separator == '\0')
	{
		if (flags & PATH_MUSTBEDIR)
		{
			/* do we specify a path when paths are not supported? */
			if (*path && **path)
			{
				err = (imgtoolerr_t)(IMGTOOLERR_CANNOTUSEPATH | IMGTOOLERR_SRC_FUNCTIONALITY);
				goto done;
			}
			*path = nullptr;   /* normalize empty path */
		}
	}
	else
	{
		s = *path ? *path : "";

		/* allocate space for a new cannonical path */
		new_path = (char*)malloc(strlen(s) + 4);
		if (!new_path)
		{
			err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
			goto done;
		}

		/* copy the path */
		in_path_separator = TRUE;
		i = j = 0;
		do
		{
			if ((s[i] != '\0') && (s[i] != path_separator) && (s[i] != alt_path_separator))
			{
				new_path[j++] = s[i];
				in_path_separator = FALSE;
			}
			else if (!in_path_separator)
			{
				new_path[j++] = '\0';
				in_path_separator = TRUE;
			}
		}
		while(s[i++] != '\0');
		new_path[j++] = '\0';
		new_path[j++] = '\0';
		*path = new_path;
	}

done:
	*alloc_path = new_path;
	return err;
}



static imgtoolerr_t cannonicalize_fork(imgtool_partition *partition, const char **fork)
{
	/* does this module support forks? */
	if (partition->list_forks)
	{
		/* this module supports forks; make sure that fork is non-NULL */
		if (!*fork)
			*fork = "";
	}
	else
	{
		/* this module does not support forks; make sure that fork is NULL */
		if (*fork)
			return IMGTOOLERR_NOFORKS;
	}
	return IMGTOOLERR_SUCCESS;
}



/*-------------------------------------------------
    imgtool_partition_get_directory_entry - retrieves
    the nth directory entry within a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_directory_entry(imgtool_partition *partition, const char *path, int index, imgtool_dirent *ent)
{
	imgtoolerr_t err;
	imgtool_directory *imgenum = nullptr;

	if (index < 0)
	{
		err = (imgtoolerr_t)IMGTOOLERR_PARAMTOOSMALL;
		goto done;
	}

	err = imgtool_directory_open(partition, path, &imgenum);
	if (err)
		goto done;

	do
	{
		err = imgtool_directory_get_next(imgenum, ent);
		if (err)
			goto done;

		if (ent->eof)
		{
			err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;
			goto done;
		}
	}
	while(index--);

done:
	if (err)
		memset(ent->filename, 0, sizeof(ent->filename));
	if (imgenum)
		imgtool_directory_close(imgenum);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_file_size - returns free
    space on a partition, in bytes
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_file_size(imgtool_partition *partition, const char *fname, UINT64 *filesize)
{
	imgtoolerr_t err;
	imgtool_directory *imgenum;
	imgtool_dirent ent;
	const char *path;

	path = nullptr;    /* TODO: Need to parse off the path */

	*filesize = -1;
	memset(&ent, 0, sizeof(ent));

	err = imgtool_directory_open(partition, path, &imgenum);
	if (err)
		goto done;

	do
	{
		err = imgtool_directory_get_next(imgenum, &ent);
		if (err)
			goto done;

		if (!core_stricmp(fname, ent.filename))
		{
			*filesize = ent.filesize;
			goto done;
		}
	}
	while(ent.filename[0]);

	err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;

done:
	if (imgenum)
		imgtool_directory_close(imgenum);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_list_file_attributes - identifies
    all attributes on a file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_list_file_attributes(imgtool_partition *partition, const char *path, UINT32 *attrs, size_t len)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_fname = nullptr;

	memset(attrs, 0, sizeof(*attrs) * len);

	if (!partition->list_attrs)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_fname = normalize_filename(partition, path);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_fname;

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_LEAVENULLALONE, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->list_attrs(partition, path, attrs, len);
	if (err)
		goto done;

done:
	if (alloc_path != nullptr)
		free(alloc_path);
	if (new_fname != nullptr)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_file_attributes - retrieves
    attributes on a file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_file_attributes(imgtool_partition *partition, const char *path, const UINT32 *attrs, imgtool_attribute *values)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_fname = nullptr;

	if (!partition->get_attrs)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_fname = normalize_filename(partition, path);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_fname;

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_LEAVENULLALONE, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->get_attrs(partition, path, attrs, values);
	if (err)
		goto done;

done:
	if (alloc_path != nullptr)
		free(alloc_path);
	if (new_fname != nullptr)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_put_file_attributes - sets
    attributes on a file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_put_file_attributes(imgtool_partition *partition, const char *path, const UINT32 *attrs, const imgtool_attribute *values)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;

	if (!partition->set_attrs)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_LEAVENULLALONE, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->set_attrs(partition, path, attrs, values);
	if (err)
		goto done;

done:
	if (alloc_path)
		free(alloc_path);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_file_attributes - retrieves
    an attribute on a single file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_file_attribute(imgtool_partition *partition, const char *path, UINT32 attr, imgtool_attribute *value)
{
	UINT32 attrs[2];
	attrs[0] = attr;
	attrs[1] = 0;
	return imgtool_partition_get_file_attributes(partition, path, attrs, value);
}



/*-------------------------------------------------
    imgtool_partition_put_file_attribute - sets
    attributes on a single file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_put_file_attribute(imgtool_partition *partition, const char *path, UINT32 attr, imgtool_attribute value)
{
	UINT32 attrs[2];
	attrs[0] = attr;
	attrs[1] = 0;
	return imgtool_partition_put_file_attributes(partition, path, attrs, &value);
}



/*-------------------------------------------------
    imgtool_partition_get_icon_info - retrieves the
    icon for a file stored on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_icon_info(imgtool_partition *partition, const char *path, imgtool_iconinfo *iconinfo)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_fname = nullptr;

	if (!partition->get_iconinfo)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_fname = normalize_filename(partition, path);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_fname;

	/* cannonicalize path */
	err = cannonicalize_path(partition, 0, &path, &alloc_path);
	if (err)
		goto done;

	memset(iconinfo, 0, sizeof(*iconinfo));
	err = partition->get_iconinfo(partition, path, iconinfo);
	if (err)
		goto done;

done:
	if (alloc_path)
		free(alloc_path);
	if (new_fname)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_suggest_file_filters - suggests
    a list of filters appropriate for a file on a
    partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_suggest_file_filters(imgtool_partition *partition, const char *path,
	imgtool_stream *stream, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	int i, j;
	char *alloc_path = nullptr;
	imgtoolerr_t (*check_stream)(imgtool_stream *stream, imgtool_suggestion_viability_t *viability);
	size_t position;

	/* clear out buffer */
	memset(suggestions, 0, sizeof(*suggestions) * suggestions_length);

	if (!partition->suggest_transfer)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_LEAVENULLALONE, &path, &alloc_path);
	if (err)
		goto done;

	/* invoke the module's suggest call */
	err = partition->suggest_transfer(partition, path, suggestions, suggestions_length);
	if (err)
		goto done;

	/* Loop on resulting suggestions, and do the following:
	 * 1.  Call check_stream if present, and remove disqualified streams
	 * 2.  Fill in missing descriptions
	 */
	i = j = 0;
	while(suggestions[i].viability)
	{
		if (stream && suggestions[i].filter)
		{
			check_stream = (imgtoolerr_t (*)(imgtool_stream *, imgtool_suggestion_viability_t *)) filter_get_info_fct(suggestions[i].filter, FILTINFO_PTR_CHECKSTREAM);
			if (check_stream)
			{
				position = stream_tell(stream);
				err = check_stream(stream, &suggestions[i].viability);
				stream_seek(stream, position, SEEK_SET);
				if (err)
					goto done;
			}
		}

		/* the check_stream proc can remove the option by clearing out the viability */
		if (suggestions[i].viability)
		{
			/* we may have to move this suggestion, if one was removed */
			if (i != j)
				memcpy(&suggestions[j], &suggestions[i], sizeof(*suggestions));

			/* if the description is missing, fill it in */
			if (!suggestions[j].description)
			{
				if (suggestions[j].filter)
					suggestions[j].description = filter_get_info_string(suggestions[i].filter, FILTINFO_STR_HUMANNAME);
				else
					suggestions[j].description = "Raw";
			}

			j++;
		}
		i++;
	}
	suggestions[j].viability = (imgtool_suggestion_viability_t)0;

done:
	if (alloc_path)
		free(alloc_path);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_chain - retrieves the block
    chain for a file or directory on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_chain(imgtool_partition *partition, const char *path, imgtool_chainent *chain, size_t chain_size)
{
	size_t i;

	assert(chain_size > 0);

	if (!partition->get_chain)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	/* initialize the chain array, so the module's get_chain function can be lazy */
	for (i = 0; i < chain_size; i++)
	{
		chain[i].level = 0;
		chain[i].block = ~0;
	}

	return partition->get_chain(partition, path, chain, chain_size - 1);
}



/*-------------------------------------------------
    imgtool_partition_get_chain_string - retrieves
    the block chain for a file or directory on a
    partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_chain_string(imgtool_partition *partition, const char *path, char *buffer, size_t buffer_len)
{
	imgtoolerr_t err;
	imgtool_chainent chain[512];
	UINT64 last_block;
	UINT8 cur_level = 0;
	int len, i;
	int comma_needed = FALSE;

	/* determine the last block identifier */
	chain[0].block = ~0;
	last_block = chain[0].block;

	err = imgtool_partition_get_chain(partition, path, chain, ARRAY_LENGTH(chain));
	if (err)
		return err;

	len = snprintf(buffer, buffer_len, "[");
	buffer += len;
	buffer_len -= len;

	for (i = 0; chain[i].block != last_block; i++)
	{
		while(cur_level < chain[i].level)
		{
			len = snprintf(buffer, buffer_len, " [");
			buffer += len;
			buffer_len -= len;
			cur_level++;
			comma_needed = FALSE;
		}
		while(cur_level > chain[i].level)
		{
			len = snprintf(buffer, buffer_len, "]");
			buffer += len;
			buffer_len -= len;
			cur_level--;
		}

		if (comma_needed)
		{
			len = snprintf(buffer, buffer_len, ", ");
			buffer += len;
			buffer_len -= len;
		}

		len = snprintf(buffer, buffer_len, "%u", (unsigned) chain[i].block);
		buffer += len;
		buffer_len -= len;
		comma_needed = TRUE;
	}

	do
	{
		len = snprintf(buffer, buffer_len, "]");
		buffer += len;
		buffer_len -= len;
	}
	while(cur_level-- > 0);

	return IMGTOOLERR_SUCCESS;
}



/*-------------------------------------------------
    imgtool_partition_get_free_space - returns the
    amount of free space on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_free_space(imgtool_partition *partition, UINT64 *sz)
{
	imgtoolerr_t err;
	UINT64 size;

	if (!partition->free_space)
		return (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	err = partition->free_space(partition, &size);
	if (err)
		return (imgtoolerr_t)(err | IMGTOOLERR_SRC_IMAGEFILE);

	if (sz)
		*sz = size;
	return IMGTOOLERR_SUCCESS;
}



/*-------------------------------------------------
    imgtool_partition_read_file - starts reading
    from a file on a partition with a stream
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_read_file(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	union filterinfo u;

	if (!partition->read_file)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	if (filter)
	{
		/* use a filter */
		filter(FILTINFO_PTR_READFILE, &u);
		if (!u.read_file)
		{
			err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
			goto done;
		}

		err = u.read_file(partition, filename, fork, destf);
		if (err)
		{
			err = markerrorsource(err);
			goto done;
		}
	}
	else
	{
		/* cannonicalize path */
		err = cannonicalize_path(partition, PATH_CANBEBOOTBLOCK, &filename, &alloc_path);
		if (err)
			goto done;

		err = cannonicalize_fork(partition, &fork);
		if (err)
			goto done;

		/* invoke the actual module */
		err = partition->read_file(partition, filename, fork, destf);
		if (err)
		{
			err = markerrorsource(err);
			goto done;
		}
	}

done:
	if (alloc_path)
		free(alloc_path);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_write_file - starts writing
    to a new file on an image with a stream
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_write_file(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	char *buf = nullptr;
	char *s;
	option_resolution *alloc_resolution = nullptr;
	char *alloc_path = nullptr;
	UINT64 free_space;
	UINT64 file_size;
	union filterinfo u;

	if (!partition->write_file)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	if (filter)
	{
		/* use a filter */
		filter(FILTINFO_PTR_WRITEFILE, &u);
		if (!u.write_file)
		{
			err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
			goto done;
		}

		err = u.write_file(partition, filename, fork, sourcef, opts);
		if (err)
		{
			err = markerrorsource(err);
			goto done;
		}
	}
	else
	{
		/* does this partition prefer upper case file names? */
		if (partition->prefer_ucase)
		{
			buf = (char*)malloc(strlen(filename) + 1);
			if (!buf)
			{
				err = (imgtoolerr_t)(IMGTOOLERR_OUTOFMEMORY);
				goto done;
			}
			strcpy(buf, filename);
			for (s = buf; *s; s++)
				*s = toupper(*s);
			filename = buf;
		}

		/* cannonicalize path */
		err = cannonicalize_path(partition, PATH_CANBEBOOTBLOCK, &filename, &alloc_path);
		if (err)
			goto done;

		err = cannonicalize_fork(partition, &fork);
		if (err)
			goto done;

		/* allocate dummy options if necessary */
		if (!opts && partition->writefile_optguide)
		{
			alloc_resolution = option_resolution_create(partition->writefile_optguide, partition->writefile_optspec);
			if (!alloc_resolution)
			{
				err = IMGTOOLERR_OUTOFMEMORY;
				goto done;
			}
			opts = alloc_resolution;
		}
		if (opts)
			option_resolution_finish(opts);

		/* if free_space is implemented; do a quick check to see if space is available */
		if (partition->free_space)
		{
			err = partition->free_space(partition, &free_space);
			if (err)
			{
				err = markerrorsource(err);
				goto done;
			}

			file_size = stream_size(sourcef);

			if (file_size > free_space)
			{
				err = markerrorsource(IMGTOOLERR_NOSPACE);
				goto done;
			}
		}

		/* actually invoke the write file handler */
		err = partition->write_file(partition, filename, fork, sourcef, opts);
		if (err)
		{
			err = markerrorsource(err);
			goto done;
		}
	}

done:
	if (buf)
		free(buf);
	if (alloc_path)
		free(alloc_path);
	if (alloc_resolution)
		option_resolution_close(alloc_resolution);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_file - read a file from
    an image, storing it into a native file
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_file(imgtool_partition *partition, const char *filename, const char *fork,
	const char *dest, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	imgtool_stream *f;
	char *new_fname = nullptr;
	char *alloc_dest = nullptr;
	const char *filter_extension = nullptr;

	if (!dest)
	{
		/* determine the filter extension, if appropriate */
		if (filter != nullptr)
			filter_extension = filter_get_info_string(filter, FILTINFO_STR_EXTENSION);

		if (filter_extension != nullptr)
		{
			alloc_dest = (char*)malloc(strlen(filename) + 1 + strlen(filter_extension) + 1);
			if (!alloc_dest)
				return IMGTOOLERR_OUTOFMEMORY;

			sprintf(alloc_dest, "%s.%s", filename, filter_extension);
			dest = alloc_dest;
		}
		else
		{
			if (filename == FILENAME_BOOTBLOCK)
				dest = "boot.bin";
			else
				dest = filename;
		}
	}

	f = stream_open(dest, OSD_FOPEN_WRITE);
	if (!f)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);
		goto done;
	}

	new_fname = normalize_filename(partition, filename);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	err = imgtool_partition_read_file(partition, new_fname, fork, f, filter);
	if (err)
		goto done;

done:
	if (f != nullptr)
		stream_close(f);
	if (alloc_dest != nullptr)
		free(alloc_dest);
	if (new_fname != nullptr)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_put_file - read a native file
    and store it on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_put_file(imgtool_partition *partition, const char *newfname, const char *fork,
	const char *source, option_resolution *opts, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	imgtool_stream *f = nullptr;
	imgtool_charset charset;
	char *alloc_newfname = nullptr;

	if (!newfname)
		newfname = (const char *) imgtool_basename((char *) source);

	charset = (imgtool_charset) (int) imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_CHARSET);
	if (charset != IMGTOOL_CHARSET_UTF8)
	{
		/* convert to native format */
		alloc_newfname = native_from_utf8(charset, newfname);
		if (alloc_newfname == nullptr)
		{
			err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_NATIVEFILE);
			goto done;
		}

		newfname = alloc_newfname;
	}

	f = stream_open(source, OSD_FOPEN_READ);
	if (f)
		err = imgtool_partition_write_file(partition, newfname, fork, f, opts, filter);
	else
		err = (imgtoolerr_t)(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);

done:
	/* clean up */
	if (f != nullptr)
		stream_close(f);
	if (alloc_newfname != nullptr)
		osd_free(alloc_newfname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_delete_file - delete a file
    on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_delete_file(imgtool_partition *partition, const char *fname)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_fname = nullptr;

	if (!partition->delete_file)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_fname = normalize_filename(partition, fname);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	fname = new_fname;

	/* cannonicalize path */
	err = cannonicalize_path(partition, 0, &fname, &alloc_path);
	if (err)
		goto done;

	err = partition->delete_file(partition, fname);
	if (err)
	{
		err = markerrorsource(err);
		goto done;
	}

done:
	if (alloc_path)
		free(alloc_path);
	if (new_fname)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_list_file_forks - lists all
    forks on an image
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_list_file_forks(imgtool_partition *partition, const char *path, imgtool_forkent *ents, size_t len)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_fname = nullptr;

	if (!partition->list_forks)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_fname = normalize_filename(partition, path);
	if (new_fname == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_fname;

	/* cannonicalize path */
	err = cannonicalize_path(partition, 0, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->list_forks(partition, path, ents, len);
	if (err)
		goto done;

done:
	if (alloc_path)
		free(alloc_path);
	if (new_fname)
		osd_free(new_fname);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_create_directory - creates a
    directory on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_create_directory(imgtool_partition *partition, const char *path)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_path = nullptr;

	/* implemented? */
	if (!partition->create_dir)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_path = normalize_filename(partition, path);
	if (new_path == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_path;

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_MUSTBEDIR, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->create_dir(partition, path);
	if (err)
		goto done;

done:
	if (alloc_path)
		free(alloc_path);
	if (new_path)
		osd_free(new_path);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_delete_directory - deletes a
    directory on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_delete_directory(imgtool_partition *partition, const char *path)
{
	imgtoolerr_t err;
	char *alloc_path = nullptr;
	char *new_path = nullptr;

	/* implemented? */
	if (!partition->delete_dir)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_path = normalize_filename(partition, path);
	if (new_path == nullptr)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	path = new_path;

	/* cannonicalize path */
	err = cannonicalize_path(partition, PATH_MUSTBEDIR, &path, &alloc_path);
	if (err)
		goto done;

	err = partition->delete_dir(partition, path);
	if (err)
		goto done;

done:
	if (alloc_path)
		free(alloc_path);
	if (new_path)
		osd_free(new_path);
	return err;
}



/*-------------------------------------------------
    imgtool_partition_get_block_size - gets the
    size of a standard block on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_get_block_size(imgtool_partition *partition, UINT32 *length)
{
	return imgtool_image_get_block_size(partition->image, length);
}



/*-------------------------------------------------
    imgtool_partition_read_block - reads a standard
    block on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_read_block(imgtool_partition *partition, UINT64 block, void *buffer)
{
	if (block >= partition->block_count)
		return IMGTOOLERR_SEEKERROR;
	return imgtool_image_read_block(partition->image, block + partition->base_block, buffer);
}



/*-------------------------------------------------
    imgtool_partition_write_block - writes a
    standard block on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_partition_write_block(imgtool_partition *partition, UINT64 block, const void *buffer)
{
	if (block >= partition->block_count)
		return IMGTOOLERR_SEEKERROR;
	return imgtool_image_write_block(partition->image, block + partition->base_block, buffer);
}



/*-------------------------------------------------
    imgtool_partition_get_features - retrieves a
    structure identifying this partition's features
    associated with an image
-------------------------------------------------*/

imgtool_partition_features imgtool_partition_get_features(imgtool_partition *partition)
{
	imgtool_partition_features features;
	memset(&features, 0, sizeof(features));

	if (partition->read_file)
		features.supports_reading = 1;
	if (partition->write_file)
		features.supports_writing = 1;
	if (partition->delete_file)
		features.supports_deletefile = 1;
	if (partition->path_separator)
		features.supports_directories = 1;
	if (partition->free_space)
		features.supports_freespace = 1;
	if (partition->create_dir)
		features.supports_createdir = 1;
	if (partition->delete_dir)
		features.supports_deletedir = 1;
	if (partition->supports_creation_time)
		features.supports_creation_time = 1;
	if (partition->supports_lastmodified_time)
		features.supports_lastmodified_time = 1;
	if (!features.supports_writing && !features.supports_createdir && !features.supports_deletefile && !features.supports_deletedir)
		features.is_read_only = 1;
	return features;
}



/*-------------------------------------------------
    imgtool_partition_get_info_ptr - retrieves a
    pointer associated with a partition's format
-------------------------------------------------*/

void *imgtool_partition_get_info_ptr(imgtool_partition *partition, UINT32 state)
{
	return imgtool_get_info_ptr(&partition->imgclass, state);
}



/*-------------------------------------------------
    imgtool_partition_get_info_string - retrieves a
    string associated with a partition's format
-------------------------------------------------*/

const char *imgtool_partition_get_info_string(imgtool_partition *partition, UINT32 state)
{
	return imgtool_get_info_string(&partition->imgclass, state);
}



/*-------------------------------------------------
    imgtool_partition_get_info_int - retrieves a
    pointer associated with a partition's format
-------------------------------------------------*/

UINT64 imgtool_partition_get_info_int(imgtool_partition *partition, UINT32 state)
{
	return imgtool_get_info_int(&partition->imgclass, state);
}



/*-------------------------------------------------
    imgtool_partition_extra_bytes - returns extra
    bytes on a partition
-------------------------------------------------*/

void *imgtool_partition_extra_bytes(imgtool_partition *partition)
{
	void *ptr = nullptr;
	if (partition->partition_extra_bytes > 0)
		ptr = ((UINT8 *) partition) + sizeof(*partition);
	assert(ptr);
	return ptr;
}



/*-------------------------------------------------
    imgtool_partition_rand - returns random number
-------------------------------------------------*/

UINT64 imgtool_partition_rand(imgtool_partition *partition)
{
	return imgtool_image_rand(partition->image);
}



/***************************************************************************

    Path handling functions

***************************************************************************/

/*-------------------------------------------------
    imgtool_partition_get_root_path - retrieves
    the path root of this partition
-------------------------------------------------*/

const char *imgtool_partition_get_root_path(imgtool_partition *partition)
{
	int initial_path_separator;
	char path_separator;
	char *buf;
	int pos = 0;

	initial_path_separator = imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR) ? 1 : 0;
	path_separator = (char) imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_PATH_SEPARATOR);
	buf = imgtool_temp_str();

	if (initial_path_separator)
		buf[pos++] = path_separator;
	buf[pos] = '\0';
	return buf;
}



/*-------------------------------------------------
    imgtool_partition_path_concatenate - retrieves
    a pointer associated with a partition's format
-------------------------------------------------*/

const char *imgtool_partition_path_concatenate(imgtool_partition *partition, const char *path1, const char *path2)
{
	char path_separator;
	size_t len;
	char *buffer;
	size_t buffer_len;

	path_separator = (char) imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_PATH_SEPARATOR);
	len = strlen(path1);

	/* prepare buffer */
	buffer = imgtool_temp_str();
	buffer_len = 256;

	if (!strcmp(path2, "."))
	{
		/* keep the same directory */
		snprintf(buffer, buffer_len, "%s", path1);
	}
	else if (!strcmp(path2, ".."))
	{
		/* up one directory */
		while((len > 0) && (path1[len - 1] == path_separator))
			len--;
		while((len > 0) && (path1[len - 1] != path_separator))
			len--;
		while((len > 1) && (path1[len - 1] == path_separator))
			len--;
		snprintf(buffer, buffer_len, "%s", path1);
		buffer[len] = '\0';
	}
	else
	{
		/* append a path */
		if ((*path1 != 0) && (path1[strlen(path1) - 1] != path_separator))
			snprintf(buffer, buffer_len, "%s%c%s", path1, path_separator, path2);
		else
			snprintf(buffer, buffer_len, "%s%s", path1, path2);
	}
	return buffer;
}



/*-------------------------------------------------
    imgtool_partition_get_base_name - retrieves
    a base name for a partition specific path
-------------------------------------------------*/

const char *imgtool_partition_get_base_name(imgtool_partition *partition, const char *path)
{
	char path_separator;
	const char *new_path = path;
	int i;

	path_separator = (char) imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_PATH_SEPARATOR);

	for (i = 0; path[i]; i++)
	{
		if (path[i] == path_separator)
			new_path = &path[i + 1];
	}
	return new_path;
}



/***************************************************************************

    Directory handling functions

***************************************************************************/

/*-------------------------------------------------
    imgtool_directory_open - begins
    enumerating files on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_directory_open(imgtool_partition *partition, const char *path, imgtool_directory **outenum)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	imgtool_directory *enumeration = nullptr;
	char *alloc_path = nullptr;
	char *new_path = nullptr;
	size_t size;

	/* sanity checks */
	assert(partition);
	assert(outenum);

	*outenum = nullptr;

	if (!partition->next_enum)
	{
		err = (imgtoolerr_t)(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	new_path = normalize_filename(partition, path);
	path = new_path;

	err = cannonicalize_path(partition, PATH_MUSTBEDIR, &path, &alloc_path);
	if (err)
		goto done;

	size = sizeof(imgtool_directory) + partition->directory_extra_bytes;
	enumeration = (imgtool_directory *) malloc(size);
	if (!enumeration)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(enumeration, '\0', size);
	enumeration->partition = partition;

	if (partition->begin_enum)
	{
		err = partition->begin_enum(enumeration, path);
		if (err)
		{
			err = markerrorsource(err);
			goto done;
		}
	}

done:
	if (alloc_path != nullptr)
		free(alloc_path);
	if (new_path != nullptr)
		osd_free(new_path);
	if (err && (enumeration != nullptr))
	{
		free(enumeration);
		enumeration = nullptr;
	}
	*outenum = enumeration;
	return err;
}



/*-------------------------------------------------
    imgtool_directory_close - closes a directory
-------------------------------------------------*/

void imgtool_directory_close(imgtool_directory *directory)
{
	imgtool_partition *partition;
	partition = imgtool_directory_partition(directory);
	if (partition->close_enum)
		partition->close_enum(directory);
	free(directory);
}



/*-------------------------------------------------
    imgtool_directory_get_next - continues
    enumerating files within a partition
-------------------------------------------------*/

imgtoolerr_t imgtool_directory_get_next(imgtool_directory *directory, imgtool_dirent *ent)
{
	imgtoolerr_t err;
	imgtool_partition *partition;
	int charset;

	partition = imgtool_directory_partition(directory);

	/* This makes it so that drivers don't have to take care of clearing
	 * the attributes if they don't apply
	 */
	memset(ent, 0, sizeof(*ent));

	err = partition->next_enum(directory, ent);
	if (err)
		return markerrorsource(err);

	charset = imgtool_partition_get_info_int(partition, IMGTOOLINFO_INT_CHARSET);

	if (charset)
	{
		char *new_fname = utf8_from_native((imgtool_charset)charset, ent->filename);

		if (!new_fname)
			return IMGTOOLERR_BADFILENAME;

		strcpy(ent->filename, new_fname);
		osd_free(new_fname);
	}

	/* don't trust the module! */
	if (!partition->supports_creation_time && (ent->creation_time != 0))
	{
		internal_error(nullptr, "next_enum() specified creation_time, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	if (!partition->supports_lastmodified_time && (ent->lastmodified_time != 0))
	{
		internal_error(nullptr, "next_enum() specified lastmodified_time, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	if (!partition->path_separator && ent->directory)
	{
		internal_error(nullptr, "next_enum() returned a directory, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	return IMGTOOLERR_SUCCESS;
}



/*-------------------------------------------------
    imgtool_directory_module - returns the module associated
    with this directory
-------------------------------------------------*/

const imgtool_module *imgtool_directory_module(imgtool_directory *directory)
{
	return directory->partition->image->module;
}



/*-------------------------------------------------
    imgtool_directory_extrabytes - returns extra
    bytes on a directory
-------------------------------------------------*/

void *imgtool_directory_extrabytes(imgtool_directory *directory)
{
	assert(directory->partition->directory_extra_bytes > 0);
	return ((UINT8 *) directory) + sizeof(*directory);
}



/*-------------------------------------------------
    imgtool_directory_partition - returns the
    partition associated with this directory
-------------------------------------------------*/

imgtool_partition *imgtool_directory_partition(imgtool_directory *directory)
{
	return directory->partition;
}



/*-------------------------------------------------
    imgtool_directory_image - returns the image
    associated with this directory
-------------------------------------------------*/

imgtool_image *imgtool_directory_image(imgtool_directory *directory)
{
	return directory->partition->image;
}



/*-------------------------------------------------
    unknown_partition_get_info - represents an
    unknown partition
-------------------------------------------------*/

void unknown_partition_get_info(const imgtool_class *imgclass, UINT32 state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "unknown"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Unknown partition type"); break;
	}
}
