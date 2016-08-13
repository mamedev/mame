// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII RAM related functions
 *
 *****************************************************************************/
#include "alto2cpu.h"

#define DEBUG_WRTRAM        0       //!< define to 1 to print CRAM writes
#define DEBUG_RDRAM         0       //!< define to 1 to print CRAM reads
#define DEBUG_BRANCH        0       //!< define to 1 to print branching to ROM/RAM

//! direct read access to the microcode CRAM
#define RD_CRAM(addr) (*reinterpret_cast<UINT32 *>(m_ucode_cram.get() + addr * 4))

/**
 * @brief read the microcode ROM/RAM halfword
 *
 * Note: HALFSEL is selecting the even (0) or odd (1) half of the
 * microcode RAM 32-bit word. Here's how the demultiplexers (74298)
 * u8, u18, u28 and u38 select the bits:
 *
 *           SN74298
 *         +---+-+---+
 *         |   +-+   |
 *    B2  -|1      16|-  Vcc
 *         |         |
 *    A2  -|2      15|-  QA
 *         |         |
 *    A1  -|3      14|-  QB
 *         |         |
 *    B1  -|4      13|-  QC
 *         |         |
 *    C2  -|5      12|-  QD
 *         |         |
 *    D2  -|6      11|-  CLK
 *         |         |
 *    D1  -|7      10|-  SEL
 *         |         |
 *   GND  -|8       9|-  C1
 *         |         |
 *         +---------+
 *
 *  chip  out pin  BUS in   pin HSEL=0      in   pin HSEL=1
 *  --------------------------------------------------------------
 *  u8    QA  15   0   A1   3   DRSEL(0)'   A2   2   DF2(0)
 *  u8    QB  14   1   B1   4   DRSEL(1)'   B2   1   DF2(1)'
 *  u8    QC  13   2   C1   9   DRSEL(2)'   C2   5   DF2(2)'
 *  u8    QD  12   3   D1   7   DRSEL(3)'   D2   6   DF2(3)'
 *
 *  u18   QA  15   4   A1   3   DRSEL(4)'   A2   2   LOADT'
 *  u18   QB  14   5   B1   4   DALUF(0)'   B2   1   LOADL
 *  u18   QC  13   6   C1   9   DALUF(1)'   C2   5   NEXT(00)'
 *  u18   QD  12   7   D1   7   DALUF(2)'   D2   6   NEXT(01)'
 *
 *  u28   QA  15   8   A1   3   DALUF(3)'   A2   2   NEXT(02)'
 *  u28   QB  14   9   B1   4   DBS(0)'     B2   1   NEXT(03)'
 *  u28   QC  13   10  C1   9   DBS(1)'     C2   5   NEXT(04)'
 *  u28   QD  12   11  D1   7   DBS(2)'     D2   6   NEXT(05)'
 *
 *  u38   QA  15   12  A1   3   DF1(0)      A2   2   NEXT(06)'
 *  u38   QB  14   13  B1   4   DF1(1)'     B2   1   NEXT(07)'
 *  u38   QC  13   14  C1   9   DF1(2)'     C2   5   NEXT(08)'
 *  u38   QD  12   15  D1   7   DF1(3)'     D2   6   NEXT(09)'
 *
 * The HALFSEL signal to the demultiplexers is the inverted bit BUS(5):
 * BUS(5)=1, HALFSEL=0, A1,B1,C1,D1 inputs, upper half of the 32-bit word
 * BUS(5)=0, HALFSEL=1, A2,B2,C2,D2 inputs, lower half of the 32-bit word
 */
void alto2_cpu_device::rdram()
{
	UINT32 addr, value;
	UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr);
	UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);

	if (GET_CRAM_RAMROM(m_cram_addr)) {
		/* read CROM 0 at current mpc */
		addr = m_mpc & ALTO2_UCODE_PAGE_MASK;
		LOG((this,LOG_CPU,0,"    rdram: ROM [%05o] ", addr));
	} else {
		/* read CRAM[bank] */
		addr = bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;
		LOG((this,LOG_CPU,0,"    rdram: RAM%d [%04o] ", bank, wordaddr));
	}

	m_rdram_flag = false;
	if (m_ucode_ram_base + addr >= m_ucode_size) {
		value = 0177777;    /* ??? */
		LOG((this,LOG_CPU,0,"invalid address (%06o)\n", addr));
#if DEBUG_RDRAM
		printf("RD CRAM_BANKSEL=%d RAM%d [%04o] invalid address!\n",
			GET_CRAM_BANKSEL(m_cram_addr), bank, wordaddr);
#endif
		return;
	}
	value = *reinterpret_cast<UINT32 *>(m_ucode_cram.get() + addr * 4) ^ ALTO2_UCODE_INVERTED;
#if DEBUG_RDRAM
	char buffer[256];
	UINT8* oprom = m_ucode_cram.get() + 4 * wordaddr;
	disasm_disassemble(buffer, wordaddr, oprom, oprom, 0);
	printf("RD CRAM_BANKSEL=%d RAM%d [%04o] upper:%06o lower:%06o value:%011o '%s'\n",
			GET_CRAM_BANKSEL(m_cram_addr), bank, wordaddr, m_myl, m_alu,
			value, buffer);
#endif
	if (GET_CRAM_HALFSEL(m_cram_addr)) {
		value >>= 16;
		LOG((this,LOG_CPU,0,"upper:%06o\n", value));
	} else {
		value &= 0177777;
		LOG((this,LOG_CPU,0,"lower:%06o\n", value));
	}
	m_bus &= value;
}

/**
 * @brief write the microcode RAM from M register and ALU
 *
 * Note: M is a latch (MYL, i.e. memory L) on the CRAM board that latches
 * the ALU whenever LOADL and GOODTASK are met. GOODTASK is the Emulator
 * task and something I have not yet found out about: TASKA' and TASKB'.
 *
 * There's also an undumped PROM u21 which is addressed by GOODTASK and
 * 7 other signals...
 */
void alto2_cpu_device::wrtram()
{
	const UINT32 bank = GET_CRAM_BANKSEL(m_cram_addr);
	const UINT32 wordaddr = GET_CRAM_WORDADDR(m_cram_addr);
	const UINT32 addr = bank * ALTO2_UCODE_PAGE_SIZE + wordaddr;  // write RAM 0,1,2
	const UINT32 value = (m_myl << 16) | m_alu;

	LOG((this,LOG_CPU,0,"    wrtram: RAM%d [%04o] upper:%06o lower:%06o", bank, wordaddr, m_myl, m_alu));

	m_wrtram_flag = false;
	if (m_ucode_ram_base + addr >= m_ucode_size) {
		LOG((this,LOG_CPU,0," invalid address %06o\n", addr));
		return;
	}
	LOG((this,LOG_CPU,0,"\n"));
	*reinterpret_cast<UINT32 *>(m_ucode_cram.get() + addr * 4) = value ^ ALTO2_UCODE_INVERTED;

#if DEBUG_WRTRAM
	char buffer[256];
	UINT8* oprom = m_ucode_cram.get() + 4 * wordaddr;
	disasm_disassemble(buffer, wordaddr, oprom, oprom, 0);
	printf("WR CRAM_BANKSEL=%d RAM%d [%04o] upper:%06o lower:%06o value:%011o '%s'\n",
			GET_CRAM_BANKSEL(m_cram_addr), bank, wordaddr, m_myl, m_alu,
			value, buffer);
#endif
}

/**
 * @brief bs_read_sreg early: drive bus by S register or M (MYL), if rsel is = 0
 *
 * Note: RSEL == 0 can't be read, because it is decoded as
 * access to the M register (MYL latch access, LREF' in the schematics)
 */
void alto2_cpu_device::bs_early_read_sreg()
{
	UINT16 r;

	if (rsel()) {
		UINT8 bank = m_s_reg_bank[m_task];
		r = m_s[bank][rsel()];
		LOG((this,LOG_RAM,2,"    <-S%02o; bus &= S[%o][%02o] (%#o)\n", rsel(), bank, rsel(), r));
	} else {
		r = m_myl;
		LOG((this,LOG_RAM,2,"    <-S%02o; bus &= M (%#o)\n", rsel(), r));
	}
	m_bus &= r;
}

/**
 * @brief bs_load_sreg early: load S register puts garbage on the bus
 */
void alto2_cpu_device::bs_early_load_sreg()
{
	int r = 0;  /* ??? */
	LOG((this,LOG_RAM,2,"    S%02o<- BUS &= garbage (%#o)\n", rsel(), r));
	m_bus &= r;
}

/**
 * @brief bs_load_sreg late: load S register from M
 */
void alto2_cpu_device::bs_late_load_sreg()
{
	UINT8 bank = m_s_reg_bank[m_task];
	m_s[bank][rsel()] = m_myl;
	LOG((this,LOG_RAM,2,"    S%02o<- S[%o][%02o] := %#o\n", rsel(), bank, rsel(), m_myl));
}

/**
 * @brief branch to ROM page
 */
void alto2_cpu_device::branch_ROM(const char *from, int page)
{
	(void)from;
#if DEBUG_BRANCH
	printf("SWMODE: branch from %s to ROM%d (%#o)\n", from, page, m_next2);
#endif
	m_next2 = (m_next2 & ALTO2_UCODE_PAGE_MASK) + page * ALTO2_UCODE_PAGE_SIZE;
	LOG((this,LOG_RAM,2,"    SWMODE: branch from %s to ROM%d (%#o)\n", from, page, m_next2));
}

/**
 * @brief branch to RAM page
 */
void alto2_cpu_device::branch_RAM(const char *from, int page)
{
	(void)from;
#if DEBUG_BRANCH
	printf("SWMODE: branch from %s to RAM%d (%#o)\n", from, page, m_next2);
#endif
	m_next2 = (m_next2 & ALTO2_UCODE_PAGE_MASK) + m_ucode_ram_base + page * ALTO2_UCODE_PAGE_SIZE;
	LOG((this,LOG_RAM,2,"    SWMODE: branch from %s to RAM%d\n", from, page, m_next2));
}

/**
 * @brief f1_swmode early: switch to micro program counter BUS[6-15] in other bank
 *
 * Note: Jumping to uninitialized CRAM
 *
 * When jumping to uninitialized RAM, which, because of the inverted bits of the
 * microcode words F1(0), F2(0) and LOADL, it is then read as F1=010 (SWMODE),
 * F2=010 (BUSODD) and LOADL=1, loading the M register (MYL latch), too.
 * This causes control to go back to the Emulator task at 0, because the
 * NEXT[0-9] of uninitialized RAM is 0.
 *
 */
void alto2_cpu_device::f1_late_swmode()
{
	// Currently executing in what CROM/CRAM page?
	UINT16 page = m_mpc / ALTO2_UCODE_PAGE_SIZE;
	UINT16 next;

	switch (m_cram_config) {
	case 1:	// 1K CROM, 1K CRAM
		switch (page) {
		case 0:
			branch_RAM("ROM0", 0);
			break;
		case 1:
			branch_ROM("RAM0", 0);
			break;
		default:
			fatal(1, "Impossible current mpc %u\n", page);
		}
		break;

	case 2:	// 2K CROM, 1K CRAM
		next = X_RDBITS(m_next2,10,1,1);
		switch (page) {
		case 0: /* ROM0 to RAM0 or ROM1 */
			switch (next) {
			case 0:
				branch_RAM("ROM0", 0);
				break;
			case 1:
				branch_ROM("ROM0", 1);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		case 1: /* ROM1 to ROM0 or RAM0 */
			switch (next) {
			case 0:
				branch_ROM("ROM1", 0);
				break;
			case 1:
				branch_RAM("ROM1", 0);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		case 2: /* RAM0 to ROM0 or ROM1 */
			switch (next) {
			case 0:
				branch_ROM("RAM0", 0);
				break;
			case 1:
				branch_ROM("RAM0", 1);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		default:
			fatal(1, "Impossible current mpc %u\n", page);
		}
		break;

	case 3:	// 1K CROM, 3K CRAM
		next = X_RDBITS(m_next2,10,1,2);

		switch (page) {
		case 0: /* ROM0 to RAM0, RAM2, RAM1, RAM0 */
			switch (next) {
			case 0:
				branch_RAM("ROM0", 0);
				break;
			case 1:
				branch_RAM("ROM0", 2);
				break;
			case 2:
				branch_RAM("ROM0", 1);
				break;
			case 3:
				branch_RAM("ROM0", 0);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		case 1: /* RAM0 to ROM0, RAM2, RAM1, RAM1 */
			switch (next) {
			case 0:
				branch_ROM("RAM0", 0);
				break;
			case 1:
				branch_RAM("RAM0", 2);
				break;
			case 2:
				branch_RAM("RAM0", 1);
				break;
			case 3:
				branch_RAM("RAM0", 1);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		case 2: /* RAM1 to ROM0, RAM2, RAM0, RAM0 */
			switch (next) {
			case 0:
				branch_ROM("RAM1", 0);
				break;
			case 1:
				branch_RAM("RAM1", 2);
				break;
			case 2:
				branch_RAM("RAM1", 0);
				break;
			case 3:
				branch_RAM("RAM1", 0);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		case 3: /* RAM2 to ROM0, RAM1, RAM0, RAM0 */
			switch (next) {
			case 0:
				branch_ROM("RAM2", 0);
				break;
			case 1:
				branch_RAM("RAM2", 1);
				break;
			case 2:
				branch_RAM("RAM2", 0);
				break;
			case 3:
				branch_RAM("RAM2", 0);
				break;
			default:
				fatal(1, "Impossible next %u\n", next);
			}
			break;
		default:
			fatal(1, "Impossible current mpc %u\n", page);
		}
		break;
	default:
		fatal(1, "Impossible control ROM/RAM config %u (%u CROM pages, %u CRAM pages)\n",
			m_cram_config, m_ucode_rom_pages, m_ucode_ram_pages);
	}
}

/**
 * @brief f1_wrtram late: start WRTRAM cycle
 */
void alto2_cpu_device::f1_late_wrtram()
{
	m_wrtram_flag = true;
	LOG((this,LOG_RAM,2,"    WRTRAM\n"));
}

/**
 * @brief f1_rdram late: start RDRAM cycle
 */
void alto2_cpu_device::f1_late_rdram()
{
	m_rdram_flag = true;
	LOG((this,LOG_RAM,2,"    RDRAM\n"));
}

/**
 * @brief f1_load_rmr late: load the reset mode register
 *
 * F1=013 corresponds to RMR<- in the emulator. In Altos with the 3K
 * RAM option, F1=013 performs RMR<- in all RAM-related tasks, including
 * the emulator.
 */
void alto2_cpu_device::f1_late_load_rmr()
{
	LOG((this,LOG_RAM,2,"    RMR<-; BUS (%#o)\n", m_bus));
	m_reset_mode = m_bus;
}

/**
 * @brief f1_load_srb late: load the S register bank from BUS[12-14]
 * F1=013 corresponds to SRB<- in the emulator, if the RAM size is
 * just 1K. It sets the S register bank for the current task.
 */
void alto2_cpu_device::f1_late_load_srb()
{
	m_s_reg_bank[m_task] = X_RDBITS(m_bus,16,12,14) % m_sreg_banks;
	LOG((this,LOG_RAM,2,"    SRB<-; srb[%d] := %#o\n", m_task, m_s_reg_bank[m_task]));
}

/**
 * @brief RAM related task slots initialization
 */
void alto2_cpu_device::init_ram(int task)
{
	m_ram_related[task] = true;
}

void alto2_cpu_device::exit_ram()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_ram()
{
	m_rdram_flag = false;
	m_wrtram_flag = false;
	m_myl = 0;
	memset(m_s, 0, sizeof(m_s));
}
