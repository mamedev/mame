// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "upd777.h"
#include "upd777dasm.h"

#define LOG_UNHANDLED_OPS       (1U << 1)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(UPD777_CPU, upd777_cpu_device, "upd777cpu", "NEC uPD777 (CPU)")



upd777_cpu_device::upd777_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 16, 11, -1, address_map_constructor(FUNC(upd777_cpu_device::internal_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 7, 0, data)
	, m_datamem(*this, "datamem")
{
}

upd777_cpu_device::upd777_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd777_cpu_device(mconfig, UPD777_CPU, tag, owner, clock, address_map_constructor(FUNC(upd777_cpu_device::internal_data_map), this))
{
}

std::unique_ptr<util::disasm_interface> upd777_cpu_device::create_disassembler()
{
	return std::make_unique<upd777_disassembler>();
}

device_memory_interface::space_config_vector upd777_cpu_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void upd777_cpu_device::internal_map(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void upd777_cpu_device::internal_data_map(address_map &map)
{
	// 0x20 groups of 4 7-bit values
	// groups 0x00-0x18 are used for sprite/pattern entries with the format

	//     6543210
	// 00  yyyyyyp   (y = ypos, p = PRIO)
	// 01  xxxxxxx   (x = xpos)
	// 02  ttttttt   (t = pattern)
	// 03  YYYRGBS   (Y = , RGB = color, S=ySUB)  

	map(0x00, 0x7f).ram().share("datamem");
}

void upd777_cpu_device::increment_pc()
{
	u16 lowpc = m_pc & 0x07f;
	u16 highpc = m_pc & 0x780;

	const int top1 = (lowpc & 0x40) >> 6;
	const int top2 = (lowpc & 0x20) >> 5;
	const int nor = (top1 ^ top2) ^ 1;
	lowpc = (lowpc << 1) | nor;
	lowpc &= 0x7f;

	m_pc = highpc | lowpc;
}

void upd777_cpu_device::device_start()
{
	space(AS_PROGRAM).specific(m_space);
	space(AS_DATA).specific(m_data);

	set_icountptr(m_icount);

	state_add(UPD777_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	state_add(UPD777_A1, "A1", m_a[0]);
	state_add(UPD777_A2, "A2", m_a[1]);
	state_add(UPD777_A3, "A3", m_a[2]);
	state_add(UPD777_A4, "A4", m_a[3]);

	state_add(UPD777_L, "L", m_l);
	state_add(UPD777_H, "H", m_h);

	state_add(UPD777_ADDR_STACK0, "ADDR_STACK0", m_stack[0]);
	state_add(UPD777_ADDR_STACK1, "ADDR_STACK1", m_stack[1]);
	state_add(UPD777_ADDR_STACK2, "ADDR_STACK2", m_stack[2]);
	state_add(UPD777_ADDR_STACK_POS, "ADDR_STACK_POS", m_stackpos);

	//state_add(UPD777_SKIP, "SKIP", m_skip); // will always be showing 0 as debugger doesn't hook on skipped opcodes

	save_item(NAME(m_ppc));
	save_item(NAME(m_pc));
	save_item(NAME(m_skip));
	save_item(NAME(m_a));
	save_item(NAME(m_l));
	save_item(NAME(m_h));

	save_item(NAME(m_frs));
	save_item(NAME(m_fls));

	save_item(NAME(m_stack));
	save_item(NAME(m_stackpos));

	save_item(NAME(m_disp));
	save_item(NAME(m_gpe));
	save_item(NAME(m_kie));
	save_item(NAME(m_sme));
}

void upd777_cpu_device::device_reset()
{
	m_ppc = 0;
	m_pc = 0;

	m_a[0] = m_a[1] = m_a[2] = m_a[3] = 0;
	m_l = 0;
	m_h = 0;

	m_frs = 0;
	m_fls = 0;

	m_skip = 1; // the first opcode is always 'NOP' so maybe skip is 1 on startup?

	m_stack[0] = m_stack[1] = m_stack[2] = 0;
	m_stackpos = 0;

	m_disp = 0;
	m_gpe = 0;
	m_kie = 0;
	m_sme = 0;
}

u16 upd777_cpu_device::fetch()
{
	u16 opcode = m_space.read_word(m_pc);
	m_ppc = m_pc;
	increment_pc();
	return opcode;
}

void upd777_cpu_device::push_to_stack(u16 addr)
{
	if (m_stackpos < 3)
	{
		m_stack[m_stackpos] = addr;
		m_stackpos++;
	}
	else
	{
		logerror("attempting to push to full address stack\n");
	}
}

u16 upd777_cpu_device::pull_from_stack()
{
	if (m_stackpos > 0)
	{
		m_stackpos--;
		return m_stack[m_stackpos];
	}
	else
	{
		logerror("attempting to pull from empty address stack\n");
		return 0;
	}
}


void upd777_cpu_device::set_a11(int a11)
{
	m_pc = (m_pc & 0x3ff) | (a11 & 1) << 10;
}

void upd777_cpu_device::set_new_pc(int newpc)
{
	m_pc = newpc;
}

// L reg (lower memory pointer is 2 bit
void upd777_cpu_device::set_l(int l)
{
	m_l = l & 0x3;
}

u8 upd777_cpu_device::get_l()
{
	return m_l & 0x3;
}

// H reg (upper memory pointer) is 5-bit
void upd777_cpu_device::set_h(int h)
{
	m_h = h & 0x1f;
}

// H reg is used as upper bits of memory address
u8 upd777_cpu_device::get_h_shifted()
{
	return (m_h & 0x1f) << 2;
}

u8 upd777_cpu_device::get_h()
{
	return m_h & 0x1f;
}

// M is the content of memory address pointed to by H and L
u8 upd777_cpu_device::get_m_data()
{
	u8 addr = get_h_shifted() | get_l();
	return read_data_mem(addr & 0x7f) & 0x7f;
}

void upd777_cpu_device::set_m_data(u8 data)
{
	u8 addr = get_h_shifted() | get_l();
	return write_data_mem(addr & 0x7f, data & 0x7f);
}

// 'A' regs are 7-bit
void upd777_cpu_device::set_a1(u8 data) { m_a[0] = data & 0x7f; }
void upd777_cpu_device::set_a2(u8 data) { m_a[1] = data & 0x7f; }
void upd777_cpu_device::set_a3(u8 data) { m_a[2] = data & 0x7f; }
void upd777_cpu_device::set_a4(u8 data) { m_a[3] = data & 0x7f; }

// 'A' regs are 7-bit
u8 upd777_cpu_device::get_a1() { return m_a[0] & 0x7f; }
u8 upd777_cpu_device::get_a2() { return m_a[1] & 0x7f; }
u8 upd777_cpu_device::get_a3() { return m_a[2] & 0x7f; }
u8 upd777_cpu_device::get_a4() { return m_a[3] & 0x7f; }

// FRS/FLS are the 2 7-bit sound registers
void upd777_cpu_device::set_frs(u8 frs) { m_frs = frs & 0x7f; }
void upd777_cpu_device::set_fls(u8 fls) { m_fls = fls & 0x7f; }

// MODE is a 7-bit register with the following format
// 6543210  
// rbhpRGB (r = reverberate sound effect, b = brightness, h = hue, p = black/prio, RGB = color)
void upd777_cpu_device::set_mode(u8 mode) { m_mode = mode & 0x7f; }

// single bit enable registers, although they have an important effect on the K->M opcode
void upd777_cpu_device::set_disp(u8 data) { m_disp = data & 1; }
void upd777_cpu_device::set_gpe(u8 data) { m_gpe = data & 1; }
void upd777_cpu_device::set_kie(u8 data) { m_kie = data & 1; }
void upd777_cpu_device::set_sme(u8 data) { m_sme = data & 1; }

u8 upd777_cpu_device::get_kie() { return m_kie & 1; }
u8 upd777_cpu_device::get_sme() { return m_sme & 1; }


u8 upd777_cpu_device::read_data_mem(u8 addr)
{
	// data memory is 7-bit
	return m_data.read_byte(addr) & 0x7f;
}

void upd777_cpu_device::write_data_mem(u8 addr, u8 data)
{
	// data memory is 7-bit
	m_data.write_byte(addr, data & 0x7f);
}

// temporary, for the opcode logging
std::string upd777_cpu_device::get_300optype_name(int optype)
{
	switch (optype)
	{
	case 0x00: return "."; // 'AND' expressed as '·' in documentation, but disassembler isn't keen on that
	case 0x01: return "+";
	case 0x02: return "v"; // 'OR'
	case 0x03: return "-";
	}
	return "<invalid optype>";
}

std::string upd777_cpu_device::get_200optype_name(int optype)
{
	switch (optype)
	{
	case 0x00: return "."; // 'AND' expressed as '·' in documentation, but disassembler isn't keen on that
	case 0x01: return "<invalid optype>";
	case 0x02: return "=";
	case 0x03: return "-";
	}
	return "<invalid optype>";
}

std::string upd777_cpu_device::get_reg_name(int reg)
{
	switch (reg)
	{
	case 0x00: return "A1"; // general reg A1
	case 0x01: return "A2"; // general reg A2
	case 0x02: return "M"; // content of memory
	case 0x03: return "H"; // high address
	}
	return "<invalid reg>";
}


void upd777_cpu_device::do_op()
{
	const u16 inst = fetch();

	if (inst >= 0b0000'1000'0000 && inst <= 0b0000'1111'1111)
	{
		// 080 - 0ff Skip if (M[H[5:1],L[2:1]][7:1]-K[7:1]) makes borrow
		const int k = inst & 0x7f;
		LOGMASKED(LOG_UNHANDLED_OPS, "M-0x%02x\n", k);
	}
	else if (inst >= 0b0001'0000'0000 && inst <= 0b0001'0111'1111)
	{
		// 100-17f M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		u8 m = get_m_data();
		m = m + k;
		if (m & 0x80)
			m_skip = 1;
		set_m_data(m & 0x7f);
		set_l(n);
	}
	else if (inst >= 0b0001'1000'0000 && inst <= 0b0001'1111'1111)
	{
		// 180-1ff M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		u8 m = get_m_data();
		m = m - k;
		if (m & 0x80)
			m_skip = 1;
		set_m_data(m & 0x7f);
		set_l(n);
	}
	else if (inst >= 0b0100'1000'0000 && inst <= 0b0100'1011'1111)
	{
		// 480-4bf H[5:1]-K[5:1]->H[5:1], Skip if borrow
		const int k = inst & 0x1f;
		u8 h = get_h();
		h = h - k;
		if (h & 0x20)
			m_skip = 1;
		set_h(h & 0x1f);
	}
	else if (inst >= 0b0100'1100'0000 && inst <= 0b0100'1111'1111)
	{
		// 4c0 - 4ff H[5:1]+K[5:1]->H[5:1], Skip if carry
		const int k = inst & 0x1f;
		u8 h = get_h();
		h = h + k;
		if (h & 0x20)
			m_skip = 1;
		set_h(h & 0x1f);
	}
	else if (inst >= 0b0101'0000'0000 && inst <= 0b0101'0111'1111)
	{
		// 500 - 57f
		// When (KIE=0)&(SME=0), Store K[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (KIE=1), Store KIN[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (SME=1), Store HCL[7:1] to M[H[5:1],L[2:1]][7:1]
		const int k = inst & 0x7f;

		if (get_kie()) // documentation does not state if KIE or SME has priority
		{
			LOGMASKED(LOG_UNHANDLED_OPS, "KIE->M\n", k);
		}
		else if (get_sme())
		{
			LOGMASKED(LOG_UNHANDLED_OPS, "SME->M\n", k);
		}
		else
		{
			set_m_data(k & 0x7f);
		}
	}
	else if (inst >= 0b0101'1000'0000 && inst <= 0b0101'1111'1111)
	{
		// 580 - 5ff Store K[7:6] to L[2:1] and K[5:1] to H[5:1]
		const int k = inst & 0x7f;
		set_l(k >> 5);
		set_h(k & 0x1f);
	}
	else if (inst >= 0b0110'0000'0000 && inst <= 0b0111'1111'1111)
	{
		// 600-67f Store K[7:1] to A1[7:1]
		// 680-6ff Store K[7:1] to A2[7:1]
		// 700-77f Store K[7:1] to A3[7:1]
		// 780-7ff Store K[7:1] to A4[7:1]
		const int reg = (inst & 0x180) >> 7;
		const int k = inst & 0x7f;
		switch (reg)
		{
		case 0: set_a1(k); break;
		case 1: set_a2(k); break;
		case 2: set_a3(k); break;
		case 3: set_a4(k); break;
		}
	}
	else if (inst >= 0b1000'0000'0000 && inst <= 0b1011'1111'1111)
	{
		// 800 - bff Move K[10:1] to A[10:1], Jump to A[11:1]
		u16 fulladdress = (m_ppc & 0x400) | (inst & 0x3ff);
		set_new_pc(fulladdress);
	}
	else if (inst >= 0b1100'0000'0000 && inst <= 0b1111'1111'1111)
	{
		// c00 - fff Move K[10:1] to A[10:1], 0 to A11, Jump to A[11:1], Push next A[11:1] up to ROM address stack
		const int k = inst & 0x3ff;
		push_to_stack(m_pc);
		set_new_pc(k);
	}
	else
	{
		switch (inst)
		{
		case 0b0000'0000'0000:
		{
			// 000 No Operation
			// nothing
			break;
		}
		case 0b0000'0000'0100:
		{
			// 004 Skip if (Gun Port Latch) = 1
			LOGMASKED(LOG_UNHANDLED_OPS, "GPL\n");
			break;
		}
		case 0b0000'0000'1000:
		{
			// 008 Move H[5:1] to Line Buffer Register[5:1]
			LOGMASKED(LOG_UNHANDLED_OPS, "H->NRM\n");
			break;
		}
		case 0b0000'0001'1000:
		{
			// 018 H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
			LOGMASKED(LOG_UNHANDLED_OPS, "H<->X\n");
			break;
		}
		case 0b0000'0010'0000:
		{
			// 020 Subroutine End, Pop down address stack
			u16 addr = pull_from_stack();
			set_new_pc(addr);
			break;
		}
		case 0b0000'0010'1000: case 0b0000'0010'1001:
		{
			// 028 Shift STB[4:1], N->STB[1]
			// STB is an input strobe / shifter
			const int n = inst & 1;
			LOGMASKED(LOG_UNHANDLED_OPS, "0x%d->STB\n", n);
			break;
		}
		case 0b0000'0011'0000:
		case 0b0000'0011'0100:
		case 0b0000'0011'1000:
		case 0b0000'0011'1100:
		case 0b0000'0111'0000:
		case 0b0000'0111'0100:
		case 0b0000'0111'1000:
		case 0b0000'0111'1100:
		{
			// 30 Skip if (PD1 input) = 1
			// 34 Skip if (PD2 input) = 1
			// 38 Skip if (PD3 input) = 1
			// 3c Skip if (PD4 input) = 1
			// 70 Skip if (PD1 input) = 0
			// 74 Skip if (PD2 input) = 0
			// 78 Skip if (PD3 input) = 0
			// 7c Skip if (PD4 input) = 0
			const int which = (inst & 0x00c) >> 2;
			const int inv = inst & 0x40;
			LOGMASKED(LOG_UNHANDLED_OPS, "PD%d %sJ\n", which + 1, inv ? "/" : "");
			break;
		}
		case 0b0000'0100'1001:
		{
			// 049 Skip if (4H Horizontal Blank) = 1
			LOGMASKED(LOG_UNHANDLED_OPS, "4H BLK\n");
			m_skip = 1;
			break;
		}
		case 0b0000'0100'1010:
		{
			// 04a Skip if (Vertical Blank) = 1, 0->M[[18:00],[3]][1]
			LOGMASKED(LOG_UNHANDLED_OPS, "VBLK\n");
			break;
		}
		case 0b0000'0100'1100:
		{
			// 04c Skip if (GP&SW/ input) = 1
			LOGMASKED(LOG_UNHANDLED_OPS, "GPSW/\n");
			break;
		}
		case 0b0000'0101'0100:
		{
			// 054 Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
			write_data_mem(get_h_shifted() | 0, get_a1());
			write_data_mem(get_h_shifted() | 1, get_a2());
			write_data_mem(get_h_shifted() | 2, get_a3());
			write_data_mem(get_h_shifted() | 3, get_a4());
			break;
		}
		case 0b0000'0101'1000:
		{
			// 058 Move M[H[5:1]][28:1] to (A4[7:1],A3[7:1],A2[7:1],A1[7:1])
			u8 m1 = read_data_mem(get_h_shifted() | 0);
			u8 m2 = read_data_mem(get_h_shifted() | 1);
			u8 m3 = read_data_mem(get_h_shifted() | 2);
			u8 m4 = read_data_mem(get_h_shifted() | 3);
			set_a1(m1);
			set_a2(m2);
			set_a3(m3);
			set_a4(m4);
			break;
		}
		case 0b0000'0101'1100:
		{
			// 05c Exchange (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) and M[H[5:1]][28:1]
			u8 m1 = read_data_mem(get_h_shifted() | 0);
			u8 m2 = read_data_mem(get_h_shifted() | 1);
			u8 m3 = read_data_mem(get_h_shifted() | 2);
			u8 m4 = read_data_mem(get_h_shifted() | 3);
			write_data_mem(get_h_shifted() | 0, get_a1());
			write_data_mem(get_h_shifted() | 1, get_a2());
			write_data_mem(get_h_shifted() | 2, get_a3());
			write_data_mem(get_h_shifted() | 3, get_a4());
			set_a1(m1);
			set_a2(m2);
			set_a3(m3);
			set_a4(m4);
			break;
		}
		case 0b0000'0110'0000:
		{
			// 060 Subroutine End, Pop down address stack, Skip
			u16 addr = pull_from_stack();
			set_new_pc(addr);
			m_skip = 1;
			break;
		}

		case 0b0010'0000'0000: case 0b0010'0000'0001: case 0b0010'0000'0010: case 0b0010'0000'0011:
		case 0b0010'0010'0000: case 0b0010'0010'0001: case 0b0010'0010'0010: case 0b0010'0010'0011:
		case 0b0010'0000'1000: case 0b0010'0000'1001: case 0b0010'0000'1010: case 0b0010'0000'1011:
		case 0b0010'0010'1000: case 0b0010'0010'1001: case 0b0010'0010'1010: case 0b0010'0010'1011:
		case 0b0010'0000'1100: case 0b0010'0000'1101: case 0b0010'0000'1110: case 0b0010'0000'1111:
		case 0b0010'0010'1100: case 0b0010'0010'1101: case 0b0010'0010'1110: case 0b0010'0010'1111:
		case 0b0010'0001'0000: case 0b0010'0001'0001: case 0b0010'0001'0010: case 0b0010'0001'0011:
		case 0b0010'0011'0000: case 0b0010'0011'0001: case 0b0010'0011'0010: case 0b0010'0011'0011:
		case 0b0010'0001'1000: case 0b0010'0001'1001: case 0b0010'0001'1010: case 0b0010'0001'1011:
		case 0b0010'0011'1000: case 0b0010'0011'1001: case 0b0010'0011'1010: case 0b0010'0011'1011:
		case 0b0010'0001'1100: case 0b0010'0001'1101: case 0b0010'0001'1110: case 0b0010'0001'1111:
		case 0b0010'0011'1100: case 0b0010'0011'1101: case 0b0010'0011'1110: case 0b0010'0011'1111:
		case 0b0010'0100'0000: case 0b0010'0100'0001: case 0b0010'0100'0010: case 0b0010'0100'0011:
		case 0b0010'0110'0000: case 0b0010'0110'0001: case 0b0010'0110'0010: case 0b0010'0110'0011:
		case 0b0010'0100'1000: case 0b0010'0100'1001: case 0b0010'0100'1010: case 0b0010'0100'1011:
		case 0b0010'0110'1000: case 0b0010'0110'1001: case 0b0010'0110'1010: case 0b0010'0110'1011:
		case 0b0010'0100'1100: case 0b0010'0100'1101: case 0b0010'0100'1110: case 0b0010'0100'1111:
		case 0b0010'0110'1100: case 0b0010'0110'1101: case 0b0010'0110'1110: case 0b0010'0110'1111:
		case 0b0010'0101'0000: case 0b0010'0101'0001: case 0b0010'0101'0010: case 0b0010'0101'0011:
		case 0b0010'0111'0000: case 0b0010'0111'0001: case 0b0010'0111'0010: case 0b0010'0111'0011:
		case 0b0010'0101'1000: case 0b0010'0101'1001: case 0b0010'0101'1010: case 0b0010'0101'1011:
		case 0b0010'0111'1000: case 0b0010'0111'1001: case 0b0010'0111'1010: case 0b0010'0111'1011:
		case 0b0010'0101'1100: case 0b0010'0101'1101: case 0b0010'0101'1110: case 0b0010'0101'1111:
		case 0b0010'0111'1100: case 0b0010'0111'1101: case 0b0010'0111'1110: case 0b0010'0111'1111:
		case 0b0010'1000'0000: case 0b0010'1000'0001: case 0b0010'1000'0010: case 0b0010'1000'0011:
		case 0b0010'1010'0000: case 0b0010'1010'0001: case 0b0010'1010'0010: case 0b0010'1010'0011:
		case 0b0010'1000'1000: case 0b0010'1000'1001: case 0b0010'1000'1010: case 0b0010'1000'1011:
		case 0b0010'1010'1000: case 0b0010'1010'1001: case 0b0010'1010'1010: case 0b0010'1010'1011:
		case 0b0010'1000'1100: case 0b0010'1000'1101: case 0b0010'1000'1110: case 0b0010'1000'1111:
		case 0b0010'1010'1100: case 0b0010'1010'1101: case 0b0010'1010'1110: case 0b0010'1010'1111:
		case 0b0010'1001'0000: case 0b0010'1001'0001: case 0b0010'1001'0010: case 0b0010'1001'0011:
		case 0b0010'1011'0000: case 0b0010'1011'0001: case 0b0010'1011'0010: case 0b0010'1011'0011:
		case 0b0010'1001'1000: case 0b0010'1001'1001: case 0b0010'1001'1010: case 0b0010'1001'1011:
		case 0b0010'1011'1000: case 0b0010'1011'1001: case 0b0010'1011'1010: case 0b0010'1011'1011:
		case 0b0010'1001'1100: case 0b0010'1001'1101: case 0b0010'1001'1110: case 0b0010'1001'1111:
		case 0b0010'1011'1100: case 0b0010'1011'1101: case 0b0010'1011'1110: case 0b0010'1011'1111:
		case 0b0010'1100'0000: case 0b0010'1100'0001: case 0b0010'1100'0010: case 0b0010'1100'0011:
		case 0b0010'1110'0000: case 0b0010'1110'0001: case 0b0010'1110'0010: case 0b0010'1110'0011:
		case 0b0010'1100'1000: case 0b0010'1100'1001: case 0b0010'1100'1010: case 0b0010'1100'1011:
		case 0b0010'1110'1000: case 0b0010'1110'1001: case 0b0010'1110'1010: case 0b0010'1110'1011:
		case 0b0010'1100'1100: case 0b0010'1100'1101: case 0b0010'1100'1110: case 0b0010'1100'1111:
		case 0b0010'1110'1100: case 0b0010'1110'1101: case 0b0010'1110'1110: case 0b0010'1110'1111:
		case 0b0010'1101'0000: case 0b0010'1101'0001: case 0b0010'1101'0010: case 0b0010'1101'0011:
		case 0b0010'1111'0000: case 0b0010'1111'0001: case 0b0010'1111'0010: case 0b0010'1111'0011:
		case 0b0010'1101'1000: case 0b0010'1101'1001: case 0b0010'1101'1010: case 0b0010'1101'1011:
		case 0b0010'1111'1000: case 0b0010'1111'1001: case 0b0010'1111'1010: case 0b0010'1111'1011:
		case 0b0010'1101'1100: case 0b0010'1101'1101: case 0b0010'1101'1110: case 0b0010'1101'1111:
		case 0b0010'1111'1100: case 0b0010'1111'1101: case 0b0010'1111'1110: case 0b0010'1111'1111:
		{
			// optype · (AND)
			// 200 Skip if (A1[7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 220 Skip if (A1[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 210 Skip if (A1[7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 230 Skip if (A1[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 240 Skip if (A2[7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 260 Skip if (A2[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 250 Skip if (A2[7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 270 Skip if (A2[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 280 Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 2a0 Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 290 Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 2b0 Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 2c0 Skip if (H[5:1]·A1[5:1]) makes zero, N->L[2:1]
			// 2e0 Skip if (H[5:1]·A1[5:1]) makes non zero, N->L[2:1]
			// 2d0 Skip if (H[5:1]·A2[5:1]) makes zero, N->L[2:1]
			// 2f0 Skip if (H[5:1]·A2[5:1]) makes non zero, N->L[2:1]

			// optype = (these are expressed as x=y in the opcopde syntax, but x-y in the description, in reality it seems to act as 'CMP' so x-y = 0)
			// 208 Skip if (A1[7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 228 Skip if (A1[7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 218 Skip if (A1[7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 238 Skip if (A1[7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 248 Skip if (A2[7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 268 Skip if (A2[7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 258 Skip if (A2[7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 278 Skip if (A2[7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 288 Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 2a8 Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 298 Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 2b8 Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 2c8 Skip if (H[5:1]-A1[5:1]) makes zero, N->L[2:1]
			// 2e8 Skip if (H[5:1]-A1[5:1]) makes non zero, N->L[2:1]
			// 2d8 Skip if (H[5:1]-A2[5:1]) makes zero, N->L[2:1]
			// 2f8 Skip if (H[5:1]-A2[5:1]) makes non zero, N->L[2:1]

			// optype -
			// 20c Skip if (A1[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 22c Skip if (A1[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 21c Skip if (A1[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 23c Skip if (A1[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 24c Skip if (A2[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 26c Skip if (A2[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 25c Skip if (A2[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 27c Skip if (A2[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 28c Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 2ac Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 29c Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 2bc Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 2cc Skip if (H[5:1]-A1[5:1]) makes borrow, N->L[2:1]
			// 2ec Skip if (H[5:1]-A1[5:1]) makes non borrow, N->L[2:1]
			// 2dc Skip if (H[5:1]-A2[5:1]) makes borrow, N->L[2:1]
			// 2fc Skip if (H[5:1]-A2[5:1]) makes non borrow, N->L[2:1]
			const int non = inst & 0x20;
			const int optype = (inst & 0x0c) >> 2;
			const int reg1 = (inst & 0xc0) >> 6;
			const int reg2 = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "%s%s%s, 0x%d->L %s%s\n", get_reg_name(reg1), get_200optype_name(optype), get_reg_name(reg2), n, (optype == 3) ? "BOJ" : "EQJ", non ? "/" : "");
			set_l(n);
			break;
		}

		case 0b0011'0000'0000: case 0b0011'0000'0001: case 0b0011'0000'0010: case 0b0011'0000'0011:
		{
			// 300 N->L[2:1]
			const int n = inst & 0x3;
			set_l(n);
			break;
		}

		case 0b0011'0000'1000:
		{
			// 308 Move A1[7:1] to FLS[7:1], 0->L[2:1]
			u8 a1 = get_a1();
			set_fls(a1);
			set_l(0);
			break;
		}
		case 0b0011'0100'1000:
		{
			// 348 Move A2[7:1] to FLS[7:1], 0->L[2:1]
			u8 a2 = get_a2();
			set_fls(a2);
			set_l(0);
			break;
		}
		case 0b0011'1000'1000:
		{
			// 388 Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
			u8 m = get_m_data();
			set_fls(m);
			set_l(0);
			break;
		}

		case 0b0011'0000'1001:
		{
			// 309 Move A1[7:1] to FRS[7:1], 1->L[2:1]
			u8 a1 = get_a1();
			set_frs(a1);
			set_l(1);
			break;
		}
		case 0b0011'0100'1001:
		{
			// 349 Move A2[7:1] to FRS[7:1], 1->L[2:1]
			u8 a2 = get_a2();
			set_frs(a2);
			set_l(1);
			break;
		}
		case 0b0011'1000'1001:
		{
			// 389 Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
			u8 m = get_m_data();
			set_frs(m);
			set_l(1);
			break;
		}

		case 0b0011'0000'1010: case 0b0011'0000'1011:
		{
			// 30a Move A1[7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			u8 a1 = get_a1();
			set_mode(a1);
			set_l(n);
			break;
		}
		case 0b0011'01001010: case 0b0011'0100'1011:
		{
			// 34a Move A2[7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			u8 a2 = get_a2();
			set_mode(a2);
			set_l(n);
			break;
		}
		case 0b0011'1000'1010: case 0b00111000'1011:
		{
			// 38a Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			u8 m = get_m_data();
			set_mode(m);
			set_l(n);
			break;
		}

		case 0b0011'0001'0000: case 0b0011'0001'0001: case 0b0011'0001'0010: case 0b0011'0001'0011:
		{
			// 310 Move A2[7:1] to A1[7:1], N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A2->A1, 0x%d->L\n", n);
			set_l(n);
			break;
		}
		case 0b0011'0100'0000: case 0b0011'0100'0001: case 0b0011'0100'0010: case 0b0011'0100'0011:
		{
			// 340 Move A1[7:1] to A2[7:1], N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A1->A2, 0x%d->L\n", n);
			set_l(n);
			break;
		}

		case 0b0011'0001'1000: case 0b0011'0001'1001: case 0b0011'0001'1010: case 0b0011'0001'1011:
		{
			// 318 Right shift A1[7:1], 0->A1[7], N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A1->RS, 0x%d->L\n", n);
			set_l(n);
			break;
		}
		case 0b0011'0101'1000: case 0b0011'0101'1001: case 0b0011'0101'1010: case 0b0011'0101'1011:
		{
			// 358 Right shift A2[7:1], 0->A2[7], N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A2->RS, 0x%d->L\n", n);
			set_l(n);
			break;
		}
		case 0b0011'1001'1000: case 0b0011'1001'1001: case 0b0011'1001'1010: case 0b0011'1001'1011:
		{
			// 398 Right shift M[H[5:1],L[2:1]][7:1], 0->M[H[5:1],L[2:1]][7], N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "M->RS, 0x%d->L\n", n);
			set_l(n);
			break;
		}

		case 0b0011'0001'1100: case 0b0011'0001'1101: case 0b0011'0001'1110: case 0b0011'0001'1111:
		{
			// 31c Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A1-A2->A2, 0x%d->L\n", n);
			set_l(n);
			break;
		}
		case 0b0011'0100'1100: case 0b0011'0100'1101: case 0b0011'0100'1110: case 0b0011'0100'1111:
		{
			// 34c Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A2-A1->A1, 0x%d->L\n", n);
			set_l(n);
			break;
		}

		case 0b0011'0010'0000: case 0b0011'0010'0001: case 0b0011'0010'0010: case 0b0011'0010'0011:
		case 0b0011'0010'0100: case 0b0011'0010'0101: case 0b0011'0010'0110: case 0b0011'0010'0111:
		case 0b0011'0010'1000: case 0b0011'0010'1001: case 0b0011'0010'1010: case 0b0011'0010'1011:
		case 0b0011'0010'1100: case 0b0011'0010'1101: case 0b0011'0010'1110: case 0b0011'0010'1111:
		case 0b0011'0011'0000: case 0b0011'0011'0001: case 0b0011'0011'0010: case 0b0011'0011'0011:
		case 0b0011'0011'0100: case 0b0011'0011'0101: case 0b0011'0011'0110: case 0b0011'0011'0111:
		case 0b0011'0011'1000: case 0b0011'0011'1001: case 0b0011'0011'1010: case 0b0011'0011'1011:
		case 0b0011'0011'1100: case 0b0011'0011'1101: case 0b0011'0011'1110: case 0b0011'0011'1111:
		case 0b0011'0110'0000: case 0b0011'0110'0001: case 0b0011'0110'0010: case 0b0011'0110'0011:
		case 0b0011'0110'0100: case 0b0011'0110'0101: case 0b0011'0110'0110: case 0b0011'0110'0111:
		case 0b0011'0110'1000: case 0b0011'0110'1001: case 0b0011'0110'1010: case 0b0011'0110'1011:
		case 0b0011'0110'1100: case 0b0011'0110'1101: case 0b0011'0110'1110: case 0b0011'0110'1111:
		case 0b0011'0111'0000: case 0b0011'0111'0001: case 0b0011'0111'0010: case 0b0011'0111'0011:
		case 0b0011'0111'0100: case 0b0011'0111'0101: case 0b0011'0111'0110: case 0b0011'0111'0111:
		case 0b0011'0111'1000: case 0b0011'0111'1001: case 0b0011'0111'1010: case 0b0011'0111'1011:
		case 0b0011'0111'1100: case 0b0011'0111'1101: case 0b0011'0111'1110: case 0b0011'0111'1111:
		{
			// 320 AND A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 324 Add A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 328 OR A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 32c Subtract A1[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			// 330 AND A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 334 Add A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 338 OR A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 33c Subtract A1[7:1] and A2[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			// 360 AND A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 364 Add A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 368 OR A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 36c Subtract A2[7:1] and A1[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			// 370 AND A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 374 Add A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 378 OR A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 37c Subtract A2[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			const int optype = (inst & 0x0c) >> 2;
			const int reg2 = (inst & 0x10) >> 4;
			const int reg1 = (inst & 0x40) >> 6;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "%s%s%s->%s, 0x%d->L %s\n", get_reg_name(reg1), get_300optype_name(optype), get_reg_name(reg2), get_reg_name(reg1), n, (optype == 3) ? "BOJ" : "");
			set_l(n);
			break;
		}

		case 0b0011'1000'0000: case 0b0011'1000'0001: case 0b0011'1000'0010: case 0b0011'1000'0011:
		case 0b0011'1001'0000: case 0b0011'1001'0001: case 0b0011'1001'0010: case 0b0011'1001'0011:
		{
			// 380 Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 390 Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A%d->M, 0x%d->L\n", reg + 1, n);
			set_l(n);
			break;
		}

		case 0b0011'1000'0100: case 0b0011'1000'0101: case 0b0011'1000'0110: case 0b0011'1000'0111:
		case 0b0011'1001'0100: case 0b0011'1001'0101: case 0b0011'1001'0110: case 0b0011'1001'0111:
		{
			// 384 Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
			// 394 Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "M<->A%d, 0x%d->L\n", reg + 1, n);
			set_l(n);
			break;
		}

		case 0b0011'1000'1100: case 0b0011'1000'1101: case 0b0011'1000'1110: case 0b0011'1000'1111:
		case 0b0011'1001'1100: case 0b0011'1001'1101: case 0b0011'1001'1110: case 0b0011'1001'1111:
		{
			// 38c Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
			// 39c Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "M->A%d, 0x%d->L\n", reg + 1, n);
			set_l(n);
			break;
		}

		case 0b0011'1010'0000: case 0b0011'1010'0001: case 0b0011'1010'0010: case 0b0011'1010'0011:
		case 0b0011'1010'0100: case 0b0011'1010'0101: case 0b0011'1010'0110: case 0b0011'1010'0111:
		case 0b0011'1010'1000: case 0b0011'1010'1001: case 0b0011'1010'1010: case 0b0011'1010'1011:
		case 0b0011'1010'1100: case 0b0011'1010'1101: case 0b0011'1010'1110: case 0b0011'1010'1111:
		case 0b0011'1011'0000: case 0b0011'1011'0001: case 0b0011'1011'0010: case 0b0011'1011'0011:
		case 0b0011'1011'0100: case 0b0011'1011'0101: case 0b0011'1011'0110: case 0b0011'1011'0111:
		case 0b0011'1011'1000: case 0b0011'1011'1001: case 0b0011'1011'1010: case 0b0011'1011'1011:
		case 0b0011'1011'1100: case 0b0011'1011'1101: case 0b0011'1011'1110: case 0b0011'1011'1111:
		{
			// 3a0 AND M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3a4 Add M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			// 3a8 OR M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3ac Subtract M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			// 3b0 AND M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3b4 Add M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			// 3b8 OR M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3bc Subtract M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			const int optype = (inst & 0x0c) >> 2;
			const int reg2 = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "M%s%s->M, 0x%d->L\n", get_300optype_name(optype), get_reg_name(reg2), n);
			set_l(n);
			break;
		}

		case 0b0011'1110'0000: case 0b0011'1110'0001: case 0b0011'1110'0010: case 0b0011'1110'0011:
		case 0b0011'1110'0100: case 0b0011'1110'0101: case 0b0011'1110'0110: case 0b0011'1110'0111:
		case 0b0011'1110'1000: case 0b0011'1110'1001: case 0b0011'1110'1010: case 0b0011'1110'1011:
		case 0b0011'1110'1100: case 0b0011'1110'1101: case 0b0011'1110'1110: case 0b0011'1110'1111:
		case 0b0011'1111'0000: case 0b0011'1111'0001: case 0b0011'1111'0010: case 0b0011'1111'0011:
		case 0b0011'1111'0100: case 0b0011'1111'0101: case 0b0011'1111'0110: case 0b0011'1111'0111:
		case 0b0011'1111'1000: case 0b0011'1111'1001: case 0b0011'1111'1010: case 0b0011'1111'1011:
		case 0b0011'1111'1100: case 0b0011'1111'1101: case 0b0011'1111'1110: case 0b0011'1111'1111:
		{
			// 3e0 AND H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3e4 Add H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3e8 OR H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3ec Subtract H[5:1] and A1[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			// 3f0 AND H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3f4 Add H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3f8 OR H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3fc Subtract H[5:1] and A2[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			const int optype = (inst & 0x0c) >> 2;
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "H%s%s->H, 0x%d->L\n", get_300optype_name(optype), get_reg_name(reg), n);
			set_l(n);
			break;
		}

		case 0b0011'1100'0000: case 0b0011'1100'0001: case 0b0011'1100'0010: case 0b0011'1100'0011:
		case 0b0011'1101'0000: case 0b0011'1101'0001: case 0b0011'1101'0010: case 0b0011'1101'0011:
		{
			// 3c0 Move A1[5:1] to H[5:1], N->L[2:1]
			// 3d0 Move A2[5:1] to H[5:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "A%d->H, 0x%d->L\n", reg + 1, n);
			set_l(n);
			break;
		}
		case 0b0011'1100'1100: case 0b0011'1100'1101: case 0b0011'1100'1110: case 0b0011'1100'1111:
		case 0b0011'1101'1100: case 0b0011'1101'1101: case 0b0011'1101'1110: case 0b0011'1101'1111:
		{
			// 3cc Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
			// 3dc Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			LOGMASKED(LOG_UNHANDLED_OPS, "H->A%d, 0x%d->L\n", reg + 1, n);
			set_l(n);
			break;
		}

		case 0b0100'0000'0000: case 0b0100'0000'0001:
		{
			// 400 N->A[11]
			const int n = inst & 0x1;
			set_a11(n);
			break;
		}
		case 0b0100'0000'0010: case 0b0100'0000'0011:
		{
			// 402 Jump to (000,M[H[5:1],L[2:1]][5:1],1N), 0->L[2:1], N->A[11]
			const int n = inst & 0x1;
			LOGMASKED(LOG_UNHANDLED_OPS, "JPM, 0->L, %d->A11\n", n);
			set_a11(n);
			break;
		}
		case 0b0100'0100'0000: case 0b0100'0100'0001: case 0b0100'0100'0100: case 0b0100'0100'0101:
		case 0b0100'0100'1000: case 0b0100'0100'1001: case 0b0100'0100'1100: case 0b0100'0100'1101:
		case 0b0100'0101'0000: case 0b0100'0101'0001: case 0b0100'0101'0100: case 0b0100'0101'0101:
		case 0b0100'0101'1000: case 0b0100'0101'1001: case 0b0100'0101'1100: case 0b0100'0101'1101:
		case 0b0100'0110'0000: case 0b0100'0110'0001: case 0b0100'0110'0100: case 0b0100'0110'0101:
		case 0b0100'0110'1000: case 0b0100'0110'1001: case 0b0100'0110'1100: case 0b0100'0110'1101:
		case 0b0100'0111'0000: case 0b0100'0111'0001: case 0b0100'0111'0100: case 0b0100'0111'0101:
		case 0b0100'0111'1000: case 0b0100'0111'1001: case 0b0100'0111'1100: case 0b0100'0111'1101:
		{
			// 440 Set D to DISP, G to GPE, K to KIE, S to SME, N->A[11]
			const int d = (inst >> 5) & 0x1;
			const int g = (inst >> 4) & 0x1;
			const int k = (inst >> 3) & 0x1;
			const int s = (inst >> 2) & 0x1;
			const int n = inst & 0x1;
			set_disp(d);
			set_gpe(g);
			set_kie(k);
			set_sme(s);
			set_a11(n);
			break;
		}

		default:
		{
			LOGMASKED(LOG_UNHANDLED_OPS, "%04x <ILLEGAL>\n", inst);
			break;
		}
		}
	}
}

void upd777_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_skip)
		{
			fetch();
			m_skip = 0;
		}

		debugger_instruction_hook(m_pc);
		do_op();
		m_icount--;
	}
}

void upd777_cpu_device::execute_set_input(int inputnum, int state)
{
}
