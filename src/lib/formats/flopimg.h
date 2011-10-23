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
	floppy_image_format_t();
	virtual ~floppy_image_format_t();

	virtual int identify(io_generic *io) = 0;
	virtual bool load(io_generic *io, floppy_image *image) = 0;
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const = 0;
	virtual const char *description() const = 0;
	virtual const char *extensions() const = 0;
	virtual bool supports_save() const = 0;

	floppy_image_format_t *next;
	void append(floppy_image_format_t *_next);

protected:
	// Input for convert_to_edge
	enum {
		MG_SHIFT  = 28,

		MG_0      = (4 << MG_SHIFT),
		MG_1      = (5 << MG_SHIFT),
		MG_W      = (6 << MG_SHIFT)
	};

	// **** Reader helpers ****

	// Struct designed for easy track data description
	// Optional, you can always do things by hand, but useful nevertheless
	// A vector of these structures describes one track.

	struct desc_e {
		int type, p1, p2;
	};

	enum {
		END,               // End of description
		MFM,               // One byte in p1 to be mfm-encoded, msb first, repeated p2 times
		MFMBITS,           // A value of p2 bits in p1 to be mfm-encoded, msb first
		RAW,               // One 16 bits word in p1 to be written raw, msb first, repeated p2 times
		RAWBITS,           // A value of p2 bits in p1 to be copied as-is, msb first
		TRACK_ID,          // Track id byte, mfm-encoded
		HEAD_ID,           // Head id byte, mfm-encoded
		SECTOR_ID,         // Sector id byte, mfm-encoded
		SIZE_ID,           // Sector size code on one byte [log2(size/128)], mfm-encoded
		OFFSET_ID_O,       // Offset (track*2+head) byte, odd bits, mfm-encoded
		OFFSET_ID_E,       // Offset (track*2+head) byte, even bits, mfm-encoded
		SECTOR_ID_O,       // Sector id byte, odd bits, mfm-encoded
		SECTOR_ID_E,       // Sector id byte, even bits, mfm-encoded
		REMAIN_O,          // Remaining sector count, odd bits, mfm-encoded, total sector count in p1
		REMAIN_E,          // Remaining sector count, even bits, mfm-encoded, total sector count in p1

		SECTOR_DATA,       // Sector data to mfm-encode, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_O,     // Sector data to mfm-encode, odd bits only, which in p1, -1 for the current one per the sector id
		SECTOR_DATA_E,     // Sector data to mfm-encode, even bits only, which in p1, -1 for the current one per the sector id

		CRC_CCITT_START,   // Start a CCITT CRC calculation, with the usual x^16 + x^12 + x^5 + 1 (11021) polynomial, p1 = crc id
		CRC_AMIGA_START,   // Start an amiga checksum calculation, p1 = crc id
		CRC_END,           // End the checksum, p1 = crc id
		CRC,               // Write a checksum in the apporpriate format, p1 = crc id

		SECTOR_LOOP_START, // Start of the per-sector loop, sector number goes from p1 to p2 inclusive
		SECTOR_LOOP_END,   // End of the per-sector loop
		SECTOR_INTERLEAVE_SKEW, // Defines interleave and skew for sector counting
	};

	// Sector data description
	struct desc_s {
		int size;          // Sector size, int bytes
		const UINT8 *data; // Sector data
		UINT8 sector_id;   // Sector ID
	};


	// Generate one track according to the description vector
	// "sect" is a vector indexed by sector id
	// "track_size" is in _cells_, i.e. 100000 for a usual 2us-per-cell track at 300rpm

	void generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image *image);

	// Generate a track from cell binary values, MSB-first, size in cells and not bytes
	void generate_track_from_bitstream(int track, int head, const UINT8 *trackbuf, int track_size, floppy_image *image);

	// Generate a track from cell level values (0/1/W/D/N)
	//
	// Splice pos is the position of the track splice.  For normal
	// formats, use -1.  For protected formats, you're supposed to
	// know. trackbuf may be modified at that position or after.
	//
	// Note that this function needs to be able to split cells in two,
	// so no time value should be less than 2, and even values are a
	// good idea.

	void generate_track_from_levels(int track, int head, UINT32 *trackbuf, int track_size, int splice_pos, floppy_image *image);

	// Normalize the times in a cell buffer to sum up to 200000000
	void normalize_times(UINT32 *buffer, int bitlen);


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


	// **** Writer helpers ****

	// Rebuild a cell bitstream for a track.  Takes the cell standard
	// angular size as a parameter, gives out a msb-first bitstream.
	// Beware that fuzzy bits will always give out the same value.
	//
	// Output buffer size should be 34% more than the nominal number
	// of cells (the dpll tolerates a cell size down to 75% of the
	// nominal one, with gives a cell count of 1/0.75=1.333... times
	// the nominal one).
	//
	// Output size is given in bits (cells).
	//
	// Computing the standard angular size of a cell is
	// simple. Noting:
	//   d = standard cell duration in microseconds
	//   r = motor rotational speed in rpm
	// then:
	//   a = r * d * 10 / 3
	//
	// Some values:
	//   Type           Cell    RPM    Size

	// C1541 tr  1-17   3.25    300    3250
	// C1541 tr 18-24   3.50    300    3500
	// C1541 tr 25-30   3.75    300    3750
	// C1541 tr 31+     4.00    300    4000
	// 5.25" SD         4       300    4000
	// 5.25" DD         2       300    2000
	// 5.25" HD         1       360    1200
	// 3.5" SD          4       300    4000
	// 3.5" DD          2       300    2000
	// 3.5" HD          1       300    1000
	// 3.5" ED          0.5     300     500

	void generate_bitstream_from_track(int track, int head, int cell_size,  UINT8 *trackbuf, int &track_size, floppy_image *image);

	struct desc_xs {
		int track, head, size;
		const UINT8 *data;
	};

	// Extract standard sectors from a regenerated bitstream
	// sectors must point to an array of 256 desc_xs
	// An existing sector is reconizable by having ->data non-null
	// Sector data is written in sectdata up to sectdata_size bytes

	// The ones implemented here are the ones used by multiple
	// systems.

	//   PC-type sectors with MFM encoding, sector size can go from 128 bytes to 16K
	void extract_sectors_from_bitstream_mfm_pc(const UINT8 *bitstream, int track_size, desc_xs *sectors, UINT8 *sectdata, int sectdata_size);


	// Get a geometry (including sectors) from an image
	//   PC-type sectors with MFM encoding
	void get_geometry_mfm_pc(floppy_image *image, int cell_size, int &track_count, int &head_count, int &sector_count);


	// Regenerate the data for a full track
	//   PC-type sectors with MFM encoding and fixed-size
	void get_track_data_mfm_pc(int track, int head, floppy_image *image, int cell_size, int sector_size, int sector_count, UINT8 *sectdata);

private:
	enum { CRC_NONE, CRC_AMIGA, CRC_CCITT };
	enum { MAX_CRC_COUNT = 64 };
	struct gen_crc_info {
		int type, start, end, write;
		bool fixup_mfm_clock;
	};

	bool type_no_data(int type) const;
	bool type_data_mfm(int type, int p1, const gen_crc_info *crcs) const;

	bool bit_r(UINT8 *buffer, int offset);
	void bit_w(UINT8 *buffer, int offset, bool val);

	int crc_cells_size(int type) const;
	void fixup_crc_amiga(UINT8 *buffer, const gen_crc_info *crc);
	void fixup_crc_ccitt(UINT8 *buffer, const gen_crc_info *crc);
	void fixup_crcs(UINT8 *buffer, gen_crc_info *crcs);
	void raw_w(UINT8 *buffer, int &offset, int n, UINT32 val);
	void mfm_w(UINT8 *buffer, int &offset, int n, UINT32 val);
	void mfm_half_w(UINT8 *buffer, int &offset, int start_bit, UINT32 val);
	void collect_crcs(const desc_e *desc, gen_crc_info *crcs);

	int sbit_r(const UINT8 *bitstream, int pos);
	int sbit_rp(const UINT8 *bitstream, int &pos, int track_size);
	UINT8 sbyte_mfm_r(const UINT8 *bitstream, int &pos, int track_size);

	int calc_sector_index(int num, int interleave, int skew, int total_sectors, int track_head);
};

// a device_type is simply a pointer to its alloc function
typedef floppy_image_format_t *(*floppy_format_type)();

// this template function creates a stub which constructs a image format
template<class _FormatClass>
floppy_image_format_t *floppy_image_format_creator()
{
	return new _FormatClass();
}

// ======================> floppy_image

// class representing floppy image
class floppy_image
{
public:
	// Internal format is close but not identical to the mfi format.
	//
	//
	// Track data consists of a series of 32-bits lsb-first values
	// representing magnetic cells.  Bits 0-27 indicate the absolute
	// position of the start of the cell (not the size), and bits
	// 28-31 the type.  Type can be:
	// - 0, MG_A -> Magnetic orientation A
	// - 1, MG_B -> Magnetic orientation B
	// - 2, MG_N -> Non-magnetized zone (neutral)
	// - 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
	//
	// The position is in angular units of 1/200,000,000th of a turn.
	// The last cell implicit end position is of course 200,000,000.
	//
	// Unformatted tracks are encoded as zero-size.

	enum {
		TIME_MASK = 0x0fffffff,
		MG_MASK   = 0xf0000000,
		MG_SHIFT  = 28,

		MG_A      = (0 << MG_SHIFT),
		MG_B      = (1 << MG_SHIFT),
		MG_N      = (2 << MG_SHIFT),
		MG_D      = (3 << MG_SHIFT)
	};

	// construction/destruction
	floppy_image(int tracks, int heads);
	virtual ~floppy_image();

	void set_track_size(int track, int head, UINT32 size) { track_size[(track << 1) + head] = size; ensure_alloc(track, head); }
	UINT32 *get_buffer(int track, int head) { return cell_data[(track << 1) + head]; }
	UINT32 get_track_size(int track, int head) { return track_size[(track << 1) + head]; }

	void get_maximal_geometry(int &tracks, int &heads);
	void get_actual_geometry(int &tracks, int &heads);

private:
	enum {
		MAX_FLOPPY_HEADS = 2,
		MAX_FLOPPY_TRACKS = 84
	};

	int tracks, heads;

	UINT32 *cell_data[MAX_FLOPPY_HEADS * MAX_FLOPPY_TRACKS];
	UINT32 track_size[MAX_FLOPPY_HEADS * MAX_FLOPPY_TRACKS];
	UINT32 track_alloc_size[MAX_FLOPPY_HEADS * MAX_FLOPPY_TRACKS];

	void ensure_alloc(int track, int head);
};

#endif /* FLOPIMG_H */

