/***************************************************************************

    upd7725.h

    Core implementation for the portable NEC uPD7725 emulator
 
    Original by byuu in the public domain.
    MAME conversion by R. Belmont
 
****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "upd7725.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type UPD7725 = upd7725_device_config::static_alloc_device_config;

//**************************************************************************
//  UPD7725 DEVICE CONFIG
//**************************************************************************

//-------------------------------------------------
//  upd7725_device_config - constructor
//-------------------------------------------------

upd7725_device_config::upd7725_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: cpu_device_config(mconfig, static_alloc_device_config, "upd7725", tag, owner, clock),
	  m_program_config("program", ENDIANNESS_BIG, 32, 13, -2),	// data bus width, address bus width, -2 means DWORD-addressable
	  m_data_config("data", ENDIANNESS_BIG, 16, 11, -1)	// -1 for WORD-addressable
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *upd7725_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(upd7725_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *upd7725_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, upd7725_device(machine, *this));
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 upd7725_device_config::execute_min_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 upd7725_device_config::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 upd7725_device_config::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *upd7725_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : &m_data_config;
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 upd7725_device_config::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 upd7725_device_config::disasm_max_opcode_bytes() const
{
	return 4;
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

upd7725_device::upd7725_device(running_machine &_machine, const upd7725_device_config &config)
	: cpu_device(_machine, config),
	m_program(NULL),
	m_data(NULL),
	m_direct(NULL)
{
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void upd7725_device::device_start()
{
	// get our address spaces
	m_program = space(AS_PROGRAM);
	m_data = space(AS_DATA);
	m_direct = &m_program->direct();

	// register our state for the debugger
	astring tempstr;
	state_add(STATE_GENPC, "GENPC", regs.pc).noshow();
	state_add(UPD7725_PC, "PC", regs.pc);
	state_add(UPD7725_RP, "RP", regs.rp);
	state_add(UPD7725_DP, "DP", regs.dp);
	state_add(UPD7725_K, "K", regs.k);
	state_add(UPD7725_L, "L", regs.l);
	state_add(UPD7725_M, "M", regs.m);
	state_add(UPD7725_N, "N", regs.n);
	state_add(UPD7725_A, "A", regs.a);
	state_add(UPD7725_B, "B", regs.b);
	state_add(UPD7725_SR, "SR", regs.sr);
	state_add(UPD7725_DR, "DR", regs.dr);

	m_icountptr = &m_icount;
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void upd7725_device::device_reset()
{
	for (unsigned i = 0; i <  256; i++) 
	{
		dataRAM[i] = 0x0000;
	}

	regs.pc = 0x000;
	regs.stack[0] = 0x000;
	regs.stack[1] = 0x000;
	regs.stack[2] = 0x000;
	regs.stack[3] = 0x000;
	regs.flaga = 0x00;
	regs.flagb = 0x00;
	regs.sr = 0x0000;
	regs.rp = 0x3ff;
	regs.siack = 0;
	regs.soack = 0;
}

//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void upd7725_device::state_import(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void upd7725_device::state_export(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void upd7725_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case UPD7725_FLAGA:
			string.printf("%s %s %c%c %s %s",
						  regs.flaga.s1 ? "S1" : "s1",
						  regs.flaga.s0 ? "S0" : "s0",
						  regs.flaga.c ? "C" : "c",
						  regs.flaga.z ? "Z" : "z",
						  regs.flaga.ov1 ? "OV1" : "ov1",
						  regs.flaga.ov0 ? "OV0" : "ov0");
			break;

		case UPD7725_FLAGB:
			string.printf("%s %s %c%c %s %s",
						  regs.flagb.s1 ? "S1" : "s1",
						  regs.flagb.s0 ? "S0" : "s0",
						  regs.flagb.c ? "C" : "c",
						  regs.flagb.z ? "Z" : "z",
						  regs.flagb.ov1 ? "OV1" : "ov1",
						  regs.flagb.ov0 ? "OV0" : "ov0");
			break;
	}
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t upd7725_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd7725 );
	return CPU_DISASSEMBLE_NAME(upd7725)(NULL, buffer, pc, oprom, opram, 0);
}

void upd7725_device::execute_run() 
{
    UINT32 opcode;

	do
	{
		// call debugger hook if necessary
		if (device_t::m_machine.debug_flags & DEBUG_FLAG_ENABLED)
		{
			debugger_instruction_hook(this, regs.pc);
		}

		opcode = m_direct->read_decrypted_dword(regs.pc<<2)>>8;
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

void upd7725_device::exec_op(UINT32 opcode) {
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
    case  7: regs.idb = 0x8000 - regs.flaga.s1; break;
    case  8: regs.idb = regs.dr; regs.sr.rqm = 1; break;
    case  9: regs.idb = regs.dr; break;
    case 10: regs.idb = regs.sr; break;
  //case 11: regs.idb = regs.sim; break;
  //case 12: regs.idb = regs.sil; break;
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
      case 12: r = (q << 1) | c; break;             //SHL1 (ROL)
      case 13: r = (q << 2) | 3; break;             //SHL2
      case 14: r = (q << 4) | 15; break;            //SHL4
      case 15: r = (q << 8) | (q >> 8); break;      //XCHG
    }

    flag.s0 = (r & 0x8000);
    flag.z = (r == 0);

    switch(alu) {
      case  1: case  2: case  3: case 10: case 13: case 14: case 15: {
        flag.c = 0;
        flag.ov0 = 0;
        flag.ov1 = 0;
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
        if(flag.ov0) {
          flag.s1 = flag.ov1 ^ !(r & 0x8000);
          flag.ov1 = !flag.ov1;
        }
        break;
      }
      case 11: {
        flag.c = q & 1;
        flag.ov0 = 0;
        flag.ov1 = 0;
        break;
      }
      case 12: {
        flag.c = q >> 15;
        flag.ov0 = 0;
        flag.ov1 = 0;
        break;
      }
    }

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

void upd7725_device::exec_rt(UINT32 opcode) {
  exec_op(opcode);
  stack_pull();
}

void upd7725_device::exec_jp(UINT32 opcode) {
  UINT16 brch = (opcode >> 13) & 0x1ff;  //branch
  UINT16 na  = (opcode >>  2) & 0x7ff;  //next address

  bool r = false;

  switch(brch) {
    case 0x080: r = (regs.flaga.c == 0);      break;  //JNCA
    case 0x082: r = (regs.flaga.c == 1);      break;  //JCA
    case 0x084: r = (regs.flagb.c == 0);      break;  //JNCB
    case 0x086: r = (regs.flagb.c == 1);      break;  //JCB
    case 0x088: r = (regs.flaga.z == 0);      break;  //JNZA
    case 0x08a: r = (regs.flaga.z == 1);      break;  //JZA
    case 0x08c: r = (regs.flagb.z == 0);      break;  //JNZB
    case 0x08e: r = (regs.flagb.z == 1);      break;  //JZB
    case 0x090: r = (regs.flaga.ov0 == 0);    break;  //JNOVA0
    case 0x092: r = (regs.flaga.ov0 == 1);    break;  //JOVA0
    case 0x094: r = (regs.flagb.ov0 == 0);    break;  //JNOVB0
    case 0x096: r = (regs.flagb.ov0 == 1);    break;  //JOVB0
    case 0x098: r = (regs.flaga.ov1 == 0);    break;  //JNOVA1
    case 0x09a: r = (regs.flaga.ov1 == 1);    break;  //JOVA1
    case 0x09c: r = (regs.flagb.ov1 == 0);    break;  //JNOVB1
    case 0x09e: r = (regs.flagb.ov1 == 1);    break;  //JOVB1
    case 0x0a0: r = (regs.flaga.s0 == 0);     break;  //JNSA0
    case 0x0a2: r = (regs.flaga.s0 == 1);     break;  //JSA0
    case 0x0a4: r = (regs.flagb.s0 == 0);     break;  //JNSB0
    case 0x0a6: r = (regs.flagb.s0 == 1);     break;  //JSB0
    case 0x0a8: r = (regs.flaga.s1 == 0);     break;  //JNSA1
    case 0x0aa: r = (regs.flaga.s1 == 1);     break;  //JSA1
    case 0x0ac: r = (regs.flagb.s1 == 0);     break;  //JNSB1
    case 0x0ae: r = (regs.flagb.s1 == 1);     break;  //JSB1

    case 0x0b0: r = (regs.dp & 0x0f) == 0x00; break;  //JDPL0
    case 0x0b1: r = (regs.dp & 0x0f) != 0x00; break;  //JDPLN0
    case 0x0b2: r = (regs.dp & 0x0f) == 0x0f; break;  //JDPLF
    case 0x0b3: r = (regs.dp & 0x0f) != 0x0f; break;  //JDPLNF

    case 0x0b4: r = (regs.siack == 0);        break;  //JNSIAK
    case 0x0b6: r = (regs.siack == 1);        break;  //JSIAK

    case 0x0b8: r = (regs.soack == 0);        break;  //JNSOAK
    case 0x0ba: r = (regs.soack == 1);        break;  //JSOAK

    case 0x0bc: r = (regs.sr.rqm == 0);       break;  //JNRQM
    case 0x0be: r = (regs.sr.rqm == 1);       break;  //JRQM

    case 0x100: r = true;                     break;  //JMP
    case 0x140: r = true; stack_push();       break;  //CALL
  }

  if(r) regs.pc = na;
}

void upd7725_device::exec_ld(UINT32 opcode) {
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
    case  7: regs.sr = (regs.sr & 0x907c) | (id & ~0x907c); break;
  //case  8: regs.sol = id; break;
  //case  9: regs.som = id; break;
    case 10: regs.k = id; break;
    case 11: regs.k = id; regs.l = m_data->read_word(regs.rp<<1); break;
    case 12: regs.l = id; regs.k = dataRAM[regs.dp | 0x40]; break;
    case 13: regs.l = id; break;
    case 14: regs.trb = id; break;
    case 15: dataRAM[regs.dp] = id; break;
  }
}

void upd7725_device::stack_push() {
  regs.stack[3] = regs.stack[2];
  regs.stack[2] = regs.stack[1];
  regs.stack[1] = regs.stack[0];
  regs.stack[0] = regs.pc & 0x7ff;
}

void upd7725_device::stack_pull() {
  regs.pc = regs.stack[0] & 0x7ff;
  regs.stack[0] = regs.stack[1];
  regs.stack[1] = regs.stack[2];
  regs.stack[2] = regs.stack[3];
  regs.stack[3] = 0x000;
}

UINT8 upd7725_device::snesdsp_read(bool mode) {
	
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

void upd7725_device::snesdsp_write(bool mode, UINT8 data) {
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
