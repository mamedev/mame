// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    flopimg.h

    Floppy disk image abstraction code

*********************************************************************/

#ifndef MAME_FORMATS_FLOPIMG_H
#define MAME_FORMATS_FLOPIMG_H

#pragma once

#include "utilfwd.h"

#include <memory>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>


//////////////////////////////////////////////////////////
// New implementation
//////////////////////////////////////////////////////////

class floppy_image;

//! Class representing a floppy image format.
class floppy_image_format_t
{
public:
	virtual ~floppy_image_format_t() = default;

	// The result of identify is a binary or of these flags, comparison afterwards is numerical.
	// If a match is incorrect (bad signature for instance), result must be 0.  The non-zero
	// result helps to decide how reliable the identification is, for choice classification.

	enum {
		FIFID_HINT   = 0x01, // All other things being equal, favorise this format
		FIFID_EXT    = 0x02, // Extension matches one of the list (set outside of identify)
		FIFID_SIZE   = 0x04, // File size matches what is expected
		FIFID_SIGN   = 0x08, // The file signature matches
		FIFID_STRUCT = 0x10, // Some file internal structure aspects have been verified
	};

	/*! @brief Identify an image.
	  The identify function tests if the image is valid
	  for this particular format.
	  @param io buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @param variants the variants from floppy_image the drive can handle
	  @return Binary or of FIFID flags, 0 if invalid for that format
	*/
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const = 0;

	/*! @brief Load an image.
	  The load function opens an image file and converts it to the
	  internal MAME floppy representation.
	  @param io source buffer containing the image data.
	  @param form_factor Physical form factor of disk, from the enum
	  in floppy_image
	  @param variants the variants from floppy_image the drive can handle
	  @param image output buffer for data in MAME internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const = 0;

	/*! @brief Save an image.
	  The save function writes back an image from the MAME internal
	  floppy representation to the appropriate format on disk.
	  @param io output buffer for the data in the on-disk format.
	  @param variants the variants from floppy_image the drive can handle
	  @param image source buffer containing data in MAME internal format.
	  @return true on success, false otherwise.
	*/
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const;

	//! @returns string containing name of format.
	virtual const char *name() const noexcept = 0;
	//! @returns string containing description of format.
	virtual const char *description() const noexcept = 0;
	//! @returns string containing comma-separated list of file
	//! extensions the format may use.
	virtual const char *extensions() const noexcept = 0;
	//! @returns true if format supports saving.
	virtual bool supports_save() const noexcept = 0;

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
	static bool has_variant(const std::vector<uint32_t> &variants, uint32_t variant) noexcept;

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
	static void generate_track(const desc_e *desc, int track, int head, const desc_s *sect, int sect_count, int track_size, floppy_image &image);

	/*! @brief Generate a track from cell binary values, MSB-first.
	    @param track
	    @param head
	    @param trackbuf track input buffer.
	    @param track_size in cells, not bytes.
	    @param image
	    @param subtrack subtrack index, 0-3
	    @param splice write splice position
	*/
	static void generate_track_from_bitstream(int track, int head, const uint8_t *trackbuf, int track_size, floppy_image &image, int subtrack = 0, int splice = 0);

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
	static void generate_track_from_levels(int track, int head, const std::vector<uint32_t> &trackbuf, int splice_pos, floppy_image &image);

	//! Normalize the times in a cell buffer to bring the
	//! 0..last_position range up to 0..200000000
	static void normalize_times(std::vector<uint32_t> &buffer, uint32_t last_position);

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

	static std::vector<bool> generate_bitstream_from_track(int track, int head, int cell_size, const floppy_image &image, int subtrack = 0, int *max_delta = nullptr);
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
	static void build_wd_track_fm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2);
	static void build_wd_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2=22);
	static void build_pc_track_fm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=40, int gap_1=26, int gap_2=11);
	static void build_pc_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_4a=80, int gap_1=50, int gap_2=22);
	static void build_mac_track_gcr(int track, int head, floppy_image &image, const desc_gcr_sector *sects);

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
	static std::vector<std::vector<uint8_t>> extract_sectors_from_track_mac_gcr6(int head, int track, const floppy_image &image);


	//! @brief Get a geometry (including sectors) from an image.

	//!   PC-type sectors with MFM encoding
	static void get_geometry_mfm_pc(const floppy_image &image, int cell_size, int &track_count, int &head_count, int &sector_count);
	//!   PC-type sectors with FM encoding
	static void get_geometry_fm_pc(const floppy_image &image, int cell_size, int &track_count, int &head_count, int &sector_count);


	//!  Regenerate the data for a full track.
	//!  PC-type sectors with MFM encoding and fixed-size.
	static void get_track_data_mfm_pc(int track, int head, const floppy_image &image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata);

	//!  Regenerate the data for a full track.
	//!  PC-type sectors with FM encoding and fixed-size.
	static void get_track_data_fm_pc(int track, int head, const floppy_image &image, int cell_size, int sector_size, int sector_count, uint8_t *sectdata);

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

// ======================> floppy_image

//! Class representing floppy image

//! Internal format is close but not identical to the mfi format.
//!
//!

//! Track data consists of a series of 32-bits lsb-first values
//! representing the magnetic state.  Bits 0-27 indicate the absolute
//! position of encoded event, and bits ! 28-31 the type.  Type can be:
//! - 0, MG_F -> Flux orientation change
//! - 1, MG_N -> Start of a non-magnetized zone (neutral)
//! - 2, MG_D -> Start of a damaged zone, reads as neutral but cannot be changed by writing
//! - 3, MG_E -> End of one of the previous zones, *inclusive*
//!
//! The position is in angular units of 1/200,000,000th of a turn.
//! A N or D zone must not wrap at the 200,000,000 position, it has to
//! be split in two (the first finishing at 199,999,999, the second
//! starting at 0)
//!
//! Unformatted tracks are encoded as zero-size, and are strictly equivalent
//! to (MG_N, 0), (MG_E, 199,999,999)
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
		MG_F      = (0 << MG_SHIFT),    //!< - 0, MG_F -> Flux orientation change
		MG_N      = (1 << MG_SHIFT),    //!< - 1, MG_N -> Non-magnetized zone (neutral)
		MG_D      = (2 << MG_SHIFT),    //!< - 2, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
		MG_E      = (3 << MG_SHIFT)     //!< - 3, MG_E -> End of zone
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
		SSSD   = 0x44535353, //!< "SSSD", Single-sided single-density
		SSSD10 = 0x30315353, //!< "SS10", Single-sided single-density 10 hard sector
		SSSD16 = 0x36315353, //!< "SS16", Single-sided single-density 16 hard sector
		SSSD32 = 0x32335353, //!< "SS32", Single-sided single-density 32 hard sector
		SSDD   = 0x44445353, //!< "SSDD", Single-sided double-density
		SSDD10 = 0x30314453, //!< "SD10", Single-sided double-density 10 hard sector
		SSDD16 = 0x36314453, //!< "SD16", Single-sided double-density 16 hard sector
		SSDD32 = 0x32334453, //!< "SD32", Single-sided double-density 32 hard sector
		SSQD   = 0x44515353, //!< "SSQD", Single-sided quad-density
		SSQD10 = 0x30315153, //!< "SQ10", Single-sided quad-density 10 hard sector
		SSQD16 = 0x36315153, //!< "SQ16", Single-sided quad-density 16 hard sector
		DSSD   = 0x44535344, //!< "DSSD", Double-sided single-density
		DSSD10 = 0x30315344, //!< "DS10", Double-sided single-density 10 hard sector
		DSSD16 = 0x36315344, //!< "DS16", Double-sided single-density 16 hard sector
		DSSD32 = 0x32335344, //!< "DS32", Double-sided single-density 32 hard sector
		DSDD   = 0x44445344, //!< "DSDD", Double-sided double-density (720K in 3.5, 360K in 5.25)
		DSDD10 = 0x30314444, //!< "DD10", Double-sided double-density 10 hard sector
		DSDD16 = 0x36314444, //!< "DD16", Double-sided double-density 16 hard sector (360K in 5.25)
		DSDD32 = 0x32334444, //!< "DD32", Double-sided double-density 32 hard sector
		DSQD   = 0x44515344, //!< "DSQD", Double-sided quad-density (720K in 5.25, means DD+80 tracks)
		DSQD10 = 0x30315144, //!< "DQ10", Double-sided quad-density 10 hard sector
		DSQD16 = 0x36315144, //!< "DQ16", Double-sided quad-density 16 hard sector (720K in 5.25, means DD+80 tracks)
		DSHD   = 0x44485344, //!< "DSHD", Double-sided high-density (1440K)
		DSED   = 0x44455344  //!< "DSED", Double-sided extra-density (2880K)
	};

	//! Encodings
	enum {
		FM   = 0x2020464D, //!< "  FM", frequency modulation
		MFM  = 0x204D464D, //!< " MFM", modified frequency modulation
		M2FM = 0x4D32464D  //!< "M2FM", modified modified frequency modulation
	};

	//! Sectoring
	enum {
		SOFT = 0x54464F53,  //!< "SOFT", Soft-sectored
		H10  = 0x20303148,  //!< "H10 ", Hard 10-sectored
		H16  = 0x20363148,  //!< "H16 ", Hard 16-sectored
		H32  = 0x20323348   //!< "H32 ", Hard 32-sectored (8 inch disk)
	};

	// construction/destruction


	//! floppy_image constructor
	/*!
	  @param tracks number of tracks.
	  @param heads number of heads.
	  @param form_factor form factor of drive (from enum)
	*/
	floppy_image(int tracks, int heads, uint32_t form_factor);
	~floppy_image();

	//! @return the form factor.
	uint32_t get_form_factor() const noexcept { return form_factor; }
	//! @return the variant.
	uint32_t get_variant() const noexcept { return variant; }
	//! @return the disk sectoring.
	uint32_t get_sectoring() const noexcept { return sectoring; }
	//! @param v the variant.
	void set_variant(uint32_t v);
	//! @param v the variant.
	void set_form_variant(uint32_t f, uint32_t v) { if(form_factor == FF_UNKNOWN) form_factor = f; set_variant(v); }
	//! @param s the sectoring.
	void set_sectoring(uint32_t s) { sectoring = s; }

	//! Find most recent and next index hole for provided angular position.
	//! The most recent hole may be equal to provided position. The next
	//! hole will be 200000000 if all holes of the current rotation are in
	//! the past.

	/*! @param pos angular position
	    @param last most recent index hole
	    @param next next index hole
	*/
	void find_index_hole(uint32_t pos, uint32_t &last, uint32_t &next) const;

	/*!
	  @param track
	  @param subtrack
	  @param head head number
	  @return a pointer to the data buffer for this track and head
	*/
	std::vector<uint32_t> &get_buffer(int track, int head, int subtrack = 0) noexcept { assert(track < tracks && head < heads); return track_array[track*4+subtrack][head].cell_data; }
	const std::vector<uint32_t> &get_buffer(int track, int head, int subtrack = 0) const noexcept { assert(track < tracks && head < heads); return track_array[track*4+subtrack][head].cell_data; }

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
	void set_write_splice_position(int track, int head, uint32_t pos, int subtrack = 0) noexcept { assert(track < tracks && head < heads); track_array[track*4+subtrack][head].write_splice = pos; }
	//! @return the current write splice position.
	uint32_t get_write_splice_position(int track, int head, int subtrack = 0) const noexcept { assert(track < tracks && head < heads); return track_array[track*4+subtrack][head].write_splice; }
	//! @return the maximal geometry supported by this format.
	void get_maximal_geometry(int &tracks, int &heads) const noexcept;

	//! @return the current geometry of the loaded image.
	void get_actual_geometry(int &tracks, int &heads) const noexcept;

	//! @return the track resolution (0=full track, 1 = half-track, 2 = quarter track)
	int get_resolution() const noexcept;

	//! @return whether a given track is formatted
	bool track_is_formatted(int track, int head, int subtrack = 0) const noexcept;

	//! Returns the variant name for the particular disk form factor/variant
	//! @param form_factor
	//! @param variant
	//! @return a string containing the variant name.
	static const char *get_variant_name(uint32_t form_factor, uint32_t variant) noexcept;

private:
	int tracks, heads;

	uint32_t form_factor, variant, sectoring;

	struct track_info
	{
		std::vector<uint32_t> cell_data;
		uint32_t write_splice;

		track_info() { write_splice = 0; }
	};

	// track number multiplied by 4 then head
	// last array size may be bigger than actual track size
	std::vector<std::vector<track_info> > track_array;

	// Additional index holes in increasing order. Entries are absolute
	// positions of index holes in the same units as cell_data. The
	// positions are the start of the hole, not the center of the hole. The
	// hole at angular position 0 is implicit, so an empty list encodes a
	// regular soft-sectored disk. Additional holes are found on
	// hard-sectored disks.
	std::vector<uint32_t> index_array;
};

#endif // MAME_FORMATS_FLOPIMG_H
