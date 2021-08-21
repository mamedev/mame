// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg.h

    Floppy disk image abstraction code

*********************************************************************/
#ifndef MAME_FORMATS_FLOPIMG_H
#define MAME_FORMATS_FLOPIMG_H

#pragma once

#include "coretmpl.h"
#include "opresolv.h"
#include "utilfwd.h"

#include "osdcore.h"

#include <memory>
#include <vector>

#ifndef LOG_FORMATS
#define LOG_FORMATS if (0) printf
#endif


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

OPTION_GUIDE_EXTERN(floppy_option_guide);

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


//////////////////////////////////////////////////////////
// New implementation
//////////////////////////////////////////////////////////

class floppy_image;

//! Class representing a floppy image format.
class floppy_image_format_t
{
public:
	virtual ~floppy_image_format_t() = default;

	/*! @brief Identify an image.
	  The identify function tests if the image is valid
	  for this particular format.
	  @param io buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @param variants the variants from floppy_image the drive can handle
	  @return 1 if image valid, 0 otherwise.
	*/
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) = 0;

	/*! @brief Load an image.
	  The load function opens an image file and converts it to the
	  internal MESS floppy representation.
	  @param io source buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @param variants the variants from floppy_image the drive can handle
	  @param image output buffer for data in MESS internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) = 0;

	/*! @brief Save an image.
	  The save function writes back an image from the MESS internal
	  floppy representation to the appropriate format on disk.
	  @param io output buffer for the data in the on-disk format.
	  @param variants the variants from floppy_image the drive can handle
	  @param image source buffer containing data in MESS internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image);

	//! @returns string containing name of format.
	virtual const char *name() const = 0;
	//! @returns string containing description of format.
	virtual const char *description() const = 0;
	//! @returns string containing comma-separated list of file
	//! extensions the format may use.
	virtual const char *extensions() const = 0;
	//! @returns true if format supports saving.
	virtual bool supports_save() const = 0;

	//! This checks if the file has the proper extension for this format.
	//! @param file_name
	//! @returns true if file matches the extension.
	bool extension_matches(const char *file_name) const;

protected:
	//! Input for convert_to_edge
	enum {
		MG_SHIFT  = 28,

		MG_0      = (4 << MG_SHIFT),
		MG_1      = (5 << MG_SHIFT),
		MG_W      = (6 << MG_SHIFT)
	};

	// **** Reader helpers ****

	//! Struct designed for easy track data description. Contains an opcode and two params.

	//! Optional, you can always do things by hand, but useful nevertheless.
	//! A vector of these structures describes one track.

	struct desc_e
	{
		int type,   //!< An opcode
			p1,     //!< first param
			p2;     //!< second param
	};

	//! Opcodes of the format description language used by generate_track()
	enum {
		END,                    //!< End of description
		FM,                     //!< One byte in p1 to be fm-encoded, msb first, repeated p2 times
		MFM,                    //!< One byte in p1 to be mfm-encoded, msb first, repeated p2 times
		MFMBITS,                //!< A value of p2 bits in p1 to be mfm-encoded, msb first
		GCR5,                   //!< One byte in p1 to be gcr5-encoded, repeated p2 times
		_8N1,                   //!< One byte in p1 to be 8N1-encoded, repeated p2 times
		RAW,                    //!< One 16 bits word in p1 to be written raw, msb first, repeated p2 times
		RAWBYTE,                //!< One 8 bit byte in p1 to be written raw, msb first, repeated p2 times
		RAWBITS,                //!< A value of p2 bits in p1 to be copied as-is, msb first
		SYNC_GCR5,              //!< gcr5 sync byte
		TRACK_ID,               //!< Track id byte, mfm-encoded
		TRACK_ID_FM,            //!< Track id byte, fm-encoded
		TRACK_ID_DOS2_GCR5,     //!< Track id byte, gcr5-encoded
		TRACK_ID_DOS25_GCR5,    //!< Track id byte, gcr5-encoded
		TRACK_ID_GCR6,          //!< Track id low 6 bits, gcr6-encoded
		TRACK_ID_8N1,           //!< Track id byte, 8N1-encoded
		TRACK_ID_VICTOR_GCR5,   //!< Track id byte, gcr5-encoded
		HEAD_ID,                //!< Head id byte, mfm-encoded
		HEAD_ID_FM,             //!< Head id byte, fm-encoded
		HEAD_ID_SWAP,           //!< Head id byte swapped (0->1, 1->0), mfm-encoded
		TRACK_HEAD_ID_GCR6,     //!< Track id 7th bit + head, gc6-encoded
		SECTOR_ID,              //!< Sector id byte, mfm-encoded
		SECTOR_ID_FM,           //!< Sector id byte, fm-encoded
		SECTOR_ID_GCR5,         //!< Sector id byte, gcr5-encoded
		SECTOR_ID_GCR6,         //!< Sector id byte, gcr6-encoded
		SECTOR_ID_8N1,          //!< Sector id byte, 8N1-encoded
		SIZE_ID,                //!< Sector size code on one byte [log2(size/128)], mfm-encoded
		SIZE_ID_FM,             //!< Sector size code on one byte [log2(size/128)], fm-encoded
		SECTOR_INFO_GCR6,       //!< Sector info byte, gcr6-encoded
		OFFSET_ID_O,            //!< Offset (track*2+head) byte, odd bits, mfm-encoded
		OFFSET_ID_E,            //!< Offset (track*2+head) byte, even bits, mfm-encoded
		OFFSET_ID_FM,           //!< Offset (track*2+head) byte, fm-encoded
		OFFSET_ID,              //!< Offset (track*2+head) byte, mfm-encoded
		SECTOR_ID_O,            //!< Sector id byte, odd bits, mfm-encoded
		SECTOR_ID_E,            //!< Sector id byte, even bits, mfm-encoded
		REMAIN_O,               //!< Remaining sector count, odd bits, mfm-encoded, total sector count in p1
		REMAIN_E,               //!< Remaining sector count, even bits, mfm-encoded, total sector count in p1

		SECTOR_DATA,            //!< Sector data to mfm-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_FM,         //!< Sector data to fm-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_O,          //!< Sector data to mfm-encode, odd bits only, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_E,          //!< Sector data to mfm-encode, even bits only, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_GCR5,       //!< Sector data to gcr5-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_MAC,        //!< Transformed sector data + checksum, mac style, id in p1, -1 for the current one per the sector id
		SECTOR_DATA_8N1,        //!< Sector data to 8N1-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_MX,         //!< Sector data to MX-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_DS9,        //!< Sector data to DS9-encode, which in p1, -1 for the current one per the sector id

		CRC_CCITT_START,        //!< Start a CCITT CRC calculation, with the usual x^16 + x^12 + x^5 + 1 (11021) polynomial, p1 = crc id
		CRC_CCITT_FM_START,     //!< Start a CCITT CRC calculation, with the usual x^16 + x^12 + x^5 + 1 (11021) polynomial, p1 = crc id
		CRC_AMIGA_START,        //!< Start an amiga checksum calculation, p1 = crc id
		CRC_CBM_START,          //!< Start a CBM checksum calculation (xor of original data values, gcr5-encoded), p1 = crc id
		CRC_MACHEAD_START,      //!< Start of the mac gcr6 sector header checksum calculation (xor of pre-encode 6-bits values, gcr6-encoded)
		CRC_FCS_START,          //!< Start a Compucolor File Control System checksum calculation, p1 = crc id
		CRC_VICTOR_HDR_START,   //!< Start a Victor 9000 checksum calculation, p1 = crc id
		CRC_VICTOR_DATA_START,  //!< Start a Victor 9000 checksum calculation, p1 = crc id
		CRC_END,                //!< End the checksum, p1 = crc id
		CRC,                    //!< Write a checksum in the appropriate format, p1 = crc id

		SECTOR_LOOP_START,      //!< Start of the per-sector loop, sector number goes from p1 to p2 inclusive
		SECTOR_LOOP_END,        //!< End of the per-sector loop
		SECTOR_INTERLEAVE_SKEW  //!< Defines interleave and skew for sector counting
	};


	/*! @brief Test if a variant is present in the variant vector
	    @param variants the variant vector
	    @param variant the variant to test
	    @result true if variant is in variants
	*/
	static bool has_variant(const std::vector<uint32_t> &variants, uint32_t variant);

	//! Sector data description
	struct desc_s
	{
		int size;          //!< Sector size, int bytes
		const uint8_t *data; //!< Sector data
		uint8_t sector_id;   //!< Sector ID
		uint8_t sector_info; //!< Sector free byte
	};


	/*! @brief Generate one track according to the description vector.
	    @param desc track data description
	    @param track
	    @param head
	    @param sect a vector indexed by sector id.
	    @param sect_count number of sectors.
	    @param track_size in _cells_, i.e. 100000 for a usual 2us-per-cell track at 300rpm.
	    @param image
	*/
	static void generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image *image);

	/*! @brief Generate a track from cell binary values, MSB-first.
	    @param track
	    @param head
	    @param trackbuf track input buffer.
	    @param track_size in cells, not bytes.
	    @param image
	    @param subtrack subtrack index, 0-3
	    @param splice write splice position
	*/
	static void generate_track_from_bitstream(int track, int head, const uint8_t *trackbuf, int track_size, floppy_image *image, int subtrack = 0, int splice = 0);

	//! @brief Generate a track from cell level values (0/1/W/D/N).

	/*! Note that this function needs to be able to split cells in two,
	    so no time value should be less than 2, and even values are a
	    good idea.
	*/
	/*! @param track
	    @param head
	    @param trackbuf track input buffer.
	    @param track_size in cells, not bytes.
	    @param splice_pos is the position of the track splice.  For normal
	    formats, use -1.  For protected formats, you're supposed to
	    know. trackbuf may be modified at that position or after.
	    @param image
	*/
	static void generate_track_from_levels(int track, int head, std::vector<uint32_t> &trackbuf, int splice_pos, floppy_image *image);

	//! Normalize the times in a cell buffer to sum up to 200000000
	static void normalize_times(std::vector<uint32_t> &buffer);

	// Some conversion tables for gcr
	static const uint8_t gcr5fw_tb[0x10], gcr5bw_tb[0x20];
	static const uint8_t gcr6fw_tb[0x40], gcr6bw_tb[0x100];

	// Some useful descriptions shared by multiple formats

	// Atari ST formats (100K cells)
	//   Standard TOS 9 sectors-per-track format
	static const desc_e atari_st_9[];

	//   Usual 10 sectors-per-track format
	static const desc_e atari_st_10[];

	//   Fastcopy Pro optimized formats, with fake sector header for
	//   faster verify and skew/interleave where appropriate
	static const desc_e atari_st_fcp_9[];
	static const desc_e *const atari_st_fcp_10[10];
	static const desc_e atari_st_fcp_11[];

	static const desc_e atari_st_fcp_10_0[], atari_st_fcp_10_1[], atari_st_fcp_10_2[], atari_st_fcp_10_3[];
	static const desc_e atari_st_fcp_10_4[], atari_st_fcp_10_5[], atari_st_fcp_10_6[], atari_st_fcp_10_7[];
	static const desc_e atari_st_fcp_10_8[], atari_st_fcp_10_9[];

	static const desc_e *atari_st_fcp_get_desc(int track, int head, int head_count, int sect_count);

	// Amiga formats (100K cells)
	//   Standard 11 sectors per track format
	static const desc_e amiga_11[];

	//   Standard 22 sectors per track format (guessed, to check w.r.t the real thing)
	static const desc_e amiga_22[];


	// **** Writer helpers ****

	/*! @brief Rebuild a cell bitstream for a track.
	    Takes the cell standard
	    angular size as a parameter, gives out a msb-first bitstream.

	    Beware that fuzzy bits will always give out the same value.
	    @param track
	    @param head
	    @param cell_size
	    @param trackbuf Output buffer size should be 34% more than the nominal number
	    of cells (the dpll tolerates a cell size down to 75% of the
	    nominal one, with gives a cell count of 1/0.75=1.333... times
	    the nominal one).
	    @param track_size Output size is given in bits (cells).
	    @param image
	*/
	/*! @verbatim
	 Computing the standard angular size of a cell is
	 simple. Noting:
	   d = standard cell duration in microseconds
	   r = motor rotational speed in rpm
	 then:
	   a = r * d * 10 / 3.
	 Some values:
	   Type           Cell    RPM    Size

	 C1541 tr  1-17   3.25    300    3250
	 C1541 tr 18-24   3.50    300    3500
	 C1541 tr 25-30   3.75    300    3750
	 C1541 tr 31+     4.00    300    4000
	 8" DD            1       360    1200
	 5.25" SD         4       300    4000
	 5.25" DD         2       300    2000
	 5.25" HD         1       360    1200
	 3.5" SD          4       300    4000
	 3.5" DD          2       300    2000
	 3.5" HD          1       300    1000
	 3.5" ED          0.5     300     500
	 @endverbatim
	 */

	static std::vector<bool> generate_bitstream_from_track(int track, int head, int cell_size, floppy_image *image, int subtrack = 0);
	static std::vector<uint8_t> generate_nibbles_from_bitstream(const std::vector<bool> &bitstream);

	struct desc_pc_sector
	{
		uint8_t track, head, sector, size;
		int actual_size;
		uint8_t *data;
		bool deleted;
		bool bad_crc;
	};

	struct desc_gcr_sector
	{
		uint8_t track, head, sector, info;
		uint8_t *tag;
		uint8_t *data;
	};

	static int calc_default_pc_gap3_size(uint32_t form_factor, int sector_size);
	static void build_wd_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2);
	static void build_wd_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2=22);
	static void build_pc_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=40, int gap_1=26, int gap_2=11);
	static void build_pc_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=80, int gap_1=50, int gap_2=22);
	static void build_mac_track_gcr(int track, int head, floppy_image *image, const desc_gcr_sector *sects);

	//! @brief Extract standard sectors from a regenerated bitstream.
	//! Returns a vector of the vector contents, indexed by the sector id.  Missing sectors have size zero.

	//! PC-type sectors with MFM encoding, sector size can go from 128 bytes to 16K.
	static std::vector<std::vector<uint8_t>> extract_sectors_from_bitstream_mfm_pc(const std::vector<bool> &bitstream);

	//! PC-type sectors with FM encoding
	static std::vector<std::vector<uint8_t>> extract_sectors_from_bitstream_fm_pc(const std::vector<bool> &bitstream);

	//! Commodore type sectors with GCR5 encoding
	static std::vector<std::vector<uint8_t>> extract_sectors_from_bitstream_gcr5(const std::vector<bool> &bitstream, int head, int tracks);

	//! Victor 9000 type sectors with GCR5 encoding
	static std::vector<std::vector<uint8_t>> extract_sectors_from_bitstream_victor_gcr5(const std::vector<bool> &bitstream);

	//! Mac type sectors with GCR6 encoding
	static std::vector<std::vector<uint8_t>> extract_sectors_from_track_mac_gcr6(int head, int track, floppy_image *image);


	//! @brief Get a geometry (including sectors) from an image.

	//!   PC-type sectors with MFM encoding
	static void get_geometry_mfm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count);
	//!   PC-type sectors with FM encoding
	static void get_geometry_fm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count);


	//!  Regenerate the data for a full track.
	//!  PC-type sectors with MFM encoding and fixed-size.
	static void get_track_data_mfm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata);

	//!  Regenerate the data for a full track.
	//!  PC-type sectors with FM encoding and fixed-size.
	static void get_track_data_fm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata);

	//! Look up a bit in a level-type stream.
	static bool bit_r(const std::vector<uint32_t> &buffer, int offset);
	//! Look up multiple bits
	static uint32_t bitn_r(const std::vector<uint32_t> &buffer, int offset, int count);
	//! Write a bit with a given size.
	static void bit_w(std::vector<uint32_t> &buffer, bool val, uint32_t size = 1000);
	static void bit_w(std::vector<uint32_t> &buffer, bool val, uint32_t size, int offset);
	//! Calculate a CCITT-type CRC.
	static uint16_t calc_crc_ccitt(const std::vector<uint32_t> &buffer, int start, int end);
	//! Write a series of (raw) bits
	static void raw_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size = 1000);
	static void raw_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset);
	//! FM-encode and write a series of bits
	static void fm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size = 1000);
	static void fm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset);
	//! MFM-encode and write a series of bits
	static void mfm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size = 1000);
	static void mfm_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size, int offset);
	//! MFM-encode every two bits and write
	static void mfm_half_w(std::vector<uint32_t> &buffer, int start_bit, uint32_t val, uint32_t size = 1000);
	//! GCR5-encode and write a series of bits
	static void gcr5_w(std::vector<uint32_t> &buffer, uint8_t val, uint32_t size = 1000);
	static void gcr5_w(std::vector<uint32_t> &buffer, uint8_t val, uint32_t size, int offset);
	//! 8N1-encode and write a series of bits
	static void _8n1_w(std::vector<uint32_t> &buffer, int n, uint32_t val, uint32_t size = 1000);
	//! GCR4 encode (Apple II sector header)
	static uint16_t gcr4_encode(uint8_t va);
	//! GCR4 decode
	static uint8_t gcr4_decode(uint8_t e0, uint8_t e1);
	//! GCR6 encode (Apple II 16-sector and Mac-style GCR)
	static uint32_t gcr6_encode(uint8_t va, uint8_t vb, uint8_t vc);
	//! GCR6 decode
	static void gcr6_decode(uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3, uint8_t &va, uint8_t &vb, uint8_t &vc);

	static uint8_t sbyte_mfm_r(const std::vector<bool> &bitstream, uint32_t &pos);
	static uint8_t sbyte_gcr5_r(const std::vector<bool> &bitstream, uint32_t &pos);

	//! Max number of excess tracks to be discarded from disk image to fit floppy drive
	enum { DUMP_THRESHOLD = 2 };

private:
	enum { CRC_NONE, CRC_AMIGA, CRC_CBM, CRC_CCITT, CRC_CCITT_FM, CRC_MACHEAD, CRC_FCS, CRC_VICTOR_HDR, CRC_VICTOR_DATA };
	enum { MAX_CRC_COUNT = 64 };

	//! Holds data used internally for generating CRCs.
	struct gen_crc_info
	{
		int type, //!< Type of CRC
			start, //!< Start position
			end, //!< End position
			write; //!< where to write the CRC
		bool fixup_mfm_clock; //!< would the MFM clock bit after the CRC need to be fixed?
	};

	static bool type_no_data(int type);
	static bool type_data_mfm(int type, int p1, const gen_crc_info *crcs);

	static int crc_cells_size(int type);
	static void fixup_crc_amiga(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_cbm(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_ccitt(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_ccitt_fm(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_machead(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_fcs(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_victor_header(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crc_victor_data(std::vector<uint32_t> &buffer, const gen_crc_info *crc);
	static void fixup_crcs(std::vector<uint32_t> &buffer, gen_crc_info *crcs);
	static void collect_crcs(const desc_e *desc, gen_crc_info *crcs);

	static int sbit_rp(const std::vector<bool> &bitstream, uint32_t &pos);

	static int calc_sector_index(int num, int interleave, int skew, int total_sectors, int track_head);
};

// a dce_type is simply a pointer to its alloc function
typedef floppy_image_format_t *(*floppy_format_type)();

// this template function creates a stub which constructs a image format
template<class _FormatClass>
floppy_image_format_t *floppy_image_format_creator()
{
	return new _FormatClass();
}

// ======================> floppy_image

//! Class representing floppy image

//! Internal format is close but not identical to the mfi format.
//!
//!
//! Track data consists of a series of 32-bits lsb-first values
//! representing magnetic cells.  Bits 0-27 indicate the absolute
//! position of the start of the cell (not the size), and bits
//! 28-31 the type.  Type can be:
//! - 0, MG_A -> Magnetic orientation A
//! - 1, MG_B -> Magnetic orientation B
//! - 2, MG_N -> Non-magnetized zone (neutral)
//! - 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
//!
//! The position is in angular units of 1/200,000,000th of a turn.
//! The last cell implicit end position is of course 200,000,000.
//!
//! Unformatted tracks are encoded as zero-size.
//!
//! The "track splice" information indicates where to start writing
//! if you try to rewrite a physical disk with the data.  Some
//! preservation formats encode that information, it is guessed for
//! others.  The write track function of fdcs should set it.  The
//! representation is the angular position relative to the index.
//!
//! The media type is divided in two parts.  The first half
//! indicates the physical form factor, i.e. all medias with that
//! form factor can be physically inserted in a reader that handles
//! it.  The second half indicates the variants which are usually
//! detectable by the reader, such as density and number of sides.
//!
//! Resolution is quarter-track.  The optional subtrack parameter is
//! 0-3:
//! - 0 = Track itself
//! - 1 = 1st quarter track
//! - 2 = Half track
//! - 3 = 2nd quarter track

class floppy_image
{
public:
	//! Floppy format data
	enum {
		TIME_MASK = 0x0fffffff,
		MG_MASK   = 0xf0000000,
		MG_SHIFT  = 28, //!< Bitshift constant for magnetic orientation data
		MG_A      = (0 << MG_SHIFT),    //!< - 0, MG_A -> Magnetic orientation A
		MG_B      = (1 << MG_SHIFT),    //!< - 1, MG_B -> Magnetic orientation B
		MG_N      = (2 << MG_SHIFT),    //!< - 2, MG_N -> Non-magnetized zone (neutral)
		MG_D      = (3 << MG_SHIFT)     //!< - 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
	};


	//! Form factors
	enum {
		FF_UNKNOWN  = 0x00000000, //!< Unknown, useful when converting
		FF_3        = 0x20202033, //!< "3   " 3 inch disk
		FF_35       = 0x20203533, //!< "35  " 3.5 inch disk
		FF_525      = 0x20353235, //!< "525 " 5.25 inch disk
		FF_8        = 0x20202038  //!< "8   " 8 inch disk
	};

	//! Variants
	enum {
		SSSD  = 0x44535353, //!< "SSSD", Single-sided single-density
		SSDD  = 0x44445353, //!< "SSDD", Single-sided double-density
		SSQD  = 0x44515353, //!< "SSQD", Single-sided quad-density
		DSSD  = 0x44535344, //!< "DSSD", Double-sided single-density
		DSDD  = 0x44445344, //!< "DSDD", Double-sided double-density (720K in 3.5, 360K in 5.25)
		DSQD  = 0x44515344, //!< "DSQD", Double-sided quad-density (720K in 5.25, means DD+80 tracks)
		DSHD  = 0x44485344, //!< "DSHD", Double-sided high-density (1440K)
		DSED  = 0x44455344  //!< "DSED", Double-sided extra-density (2880K)
	};

	//! Encodings
	enum {
		FM   = 0x2020464D, //!< "  FM", frequency modulation
		MFM  = 0x204D464D, //!< " MFM", modified frequency modulation
		M2FM = 0x4D32464D  //!< "M2FM", modified modified frequency modulation
	};

	// construction/destruction


	//! floppy_image constructor
	/*!
	  @param _tracks number of tracks.
	  @param _heads number of heads.
	  @param _form_factor form factor of drive (from enum)
	*/
	floppy_image(int tracks, int heads, uint32_t form_factor);
	virtual ~floppy_image();

	//! @return the form factor.
	uint32_t get_form_factor() const { return form_factor; }
	//! @return the variant.
	uint32_t get_variant() const { return variant; }
	//! @param v the variant.
	void set_variant(uint32_t v) { variant = v; }
	//! @param v the variant.
	void set_form_variant(uint32_t f, uint32_t v) { if(form_factor == FF_UNKNOWN) form_factor = f; variant = v; }

	/*!
	  @param track
	  @param subtrack
	  @param head head number
	  @return a pointer to the data buffer for this track and head
	*/
	std::vector<uint32_t> &get_buffer(int track, int head, int subtrack = 0) { assert(track < tracks && head < heads); return track_array[track*4+subtrack][head].cell_data; }

	//! Sets the write splice position.
	//! The "track splice" information indicates where to start writing
	//! if you try to rewrite a physical disk with the data.  Some
	//! preservation formats encode that information, it is guessed for
	//! others.  The write track function of fdcs should set it.  The
	//! representation is the angular position relative to the index.

	/*! @param track
	    @param subtrack
	    @param head
	    @param pos the position
	*/
	void set_write_splice_position(int track, int head, uint32_t pos, int subtrack = 0) { assert(track < tracks && head < heads); track_array[track*4+subtrack][head].write_splice = pos; }
	//! @return the current write splice position.
	uint32_t get_write_splice_position(int track, int head, int subtrack = 0) const { assert(track < tracks && head < heads); return track_array[track*4+subtrack][head].write_splice; }
	//! @return the maximal geometry supported by this format.
	void get_maximal_geometry(int &tracks, int &heads) const;

	//! @return the current geometry of the loaded image.
	void get_actual_geometry(int &tracks, int &heads);

	//! @return the track resolution (0=full track, 1 = half-track, 2 = quarter track)
	int get_resolution() const;

	//! @return whether a given track is formatted
	bool track_is_formatted(int track, int head, int subtrack = 0);

	//! Returns the variant name for the particular disk form factor/variant
	//! @param form_factor
	//! @param variant
	//! @param returns a string containing the variant name.
	static const char *get_variant_name(uint32_t form_factor, uint32_t variant);

private:
	int tracks, heads;

	uint32_t form_factor, variant;

	struct track_info
	{
		std::vector<uint32_t> cell_data;
		uint32_t write_splice;

		track_info() { write_splice = 0; }
	};

	// track number multiplied by 4 then head
	// last array size may be bigger than actual track size
	std::vector<std::vector<track_info> > track_array;
};

#endif // MAME_FORMATS_FLOPIMG_H
