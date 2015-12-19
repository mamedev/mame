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
#ifndef _A2MEM_H_
#define _A2MEM_H_
//! memory access mode
enum {
	ALTO2_MEM_NONE,
	ALTO2_MEM_ODD       = (1 << 0),
	ALTO2_MEM_RAM       = (1 << 1),
	ALTO2_MEM_REFRESH   = (1 << 2),
	ALTO2_MEM_INVALID   = (1 << 3)
};

struct {
	UINT32 size;                        //!< main memory size (64K or 128K)
	std::unique_ptr<UINT32[]> ram;                        //!< main memory organized as double-words
	std::unique_ptr<UINT8[]> hpb;                         //!< Hamming Code bits (6) and Parity bits (1) per double word
	UINT32 mar;                         //!< memory address register
	UINT32 rmdd;                        //!< read memory data double-word
	UINT32 wmdd;                        //!< write memory data double-word
	UINT16 md;                          //!< memory data register
	UINT64 cycle;                       //!< cycle when the memory address register was loaded

	/**
	 * @brief memory access under the way if non-zero
	 * 0: no memory access (MEM_NONE)
	 * 1: invalid
	 * 2: memory access even word (MEM_RAM)
	 * 3: memory access odd word (MEM_RAM | MEM_ODD)
	 */
	int access;
	bool error;                         //!< non-zero after a memory error was detected
	UINT32 mear;                        //!< memory error address register
	UINT16 mesr;                        //!< memory error status register
	UINT16 mecr;                        //!< memory error control register
}   m_mem;

/**
 * @brief check if memory address register load is yet possible
 * suspend if accessing RAM and previous MAR<- was less than 5 cycles ago
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
inline bool check_mem_load_mar_stall(UINT8 rsel) {
	if (ALTO2_MEM_NONE == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+5;
}

/**
 * @brief check if memory read is yet possible
 * MAR<- = cycle #1, earliest read at cycle #5, i.e. + 4
 *
 * 1.  MAR<- ANY
 * 2.  REQUIRED
 * 3.  SUSPEND
 * 4.  SUSPEND
 * 5.  whereever <-MD
 *
 * @return false, if memory can be read without wait cycle
 */
inline bool check_mem_read_stall() {
	if (ALTO2_MEM_NONE == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+4;
}

/**
 * @brief check if memory write is yet possible
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
	if (ALTO2_MEM_NONE == m_mem.access)
		return false;
	return cycle() < m_mem.cycle+2;
}


DECLARE_READ16_MEMBER ( mear_r );       //!< memory error address register read
DECLARE_READ16_MEMBER ( mesr_r );       //!< memory error status register read
DECLARE_WRITE16_MEMBER( mesr_w );       //!< memory error status register write (clear)
DECLARE_READ16_MEMBER ( mecr_r );       //!< memory error control register read
DECLARE_WRITE16_MEMBER( mecr_w );       //!< memory error control register write

//! read or write a memory double-word and caluclate its Hamming code
UINT32 hamming_code(int write, UINT32 dw_addr, UINT32 dw_data);

//! load the memory address register with some value
void load_mar(UINT8 rsel, UINT32 addr);

//! read memory or memory mapped I/O from the address in mar to md
UINT16 read_mem();

//! write memory or memory mapped I/O from md to the address in mar
void write_mem(UINT16 data);

//! debugger interface to read memory
UINT16 debug_read_mem(UINT32 addr);

//! debugger interface to write memory
void debug_write_mem(UINT32 addr, UINT16 data);

#if ALTO2_DEBUG
void watch_write(UINT32 addr, UINT32 data);
void watch_read(UINT32 addr, UINT32 data);
#endif

void init_memory();                             //!< initialize the memory system
void exit_memory();                             //!< deinitialize the memory system
void reset_memory();                            //!< reset the memory system
#endif // _A2MEM_H_
#endif  // ALTO2_DEFINE_CONSTANTS
