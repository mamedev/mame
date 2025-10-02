// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*************************************************************************
 *
 *      Portable Signetics 2650 cpu emulation
 *
 *  Version 1.2
 *  - changed to clock cycle counts from machine cycles
 *  - replaced cycle table with inline code (M_RET conditional case)
 *  - removed wrong distinct add/sub CC and OVF handling
 *  - cosmetics, readability
 *
 *************************************************************************/

#include "emu.h"
#include "s2650.h"

// define this to have some interrupt information logged
//#define VERBOSE 1
#include "logmacro.h"

// define this to enable Z80 mnemonics in the debugger
#define DEBUG_Z80   0

// define this to expand all EA calculations inline
#define INLINE_EA   1


DEFINE_DEVICE_TYPE(S2650, s2650_device, "s2650", "Signetics 2650")


s2650_device::s2650_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, S2650, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 15)
	, m_io_config("io", ENDIANNESS_BIG, 8, 8)
	, m_data_config("data", ENDIANNESS_BIG, 8, 1)
	, m_sense_handler(*this, 0)
	, m_flag_handler(*this)
	, m_intack_handler(*this, 0x00)
	, m_ppc(0), m_page(0), m_iar(0), m_ea(0), m_psl(0), m_psu(0), m_r(0)
	, m_halt(0), m_ir(0), m_irq_state(0)
	, m_debugger_temp(0)
{
	memset(m_reg, 0x00, sizeof(m_reg));
}

bool s2650_device::get_z80_mnemonics_mode() const
{
	return bool(DEBUG_Z80);
}

std::unique_ptr<util::disasm_interface> s2650_device::create_disassembler()
{
	return std::make_unique<s2650_disassembler>(this);
}

device_memory_interface::space_config_vector s2650_device::memory_space_config() const
{
	return space_config_vector
	{
		// Memory-mapped: M/~IO=1
		std::make_pair(AS_PROGRAM, &m_program_config),

		// Non-extended I/O: M/~IO=0 ADR13(~NE)=0 ADR14=D/~C
		// "The D/~C line can be used as a 1-bit device address in simple systems."
		// -- Signetics 2650 Microprocessor databook, page 41
		std::make_pair(AS_DATA,    &m_data_config),

		// Extended I/O: M/~IO=0 ADR13(E)=1 ADR14=Don't Care
		std::make_pair(AS_IO,      &m_io_config)
	};
}


// condition code changes for a byte
static const uint8_t ccc[0x100] =
{
	0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
};


/***************************************************************
 * macros for CPU registers/flags
 ***************************************************************/
#define PMSK    0x1fff          // mask page offset
#define PLEN    0x2000          // page length
#define PAGE    0x6000          // mask page
#define AMSK    0x7fff          // mask address range

// processor status lower
#define C       0x01            // carry flag
#define COM     0x02            // compare: 0 binary, 1 2s complement
#define OVF     0x04            // 2s complement overflow
#define WC      0x08            // with carry: use carry in arithmetic / rotate ops
#define RS      0x10            // register select 0: R0/R1/R2/R3 1: R0/R4/R5/R6
#define IDC     0x20            // inter digit carry: bit-3-to-bit-4 carry
#define CC      0xc0            // condition code

// processor status upper
#define SP      0x07            // stack pointer: indexing 8 15bit words
#define PSU34   0x18            // unused bits
#define II      0x20            // interrupt inhibit 0: allow, 1: inhibit
#define FO      0x40            // flag output
#define SI      0x80            // sense input

#define R0      m_reg[0]
#define R1      m_reg[1]
#define R2      m_reg[2]
#define R3      m_reg[3]


/***************************************************************
 * RDMEM
 * read memory byte from addr
 ***************************************************************/
#define RDMEM(addr) m_program.read_byte(addr)

inline void s2650_device::set_psu(uint8_t new_val)
{
	uint8_t old = m_psu;

	m_psu = new_val;
	if ((new_val ^ old) & FO)
		m_flag_handler((new_val & FO) ? 1 : 0);
}

inline uint8_t s2650_device::get_psu()
{
	if (!m_sense_handler.isunset())
	{
		if (m_sense_handler())
			m_psu |= SI;
		else
			m_psu &= ~SI;
	}

	return m_psu;
}

inline uint8_t s2650_device::get_sp()
{
	return (m_psu & SP);
}

inline void s2650_device::set_sp(uint8_t new_sp)
{
	m_psu = (m_psu & ~SP) | (new_sp & SP);
}

inline int s2650_device::check_irq_line()
{
	int cycles = 0;

	if (m_irq_state != CLEAR_LINE)
	{
		if ((m_psu & II) == 0)
		{
			if (m_halt)
			{
				m_halt = 0;
				m_iar = (m_iar + 1) & PMSK;
			}
			standard_irq_callback(0, m_page + m_iar);

			// Say hi
			int vector = m_intack_handler();

			// build effective address within first 8K page
			cycles += 9; // ZBSR
			m_ea = util::sext(vector, 7) & PMSK;
			if (vector & 0x80) // indirect bit set ?
			{
				int addr = m_ea;
				cycles += 6;

				// build indirect 32K address
				m_ea = RDMEM(addr) << 8;
				if (!(++addr & PMSK)) addr -= PLEN;
				m_ea = (m_ea + RDMEM(addr)) & AMSK;
			}
			LOG("S2650 interrupt to $%04x\n", m_ea);
			set_sp(get_sp() + 1);
			set_psu(m_psu | II);
			m_ras[get_sp()] = m_page + m_iar;
			m_page = m_ea & PAGE;
			m_iar  = m_ea & PMSK;
		}
	}

	return cycles;
}

/***************************************************************
 *
 * set condition code (zero,plus,minus) from result
 ***************************************************************/
#define SET_CC(result)                                          \
	m_psl = (m_psl & ~CC) | ccc[result & 0xff]

/***************************************************************
 * ROP
 * read next opcode
 ***************************************************************/
inline uint8_t s2650_device::ROP()
{
	uint8_t result = m_cprogram.read_byte(m_page + m_iar);
	m_iar = (m_iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * ARG
 * read next opcode argument
 ***************************************************************/
inline uint8_t s2650_device::ARG()
{
	uint8_t result = m_cprogram.read_byte(m_page + m_iar);
	m_iar = (m_iar + 1) & PMSK;
	return result;
}

/***************************************************************
 * _REL_EA
 * build effective address with relative addressing
 ***************************************************************/
#define _REL_EA(page)                                           \
{                                                               \
	uint8_t hr = ARG(); /* get 'holding register' */            \
	/* build effective address within current 8K page */        \
	m_ea = page + ((m_iar + util::sext(hr, 7)) & PMSK);         \
	if (hr & 0x80)                                              \
	{                                                           \
		/* indirect bit set ? */                                \
		int addr = m_ea;                                        \
		m_icount -= 6;                                          \
		/* build indirect 32K address */                        \
		m_ea = RDMEM(addr) << 8;                                \
		if ((++addr & PMSK) == 0) addr -= PLEN; /* page wrap */ \
		m_ea = (m_ea + RDMEM(addr)) & AMSK;                     \
	}                                                           \
}

/***************************************************************
 * _REL_ZERO
 * build effective address with zero relative addressing
 ***************************************************************/
#define _REL_ZERO(page)                                         \
{                                                               \
	uint8_t hr = ARG(); /* get 'holding register' */            \
	/* build effective address from 0 */                        \
	m_ea = (util::sext(hr, 7) & PMSK);                          \
	if (hr & 0x80)                                              \
	{                                                           \
		/* indirect bit set ? */                                \
		int addr = m_ea;                                        \
		m_icount -= 6;                                          \
		/* build indirect 32K address */                        \
		m_ea = RDMEM(addr) << 8;                                \
		if ((++addr & PMSK) == 0) addr -= PLEN; /* page wrap */ \
		m_ea = (m_ea + RDMEM(addr)) & AMSK;                     \
	}                                                           \
}

/***************************************************************
 * _ABS_EA
 * build effective address with absolute addressing
 ***************************************************************/
#define _ABS_EA()                                               \
{                                                               \
	uint8_t hr, dr;                                             \
	hr = ARG(); /* get 'holding register' */                    \
	dr = ARG(); /* get 'data bus register' */                   \
	/* build effective address within current 8K page */        \
	m_ea = m_page + (((hr << 8) + dr) & PMSK);                  \
	/* indirect addressing ? */                                 \
	if (hr & 0x80)                                              \
	{                                                           \
		int addr = m_ea;                                        \
		m_icount -= 6;                                          \
		/* build indirect 32K address */                        \
		/* build indirect 32K address */                        \
		m_ea = RDMEM(addr) << 8;                                \
		if ((++addr & PMSK) == 0) addr -= PLEN; /* page wrap */ \
		m_ea = (m_ea + RDMEM(addr)) & AMSK;                     \
	}                                                           \
	/* check indexed addressing modes */                        \
	switch (hr & 0x60)                                          \
	{                                                           \
		case 0x00: /* not indexed */                            \
			break;                                              \
		case 0x20: /* auto increment indexed */                 \
			m_reg[m_r] += 1;                                    \
			m_ea = (m_ea & PAGE) + ((m_ea + m_reg[m_r]) & PMSK);\
			m_r = 0; /* absolute addressing reg is R0 */        \
			break;                                              \
		case 0x40: /* auto decrement indexed */                 \
			m_reg[m_r] -= 1;                                    \
			m_ea = (m_ea & PAGE) + ((m_ea + m_reg[m_r]) & PMSK);\
			m_r = 0; /* absolute addressing reg is R0 */        \
			break;                                              \
		case 0x60: /* indexed */                                \
			m_ea = (m_ea & PAGE) + ((m_ea + m_reg[m_r]) & PMSK);\
			m_r = 0; /* absolute addressing reg is R0 */        \
			break;                                              \
	}                                                           \
}

/***************************************************************
 * _BRA_EA
 * build effective address with absolute addressing (branch)
 ***************************************************************/
#define _BRA_EA()                                               \
{                                                               \
	uint8_t hr, dr;                                             \
	hr = ARG(); /* get 'holding register' */                    \
	dr = ARG(); /* get 'data bus register' */                   \
	/* build address in 32K address space */                    \
	m_ea = ((hr << 8) + dr) & AMSK;                             \
	/* indirect addressing ? */                                 \
	if (hr & 0x80)                                              \
	{                                                           \
		int addr = m_ea;                                        \
		m_icount -= 6;                                          \
		/* build indirect 32K address */                        \
		m_ea = RDMEM(addr) << 8;                                \
		if ((++addr & PMSK) == 0) addr -= PLEN; /* page wrap */ \
		m_ea = (m_ea + RDMEM(addr)) & AMSK;                     \
	}                                                           \
}

/***************************************************************
 * SWAP_REGS
 * Swap registers r1-r3 with r4-r6 (the second set)
 * This is done everytime the RS bit in PSL changes
 ***************************************************************/
#define SWAP_REGS                                               \
{                                                               \
	uint8_t tmp;                                                \
	tmp = m_reg[1];                                             \
	m_reg[1] = m_reg[4];                                        \
	m_reg[4] = tmp;                                             \
	tmp = m_reg[2];                                             \
	m_reg[2] = m_reg[5];                                        \
	m_reg[5] = tmp;                                             \
	tmp = m_reg[3];                                             \
	m_reg[3] = m_reg[6];                                        \
	m_reg[6] = tmp;                                             \
}

/***************************************************************
 * M_BRR
 * Branch relative if cond is true
 ***************************************************************/
#define M_BRR(cond)                                             \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		REL_EA(m_page);                                         \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
	}                                                           \
	else                                                        \
	{                                                           \
		m_iar = (m_iar + 1) & PMSK;                             \
	}                                                           \
}

/***************************************************************
 * M_ZBRR
 * Branch relative to page zero
 ***************************************************************/
#define M_ZBRR()                                                \
{                                                               \
	REL_ZERO(0);                                                \
	m_page = m_ea & PAGE;                                       \
	m_iar  = m_ea & PMSK;                                       \
}

/***************************************************************
 * M_BRA
 * Branch absolute if cond is true
 ***************************************************************/
#define M_BRA(cond)                                             \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		BRA_EA();                                               \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
	}                                                           \
	else                                                        \
	{                                                           \
		m_iar = (m_iar + 2) & PMSK;                             \
	}                                                           \
}

/***************************************************************
 * M_BXA
 * Branch indexed absolute (EA + R3)
 ***************************************************************/
#define M_BXA()                                                 \
{                                                               \
	BRA_EA();                                                   \
	m_ea   = (m_ea + m_reg[3]) & AMSK;                          \
	m_page = m_ea & PAGE;                                       \
	m_iar  = m_ea & PMSK;                                       \
}

/***************************************************************
 * M_BSR
 * Branch to subroutine relative if cond is true
 ***************************************************************/
#define M_BSR(cond)                                             \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		REL_EA(m_page);                                         \
		set_sp(get_sp() + 1);                                   \
		m_ras[get_sp()] = m_page + m_iar;                       \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
	}                                                           \
	else                                                        \
	{                                                           \
		m_iar = (m_iar + 1) & PMSK;                             \
	}                                                           \
}

/***************************************************************
 * M_ZBSR
 * Branch to subroutine relative to page zero
 ***************************************************************/
#define M_ZBSR()                                                \
{                                                               \
	REL_ZERO(0);                                                \
	set_sp(get_sp() + 1);                                       \
	m_ras[get_sp()] = m_page + m_iar;                           \
	m_page = m_ea & PAGE;                                       \
	m_iar  = m_ea & PMSK;                                       \
}

/***************************************************************
 * M_BSA
 * Branch to subroutine absolute
 ***************************************************************/
#define M_BSA(cond)                                             \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		BRA_EA();                                               \
		set_sp(get_sp() + 1);                                   \
		m_ras[get_sp()] = m_page + m_iar;                       \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
	}                                                           \
	else                                                        \
	{                                                           \
		m_iar = (m_iar + 2) & PMSK;                             \
	}                                                           \
}

/***************************************************************
 * M_BSXA
 * Branch to subroutine indexed absolute (EA + R3)
 ***************************************************************/
#define M_BSXA()                                                \
{                                                               \
	BRA_EA();                                                   \
	m_ea = (m_ea + m_reg[3]) & AMSK;                            \
	set_sp(get_sp() + 1);                                       \
	m_ras[get_sp()] = m_page + m_iar;                           \
	m_page = m_ea & PAGE;                                       \
	m_iar  = m_ea & PMSK;                                       \
}

/***************************************************************
 * M_RET
 * Return from subroutine if cond is true
 ***************************************************************/
#define M_RET(cond)                                             \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		m_icount -= 6;                                          \
		m_ea = m_ras[get_sp()];                                 \
		set_sp(get_sp() - 1);                                   \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
	}                                                           \
}

/***************************************************************
 * M_RETE
 * Return from subroutine if cond is true
 * and enable interrupts; afterwards check IRQ line
 * state and eventually take next interrupt
 ***************************************************************/
#define M_RETE(cond)                                            \
{                                                               \
	if (cond)                                                   \
	{                                                           \
		m_ea = m_ras[get_sp()];                                 \
		set_sp(get_sp() - 1);                                   \
		m_page = m_ea & PAGE;                                   \
		m_iar  = m_ea & PMSK;                                   \
		set_psu(m_psu & ~II);                                   \
		m_icount -= check_irq_line();                           \
	}                                                           \
}

/***************************************************************
 * M_LOD
 * Load destination with source register
 ***************************************************************/
#define M_LOD(dest,source)                                      \
{                                                               \
	dest = source;                                              \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_STR
 * Store source register to memory addr (CC unchanged)
 ***************************************************************/
#define M_STR(address,source)                                   \
	m_program.write_byte(address, source)

/***************************************************************
 * M_AND
 * Logical and destination with source
 ***************************************************************/
#define M_AND(dest,source)                                      \
{                                                               \
	dest &= source;                                             \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_IOR
 * Logical inclusive or destination with source
 ***************************************************************/
#define M_IOR(dest,source)                                      \
{                                                               \
	dest |= source;                                             \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_EOR
 * Logical exclusive or destination with source
 ***************************************************************/
#define M_EOR(dest,source)                                      \
{                                                               \
	dest ^= source;                                             \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_ADD
 * Add source to destination
 * Add with carry if WC flag of PSL is set
 ***************************************************************/
#define M_ADD(dest,_source)                                     \
{                                                               \
	uint8_t source = _source;                                   \
	uint8_t before = dest;                                      \
	/* add source; carry only if WC is set */                   \
	uint16_t res = dest + source + ((m_psl >> 3) & m_psl & C);  \
	m_psl &= ~(C | OVF | IDC);                                  \
	if (res & 0x100) m_psl |= C;                                \
	dest = res & 0xff;                                          \
	if ((dest ^ before ^ source) & 0x10) m_psl |= IDC;          \
	if ((before ^ dest) & (source ^ dest) & 0x80) m_psl |= OVF; \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_SUB
 * Subtract source from destination
 * Subtract with borrow if WC flag of PSL is set
 ***************************************************************/
#define M_SUB(dest,_source)                                     \
{                                                               \
	uint8_t source = _source;                                   \
	uint8_t before = dest;                                      \
	/* subtract source; borrow only if WC is set */             \
	uint16_t res = dest - source - ((m_psl >> 3) & (m_psl ^ C) & C); \
	m_psl &= ~(C | OVF | IDC);                                  \
	if ((res & 0x100) == 0) m_psl |= C;                         \
	dest = res & 0xff;                                          \
	if (~(dest ^ before ^ source) & 0x10) m_psl |= IDC;         \
	if ((before ^ source) & (before ^ dest) & 0x80) m_psl |= OVF; \
	SET_CC(dest);                                               \
}

/***************************************************************
 * M_COM
 * Compare register against value. If COM of PSL is set,
 * use unsigned, else signed comparison
 ***************************************************************/
#define M_COM(reg,val)                                          \
{                                                               \
	int16_t res;                                                \
	m_psl &= ~CC;                                               \
	if (m_psl & COM) res = (uint8_t)reg - (uint8_t)val;         \
	else res = (int8_t)reg - (int8_t)val;                       \
	if (res < 0) m_psl |= 0x80;                                 \
	else if (res > 0) m_psl |= 0x40;                            \
}

/***************************************************************
 * M_DAR
 * Decimal adjust register
 ***************************************************************/
#define M_DAR(dest)                                             \
{                                                               \
	if ((m_psl & C) == 0) dest += 0xa0;                         \
	if ((m_psl & IDC) == 0) dest = (dest & 0xf0) | ((dest + 0x0a) & 0x0f); \
}

/***************************************************************
 * M_RRL
 * Rotate register left; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRL(dest)                                             \
{                                                               \
	uint8_t before = dest;                                      \
	if (m_psl & WC)                                             \
	{                                                           \
		uint8_t c = m_psl & C;                                  \
		m_psl &= ~(C + IDC);                                    \
		dest = (before << 1) | c;                               \
		m_psl |= (before >> 7) + (dest & IDC);                  \
	}                                                           \
	else                                                        \
	{                                                           \
		dest = (before << 1) | (before >> 7);                   \
	}                                                           \
	SET_CC(dest);                                               \
	m_psl = (m_psl & ~OVF) | (((dest ^ before) >> 5) & OVF);    \
}

/***************************************************************
 * M_RRR
 * Rotate register right; If WC of PSL is set, rotate
 * through carry, else rotate circular
 ***************************************************************/
#define M_RRR(dest)                                             \
{                                                               \
	uint8_t before = dest;                                      \
	if (m_psl & WC)                                             \
	{                                                           \
		uint8_t c = m_psl & C;                                  \
		m_psl &= ~(C + IDC);                                    \
		dest = (before >> 1) | (c << 7);                        \
		m_psl |= (before & C) + (dest & IDC);                   \
	}                                                           \
	else                                                        \
	{                                                           \
		dest = (before >> 1) | (before << 7);                   \
	}                                                           \
	SET_CC(dest);                                               \
	m_psl = (m_psl & ~OVF) | (((dest ^ before) >> 5) & OVF);    \
}

// bxd() not necessary

/***************************************************************
 * M_SPSU
 * Store processor status upper (PSU) to register R0
 * Checks for External Sense IO port
 ***************************************************************/
#define M_SPSU()                                                \
{                                                               \
	R0 = get_psu() & ~PSU34;                                    \
	SET_CC(R0);                                                 \
}

/***************************************************************
 * M_SPSL
 * Store processor status lower (PSL) to register R0
 ***************************************************************/
#define M_SPSL()                                                \
{                                                               \
	R0 = m_psl;                                                 \
	SET_CC(R0);                                                 \
}

/***************************************************************
 * M_CPSU
 * Clear processor status upper (PSU), selective
 ***************************************************************/
#define M_CPSU()                                                \
{                                                               \
	uint8_t cpsu = ARG() & ~SI;                                 \
	set_psu(m_psu & ~cpsu);                                     \
	m_icount -= check_irq_line();                               \
}

/***************************************************************
 * M_CPSL
 * Clear processor status lower (PSL), selective
 ***************************************************************/
#define M_CPSL()                                                \
{                                                               \
	uint8_t cpsl = ARG();                                       \
	/* select other register set now ? */                       \
	if ((cpsl & RS) && (m_psl & RS))                            \
		SWAP_REGS;                                              \
	m_psl = m_psl & ~cpsl;                                      \
}

/***************************************************************
 * M_PPSU
 * Preset processor status upper (PSU), selective
 * Unused bits 3 and 4 can't be set
 ***************************************************************/
#define M_PPSU()                                                \
{                                                               \
	uint8_t ppsu = (ARG() & ~PSU34) & ~SI;                      \
	set_psu(m_psu | ppsu);                                      \
}

/***************************************************************
 * M_PPSL
 * Preset processor status lower (PSL), selective
 ***************************************************************/
#define M_PPSL()                                                \
{                                                               \
	uint8_t ppsl = ARG();                                       \
	/* select 2nd register set now ? */                         \
	if ((ppsl & RS) && !(m_psl & RS))                           \
		SWAP_REGS;                                              \
	m_psl = m_psl | ppsl;                                       \
}

/***************************************************************
 * M_TPSU
 * Test processor status upper (PSU)
 ***************************************************************/
#define M_TPSU()                                                \
{                                                               \
	uint8_t tpsu = ARG();                                       \
	uint8_t rpsu = get_psu();                                   \
	m_psl &= ~CC;                                               \
	if ((rpsu & tpsu) != tpsu)                                  \
		m_psl |= 0x80;                                          \
}

/***************************************************************
 * M_TPSL
 * Test processor status lower (PSL)
 ***************************************************************/
#define M_TPSL()                                                \
{                                                               \
	uint8_t tpsl = ARG();                                       \
	if ((m_psl & tpsl) != tpsl)                                 \
		m_psl = (m_psl & ~CC) | 0x80;                           \
	else                                                        \
		m_psl &= ~CC;                                           \
}

/***************************************************************
 * M_TMI
 * Test under mask immediate
 ***************************************************************/
#define M_TMI(value)                                            \
{                                                               \
	uint8_t tmi = ARG();                                        \
	m_psl &= ~CC;                                               \
	if ((value & tmi) != tmi)                                   \
		m_psl |= 0x80;                                          \
}

#if INLINE_EA
#define REL_EA(page) _REL_EA(page)
#define REL_ZERO(page) _REL_ZERO(page)
#define ABS_EA() _ABS_EA()
#define BRA_EA() _BRA_EA()
#else
static void REL_EA(unsigned short page) _REL_EA(page)
static void REL_ZERO(unsigned short page) _REL_ZERO(page)
static void ABS_EA(void) _ABS_EA()
static void BRA_EA(void) _BRA_EA()
#endif

void s2650_device::device_start()
{
	space(AS_PROGRAM).cache(m_cprogram);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_ppc));
	save_item(NAME(m_page));
	save_item(NAME(m_iar));
	save_item(NAME(m_ea));
	save_item(NAME(m_psl));
	save_item(NAME(m_psu));
	save_item(NAME(m_r));
	save_item(NAME(m_reg));
	save_item(NAME(m_halt));
	save_item(NAME(m_ir));
	save_item(NAME(m_ras));
	save_item(NAME(m_irq_state));

	state_add( S2650_PC,   "PC", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( S2650_PS,   "PS", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( S2650_R0,   "R0", m_reg[0]).formatstr("%02X");
	state_add( S2650_R1,   "R1", m_reg[1]).formatstr("%02X");
	state_add( S2650_R2,   "R2", m_reg[2]).formatstr("%02X");
	state_add( S2650_R3,   "R3", m_reg[3]).formatstr("%02X");
	state_add( S2650_R1A,  "R1'", m_reg[4]).formatstr("%02X");
	state_add( S2650_R2A,  "R2'", m_reg[5]).formatstr("%02X");
	state_add( S2650_R3A,  "R3'", m_reg[6]).formatstr("%02X");
	state_add( S2650_HALT, "HALT", m_halt).formatstr("%01X");
	state_add( S2650_SI,   "SI", m_debugger_temp).mask(0x01).callimport().callexport().formatstr("%01X");
	state_add( S2650_FO,   "FO", m_debugger_temp).mask(0x01).callimport().callexport().formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_ppc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%16s").noshow();

	set_icountptr(m_icount);
}

void s2650_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case S2650_PC:
			m_page = m_debugger_temp & PAGE;
			m_iar = m_debugger_temp & PMSK;
			break;

		case S2650_PS:
			m_psl = m_debugger_temp & 0xff;
			set_psu(m_debugger_temp >> 8);
			break;

		case S2650_SI:
			s2650_set_sense(m_debugger_temp);
			break;

		case S2650_FO:
			s2650_set_flag(m_debugger_temp);
			break;
	}
}

void s2650_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case S2650_PC:
			m_debugger_temp = m_page + m_iar;
			break;

		case S2650_PS:
			m_debugger_temp = (m_psu << 8) | m_psl;
			break;

		case S2650_SI:
			m_debugger_temp = (m_psu & SI) ? 1 : 0;
			break;

		case S2650_FO:
			m_debugger_temp = (m_psu & FO) ? 1 : 0;
			break;
	}
}

void s2650_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",
				m_psu & 0x80 ? 'S':'.',
				m_psu & 0x40 ? 'O':'.',
				m_psu & 0x20 ? 'I':'.',
				m_psu & 0x10 ? '?':'.',
				m_psu & 0x08 ? '?':'.',
				m_psu & 0x04 ? 's':'.',
				m_psu & 0x02 ? 's':'.',
				m_psu & 0x01 ? 's':'.',
				m_psl & 0x80 ? 'M':'.',
				m_psl & 0x40 ? 'P':'.',
				m_psl & 0x20 ? 'H':'.',
				m_psl & 0x10 ? 'R':'.',
				m_psl & 0x08 ? 'W':'.',
				m_psl & 0x04 ? 'V':'.',
				m_psl & 0x02 ? '2':'.',
				m_psl & 0x01 ? 'C':'.');
			break;
	}
}

void s2650_device::device_reset()
{
	m_ppc = 0;
	m_page = 0,
	m_iar = 0;
	m_ea = 0;
	m_r = 0;
	m_halt = 0;
	m_ir = 0;
	m_irq_state = CLEAR_LINE;
	memset(m_reg, 0, sizeof(m_reg));
	memset(m_ras, 0, sizeof(m_ras));

	m_psl = COM | WC;
	// force write
	m_psu = 0xff;
	set_psu(0);
}


void s2650_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_IRQ0:
		m_irq_state = state;
		break;

	case S2650_SENSE_LINE:
		if (state == CLEAR_LINE)
			s2650_set_sense(0);
		else
			s2650_set_sense(1);
		break;
	}
}

void s2650_device::s2650_set_flag(int state)
{
	if (state)
		set_psu(m_psu | FO);
	else
		set_psu(m_psu & ~FO);
}

int s2650_device::s2650_get_flag()
{
	return (m_psu & FO) ? 1 : 0;
}

void s2650_device::s2650_set_sense(int state)
{
	if (state)
		set_psu(m_psu | SI);
	else
		set_psu(m_psu & ~SI);
}

void s2650_device::execute_run()
{
	// check for external irqs
	m_icount -= check_irq_line();

	do
	{
		m_ppc = m_page + m_iar;

		debugger_instruction_hook(m_page + m_iar);

		m_ir = ROP();
		m_r = m_ir & 3;     // register / value
		switch (m_ir)
		{
			case 0x00:      // LODZ,0
			case 0x01:      // LODZ,1
			case 0x02:      // LODZ,2
			case 0x03:      // LODZ,3
				m_icount -= 6;
				M_LOD(R0, m_reg[m_r]);
				break;

			case 0x04:      // LODI,0 v
			case 0x05:      // LODI,1 v
			case 0x06:      // LODI,2 v
			case 0x07:      // LODI,3 v
				m_icount -= 6;
				M_LOD(m_reg[m_r], ARG());
				break;

			case 0x08:      // LODR,0 (*)a
			case 0x09:      // LODR,1 (*)a
			case 0x0a:      // LODR,2 (*)a
			case 0x0b:      // LODR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_LOD(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x0c:      // LODA,0 (*)a(,X)
			case 0x0d:      // LODA,1 (*)a(,X)
			case 0x0e:      // LODA,2 (*)a(,X)
			case 0x0f:      // LODA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_LOD(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x12:      // SPSU
				m_icount -= 6;
				M_SPSU();
				break;
			case 0x13:      // SPSL
				m_icount -= 6;
				M_SPSL();
				break;

			case 0x14:      // RETC,0   (zero)
			case 0x15:      // RETC,1   (plus)
			case 0x16:      // RETC,2   (minus)
				m_icount -= 9;    // +2 cycles if condition is true
				M_RET((m_psl >> 6) == m_r);
				break;
			case 0x17:      // RETC,3   (always)
				m_icount -= 9;    // +2 cycles if condition is true
				M_RET(1);
				break;

			case 0x18:      // BCTR,0  (*)a
			case 0x19:      // BCTR,1  (*)a
			case 0x1a:      // BCTR,2  (*)a
				m_icount -= 9;
				M_BRR((m_psl >> 6) == m_r);
				break;
			case 0x1b:      // BCTR,3  (*)a
				m_icount -= 9;
				M_BRR(1);
				break;

			case 0x1c:      // BCTA,0  (*)a
			case 0x1d:      // BCTA,1  (*)a
			case 0x1e:      // BCTA,2  (*)a
				m_icount -= 9;
				M_BRA((m_psl >> 6) == m_r);
				break;
			case 0x1f:      // BCTA,3  (*)a
				m_icount -= 9;
				M_BRA(1);
				break;

			case 0x20:      // EORZ,0
			case 0x21:      // EORZ,1
			case 0x22:      // EORZ,2
			case 0x23:      // EORZ,3
				m_icount -= 6;
				M_EOR(R0, m_reg[m_r]);
				break;

			case 0x24:      // EORI,0 v
			case 0x25:      // EORI,1 v
			case 0x26:      // EORI,2 v
			case 0x27:      // EORI,3 v
				m_icount -= 6;
				M_EOR(m_reg[m_r], ARG());
				break;

			case 0x28:      // EORR,0 (*)a
			case 0x29:      // EORR,1 (*)a
			case 0x2a:      // EORR,2 (*)a
			case 0x2b:      // EORR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_EOR(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x2c:      // EORA,0 (*)a(,X)
			case 0x2d:      // EORA,1 (*)a(,X)
			case 0x2e:      // EORA,2 (*)a(,X)
			case 0x2f:      // EORA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_EOR(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x30:      // REDC,0
			case 0x31:      // REDC,1
			case 0x32:      // REDC,2
			case 0x33:      // REDC,3
				m_icount -= 6;
				m_reg[m_r] = m_data.read_byte(S2650_CTRL_PORT);
				SET_CC(m_reg[m_r]);
				break;

			case 0x34:      // RETE,0
			case 0x35:      // RETE,1
			case 0x36:      // RETE,2
				m_icount -= 9;
				M_RETE((m_psl >> 6) == m_r);
				break;
			case 0x37:      // RETE,3
				m_icount -= 9;
				M_RETE(1);
				break;

			case 0x38:      // BSTR,0 (*)a
			case 0x39:      // BSTR,1 (*)a
			case 0x3a:      // BSTR,2 (*)a
				m_icount -= 9;
				M_BSR((m_psl >> 6) == m_r);
				break;
			case 0x3b:      // BSTR,R3 (*)a
				m_icount -= 9;
				M_BSR(1);
				break;

			case 0x3c:      // BSTA,0 (*)a
			case 0x3d:      // BSTA,1 (*)a
			case 0x3e:      // BSTA,2 (*)a
				m_icount -= 9;
				M_BSA((m_psl >> 6) == m_r);
				break;
			case 0x3f:      // BSTA,3 (*)a
				m_icount -= 9;
				M_BSA(1);
				break;

			case 0x40:      // HALT
				m_icount -= 6;
				m_iar = (m_iar - 1) & PMSK;
				m_halt = 1;
				if (m_icount > 0)
					m_icount = 0;
				break;
			case 0x41:      // ANDZ,1
			case 0x42:      // ANDZ,2
			case 0x43:      // ANDZ,3
				m_icount -= 6;
				M_AND(R0, m_reg[m_r]);
				break;

			case 0x44:      // ANDI,0 v
			case 0x45:      // ANDI,1 v
			case 0x46:      // ANDI,2 v
			case 0x47:      // ANDI,3 v
				m_icount -= 6;
				M_AND(m_reg[m_r], ARG());
				break;

			case 0x48:      // ANDR,0 (*)a
			case 0x49:      // ANDR,1 (*)a
			case 0x4a:      // ANDR,2 (*)a
			case 0x4b:      // ANDR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_AND(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x4c:      // ANDA,0 (*)a(,X)
			case 0x4d:      // ANDA,1 (*)a(,X)
			case 0x4e:      // ANDA,2 (*)a(,X)
			case 0x4f:      // ANDA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_AND(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x50:      // RRR,0
			case 0x51:      // RRR,1
			case 0x52:      // RRR,2
			case 0x53:      // RRR,3
				m_icount -= 6;
				M_RRR(m_reg[m_r]);
				break;

			case 0x54:      // REDE,0 v
			case 0x55:      // REDE,1 v
			case 0x56:      // REDE,2 v
			case 0x57:      // REDE,3 v
				m_icount -= 9;
				m_reg[m_r] = m_io.read_byte(ARG());
				SET_CC(m_reg[m_r]);
				break;

			case 0x58:      // BRNR,0 (*)a
			case 0x59:      // BRNR,1 (*)a
			case 0x5a:      // BRNR,2 (*)a
			case 0x5b:      // BRNR,3 (*)a
				m_icount -= 9;
				M_BRR(m_reg[m_r]);
				break;

			case 0x5c:      // BRNA,0 (*)a
			case 0x5d:      // BRNA,1 (*)a
			case 0x5e:      // BRNA,2 (*)a
			case 0x5f:      // BRNA,3 (*)a
				m_icount -= 9;
				M_BRA(m_reg[m_r]);
				break;

			case 0x60:      // IORZ,0
			case 0x61:      // IORZ,1
			case 0x62:      // IORZ,2
			case 0x63:      // IORZ,3
				m_icount -= 6;
				M_IOR(R0, m_reg[m_r]);
				break;

			case 0x64:      // IORI,0 v
			case 0x65:      // IORI,1 v
			case 0x66:      // IORI,2 v
			case 0x67:      // IORI,3 v
				m_icount -= 6;
				M_IOR(m_reg[m_r], ARG());
				break;

			case 0x68:      // IORR,0 (*)a
			case 0x69:      // IORR,1 (*)a
			case 0x6a:      // IORR,2 (*)a
			case 0x6b:      // IORR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_IOR(m_reg[m_r],RDMEM(m_ea));
				break;

			case 0x6c:      // IORA,0 (*)a(,X)
			case 0x6d:      // IORA,1 (*)a(,X)
			case 0x6e:      // IORA,2 (*)a(,X)
			case 0x6f:      // IORA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_IOR(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x70:      // REDD,0
			case 0x71:      // REDD,1
			case 0x72:      // REDD,2
			case 0x73:      // REDD,3
				m_icount -= 6;
				m_reg[m_r] = m_data.read_byte(S2650_DATA_PORT);
				SET_CC(m_reg[m_r]);
				break;

			case 0x74:      // CPSU
				m_icount -= 9;
				M_CPSU();
				break;
			case 0x75:      // CPSL
				m_icount -= 9;
				M_CPSL();
				break;
			case 0x76:      // PPSU
				m_icount -= 9;
				M_PPSU();
				break;
			case 0x77:      // PPSL
				m_icount -= 9;
				M_PPSL();
				break;

			case 0x78:      // BSNR,0 (*)a
			case 0x79:      // BSNR,1 (*)a
			case 0x7a:      // BSNR,2 (*)a
			case 0x7b:      // BSNR,3 (*)a
				m_icount -= 9;
				M_BSR(m_reg[m_r]);
				break;

			case 0x7c:      // BSNA,0 (*)a
			case 0x7d:      // BSNA,1 (*)a
			case 0x7e:      // BSNA,2 (*)a
			case 0x7f:      // BSNA,3 (*)a
				m_icount -= 9;
				M_BSA(m_reg[m_r]);
				break;

			case 0x80:      // ADDZ,0
			case 0x81:      // ADDZ,1
			case 0x82:      // ADDZ,2
			case 0x83:      // ADDZ,3
				m_icount -= 6;
				M_ADD(R0,m_reg[m_r]);
				break;

			case 0x84:      // ADDI,0 v
			case 0x85:      // ADDI,1 v
			case 0x86:      // ADDI,2 v
			case 0x87:      // ADDI,3 v
				m_icount -= 6;
				M_ADD(m_reg[m_r], ARG());
				break;

			case 0x88:      // ADDR,0 (*)a
			case 0x89:      // ADDR,1 (*)a
			case 0x8a:      // ADDR,2 (*)a
			case 0x8b:      // ADDR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_ADD(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x8c:      // ADDA,0 (*)a(,X)
			case 0x8d:      // ADDA,1 (*)a(,X)
			case 0x8e:      // ADDA,2 (*)a(,X)
			case 0x8f:      // ADDA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_ADD(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0x92:      // LPSU
				m_icount -= 6;
				set_psu((R0 & ~PSU34 & ~SI) | (m_psu & SI));
				break;
			case 0x93:      // LPSL
				m_icount -= 6;
				// change register set ?
				if ((m_psl ^ R0) & RS)
					SWAP_REGS;
				m_psl = R0;
				break;

			case 0x94:      // DAR,0
			case 0x95:      // DAR,1
			case 0x96:      // DAR,2
			case 0x97:      // DAR,3
				m_icount -= 9;
				M_DAR(m_reg[m_r]);
				break;

			case 0x98:      // BCFR,0 (*)a
			case 0x99:      // BCFR,1 (*)a
			case 0x9a:      // BCFR,2 (*)a
				m_icount -= 9;
				M_BRR((m_psl >> 6) != m_r);
				break;
			case 0x9b:      // ZBRR    (*)a
				m_icount -= 9;
				M_ZBRR();
				break;

			case 0x9c:      // BCFA,0 (*)a
			case 0x9d:      // BCFA,1 (*)a
			case 0x9e:      // BCFA,2 (*)a
				m_icount -= 9;
				M_BRA((m_psl >> 6) != m_r);
				break;
			case 0x9f:      // BXA     (*)a
				m_icount -= 9;
				M_BXA();
				break;

			case 0xa0:      // SUBZ,0
			case 0xa1:      // SUBZ,1
			case 0xa2:      // SUBZ,2
			case 0xa3:      // SUBZ,3
				m_icount -= 6;
				M_SUB(R0, m_reg[m_r]);
				break;

			case 0xa4:      // SUBI,0 v
			case 0xa5:      // SUBI,1 v
			case 0xa6:      // SUBI,2 v
			case 0xa7:      // SUBI,3 v
				m_icount -= 6;
				M_SUB(m_reg[m_r], ARG());
				break;

			case 0xa8:      // SUBR,0 (*)a
			case 0xa9:      // SUBR,1 (*)a
			case 0xaa:      // SUBR,2 (*)a
			case 0xab:      // SUBR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_SUB(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0xac:      // SUBA,0 (*)a(,X)
			case 0xad:      // SUBA,1 (*)a(,X)
			case 0xae:      // SUBA,2 (*)a(,X)
			case 0xaf:      // SUBA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_SUB(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0xb0:      // WRTC,0
			case 0xb1:      // WRTC,1
			case 0xb2:      // WRTC,2
			case 0xb3:      // WRTC,3
				m_icount -= 6;
				m_data.write_byte(S2650_CTRL_PORT,m_reg[m_r]);
				break;

			case 0xb4:      // TPSU
				m_icount -= 9;
				M_TPSU();
				break;
			case 0xb5:      // TPSL
				m_icount -= 9;
				M_TPSL();
				break;

			case 0xb8:      // BSFR,0 (*)a
			case 0xb9:      // BSFR,1 (*)a
			case 0xba:      // BSFR,2 (*)a
				m_icount -= 9;
				M_BSR((m_psl >> 6) != m_r);
				break;
			case 0xbb:      // ZBSR    (*)a
				m_icount -= 9;
				M_ZBSR();
				break;

			case 0xbc:      // BSFA,0 (*)a
			case 0xbd:      // BSFA,1 (*)a
			case 0xbe:      // BSFA,2 (*)a
				m_icount -= 9;
				M_BSA((m_psl >> 6) != m_r);
				break;
			case 0xbf:      // BSXA    (*)a
				m_icount -= 9;
				M_BSXA();
				break;

			case 0xc0:      // NOP
				m_icount -= 6;
				break;
			case 0xc1:      // STRZ,1
			case 0xc2:      // STRZ,2
			case 0xc3:      // STRZ,3
				m_icount -= 6;
				M_LOD(m_reg[m_r], R0);
				break;

			case 0xc8:      // STRR,0 (*)a
			case 0xc9:      // STRR,1 (*)a
			case 0xca:      // STRR,2 (*)a
			case 0xcb:      // STRR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_STR(m_ea, m_reg[m_r]);
				break;

			case 0xcc:      // STRA,0 (*)a(,X)
			case 0xcd:      // STRA,1 (*)a(,X)
			case 0xce:      // STRA,2 (*)a(,X)
			case 0xcf:      // STRA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_STR(m_ea, m_reg[m_r]);
				break;

			case 0xd0:      // RRL,0
			case 0xd1:      // RRL,1
			case 0xd2:      // RRL,2
			case 0xd3:      // RRL,3
				m_icount -= 6;
				M_RRL(m_reg[m_r]);
				break;

			case 0xd4:      // WRTE,0 v
			case 0xd5:      // WRTE,1 v
			case 0xd6:      // WRTE,2 v
			case 0xd7:      // WRTE,3 v
				m_icount -= 9;
				m_io.write_byte(ARG(), m_reg[m_r]);
				break;

			case 0xd8:      // BIRR,0 (*)a
			case 0xd9:      // BIRR,1 (*)a
			case 0xda:      // BIRR,2 (*)a
			case 0xdb:      // BIRR,3 (*)a
				m_icount -= 9;
				M_BRR(++m_reg[m_r]);
				break;

			case 0xdc:      // BIRA,0 (*)a
			case 0xdd:      // BIRA,1 (*)a
			case 0xde:      // BIRA,2 (*)a
			case 0xdf:      // BIRA,3 (*)a
				m_icount -= 9;
				M_BRA(++m_reg[m_r]);
				break;

			case 0xe0:      // COMZ,0
			case 0xe1:      // COMZ,1
			case 0xe2:      // COMZ,2
			case 0xe3:      // COMZ,3
				m_icount -= 6;
				M_COM(R0, m_reg[m_r]);
				break;

			case 0xe4:      // COMI,0 v
			case 0xe5:      // COMI,1 v
			case 0xe6:      // COMI,2 v
			case 0xe7:      // COMI,3 v
				m_icount -= 6;
				M_COM(m_reg[m_r], ARG());
				break;

			case 0xe8:      // COMR,0 (*)a
			case 0xe9:      // COMR,1 (*)a
			case 0xea:      // COMR,2 (*)a
			case 0xeb:      // COMR,3 (*)a
				m_icount -= 9;
				REL_EA(m_page);
				M_COM(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0xec:      // COMA,0 (*)a(,X)
			case 0xed:      // COMA,1 (*)a(,X)
			case 0xee:      // COMA,2 (*)a(,X)
			case 0xef:      // COMA,3 (*)a(,X)
				m_icount -= 12;
				ABS_EA();
				M_COM(m_reg[m_r], RDMEM(m_ea));
				break;

			case 0xf0:      // WRTD,0
			case 0xf1:      // WRTD,1
			case 0xf2:      // WRTD,2
			case 0xf3:      // WRTD,3
				m_icount -= 6;
				m_data.write_byte(S2650_DATA_PORT, m_reg[m_r]);
				break;

			case 0xf4:      // TMI,0  v
			case 0xf5:      // TMI,1  v
			case 0xf6:      // TMI,2  v
			case 0xf7:      // TMI,3  v
				m_icount -= 9;
				M_TMI(m_reg[m_r]);
				break;

			case 0xf8:      // BDRR,0 (*)a
			case 0xf9:      // BDRR,1 (*)a
			case 0xfa:      // BDRR,2 (*)a
			case 0xfb:      // BDRR,3 (*)a
				m_icount -= 9;
				M_BRR(--m_reg[m_r]);
				break;

			case 0xfc:      // BDRA,0 (*)a
			case 0xfd:      // BDRA,1 (*)a
			case 0xfe:      // BDRA,2 (*)a
			case 0xff:      // BDRA,3 (*)a
				m_icount -= 9;
				M_BRA(--m_reg[m_r]);
				break;

			default:        // illegal
				m_icount -= 6;
				logerror("%s: illegal opcode $%02X @ $%04X\n", tag(), m_ppc, m_ir);
				break;
		}
	} while (m_icount > 0);
}
