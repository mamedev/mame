// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS2100, TMS2170, TMS2300, TMS2370

TMS2100 is an enhanced version of TMS1100, adding interrupt, timer, A/D converter, and a 4-level callstack
- the mpla has a similar layout as TMS1400, terms reduced to 26
  (looks like it's optimized and not meant to be custom)
- the opla is the same as TMS1400

Extra functions are controlled with the R register (not mapped to pins):
- R15: enable external interrupt
- R16: K/J input select
- R17: load initial value register with TMA
- R18: internal/external counter clock control
- R19: A1/A2 input select
- R20: enable A/D
- R21: R0-R3 I/O control
- R22: R0-R3/ACC2 output select
- R23: enable decrementer load
- R24: enable interrupts

TODO:
- R0-R3 I/O, TRA opcode
- A/D converter, TADM opcode
- what happens if decrementer is loaded when IVR is 0?

*/

#include "emu.h"
#include "tms2100.h"
#include "tms1k_dasm.h"


// device definitions
DEFINE_DEVICE_TYPE(TMS2100, tms2100_cpu_device, "tms2100", "Texas Instruments TMS2100") // 28-pin DIP, 7 R pins
DEFINE_DEVICE_TYPE(TMS2170, tms2170_cpu_device, "tms2170", "Texas Instruments TMS2170") // high voltage version, 1 R pin removed for Vpp
DEFINE_DEVICE_TYPE(TMS2300, tms2300_cpu_device, "tms2300", "Texas Instruments TMS2300") // 40-pin DIP, 15 R pins, J pins
DEFINE_DEVICE_TYPE(TMS2370, tms2370_cpu_device, "tms2370", "Texas Instruments TMS2370") // high voltage version, 1 R pin removed for Vpp


tms2100_cpu_device::tms2100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2100_cpu_device(mconfig, TMS2100, tag, owner, clock, 8 /* o pins */, 7 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 4 /* stack levels */, 11 /* rom width */, address_map_constructor(FUNC(tms2100_cpu_device::rom_11bit), this), 7 /* ram width */, address_map_constructor(FUNC(tms2100_cpu_device::ram_7bit), this))
{ }

tms2100_cpu_device::tms2100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2170_cpu_device::tms2170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2100_cpu_device(mconfig, TMS2170, tag, owner, clock, 8, 6, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2170_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2170_cpu_device::ram_7bit), this))
{ }

tms2300_cpu_device::tms2300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2300_cpu_device(mconfig, TMS2300, tag, owner, clock, 8, 15, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2300_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2300_cpu_device::ram_7bit), this))
{ }

tms2300_cpu_device::tms2300_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms2100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms2370_cpu_device::tms2370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms2300_cpu_device(mconfig, TMS2370, tag, owner, clock, 8, 14, 6, 8, 3, 4, 11, address_map_constructor(FUNC(tms2370_cpu_device::rom_11bit), this), 7, address_map_constructor(FUNC(tms2370_cpu_device::ram_7bit), this))
{ }


// machine configs
void tms2100_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 26).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 32).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tms2100_cpu_device::create_disassembler()
{
	return std::make_unique<tms2100_disassembler>();
}


// device_start/reset
void tms2100_cpu_device::device_start()
{
	tms1100_cpu_device::device_start();

	m_prescaler = timer_alloc(FUNC(tms2100_cpu_device::prescaler_timeout), this);
	reset_prescaler();

	// zerofill
	m_ac2 = 0;
	m_ivr = 0xff;
	m_dec = 0xff;
	m_il = 0;
	m_int_pending = false;
	m_r_prev = 0;
	m_int_pin = 0;
	m_ec1_pin = 0;

	m_pb_save = 0;
	m_cb_save = 0;
	m_a_save = 0;
	m_ac2_save = 0;
	m_x_save = 0;
	m_y_save = 0;
	m_s_save = 0;
	m_sl_save = 0;
	m_o_save = 0;

	// register for savestates
	save_item(NAME(m_ac2));
	save_item(NAME(m_ivr));
	save_item(NAME(m_dec));
	save_item(NAME(m_il));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_r_prev));
	save_item(NAME(m_int_pin));
	save_item(NAME(m_ec1_pin));

	save_item(NAME(m_pb_save));
	save_item(NAME(m_cb_save));
	save_item(NAME(m_a_save));
	save_item(NAME(m_ac2_save));
	save_item(NAME(m_x_save));
	save_item(NAME(m_y_save));
	save_item(NAME(m_s_save));
	save_item(NAME(m_sl_save));
	save_item(NAME(m_o_save));

	state_add(++m_state_count, "AC2", m_ac2).formatstr("%01X"); // 9
}

void tms2100_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	m_il = 0;
	m_int_pending = false;

	// changed/added fixed instructions
	m_fixed_decode[0x09] = F_TAX;
	m_fixed_decode[0x0e] = F_TADM;
	m_fixed_decode[0x21] = F_TMA;
	m_fixed_decode[0x26] = F_TAC;
	m_fixed_decode[0x73] = F_TCA;
	m_fixed_decode[0x7b] = F_TRA;
}


// interrupt/timer
void tms2100_cpu_device::execute_set_input(int line, int state)
{
	switch (line)
	{
		case TMS2100_INPUT_LINE_INT:
			if (state && !m_int_pin)
			{
				// load decrementer if it's not counting
				if (BIT(m_r, 15) && m_dec == 0xff)
					m_dec = m_ivr;
			}
			m_int_pin = state;
			break;

		case TMS2100_INPUT_LINE_EC1:
			if (state && !m_ec1_pin)
			{
				// clock decrementer if event counter is enabled
				if (BIT(m_r, 18))
					clock_decrementer();
			}
			m_ec1_pin = state;
			break;

		default:
			break;
	}
}

void tms2100_cpu_device::clock_decrementer()
{
	if (m_dec != 0xff && --m_dec == 0)
	{
		// possible pending interrupt
		m_int_pending = true;

		// reload decrementer
		m_dec = BIT(m_r, 23) ? m_ivr : 0xff;
	}
}

TIMER_CALLBACK_MEMBER(tms2100_cpu_device::prescaler_timeout)
{
	// clock decrementer if internal timer is enabled
	if (!BIT(m_r, 18))
		clock_decrementer();
}

void tms2100_cpu_device::reset_prescaler()
{
	// prescaler divider is a mask option
	static const u16 div[4] = { 32, 128, 256, 1024 };
	attotime period = attotime::from_ticks(div[m_option_dec_div & 3] * 6, clock());
	m_prescaler->adjust(period, 0, period);
}

void tms2100_cpu_device::read_opcode()
{
	// return from interrupt
	if ((m_fixed & F_RETN) && m_il == 1)
	{
		m_il = 0;

		// restore registers
		m_pb = m_pb_save;
		m_cb = m_cb_save;
		m_a = m_a_save;
		m_ac2 = m_ac2_save;
		m_x = m_x_save;
		m_y = m_y_save;
		m_status = m_s_save;
		m_status_latch = m_sl_save;
		write_o_reg(m_o_save);
	}

	// interrupt pending (blocked during jump opcodes)
	if (m_int_pending && !(m_fixed & (F_BR | F_CALL | F_RETN)))
	{
		m_int_pending = false;

		// interrupt enabled
		if (BIT(m_r, 24))
		{
			interrupt();
			return;
		}
	}

	tms1100_cpu_device::read_opcode();
}

void tms2100_cpu_device::interrupt()
{
	// save registers
	m_pb_save = m_pb;
	m_cb_save = m_cb;
	m_a_save = m_a;
	m_ac2_save = m_ac2;
	m_x_save = m_x;
	m_y_save = m_y;
	m_s_save = m_status;
	m_sl_save = m_status_latch;
	m_o_save = m_o_index;

	// insert CALL to 0
	m_opcode = 0xc0;
	m_fixed = m_fixed_decode[m_opcode];
	m_micro = m_micro_decode[m_opcode];

	m_pb = 0;
	m_cb = 0;
	m_status = 1;
	m_il |= 1;

	standard_irq_callback(0);
}


// i/o handling
u8 tms2100_cpu_device::read_k_input()
{
	// select K/J port with R16
	return (BIT(m_r, 16) ? m_read_j() : m_read_k()) & 0xf;
}

void tms2100_cpu_device::write_r_output(u32 data)
{
	if (m_r != m_r_prev)
	{
		// R15 falling edge: reset decrementer
		if (BIT(m_r_prev, 15) && !BIT(m_r, 15))
			m_dec = 0xff;

		// R23 rising edge: load decrementer
		if (!BIT(m_r_prev, 23) && BIT(m_r, 23))
			m_dec = m_ivr;

		m_r_prev = m_r;
	}

	tms1100_cpu_device::write_r_output(data);
}


// opcode deviations
void tms2100_cpu_device::op_call()
{
	// CALL: take IL/ILC into account
	// it can only do 1 extra call inside an interrupt routine
	if (m_status)
		m_il = ((m_il << 1) | (m_il & 1)) & 7;
	tms1100_cpu_device::op_call();
}

void tms2100_cpu_device::op_retn()
{
	// RETN: take IL/ILC into account
	m_il >>= 1;
	tms1100_cpu_device::op_retn();
}

void tms2100_cpu_device::op_tax()
{
	// TAX: transfer accumulator to X register
	m_x = m_a & m_x_mask;
}

void tms2100_cpu_device::op_tra()
{
	// TRA: transfer R inputs to accumulator
}

void tms2100_cpu_device::op_tac()
{
	// TAC: transfer accumulator to AC2
	m_ac2 = m_a;
}

void tms2100_cpu_device::op_tca()
{
	// TCA: transfer AC2 to accumulator
	m_a = m_ac2;
}

void tms2100_cpu_device::op_tadm()
{
	// TADM: transfer A/D register to memory
}

void tms2100_cpu_device::op_tma()
{
	// TMA: if R17 is high, destination is IVR instead of A
	if (BIT(m_r, 17))
	{
		u8 shift = (m_y & 1) * 4;
		m_ivr = (m_ivr & ~(0xf << shift)) | (m_ram_in << shift);
		m_micro &= ~M_AUTA; // don't store in A
	}
}
