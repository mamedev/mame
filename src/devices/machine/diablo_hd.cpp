// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 *   DIABLO31 and DIABLO44 hard drive support
 **********************************************************/
#include "diablo_hd.h"

/**
 *
 * Just for completeness' sake:
 * The mapping of disk controller connector P2 pins to the
 * Winchester disk drive signals (see drive.h)
 * <PRE>
 * Alto Controller     Winchester
 * P2 signal           disk bus
 * -----------------------------------------------
 *  1 GND              D_GROUND
 *  2 RDCLK'           A_READ_CLOCK
 *  3 WRDATA'          B_WRITE_DATA_AND_CLOCK
 *  4 SRWRDY'          F_S_R_W
 *  5 DISK             L_SELECT_LINE_UNIT_1
 *  6 CYL(7)'          N_CYL_7
 *  7 DISK'            R_SELECT_LINE_UNIT_2
 *  8 CYL(2)'          T_CYL_2
 *  9 ???              V_SELECT_LINE_UNIT_3
 * 10 CYL(4)'          X_CYL_4
 * 11 CYL(0)'          Z_CYL_0
 * 12 CYL(1)'          BB_CYL_1
 * 13 CYL(3)'          FF_CYL_3
 * 14 ???              KK_BIT_2
 * 15 CYL(8)'          LL_CYL_8
 * 16 ADRACK'          NN_ADDX_ACKNOWLEDGE
 * 17 SKINC'           TT_SEEK_INCOMPLETE
 * 18 LAI'             XX_LOG_ADDX_INTERLOCK
 * 19 CYL(6)'          RR_CYL_6
 * 20 RESTOR'          VV_RESTORE
 * 21 ???              UU_BIT_16
 * 22 STROBE'          SS_STROBE
 * 23 ???              MM_BIT_8
 * 24 ???              KK_BIT_4
 * 25 ???              HH_WRITE_CHK
 * 26 WRTGATE'         EE_WRITE_GATE
 * 27 ???              CC_BIT_SECTOR_ADDX
 * 28 HEAD'            AA_HEAD_SELECT
 * 29 ???              Y_INDEX_MARK
 * 30 SECT(4)'         W_SECTOR_MARK
 * 31 READY'           U_FILE_READY
 * 32 ???              S_PSEUDO_SECTOR_MARK
 * 33 ???              P_WRITE_PROTECT_IND
 * 34 ???              H_WRITE_PROTECT_INPUT_ATTENTION
 * 35 ERGATE'          K_ERASE_GATE
 * 36 ???              M_HIGH_DENSITY
 * 37 CYL(5)'          J_CYL_5
 * 38 RDDATA'          C_READ_DATA
 * 39 RDGATE'          E_READ_GATE
 * 40 GND              ??
 * </PRE>
 */

diablo_hd_device::diablo_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DIABLO_HD, "Diablo Disk", tag, owner, clock, "diablo_hd", __FILE__),
#if DIABLO_DEBUG
	m_log_level(8),
#endif
	m_diablo31(true),
	m_unit(0),
	m_packs(1),
	m_rotation_time(),
	m_sector_time(),
	m_sector_mark_0_time(),
	m_sector_mark_1_time(),
	m_bit_time(),
	m_s_r_w_0(1),
	m_ready_0(1),
	m_sector_mark_0(1),
	m_addx_acknowledge_0(1),
	m_log_addx_interlock_0(1),
	m_seek_incomplete_0(1),
	m_egate_0(1),
	m_wrgate_0(1),
	m_rdgate_0(1),
	m_cylinders(DIABLO_CYLINDERS),
	m_pages(DIABLO_PAGES),
	m_seekto(0),
	m_restore(0),
	m_cylinder(-1),
	m_head(-1),
	m_sector(-1),
	m_page(-1),
	m_bits(nullptr),
	m_rdfirst(-1),
	m_rdlast(-1),
	m_wrfirst(-1),
	m_wrlast(-1),
	m_sector_callback_cookie(nullptr),
	m_sector_callback(nullptr),
	m_timer(nullptr),
	m_image(nullptr),
	m_handle(nullptr),
	m_disk(nullptr)
{
	memset(m_description, 0x00, sizeof(m_description));
}

/**
 * @brief diablo_hd_device destructor
 * Free all m_cache and m_bits pages and the arrays
 */
diablo_hd_device::~diablo_hd_device()
{
}

#if DIABLO_DEBUG
template <typename Format, typename... Params>
void diablo_hd_device::logprintf(int level, Format &&fmt, Params &&... args)
{
	if (level <= m_log_level)
		logerror(std::forward<Format>(fmt), std::forward<Params>(args)...);
}
#endif

void diablo_hd_device::set_sector_callback(void *cookie, void (*callback)(void *, int))
{
	if (m_sector_callback_cookie == cookie && m_sector_callback == callback)
		return;
	LOG_DRIVE((0,"[DHD%u] cookie=%p callback=%p\n", m_unit, cookie, (void *)callback));
	m_sector_callback_cookie = cookie;
	m_sector_callback = callback;
}

#define DIABLO31_ROTATION_TIME attotime::from_usec(39900)       //!< DIABLO 31 rotation time is approx. 40ms
#define DIABLO31_SECTOR_TIME attotime::from_usec(39900/12)      //!< DIABLO 31 sector time
/**
 * @brief DIABLO 31 bit clock is 3330kHz ~= 300ns per bit
 * ~= 133333 bits/track (?)
 * ~= 11111 bits/sector
 * ~= 347 words/sector
 */
#define DIABLO31_BIT_TIME(bits) attotime::from_nsec(300*(bits))
#define DIABLO31_SECTOR_BITS    10432
#define DIABLO31_SECTOR_WORDS   347                             //!< DIABLO 31 possible sector words
#define DIABLO31_SECTOR_MARK_PULSE_PRE DIABLO31_BIT_TIME(16)    //!< pulse width of sector mark before the next sector begins
#define DIABLO31_SECTOR_MARK_PULSE_POST DIABLO31_BIT_TIME(16)   //!< pulse width of sector mark after the next sector began

#define DIABLO44_ROTATION_TIME attotime::from_usec(25000)       //!< DIABLO 44 rotation time is approx. 25ms
#define DIABLO44_SECTOR_TIME attotime::from_usec(25000/12)      //!< DIABLO 44 sector time
/**
 * @brief DIABLO 44 bit clock is 5000kHz ~= 200ns per bit
 * ~= 125184 bits/track (?)
 * ~= 10432 bits/sector
 * ~= 325 words/sector
 */
#define DIABLO44_BIT_TIME(bits) attotime::from_nsec(200*(bits))
#define DIABLO44_SECTOR_BITS    10432
#define DIABLO44_SECTOR_WORDS   325                             //!< DIABLO 44 possible sector words
#define DIABLO44_SECTOR_MARK_PULSE_PRE DIABLO44_BIT_TIME(16)    //!< pulse width of sector mark before the next sector begins
#define DIABLO44_SECTOR_MARK_PULSE_POST DIABLO44_BIT_TIME(16)   //!< pulse width of sector mark after the next sector began

#define MFROBL          34      //!< from the microcode: disk header preamble is 34 words
#define MFRRDL          21      //!< from the microcode: disk header read delay is 21 words
#define MIRRDL          4       //!< from the microcode: interrecord read delay is 4 words
#define MIROBL          3       //!< from the microcode: disk interrecord preamble is 3 words
#define MRPAL           3       //!< from the microcode: disk read postamble length is 3 words
#define MWPAL           5       //!< from the microcode: disk write postamble length is 5 words

#define GUARD_ZONE_BITS (16*32) //!< end of the guard zone at the beginning of a sector (wild guess!)

/**
 * @brief description of the sector layout (reverse engineered)
 * <PRE>
 *
 *                                   xx.x msec sector mark pulses
 * -+   +-------------------------------------------------------------------------------+   +--
 *  |   |                                                                               |   |
 *  +---+                                                                               +---+
 *
 *    |                                                                                   |
 *
 *    +------+----+------+-----+------+----+-------+-----+------+----+-------+-----+------+
 *    | PRE- |SYNC|HEADER|CKSUM| PRE- |SYNC| LABEL |CKSUM| PRE- |SYNC| DATA  |CKSUM| POST |
 *    |AMBLE1|  1 |      |  1  |AMBLE2|  2 |       |  2  |AMBLE3|  3 |       |  3  |AMBLE |
 *    +------+----+------+-----+------+----+-------+-----+------+----+-------+-----+------+
 *
 *    |                                                                                   |
 *
 *    +-----------------------------------------------------------------------------------+
 *    |                                                                                   |
 * ---+                                                                                   +----
 *      FORMAT WRITE GATE FOR INITIALIZING
 *    |                                                                                   |
 *
 *    |                                                    +------------------------------+
 *                                                         |                              |
 * ---|----------------------------------------------------+                              +----
 *      WRITE GATE FOR DATA XFER (*)
 *    |                                                                                   |
 *
 *    |                          +-----------------------+-+------------------------------+
 *                               |                       | | may be continuous (?)        |
 * ------------------------------+                       +-+                              +----
 * ???  WRITE GATE FOR LABEL AND DATA XFER (*)
 *    |                                                                                   |
 *
 *    |   +--------------------+   +---------------------+   +----------------------------+
 *        |                    |   |                     |   |                            |
 * -------+                    +---+                     +---+                            +----
 *      READ GATE FOR INITIALIZING OR DATA XFER (**)
 *
 *
 *  (*) Enable should be delayed 1 byte/word time from last bit of checks sum.
 *  (**) Read Gate should be enabled half way through the preamble area. This
 *       ensures reading a zero field for data separator synchronization.
 *
 * </PRE>
 */

#define DIABLO_PAGENO_WORDS 1       //!< number of words in a page number (this doesn't really belong here)
#define DIABLO_HEADER_WORDS 2       //!< number of words in a header (this doesn't really belong here)
#define DIABLO_LABEL_WORDS  8       //!< number of words in a label (this doesn't really belong here)
#define DIABLO_DATA_WORDS   256     //!< number of data words (this doesn't really belong here)
#define DIABLO_CKSUM_WORDS  1       //!< number of words for a checksum (this doesn't really belong here)

/**
 * @brief format of the cooked disk image sectors, i.e. pure data
 *
 * The available images are a multiple of 267 words (534 bytes) per sector,
 * 1 word page number
 * 2 words header
 * 8 words label
 * 256 words data
 */
typedef struct {
	UINT8 pageno[2*DIABLO_PAGENO_WORDS];    //!< sector page number
	UINT8 header[2*DIABLO_HEADER_WORDS];    //!< sector header words
	UINT8 label[2*DIABLO_LABEL_WORDS];      //!< sector label words
	UINT8 data[2*DIABLO_DATA_WORDS];        //!< sector data words
}   diablo_sector_t;

/**
 * @brief write a bit into an array of UINT32
 * @param bits pointer to array of bits
 * @param dst destination index
 * @param bit bit value
 * @return next destination index
 */
static inline size_t WRBIT(UINT32* bits, size_t dst, int bit)
{
	if (bit) {
		bits[(dst)/32] |= 1 << ((dst) % 32);
	} else {
		bits[(dst)/32] &= ~(1 << ((dst) % 32));
	}
	return ++dst;
}

/**
 * @brief read a bit from an array of UINT32
 * @param bits pointer to array of bits
 * @param src source index
 * @param bit reference to the bit to set
 * @return next source index
 */
static inline size_t RDBIT(UINT32* bits, size_t src, int& bit)
{
	bit = (bits[src/32] >> (src % 32)) & 1;
	return ++src;
}

/**
 * @brief calculate the sector from the logical block address and read it
 *
 * Modifies drive's page by calculating the logical
 * block address from cylinder, head, and sector.
 */
void diablo_hd_device::read_sector()
{
	/* If there's no drive, just reset the page number */
	if (!m_image) {
		LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d => no image\n", m_unit, m_cylinder, m_head, m_sector));
		m_page = -1;
		return;
	}
	if (m_cylinder < 0 || m_cylinder >= m_cylinders) {
		LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d => invalid cylinder\n", m_unit, m_cylinder, m_head, m_sector));
		m_page = -1;
		return;
	}
	if (m_head < 0 || m_head >= DIABLO_HEADS) {
		LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d => invalid head\n", m_unit, m_cylinder, m_head, m_sector));
		m_page = -1;
		return;
	}
	if (m_sector < 0 || m_sector >= DIABLO_SPT) {
		LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d => invalid sector\n", m_unit, m_cylinder, m_head, m_sector));
		m_page = -1;
		return;
	}
	/* calculate the new disk relative sector offset */
	m_page = DIABLO_PAGE(m_cylinder, m_head, m_sector);

	// already have the sector image?
	if (m_cache[m_page]) {
		LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d => page:%d is cached\n", m_unit, m_cylinder, m_head, m_sector, m_page));
		return;
	}

	if (m_disk) {
		// allocate a buffer for this page
		m_cache[m_page] = std::make_unique<UINT8[]>(sizeof(diablo_sector_t));
		// and read the page from the hard_disk image
		if (hard_disk_read(m_disk, m_page, m_cache[m_page].get())) {
			LOG_DRIVE((2,"[DHD%u]   CHS:%03d/%d/%02d => page:%d loaded\n", m_unit, m_cylinder, m_head, m_sector, m_page));
		} else {
			LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d => page:%d read failed\n", m_unit, m_cylinder, m_head, m_sector, m_page));
			m_cache[m_page] = nullptr;
		}
	} else {
		LOG_DRIVE((2,"[DHD%u]   no disk\n", m_unit));
	}
}

/**
 * @brief compute the checksum of a record
 *
 * @param src pointer to a record (header, label, data)
 * @param size size of the record in bytes
 * @param start start value for the checksum
 * @return returns the checksum of the record
 */
int diablo_hd_device::cksum(UINT8 *src, size_t size, int start)
{
	int sum = start;
	/* compute XOR of all words */
	for (size_t offs = 0; offs < size; offs += 2) {
		int word = src[size - 2 - offs] + 256 * src[size - 2 - offs + 1];
		sum ^= word;
	}
	return sum;
}

/**
 * @brief expand a series of clock bits and 0 data bits
 *
 * @param bits pointer to the sector bits
 * @param dst destination offset into bits (bit number)
 * @param size number of words to write
 * @return offset to next destination bit
 */
size_t diablo_hd_device::expand_zeroes(UINT32 *bits, size_t dst, size_t size)
{
	for (size_t offs = 0; offs < 32 * size; offs += 2) {
		dst = WRBIT(bits, dst, 1);      // write the clock bit
		dst = WRBIT(bits, dst, 0);      // write the 0 data bit
	}
	return dst;
}

/**
 * @brief expand a series of 0 words and write a final sync bit
 *
 * @param bits pointer to the sector bits
 * @param dst destination offset into bits (bit number)
 * @param size number of words to write
 * @return offset to next destination bit
 */
size_t diablo_hd_device::expand_sync(UINT32 *bits, size_t dst, size_t size)
{
	for (size_t offs = 0; offs < 32 * size - 2; offs += 2) {
		dst = WRBIT(bits, dst, 1);      // write the clock bit
		dst = WRBIT(bits, dst, 0);      // write the 0 data bit
	}
	dst = WRBIT(bits, dst, 1);  // write the final clock bit
	dst = WRBIT(bits, dst, 1);  // write the 1 data bit
	return dst;
}

/**
 * @brief expand a record of words into a array of bits at dst
 *
 * @param bits pointer to the sector bits
 * @param dst destination offset into bits (bit number)
 * @param field pointer to the record data (bytes)
 * @param size size of the record in bytes
 * @return offset to next destination bit
 */
size_t diablo_hd_device::expand_record(UINT32 *bits, size_t dst, UINT8 *field, size_t size)
{
	for (size_t offs = 0; offs < size; offs += 2) {
		int word = field[size - 2 - offs] + 256 * field[size - 2 - offs + 1];
		for (size_t bit = 0; bit < 16; bit++) {
			dst = WRBIT(bits, dst, 1);                  // write the clock bit
			dst = WRBIT(bits, dst, (word >> 15) & 1);   // write the data bit
			word <<= 1;
		}
	}
	return dst;
}

/**
 * @brief expand a record's checksum word to 32 bits
 *
 * @param bits pointer to the sector bits
 * @param dst destination offset into bits (bit number)
 * @param field pointer to the record data (bytes)
 * @param size size of the record in bytes
 * @return offset to next destination bit
 */
size_t diablo_hd_device::expand_cksum(UINT32 *bits, size_t dst, UINT8 *field, size_t size)
{
	int word = cksum(field, size, 0521);
	for (size_t bit = 0; bit < 32; bit += 2) {
		dst = WRBIT(bits, dst, 1);              // write the clock bit
		dst = WRBIT(bits, dst, (word >> 15) & 1);   // write the data bit
		word <<= 1;
	}
	return dst;
}

/**
 * @brief expand a sector into an array of clock and data bits
 *
 * @param page page number (0 to DRIVE_PAGES-1)
 * @return pointer to the newly allocated array of bits
 */
UINT32* diablo_hd_device::expand_sector()
{
	size_t dst;

	if (!m_bits)
		return nullptr;
	/* already expanded this sector? */
	if (m_bits[m_page])
		return m_bits[m_page];

	/* allocate a sector buffer */
	if (!m_cache[m_page]) {
		LOG_DRIVE((0,"[DHD%u]   no image for page #%d\n", m_unit, m_page));
		return nullptr;
	}
	diablo_sector_t *s = reinterpret_cast<diablo_sector_t *>(m_cache[m_page].get());

	/* allocate a bits image */
	UINT32 *bits = auto_alloc_array_clear(machine(), UINT32, 400);

	if (m_diablo31) {
		/* write sync bit after (MFROBL-MRPAL) words - 1 bit */
		dst = expand_sync(bits, 0, (MFROBL - MRPAL));
		dst = expand_record(bits, dst, s->header, sizeof(s->header));
		dst = expand_cksum(bits, dst, s->header, sizeof(s->header));

		/* write sync bit after 2 * MWPAL + 1 words - 1 bit */
		dst = expand_sync(bits, dst, 2 * MWPAL);
		dst = expand_record(bits, dst, s->label, sizeof(s->label));
		dst = expand_cksum(bits, dst, s->label, sizeof(s->label));

		/* write sync bit after 2 * MWPAL + 1 words - 1 bit */
		dst = expand_sync(bits, dst, 2 * MWPAL);
		dst = expand_record(bits, dst, s->data, sizeof(s->data));
		dst = expand_cksum(bits, dst, s->data, sizeof(s->data));

		/* fill MWPAL words of clock and 0 data bits */
		dst = expand_zeroes(bits, dst, MWPAL);
	} else {
		/* write sync bit after (MFROBL - MRPAL) words - 1 bit */
		dst = expand_sync(bits, 0, (MFROBL - MRPAL));
		dst = expand_record(bits, dst, s->header, sizeof(s->header));
		dst = expand_cksum(bits, dst, s->header, sizeof(s->header));

		/* write sync bit after 2 * MWPAL words - 1 bit */
		dst = expand_sync(bits, dst, 2 * MWPAL);
		dst = expand_record(bits, dst, s->label, sizeof(s->label));
		dst = expand_cksum(bits, dst, s->label, sizeof(s->label));

		/* write sync bit after 2 * MWPAL words - 1 bit */
		dst = expand_sync(bits, dst, 2 * MWPAL);
		dst = expand_record(bits, dst, s->data, sizeof(s->data));
		dst = expand_cksum(bits, dst, s->data, sizeof(s->data));

		/* fill MWPAL words of clock and 0 data bits */
		dst = expand_zeroes(bits, dst, MWPAL);
	}
	m_bits[m_page] = bits;

	LOG_DRIVE((0,"[DHD%u]   CHS:%03d/%d/%02d #%5d bits\n", m_unit, m_cylinder, m_head, m_sector, dst));
#if DIABLO_DEBUG
	dump_record(s->pageno, 0, sizeof(s->pageno), "pageno", 0);
	dump_record(s->header, 0, sizeof(s->header), "header", 0);
	dump_record(s->label, 0, sizeof(s->label), "label", 0);
	dump_record(s->data, 0, sizeof(s->data), "data", 1);
#endif
	return bits;
}

#if DIABLO_DEBUG
void diablo_hd_device::dump_ascii(UINT8 *src, size_t size)
{
	size_t offs;
	LOG_DRIVE((0," ["));
	for (offs = 0; offs < size; offs++) {
		char ch = (char)src[offs ^ 1];
		LOG_DRIVE((0, "%c", ch < 32 || ch > 126 ? '.' : ch));
	}
	LOG_DRIVE((0,"]\n"));
}


/**
 * @brief dump a record's contents
 *
 * @param src pointer to a record (header, label, data)
 * @param size size of the record in bytes
 * @param name name to print before the dump
 */
size_t diablo_hd_device::dump_record(UINT8 *src, size_t addr, size_t size, const char *name, int cr)
{
	size_t offs;
	LOG_DRIVE((0,"%s:", name));
	for (offs = 0; offs < size; offs += 2) {
		int word = src[offs] + 256 * src[offs + 1];
		if (offs % 16) {
			LOG_DRIVE((0," %06o", word));
		} else {
			if (offs > 0)
				dump_ascii(&src[offs-16], 16);
			LOG_DRIVE((0,"\t%05o: %06o", (addr + offs) / 2, word));
		}
	}
	if (offs % 16) {
		dump_ascii(&src[offs - (offs % 16)], offs % 16);
	} else {
		dump_ascii(&src[offs-16], 16);
	}
	if (cr) {
		LOG_DRIVE((0,"\n"));
	}
	return size;
}
#endif

/**
 * @brief find a sync bit in an array of clock and data bits
 *
 * @param bits pointer to the sector's bits
 * @param src source index into bits (bit number)
 * @param size number of words to scan for a sync word
 * @return next source index for reading
 */
size_t diablo_hd_device::squeeze_sync(UINT32 *bits, size_t src, size_t size)
{
	UINT32 accu = 0;
	/* hunt for the first 0x0001 word */
	for (size_t bitcount = 0, offs = 0; offs < size; /* */) {
		/*
		 * accumulate clock and data bits until we are
		 * on the clock bit boundary
		 */
		int bit;
		src = RDBIT(bits,src,bit);
		accu = (accu << 1) | bit;
		/*
		 * look for 15 alternating clocks and 0-bits
		 * and the 16th clock with a 1-bit
		 */
		if (accu == 0xaaaaaaab)
			return src;
		if (++bitcount == 32) {
			bitcount = 0;
			offs++;
		}
	}
	/* return if no sync found within size*32 clock and data bits */
	LOG_DRIVE((0,"[DHD%u]   no sync within %d words\n", m_unit, size));
	return src;
}

/**
 * @brief find a 16 x 0 bits sequence in an array of clock and data bits
 *
 * @param bits pointer to the sector's bits
 * @param src source index into bits (bit number)
 * @param size number of words to scan for a sync word
 * @return next source index for reading
 */
size_t diablo_hd_device::squeeze_unsync(UINT32 *bits, size_t src, size_t size)
{
	UINT32 accu = 0;
	/* hunt for the first 0 word (16 x 0 bits) */
	for (size_t bitcount = 0, offs = 0; offs < size; /* */) {
		/*
		 * accumulate clock and data bits until we are
		 * on the clock bit boundary
		 */
		int bit;
		src = RDBIT(bits,src,bit);
		accu = (accu << 1) | bit;
		/*
		 * look for 16 alternating clocks and 0 data bits
		 */
		if (accu == 0xaaaaaaaa)
			return src;
		if (++bitcount == 32) {
			bitcount = 0;
			offs++;
		}
	}
	/* return if no sync found within size*32 clock and data bits */
	LOG_DRIVE((0,"[DHD%u]   no unsync within %d words\n", m_unit, size));
	return src;
}

/**
 * @brief squeeze an array of clock and data bits into a sector's record
 *
 * @param bits pointer to the sector's bits
 * @param src source index into bits (bit number)
 * @param field pointer to the record data (bytes)
 * @param size size of the record in bytes
 * @return next source index for reading
 */
size_t diablo_hd_device::squeeze_record(UINT32 *bits, size_t src, UINT8 *field, size_t size)
{
	UINT32 accu = 0;
	for (size_t bitcount = 0, offs = 0; offs < size; /* */) {
		int bit;
		src = RDBIT(bits,src,bit);      // skip clock
		assert(bit == 1);
		src = RDBIT(bits,src,bit);      // get data bit
		accu = (accu << 1) | bit;
		bitcount += 2;
		if (bitcount == 32) {
			/* collected a word */
			field[size - 2 - offs + 0] = accu % 256;
			field[size - 2 - offs + 1] = accu / 256;
			offs += 2;
			bitcount = 0;
		}
	}
	return src;
}

/**
 * @brief squeeze an array of 32 clock and data bits into a checksum word
 *
 * @param bits pointer to the sector's bits
 * @param src source index into bits (bit number)
 * @param cksum pointer to an int to receive the checksum word
 * @return next source index for reading
 */
size_t diablo_hd_device::squeeze_cksum(UINT32 *bits, size_t src, int *cksum)
{
	UINT32 accu = 0;

	for (size_t bitcount = 0; bitcount < 32; bitcount += 2) {
		int bit;
		src = RDBIT(bits,src,bit);      // skip clock
		assert(bit == 1);
		src = RDBIT(bits,src,bit);      // get data bit
		accu = (accu << 1) | bit;
	}

	/* set the cksum to the extracted word */
	*cksum = accu;
	return src;
}

/**
 * @brief squeeze a array of clock and data bits into a sector's data
 *
 * Find and squeeze header, label and data fields and verify for
 * zero checksums, starting with a value of 0521.
 * Write the page back to the media and free the bitmap
 */
void diablo_hd_device::squeeze_sector()
{
	diablo_sector_t *s;
	size_t src;
	int cksum_header, cksum_label, cksum_data;

	if (m_rdfirst >= 0) {
		LOG_DRIVE((0, "[DHD%u]  READ CHS:%03d/%d/%02d bit#%d ... bit#%d\n",
					m_unit, m_cylinder, m_head, m_sector, m_rdfirst, m_rdlast));
	}
	m_rdfirst = -1;
	m_rdlast = -1;

	/* not written to, just drop it now */
	if (m_wrfirst < 0) {
		m_wrfirst = -1;
		m_wrlast = -1;
		return;
	}

	/* did write into the next sector (?) */
	if (m_wrlast > m_wrfirst && m_wrlast < 256) {
		m_wrfirst = -1;
		m_wrlast = -1;
		return;
	}

	if (m_wrfirst >= 0) {
		LOG_DRIVE((0, "[DHD%u]  WRITE CHS:%03d/%d/%02d bit#%d ... bit#%d\n",
					m_unit, m_cylinder, m_head, m_sector, m_wrfirst, m_wrlast));
	}
	m_wrfirst = -1;
	m_wrlast = -1;

	if (m_page < 0 || m_page >= m_pages) {
		LOG_DRIVE((0,"[DHD%u]   page not set\n", m_unit));
		return;
	}

	if (!m_cache[m_page]) {
		LOG_DRIVE((0,"[DHD%u]   no image\n", m_unit));
		return;
	}

	/* no bits to write? */
	if (!m_bits[m_page]) {
		LOG_DRIVE((0,"[DHD%u]   no bits\n", m_unit));
		return;
	}
	UINT32 *bits = m_bits[m_page];

	// pointer to sector buffer
	s = reinterpret_cast<diablo_sector_t *>(m_cache[m_page].get());

	// zap the sector first
	memset(s, 0, sizeof(*s));

	src = MFRRDL * 32;
	src = squeeze_unsync(bits, src, 40);        // skip first words and garbage until 0 bits are coming in
	src = squeeze_sync(bits, src, 40);          // sync on header preamble
	LOG_DRIVE((0,"[DHD%u]   header sync bit #%5d\n", m_unit, src));
	src = squeeze_record(bits, src, s->header, sizeof(s->header));
	LOG_DRIVE((0,"[DHD%u]   header CRC bit #%5d\n", m_unit, src));
	src = squeeze_cksum(bits, src, &cksum_header);
#if DIABLO_DEBUG
	dump_record(s->header, 0, sizeof(s->header), "header", 0);
#endif

	src = squeeze_unsync(bits, src, 40);        // skip garbage until 0 bits are coming in
	src = squeeze_sync(bits, src, 40);          // sync on label preamble
	LOG_DRIVE((0,"[DHD%u]   label sync bit #%5d\n", m_unit, src));
	src = squeeze_record(bits, src, s->label, sizeof(s->label));
	LOG_DRIVE((0,"[DHD%u]   label CRC bit #%5d\n", m_unit, src));
	src = squeeze_cksum(bits, src, &cksum_label);
#if DIABLO_DEBUG
	dump_record(s->label, 0, sizeof(s->label), "label", 0);
#endif

	src = squeeze_unsync(bits, src, 40);        // skip garbage until 0 bits are coming in
	src = squeeze_sync(bits, src, 40);          // sync on data preamble
	LOG_DRIVE((0,"[DHD%u]   data sync bit #%5d\n", m_unit, src));
	src = squeeze_record(bits, src, s->data, sizeof(s->data));
	LOG_DRIVE((0,"[DHD%u]   data CRC bit #%5d\n", m_unit, src));
	src = squeeze_cksum(bits, src, &cksum_data);
#if DIABLO_DEBUG
	dump_record(s->data, 0, sizeof(s->data), "data", 1);
#endif
	LOG_DRIVE((0,"[DHD%u]   postamble bit #%5d\n", m_unit, src));

	/* The checksum start value always seems to be 0521 */
	cksum_header ^= cksum(s->header, sizeof(s->header), 0521);
	cksum_label ^= cksum(s->label, sizeof(s->label), 0521);
	cksum_data ^= cksum(s->data, sizeof(s->data), 0521);

	if (cksum_header || cksum_label || cksum_data) {
#if DIABLO_DEBUG
		LOG_DRIVE((0,"[DHD%u]   cksum check - header:%06o label:%06o data:%06o\n", m_unit, cksum_header, cksum_label, cksum_data));
#endif
	}
	auto_free(machine(), m_bits[m_page]);
	m_bits[m_page] = nullptr;

	if (m_disk) {
		if (!hard_disk_write(m_disk, m_page, m_cache[m_page].get())) {
			LOG_DRIVE((0,"[DHD%u]   write failed for page #%d\n", m_unit, m_page));
		}
	} else {
		LOG_DRIVE((2,"[DHD%u]   no disk\n", m_unit));
	}
}

/**
 * @brief return number of bit clocks for a sector (clock and data)
 * @return number of bitclks for a sector
 */
int diablo_hd_device::bits_per_sector() const
{
	return m_diablo31 ? DIABLO31_SECTOR_BITS : DIABLO44_SECTOR_BITS;
}

/**
 * @brief return a pointer to a drive's description
 * @return a pointer to the string description
 */
const char* diablo_hd_device::description() const
{
	return m_description;
}

/**
 * @brief return the number of a drive unit
 * @return the unit number of this instance
 */
int diablo_hd_device::unit() const
{
	return m_unit;
}

/**
 * @brief return the time for a full rotation
 * @return the time for a full track rotation in atto seconds
 */
attotime diablo_hd_device::rotation_time() const
{
	return m_rotation_time;
}

/**
 * @brief return the time for a sector
 * @return the time for a sector in atto seconds
 */
attotime diablo_hd_device::sector_time() const
{
	return m_sector_time;
}

/**
 * @brief return the time for a data bit
 * @return the time in atto seconds per bit clock
 */
attotime diablo_hd_device::bit_time() const
{
	return m_bit_time;
}

/**
 * @brief return the seek/read/write status of a drive
 * @return the seek/read/write status for the drive unit (0:active 1:inactive)
 */
int diablo_hd_device::get_seek_read_write_0() const
{
	return m_s_r_w_0;
}

/**
 * @brief return the ready status of a drive
 * @return the ready status for the drive unit (0:ready 1:not ready)
 */
int diablo_hd_device::get_ready_0() const
{
	return m_ready_0;
}

/**
 * @brief return the current sector mark status of a drive
 *
 * The sector mark is derived from the offset into the current sector.
 * It is deasserted except for a short time (a few micro seconds)
 * around each new sector.
 *
 * @return the current sector mark for the drive (0:active 1:inactive)
 */
int diablo_hd_device::get_sector_mark_0() const
{
	/* no sector marks while seeking (?) */
	if (m_s_r_w_0)
		return 1;

	/* return the sector mark */
	return m_sector_mark_0;
}

/**
 * @brief return the address acknowledge state
 * @return address acknowledge state (0:active 1:inactive)
 */
int diablo_hd_device::get_addx_acknowledge_0() const
{
	return m_addx_acknowledge_0;
}

/**
 * @brief return the log address interlock state
 * @return log address interlock state (0:active 1:inactive)
 */
int diablo_hd_device::get_log_addx_interlock_0() const
{
	return m_log_addx_interlock_0;
}

/**
 * @brief return the seek incomplete state
 * @return address acknowledge state (0:active 1:inactive)
 */
int diablo_hd_device::get_seek_incomplete_0() const
{
	return m_seek_incomplete_0;
}

/**
 * @brief return the current cylinder of a drive unit
 *
 * This is a convenience function.
 * There is no such signal on the BUS.
 *
 * Note: The bus lines are active low
 * The value on the BUS needs an XOR with DIABLO_CYLINDER_MASK
 * to resemble the physical line levels.
 *
 * @return current cylinder number for the drive
 */
int diablo_hd_device::get_cylinder() const
{
	return m_cylinder;
}

/**
 * @brief return the current head of a drive unit
 *
 * This is a convenience function.
 * There is no such signal on the BUS.
 *
 * Note: The bus lines are active low
 * The value on the BUS needs an XOR with DIABLO_HEAD_MASK
 * to resemble the physical line levels.
 *
 * @return currently selected head for the drive
 */
int diablo_hd_device::get_head() const
{
	return m_head;
}

/**
 * @brief return the current sector of a drive unit
 *
 * The current sector number is derived from the time since the
 * most recent track rotation started.
 * It counts modulo DIABLO_SPT (12).
 *
 * Note: The bus lines are active low
 * The value on the BUS needs an XOR with DIABLO_SECTOR_MASK
 * to resemble the physical line levels.
 *
 * @return current sector for the drive
 */
int diablo_hd_device::get_sector() const
{
	return m_sector;
}

/**
 * @brief return the current page of a drive unit
 *
 * This is a convenience function.
 * There is no such signal on the BUS.
 *
 * The current page number is derived from the cylinder,
 * head, and sector numbers.
 *
 * @return the current page for the drive
 */
int diablo_hd_device::get_page() const
{
	return m_page;
}

/**
 * @brief select a drive unit
 *
 * Selecting a drive unit updates the ready status
 *
 * @param unit unit number
 */
void diablo_hd_device::select(int unit)
{
	assert(unit == m_unit); // this drive is selected

	if (m_disk) {
		m_ready_0 = 0;                  // it is ready
		m_s_r_w_0 = 0;                  // and can take seek/read/write commands
		m_addx_acknowledge_0 = 0;       // assert address acknowledge (?)
		m_log_addx_interlock_0 = 1;     // deassert log address interlock (?)
		LOG_DRIVE((1,"[DHD%u]   select unit:%d ready\n", m_unit, unit));
		read_sector();
	} else {
		m_ready_0 = 1;                  // it is not ready (?)
		m_s_r_w_0 = 1;                  // can't take seek/read/write commands (?)
		m_addx_acknowledge_0 = 0;       // assert address acknowledge (?)
		m_log_addx_interlock_0 = 1;     // deassert log address interlock (?)
		LOG_DRIVE((1,"[DHD%u]   select unit:%d not ready (no image)\n", m_unit, unit));
	}
}

/**
 * @brief set the selected head
 * @param head head number
 */
void diablo_hd_device::set_head(int head)
{
	if ((head & DIABLO_HEAD_MASK) != m_head) {
		m_head = head & DIABLO_HEAD_MASK;
		LOG_DRIVE((0,"[DHD%u]   select head:%d\n", m_unit, m_head));
	}
}

/**
 * @brief set the cylinder number to seek to
 *
 * This defines the cylinder to seek when the
 * STROBE line is pulsed.
 *
 * @param cylinder cylinder number (bus lines CYL[0-9])
 */
void diablo_hd_device::set_cylinder(int cylinder)
{
	if ((cylinder & DIABLO_CYLINDER_MASK) != m_seekto) {
		m_seekto = cylinder & DIABLO_CYLINDER_MASK;
		LOG_DRIVE((0,"[DHD%u]   seek to cylinder:%d\n", m_unit, m_seekto));
	}
}

/**
 * @brief set the restore line
 *
 * If the restore line is asserted when the
 * STROBE line is pulsed, the drive seeks
 * towards cylinder 0.
 *
 * @param restore state of the restore line
 */
void diablo_hd_device::set_restore(int restore)
{
	if ((restore & 1) != m_restore) {
		m_restore = restore & 1;
		LOG_DRIVE((0,"[DHD%u]   restore:%d\n", m_unit, m_restore));
	}
}

/**
 * @brief strobe a seek operation
 *
 * Seek to the specified cylinder m_seekto,
 * or restore to cylinder 0, if m_restore is set.
 *
 * @param strobe current level of the strobe signal (for edge detection)
 */
void diablo_hd_device::set_strobe(int strobe)
{
	int seekto = m_restore ? 0 : m_seekto;
	if (strobe) {
		LOG_DRIVE((1,"[DHD%u]   STROBE end of interlock\n", m_unit));
		// deassert the log address interlock
		m_log_addx_interlock_0 = 1;
		return;
	}

	// assert the log address interlock
	m_log_addx_interlock_0 = 0;

	if (seekto == m_cylinder) {
		LOG_DRIVE((1,"[DHD%u]   STROBE to cylinder %d acknowledge\n", m_unit, seekto));
		m_addx_acknowledge_0 = 0;   // address acknowledge, if cylinder is reached
		m_seek_incomplete_0 = 1;    // reset seek incomplete
		return;
	}
	// assert the seek-read-write signal
	m_s_r_w_0 = 0;

	bool complete = true;
	if (seekto < m_cylinder) {
		m_cylinder--;                   // previous cylinder
		if (m_cylinder < 0) {
			m_cylinder = 0;
			complete = false;
		}
	}
	if (seekto > m_cylinder) {
		/* increment cylinder */
		m_cylinder++;
		if (m_cylinder >= m_cylinders) {
			m_cylinder = m_cylinders - 1;
			complete = false;
		}
	}
	if (complete) {
		LOG_DRIVE((1,"[DHD%u]   STROBE to cylinder %d (now %d) - interlock\n", m_unit, seekto, m_cylinder));
		m_addx_acknowledge_0 = 1;   // deassert address acknowledge signal
		m_seek_incomplete_0 = 1;    // deassert seek incomplete signal
		read_sector();
	} else {
		m_log_addx_interlock_0 = 0; // deassert the log address interlock signal
		m_seek_incomplete_0 = 1;    // deassert seek incomplete signal
		m_addx_acknowledge_0 = 0;   // assert address acknowledge signal
		LOG_DRIVE((1,"[DHD%u]   STROBE to cylinder %d incomplete\n", m_unit, seekto));
	}
}

/**
 * @brief set the drive erase gate
 * @param gate value of erase gate
 */
void diablo_hd_device::set_egate(int gate)
{
	m_egate_0 = gate & 1;
}

/**
 * @brief set the drive write gate
 * @param gate value of write gate
 */
void diablo_hd_device::set_wrgate(int gate)
{
	m_wrgate_0 = gate & 1;
}

/**
 * @brief set the drive read gate
 * @param gate value of read gate
 */
void diablo_hd_device::set_rdgate(int gate)
{
	m_rdgate_0 = gate & 1;
}

/**
 * @brief write the sector relative bit at index
 *
 * The disk controller writes a combined clock and data pulse to one output
 * <PRE>
 * Encoding of binary 01011
 *
 *   clk   data  clk   data  clk   data  clk   data  clk   data
 *   0     1     2     3     4     5     6     7     8     9
 *   +--+        +--+  +--+  +--+        +--+  +--+  +--+  +--+  +--
 *   |  |        |  |  |  |  |  |        |  |  |  |  |  |  |  |  |
 * --+  +--------+  +--+  +--+  +--------+  +--+  +--+  +--+  +--+
 * </PRE>
 *
 * @param index relative index of bit/clock into sector
 * @param wrdata write data clock or bit
 */
void diablo_hd_device::wr_data(int index, int wrdata)
{
	if (m_wrgate_0) {
		LOG_DRIVE((0,"[DHD%u]   index=%d wrgate not asserted\n", m_unit, index));
		return; // write gate is not asserted (active 0)
	}

	if (index < 0 || index >= bits_per_sector()) {
		LOG_DRIVE((0,"[DHD%u]   index=%d out of range\n", m_unit, index));
		return; // don't write before or beyond the sector
	}

	if (-1 == m_page) {
		LOG_DRIVE((0,"[DHD%u]   invalid page\n", m_unit));
		return; // invalid page
	}

	UINT32 *bits = expand_sector();
	if (!bits) {
		LOG_DRIVE((0,"[DHD%u]   no bits\n", m_unit));
		return; // invalid unit
	}

	if (-1 == m_wrfirst)
		m_wrfirst = index;

	LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d index #%d bit:%d\n", m_unit, m_cylinder, m_head, m_sector, index, wrdata));

	if (index < GUARD_ZONE_BITS) {
		/* don't write in the guard zone (?) */
	} else {
		WRBIT(bits,index,wrdata);
	}
	m_wrlast = index;
}

/**
 * @brief read the sector relative bit at index
 *
 * Note: this is a gross hack to allow the controller pulling bits
 * at its will, rather than clocking them with the drive's RDCLK-
 *
 * @param index is the sector relative bit index
 * @return returns the sector's bit by index
 */
int diablo_hd_device::rd_data(int index)
{
	int bit = 0;

	if (m_rdgate_0) {
		LOG_DRIVE((1,"[DHD%u]   index=%d rdgate not asserted\n", m_unit, index));
		return 0;   // read gate is not asserted (active 0)
	}

	if (index < 0 || index >= bits_per_sector()) {
		LOG_DRIVE((0,"[DHD%u]   index=%d out of range\n", m_unit, index));
		return 1;   // don't read before or beyond the sector
	}

	if (0 == m_sector_mark_0) {
		LOG_DRIVE((0,"[DHD%u]   read while sector mark is asserted\n", m_unit));
		return 1;   // no data while sector mark is asserted
	}

	if (-1 == m_page) {
		LOG_DRIVE((0,"[DHD%u]   invalid page\n", m_unit));
		return 1;   // invalid unit
	}

	UINT32 *bits = expand_sector();
	if (!bits) {
		LOG_DRIVE((0,"[DHD%u]   no bits\n", m_unit));
		return 1;   // invalid page
	}

	if (-1 == m_rdfirst)
		m_rdfirst = index;

	RDBIT(bits,index,bit);
	LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d index #%d bit:%d\n", m_unit, m_cylinder, m_head, m_sector, index, bit));
	m_rdlast = index;
	return bit;
}

/**
 * @brief get the sector relative clock at index
 *
 * Note: this is a gross hack to allow the controller pulling bits
 * at its will, rather than clocking them with the drive's RDCLK-
 *
 * @param index is the sector relative bit index
 * @return returns the sector's clock bit by index
 */
int diablo_hd_device::rd_clock(int index)
{
	int clk = 0;

	if (index < 0 || index >= bits_per_sector()) {
		LOG_DRIVE((0,"[DHD%u]   index out of range (%d)\n", m_unit, index));
		return 1;   // don't read before or beyond the sector
	}

	if (0 == m_sector_mark_0) {
		LOG_DRIVE((0,"[DHD%u]   read while sector mark is asserted\n", m_unit));
		return 1;   // no clock while sector mark is low (?)
	}

	if (-1 == m_page) {
		LOG_DRIVE((0,"[DHD%u]   invalid page\n", m_unit));
		return 1;   // invalid page
	}

	UINT32 *bits = expand_sector();
	if (!bits) {
		LOG_DRIVE((0,"[DHD%u]   no bits\n", m_unit));
		return 1;   // invalid unit
	}

	if (-1 == m_rdfirst)
		m_rdfirst = index;

	if (index & 1) {
		// clock bits are on even bit positions only
		clk = 0;
	} else if (bits) {
		RDBIT(bits,index,clk);
	} else {
		clk = 0;
	}
	LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d index #%d clk:%d\n", m_unit, m_cylinder, m_head, m_sector, index, clk));
	m_rdlast = index;
	return clk ^ 1;
}

/**
 * @brief deassert the sector mark
 *
 */
void diablo_hd_device::sector_mark_1()
{
	LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d sector_mark_0=1\n", m_unit, m_cylinder, m_head, m_sector));
	m_sector_mark_0 = 1;    // deassert sector mark (set to 1)
}

/**
 * @brief assert the sector mark and read the next sector
 *
 * Assert the sector mark and reset the read and write
 * first and last bit indices.
 * Increment the sector number, wrap and read the
 * next sector from the media.
 */
void diablo_hd_device::sector_mark_0()
{
	LOG_DRIVE((9,"[DHD%u]   CHS:%03d/%d/%02d sector_mark_0=0\n", m_unit, m_cylinder, m_head, m_sector));

	// HACK: deassert wrgate
	//  m_wrgate_0 = 1;

	squeeze_sector();       // squeeze previous sector bits, if it was written to
	m_sector_mark_0 = 0;    // assert sector mark (set to 0)
	// reset read and write bit locations
	m_rdfirst = -1;
	m_rdlast = -1;
	m_wrfirst = -1;
	m_wrlast = -1;

	// count up the sector number
	m_sector = (m_sector + 1) % DIABLO_SPT;
	read_sector();
}

void diablo_hd_device::device_start()
{
	m_image = static_cast<diablo_image_device *>(subdevice("drive"));

	m_packs = 1;        // FIXME: get from configuration?
	m_unit = strstr(m_image->tag(), "diablo0") ? 0 : 1;
	m_timer = timer_alloc(1, nullptr);
}

void diablo_hd_device::device_reset()
{
	// free previous page cache
	if (m_cache) {
		for (int page = 0; page < m_pages; page++)
			if (m_cache[page])
				m_cache[page] = nullptr;
	}
	// free previous bits cache
	if (m_bits) {
		for (int page = 0; page < m_pages; page++)
			if (m_bits[page])
				auto_free(machine(), m_bits[page]);
		auto_free(machine(), m_bits);
		m_bits = nullptr;
	}
	m_handle = m_image->get_chd_file();
	m_diablo31 = true;  // FIXME: get from m_handle meta data?
	m_disk = m_image->get_hard_disk_file();
	if (m_diablo31) {
		snprintf(m_description, sizeof(m_description), "DIABLO31");
		m_rotation_time = DIABLO31_ROTATION_TIME;
		m_sector_time = DIABLO31_ROTATION_TIME / DIABLO_SPT;
		m_sector_mark_0_time = DIABLO31_SECTOR_MARK_PULSE_PRE;
		m_sector_mark_1_time = DIABLO31_SECTOR_MARK_PULSE_PRE;
		m_bit_time = DIABLO31_BIT_TIME(1);
		m_cylinders = DIABLO_CYLINDERS;
		m_pages = DIABLO_PAGES;
	} else {
		snprintf(m_description, sizeof(m_description), "DIABLO44");
		m_rotation_time = DIABLO44_ROTATION_TIME;
		m_sector_time = DIABLO44_ROTATION_TIME / DIABLO_SPT;
		m_sector_mark_0_time = DIABLO44_SECTOR_MARK_PULSE_PRE;
		m_sector_mark_1_time = DIABLO44_SECTOR_MARK_PULSE_PRE;
		m_bit_time = DIABLO44_BIT_TIME(1);
		m_cylinders = 2 * DIABLO_CYLINDERS;
		m_pages = 2 * DIABLO_PAGES;
	}
	LOG_DRIVE((0,"[DHD%u]   m_handle            : %p\n", m_unit, m_handle));
	LOG_DRIVE((0,"[DHD%u]   m_disk              : %p\n", m_unit, m_disk));
	LOG_DRIVE((0,"[DHD%u]   rotation time       : %.0fns\n", m_unit, m_rotation_time.as_double() * ATTOSECONDS_PER_NANOSECOND));
	LOG_DRIVE((0,"[DHD%u]   sector time         : %.0fns\n", m_unit, m_sector_time.as_double() * ATTOSECONDS_PER_NANOSECOND));
	LOG_DRIVE((0,"[DHD%u]   sector mark 0 time  : %.0fns\n", m_unit, m_sector_mark_0_time.as_double() * ATTOSECONDS_PER_NANOSECOND));
	LOG_DRIVE((0,"[DHD%u]   sector mark 1 time  : %.0fns\n", m_unit, m_sector_mark_1_time.as_double() * ATTOSECONDS_PER_NANOSECOND));
	LOG_DRIVE((0,"[DHD%u]   bit time            : %.0fns\n", m_unit, m_bit_time.as_double() * ATTOSECONDS_PER_NANOSECOND));

	m_s_r_w_0 = 1;                  // deassert seek/read/write ready
	m_ready_0 = 1;                  // deassert drive ready
	m_sector_mark_0 = 1;            // deassert sector mark
	m_addx_acknowledge_0 = 1;       // deassert drive address acknowledge
	m_log_addx_interlock_0 = 1;     // deassert drive log address interlock
	m_seek_incomplete_0 = 1;        // deassert drive seek incomplete

	// reset the disk drive's strobe info
	m_seekto = 0;
	m_restore = 0;
	// reset the disk drive's address
	m_cylinder = 0;
	m_head = 0;
	m_sector = 0;
	m_page = 0;

	// disable the erase, write and read gates
	m_egate_0 = 1;
	m_wrgate_0 = 1;
	m_rdgate_0 = 1;

	// reset read and write first and last indices
	m_wrfirst = -1;
	m_wrlast = -1;
	m_rdfirst = -1;
	m_rdlast = -1;

	if (!m_handle)
		return;
	// for units with a CHD assigned to them start the timer
	m_bits = auto_alloc_array_clear(machine(), UINT32*, m_pages);
	timer_set(m_sector_time - m_sector_mark_0_time, 1, 0);
	read_sector();
}

/**
 * @brief timer callback that is called thrice per sector in the rotation
 *
 * The timer is called three times at the events:
 * 0: sector mark goes active
 * 1: sector mark goes inactive
 * 2: in the middle of the active phase
 *
 * @param id timer id
 * @param arg argument supplied to timer_insert (unused)
 */
void diablo_hd_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_DRIVE((9,"[DHD%u]   TIMER id=%d param=%d ptr=%p @%.0fns\n", m_unit, id, param, ptr, timer.elapsed().as_double() * ATTOSECONDS_PER_NANOSECOND));
	if (!m_disk)
		return;

	switch (param) {
	case 0:
		// assert sector mark
		sector_mark_0();
		// next sector timer event is in the middle between sector_mark going 0 and back to 1
		timer.adjust(m_sector_mark_0_time, 1);
		break;
	case 1:
		/* call the sector_callback, if any */
		if (m_sector_callback)
			(void)(*m_sector_callback)(m_sector_callback_cookie, m_unit);
		// next sector timer event is deassert of sector_mark_0 (set to 1)
		timer.adjust(m_sector_mark_1_time, 2);
		break;
	case 2:
		// deassert sector mark
		sector_mark_1();
		// next sector timer event is sector_mark_0 for next sector
		timer.adjust(m_sector_time - m_sector_mark_0_time, 0);
		break;
	}
}

MACHINE_CONFIG_FRAGMENT( diablo_drive )
	MCFG_DIABLO_ADD("drive")
MACHINE_CONFIG_END

machine_config_constructor diablo_hd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( diablo_drive );
}

const device_type DIABLO_HD = &device_creator<diablo_hd_device>;
