// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    library.h

    Code relevant to the Imgtool library; analgous to the MESS/MAME driver
    list.

    Unlike MESS and MAME which have static driver lists, Imgtool has a
    concept of a library and this library is built at startup time.
    dynamic for which modules are added to.  This makes "dynamic" modules
    much easier

****************************************************************************/

#ifndef LIBRARY_H
#define LIBRARY_H

#include <time.h>

#include "corestr.h"
#include "opresolv.h"
#include "stream.h"
#include "unicode.h"
#include "charconv.h"


struct imgtool_image;
struct imgtool_partition;
struct imgtool_directory;
struct imgtool_library;

enum imgtool_suggestion_viability_t
{
	SUGGESTION_END,
	SUGGESTION_POSSIBLE,
	SUGGESTION_RECOMMENDED
};

union filterinfo
{
	INT64   i;                                          /* generic integers */
	void *  p;                                          /* generic pointers */
	void *  f;                                          /* generic function pointers */
	const char *s;                                      /* generic strings */

	imgtoolerr_t (*read_file)(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf);
	imgtoolerr_t (*write_file)(imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts);
	imgtoolerr_t (*check_stream)(imgtool_stream *stream, imgtool_suggestion_viability_t *viability);
};

typedef void (*filter_getinfoproc)(UINT32 state, union filterinfo *info);

enum imgtool_libsort_t
{
	ITLS_NAME,
	ITLS_DESCRIPTION
};

struct imgtool_dirent
{
	char filename[1024];
	char attr[64];
	UINT64 filesize;

	time_t creation_time;
	time_t lastmodified_time;
	time_t lastaccess_time;

	char softlink[1024];
	char comment[256];

	/* flags */
	unsigned int eof : 1;
	unsigned int corrupt : 1;
	unsigned int directory : 1;
	unsigned int hardlink : 1;
};

struct imgtool_chainent
{
	UINT8 level;
	UINT64 block;
};

enum imgtool_forktype_t
{
	FORK_END,
	FORK_DATA,
	FORK_RESOURCE,
	FORK_ALTERNATE
};

struct imgtool_forkent
{
	imgtool_forktype_t type;
	UINT64 size;
	char forkname[64];
};

struct imgtool_transfer_suggestion
{
	imgtool_suggestion_viability_t viability;
	filter_getinfoproc filter;
	const char *fork;
	const char *description;
};

enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	IMGTOOLATTR_INT_FIRST = 0x00000,
	IMGTOOLATTR_INT_MAC_TYPE,
	IMGTOOLATTR_INT_MAC_CREATOR,
	IMGTOOLATTR_INT_MAC_FINDERFLAGS,
	IMGTOOLATTR_INT_MAC_COORDX,
	IMGTOOLATTR_INT_MAC_COORDY,
	IMGTOOLATTR_INT_MAC_FINDERFOLDER,
	IMGTOOLATTR_INT_MAC_ICONID,
	IMGTOOLATTR_INT_MAC_SCRIPTCODE,
	IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS,
	IMGTOOLATTR_INT_MAC_COMMENTID,
	IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY,

	/* --- the following bits of info are returned as pointers to data or functions --- */
	IMGTOOLATTR_PTR_FIRST = 0x10000,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	IMGTOOLATTR_STR_FIRST = 0x20000,

	/* --- the following bits of info are returned as time_t values --- */
	IMGTOOLATTR_TIME_FIRST = 0x30000,
	IMGTOOLATTR_TIME_CREATED,
	IMGTOOLATTR_TIME_LASTMODIFIED
};

union imgtool_attribute
{
	INT64   i;
	time_t  t;
};

struct imgtool_iconinfo
{
	unsigned icon16x16_specified : 1;
	unsigned icon32x32_specified : 1;
	UINT32 icon16x16[16][16];
	UINT32 icon32x32[32][32];
};

enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	IMGTOOLINFO_INT_FIRST = 0x00000,
	IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES,
	IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES,
	IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES,
	IMGTOOLINFO_INT_PATH_SEPARATOR,
	IMGTOOLINFO_INT_ALTERNATE_PATH_SEPARATOR,
	IMGTOOLINFO_INT_PREFER_UCASE,
	IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR,
	IMGTOOLINFO_INT_OPEN_IS_STRICT,
	IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME,
	IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME,
	IMGTOOLINFO_INT_TRACKS_ARE_CALLED_CYLINDERS,
	IMGTOOLINFO_INT_WRITING_UNTESTED,
	IMGTOOLINFO_INT_CREATION_UNTESTED,
	IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK,
	IMGTOOLINFO_INT_BLOCK_SIZE,
	IMGTOOLINFO_INT_CHARSET,

	IMGTOOLINFO_INT_CLASS_SPECIFIC = 0x08000,

	/* --- the following bits of info are returned as pointers to data or functions --- */
	IMGTOOLINFO_PTR_FIRST = 0x10000,

	IMGTOOLINFO_PTR_OPEN,
	IMGTOOLINFO_PTR_CREATE,
	IMGTOOLINFO_PTR_CLOSE,
	IMGTOOLINFO_PTR_OPEN_PARTITION,
	IMGTOOLINFO_PTR_CREATE_PARTITION,
	IMGTOOLINFO_PTR_INFO,
	IMGTOOLINFO_PTR_BEGIN_ENUM,
	IMGTOOLINFO_PTR_NEXT_ENUM,
	IMGTOOLINFO_PTR_CLOSE_ENUM,
	IMGTOOLINFO_PTR_FREE_SPACE,
	IMGTOOLINFO_PTR_READ_FILE,
	IMGTOOLINFO_PTR_WRITE_FILE,
	IMGTOOLINFO_PTR_DELETE_FILE,
	IMGTOOLINFO_PTR_LIST_FORKS,
	IMGTOOLINFO_PTR_CREATE_DIR,
	IMGTOOLINFO_PTR_DELETE_DIR,
	IMGTOOLINFO_PTR_LIST_ATTRS,
	IMGTOOLINFO_PTR_GET_ATTRS,
	IMGTOOLINFO_PTR_SET_ATTRS,
	IMGTOOLINFO_PTR_ATTR_NAME,
	IMGTOOLINFO_PTR_GET_ICON_INFO,
	IMGTOOLINFO_PTR_SUGGEST_TRANSFER,
	IMGTOOLINFO_PTR_GET_CHAIN,
	IMGTOOLINFO_PTR_GET_SECTOR_SIZE,
	IMGTOOLINFO_PTR_GET_GEOMETRY,
	IMGTOOLINFO_PTR_READ_SECTOR,
	IMGTOOLINFO_PTR_WRITE_SECTOR,
	IMGTOOLINFO_PTR_READ_BLOCK,
	IMGTOOLINFO_PTR_WRITE_BLOCK,
	IMGTOOLINFO_PTR_APPROVE_FILENAME_CHAR,
	IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE,
	IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE,
	IMGTOOLINFO_PTR_MAKE_CLASS,
	IMGTOOLINFO_PTR_LIST_PARTITIONS,

	IMGTOOLINFO_PTR_CLASS_SPECIFIC = 0x18000,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	IMGTOOLINFO_STR_FIRST = 0x20000,

	IMGTOOLINFO_STR_NAME,
	IMGTOOLINFO_STR_DESCRIPTION,
	IMGTOOLINFO_STR_FILE,
	IMGTOOLINFO_STR_FILE_EXTENSIONS,
	IMGTOOLINFO_STR_EOLN,
	IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC,
	IMGTOOLINFO_STR_WRITEFILE_OPTSPEC,

	IMGTOOLINFO_STR_CLASS_SPECIFIC = 0x28000
};



union imgtoolinfo;

struct imgtool_class;
typedef void (*imgtool_get_info)(const imgtool_class *, UINT32, union imgtoolinfo *);

struct imgtool_class
{
	imgtool_get_info get_info;
	imgtool_get_info derived_get_info;
	void *derived_param;
};



struct imgtool_partition_info
{
	imgtool_get_info get_info;
	UINT64 base_block;
	UINT64 block_count;
};



union imgtoolinfo
{
	INT64   i;                                          /* generic integers */
	void *  p;                                          /* generic pointers */
	void *  f;                                          /* generic function pointers */
	char *  s;                                          /* generic strings */

	imgtoolerr_t    (*open)             (imgtool_image *image, imgtool_stream *stream);
	void            (*close)            (imgtool_image *image);
	imgtoolerr_t    (*create)           (imgtool_image *image, imgtool_stream *stream, option_resolution *opts);
	imgtoolerr_t    (*create_partition) (imgtool_image *image, UINT64 first_block, UINT64 block_count);
	void            (*info)             (imgtool_image *image, char *string, size_t len);
	imgtoolerr_t    (*begin_enum)       (imgtool_directory *enumeration, const char *path);
	imgtoolerr_t    (*next_enum)        (imgtool_directory *enumeration, imgtool_dirent *ent);
	void            (*close_enum)       (imgtool_directory *enumeration);
	imgtoolerr_t    (*open_partition)   (imgtool_partition *partition, UINT64 first_block, UINT64 block_count);
	imgtoolerr_t    (*free_space)       (imgtool_partition *partition, UINT64 *size);
	imgtoolerr_t    (*read_file)        (imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *destf);
	imgtoolerr_t    (*write_file)       (imgtool_partition *partition, const char *filename, const char *fork, imgtool_stream *sourcef, option_resolution *opts);
	imgtoolerr_t    (*delete_file)      (imgtool_partition *partition, const char *filename);
	imgtoolerr_t    (*list_forks)       (imgtool_partition *partition, const char *path, imgtool_forkent *ents, size_t len);
	imgtoolerr_t    (*create_dir)       (imgtool_partition *partition, const char *path);
	imgtoolerr_t    (*delete_dir)       (imgtool_partition *partition, const char *path);
	imgtoolerr_t    (*list_attrs)       (imgtool_partition *partition, const char *path, UINT32 *attrs, size_t len);
	imgtoolerr_t    (*get_attrs)        (imgtool_partition *partition, const char *path, const UINT32 *attrs, imgtool_attribute *values);
	imgtoolerr_t    (*set_attrs)        (imgtool_partition *partition, const char *path, const UINT32 *attrs, const imgtool_attribute *values);
	imgtoolerr_t    (*attr_name)        (UINT32 attribute, const imgtool_attribute *attr, char *buffer, size_t buffer_len);
	imgtoolerr_t    (*get_iconinfo)     (imgtool_partition *partition, const char *path, imgtool_iconinfo *iconinfo);
	imgtoolerr_t    (*suggest_transfer) (imgtool_partition *partition, const char *path, imgtool_transfer_suggestion *suggestions, size_t suggestions_length);
	imgtoolerr_t    (*get_chain)        (imgtool_partition *partition, const char *path, imgtool_chainent *chain, size_t chain_size);
	imgtoolerr_t    (*get_sector_size)  (imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, UINT32 *sector_size);
	imgtoolerr_t    (*get_geometry)     (imgtool_image *image, UINT32 *tracks, UINT32 *heads, UINT32 *sectors);
	imgtoolerr_t    (*read_sector)      (imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, void *buffer, size_t len);
	imgtoolerr_t    (*write_sector)     (imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, const void *buffer, size_t len, int ddam);
	imgtoolerr_t    (*read_block)       (imgtool_image *image, void *buffer, UINT64 block);
	imgtoolerr_t    (*write_block)      (imgtool_image *image, const void *buffer, UINT64 block);
	imgtoolerr_t    (*list_partitions)  (imgtool_image *image, imgtool_partition_info *partitions, size_t len);
	int             (*approve_filename_char)(unicode_char ch);
	int             (*make_class)(int index, imgtool_class *imgclass);

	const option_guide *createimage_optguide;
	const option_guide *writefile_optguide;
};



INLINE INT64 imgtool_get_info_int(const imgtool_class *imgclass, UINT32 state)
{
	union imgtoolinfo info;
	info.i = 0;
	imgclass->get_info(imgclass, state, &info);
	return info.i;
}

INLINE void *imgtool_get_info_ptr(const imgtool_class *imgclass, UINT32 state)
{
	union imgtoolinfo info;
	info.p = NULL;
	imgclass->get_info(imgclass, state, &info);
	return info.p;
}

INLINE void *imgtool_get_info_fct(const imgtool_class *imgclass, UINT32 state)
{
	union imgtoolinfo info;
	info.f = NULL;
	imgclass->get_info(imgclass, state, &info);
	return info.f;
}

INLINE char *imgtool_get_info_string(const imgtool_class *imgclass, UINT32 state)
{
	union imgtoolinfo info;
	info.s = NULL;
	imgclass->get_info(imgclass, state, &info);
	return info.s;
}

/* circular string buffer */
char *imgtool_temp_str(void);



struct imgtool_module
{
	imgtool_module *previous;
	imgtool_module *next;

	imgtool_class imgclass;

	const char *name;
	const char *description;
	const char *extensions;
	const char *eoln;

	size_t image_extra_bytes;

	/* flags */
	unsigned int initial_path_separator : 1;
	unsigned int open_is_strict : 1;
	unsigned int tracks_are_called_cylinders : 1;   /* used for hard drivers */
	unsigned int writing_untested : 1;              /* used when we support writing, but not in main build */
	unsigned int creation_untested : 1;             /* used when we support creation, but not in main build */

	imgtoolerr_t    (*open)         (imgtool_image *image, imgtool_stream *f);
	void            (*close)        (imgtool_image *image);
	void            (*info)         (imgtool_image *image, char *string, size_t len);
	imgtoolerr_t    (*create)       (imgtool_image *image, imgtool_stream *f, option_resolution *opts);
	imgtoolerr_t    (*get_sector_size)(imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, UINT32 *sector_size);
	imgtoolerr_t    (*get_geometry) (imgtool_image *image, UINT32 *track, UINT32 *heads, UINT32 *sectors);
	imgtoolerr_t    (*read_sector)  (imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, void *buffer, size_t len);
	imgtoolerr_t    (*write_sector) (imgtool_image *image, UINT32 track, UINT32 head, UINT32 sector, const void *buffer, size_t len);
	imgtoolerr_t    (*read_block)   (imgtool_image *image, void *buffer, UINT64 block);
	imgtoolerr_t    (*write_block)  (imgtool_image *image, const void *buffer, UINT64 block);
	imgtoolerr_t    (*list_partitions)(imgtool_image *image, imgtool_partition_info *partitions, size_t len);

	UINT32 block_size;

	const option_guide *createimage_optguide;
	const char *createimage_optspec;

	const void *extra;
};

/* creates an imgtool library */
imgtool_library *imgtool_library_create(void);

/* closes an imgtool library */
void imgtool_library_close(imgtool_library *library);

/* adds a module to an imgtool library */
void imgtool_library_add(imgtool_library *library, imgtool_get_info get_info);

/* seeks out and removes a module from an imgtool library */
const imgtool_module *imgtool_library_unlink(imgtool_library *library,
	const char *module);

/* sorts an imgtool library */
void imgtool_library_sort(imgtool_library *library, imgtool_libsort_t sort);

/* finds a module */
const imgtool_module *imgtool_library_findmodule(
	imgtool_library *library, const char *module_name);

/* memory allocators for pooled library memory */
void *imgtool_library_malloc(imgtool_library *library, size_t mem);
char *imgtool_library_strdup(imgtool_library *library, const char *s);
char *imgtool_library_strdup_allow_null(imgtool_library *library, const char *s);

imgtool_module *imgtool_library_iterate(
	imgtool_library *library, const imgtool_module *module);
imgtool_module *imgtool_library_index(
	imgtool_library *library, int i);

#endif /* LIBRARY_H */
