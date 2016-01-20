// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg.h

    Floppy disk image abstraction code

*********************************************************************/

#ifndef FLOPIMG_H
#define FLOPIMG_H

#include "osdcore.h"
#include "ioprocs.h"
#include "opresolv.h"
#include "coretmpl.h"

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
	FLOPPY_ERROR_SEEKERROR,         /* attempted to seek to nonexistant location */
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

#define FLOPPY_IDENTIFY(name)   floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, int *vote)
#define FLOPPY_CONSTRUCT(name)  floperr_t name(floppy_image_legacy *floppy, const struct FloppyFormat *format, option_resolution *params)
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
		{ NULL }                            \
	};

#define LEGACY_FLOPPY_OPTIONS_EXTERN(name)                                              \
	extern const struct FloppyFormat floppyoptions_##name[]
#define LEGACY_FLOPPY_OPTION(name, extensions_, description_, identify_, construct_, destruct_, ranges_)\
	{ #name, extensions_, description_, identify_, construct_, destruct_, ranges_ },
#define LEGACY_FLOPPY_OPTIONS_END                                                       \
		LEGACY_FLOPPY_OPTION( fdi, "fdi", "Formatted Disk Image", fdi_dsk_identify, fdi_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( td0, "td0", "Teledisk floppy disk image", td0_dsk_identify, td0_dsk_construct, td0_dsk_destruct, NULL) \
		LEGACY_FLOPPY_OPTION( imd, "imd", "IMD floppy disk image",  imd_dsk_identify, imd_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( cqm, "cqm,dsk", "CopyQM floppy disk image",   cqm_dsk_identify, cqm_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( dsk, "dsk", "DSK floppy disk image",  dsk_dsk_identify, dsk_dsk_construct, NULL, NULL) \
		LEGACY_FLOPPY_OPTION( d88, "d77,d88,1dd", "D88 Floppy Disk image", d88_dsk_identify, d88_dsk_construct, NULL, NULL) \
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
// New implementation
//////////////////////////////////////////////////////////

class floppy_image;

//! Class representing a floppy image format.
class floppy_image_format_t
{
public:
	floppy_image_format_t();
	virtual ~floppy_image_format_t();

	/*! @brief Identify an image.
	  The identify function tests if the image is valid
	  for this particular format.
	  @param io buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @return 1 if image valid, 0 otherwise.
	*/
	virtual int identify(io_generic *io, UINT32 form_factor) = 0;

	/*! @brief Load an image.
	  The load function opens an image file and converts it to the
	  internal MESS floppy representation.
	  @param io source buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @param image output buffer for data in MESS internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) = 0;

	/*! @brief Save an image.
	  The save function writes back an image from the MESS internal
	  floppy representation to the appropriate format on disk.
	  @param io output buffer for the data in the on-disk format.
	  @param image source buffer containing data in MESS internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool save(io_generic *io, floppy_image *image);

	//! @returns string containing name of format.
	virtual const char *name() const = 0;
	//! @returns string containing description of format.
	virtual const char *description() const = 0;
	//! @returns string containing comma-separated list of file
	//! extensions the format may use.
	virtual const char *extensions() const = 0;
	//! @returns true if format supports saving.
	virtual bool supports_save() const = 0;

	//! Used if a linked list of formats is needed
	floppy_image_format_t *next;
	//! This appends a format to the linked list of formats, needed for floppy_image_device().
	void append(floppy_image_format_t *_next);
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

	struct desc_e {
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

		CRC_CCITT_START,        //!< Start a CCITT CRC calculation, with the usual x^16 + x^12 + x^5 + 1 (11021) polynomial, p1 = crc id
		CRC_CCITT_FM_START,     //!< Start a CCITT CRC calculation, with the usual x^16 + x^12 + x^5 + 1 (11021) polynomial, p1 = crc id
		CRC_AMIGA_START,        //!< Start an amiga checksum calculation, p1 = crc id
		CRC_CBM_START,          //!< Start a CBM checksum calculation (xor of original data values, gcr5-encoded), p1 = crc id
		CRC_MACHEAD_START,      //!< Start of the mac gcr6 sector header checksum calculation (xor of pre-encode 6-bits values, gcr6-encoded)
		CRC_FCS_START,          //!< Start a Compucolor File Control System checksum calculation, p1 = crc id
		CRC_VICTOR_HDR_START,   //!< Start a Victor 9000 checksum calculation, p1 = crc id
		CRC_VICTOR_DATA_START,  //!< Start a Victor 9000 checksum calculation, p1 = crc id
		CRC_END,                //!< End the checksum, p1 = crc id
		CRC,                    //!< Write a checksum in the apporpriate format, p1 = crc id

		SECTOR_LOOP_START,      //!< Start of the per-sector loop, sector number goes from p1 to p2 inclusive
		SECTOR_LOOP_END,        //!< End of the per-sector loop
		SECTOR_INTERLEAVE_SKEW  //!< Defines interleave and skew for sector counting
	};

	//! Sector data description
	struct desc_s {
		int size;          //!< Sector size, int bytes
		const UINT8 *data; //!< Sector data
		UINT8 sector_id;   //!< Sector ID
		UINT8 sector_info; //!< Sector free byte
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
	void generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image *image);

	/*! @brief Generate a track from cell binary values, MSB-first.
	    @param track
	    @param head
	    @param trackbuf track input buffer.
	    @param track_size in cells, not bytes.
	    @param image
	*/
	void generate_track_from_bitstream(int track, int head, const UINT8 *trackbuf, int track_size, floppy_image *image, int subtrack = 0);

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
	void generate_track_from_levels(int track, int head, std::vector<UINT32> &trackbuf, int splice_pos, floppy_image *image);

	//! Normalize the times in a cell buffer to sum up to 200000000
	void normalize_times(std::vector<UINT32> &buffer);

	// Some conversion tables for gcr
	static const UINT8 gcr5fw_tb[0x10], gcr5bw_tb[0x20];
	static const UINT8 gcr6fw_tb[0x40], gcr6bw_tb[0x100];

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

	void generate_bitstream_from_track(int track, int head, int cell_size, UINT8 *trackbuf, int &track_size, floppy_image *image, int subtrack = 0);

	//! Defines a standard sector for extracting.
	struct desc_xs {
		int track,  //!< Track for this sector
			head,   //!< Head for this sector
			size;   //!< Size of this sector
		const UINT8 *data; //!< Data within this sector
	};

	struct desc_pc_sector {
		UINT8 track, head, sector, size;
		int actual_size;
		UINT8 *data;
		bool deleted;
		bool bad_crc;
	};

	int calc_default_pc_gap3_size(UINT32 form_factor, int sector_size);
	void build_wd_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2);
	void build_wd_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2=22);
	void build_pc_track_fm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=40, int gap_1=26, int gap_2=11);
	void build_pc_track_mfm(int track, int head, floppy_image *image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=80, int gap_1=50, int gap_2=22);


	//! @brief Extract standard sectors from a regenerated bitstream.
	//! Sectors must point to an array of 256 desc_xs.

	//! An existing sector is recognizable by having ->data non-null.
	//! Sector data is written in sectdata up to sectdata_size bytes.

	//! The ones implemented here are the ones used by multiple
	//! systems.

	//! PC-type sectors with MFM encoding, sector size can go from 128 bytes to 16K.
	void extract_sectors_from_bitstream_mfm_pc(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size);
	//! PC-type sectors with FM encoding
	void extract_sectors_from_bitstream_fm_pc(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size);
	//! Commodore type sectors with GCR5 encoding
	void extract_sectors_from_bitstream_gcr5(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size, int head, int tracks);
	//! Victor 9000 type sectors with GCR5 encoding
	void extract_sectors_from_bitstream_victor_gcr5(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size);


	//! @brief Get a geometry (including sectors) from an image.

	//!   PC-type sectors with MFM encoding
	void get_geometry_mfm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count);
	//!   PC-type sectors with FM encoding
	void get_geometry_fm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count);


	//!  Regenerate the data for a full track.
	//!  PC-type sectors with MFM encoding and fixed-size.
	void get_track_data_mfm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, UINT8 *sectdata);

	//!  Regenerate the data for a full track.
	//!  PC-type sectors with FM encoding and fixed-size.
	void get_track_data_fm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, UINT8 *sectdata);

	//! Look up a bit in a level-type stream.
	bool bit_r(const std::vector<UINT32> &buffer, int offset);
	//! Look up multiple bits
	UINT32 bitn_r(const std::vector<UINT32> &buffer, int offset, int count);
	//! Write a bit with a given size.
	void bit_w(std::vector<UINT32> &buffer, bool val, UINT32 size = 1000);
	void bit_w(std::vector<UINT32> &buffer, bool val, UINT32 size, int offset);
	//! Calculate a CCITT-type CRC.
	UINT16 calc_crc_ccitt(const std::vector<UINT32> &buffer, int start, int end);
	//! Write a series of (raw) bits
	void raw_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size = 1000);
	void raw_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset);
	//! FM-encode and write a series of bits
	void fm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size = 1000);
	void fm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset);
	//! MFM-encode and write a series of bits
	void mfm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size = 1000);
	void mfm_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size, int offset);
	//! MFM-encode every two bits and write
	void mfm_half_w(std::vector<UINT32> &buffer, int start_bit, UINT32 val, UINT32 size = 1000);
	//! GCR5-encode and write a series of bits
	void gcr5_w(std::vector<UINT32> &buffer, UINT8 val, UINT32 size = 1000);
	void gcr5_w(std::vector<UINT32> &buffer, UINT8 val, UINT32 size, int offset);
	//! 8N1-encode and write a series of bits
	void _8n1_w(std::vector<UINT32> &buffer, int n, UINT32 val, UINT32 size = 1000);
	//! GCR4 encode (Apple II sector header)
	UINT16 gcr4_encode(UINT8 va);
	//! GCR4 decode
	UINT8 gcr4_decode(UINT8 e0, UINT8 e1);
	//! GCR6 encode (Apple II 16-sector and Mac-style GCR)
	UINT32 gcr6_encode(UINT8 va, UINT8 vb, UINT8 vc);
	//! GCR6 decode
	void gcr6_decode(UINT8 e0, UINT8 e1, UINT8 e2, UINT8 e3, UINT8 &va, UINT8 &vb, UINT8 &vc);

	UINT8 sbyte_mfm_r(const UINT8 *bitstream, int &pos, int track_size);
	UINT8 sbyte_gcr5_r(const UINT8 *bitstream, int &pos, int track_size);

private:
	enum { CRC_NONE, CRC_AMIGA, CRC_CBM, CRC_CCITT, CRC_CCITT_FM, CRC_MACHEAD, CRC_FCS, CRC_VICTOR_HDR, CRC_VICTOR_DATA };
	enum { MAX_CRC_COUNT = 64 };

	//! Holds data used internally for generating CRCs.
	struct gen_crc_info {
		int type, //!< Type of CRC
			start, //!< Start position
			end, //!< End position
			write; //!< where to write the CRC
		bool fixup_mfm_clock; //!< would the MFM clock bit after the CRC need to be fixed?
	};

	bool type_no_data(int type) const;
	bool type_data_mfm(int type, int p1, const gen_crc_info *crcs) const;

	int crc_cells_size(int type) const;
	void fixup_crc_amiga(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_cbm(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_ccitt(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_ccitt_fm(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_machead(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_fcs(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_victor_header(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crc_victor_data(std::vector<UINT32> &buffer, const gen_crc_info *crc);
	void fixup_crcs(std::vector<UINT32> &buffer, gen_crc_info *crcs);
	void collect_crcs(const desc_e *desc, gen_crc_info *crcs) const;

	int sbit_r(const UINT8 *bitstream, int pos);
	int sbit_rp(const UINT8 *bitstream, int &pos, int track_size);

	int calc_sector_index(int num, int interleave, int skew, int total_sectors, int track_head);
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
	floppy_image(int tracks, int heads, UINT32 form_factor);
	virtual ~floppy_image();

	//! @return the form factor.
	UINT32 get_form_factor() const { return form_factor; }
	//! @return the variant.
	UINT32 get_variant() const { return variant; }
	//! @param v the variant.
	void set_variant(UINT32 v) { variant = v; }

	/*!
	  @param track
	  @param subtrack
	  @param head head number
	  @return a pointer to the data buffer for this track and head
	*/
	std::vector<UINT32> &get_buffer(int track, int head, int subtrack = 0) { return track_array[track*4+subtrack][head].cell_data; }

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
	void set_write_splice_position(int track, int head, UINT32 pos, int subtrack = 0) { track_array[track*4+subtrack][head].write_splice = pos; }
	//! @return the current write splice position.
	UINT32 get_write_splice_position(int track, int head, int subtrack = 0) const { return track_array[track*4+subtrack][head].write_splice; }
	//! @return the maximal geometry supported by this format.
	void get_maximal_geometry(int &tracks, int &heads) const;

	//! @return the current geometry of the loaded image.
	void get_actual_geometry(int &tracks, int &heads);

	//! @return the track resolution (0=full track, 1 = half-track, 2 = quarter track)
	int get_resolution() const;

	//! Returns the variant name for the particular disk form factor/variant
	//! @param form_factor
	//! @param variant
	//! @param returns a string containing the variant name.
	static const char *get_variant_name(UINT32 form_factor, UINT32 variant);

private:
	int tracks, heads;

	UINT32 form_factor, variant;

	struct track_info {
		std::vector<UINT32> cell_data;
		UINT32 write_splice;

		track_info() { write_splice = 0; }
	};

	// track number multiplied by 4 then head
	// last array size may be bigger than actual track size
	std::vector<std::vector<track_info> > track_array;
};

#endif /* FLOPIMG_H */
