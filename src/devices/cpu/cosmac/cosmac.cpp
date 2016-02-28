// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA COSMAC CPU emulation

**********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cosmac.h"
#include "coreutil.h"

// permit our enums to be saved
ALLOW_SAVE_TYPE(cosmac_device::cosmac_mode);
ALLOW_SAVE_TYPE(cosmac_device::cosmac_state);


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CLOCKS_RESET        8
#define CLOCKS_INIT         8 // really 9, but needs to be 8 to synchronize cdp1861 video timings
#define CLOCKS_FETCH        8
#define CLOCKS_EXECUTE      8
#define CLOCKS_DMA          8
#define CLOCKS_INTERRUPT    8

const cosmac_state_code COSMAC_STATE_CODE[] =
{
	COSMAC_STATE_CODE_S0_FETCH,     // COSMAC_STATE_0_FETCH
	COSMAC_STATE_CODE_S1_EXECUTE,   // COSMAC_STATE_1_RESET
	COSMAC_STATE_CODE_S1_EXECUTE,   // COSMAC_STATE_1_INIT
	COSMAC_STATE_CODE_S1_EXECUTE,   // COSMAC_STATE_1_EXECUTE
	COSMAC_STATE_CODE_S2_DMA,       // COSMAC_STATE_2_DMA_IN
	COSMAC_STATE_CODE_S2_DMA,       // COSMAC_STATE_2_DMA_OUT
	COSMAC_STATE_CODE_S3_INTERRUPT  // COSMAC_STATE_3_INT
};



//**************************************************************************
//  MACROS
//**************************************************************************

#define OPCODE_R(addr)      read_opcode(addr)
#define RAM_R(addr)         read_byte(addr)
#define RAM_W(addr, data)   write_byte(addr, data)
#define IO_R(addr)          read_io_byte(addr)
#define IO_W(addr, data)    write_io_byte(addr, data)

#define P   m_p
#define X   m_x
#define D   m_d
#define B   m_b
#define T   m_t
#define R   m_r
#define DF  m_df
#define IE  m_ie
#define Q   m_q
#define N   m_n
#define I   m_i
#define EF  m_ef

#define GET_FLAGS()             ((m_df << 2) | (m_ie << 1) | m_q)

#define SET_FLAGS(v)            do { \
									m_df = BIT(v, 2); \
									m_ie = BIT(v, 1); \
									m_q = BIT(v, 0); \
								} while (0);



//**************************************************************************
//  STATIC OPCODE TABLES
//**************************************************************************

const cosmac_device::ophandler cdp1801_device::s_opcodetable[256] =
{
	&cdp1801_device::idl,    &cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,
	&cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,
	&cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,
	&cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,    &cdp1801_device::ldn,

	&cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,
	&cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,
	&cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,
	&cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,    &cdp1801_device::inc,

	&cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,
	&cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,
	&cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,
	&cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,    &cdp1801_device::dec,

	&cdp1801_device::br,     &cdp1801_device::und,    &cdp1801_device::bz,     &cdp1801_device::bdf,
	&cdp1801_device::b,      &cdp1801_device::b,      &cdp1801_device::b,      &cdp1801_device::b,
	&cdp1801_device::nbr,    &cdp1801_device::und,    &cdp1801_device::bnz,    &cdp1801_device::bnf,
	&cdp1801_device::bn,     &cdp1801_device::bn,     &cdp1801_device::bn,     &cdp1801_device::bn,

	&cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,
	&cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,
	&cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,
	&cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,    &cdp1801_device::lda,

	&cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,
	&cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,
	&cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,
	&cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,    &cdp1801_device::str,

	&cdp1801_device::und,    &cdp1801_device::out,    &cdp1801_device::out,    &cdp1801_device::out,
	&cdp1801_device::out,    &cdp1801_device::out,    &cdp1801_device::out,    &cdp1801_device::out,
	&cdp1801_device::und,    &cdp1801_device::inp,    &cdp1801_device::inp,    &cdp1801_device::inp,
	&cdp1801_device::inp,    &cdp1801_device::inp,    &cdp1801_device::inp,    &cdp1801_device::inp,

	&cdp1801_device::ret,    &cdp1801_device::dis,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::sav,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,

	&cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,
	&cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,
	&cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,
	&cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,    &cdp1801_device::glo,

	&cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,
	&cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,
	&cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,
	&cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,    &cdp1801_device::ghi,

	&cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,
	&cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,
	&cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,
	&cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,    &cdp1801_device::plo,

	&cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,
	&cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,
	&cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,
	&cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,    &cdp1801_device::phi,

	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,
	&cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,    &cdp1801_device::und,

	&cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,
	&cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,
	&cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,
	&cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,    &cdp1801_device::sep,

	&cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,
	&cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,
	&cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,
	&cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,    &cdp1801_device::sex,

	&cdp1801_device::ldx,    &cdp1801_device::_or,    &cdp1801_device::_and,   &cdp1801_device::_xor,
	&cdp1801_device::add,    &cdp1801_device::sd,     &cdp1801_device::shr,    &cdp1801_device::sm,
	&cdp1801_device::ldi,    &cdp1801_device::ori,    &cdp1801_device::ani,    &cdp1801_device::xri,
	&cdp1801_device::adi,    &cdp1801_device::sdi,    &cdp1801_device::und,    &cdp1801_device::smi
};

cosmac_device::ophandler cdp1801_device::get_ophandler(UINT8 opcode)
{
	return s_opcodetable[opcode];
}

const cosmac_device::ophandler cdp1802_device::s_opcodetable[256] =
{
	&cdp1802_device::idl,    &cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,
	&cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,
	&cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,
	&cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,    &cdp1802_device::ldn,

	&cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,
	&cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,
	&cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,
	&cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,    &cdp1802_device::inc,

	&cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,
	&cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,
	&cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,
	&cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,    &cdp1802_device::dec,

	&cdp1802_device::br,     &cdp1802_device::bq,     &cdp1802_device::bz,     &cdp1802_device::bdf,
	&cdp1802_device::b,      &cdp1802_device::b,      &cdp1802_device::b,      &cdp1802_device::b,
	&cdp1802_device::nbr,    &cdp1802_device::bnq,    &cdp1802_device::bnz,    &cdp1802_device::bnf,
	&cdp1802_device::bn,     &cdp1802_device::bn,     &cdp1802_device::bn,     &cdp1802_device::bn,

	&cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,
	&cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,
	&cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,
	&cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,    &cdp1802_device::lda,

	&cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,
	&cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,
	&cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,
	&cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,    &cdp1802_device::str,

	&cdp1802_device::irx,    &cdp1802_device::out,    &cdp1802_device::out,    &cdp1802_device::out,
	&cdp1802_device::out,    &cdp1802_device::out,    &cdp1802_device::out,    &cdp1802_device::out,
	&cdp1802_device::inp,    &cdp1802_device::inp,    &cdp1802_device::inp,    &cdp1802_device::inp,
	&cdp1802_device::inp,    &cdp1802_device::inp,    &cdp1802_device::inp,    &cdp1802_device::inp,

	&cdp1802_device::ret,    &cdp1802_device::dis,    &cdp1802_device::ldxa,   &cdp1802_device::stxd,
	&cdp1802_device::adc,    &cdp1802_device::sdb,    &cdp1802_device::shrc,   &cdp1802_device::smb,
	&cdp1802_device::sav,    &cdp1802_device::mark,   &cdp1802_device::req,    &cdp1802_device::seq,
	&cdp1802_device::adci,   &cdp1802_device::sdbi,   &cdp1802_device::shlc,   &cdp1802_device::smbi,

	&cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,
	&cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,
	&cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,
	&cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,    &cdp1802_device::glo,

	&cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,
	&cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,
	&cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,
	&cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,    &cdp1802_device::ghi,

	&cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,
	&cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,
	&cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,
	&cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,    &cdp1802_device::plo,

	&cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,
	&cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,
	&cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,
	&cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,    &cdp1802_device::phi,

	&cdp1802_device::lbr,    &cdp1802_device::lbq,    &cdp1802_device::lbz,    &cdp1802_device::lbdf,
	&cdp1802_device::nop,    &cdp1802_device::lsnq,   &cdp1802_device::lsnz,   &cdp1802_device::lsnf,
	&cdp1802_device::nlbr,   &cdp1802_device::lbnq,   &cdp1802_device::lbnz,   &cdp1802_device::lbnf,
	&cdp1802_device::lsie,   &cdp1802_device::lsq,    &cdp1802_device::lsz,    &cdp1802_device::lsdf,

	&cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,
	&cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,
	&cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,
	&cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,    &cdp1802_device::sep,

	&cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,
	&cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,
	&cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,
	&cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,    &cdp1802_device::sex,

	&cdp1802_device::ldx,    &cdp1802_device::_or,    &cdp1802_device::_and,   &cdp1802_device::_xor,
	&cdp1802_device::add,    &cdp1802_device::sd,     &cdp1802_device::shr,    &cdp1802_device::sm,
	&cdp1802_device::ldi,    &cdp1802_device::ori,    &cdp1802_device::ani,    &cdp1802_device::xri,
	&cdp1802_device::adi,    &cdp1802_device::sdi,    &cdp1802_device::shl,    &cdp1802_device::smi
};

cosmac_device::ophandler cdp1802_device::get_ophandler(UINT8 opcode)
{
	return s_opcodetable[opcode];
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

// device type definition
const device_type CDP1801 = &device_creator<cdp1801_device>;
const device_type CDP1802 = &device_creator<cdp1802_device>;


//-------------------------------------------------
//  cosmac_device - constructor
//-------------------------------------------------

cosmac_device::cosmac_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_LITTLE, 8, 16),
		m_io_config("io", ENDIANNESS_LITTLE, 8, 3),
		m_read_wait(*this),
		m_read_clear(*this),
		m_read_ef1(*this),
		m_read_ef2(*this),
		m_read_ef3(*this),
		m_read_ef4(*this),
		m_write_q(*this),
		m_read_dma(*this),
		m_write_dma(*this),
		m_write_sc(*this),
		m_op(0),
		m_state(COSMAC_STATE_1_RESET),
		m_mode(COSMAC_MODE_RESET),
		m_irq(CLEAR_LINE),
		m_dmain(CLEAR_LINE),
		m_dmaout(CLEAR_LINE),
		m_program(nullptr),
		m_io(nullptr),
		m_direct(nullptr)
{
	for (auto & elem : m_ef)
		elem = CLEAR_LINE;
}


//-------------------------------------------------
//  cdp1801_device - constructor
//-------------------------------------------------

cdp1801_device::cdp1801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cosmac_device(mconfig, CDP1801, "CDP1801", tag, owner, clock, "cdp1801", __FILE__)
{
}


//-------------------------------------------------
//  cdp1802_device - constructor
//-------------------------------------------------

cdp1802_device::cdp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cosmac_device(mconfig, CDP1802, "CDP1802", tag, owner, clock, "cdp1802", __FILE__)
{
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void cosmac_device::device_start()
{
	// resolve callbacks
	m_read_wait.resolve_safe(0);
	m_read_clear.resolve_safe(0);
	m_read_ef1.resolve();
	m_read_ef2.resolve();
	m_read_ef3.resolve();
	m_read_ef4.resolve();
	m_write_q.resolve_safe();
	m_read_dma.resolve_safe(0);
	m_write_dma.resolve_safe();
	m_write_sc.resolve_safe();

	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",        m_pc).callimport().callexport().noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",     m_flagsio).mask(0x7).callimport().callexport().noshow().formatstr("%3s");

	state_add(COSMAC_P,     "P",    m_p).mask(0xf);
	state_add(COSMAC_X,     "X",    m_x).mask(0xf);
	state_add(COSMAC_D,     "D",    m_d);
	state_add(COSMAC_B,     "B",    m_b);
	state_add(COSMAC_T,     "T",    m_t);

	state_add(COSMAC_I,     "I",    m_i).mask(0xf);
	state_add(COSMAC_N,     "N",    m_n).mask(0xf);

	for (int regnum = 0; regnum < 16; regnum++)
		state_add(COSMAC_R0 + regnum, string_format("R%x", regnum).c_str(), m_r[regnum]);

	state_add(COSMAC_DF,    "DF",   m_df).mask(0x1).noshow();
	state_add(COSMAC_IE,    "IE",   m_ie).mask(0x1).noshow();
	state_add(COSMAC_Q,     "Q",    m_q).mask(0x1).noshow();

	// register our state for saving
	save_item(NAME(m_op));
	save_item(NAME(m_flagsio));
	save_item(NAME(m_state));
	save_item(NAME(m_mode));
	save_item(NAME(m_pmode));
	save_item(NAME(m_irq));
	save_item(NAME(m_dmain));
	save_item(NAME(m_dmaout));
	save_item(NAME(m_ef));
	save_item(NAME(m_d));
	save_item(NAME(m_b));
	save_item(NAME(m_r));
	save_item(NAME(m_p));
	save_item(NAME(m_x));
	save_item(NAME(m_n));
	save_item(NAME(m_i));
	save_item(NAME(m_t));
	save_item(NAME(m_df));
	save_item(NAME(m_ie));
	save_item(NAME(m_q));

	// set our instruction counter
	m_icountptr = &m_icount;
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void cosmac_device::device_reset()
{
	m_ie = 0;
	m_q = 0;
	m_df = 0;
	m_p = 0;
	rand_memory(m_r, sizeof(m_r));
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *cosmac_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
	case AS_PROGRAM:
		return &m_program_config;

	case AS_IO:
		return &m_io_config;

	default:
		return nullptr;
	}
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void cosmac_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			R[P] = m_pc;
			break;

		case STATE_GENFLAGS:
			SET_FLAGS(m_flagsio);
			break;
	}
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void cosmac_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			m_pc = R[P];
			break;

		case STATE_GENFLAGS:
			m_flagsio = GET_FLAGS();
			break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void cosmac_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c",
					m_df ? 'D' : '.',
					m_ie ? 'I' : '.',
					m_q  ? 'Q' : '.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 cosmac_device::disasm_min_opcode_bytes() const
{
	return 1;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 cosmac_device::disasm_max_opcode_bytes() const
{
	return 3;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t cdp1801_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( cdp1801 );
	return CPU_DISASSEMBLE_NAME( cdp1801 )(this, buffer, pc, oprom, opram, options);
}

offs_t cdp1802_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( cdp1802 );
	return CPU_DISASSEMBLE_NAME( cdp1802 )(this, buffer, pc, oprom, opram, options);
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_opcode - read an opcode at the given address
//-------------------------------------------------

inline UINT8 cosmac_device::read_opcode(offs_t pc)
{
	return m_direct->read_byte(pc);
}


//-------------------------------------------------
//  read_byte - read a byte at the given address
//-------------------------------------------------

inline UINT8 cosmac_device::read_byte(offs_t address)
{
	return m_program->read_byte(address);
}


//-------------------------------------------------
//  read_io_byte - read an I/O byte at the given
//  address
//-------------------------------------------------

inline UINT8 cosmac_device::read_io_byte(offs_t address)
{
	return m_io->read_byte(address);
}


//-------------------------------------------------
//  write_byte - write a byte at the given address
//-------------------------------------------------

inline void cosmac_device::write_byte(offs_t address, UINT8 data)
{
	m_program->write_byte(address, data);
}


//-------------------------------------------------
//  write_io_byte - write an I/O byte at the given
//  address
//-------------------------------------------------

inline void cosmac_device::write_io_byte(offs_t address, UINT8 data)
{
	m_io->write_byte(address, data);
}



//**************************************************************************
//  CORE EXECUTION
//**************************************************************************

//-------------------------------------------------
//  get_memory_address - get current memory address
//-------------------------------------------------

offs_t cosmac_device::get_memory_address()
{
	// this is valid for INP/OUT opcodes
	return R[X];
}

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 cosmac_device::execute_min_cycles() const
{
	return 8 * 2;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 cosmac_device::execute_max_cycles() const
{
	return 8 * 3;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 cosmac_device::execute_input_lines() const
{
	return 7;
}


//-------------------------------------------------
//  execute_set_input -
//-------------------------------------------------

void cosmac_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case COSMAC_INPUT_LINE_INT:
		m_irq = state;
		break;

	case COSMAC_INPUT_LINE_DMAIN:
		m_dmain = state;
		break;

	case COSMAC_INPUT_LINE_DMAOUT:
		m_dmaout = state;
		break;

	case COSMAC_INPUT_LINE_EF1:
	case COSMAC_INPUT_LINE_EF2:
	case COSMAC_INPUT_LINE_EF3:
	case COSMAC_INPUT_LINE_EF4:
		EF[inputnum - COSMAC_INPUT_LINE_EF1] = state;
		break;
	}
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void cosmac_device::execute_run()
{
	do
	{
		sample_wait_clear();

		switch (m_mode)
		{
		case COSMAC_MODE_LOAD:
			if (m_pmode == COSMAC_MODE_RESET)
			{
				m_pmode = COSMAC_MODE_LOAD;

				// execute initialization cycle
				m_state = COSMAC_STATE_1_INIT;
				run();

				// next state is IDLE
				m_state = COSMAC_STATE_1_EXECUTE;
			}
			else
			{
				// idle
				m_op = 0;
				I = 0;
				N = 0;
				run();
			}
			break;

		case COSMAC_MODE_RESET:
			m_state = COSMAC_STATE_1_RESET;
			run();
			break;

		case COSMAC_MODE_PAUSE:
			m_icount--;
			break;

		case COSMAC_MODE_RUN:
			switch (m_pmode)
			{
			case COSMAC_MODE_LOAD:
				// RUN mode cannot be initiated from LOAD mode
				logerror("COSMAC '%s' Tried to initiate RUN mode from LOAD mode\n", tag());
				m_mode = COSMAC_MODE_LOAD;
				break;

			case COSMAC_MODE_RESET:
				m_pmode = COSMAC_MODE_RUN;
				m_state = COSMAC_STATE_1_INIT;
				run();
				break;

			case COSMAC_MODE_PAUSE:
				m_pmode = COSMAC_MODE_RUN;
				m_state = COSMAC_STATE_0_FETCH;
				run();
				break;

			case COSMAC_MODE_RUN:
				run();
				break;
			}
			break;
		}
	}
	while (m_icount > 0);
}


//-------------------------------------------------
//  run - run the CPU state machine
//-------------------------------------------------

inline void cosmac_device::run()
{
	output_state_code();

	switch (m_state)
	{
	case COSMAC_STATE_0_FETCH:
		fetch_instruction();
		break;

	case COSMAC_STATE_1_RESET:
		reset();
		debug();
		break;

	case COSMAC_STATE_1_INIT:
		initialize();
		debug();
		break;

	case COSMAC_STATE_1_EXECUTE:
		sample_ef_lines();
		execute_instruction();
		debug();
		break;

	case COSMAC_STATE_2_DMA_IN:
		dma_input();
		break;

	case COSMAC_STATE_2_DMA_OUT:
		dma_output();
		break;

	case COSMAC_STATE_3_INT:
		interrupt();
		debug();
		break;
	}
}


//-------------------------------------------------
//  debug - hook into debugger
//-------------------------------------------------

inline void cosmac_device::debug()
{
	if (device_t::machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debugger_instruction_hook(this, R[P]);
	}
}


//-------------------------------------------------
//  sample_wait_clear - sample wait/clear lines
//-------------------------------------------------

inline void cosmac_device::sample_wait_clear()
{
	int wait = m_read_wait();
	int clear = m_read_clear();

	m_pmode = m_mode;
	m_mode = (cosmac_mode) ((clear << 1) | wait);
}


//-------------------------------------------------
//  sample_ef_lines - sample EF input lines
//-------------------------------------------------

inline void cosmac_device::sample_ef_lines()
{
	if (!m_read_ef1.isnull()) EF[0] = m_read_ef1();
	if (!m_read_ef2.isnull()) EF[1] = m_read_ef2();
	if (!m_read_ef3.isnull()) EF[2] = m_read_ef3();
	if (!m_read_ef4.isnull()) EF[3] = m_read_ef4();
}


//-------------------------------------------------
//  output_state_code - output state code
//-------------------------------------------------

inline void cosmac_device::output_state_code()
{
	m_write_sc((offs_t)0, COSMAC_STATE_CODE[m_state]);
}


//-------------------------------------------------
//  set_q_flag - set Q flag state and output it
//-------------------------------------------------

inline void cosmac_device::set_q_flag(int state)
{
	Q = state;

	m_write_q(Q);
}


//-------------------------------------------------
//  fetch_instruction - fetch instruction from
//  the program memory
//-------------------------------------------------

inline void cosmac_device::fetch_instruction()
{
	// instruction fetch
	m_op = read_opcode(R[P]);
	R[P]++;

	I = m_op >> 4;
	N = m_op & 0x0f;

	m_icount -= CLOCKS_FETCH;

	m_state = COSMAC_STATE_1_EXECUTE;
}


//-------------------------------------------------
//  reset - handle reset state
//-------------------------------------------------

inline void cosmac_device::reset()
{
	m_op = 0;
	I = 0;
	N = 0;
	Q = 0;
	IE = 1;

	m_icount -= CLOCKS_RESET;
}


//-------------------------------------------------
//  initialize - handle initialization state
//-------------------------------------------------

inline void cosmac_device::initialize()
{
	X = 0;
	P = 0;
	R[0] = 0;

	m_icount -= CLOCKS_INIT;

	if (m_dmain)
	{
		m_state = COSMAC_STATE_2_DMA_IN;
	}
	else if (m_dmaout)
	{
		m_state = COSMAC_STATE_2_DMA_OUT;
	}
	else
	{
		m_state = COSMAC_STATE_0_FETCH;
	}
}


//-------------------------------------------------
//  execute_instruction - execute instruction
//-------------------------------------------------

inline void cosmac_device::execute_instruction()
{
	// parse the instruction
	(this->*this->get_ophandler(m_op))();

	m_icount -= CLOCKS_EXECUTE;

	if (m_dmain)
	{
		m_state = COSMAC_STATE_2_DMA_IN;
	}
	else if (m_dmaout)
	{
		m_state = COSMAC_STATE_2_DMA_OUT;
	}
	else if (IE && m_irq)
	{
		m_state = COSMAC_STATE_3_INT;
	}
	else if ((I > 0) || (N > 0)) // not idling
	{
		m_state = COSMAC_STATE_0_FETCH;
	}
}


//-------------------------------------------------
//  dma_input - handle DMA input state
//-------------------------------------------------

inline void cosmac_device::dma_input()
{
	RAM_W(R[0], m_read_dma(R[0]));

	R[0]++;

	m_icount -= CLOCKS_DMA;

	if (m_dmain)
	{
		m_state = COSMAC_STATE_2_DMA_IN;
	}
	else if (m_dmaout)
	{
		m_state = COSMAC_STATE_2_DMA_OUT;
	}
	else if (IE && m_irq)
	{
		m_state = COSMAC_STATE_3_INT;
	}
	else if (m_mode == COSMAC_MODE_LOAD)
	{
		m_state = COSMAC_STATE_1_EXECUTE;
	}
	else
	{
		m_state = COSMAC_STATE_0_FETCH;
	}

	standard_irq_callback(COSMAC_INPUT_LINE_DMAIN);
}


//-------------------------------------------------
//  dma_output - handle DMA output state
//-------------------------------------------------

inline void cosmac_device::dma_output()
{
	m_write_dma((offs_t)R[0], RAM_R(R[0]));

	R[0]++;

	m_icount -= CLOCKS_DMA;

	if (m_dmain)
	{
		m_state = COSMAC_STATE_2_DMA_IN;
	}
	else if (m_dmaout)
	{
		m_state = COSMAC_STATE_2_DMA_OUT;
	}
	else if (IE && m_irq)
	{
		m_state = COSMAC_STATE_3_INT;
	}
	else
	{
		m_state = COSMAC_STATE_0_FETCH;
	}

	standard_irq_callback(COSMAC_INPUT_LINE_DMAOUT);
}


//-------------------------------------------------
//  interrupt - handle interrupt state
//-------------------------------------------------

inline void cosmac_device::interrupt()
{
	T = (X << 4) | P;
	X = 2;
	P = 1;
	IE = 0;

	m_icount -= CLOCKS_INTERRUPT;

	if (m_dmain)
	{
		m_state = COSMAC_STATE_2_DMA_IN;
	}
	else if (m_dmaout)
	{
		m_state = COSMAC_STATE_2_DMA_OUT;
	}
	else
	{
		m_state = COSMAC_STATE_0_FETCH;
	}

	standard_irq_callback(COSMAC_INPUT_LINE_INT);
}



//**************************************************************************
//  OPCODE IMPLEMENTATIONS
//**************************************************************************

// memory reference opcode handlers
void cosmac_device::ldn()   { D = RAM_R(R[N]); }
void cosmac_device::lda()   { D = RAM_R(R[N]); R[N]++; }
void cosmac_device::ldx()   { D = RAM_R(R[X]); }
void cosmac_device::ldxa()  { D = RAM_R(R[X]); R[X]++; }
void cosmac_device::ldi()   { D = RAM_R(R[P]); R[P]++; }
void cosmac_device::str()   { RAM_W(R[N], D); }
void cosmac_device::stxd()  { RAM_W(R[X], D); R[X]--; }

// register operations opcode handlers
void cosmac_device::inc()   { R[N]++; }
void cosmac_device::dec()   { R[N]--; }
void cosmac_device::irx()   { R[X]++; }
void cosmac_device::glo()   { D = R[N] & 0xff; }
void cosmac_device::plo()   { R[N] = (R[N] & 0xff00) | D; }
void cosmac_device::ghi()   { D = R[N] >> 8; }
void cosmac_device::phi()   { R[N] = (D << 8) | (R[N] & 0xff); }

// logic operations opcode handlers
void cosmac_device::_or()   { D = RAM_R(R[X]) | D; }
void cosmac_device::ori()   { D = RAM_R(R[P]) | D; R[P]++; }
void cosmac_device::_xor()  { D = RAM_R(R[X]) ^ D; }
void cosmac_device::xri()   { D = RAM_R(R[P]) ^ D; R[P]++; }
void cosmac_device::_and()  { D = RAM_R(R[X]) & D; }
void cosmac_device::ani()   { D = RAM_R(R[P]) & D; R[P]++; }
void cosmac_device::shr()   { DF = BIT(D, 0); D >>= 1; }
void cosmac_device::shrc()  { int b = DF; DF = BIT(D, 0); D >>= 1; D |= b << 7; }
void cosmac_device::shl()   { DF = BIT(D, 7); D <<= 1; }
void cosmac_device::shlc()  { int b = DF; DF = BIT(D, 7); D <<= 1; D |= b; }

// arithmetic operations opcode handlers
void cosmac_device::add(int left, int right)
{
	int result = left + right;

	D = result & 0xff;
	DF = result > 0xff;
}

void cosmac_device::add_with_carry(int left, int right)
{
	int result = left + right + DF;

	D = result & 0xff;
	DF = result > 0xff;
}

void cosmac_device::subtract(int left, int right)
{
	int result = left + (right ^ 0xff) + 1;

	D = result & 0xff;
	DF = result > 0xff;
}

void cosmac_device::subtract_with_borrow(int left, int right)
{
	int result = left + (right ^ 0xff) + DF;

	D = result & 0xff;
	DF = result > 0xff;
}

void cosmac_device::add()   { add(RAM_R(R[X]), D); }
void cosmac_device::adi()   { add(RAM_R(R[P]), D); R[P]++; }
void cosmac_device::adc()   { add_with_carry(RAM_R(R[X]), D); }
void cosmac_device::adci()  { add_with_carry(RAM_R(R[P]), D); R[P]++; }
void cosmac_device::sd()    { subtract(RAM_R(R[X]), D); }
void cosmac_device::sdi()   { subtract(RAM_R(R[P]), D); R[P]++; }
void cosmac_device::sdb()   { subtract_with_borrow(RAM_R(R[X]), D); }
void cosmac_device::sdbi()  { subtract_with_borrow(RAM_R(R[P]), D); R[P]++; }
void cosmac_device::sm()    { subtract(D, RAM_R(R[X])); }
void cosmac_device::smi()   { subtract(D, RAM_R(R[P])); R[P]++; }
void cosmac_device::smb()   { subtract_with_borrow(D, RAM_R(R[X])); }
void cosmac_device::smbi()  { subtract_with_borrow(D, RAM_R(R[P])); R[P]++; }

// short branch instructions opcode handlers
void cosmac_device::short_branch(int taken)
{
	if (taken)
	{
		R[P] = (R[P] & 0xff00) | OPCODE_R(R[P]);
	}
	else
	{
		R[P]++;
	}
}

void cosmac_device::br()    { short_branch(1); }
void cosmac_device::nbr()   { short_branch(0); }
void cosmac_device::bz()    { short_branch(D == 0); }
void cosmac_device::bnz()   { short_branch(D != 0); }
void cosmac_device::bdf()   { short_branch(DF); }
void cosmac_device::bnf()   { short_branch(!DF); }
void cosmac_device::bq()    { short_branch(Q); }
void cosmac_device::bnq()   { short_branch(!Q); }
void cosmac_device::b()     { short_branch(EF[N & 0x03]); }
void cosmac_device::bn()    { short_branch(!EF[N & 0x03]); }

// long branch instructions opcode handlers
void cosmac_device::long_branch(int taken)
{
	if (taken)
	{
		// S1#1
		B = OPCODE_R(R[P]++);

		// S1#2
		R[P] = (B << 8) | OPCODE_R(R[P]);
	}
	else
	{
		// S1#1
		R[P]++;

		// S1#2
		R[P]++;
	}

	m_icount -= CLOCKS_EXECUTE;
}

void cosmac_device::lbr()   { long_branch(1); }
void cosmac_device::nlbr()  { long_skip(1); }
void cosmac_device::lbz()   { long_branch(D == 0); }
void cosmac_device::lbnz()  { long_branch(D != 0); }
void cosmac_device::lbdf()  { long_branch(DF); }
void cosmac_device::lbnf()  { long_branch(!DF); }
void cosmac_device::lbq()   { long_branch(Q); }
void cosmac_device::lbnq()  { long_branch(!Q); }

// skip instructions opcode handlers
void cosmac_device::long_skip(int taken)
{
	if (taken)
	{
		// S1#1
		R[P]++;

		// S1#2
		R[P]++;
	}

	m_icount -= CLOCKS_EXECUTE;
}

void cosmac_device::lsz()   { long_skip(D == 0); }
void cosmac_device::lsnz()  { long_skip(D != 0); }
void cosmac_device::lsdf()  { long_skip(DF); }
void cosmac_device::lsnf()  { long_skip(!DF); }
void cosmac_device::lsq()   { long_skip(Q); }
void cosmac_device::lsnq()  { long_skip(!Q); }
void cosmac_device::lsie()  { long_skip(IE); }

// control instructions opcode handlers
void cosmac_device::idl()   { /* idle */ }
void cosmac_device::nop()   { m_icount -= CLOCKS_EXECUTE; }
void cosmac_device::und()   { /* undefined opcode in CDP1801 */ m_icount -= CLOCKS_EXECUTE; }
void cosmac_device::sep()   { P = N; }
void cosmac_device::sex()   { X = N; }
void cosmac_device::seq()   { set_q_flag(1); }
void cosmac_device::req()   { set_q_flag(0); }
void cosmac_device::sav()   { RAM_W(R[X], T); }

void cosmac_device::mark()
{
	T = (X << 4) | P;
	RAM_W(R[2], T);
	X = P;
	R[2]--;
}

void cosmac_device::return_from_interrupt(int ie)
{
	UINT8 data = RAM_R(R[X]);
	R[X]++;
	P = data & 0xf;
	X = data >> 4;
	IE = ie;
}

void cosmac_device::ret()   { return_from_interrupt(1); }
void cosmac_device::dis()   { return_from_interrupt(0); }

// input/output byte transfer opcode handlers
void cosmac_device::out()   { IO_W(N, RAM_R(R[X])); R[X]++; }

/*

    A note about INP 0 (0x68) from Tom Pittman's "A Short Course in Programming":

    If you look carefully, you will notice that we never studied the opcode "68".
    That's because it is not a defined 1802 instruction. It has the form of an INP
    instruction, but 0 is not a defined input port, so if you execute it (try it!)
    nothing is input. "Nothing" is the answer to a question; it is data, and something
    will be put in the accumulator and memory (so now you know what the computer uses
    to mean "nothing").

    However, since the result of the "68" opcode is unpredictable, it should not be
    used in your programs. In fact, "68" is the first byte of a series of additional
    instructions for the 1804 and 1805 microprocessors.

    http://www.ittybittycomputers.com/IttyBitty/ShortCor.htm

*/
void cosmac_device::inp()   { D = IO_R(N & 0x07); RAM_W(R[X], D); }
