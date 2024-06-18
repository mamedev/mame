// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "upd777.h"

#include "upd777dasm.h"


#define LOG_UNHANDLED_OPS       (1U << 1)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(UPD777,    upd777_cpu_device,    "upd777",    "uPD777")

upd777_cpu_device::upd777_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_datamem(*this, "datamem")
	, m_space_config("program", ENDIANNESS_BIG, 16, 11, -1, address_map_constructor(FUNC(upd777_cpu_device::internal_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 8, 7, 0, data)
	, m_gfxdecode(*this, "gfxdecode")
	, m_palette(*this, "palette")
	, m_screen(*this, "screen")
	, m_prgregion(*this, "prg")
	, m_patregion(*this, "patterns")
	, m_port_in(*this, 0xff)
{
}

upd777_cpu_device::upd777_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: upd777_cpu_device(mconfig, UPD777, tag, owner, clock, address_map_constructor(FUNC(upd777_cpu_device::internal_data_map), this))
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
	map(0x000, 0x7ff).rom().region("prg", 0);
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

	const u16 xnor = BIT(lowpc, 6) ^ BIT(lowpc, 5) ^ 1;
	lowpc = ((lowpc << 1) | xnor) & 0x7f;

	// is returning to the start of the page the correct behavior?
	if (lowpc == 0x00)
	{
		logerror("overflowing PC, returning to start of current page %03x\n", highpc);
		// pakpak runs better if you do this, but that is clearly by chance as
		// most other cases show this isn't meant to happen
		//highpc += 0x80;
		//highpc &= 0x7ff;
	}

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
	save_item(NAME(m_ldash));
	save_item(NAME(m_x4));
	save_item(NAME(m_h));

	save_item(NAME(m_frs));
	save_item(NAME(m_fls));

	save_item(NAME(m_mode));
	save_item(NAME(m_stb));

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
	m_ldash = 0;
	m_x4 = 0;
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

inline u16 upd777_cpu_device::fetch()
{
	u16 opcode = m_space.read_word(m_pc);
	m_ppc = m_pc;
	increment_pc();
	return opcode;
}


inline void upd777_cpu_device::set_a11(int a11)
{
	m_pc = (m_pc & 0x3ff) | (a11 & 1) << 10;
}

inline void upd777_cpu_device::set_new_pc(int newpc)
{
	m_pc = newpc;
}

// L reg (lower memory pointer) is 2 bit
inline void upd777_cpu_device::set_l(int l)
{
	m_l = l & 0x3;
}

inline u8 upd777_cpu_device::get_l() const
{
	return m_l & 0x3;
}

// H reg (upper memory pointer) is 5-bit
inline void upd777_cpu_device::set_h(int h)
{
	m_h = h & 0x1f;
}

// H reg is used as upper bits of memory address
inline u8 upd777_cpu_device::get_h_shifted() const
{
	return (m_h & 0x1f) << 2;
}

inline u8 upd777_cpu_device::get_h() const
{
	return m_h & 0x1f;
}

// M is the content of memory address pointed to by H and L
inline u8 upd777_cpu_device::get_m_data()
{
	u8 addr = get_h_shifted() | get_l();
	return read_data_mem(addr & 0x7f) & 0x7f;
}

inline void upd777_cpu_device::set_m_data(u8 data)
{
	u8 addr = get_h_shifted() | get_l();
	write_data_mem(addr & 0x7f, data & 0x7f);
}

// 'A' regs are 7-bit
inline void upd777_cpu_device::set_a1(u8 data) { m_a[0] = data & 0x7f; }
inline void upd777_cpu_device::set_a2(u8 data) { m_a[1] = data & 0x7f; }
inline void upd777_cpu_device::set_a3(u8 data) { m_a[2] = data & 0x7f; }
inline void upd777_cpu_device::set_a4(u8 data) { m_a[3] = data & 0x7f; }

inline void upd777_cpu_device::set_a1_or_a2(int reg, u8 value)
{
	if (reg == 0)
		set_a1(value);
	else
		set_a2(value);
}

// 'A' regs are 7-bit
inline u8 upd777_cpu_device::get_a1() const { return m_a[0] & 0x7f; }
inline u8 upd777_cpu_device::get_a2() const { return m_a[1] & 0x7f; }
inline u8 upd777_cpu_device::get_a3() const { return m_a[2] & 0x7f; }
inline u8 upd777_cpu_device::get_a4() const { return m_a[3] & 0x7f; }

inline u8 upd777_cpu_device::get_a1_or_a2(int reg) const
{
	if (reg == 0)
		return get_a1();
	else
		return get_a2();
}

// FRS/FLS are the 2 7-bit sound registers
inline void upd777_cpu_device::set_frs(u8 frs) { m_frs = frs & 0x7f; }
inline void upd777_cpu_device::set_fls(u8 fls) { m_fls = fls & 0x7f; }

// MODE is a 7-bit register with the following format
// 6543210  
// rbhpRGB (r = reverberate sound effect, b = brightness, h = hue, p = black/prio, RGB = color)
inline void upd777_cpu_device::set_mode(u8 mode) { m_mode = mode & 0x7f; }

// single bit enable registers, although they have an important effect on the K->M opcode
inline void upd777_cpu_device::set_disp(u8 data) { m_disp = data & 1; }
inline void upd777_cpu_device::set_gpe(u8 data) { m_gpe = data & 1; }
inline void upd777_cpu_device::set_kie(u8 data) { m_kie = data & 1; }
inline void upd777_cpu_device::set_sme(u8 data) { m_sme = data & 1; }

inline u8 upd777_cpu_device::get_kie() const { return m_kie & 1; }
inline u8 upd777_cpu_device::get_sme() const { return m_sme & 1; }


inline u8 upd777_cpu_device::read_data_mem(u8 addr)
{
	// data memory is 7-bit
	return m_data.read_byte(addr) & 0x7f;
}

inline void upd777_cpu_device::write_data_mem(u8 addr, u8 data)
{
	// data memory is 7-bit
	m_data.write_byte(addr, data & 0x7f);
}

inline void upd777_cpu_device::push_to_stack(u16 addr)
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

inline u16 upd777_cpu_device::pull_from_stack()
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

void upd777_cpu_device::do_op()
{
	const u16 inst = fetch();

	if (inst >= 0b0000'1000'0000 && inst <= 0b0000'1111'1111)
	{
		// 080 - 0ff Skip if (M[H[5:1],L[2:1]][7:1]-K[7:1]) makes borrow
		const int k = inst & 0x7f;
		u8 m = get_m_data();
		m = m - k;
		if (m & 0x80)
			m_skip = 1;
	}
	else if (inst >= 0b0001'0000'0000 && inst <= 0b0001'0111'1111)
	{
		// 100-17f M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		u8 m = get_m_data();
		m = m + k;
		set_m_data(m & 0x7f);

		if (m & 0x80)
			m_skip = 1;

		// TODO: prevents infinitely scrolling screen, and allows pakpak to boot, but this does't make logical
		//       sense, unless the 'Skip if carry' in the text above is specifically meaning this only applies
		//       when skipping, however if you apply the same to the next opcode it breaks text rendering
		//       eg 'score' text in the same game.
		if (m_skip)
			set_l(n);
	}
	else if (inst >= 0b0001'1000'0000 && inst <= 0b0001'1111'1111)
	{
		// 180-1ff M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		u8 m = get_m_data();
		m = m - k;
		set_m_data(m & 0x7f);

		if (m & 0x80)
			m_skip = 1;

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
			// Inputs for Cassette Vision appear to be read by this
			// (selected based on the STB output value?)
			set_m_data(m_port_in(m_stb));
		}
		else if (get_sme())
		{
			LOGMASKED(LOG_UNHANDLED_OPS, "SME->M\n", k);
			set_m_data(0);
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
	else if (((inst & 0b1111'0000'0000) == 0b0010'0000'0000) && ((inst & 0b0000'0000'1100) != 0b0000'0000'0100))
	{
		// 0b0010'rrnR'oonn where rr = reg1 (A1, A2, M or H), n = invert condition, R = reg2 (A1 or A2) and oo = optype (only 0,2,3 are valid, no cases here for 1) nn = next l value

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
		u8 srcreg2 = get_a1_or_a2(reg2);
		u8 srcreg1 = 0;
		switch (reg1)
		{
		case 0: srcreg1 = get_a1(); break;
		case 1: srcreg1 = get_a2(); break;
		case 2: srcreg1 = get_m_data(); break;
		case 3:
		{
			srcreg1 = get_h();
			srcreg2 &= 0x1f;
			break;
		}
		}

		switch (optype)
		{
		case 0: // AND
		{
			if (!non)
			{
				if ((srcreg1 & srcreg2) == 0) // skip if (x·y) makes zero, N->L[2:1] 
					m_skip = 1;
			}
			else
			{
				if ((srcreg1 & srcreg2) != 0) // skip if (x·y) makes non zero, N->L[2:1]
					m_skip = 1;
			}
			break;
		}
		case 1: // invalid
		{
			// can't happen, no switch case leads here
			break;
		}
		case 2: // =
		{
			if (!non)
			{
				if (srcreg1 == srcreg2) // skip if (x-y) makes zero, N->L[2:1]
					m_skip = 1;
			}
			else
			{
				if (srcreg1 != srcreg2) // skip if (x-y) makes non zero, N->L[2:1]
					m_skip = 1;
			}
			break;
		}
		case 3: // -
		{
			u8 result = srcreg1 - srcreg2;

			if (!non)
			{
				if (result & 0x80) // skip if (x-y) makes borrow, N->L[2:1]
					m_skip = 1;
			}
			else
			{
				if ((result & 0x80) == 0) // skip if (x-y) makes non borrow, N->L[2:1]
					m_skip = 1;
			}

			break;
		}
		}

		set_l(n);
	}
	else if ((inst & 0b1111'1010'0000) == 0b0011'0010'0000)
	{
		//   0b0011'0r1R'oonn (where r = reg1, R = reg2, o = optype, and n = next l value)	
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

		u8 src1 = get_a1_or_a2(reg1);
		u8 src2 = get_a1_or_a2(reg2);

		switch (optype)
		{
		case 0: // AND
		{
			src1 = src1 & src2;
			break;
		}
		case 1: // ADD
		{
			src1 = src1 + src2;
			// not in this case?
			//if (src1 & 0x80)
			//	m_skip = 1;
			break;
		}
		case 2: // OR
		{
			src1 = src1 | src2;
			break;
		}
		case 3: // MINUS
		{
			src1 = src1 - src2;
			if (src1 & 0x80)
				m_skip = 1;
			break;
		}
		}
		set_a1_or_a2(reg1, src1);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'0000) == 0b0011'1010'0000)
	{
		//   0b0011'101r'oonn (where r = reg, o = optype, n = next l value)
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
		u8 src2 = get_a1_or_a2(reg2);
		u8 m = get_m_data();

		switch (optype)
		{
		case 0: // AND
		{
			m = m & src2;
			break;
		}
		case 1: // ADD
		{
			m = m + src2;
			if (m & 0x80)
				m_skip = 1;
			break;
		}
		case 2: // OR
		{
			m = m | src2;
			break;
		}
		case 3: // MINUS
		{
			m = m - src2;
			if (m & 0x80)
				m_skip = 1;
			break;
		}
		}
		set_m_data(m);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'0000) == 0b0011'1110'0000)
	{
		//   0b0011'111r'oonn (where r = reg, o = optype, n = next l value)
		// 3e0 AND H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
		// 3e4 Add H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
		// 3e8 OR H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
		// 3ec Subtract H[5:1] and A1[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
		// 3f0 AND H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
		// 3f4 Add H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
		// 3f8 OR H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
		// 3fc Subtract H[5:1] and A2[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
		const int optype = (inst & 0x0c) >> 2;
		const int reg2 = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 src2 = get_a1_or_a2(reg2) & 0x1f;
		u8 h = get_h();

		switch (optype)
		{
		case 0: // AND
		{
			h = h & src2;
			break;
		}
		case 1: // ADD
		{
			h = h + src2;
			break;
		}
		case 2: // OR
		{
			h = h | src2;
			break;
		}
		case 3: // MINUS
		{
			h = h - src2;
			if (h & 0x20)
				m_skip = 1;
			break;
		}
		}
		set_h(h & 0x1f);
		set_l(n);
	}
	else if ((inst & 0b1111'1100'0010) == 0b0100'0100'0000)
	{
		//   0b0100'01dg'ks0n (where  d = DISP, G = GPE, K = KIE, S = SME, n = A11)	
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
	}
	else if (inst == 0b0000'0000'0000)
	{
		// 000 No Operation
		// nothing
	}
	else if (inst == 0b0000'0000'0100)
	{
		// 004 Skip if (Gun Port Latch) = 1
		LOGMASKED(LOG_UNHANDLED_OPS, "GPL\n");
	}
	else if (inst == 0b0000'0000'1000)
	{
		// 008 Move H[5:1] to Line Buffer Register[5:1]
		u8 h = get_h();
		//LOGMASKED(LOG_UNHANDLED_OPS, "H(%02x)->NRM\n", h);

		// this seems to push a value from RAM into the line buffer for the current 4(?) scanlines
		u8 m1 = read_data_mem(get_h_shifted() | 0);
		u8 m2 = read_data_mem(get_h_shifted() | 1);
		u8 m3 = read_data_mem(get_h_shifted() | 2);
		u8 m4 = read_data_mem(get_h_shifted() | 3);
		push_to_line_buffer(h, m1,m2,m3,m4);
	}
	else if (inst == 0b0000'0001'1000)
	{
		// 018 H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
		LOGMASKED(LOG_UNHANDLED_OPS, "H<->X\n");
#if 1
		// this opcode is not well explained! but X4 etc. are referenced on Data RAM & Register Files which makes
		// it even more confusing (while L' isn't referenced anywhere else at all!)
		u8 temp;

		temp = m_x4;
		m_x4 = get_h();
		m_h = temp;

		temp = m_ldash;
		m_ldash = get_l();
		set_l(temp);
#endif
	}
	else if (inst == 0b0000'0010'0000)
	{
		// 020 Subroutine End, Pop down address stack
		u16 addr = pull_from_stack();
		set_new_pc(addr);
	}
	else if (inst == 0b0000'0100'1001)
	{
		// 049 Skip if (4H Horizontal Blank) = 1
		//LOGMASKED(LOG_UNHANDLED_OPS, "4H BLK\n");
		if (get_hbl_4_state())
			m_skip = 1;
	}
	else if (inst == 0b0000'0100'1010)
	{
		// 04a Skip if (Vertical Blank) = 1, 0->M[[18:00],[3]][1]
		LOGMASKED(LOG_UNHANDLED_OPS, "VBLK\n");
		if (get_vbl_state())
			m_skip = 1;

		// TODO:
		// need to do the 0->M[[18:00],[3]][1] bit
	}
	else if (inst == 0b0000'0100'1100)
	{
		// 04c Skip if (GP&SW/ input) = 1
		LOGMASKED(LOG_UNHANDLED_OPS, "GPSW/\n");
	}
	else if (inst == 0b0000'0101'0100)
	{
		// 054 Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
		write_data_mem(get_h_shifted() | 0, get_a1());
		write_data_mem(get_h_shifted() | 1, get_a2());
		write_data_mem(get_h_shifted() | 2, get_a3());
		write_data_mem(get_h_shifted() | 3, get_a4());
	}
	else if (inst == 0b0000'0101'1000)
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
	}
	else if (inst == 0b0000'0101'1100)
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
	}
	else if (inst == 0b0000'0110'0000)
	{
		// 060 Subroutine End, Pop down address stack, Skip
		u16 addr = pull_from_stack();
		set_new_pc(addr);
		m_skip = 1;
	}
	else if (inst == 0b0011'0000'1000)
	{
		// 308 Move A1[7:1] to FLS[7:1], 0->L[2:1]
		u8 a1 = get_a1();
		set_fls(a1);
		set_l(0);
	}
	else if (inst == 0b0011'0100'1000)
	{
		// 348 Move A2[7:1] to FLS[7:1], 0->L[2:1]
		u8 a2 = get_a2();
		set_fls(a2);
		set_l(0);
	}
	else if (inst == 0b0011'1000'1000)
	{
		// 388 Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
		u8 m = get_m_data();
		set_fls(m);
		set_l(0);
	}
	else if (inst == 0b0011'0000'1001)
	{
		// 309 Move A1[7:1] to FRS[7:1], 1->L[2:1]
		u8 a1 = get_a1();
		set_frs(a1);
		set_l(1);
	}
	else if (inst == 0b0011'0100'1001)
	{
		// 349 Move A2[7:1] to FRS[7:1], 1->L[2:1]
		u8 a2 = get_a2();
		set_frs(a2);
		set_l(1);
	}
	else if (inst == 0b0011'1000'1001)
	{
		// 389 Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
		u8 m = get_m_data();
		set_frs(m);
		set_l(1);
	}
	else if ((inst == 0b0000'0010'1000) || (inst == 0b0000'0010'1001))
	{
		// 028 Shift STB[4:1], N->STB[1]
		// STB is an input strobe / shifter
		const int n = inst & 1;
		LOGMASKED(LOG_UNHANDLED_OPS, "0x%d->STB\n", n);

		m_stb = (m_stb << 1) | n;
		m_stb &= 0xf;
	}

	else if ((inst == 0b0011'0000'1010) || (inst == 0b0011'0000'1011))
	{
		// 30a Move A1[7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		u8 a1 = get_a1();
		set_mode(a1);
		set_l(n);
	}
	else if ((inst == 0b0011'01001010) || (inst == 0b0011'0100'1011))
	{
		// 34a Move A2[7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		u8 a2 = get_a2();
		set_mode(a2);
		set_l(n);
	}
	else if ((inst == 0b0011'1000'1010) || (inst == 0b00111000'1011))
	{
		// 38a Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		u8 m = get_m_data();
		set_mode(m);
		set_l(n);
	}
	else if ((inst == 0b0100'0000'0000) || (inst == 0b0100'0000'0001))
	{
		// 400 N->A[11]
		const int n = inst & 0x1;
		set_a11(n);
	}
	else if ((inst == 0b0100'0000'0010) || (inst == 0b0100'0000'0011))
	{
		// 402 Jump to (000,M[H[5:1],L[2:1]][5:1],1N), 0->L[2:1], N->A[11]
		const int n = inst & 0x1;
		LOGMASKED(LOG_UNHANDLED_OPS, "JPM, 0->L, %d->A11\n", n);
		set_a11(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0000'0000)
	{
		// 300 N->L[2:1]
		const int n = inst & 0x3;
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'0000)
	{
		// 310 Move A2[7:1] to A1[7:1], N->L[2:1]
		const int n = inst & 0x3;
		u8 a2 = get_a2();
		set_a1(a2);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0100'0000)
	{
		// 340 Move A1[7:1] to A2[7:1], N->L[2:1]
		const int n = inst & 0x3;
		u8 a1 = get_a1();
		set_a2(a1);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'1000)
	{
		// 318 Right shift A1[7:1], 0->A1[7], N->L[2:1]
		const int n = inst & 0x3;
		u8 a1 = get_a1();
		a1 = a1 >> 1;
		set_a1(a1);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0101'1000)
	{
		// 358 Right shift A2[7:1], 0->A2[7], N->L[2:1]
		const int n = inst & 0x3;
		u8 a2 = get_a2();
		a2 = a2 >> 1;
		set_a2(a2);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'1001'1000)
	{
		// 398 Right shift M[H[5:1],L[2:1]][7:1], 0->M[H[5:1],L[2:1]][7], N->L[2:1]
		const int n = inst & 0x3;
		u8 m = get_m_data();
		m = m >> 1;
		set_m_data(m);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'1100)
	{
		// 31c Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
		const int n = inst & 0x3;
		u8 a1 = get_a1();
		u8 a2 = get_a2();
		a2 = a1 - a2;
		if (a2 & 0x80)
			m_skip = 1;
		set_a2(a2);
		set_l(n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0100'1100)
	{
		// 34c Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
		const int n = inst & 0x3;
		u8 a1 = get_a1();
		u8 a2 = get_a2();
		a1 = a2 - a1;
		if (a1 & 0x80)
			m_skip = 1;
		set_a1(a1);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'0000)
	{
		// 380 Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
		// 390 Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 src = get_a1_or_a2(reg);
		set_m_data(src);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'0100)
	{
		// 384 Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
		// 394 Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 src = get_a1_or_a2(reg);
		u8 m = get_m_data();
		set_m_data(src);
		set_a1_or_a2(reg,m);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'1100)
	{
		// 38c Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
		// 39c Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 m = get_m_data();
		set_a1_or_a2(reg,m);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1100'0000)
	{
		// 3c0 Move A1[5:1] to H[5:1], N->L[2:1]
		// 3d0 Move A2[5:1] to H[5:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 src = get_a1_or_a2(reg);
		set_h(src);
		set_l(n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1100'1100)
	{
		// 3cc Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
		// 3dc Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		u8 h = get_h() & 0x1f;
		set_a1_or_a2(reg,h);
		set_l(n);
	}
	else if ((inst & 0b1111'1011'0011) == 0b0000'0011'0000)
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
	}
	else
	{
		LOGMASKED(LOG_UNHANDLED_OPS, "%04x <ILLEGAL>\n", inst);
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool upd777_cpu_device::get_vbl_state()
{
	int vpos = m_screen->vpos();
	if (vpos > 240)
		return true;

	return false;
}

bool upd777_cpu_device::get_hbl_4_state()
{
	// I *think* this is Hblank for every 4th line (so a new display list can be written?)
	int vpos = m_screen->vpos();
	if ((vpos % 4) == 0)
	{
		int hpos = m_screen->hpos();
		if (hpos > 200)
			return true;
	}

	return false;
}

uint32_t upd777_cpu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// this needs to be scanline based, drawing whatever has been pushed to the linebuffer for the current
	// group of scanlines!

	bitmap.fill(0, cliprect);
	gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *gfx2 = m_gfxdecode->gfx(1);

	for (int i = 0; i <= 0x18; i++)
	{
		u8 s0 = m_datamem[(i * 4) + 0];
		u8 s1 = m_datamem[(i * 4) + 1];
		u8 s2 = m_datamem[(i * 4) + 2];
		u8 s3 = m_datamem[(i * 4) + 3];

		int ypos = (s0 & 0x7e) >> 1;
		//int prio = (s0 & 0x01);
		int xpos = (s1 & 0x7f);
		int patn = (s2 & 0x7f);
		//int ylow = (s3 & 0x70);
		int pal = (s3 & 0x0e) >> 1;
		//int ysub = (s3 & 0x01);

		if (patn<0x70)
			gfx->zoom_transpen(bitmap, cliprect, patn, pal, 0, 0, xpos * 4, ypos * 4, 0x40000, 0x40000, 0);
		else
			gfx2->zoom_transpen(bitmap, cliprect, patn-0x70, pal, 0, 0, xpos * 4, ypos * 4, 0x40000, 0x40000, 0);
	}

	return 0;
}

// documentation says patterns 0x00 - 0x6e are 7x7
// and patterns 0x70 - 0x7e are 8x7
// but they all seem to be stored at 11x7, just with some columns blank?
// this is probably because of how they were read out, over 11 data lines

// 0x00-0x2f are 'Normal (7x7)
// 0x30-0x67 are 'Bent' (7x7)
// 0x68-0x6f are 'Y Repeat' (7x7)
// 0x70-0x77 are 'XY Repeat' (8x7)
// 0x78-0x7f are 'X Repeat' (8x7)
// 
// NOTE, sprite patterns *7 and *f are unused so documentation expresses these ranges as to 66, 6e etc. rather than 67 6f
//
// it isn't clear how the 'Bent' effect etc. is enabled, as clearly not all patterns in this range should use it?

static const gfx_layout test_layout =
{
	7,7,
	0x70,
	1,
	{ 0 },
	{ 4,5,6,7,8,9,10 },
	{ 0*11,1*11,2*11,3*11,4*11,5*11,6*11 },
	7*11
};

static const gfx_layout test2_layout =
{
	8,7,
	0x10,
	1,
	{ 0 },
	{ 3,4,5,6,7,8,9,10 },
	{ 0*11,1*11,2*11,3*11,4*11,5*11,6*11 },
	7*11
};

static GFXDECODE_START( gfx_ud777 )
	GFXDECODE_ENTRY( "patterns", 0x000, test_layout,  0, 8 )
	GFXDECODE_ENTRY( "patterns", 0x436, test2_layout, 0, 8 )
GFXDECODE_END


void upd777_cpu_device::palette_init(palette_device &palette) const
{
	// just a fake palette for now
	for (int i = 0; i < palette.entries(); i++)
	{
		if (i & 1)
		{
			palette.set_pen_color(i, rgb_t(((i >> 1) & 1) ? 0xff : 0x7f, ((i >> 2) & 1) ? 0xff : 0x7f, ((i >> 3) & 1) ? 0xff : 0x7f));
		}
		else
		{
			palette.set_pen_color(i, rgb_t(0, 0, 0));
		}
	}
}

void upd777_cpu_device::push_to_line_buffer(u8 h, u8 m1, u8 m2, u8 m3, u8 m4)
{
	logerror("sprite %02x pushed to line buffer at scanline %d hpos %d: details %02x %02x %02x %02x\n", h, m_screen->vpos(), m_screen->hpos(), m1, m2, m3, m4);
}

TIMER_DEVICE_CALLBACK_MEMBER(upd777_cpu_device::scanline_timer)
{
	int scanline = param;

	logerror("scanline %d\n", scanline);
}

void upd777_cpu_device::device_add_mconfig(machine_config &config)
{
	// or pass the screen from the driver?
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-0-1);
	m_screen->set_screen_update(FUNC(upd777_cpu_device::screen_update));
	m_screen->set_palette(m_palette);

	TIMER(config, "scantimer").configure_scanline(FUNC(upd777_cpu_device::scanline_timer), "screen", 0, 1);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ud777);
	PALETTE(config, m_palette, FUNC(upd777_cpu_device::palette_init), 32 * 3).set_entries(0x10);
}

ROM_START( upd777 )
	ROM_REGION16_BE( 0x1000, "prg", ROMREGION_ERASEFF )
	ROM_REGION( 0x4d0, "patterns", ROMREGION_ERASEFF )
ROM_END

const tiny_rom_entry *upd777_cpu_device::device_rom_region() const
{
	return ROM_NAME(upd777);
}

