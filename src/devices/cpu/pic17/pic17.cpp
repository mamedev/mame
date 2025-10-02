// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Microchip PIC17CXXX High-Performance 8-bit MCU family

    Common features of this family include:
    — Four oscillator options (maximum rate of 33 MHz except on
      PIC17C42)
    — 2048 to 16,384 16-bit words of internal EPROM program memory
      in microcontroller modes (may be protected; certain models use
      mask ROM instead); special “long” table writes are used to
      program EPROM when Vpp is applied
    — 64K 16-bit words of external address space in microprocessor
      mode, readable and writable through TABPTR and TABLAT
    — 232 or more bytes of internal data RAM
    — Two registers for indirect access to entire register file, with
      optional post-increment or post-decrement for each
    — Independent bank selection for peripheral SFRs and general-
      purpose RAM (up to 16 banks are selectable for each, though no
      more than 8 of the former and 4 of the latter were implemented)
    — All instructions execute in a single cycle except for program
      transfers and table reads/writes
    – Signed overflow flag and decimal adjustment for addition
    — 8 x 8 unsigned multiplication in hardware (except PIC17C42)
    — Single-bit instructions can manipulate any addressable register
    — MOVFP and MOVPF can transfer data between registers without
      touching the working register
    — CALL and GOTO allow efficient program transfers within 8K page
    — 16-level on-chip program stack (non-addressable)
    — Multiple 8-bit and 16-bit on-chip timer/counters, with 16-bit
      input capture and 10-bit PWM output
    — USART serial I/O with dedicated 8-bit baud rate generator
    — Watchdog timer with internal RC oscillator
    — SLEEP mode with wake-up upon interrupt

    Notable architectural quirks:
    — PCH is copied to PCLATH whenever PCL is read unless the
      operation on that register is of the read-modify-write type.
    — MOVWF, CLRF, SETF, NEGW and DAW read the previous value of
      the destination register before discarding and replacing it.
    — A quirk of instruction decoding causes RETURN, SLEEP, CLRWDT
      and RETFIE to perform dummy reads from PCL, PCLATH, ALUSTA
      and T0STA respectively. This means that RETURN also copies PCH
      to PCLATH before it pops the stack.
    — The internal workings of the stack are fully specified, even
      though no instruction can push or pop values from the stack,
      adjust the stack pointer or even sense its current level
      (aside from the crude heuristic of the STKAV flag).
    — TABLWT and TABLRD access different implementations of TABLAT.
      Neither half of a data word latched from external memory by
      TABLRD will be written back by a following TABLWT. To ensure
      correct data is present on both halves of the bus, TLWT must
      precede TABLWT to refresh the alternate half of TABLAT.
    — “Long” writes to internal EPROM (not emulated) can be aborted
      by the setting of any interrupt flag, whether enabled or not.
    — SFR bank 15 is “reserved for Microchip use.”

**********************************************************************/

#include "emu.h"
#include "pic17.h"
#include "pic17d.h"

ALLOW_SAVE_TYPE(pic17_cpu_device::exec_phase);

pic17_cpu_device::pic17_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1, address_map_constructor(FUNC(pic17_cpu_device::program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 12, 0, data_map)
	, m_rom_size(rom_size)
	, m_mode(mode::EXTENDED_MICROCONTROLLER)
	, m_pc(0)
	, m_ppc(0)
	, m_paddr(0)
	, m_raddr(0)
	, m_ir(0)
	, m_tmp(0)
	, m_execphase(exec_phase::Q1)
	, m_execflags(0)
	, m_icount(0)
	, m_pclath(0)
	, m_wreg(0)
	, m_alusta(0)
	, m_cpusta(0x3c)
	, m_intsta(0)
	, m_intsample(0)
	, m_tblptr(0)
	, m_tablat(0)
	, m_tlwt(0)
	, m_prod(0)
	, m_fsr{0, 0}
	, m_bsr(0)
	, m_stkptr(0)
	, m_t0sta(0)
	, m_tmr0(0)
	, m_ps(0)
{
	std::fill_n(&m_stack[0], 16, 0);
}

void pic17_cpu_device::program_map(address_map &map)
{
	if (m_mode != mode::MICROPROCESSOR)
	{
		map(0x0000, m_rom_size - 1).rom().region(DEVICE_SELF, 0);
		//map(0xfe00, 0xffff).rom().region("config", 0);
	}
}

void pic17_cpu_device::core_data_map(address_map &map)
{
	// Unbanked registers (INDF0 and INDF1 are not physical registers)
	map(0x000, 0x000).lr8([]() { return 0; }, "0").nopw();
	map(0x001, 0x001).rw(FUNC(pic17_cpu_device::fsr0_r), FUNC(pic17_cpu_device::fsr0_w));
	map(0x002, 0x002).rw(FUNC(pic17_cpu_device::pcl_r), FUNC(pic17_cpu_device::pcl_w));
	map(0x003, 0x003).rw(FUNC(pic17_cpu_device::pclath_r), FUNC(pic17_cpu_device::pclath_w));
	map(0x004, 0x004).rw(FUNC(pic17_cpu_device::alusta_r), FUNC(pic17_cpu_device::alusta_w));
	map(0x005, 0x005).rw(FUNC(pic17_cpu_device::t0sta_r), FUNC(pic17_cpu_device::t0sta_w));
	map(0x006, 0x006).rw(FUNC(pic17_cpu_device::cpusta_r), FUNC(pic17_cpu_device::cpusta_w));
	map(0x007, 0x007).rw(FUNC(pic17_cpu_device::intsta_r), FUNC(pic17_cpu_device::intsta_w));
	map(0x008, 0x008).lr8([]() { return 0; }, "0").nopw();
	map(0x009, 0x009).rw(FUNC(pic17_cpu_device::fsr1_r), FUNC(pic17_cpu_device::fsr1_w));
	map(0x00a, 0x00a).rw(FUNC(pic17_cpu_device::wreg_r), FUNC(pic17_cpu_device::wreg_w));
	map(0x00b, 0x00b).rw(FUNC(pic17_cpu_device::tmr0l_r), FUNC(pic17_cpu_device::tmr0l_w));
	map(0x00c, 0x00c).rw(FUNC(pic17_cpu_device::tmr0h_r), FUNC(pic17_cpu_device::tmr0h_w));
	map(0x00d, 0x00d).rw(FUNC(pic17_cpu_device::tblptrl_r), FUNC(pic17_cpu_device::tblptrl_w));
	map(0x00e, 0x00e).rw(FUNC(pic17_cpu_device::tblptrh_r), FUNC(pic17_cpu_device::tblptrh_w));
	map(0x00f, 0x00f).rw(FUNC(pic17_cpu_device::bsr_r), FUNC(pic17_cpu_device::bsr_w));
	map(0x018, 0x018).rw(FUNC(pic17_cpu_device::prodl_r), FUNC(pic17_cpu_device::prodl_w)); // not on PIC17C42
	map(0x019, 0x019).rw(FUNC(pic17_cpu_device::prodh_r), FUNC(pic17_cpu_device::prodh_w)); // not on PIC17C42
}

std::unique_ptr<util::disasm_interface> pic17_cpu_device::create_disassembler()
{
	return std::make_unique<pic17_disassembler>();
}

device_memory_interface::space_config_vector pic17_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

u8 pic17_cpu_device::pcl_r()
{
	// Note that this side effect is implicitly performed by the RETURN instruction
	if (!machine().side_effects_disabled() && (m_execflags & REGWT) == 0)
		m_pclath = m_pc >> 8;
	return m_pc & 0x00ff;
}

void pic17_cpu_device::pcl_w(u8 data)
{
	m_pc = u8(m_pclath) << 8 | data;
	m_execflags |= FORCENOP;
}

void pic17_cpu_device::debug_set_pc(u16 data)
{
	m_pc = m_ppc = data;
	m_ir = m_cache.read_word(m_pc);
}

u8 pic17_cpu_device::pclath_r()
{
	return m_pclath;
}

void pic17_cpu_device::pclath_w(u8 data)
{
	m_pclath = data;
}

u8 pic17_cpu_device::wreg_r()
{
	return m_wreg;
}

void pic17_cpu_device::wreg_w(u8 data)
{
	m_wreg = data;
}

u8 pic17_cpu_device::alusta_r()
{
	return m_alusta;
}

void pic17_cpu_device::alusta_w(u8 data)
{
	m_alusta = data;
}

u8 pic17_cpu_device::cpusta_r()
{
	// STKAV is read as 0 when stack is full
	if (m_stkptr == 0xf)
		return m_cpusta & 0xdf;
	else
		return m_cpusta;
}

void pic17_cpu_device::cpusta_w(u8 data)
{
	// Only GLINTD is writable
	if (BIT(data, 4))
	{
		m_cpusta |= 0x10;

		// PIC17C42 lacks this failsafe, allowing an interrupt to be taken right after an instruction that sets GLINTD
		m_execflags &= ~INTRPT;
	}
	else
		m_cpusta &= 0xef;
}

u8 pic17_cpu_device::intsta_r()
{
	return m_intsta;
}

void pic17_cpu_device::intsta_w(u8 data)
{
	// PEIF is read-only
	m_intsta = (m_intsta & 0x80) | (data & 0x7f);
}

void pic17_cpu_device::int_edge(bool rising)
{
	if (BIT(m_t0sta, 7) == rising)
		m_intsta |= 0x10;
}

void pic17_cpu_device::t0cki_edge(bool rising)
{
	if (BIT(m_t0sta, 6) == rising)
	{
		m_intsta |= 0x40;
		if (!BIT(m_t0sta, 5))
			increment_tmr0();
	}
}

void pic17_cpu_device::set_peif(bool state)
{
	if (state)
		m_intsta |= 0x80;
	else
	{
		m_intsta &= 0x7f;
		if (((m_intsta >> 4) & m_intsta) == 0)
			m_execflags &= ~INTRPT;
	}
}

u8 pic17_cpu_device::t0sta_r()
{
	return m_t0sta;
}

void pic17_cpu_device::t0sta_w(u8 data)
{
	m_t0sta = data & 0xfe;
}

u8 pic17_cpu_device::tmr0l_r()
{
	return m_tmr0 & 0x00ff;
}

void pic17_cpu_device::tmr0l_w(u8 data)
{
	m_tmr0 = u8(data) | (m_tmr0 & 0xff00);
	m_ps = 0;
}

u8 pic17_cpu_device::tmr0h_r()
{
	return m_tmr0 >> 8;
}

void pic17_cpu_device::tmr0h_w(u8 data)
{
	m_tmr0 = u8(data) << 8 | (m_tmr0 & 0x00ff);
	m_ps = 0;
}

void pic17_cpu_device::increment_tmr0()
{
	// Prescaler is an 8-bit ripple counter
	++m_ps;

	// Tap the prescaler output (PSOUT)
	if ((m_ps & make_bitmask<u8>((m_t0sta & 0x1e) >> 1)) == 0)
	{
		// Set T0IF on overflow
		if (m_tmr0 == 0xffff)
			m_intsta |= 0x20;
		++m_tmr0;
	}
}

u8 pic17_cpu_device::fsr0_r()
{
	return m_fsr[0];
}

void pic17_cpu_device::fsr0_w(u8 data)
{
	m_fsr[0] = data;
}

u8 pic17_cpu_device::fsr1_r()
{
	return m_fsr[1];
}

void pic17_cpu_device::fsr1_w(u8 data)
{
	m_fsr[1] = data;
}

u8 pic17_cpu_device::bsr_r()
{
	return m_bsr;
}

void pic17_cpu_device::bsr_w(u8 data)
{
	m_bsr = data;
}

u8 pic17_cpu_device::tblptrl_r()
{
	return m_tblptr & 0x00ff;
}

void pic17_cpu_device::tblptrl_w(u8 data)
{
	m_tblptr = (m_tblptr & 0xff00) | data;
}

u8 pic17_cpu_device::tblptrh_r()
{
	return m_tblptr >> 8;
}

void pic17_cpu_device::tblptrh_w(u8 data)
{
	m_tblptr = (m_tblptr & 0x00ff) | u16(data) << 8;
}

u8 pic17_cpu_device::prodl_r()
{
	return m_prod & 0x00ff;
}

void pic17_cpu_device::prodl_w(u8 data)
{
	m_prod = (m_prod & 0xff00) | data;
}

u8 pic17_cpu_device::prodh_r()
{
	return m_prod >> 8;
}

void pic17_cpu_device::prodh_w(u8 data)
{
	m_prod = (m_prod & 0x00ff) | u16(data) << 8;
}

void pic17_cpu_device::set_skip()
{
	m_execflags |= SKIP;
}

void pic17_cpu_device::clear_watchdog_timer()
{
	//logerror("%04X: Watchdog timer cleared\n", m_ppc);
}

u16 pic17_cpu_device::banked_register(u8 r)
{
	// INDF0 and INDF1 substitute contents of FSR0/FSR1
	if ((r & 0xf7) == 0)
	{
		// Post auto-decrement, post auto-increment or no change
		u8 fsmode = m_alusta >> (BIT(r, 3) ? 6 : 4);
		if (BIT(fsmode, 1))
			r = m_fsr[BIT(r, 3)];
		else if (BIT(fsmode, 0))
			r = m_fsr[BIT(r, 3)]++;
		else
			r = m_fsr[BIT(r, 3)]--;
	}

	if (r >= 0x20)
	{
		// Addresses from 20h to FFh use GPR banks
		return u16(m_bsr & 0xf0) << 4 | r;
	}
	else if ((r & 0x18) == 0x10)
	{
		// Addresses from 10h to 17h use SFR banks
		return u16(m_bsr & 0x0f) << 8 | r;
	}
	else
	{
		// Other addresses are unbanked
		return r;
	}
}

void pic17_cpu_device::stack_push(u16 addr)
{
	m_stack[m_stkptr] = addr;

	// Check for stack overflow
	if (m_stkptr == 0xf)
	{
		logerror("%04X: Stack overflow\n", m_pc);

		// STKAV remains cleared until reset
		m_cpusta &= 0xdf;
	}

	m_stkptr = (m_stkptr + 1) & 0xf;
}

u16 pic17_cpu_device::stack_pop()
{
	m_stkptr = (m_stkptr - 1) & 0xf;
	return m_stack[m_stkptr];
}

u16 pic17_cpu_device::interrupt_vector()
{
	u8 active_ints = m_intsample & m_intsta;
	if (BIT(active_ints, 0))
	{
		// Interrupt on RA0/INT pin edge
		standard_irq_callback(0, m_pc);
		m_intsta &= 0xef; // INTF is cleared
		return 0x0008;
	}
	else if (BIT(active_ints, 1))
	{
		// Interrupt on TMR0 overflow
		standard_irq_callback(1, m_pc);
		m_intsta &= 0xdf; // T0IF is cleared
		return 0x0010;
	}
	else if (BIT(active_ints, 2))
	{
		// Interrupt on RA1/T0CKI pin edge
		standard_irq_callback(2, m_pc);
		m_intsta &= 0xbf; // T0CKIF is cleared
		return 0x0018;
	}
	else if (BIT(active_ints, 3))
	{
		// Peripheral interrupts (not further distinguished here, and PEIF is *not* cleared)
		standard_irq_callback(3, m_pc);
		return 0x0020;
	}
	else
	{
		// This may happen when interrupt enable flags are cleared at the wrong moment
		logerror("%04X: Spurious interrupt taken\n", m_pc);
		return 0x0000;
	}
}

void pic17_cpu_device::set_zero(bool z)
{
	if (z)
		m_alusta |= 0x04;
	else
		m_alusta &= 0xfb;
}

void pic17_cpu_device::set_carry(bool c)
{
	if (c)
		m_alusta |= 0x01;
	else
		m_alusta &= 0xfe;
}

void pic17_cpu_device::add_with_carry(u8 augend, bool cin)
{
	// Carry out is inverted borrow for subtraction
	bool cout = m_tmp + augend + cin >= 0x100;
	bool dc = (m_tmp & 0x0f) + (augend & 0x0f) + cin >= 0x10;
	s16 sum = s8(m_tmp) + s8(augend) + cin;
	m_tmp = sum & 0x00ff;

	// Set OV, Z, DC and C flags
	m_alusta &= 0xf0;
	if (sum < -128 || sum >= 128)
		m_alusta |= 0x08;
	if (m_tmp == 0)
		m_alusta |= 0x04;
	if (dc)
		m_alusta |= 0x02;
	if (cout)
		m_alusta |= 0x01;
}

void pic17_cpu_device::decimal_adjust()
{
	u16 result = m_wreg;

	if (BIT(m_alusta, 1) || (result & 0x0f) > 0x09)
	{
		result += 0x06;
		if (result >= 0x100)
			m_alusta |= 0x01;
	}
	if (BIT(m_alusta, 0) || (result & 0xf0) > 0x90)
	{
		result += 0x60;
		if (result >= 0x100)
			m_alusta |= 0x01;
	}
}

void pic17_cpu_device::q1_decode()
{
	m_raddr = 0x000;
	if (m_ir >= 0xc000)
	{
		// GOTO and CALL write to PCL (after setting PCLATH)
		m_raddr = 0x002;
		m_execflags |= REGWT;
	}
	else if (m_ir >= 0xb000)
	{
		if ((m_ir & 0x0f00) == 0x700)
		{
			// LCALL is a literal write to PCL
			m_raddr = 0x002;
			m_execflags |= REGWT;
		}
		else if ((m_ir & 0x0f00) == 0x800 || (m_ir & 0x0e00) == 0xa00)
		{
			// MOVLB and MOVLR operate on BSR
			m_raddr = 0x00f;
			m_execflags |= REGWT;
		}
	}
	else if ((m_ir & 0xe000) == 0x4000)
	{
		// MOVPF reads register in 'p' field first
		m_raddr = banked_register((m_ir & 0x1f00) >> 8);
	}
	else
	{
		if (m_ir != 0x0000)
			m_raddr = banked_register(m_ir & 0x00ff);
		if (m_ir < 0x3000)
		{
			// Byte-oriented register operations with d = 1
			// CLRF, SETF, NEGW, DAW always copy their result to the register, even when s = 0
			if (m_ir >= 0x2800 || BIT(m_ir, 8))
				m_execflags |= REGWT;
		}
		else if ((m_ir & 0xf800) == 0x3800 || (m_ir & 0xf000) == 0x8000)
		{
			// Bitwise read-modify-write instructions
			m_execflags |= REGWT;
		}
		else if ((m_ir & 0xf400) == 0xa000)
		{
			// TABLRD/TBRD
			m_execflags |= REGWT;
		}
	}
}

void pic17_cpu_device::q2_read()
{
	if (m_ir >= 0xb000)
	{
		// Read literal data only
		m_tmp = m_ir & 0x00ff;

		// Set PCLATH for GOTO/CALL
		if (m_ir >= 0xc000)
			m_pclath = (m_pc & 0xe000) >> 8 | (m_ir & 0x1f00) >> 8;
	}
	else if ((m_ir & 0xa400) == 0xa000)
	{
		// Read TABLATH or TABLATL
		if (BIT(m_ir, 9))
			m_tmp = m_tablat >> 8;
		else
			m_tmp = m_tablat & 0x00ff;
	}
	else if (m_ir != 0x0000)
	{
		// Read register (dummy read for some instructions)
		m_tmp = m_data.read_byte(m_raddr);

		if ((m_ir & 0xc000) == 0x4000)
		{
			// Swap out address for MOVFP/MOVPF
			u8 f = m_ir & 0x00ff;
			u8 p = (m_ir & 0x1f00) >> 8;
			if (p != f)
				m_raddr = banked_register(BIT(m_ir, 13) ? p : f);

			// Change from read to write mode
			m_execflags |= REGWT;
		}
	}
}

void pic17_cpu_device::q3_execute()
{
	if (m_ir >= 0xe000)
		stack_push(m_pc);
	else switch (m_ir & 0xff00)
	{
	case 0x0000:
		switch (m_ir & 0x00ff)
		{
		case 0x00: // NOP
			break;

		case 0x02: // RETURN
			m_pc = stack_pop();
			m_execflags |= FORCENOP;
			break;

		case 0x03: // SLEEP
			clear_watchdog_timer();
			m_cpusta |= 0x08; // set TO
			m_cpusta &= 0xfb; // clear PD
			m_execflags |= SLEEP;
			break;

		case 0x04: // CLRWDT
			clear_watchdog_timer();
			break;

		case 0x05: // RETFIE
			m_pc = stack_pop();
			m_execflags |= FORCENOP;

			// Clear global interrupt disable bit
			m_cpusta &= 0xef;
			break;
		}
		break;

	case 0x0100: // MOVWF
		m_tmp = m_wreg;
		break;

	case 0x0200: case 0x0300: // SUBWFB
		add_with_carry(~m_wreg, BIT(m_alusta, 0));
		break;

	case 0x0400: case 0x0500: // SUBWF
	case 0xb200: // SUBLW
		add_with_carry(~m_wreg, true);
		break;

	case 0x0600: case 0x0700: // DECF
		add_with_carry(0xff, false);
		break;

	case 0x0800: case 0x0900: // IORWF
	case 0xb300: // IORLW
		m_tmp |= m_wreg;
		set_zero(m_tmp);
		break;

	case 0x0a00: case 0x0b00: // ANDWF
	case 0xb500: // ANDLW
		m_tmp &= m_wreg;
		set_zero(m_tmp);
		break;

	case 0x0c00: case 0x0d00: // XORWF
	case 0xb400: // XORLW
		m_tmp ^= m_wreg;
		set_zero(m_tmp);
		break;

	case 0x0e00: case 0x0f00: // ADDWF
	case 0xb100: // ADDLW
		add_with_carry(m_wreg, false);
		break;

	case 0x1000: case 0x1100: // ADDWFC
		add_with_carry(m_wreg, BIT(m_alusta, 0));
		break;

	case 0x1200: case 0x1300: // COMF
		m_tmp ^= 0xff;
		set_zero(m_tmp);
		break;

	case 0x1400: case 0x1500: // INCF
		add_with_carry(1, false);
		break;

	case 0x1600: case 0x1700: // DECFSZ
		--m_tmp;
		if (m_tmp == 0)
			set_skip();
		break;

	case 0x1800: case 0x1900: // RRCF
		set_carry(BIT(std::exchange(m_tmp, (m_tmp >> 1) | (m_alusta << 7)), 0));
		break;

	case 0x1a00: case 0x1b00: // RLCF
		set_carry(BIT(std::exchange(m_tmp, (m_tmp << 1) | (m_alusta & 0x01)), 7));
		break;

	case 0x1c00: case 0x1d00: // SWAPF
		m_tmp = (m_tmp << 4) | (m_tmp >> 4);
		break;

	case 0x1e00: case 0x1f00: // INCFSZ
		++m_tmp;
		if (m_tmp == 0)
			set_skip();
		break;

	case 0x2000: case 0x2100: // RRNCF
		m_tmp = (m_tmp >> 1) | (m_tmp << 7);
		break;

	case 0x2200: case 0x2300: // RLNCF
		m_tmp = (m_tmp << 1) | (m_tmp >> 7);
		break;

	case 0x2400: case 0x2500: // INFSNZ
		++m_tmp;
		if (m_tmp != 0)
			set_skip();
		break;

	case 0x2600: case 0x2700: // DCFSNZ
		--m_tmp;
		if (m_tmp != 0)
			set_skip();
		break;

	case 0x2800: case 0x2900: // CLRF
		m_tmp = 0;
		break;

	case 0x2a00: case 0x2b00: // SETF
		m_tmp = 0xff;
		break;

	case 0x2c00: case 0x2d00: // NEGW
		m_tmp = 0;
		add_with_carry(~m_wreg, true);
		break;

	case 0x2e00: case 0x2f00: // DAW
		decimal_adjust();
		break;

	case 0x3000: // CPFSLT (unsigned comparison)
		if (m_tmp < m_wreg)
			set_skip();
		break;

	case 0x3100: // CPFSEQ
		if (m_tmp == m_wreg)
			set_skip();
		break;

	case 0x3200: // CPFSGT (unsigned comparison)
		if (m_tmp > m_wreg)
			set_skip();
		break;

	case 0x3300: // TSTFSZ
		if (m_tmp == 0)
			set_skip();
		break;

	case 0x3400: // MULWF
	case 0xbc00: // MULLW
		m_prod = u16(m_wreg) * u16(m_tmp);
		break;

	case 0x3800: case 0x3900: case 0x3a00: case 0x3b00: case 0x3c00: case 0x3d00: case 0x3e00: case 0x3f00: // BTG
		m_tmp ^= 1 << ((m_ir & 0x0700) >> 8);
		break;

	case 0x8000: case 0x8100: case 0x8200: case 0x8300: case 0x8400: case 0x8500: case 0x8600: case 0x8700: // BSF
		m_tmp |= 1 << ((m_ir & 0x0700) >> 8);
		break;

	case 0x8800: case 0x8900: case 0x8a00: case 0x8b00: case 0x8c00: case 0x8d00: case 0x8e00: case 0x8f00: // BCF
		m_tmp &= ~(1 << ((m_ir & 0x0700) >> 8));
		break;

	case 0x9000: case 0x9100: case 0x9200: case 0x9300: case 0x9400: case 0x9500: case 0x9600: case 0x9700: // BTFSS
		if ((m_tmp & (1 << ((m_ir & 0x0700) >> 8))) != 0)
			set_skip();
		break;

	case 0x9800: case 0x9900: case 0x9a00: case 0x9b00: case 0x9c00: case 0x9d00: case 0x9e00: case 0x9f00: // BTFSC
		if ((m_tmp & (1 << ((m_ir & 0x0700) >> 8))) == 0)
			set_skip();
		break;

	case 0xb600: // RETLW
		m_pc = stack_pop();
		m_execflags |= FORCENOP;
		break;

	case 0xb700: // LCALL
		stack_push(m_pc);
		break;

	case 0xb800: // MOVLB
		m_tmp = (m_bsr & 0xf0) | (m_tmp & 0x0f);
		break;

	case 0xba00: case 0xbb00: // MOVLR
		m_tmp = (m_tmp & 0xf0) | (m_bsr & 0x0f);
		break;
	}
}

void pic17_cpu_device::q4_write()
{
	if ((m_execflags & REGWT) != 0)
	{
		m_data.write_byte(m_raddr, m_tmp);
		m_execflags &= ~REGWT;
		if ((m_ir & 0xfc00) == 0xa800)
		{
			m_execflags |= TABLRD;
			if (BIT(m_ir, 8))
				m_execflags |= TBLPTRI;
		}
	}
	else if ((m_ir & 0xf400) == 0xa400)
	{
		// Set TABLATH or TABLATL for TABLWT/TLWT
		if (BIT(m_ir, 9))
		{
			m_tablat = (m_tablat & 0x00ff) | u16(m_tmp) << 8;
			m_tlwt = (m_tlwt & 0x00ff) | u16(m_tmp) << 8;
		}
		else
		{
			m_tablat = (m_tablat & 0xff00) | m_tmp;
			m_tlwt = (m_tlwt & 0xff00) | m_tmp;
		}
		if (BIT(m_ir, 11))
		{
			m_execflags |= TABLWT;
			if (BIT(m_ir, 8))
				m_execflags |= TBLPTRI;
		}
	}

	// MOVLW, ADDLW, SUBLW, IORLW, XORLW, ANDLW, RETLW always store result in W; other ALU instructions do so when d = 0
	if ((m_ir >= 0x0200 && m_ir < 0x3000 && !BIT(m_ir, 8)) || (m_ir >= 0xb000 && m_ir < 0xb700))
		m_wreg = m_tmp;
}

void pic17_cpu_device::increment_timers()
{
	// TMR0 increment is inhibited by count writes to TMR0L
	if (BIT(m_t0sta, 5) && ((m_execflags & REGWT) == 0 || m_raddr != 0x00b))
		increment_tmr0();
}

void pic17_cpu_device::execute_run()
{
	switch (m_execphase)
	{
		while (true)
		{
			[[fallthrough]];
		case exec_phase::Q1:
			if ((m_execflags & SLEEP) != 0)
			{
				if (((m_intsta >> 4) & m_intsta) != 0)
					m_execflags &= ~SLEEP;
				else
				{
					// Do nothing until time to wake up
					debugger_wait_hook();
					m_icount = 0;
					m_execphase = exec_phase::Q1;
					return;
				}
			}
			if (m_execflags == 0)
			{
				m_ppc = m_pc;
				debugger_instruction_hook(m_pc);
				++m_pc;
				q1_decode();
			}
			else if ((m_execflags & (TABLRD | TABLWT)) == 0)
			{
				if ((m_execflags & SKIP) != 0)
					++m_pc;
				m_ir = 0;
				m_execflags &= ~(SKIP | FORCENOP);
			}
			if (--m_icount <= 0)
			{
				m_execphase = exec_phase::Q2;
				return;
			}
			[[fallthrough]];

		case exec_phase::Q2:
			if ((m_execflags & INTRPT) == 0)
				m_intsample = BIT(m_cpusta, 4) ? 0 : m_intsta >> 4;
			if ((m_execflags & (TABLRD | TABLWT)) != 0)
			{
				m_paddr = m_tblptr;
				if ((m_execflags & TBLPTRI) != 0)
					++m_tblptr;
			}
			else
			{
				m_paddr = m_pc;
				q2_read();
			}
			if (--m_icount <= 0)
			{
				m_execphase = exec_phase::Q3;
				return;
			}
			[[fallthrough]];

		case exec_phase::Q3:
			if ((m_execflags & (TABLRD | TABLWT)) == 0)
			{
				if ((m_execflags & INTRPT) != 0)
				{
					m_execflags &= ~INTRPT;

					// Call to interrupt vector
					stack_push(m_pc);
					m_pc = interrupt_vector();
					m_execflags |= FORCENOP;

					// Set global interrupt disable flag
					m_cpusta |= 0x10;
					m_intsample = 0;
				}
				else
					q3_execute();
			}
			if (--m_icount <= 0)
			{
				m_execphase = exec_phase::Q4;
				return;
			}
			[[fallthrough]];

		case exec_phase::Q4:
			if ((m_intsample & m_intsta) != 0)
				m_execflags = (m_execflags | INTRPT) & ~SLEEP;
			increment_timers();
			if ((m_execflags & TABLWT) != 0)
			{
				// TABLAT value used here may not be from TABLRD, but must be written with TABLAT/TLWT.
				// See PIC17C44 Rev. A Silicon Errata Sheet, DS30412C/44/E1A2.
				m_program.write_word(m_paddr, m_tlwt);
				m_execflags &= ~(TABLWT | TBLPTRI);
			}
			else if ((m_execflags & TABLRD) != 0)
			{
				m_tablat = m_program.read_word(m_paddr);
				m_execflags &= ~(TABLRD | TBLPTRI);
			}
			else
			{
				u16 inst = m_cache.read_word(m_paddr);
				q4_write();
				m_ir = inst;
			}
			if (--m_icount <= 0)
			{
				m_execphase = exec_phase::Q1;
				return;
			}
		}
	}
}

void pic17_cpu_device::device_start()
{
	// Hook address spaces
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	set_icountptr(m_icount);

	// Register debug state
	state_add<u16>(PIC17_PC, "PC",
		[this]() { return m_pc; },
		[this](u16 data) { debug_set_pc(data); }
	);
	state_add<u16>(STATE_GENPC, "GENPC",
		[this]() { return m_pc; },
		[this](u16 data) { debug_set_pc(data); }
	).noshow();
	state_add<u16>(STATE_GENPCBASE, "CURPC",
		[this]() { return m_ppc; },
		[this](u16 data) { debug_set_pc(data); }
	).noshow();
	state_add(PIC17_PCLATH, "PCLATH", m_pclath);
	state_add(PIC17_STKPTR, "STKPTR", m_stkptr).mask(0xf);
	state_add<u16>(PIC17_TOS, "TOS",
		[this]() { return m_stack[(m_stkptr - 1) & 0xf]; },
		[this](u16 data) { m_stack[(m_stkptr - 1) & 0xf] = data; }
	);
	state_add(PIC17_WREG, "W", m_wreg);
	state_add(PIC17_PROD, "PROD", m_prod); // not in PIC17C42
	state_add(PIC17_ALUSTA, "ALUSTA", m_alusta);
	state_add(STATE_GENFLAGS, "FLAGS", m_alusta).formatstr("%10s").noshow();
	state_add(PIC17_CPUSTA, "CPUSTA", m_cpusta).mask(0x3c); // PIC17C75X also implements bits 1 and 0
	state_add(PIC17_INTSTA, "INTSTA", m_intsta);
	state_add(PIC17_FSR0, "FSR0", m_fsr[0]);
	state_add(PIC17_FSR1, "FSR1", m_fsr[1]);
	state_add(PIC17_BSR, "BSR", m_bsr);
	state_add(PIC17_TBLPTR, "TBLPTR", m_tblptr);
	state_add(PIC17_TABLAT, "TABLAT", m_tablat);
	state_add(PIC17_TLWT, "TLWT", m_tlwt);
	state_add(PIC17_T0STA, "T0STA", m_t0sta).mask(0xfe);
	state_add(PIC17_TMR0, "TMR0", m_tmr0);
	state_add(PIC17_PS, "PS", m_ps);

	// Save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_paddr));
	save_item(NAME(m_raddr));
	save_item(NAME(m_ir));
	save_item(NAME(m_tmp));
	save_item(NAME(m_execphase));
	save_item(NAME(m_execflags));
	save_item(NAME(m_pclath));
	save_item(NAME(m_wreg));
	save_item(NAME(m_alusta));
	save_item(NAME(m_cpusta));
	save_item(NAME(m_intsta));
	save_item(NAME(m_intsample));
	save_item(NAME(m_tblptr));
	save_item(NAME(m_tablat));
	save_item(NAME(m_tlwt));
	save_item(NAME(m_prod));
	save_item(NAME(m_fsr));
	save_item(NAME(m_bsr));
	save_item(NAME(m_stkptr));
	save_item(NAME(m_stack));
	save_item(NAME(m_t0sta));
	save_item(NAME(m_tmr0));
	save_item(NAME(m_ps));
}

void pic17_cpu_device::device_reset()
{
	// Reset CPU
	m_pc = 0;
	m_execphase = exec_phase::Q1;
	m_execflags = FORCENOP;
	m_alusta |= 0xf0;
	m_t0sta = 0;
	m_cpusta |= 0x3c;
	m_intsta = 0;
	m_intsample = 0;
	m_tblptr = 0; // not cleared on PIC17C42
	m_bsr = 0;
	m_stkptr = 0;
}

void pic17_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("1%c 0%c %c%c%c%c",
			BIT(m_alusta, 7) ? '=' : BIT(m_alusta, 6) ? '+' : '-',
			BIT(m_alusta, 5) ? '=' : BIT(m_alusta, 4) ? '+' : '-',
			BIT(m_alusta, 3) ? 'V' : '.', // OV
			BIT(m_alusta, 2) ? 'Z' : '.',
			BIT(m_alusta, 1) ? 'D' : '.', // DC
			BIT(m_alusta, 0) ? 'C' : '.');
		break;
	}
}
