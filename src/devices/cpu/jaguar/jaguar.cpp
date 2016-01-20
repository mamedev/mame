// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jaguar.c
    Core implementation for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "jaguar.h"


#define LOG_GPU_IO      0
#define LOG_DSP_IO      0


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ZFLAG                   0x00001
#define CFLAG                   0x00002
#define NFLAG                   0x00004
#define IFLAG                   0x00008
#define EINT0FLAG               0x00010
#define EINT1FLAG               0x00020
#define EINT2FLAG               0x00040
#define EINT3FLAG               0x00080
#define EINT4FLAG               0x00100
#define EINT04FLAGS             (EINT0FLAG | EINT1FLAG | EINT2FLAG | EINT3FLAG | EINT4FLAG)
#define CINT0FLAG               0x00200
#define CINT1FLAG               0x00400
#define CINT2FLAG               0x00800
#define CINT3FLAG               0x01000
#define CINT4FLAG               0x02000
#define CINT04FLAGS             (CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)
#define RPAGEFLAG               0x04000
#define DMAFLAG                 0x08000
#define EINT5FLAG               0x10000     /* DSP only */
#define CINT5FLAG               0x20000     /* DSP only */

#define CLR_Z()                 (FLAGS &= ~ZFLAG)
#define CLR_ZN()                (FLAGS &= ~(ZFLAG | NFLAG))
#define CLR_ZNC()               (FLAGS &= ~(CFLAG | ZFLAG | NFLAG))
#define SET_Z(r)                (FLAGS |= ((r) == 0))
#define SET_C_ADD(a,b)          (FLAGS |= ((UINT32)(b) > (UINT32)(~(a))) << 1)
#define SET_C_SUB(a,b)          (FLAGS |= ((UINT32)(b) > (UINT32)(a)) << 1)
#define SET_N(r)                (FLAGS |= (((UINT32)(r) >> 29) & 4))
#define SET_ZN(r)               SET_N(r); SET_Z(r)
#define SET_ZNC_ADD(a,b,r)      SET_N(r); SET_Z(r); SET_C_ADD(a,b)
#define SET_ZNC_SUB(a,b,r)      SET_N(r); SET_Z(r); SET_C_SUB(a,b)



/***************************************************************************
    MACROS
***************************************************************************/

#define PC                  m_ctrl[G_PC]
#define FLAGS               m_ctrl[G_FLAGS]

#define CONDITION(x)        condition_table[(x) + ((FLAGS & 7) << 5)]

#define READBYTE(a)         m_program->read_byte(a)
#define READWORD(a)         m_program->read_word(a)
#define READLONG(a)         m_program->read_dword(a)

#define WRITEBYTE(a,v)      m_program->write_byte(a, v)
#define WRITEWORD(a,v)      m_program->write_word(a, v)
#define WRITELONG(a,v)      m_program->write_dword(a, v)



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

const UINT32 jaguar_cpu_device::convert_zero[32] =
{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };



/***************************************************************************
    FUNCTION TABLES
***************************************************************************/

const jaguar_cpu_device::op_func jaguar_cpu_device::gpu_op_table[64] =
{
	/* 00-03 */ &jaguar_cpu_device::add_rn_rn,      &jaguar_cpu_device::addc_rn_rn,     &jaguar_cpu_device::addq_n_rn,      &jaguar_cpu_device::addqt_n_rn,
	/* 04-07 */ &jaguar_cpu_device::sub_rn_rn,      &jaguar_cpu_device::subc_rn_rn,     &jaguar_cpu_device::subq_n_rn,      &jaguar_cpu_device::subqt_n_rn,
	/* 08-11 */ &jaguar_cpu_device::neg_rn,         &jaguar_cpu_device::and_rn_rn,      &jaguar_cpu_device::or_rn_rn,       &jaguar_cpu_device::xor_rn_rn,
	/* 12-15 */ &jaguar_cpu_device::not_rn,         &jaguar_cpu_device::btst_n_rn,      &jaguar_cpu_device::bset_n_rn,      &jaguar_cpu_device::bclr_n_rn,
	/* 16-19 */ &jaguar_cpu_device::mult_rn_rn,     &jaguar_cpu_device::imult_rn_rn,    &jaguar_cpu_device::imultn_rn_rn,   &jaguar_cpu_device::resmac_rn,
	/* 20-23 */ &jaguar_cpu_device::imacn_rn_rn,    &jaguar_cpu_device::div_rn_rn,      &jaguar_cpu_device::abs_rn,         &jaguar_cpu_device::sh_rn_rn,
	/* 24-27 */ &jaguar_cpu_device::shlq_n_rn,      &jaguar_cpu_device::shrq_n_rn,      &jaguar_cpu_device::sha_rn_rn,      &jaguar_cpu_device::sharq_n_rn,
	/* 28-31 */ &jaguar_cpu_device::ror_rn_rn,      &jaguar_cpu_device::rorq_n_rn,      &jaguar_cpu_device::cmp_rn_rn,      &jaguar_cpu_device::cmpq_n_rn,
	/* 32-35 */ &jaguar_cpu_device::sat8_rn,        &jaguar_cpu_device::sat16_rn,       &jaguar_cpu_device::move_rn_rn,     &jaguar_cpu_device::moveq_n_rn,
	/* 36-39 */ &jaguar_cpu_device::moveta_rn_rn,   &jaguar_cpu_device::movefa_rn_rn,   &jaguar_cpu_device::movei_n_rn,     &jaguar_cpu_device::loadb_rn_rn,
	/* 40-43 */ &jaguar_cpu_device::loadw_rn_rn,    &jaguar_cpu_device::load_rn_rn,     &jaguar_cpu_device::loadp_rn_rn,    &jaguar_cpu_device::load_r14n_rn,
	/* 44-47 */ &jaguar_cpu_device::load_r15n_rn,   &jaguar_cpu_device::storeb_rn_rn,   &jaguar_cpu_device::storew_rn_rn,   &jaguar_cpu_device::store_rn_rn,
	/* 48-51 */ &jaguar_cpu_device::storep_rn_rn,   &jaguar_cpu_device::store_rn_r14n,  &jaguar_cpu_device::store_rn_r15n,  &jaguar_cpu_device::move_pc_rn,
	/* 52-55 */ &jaguar_cpu_device::jump_cc_rn,     &jaguar_cpu_device::jr_cc_n,        &jaguar_cpu_device::mmult_rn_rn,    &jaguar_cpu_device::mtoi_rn_rn,
	/* 56-59 */ &jaguar_cpu_device::normi_rn_rn,    &jaguar_cpu_device::nop,            &jaguar_cpu_device::load_r14rn_rn,  &jaguar_cpu_device::load_r15rn_rn,
	/* 60-63 */ &jaguar_cpu_device::store_rn_r14rn, &jaguar_cpu_device::store_rn_r15rn, &jaguar_cpu_device::sat24_rn,       &jaguar_cpu_device::pack_rn
};

const jaguar_cpu_device::op_func jaguar_cpu_device::dsp_op_table[64] =
{
	/* 00-03 */ &jaguar_cpu_device::add_rn_rn,      &jaguar_cpu_device::addc_rn_rn,     &jaguar_cpu_device::addq_n_rn,      &jaguar_cpu_device::addqt_n_rn,
	/* 04-07 */ &jaguar_cpu_device::sub_rn_rn,      &jaguar_cpu_device::subc_rn_rn,     &jaguar_cpu_device::subq_n_rn,      &jaguar_cpu_device::subqt_n_rn,
	/* 08-11 */ &jaguar_cpu_device::neg_rn,         &jaguar_cpu_device::and_rn_rn,      &jaguar_cpu_device::or_rn_rn,       &jaguar_cpu_device::xor_rn_rn,
	/* 12-15 */ &jaguar_cpu_device::not_rn,         &jaguar_cpu_device::btst_n_rn,      &jaguar_cpu_device::bset_n_rn,      &jaguar_cpu_device::bclr_n_rn,
	/* 16-19 */ &jaguar_cpu_device::mult_rn_rn,     &jaguar_cpu_device::imult_rn_rn,    &jaguar_cpu_device::imultn_rn_rn,   &jaguar_cpu_device::resmac_rn,
	/* 20-23 */ &jaguar_cpu_device::imacn_rn_rn,    &jaguar_cpu_device::div_rn_rn,      &jaguar_cpu_device::abs_rn,         &jaguar_cpu_device::sh_rn_rn,
	/* 24-27 */ &jaguar_cpu_device::shlq_n_rn,      &jaguar_cpu_device::shrq_n_rn,      &jaguar_cpu_device::sha_rn_rn,      &jaguar_cpu_device::sharq_n_rn,
	/* 28-31 */ &jaguar_cpu_device::ror_rn_rn,      &jaguar_cpu_device::rorq_n_rn,      &jaguar_cpu_device::cmp_rn_rn,      &jaguar_cpu_device::cmpq_n_rn,
	/* 32-35 */ &jaguar_cpu_device::subqmod_n_rn,   &jaguar_cpu_device::sat16s_rn,      &jaguar_cpu_device::move_rn_rn,     &jaguar_cpu_device::moveq_n_rn,
	/* 36-39 */ &jaguar_cpu_device::moveta_rn_rn,   &jaguar_cpu_device::movefa_rn_rn,   &jaguar_cpu_device::movei_n_rn,     &jaguar_cpu_device::loadb_rn_rn,
	/* 40-43 */ &jaguar_cpu_device::loadw_rn_rn,    &jaguar_cpu_device::load_rn_rn,     &jaguar_cpu_device::sat32s_rn,      &jaguar_cpu_device::load_r14n_rn,
	/* 44-47 */ &jaguar_cpu_device::load_r15n_rn,   &jaguar_cpu_device::storeb_rn_rn,   &jaguar_cpu_device::storew_rn_rn,   &jaguar_cpu_device::store_rn_rn,
	/* 48-51 */ &jaguar_cpu_device::mirror_rn,      &jaguar_cpu_device::store_rn_r14n,  &jaguar_cpu_device::store_rn_r15n,  &jaguar_cpu_device::move_pc_rn,
	/* 52-55 */ &jaguar_cpu_device::jump_cc_rn,     &jaguar_cpu_device::jr_cc_n,        &jaguar_cpu_device::mmult_rn_rn,    &jaguar_cpu_device::mtoi_rn_rn,
	/* 56-59 */ &jaguar_cpu_device::normi_rn_rn,    &jaguar_cpu_device::nop,            &jaguar_cpu_device::load_r14rn_rn,  &jaguar_cpu_device::load_r15rn_rn,
	/* 60-63 */ &jaguar_cpu_device::store_rn_r14rn, &jaguar_cpu_device::store_rn_r15rn, &jaguar_cpu_device::illegal,        &jaguar_cpu_device::addqmod_n_rn
};



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)           (m_direct->read_word(pc, WORD_XOR_BE(0)))


const device_type JAGUARGPU = &device_creator<jaguargpu_cpu_device>;
const device_type JAGUARDSP = &device_creator<jaguardsp_cpu_device>;


jaguar_cpu_device::jaguar_cpu_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, bool isdsp)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 32, 24, 0)
	, m_isdsp(isdsp)
	, m_cpu_interrupt(*this)
	, m_tables_referenced(false)
	, table_refcount(0)
	, m_table(isdsp ? dsp_op_table : gpu_op_table)
{
	if (isdsp)
	{
		m_internal_ram_start = 0xf1b000;
		m_internal_ram_end = 0xf1cfff;
	}
	else
	{
		m_internal_ram_start = 0xf03000;
		m_internal_ram_end = 0xf03fff;
	}
}


jaguargpu_cpu_device::jaguargpu_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: jaguar_cpu_device(mconfig, JAGUARGPU, "Jaguar GPU", tag, owner, clock, "jaguargpu", __FILE__, false)
{
}


jaguardsp_cpu_device::jaguardsp_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: jaguar_cpu_device(mconfig, JAGUARDSP, "Jaguar DSP", tag, owner, clock, "jaguardsp", __FILE__, true)
{
}


void jaguar_cpu_device::update_register_banks()
{
	UINT32 temp;
	int i, bank;

	/* pick the bank */
	bank = FLAGS & RPAGEFLAG;
	if (FLAGS & IFLAG) bank = 0;

	/* do we need to swap? */
	if ((bank == 0 && m_b0 != m_r) || (bank != 0 && m_b1 != m_r))
	{
		/* remember the icount of the instruction after we swap */
		m_bankswitch_icount = m_icount - 1;

		/* exchange the contents */
		for (i = 0; i < 32; i++)
			temp = m_r[i], m_r[i] = m_a[i], m_a[i] = temp;

		/* swap the bank pointers */
		if (bank == 0)
		{
			m_b0 = m_r;
			m_b1 = m_a;
		}
		else
		{
			m_b0 = m_a;
			m_b1 = m_r;
		}
	}
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

void jaguar_cpu_device::check_irqs()
{
	int bits, mask, which = 0;

	/* if the IMASK is set, bail */
	if (FLAGS & IFLAG)
		return;

	/* get the active interrupt bits */
	bits = (m_ctrl[G_CTRL] >> 6) & 0x1f;
	bits |= (m_ctrl[G_CTRL] >> 10) & 0x20;

	/* get the interrupt mask */
	mask = (FLAGS >> 4) & 0x1f;
	mask |= (FLAGS >> 11) & 0x20;

	/* bail if nothing is available */
	bits &= mask;
	if (bits == 0)
		return;

	/* determine which interrupt */
	if (bits & 0x01) which = 0;
	if (bits & 0x02) which = 1;
	if (bits & 0x04) which = 2;
	if (bits & 0x08) which = 3;
	if (bits & 0x10) which = 4;
	if (bits & 0x20) which = 5;

	/* set the interrupt flag */
	FLAGS |= IFLAG;
	update_register_banks();

	/* push the PC-2 on the stack */
	m_r[31] -= 4;
	WRITELONG(m_r[31], PC - 2);

	/* dispatch */
	PC = (m_isdsp) ? 0xf1b000 : 0xf03000;
	PC += which * 0x10;
}


void jaguar_cpu_device::execute_set_input(int irqline, int state)
{
	int mask = (irqline < 5) ? (0x40 << irqline) : 0x10000;
	m_ctrl[G_CTRL] &= ~mask;
	if (state != CLEAR_LINE)
	{
		m_ctrl[G_CTRL] |= mask;
		check_irqs();
	}
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

void jaguar_cpu_device::init_tables()
{
	int i, j;

	m_tables_referenced = true;

	/* if we're not the first, skip */
	if (table_refcount++ != 0)
	{
		assert(mirror_table != nullptr);
		assert(condition_table != nullptr);
		return;
	}

	/* fill in the mirror table */
	mirror_table = std::make_unique<UINT16[]>(65536);
	for (i = 0; i < 65536; i++)
		mirror_table[i] = ((i >> 15) & 0x0001) | ((i >> 13) & 0x0002) |
							((i >> 11) & 0x0004) | ((i >> 9)  & 0x0008) |
							((i >> 7)  & 0x0010) | ((i >> 5)  & 0x0020) |
							((i >> 3)  & 0x0040) | ((i >> 1)  & 0x0080) |
							((i << 1)  & 0x0100) | ((i << 3)  & 0x0200) |
							((i << 5)  & 0x0400) | ((i << 7)  & 0x0800) |
							((i << 9)  & 0x1000) | ((i << 11) & 0x2000) |
							((i << 13) & 0x4000) | ((i << 15) & 0x8000);

	/* fill in the condition table */
	condition_table = std::make_unique<UINT8[]>(32 * 8);
	for (i = 0; i < 8; i++)
		for (j = 0; j < 32; j++)
		{
			int result = 1;
			if (j & 1)
				if (i & ZFLAG) result = 0;
			if (j & 2)
				if (!(i & ZFLAG)) result = 0;
			if (j & 4)
				if (i & (CFLAG << (j >> 4))) result = 0;
			if (j & 8)
				if (!(i & (CFLAG << (j >> 4)))) result = 0;
			condition_table[i * 32 + j] = result;
		}
}


void jaguar_cpu_device::jaguar_postload()
{
	update_register_banks();
	check_irqs();
}


void jaguar_cpu_device::device_start()
{
	init_tables();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_cpu_interrupt.resolve_safe();

	save_item(NAME(m_r));
	save_item(NAME(m_a));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_ppc));
	machine().save().register_postload(save_prepost_delegate(FUNC(jaguar_cpu_device::jaguar_postload), this));

	if (m_isdsp)
	{
		m_internal_ram_start = 0xf1b000;
		m_internal_ram_end = 0xf1cfff;
	}
	else
	{
		m_internal_ram_start = 0xf03000;
		m_internal_ram_end = 0xf03fff;
	}

	memset(m_r, 0, sizeof(m_r));
	memset(m_a, 0, sizeof(m_a));
	memset(m_ctrl, 0, sizeof(m_ctrl));
	m_ppc = 0;
	m_accum = 0;
	m_bankswitch_icount = 0;

	state_add( JAGUAR_PC,    "PC", PC).formatstr("%08X");
	state_add( JAGUAR_FLAGS, "FLAGS", FLAGS).formatstr("%08X");
	state_add( JAGUAR_R0,    "R0", m_r[0]).formatstr("%08X");
	state_add( JAGUAR_R1,    "R1", m_r[1]).formatstr("%08X");
	state_add( JAGUAR_R2,    "R2", m_r[2]).formatstr("%08X");
	state_add( JAGUAR_R3,    "R3", m_r[3]).formatstr("%08X");
	state_add( JAGUAR_R4,    "R4", m_r[4]).formatstr("%08X");
	state_add( JAGUAR_R5,    "R5", m_r[5]).formatstr("%08X");
	state_add( JAGUAR_R6,    "R6", m_r[6]).formatstr("%08X");
	state_add( JAGUAR_R7,    "R7", m_r[7]).formatstr("%08X");
	state_add( JAGUAR_R8,    "R8", m_r[8]).formatstr("%08X");
	state_add( JAGUAR_R9,    "R9", m_r[9]).formatstr("%08X");
	state_add( JAGUAR_R10,   "R10", m_r[10]).formatstr("%08X");
	state_add( JAGUAR_R11,   "R11", m_r[11]).formatstr("%08X");
	state_add( JAGUAR_R12,   "R12", m_r[12]).formatstr("%08X");
	state_add( JAGUAR_R13,   "R13", m_r[13]).formatstr("%08X");
	state_add( JAGUAR_R14,   "R14", m_r[14]).formatstr("%08X");
	state_add( JAGUAR_R15,   "R15", m_r[15]).formatstr("%08X");
	state_add( JAGUAR_R16,   "R16", m_r[16]).formatstr("%08X");
	state_add( JAGUAR_R17,   "R17", m_r[17]).formatstr("%08X");
	state_add( JAGUAR_R18,   "R18", m_r[18]).formatstr("%08X");
	state_add( JAGUAR_R19,   "R19", m_r[19]).formatstr("%08X");
	state_add( JAGUAR_R20,   "R20", m_r[20]).formatstr("%08X");
	state_add( JAGUAR_R21,   "R21", m_r[21]).formatstr("%08X");
	state_add( JAGUAR_R22,   "R22", m_r[22]).formatstr("%08X");
	state_add( JAGUAR_R23,   "R23", m_r[23]).formatstr("%08X");
	state_add( JAGUAR_R24,   "R24", m_r[24]).formatstr("%08X");
	state_add( JAGUAR_R25,   "R25", m_r[25]).formatstr("%08X");
	state_add( JAGUAR_R26,   "R26", m_r[26]).formatstr("%08X");
	state_add( JAGUAR_R27,   "R27", m_r[27]).formatstr("%08X");
	state_add( JAGUAR_R28,   "R28", m_r[28]).formatstr("%08X");
	state_add( JAGUAR_R29,   "R29", m_r[29]).formatstr("%08X");
	state_add( JAGUAR_R30,   "R30", m_r[30]).formatstr("%08X");
	state_add( JAGUAR_R31,   "R31", m_r[31]).formatstr("%08X");

	state_add( STATE_GENPC, "GENPC", PC).noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", FLAGS).formatstr("%11s").noshow();

	m_icountptr = &m_icount;
}


void jaguar_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c",
				FLAGS & 0x8000 ? 'D':'.',
				FLAGS & 0x4000 ? 'A':'.',
				FLAGS & 0x0100 ? '4':'.',
				FLAGS & 0x0080 ? '3':'.',
				FLAGS & 0x0040 ? '2':'.',
				FLAGS & 0x0020 ? '1':'.',
				FLAGS & 0x0010 ? '0':'.',
				FLAGS & 0x0008 ? 'I':'.',
				FLAGS & 0x0004 ? 'N':'.',
				FLAGS & 0x0002 ? 'C':'.',
				FLAGS & 0x0001 ? 'Z':'.');
			break;
	}
}


void jaguar_cpu_device::device_reset()
{
	m_b0 = m_r;
	m_b1 = m_a;
}


jaguar_cpu_device::~jaguar_cpu_device()
{
	if ( !m_tables_referenced )
		return;

	if (--table_refcount != 0)
		return;

	mirror_table = nullptr;
	condition_table = nullptr;
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

void jaguargpu_cpu_device::execute_run()
{
	/* if we're halted, we shouldn't be here */
	if (!(m_ctrl[G_CTRL] & 1))
	{
		//device->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_icount = 0;
		return;
	}

	/* check for IRQs */
	check_irqs();

	/* count cycles and interrupt cycles */
	m_bankswitch_icount = -1000;

	/* core execution loop */
	do
	{
		UINT32 op;

		/* debugging */
		//if (PC < 0xf03000 || PC > 0xf04000) { fatalerror("GPU: PC = %06X (ppc = %06X)\n", PC, m_ppc); }
		m_ppc = PC;
		debugger_instruction_hook(this, PC);

		/* instruction fetch */
		op = ROPCODE(PC);
		PC += 2;

		/* parse the instruction */
		(this->*gpu_op_table[op >> 10])(op);
		m_icount--;

	} while (m_icount > 0 || m_icount == m_bankswitch_icount);
}

void jaguardsp_cpu_device::execute_run()
{
	/* if we're halted, we shouldn't be here */
	if (!(m_ctrl[G_CTRL] & 1))
	{
		//device->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_icount = 0;
		return;
	}

	/* check for IRQs */
	check_irqs();

	/* count cycles and interrupt cycles */
	m_bankswitch_icount = -1000;

	/* core execution loop */
	do
	{
		UINT32 op;

		/* debugging */
		//if (PC < 0xf1b000 || PC > 0xf1d000) { fatalerror(stderr, "DSP: PC = %06X\n", PC); }
		m_ppc = PC;
		debugger_instruction_hook(this, PC);

		/* instruction fetch */
		op = ROPCODE(PC);
		PC += 2;

		/* parse the instruction */
		(this->*dsp_op_table[op >> 10])(op);
		m_icount--;

	} while (m_icount > 0 || m_icount == m_bankswitch_icount);
}



/***************************************************************************
    OPCODES
***************************************************************************/

void jaguar_cpu_device::abs_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 res = m_r[dreg];
	CLR_ZNC();
	if (res & 0x80000000)
	{
		m_r[dreg] = res = -res;
		FLAGS |= CFLAG;
	}
	SET_Z(res);
}

void jaguar_cpu_device::add_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 + r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addc_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 c = ((FLAGS >> 1) & 1);
	UINT32 res = r2 + r1 + c;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1 + c, res);
}

void jaguar_cpu_device::addq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 + r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addqmod_n_rn(UINT16 op)  /* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 + r1;
	res = (res & ~m_ctrl[D_MOD]) | (r2 & ~m_ctrl[D_MOD]);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addqt_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 + r1;
	m_r[dreg] = res;
}

void jaguar_cpu_device::and_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 & r1;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::bclr_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 & ~(1 << r1);
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::bset_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 | (1 << r1);
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::btst_n_rn(UINT16 op)
{
	UINT32 r1 = (op >> 5) & 31;
	UINT32 r2 = m_r[op & 31];
	CLR_Z(); FLAGS |= (~r2 >> r1) & 1;
}

void jaguar_cpu_device::cmp_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::cmpq_n_rn(UINT16 op)
{
	UINT32 r1 = (INT8)(op >> 2) >> 3;
	UINT32 r2 = m_r[op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::div_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	if (r1)
	{
		if (m_ctrl[D_DIVCTRL] & 1)
		{
			m_r[dreg] = ((UINT64)r2 << 16) / r1;
			m_ctrl[D_REMAINDER] = ((UINT64)r2 << 16) % r1;
		}
		else
		{
			m_r[dreg] = r2 / r1;
			m_ctrl[D_REMAINDER] = r2 % r1;
		}
	}
	else
		m_r[dreg] = 0xffffffff;
}

void jaguar_cpu_device::illegal(UINT16 op)
{
}

void jaguar_cpu_device::imacn_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[op & 31];
	m_accum += (INT64)((INT16)r1 * (INT16)r2);
	logerror("Unexpected IMACN instruction!\n");
}

void jaguar_cpu_device::imult_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::imultn_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	m_accum = (INT32)res;
	CLR_ZN(); SET_ZN(res);

	op = ROPCODE(PC);
	while ((op >> 10) == 20)
	{
		r1 = m_r[(op >> 5) & 31];
		r2 = m_r[op & 31];
		m_accum += (INT64)((INT16)r1 * (INT16)r2);
		PC += 2;
		op = ROPCODE(PC);
	}
	if ((op >> 10) == 19)
	{
		PC += 2;
		m_r[op & 31] = (UINT32)m_accum;
	}
}

void jaguar_cpu_device::jr_cc_n(UINT16 op)
{
	if (CONDITION(op & 31))
	{
		INT32 r1 = (INT8)((op >> 2) & 0xf8) >> 2;
		UINT32 newpc = PC + r1;
		debugger_instruction_hook(this, PC);
		op = ROPCODE(PC);
		PC = newpc;
		(this->*m_table[op >> 10])(op);

		m_icount -= 3;    /* 3 wait states guaranteed */
	}
}

void jaguar_cpu_device::jump_cc_rn(UINT16 op)
{
	if (CONDITION(op & 31))
	{
		UINT8 reg = (op >> 5) & 31;

		/* special kludge for risky code in the cojag DSP interrupt handlers */
		UINT32 newpc = (m_icount == m_bankswitch_icount) ? m_a[reg] : m_r[reg];
		debugger_instruction_hook(this, PC);
		op = ROPCODE(PC);
		PC = newpc;
		(this->*m_table[op >> 10])(op);

		m_icount -= 3;    /* 3 wait states guaranteed */
	}
}

void jaguar_cpu_device::load_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(r1);
}

void jaguar_cpu_device::load_r14n_rn(UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[14] + 4 * r1);
}

void jaguar_cpu_device::load_r15n_rn(UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[15] + 4 * r1);
}

void jaguar_cpu_device::load_r14rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[14] + r1);
}

void jaguar_cpu_device::load_r15rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[15] + r1);
}

void jaguar_cpu_device::loadb_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_r[op & 31] = READBYTE(r1);
	}
}

void jaguar_cpu_device::loadw_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_r[op & 31] = READWORD(r1);
	}
}

void jaguar_cpu_device::loadp_rn_rn(UINT16 op)   /* GPU only */
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_ctrl[G_HIDATA] = READLONG(r1);
		m_r[op & 31] = READLONG(r1+4);
	}
}

void jaguar_cpu_device::mirror_rn(UINT16 op) /* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = m_r[dreg];
	UINT32 res = (mirror_table[r1 & 0xffff] << 16) | mirror_table[r1 >> 16];
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::mmult_rn_rn(UINT16 op)
{
	int count = m_ctrl[G_MTXC] & 15, i;
	int sreg = (op >> 5) & 31;
	int dreg = op & 31;
	UINT32 addr = m_ctrl[G_MTXA];
	INT64 accum = 0;
	UINT32 res;

	if (!(m_ctrl[G_MTXC] & 0x10))
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(m_b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)READWORD(addr);
			addr += 2;
		}
	}
	else
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(m_b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)READWORD(addr);
			addr += 2 * count;
		}
	}
	m_r[dreg] = res = (UINT32)accum;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::move_rn_rn(UINT16 op)
{
	m_r[op & 31] = m_r[(op >> 5) & 31];
}

void jaguar_cpu_device::move_pc_rn(UINT16 op)
{
	m_r[op & 31] = m_ppc;
}

void jaguar_cpu_device::movefa_rn_rn(UINT16 op)
{
	m_r[op & 31] = m_a[(op >> 5) & 31];
}

void jaguar_cpu_device::movei_n_rn(UINT16 op)
{
	UINT32 res = ROPCODE(PC) | (ROPCODE(PC + 2) << 16);
	PC += 4;
	m_r[op & 31] = res;
}

void jaguar_cpu_device::moveq_n_rn(UINT16 op)
{
	m_r[op & 31] = (op >> 5) & 31;
}

void jaguar_cpu_device::moveta_rn_rn(UINT16 op)
{
	m_a[op & 31] = m_r[(op >> 5) & 31];
}

void jaguar_cpu_device::mtoi_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = (((INT32)r1 >> 8) & 0xff800000) | (r1 & 0x007fffff);
}

void jaguar_cpu_device::mult_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = (UINT16)r1 * (UINT16)r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::neg_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r2 = m_r[dreg];
	UINT32 res = -r2;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(0, r2, res);
}

void jaguar_cpu_device::nop(UINT16 op)
{
}

void jaguar_cpu_device::normi_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 res = 0;
	if (r1 != 0)
	{
		while ((r1 & 0xffc00000) == 0)
		{
			r1 <<= 1;
			res--;
		}
		while ((r1 & 0xff800000) != 0)
		{
			r1 >>= 1;
			res++;
		}
	}
	m_r[op & 31] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::not_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 res = ~m_r[dreg];
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::or_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r1 | r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::pack_rn(UINT16 op)       /* GPU only */
{
	int dreg = op & 31;
	int pack = (op >> 5) & 31;
	UINT32 r2 = m_r[dreg];
	UINT32 res;
	if (pack == 0)    /* PACK */
		res = ((r2 >> 10) & 0xf000) | ((r2 >> 5) & 0x0f00) | (r2 & 0xff);
	else            /* UNPACK */
		res = ((r2 & 0xf000) << 10) | ((r2 & 0x0f00) << 5) | (r2 & 0xff);
	m_r[dreg] = res;
}

void jaguar_cpu_device::resmac_rn(UINT16 op)
{
	m_r[op & 31] = (UINT32)m_accum;
}

void jaguar_cpu_device::ror_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31] & 31;
	UINT32 r2 = m_r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); FLAGS |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::rorq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); FLAGS |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::sat8_rn(UINT16 op)       /* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = m_r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 255) ? 255 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat16_rn(UINT16 op)      /* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = m_r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 65535) ? 65535 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat16s_rn(UINT16 op)     /* DSP only */
{
	int dreg = op & 31;
	INT32 r2 = m_r[dreg];
	UINT32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat24_rn(UINT16 op)          /* GPU only */
{
	int dreg = op & 31;
	INT32 r2 = m_r[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 16777215) ? 16777215 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat32s_rn(UINT16 op)     /* DSP only */
{
	int dreg = op & 31;
	INT32 r2 = (UINT32)m_r[dreg];
	INT32 temp = m_accum >> 32;
	UINT32 res = (temp < -1) ? (INT32)0x80000000 : (temp > 0) ? (INT32)0x7fffffff : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sh_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = (INT32)m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res;

	CLR_ZNC();
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		FLAGS |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? 0 : (r2 >> r1);
		FLAGS |= (r2 << 1) & 2;
	}
	m_r[dreg] = res;
	SET_ZN(res);
}

void jaguar_cpu_device::sha_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = (INT32)m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res;

	CLR_ZNC();
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		FLAGS |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? ((INT32)r2 >> 31) : ((INT32)r2 >> r1);
		FLAGS |= (r2 << 1) & 2;
	}
	m_r[dreg] = res;
	SET_ZN(res);
}

void jaguar_cpu_device::sharq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = (INT32)r2 >> r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); FLAGS |= (r2 << 1) & 2;
}

void jaguar_cpu_device::shlq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 << (32 - r1);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); FLAGS |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::shrq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	INT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 >> r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); FLAGS |= (r2 << 1) & 2;
}

void jaguar_cpu_device::store_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(r1, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r14n(UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(m_r[14] + r1 * 4, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r15n(UINT16 op)
{
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(m_r[15] + r1 * 4, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r14rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(m_r[14] + r1, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r15rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(m_r[15] + r1, m_r[op & 31]);
}

void jaguar_cpu_device::storeb_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITEBYTE(r1, m_r[op & 31]);
	}
}

void jaguar_cpu_device::storew_rn_rn(UINT16 op)
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITEWORD(r1, m_r[op & 31]);
	}
}

void jaguar_cpu_device::storep_rn_rn(UINT16 op)  /* GPU only */
{
	UINT32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITELONG(r1, m_ctrl[G_HIDATA]);
		WRITELONG(r1+4, m_r[op & 31]);
	}
}

void jaguar_cpu_device::sub_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 - r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subc_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 c = ((FLAGS >> 1) & 1);
	UINT32 res = r2 - r1 - c;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1 + c, res);
}

void jaguar_cpu_device::subq_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 - r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subqmod_n_rn(UINT16 op)  /* DSP only */
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 - r1;
	res = (res & ~m_ctrl[D_MOD]) | (r2 & ~m_ctrl[D_MOD]);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subqt_n_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = convert_zero[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r2 - r1;
	m_r[dreg] = res;
}

void jaguar_cpu_device::xor_rn_rn(UINT16 op)
{
	int dreg = op & 31;
	UINT32 r1 = m_r[(op >> 5) & 31];
	UINT32 r2 = m_r[dreg];
	UINT32 res = r1 ^ r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}



/***************************************************************************
    I/O HANDLING
***************************************************************************/

READ32_MEMBER( jaguargpu_cpu_device::ctrl_r  )
{
	if (LOG_GPU_IO) logerror("GPU read register @ F021%02X\n", offset * 4);

	return m_ctrl[offset];
}


WRITE32_MEMBER( jaguargpu_cpu_device::ctrl_w )
{
	UINT32 oldval, newval;

	if (LOG_GPU_IO && offset != G_HIDATA)
		logerror("GPU write register @ F021%02X = %08X\n", offset * 4, data);

	/* remember the old and set the new */
	oldval = m_ctrl[offset];
	newval = oldval;
	COMBINE_DATA(&newval);

	/* handle the various registers */
	switch (offset)
	{
		case G_FLAGS:

			/* combine the data properly */
			m_ctrl[offset] = newval & (ZFLAG | CFLAG | NFLAG | EINT04FLAGS | RPAGEFLAG);
			if (newval & IFLAG)
				m_ctrl[offset] |= oldval & IFLAG;

			/* clear interrupts */
			m_ctrl[G_CTRL] &= ~((newval & CINT04FLAGS) >> 3);

			/* determine which register bank should be active */
			update_register_banks();

			/* update IRQs */
			check_irqs();
			break;

		case G_MTXC:
		case G_MTXA:
			m_ctrl[offset] = newval;
			break;

		case G_END:
			m_ctrl[offset] = newval;
			if ((newval & 7) != 7)
				logerror("GPU to set to little-endian!\n");
			break;

		case G_PC:
			PC = newval & 0xffffff;
			break;

		case G_CTRL:
			m_ctrl[offset] = newval;
			if ((oldval ^ newval) & 0x01)
			{
				set_input_line(INPUT_LINE_HALT, (newval & 1) ? CLEAR_LINE : ASSERT_LINE);
				yield();
			}
			if (newval & 0x02)
			{
				m_cpu_interrupt(ASSERT_LINE);
				m_ctrl[offset] &= ~0x02;
			}
			if (newval & 0x04)
			{
				m_ctrl[G_CTRL] |= 1 << 6;
				m_ctrl[offset] &= ~0x04;
				check_irqs();
			}
			if (newval & 0x18)
			{
				logerror("GPU single stepping was enabled!\n");
			}
			break;

		case G_HIDATA:
		case G_DIVCTRL:
			m_ctrl[offset] = newval;
			break;
	}
}



/***************************************************************************
    I/O HANDLING
***************************************************************************/

READ32_MEMBER( jaguardsp_cpu_device::ctrl_r )
{
	if (LOG_DSP_IO && offset != D_FLAGS)
		logerror("DSP read register @ F1A1%02X\n", offset * 4);

	/* switch to the target context */
	return m_ctrl[offset];
}


WRITE32_MEMBER( jaguardsp_cpu_device::ctrl_w )
{
	UINT32 oldval, newval;

	if (LOG_DSP_IO && offset != D_FLAGS)
		logerror("DSP write register @ F1A1%02X = %08X\n", offset * 4, data);

	/* remember the old and set the new */
	oldval = m_ctrl[offset];
	newval = oldval;
	COMBINE_DATA(&newval);

	/* handle the various registers */
	switch (offset)
	{
		case D_FLAGS:

			/* combine the data properly */
			m_ctrl[offset] = newval & (ZFLAG | CFLAG | NFLAG | EINT04FLAGS | EINT5FLAG | RPAGEFLAG);
			if (newval & IFLAG)
				m_ctrl[offset] |= oldval & IFLAG;

			/* clear interrupts */
			m_ctrl[D_CTRL] &= ~((newval & CINT04FLAGS) >> 3);
			m_ctrl[D_CTRL] &= ~((newval & CINT5FLAG) >> 1);

			/* determine which register bank should be active */
			update_register_banks();

			/* update IRQs */
			check_irqs();
			break;

		case D_MTXC:
		case D_MTXA:
			m_ctrl[offset] = newval;
			break;

		case D_END:
			m_ctrl[offset] = newval;
			if ((newval & 7) != 7)
				logerror("DSP to set to little-endian!\n");
			break;

		case D_PC:
			PC = newval & 0xffffff;
			break;

		case D_CTRL:
			m_ctrl[offset] = newval;
			if ((oldval ^ newval) & 0x01)
			{
				set_input_line(INPUT_LINE_HALT, (newval & 1) ? CLEAR_LINE : ASSERT_LINE);
				yield();
			}
			if (newval & 0x02)
			{
				m_cpu_interrupt(ASSERT_LINE);
				m_ctrl[offset] &= ~0x02;
			}
			if (newval & 0x04)
			{
				m_ctrl[D_CTRL] |= 1 << 6;
				m_ctrl[offset] &= ~0x04;
				check_irqs();
			}
			if (newval & 0x18)
			{
				logerror("DSP single stepping was enabled!\n");
			}
			break;

		case D_MOD:
		case D_DIVCTRL:
			m_ctrl[offset] = newval;
			break;
	}
}


offs_t jaguargpu_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( jaguargpu );
	return CPU_DISASSEMBLE_NAME(jaguargpu)(this, buffer, pc, oprom, opram, options);
}


offs_t jaguardsp_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( jaguardsp );
	return CPU_DISASSEMBLE_NAME(jaguardsp)(this, buffer, pc, oprom, opram, options);
}
