/*********************************************************************

    flopimg.h

    Floppy disk image abstraction code

*********************************************************************/

#ifndef FLOPIMG_H
#define FLOPIMG_H

#include "osdcore.h"
#include "ioprocs.h"
#include "opresolv.h"

#ifndef LOG_FORMATS
#define LOG_FORMATS if (0) printf
#endif


/***************************************************************************

    Constants

***************************************************************************/

#define FLOPPY_FLAGS_READWRITE		0
#define FLOPPY_FLAGS_READONLY		1

/* sector has a deleted data address mark */
#define ID_FLAG_DELETED_DATA	0x0001
/* CRC error in id field */
#define ID_FLAG_CRC_ERROR_IN_ID_FIELD 0x0002
/* CRC error in data field */
#define ID_FLAG_CRC_ERROR_IN_DATA_FIELD 0x0004


/***************************************************************************

    Type definitions

***************************************************************************/

typedef enum
{
	FLOPPY_ERROR_SUCCESS,			/* no error */
	FLOPPY_ERROR_INTERNAL,			/* fatal internal error */
	FLOPPY_ERROR_UNSUPPORTED,		/* this operation is unsupported */
	FLOPPY_ERROR_OUTOFMEMORY,		/* ran out of memory */
	FLOPPY_ERROR_SEEKERROR,			/* attempted to seek to nonexistant location */
	FLOPPY_ERROR_INVALIDIMAGE,		/* this image in invalid */
	FLOPPY_ERROR_READONLY,			/* attempt to write to read-only image */
	FLOPPY_ERROR_NOSPACE,
	FLOPPY_ERROR_PARAMOUTOFRANGE,
	FLOPPY_ERROR_PARAMNOTSPECIFIED
}
floperr_t;

typedef struct _floppy_image floppy_image_legacy;

struct FloppyCallbacks
{
	floperr_t (*read_sector)(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
	floperr_t (*write_sector)(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
	floperr_t (*read_indexed_sector)(floppy_image_legacy *floppy, int head, int track, int sector_index, void *buffer, size_t buflen);
	floperr_t (*write_indexed_sector)(floppy_image_legacy *floppy, int head, int track, int sector_index, const void *buffer, size_t buflen, int ddam);
	floperr_t (*read_track)(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen);
	floperr_t (*write_track)(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen);
	floperr_t (*format_track)(floppy_image_legacy *floppy, int head, int track, option_resolution *params);
	floperr_t (*post_format)(floppy_image_legacy *floppy, option_resolution *params);
	int (*get_heads_per_disk)(floppy_image_legacy *floppy);
	int (*get_tracks_per_disk)(floppy_image_legacy *floppy);
	int (*get_sectors_per_track)(floppy_image_legacy *floppy, int head, int track);
	UINT32 (*get_track_size)(floppy_image_legacy *floppy, int head, int track);
	floperr_t (*get_sector_length)(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length);
	floperr_t (*get_indexed_sector_info)(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags);
	floperr_t (*get_track_data_offset)(floppy_image_legacy *floppy, int head, int track, UINT64 *offset);
};



struct FloppyFormat
{
	const char *name;
	const char *extensions;
	const char *description;
	floperr_t (*identify)(floppy_image_legacy *floppy, const struct FloppyFormat *format, int *vote);
	floperr_t (*construct)(floppy_image_legacy *floppy, const struct FloppyFormat *format, option_resolution *params);
	floperr_t (*destruct)(floppy_image_legacy *floppy, const struct FloppyFormat *format);
	const char *param_guidelines;
};

#define FLOPPY_IDENTIFY(name)	floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, int *vote)
#define FLOPPY_CONSTRUCT(name)	floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, option_resolution *params)
#define FLOPPY_DESTRUCT(name)	floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format)

FLOPPY_IDENTIFY(td0_dsk_identify);
FLOPPY_CONSTRUCT(td0_dsk_construct);
FLOPPY_DESTRUCT(td0_dsk_destruct);

FLOPPY_IDENTIFY(imd_dsk_identify);
FLOPPY_CONSTRUCT(imd_dsk_construct);

FLOPPY_IDENTIFY(cqm_dsk_identify);
FLOPPY_CONSTRUCT(cqm_dsk_construct);

FLOPPY_IDENTIFY(dsk_dsk_identify);
FLOPPY_CONSTRUCT(dsk_dsk_construct);

FLOPPY_IDENTIFY(d88_dsk_identify);
FLOPPY_CONSTRUCT(d88_dsk_construct);

FLOPPY_IDENTIFY(fdi_dsk_identify);
FLOPPY_CONSTRUCT(fdi_dsk_construct);

#define LEGACY_FLOPPY_OPTIONS_NAME(name)	floppyoptions_##name

#define LEGACY_FLOPPY_OPTIONS_START(name)												\
	const struct FloppyFormat floppyoptions_##name[] =								\
	{																			\

#define LEGACY_FLOPPY_OPTIONS_END0 \
		{ NULL }							\
	};

#define LEGACY_FLOPPY_OPTIONS_EXTERN(name)												\
	extern const struct FloppyFormat floppyoptions_##name[]							\

#define LEGACY_FLOPPY_OPTION(name, extensions_, description_, identify_, construct_, destruct_, ranges_)\
	{ #name, extensions_, description_, identify_, construct_, destruct_, ranges_ },				\

#define LEGACY_FLOPPY_OPTIONS_END														\
		LEGACY_FLOPPY_OPTION( fdi, "fdi", "Formatted Disk Image", fdi_dsk_identify, fdi_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( td0, "td0", "Teledisk floppy disk image",	td0_dsk_identify, td0_dsk_construct, td0_dsk_destruct, NULL) \
		LEGACY_FLOPPY_OPTION( imd, "imd", "IMD floppy disk image",	imd_dsk_identify, imd_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( cqm, "cqm,dsk", "CopyQM floppy disk image",	cqm_dsk_identify, cqm_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( dsk, "dsk", "DSK floppy disk image",	dsk_dsk_identify, dsk_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( d88, "d77,d88,1dd", "D88 Floppy Disk image", d88_dsk_identify, d88_dsk_construct, NULL, NULL) \
	LEGACY_FLOPPY_OPTIONS_END0

LEGACY_FLOPPY_OPTIONS_EXTERN(default);

#define PARAM_END				'\0'
#define PARAM_HEADS				'H'
#define PARAM_TRACKS			'T'
#define PARAM_SECTORS			'S'
#define PARAM_SECTOR_LENGTH		'L'
#define PARAM_INTERLEAVE		'I'
#define PARAM_FIRST_SECTOR_ID	'F'

#define HEADS(range)			"H" #range
#define TRACKS(range)			"T" #range
#define SECTORS(range)			"S" #range
#define SECTOR_LENGTH(range)	"L" #range
#define INTERLEAVE(range)		"I" #range
#define FIRST_SECTOR_ID(range)	"F" #range


/***************************************************************************

    Prototypes

***************************************************************************/

OPTION_GUIDE_EXTERN(floppy_option_guide);

/* opening, closing and creating of floppy images */
floperr_t floppy_open(void *fp, const struct io_procs *procs, const char *extension, const struct FloppyFormat *format, int flags, floppy_image_legacy **outfloppy);
floperr_t floppy_open_choices(void *fp, const struct io_procs *procs, const char *extension, const struct FloppyFormat *formats, int flags, floppy_image_legacy **outfloppy);
floperr_t floppy_create(void *fp, const struct io_procs *procs, const struct FloppyFormat *format, option_resolution *parameters, floppy_image_legacy **outfloppy);
void floppy_close(floppy_image_legacy *floppy);

/* useful for identifying a floppy image */
floperr_t floppy_identify(void *fp, const struct io_procs *procs, const char *extension,
	const struct FloppyFormat *formats, int *identified_format);

/* functions useful within format constructors */
void *floppy_tag(floppy_image_legacy *floppy);
void *floppy_create_tag(floppy_image_legacy *floppy, size_t tagsize);
struct FloppyCallbacks *floppy_callbacks(floppy_image_legacy *floppy);
UINT8 floppy_get_filler(floppy_image_legacy *floppy);
void floppy_set_filler(floppy_image_legacy *floppy, UINT8 filler);

/* calls for accessing disk image data */
floperr_t floppy_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, void *buffer, size_t buffer_len);
floperr_t floppy_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, const void *buffer, size_t buffer_len, int ddam);
floperr_t floppy_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, void *buffer, size_t buffer_len);
floperr_t floppy_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, const void *buffer, size_t buffer_len, int ddam);
floperr_t floppy_read_track(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len);
floperr_t floppy_write_track(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len);
floperr_t floppy_read_track_data(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len);
floperr_t floppy_write_track_data(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len);
floperr_t floppy_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *params);
int floppy_get_tracks_per_disk(floppy_image_legacy *floppy);
int floppy_get_heads_per_disk(floppy_image_legacy *floppy);
UINT32 floppy_get_track_size(floppy_image_legacy *floppy, int head, int track);
floperr_t floppy_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length);
floperr_t floppy_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags);
floperr_t floppy_get_sector_count(floppy_image_legacy *floppy, int head, int track, int *sector_count);
floperr_t floppy_load_track(floppy_image_legacy *floppy, int head, int track, int dirtify, void **track_data, size_t *track_length);
int floppy_is_read_only(floppy_image_legacy *floppy);
UINT8 floppy_random_byte(floppy_image_legacy *floppy);

/* accessors for meta information about the image */
const char *floppy_format_description(floppy_image_legacy *floppy);

/* calls for accessing the raw disk image */
void floppy_image_read(floppy_image_legacy *floppy, void *buffer, UINT64 offset, size_t length);
void floppy_image_write(floppy_image_legacy *floppy, const void *buffer, UINT64 offset, size_t length);
void floppy_image_write_filler(floppy_image_legacy *floppy, UINT8 filler, UINT64 offset, size_t length);
UINT64 floppy_image_size(floppy_image_legacy *floppy);

/* misc */
const char *floppy_error(floperr_t err);

//////////////////////////////////////////////////////////
/// New implementation
//////////////////////////////////////////////////////////

class floppy_image;

class floppy_image_format_t
{
public:
	floppy_image_format_t(const char *name,const char *extensions,const char *description,const char *param_guidelines);
	virtual ~floppy_image_format_t();

	virtual int identify(floppy_image *image) = 0;
	virtual bool load(floppy_image *image) = 0;
protected:
	const char *m_name;
	const char *m_extensions;
	const char *m_description;
	const char *m_param_guidelines;
};


// a device_type is simply a pointer to its alloc function
typedef floppy_image_format_t *(*floppy_format_type)(const char *name,const char *extensions,const char *description,const char *param_guidelines);

// this template function creates a stub which constructs a image format
template<class _FormatClass>
floppy_image_format_t *floppy_image_format_creator(const char *name,const char *extensions,const char *description,const char *param_guidelines)
{
	return new _FormatClass(name, extensions, description, param_guidelines);
}

struct floppy_format_def
{
	const char *name;
	const char *extensions;
	const char *description;
	const floppy_format_type type;
	const char *param_guidelines;
};

#define FLOPPY_OPTIONS_NAME(name)	floppyoptions_##name

#define FLOPPY_OPTIONS_START(name)												\
	const struct floppy_format_def floppyoptions_##name[] =								\
	{																			\

#define FLOPPY_OPTIONS_END0 \
		{ NULL, NULL, NULL, NULL, NULL }							\
	};

#define FLOPPY_OPTIONS_EXTERN(name)												\
	extern const struct floppy_format_def floppyoptions_##name[]							\

#define FLOPPY_OPTION(name, extensions_, description_, type_, ranges_)\
	{ #name, extensions_, description_, type_, ranges_ },				\

#define FLOPPY_OPTIONS_END													\
	FLOPPY_OPTIONS_END0

// ======================> floppy_image

#define MAX_FLOPPY_SIDES   2
#define MAX_FLOPPY_TRACKS  84
#define MAX_TRACK_DATA     16384

// class representing floppy image
class floppy_image
{
public:
	// construction/destruction
	floppy_image(void *fp, const struct io_procs *procs,const struct floppy_format_def *formats);
	virtual ~floppy_image();

	void image_read(void *buffer, UINT64 offset, size_t length);
	void image_write(const void *buffer, UINT64 offset, size_t length);
	void image_write_filler(UINT8 filler, UINT64 offset, size_t length);
	UINT64 image_size();
	void close();

	void set_meta_data(UINT16 tracks, UINT8 sides, UINT16 rpm, UINT16 bitrate);
	void set_track_size(UINT16 track, UINT8 side, UINT16 size) { m_track_size[(track << 1) + side] = size; }
	const struct floppy_format_def *identify(int *best);
	UINT8* get_buffer(UINT16 track, UINT8 side) { return m_native_data[(track << 1) + side]; }
	UINT16 get_track_size(UINT16 track, UINT8 side) { return m_track_size[(track << 1) + side]; }
	bool load(int num);
private:
	void close_internal(bool close_file);
	struct io_generic		m_io;
	const struct floppy_format_def *m_formats;
	UINT16 m_tracks;
	UINT8  m_sides;
	UINT16 m_rpm;
	UINT16 m_bitrate;

	UINT8 m_native_data[MAX_FLOPPY_SIDES * MAX_FLOPPY_TRACKS][MAX_TRACK_DATA];
	UINT16 m_track_size[MAX_FLOPPY_SIDES * MAX_FLOPPY_TRACKS];
};

#endif /* FLOPIMG_H */

