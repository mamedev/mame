// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg_legacy.h

    Floppy disk image abstraction code (legacy implementation)

*********************************************************************/

#ifndef MAME_FORMATS_FLOPIMG_LEGACY_H
#define MAME_FORMATS_FLOPIMG_LEGACY_H

#pragma once

#include "utilfwd.h"

#include <memory>
#include <string>

#include <cstddef>
#include <cstdint>


/***************************************************************************

    Constants

***************************************************************************/

#define FLOPPY_FLAGS_READWRITE      0
#define FLOPPY_FLAGS_READONLY       1

/* sector has a deleted data address mark */
#define ID_FLAG_DELETED_DATA    0x0001
/* CRC error in id field */
#define ID_FLAG_CRC_ERROR_IN_ID_FIELD 0x0002
/* CRC error in data field */
#define ID_FLAG_CRC_ERROR_IN_DATA_FIELD 0x0004


/***************************************************************************

    Type definitions

***************************************************************************/

enum floperr_t
{
	FLOPPY_ERROR_SUCCESS,           /* no error */
	FLOPPY_ERROR_INTERNAL,          /* fatal internal error */
	FLOPPY_ERROR_UNSUPPORTED,       /* this operation is unsupported */
	FLOPPY_ERROR_OUTOFMEMORY,       /* ran out of memory */
	FLOPPY_ERROR_SEEKERROR,         /* attempted to seek to nonexistent location */
	FLOPPY_ERROR_INVALIDIMAGE,      /* this image in invalid */
	FLOPPY_ERROR_READONLY,          /* attempt to write to read-only image */
	FLOPPY_ERROR_NOSPACE,
	FLOPPY_ERROR_PARAMOUTOFRANGE,
	FLOPPY_ERROR_PARAMNOTSPECIFIED
};

struct floppy_image_legacy;

struct FloppyCallbacks
{
	floperr_t (*read_sector)(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
	floperr_t (*write_sector)(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
	floperr_t (*read_indexed_sector)(floppy_image_legacy *floppy, int head, int track, int sector_index, void *buffer, size_t buflen);
	floperr_t (*write_indexed_sector)(floppy_image_legacy *floppy, int head, int track, int sector_index, const void *buffer, size_t buflen, int ddam);
	floperr_t (*read_track)(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen);
	floperr_t (*write_track)(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buflen);
	floperr_t (*format_track)(floppy_image_legacy *floppy, int head, int track, util::option_resolution *params);
	floperr_t (*post_format)(floppy_image_legacy *floppy, util::option_resolution *params);
	int (*get_heads_per_disk)(floppy_image_legacy *floppy);
	int (*get_tracks_per_disk)(floppy_image_legacy *floppy);
	int (*get_sectors_per_track)(floppy_image_legacy *floppy, int head, int track);
	uint32_t (*get_track_size)(floppy_image_legacy *floppy, int head, int track);
	floperr_t (*get_sector_length)(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length);
	floperr_t (*get_indexed_sector_info)(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags);
	floperr_t (*get_track_data_offset)(floppy_image_legacy *floppy, int head, int track, uint64_t *offset);
};



struct FloppyFormat
{
	const char *name;
	const char *extensions;
	const char *description;
	floperr_t (*identify)(floppy_image_legacy *floppy, const struct FloppyFormat *format, int *vote);
	floperr_t (*construct)(floppy_image_legacy *floppy, const struct FloppyFormat *format, util::option_resolution *params);
	floperr_t (*destruct)(floppy_image_legacy *floppy, const struct FloppyFormat *format);
	const char *param_guidelines;
};

#define FLOPPY_IDENTIFY(name)   floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, int *vote)
#define FLOPPY_CONSTRUCT(name)  floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, util::option_resolution *params)
#define FLOPPY_DESTRUCT(name)   floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format)

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

#define LEGACY_FLOPPY_OPTIONS_NAME(name)    floppyoptions_##name

#define LEGACY_FLOPPY_OPTIONS_START(name)                                               \
	const struct FloppyFormat floppyoptions_##name[] =                              \
	{
#define LEGACY_FLOPPY_OPTIONS_END0 \
		{ nullptr }                            \
	};

#define LEGACY_FLOPPY_OPTIONS_EXTERN(name)                                              \
	extern const struct FloppyFormat floppyoptions_##name[]
#define LEGACY_FLOPPY_OPTION(name, extensions_, description_, identify_, construct_, destruct_, ranges_)\
	{ #name, extensions_, description_, identify_, construct_, destruct_, ranges_ },
#define LEGACY_FLOPPY_OPTIONS_END                                                       \
		LEGACY_FLOPPY_OPTION( fdi, "fdi", "Formatted Disk Image", fdi_dsk_identify, fdi_dsk_construct, nullptr, nullptr) \
		LEGACY_FLOPPY_OPTION( td0, "td0", "Teledisk floppy disk image", td0_dsk_identify, td0_dsk_construct, td0_dsk_destruct, nullptr) \
		LEGACY_FLOPPY_OPTION( imd, "imd", "IMD floppy disk image",  imd_dsk_identify, imd_dsk_construct, nullptr, nullptr) \
		LEGACY_FLOPPY_OPTION( cqm, "cqm,dsk", "CopyQM floppy disk image",   cqm_dsk_identify, cqm_dsk_construct, nullptr, nullptr) \
		LEGACY_FLOPPY_OPTION( dsk, "dsk", "DSK floppy disk image",  dsk_dsk_identify, dsk_dsk_construct, nullptr, nullptr) \
		LEGACY_FLOPPY_OPTION( d88, "d77,d88,1dd", "D88 Floppy Disk image", d88_dsk_identify, d88_dsk_construct, nullptr, nullptr) \
	LEGACY_FLOPPY_OPTIONS_END0

LEGACY_FLOPPY_OPTIONS_EXTERN(default);

#define PARAM_END               '\0'
#define PARAM_HEADS             'H'
#define PARAM_TRACKS            'T'
#define PARAM_SECTORS           'S'
#define PARAM_SECTOR_LENGTH     'L'
#define PARAM_INTERLEAVE        'I'
#define PARAM_FIRST_SECTOR_ID   'F'

#define HEADS(range)            "H" #range
#define TRACKS(range)           "T" #range
#define SECTORS(range)          "S" #range
#define SECTOR_LENGTH(range)    "L" #range
#define INTERLEAVE(range)       "I" #range
#define FIRST_SECTOR_ID(range)  "F" #range

/***************************************************************************

    Prototypes

***************************************************************************/

/* opening, closing and creating of floppy images */
floperr_t floppy_open(std::unique_ptr<util::random_read_write> &&io, const std::string &extension, const struct FloppyFormat *format, int flags, floppy_image_legacy **outfloppy);
floperr_t floppy_open_choices(std::unique_ptr<util::random_read_write> &&io, const std::string &extension, const struct FloppyFormat *formats, int flags, floppy_image_legacy **outfloppy);
floperr_t floppy_create(std::unique_ptr<util::random_read_write> &&io, const struct FloppyFormat *format, util::option_resolution *parameters, floppy_image_legacy **outfloppy);
void floppy_close(floppy_image_legacy *floppy);

/* useful for identifying a floppy image */
floperr_t floppy_identify(std::unique_ptr<util::random_read_write> &&io, const char *extension, const struct FloppyFormat *formats, int *identified_format);

/* functions useful within format constructors */
void *floppy_tag(floppy_image_legacy *floppy);
void *floppy_create_tag(floppy_image_legacy *floppy, size_t tagsize);
struct FloppyCallbacks *floppy_callbacks(floppy_image_legacy *floppy);
uint8_t floppy_get_filler(floppy_image_legacy *floppy);
util::random_read_write &floppy_get_io(floppy_image_legacy *floppy);

/* calls for accessing disk image data */
floperr_t floppy_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, void *buffer, size_t buffer_len);
floperr_t floppy_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int offset, const void *buffer, size_t buffer_len, int ddam);
floperr_t floppy_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, void *buffer, size_t buffer_len);
floperr_t floppy_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, int offset, const void *buffer, size_t buffer_len, int ddam);
floperr_t floppy_read_track(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len);
floperr_t floppy_write_track(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len);
floperr_t floppy_read_track_data(floppy_image_legacy *floppy, int head, int track, void *buffer, size_t buffer_len);
floperr_t floppy_write_track_data(floppy_image_legacy *floppy, int head, int track, const void *buffer, size_t buffer_len);
floperr_t floppy_format_track(floppy_image_legacy *floppy, int head, int track, util::option_resolution *params);
int floppy_get_tracks_per_disk(floppy_image_legacy *floppy);
int floppy_get_heads_per_disk(floppy_image_legacy *floppy);
uint32_t floppy_get_track_size(floppy_image_legacy *floppy, int head, int track);
floperr_t floppy_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length);
floperr_t floppy_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags);
floperr_t floppy_get_sector_count(floppy_image_legacy *floppy, int head, int track, int *sector_count);
floperr_t floppy_load_track(floppy_image_legacy *floppy, int head, int track, int dirtify, void **track_data, size_t *track_length);
int floppy_is_read_only(floppy_image_legacy *floppy);
uint8_t floppy_random_byte(floppy_image_legacy *floppy);

/* accessors for meta information about the image */
const char *floppy_format_description(floppy_image_legacy *floppy);

/* calls for accessing the raw disk image */
void floppy_image_read(floppy_image_legacy *floppy, void *buffer, uint64_t offset, size_t length);
void floppy_image_write(floppy_image_legacy *floppy, const void *buffer, uint64_t offset, size_t length);
void floppy_image_write_filler(floppy_image_legacy *floppy, uint8_t filler, uint64_t offset, size_t length);
uint64_t floppy_image_size(floppy_image_legacy *floppy);

/* misc */
const char *floppy_error(floperr_t err);

const util::option_guide &floppy_option_guide();

#endif // MAME_FORMATS_FLOPIMG_LEGACY_H
