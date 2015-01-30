// AMI S2000 opcode handlers

// internal helpers

UINT8 amis2000_device::ram_r()
{
	UINT16 address = m_bu << m_bu_bits | m_bl;
	return m_data->read_byte(address) & 0xf;
}

void amis2000_device::ram_w(UINT8 data)
{
	UINT16 address = m_bu << m_bu_bits | m_bl;
	m_data->write_byte(address, data & 0xf);
}

void amis2000_device::pop_callstack()
{
	m_pc = (m_pc & ~m_callstack_mask) | (m_callstack[0] & m_callstack_mask);
	for (int i = 0; i < m_callstack_depth-1; i++)
	{
		m_callstack[i] = m_callstack[i+1];
		m_callstack[i+1] = 0;
	}
}

void amis2000_device::push_callstack()
{
	for (int i = m_callstack_depth-1; i >= 1; i--)
	{
		m_callstack[i] = m_callstack[i-1];
	}
	m_callstack[0] = m_pc & m_callstack_mask;
}

void amis2000_device::op_illegal()
{
	logerror("%s unknown opcode $%02X at $%04X\n", tag(), m_op, m_pc);
}


// Register Instructions

void amis2000_device::op_lai()
{
	// LAI X: load ACC with X, select I and K inputs
	UINT8 param = m_op & 0x0f;
	m_acc = param;
	m_i = m_read_i(0, 0xff) & param;
	m_k = m_read_k(0, 0xff) & param;
}

void amis2000_device::op_lab()
{
	// LAB: load ACC with BL
	m_acc = m_bl;
}

void amis2000_device::op_lae()
{
	// LAE: load ACC with E
	m_acc = m_e;
}

void amis2000_device::op_xab()
{
	// XAB: exchange ACC with BL
	UINT8 old_acc = m_acc;
	m_acc = m_bl;
	m_bl = old_acc;
}

void amis2000_device::op_xabu()
{
	// XABU: exchange ACC with BU
	UINT8 old_acc = m_acc;
	m_acc = (m_acc & ~m_bu_mask) | (m_bu & m_bu_mask);
	m_bu = old_acc & m_bu_mask;
}

void amis2000_device::op_xae()
{
	// XAE: exchange ACC with E
	UINT8 old_acc = m_acc;
	m_acc = m_e;
	m_e = old_acc;
}

void amis2000_device::op_lbe()
{
	// LBE Y: load BU with Y, load BL with E
	UINT8 param = m_op & 0x03;
	m_bu = param & m_bu_mask;
	m_bl = m_e;
}

void amis2000_device::op_lbep()
{
	// LBEP Y: load BU with Y, load BL with E+1
	UINT8 param = m_op & 0x03;
	m_bu = param & m_bu_mask;
	m_bl = m_e + 1;
}

void amis2000_device::op_lbz()
{
	// LBZ Y: load BU with Y, load BL with 0
	UINT8 param = m_op & 0x03;
	m_bu = param & m_bu_mask;
	m_bl = 0;
}

void amis2000_device::op_lbf()
{
	// LBF Y: load BU with Y, load BL with 15
	UINT8 param = m_op & 0x03;
	m_bu = param & m_bu_mask;
	m_bl = 0xf;
}


// RAM Instructions

void amis2000_device::op_lam()
{
	// LAM _Y: load ACC with RAM, xor BU with _Y
	m_acc = ram_r();
	UINT8 param = ~m_op & 0x03;
	m_bu ^= param & m_bu_mask;
}

void amis2000_device::op_xc()
{
	// XC _Y: exchange ACC with RAM, xor BU with _Y
	UINT8 old_acc = m_acc;
	m_acc = ram_r();
	ram_w(old_acc);
	UINT8 param = ~m_op & 0x03;
	m_bu ^= param & m_bu_mask;
}

void amis2000_device::op_xci()
{
	// XCI _Y: exchange ACC with RAM, increment BL(skip next on carry), xor BU with _Y
	op_xc();
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 0);
}

void amis2000_device::op_xcd()
{
	// XCD _Y: exchange ACC with RAM, decrement BL(skip next on carry), xor BU with _Y
	op_xc();
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}

void amis2000_device::op_stm()
{
	// STM Z: set RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	ram_w(ram_r() | param);
}

void amis2000_device::op_rsm()
{
	// RSM Z: reset RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	ram_w(ram_r() & ~param);
}


// Input/Output Instructions

void amis2000_device::op_inp()
{
	// INP: input D-pins to ACC and RAM
	op_illegal();
}

void amis2000_device::op_out()
{
	// OUT: pulse output ACC and RAM to D-pins
	op_illegal();
}

void amis2000_device::op_disb()
{
	// DISB: set D-latch to ACC and RAM directly
	op_illegal();
}

void amis2000_device::op_disn()
{
	// DISN: set D-latch to ACC+carry via segment decoder
	op_illegal();
}

void amis2000_device::op_mvs()
{
	// MVS: output master strobe latch to A-pins
	op_illegal();
}

void amis2000_device::op_psh()
{
	// PSH: preset high(BL) master strobe latch
	op_illegal();
}

void amis2000_device::op_psl()
{
	// PSL: preset low(BL) master strobe latch
	op_illegal();
}

void amis2000_device::op_eur()
{
	// EUR: set timer frequency(European) and D-latch polarity
	op_illegal();
}


// Program Control Instructions

void amis2000_device::op_pp()
{
	// PP _X: prepare page/bank with _X
	UINT8 param = ~m_op & 0x0f;
	if (m_pp_index == 0)
		m_ppr = param;
	else
		m_pbr = param & 7;
}

void amis2000_device::op_jmp()
{
	// JMP X: jump to X(+PP)
	UINT16 mask = 0x3f;
	UINT16 param = m_op & mask;
	if (m_pp_index > 0)
	{
		param |= m_ppr << 6;
		mask |= 0x3c0;
	}
	if (m_pp_index > 1)
	{
		param |= m_pbr << 10;
		mask |= 0x1c00;
	}
	m_pc = (m_pc & ~mask) | param;
}

void amis2000_device::op_jms()
{
	// JMS X: call to X(+PP)
	m_icount--;
	push_callstack();
	if (m_pp_index == 0)
	{
		// subroutines default location is page 15
		m_ppr = 0xf;
		m_pp_index++;
	}
	op_jmp();
}

void amis2000_device::op_rt()
{
	// RT: return from subroutine
	pop_callstack();
}

void amis2000_device::op_rts()
{
	// RTS: return from subroutine and skip next
	op_rt();
	m_skip = true;
}

void amis2000_device::op_nop()
{
	// NOP: no operation
}


// Skip Instructions

void amis2000_device::op_szc()
{
	// SZC: skip next on zero(no) carry
	m_skip = !m_carry;
}

void amis2000_device::op_szm()
{
	// SZM Z: skip next on zero RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	m_skip = !(ram_r() & param);
}

void amis2000_device::op_szi()
{
	// SZI: skip next on zero I pin(s)
	m_skip = !m_i;
}

void amis2000_device::op_szk()
{
	// SZK: skip next on zero K pin(s)
	m_skip = !m_k;
}

void amis2000_device::op_sbe()
{
	// SBE: skip next on BL equ E
	m_skip = (m_bl == m_e);
}

void amis2000_device::op_sam()
{
	// SAM: skip next on ACC equ RAM
	m_skip = (m_acc == ram_r());
}

void amis2000_device::op_sos()
{
	// SOS: skip next on SF(timer output), clear SF
	op_illegal();
}

void amis2000_device::op_tf1()
{
	// TF1: skip next on flag 1
	m_skip = (m_f & 0x01);
}

void amis2000_device::op_tf2()
{
	// TF2: skip next on flag 2
	m_skip = (m_f & 0x02);
}


// Arithmetic and Logical Instructions

void amis2000_device::op_adcs()
{
	// ADCS: add RAM to ACC+carry, skip next on no carry
	m_acc += ram_r() + m_carry;
	m_carry = m_acc >> 4 & 1;
	m_skip = !m_carry;
	m_acc &= 0xf;
}

void amis2000_device::op_adis()
{
	// ADIS X: add X to ACC, skip next on no carry
	UINT8 param = m_op & 0x0f;
	m_acc += param;
	m_skip = !(m_acc >> 4 & 1);
	m_acc &= 0xf;
}

void amis2000_device::op_add()
{
	// ADD: add RAM to ACC
	m_acc = (m_acc + ram_r()) & 0xf;
}

void amis2000_device::op_and()
{
	// AND: and ACC with RAM
	m_acc &= ram_r();
}

void amis2000_device::op_xor()
{
	// XOR: xor ACC with RAM
	m_acc ^= ram_r();
}

void amis2000_device::op_stc()
{
	// STC: set carry
	m_carry = 1;
}

void amis2000_device::op_rsc()
{
	// RSC: reset carry
	m_carry = 0;
}

void amis2000_device::op_cma()
{
	// CMA: complement ACC
	m_acc ^= 0xf;
}

void amis2000_device::op_sf1()
{
	// SF1: set flag 1
	m_f |= 0x01;
}

void amis2000_device::op_rf1()
{
	// RF1: reset flag 1
	m_f &= ~0x01;
}

void amis2000_device::op_sf2()
{
	// SF2: set flag 2
	m_f |= 0x02;
}

void amis2000_device::op_rf2()
{
	// RF2: reset flag 2
	m_f &= ~0x02;
}
