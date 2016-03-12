// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Texas Instruments TMS320C82 DSP Emulator

    Written by Ville Linde

*/

#include "emu.h"
#include "debugger.h"
#include "tms32082.h"

extern CPU_DISASSEMBLE(tms32082_mp);
extern CPU_DISASSEMBLE(tms32082_pp);

const device_type TMS32082_MP = &device_creator<tms32082_mp_device>;
const device_type TMS32082_PP = &device_creator<tms32082_pp_device>;




// Master Processor

// internal memory map
static ADDRESS_MAP_START(mp_internal_map, AS_PROGRAM, 32, tms32082_mp_device)
	AM_RANGE(0x00000000, 0x00000fff) AM_RAM AM_SHARE("pp0_data0")
	AM_RANGE(0x00001000, 0x00001fff) AM_RAM AM_SHARE("pp1_data0")
	AM_RANGE(0x00008000, 0x00008fff) AM_RAM AM_SHARE("pp0_data1")
	AM_RANGE(0x00009000, 0x00009fff) AM_RAM AM_SHARE("pp1_data1")
	AM_RANGE(0x01000000, 0x01000fff) AM_RAM AM_SHARE("pp0_param")
	AM_RANGE(0x01001000, 0x01001fff) AM_RAM AM_SHARE("pp1_param")
	AM_RANGE(0x01010000, 0x010107ff) AM_READWRITE(mp_param_r, mp_param_w)
ADDRESS_MAP_END

const UINT32 tms32082_mp_device::SHIFT_MASK[] =
{
	0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
	0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
	0xffffffff
};



tms32082_mp_device::tms32082_mp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS32082_MP, "TMS32082 MP", tag, owner, clock, "tms32082_mp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, ADDRESS_MAP_NAME(mp_internal_map))
{
}


offs_t tms32082_mp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return CPU_DISASSEMBLE_NAME(tms32082_mp)(this, buffer, pc, oprom, opram, options);
}



void tms32082_mp_device::set_command_callback(write32_delegate callback)
{
	m_cmd_callback = callback;
}


READ32_MEMBER(tms32082_mp_device::mp_param_r)
{
	//printf("mp_param_w: %08X, %08X\n", offset, mem_mask);
	return m_param_ram[offset];
}

WRITE32_MEMBER(tms32082_mp_device::mp_param_w)
{
	//printf("mp_param_w: %08X, %08X, %08X\n", offset, data, mem_mask);

	COMBINE_DATA(&m_param_ram[offset]);

	if (offset == 0x3f)
	{
		// initiate Transfer Controller operation
		// TODO: move TC functionality to separate device
		UINT32 address = data;

		UINT32 next_entry = m_program->read_dword(address + 0);
		UINT32 pt_options = m_program->read_dword(address + 4);
		UINT32 src_addr = m_program->read_dword(address + 8);
		UINT32 dst_addr = m_program->read_dword(address + 12);
		UINT32 src_b_count = m_program->read_word(address + 16);
		UINT32 src_a_count = m_program->read_word(address + 18);
		UINT32 dst_b_count = m_program->read_word(address + 20);
		UINT32 dst_a_count = m_program->read_word(address + 22);
		UINT32 src_c_count = m_program->read_dword(address + 24);
		UINT32 dst_c_count = m_program->read_dword(address + 28);
		UINT32 src_b_pitch = m_program->read_dword(address + 32);
		UINT32 dst_b_pitch = m_program->read_dword(address + 36);
		UINT32 src_c_pitch = m_program->read_dword(address + 40);
		UINT32 dst_c_pitch = m_program->read_dword(address + 44);

		printf("TC operation:\n");
		printf("   Next entry: %08X\n", next_entry);
		printf("   PT options: %08X\n", pt_options);
		printf("   SRC addr:   %08X\n", src_addr);
		printf("   DST addr:   %08X\n", dst_addr);
		printf("   SRC count A: %04X, B: %04X\n", src_a_count, src_b_count);
		printf("   DST count A: %04X, B: %04X\n", dst_a_count, dst_b_count);
		printf("   SRC count C: %08X\n", src_c_count);
		printf("   DST count C: %08X\n", dst_c_count);
		printf("   SRC B pitch: %08X\n", src_b_pitch);
		printf("   DST B pitch: %08X\n", dst_b_pitch);
		printf("   SRC C pitch: %08X\n", src_c_pitch);
		printf("   DST C pitch: %08X\n", dst_c_pitch);

		if (pt_options != 0x80000000)
			fatalerror("TC transfer, options = %08X\n", pt_options);

		for (int ic = 0; ic <= src_c_count; ic++)
		{
			UINT32 c_src_offset = ic * src_c_pitch;
			UINT32 c_dst_offset = ic * dst_c_pitch;

			for (int ib = 0; ib <= src_b_count; ib++)
			{
				UINT32 b_src_offset = ib * src_b_pitch;
				UINT32 b_dst_offset = ib * dst_b_pitch;

				for (int ia = 0; ia < src_a_count; ia++)
				{
					UINT32 src = src_addr + c_src_offset + b_src_offset + ia;
					UINT32 dst = dst_addr + c_dst_offset + b_dst_offset + ia;

					UINT32 data = m_program->read_byte(src);
					m_program->write_byte(dst, data);

					//printf("%08X: %02X -> %08X\n", src, data, dst);
				}
			}
		}
	}
}



void tms32082_mp_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_fetchpc));
	save_item(NAME(m_reg));
	save_item(NAME(m_acc));

	save_item(NAME(m_in0p));
	save_item(NAME(m_in1p));
	save_item(NAME(m_outp));
	save_item(NAME(m_ie));
	save_item(NAME(m_intpen));

	// Register state for debugger
	state_add(MP_PC, "pc", m_pc).formatstr("%08X");

	state_add(MP_R0, "r0", m_reg[0]).formatstr("%08X");
	state_add(MP_R1, "r1", m_reg[1]).formatstr("%08X");
	state_add(MP_R2, "r2", m_reg[2]).formatstr("%08X");
	state_add(MP_R3, "r3", m_reg[3]).formatstr("%08X");
	state_add(MP_R4, "r4", m_reg[4]).formatstr("%08X");
	state_add(MP_R5, "r5", m_reg[5]).formatstr("%08X");
	state_add(MP_R6, "r6", m_reg[6]).formatstr("%08X");
	state_add(MP_R7, "r7", m_reg[7]).formatstr("%08X");
	state_add(MP_R8, "r8", m_reg[8]).formatstr("%08X");
	state_add(MP_R9, "r9", m_reg[9]).formatstr("%08X");
	state_add(MP_R10, "r10", m_reg[10]).formatstr("%08X");
	state_add(MP_R11, "r11", m_reg[11]).formatstr("%08X");
	state_add(MP_R12, "r12", m_reg[12]).formatstr("%08X");
	state_add(MP_R13, "r13", m_reg[13]).formatstr("%08X");
	state_add(MP_R14, "r14", m_reg[14]).formatstr("%08X");
	state_add(MP_R15, "r15", m_reg[15]).formatstr("%08X");
	state_add(MP_R16, "r16", m_reg[16]).formatstr("%08X");
	state_add(MP_R17, "r17", m_reg[17]).formatstr("%08X");
	state_add(MP_R18, "r18", m_reg[18]).formatstr("%08X");
	state_add(MP_R19, "r19", m_reg[19]).formatstr("%08X");
	state_add(MP_R20, "r20", m_reg[20]).formatstr("%08X");
	state_add(MP_R21, "r21", m_reg[21]).formatstr("%08X");
	state_add(MP_R22, "r22", m_reg[22]).formatstr("%08X");
	state_add(MP_R23, "r23", m_reg[23]).formatstr("%08X");
	state_add(MP_R24, "r24", m_reg[24]).formatstr("%08X");
	state_add(MP_R25, "r25", m_reg[25]).formatstr("%08X");
	state_add(MP_R26, "r26", m_reg[26]).formatstr("%08X");
	state_add(MP_R27, "r27", m_reg[27]).formatstr("%08X");
	state_add(MP_R28, "r28", m_reg[28]).formatstr("%08X");
	state_add(MP_R29, "r29", m_reg[29]).formatstr("%08X");
	state_add(MP_R30, "r30", m_reg[30]).formatstr("%08X");
	state_add(MP_R31, "r31", m_reg[31]).formatstr("%08X");

	state_add(MP_ACC0, "acc0", m_acc[0]).formatstr("%016X");
	state_add(MP_ACC1, "acc1", m_acc[1]).formatstr("%016X");
	state_add(MP_ACC2, "acc2", m_acc[2]).formatstr("%016X");
	state_add(MP_ACC3, "acc3", m_acc[3]).formatstr("%016X");

	state_add(MP_IN0P, "in0p", m_in0p).formatstr("%08X");
	state_add(MP_IN1P, "in1p", m_in1p).formatstr("%08X");
	state_add(MP_OUTP, "outp", m_outp).formatstr("%08X");
	state_add(MP_IE, "ie", m_ie).formatstr("%08X");
	state_add(MP_INTPEN, "intpen", m_intpen).formatstr("%08X");

	state_add(MP_TCOUNT, "tcount", m_tcount).formatstr("%08X");
	state_add(MP_TSCALE, "tscale", m_tscale).formatstr("%08X");

	state_add(STATE_GENPC, "curpc", m_pc).noshow();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	m_icountptr = &m_icount;
}

void tms32082_mp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = "?";
			break;
	}
}

void tms32082_mp_device::device_reset()
{
	m_pc = 0;
	m_fetchpc = 0xfffffff8;

	for (int i=0; i < 32; i++)
	{
		m_reg[i] = 0;
	}

	m_acc[0] = 0;
	m_acc[1] = 0;
	m_acc[2] = 0;
	m_acc[3] = 0;

	m_in0p = 0;
	m_in1p = 0;
	m_outp = 0;

	m_intpen = 0;
	m_ie = 0;
}

void tms32082_mp_device::processor_command(UINT32 command)
{
	printf("MP CMND %08X: ", command);

	if (command & 0x80000000)
		printf("Reset ");
	if (command & 0x40000000)
		printf("Halt ");
	if (command & 0x20000000)
		printf("Unhalt ");
	if (command & 0x10000000)
		printf("ICR ");
	if (command & 0x08000000)
		printf("DCR ");
	if (command & 0x00004000)
		printf("Task ");
	if (command & 0x00002000)
		printf("Msg ");

	printf("to: ");

	if (command & 0x00000400)
		printf("VC ");
	if (command & 0x00000200)
		printf("TC ");
	if (command & 0x00000100)
		printf("MP ");
	if (command & 0x00000008)
		printf("PP3 ");
	if (command & 0x00000004)
		printf("PP2 ");
	if (command & 0x00000002)
		printf("PP1 ");
	if (command & 0x00000001)
		printf("PP0 ");

	if (!m_cmd_callback.isnull())
		m_cmd_callback(*m_program, 0, command, 0xffffffff);

	printf("\n");
}

UINT32 tms32082_mp_device::read_creg(int reg)
{
	switch (reg)
	{
		case 0x0:           // EPC
			return m_epc;

		case 0x1:           // EIP
			return m_eip;

		case 0x4:           // INTPEN
			return m_intpen;

		case 0x6:           // IE
			return m_ie;

		case 0xa:           // PPERROR
			return 0xe0000;

		case 0xe:           // TCOUNT
			return m_tcount;

		case 0x4000:        // IN0P
			return m_in0p;

		case 0x4001:        // IN1P
			return m_in1p;

		case 0x4002:        // OUTP
			return m_outp;

		default:
			printf("read_creg(): %08X\n", reg);
			break;
	}
	return 0;
}

void tms32082_mp_device::write_creg(int reg, UINT32 data)
{
	switch (reg)
	{
		case 0x0:           // EPC
			m_epc = data;
			break;

		case 0x1:           // EIP
			m_eip = data;
			break;

		case 0x4:           // INTPEN
		{
			for (int i=0; i < 32; i++)
			{
				if (data & (1 << i))
					m_intpen &= ~(1 << i);
			}
			break;
		}

		case 0x6:           // IE
			m_ie = data;
			printf("IE = %08X\n", data);
			break;

		case 0xe:           // TCOUNT
			m_tcount = data;
			break;

		case 0x4000:        // IN0P
			m_in0p = data;
			break;

		case 0x4001:        // IN1P
			m_in1p = data;
			break;

		case 0x4002:        // OUTP
			m_outp = data;
			break;

		default:
			printf("write_creg(): %08X, %08X\n", reg, data);
			break;
	}
}

void tms32082_mp_device::check_interrupts()
{
	if (m_ie & 1)       // global interrupt mask
	{
		for (int i=1; i < 32; i++)
		{
			if (m_ie & m_intpen & (1 << i))
			{
				m_epc = (m_fetchpc & ~3);
				m_epc |= (m_ie & 1);        // save global interrupt mask
				// TODO: user mode bit to EPC

				m_eip = m_pc;

				m_ie &= ~1;                 // clear global interrupt mask

				// get new pc from vector table
				m_fetchpc = m_pc = m_program->read_dword(0x01010180 + (i * 4));
				return;
			}
		}
	}
}

void tms32082_mp_device::execute_set_input(int inputnum, int state)
{
	if (state == ASSERT_LINE)
	{
		switch (inputnum)
		{
			case INPUT_X1:
				m_intpen |= (1 << 11);
				break;
			case INPUT_X2:
				m_intpen |= (1 << 12);
				break;
			case INPUT_X3:
				m_intpen |= (1 << 29);
				break;
			case INPUT_X4:
				m_intpen |= (1 << 30);
				break;
			default:
				break;
		}
	}
}

UINT32 tms32082_mp_device::fetch()
{
	UINT32 w = m_direct->read_dword(m_fetchpc);
	m_fetchpc += 4;
	return w;
}

void tms32082_mp_device::delay_slot()
{
	debugger_instruction_hook(this, m_pc);
	m_ir = fetch();
	execute();

	m_icount--;
}

void tms32082_mp_device::execute_run()
{
	while (m_icount > 0)
	{
		m_pc = m_fetchpc;

		check_interrupts();

		debugger_instruction_hook(this, m_pc);

		m_ir = fetch();
		execute();

		m_tcount--;
		if (m_tcount < 0)
		{
			// TODO: timer interrupt
			m_tcount = m_tscale;
		}

		m_icount--;
	};

	return;
}




// Parallel Processor

// internal memory map
static ADDRESS_MAP_START(pp_internal_map, AS_PROGRAM, 32, tms32082_pp_device)
	AM_RANGE(0x00000000, 0x00000fff) AM_RAM AM_SHARE("pp0_data0")
	AM_RANGE(0x00001000, 0x00001fff) AM_RAM AM_SHARE("pp1_data0")
	AM_RANGE(0x00008000, 0x00008fff) AM_RAM AM_SHARE("pp0_data1")
	AM_RANGE(0x00009000, 0x00009fff) AM_RAM AM_SHARE("pp1_data1")
	AM_RANGE(0x01000000, 0x01000fff) AM_RAM AM_SHARE("pp0_param")
	AM_RANGE(0x01001000, 0x01001fff) AM_RAM AM_SHARE("pp1_param")
ADDRESS_MAP_END

tms32082_pp_device::tms32082_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS32082_PP, "TMS32082 PP", tag, owner, clock, "tms32082_pp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, ADDRESS_MAP_NAME(pp_internal_map))
{
}


offs_t tms32082_pp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return CPU_DISASSEMBLE_NAME(tms32082_pp)(this, buffer, pc, oprom, opram, options);
}

void tms32082_pp_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_fetchpc));

	// Register state for debugger
	state_add(PP_PC, "pc", m_pc).formatstr("%08X");

	state_add(STATE_GENPC, "curpc", m_pc).noshow();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	m_icountptr = &m_icount;
}

void tms32082_pp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = "?";
			break;
	}
}

void tms32082_pp_device::device_reset()
{
	m_pc = 0;
	m_fetchpc = 0x400010a0;
}

void tms32082_pp_device::execute_run()
{
	m_pc = m_fetchpc;
	debugger_instruction_hook(this, m_pc);

	m_icount = 0;

	return;
}
