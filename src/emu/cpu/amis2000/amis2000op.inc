// AMI S2000 opcode handlers

// internal helpers

void amis2000_device::op_illegal()
{
	logerror("%s unknown opcode $%02X at $%04X\n", tag(), m_op, m_pc);
}


// Register Instructions

void amis2000_device::op_lai()
{
	// LAI X: load ACC with X, select I and K inputs
	op_illegal();
}

void amis2000_device::op_lab()
{
	// LAB: load ACC with BL
	op_illegal();
}

void amis2000_device::op_lae()
{
	// LAE: load ACC with E
	op_illegal();
}

void amis2000_device::op_xab()
{
	// XAB: exchange ACC with BL
	op_illegal();
}

void amis2000_device::op_xabu()
{
	// XABU: exchange ACC with BU
	op_illegal();
}

void amis2000_device::op_xae()
{
	// XAE: exchange ACC with E
	op_illegal();
}

void amis2000_device::op_lbe()
{
	// LBE Y: load BU with Y, load BL with E
	op_illegal();
}

void amis2000_device::op_lbep()
{
	// LBEP Y: load BU with Y, load BL with E+1
	op_illegal();
}

void amis2000_device::op_lbz()
{
	// LBZ Y: load BU with Y, load BL with 0
	op_illegal();
}

void amis2000_device::op_lbf()
{
	// LBF Y: load BU with Y, load BL with 15
	op_illegal();
}


// RAM Instructions

void amis2000_device::op_lam()
{
	// LAM _Y: load ACC with RAM, xor BU with _Y
	op_illegal();
}

void amis2000_device::op_xc()
{
	// XC _Y: exchange ACC with RAM, xor BU with _Y
	op_illegal();
}

void amis2000_device::op_xci()
{
	// XCI _Y: exchange ACC with RAM, increment BL(skip next on carry), xor BU with _Y
	op_illegal();
}

void amis2000_device::op_xcd()
{
	// XCD _Y: exchange ACC with RAM, decrement BL(skip next on carry), xor BU with _Y
	op_illegal();
}

void amis2000_device::op_stm()
{
	// STM Z: set RAM bit Z
	op_illegal();
}

void amis2000_device::op_rsm()
{
	// RSM Z: reset RAM bit Z
	op_illegal();
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
	op_illegal();
}

void amis2000_device::op_jmp()
{
	// JMP X: jump to X(+PP)
	op_illegal();
}

void amis2000_device::op_jms()
{
	// JMS X: call to X(+PP)
	op_illegal();
}

void amis2000_device::op_rt()
{
	// RT: return from subroutine
	op_illegal();
}

void amis2000_device::op_rts()
{
	// RTS: return from subroutine and skip next
	op_illegal();
}

void amis2000_device::op_nop()
{
	// NOP: no operation
}


// Skip Instructions

void amis2000_device::op_szc()
{
	// SZC: skip next on zero(no) carry
	op_illegal();
}

void amis2000_device::op_szm()
{
	// SZM Z: skip next on zero RAM bit Z
	op_illegal();
}

void amis2000_device::op_szi()
{
	// SZI: skip next on zero I pin(s)
	op_illegal();
}

void amis2000_device::op_szk()
{
	// SZK: skip next on zero K pin(s)
	op_illegal();
}

void amis2000_device::op_sbe()
{
	// SBE: skip next on BL equ E
	op_illegal();
}

void amis2000_device::op_sam()
{
	// SAM: skip next on ACC equ RAM
	op_illegal();
}

void amis2000_device::op_sos()
{
	// SOS: skip next on SF(timer output), clear SF
	op_illegal();
}

void amis2000_device::op_tf1()
{
	// TF1: skip next on flag 1
	op_illegal();
}

void amis2000_device::op_tf2()
{
	// TF2: skip next on flag 2
	op_illegal();
}


// Arithmetic and Logical Instructions

void amis2000_device::op_adcs()
{
	// ADCS: add RAM to ACC+carry, skip next on no carry
	op_illegal();
}

void amis2000_device::op_adis()
{
	// ADIS X: add X to ACC, skip next on no carry
	op_illegal();
}

void amis2000_device::op_add()
{
	// ADD: add RAM to ACC
	op_illegal();
}

void amis2000_device::op_and()
{
	// AND: and ACC with RAM
	op_illegal();
}

void amis2000_device::op_xor()
{
	// XOR: xor ACC with RAM
	op_illegal();
}

void amis2000_device::op_stc()
{
	// STC: set carry
	op_illegal();
}

void amis2000_device::op_rsc()
{
	// RSC: reset carry
	op_illegal();
}

void amis2000_device::op_cma()
{
	// CMA: complement ACC
	op_illegal();
}

void amis2000_device::op_sf1()
{
	// SF1: set flag 1
	op_illegal();
}

void amis2000_device::op_rf1()
{
	// RF1: reset flag 1
	op_illegal();
}

void amis2000_device::op_sf2()
{
	// SF2: set flag 2
	op_illegal();
}

void amis2000_device::op_rf2()
{
	// RF2: reset flag 2
	op_illegal();
}
