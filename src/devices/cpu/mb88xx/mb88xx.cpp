// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi

    TODO:
    - Verify timer, Bosconian voice samples are currently a good test case,
      and they match PCB recordings.
    - Improve support for the serial interface.
    - Split the core to support multiple CPU types?

***************************************************************************/

#include "emu.h"
#include "mb88xx.h"
#include "mb88dasm.h"


DEFINE_DEVICE_TYPE(MB88201, mb88201_cpu_device, "mb88201", "Fujitsu MB88201")
DEFINE_DEVICE_TYPE(MB88202, mb88202_cpu_device, "mb88202", "Fujitsu MB88202")
DEFINE_DEVICE_TYPE(MB8841,  mb8841_cpu_device,  "mb8841",  "Fujitsu MB8841")
DEFINE_DEVICE_TYPE(MB8842,  mb8842_cpu_device,  "mb8842",  "Fujitsu MB8842")
DEFINE_DEVICE_TYPE(MB8843,  mb8843_cpu_device,  "mb8843",  "Fujitsu MB8843")
DEFINE_DEVICE_TYPE(MB8844,  mb8844_cpu_device,  "mb8844",  "Fujitsu MB8844")


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SERIAL_PRESCALE         6       /* guess */
#define TIMER_PRESCALE          32      /* guess */

#define SERIAL_DISABLE_THRESH   1000    /* at this value, we give up driving the serial port */

#define INT_CAUSE_SERIAL        0x01
#define INT_CAUSE_TIMER         0x02
#define INT_CAUSE_EXTERNAL      0x04


/***************************************************************************
    MACROS
***************************************************************************/

#define READOP(a)           (m_cache.read_byte(a))

#define RDMEM(a)            (m_data.read_byte(a) & 0x0f)
#define WRMEM(a,v)          (m_data.write_byte((a), (v) & 0x0f))

#define TEST_ST()           (m_st & 1)
#define TEST_ZF()           (m_zf & 1)
#define TEST_CF()           (m_cf & 1)
#define TEST_VF()           (m_vf & 1)
#define TEST_SF()           (m_sf & 1)
#define TEST_IF()           (m_if & 1)

#define UPDATE_ST_C(v)      m_st = (v & 0x10) ? 0 : 1
#define UPDATE_ST_Z(v)      m_st = (v == 0) ? 0 : 1

#define UPDATE_CF(v)        m_cf = ((v & 0x10) == 0) ? 0 : 1
#define UPDATE_ZF(v)        m_zf = (v != 0) ? 0 : 1

#define GETPC()             (((int)m_PA << 6) + m_PC)
#define GETEA()             ((m_X << 4) + m_Y)

#define INCPC()             do { m_PC++; if (m_PC >= 0x40) { m_PC = 0; m_PA++; } } while (0)


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mb88_cpu_device::program_9bit(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void mb88_cpu_device::program_10bit(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void mb88_cpu_device::program_11bit(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void mb88_cpu_device::data_4bit(address_map &map)
{
	map(0x00, 0x0f).ram();
}

void mb88_cpu_device::data_5bit(address_map &map)
{
	map(0x00, 0x1f).ram();
}

void mb88_cpu_device::data_6bit(address_map &map)
{
	map(0x00, 0x3f).ram();
}

void mb88_cpu_device::data_7bit(address_map &map)
{
	map(0x00, 0x7f).ram();
}


mb88_cpu_device::mb88_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int data_width)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, program_width, 0,
			(program_width == 9) ? address_map_constructor(FUNC(mb88_cpu_device::program_9bit), this) :
			(program_width == 10) ? address_map_constructor(FUNC(mb88_cpu_device::program_10bit), this) :
			address_map_constructor(FUNC(mb88_cpu_device::program_11bit), this))
	, m_data_config("data", ENDIANNESS_BIG, 8, data_width, 0,
			(data_width == 4) ? address_map_constructor(FUNC(mb88_cpu_device::data_4bit), this) :
			(data_width == 5) ? address_map_constructor(FUNC(mb88_cpu_device::data_5bit), this) :
			(data_width == 6) ? address_map_constructor(FUNC(mb88_cpu_device::data_6bit), this) :
			address_map_constructor(FUNC(mb88_cpu_device::data_7bit), this))
	, m_pla_data(nullptr)
	, m_pla_bits(8)
	, m_read_k(*this, 0)
	, m_write_o(*this)
	, m_write_p(*this)
	, m_read_r(*this, 0)
	, m_write_r(*this)
	, m_read_si(*this, 0)
	, m_write_so(*this)
{
}

mb88201_cpu_device::mb88201_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB88201, tag, owner, clock, 9, 4)
{
}

mb88202_cpu_device::mb88202_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB88202, tag, owner, clock, 10, 5)
{
}


mb8841_cpu_device::mb8841_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB8841, tag, owner, clock, 11, 7)
{
}


mb8842_cpu_device::mb8842_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB8842, tag, owner, clock, 11, 7)
{
}


mb8843_cpu_device::mb8843_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB8843, tag, owner, clock, 10, 6)
{
}


mb8844_cpu_device::mb8844_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb88_cpu_device(mconfig, MB8844, tag, owner, clock, 10, 6)
{
}

device_memory_interface::space_config_vector mb88_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> mb88_cpu_device::create_disassembler()
{
	return std::make_unique<mb88_disassembler>();
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

void mb88_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);

	m_serial = timer_alloc(FUNC(mb88_cpu_device::serial_timer), this);

	m_if = 0;
	m_ctr = 0;
	m_o_output = 0;

	save_item(NAME(m_PC));
	save_item(NAME(m_PA));
	save_item(NAME(m_SP));
	save_item(NAME(m_SI));
	save_item(NAME(m_A));
	save_item(NAME(m_X));
	save_item(NAME(m_Y));
	save_item(NAME(m_st));
	save_item(NAME(m_zf));
	save_item(NAME(m_cf));
	save_item(NAME(m_vf));
	save_item(NAME(m_sf));
	save_item(NAME(m_if));
	save_item(NAME(m_pio));
	save_item(NAME(m_TH));
	save_item(NAME(m_TL));
	save_item(NAME(m_TP));
	save_item(NAME(m_ctr));
	save_item(NAME(m_SB));
	save_item(NAME(m_SBcount));
	save_item(NAME(m_o_output));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_in_irq));

	state_add(MB88XX_PC,  "PC",  m_PC).formatstr("%02X");
	state_add(MB88XX_PA,  "PA",  m_PA).formatstr("%02X");
	state_add(MB88XX_SI,  "SI",  m_SI).formatstr("%01X");
	state_add(MB88XX_A,   "A",   m_A).formatstr("%01X");
	state_add(MB88XX_X,   "X",   m_X).formatstr("%01X");
	state_add(MB88XX_Y,   "Y",   m_Y).formatstr("%01X");
	state_add(MB88XX_PIO, "PIO", m_pio).formatstr("%02X");
	state_add(MB88XX_TH,  "TH",  m_TH).formatstr("%01X");
	state_add(MB88XX_TL,  "TL",  m_TL).formatstr("%01X");
	state_add(MB88XX_SB,  "SB",  m_SB).formatstr("%01X");

	state_add(STATE_GENPC, "GENPC", m_debugger_pc).callimport().callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_debugger_pc).callimport().callexport().noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_debugger_flags).callimport().callexport().formatstr("%6s").noshow();
	set_icountptr(m_icount);
}


void mb88_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_st = (m_debugger_flags & 0x01) ? 1 : 0;
			m_zf = (m_debugger_flags & 0x02) ? 1 : 0;
			m_cf = (m_debugger_flags & 0x04) ? 1 : 0;
			m_vf = (m_debugger_flags & 0x08) ? 1 : 0;
			m_sf = (m_debugger_flags & 0x10) ? 1 : 0;
			m_if = (m_debugger_flags & 0x20) ? 1 : 0;
			break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_PC = m_debugger_pc & 0x3f;
			m_PA = (m_debugger_pc >> 6) & 0x1f;
			break;
	}
}


void mb88_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_debugger_flags = 0;
			if (TEST_ST()) m_debugger_flags |= 0x01;
			if (TEST_ZF()) m_debugger_flags |= 0x02;
			if (TEST_CF()) m_debugger_flags |= 0x04;
			if (TEST_VF()) m_debugger_flags |= 0x08;
			if (TEST_SF()) m_debugger_flags |= 0x10;
			if (TEST_IF()) m_debugger_flags |= 0x20;
			break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_debugger_pc = GETPC();
			break;
	}
}


void mb88_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c",
					TEST_ST() ? 'T' : 't',
					TEST_ZF() ? 'Z' : 'z',
					TEST_CF() ? 'C' : 'c',
					TEST_VF() ? 'V' : 'v',
					TEST_SF() ? 'S' : 's',
					TEST_IF() ? 'I' : 'i');

			break;
	}
}


void mb88_cpu_device::device_reset()
{
	// zero registers and flags
	m_PC = 0;
	m_PA = 0;
	m_SP[0] = m_SP[1] = m_SP[2] = m_SP[3] = 0;
	m_SI = 0;
	m_A = 0;
	m_X = 0;
	m_Y = 0;
	m_st = 1; // start off with st=1
	m_zf = 0;
	m_cf = 0;
	m_vf = 0;
	m_sf = 0;
	m_pio = 0;
	m_TH = 0;
	m_TL = 0;
	m_TP = 0;
	m_SB = 0;
	m_SBcount = 0;
	m_pending_irq = 0;
	m_in_irq = 0;
}

/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

TIMER_CALLBACK_MEMBER(mb88_cpu_device::serial_timer)
{
	m_SBcount++;

	// if we get too many interrupts with no servicing, disable the timer until somebody does something
	if (m_SBcount >= SERIAL_DISABLE_THRESH)
		m_serial->adjust(attotime::never);

	// only read if not full; this is needed by the Namco 52xx to ensure that
	// the program can write to S and recover the value even if serial is enabled
	if (!m_sf)
	{
		m_SB = (m_SB >> 1) | (m_read_si() ? 8 : 0);

		if (m_SBcount >= 4)
		{
			m_sf = 1;
			m_pending_irq |= INT_CAUSE_SERIAL;
		}
	}
}

void mb88_cpu_device::write_pla(u8 index)
{
	u8 mask = 0xff;

	if (m_pla_bits == 8)
	{
		const u8 shift = (index & 0x10) ? 4 : 0;
		mask = 0xf << shift;
		m_o_output = (m_o_output & ~mask) | (index << shift & mask);
	}
	else
	{
		// if the driver hasn't supplied PLA data, just output the index
		if (m_pla_data)
			m_o_output = m_pla_data[index];
		else
			m_o_output = index;
	}

	m_write_o(0, m_o_output, mask);
}

void mb88_cpu_device::execute_set_input(int inputnum, int state)
{
	state = state ? 1 : 0;

	switch (inputnum)
	{
		case MB88XX_IRQ_LINE:
			// On rising edge trigger interrupt.
			// Note this is a logical level, the actual pin is high-to-low voltage triggered.
			if (!m_if && state && (m_pio & INT_CAUSE_EXTERNAL))
				m_pending_irq |= INT_CAUSE_EXTERNAL;

			m_if = state;
			break;

		case MB88XX_TC_LINE:
			// on a falling clock, increment the timer, but only if enabled
			if (m_ctr && !state && (m_pio & 0x40))
				increment_timer();

			m_ctr = state;
			break;

		default:
			break;
	}
}

void mb88_cpu_device::pio_enable(u8 newpio)
{
	// if the serial state has changed, configure the timer
	if ((m_pio ^ newpio) & 0x30)
	{
		if ((newpio & 0x30) == 0)
			m_serial->adjust(attotime::never);
		else if ((newpio & 0x30) == 0x20)
			m_serial->adjust(attotime::from_hz(clock() / SERIAL_PRESCALE), 0, attotime::from_hz(clock() / SERIAL_PRESCALE));
		else
			fatalerror("mb88xx: pio_enable set serial enable to unsupported value %02X\n", newpio & 0x30);
	}

	m_pio = newpio;
}

void mb88_cpu_device::increment_timer()
{
	m_TL = (m_TL + 1) & 0x0f;
	if (m_TL == 0)
	{
		m_TH = (m_TH + 1) & 0x0f;
		if (m_TH == 0)
		{
			m_vf = 1;
			m_pending_irq |= INT_CAUSE_TIMER;
		}
	}
}

void mb88_cpu_device::burn_cycles(int cycles)
{
	m_icount -= cycles;

	// internal clock enable
	if (m_pio & 0x80)
	{
		m_TP += cycles;
		while (m_TP >= TIMER_PRESCALE)
		{
			m_TP -= TIMER_PRESCALE;
			increment_timer();
		}
	}

	// process pending interrupts
	if (!m_in_irq && m_pending_irq & m_pio)
	{
		m_in_irq = true;
		u16 intpc = GETPC();

		m_SP[m_SI] = intpc;
		m_SP[m_SI] |= TEST_CF() << 15;
		m_SP[m_SI] |= TEST_ZF() << 14;
		m_SP[m_SI] |= TEST_ST() << 13;
		m_SI = (m_SI + 1) & 3;

		// the datasheet doesn't mention interrupt vectors but the Arabian MCU program expects the following
		if (m_pending_irq & m_pio & INT_CAUSE_EXTERNAL)
		{
			standard_irq_callback(0, intpc);
			m_PC = 0x02;
		}
		else if (m_pending_irq & m_pio & INT_CAUSE_TIMER)
		{
			standard_irq_callback(1, intpc);
			m_PC = 0x04;
		}
		else if (m_pending_irq & m_pio & INT_CAUSE_SERIAL)
		{
			standard_irq_callback(2, intpc);
			m_PC = 0x06;
		}

		m_PA = 0x00;
		m_st = 1;
		m_pending_irq = 0;

		burn_cycles(3); // ?
	}
}


void mb88_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		u8 opcode, arg, oc;

		// fetch the opcode
		debugger_instruction_hook(GETPC());
		opcode = READOP(GETPC());

		// increment the PC
		INCPC();

		// start with instruction doing 1 cycle
		oc = 1;

		switch (opcode)
		{
			case 0x00: // nop ZCS:...
				m_st = 1;
				break;

			case 0x01: // outO ZCS:...
				write_pla(TEST_CF() << 4 | m_A);
				m_st = 1;
				break;

			case 0x02: // outP ZCS:...
				m_write_p(m_A);
				m_st = 1;
				break;

			case 0x03: // outR ZCS:...
				arg = m_Y;
				m_write_r[arg & 3](m_A);
				m_st = 1;
				break;

			case 0x04: // tay ZCS:...
				m_Y = m_A;
				m_st = 1;
				break;

			case 0x05: // tath ZCS:...
				m_TH = m_A;
				m_st = 1;
				break;

			case 0x06: // tatl ZCS:...
				m_TL = m_A;
				m_st = 1;
				break;

			case 0x07: // tas ZCS:...
				m_SB = m_A;
				m_st = 1;
				break;

			case 0x08: // icy ZCS:x.x
				m_Y++;
				UPDATE_ST_C(m_Y);
				m_Y &= 0x0f;
				UPDATE_ZF(m_Y);
				break;

			case 0x09: // icm ZCS:x.x
				arg = RDMEM(GETEA());
				arg++;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(), arg);
				break;

			case 0x0a: // stic ZCS:x.x
				WRMEM(GETEA(), m_A);
				m_Y++;
				UPDATE_ST_C(m_Y);
				m_Y &= 0x0f;
				UPDATE_ZF(m_Y);
				break;

			case 0x0b: // x ZCS:x..
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), m_A);
				m_A = arg;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x0c: // rol ZCS:xxx
				m_A <<= 1;
				m_A |= TEST_CF();
				UPDATE_ST_C(m_A);
				m_cf = m_st ^ 1;
				m_A &= 0x0f;
				UPDATE_ZF(m_A);
				break;

			case 0x0d: // l ZCS:x..
				m_A = RDMEM(GETEA());
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x0e: // adc ZCS:xxx
				arg = RDMEM(GETEA());
				arg += m_A;
				arg += TEST_CF();
				UPDATE_ST_C(arg);
				m_cf = m_st ^ 1;
				m_A = arg & 0x0f;
				UPDATE_ZF(m_A);
				break;

			case 0x0f: // and ZCS:x.x
				m_A &= RDMEM(GETEA());
				UPDATE_ZF(m_A);
				m_st = m_zf ^ 1;
				break;

			case 0x10: // daa ZCS:.xx
				if (TEST_CF() || m_A > 9) m_A += 6;
				UPDATE_ST_C(m_A);
				m_cf = m_st ^ 1;
				m_A &= 0x0f;
				break;

			case 0x11: // das ZCS:.xx
				if (TEST_CF() || m_A > 9) m_A += 10;
				UPDATE_ST_C(m_A);
				m_cf = m_st ^ 1;
				m_A &= 0x0f;
				break;

			case 0x12: // inK ZCS:x..
				m_A = m_read_k() & 0x0f;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x13: // inR ZCS:x..
				arg = m_Y;
				m_A = m_read_r[arg & 3]() & 0x0f;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x14: // tya ZCS:x..
				m_A = m_Y;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x15: // ttha ZCS:x..
				m_A = m_TH;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x16: // ttla ZCS:x..
				m_A = m_TL;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x17: // tsa ZCS:x..
				m_A = m_SB;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x18: // dcy ZCS:..x
				m_Y--;
				UPDATE_ST_C(m_Y);
				m_Y &= 0x0f;
				break;

			case 0x19: // dcm ZCS:x.x
				arg = RDMEM(GETEA());
				arg--;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(), arg);
				break;

			case 0x1a: // stdc ZCS:x.x
				WRMEM(GETEA(), m_A);
				m_Y--;
				UPDATE_ST_C(m_Y);
				m_Y &= 0x0f;
				UPDATE_ZF(m_Y);
				break;

			case 0x1b: // xx ZCS:x..
				arg = m_X;
				m_X = m_A;
				m_A = arg;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x1c: // ror ZCS:xxx
				m_A |= TEST_CF() << 4;
				UPDATE_ST_C(m_A << 4);
				m_cf = m_st ^ 1;
				m_A >>= 1;
				m_A &= 0x0f;
				UPDATE_ZF(m_A);
				break;

			case 0x1d: // st ZCS:x..
				WRMEM(GETEA(), m_A);
				m_st = 1;
				break;

			case 0x1e: // sbc ZCS:xxx
				arg = RDMEM(GETEA());
				arg -= m_A;
				arg -= TEST_CF();
				UPDATE_ST_C(arg);
				m_cf = m_st ^ 1;
				m_A = arg & 0x0f;
				UPDATE_ZF(m_A);
				break;

			case 0x1f: // or ZCS:x.x
				m_A |= RDMEM(GETEA());
				UPDATE_ZF(m_A);
				m_st = m_zf ^ 1;
				break;

			case 0x20: // setR ZCS:...
				arg = m_read_r[m_Y >> 2]() & 0x0f;
				m_write_r[m_Y >> 2](arg | (1 << (m_Y & 3)));
				m_st = 1;
				break;

			case 0x21: // setc ZCS:.xx
				m_cf = 1;
				m_st = 1;
				break;

			case 0x22: // rstR ZCS:...
				arg = m_read_r[m_Y >> 2]() & 0x0f;
				m_write_r[m_Y >> 2](arg & ~(1 << (m_Y & 3)));
				m_st = 1;
				break;

			case 0x23: // rstc ZCS:.xx
				m_cf = 0;
				m_st = 1;
				break;

			case 0x24: // tstr ZCS:..x
				arg = m_read_r[m_Y >> 2]() & 0x0f;
				m_st = (arg & (1 << (m_Y & 3))) ? 0 : 1;
				break;

			case 0x25: // tsti ZCS:..x
				m_st = m_if ^ 1;
				break;

			case 0x26: // tstv ZCS:..x
				m_st = m_vf ^ 1;
				m_vf = 0;
				break;

			case 0x27: // tsts ZCS:..x
				m_st = m_sf ^ 1;
				if (m_sf)
				{
					// re-enable the timer if we disabled it previously
					if (m_SBcount >= SERIAL_DISABLE_THRESH)
						m_serial->adjust(attotime::from_hz(clock() / SERIAL_PRESCALE), 0, attotime::from_hz(clock() / SERIAL_PRESCALE));
					m_SBcount = 0;
				}
				m_sf = 0;
				break;

			case 0x28: // tstc ZCS:..x
				m_st = m_cf ^ 1;
				break;

			case 0x29: // tstz ZCS:..x
				m_st = m_zf ^ 1;
				break;

			case 0x2a: // sts ZCS:x..
				WRMEM(GETEA(), m_SB);
				UPDATE_ZF(m_SB);
				m_st = 1;
				break;

			case 0x2b: // ls ZCS:x..
				m_SB = RDMEM(GETEA());
				UPDATE_ZF(m_SB);
				m_st = 1;
				break;

			case 0x2c: // rts ZCS:...
				m_SI = (m_SI - 1) & 3;
				m_PC = m_SP[m_SI] & 0x3f;
				m_PA = (m_SP[m_SI] >> 6) & 0x1f;
				m_st = 1;
				break;

			case 0x2d: // neg ZCS: ..x
				m_A = (~m_A) + 1;
				m_A &= 0x0f;
				UPDATE_ST_Z(m_A);
				break;

			case 0x2e: // c ZCS:xxx
				arg = RDMEM(GETEA());
				arg -= m_A;
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				m_zf = m_st ^ 1;
				break;

			case 0x2f: // eor ZCS:x.x
				m_A ^= RDMEM(GETEA());
				UPDATE_ST_Z(m_A);
				m_zf = m_st ^ 1;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: // sbit ZCS:...
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg | (1 << (opcode & 3)));
				m_st = 1;
				break;

			case 0x34: case 0x35: case 0x36: case 0x37: // rbit ZCS:...
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg & ~(1 << (opcode & 3)));
				m_st = 1;
				break;

			case 0x38: case 0x39: case 0x3a: case 0x3b: // tbit ZCS:...
				arg = RDMEM(GETEA());
				m_st = (arg & (1 << (opcode & 3))) ? 0 : 1;
				break;

			case 0x3c: // rti ZCS:...
				// restore address and saved state flags on the top bits of the stack
				m_in_irq = false;
				m_SI = (m_SI - 1) & 3;
				m_PC = m_SP[m_SI] & 0x3f;
				m_PA = (m_SP[m_SI] >> 6) & 0x1f;
				m_st = (m_SP[m_SI] >> 13) & 1;
				m_zf = (m_SP[m_SI] >> 14) & 1;
				m_cf = (m_SP[m_SI] >> 15) & 1;
				break;

			case 0x3d: // jpa imm ZCS:..x
				m_PA = READOP(GETPC()) & 0x1f;
				m_PC = m_A * 4;
				oc++;
				m_st = 1;
				break;

			case 0x3e: // en imm ZCS:...
				pio_enable(m_pio | READOP(GETPC()));
				INCPC();
				oc++;
				m_st = 1;
				break;

			case 0x3f: // dis imm ZCS:...
				pio_enable(m_pio & ~(READOP(GETPC())));
				INCPC();
				oc++;
				m_st = 1;
				break;

			case 0x40: case 0x41: case 0x42: case 0x43: // setD ZCS:...
				arg = m_read_r[0]() & 0x0f;
				arg |= (1 << (opcode & 3));
				m_write_r[0](arg);
				m_st = 1;
				break;

			case 0x44: case 0x45: case 0x46: case 0x47: // rstD ZCS:...
				arg = m_read_r[0]() & 0x0f;
				arg &= ~(1 << (opcode & 3));
				m_write_r[0](arg);
				m_st = 1;
				break;

			case 0x48: case 0x49: case 0x4a: case 0x4b: // tstD ZCS:..x
				arg = m_read_r[2]() & 0x0f;
				m_st = (arg & (1 << (opcode & 3))) ? 0 : 1;
				break;

			case 0x4c: case 0x4d: case 0x4e: case 0x4f: // tba ZCS:..x
				m_st = (m_A & (1 << (opcode & 3))) ? 0 : 1;
				break;

			case 0x50: case 0x51: case 0x52: case 0x53: // xd ZCS:x..
				arg = RDMEM(opcode&3);
				WRMEM((opcode & 3), m_A);
				m_A = arg;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0x54: case 0x55: case 0x56: case 0x57: // xyd ZCS:x..
				arg = RDMEM((opcode & 3) + 4);
				WRMEM((opcode & 3) + 4, m_Y);
				m_Y = arg;
				UPDATE_ZF(m_Y);
				m_st = 1;
				break;

			case 0x58: case 0x59: case 0x5a: case 0x5b:
			case 0x5c: case 0x5d: case 0x5e: case 0x5f: // lxi ZCS:x..
				m_X = opcode & 7;
				UPDATE_ZF(m_X);
				m_st = 1;
				break;

			case 0x60: case 0x61: case 0x62: case 0x63:
			case 0x64: case 0x65: case 0x66: case 0x67: // call imm ZCS:..x
				arg = READOP(GETPC());
				INCPC();
				oc++;
				if (TEST_ST())
				{
					m_SP[m_SI] = GETPC();
					m_SI = (m_SI + 1) & 3;
					m_PC = arg & 0x3f;
					m_PA = ((opcode & 7) << 2) | (arg >> 6);
				}
				m_st = 1;
				break;

			case 0x68: case 0x69: case 0x6a: case 0x6b:
			case 0x6c: case 0x6d: case 0x6e: case 0x6f: // jpl imm ZCS:..x
				arg = READOP(GETPC());
				INCPC();
				oc++;
				if (TEST_ST())
				{
					m_PC = arg & 0x3f;
					m_PA = ((opcode & 7) << 2) | (arg >> 6);
				}
				m_st = 1;
				break;

			case 0x70: case 0x71: case 0x72: case 0x73:
			case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7a: case 0x7b:
			case 0x7c: case 0x7d: case 0x7e: case 0x7f: // ai ZCS:xxx
				arg = opcode & 0x0f;
				arg += m_A;
				UPDATE_ST_C(arg);
				m_cf = m_st ^ 1;
				m_A = arg & 0x0f;
				UPDATE_ZF(m_A);
				break;

			case 0x80: case 0x81: case 0x82: case 0x83:
			case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b:
			case 0x8c: case 0x8d: case 0x8e: case 0x8f: // lxi ZCS:x..
				m_Y = opcode & 0x0f;
				UPDATE_ZF(m_Y);
				m_st = 1;
				break;

			case 0x90: case 0x91: case 0x92: case 0x93:
			case 0x94: case 0x95: case 0x96: case 0x97:
			case 0x98: case 0x99: case 0x9a: case 0x9b:
			case 0x9c: case 0x9d: case 0x9e: case 0x9f: // li ZCS:x..
				m_A = opcode & 0x0f;
				UPDATE_ZF(m_A);
				m_st = 1;
				break;

			case 0xa0: case 0xa1: case 0xa2: case 0xa3:
			case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab:
			case 0xac: case 0xad: case 0xae: case 0xaf: // cyi ZCS:xxx
				arg = (opcode & 0x0f) - m_Y;
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				m_zf = m_st ^ 1;
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3:
			case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb:
			case 0xbc: case 0xbd: case 0xbe: case 0xbf: // ci ZCS:xxx
				arg = (opcode & 0x0f) - m_A;
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				m_zf = m_st ^ 1;
				break;

			default: // jmp ZCS:..x
				if (TEST_ST())
					m_PC = opcode & 0x3f;
				m_st = 1;
				break;
		}

		// update cycle count, also update interrupts, serial and timer flags
		burn_cycles(oc);
	}
}
