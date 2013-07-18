/*
	Texas Instruments TMS320C82 DSP Emulator

	Written by Ville Linde

*/

#include "emu.h"
#include "debugger.h"
#include "tms32082.h"

extern CPU_DISASSEMBLE(tms32082_mp);

const device_type TMS32082_MP = &device_creator<tms32082_mp_device>;

// internal memory map
static ADDRESS_MAP_START(internal_map, AS_PROGRAM, 32, tms32082_mp_device)
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
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0, ADDRESS_MAP_NAME(internal_map))
{
}


offs_t tms32082_mp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return CPU_DISASSEMBLE_NAME(tms32082_mp)(this, buffer, pc, oprom, opram, options);
}



READ32_MEMBER(tms32082_mp_device::mp_param_r)
{
	printf("mp_param_w: %08X, %08X\n", offset, mem_mask);
	return 0;
}

WRITE32_MEMBER(tms32082_mp_device::mp_param_w)
{
	printf("mp_param_w: %08X, %08X, %08X\n", offset, data, mem_mask);
}



void tms32082_mp_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_fetchpc));
	save_item(NAME(m_reg));
	save_item(NAME(m_acc));

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

	state_add(STATE_GENPC, "curpc", m_pc).noshow();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	m_icountptr = &m_icount;
}

void tms32082_mp_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("?");
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
}

UINT32 tms32082_mp_device::read_creg(int reg)
{
	printf("read_creg(): %08X\n", reg);
	return 0;
}

void tms32082_mp_device::write_creg(int reg, UINT32 data)
{
	printf("write_creg(): %08X, %08X\n", reg, data);
}

UINT32 tms32082_mp_device::fetch()
{
	UINT32 w = m_direct->read_decrypted_dword(m_fetchpc);
	m_fetchpc += 4;
	return w;
}

void tms32082_mp_device::execute_run()
{
	while (m_icount > 0)
	{
		m_pc = m_fetchpc;
		debugger_instruction_hook(this, m_pc);

		m_ir = fetch();
		execute();

		m_icount--;
	};

	return;
}