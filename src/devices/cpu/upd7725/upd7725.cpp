// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    upd7725.c

    Core implementation for the portable NEC uPD7725/uPD96050 emulator

    Original by byuu in the public domain.
    MAME conversion by R. Belmont

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "upd7725.h"


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

// device type definition
const device_type UPD7725 = &device_creator<upd7725_device>;
const device_type UPD96050 = &device_creator<upd96050_device>;

necdsp_device::necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, UINT32 abits, UINT32 dbits, const char *name, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_BIG, 32, abits, -2), // data bus width, address bus width, -2 means DWORD-addressable
		m_data_config("data", ENDIANNESS_BIG, 16, dbits, -1), m_icount(0),   // -1 for WORD-addressable
		m_irq(0),
		m_program(nullptr),
		m_data(nullptr),
		m_direct(nullptr),
		m_in_int_cb(*this),
		//m_in_si_cb(*this),
		//m_in_sck_cb(*this),
		//m_in_sien_cb(*this),
		//m_in_soen_cb(*this),
		//m_in_dack_cb(*this),
		m_out_p0_cb(*this),
		m_out_p1_cb(*this)
		//m_out_so_cb(*this),
		//m_out_sorq_cb(*this),
		//m_out_drq_cb(*this)
{
}


upd7725_device::upd7725_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: necdsp_device(mconfig, UPD7725, tag, owner, clock, 11, 11, "uPD7725", "upd7725", __FILE__)
{
}

upd96050_device::upd96050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: necdsp_device(mconfig, UPD96050, tag, owner, clock, 14, 12, "uPD96050", "upd96050", __FILE__)
{
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void necdsp_device::device_start()
{
	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_direct = &m_program->direct();

	// register our state for the debugger
	state_add(STATE_GENPC, "GENPC", regs.pc).noshow();
	state_add(UPD7725_PC, "PC", regs.pc);
	state_add(UPD7725_RP, "RP", regs.rp);
	state_add(UPD7725_DP, "DP", regs.dp);
	state_add(UPD7725_SP, "SP", regs.sp);
	state_add(UPD7725_K, "K", regs.k);
	state_add(UPD7725_L, "L", regs.l);
	state_add(UPD7725_M, "M", regs.m);
	state_add(UPD7725_N, "N", regs.n);
	state_add(UPD7725_A, "A", regs.a);
	state_add(UPD7725_B, "B", regs.b);
	state_add(UPD7725_TR, "TR", regs.tr);
	state_add(UPD7725_TRB, "TRB", regs.trb);
	state_add(UPD7725_DR, "DR", regs.dr);
	state_add(UPD7725_SI, "SI", regs.si);
	state_add(UPD7725_SO, "SO", regs.so);
	state_add(UPD7725_IDB, "IDB", regs.idb);

	// resolve callbacks
	m_in_int_cb.resolve_safe(0);
	//m_in_si_cb.resolve_safe(0);
	//m_in_sck_cb.resolve_safe(0);
	//m_in_sien_cb.resolve_safe(0);
	//m_in_soen_cb.resolve_safe(0);
	//m_in_dack_cb.resolve_safe(0);
	m_out_p0_cb.resolve_safe();
	m_out_p1_cb.resolve_safe();
	//m_out_so_cb.resolve_safe();
	//m_out_sorq_cb.resolve_safe();
	//m_out_drq_cb.resolve_safe();

	// save state registrations
	save_item(NAME(regs.pc));
	save_item(NAME(regs.rp));
	save_item(NAME(regs.dp));
	save_item(NAME(regs.sp));
	save_item(NAME(regs.k));
	save_item(NAME(regs.l));
	save_item(NAME(regs.m));
	save_item(NAME(regs.n));
	save_item(NAME(regs.a));
	save_item(NAME(regs.b));
	save_item(NAME(regs.tr));
	save_item(NAME(regs.trb));
	save_item(NAME(regs.dr));
	save_item(NAME(regs.so));
	save_item(NAME(regs.idb));
	save_item(NAME(regs.sr.rqm));
	save_item(NAME(regs.sr.usf0));
	save_item(NAME(regs.sr.usf1));
	save_item(NAME(regs.sr.drs));
	save_item(NAME(regs.sr.dma));
	save_item(NAME(regs.sr.drc));
	save_item(NAME(regs.sr.soc));
	save_item(NAME(regs.sr.sic));
	save_item(NAME(regs.sr.ei));
	save_item(NAME(regs.sr.p0));
	save_item(NAME(regs.sr.p1));
	save_item(NAME(regs.stack));
	save_item(NAME(dataRAM));

	m_icountptr = &m_icount;
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void necdsp_device::device_reset()
{
	for (auto & elem : dataRAM)
	{
		elem = 0x0000;
	}

	regs.pc = 0x0000;
	regs.rp = 0x0000;
	regs.dp = 0x0000;
	regs.sp = 0x0;
	regs.k = 0x0000;
	regs.l = 0x0000;
	regs.m = 0x0000;
	regs.n = 0x0000;
	regs.a = 0x0000;
	regs.b = 0x0000;
	regs.flaga = 0x00;
	regs.flagb = 0x00;
	regs.tr = 0x0000;
	regs.trb = 0x0000;
	regs.sr = 0x0000;
	regs.dr = 0x0000;
	regs.si = 0x0000;
	regs.so = 0x0000;
	regs.idb = 0x0000;
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *necdsp_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : &m_data_config;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void necdsp_device::state_import(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void necdsp_device::state_export(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void necdsp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case UPD7725_FLAGA:
			strprintf(str, "%s %s %c%c %s %s %s %s",
							regs.flaga.s1 ? "S1" : "s1",
							regs.flaga.s0 ? "S0" : "s0",
							regs.flaga.c ? 'C' : 'c',
							regs.flaga.z ? 'Z' : 'z',
							regs.flaga.ov1 ? "OV1" : "ov1",
							regs.flaga.ov0 ? "OV0" : "ov0",
							regs.flaga.ov0p ? "OV0P" : "ov0p",
							regs.flaga.ov0pp ? "OV0PP" : "ov0pp");
			break;

		case UPD7725_FLAGB:
			strprintf(str, "%s %s %c%c %s %s %s %s",
							regs.flagb.s1 ? "S1" : "s1",
							regs.flagb.s0 ? "S0" : "s0",
							regs.flagb.c ? 'C' : 'c',
							regs.flagb.z ? 'Z' : 'z',
							regs.flagb.ov1 ? "OV1" : "ov1",
							regs.flagb.ov0 ? "OV0" : "ov0",
							regs.flagb.ov0p ? "OV0P" : "ov0p",
							regs.flagb.ov0pp ? "OV0PP" : "ov0pp");
			break;
	}
}

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 necdsp_device::execute_min_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 necdsp_device::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 necdsp_device::execute_input_lines() const
{
	return 3; // TODO: there should be 11: INT, SCK, /SIEN, /SOEN, SI, and /DACK, plus SO, /SORQ and DRQ; for now, just INT, P0, and P1 are enough.
}


//-------------------------------------------------
//  execute_set_input -
//-------------------------------------------------

void necdsp_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case NECDSP_INPUT_LINE_INT:
		//TODO: detect rising edge; if rising edge found AND IE = 1, push PC, pc = 0x100; else do nothing
		m_irq = state; // set old state to current state
		break;
	// add more when needed
	}
}

//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 necdsp_device::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 necdsp_device::disasm_max_opcode_bytes() const
{
	return 4;
}

//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t necdsp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd7725 );
	return CPU_DISASSEMBLE_NAME(upd7725)(this, buffer, pc, oprom, opram, options);
}

void necdsp_device::execute_run()
{
	UINT32 opcode;

	do
	{
		// call debugger hook if necessary
		if (device_t::machine().debug_flags & DEBUG_FLAG_ENABLED)
		{
			debugger_instruction_hook(this, regs.pc);
		}

		opcode = m_direct->read_dword(regs.pc<<2)>>8;
		regs.pc++;
		switch(opcode >> 22)
		{
			case 0: exec_op(opcode); break;
			case 1: exec_rt(opcode); break;
			case 2: exec_jp(opcode); break;
			case 3: exec_ld(opcode); break;
		}

		INT32 result = (INT32)regs.k * regs.l;  //sign + 30-bit result
		regs.m = result >> 15;  //store sign + top 15-bits
		regs.n = result <<  1;  //store low 15-bits + zero

		m_icount--;

	} while (m_icount > 0);
}

void necdsp_device::exec_op(UINT32 opcode) {
	UINT8 pselect = (opcode >> 20)&0x3;  //P select
	UINT8 alu     = (opcode >> 16)&0xf;  //ALU operation mode
	UINT8 asl     = (opcode >> 15)&0x1;  //accumulator select
	UINT8 dpl     = (opcode >> 13)&0x3;  //DP low modify
	UINT8 dphm    = (opcode >>  9)&0xf;  //DP high XOR modify
	UINT8 rpdcr   = (opcode >>  8)&0x1;  //RP decrement
	UINT8 src     = (opcode >>  4)&0xf;  //move source
	UINT8 dst     = (opcode >>  0)&0xf;  //move destination

	switch(src) {
	case  0: regs.idb = regs.trb; break;
	case  1: regs.idb = regs.a; break;
	case  2: regs.idb = regs.b; break;
	case  3: regs.idb = regs.tr; break;
	case  4: regs.idb = regs.dp; break;
	case  5: regs.idb = regs.rp; break;
	case  6: regs.idb = m_data->read_word(regs.rp<<1); break;
	case  7: regs.idb = 0x8000 - regs.flaga.s1; break;  //SGN
	case  8: regs.idb = regs.dr; regs.sr.rqm = 1; break;
	case  9: regs.idb = regs.dr; break;
	case 10: regs.idb = regs.sr; break;
	case 11: regs.idb = regs.si; break;  //MSB
	case 12: regs.idb = regs.si; break;  //LSB
	case 13: regs.idb = regs.k; break;
	case 14: regs.idb = regs.l; break;
	case 15: regs.idb = dataRAM[regs.dp]; break;
	}

	if(alu) {
	UINT16 p=0, q=0, r=0;
	Flag flag;
	bool c=0;

	flag.c = 0;
	flag.s1 = 0;
	flag.ov0 = 0;
	flag.ov1 = 0;
	flag.ov0p = 0;
	flag.ov0pp = 0;

	switch(pselect) {
		case 0: p = dataRAM[regs.dp]; break;
		case 1: p = regs.idb; break;
		case 2: p = regs.m; break;
		case 3: p = regs.n; break;
	}

	switch(asl) {
		case 0: q = regs.a; flag = regs.flaga; c = regs.flagb.c; break;
		case 1: q = regs.b; flag = regs.flagb; c = regs.flaga.c; break;
	}

	switch(alu) {
		case  1: r = q | p; break;                    //OR
		case  2: r = q & p; break;                    //AND
		case  3: r = q ^ p; break;                    //XOR
		case  4: r = q - p; break;                    //SUB
		case  5: r = q + p; break;                    //ADD
		case  6: r = q - p - c; break;                //SBB
		case  7: r = q + p + c; break;                //ADC
		case  8: r = q - 1; p = 1; break;             //DEC
		case  9: r = q + 1; p = 1; break;             //INC
		case 10: r = ~q; break;                       //CMP
		case 11: r = (q >> 1) | (q & 0x8000); break;  //SHR1 (ASR)
		case 12: r = (q << 1) | (c ? 1 : 0); break;             //SHL1 (ROL)
		case 13: r = (q << 2) | 3; break;             //SHL2
		case 14: r = (q << 4) | 15; break;            //SHL4
		case 15: r = (q << 8) | (q >> 8); break;      //XCHG
	}

	flag.s0 = (r & 0x8000);
	flag.z = (r == 0);
	flag.ov0pp = flag.ov0p;
	flag.ov0p = flag.ov0;

	switch(alu) {
		case  1: case  2: case  3: case 10: case 13: case 14: case 15: {
		flag.c = 0;
		flag.ov0 = flag.ov0p = flag.ov0pp = 0; // ASSUMPTION: previous ov0 values are nulled here to make ov1 zero
		break;
		}
		case  4: case  5: case  6: case  7: case  8: case  9: {
		if(alu & 1) {
			//addition
			flag.ov0 = (q ^ r) & ~(q ^ p) & 0x8000;
			flag.c = (r < q);
		} else {
			//subtraction
			flag.ov0 = (q ^ r) &  (q ^ p) & 0x8000;
			flag.c = (r > q);
		}
		break;
		}
		case 11: {
		flag.c = q & 1;
		flag.ov0 = flag.ov0p = flag.ov0pp = 0; // ASSUMPTION: previous ov0 values are nulled here to make ov1 zero
		break;
		}
		case 12: {
		flag.c = q >> 15;
		flag.ov0 = flag.ov0p = flag.ov0pp = 0; // ASSUMPTION: previous ov0 values are nulled here to make ov1 zero
		break;
		}
	}
	// flag.ov1 is only set if the number of overflows of the past 3 opcodes (of type 4,5,6,7,8,9) is odd
	flag.ov1 = (flag.ov0 + flag.ov0p + flag.ov0pp) & 1;
	// flag.s1 is based on ov1: s1 = ov1 ^ s0;
	flag.s1 = flag.ov1 ^ flag.s0;

	switch(asl) {
		case 0: regs.a = r; regs.flaga = flag; break;
		case 1: regs.b = r; regs.flagb = flag; break;
	}
	}

	exec_ld((regs.idb << 6) + dst);

	switch(dpl) {
	case 1: regs.dp = (regs.dp & 0xf0) + ((regs.dp + 1) & 0x0f); break;  //DPINC
	case 2: regs.dp = (regs.dp & 0xf0) + ((regs.dp - 1) & 0x0f); break;  //DPDEC
	case 3: regs.dp = (regs.dp & 0xf0); break;  //DPCLR
	}

	regs.dp ^= dphm << 4;

	if(rpdcr) regs.rp--;
}

void necdsp_device::exec_rt(UINT32 opcode) {
	exec_op(opcode);
	regs.pc = regs.stack[--regs.sp];
	regs.sp &= 0xf;
}

void necdsp_device::exec_jp(UINT32 opcode) {
	UINT16 brch = (opcode >> 13) & 0x1ff;  //branch
	UINT16 na  =  (opcode >>  2) & 0x7ff;  //next address
	UINT16 bank = (opcode >>  0) & 0x3;  //bank address

	UINT16 jps = (regs.pc & 0x2000) | (bank << 11) | (na << 0);
	UINT16 jpl = (bank << 11) | (na << 0);

	switch(brch) {
		case 0x000: regs.pc = regs.so; return;  //JMPSO

		case 0x080: if(regs.flaga.c == 0) regs.pc = jps; return;  //JNCA
		case 0x082: if(regs.flaga.c == 1) regs.pc = jps; return;  //JCA
		case 0x084: if(regs.flagb.c == 0) regs.pc = jps; return;  //JNCB
		case 0x086: if(regs.flagb.c == 1) regs.pc = jps; return;  //JCB

		case 0x088: if(regs.flaga.z == 0) regs.pc = jps; return;  //JNZA
		case 0x08a: if(regs.flaga.z == 1) regs.pc = jps; return;  //JZA
		case 0x08c: if(regs.flagb.z == 0) regs.pc = jps; return;  //JNZB
		case 0x08e: if(regs.flagb.z == 1) regs.pc = jps; return;  //JZB

		case 0x090: if(regs.flaga.ov0 == 0) regs.pc = jps; return;  //JNOVA0
		case 0x092: if(regs.flaga.ov0 == 1) regs.pc = jps; return;  //JOVA0
		case 0x094: if(regs.flagb.ov0 == 0) regs.pc = jps; return;  //JNOVB0
		case 0x096: if(regs.flagb.ov0 == 1) regs.pc = jps; return;  //JOVB0

		case 0x098: if(regs.flaga.ov1 == 0) regs.pc = jps; return;  //JNOVA1
		case 0x09a: if(regs.flaga.ov1 == 1) regs.pc = jps; return;  //JOVA1
		case 0x09c: if(regs.flagb.ov1 == 0) regs.pc = jps; return;  //JNOVB1
		case 0x09e: if(regs.flagb.ov1 == 1) regs.pc = jps; return;  //JOVB1

		case 0x0a0: if(regs.flaga.s0 == 0) regs.pc = jps; return;  //JNSA0
		case 0x0a2: if(regs.flaga.s0 == 1) regs.pc = jps; return;  //JSA0
		case 0x0a4: if(regs.flagb.s0 == 0) regs.pc = jps; return;  //JNSB0
		case 0x0a6: if(regs.flagb.s0 == 1) regs.pc = jps; return;  //JSB0

		case 0x0a8: if(regs.flaga.s1 == 0) regs.pc = jps; return;  //JNSA1
		case 0x0aa: if(regs.flaga.s1 == 1) regs.pc = jps; return;  //JSA1
		case 0x0ac: if(regs.flagb.s1 == 0) regs.pc = jps; return;  //JNSB1
		case 0x0ae: if(regs.flagb.s1 == 1) regs.pc = jps; return;  //JSB1

		case 0x0b0: if((regs.dp & 0x0f) == 0x00) regs.pc = jps; return;  //JDPL0
		case 0x0b1: if((regs.dp & 0x0f) != 0x00) regs.pc = jps; return;  //JDPLN0
		case 0x0b2: if((regs.dp & 0x0f) == 0x0f) regs.pc = jps; return;  //JDPLF
		case 0x0b3: if((regs.dp & 0x0f) != 0x0f) regs.pc = jps; return;  //JDPLNF

		case 0x0bc: if(regs.sr.rqm == 0) regs.pc = jps; return;  //JNRQM
		case 0x0be: if(regs.sr.rqm == 1) regs.pc = jps; return;  //JRQM

		case 0x100: regs.pc = 0x0000 | jpl; return;  //LJMP
		case 0x101: regs.pc = 0x2000 | jpl; return;  //HJMP

		case 0x140: regs.stack[regs.sp++] = regs.pc; regs.pc = 0x0000 | jpl; regs.sp &= 0xf; return;  //LCALL
		case 0x141: regs.stack[regs.sp++] = regs.pc; regs.pc = 0x2000 | jpl; regs.sp &= 0xf; return;  //HCALL
	}
}

void necdsp_device::exec_ld(UINT32 opcode) {
	UINT16 id = opcode >> 6;  //immediate data
	UINT8 dst = (opcode >> 0) & 0xf;  //destination

	regs.idb = id;

	switch(dst) {
	case  0: break;
	case  1: regs.a = id; break;
	case  2: regs.b = id; break;
	case  3: regs.tr = id; break;
	case  4: regs.dp = id; break;
	case  5: regs.rp = id; break;
	case  6: regs.dr = id; regs.sr.rqm = 1; break;
	case  7: regs.sr = (regs.sr & 0x907c) | (id & ~0x907c);
				m_out_p0_cb(regs.sr&0x1);
				m_out_p1_cb((regs.sr&0x2)>>1);
				break;
	case  8: regs.so = id; break;  //LSB
	case  9: regs.so = id; break;  //MSB
	case 10: regs.k = id; break;
	case 11: regs.k = id; regs.l = m_data->read_word(regs.rp<<1); break;
	case 12: regs.l = id; regs.k = dataRAM[regs.dp | 0x40]; break;
	case 13: regs.l = id; break;
	case 14: regs.trb = id; break;
	case 15: dataRAM[regs.dp] = id; break;
	}
}

UINT8 necdsp_device::snesdsp_read(bool mode) {
	if (!mode)
	{
		return regs.sr >> 8;
	}

	if (regs.sr.drc == 0)
	{
		//16-bit
		if(regs.sr.drs == 0)
		{
			regs.sr.drs = 1;
			return regs.dr >> 0;
		}
		else
		{
			regs.sr.rqm = 0;
			regs.sr.drs = 0;
			return regs.dr >> 8;
		}
	}
	else
	{
		//8-bit
		regs.sr.rqm = 0;
		return regs.dr >> 0;
	}
}

void necdsp_device::snesdsp_write(bool mode, UINT8 data) {
	if (!mode) return;

	if (regs.sr.drc == 0)
	{
		//16-bit
		if (regs.sr.drs == 0)
		{
			regs.sr.drs = 1;
			regs.dr = (regs.dr & 0xff00) | (data << 0);
		}
		else
		{
			regs.sr.rqm = 0;
			regs.sr.drs = 0;
			regs.dr = (data << 8) | (regs.dr & 0x00ff);
		}
	}
	else
	{
		//8-bit
		regs.sr.rqm = 0;
		regs.dr = (regs.dr & 0xff00) | (data << 0);
	}
}
