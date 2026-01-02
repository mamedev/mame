// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jaguar.cpp
    Core implementation for the portable Jaguar DSP emulator.
    Written by Aaron Giles

    TODO:
    - Implement pipeline, actually instruction cycles;
      Currently implementation is similar to single stepping
      with single cycle
    - Implement and acknowlodge remain registers;
    - Improve delay slot display in debugger (highlight current instruction
      doesn't work but instruction hook does);

***************************************************************************/

#include "emu.h"
#include "jaguar.h"
#include "jagdasm.h"

#include <algorithm>

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum : u32
{
	ZFLAG       = 0x00001,
	CFLAG       = 0x00002,
	NFLAG       = 0x00004,
	IFLAG       = 0x00008,
	EINT0FLAG   = 0x00010,
	EINT1FLAG   = 0x00020,
	EINT2FLAG   = 0x00040,
	EINT3FLAG   = 0x00080,
	EINT4FLAG   = 0x00100,
	EINT04FLAGS = EINT0FLAG | EINT1FLAG | EINT2FLAG | EINT3FLAG | EINT4FLAG,
	CINT0FLAG   = 0x00200,
	CINT1FLAG   = 0x00400,
	CINT2FLAG   = 0x00800,
	CINT3FLAG   = 0x01000,
	CINT4FLAG   = 0x02000,
	CINT04FLAGS = CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG,
	RPAGEFLAG   = 0x04000,
	DMAFLAG     = 0x08000,
	EINT5FLAG   = 0x10000,      // DSP only
	CINT5FLAG   = 0x20000       // DSP only
};

inline void jaguar_cpu_device::CLR_Z()                          { m_flags &= ~ZFLAG; }
inline void jaguar_cpu_device::CLR_ZN()                         { m_flags &= ~(ZFLAG | NFLAG); }
inline void jaguar_cpu_device::CLR_ZNC()                        { m_flags &= ~(CFLAG | ZFLAG | NFLAG); }
inline void jaguar_cpu_device::SET_Z(u32 r)                     { m_flags |= (r == 0); }
inline void jaguar_cpu_device::SET_C_ADD(u32 a, u32 b)          { m_flags |= (b > (~a)) << 1; }
inline void jaguar_cpu_device::SET_C_SUB(u32 a, u32 b)          { m_flags |= (b > a) << 1; }
inline void jaguar_cpu_device::SET_N(u32 r)                     { m_flags |= ((r >> 29) & 4); }
inline void jaguar_cpu_device::SET_ZN(u32 r)                    { SET_N(r); SET_Z(r); }
inline void jaguar_cpu_device::SET_ZNC_ADD(u32 a, u32 b, u32 r) { SET_N(r); SET_Z(r); SET_C_ADD(a, b); }
inline void jaguar_cpu_device::SET_ZNC_SUB(u32 a, u32 b, u32 r) { SET_N(r); SET_Z(r); SET_C_SUB(a, b); }


/***************************************************************************
    MACROS
***************************************************************************/

inline u8 jaguar_cpu_device::CONDITION(u8 x)
{
	return condition_table[x + ((m_flags & 7) << 5)];
}

inline u8 jaguar_cpu_device::READBYTE(offs_t a)  { return m_program.read_byte(a); }
inline u16 jaguar_cpu_device::READWORD(offs_t a) { return m_program.read_word(a); }
inline u32 jaguar_cpu_device::READLONG(offs_t a)
{
	// - fball95 GPU
	// - kasumi GPU 0x01e0'34ce (???)
	// - sensible DSP $b14689e2 (???)
	//if (a & 2)
	//{
	//	printf("%d: %08x R\n", m_isdsp, a);
	//	u32 res = READWORD(a) << 16;
	//	res |= READWORD(a + 2) & 0xffff;
	//	//machine().debug_break();
	//	return res;
	//}

	return m_program.read_dword(a);
}

inline void jaguar_cpu_device::WRITEBYTE(offs_t a, u8 v)  { m_program.write_byte(a, v); }
inline void jaguar_cpu_device::WRITEWORD(offs_t a, u16 v) { m_program.write_word(a, v); }
inline void jaguar_cpu_device::WRITELONG(offs_t a, u32 v)
{
	// TODO: protect/protctse wants proper alignment at PC=f03004
	//   (wants to reprogram border color registers, would otherwise hit VMODE)
	// - atarikrt $06bf 0x0000'0007 (?)
	// - barkley/bretth/chekflag DSP
	// - kasumi $f0'34ab 0x0000'0003 (?)
	// - pdrive $580e (?)
	//if (a & 2)
	//{
	//	printf("%d: %08x %08x W\n", m_isdsp, a, v);
	//	WRITEWORD(a, v >> 16);
	//	WRITEWORD(a + 2, v & 0xffff);
	//  return;
	//}
	m_program.write_dword(a, v);
}


/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

const u32 jaguar_cpu_device::convert_zero[32] =
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

inline u16 jaguar_cpu_device::ROPCODE(offs_t pc) { return m_cache.read_word(pc); }

// SC414200AT
DEFINE_DEVICE_TYPE(JAGUARGPU, jaguargpu_cpu_device, "jaguargpu", "Motorola Atari Jaguar GPU \"Tom\"")
// SC414201FT
DEFINE_DEVICE_TYPE(JAGUARDSP, jaguardsp_cpu_device, "jaguardsp", "Motorola Atari Jaguar DSP \"Jerry\"")


jaguar_cpu_device::jaguar_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 version, bool isdsp, address_map_constructor io_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 24, 0)
	, m_io_config("io", ENDIANNESS_BIG, 32, 8, 0, io_map)
	, m_version(version) // 1 : Jaguar prototype, 2 : Jaguar first release, 3 : Midsummer prototype, Other : unknown/reserved
	, m_isdsp(isdsp)
	, m_cpu_interrupt(*this)
	, m_tables_referenced(false)
	, table_refcount(0)
	, m_table(isdsp ? dsp_op_table : gpu_op_table)
	, m_io_end(0x00070007)
	, m_io_pc(0)
	, m_io_status(0)
	, m_pc(0)
	, m_flags(0)
	, m_imask(false)
	, m_maddw(0)
	, m_mwidth(0)
	, m_mtxaddr(0)
	, m_go(false)
	, m_int_latch(0)
	, m_int_mask(0)
	, m_bus_hog(false)
	, m_div_remainder(0)
	, m_div_offset(false)
	, m_hidata(0)
	, m_modulo(0xffffffff)
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


jaguargpu_cpu_device::jaguargpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaguar_cpu_device(mconfig, JAGUARGPU, tag, owner, clock, 2, false, address_map_constructor(FUNC(jaguargpu_cpu_device::io_map), this))
{
}


jaguardsp_cpu_device::jaguardsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaguar_cpu_device(mconfig, JAGUARDSP, tag, owner, clock, 2, true, address_map_constructor(FUNC(jaguardsp_cpu_device::io_map), this))
{
}

device_memory_interface::space_config_vector jaguar_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}


void jaguar_cpu_device::update_register_banks()
{
	// - feverpit doesn't want this
//	if (m_go == false)
//		return;

	/* pick the bank */
	u32 bank = m_flags & RPAGEFLAG;
	if (m_imask == true) bank = 0;

	/* do we need to swap? */
	if ((bank == 0 && m_b0 != m_r) || (bank != 0 && m_b1 != m_r))
	{
		/* remember the icount of the instruction after we swap */
		m_bankswitch_icount = m_icount - 1;

		/* exchange the contents */
		for (int i = 0; i < 32; i++)
		{
			const u32 temp = m_r[i];
			m_r[i] = m_a[i];
			m_a[i] = temp;
		}

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
	int which = 0;

	/* if the IMASK is set, bail */
	if (m_imask == true || m_go == false)
		return;

	u8 latch = m_int_latch;
	u8 mask = m_int_mask;

	/* bail if nothing is available */
//	if (m_isdsp)
//		printf("%02x %02x\n", latch, mask);
	latch &= mask;
	if (latch == 0)
		return;

	/* determine which interrupt */
	for (int i = 0; i < 6; i++)
		if (latch & (1 << i))
			which = i;

	/* set the interrupt flag */
	m_imask = true;
	update_register_banks();

	/* push the m_pc-2 on the stack */
	m_r[31] -= 4;
	WRITELONG(m_r[31], m_pc - 2);

	/* dispatch */
	m_pc = m_internal_ram_start;
	m_pc += which * 0x10;
}


void jaguar_cpu_device::execute_set_input(int irqline, int state)
{
	const u32 mask = (1 << irqline);
	m_int_latch &= ~mask;
	// Ignore irq if masked
	// - barkley, breakout, clubdriv, ironsol2, skyhamm, ultravor all wants to not read a pending
	//   DSP serial irq *before* JPIT
	if (state != CLEAR_LINE && BIT(m_int_mask, irqline))
	{
		m_int_latch |= mask;
		check_irqs();
	}
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

void jaguar_cpu_device::init_tables()
{
	m_tables_referenced = true;

	/* if we're not the first, skip */
	if (table_refcount++ != 0)
	{
		assert(mirror_table != nullptr);
		assert(condition_table != nullptr);
		return;
	}

	/* fill in the mirror table */
	mirror_table = std::make_unique<u16[]>(65536);
	for (int i = 0; i < 65536; i++)
		mirror_table[i] = ((i >> 15) & 0x0001) | ((i >> 13) & 0x0002) |
							((i >> 11) & 0x0004) | ((i >> 9)  & 0x0008) |
							((i >> 7)  & 0x0010) | ((i >> 5)  & 0x0020) |
							((i >> 3)  & 0x0040) | ((i >> 1)  & 0x0080) |
							((i << 1)  & 0x0100) | ((i << 3)  & 0x0200) |
							((i << 5)  & 0x0400) | ((i << 7)  & 0x0800) |
							((i << 9)  & 0x1000) | ((i << 11) & 0x2000) |
							((i << 13) & 0x4000) | ((i << 15) & 0x8000);

	/* fill in the condition table */
	condition_table = std::make_unique<u8[]>(32 * 8);
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 32; j++)
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


void jaguar_cpu_device::device_post_load()
{
	update_register_banks();
	check_irqs();
}


void jaguar_cpu_device::device_start()
{
	init_tables();

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_r));
	save_item(NAME(m_a));
	save_item(NAME(m_ppc));
	save_item(NAME(m_go));
	save_item(NAME(m_int_latch));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_bus_hog));
	save_item(NAME(m_flags));
	save_item(NAME(m_imask));
	save_item(NAME(m_div_remainder));
	save_item(NAME(m_div_offset));

	save_item(NAME(m_io_end));
	save_item(NAME(m_io_pc));
	save_item(NAME(m_io_status));
	save_item(NAME(m_io_mtxc));
	save_item(NAME(m_io_mtxa));

	// TODO: data map
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

	std::fill(std::begin(m_r), std::end(m_r), 0);
	std::fill(std::begin(m_a), std::end(m_a), 0);
	m_ppc = 0;
	m_accum = 0;
	m_bankswitch_icount = 0;

	state_add( JAGUAR_PC,    "PC", m_pc).formatstr("%08X");
	state_add( JAGUAR_FLAGS, "FLAGS", m_flags).formatstr("%08X");
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

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_ppc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%11s").noshow();

	set_icountptr(m_icount);
}


void jaguar_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c",
				m_flags & 0x8000 ? 'D':'.',
				m_flags & 0x4000 ? 'A':'.',
				m_flags & 0x0100 ? '4':'.',
				m_flags & 0x0080 ? '3':'.',
				m_flags & 0x0040 ? '2':'.',
				m_flags & 0x0020 ? '1':'.',
				m_flags & 0x0010 ? '0':'.',
				m_imask == true  ? 'I':'.',
				m_flags & 0x0004 ? 'N':'.',
				m_flags & 0x0002 ? 'C':'.',
				m_flags & 0x0001 ? 'Z':'.');
			break;
	}
}


void jaguar_cpu_device::device_reset()
{
	m_b0 = m_r;
	m_b1 = m_a;
	m_modulo = 0xffffffff;
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
	if (m_go == false)
	{
		//device->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		debugger_wait_hook();
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
		/* debugging */
		//if ((m_version < 3) && (m_pc < 0xf03000 || m_pc > 0xf04000)) { fatalerror("GPU: m_pc = %06X (ppc = %06X)\n", m_pc, m_ppc); }
		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		/* instruction fetch */
		const u16 op = ROPCODE(m_pc);
		m_pc += 2;

		/* parse the instruction */
		(this->*gpu_op_table[op >> 10])(op);
		m_icount--;

	} while (m_icount > 0 || m_icount == m_bankswitch_icount);
}

void jaguardsp_cpu_device::execute_run()
{
	/* if we're halted, we shouldn't be here */
	if (m_go == false)
	{
		//device->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		debugger_wait_hook();
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
		/* debugging */
		//if (m_pc < 0xf1b000 || m_pc > 0xf1d000) { fatalerror(stderr, "DSP: m_pc = %06X\n", m_pc); }
		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		/* instruction fetch */
		const u16 op = ROPCODE(m_pc);
		m_pc += 2;

		/* parse the instruction */
		(this->*dsp_op_table[op >> 10])(op);
		m_icount--;

	} while (m_icount > 0 || m_icount == m_bankswitch_icount);
}


/***************************************************************************
    OPCODES
***************************************************************************/

void jaguar_cpu_device::abs_rn(u16 op)
{
	const u8 dreg = op & 31;
	u32 res = m_r[dreg];
	CLR_ZNC();
	if (res & 0x80000000)
	{
		m_r[dreg] = res = -res;
		m_flags |= CFLAG;
	}
	SET_Z(res);
}

void jaguar_cpu_device::add_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 + r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addc_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 c = ((m_flags >> 1) & 1);
	const u32 res = r2 + r1 + c;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1 + c, res);
}

void jaguar_cpu_device::addq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 + r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addqmod_n_rn(u16 op)  /* DSP only */
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 res = r2 + r1;
	res = (res & ~m_modulo) | (r2 & m_modulo);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_ADD(r2, r1, res);
}

void jaguar_cpu_device::addqt_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 + r1;
	m_r[dreg] = res;
}

void jaguar_cpu_device::and_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 & r1;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::bclr_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = (op >> 5) & 31;
	const u32 r2 = m_r[dreg];
	const u32 res = r2 & ~(1 << r1);
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::bset_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = (op >> 5) & 31;
	const u32 r2 = m_r[dreg];
	const u32 res = r2 | (1 << r1);
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::btst_n_rn(u16 op)
{
	const u32 r1 = (op >> 5) & 31;
	const u32 r2 = m_r[op & 31];
	CLR_Z(); m_flags |= (~r2 >> r1) & 1;
}

void jaguar_cpu_device::cmp_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[op & 31];
	const u32 res = r2 - r1;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::cmpq_n_rn(u16 op)
{
	const u32 r1 = (s8)(op >> 2) >> 3;
	const u32 r2 = m_r[op & 31];
	const u32 res = r2 - r1;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::div_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	if (r1)
	{
		if (m_div_offset & 1)
		{
			m_r[dreg] = ((u64)r2 << 16) / r1;
			m_div_remainder = ((u64)r2 << 16) % r1;
		}
		else
		{
			m_r[dreg] = r2 / r1;
			m_div_remainder = r2 % r1;
		}
	}
	else
	{
		// TODO: exact values for divide by zero
		m_r[dreg] = 0xffffffff;
		m_div_remainder = 0xffffffff;
	}
}

void jaguar_cpu_device::illegal(u16 op)
{
}

void jaguar_cpu_device::imacn_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[op & 31];
	m_accum += (s64)((int16_t)r1 * (int16_t)r2);
	// TODO: what's really "unexpected"?
	logerror("Unexpected IMACN instruction!\n");
}

void jaguar_cpu_device::imult_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = (int16_t)r1 * (int16_t)r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::imultn_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	u32 r1 = m_r[(op >> 5) & 31];
	u32 r2 = m_r[dreg];
	const u32 res = (int16_t)r1 * (int16_t)r2;
	m_accum = (s32)res;
	CLR_ZN(); SET_ZN(res);

	op = ROPCODE(m_pc);
	while ((op >> 10) == 20)
	{
		r1 = m_r[(op >> 5) & 31];
		r2 = m_r[op & 31];
		m_accum += (s64)((int16_t)r1 * (int16_t)r2);
		m_pc += 2;
		op = ROPCODE(m_pc);
	}
	if ((op >> 10) == 19)
	{
		m_pc += 2;
		m_r[op & 31] = (u32)m_accum;
	}
}

void jaguar_cpu_device::jr_cc_n(u16 op)
{
	if (CONDITION(op & 31))
	{
		const s32 r1 = (s8)((op >> 2) & 0xf8) >> 2;
		const u32 newpc = m_pc + r1;
		debugger_instruction_hook(m_pc);
		op = ROPCODE(m_pc);
		m_pc = newpc;
		(this->*m_table[op >> 10])(op);

		m_icount -= 3;    /* 3 wait states guaranteed */
	}
}

void jaguar_cpu_device::jump_cc_rn(u16 op)
{
	if (CONDITION(op & 31))
	{
		const u8 reg = (op >> 5) & 31;

		// HACK: kludge for risky code in the cojag DSP interrupt handlers
		const u32 newpc = (m_icount == m_bankswitch_icount) ? m_a[reg] : m_r[reg];
		debugger_instruction_hook(m_pc);
		op = ROPCODE(m_pc);
		m_pc = newpc;
		(this->*m_table[op >> 10])(op);

		m_icount -= 3;    /* 3 wait states guaranteed */
	}
}

void jaguar_cpu_device::load_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(r1);
}

void jaguar_cpu_device::load_r14n_rn(u16 op)
{
	const u32 r1 = convert_zero[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[14] + 4 * r1);
}

void jaguar_cpu_device::load_r15n_rn(u16 op)
{
	const u32 r1 = convert_zero[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[15] + 4 * r1);
}

void jaguar_cpu_device::load_r14rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[14] + r1);
}

void jaguar_cpu_device::load_r15rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = READLONG(m_r[15] + r1);
}

void jaguar_cpu_device::loadb_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_r[op & 31] = READBYTE(r1);
	}
}

void jaguar_cpu_device::loadw_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_r[op & 31] = READWORD(r1);
	}
}

void jaguar_cpu_device::loadp_rn_rn(u16 op)   /* GPU only */
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		m_r[op & 31] = READLONG(r1 & ~3);
	}
	else
	{
		m_hidata = READLONG(r1);
		m_r[op & 31] = READLONG(r1+4);
	}
}

void jaguar_cpu_device::mirror_rn(u16 op) /* DSP only */
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[dreg];
	const u32 res = (mirror_table[r1 & 0xffff] << 16) | mirror_table[r1 >> 16];
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::mmult_rn_rn(u16 op)
{
	const u8 count = m_mwidth;
	const u8 sreg = (op >> 5) & 31;
	const u8 dreg = op & 31;
	u32 addr = m_mtxaddr;
	s64 accum = 0;

	if (m_maddw == false)
	{
		for (int i = 0; i < count; i++)
		{
			accum += (int16_t)(m_b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (int16_t)READWORD(addr);
			addr += 2;
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			accum += (int16_t)(m_b1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (int16_t)READWORD(addr);
			addr += 2 * count;
		}
	}
	const u32 res = (u32)accum;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::move_rn_rn(u16 op)
{
	m_r[op & 31] = m_r[(op >> 5) & 31];
}

void jaguar_cpu_device::move_pc_rn(u16 op)
{
	m_r[op & 31] = m_ppc;
}

void jaguar_cpu_device::movefa_rn_rn(u16 op)
{
	m_r[op & 31] = m_a[(op >> 5) & 31];
}

void jaguar_cpu_device::movei_n_rn(u16 op)
{
	const u32 res = ROPCODE(m_pc) | (ROPCODE(m_pc + 2) << 16);
	m_pc += 4;
	m_r[op & 31] = res;
}

void jaguar_cpu_device::moveq_n_rn(u16 op)
{
	m_r[op & 31] = (op >> 5) & 31;
}

void jaguar_cpu_device::moveta_rn_rn(u16 op)
{
	m_a[op & 31] = m_r[(op >> 5) & 31];
}

void jaguar_cpu_device::mtoi_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	m_r[op & 31] = (((s32)r1 >> 8) & 0xff800000) | (r1 & 0x007fffff);
}

void jaguar_cpu_device::mult_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = (u16)r1 * (u16)r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::neg_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r2 = m_r[dreg];
	const u32 res = -r2;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(0, r2, res);
}

void jaguar_cpu_device::nop(u16 op)
{
}

void jaguar_cpu_device::normi_rn_rn(u16 op)
{
	u32 r1 = m_r[(op >> 5) & 31];
	u32 res = 0;
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

void jaguar_cpu_device::not_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 res = ~m_r[dreg];
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::or_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r1 | r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::pack_rn(u16 op)       /* GPU only */
{
	const u8 dreg = op & 31;
	int pack = (op >> 5) & 31;
	const u32 r2 = m_r[dreg];
	u32 res;
	if (pack == 0)    /* PACK */
		res = ((r2 >> 10) & 0xf000) | ((r2 >> 5) & 0x0f00) | (r2 & 0xff);
	else            /* UNPACK */
		res = ((r2 & 0xf000) << 10) | ((r2 & 0x0f00) << 5) | (r2 & 0xff);
	m_r[dreg] = res;
}

void jaguar_cpu_device::resmac_rn(u16 op)
{
	m_r[op & 31] = (u32)m_accum;
}

void jaguar_cpu_device::ror_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31] & 31;
	const u32 r2 = m_r[dreg];
	const u32 res = rotr_32(r2, r1);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); m_flags |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::rorq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = rotr_32(r2, r1);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); m_flags |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::sat8_rn(u16 op)       /* GPU only */
{
	const u8 dreg = op & 31;
	s32 r2 = m_r[dreg];
	const u32 res = (r2 < 0) ? 0 : (r2 > 255) ? 255 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat16_rn(u16 op)      /* GPU only */
{
	const u8 dreg = op & 31;
	s32 r2 = m_r[dreg];
	const u32 res = (r2 < 0) ? 0 : (r2 > 65535) ? 65535 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat16s_rn(u16 op)     /* DSP only */
{
	const u8 dreg = op & 31;
	s32 r2 = m_r[dreg];
	const u32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat24_rn(u16 op)          /* GPU only */
{
	const u8 dreg = op & 31;
	s32 r2 = m_r[dreg];
	const u32 res = (r2 < 0) ? 0 : (r2 > 16777215) ? 16777215 : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sat32s_rn(u16 op)     /* DSP only */
{
	const u8 dreg = op & 31;
	s32 r2 = (u32)m_r[dreg];
	s32 temp = m_accum >> 32;
	const u32 res = (temp < -1) ? (s32)0x80000000 : (temp > 0) ? (s32)0x7fffffff : r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}

void jaguar_cpu_device::sh_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const s32 r1 = (s32)m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 res;

	CLR_ZNC();
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		m_flags |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? 0 : (r2 >> r1);
		m_flags |= (r2 << 1) & 2;
	}
	m_r[dreg] = res;
	SET_ZN(res);
}

void jaguar_cpu_device::sha_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const s32 r1 = (s32)m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 res;

	CLR_ZNC();
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		m_flags |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? ((s32)r2 >> 31) : ((s32)r2 >> r1);
		m_flags |= (r2 << 1) & 2;
	}
	m_r[dreg] = res;
	SET_ZN(res);
}

void jaguar_cpu_device::sharq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const s32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = (s32)r2 >> r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); m_flags |= (r2 << 1) & 2;
}

void jaguar_cpu_device::shlq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const s32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 << (32 - r1);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); m_flags |= (r2 >> 30) & 2;
}

void jaguar_cpu_device::shrq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const s32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 >> r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZN(res); m_flags |= (r2 << 1) & 2;
}

void jaguar_cpu_device::store_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(r1, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r14n(u16 op)
{
	const u32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(m_r[14] + r1 * 4, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r15n(u16 op)
{
	const u32 r1 = convert_zero[(op >> 5) & 31];
	WRITELONG(m_r[15] + r1 * 4, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r14rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(m_r[14] + r1, m_r[op & 31]);
}

void jaguar_cpu_device::store_rn_r15rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	WRITELONG(m_r[15] + r1, m_r[op & 31]);
}

void jaguar_cpu_device::storeb_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITEBYTE(r1, m_r[op & 31]);
	}
}

void jaguar_cpu_device::storew_rn_rn(u16 op)
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITEWORD(r1, m_r[op & 31]);
	}
}

void jaguar_cpu_device::storep_rn_rn(u16 op)  /* GPU only */
{
	const u32 r1 = m_r[(op >> 5) & 31];
	if (r1 >= m_internal_ram_start && r1 <= m_internal_ram_end)
	{
		WRITELONG(r1 & ~3, m_r[op & 31]);
	}
	else
	{
		WRITELONG(r1, m_hidata);
		WRITELONG(r1+4, m_r[op & 31]);
	}
}

void jaguar_cpu_device::sub_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 - r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subc_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 c = ((m_flags >> 1) & 1);
	const u32 res = r2 - r1 - c;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1 + c, res);
}

void jaguar_cpu_device::subq_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 - r1;
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subqmod_n_rn(u16 op)  /* DSP only */
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	u32 res = r2 - r1;
	res = (res & ~m_modulo) | (r2 & m_modulo);
	m_r[dreg] = res;
	CLR_ZNC(); SET_ZNC_SUB(r2, r1, res);
}

void jaguar_cpu_device::subqt_n_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = convert_zero[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r2 - r1;
	m_r[dreg] = res;
}

void jaguar_cpu_device::xor_rn_rn(u16 op)
{
	const u8 dreg = op & 31;
	const u32 r1 = m_r[(op >> 5) & 31];
	const u32 r2 = m_r[dreg];
	const u32 res = r1 ^ r2;
	m_r[dreg] = res;
	CLR_ZN(); SET_ZN(res);
}



/***************************************************************************
    I/O HANDLING
***************************************************************************/

void jaguar_cpu_device::io_common_map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(jaguar_cpu_device::flags_r), FUNC(jaguar_cpu_device::flags_w));
	map(0x04, 0x07).w(FUNC(jaguar_cpu_device::matrix_control_w));
	map(0x08, 0x0b).w(FUNC(jaguar_cpu_device::matrix_address_w));
//  map(0x0c, 0x0f) endian
	map(0x10, 0x13).rw(FUNC(jaguar_cpu_device::pc_r), FUNC(jaguar_cpu_device::pc_w));
	map(0x14, 0x17).rw(FUNC(jaguar_cpu_device::status_r), FUNC(jaguar_cpu_device::control_w));
//  map(0x18, 0x1b) implementation specific
	map(0x1c, 0x1f).rw(FUNC(jaguar_cpu_device::div_remainder_r), FUNC(jaguar_cpu_device::div_control_w));
}

// $f02100
void jaguargpu_cpu_device::io_map(address_map &map)
{
	jaguar_cpu_device::io_common_map(map);
	map(0x0c, 0x0f).w(FUNC(jaguargpu_cpu_device::endian_w));
	map(0x18, 0x1b).rw(FUNC(jaguargpu_cpu_device::hidata_r), FUNC(jaguargpu_cpu_device::hidata_w));
}

// $f0a100
void jaguardsp_cpu_device::io_map(address_map &map)
{
	jaguar_cpu_device::io_common_map(map);
	map(0x0c, 0x0f).w(FUNC(jaguardsp_cpu_device::dsp_endian_w));
	map(0x18, 0x1b).w(FUNC(jaguardsp_cpu_device::modulo_w));
	map(0x20, 0x23).r(FUNC(jaguardsp_cpu_device::high_accum_r));
}

u32 jaguar_cpu_device::flags_r()
{
	return (m_flags & 0x1c1f7) | (m_imask << 3);
}

void jaguar_cpu_device::flags_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_flags);
	if (ACCESSING_BITS_0_15)
	{
		// clear imask only on bit 3 clear (1 has no effect)
		if ((m_flags & 0x08) == 0)
			m_imask = false;

		// update int latch & mask
		m_int_mask = (m_flags >> 4) & 0x1f;
		m_int_latch &= ~((m_flags >> 9) & 0x1f);

		//for (int i = 0; i < 5; i++)
		//{
		//	if (BIT(m_flags, 9 + i))
		//		set_input_line(i, CLEAR_LINE);
		//}

		// TODO: DMAEN (bit 15)
	}

	// TODO: move to specific handler
	if (m_isdsp && ACCESSING_BITS_16_31)
	{
		m_int_mask |= (BIT(m_flags, 16) << 5);
		m_int_latch &= ~(BIT(m_flags, 17) << 5);
	}

	update_register_banks();
	check_irqs();
}

void jaguar_cpu_device::matrix_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_mtxc);
	m_mwidth = m_io_mtxc & 0xf;
	m_maddw = BIT(m_io_mtxc, 4);
}

void jaguar_cpu_device::matrix_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_mtxa);
	// matrix can be long word address only, and only read from internal RAM
	m_mtxaddr = m_internal_ram_start | (m_io_mtxa & 0xffc);
}

// TODO: ruinerp 68k reads this twice
// (which allegedly wants a tight sync ...)
uint32_t jaguar_cpu_device::pc_r(offs_t offset)
{
	return m_pc;
}

void jaguar_cpu_device::pc_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_pc);
	m_pc = m_io_pc & 0xffffff;
	// JTRM warns against changing PC while GPU/DSP is running
	// - speedst2 does it anyway on DSP side
	if (m_go == true)
		logerror("%s: inflight PC write %08x\n", this->tag(), m_pc);
}

/*
 * Data Organization Register
 * Note: The canonical way to set this up from 68k is $00070007,
 * so that Power-On endianness doesn't matter. 1=Big Endian
 * ---- -x-- Instruction endianness
 * ---- --x- Pixel endianness (GPU only)
 * ---- ---x I/O endianness
 */
// TODO: just log if anything farts for now, change to bit struct once we have something to test out
void jaguar_cpu_device::endian_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_end);
	if (ACCESSING_BITS_0_7)
	{
		// sburnout sets bit 1 == 0
		if ((m_io_end & 0x7) != 0x7)
			throw emu_fatalerror("%s: fatal endian setup %08x", this->tag(), m_io_end);
	}
}

void jaguardsp_cpu_device::dsp_endian_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_end);
	if (ACCESSING_BITS_0_7)
	{
		// wolfn3d writes a '0' to bit 1 (which is a NOP for DSP)
		// bretth sets 0x7e06 after dyna cam logo
		if ((m_io_end & 0x5) != 0x5)
			throw emu_fatalerror("%s: fatal endian setup %08x", this->tag(), m_io_end);
	}
}

/*
 * Control/Status Register
 * - xxxx ---- ---- ---- chip version number
 * - ---- x--- ---- ---- bus hog (increase self chip priority on bus)
 * y ---- -xxx xx-- ---- interrupt latch (y is DSP specific) (r/o)
 * - ---- ---- --0- ---- <unused>
 * - ---- ---- ---x x--- single step regs
 * - ---- ---- ---- -x-- GPUINT0 or DSPINT0
 * - ---- ---- ---- --x- Host interrupt (w/o)
 * - ---- ---- ---- ---x GPUGO or DSPGO flag
 *
 */
u32 jaguar_cpu_device::status_r()
{
	u32 result = ((m_version & 0xf)<<12) | (m_bus_hog<<11) | m_go;
	result |= (m_int_latch & 0x1f) << 6;
	// TODO: make it DSP specific
	if (m_isdsp == true)
		result |= (m_int_latch & 0x20) << 11;
	return result;
}

void jaguar_cpu_device::go_w(int state)
{
	m_go = state;
	set_input_line(INPUT_LINE_HALT, (m_go == true) ? CLEAR_LINE : ASSERT_LINE);
	yield();
}

void jaguar_cpu_device::control_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_io_status);
	if (ACCESSING_BITS_0_15)
	{
		bool new_go = BIT(m_io_status, 0);
		if (new_go != m_go)
			go_w(new_go);

		if (BIT(m_io_status, 1))
			m_cpu_interrupt(ASSERT_LINE);

		// TODO: following does nothing if set by itself, or acts as a trap?
		if (BIT(m_io_status, 2))
		{
			// whitemen/missil3d wants gating thru the mask, as above
			//m_int_latch |= 1;
			//check_irqs();
			set_input_line(0, ASSERT_LINE);
		}


		// TODO: single step handling

		m_bus_hog = BIT(m_io_status, 11);
		// TODO: protect/protectse uses this, why?
		if (m_bus_hog == true)
			logerror("%s: bus hog enabled\n", this->tag());
	}
}

u32 jaguargpu_cpu_device::hidata_r()
{
	return m_hidata;
}

void jaguargpu_cpu_device::hidata_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_hidata);
}

u32 jaguar_cpu_device::div_remainder_r()
{
	// TODO: truly 32-bit?
	return m_div_remainder;
}

void jaguar_cpu_device::div_control_w(u32 data)
{
	m_div_offset = BIT(data, 0);
}

void jaguardsp_cpu_device::modulo_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_modulo);
}

u32 jaguardsp_cpu_device::high_accum_r()
{
	logerror("%s: high 16-bit accumulator read\n", this->tag());
	return (m_accum >> 32) & 0xff;
}

u32 jaguar_cpu_device::iobus_r(offs_t offset, u32 mem_mask)
{
	return m_io.read_dword(offset*4, mem_mask);
}

void jaguar_cpu_device::iobus_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_io.write_dword(offset*4, data, mem_mask);
}

/***************************************************************************
    DASM
***************************************************************************/

std::unique_ptr<util::disasm_interface> jaguargpu_cpu_device::create_disassembler()
{
	return std::make_unique<jaguar_disassembler>(jaguar_disassembler::variant::GPU);
}

std::unique_ptr<util::disasm_interface> jaguardsp_cpu_device::create_disassembler()
{
	return std::make_unique<jaguar_disassembler>(jaguar_disassembler::variant::DSP);
}
