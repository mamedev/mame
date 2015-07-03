// license:BSD-3-Clause
// copyright-holders:hap

// MELPS 4 opcode handlers

#include "melps4.h"


// internal helpers

inline UINT8 melps4_cpu_device::ram_r()
{
	UINT8 address = (m_z << 6 | m_x << 4 | m_y) & m_datamask;
	return m_data->read_byte(address) & 0xf;
}

inline void melps4_cpu_device::ram_w(UINT8 data)
{
	UINT8 address = (m_z << 6 | m_x << 4 | m_y) & m_datamask;
	m_data->write_byte(address, data & 0xf);
}

void melps4_cpu_device::pop_pc()
{
	m_pc = m_stack[0];
	for (int i = 0; i < m_stack_levels-1; i++)
		m_stack[i] = m_stack[i+1];
}

void melps4_cpu_device::push_pc()
{
	for (int i = m_stack_levels-1; i >= 1; i--)
		m_stack[i] = m_stack[i-1];
	m_stack[0] = m_pc;
}


// Register-to-register transfers

void melps4_cpu_device::op_tab()
{
	// TAB: transfer B to A
	m_a = m_b;
}

void melps4_cpu_device::op_tba()
{
	// TBA: transfer A to B
	m_b = m_a;
}

void melps4_cpu_device::op_tay()
{
	// TAY: transfer Y to A
	m_a = m_y;
}

void melps4_cpu_device::op_tya()
{
	// TYA: transfer A to Y
	m_y = m_a;
}

void melps4_cpu_device::op_teab()
{
	// TEAB: transfer A and B to E
	m_e = m_b << 4 | m_a;
}

void melps4_cpu_device::op_tabe()
{
	// TABE(undocumented): transfer E to A and B
	m_a = m_e & 0xf;
	m_b = m_e >> 4;
}

void melps4_cpu_device::op_tepa()
{
	// TEPA: decode A by PLA and transfer to E
	op_illegal();
}

void melps4_cpu_device::op_txa()
{
	// TXA: transfer bits 0,1 of A to X, inverted bit 2 to Z, inverted bit 3 to carry
	op_illegal();
}

void melps4_cpu_device::op_tax()
{
	// TAX: transfer X to bits 0,1 of A, inverted Z to bit 2, inverted carry to bit 3
	op_illegal();
}


// RAM addresses

void melps4_cpu_device::op_lxy()
{
	// LXY x,y: load immediate into X,Y, skip any next LXY
	m_prohibit_irq = true;
	if ((m_op & ~0x3f) != (m_prev_op & ~0x3f))
	{
		m_x = m_op >> 4 & 3;
		m_y = m_op & 0xf;
	}
}

void melps4_cpu_device::op_lz()
{
	// LZ z: load immediate into Z
	m_z = m_op & 1;
}

void melps4_cpu_device::op_iny()
{
	// INY: increment Y, skip next on overflow
	m_y = (m_y + 1) & 0xf;
	m_skip = (m_y == 0);
}

void melps4_cpu_device::op_dey()
{
	// DEY: decrement Y, skip next on overflow
	m_y = (m_y - 1) & 0xf;
	m_skip = (m_y == 0xf);
}

void melps4_cpu_device::op_lcps()
{
	// LCPS i: choose active DP,CY or DP',CY'
	if ((m_op & 1) != m_cps)
	{
		m_cps = m_op & 1;

		// swap registers
		UINT8 x, y, z, cy;
		x = m_x;
		y = m_y;
		z = m_z;
		cy = m_cy;

		m_x = m_x2;
		m_y = m_y2;
		m_z = m_z2;
		m_cy = m_cy2;

		m_x2 = x;
		m_y2 = y;
		m_z2 = z;
		m_cy2 = cy;
	}
}

void melps4_cpu_device::op_sadr()
{
	// SADR j: ..
	op_illegal();
}


// RAM-accumulator transfers

void melps4_cpu_device::op_tam()
{
	// TAM j: transfer RAM to A, xor X with j
	m_a = ram_r();
	m_x ^= m_op & 3;
}

void melps4_cpu_device::op_xam()
{
	// XAM j: exchange RAM with A, xor X with j
	UINT8 a = m_a;
	m_a = ram_r();
	ram_w(a);
	m_x ^= m_op & 3;
}

void melps4_cpu_device::op_xamd()
{
	// XAMD j: XAM j, DEY
	op_xam();
	op_dey();
}

void melps4_cpu_device::op_xami()
{
	// XAMI j: XAM j, skip next on Y mask(default 0xf), increment Y
	op_xam();
	m_skip = ((m_y & m_xami_mask) == m_xami_mask);
	m_y = (m_y + 1) & 0xf;
}


// Arithmetic Operations

void melps4_cpu_device::op_la()
{
	// LA n: load immediate into A, skip any next LA
	m_prohibit_irq = true;
	if ((m_op & ~0xf) != (m_prev_op & ~0xf))
		m_a = m_op & 0xf;
}

void melps4_cpu_device::op_am()
{
	// AM: add RAM to A
	m_a = (m_a + ram_r()) & 0xf;
}

void melps4_cpu_device::op_amc()
{
	// AMC: add RAM+CY to A and CY
	m_a += ram_r() + m_cy;
	m_cy = m_a >> 4 & 1;
	m_a &= 0xf;
}

void melps4_cpu_device::op_amcs()
{
	// AMCS: AMC, skip next on carry
	op_amc();
	m_skip = (m_cy != 0);
}

void melps4_cpu_device::op_a()
{
	// A n: add immediate to A, skip next on no carry (except when n=6)
	UINT8 n = m_op & 0xf;
	m_a += n;
	m_skip = !(m_a & 0x10 || n == 6);
	m_a &= 0xf;
}

void melps4_cpu_device::op_sc()
{
	// SC: set carry
	m_cy = 1;
}

void melps4_cpu_device::op_rc()
{
	// RC: reset carry
	m_cy = 0;
}

void melps4_cpu_device::op_szc()
{
	// SZC: skip next on no carry
	m_skip = !m_cy;
}

void melps4_cpu_device::op_cma()
{
	// CMA: complement A
	m_a ^= 0xf;
}

void melps4_cpu_device::op_rl()
{
	// RL(undocumented): rotate A left through carry
	UINT8 c = m_a >> 3 & 1;
	m_a = (m_a << 1 | m_cy) & 0xf;
	m_cy = c;
}

void melps4_cpu_device::op_rr()
{
	// RR(undocumented): rotate A right through carry
	UINT8 c = m_a & 1;
	m_a = m_a >> 1 | m_cy << 3;
	m_cy = c;
}


// Bit operations

void melps4_cpu_device::op_sb()
{
	// SB j: set RAM bit
	ram_w(ram_r() | m_bitmask);
}

void melps4_cpu_device::op_rb()
{
	// RB j: reset RAM bit
	ram_w(ram_r() & ~m_bitmask);
}

void melps4_cpu_device::op_szb()
{
	// SZB j: skip next if RAM bit is 0
	m_skip = !(ram_r() & m_bitmask);
}


// Compares

void melps4_cpu_device::op_seam()
{
	// SEAM: skip next if A equals RAM
	m_skip = (m_a == ram_r());
}

void melps4_cpu_device::op_sey()
{
	// SEY y: skip next if Y equals immediate
	m_skip = (m_y == (m_op & 0xf));
}


// A/D converter operations

void melps4_cpu_device::op_tla()
{
	// TLA: transfer A to L
	m_l = m_a;
}

void melps4_cpu_device::op_tha()
{
	// THA: transfer A to H
	m_h = m_a;
}

void melps4_cpu_device::op_taj()
{
	// TAJ: transfer J(hi/lo) to A designated by Y
	op_illegal();
}

void melps4_cpu_device::op_xal()
{
	// XAL: exchange A with L
	UINT8 a = m_a;
	m_a = m_l;
	m_l = a;
}

void melps4_cpu_device::op_xah()
{
	// XAH: exchange A with H
	UINT8 a = m_a;
	m_a = m_h;
	m_h = a;
}

void melps4_cpu_device::op_lc7()
{
	// LC7: load 7 into C
	m_c = 7;
}

void melps4_cpu_device::op_dec()
{
	// DEC: decrement C, skip next on overflow
	m_c = (m_c - 1) & 7;
	m_skip = (m_c == 7);
}

void melps4_cpu_device::op_shl()
{
	// SHL: set bit in L or H designated by C
	UINT8 mask = 1 << (m_c & 3);
	if (m_c & 4)
		m_h |= mask;
	else
		m_l |= mask;
}

void melps4_cpu_device::op_rhl()
{
	// RHL: reset bit in L or H designated by C
	UINT8 mask = 1 << (m_c & 3);
	if (m_c & 4)
		m_h &= ~mask;
	else
		m_l &= ~mask;
}

void melps4_cpu_device::op_cpa()
{
	// CPA: ..
	op_illegal();
}

void melps4_cpu_device::op_cpas()
{
	// CPAS: ..
	op_illegal();
}

void melps4_cpu_device::op_cpae()
{
	// CPAE: ..
	op_illegal();
}

void melps4_cpu_device::op_szj()
{
	// SZJ: skip next if J bit designated by Y is 0
	op_illegal();
}


// Timer instruction

void melps4_cpu_device::op_t1ab()
{
	// T1AB: transfer A and B to timer 1
	m_tmr_count[0] = m_b << 4 | m_a;
}

void melps4_cpu_device::op_trab()
{
	// TRAB: transfer A and B to timer 2 reload
	m_tmr_reload = m_b << 4 | m_a;
}

void melps4_cpu_device::op_t2ab()
{
	// T2AB: transfer A and B to timer 2 and timer 2 reload
	m_tmr_reload = m_tmr_count[1] = m_b << 4 | m_a;
}

void melps4_cpu_device::op_tab1()
{
	// TAB1: transfer timer 1 to A and B
	m_a = m_tmr_count[0] & 0xf;
	m_b = m_tmr_count[0] >> 4;
}

void melps4_cpu_device::op_tabr()
{
	// TABR: transfer timer 2 reload to A and B
	m_a = m_tmr_reload & 0xf;
	m_b = m_tmr_reload >> 4;
}

void melps4_cpu_device::op_tab2()
{
	// TAB2: transfer timer 2 to A and B
	m_a = m_tmr_count[1] & 0xf;
	m_b = m_tmr_count[1] >> 4;
}

void melps4_cpu_device::op_tva()
{
	// TVA: transfer A to timer control V
	write_v(m_a);
}

void melps4_cpu_device::op_twa()
{
	// TWA: transfer A to timer control W
	write_w(m_a);
}

void melps4_cpu_device::op_snz1()
{
	// SNZ1: skip next on flag 1F
	m_skip = m_irqflag[1];
	m_irqflag[1] = false;
}

void melps4_cpu_device::op_snz2()
{
	// SNZ2: skip next on flag 2F
	m_skip = m_irqflag[2];
	m_irqflag[2] = false;
}


// Jumps

void melps4_cpu_device::op_ba()
{
	// BA: indicate next branch is indirect
	m_prohibit_irq = true;
	m_ba_flag = true;
}

void melps4_cpu_device::op_sp()
{
	// SP: set page for next branch
	// note: mnemonic is guessed, manual names it BL or BML
	m_prohibit_irq = true;
	m_sp_param = m_op & 0xf;
}

void melps4_cpu_device::op_b()
{
	// B xy: branch
	m_prohibit_irq = true;

	// determine new page:
	// - short call: subroutine page
	// - short jump: current page, or sub. page + 1 when in sub. mode
	// - long jump/call(B/BM preceded by SP): temp SP register
	UINT8 page = m_pc >> 7;
	if ((m_prev_op & ~0xf) == m_sp_mask)
	{
		m_sm = false;
		page = m_sp_param;
	}
	else if (m_sm)
		page = m_sm_page | (m_op >> 7 & 1);

	m_pc = page << 7 | (m_op & 0x7f);

	// if BA opcode was executed, set PC low 4 bits to A
	if (m_ba_flag)
	{
		m_ba_flag = false;
		m_pc = (m_pc & ~0xf) | m_a;
	}
}

void melps4_cpu_device::op_bm()
{
	// BM xy call subroutine
	// don't push stack on short calls when in subroutine mode
	if (!m_sm || (m_prev_op & ~0xf) == m_sp_mask)
		push_pc();

	// set subroutine mode - it is reset after long jump/call or return
	m_sm = true;
	op_b();
}


// Program returns

void melps4_cpu_device::op_rt()
{
	// RT: return from subroutine
	m_prohibit_irq = true;
	m_sm = false;
	pop_pc();
}

void melps4_cpu_device::op_rts()
{
	// RTS: RT, skip next
	op_rt();
	m_skip = true;
}

void melps4_cpu_device::op_rti()
{
	// RTI: return from interrupt routine
	op_rt();
	m_sm = m_sms;
}


// Input/Output

void melps4_cpu_device::op_cld()
{
	// CLD: clear port D
	write_d_pin(MELPS4_PORTD_CLR, 0);
}

void melps4_cpu_device::op_cls()
{
	// CLS: clear port S
	write_gen_port(MELPS4_PORTS, 0);
}

void melps4_cpu_device::op_clds()
{
	// CLDS: CLD, CLS
	op_cld();
	op_cls();
}

void melps4_cpu_device::op_sd()
{
	// SD: set port D pin designated by Y
	write_d_pin(m_y, 1);
}

void melps4_cpu_device::op_rd()
{
	// RD: reset port D pin designated by Y
	write_d_pin(m_y, 0);
}

void melps4_cpu_device::op_szd()
{
	// SZD: skip next if port D pin designated by Y is 0
	m_skip = !read_d_pin(m_y);
}

void melps4_cpu_device::op_osab()
{
	// OSAB: output A and B to port S
	write_gen_port(MELPS4_PORTS, m_b << 4 | m_a);
}

void melps4_cpu_device::op_ospa()
{
	// OSPA: decode A by PLA and output to port S
	op_illegal();
}

void melps4_cpu_device::op_ose()
{
	// OSE: output E to port S
	write_gen_port(MELPS4_PORTS, m_e);
}

void melps4_cpu_device::op_ias()
{
	// IAS i: transfer port S(hi/lo) to A
	int shift = (m_op & 1) ? 0 : 4;
	m_a = read_gen_port(MELPS4_PORTS) >> shift & 0xf;
}

void melps4_cpu_device::op_ofa()
{
	// OFA: output A to port F
	write_gen_port(MELPS4_PORTF, m_a);
}

void melps4_cpu_device::op_iaf()
{
	// IAF: input port F to A
	m_a = read_gen_port(MELPS4_PORTF);
}

void melps4_cpu_device::op_oga()
{
	// OGA: output A to port G
	write_gen_port(MELPS4_PORTG, m_a);
}

void melps4_cpu_device::op_iak()
{
	// IAK: input port K to A
	m_a = m_read_k(0, 0xffff) & 0xf;
}

void melps4_cpu_device::op_szk()
{
	// SZK j: skip next if port K bit is reset
	m_skip = !(m_read_k(0, 0xffff) & m_bitmask);
}

void melps4_cpu_device::op_su()
{
	// SU/RU: set/reset port U
	write_gen_port(MELPS4_PORTU, m_op & 1);
}


// Interrupts

void melps4_cpu_device::op_ei()
{
	// EI: enable interrupt flag
	m_prohibit_irq = true;
	m_possible_irq = true;
	m_inte = 1;
}

void melps4_cpu_device::op_di()
{
	// DI: disable interrupt flag
	m_prohibit_irq = true;
	m_inte = 0;
}

void melps4_cpu_device::op_inth()
{
	// INTH: set external interrupt polarity high (rising edge)
	m_intp = 1;
}

void melps4_cpu_device::op_intl()
{
	// INTL: set external interrupt polarity low (falling edge)
	m_intp = 0;
}


// Misc

void melps4_cpu_device::op_nop()
{
	// NOP: no operation
}

void melps4_cpu_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
}
