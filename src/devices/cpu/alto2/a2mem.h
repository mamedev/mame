// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory block (MEM)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#define ALTO2_RAM_SIZE          0200000                     //!< size of main memory in words
#define ALTO2_IO_PAGE_BASE      0177000                     //!< base address of the memory mapped io range
#define ALTO2_IO_PAGE_SIZE      0001000                     //!< size of the memory mapped io range

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef MAME_CPU_ALTO2_A2MEM_H
#define MAME_CPU_ALTO2_A2MEM_H
//! memory access mode
enum {
	ALTO2_MEM_NONE,
	ALTO2_MEM_ODD       = (1 << 0),
	ALTO2_MEM_RAM       = (1 << 1),
	ALTO2_MEM_LATCHED   = (1 << 2),
	ALTO2_MEM_REFRESH   = (1 << 3),
	ALTO2_MEM_INVALID   = (1 << 4)
};

struct {
	uint32_t size = 0;                  //!< main memory size (64K or 128K)
	std::unique_ptr<uint32_t []> ram;   //!< main memory organized as double-words
	std::unique_ptr<uint8_t []> hpb;    //!< Hamming Code bits (6) and Parity bits (1) per double word
	uint32_t mar = 0;                   //!< memory address register
	uint32_t rmdd = 0;                  //!< read memory data double-word
	uint32_t wmdd = 0;                  //!< write memory data double-word
	uint32_t md = 0;                    //!< memory data register
	uint64_t cycle = 0;                 //!< cycle when the memory address register was loaded

	/**
	 * @brief memory access under the way if non-zero
	 * 0: no memory access (MEM_NONE)
	 * 1: invalid
	 * 2: memory access even word (MEM_RAM)
	 * 3: memory access odd word (MEM_RAM | MEM_ODD)
	 */
	int access = 0;
	bool error = false;                 //!< non-zero after a memory error was detected
	uint32_t mear = 0;                  //!< memory error address register
	uint32_t mesr = 0;                  //!< memory error status register
	uint32_t mecr = 0;                  //!< memory error control register
}   m_mem;

/**
 * @brief Check if memory address register load is yet possible.
 * Suspend if accessing RAM and previous MAR<- was less than 5 cycles ago.
 *
 * 1.  MAR<- ANY
 * 2.  REQUIRED
 * 3.  MD<- whatever
 * 4.  SUSPEND
 * 5.  SUSPEND
 * 6.  MAR<- ANY
 *
 * @return false, if memory address can be loaded
 */
inline bool check_mem_load_mar_stall(uint8_t rsel) {
	if (ALTO2_MEM_NONE == m_mem.access || ALTO2_MEM_REFRESH == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+5;
}

/**
 * @brief Check if memory read is yet possible.
 * MAR<- = cycle #1, earliest read at cycle #5, i.e. + 4
 *
 * 1.  MAR<- ANY
 * 2.  REQUIRED
 * 3.  SUSPEND
 * 4.  SUSPEND
 * 5.  wherever <-MD
 *
 * @return false, if memory can be read without wait cycle
 */
inline bool check_mem_read_stall() {
	if (ALTO2_MEM_NONE == m_mem.access || ALTO2_MEM_REFRESH == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+4;
}

/**
 * @brief Check if memory write is yet possible.
 * MAR<- = cycle #1, earliest write at cycle #3, i.e. + 2
 *
 * 1.  MAR<- ANY
 * 2.  REQUIRED
 * 3.  OPTIONAL
 * 4.  MD<- whatever
 *
 * @return false, if memory can be written without wait cycle
 */
inline bool check_mem_write_stall() {
	if (ALTO2_MEM_NONE == m_mem.access || ALTO2_MEM_REFRESH == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+2;
}


uint16_t mear_r();           //!< memory error address register read
uint16_t mesr_r();           //!< memory error status register read
void mesr_w(uint16_t data);  //!< memory error status register write (clear)
uint16_t mecr_r();           //!< memory error control register read
void mecr_w(uint16_t data);  //!< memory error control register write

//! Read or write a memory double-word and calculate or compare its Hamming code.
uint32_t hamming_code(bool write, uint32_t dw_addr, uint32_t dw_data);

//! Load the memory address register with some value.
void load_mar(uint8_t rsel, uint32_t addr);

//! Read memory or memory mapped I/O from the address in mar to md.
uint16_t read_mem();

//! Write memory or memory mapped I/O from md to the address in mar.
void write_mem(uint16_t data);

//! Debugger interface to read memory.
uint16_t debug_read_mem(uint32_t addr);

//! Debugger interface to write memory.
void debug_write_mem(uint32_t addr, uint16_t data);

#if ALTO2_DEBUG
void watch_write(uint32_t addr, uint32_t data);
void watch_read(uint32_t addr, uint32_t data);
#endif

void init_memory();                             //!< initialize the memory system
void exit_memory();                             //!< deinitialize the memory system
void reset_memory();                            //!< reset the memory system
#endif // MAME_CPU_ALTO2_A2MEM_H
#endif  // ALTO2_DEFINE_CONSTANTS
