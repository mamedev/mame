// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family - known chips:
  - SM510: 2.7Kx8 ROM, 128x4 RAM(32x4 for LCD)
  - SM511: 4Kx8 ROM, 128x4 RAM(32x4 for LCD), melody controller
  - SM512: 4Kx8 ROM, 128x4 RAM(48x4 for LCD), melody controller

  Other chips that may be in the same family, investigate more when one of
  them needs to get emulated: SM500, SM530/31, SM4A, SM3903, ..

  References:
  - 1990 Sharp Microcomputers Data Book
  - 1996 Sharp Microcomputer Databook

  TODO:
  - proper support for LFSR program counter in debugger
  - callback for lcd screen as MAME bitmap (when needed)
  - LCD bs pin blink mode via Y register (0.5s off, 0.5s on)
  - LB/SBM is correct?
  - SM511 unknown opcodes

*/

#include "sm510.h"
#include "debugger.h"


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	SM510_PC=1, SM510_ACC, SM510_BL, SM510_BM,
	SM510_C, SM510_W
};

void sm510_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_read_ba.resolve_safe(1);
	m_read_b.resolve_safe(1);
	m_write_s.resolve_safe();
	m_write_r.resolve_safe();

	m_write_sega.resolve_safe();
	m_write_segb.resolve_safe();
	m_write_segbs.resolve_safe();
	m_write_segc.resolve_safe();

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_acc = 0;
	m_bl = 0;
	m_bm = 0;
	m_c = 0;
	m_skip = false;
	m_w = 0;
	m_r = 0;
	m_div = 0;
	m_1s = false;
	m_k_active = false;
	m_l = 0;
	m_x = 0;
	m_y = 0;
	m_bp = false;
	m_bc = false;
	m_halt = false;
	m_melody_rd = 0;
	m_melody_step_count = 0;
	m_melody_duty_count = 0;
	m_melody_duty_index = 0;
	m_melody_address = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_param));
	save_item(NAME(m_acc));
	save_item(NAME(m_bl));
	save_item(NAME(m_bm));
	save_item(NAME(m_c));
	save_item(NAME(m_skip));
	save_item(NAME(m_w));
	save_item(NAME(m_r));
	save_item(NAME(m_div));
	save_item(NAME(m_1s));
	save_item(NAME(m_k_active));
	save_item(NAME(m_l));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_bp));
	save_item(NAME(m_bc));
	save_item(NAME(m_halt));
	save_item(NAME(m_melody_rd));
	save_item(NAME(m_melody_step_count));
	save_item(NAME(m_melody_duty_count));
	save_item(NAME(m_melody_duty_index));
	save_item(NAME(m_melody_address));

	// register state for debugger
	state_add(SM510_PC,  "PC",  m_pc).formatstr("%04X");
	state_add(SM510_ACC, "ACC", m_acc).formatstr("%01X");
	state_add(SM510_BL,  "BL",  m_bl).formatstr("%01X");
	state_add(SM510_BM,  "BM",  m_bm).formatstr("%01X");
	state_add(SM510_C,   "C",   m_c).formatstr("%01X");
	state_add(SM510_W,   "W",   m_w).formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_c).formatstr("%1s").noshow();

	m_icountptr = &m_icount;

	// init peripherals
	init_divider();
	init_lcd_driver();
	init_melody();
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm510_base_device::device_reset()
{
	m_skip = false;
	m_halt = false;
	m_op = m_prev_op = 0;
	do_branch(3, 7, 0);
	m_prev_pc = m_pc;

	// lcd is on (Bp on, BC off, bs(y) off)
	m_bp = true;
	m_bc = false;
	m_y = 0;

	m_r = 0;
	m_write_r(0, 0, 0xff);
	m_melody_rd &= ~1;
}



//-------------------------------------------------
//  lcd driver
//-------------------------------------------------

inline UINT16 sm510_base_device::get_lcd_row(int column, UINT8* ram)
{
	// output 0 if lcd blackpate/bleeder is off, or in case row doesn't exist
	if (ram == NULL || m_bc || !m_bp)
		return 0;

	UINT16 rowdata = 0;
	for (int i = 0; i < 0x10; i++)
		rowdata |= (ram[i] >> column & 1) << i;

	return rowdata;
}

TIMER_CALLBACK_MEMBER(sm510_base_device::lcd_timer_cb)
{
	// 4 columns
	for (int h = 0; h < 4; h++)
	{
		// 16 segments per row from upper part of RAM
		m_write_sega(h | SM510_PORT_SEGA, get_lcd_row(h, m_lcd_ram_a), 0xffff);
		m_write_segb(h | SM510_PORT_SEGB, get_lcd_row(h, m_lcd_ram_b), 0xffff);
		m_write_segc(h | SM510_PORT_SEGC, get_lcd_row(h, m_lcd_ram_c), 0xffff);

		// bs output from L/X and Y regs
		UINT8 bs = (m_l >> h & 1) | ((m_x*2) >> h & 2);
		m_write_segbs(h | SM510_PORT_SEGBS, (m_bc || !m_bp) ? 0 : bs, 0xffff);
	}

	// schedule next timeout
	m_lcd_timer->adjust(attotime::from_ticks(0x200, unscaled_clock()));
}

void sm510_base_device::init_lcd_driver()
{
	// note: in reality, this timer runs at high frequency off the main divider, strobing one segment at a time
	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sm510_base_device::lcd_timer_cb), this));
	m_lcd_timer->adjust(attotime::from_ticks(0x200, unscaled_clock())); // 64hz default
}



//-------------------------------------------------
//  melody controller
//-------------------------------------------------

void sm510_base_device::clock_melody()
{
	if (!m_melody_rom)
		return;

	// tone cycle table (SM511/SM512 datasheet fig.5)
	// cmd 0 = cmd, 1 = stop, > 13 = illegal(unknown)
	static const UINT8 lut_tone_cycles[4*16] =
	{
		0, 0, 7, 8, 8, 9, 9, 10,11,11,12,13,14,14, 7*2, 8*2,
		0, 0, 8, 8, 9, 9, 10,11,11,12,13,13,14,15, 8*2, 8*2,
		0, 0, 8, 8, 9, 9, 10,10,11,12,12,13,14,15, 8*2, 8*2,
		0, 0, 8, 9, 9, 10,10,11,11,12,13,14,14,15, 8*2, 9*2
	};

	UINT8 cmd = m_melody_rom[m_melody_address] & 0x3f;
	UINT8 out = 0;

	// clock duty cycle if tone is active
	if ((cmd & 0xf) > 1)
	{
		out = m_melody_duty_index & m_melody_rd & 1;
		m_melody_duty_count++;
		int index = m_melody_duty_index << 4 | (cmd & 0xf);
		int shift = ~cmd >> 4 & 1; // OCT

		if (m_melody_duty_count >= (lut_tone_cycles[index] << shift))
		{
			m_melody_duty_count = 0;
			m_melody_duty_index = (m_melody_duty_index + 1) & 3;
		}
	}
	else if ((cmd & 0xf) == 1)
	{
		// rest tell signal
		m_melody_rd |= 2;
	}

	// clock time base on F8(d7)
	if ((m_div & 0x7f) == 0)
	{
		UINT8 mask = (cmd & 0x20) ? 0x1f : 0x0f;
		m_melody_step_count = (m_melody_step_count + 1) & mask;

		if (m_melody_step_count == 0)
			m_melody_address++;
	}

	// output to R pin
	if (out != m_r)
	{
		m_write_r(0, out, 0xff);
		m_r = out;
	}
}

void sm510_base_device::init_melody()
{
	if (!m_melody_rom)
		return;

	// verify melody rom
	for (int i = 0; i < 0x100; i++)
	{
		UINT8 data = m_melody_rom[i];
		if (data & 0xc0 || (data & 0x0f) > 13)
			logerror("%s unknown melody ROM data $%02X at $%02X\n", tag(), data, i);
	}
}



//-------------------------------------------------
//  interrupt/divider
//-------------------------------------------------

bool sm510_base_device::wake_me_up()
{
	// in halt mode, wake up after 1S signal or K input
	if (m_k_active || m_1s)
	{
		// note: official doc warns that Bl/Bm and the stack are undefined
		// after waking up, but we leave it unchanged
		m_halt = false;
		do_branch(1, 0, 0);

		standard_irq_callback(0);
		return true;
	}
	else
		return false;
}

void sm510_base_device::execute_set_input(int line, int state)
{
	if (line != SM510_INPUT_LINE_K)
		return;

	// set K input lines active state
	m_k_active = (state != 0);
}

TIMER_CALLBACK_MEMBER(sm510_base_device::div_timer_cb)
{
	m_div = (m_div + 1) & 0x7fff;

	// 1S signal on overflow(falling edge of f1)
	if (m_div == 0)
		m_1s = true;

	clock_melody();
}

void sm510_base_device::init_divider()
{
	m_div_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sm510_base_device::div_timer_cb), this));
	m_div_timer->adjust(attotime::from_ticks(1, unscaled_clock()), 0, attotime::from_ticks(1, unscaled_clock()));
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm510_base_device::increment_pc()
{
	// PL(program counter low 6 bits) is a simple LFSR: newbit = (bit0==bit1)
	// PU,PM(high bits) specify page, PL specifies steps within page
	int feed = ((m_pc >> 1 ^ m_pc) & 1) ? 0 : 0x20;
	m_pc = feed | (m_pc >> 1 & 0x1f) | (m_pc & ~0x3f);
}

void sm510_base_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;

		if (m_halt && !wake_me_up())
		{
			// got nothing to do
			m_icount = 0;
			return;
		}

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_byte(m_pc);
		increment_pc();
		get_opcode_param();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			execute_one();
	}
}
