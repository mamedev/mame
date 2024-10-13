// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    STmicro ST6-series microcontroller emulation skeleton

    To Do:
        - Cycle counts
        - STOP, WAIT opcodes
        - ADC, UART, SPI, Watchdog, Auto-Reload Timer peripherals
        - Interrupts

**********************************************************************/

#include "emu.h"
#include "st62xx.h"
#include "st62xx_dasm.h"

#define LOG_UNIMPL      (1U << 1)
#define LOG_GPIO        (1U << 2)
#define LOG_TIMER       (1U << 3)
#define LOG_ALL         (LOG_UNIMPL | LOG_GPIO | LOG_TIMER)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ST6228,   st6228_device,   "st6228",   "STmicro ST6228")

st6228_device::st6228_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, ST6228, tag, owner, clock)
	, m_pc(0)
	, m_mode(MODE_NMI)
	, m_prev_mode(MODE_NORMAL)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, address_map_constructor(FUNC(st6228_device::st6228_program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(st6228_device::st6228_data_map), this))
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_portd_out(*this)
	, m_timer_out(*this)
	, m_program(nullptr)
	, m_data(nullptr)
	, m_ram_bank(*this, "rambank")
	, m_data_bank(*this, "databank")
	, m_program_rom_bank(*this, "program_rombank")
	, m_data_rom_bank(*this, "data_rombank")
	, m_rom(*this, this->tag())
{
}

void st6228_device::st6228_program_map(address_map &map)
{
	map(0x000, 0x7ff).bankr(m_program_rom_bank);
	map(0x800, 0xfff).rom().region(tag(), 0x800);
}

void st6228_device::st6228_data_map(address_map &map)
{
	map(0x00, 0x3f).bankrw(m_ram_bank);
	map(0x40, 0x7f).bankr(m_data_rom_bank);
	map(0x80, 0xbf).bankrw(m_data_bank);
	map(0xc0, 0xff).rw(FUNC(st6228_device::unimplemented_reg_r), FUNC(st6228_device::unimplemented_reg_w));
	map(0xc0, 0xc3).rw(FUNC(st6228_device::gpio_data_r), FUNC(st6228_device::gpio_data_w));
	map(0xc4, 0xc7).rw(FUNC(st6228_device::gpio_dir_r), FUNC(st6228_device::gpio_dir_w));
	map(0xc9, 0xc9).w(FUNC(st6228_device::data_rom_window_w));
	map(0xca, 0xca).w(FUNC(st6228_device::rom_bank_select_w));
	map(0xcb, 0xcb).w(FUNC(st6228_device::ram_bank_select_w));
	map(0xcc, 0xcf).rw(FUNC(st6228_device::gpio_option_r), FUNC(st6228_device::gpio_option_w));
	map(0xd2, 0xd2).rw(FUNC(st6228_device::timer_prescale_r), FUNC(st6228_device::timer_prescale_w));
	map(0xd3, 0xd3).rw(FUNC(st6228_device::timer_counter_r), FUNC(st6228_device::timer_counter_w));
	map(0xd4, 0xd4).rw(FUNC(st6228_device::timer_status_r), FUNC(st6228_device::timer_control_w));
	map(0xd8, 0xd8).w(FUNC(st6228_device::watchdog_w));
}

void st6228_device::device_start()
{
	m_pc = 0;

	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",    m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_flags[0]).callimport().callexport().formatstr("%6s").noshow();
	state_add(STATE_FLAGS,     "FLAGS",    m_flags[0]).mask(0x3f);
	state_add(STATE_PC,        "PC",       m_pc).mask(0xfff);
	state_add(STATE_SP,        "SP",       m_stack_index).mask(0x7);
	state_add(STATE_STACK0,    "STACK0",   m_stack[0]).formatstr("%03X");
	state_add(STATE_STACK1,    "STACK1",   m_stack[1]).formatstr("%03X");
	state_add(STATE_STACK2,    "STACK2",   m_stack[2]).formatstr("%03X");
	state_add(STATE_STACK3,    "STACK3",   m_stack[3]).formatstr("%03X");
	state_add(STATE_STACK4,    "STACK4",   m_stack[4]).formatstr("%03X");
	state_add(STATE_STACK5,    "STACK5",   m_stack[5]).formatstr("%03X");
	state_add(STATE_A,         "A",        m_regs[REG_A]);
	state_add(STATE_X,         "X",        m_regs[REG_X]);
	state_add(STATE_Y,         "Y",        m_regs[REG_Y]);
	state_add(STATE_V,         "V",        m_regs[REG_V]);
	state_add(STATE_W,         "W",        m_regs[REG_W]);

	save_item(NAME(m_regs));
	save_item(NAME(m_ram));
	save_item(NAME(m_pc));
	save_item(NAME(m_mode));
	save_item(NAME(m_prev_mode));
	save_item(NAME(m_flags));
	save_item(NAME(m_stack));
	save_item(NAME(m_stack_index));
	save_item(NAME(m_icount));
	save_item(NAME(m_port_dir));
	save_item(NAME(m_port_option));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_pullup));
	save_item(NAME(m_port_analog));
	save_item(NAME(m_port_input));
	save_item(NAME(m_port_irq_enable));

	save_item(NAME(m_timer_divider));
	save_item(NAME(m_timer_pin));
	save_item(NAME(m_timer_active));

	// set our instruction counter
	set_icountptr(m_icount);

	m_ram_bank->configure_entries(0, 2, m_ram, 0x40);
	m_program_rom_bank->configure_entries(0, 4, m_rom->base(), 0x800);
	m_data_rom_bank->configure_entries(0, 128, m_rom->base(), 0x40);

	m_data_bank->set_base(&m_regs[0x80]);
}

void st6228_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	std::fill(std::begin(m_stack), std::end(m_stack), 0);
	std::fill(std::begin(m_flags), std::end(m_flags), 0);

	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0);
	std::fill(std::begin(m_port_option), std::end(m_port_option), 0);
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_pullup), std::end(m_port_pullup), 0);
	std::fill(std::begin(m_port_analog), std::end(m_port_analog), 0);
	std::fill(std::begin(m_port_input), std::end(m_port_input), 0);
	std::fill(std::begin(m_port_irq_enable), std::end(m_port_irq_enable), 0);

	m_pc = VEC_RESET;
	m_stack_index = 0;
	m_mode = MODE_NMI;
	m_prev_mode = MODE_NORMAL;

	m_ram_bank->set_entry(0);
	m_program_rom_bank->set_entry(0);
	m_data_rom_bank->set_entry(0);

	m_regs[REG_TIMER_COUNT] = 0xff;
	m_regs[REG_TIMER_PRESCALE] = 0x7f;
	m_regs[REG_WATCHDOG] = 0xfe;
	m_regs[REG_AD_CONTROL] = 0x40;

	m_timer_divider = 12;
	m_timer_pin = 0;
	m_timer_active = false;
}

device_memory_interface::space_config_vector st6228_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

void st6228_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c",
				(m_flags[2] & FLAG_C) ? 'C' : '.',
				(m_flags[2] & FLAG_Z) ? 'Z' : '.',
				(m_flags[1] & FLAG_C) ? 'C' : '.',
				(m_flags[1] & FLAG_Z) ? 'Z' : '.',
				(m_flags[0] & FLAG_C) ? 'C' : '.',
				(m_flags[0] & FLAG_Z) ? 'Z' : '.');
			break;
	}
}

std::unique_ptr<util::disasm_interface> st6228_device::create_disassembler()
{
	return std::make_unique<st62xx_disassembler>();
}

// TODO: interrupts
void st6228_device::porta0_w(int state) { m_port_input[PORT_A] &= ~(1 << 0); m_port_input[PORT_A] |= (state << 0); }
void st6228_device::porta1_w(int state) { m_port_input[PORT_A] &= ~(1 << 1); m_port_input[PORT_A] |= (state << 1); }
void st6228_device::porta2_w(int state) { m_port_input[PORT_A] &= ~(1 << 2); m_port_input[PORT_A] |= (state << 2); }
void st6228_device::porta3_w(int state) { m_port_input[PORT_A] &= ~(1 << 3); m_port_input[PORT_A] |= (state << 3); }
void st6228_device::porta4_w(int state) { m_port_input[PORT_A] &= ~(1 << 4); m_port_input[PORT_A] |= (state << 4); }
void st6228_device::porta5_w(int state) { m_port_input[PORT_A] &= ~(1 << 5); m_port_input[PORT_A] |= (state << 5); }

void st6228_device::portb4_w(int state) { m_port_input[PORT_B] &= ~(1 << 4); m_port_input[PORT_B] |= (state << 4); }
void st6228_device::portb5_w(int state) { m_port_input[PORT_B] &= ~(1 << 5); m_port_input[PORT_B] |= (state << 5); }
void st6228_device::portb6_w(int state) { m_port_input[PORT_B] &= ~(1 << 6); m_port_input[PORT_B] |= (state << 6); }

void st6228_device::portc4_w(int state) { m_port_input[PORT_C] &= ~(1 << 4); m_port_input[PORT_C] |= (state << 4); }
void st6228_device::portc5_w(int state) { m_port_input[PORT_C] &= ~(1 << 5); m_port_input[PORT_C] |= (state << 5); }
void st6228_device::portc6_w(int state) { m_port_input[PORT_C] &= ~(1 << 6); m_port_input[PORT_C] |= (state << 6); }
void st6228_device::portc7_w(int state) { m_port_input[PORT_C] &= ~(1 << 7); m_port_input[PORT_C] |= (state << 7); }

void st6228_device::portd1_w(int state) { m_port_input[PORT_D] &= ~(1 << 1); m_port_input[PORT_D] |= (state << 1); }
void st6228_device::portd2_w(int state) { m_port_input[PORT_D] &= ~(1 << 2); m_port_input[PORT_D] |= (state << 2); }
void st6228_device::portd3_w(int state) { m_port_input[PORT_D] &= ~(1 << 3); m_port_input[PORT_D] |= (state << 3); }
void st6228_device::portd4_w(int state) { m_port_input[PORT_D] &= ~(1 << 4); m_port_input[PORT_D] |= (state << 4); }
void st6228_device::portd5_w(int state) { m_port_input[PORT_D] &= ~(1 << 5); m_port_input[PORT_D] |= (state << 5); }
void st6228_device::portd6_w(int state) { m_port_input[PORT_D] &= ~(1 << 6); m_port_input[PORT_D] |= (state << 6); }
void st6228_device::portd7_w(int state) { m_port_input[PORT_D] &= ~(1 << 7); m_port_input[PORT_D] |= (state << 7); }

void st6228_device::gpio_set_output_bit(uint8_t index, uint8_t bit, uint8_t state)
{
	switch (index)
	{
		case PORT_A:
			if (bit < 6)
				m_porta_out[bit](state);
			break;
		case PORT_B:
			if (bit >= 4 && bit <= 6)
				m_portb_out[bit - 4](state);
			break;
		case PORT_C:
			if (bit >= 4)
				m_portc_out[bit - 4](state);
			break;
		case PORT_D:
			if (bit >= 1)
				m_portd_out[bit - 1](state);
			break;
	}
}

void st6228_device::gpio_update_mode(uint8_t index, uint8_t changed)
{
	const uint8_t dir = m_port_dir[index];
	const uint8_t option = m_port_option[index];
	for (uint8_t bit = 0; bit < 8; bit++)
	{
		const uint8_t mask = (1 << bit);
		if (BIT(changed, bit) && !BIT(dir, bit))
		{
			if (BIT(m_port_data[index], bit))
			{
				m_port_irq_enable[index] &= ~mask;
				m_port_pullup[index] &= ~mask;

				if (BIT(option, bit))
					m_port_analog[index] |= mask;
				else
					m_port_analog[index] &= ~mask;
			}
			else
			{
				m_port_pullup[index] |= mask;
				m_port_analog[index] &= ~mask;

				if (BIT(option, bit))
					m_port_irq_enable[index] |= mask;
				else
					m_port_irq_enable[index] &= ~mask;
			}
		}
		else if (BIT(dir, bit))
		{
			m_port_pullup[index] &= ~mask;
			m_port_analog[index] &= ~mask;
			m_port_irq_enable[index] &= ~mask;
		}
	}
}

void st6228_device::unimplemented_reg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_UNIMPL, "%s: unimplemented_reg_w: Unimplemented write: %02x = %02x\n", machine().describe_context(), offset + 0xc0, data);
}

uint8_t st6228_device::unimplemented_reg_r(offs_t offset)
{
	LOGMASKED(LOG_UNIMPL, "%s: unimplemented_reg_r: Unimplemented read: %02x\n", machine().describe_context(), offset + 0xc0);
	return 0;
}

void st6228_device::data_rom_window_w(offs_t offset, uint8_t data)
{
	m_regs[REG_DATA_ROM_WINDOW] = data;
	m_data_rom_bank->set_entry(data & 0x7f);
}

void st6228_device::rom_bank_select_w(offs_t offset, uint8_t data)
{
	m_regs[REG_ROM_BANK_SELECT] = data;
	m_program_rom_bank->set_entry(data & 3);
}

void st6228_device::ram_bank_select_w(offs_t offset, uint8_t data)
{
	m_regs[REG_RAM_BANK_SELECT] = data;
	m_ram_bank->set_entry(data & 1);
}

void st6228_device::gpio_data_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_data_w: Port %c Data = %02x\n", machine().describe_context(), offset + 'A', data);
	const uint8_t old_data = m_port_data[offset];
	const uint8_t changed = old_data ^ data;

	m_port_data[offset] = data;
	gpio_update_mode(offset, changed);

	if (changed & m_port_dir[offset])
	{
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (BIT(changed, bit))
				gpio_set_output_bit(offset, bit, BIT(data, bit));
		}
	}
}

void st6228_device::gpio_dir_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_dir_w: Port %c Direction = %02x\n", machine().describe_context(), offset + 'A', data);
	const uint8_t old_dir = m_port_dir[offset];
	const uint8_t changed = old_dir ^ data;

	m_port_dir[offset] = data;
	gpio_update_mode(offset, changed);

	if (changed)
	{
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (BIT(changed, bit))
			{
				gpio_set_output_bit(offset, bit, BIT(m_port_data[offset], bit));
			}
		}
	}
}

void st6228_device::gpio_option_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_option_w: Port %c Option = %02x\n", machine().describe_context(), offset + 'A', data);

	const uint8_t changed = m_port_option[offset] ^ data;
	m_port_option[offset] = data;

	gpio_update_mode(offset, changed);
}

void st6228_device::watchdog_w(offs_t offset, uint8_t data)
{
	// FIXME: Not yet implemented.
}

uint8_t st6228_device::gpio_data_r(offs_t offset)
{
	const uint8_t data = (m_port_data[offset] & m_port_dir[offset]) |
						 (m_port_input[offset] & ~m_port_dir[offset]) |
						 (m_port_pullup[offset] & ~m_port_dir[offset]);
	LOGMASKED(LOG_GPIO, "%s: gpio_data_r: Port %c Data: %02x\n", machine().describe_context(), offset + 'A', data);
	return data;
}

uint8_t st6228_device::gpio_dir_r(offs_t offset)
{
	const uint8_t data = m_port_dir[offset];
	LOGMASKED(LOG_GPIO, "%s: gpio_dir_r: Port %c Direction: %02x\n", machine().describe_context(), offset + 'A', data);
	return data;
}

uint8_t st6228_device::gpio_option_r(offs_t offset)
{
	const uint8_t data = m_port_option[offset];
	LOGMASKED(LOG_GPIO, "%s: gpio_option_r: Port %c Option: %02x\n", machine().describe_context(), offset + 'A', data);
	return data;
}

void st6228_device::timer_w(int state)
{
	const int old = m_timer_pin;
	m_timer_pin = state;
	if (old != m_timer_pin && m_timer_pin) // Rising Edge
	{
		if ((m_regs[REG_TIMER_CONTROL] & TSCR_MODE_MASK) == TSCR_MODE_EVENT) // Event-Counter mode
		{
			timer_prescaler_tick();
		}
	}
}

void st6228_device::timer_counter_tick()
{
	m_regs[REG_TIMER_COUNT]--;
	if (m_regs[REG_TIMER_COUNT])
	{
		m_regs[REG_TIMER_CONTROL] &= ~(1 << TSCR_TMZ_BIT);
	}
	else
	{
		m_regs[REG_TIMER_CONTROL] |= (1 << TSCR_TMZ_BIT);
		const uint8_t control = m_regs[REG_TIMER_CONTROL];
		if (BIT(control, TSCR_ETI_BIT))
		{
			// TODO: Request interrupt
		}

		if (BIT(control, TSCR_TOUT_BIT))
			m_timer_out(BIT(control, TSCR_DOUT_BIT));
	}
}

void st6228_device::timer_prescaler_tick()
{
	const uint8_t old_prescaler = m_regs[REG_TIMER_PRESCALE];
	m_regs[REG_TIMER_PRESCALE]--;
	const uint8_t prescaler_rising = ~old_prescaler & m_regs[REG_TIMER_PRESCALE];

	const uint8_t timer_ctrl = m_regs[REG_TIMER_CONTROL];

	const uint8_t prescaler_divider = (timer_ctrl & TSCR_PS_MASK) >> TSCR_PS_BIT;
	if (prescaler_divider == 0 || BIT(prescaler_rising, prescaler_divider - 1))
	{
		timer_counter_tick();
	}
}

void st6228_device::timer_prescale_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_TIMER, "%s: timer_prescale_w: Timer Prescale = %02x\n", machine().describe_context(), data);
	m_regs[REG_TIMER_PRESCALE] = data;
}

void st6228_device::timer_counter_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_TIMER, "%s: timer_counter_w: Timer Count = %02x\n", machine().describe_context(), data);
	m_regs[REG_TIMER_COUNT] = data;
}

void st6228_device::timer_control_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_TIMER, "%s: timer_control_w: Timer Control = %02x\n", machine().describe_context(), data);
	m_regs[REG_TIMER_CONTROL] = data;
	if (BIT(data, TSCR_ETI_BIT) && BIT(data, TSCR_TMZ_BIT))
	{
		// TODO: Request interrupt
	}
	else
	{
		// TODO: Clear interrupt
	}

	const bool active_mode = (BIT(data, TSCR_TOUT_BIT) || (BIT(data, TSCR_DOUT_BIT) && m_timer_pin));
	m_timer_active = BIT(data, TSCR_PSI_BIT) && active_mode;
}

uint8_t st6228_device::timer_prescale_r(offs_t offset)
{
	const uint8_t data = m_regs[REG_TIMER_PRESCALE];
	LOGMASKED(LOG_TIMER, "%s: timer_prescale_r: Timer Prescale: %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t st6228_device::timer_counter_r(offs_t offset)
{
	const uint8_t data = m_regs[REG_TIMER_COUNT];
	LOGMASKED(LOG_TIMER, "%s: timer_counter_r: Timer Count: %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t st6228_device::timer_status_r(offs_t offset)
{
	const uint8_t data = m_regs[REG_TIMER_CONTROL];
	LOGMASKED(LOG_TIMER, "%s: timer_status_r: Timer Status: %02x\n", machine().describe_context(), data);
	return data;
}

uint32_t st6228_device::execute_min_cycles() const noexcept
{
	return 2;
}

uint32_t st6228_device::execute_max_cycles() const noexcept
{
	return 5;
}

void st6228_device::execute_set_input(int inputnum, int state)
{
	logerror("%s: Unimplemented: execute_set_input line %d = %d\n", machine().describe_context(), inputnum, state);
}

void st6228_device::unimplemented_opcode(uint8_t op)
{
	fatalerror("ST62xx: unknown opcode (%02x) at %04x\n", op, m_pc);
}

void st6228_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		uint8_t op = m_program->read_byte(m_pc);

		int cycles = 4;

		switch (op)
		{
			case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70:
			case 0x80:            case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58: case 0x68: case 0x78:
			case 0x88:            case 0xa8: case 0xb8: case 0xc8: case 0xd8: case 0xe8: case 0xf8: // JRNZ e
			{
				const int8_t e = ((int8_t)op) >> 3;
				if (!(m_flags[m_mode] & FLAG_Z))
					m_pc += e;
				break;
			}
			case 0x01: case 0x11: case 0x21: case 0x31: case 0x41: case 0x51: case 0x61: case 0x71:
			case 0x81: case 0x91: case 0xa1: case 0xb1: case 0xc1: case 0xd1: case 0xe1: case 0xf1: // CALL abc
			{
				const uint8_t ab = m_program->read_byte(m_pc+1);
				m_pc += 2;
				const uint16_t abc = ((op & 0xf0) >> 4) | (ab << 4);
				if (m_stack_index < 6) // FIXME: magic numbers
				{
					m_stack[m_stack_index] = m_pc;
					m_stack_index++;
				}
				else
				{
					// Per documentation: "If more calls [than the maximum] are nested, the latest stacked PC
					//                     values will be lost. In this case, returns will return to the PC
					//                     values stacked first."
				}
				m_pc = abc-1;
				break;
			}
			case 0x09: case 0x19: case 0x29: case 0x39: case 0x49: case 0x59: case 0x69: case 0x79:
			case 0x89: case 0x99: case 0xa9: case 0xb9: case 0xc9: case 0xd9: case 0xe9: case 0xf9: // JP abc
			{
				const uint8_t ab = m_program->read_byte(m_pc+1);
				const uint16_t abc = ((op & 0xf0) >> 4) | (ab << 4);
				m_pc = abc-1;
				break;
			}
			case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62: case 0x72:
			case 0x82: case 0x92: case 0xa2: case 0xb2: case 0xc2: case 0xd2: case 0xe2: case 0xf2:
			case 0x0a: case 0x1a: case 0x2a: case 0x3a: case 0x4a: case 0x5a: case 0x6a: case 0x7a:
			case 0x8a: case 0x9a: case 0xaa: case 0xba: case 0xca: case 0xda: case 0xea: case 0xfa: // JRNC abc
			{
				const int8_t e = ((int8_t)op) >> 3;
				if (!(m_flags[m_mode] & FLAG_C))
					m_pc += e;
				break;
			}
			case 0x03: case 0x23: case 0x43: case 0x63: case 0x83: case 0xa3: case 0xc3: case 0xe3: // JRR b,rr,ee
			{
				const uint8_t b = (op >> 5) & 7;
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const int8_t ee = (int8_t)m_program->read_byte(m_pc+2);
				const uint8_t value = m_data->read_byte(rr);
				m_pc += 2;
				if (!BIT(value, b))
					m_pc += ee;
				break;
			}
			case 0x13: case 0x33: case 0x53: case 0x73: case 0x93: case 0xb3: case 0xd3: case 0xf3: // JRS b,rr,ee
			{
				const uint8_t b = (op >> 5) & 7;
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const int8_t ee = (int8_t)m_program->read_byte(m_pc+2);
				const uint8_t value = m_data->read_byte(rr);
				m_pc += 2;
				if (BIT(value, b))
					m_pc += ee;
				break;
			}
			case 0x0b: case 0x2b: case 0x4b: case 0x6b: case 0x8b: case 0xab: case 0xcb: case 0xeb: // RES b,rr
			{
				const uint8_t b = (op >> 5) & 7;
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const uint8_t nn = m_data->read_byte(rr);
				m_data->write_byte(rr, nn & ~(1 << b));
				m_pc++;
				break;
			}
			case 0x1b: case 0x3b: case 0x5b: case 0x7b: case 0x9b: case 0xbb: case 0xdb: case 0xfb: // SET b,rr
			{
				const uint8_t b = (op >> 5) & 7;
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const uint8_t nn = m_data->read_byte(rr);
				m_data->write_byte(rr, nn | (1 << b));
				m_pc++;
				break;
			}
			case 0x04: case 0x14: case 0x24: case 0x34: case 0x44: case 0x54: case 0x64: case 0x74:
			case 0x84: case 0x94: case 0xa4: case 0xb4: case 0xc4: case 0xd4: case 0xe4: case 0xf4:
			case 0x0c: case 0x1c: case 0x2c: case 0x3c: case 0x4c: case 0x5c: case 0x6c: case 0x7c:
			case 0x8c: case 0x9c: case 0xac: case 0xbc: case 0xcc: case 0xdc: case 0xec: case 0xfc: // JRZ e
			{
				const int8_t e = ((int8_t)op) >> 3;
				if (m_flags[m_mode] & FLAG_Z)
					m_pc += e;
				break;
			}
			case 0x06: case 0x16: case 0x26: case 0x36: case 0x46: case 0x56: case 0x66: case 0x76:
			case 0x86: case 0x96: case 0xa6: case 0xb6: case 0xc6: case 0xd6: case 0xe6: case 0xf6:
			case 0x0e: case 0x1e: case 0x2e: case 0x3e: case 0x4e: case 0x5e: case 0x6e: case 0x7e:
			case 0x8e: case 0x9e: case 0xae: case 0xbe: case 0xce: case 0xde: case 0xee: case 0xfe: // JRC e
			{
				const int8_t e = ((int8_t)op) >> 3;
				if (m_flags[m_mode] & FLAG_C)
					m_pc += e;
				break;
			}
			case 0x15: // INC X
				m_regs[REG_X]++;

				if (m_regs[REG_X])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x35: // LD A,X
				m_regs[REG_A] = m_regs[REG_X];

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x55: // INC Y
				m_regs[REG_Y]++;

				if (m_regs[REG_Y])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x75: // LD A,Y
				m_regs[REG_A] = m_regs[REG_Y];

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x95: // INC V
				m_regs[REG_V]++;

				if (m_regs[REG_V])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xb5: // LD A,V
				m_regs[REG_A] = m_regs[REG_V];

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xd5: // INC W
				m_regs[REG_W]++;

				if (m_regs[REG_W])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xf5: // LD A,W
				m_regs[REG_A] = m_regs[REG_W];

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x0d:  // LDI rr,nn
			{
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const uint8_t nn = m_program->read_byte(m_pc+2);
				m_data->write_byte(rr, nn);

				if (nn)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc += 2;
				break;
			}
			case 0x1d: // DEC X
				m_regs[REG_X]--;

				if (m_regs[REG_X])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x2d: // COM A
				if (BIT(m_regs[REG_A], 7))
					m_flags[m_mode] |= FLAG_C;
				else
					m_flags[m_mode] &= FLAG_C;

				m_regs[REG_A] = ~m_regs[REG_A];

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x3d: // LD X,A
				m_regs[REG_X] = m_regs[REG_A];

				if (m_regs[REG_X])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x4d:
				if (m_stack_index > 0)
				{
					m_stack_index--;
					m_pc = m_stack[m_stack_index] - 1;
					m_mode = m_prev_mode;
					m_prev_mode = MODE_NORMAL;
				}
				else
				{
					m_mode = MODE_NORMAL;
				}
				break;
			case 0x5d: // DEC Y
				m_regs[REG_Y]--;

				if (m_regs[REG_Y])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x6d:
				//util::stream_format(stream, "STOP");
				break;
			case 0x7d: // LD Y,A
				m_regs[REG_Y] = m_regs[REG_A];

				if (m_regs[REG_Y])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x9d: // DEC V
				m_regs[REG_V]--;

				if (m_regs[REG_V])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xad: // RLC
			{
				const uint8_t old_c = (m_flags[m_mode] & FLAG_C) ? 1 : 0;
				if (BIT(m_regs[REG_A], 7))
					m_flags[m_mode] |= FLAG_C;
				else
					m_flags[m_mode] &= ~FLAG_C;
				m_regs[REG_A] = (m_regs[REG_A] << 1) | old_c;

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			}
			case 0xbd: // LD V,A
				m_regs[REG_V] = m_regs[REG_A];

				if (m_regs[REG_V])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xcd:
				if (m_stack_index > 0)
				{
					m_stack_index--;
					m_pc = m_stack[m_stack_index] - 1;
				}
				else
				{
					fatalerror("Attempted to RET with nothing on the stack");
				}
				break;
			case 0xdd: // DEC W
				m_regs[REG_W]--;

				if (m_regs[REG_W])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xed:
				//util::stream_format(stream, "WAIT");
				break;
			case 0xfd: // LD W,A
				m_regs[REG_W] = m_regs[REG_A];

				if (m_regs[REG_W])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x07: // LD A,(X)
				m_regs[REG_A] = m_data->read_byte(m_regs[REG_X]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x17:  // LDI A,rr
			{
				m_regs[REG_A] = m_program->read_byte(m_pc+1);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0x27: // CP A,(X)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_X]);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				break;
			}
			case 0x37: // CPI A,nn
			{
				const uint8_t nn = m_program->read_byte(m_pc+1);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_pc++;
				break;
			}
			case 0x47: // ADD A,(X)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_X]);
				const uint16_t sum = m_regs[REG_A] + nn;

				if (sum > 0xff)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (sum == 0)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] = (uint8_t)sum;
				break;
			}
			case 0x57: // ADDI A,nn
			{
				const uint8_t nn = m_program->read_byte(m_pc+1);
				const uint16_t sum = m_regs[REG_A] + nn;

				if (sum > 0xff)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (sum == 0)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] = (uint8_t)sum;
				m_pc++;
				break;
			}
			case 0x67: // INC (X)
			{
				const uint8_t rr = m_data->read_byte(m_regs[REG_X]) + 1;
				m_data->write_byte(m_regs[REG_X], rr);

				if (rr)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			}
			case 0x87: // LD (X),A
				m_data->write_byte(m_regs[REG_X], m_regs[REG_A]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xa7: // AND A,(X)
				m_regs[REG_A] &= m_data->read_byte(m_regs[REG_X]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0xb7: // ANDI A,nn
			{
				m_regs[REG_A] &= m_program->read_byte(m_pc+1);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0xc7: // SUB A,(X)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_X]);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] -= nn;
				break;
			}
			case 0xd7: // SUBI A,nn
			{
				const uint8_t nn = m_program->read_byte(m_pc+1);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] -= nn;
				m_pc++;
				break;
			}
			case 0xe7: // DEC (X)
			{
				const uint8_t rr = m_data->read_byte(m_regs[REG_X]) - 1;
				m_data->write_byte(m_regs[REG_X], rr);

				if (rr)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			}
			case 0x0f: // LD A,(Y)
				m_regs[REG_A] = m_data->read_byte(m_regs[REG_Y]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x1f: // LD A,rr
			{
				m_regs[REG_A] = m_data->read_byte(m_program->read_byte(m_pc+1));

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0x2f: // CP A,(Y)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_Y]);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				break;
			}
			case 0x3f: // CP A,rr
			{
				const uint8_t nn = m_data->read_byte(m_program->read_byte(m_pc+1));

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_pc++;
				break;
			}
			case 0x4f: // ADD A,(Y)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_Y]);
				const uint16_t sum = m_regs[REG_A] + nn;

				if (sum > 0xff)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (sum == 0)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] = (uint8_t)sum;
				break;
			}
			case 0x5f: // ADD A,rr
			{
				const uint8_t nn = m_data->read_byte(m_program->read_byte(m_pc+1));
				const uint16_t sum = m_regs[REG_A] + nn;

				if (sum > 0xff)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (sum == 0)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] = (uint8_t)sum;
				m_pc++;
				break;
			}
			case 0x6f: // INC (Y)
			{
				const uint8_t rr = m_data->read_byte(m_regs[REG_Y]) + 1;
				m_data->write_byte(m_regs[REG_Y], rr);

				if (rr)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			}
			case 0x7f: // INC rr
			{
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const uint8_t nn = m_data->read_byte(rr) + 1;
				m_data->write_byte(rr, nn);

				if (nn)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0x8f: // LD (Y),A
				m_data->write_byte(m_regs[REG_Y], m_regs[REG_A]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			case 0x9f: // LD rr,A
			{
				m_data->write_byte(m_program->read_byte(m_pc+1), m_regs[REG_A]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0xaf: // AND A,(Y)
				m_regs[REG_A] &= m_data->read_byte(m_regs[REG_Y]);

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;

				break;
			case 0xbf: // AND A,rr
			{
				m_regs[REG_A] &= m_data->read_byte(m_program->read_byte(m_pc+1));

				if (m_regs[REG_A])
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			case 0xcf: // SUB A,(Y)
			{
				const uint8_t nn = m_data->read_byte(m_regs[REG_Y]);

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] -= nn;
				break;
			}
			case 0xdf: // SUB A,rr
			{
				const uint8_t nn = m_data->read_byte(m_program->read_byte(m_pc+1));

				if (m_regs[REG_A] < nn)
				{
					m_flags[m_mode] |= FLAG_C;
					m_flags[m_mode] &= ~FLAG_Z;
				}
				else
				{
					m_flags[m_mode] &= ~FLAG_C;
					if (m_regs[REG_A] == nn)
						m_flags[m_mode] |= FLAG_Z;
					else
						m_flags[m_mode] &= ~FLAG_Z;
				}
				m_regs[REG_A] -= nn;
				m_pc++;
				break;
			}
			case 0xef: // DEC (Y)
			{
				const uint8_t rr = m_data->read_byte(m_regs[REG_Y]) - 1;
				m_data->write_byte(m_regs[REG_Y], rr);

				if (rr)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				break;
			}
			case 0xff: // DEC rr
			{
				const uint8_t rr = m_program->read_byte(m_pc+1);
				const uint8_t value = m_data->read_byte(rr) - 1;
				m_data->write_byte(rr, value);

				if (value)
					m_flags[m_mode] &= ~FLAG_Z;
				else
					m_flags[m_mode] |= FLAG_Z;
				m_pc++;
				break;
			}
			default:
				logerror("%s: Unsupported opcode: %02x\n", op);
		}

		m_pc++;

		if (m_timer_active)
		{
			m_timer_divider -= cycles;
			if (m_timer_divider <= 0)
			{
				m_timer_divider += 12;
				timer_prescaler_tick();
			}
		}

		m_icount -= cycles;
	}
}
