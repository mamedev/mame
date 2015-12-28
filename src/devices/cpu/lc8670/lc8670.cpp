// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sanyo LC8670 "Potato" CPU core
    by Sandro Ronco

    Based on:
    - Sega VMU hardware manual
    - Sanyo LC86104C datasheet

    TODO:
    - SIO
    - HOLD state

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "lc8670.h"

//***************************************************************************
//    DEBUGGING
//***************************************************************************

#define LOG_TIMERS          0
#define LOG_IRQ             0


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const device_type LC8670 = &device_creator<lc8670_cpu_device>;


//**************************************************************************
//  MACROS
//**************************************************************************

// registers
#define     REG_A       m_sfr[0x00]
#define     REG_PSW     m_sfr[0x01]
#define     REG_B       m_sfr[0x02]
#define     REG_C       m_sfr[0x03]
#define     REG_TRL     m_sfr[0x04]
#define     REG_TRH     m_sfr[0x05]
#define     REG_SP      m_sfr[0x06]
#define     REG_PCON    m_sfr[0x07]
#define     REG_IE      m_sfr[0x08]
#define     REG_IP      m_sfr[0x09]
#define     REG_EXT     m_sfr[0x0d]
#define     REG_OCR     m_sfr[0x0e]
#define     REG_T0CNT   m_sfr[0x10]
#define     REG_T0PRR   m_sfr[0x11]
#define     REG_T0LR    m_sfr[0x13]
#define     REG_T0HR    m_sfr[0x15]
#define     REG_T1CNT   m_sfr[0x18]
#define     REG_T1LC    m_sfr[0x1a]
#define     REG_T1LR    m_sfr[0x1b]
#define     REG_T1HC    m_sfr[0x1c]
#define     REG_T1HR    m_sfr[0x1d]
#define     REG_MCR     m_sfr[0x20]
#define     REG_STAD    m_sfr[0x22]
#define     REG_CNR     m_sfr[0x23]
#define     REG_TDR     m_sfr[0x24]
#define     REG_XBNK    m_sfr[0x25]
#define     REG_VCCR    m_sfr[0x27]
#define     REG_SCON0   m_sfr[0x30]
#define     REG_SBUF0   m_sfr[0x31]
#define     REG_SBR     m_sfr[0x32]
#define     REG_SCON1   m_sfr[0x34]
#define     REG_SBUF1   m_sfr[0x35]
#define     REG_P1      m_sfr[0x44]
#define     REG_P1DDR   m_sfr[0x45]
#define     REG_P1FCR   m_sfr[0x46]
#define     REG_P3      m_sfr[0x4c]
#define     REG_P3DDR   m_sfr[0x4d]
#define     REG_P3INT   m_sfr[0x4e]
#define     REG_FPR     m_sfr[0x54]
#define     REG_I01CR   m_sfr[0x5d]
#define     REG_I23CR   m_sfr[0x5e]
#define     REG_ISL     m_sfr[0x5f]
#define     REG_VSEL    m_sfr[0x63]
#define     REG_VRMAD1  m_sfr[0x64]
#define     REG_VRMAD2  m_sfr[0x65]
#define     REG_BTCR    m_sfr[0x7f]

// addressing modes
#define     GET_D9      (((m_op & 0x01)<<8) | fetch())
#define     GET_D9B3    (((m_op & 0x10)<<4) | fetch())
#define     GET_I8      fetch()
#define     GET_R8      fetch()
#define     GET_RI      (m_op & 0x03)
#define     GET_B3      (m_op & 0x07)
#define     GET_A12     (((m_op & 0x10)<<7) | ((m_op & 0x07)<<8) | fetch())
#define     SIGNED(v)   ((v) - (BIT(v,7) ? 0x100 : 0))

// flags
#define     FLAG_CY     0x80
#define     FLAG_AC     0x40
#define     FLAG_OV     0x04
#define     FLAG_P      0x01
#define     GET_CY      BIT(REG_PSW,7)
#define     GET_AC      BIT(REG_PSW,6)
#define     GET_OV      BIT(REG_PSW,2)
#define     GET_P       BIT(REG_PSW,0)
#define     SET_CY(v)   do { if (v) REG_PSW |= FLAG_CY; else REG_PSW &= ~FLAG_CY; } while(0)
#define     SET_AC(v)   do { if (v) REG_PSW |= FLAG_AC; else REG_PSW &= ~FLAG_AC; } while(0)
#define     SET_OV(v)   do { if (v) REG_PSW |= FLAG_OV; else REG_PSW &= ~FLAG_OV; } while(0)
#define     CHECK_P()   check_p_flag()

// CPU state
#define     HALT_MODE   0x01
#define     HOLD_MODE   0x02


//**************************************************************************
//  Opcodes Table
//**************************************************************************

const lc8670_cpu_device::op_handler lc8670_cpu_device::s_opcode_table[] =
{
	&lc8670_cpu_device::op_nop  , &lc8670_cpu_device::op_br  , &lc8670_cpu_device::op_ld  , &lc8670_cpu_device::op_ld  , &lc8670_cpu_device::op_call,   // 0x0*
	&lc8670_cpu_device::op_callr, &lc8670_cpu_device::op_brf , &lc8670_cpu_device::op_st  , &lc8670_cpu_device::op_st  , &lc8670_cpu_device::op_call,   // 0x1*
	&lc8670_cpu_device::op_callf, &lc8670_cpu_device::op_jmpf, &lc8670_cpu_device::op_mov , &lc8670_cpu_device::op_mov , &lc8670_cpu_device::op_jmp,    // 0x2*
	&lc8670_cpu_device::op_mul  , &lc8670_cpu_device::op_be  , &lc8670_cpu_device::op_be  , &lc8670_cpu_device::op_be_ri, &lc8670_cpu_device::op_jmp,   // 0x3*
	&lc8670_cpu_device::op_div  , &lc8670_cpu_device::op_bne , &lc8670_cpu_device::op_bne , &lc8670_cpu_device::op_bne_ri, &lc8670_cpu_device::op_bpc,  // 0x4*
	&lc8670_cpu_device::op_ldf  , &lc8670_cpu_device::op_stf , &lc8670_cpu_device::op_dbnz, &lc8670_cpu_device::op_dbnz, &lc8670_cpu_device::op_bpc,    // 0x5*
	&lc8670_cpu_device::op_push , &lc8670_cpu_device::op_push, &lc8670_cpu_device::op_inc , &lc8670_cpu_device::op_inc , &lc8670_cpu_device::op_bp,     // 0x6*
	&lc8670_cpu_device::op_pop  , &lc8670_cpu_device::op_pop , &lc8670_cpu_device::op_dec , &lc8670_cpu_device::op_dec , &lc8670_cpu_device::op_bp,     // 0x7*
	&lc8670_cpu_device::op_bz   , &lc8670_cpu_device::op_add , &lc8670_cpu_device::op_add , &lc8670_cpu_device::op_add , &lc8670_cpu_device::op_bn,     // 0x8*
	&lc8670_cpu_device::op_bnz  , &lc8670_cpu_device::op_addc, &lc8670_cpu_device::op_addc, &lc8670_cpu_device::op_addc, &lc8670_cpu_device::op_bn,     // 0x9*
	&lc8670_cpu_device::op_ret  , &lc8670_cpu_device::op_sub , &lc8670_cpu_device::op_sub , &lc8670_cpu_device::op_sub , &lc8670_cpu_device::op_not1,   // 0xa*
	&lc8670_cpu_device::op_reti , &lc8670_cpu_device::op_subc, &lc8670_cpu_device::op_subc, &lc8670_cpu_device::op_subc, &lc8670_cpu_device::op_not1,   // 0xb*
	&lc8670_cpu_device::op_ror  , &lc8670_cpu_device::op_ldc , &lc8670_cpu_device::op_xch , &lc8670_cpu_device::op_xch , &lc8670_cpu_device::op_clr1,   // 0xc*
	&lc8670_cpu_device::op_rorc , &lc8670_cpu_device::op_or  , &lc8670_cpu_device::op_or  , &lc8670_cpu_device::op_or  , &lc8670_cpu_device::op_clr1,   // 0xd*
	&lc8670_cpu_device::op_rol  , &lc8670_cpu_device::op_and , &lc8670_cpu_device::op_and , &lc8670_cpu_device::op_and , &lc8670_cpu_device::op_set1,   // 0xe*
	&lc8670_cpu_device::op_rolc , &lc8670_cpu_device::op_xor , &lc8670_cpu_device::op_xor , &lc8670_cpu_device::op_xor , &lc8670_cpu_device::op_set1,   // 0xf*
};


//**************************************************************************
//  IRQ vectors
//**************************************************************************

const UINT16 lc8670_cpu_device::s_irq_vectors[] =
{
	0x0000, 0x0003, 0x000b, 0x0013, 0x001b, 0x0023, 0x002b, 0x0033,
	0x003b, 0x0043, 0x004b, 0x004f, 0x0052, 0x0055, 0x005a, 0x005d
};


//**************************************************************************
//  Internal memory map
//**************************************************************************

static ADDRESS_MAP_START( lc8670_internal_map, AS_DATA, 8, lc8670_cpu_device )
	AM_RANGE(0x000, 0x0ff) AM_READWRITE(mram_r, mram_w)
	AM_RANGE(0x100, 0x17f) AM_READWRITE(regs_r, regs_w)
	AM_RANGE(0x180, 0x1ff) AM_READWRITE(xram_r, xram_w)
ADDRESS_MAP_END


//**************************************************************************
//  LC8670 DEVICE
//**************************************************************************

//-------------------------------------------------
//  lc8670_cpu_device - constructor
//-------------------------------------------------

lc8670_cpu_device::lc8670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, LC8670, "Sanyo LC8670", tag, owner, clock, "lc8670", __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 8, 16, 0),
		m_data_config("data", ENDIANNESS_BIG, 8, 9, 0, ADDRESS_MAP_NAME(lc8670_internal_map)),
		m_io_config("io", ENDIANNESS_BIG, 8, 8, 0),
		m_pc(0),
		m_ppc(0),
		m_bankswitch_func(*this)
{
	memset(m_sfr, 0x00, sizeof(m_sfr));
	memset(m_timer0, 0x00, sizeof(m_timer0));
	memset(m_timer1, 0x00, sizeof(m_timer1));
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void lc8670_cpu_device::device_start()
{
	// find address spaces
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);
	m_direct = &m_program->direct();

	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_bankswitch_func.resolve();

	// setup timers
	m_basetimer = timer_alloc(BASE_TIMER);
	m_basetimer->adjust(attotime::from_hz(m_clocks[LC8670_SUB_CLOCK]), 0, attotime::from_hz(m_clocks[LC8670_SUB_CLOCK]));
	m_clocktimer = timer_alloc(CLOCK_TIMER);

	// register state for debugger
	state_add(LC8670_PC  , "PC"  , m_pc).callimport().callexport().formatstr("%04X");
	state_add(LC8670_SFR + 0x00, "A"     , REG_A     ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x02, "B"     , REG_B     ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x03, "C"     , REG_C     ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x06, "SP"    , REG_SP    ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x01, "PSW"   , REG_PSW   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x04, "TRL"   , REG_TRL   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x05, "TRH"   , REG_TRH   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x07, "PCON"  , REG_PCON  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x08, "IE"    , REG_IE    ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x09, "IP"    , REG_IP    ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x0d, "EXT"   , REG_EXT   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x0e, "OCR"   , REG_OCR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x10, "T0CNT" , REG_T0CNT ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x11, "T0PRR" , REG_T0PRR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x12, "T0L"   , m_timer0[0]).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x13, "T0LR"  , REG_T0LR  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x14, "T0H"   , m_timer0[1]).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x15, "T0HR"  , REG_T0HR  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x18, "T1CNT" , REG_T1CNT ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x80, "T1L"   , m_timer1[0]).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x1a, "T1LC"  , REG_T1LC  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x1b, "T1LR"  , REG_T1LR  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x81, "T1H"   , m_timer1[1]).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x1c, "T1HC"  , REG_T1HC  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x1d, "T1HR"  , REG_T1HR  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x20, "MCR"   , REG_MCR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x22, "STAD"  , REG_STAD  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x23, "CNR"   , REG_CNR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x24, "TDR"   , REG_TDR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x25, "XBNK"  , REG_XBNK  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x27, "VCCR"  , REG_VCCR  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x30, "SCON0" , REG_SCON0 ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x31, "SBUF0" , REG_SBUF0 ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x32, "SBR"   , REG_SBR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x34, "SCON1" , REG_SCON1 ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x35, "SBUF1" , REG_SBUF1 ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x44, "P1"    , REG_P1    ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x45, "P1DDR" , REG_P1DDR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x46, "P1FCR" , REG_P1FCR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x4c, "P3"    , REG_P3    ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x4d, "P3DDR" , REG_P3DDR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x4e, "P3INT" , REG_P3INT ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x54, "FPR"   , REG_FPR   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x5d, "I01CR" , REG_I01CR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x5e, "I23CR" , REG_I23CR ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x5f, "ISL"   , REG_ISL   ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x63, "VSEL"  , REG_VSEL  ).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x64, "VRMAD1", REG_VRMAD1).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x65, "VRMAD2", REG_VRMAD2).callimport().callexport().formatstr("%02X");
	state_add(LC8670_SFR + 0x7f, "BTCR"  , REG_BTCR  ).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "curpcbase", m_ppc).callimport().callexport().formatstr("%4X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  REG_PSW).mask(0xff).formatstr("%7s").noshow();

	// save state
	save_pointer(NAME(m_sfr), 0x80);
	save_pointer(NAME(m_mram), 0x200);
	save_pointer(NAME(m_xram), 0xc6);
	save_pointer(NAME(m_vtrbf), 0x200);
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_op));
	save_item(NAME(m_irq_flag));
	save_item(NAME(m_irq_lev));
	save_item(NAME(m_after_reti));
	save_item(NAME(m_p1_data));
	save_item(NAME(m_timer0_prescaler));
	save_item(NAME(m_timer0));
	save_item(NAME(m_timer1));
	save_item(NAME(m_timer1_comparator));
	save_item(NAME(m_base_timer));
	save_item(NAME(m_clock_changed));
	save_item(NAME(m_input_lines));
}


//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void lc8670_cpu_device::device_reset()
{
	m_pc = s_irq_vectors[0];
	m_ppc = m_pc;
	m_op = 0;
	m_icount = 0;
	m_irq_flag = 0;
	m_irq_lev = 0;
	m_after_reti = false;
	m_p1_data = 0;
	m_timer0_prescaler = 0;
	m_timer0[0] = m_timer0[1] = 0;
	m_timer1[0] = m_timer1[1] = 0;
	m_timer1_comparator[0] = m_timer1_comparator[1] = 0;
	m_base_timer[0] = m_base_timer[1] = 0;
	m_clock_changed = false;
	memset(m_sfr, 0, 0x80);
	memset(m_mram, 0, 0x200);
	memset(m_xram, 0, 0xc6);
	memset(m_vtrbf, 0, 0x200);

	// default values from VMU hardware manual
	REG_P1FCR = 0xbf;
	REG_P3INT = 0xfd;
	REG_ISL = 0xc0;
	REG_VSEL = 0xfc;
	REG_BTCR = 0x41;

	// reset bankswitch and clock source
	m_bankswitch_func(0);
	change_clock_source();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void lc8670_cpu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case BASE_TIMER:
			if (!(REG_ISL & 0x10))
				base_timer_tick();
			break;

		case CLOCK_TIMER:
			timer0_prescaler_tick();
			timer1_tick();

			if ((REG_ISL & 0x30) == 0x10)
				base_timer_tick();
			break;
	}
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void lc8670_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			set_pc(m_pc);
			break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void lc8670_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s%s%s%s",
				GET_CY ? "CY" : "..",
				GET_AC ? "AC" : "..",
				GET_OV ? "OV" : "..",
				GET_P  ? "P"  : "."
			);
			break;
	}
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config * lc8670_cpu_device::memory_space_config(address_spacenum spacenum) const
{
	return  (spacenum == AS_PROGRAM) ? &m_program_config :
			(spacenum == AS_DATA) ? &m_data_config :
			(spacenum == AS_IO) ? &m_io_config :
			nullptr;
}

//-------------------------------------------------
//  execute - execute for the provided number of
//  countcles
//-------------------------------------------------

void lc8670_cpu_device::execute_run()
{
	if (m_clock_changed)
	{
		change_clock_source();
		return;
	}

	do
	{
		check_irqs();

		debugger_instruction_hook(this, m_pc);

		int cycles;
		m_ppc = m_pc;

		if (REG_PCON & HALT_MODE)
		{
			// in HALT state the timers are still updated
			cycles = 1;
		}
		else
		{
			// instruction fetch
			m_op = fetch();
			int op_idx = decode_op(m_op);

			// execute the instruction
			cycles = (this->*s_opcode_table[op_idx])();
		}

		// update the instruction counter
		m_icount -= cycles;
	}
	while (m_icount > 0 && !m_clock_changed);
}


//-------------------------------------------------
//  execute_set_input
//-------------------------------------------------

void lc8670_cpu_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum)
	{
		case LC8670_EXT_INT0:
			if ((REG_I01CR & 0x0c) == 0x00 && m_input_lines[inputnum] && !state)        // falling edge
			{
				REG_I01CR |= 0x02;
				if (REG_I01CR & 0x01)
					set_irq_flag(1);
			}
			else if ((REG_I01CR & 0x0c) == 0x04 && !state)      // low level
			{
				REG_I01CR |= 0x02;
				if (REG_I01CR & 0x01)
					set_irq_flag(1);
			}
			else if ((REG_I01CR & 0x0c) == 0x08 && !m_input_lines[inputnum] && state)       // rising edge
			{
				REG_I01CR |= 0x02;
				if (REG_I01CR & 0x01)
					set_irq_flag(1);
			}
			else if ((REG_I01CR & 0x0c) == 0x0c && state)       // high level
			{
				REG_I01CR |= 0x02;
				if (REG_I01CR & 0x01)
					set_irq_flag(1);
			}
			break;
		case LC8670_EXT_INT1:
			if ((REG_I01CR & 0xc0) == 0x00 && m_input_lines[inputnum] && !state)        // falling edge
			{
				REG_I01CR |= 0x20;
				if (REG_I01CR & 0x10)
					set_irq_flag(2);
			}
			else if ((REG_I01CR & 0xc0) == 0x40 && !state)      // low level
			{
				REG_I01CR |= 0x20;
				if (REG_I01CR & 0x10)
					set_irq_flag(2);
			}
			else if ((REG_I01CR & 0xc0) == 0x80 && !m_input_lines[inputnum] && state)       // rising edge
			{
				REG_I01CR |= 0x20;
				if (REG_I01CR & 0x10)
					set_irq_flag(2);
			}
			else if ((REG_I01CR & 0xc0) == 0xc0 && state)       // high level
			{
				REG_I01CR |= 0x20;
				if (REG_I01CR & 0x10)
					set_irq_flag(2);
			}
			break;
		case LC8670_EXT_INT2:
			if ((REG_I23CR & 0x04) && m_input_lines[inputnum] && !state)    // falling edge
			{
				if (!(REG_ISL & 0x01))
					timer0_tick(true);

				REG_I23CR |= 0x02;
				if (REG_I23CR & 0x01)
					set_irq_flag(3);
			}
			if ((REG_I23CR & 0x08) && !m_input_lines[inputnum] && state)    // rising edge
			{
				if (!(REG_ISL & 0x01))
					timer0_tick(true);

				REG_I23CR |= 0x02;
				if (REG_I23CR & 0x01)
					set_irq_flag(3);
			}
			break;
		case LC8670_EXT_INT3:
			if ((REG_I23CR & 0x40) && m_input_lines[inputnum] && !state)    // falling edge
			{
				if (REG_ISL & 0x01)
					timer0_tick(true);

				REG_I23CR |= 0x20;
				if (REG_I23CR & 0x10)
					set_irq_flag(4);
			}
			if ((REG_I23CR & 0x80) && !m_input_lines[inputnum] && state)    // rising edge
			{
				if (REG_ISL & 0x01)
					timer0_tick(true);

				REG_I23CR |= 0x20;
				if (REG_I23CR & 0x10)
					set_irq_flag(4);
			}
			break;
	}

	m_input_lines[inputnum] = state;
}


//-------------------------------------------------
//  screen_update - handle updating the screen
//-------------------------------------------------

UINT32 lc8670_cpu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_lcd_update_func)
		return m_lcd_update_func(*this, bitmap, cliprect, m_xram, (REG_MCR & 0x08) && (REG_VCCR & 0x80), REG_STAD);

	return 0;
}


//-------------------------------------------------
//  check_irqs - check for interrupts request
//-------------------------------------------------

void lc8670_cpu_device::check_irqs()
{
	// update P3 interrupt
	check_p3int();

	if (m_irq_flag && !m_after_reti)
	{
		int irq = 0;
		UINT8 priority = 0;

		// highest priority IRQ
		if (!(REG_IE & 0x01) && (m_irq_flag & 0x02))
		{
			irq = 0x01;
			priority = 2;
		}
		else if (!(REG_IE & 0x02) && (m_irq_flag & 0x04))
		{
			irq = 0x02;
			priority = 2;
		}

		// high priority IRQ
		else if ((REG_IE & 0x80) && ((REG_IP<<3) & m_irq_flag))
		{
			for(int i=3; i<=10; i++)
				if ((m_irq_flag & (REG_IP<<3)) & (1<<i))
				{
					irq = i;
					priority = 1;
					break;
				}
		}

		// low priority IRQ
		else if ((REG_IE & 0x80) && (m_irq_flag & 0x02))
		{
			irq = 0x01;
			priority = 0;
		}
		else if ((REG_IE & 0x80) && (m_irq_flag & 0x04))
		{
			irq = 0x02;
			priority = 0;
		}
		else if (REG_IE & 0x80)
		{
			for(int i=3; i<=10; i++)
				if (m_irq_flag & (1<<i))
				{
					irq = i;
					priority = 0;
					break;
				}
		}

		// IRQ with less priority of current interrupt are not executed until the end of the current interrupt routine
		if (irq != 0 && ((m_irq_lev & (1<<priority)) || (priority == 0 && (m_irq_lev & 0x06)) || (priority == 1 && (m_irq_lev & 0x04))))
		{
			if (LOG_IRQ)    logerror("%s: interrupt %d (Priority=%d, Level=%d) delayed\n", tag(), irq, priority, m_irq_lev);
			irq = 0;
		}

		if (irq != 0)
		{
			if (LOG_IRQ)    logerror("%s: interrupt %d (Priority=%d, Level=%d) executed\n", tag(), irq, priority, m_irq_lev);

			m_irq_lev |= (1<<priority);

			push((m_pc>>0) & 0xff);
			push((m_pc>>8) & 0xff);

			set_pc(s_irq_vectors[irq]);

			REG_PCON &= ~HALT_MODE;     // interrupts resume from HALT state

			// clear the IRQ flag
			m_irq_flag &= ~(1<<irq);

			standard_irq_callback(irq);
		}
	}

	// at least one opcode need to be executed after a RETI before another IRQ can be accepted
	m_after_reti = false;
}


//-------------------------------------------------
//  base_timer_tick - update base timer
//-------------------------------------------------

void lc8670_cpu_device::base_timer_tick()
{
	if (REG_BTCR & 0x40)
	{
		UINT16 base_counter_l = m_base_timer[0] + 1;
		UINT16 base_counter_h = m_base_timer[1];

		if (REG_BTCR & 0x80)    // 6-bit mode
			base_counter_h++;
		else if (base_counter_l & 0x100)
			base_counter_h++;

		if (base_counter_h & 0x40)
		{
			if (LOG_TIMERS) logerror("%s: base timer 0 overflow, IRQ: %d\n", tag(), BIT(REG_BTCR,0));
			REG_BTCR |= 0x02;
			if (REG_BTCR & 0x01)
				set_irq_flag(4);
		}

		bool bt1_req = false;
		switch(REG_BTCR & 0x30)
		{
			case 0x00:
				if (base_counter_l & 0x20)
					bt1_req = true;
				break;
			case 0x10:
				if (base_counter_l & 0x80)
					bt1_req = true;
				break;
			case 0x20:
				if (base_counter_h & 0x01)
					bt1_req = true;
				break;
			case 0x30:
				if (base_counter_h & 0x04)
					bt1_req = true;
				break;
		}

		if (bt1_req)
		{
			if (LOG_TIMERS) logerror("%s: base timer 1 overflow, IRQ: %d\n", tag(), BIT(REG_BTCR,3));
			REG_BTCR |= 0x08;
			if (REG_BTCR & 0x04)
				set_irq_flag(4);
		}

		if (((base_counter_l & 0x04) && (REG_ISL & 0x08)) || ((base_counter_l & 0x08) && !(REG_ISL & 0x08)))
			update_port1(m_p1_data | 0x40);
		else
			update_port1(m_p1_data & 0xbf);

		m_base_timer[0] = base_counter_l & 0xff;
		m_base_timer[1] = base_counter_h & 0x3f;
	}
}

//-------------------------------------------------
//  update_to_prescaler - update timer 0 prescaler
//-------------------------------------------------

void lc8670_cpu_device::timer0_prescaler_tick()
{
	UINT16 prescaler = m_timer0_prescaler + 1;
	if (prescaler & 0x100)
	{
		if (LOG_TIMERS) logerror("%s: timer0 prescaler overflow\n", tag());

		if ((REG_ISL & 0x30) == 0x30)
			base_timer_tick();

		timer0_tick();

		m_timer0_prescaler = REG_T0PRR;
	}
	else
	{
		m_timer0_prescaler = prescaler & 0xff;
	}
}

//-------------------------------------------------
//  timer0_tick - update timer 0
//-------------------------------------------------

void lc8670_cpu_device::timer0_tick(bool ext_line)
{
	if (REG_T0CNT & 0xc0)
	{
		if (REG_T0CNT & 0x20)
		{
			// 16-bit timer/counter mode
			if ((REG_T0CNT & 0xc0) == 0xc0 && (((REG_T0CNT & 0x10) && ext_line) || (!(REG_T0CNT & 0x10) && !ext_line)))
			{
				UINT32 timer0 = ((m_timer0[1] << 8) | m_timer0[0]) + 1;

				if (timer0 & 0x10000)
				{
					if (LOG_TIMERS) logerror("%s: timer0 long overflow, IRQ: %d\n", tag(), BIT(REG_T0CNT,3));
					m_timer0[0] = REG_T0LR;
					m_timer0[1] = REG_T0HR;
					REG_T0CNT |= 0x0a;
					if (REG_T0CNT & 0x04)
						set_irq_flag(5);
				}
				else
				{
					m_timer0[0] = (timer0>>0) & 0xff;
					m_timer0[1] = (timer0>>8) & 0xff;
				}
			}
		}
		else
		{
			// 8-bit timer/counter mode
			if ((REG_T0CNT & 0x40) && (((REG_T0CNT & 0x10) && ext_line) || (!(REG_T0CNT & 0x10) && !ext_line)))
			{
				UINT16 timer0l = m_timer0[0] + 1;

				if (timer0l & 0x100)
				{
					if (LOG_TIMERS) logerror("%s: timer0 low overflow, IRQ: %d\n", tag(), BIT(REG_T0CNT,0));
					m_timer0[0] = REG_T0LR;
					REG_T0CNT |= 0x02;
					if (REG_T0CNT & 0x01)
						set_irq_flag(3);
				}
				else
				{
					m_timer0[0]  = timer0l & 0xff;
				}
			}
			if ((REG_T0CNT & 0x80)  && !ext_line)
			{
				UINT16 timer0h = m_timer0[1] + 1;
				if (timer0h & 0x100)
				{
					if (LOG_TIMERS) logerror("%s: timer0 high overflow, IRQ: %d\n", tag(), BIT(REG_T0CNT,3));
					m_timer0[1] = REG_T0HR;
					REG_T0CNT |= 0x08;
					if (REG_T0CNT & 0x04)
						set_irq_flag(5);
				}
				else
				{
					m_timer0[1]  = timer0h & 0xff;
				}
			}
		}
	}
}

//-------------------------------------------------
//  timer1_tick - update timer 1
//-------------------------------------------------

void lc8670_cpu_device::timer1_tick()
{
	if (REG_T1CNT & 0xc0)
	{
		if (REG_T1CNT & 0x20)
		{
			if (REG_T1CNT & 0x40)
			{
				// 16-bit timer mode
				UINT16 timer1l = m_timer1[0] + (REG_T1CNT & 0x80 ? 1 : 2);
				if (timer1l & 0x100)
				{
					UINT16 timer1h = m_timer1[1] + 1;
					m_timer1[0] = REG_T1LR;
					REG_T1CNT |= 0x04;

					if (timer1h & 0x100)
					{
						if (LOG_TIMERS) logerror("%s: timer1 long overflow, IRQ: %d\n", tag(), BIT(REG_T1CNT,3));
						m_timer1[1] = REG_T1HR;
						REG_T1CNT |= 0x08;
						if (REG_T1CNT & 0x05)
							set_irq_flag(6);
					}
					else
					{
						m_timer1[1] = timer1h & 0xff;
					}
				}
				else
				{
					m_timer1[0]  = timer1l & 0xff;
				}
			}
		}
		else
		{
			// 8-bit timer/pulse generator mode
			if (REG_T1CNT & 0x40)
			{
				UINT16 timer1l = m_timer1[0] + 1;

				if (timer1l == m_timer1_comparator[0])
					update_port1(m_p1_data | 0x80);

				if (timer1l & 0x100)
				{
					if (LOG_TIMERS) logerror("%s: timer1 low overflow, IRQ: %d\n", tag(), BIT(REG_T1CNT,0));
					m_timer1[0] = REG_T1LR;
					update_port1(m_p1_data & 0x7f);
					REG_T1CNT |= 0x02;
					if (REG_T1CNT & 0x01)
						set_irq_flag(6);
				}
				else
				{
					m_timer1[0] = timer1l & 0xff;
				}
			}
			if (REG_T1CNT & 0x80)
			{
				UINT16 timer1h = m_timer1[1] + 1;

				if (timer1h & 0x100)
				{
					if (LOG_TIMERS) logerror("%s: timer1 high overflow, IRQ: %d\n", tag(), BIT(REG_T1CNT,3));
					m_timer1[1] = REG_T1HR;
					REG_T1CNT |= 0x08;
					if (REG_T1CNT & 0x04)
						set_irq_flag(6);
				}
				else
				{
					m_timer1[1] = timer1h & 0xff;
				}
			}
		}
	}
}


//**************************************************************************
//  internal map handlers
//**************************************************************************

READ8_MEMBER(lc8670_cpu_device::mram_r)
{
	return m_mram[BIT(REG_PSW,1)*0x100 + offset];
}

WRITE8_MEMBER(lc8670_cpu_device::mram_w)
{
	m_mram[BIT(REG_PSW,1)*0x100 + offset] = data;
}

READ8_MEMBER(lc8670_cpu_device::xram_r)
{
	if (!(REG_VCCR & 0x40) || space.debugger_access())  // XRAM access enabled
	{
		UINT8 * xram_bank = m_xram + (REG_XBNK & 0x03) * 0x60;

		switch(REG_XBNK & 0x03)
		{
			case 0:
			case 1:
				if ((offset & 0x0f) < 0x0c)
					return xram_bank[(offset>>4) * 0x0c + (offset & 0x0f)];
				break;
			case 2:
				if (offset < 0x06)
					return xram_bank[offset];
				break;
		}
	}

	return 0xff;
}

WRITE8_MEMBER(lc8670_cpu_device::xram_w)
{
	if (!(REG_VCCR & 0x40) || space.debugger_access())  // XRAM access enabled
	{
		UINT8 * xram_bank = m_xram + (REG_XBNK & 0x03) * 0x60;

		switch(REG_XBNK & 0x03)
		{
			case 0:
			case 1:
				if ((offset & 0x0f) < 0x0c)
					xram_bank[(offset>>4) * 0x0c + (offset & 0x0f)] = data;
				break;
			case 2:
				if (offset < 0x06)
					xram_bank[offset] = data;
				break;
		}
	}
}

READ8_MEMBER(lc8670_cpu_device::regs_r)
{
	switch(offset)
	{
		case 0x12:
			return m_timer0[0];
		case 0x14:
			return m_timer0[1];
		case 0x1b:
			return m_timer1[0];
		case 0x1d:
			return m_timer1[1];
		case 0x44:
			return (REG_P1 & REG_P1DDR) | (m_io->read_byte(LC8670_PORT1) & (REG_P1DDR ^ 0xff));
		case 0x4c:
			return (REG_P3 & REG_P3DDR) | (m_io->read_byte(LC8670_PORT3) & (REG_P3DDR ^ 0xff));
		case 0x5c:
			return m_io->read_byte(LC8670_PORT7) | 0xf0;    // 4-bit read-only port
		case 0x66:
		{
			UINT8 data = m_vtrbf[((REG_VRMAD2<<8) | REG_VRMAD1) & 0x1ff];
			if (!space.debugger_access() && (REG_VSEL & 0x10))
			{
				UINT16 vrmad = (REG_VRMAD1 | (REG_VRMAD2<<8)) + 1;
				REG_VRMAD1 = vrmad & 0xff;
				REG_VRMAD2 = (vrmad >> 8) & 0x01;
			}
			return data;
		}

		// write-only registers
		case 0x20: case 0x23: case 0x24: case 0x27:
		case 0x45: case 0x46: case 0x4d:
			if(!space.debugger_access())    logerror("%s: read write-only SFR %04x\n", machine().describe_context(), offset);
			return 0xff;
	}
	return m_sfr[offset];
}

WRITE8_MEMBER(lc8670_cpu_device::regs_w)
{
	switch(offset)
	{
		case 0x00:
			REG_A = data;
			CHECK_P();
			break;
		case 0x07:
			if (data & HOLD_MODE)
				fatalerror("%s: unemulated HOLD mode\n", machine().describe_context());
			break;
		case 0x10:
			if (!(data & 0x80))
				m_timer0[1] = REG_T0HR;
			if (!(data & 0x40))
				m_timer0[0] = REG_T0LR;
			break;
		case 0x18:
			if ((data & 0x10) && !(REG_T1CNT & 0x10))
			{
				m_timer1_comparator[0] = REG_T1LC;
				m_timer1_comparator[1] = REG_T1HC;
			}
			if (!(data & 0x80))
				m_timer1[1] = REG_T1HR;
			if (!(data & 0x40))
				m_timer1[0] = REG_T1LR;
			break;
		case 0x1a:
			if ((REG_T1CNT & 0x10) || !(REG_T1CNT & 0x40))
				m_timer1_comparator[0] = data;
			break;
		case 0x1c:
			if ((REG_T1CNT & 0x10) || !(REG_T1CNT & 0x80))
				m_timer1_comparator[1] = data;
			break;
		case 0x0e:
			if ((data ^ REG_OCR) & 0xb0)
				m_clock_changed = true;
			break;
		case 0x44:
			m_io->write_byte(LC8670_PORT1, ((data | (m_p1_data & REG_P1FCR)) & REG_P1DDR) | (m_io->read_byte(LC8670_PORT1) & (REG_P1DDR ^ 0xff)));
			break;
		case 0x4c:
			m_io->write_byte(LC8670_PORT3, (data & REG_P3DDR) | (m_io->read_byte(LC8670_PORT3) & (REG_P3DDR ^ 0xff)));
			break;
		case 0x66:
			m_vtrbf[((REG_VRMAD2<<8) | REG_VRMAD1) & 0x1ff] = data;
			if (!space.debugger_access() && (REG_VSEL & 0x10))
			{
				UINT16 vrmad = (REG_VRMAD1 | (REG_VRMAD2<<8)) + 1;
				REG_VRMAD1 = vrmad & 0xff;
				REG_VRMAD2 = (vrmad >> 8) & 0x01;
			}
			break;
		case 0x7f:
			if (!(data & 0x40))
				m_base_timer[0] = m_base_timer[1] = 0;  // stop the timer clear the counter
			break;

		// read-only registers
		case 0x12: case 0x14: case 0x5c:
			if(!space.debugger_access())    logerror("%s: write read-only SFR %04x = %02x\n", machine().describe_context(), offset, data);
			return;
	}

	m_sfr[offset] = data;
}


//**************************************************************************
//  HELPERS
//**************************************************************************

inline UINT8 lc8670_cpu_device::fetch()
{
	UINT8 data = m_direct->read_byte(m_pc);

	set_pc(m_pc + 1);

	return data;
}

inline UINT8 lc8670_cpu_device::read_data(UINT16 offset)
{
	return m_data->read_byte(offset);
}

inline void lc8670_cpu_device::write_data(UINT16 offset, UINT8 data)
{
	m_data->write_byte(offset, data);
}

inline UINT8 lc8670_cpu_device::read_data_latch(UINT16 offset)
{
	if (offset == 0x144)
		return REG_P1;
	else if (offset == 0x14c)
		return REG_P3;
	else
		return read_data(offset);
}

inline void lc8670_cpu_device::write_data_latch(UINT16 offset, UINT8 data)
{
	if (offset == 0x144)
		REG_P1 = data;
	else if (offset == 0x14c)
		REG_P3 = data;
	else
		write_data(offset, data);
}

inline void lc8670_cpu_device::update_port1(UINT8 data)
{
	m_p1_data = data;
	m_io->write_byte(LC8670_PORT1, ((REG_P1 | (m_p1_data & REG_P1FCR)) & REG_P1DDR) | (m_io->read_byte(LC8670_PORT1) & (REG_P1DDR ^ 0xff)));
}

inline void lc8670_cpu_device::set_pc(UINT16 new_pc)
{
	m_pc = new_pc;
}

inline void lc8670_cpu_device::push(UINT8 data)
{
	REG_SP++;
	m_mram[REG_SP] = data;
}

inline UINT8 lc8670_cpu_device::pop()
{
	UINT8 data =  m_mram[REG_SP];
	REG_SP--;
	return data;
}

inline UINT16 lc8670_cpu_device::get_addr()
{
	int mode = m_op & 0x0f;
	UINT16 addr;

	if (mode > 0x01 && mode <= 0x03)
		addr = GET_D9;
	else if (mode > 0x03 && mode <= 0x07)
		addr = read_data(GET_RI | ((REG_PSW>>1) & 0x0c)) | ((GET_RI & 0x02) ? 0x100 : 0x00);
	else
		fatalerror("%s: invalid get_addr in mode %x\n", machine().describe_context(), mode);

	return addr;
}

inline UINT8 lc8670_cpu_device::get_data()
{
	int mode = m_op & 0x0f;
	UINT8 data;

	if (mode == 0x01)
		data = GET_I8;
	else
		data = read_data(get_addr());

	return data;
}

inline void lc8670_cpu_device::change_clock_source()
{
	UINT32 new_clock = 0;

	switch(REG_OCR & 0x30)
	{
		case 0x00:
			new_clock = m_clocks[LC8670_RC_CLOCK];
			break;
		case 0x20:
			new_clock = m_clocks[LC8670_SUB_CLOCK];
			break;
		case 0x10:
		case 0x30:
			new_clock = m_clocks[LC8670_CF_CLOCK];
			break;
	}

	set_unscaled_clock(new_clock);
	set_clock_scale(1.0 / (REG_OCR & 0x80 ? 6.0 : 12.0));
	m_clocktimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	m_clock_changed = false;
}

inline void lc8670_cpu_device::check_p_flag()
{
	UINT8 p_plag = 0;
	for(int i=0; i<8; i++)
		p_plag ^= BIT(REG_A, i);

	if (p_plag)
		REG_PSW |= FLAG_P;
	else
		REG_PSW &= ~FLAG_P;
}

inline void lc8670_cpu_device::check_p3int()
{
	if (REG_P3INT & 0x04)
	{
		if ((m_io->read_byte(LC8670_PORT3) ^ 0xff) & (REG_P3DDR ^ 0xff) & REG_P3)
		{
			REG_P3INT |= 0x02;
			if (REG_P3INT & 0x01)
				set_irq_flag(10);
		}
	}
}

inline void lc8670_cpu_device::set_irq_flag(int source)
{
	if (LOG_IRQ)    logerror("%s: set interrupt flag: %d\n", tag(), source);
	m_irq_flag |= 1<<source;
}

int lc8670_cpu_device::decode_op(UINT8 op)
{
	int idx;
	switch (op & 0x0f)
	{
		case 0: case 1:
			idx =  op & 0x0f;
			break;
		case 2: case 3:
			idx = 2;
			break;
		case 4: case 5: case 6: case 7:
			idx = 3;
			break;
		default:
			idx = 4;
			break;
	}

	return ((op>>4) & 0x0f) * 5 + idx;
}

//**************************************************************************
//  Opcodes
//**************************************************************************

int lc8670_cpu_device::op_nop()
{
	return 1;
}

int lc8670_cpu_device::op_br()
{
	UINT8 r8 = GET_R8;
	set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_ld()
{
	REG_A = get_data();
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_call()
{
	UINT16 new_pc = GET_A12;

	push((m_pc>>0) & 0xff);
	push((m_pc>>8) & 0xff);

	set_pc((m_pc & 0xf000) | new_pc);

	return 2;
}


int lc8670_cpu_device::op_callr()
{
	UINT16 r16 = fetch();
	r16 |= fetch()<<8;

	push((m_pc>>0) & 0xff);
	push((m_pc>>8) & 0xff);
	set_pc(m_pc - 1 + r16);

	return 4;
}

int lc8670_cpu_device::op_brf()
{
	UINT16 r16 = fetch();
	r16 |= fetch()<<8;
	set_pc(m_pc - 1 + r16);

	return 4;
}

int lc8670_cpu_device::op_st()
{
	write_data(get_addr(), REG_A);

	return 1;
}

int lc8670_cpu_device::op_callf()
{
	UINT16 a16 = fetch()<<8;
	a16 |= fetch();

	push((m_pc>>0) & 0xff);
	push((m_pc>>8) & 0xff);
	set_pc(a16);

	return 2;
}

int lc8670_cpu_device::op_jmpf()
{
	UINT16 a16 = fetch()<<8;
	a16 |= fetch();
	set_pc(a16);

	m_bankswitch_func(((REG_EXT & 0x01) ? 1 : (REG_EXT & 0x08) ? 0 : 2));

	return 2;
}

int lc8670_cpu_device::op_mov()
{
	UINT16 addr = get_addr();
	UINT8 i8 = GET_I8;
	write_data(addr, i8);

	return 1;
}

int lc8670_cpu_device::op_jmp()
{
	UINT16 new_pc = GET_A12;
	set_pc((m_pc & 0xf000) | new_pc);

	return 2;
}

int lc8670_cpu_device::op_mul()
{
	UINT32 res = REG_B * ((REG_A<<8) | REG_C);

	REG_A = (res>>8) & 0xff;
	REG_B = (res>>16) & 0xff;
	REG_C = (res>>0) & 0xff;

	SET_OV(REG_B != 0 ? 1 : 0);
	SET_CY(0);
	CHECK_P();

	return 7;
}

int lc8670_cpu_device::op_be()
{
	UINT8 data = get_data();
	UINT8 r8 = GET_R8;

	if (REG_A == data)
		set_pc(m_pc + SIGNED(r8));

	SET_CY((REG_A < data) ? 1 : 0);

	return 2;
}

int lc8670_cpu_device::op_be_ri()
{
	UINT8 data = get_data();
	UINT8 i8 = GET_I8;
	UINT8 r8 = GET_R8;

	if (i8 == data)
		set_pc(m_pc + SIGNED(r8));

	SET_CY((data < i8) ? 1 : 0);

	return 2;
}


int lc8670_cpu_device::op_div()
{
	UINT32 res, mod;

	if (REG_B != 0)
	{
		UINT16 v = ((REG_A<<8) | REG_C);
		res = v / REG_B;
		mod = v % REG_B;

		REG_A = (res>>8) & 0xff;
		REG_C = (res>>0) & 0xff;
		REG_B = mod & 0xff;
		SET_OV(0);
	}
	else
	{
		REG_A = 0xff;
		SET_OV(1);
	}

	SET_CY(0);
	CHECK_P();

	return 7;
}

int lc8670_cpu_device::op_bne()
{
	UINT8 data = get_data();
	UINT8 r8 = GET_R8;

	if (REG_A != data)
		set_pc(m_pc + SIGNED(r8));

	SET_CY((REG_A < data) ? 1 : 0);

	return 2;
}

int lc8670_cpu_device::op_bne_ri()
{
	UINT8 data = get_data();
	UINT8 i8 = GET_I8;
	UINT8 r8 = GET_R8;

	if (i8 != data)
		set_pc(m_pc + SIGNED(r8));

	SET_CY((data < i8) ? 1 : 0);

	return 2;
}

int lc8670_cpu_device::op_ldf()
{
	UINT16 addr = REG_TRL | (REG_TRH<<8);

	m_bankswitch_func(REG_FPR & 0x01 ? 2 : 1);
	REG_A = m_program->read_byte(addr);
	CHECK_P();
	m_bankswitch_func(((REG_EXT & 0x01) ? 1 : (REG_EXT & 0x08) ? 0 : 2));

	return 2;
}

int lc8670_cpu_device::op_stf()
{
	UINT16 addr = REG_TRL | (REG_TRH<<8);

	m_bankswitch_func(REG_FPR & 0x01 ? 2 : 1);
	m_program->write_byte(addr, REG_A);
	m_bankswitch_func(((REG_EXT & 0x01) ? 1 : (REG_EXT & 0x08) ? 0 : 2));

	return 2;
}

int lc8670_cpu_device::op_dbnz()
{
	UINT16 addr = get_addr();
	UINT8 r8 = GET_R8;
	UINT8 data = read_data_latch(addr) - 1;

	write_data_latch(addr, data);

	if (data != 0)
		set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_bpc()
{
	UINT8 b3 = GET_B3;
	UINT16 d9 = GET_D9B3;
	UINT8 r8 = GET_R8;
	UINT8 data = read_data_latch(d9);

	if (data & (1<<b3))
	{
		write_data_latch(d9, data & ~(1<<b3));
		set_pc(m_pc + SIGNED(r8));
	}

	return 2;
}

int lc8670_cpu_device::op_push()
{
	UINT16 d9 = GET_D9;
	push(read_data(d9));

	return 2;
}

int lc8670_cpu_device::op_inc()
{
	UINT16 addr = get_addr();
	UINT8 data = read_data_latch(addr);

	write_data_latch(addr, data + 1);

	return 1;
}

int lc8670_cpu_device::op_bp()
{
	UINT8 b3 = GET_B3;
	UINT16 d9 = GET_D9B3;
	UINT8 r8 = GET_R8;

	if (read_data(d9) & (1<<b3))
		set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_pop()
{
	UINT16 d9 = GET_D9;
	write_data(d9, pop());

	return 2;
}

int lc8670_cpu_device::op_dec()
{
	UINT16 addr = get_addr();
	UINT8 data = read_data_latch(addr);

	write_data_latch(addr, data - 1);

	return 1;
}

int lc8670_cpu_device::op_bz()
{
	UINT8 r8 = GET_R8;

	if (REG_A == 0)
		set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_add()
{
	UINT8 data = get_data();
	INT32 res = (REG_A + data);

	SET_CY(res > 0xff ? 1 : 0);
	SET_AC(((REG_A & 0x0f) + (data & 0x0f)) > 0x0f ? 1 : 0);
	SET_OV((REG_A & data) & (data ^ res) & 0x80 ? 1 : 0);

	REG_A = res & 0xff;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_bn()
{
	UINT8 b3 = GET_B3;
	UINT16 d9 = GET_D9B3;
	UINT8 r8 = GET_R8;

	if (!(read_data(d9) & (1<<b3)))
		set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_bnz()
{
	UINT8 r8 = GET_R8;

	if (REG_A != 0)
		set_pc(m_pc + SIGNED(r8));

	return 2;
}

int lc8670_cpu_device::op_addc()
{
	UINT8 data = get_data();
	INT32 res = (REG_A + data + GET_CY);

	SET_CY(res > 0xff ? 1 : 0);
	SET_AC(((REG_A & 0x0f) + (data & 0x0f) + GET_CY) > 0x0f ? 1 : 0);
	SET_OV(((REG_A+GET_CY) & data) & (data ^ res) & 0x80 ? 1 : 0);

	REG_A = res & 0xff;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_ret()
{
	UINT16 new_pc = pop()<<8;
	new_pc |= pop();
	set_pc(new_pc);

	return 2;
}

int lc8670_cpu_device::op_sub()
{
	UINT8 data = get_data();
	INT32 res = (REG_A - data);

	SET_CY(res < 0x00 ? 1 : 0);
	SET_AC(((REG_A & 0x0f) - (data & 0x0f)) < 0x00 ? 1 : 0);
	SET_OV((REG_A ^ data) & (data & res) & 0x80 ? 1 : 0);

	REG_A = res & 0xff;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_not1()
{
	UINT16 d9 = GET_D9B3;
	UINT8 data = read_data_latch(d9);

	data ^= (1<<GET_B3);
	write_data_latch(d9, data);

	return 1;
}

int lc8670_cpu_device::op_reti()
{
	UINT16 new_pc = pop()<<8;
	new_pc |= pop();
	set_pc(new_pc);

	if (LOG_IRQ)    logerror("%s: RETI from level %d\n", machine().describe_context(), m_irq_lev);
	for(int i=2; i>=0; i--)
		if (m_irq_lev & (1<<i))
		{
			m_irq_lev &= ~(1<<i);
			break;
		}

	m_after_reti = true;

	return 2;
}

int lc8670_cpu_device::op_subc()
{
	UINT8 data = get_data();
	INT32 res = (REG_A - data - GET_CY);

	SET_CY(res < 0x00 ? 1 : 0);
	SET_AC(((REG_A & 0x0f) - (data & 0x0f) - GET_CY) < 0x00 ? 1 : 0);
	SET_OV((REG_A ^ (data + GET_CY)) & (data & res) & 0x80 ? 1 : 0);

	REG_A = res & 0xff;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_ror()
{
	REG_A = ((REG_A & 0x01) << 7) | (REG_A>>1);
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_ldc()
{
	REG_A = m_program->read_byte(((REG_TRH<<8) | REG_TRL) + REG_A);
	CHECK_P();

	return 2;
}

int lc8670_cpu_device::op_xch()
{
	UINT16 addr = get_addr();
	UINT8 data = read_data(addr);

	write_data(addr, REG_A);
	REG_A = data;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_clr1()
{
	UINT16 d9 = GET_D9B3;
	UINT8 data = read_data_latch(d9);

	data &= ~(1<<GET_B3);
	write_data_latch(d9, data);

	return 1;
}

int lc8670_cpu_device::op_rorc()
{
	UINT8 a = (REG_A>>1) | (GET_CY ? 0x80 : 0x00);

	SET_CY(BIT(REG_A,0));
	REG_A = a;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_or()
{
	REG_A = REG_A | get_data();
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_rol()
{
	REG_A = ((REG_A & 0x80) >> 7) | (REG_A<<1);
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_and()
{
	REG_A = REG_A & get_data();
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_set1()
{
	UINT16 d9 = GET_D9B3;
	UINT8 data = read_data_latch(d9);

	data |= (1<<GET_B3);
	write_data_latch(d9, data);

	return 1;
}

int lc8670_cpu_device::op_rolc()
{
	UINT8 a = (REG_A<<1) | (GET_CY ? 0x01 : 0x00);

	SET_CY(BIT(REG_A,7));
	REG_A = a;
	CHECK_P();

	return 1;
}

int lc8670_cpu_device::op_xor()
{
	REG_A = REG_A ^ get_data();
	CHECK_P();

	return 1;
}
