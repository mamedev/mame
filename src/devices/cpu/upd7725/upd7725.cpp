// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    upd7725.c

    Core implementation for the portable NEC uPD7725/uPD96050 emulator

    Original by byuu in the public domain.
    MAME conversion by R. Belmont

****************************************************************************/

#include "emu.h"
#include "upd7725.h"
#include "dasm7725.h"


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UPD7725,  upd7725_device,  "upd7725",  "NEC uPD7725")
DEFINE_DEVICE_TYPE(UPD96050, upd96050_device, "upd96050", "NEC uPD96050")

necdsp_device::necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t abits, uint32_t dbits, uint32_t drambits) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, 32, abits, -2), // data bus width, address bus width, -2 means DWORD-addressable
	m_data_config("data", ENDIANNESS_BIG, 16, dbits, -1), // -1 for WORD-addressable
	m_dataram_config("dataram", ENDIANNESS_BIG, 16, drambits, -1, address_map_constructor(FUNC(necdsp_device::dataram_map), this)), // -1 for WORD-addressable
	m_drammask((1<<drambits)-1),
	m_icount(0),
	m_irq(0),
	m_irq_firing(0),
	m_in_int_cb(*this, 0),
	//m_in_si_cb(*this, 0),
	//m_in_sck_cb(*this, 0), // TODO: proper SO/SORQ/SCK support
	//m_in_sien_cb(*this, 0),
	//m_in_soen_cb(*this, 0), // TODO: proper SO/SORQ/SCK support
	//m_in_dack_cb(*this, 0), // TODO: add D7725SR_DMA support
	m_out_p0_cb(*this),
	m_out_p1_cb(*this)
	//m_out_so_cb(*this), // TODO: proper SO/SORQ/SCK support
	//m_out_sorq_cb(*this), // TODO: proper SO/SORQ/SCK support
	//m_out_drq_cb(*this)  // TODO: add D7725SR_DMA support
{
}

/*upd7720_device::upd7720_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: necdsp_device(mconfig, UPD7720, tag, owner, clock, 9, 10, 7)
{
}*/

upd7725_device::upd7725_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: necdsp_device(mconfig, UPD7725, tag, owner, clock, 11, 11, 8)
{
}

upd96050_device::upd96050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: necdsp_device(mconfig, UPD96050, tag, owner, clock, 14, 12, 11)
{
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void necdsp_device::device_start()
{
	// get our address spaces
	space(AS_PROGRAM).specific(m_program);
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_dataram);

	// register our state for the debugger
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(D7725_PC, "PC", m_pc);
	state_add(D7725_RP, "RP", m_rp);
	state_add(D7725_DP, "DP", m_dp);
	state_add(D7725_SP, "SP", m_sp);
	state_add(D7725_K, "K", m_k);
	state_add(D7725_L, "L", m_l);
	state_add(D7725_M, "M", m_m);
	state_add(D7725_N, "N", m_n);
	state_add(D7725_A, "A", m_a);
	state_add(D7725_B, "B", m_b);
	state_add(D7725_TR, "TR", m_tr);
	state_add(D7725_TRB, "TRB", m_trb);
	state_add(D7725_SR, "SR", m_sr);
	state_add(D7725_DR, "DR", m_dr);
	state_add(D7725_SI, "SI", m_si);
	state_add(D7725_SO, "SO", m_so);
	state_add(D7725_IDB, "IDB", m_idb);

	// save state registrations
	save_item(NAME(m_pc));
	save_item(NAME(m_rp));
	save_item(NAME(m_dp));
	save_item(NAME(m_sp));
	save_item(NAME(m_k));
	save_item(NAME(m_l));
	save_item(NAME(m_m));
	save_item(NAME(m_n));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_flaga.s1));
	save_item(NAME(m_flaga.s0));
	save_item(NAME(m_flaga.c));
	save_item(NAME(m_flaga.z));
	save_item(NAME(m_flaga.ov1));
	save_item(NAME(m_flaga.ov0));
	save_item(NAME(m_flagb.s1));
	save_item(NAME(m_flagb.s0));
	save_item(NAME(m_flagb.c));
	save_item(NAME(m_flagb.z));
	save_item(NAME(m_flagb.ov1));
	save_item(NAME(m_flagb.ov0));
	save_item(NAME(m_tr));
	save_item(NAME(m_trb));
	save_item(NAME(m_sr));
	save_item(NAME(m_dr));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
	save_item(NAME(m_idb));
	save_item(NAME(m_siack));
	save_item(NAME(m_soack));
	save_item(NAME(m_stack));
	save_item(NAME(m_drammask));
	save_item(NAME(m_irq));
	save_item(NAME(m_irq_firing));

	set_icountptr(m_icount);

	// reset registers not reset by the /RESET line (according to section 3.6.1 on the upd7725 advanced production datasheet)
	m_irq = 0; // not a register, but the current irq pin state
	m_rp = 0x0000;
	m_dp = 0x0000;
	m_sp = 0x0;
	m_k = 0x0000;
	m_l = 0x0000;
	m_m = 0x0000;
	m_n = 0x0000;
	m_a = 0x0000;
	m_b = 0x0000;
	m_tr = 0x0000;
	m_trb = 0x0000;
	m_dr = 0x0000;
	m_si = 0x0000;
	m_so = 0x0000;
	m_idb = 0x0000;
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void necdsp_device::device_reset()
{
	// according to 3.6.1 on the upd7725 advanced production datasheet, /RESET resets the following only:
	m_pc = 0x0000;
	m_sr = 0x0000;
	m_out_p0_cb((m_sr & D7725SR_P0) ? ASSERT_LINE : CLEAR_LINE);
	m_out_p1_cb((m_sr & D7725SR_P1) ? ASSERT_LINE : CLEAR_LINE);
	// TODO: drq callback, once added, should be forced to the inactive state here
	// TODO: the sorq pin state is also reset to 'low' state
	m_flaga = 0x00;
	m_flagb = 0x00;
	m_siack = 0;
	m_soack = 0;

	// the irq state (if mid-irq) is assumed to also be reset, since the pulse width of reset must be more than 4 opcode clocks
	m_irq_firing = 0;
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector necdsp_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_dataram_config)
	};
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
		case D7725_FLAGA:
			str = string_format("%s %s %c%c %s %s %s %s",
							m_flaga.s1 ? "S1" : "s1",
							m_flaga.s0 ? "S0" : "s0",
							m_flaga.c ? 'C' : 'c',
							m_flaga.z ? 'Z' : 'z',
							m_flaga.ov1 ? "OV1" : "ov1",
							m_flaga.ov0 ? "OV0" : "ov0");
			break;

		case D7725_FLAGB:
			str = string_format("%s %s %c%c %s %s %s %s",
							m_flagb.s1 ? "S1" : "s1",
							m_flagb.s0 ? "S0" : "s0",
							m_flagb.c ? 'C' : 'c',
							m_flagb.z ? 'Z' : 'z',
							m_flagb.ov1 ? "OV1" : "ov1",
							m_flagb.ov0 ? "OV0" : "ov0");
			break;
	}
}

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t necdsp_device::execute_min_cycles() const noexcept
{
	return 4;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t necdsp_device::execute_max_cycles() const noexcept
{
	return 4;
}


//-------------------------------------------------
//  execute_set_input -
//-------------------------------------------------

void necdsp_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case NECDSP_INPUT_LINE_INT:
			if ((!m_irq && (CLEAR_LINE != state)) && (m_sr & D7725SR_EI)) // detect rising edge AND if EI == 1;
			{
				m_irq_firing = 1;
				m_sr &= ~D7725SR_EI;
			}
			m_irq = (ASSERT_LINE == state); // set old state to current state
			break;
		// add more when needed
	}
}

//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> necdsp_device::create_disassembler()
{
	return std::make_unique<necdsp_disassembler>();
}

//-------------------------------------------------
//  set up internal memory map for the
//  data ram
//-------------------------------------------------

void necdsp_device::dataram_map(address_map &map)
{
	map(0, m_drammask).ram().share("dataram");
}


void necdsp_device::execute_run()
{
	uint32_t opcode;

	do
	{
		// call debugger hook if necessary
		if (debugger_enabled())
		{
			debugger_instruction_hook(m_pc);
		}

		if (m_irq_firing == 0) // normal opcode
		{
			opcode = m_cache.read_dword(m_pc) >> 8;
			m_pc++;
		}
		else if (m_irq_firing == 1) // if we're in an interrupt cycle, execute a op 'nop' first...
		{
			// NOP: OP  PSEL ALU  ASL DPL DPHM   RPDCR SRC  DST
			//      00  00   0000 0   00  000(0) 0     0000 0000
			opcode = 0x000000;
			m_irq_firing = 2;
		}
		else // m_irq_firing == 2 // ...then a call to 100
		{
			// LCALL: JP BRCH      NA          BNK(all 0s on 7725)
			//        10 101000000 00100000000 00
			opcode = 0xA80400;
			m_irq_firing = 0;
		}

		switch(opcode >> 22)
		{
			case 0: exec_op(opcode); break;
			case 1: exec_rt(opcode); break;
			case 2: exec_jp(opcode); break;
			case 3: exec_ld(opcode); break;
		}

		int32_t result = (int32_t)m_k * m_l;  //sign + 30-bit result
		m_m = result >> 15;  //store sign + top 15-bits
		m_n = result <<  1;  //store low 15-bits + zero

		m_icount--;

	} while (m_icount > 0);
}

void necdsp_device::exec_op(uint32_t opcode)
{
	uint8_t pselect = (opcode >> 20)&0x3;  //P select
	uint8_t alu     = (opcode >> 16)&0xf;  //ALU operation mode
	uint8_t asl     = (opcode >> 15)&0x1;  //accumulator select
	uint8_t dpl     = (opcode >> 13)&0x3;  //DP low modify
	uint8_t dphm    = (opcode >>  9)&0xf;  //DP high XOR modify
	uint8_t rpdcr   = (opcode >>  8)&0x1;  //RP decrement
	uint8_t src     = (opcode >>  4)&0xf;  //move source
	uint8_t dst     = (opcode >>  0)&0xf;  //move destination

	switch(src)
	{
		case  0: m_idb = m_trb; break;
		case  1: m_idb = m_a; break;
		case  2: m_idb = m_b; break;
		case  3: m_idb = m_tr; break;
		case  4: m_idb = m_dp; break;
		case  5: m_idb = m_rp; break;
		case  6: m_idb = m_data.read_word(m_rp); break;
		case  7: m_idb = 0x8000 - m_flaga.s1; break;  //SGN
		case  8: m_idb = m_dr; m_sr |= D7725SR_RQM; break;
		case  9: m_idb = m_dr; break;
		case 10: m_idb = m_sr; break;
		case 11: m_idb = m_si; break;  //MSB = first bit in from serial, 'natural' SI register order
		case 12: m_idb = bitswap<16>(m_si, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); break;  //LSB = first bit in from serial, 'reversed' SI register order
		case 13: m_idb = m_k; break;
		case 14: m_idb = m_l; break;
		case 15: m_idb = m_dataram.read_word(m_dp & m_drammask); break;
	}

	if(alu) // != 0 NOP
	{
		uint16_t p=0, q=0, r=0;
		Flag flag;
		bool c = false;

		flag.c = 0;
		flag.s1 = 0;
		flag.ov0 = 0;
		flag.ov1 = 0;

		switch(pselect)
		{
			case 0: p = m_dataram.read_word(m_dp & m_drammask); break;
			case 1: p = m_idb; break;
			case 2: p = m_m; break;
			case 3: p = m_n; break;
		}

		switch(asl)
		{
			case 0: q = m_a; flag = m_flaga; c = m_flagb.c; break;
			case 1: q = m_b; flag = m_flagb; c = m_flaga.c; break;
		}

		switch(alu)
		{
			case 0x1: r = q | p; break;                    // 1 OR
			case 0x2: r = q & p; break;                    // 2 AND
			case 0x3: r = q ^ p; break;                    // 3 XOR
			case 0x4: r = q - p; break;                    // 4 SUB
			case 0x5: r = q + p; break;                    // 5 ADD
			case 0x6: r = q - p - c; break;                // 6 SBB
			case 0x7: r = q + p + c; break;                // 7 ADC
			case 0x8: r = q - 1; p = 1; break;             // 8 DEC
			case 0x9: r = q + 1; p = 1; break;             // 9 INC
			case 0xa: r = ~q; break;                       // a CMP
			case 0xb: r = (q >> 1) | (q & 0x8000); break;  // b SHR1 (ASR)
			case 0xc: r = (q << 1) | (c ? 1 : 0); break;   // c SHL1 (ROL)
			case 0xd: r = (q << 2) | 3; break;             // d SHL2
			case 0xe: r = (q << 4) | 15; break;            // e SHL4
			case 0xf: r = swapendian_int16(q); break;      // f XCHG
		}

		flag.s0 = (r & 0x8000);
		flag.z = (r == 0);
		if(!flag.ov1) flag.s1 = flag.s0;

		switch(alu)
		{
			case 0x1: case 0x2: case 0x3: case 0xa: case 0xd: case 0xe: case 0xf:
				flag.c = 0;
				flag.ov0 = flag.ov1 = 0; // OV0 and OV1 are cleared by any non-add/sub/nop operation
				break;
			case 0x4: case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
				if(alu & 1)
				{
					//addition
					flag.ov0 = (q ^ r) & ~(q ^ p) & 0x8000;
					flag.c = (r < q);
				}
				else
				{
					//subtraction
					flag.ov0 = (q ^ r) &  (q ^ p) & 0x8000;
					flag.c = (r > q);
				}
				flag.ov1 = (flag.ov0 & flag.ov1) ? (flag.s1 == flag.s0) : (flag.ov0 | flag.ov1);
				break;
			case 0xb:
				flag.c = q & 1;
				flag.ov0 = flag.ov1 = 0; // OV0 and OV1 are cleared by any non-add/sub/nop operation
				break;
			case 0xc:
				flag.c = q >> 15;
				flag.ov0 = flag.ov1 = 0; // OV0 and OV1 are cleared by any non-add/sub/nop operation
				break;
		}

		switch(asl)
		{
			case 0: m_a = r; m_flaga = flag; break;
			case 1: m_b = r; m_flagb = flag; break;
		}
	}

	exec_ld((m_idb << 6) + dst);

	if (dst != 4)
	{
		switch(dpl)
		{
			case 1: m_dp = (m_dp & 0xf0) + ((m_dp + 1) & 0x0f); break;  //DPINC
			case 2: m_dp = (m_dp & 0xf0) + ((m_dp - 1) & 0x0f); break;  //DPDEC
			case 3: m_dp = (m_dp & 0xf0); break;  //DPCLR
		}

		m_dp ^= dphm << 4;
	}

	if(rpdcr && (dst != 5)) m_rp--;
}

void necdsp_device::exec_rt(uint32_t opcode)
{
	exec_op(opcode);
	m_pc = m_stack[--m_sp];
	m_sp &= 0xf;
}

void necdsp_device::exec_jp(uint32_t opcode)
{
	uint16_t brch = (opcode >> 13) & 0x1ff;  //branch
	uint16_t na  =  (opcode >>  2) & 0x7ff;  //next address
	uint16_t bank = (opcode >>  0) & 0x3;  //bank address

	uint16_t jps = (m_pc & 0x2000) | (bank << 11) | (na << 0);
	uint16_t jpl = (bank << 11) | (na << 0);

	switch(brch)
	{
		case 0x000: m_pc = m_so; return;  //JMPSO

		case 0x080: if(m_flaga.c == 0) m_pc = jps; return;  //JNCA
		case 0x082: if(m_flaga.c == 1) m_pc = jps; return;  //JCA
		case 0x084: if(m_flagb.c == 0) m_pc = jps; return;  //JNCB
		case 0x086: if(m_flagb.c == 1) m_pc = jps; return;  //JCB

		case 0x088: if(m_flaga.z == 0) m_pc = jps; return;  //JNZA
		case 0x08a: if(m_flaga.z == 1) m_pc = jps; return;  //JZA
		case 0x08c: if(m_flagb.z == 0) m_pc = jps; return;  //JNZB
		case 0x08e: if(m_flagb.z == 1) m_pc = jps; return;  //JZB

		case 0x090: if(m_flaga.ov0 == 0) m_pc = jps; return;  //JNOVA0
		case 0x092: if(m_flaga.ov0 == 1) m_pc = jps; return;  //JOVA0
		case 0x094: if(m_flagb.ov0 == 0) m_pc = jps; return;  //JNOVB0
		case 0x096: if(m_flagb.ov0 == 1) m_pc = jps; return;  //JOVB0

		case 0x098: if(m_flaga.ov1 == 0) m_pc = jps; return;  //JNOVA1
		case 0x09a: if(m_flaga.ov1 == 1) m_pc = jps; return;  //JOVA1
		case 0x09c: if(m_flagb.ov1 == 0) m_pc = jps; return;  //JNOVB1
		case 0x09e: if(m_flagb.ov1 == 1) m_pc = jps; return;  //JOVB1

		case 0x0a0: if(m_flaga.s0 == 0) m_pc = jps; return;  //JNSA0
		case 0x0a2: if(m_flaga.s0 == 1) m_pc = jps; return;  //JSA0
		case 0x0a4: if(m_flagb.s0 == 0) m_pc = jps; return;  //JNSB0
		case 0x0a6: if(m_flagb.s0 == 1) m_pc = jps; return;  //JSB0

		case 0x0a8: if(m_flaga.s1 == 0) m_pc = jps; return;  //JNSA1
		case 0x0aa: if(m_flaga.s1 == 1) m_pc = jps; return;  //JSA1
		case 0x0ac: if(m_flagb.s1 == 0) m_pc = jps; return;  //JNSB1
		case 0x0ae: if(m_flagb.s1 == 1) m_pc = jps; return;  //JSB1

		case 0x0b0: if((m_dp & 0x0f) == 0x00) m_pc = jps; return;  //JDPL0
		case 0x0b1: if((m_dp & 0x0f) != 0x00) m_pc = jps; return;  //JDPLN0
		case 0x0b2: if((m_dp & 0x0f) == 0x0f) m_pc = jps; return;  //JDPLF
		case 0x0b3: if((m_dp & 0x0f) != 0x0f) m_pc = jps; return;  //JDPLNF

		case 0x0b4: if(m_siack == 0) m_pc = jps; return;  //JNSIAK
		case 0x0b6: if(m_siack == 1) m_pc = jps; return;  //JSIAK
		case 0x0b8: if(m_soack == 0) m_pc = jps; return;  //JNSOAK
		case 0x0ba: if(m_soack == 1) m_pc = jps; return;  //JSOAK

		case 0x0bc: if(~m_sr & D7725SR_RQM) m_pc = jps; return;  //JNRQM
		case 0x0be: if( m_sr & D7725SR_RQM) m_pc = jps; return;  //JRQM

		case 0x100: m_pc = 0x0000 | jpl; return;  //LJMP
		case 0x101: m_pc = 0x2000 | jpl; return;  //HJMP

		case 0x140: m_stack[m_sp++] = m_pc; m_pc = 0x0000 | jpl; m_sp &= 0xf; return;  //LCALL
		case 0x141: m_stack[m_sp++] = m_pc; m_pc = 0x2000 | jpl; m_sp &= 0xf; return;  //HCALL
	}
}

void necdsp_device::exec_ld(uint32_t opcode)
{
	uint16_t id = opcode >> 6;  //immediate data
	uint8_t dst = (opcode >> 0) & 0xf;  //destination

	m_idb = id;

	switch(dst)
	{
		case  0: break;
		case  1: m_a = id; break;
		case  2: m_b = id; break;
		case  3: m_tr = id; break;
		case  4: m_dp = id; break;
		case  5: m_rp = id; break;
		case  6: m_dr = id; m_sr |= D7725SR_RQM; break;
		case  7: m_sr = (m_sr & 0x907c) | (id & ~0x907c);
					m_out_p0_cb((m_sr & D7725SR_P0) ? ASSERT_LINE : CLEAR_LINE);
					m_out_p1_cb((m_sr & D7725SR_P1) ? ASSERT_LINE : CLEAR_LINE);
					break;
		case  8: m_so = bitswap<16>(id, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); m_soack = 1; break;  //LSB first output, output tapped at bit 15 shifting left
		case  9: m_so = id; m_soack = 1; break;  //MSB first output, output tapped at bit 15 shifting left
		case 10: m_k = id; break;
		case 11: m_k = id; m_l = m_data.read_word(m_rp); break;
		case 12: m_l = id; m_k = m_dataram.read_word((m_dp & m_drammask) | 0x40); break;
		case 13: m_l = id; break;
		case 14: m_trb = id; break;
		case 15: m_dataram.write_word(m_dp & m_drammask, id); break;
	}
}

uint8_t necdsp_device::status_r()
{
	return m_sr >> 8;
}

uint8_t necdsp_device::data_r()
{
	if (~m_sr & D7725SR_DRC)
	{
		//16-bit
		if(~m_sr & D7725SR_DRS)
		{
			if (!machine().side_effects_disabled())
				m_sr |= D7725SR_DRS;
			return m_dr >> 0;
		}
		else
		{
			if (!machine().side_effects_disabled())
			{
				m_sr &= ~D7725SR_RQM;
				m_sr &= ~D7725SR_DRS;
			}
			return m_dr >> 8;
		}
	}
	else
	{
		//8-bit
		if (!machine().side_effects_disabled())
			m_sr &= ~D7725SR_RQM;
		return m_dr >> 0;
	}
}

void necdsp_device::data_w(uint8_t data)
{
	if (~m_sr & D7725SR_DRC)
	{
		//16-bit
		if (~m_sr & D7725SR_DRS)
		{
			m_sr |= D7725SR_DRS;
			m_dr = (m_dr & 0xff00) | (data << 0);
		}
		else
		{
			m_sr &= ~D7725SR_RQM;
			m_sr &= ~D7725SR_DRS;
			m_dr = (data << 8) | (m_dr & 0x00ff);
		}
	}
	else
	{
		//8-bit
		m_sr &= ~D7725SR_RQM;
		m_dr = (m_dr & 0xff00) | (data << 0);
	}
}
