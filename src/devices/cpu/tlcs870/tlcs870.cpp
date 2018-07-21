// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    The TLCS-870/X expands on this instruction set using the same base encoding.

    The TLCS-870/C appears to have a completely different encoding.

*************************************************************************************************************/



#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"


DEFINE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device, "tmp87ph40an", "Toshiba TMP87PH40AN")

void tlcs870_device::tmp87ph40an_mem(address_map &map)
{
#if 0
	map(0x0000, 0x0000).rw(FUNC(tlcs870_device::port0_r), FUNC(tlcs870_device::port0_w));
	map(0x0001, 0x0001).rw(FUNC(tlcs870_device::port1_r), FUNC(tlcs870_device::port1_w));
	map(0x0002, 0x0002).rw(FUNC(tlcs870_device::port2_r), FUNC(tlcs870_device::port2_w));
	map(0x0003, 0x0003).rw(FUNC(tlcs870_device::port3_r), FUNC(tlcs870_device::port3_w));
	map(0x0004, 0x0004).rw(FUNC(tlcs870_device::port4_r), FUNC(tlcs870_device::port4_w));
	map(0x0005, 0x0005).rw(FUNC(tlcs870_device::port5_r), FUNC(tlcs870_device::port5_w));
	map(0x0006, 0x0006).rw(FUNC(tlcs870_device::port6_r), FUNC(tlcs870_device::port6_w));
	map(0x0007, 0x0007).rw(FUNC(tlcs870_device::port7_r), FUNC(tlcs870_device::port7_w));
	// 0x8 reserved
	// 0x9 reserved
	map(0x000a, 0x000a).w(FUNC(tlcs870_device::p0cr_w)); // Port 0 I/O control
	map(0x000b, 0x000b).w(FUNC(tlcs870_device::p1cr_w)); // Port 1 I/O control
	map(0x000c, 0x000c).w(FUNC(tlcs870_device::p6cr_w)); // Port 6 I/O control
	map(0x000d, 0x000d).w(FUNC(tlcs870_device::p7cr_w)); // Port 7 I/O control
	map(0x000e, 0x000e).rw(FUNC(tlcs870_device::adccr_r), FUNC(tlcs870_device::adccr_w)); // A/D converter control
	map(0x000f, 0x000f).r(FUNC(tlcs870_device::adcdr_r)); // A/D converter result

	map(0x0010, 0x0010).w(FUNC(tlcs870_device::treg1a_l_w)); // Timer register 1A
	map(0x0011, 0x0011).w(FUNC(tlcs870_device::treg1a_h_w)); //
	map(0x0012, 0x0012).rw(FUNC(tlcs870_device::treg1b_l_r), FUNC(tlcs870_device::treg1b_l_w)); // Timer register 1B
	map(0x0013, 0x0013).rw(FUNC(tlcs870_device::treg1b_h_r), FUNC(tlcs870_device::treg1b_h_w)); //
	map(0x0014, 0x0014).w(FUNC(tlcs870_device::tc1cr_w)); // TC1 control
	map(0x0015, 0x0015).w(FUNC(tlcs870_device::tc2cr_w)); // TC2 control
	map(0x0016, 0x0016).w(FUNC(tlcs870_device::treg2_l_w)); // Timer register 2
	map(0x0017, 0x0017).w(FUNC(tlcs870_device::treg2_h_w)); //
	map(0x0018, 0x0018).rw(FUNC(tlcs870_device::treg3a_r), FUNC(tlcs870_device::treg3a_w)); // Timer register 3A
	map(0x0019, 0x0019).r(FUNC(tlcs870_device::treg3b_r)); // Timer register 3B
	map(0x001a, 0x001a).w(FUNC(tlcs870_device::tc3cr_w)); // TC3 control
	map(0x001b, 0x001b).r(FUNC(tlcs870_device::treg4_r)); // Timer register 4
	map(0x001c, 0x001c).w(FUNC(tlcs870_device::tc4cr_w)); // TC4 control
	// 0x1d reserved
	// 0x1e reserved
	// 0x1f reserved

	map(0x0020, 0x0020).rw(FUNC(tlcs870_device::sio1sr_r), FUNC(tlcs870_device::sio1cr1_w)); // SIO1 status / SIO1 control
	map(0x0021, 0x0021).w(FUNC(tlcs870_device::sio1cr2_w)); // SIO1 control
	map(0x0022, 0x0022).rw(FUNC(tlcs870_device::sio2sr_r), FUNC(tlcs870_device::sio2cr1_w)); // SIO2 status / SIO2 control
	map(0x0023, 0x0023).w(FUNC(tlcs870_device::sio2cr2_w)); // SIO2 control
	// 0x24 reserved
	// 0x25 reserved
	// 0x26 reserved
	// 0x27 reserved
	// 0x28 reserved
	// 0x29 reserved
	// 0x2a reserved
	// 0x2b reserved
	// 0x2c reserved
	// 0x2d reserved
	// 0x2e reserved
	// 0x2f reserved

	// 0x30 reserved
	// 0x31 reserved
	// 0x32 reserved
	// 0x33 reserved
	map(0x0034, 0x0034).w(FUNC(tlcs870_device::wdtcr1_w)); // WDT control
	map(0x0035, 0x0035).w(FUNC(tlcs870_device::wdtcr2_w)); //

	map(0x0036, 0x0036).rw(FUNC(tlcs870_device::tbtcr_r), FUNC(tlcs870_device::tbtcr_w)); // TBT / TG / DVO control
	map(0x0037, 0x0037).rw(FUNC(tlcs870_device::eintcr_r), FUNC(tlcs870_device::eintcr_w)); // External interrupt control

	map(0x0038, 0x0038).rw(FUNC(tlcs870_device::syscr1_r), FUNC(tlcs870_device::syscr1_w)); // System Control
	map(0x0039, 0x0039).rw(FUNC(tlcs870_device::syscr2_r), FUNC(tlcs870_device::syscr2_w)); //

	map(0x003a, 0x003a).rw(FUNC(tlcs870_device::eir_l_r), FUNC(tlcs870_device::eir_l_w)); // Interrupt enable register
	map(0x003b, 0x003b).rw(FUNC(tlcs870_device::eir_h_r), FUNC(tlcs870_device::eir_h_w)); //

	map(0x003c, 0x003c).rw(FUNC(tlcs870_device::il_l_r), FUNC(tlcs870_device::il_l_w)); // Interrupt latch
	map(0x003d, 0x003d).rw(FUNC(tlcs870_device::il_h_r), FUNC(tlcs870_device::il_h_w)); //
	// 0x3e reserved
	map(0x003f, 0x003f).rw(FUNC(tlcs870_device::psw_r), FUNC(tlcs870_device::rbs_w)); // Program status word / Register bank selector
#endif

	map(0x0040, 0x023f).ram().share("intram"); // register banks + internal RAM, not, code execution NOT allowed here
	map(0x0f80, 0x0fff).ram(); // DBR
	map(0xc000, 0xffff).rom();
}


tlcs870_device::tlcs870_device(const machine_config &mconfig, device_type optype, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map)
	: cpu_device(mconfig, optype, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, program_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_intram(*this, "intram")
{
}


tmp87ph40an_device::tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tlcs870_device(mconfig, TMP87PH40AN, tag, owner, clock, address_map_constructor(FUNC(tmp87ph40an_device::tmp87ph40an_mem), this))
{
}

device_memory_interface::space_config_vector tlcs870_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

bool tlcs870_device::stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb)
{
	return false;
}

void tlcs870_device::execute_set_input(int inputnum, int state)
{
#if 0
	switch (inputnum) {
	case INPUT_LINE_NMI:
		set_irq_line(INTNMI, state);
		break;
	case INPUT_LINE_IRQ0:
		set_irq_line(INT0, state);
		break;
	case INPUT_LINE_IRQ1:
		set_irq_line(INT1, state);
		break;
	case INPUT_LINE_IRQ2:
		set_irq_line(INT2, state);
		break;
	}
#endif
}

void tlcs870_device::execute_run()
{
	m_cycles = 1;

	while (m_icount > 0)
	{
		m_prvpc.d = m_pc.d;
		debugger_instruction_hook(m_pc.d);

		//check_interrupts();
		m_addr = m_pc.d;
		m_tmppc = m_addr; // used for jumps etc.
		decode();
		m_pc.d = m_addr;

		m_icount -= m_cycles * 4; // 1 machine cycle = 4 clock cycles?
	};
}

void tlcs870_device::device_reset()
{
	m_pc.d = RM16(0xfffe);
	m_RBS = 0;
}

void tlcs870_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case DEBUGGER_REG_A:
		set_reg8(REG_A, m_debugger_temp);
		break;

	case DEBUGGER_REG_W:
		set_reg8(REG_W, m_debugger_temp);
		break;

	case DEBUGGER_REG_C:
		set_reg8(REG_C, m_debugger_temp);
		break;

	case DEBUGGER_REG_B:
		set_reg8(REG_B, m_debugger_temp);
		break;

	case DEBUGGER_REG_E:
		set_reg8(REG_E, m_debugger_temp);
		break;

	case DEBUGGER_REG_D:
		set_reg8(REG_D, m_debugger_temp);
		break;

	case DEBUGGER_REG_L:
		set_reg8(REG_L, m_debugger_temp);
		break;

	case DEBUGGER_REG_H:
		set_reg8(REG_H, m_debugger_temp);
		break;

	case DEBUGGER_REG_WA:
		set_reg16(REG_WA, m_debugger_temp);
		break;

	case DEBUGGER_REG_BC:
		set_reg16(REG_BC, m_debugger_temp);
		break;

	case DEBUGGER_REG_DE:
		set_reg16(REG_DE, m_debugger_temp);
		break;

	case DEBUGGER_REG_HL:
		set_reg16(REG_HL, m_debugger_temp);
		break;
	}
}


void tlcs870_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case DEBUGGER_REG_A:
		m_debugger_temp = get_reg8(REG_A);
		break;

	case DEBUGGER_REG_W:
		m_debugger_temp = get_reg8(REG_W);
		break;

	case DEBUGGER_REG_C:
		m_debugger_temp = get_reg8(REG_C);
		break;

	case DEBUGGER_REG_B:
		m_debugger_temp = get_reg8(REG_B);
		break;

	case DEBUGGER_REG_E:
		m_debugger_temp = get_reg8(REG_E);
		break;

	case DEBUGGER_REG_D:
		m_debugger_temp = get_reg8(REG_D);
		break;

	case DEBUGGER_REG_L:
		m_debugger_temp = get_reg8(REG_L);
		break;

	case DEBUGGER_REG_H:
		m_debugger_temp = get_reg8(REG_H);
		break;

	case DEBUGGER_REG_WA:
		m_debugger_temp = get_reg16(REG_WA);
		break;

	case DEBUGGER_REG_BC:
		m_debugger_temp = get_reg16(REG_BC);
		break;

	case DEBUGGER_REG_DE:
		m_debugger_temp = get_reg16(REG_DE);
		break;

	case DEBUGGER_REG_HL:
		m_debugger_temp = get_reg16(REG_HL);
		break;

	}
}


void tlcs870_device::device_start()
{
	//  int i, p;
	m_sp.d = 0x0000;
	m_F = 0;

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	state_add(DEBUGGER_REG_A, "A", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_W, "W", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_C, "C", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_B, "B", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_E, "E", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_D, "D", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_L, "L", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_H, "H", m_debugger_temp).callimport().callexport().formatstr("%02X");

	state_add(DEBUGGER_REG_WA, "WA", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_BC, "BC", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_DE, "DE", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_HL, "HL", m_debugger_temp).callimport().callexport().formatstr("%04X");


	state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X");
	state_add(STATE_GENPCBASE, "CURPC", m_prvpc.w.l).formatstr("%04X").noshow();
	state_add(STATE_GENSP, "GENSP", m_sp.w.l).formatstr("%04X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_F).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}


void tlcs870_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	int F = m_F;

	switch (entry.index())
	{

	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c",
			F & 0x80 ? 'J' : '.',
			F & 0x40 ? 'Z' : '.',
			F & 0x20 ? 'C' : '.',
			F & 0x10 ? 'H' : '.'
		);
		break;
	}

}

std::unique_ptr<util::disasm_interface> tlcs870_device::create_disassembler()
{
	return std::make_unique<tlcs870_disassembler>();
}
