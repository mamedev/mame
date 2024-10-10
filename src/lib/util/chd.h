// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    chd.h

    MAME Compressed Hunks of Data file format

***************************************************************************/
#ifndef MAME_LIB_UTIL_CHD_H
#define MAME_LIB_UTIL_CHD_H

#pragma once

#include "chdcodec.h"
#include "hashing.h"
#include "ioprocs.h"

#include "osdcore.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>


/***************************************************************************

    Compressed Hunks of Data header format. All numbers are stored in
    Motorola (big-endian) byte ordering.

    =========================================================================

    V1 header:

    [  0] char   tag[8];        // 'MComprHD'
    [  8] uint32_t length;        // length of header (including tag and length fields)
    [ 12] uint32_t version;       // drive format version
    [ 16] uint32_t flags;         // flags (see below)
    [ 20] uint32_t compression;   // compression type
    [ 24] uint32_t hunksize;      // 512-byte sectors per hunk
    [ 28] uint32_t totalhunks;    // total # of hunks represented
    [ 32] uint32_t cylinders;     // number of cylinders on hard disk
    [ 36] uint32_t heads;         // number of heads on hard disk
    [ 40] uint32_t sectors;       // number of sectors on hard disk
    [ 44] uint8_t  md5[16];       // MD5 checksum of raw data
    [ 60] uint8_t  parentmd5[16]; // MD5 checksum of parent file
    [ 76] (V1 header length)

    Flags:
        0x00000001 - set if this drive has a parent
        0x00000002 - set if this drive allows writes

    Compression types:
        CHDCOMPRESSION_NONE = 0
        CHDCOMPRESSION_ZLIB = 1

    V1 map format:

    [  0] uint64_t offset : 44;   // starting offset within the file
    [  0] uint64_t length : 20;   // length of data; if == hunksize, data is uncompressed

    =========================================================================

    V2 header:

    [  0] char   tag[8];        // 'MComprHD'
    [  8] uint32_t length;        // length of header (including tag and length fields)
    [ 12] uint32_t version;       // drive format version
    [ 16] uint32_t flags;         // flags (see below)
    [ 20] uint32_t compression;   // compression type
    [ 24] uint32_t hunksize;      // seclen-byte sectors per hunk
    [ 28] uint32_t totalhunks;    // total # of hunks represented
    [ 32] uint32_t cylinders;     // number of cylinders on hard disk
    [ 36] uint32_t heads;         // number of heads on hard disk
    [ 40] uint32_t sectors;       // number of sectors on hard disk
    [ 44] uint8_t  md5[16];       // MD5 checksum of raw data
    [ 60] uint8_t  parentmd5[16]; // MD5 checksum of parent file
    [ 76] uint32_t seclen;        // number of bytes per sector
    [ 80] (V2 header length)

    Flags and map format are same as V1

    =========================================================================

    V3 header:

    [  0] char   tag[8];        // 'MComprHD'
    [  8] uint32_t length;        // length of header (including tag and length fields)
    [ 12] uint32_t version;       // drive format version
    [ 16] uint32_t flags;         // flags (see below)
    [ 20] uint32_t compression;   // compression type
    [ 24] uint32_t totalhunks;    // total # of hunks represented
    [ 28] uint64_t logicalbytes;  // logical size of the data (in bytes)
    [ 36] uint64_t metaoffset;    // offset to the first blob of metadata
    [ 44] uint8_t  md5[16];       // MD5 checksum of raw data
    [ 60] uint8_t  parentmd5[16]; // MD5 checksum of parent file
    [ 76] uint32_t hunkbytes;     // number of bytes per hunk
    [ 80] uint8_t  sha1[20];      // SHA1 checksum of raw data
    [100] uint8_t  parentsha1[20];// SHA1 checksum of parent file
    [120] (V3 header length)

    Flags are the same as V1

    Compression types:
        CHDCOMPRESSION_NONE = 0
        CHDCOMPRESSION_ZLIB = 1
        CHDCOMPRESSION_ZLIB_PLUS = 2

    V3 map format:

    [  0] uint64_t offset;        // starting offset within the file
    [  8] uint32_t crc32;         // 32-bit CRC of the uncompressed data
    [ 12] uint16_t length_lo;     // lower 16 bits of length
    [ 14] uint8_t length_hi;      // upper 8 bits of length
    [ 15] uint8_t flags;          // flags, indicating compression info

    =========================================================================

    V4 header:

    [  0] char   tag[8];        // 'MComprHD'
    [  8] uint32_t length;        // length of header (including tag and length fields)
    [ 12] uint32_t version;       // drive format version
    [ 16] uint32_t flags;         // flags (see below)
    [ 20] uint32_t compression;   // compression type
    [ 24] uint32_t totalhunks;    // total # of hunks represented
    [ 28] uint64_t logicalbytes;  // logical size of the data (in bytes)
    [ 36] uint64_t metaoffset;    // offset to the first blob of metadata
    [ 44] uint32_t hunkbytes;     // number of bytes per hunk
    [ 48] uint8_t  sha1[20];      // combined raw+meta SHA1
    [ 68] uint8_t  parentsha1[20];// combined raw+meta SHA1 of parent
    [ 88] uint8_t  rawsha1[20];   // raw data SHA1
    [108] (V4 header length)

    Flags are the same as V1

    Compression types:
        CHDCOMPRESSION_NONE = 0
        CHDCOMPRESSION_ZLIB = 1
        CHDCOMPRESSION_ZLIB_PLUS = 2
        CHDCOMPRESSION_AV = 3

    Map format is the same as V3

    =========================================================================

    V5 header:

    [  0] char   tag[8];        // 'MComprHD'
    [  8] uint32_t length;        // length of header (including tag and length fields)
    [ 12] uint32_t version;       // drive format version
    [ 16] uint32_t compressors[4];// which custom compressors are used?
    [ 32] uint64_t logicalbytes;  // logical size of the data (in bytes)
    [ 40] uint64_t mapoffset;     // offset to the map
    [ 48] uint64_t metaoffset;    // offset to the first blob of metadata
    [ 56] uint32_t hunkbytes;     // number of bytes per hunk (512k maximum)
    [ 60] uint32_t unitbytes;     // number of bytes per unit within each hunk
    [ 64] uint8_t  rawsha1[20];   // raw data SHA1
    [ 84] uint8_t  sha1[20];      // combined raw+meta SHA1
    [104] uint8_t  parentsha1[20];// combined raw+meta SHA1 of parent
    [124] (V5 header length)

    If parentsha1 != 0, we have a parent (no need for flags)
    If compressors[0] == 0, we are uncompressed (including maps)

    V5 uncompressed map format:

    [  0] uint32_t offset;        // starting offset / hunk size

    V5 compressed map format header:

    [  0] uint32_t length;        // length of compressed map
    [  4] UINT48 datastart;     // offset of first block
    [ 10] uint16_t crc;           // crc-16 of the map
    [ 12] uint8_t lengthbits;     // bits used to encode complength
    [ 13] uint8_t hunkbits;       // bits used to encode self-refs
    [ 14] uint8_t parentunitbits; // bits used to encode parent unit refs
    [ 15] uint8_t reserved;       // future use
    [ 16] (compressed header length)

    Each compressed map entry, once expanded, looks like:

    [  0] uint8_t compression;    // compression type
    [  1] UINT24 complength;    // compressed length
    [  4] UINT48 offset;        // offset
    [ 10] uint16_t crc;           // crc-16 of the data

***************************************************************************/


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// pseudo-codecs returned by hunk_info
constexpr chd_codec_type CHD_CODEC_SELF         = 1;    // copy of another hunk
constexpr chd_codec_type CHD_CODEC_PARENT       = 2;    // copy of a parent's hunk
constexpr chd_codec_type CHD_CODEC_MINI         = 3;    // legacy "mini" 8-byte repeat

// core types
typedef uint32_t chd_metadata_tag;

// metadata parameters
constexpr chd_metadata_tag CHDMETATAG_WILDCARD = 0;
constexpr uint32_t CHDMETAINDEX_APPEND = ~0;

// metadata flags
constexpr uint8_t CHD_MDFLAGS_CHECKSUM = 0x01;        // indicates data is checksummed

// standard hard disk metadata
constexpr chd_metadata_tag HARD_DISK_METADATA_TAG = CHD_MAKE_TAG('G','D','D','D');
extern const char *HARD_DISK_METADATA_FORMAT;

// hard disk identify information
constexpr chd_metadata_tag HARD_DISK_IDENT_METADATA_TAG = CHD_MAKE_TAG('I','D','N','T');

// hard disk key information
constexpr chd_metadata_tag HARD_DISK_KEY_METADATA_TAG = CHD_MAKE_TAG('K','E','Y',' ');

// pcmcia CIS information
constexpr chd_metadata_tag PCMCIA_CIS_METADATA_TAG = CHD_MAKE_TAG('C','I','S',' ');

// standard CD-ROM metadata
constexpr chd_metadata_tag CDROM_OLD_METADATA_TAG = CHD_MAKE_TAG('C','H','C','D');
constexpr chd_metadata_tag CDROM_TRACK_METADATA_TAG = CHD_MAKE_TAG('C','H','T','R');
extern const char *CDROM_TRACK_METADATA_FORMAT;
constexpr chd_metadata_tag CDROM_TRACK_METADATA2_TAG = CHD_MAKE_TAG('C','H','T','2');
extern const char *CDROM_TRACK_METADATA2_FORMAT;
constexpr chd_metadata_tag GDROM_OLD_METADATA_TAG = CHD_MAKE_TAG('C','H','G','T');
constexpr chd_metadata_tag GDROM_TRACK_METADATA_TAG = CHD_MAKE_TAG('C', 'H', 'G', 'D');
extern const char *GDROM_TRACK_METADATA_FORMAT;

// standard DVD metafata
constexpr chd_metadata_tag DVD_METADATA_TAG = CHD_MAKE_TAG('D', 'V', 'D', ' ');

// standard A/V metadata
constexpr chd_metadata_tag AV_METADATA_TAG = CHD_MAKE_TAG('A','V','A','V');
extern const char *AV_METADATA_FORMAT;

// A/V laserdisc frame metadata
constexpr chd_metadata_tag AV_LD_METADATA_TAG = CHD_MAKE_TAG('A','V','L','D');



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> chd_file

// core file class
class chd_file
{
	friend class chd_file_compressor;
	friend class chd_verifier;

	// constants
	static constexpr uint32_t HEADER_VERSION = 5;
	static constexpr uint32_t V3_HEADER_SIZE = 120;
	static constexpr uint32_t V4_HEADER_SIZE = 108;
	static constexpr uint32_t V5_HEADER_SIZE = 124;
	static constexpr uint32_t MAX_HEADER_SIZE = V5_HEADER_SIZE;

public:
	// error types
	enum class error
	{
		NO_INTERFACE = 1,
		NOT_OPEN,
		ALREADY_OPEN,
		INVALID_FILE,
		INVALID_DATA,
		REQUIRES_PARENT,
		FILE_NOT_WRITEABLE,
		CODEC_ERROR,
		INVALID_PARENT,
		HUNK_OUT_OF_RANGE,
		DECOMPRESSION_ERROR,
		COMPRESSION_ERROR,
		CANT_VERIFY,
		METADATA_NOT_FOUND,
		INVALID_METADATA_SIZE,
		UNSUPPORTED_VERSION,
		VERIFY_INCOMPLETE,
		INVALID_METADATA,
		INVALID_STATE,
		OPERATION_PENDING,
		UNSUPPORTED_FORMAT,
		UNKNOWN_COMPRESSION,
		WALKING_PARENT,
		COMPRESSING
	};

	using open_parent_func = std::function<std::unique_ptr<chd_file> (util::sha1_t const &)>;

	// construction/destruction
	chd_file();
	virtual ~chd_file();

	// getters
	util::random_read &file();
	bool opened() const noexcept { return bool(m_file); }
	uint32_t version() const noexcept { return m_version; }
	uint64_t logical_bytes() const noexcept { return m_logicalbytes; }
	uint32_t hunk_bytes() const noexcept { return m_hunkbytes; }
	uint32_t hunk_count() const noexcept { return m_hunkcount; }
	uint32_t unit_bytes() const noexcept { return m_unitbytes; }
	uint64_t unit_count() const noexcept { return m_unitcount; }
	bool compressed() const noexcept { return (m_compression[0] != CHD_CODEC_NONE); }
	chd_codec_type compression(int index) const noexcept { return m_compression[index]; }
	chd_file *parent() const noexcept { return m_parent.get(); }
	bool parent_missing() const noexcept;
	util::sha1_t sha1() const noexcept;
	util::sha1_t raw_sha1() const noexcept;
	util::sha1_t parent_sha1() const noexcept;
	std::error_condition hunk_info(uint32_t hunknum, chd_codec_type &compressor, uint32_t &compbytes);

	// setters
	std::error_condition set_raw_sha1(util::sha1_t rawdata) noexcept;
	std::error_condition set_parent_sha1(util::sha1_t parent) noexcept;

	// file create
	std::error_condition create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, const chd_codec_type (&compression)[4]);
	std::error_condition create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t unitbytes, const chd_codec_type (&compression)[4]);
	std::error_condition create(std::string_view filename, uint64_t logicalbytes, uint32_t hunkbytes, const chd_codec_type (&compression)[4], chd_file &parent);
	std::error_condition create(util::random_read_write::ptr &&file, uint64_t logicalbytes, uint32_t hunkbytes, const chd_codec_type (&compression)[4], chd_file &parent);

	// file open
	std::error_condition open(std::string_view filename, bool writeable = false, chd_file *parent = nullptr, const open_parent_func &open_parent = nullptr);
	std::error_condition open(util::random_read_write::ptr &&file, bool writeable = false, chd_file *parent = nullptr, const open_parent_func &open_parent = nullptr);

	// file close
	void close();

	// read/write
	std::error_condition read_hunk(uint32_t hunknum, void *buffer);
	std::error_condition write_hunk(uint32_t hunknum, const void *buffer);
	std::error_condition read_units(uint64_t unitnum, void *buffer, uint32_t count = 1);
	std::error_condition write_units(uint64_t unitnum, const void *buffer, uint32_t count = 1);
	std::error_condition read_bytes(uint64_t offset, void *buffer, uint32_t bytes);
	std::error_condition write_bytes(uint64_t offset, const void *buffer, uint32_t bytes);

	// metadata management
	std::error_condition read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::string &output);
	std::error_condition read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output);
	std::error_condition read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, void *output, uint32_t outputlen, uint32_t &resultlen);
	std::error_condition read_metadata(chd_metadata_tag searchtag, uint32_t searchindex, std::vector<uint8_t> &output, chd_metadata_tag &resulttag, uint8_t &resultflags);
	std::error_condition write_metadata(chd_metadata_tag metatag, uint32_t metaindex, const void *inputbuf, uint32_t inputlen, uint8_t flags = CHD_MDFLAGS_CHECKSUM);
	std::error_condition write_metadata(chd_metadata_tag metatag, uint32_t metaindex, const std::string &input, uint8_t flags = CHD_MDFLAGS_CHECKSUM) { return write_metadata(metatag, metaindex, input.c_str(), input.length() + 1, flags); }
	std::error_condition write_metadata(chd_metadata_tag metatag, uint32_t metaindex, const std::vector<uint8_t> &input, uint8_t flags = CHD_MDFLAGS_CHECKSUM) { return write_metadata(metatag, metaindex, &input[0], input.size(), flags); }
	std::error_condition delete_metadata(chd_metadata_tag metatag, uint32_t metaindex);
	std::error_condition clone_all_metadata(chd_file &source);

	// hashing helper
	util::sha1_t compute_overall_sha1(util::sha1_t rawsha1);

	// codec interfaces
	std::error_condition codec_configure(chd_codec_type codec, int param, void *config);

	// typing
	bool is_hd() const;
	bool is_cd() const;
	bool is_gd() const;
	bool is_dvd() const;
	bool is_av() const;

private:
	struct metadata_entry;
	struct metadata_hash;

	// inline helpers
	util::sha1_t be_read_sha1(const uint8_t *base) const noexcept;
	void be_write_sha1(uint8_t *base, util::sha1_t value) noexcept;
	void file_read(uint64_t offset, void *dest, uint32_t length) const;
	void file_write(uint64_t offset, const void *source, uint32_t length);
	uint64_t file_append(const void *source, uint32_t length, uint32_t alignment = 0);
	uint8_t bits_for_value(uint64_t value);

	// internal helpers
	uint32_t guess_unitbytes();
	void parse_v3_header(uint8_t *rawheader, util::sha1_t &parentsha1);
	void parse_v4_header(uint8_t *rawheader, util::sha1_t &parentsha1);
	void parse_v5_header(uint8_t *rawheader, util::sha1_t &parentsha1);
	std::error_condition compress_v5_map();
	void decompress_v5_map();
	std::error_condition create_common();
	std::error_condition open_common(bool writeable, const open_parent_func &open_parent);
	void create_open_common();
	void verify_proper_compression_append(uint32_t hunknum);
	void hunk_write_compressed(uint32_t hunknum, int8_t compression, const uint8_t *compressed, uint32_t complength, util::crc16_t crc16);
	void hunk_copy_from_self(uint32_t hunknum, uint32_t otherhunk);
	void hunk_copy_from_parent(uint32_t hunknum, uint64_t parentunit);
	bool metadata_find(chd_metadata_tag metatag, int32_t metaindex, metadata_entry &metaentry, bool resume = false) const;
	void metadata_set_previous_next(uint64_t prevoffset, uint64_t nextoffset);
	void metadata_update_hash();
	static int CLIB_DECL metadata_hash_compare(const void *elem1, const void *elem2);

	// file characteristics
	util::random_read_write::ptr m_file;        // handle to the open core file
	bool                    m_allow_reads;      // permit reads from this CHD?
	bool                    m_allow_writes;     // permit writes to this CHD?

	// core parameters from the header
	uint32_t                m_version;          // version of the header
	uint64_t                m_logicalbytes;     // logical size of the raw CHD data in bytes
	uint64_t                m_mapoffset;        // offset of map
	uint64_t                m_metaoffset;       // offset to first metadata bit
	uint32_t                m_hunkbytes;        // size of each raw hunk in bytes
	uint32_t                m_hunkcount;        // number of hunks represented
	uint32_t                m_unitbytes;        // size of each unit in bytes
	uint64_t                m_unitcount;        // number of units represented
	chd_codec_type          m_compression[4];   // array of compression types used
	std::shared_ptr<chd_file> m_parent;           // pointer to parent file, or nullptr if none
	bool                    m_parent_missing;   // are we missing our parent?

	// key offsets within the header
	uint64_t                m_mapoffset_offset; // offset of map offset field
	uint64_t                m_metaoffset_offset;// offset of metaoffset field
	uint64_t                m_sha1_offset;      // offset of SHA1 field
	uint64_t                m_rawsha1_offset;   // offset of raw SHA1 field
	uint64_t                m_parentsha1_offset;// offset of paren SHA1 field

	// map information
	uint32_t                m_mapentrybytes;    // length of each entry in a map
	std::vector<uint8_t>    m_rawmap;           // raw map data

	// compression management
	chd_decompressor::ptr   m_decompressor[4];  // array of decompression codecs
	std::vector<uint8_t>    m_compressed;       // temporary buffer for compressed data

	// caching
	std::vector<uint8_t>    m_cache;            // single-hunk cache for partial reads/writes
	uint32_t                m_cachehunk;        // which hunk is in the cache?
};


// ======================> chd_file_compressor

// class for creating a new compressed CHD
class chd_file_compressor : public chd_file
{
public:
	// construction/destruction
	chd_file_compressor();
	virtual ~chd_file_compressor();

	// compression management
	void compress_begin();
	std::error_condition compress_continue(double &progress, double &ratio);

protected:
	// required override: read more data
	virtual uint32_t read_data(void *dest, uint64_t offset, uint32_t length) = 0;

private:
	// hash map for looking up values
	class hashmap
	{
	public:
		// construction/destruction
		hashmap();
		~hashmap();

		// operations
		void reset();
		uint64_t find(util::crc16_t crc16, util::sha1_t sha1);
		void add(uint64_t itemnum, util::crc16_t crc16, util::sha1_t sha1);

		// constants
		static constexpr uint64_t NOT_FOUND = ~uint64_t(0);

	private:
		// internal entry
		struct entry_t
		{
			entry_t *           m_next;             // next entry in list
			uint64_t            m_itemnum;          // item number
			util::sha1_t        m_sha1;             // SHA-1 of the block
		};

		// block of entries
		struct entry_block
		{
			entry_block(entry_block *prev) : m_next(prev), m_nextalloc(0) { }

			entry_block *       m_next;             // next block in list
			uint32_t            m_nextalloc;        // next to be allocated
			entry_t             m_array[16384];     // array of entries
		};

		// internal state
		entry_t *           m_map[65536];           // map, hashed by CRC-16
		entry_block *       m_block_list;           // list of allocated blocks
	};

	// status of a given work item
	enum work_status
	{
		WS_READY = 0,
		WS_READING,
		WS_QUEUED,
		WS_COMPLETE
	};

	// a CRC-16/SHA-1 pair
	struct hash_pair
	{
		util::crc16_t             m_crc16;            // calculated CRC-16
		util::sha1_t              m_sha1;             // calculated SHA-1
	};

	// a single work item
	struct work_item
	{
		work_item()
			: m_osd(nullptr)
			, m_compressor(nullptr)
			, m_status(WS_READY)
			, m_hunknum(0)
			, m_data(nullptr)
			, m_compressed(nullptr)
			, m_complen(0)
			, m_compression(0)
			, m_codecs(nullptr)
		{ }

		osd_work_item *     m_osd;              // OSD work item running on this block
		chd_file_compressor *m_compressor;      // pointer back to the compressor
		// TODO: had to change this to be able to use atomic_* functions on this
		//volatile work_status m_status;          // current status of this item
		std::atomic<int32_t>  m_status;           // current status of this item
		uint32_t              m_hunknum;          // number of the hunk we're working on
		uint8_t *             m_data;             // pointer to the data we are working on
		uint8_t *             m_compressed;       // pointer to the compressed data
		uint32_t              m_complen;          // compressed data length
		int8_t                m_compression;      // type of compression used
		chd_compressor_group *m_codecs;         // codec instance
		std::vector<hash_pair> m_hash;        // array of hashes
	};

	// internal helpers
	static void *async_walk_parent_static(void *param, int threadid);
	void async_walk_parent(work_item &item);
	static void *async_compress_hunk_static(void *param, int threadid);
	void async_compress_hunk(work_item &item, int threadid);
	static void *async_read_static(void *param, int threadid);
	void async_read();

	// current compression status
	bool                    m_walking_parent;   // are we building the parent map?
	uint64_t                m_total_in;         // total bytes in
	uint64_t                m_total_out;        // total bytes out
	util::sha1_creator      m_compsha1;         // running SHA-1 on raw data

	// hash lookup maps
	hashmap                 m_parent_map;       // hash map for parent
	hashmap                 m_current_map;      // hash map for current

	// read I/O thread
	osd_work_queue *        m_read_queue;       // work queue for reading
	uint64_t                m_read_queue_offset;// next offset to enqueue
	uint64_t                m_read_done_offset; // next offset that will complete
	bool                    m_read_error;       // error during reading?

	// work item thread
	static constexpr int WORK_BUFFER_HUNKS = 256;
	osd_work_queue *        m_work_queue;       // queue for doing work on other threads
	std::vector<uint8_t>    m_work_buffer;      // buffer containing hunk data to work on
	std::vector<uint8_t>    m_compressed_buffer;// buffer containing compressed data
	work_item               m_work_item[WORK_BUFFER_HUNKS]; // status of each hunk
	chd_compressor_group *  m_codecs[WORK_MAX_THREADS]; // codecs to use

	// output state
	uint32_t                m_write_hunk;       // next hunk to write
};


// error category for CHD errors
std::error_category const &chd_category() noexcept;
inline std::error_condition make_error_condition(chd_file::error err) noexcept { return std::error_condition(int(err), chd_category()); }

namespace std {

template <> struct is_error_condition_enum<chd_file::error> : public std::true_type { };

} // namespace std

#endif // MAME_LIB_UTIL_CHD_H
