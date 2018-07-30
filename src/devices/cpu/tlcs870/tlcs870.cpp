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
#if 0

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
#endif
	map(0x0037, 0x0037).rw(FUNC(tlcs870_device::eintcr_r), FUNC(tlcs870_device::eintcr_w)); // External interrupt control
#if 0
	map(0x0038, 0x0038).rw(FUNC(tlcs870_device::syscr1_r), FUNC(tlcs870_device::syscr1_w)); // System Control
	map(0x0039, 0x0039).rw(FUNC(tlcs870_device::syscr2_r), FUNC(tlcs870_device::syscr2_w)); //
#endif
	map(0x003a, 0x003a).rw(FUNC(tlcs870_device::eir_l_r), FUNC(tlcs870_device::eir_l_w)); // Interrupt enable register
	map(0x003b, 0x003b).rw(FUNC(tlcs870_device::eir_h_r), FUNC(tlcs870_device::eir_h_w)); //

	map(0x003c, 0x003c).rw(FUNC(tlcs870_device::il_l_r), FUNC(tlcs870_device::il_l_w)); // Interrupt latch
	map(0x003d, 0x003d).rw(FUNC(tlcs870_device::il_h_r), FUNC(tlcs870_device::il_h_w)); //
#if 0																					
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
	, m_port_in_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_port_out_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
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

// NOT using templates here because there are subtle differences in the port behavior (the ports are multi-purpose) that still need implementing
READ8_MEMBER(tlcs870_device::port0_r)
{
	// need to use P0CR (0x000a) to control direction

	if (m_read_input_port)
		return m_port_in_cb[0]();
	else
		return m_port_out_latch[0];
}

READ8_MEMBER(tlcs870_device::port1_r)
{
	// need to use P1CR (0x000b) to control direction

	if (m_read_input_port)
		return m_port_in_cb[1]();
	else
		return m_port_out_latch[1];
}

READ8_MEMBER(tlcs870_device::port2_r) // 3-bit port
{
	if (m_read_input_port)
		return m_port_in_cb[2]() | 0xf8;
	else
		return m_port_out_latch[2];
}

READ8_MEMBER(tlcs870_device::port3_r)
{
	if (m_read_input_port)
		return m_port_in_cb[3]();
	else
		return m_port_out_latch[3];
}

READ8_MEMBER(tlcs870_device::port4_r)
{
	if (m_read_input_port)
		return m_port_in_cb[4]();
	else
		return m_port_out_latch[4];
}

READ8_MEMBER(tlcs870_device::port5_r) // 5-bit port
{
	if (m_read_input_port)
		return m_port_in_cb[5]() | 0xe0;
	else
		return m_port_out_latch[5];
}

READ8_MEMBER(tlcs870_device::port6_r) // doubles up as analog?
{
	// need to use P6CR (0x000c) to control direction

	if (m_read_input_port)
		return m_port_in_cb[6]();
	else
		return m_port_out_latch[6];
}

READ8_MEMBER(tlcs870_device::port7_r)
{
	// need to use P7CR (0x000d) to control direction

	if (m_read_input_port)
		return m_port_in_cb[7]();
	else
		return m_port_out_latch[7];
}

WRITE8_MEMBER(tlcs870_device::port0_w)
{
	m_port_out_latch[0] = data;
	m_port_out_cb[0](data);
}

WRITE8_MEMBER(tlcs870_device::port1_w)
{
	m_port_out_latch[1] = data;
	m_port_out_cb[1](data);
}

WRITE8_MEMBER(tlcs870_device::port2_w)
{
	m_port_out_latch[2] = data;
	m_port_out_cb[2](data);
}

WRITE8_MEMBER(tlcs870_device::port3_w)
{
	m_port_out_latch[3] = data;
	m_port_out_cb[3](data);
}

WRITE8_MEMBER(tlcs870_device::port4_w)
{
	m_port_out_latch[4] = data;
	m_port_out_cb[4](data);
}

WRITE8_MEMBER(tlcs870_device::port5_w)
{
	m_port_out_latch[5] = data;
	m_port_out_cb[5](data);
}

WRITE8_MEMBER(tlcs870_device::port6_w)
{
	m_port_out_latch[6] = data;
	m_port_out_cb[6](data);
}

WRITE8_MEMBER(tlcs870_device::port7_w)
{
	m_port_out_latch[7] = data;
	m_port_out_cb[7](data);
}

WRITE8_MEMBER(tlcs870_device::p0cr_w)
{
	m_port0_cr = data;
}

WRITE8_MEMBER(tlcs870_device::p1cr_w)
{
	m_port1_cr = data;
}

WRITE8_MEMBER(tlcs870_device::p6cr_w)
{
	m_port6_cr = data;
}

WRITE8_MEMBER(tlcs870_device::p7cr_w)
{
	m_port7_cr = data;
}


READ8_MEMBER(tlcs870_device::adccr_r)
{
//	logerror("adccr_r\n");
	return 0x00;
}

READ8_MEMBER(tlcs870_device::adcdr_r)
{
	logerror("adcdr_r\n");
	return 0x00;
}

WRITE8_MEMBER(tlcs870_device::adccr_w)
{
	logerror("adccr_w %02x\n", data);
}



READ8_MEMBER(tlcs870_device::eintcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(tlcs870_device::eintcr_w)
{
	m_EINTCR = data;

	logerror("m_EINTCR (External Interrupt Control) bits set to\n");
	logerror("%d: INT1NC (Interrupt noise reject)\n", (m_EINTCR & 0x80) ? 1 : 0);
	logerror("%d: INT0EN (Interrupt 0 enable)\n",     (m_EINTCR & 0x40) ? 1 : 0);
	logerror("%d: (invalid)\n",                       (m_EINTCR & 0x20) ? 1 : 0);
	logerror("%d: INT4ES (edge select)\n",            (m_EINTCR & 0x10) ? 1 : 0);
	logerror("%d: INT3ES (edge select)\n",            (m_EINTCR & 0x08) ? 1 : 0); 
	logerror("%d: INT2ES (edge select)\n",            (m_EINTCR & 0x04) ? 1 : 0); 
	logerror("%d: INT1ES (edge select)\n",            (m_EINTCR & 0x02) ? 1 : 0); 
	logerror("%d: (invalid)\n",                       (m_EINTCR & 0x01) ? 1 : 0);

	/* For edge select register 0 = rising edge, 1 = falling edge

	   For INT0EN if 1 then Port 1 bit 0 is used for IRQ0, otherwise it is used for a port bit
	   if it is used as an IRQ pin then it should also be configured as an input in P1CR
	*/
}

READ8_MEMBER(tlcs870_device::eir_l_r)
{
	return m_EIR & 0xff;
}

READ8_MEMBER(tlcs870_device::eir_h_r)
{
	return (m_EIR >> 8) & 0xff;
}

WRITE8_MEMBER(tlcs870_device::eir_l_w)
{
	m_EIR = (m_EIR & 0xff00) | data;

	logerror("m_EIR(LSB) (Interrupt Enable) bits set to\n");
	logerror("%d: EF7 (External Interrupt 2)\n",      (m_EIR & 0x0080) ? 1 : 0);
	logerror("%d: EF6 (Time Base Timer Interrupt)\n", (m_EIR & 0x0040) ? 1 : 0);
	logerror("%d: EF5 (External Interrupt 1)\n",      (m_EIR & 0x0020) ? 1 : 0);
	logerror("%d: EF4 (16-bit TC1 Interrupt)\n",      (m_EIR & 0x0010) ? 1 : 0);
	logerror("%d: (invalid)\n",                       (m_EIR & 0x0008) ? 1 : 0); // can't be External Int 0 (bit in different register is used)
	logerror("%d: (invalid)\n",                       (m_EIR & 0x0004) ? 1 : 0); // can't be Watchdog interrupt (non-maskable)
	logerror("%d: (invalid)\n",                       (m_EIR & 0x0002) ? 1 : 0); // can't be Software interrupt (non-maskable)
	logerror("%d: IMF\n",                             (m_EIR & 0x0001) ? 1 : 0); // can't be Reset interrupt (non-maskable)
}

WRITE8_MEMBER(tlcs870_device::eir_h_w)
{
	m_EIR = (m_EIR & 0x00ff) | (data << 8);

	logerror("m_EIR(MSB) (Interrupt Enable) bits set to\n");
	logerror("%d: EF15 (External Interrupt 5)\n",          (m_EIR & 0x8000) ? 1 : 0);
	logerror("%d: EF14 (16-bit TC2 Interrupt)\n",          (m_EIR & 0x4000) ? 1 : 0);
	logerror("%d: EF13 (Serial Interface 2 Interrupt)\n",  (m_EIR & 0x2000) ? 1 : 0);
	logerror("%d: EF12 (External Interrupt 4)\n",          (m_EIR & 0x1000) ? 1 : 0);
	logerror("%d: EF11 (External Interrupt 3)\n",          (m_EIR & 0x0800) ? 1 : 0);
	logerror("%d: EF10 (8-bit TC4 Interrupt)\n",           (m_EIR & 0x0400) ? 1 : 0);
	logerror("%d: EF9  (Serial Interface 1 Interrupt)\n",  (m_EIR & 0x0200) ? 1 : 0); 
	logerror("%d: EF8  (8-bit TC3 Interrupt)\n",           (m_EIR & 0x0100) ? 1 : 0);
}

/*
	the READ/WRITE/MODIFY operations cannot be used to clear interrupt latches

	also you can't set a latch by writing '1' to it, only clear a latch
	by writing 0 to it

*/
READ8_MEMBER(tlcs870_device::il_l_r)
{
	return m_IL & 0xff;
}

READ8_MEMBER(tlcs870_device::il_h_r)
{
	return (m_IL >> 8) & 0xff;
}

WRITE8_MEMBER(tlcs870_device::il_l_w)
{
	// probably not this logic
	m_IL = (m_IL & 0xff00) | data;
}

WRITE8_MEMBER(tlcs870_device::il_h_w)
{
	// probably not this logic
	m_IL = (m_EIR & 0x00ff) | (data << 8);
}

void tlcs870_device::execute_set_input(int inputnum, int state)
{
	int32_t irqline = -1;

	switch (inputnum)
	{
	case TLCS870_IRQ_INT5:
	case INPUT_LINE_IRQ5:
		irqline = 15;
		break;

	case TLCS870_IRQ_INT4:
	case INPUT_LINE_IRQ4:
		irqline = 12;
		break;

	case TLCS870_IRQ_INT3:
	case INPUT_LINE_IRQ3:
		irqline = 11;
		break;

	case TLCS870_IRQ_INT2:
	case INPUT_LINE_IRQ2:
		irqline = 7;
		break;

	case TLCS870_IRQ_INT1:
	case INPUT_LINE_IRQ1:
		irqline = 5;
		break;

	case TLCS870_IRQ_INT0:
	case INPUT_LINE_IRQ0:
		irqline = 3;
		break;
	}

	if (irqline != -1)
	{
		set_irq_line(irqline, state);
	}
}

void tlcs870_device::set_irq_line(int irqline, int state)
{
	//logerror("set_irq_line %d %d\n", irqline, state);

	if (!(m_irqstate & (1 << irqline)))
	{
		// rising edge
		if (state)
		{
			m_irqstate |= 1<<irqline;

			// TODO: add checks to see if interrupt pin(s) are configured, and if they're in rising edge mode
			m_IL |= 1<<irqline;
		}
	}
	else
	{
		if (!state)
		{
			m_irqstate &= ~(1<<irqline);
		}
	}
}

void tlcs870_device::check_interrupts()
{
	// priority 0-2 are non-maskable, and should have already been processed before we get here
	for (int priority = 0; priority <= 15; priority++)
	{
		if (priority >= 3) // only priorities 0,1,2 are non-maskable
		{
			if (!(m_EIR & 1))
			{
				// maskable interrupts are disabled, bail
				continue;
			}

			int is_latched = m_IL & (1<<priority);

			if (is_latched)
			{
				take_interrupt(priority);
				return;
			}
		}
		else
		{
			// TODO: handle non-maskable here
		}
	}
}

void tlcs870_device::take_interrupt(int priority)
{
	m_IL &= ~(1<<priority);
	m_EIR &= ~1;
	logerror("taken interrupt %d\n", priority);

	uint16_t vector = RM16(0xffe0 + ((15-priority)*2));

	WM16(m_sp.d - 1, get_PSW());
	WM16(m_sp.d - 2, m_addr);
	m_sp.d -= 3;

	m_pc.d = vector;
	logerror("setting PC to %04x\n", m_addr);

}

void tlcs870_device::execute_run()
{
	while (m_icount > 0)
	{
		check_interrupts();

		m_prvpc.d = m_pc.d;
		debugger_instruction_hook(m_pc.d);

		m_addr = m_pc.d;
		m_tmppc = m_addr; // used for jumps etc.
		m_cycles = 0;
		m_read_input_port = 1; // some operations force the output latches to read from the memory mapped ports, not input ports
		decode();
		m_pc.d = m_addr;

		if (m_cycles)
		{
			//m_icount -= m_cycles * 4; // 1 machine cycle = 4 clock cycles? (unclear, execution seems far too slow even for the ram test this way)
			m_icount -= m_cycles;
		}
		else
		{
			fatalerror("m_cycles == 0 after PC %04x\n", m_tmppc);
		}
	};
}

void tlcs870_device::device_reset()
{
	m_pc.d = RM16(0xfffe);
	m_RBS = 0;
	m_EIR = 0;
	m_IL = 0;
	m_EINTCR = 0;
	m_irqstate = 0;

	m_port_out_latch[0] = 0x00;
	m_port_out_latch[1] = 0x00;
	m_port_out_latch[2] = 0xff;
	m_port_out_latch[3] = 0xff;
	m_port_out_latch[4] = 0xff;
	m_port_out_latch[5] = 0xff;
	m_port_out_latch[6] = 0x00;
	m_port_out_latch[7] = 0x00;

	m_port0_cr = 0xff;
	m_port1_cr = 0xff;
	m_port6_cr = 0xff;
	m_port7_cr = 0xff;
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

	for (auto &cb : m_port_in_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_port_out_cb)
		cb.resolve_safe();
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
