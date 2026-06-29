// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary eDSP 16-bit execution core.

***************************************************************************/

#include "emu.h"
#include "edsp.h"
#include "edspdasm.h"

DEFINE_DEVICE_TYPE(EMG2000A, emg2000a_device, "emg2000a", "Elan eMG2000A TV Game Processor")

constexpr u16 SR_GIE = 0x0020; // 0x8000 on eSL/eSLS

edsp_device::edsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program_map, address_map_constructor data_map, address_map_constructor io_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, -1, program_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1, data_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 7, -1, io_map)
	, m_sp(0)
	, m_r{0, 0, 0, 0, 0, 0, 0, 0}
{
}

emg2000a_device::emg2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: edsp_device(mconfig, EMG2000A, tag, owner, clock,
		address_map_constructor(FUNC(emg2000a_device::program_map), this),
		address_map_constructor(FUNC(emg2000a_device::data_map), this),
		address_map_constructor(FUNC(emg2000a_device::io_map), this))
	, m_in_pa_cb(*this, 0xffff)
	, m_out_pa_cb(*this)
	, m_in_pb_cb(*this, 0xff)
{
}

void emg2000a_device::program_map(address_map &map)
{
	// TODO: internal PRAM
	//map(0x007000, 0x007fff).ram();
}

void emg2000a_device::data_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // 2KW WRAM
	map(0x2000, 0x2fff).ram(); // 4KW BGA
	map(0x3000, 0x33ff).ram(); // 1KW SPA
	map(0x3400, 0x73ff).ram(); // 16KW VRAM
	//map(0x8000, 0x8fff); // PPU Register
	//map(0x9000, 0x9fff); // APU Register
	map(0x9023, 0x9023).nopr();
	map(0xe000, 0xe3ff).ram(); // Palette table
}

void emg2000a_device::io_map(address_map &map)
{
	map(0x01, 0x01).rw(FUNC(emg2000a_device::sr_r), FUNC(emg2000a_device::sr_w));
	map(0x02, 0x02).ram(); // TODO: saved and restored during interrupts
	map(0x03, 0x03).rw(FUNC(emg2000a_device::bank_r), FUNC(emg2000a_device::bank_w));
	map(0x09, 0x09).rw(FUNC(emg2000a_device::porta_r), FUNC(emg2000a_device::porta_w));
	map(0x0a, 0x0a).r(FUNC(emg2000a_device::portb_r));
	map(0x0c, 0x0d).rw(FUNC(emg2000a_device::inte_r), FUNC(emg2000a_device::inte_w));
	map(0x0e, 0x0f).rw(FUNC(emg2000a_device::intf_r), FUNC(emg2000a_device::intf_w));
	map(0x13, 0x13).rw(FUNC(emg2000a_device::spa_r), FUNC(emg2000a_device::spa_w));
	map(0x14, 0x16).ram(); // TODO: saved and restored during interrupts
	map(0x21, 0x21).rw(FUNC(emg2000a_device::pdira_r), FUNC(emg2000a_device::pdira_w));
	map(0x24, 0x24).rw(FUNC(emg2000a_device::pcona_r), FUNC(emg2000a_device::pcona_w));
	map(0x40, 0x43).rw(FUNC(emg2000a_device::timer01_r), FUNC(emg2000a_device::timer01_w));
	// TODO: lots of other ports and registers
}

device_memory_interface::space_config_vector edsp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> edsp_device::create_disassembler()
{
	return std::make_unique<edsp_disassembler>();
}

void edsp_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_io);

	set_icountptr(m_icount);

	state_add(EDSP_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(EDSP_SP, "SP", m_sp);
	state_add(EDSP_RC, "RC", m_rcr);
	state_add(EDSP_LC, "LC", m_lcr);
	state_add(EDSP_LSA, "LSA", m_lsa);
	state_add(EDSP_LEA, "LEA", m_lea);
	state_add(EDSP_SR, "SR", m_sr);
	for (int n = 0; n < 8; n++)
		state_add(EDSP_R0 + n, util::string_format("R%d", n).c_str(), m_r[n]);
	state_add(EDSP_INTE, "INTE", m_inte);
	state_add(EDSP_INTF, "INTF", m_intf);

	for (int n = 0; n < 2; n++)
		m_timer01[n] = timer_alloc(FUNC(edsp_device::timer01_interrupt), this);

	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sp));
	save_item(NAME(m_rcr));
	save_item(NAME(m_lcr));
	save_item(NAME(m_lsa));
	save_item(NAME(m_lea));
	save_item(NAME(m_sr));
	save_item(NAME(m_r));
	save_item(NAME(m_inte));
	save_item(NAME(m_intf));
	save_item(NAME(m_bank));
	save_item(NAME(m_trl));
	save_item(NAME(m_tcon));
}

void emg2000a_device::device_start()
{
	edsp_device::device_start();

	save_item(NAME(m_pdata));
	save_item(NAME(m_pdira));
	save_item(NAME(m_pcona));
}

void edsp_device::device_reset()
{
	m_pc = 0;
	m_rcr = 0;
	m_lcr = 0;
	m_lsa = 0;
	m_lea = 0;
	m_sr = 0;
	m_inte = 0;
	m_intf = 0;
	m_bank = 0;

	for (int n = 0; n < 2; n++)
	{
		m_trl[n] = 0;
		m_tcon[n] = 0;
		m_timer01[n]->enable(false);
	}
}

void emg2000a_device::device_reset()
{
	edsp_device::device_reset();

	m_pdata = 0;
	m_pdira = 0;
	m_pcona = 0;
}

u16 edsp_device::sr_r()
{
	return m_sr;
}

void edsp_device::sr_w(u16 data)
{
	m_sr = data;
}

u16 edsp_device::bank_r()
{
	return m_bank;
}

void edsp_device::bank_w(u16 data)
{
	m_bank = data;
}

u16 edsp_device::inte_r(offs_t offset)
{
	return BIT(m_inte, offset ? 16 : 0, 16);
}

void edsp_device::inte_w(offs_t offset, u16 data)
{
	if (offset)
		m_inte = u32(data) << 16 | (m_inte & 0x0000ffff);
	else
		m_inte = data | (m_inte & 0xffff0000);
}

u16 edsp_device::intf_r(offs_t offset)
{
	return BIT(m_intf, offset ? 16 : 0, 16);
}

void edsp_device::intf_w(offs_t offset, u16 data)
{
	if (offset)
		m_intf = u32(data) << 16 | (m_intf & 0x0000ffff);
	else
		m_intf = data | (m_intf & 0xffff0000);
}

u16 edsp_device::spa_r()
{
	return m_sp;
}

void edsp_device::spa_w(u16 data)
{
	m_sp = data;
}

u16 emg2000a_device::porta_r()
{
	return (m_in_pa_cb() & ~m_pdira) | (m_pdata & m_pdira);
}

void emg2000a_device::porta_w(u16 data)
{
	m_pdata = data;
	if (m_pdira)
		m_out_pa_cb(0, m_pdata | ~m_pdira, m_pdira);
}

u16 emg2000a_device::portb_r()
{
	// TODO: data direction
	return m_in_pb_cb();
}

u16 emg2000a_device::pdira_r()
{
	return m_pdira;
}

void emg2000a_device::pdira_w(u16 data)
{
	m_pdira = data;
	m_out_pa_cb(0, m_pdata | ~m_pdira, m_pdira);
}

u16 emg2000a_device::pcona_r()
{
	return m_pcona;
}

void emg2000a_device::pcona_w(u16 data)
{
	m_pcona = data;
}

u16 edsp_device::timer01_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_tcon[offset >> 1];
	else
		return m_trl[offset >> 1];
}

void edsp_device::timer01_w(offs_t offset, u16 data)
{
	const int which = offset >> 1;
	if (BIT(offset, 0))
	{
		m_tcon[which] = data;
		if (BIT(data, 15) && !m_timer01[which]->enabled())
		{
			const attotime period = clocks_to_attotime((m_trl[which] + 1) << (m_tcon[which] + 6));
			logerror("Timer %d enabled at %.3f Hz\n", which, period.as_hz());
			m_timer01[which]->adjust(period, which);
		}
		else if (!BIT(data, 15) && m_timer01[which]->enabled())
		{
			logerror("Timer %d disabled\n", which);
			m_timer01[which]->enable(false);
		}
	}
	else
		m_trl[which] = BIT(data, 0, 8);
}

TIMER_CALLBACK_MEMBER(edsp_device::timer01_interrupt)
{
	m_intf |= 1 << (param + 1);

	const attotime period = clocks_to_attotime((m_trl[param] + 1) << (m_tcon[param] + 6));
	m_timer01[param]->adjust(period, param);
}

u16 edsp_device::add(u16 s, u16 t, bool c) noexcept
{
	u32 d = u32(s) + t + (c ? 1 : 0);
	m_sr = (m_sr & 0xfff0)
			| (BIT(d, 15) ? 0x0008 : 0)
			| (u16(d) ? 0 : 0x0004)
			| (BIT((d ^ s) & ~(s ^ t), 15) ? 0x0002 : 0)
			| (BIT(d, 16) ? 0x0001 : 0);
	return u16(d);
}

bool edsp_device::test_condition(u8 cond) const noexcept
{
	switch (cond)
	{
	case 0: // LO or CC: C==0
		return !BIT(m_sr, 0);

	case 1: // HS or CS: C==1
		return BIT(m_sr, 0);

	case 2: // VC: V==0
		return !BIT(m_sr, 1);

	case 3: // VS: V==1
		return BIT(m_sr, 1);

	case 4: // NE: Z==0
		return !BIT(m_sr, 2);

	case 5: // EQ: Z==1
		return BIT(m_sr, 2);

	case 6: // PL: N==0
		return !BIT(m_sr, 3);

	case 7: // MI: N==1
		return BIT(m_sr, 3);

	case 8: // TC: T==0
		return !BIT(m_sr, 4);

	case 9: // TS: T==1
		return BIT(m_sr, 4);

	case 10: // GE: (N^V)==0
		return BIT(m_sr, 3) == BIT(m_sr, 1);

	case 11: // LT: (N^V)==1
		return BIT(m_sr, 3) != BIT(m_sr, 1);

	case 12: // GT: Z|(N^V)==0
		return BIT(m_sr, 3) == BIT(m_sr, 1) && !BIT(m_sr, 2);

	case 13: // LE: Z|(N^V)==1
		return BIT(m_sr, 3) != BIT(m_sr, 1) || BIT(m_sr, 2);

	case 14: // LS: (C==0)|(Z==1)
		return !BIT(m_sr, 0) || BIT(m_sr, 2);

	case 15: // unconditional
		return true;

	default: // should never happen
		return false;
	}
}

u16 edsp_device::read_program_word(u16 addr)
{
	return m_program.read_word(addr >= 0x8000 ? addr + (u32(BIT(m_bank, 0, 9)) << 15) : addr);
}

void edsp_device::execute_run()
{
	do
	{
		if ((m_sr & SR_GIE) && (m_inte & m_intf) && !m_rcr)
		{
			// TODO: respect INTP
			const int i = std::countr_zero(m_inte & m_intf);
			standard_irq_callback(i, m_pc);

			m_data.write_word(m_sp, m_pc);
			m_sp--;
			m_pc = 0x0006 + i * 2;
			m_sr &= ~SR_GIE;
			m_icount -= 2;
		}
		else
		{
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);
			const u16 op = m_cache.read_word(m_pc);
			if (m_rcr)
				m_rcr--;
			else
				m_pc++;

			if (op < 0x3800)
			{
				const u16 s = BIT(op, 3, 2) == 2 ? m_data.read_word(BIT(op, 5, 3)) : m_r[BIT(op, 3, 2) == 3 ? BIT(op, 8, 3) : BIT(op, 5, 3)];
				const u16 t = BIT(op, 3, 2) == 3 ? BIT(op, 5, 3) << 3 | BIT(op, 0, 3) : m_r[BIT(op, 0, 3)];

				u16 d = 0;
				switch (BIT(op, 11, 3))
				{
				case 0: // ADD
					d = add(s, t, false);
					break;

				case 1: // ADC
					d = add(s, t, BIT(m_sr, 0));
					break;

				case 2: // SUB
					d = add(s, ~t, true);
					break;

				case 3: // SUBB
					d = add(s, ~t, BIT(m_sr, 0));
					break;

				case 4: // AND
					d = s & t;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;

				case 5: // OR
					d = s | t;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;

				case 6: // XOR
					d = s ^ t;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;
				}

				if (BIT(op, 3, 2) == 1)
					m_data.write_word(m_r[BIT(op, 8, 3)], d);
				else
					m_r[BIT(op, 8, 3)] = d;
				m_icount -= 1;
			}
			else if ((op & 0xf810) == 0x3800 && BIT(op, 0, 3) != 7)
			{
				const u16 s = m_r[BIT(op, 5, 3)];
				const u16 imm16 = m_cache.read_word(m_pc);

				u16 d = 0;
				switch (BIT(op, 0, 3))
				{
				case 0: // ADD
					d = add(s, imm16, false);
					break;

				case 1: // ADC
					d = add(s, imm16, BIT(m_sr, 0));
					break;

				case 2: // SUB
					d = add(s, ~imm16, true);
					break;

				case 3: // SUBB
					d = add(s, ~imm16, BIT(m_sr, 0));
					break;

				case 4: // AND
					d = s & imm16;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;

				case 5: // OR
					d = s | imm16;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;

				case 6: // XOR
					d = s ^ imm16;
					m_sr = (m_sr & 0xfff0) | (s16(d) < 0 ? 0x0008 : 0) | (d == 0 ? 0x0004 : 0);
					break;
				}

				if (BIT(op, 3))
					m_data.write_word(m_r[BIT(op, 8, 3)], d);
				else
					m_r[BIT(op, 8, 3)] = d;
				m_pc++;
				m_icount -= 2;
			}
			else if ((op & 0xf8ff) == 0x3817)
			{
				// RPT Rn
				m_rcr = m_r[BIT(op, 8, 3)];
				m_icount -= 1;
			}
			else if ((op & 0xf87f) == 0x3818)
			{
				// IF cond JMP Long_addr
				if (test_condition(BIT(op, 7, 4)))
				{
					const u16 addr = m_cache.read_word(m_pc);
					m_pc = addr;
				}
				else
					m_pc++;
				m_icount -= 2;
			}
			else if (op == 0x3819)
			{
				// CALL Long_addr
				const u16 addr = m_cache.read_word(m_pc);
				m_data.write_word(m_sp, m_pc + 1);
				m_sp--;
				m_pc = addr;
				m_icount -= 2;
			}
			else if (op == 0x381a)
			{
				// NOP
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x381f)
			{
				// RPT #imm6
				m_rcr = BIT(op, 5, 6);
				m_icount -= 1;
			}
			else if (op == 0x383a)
			{
				// RET
				m_sp++;
				m_pc = m_data.read_word(m_sp);
				m_icount -= 2;
			}
			else if (op == 0x385a)
			{
				// RETI
				m_sp++;
				m_pc = m_data.read_word(m_sp);
				m_sr |= SR_GIE;
				m_icount -= 2;
			}
			else if ((op & 0xf81f) == 0x381b)
			{
				m_sp += BIT(op, 5, 6);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x381c)
			{
				m_sp -= BIT(op, 5, 6);
				m_icount -= 1;
			}
			else if ((op & 0xff18) == 0x4000)
			{
				// MUL (signs unknown, assuming UU for now)
				const u32 d = u32(m_r[BIT(op, 5, 3)]) * m_r[BIT(op, 0, 3)];
				m_r[0] = BIT(d, 0, 16);
				m_r[1] = BIT(d, 16, 16);
				m_icount -= 1;
			}
			else if ((op & 0xfe18) == 0x4818)
			{
				// CMP
				const u16 s = BIT(op, 8) ? m_data.read_word(m_r[BIT(op, 5, 3)]) : m_r[BIT(op, 5, 3)];
				const u16 t = m_r[BIT(op, 0, 3)];
				(void)add(s, ~t, true);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5801)
			{
				m_r[BIT(op, 8, 3)] = m_r[BIT(op, 5, 3)];
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5803)
			{
				m_r[BIT(op, 8, 3)] = m_data.read_word(m_r[BIT(op, 5, 3)]);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5805)
			{
				const u16 data = m_data.read_word(m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 5, 3)]++;
				m_r[BIT(op, 8, 3)] = data;
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5806)
			{
				// SHL
				const u16 s = m_r[BIT(op, 5, 3)];
				m_r[BIT(op, 8, 3)] = add(s, s, false);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5807)
			{
				const u16 data = m_data.read_word(m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 5, 3)]--;
				m_r[BIT(op, 8, 3)] = data;
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5808)
			{
				// ASR
				const u16 s = m_r[BIT(op, 5, 3)];
				m_r[BIT(op, 8, 3)] = s16(s) >> 1;
				m_sr = (m_sr & 0xfff0) | (s16(s) < 0 ? 0x0008 : 0) | (s16(s) >> 1 ? 0 : 0x0004) | (BIT(s, 0) ? 0x0001 : 0);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x5809)
			{
				const u16 data = read_program_word(m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 8, 3)] = data;
				m_icount -= 2; // TODO: repeat timing
			}
			else if ((op & 0xf81f) == 0x580a)
			{
				// SHR
				const u16 s = m_r[BIT(op, 5, 3)];
				m_r[BIT(op, 8, 3)] = s >> 1;
				m_sr = (m_sr & 0xfff0) | (s >> 1 ? 0 : 0x0004) | (BIT(s, 0) ? 0x0003 : 0);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x580b)
			{
				const u16 data = read_program_word(m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 5, 3)]++;
				m_r[BIT(op, 8, 3)] = data;
				m_icount -= 2; // TODO: repeat timing
			}
			else if ((op & 0xf81f) == 0x580f)
			{
				const u16 data = read_program_word(m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 5, 3)]++;
				m_data.write_word(m_r[BIT(op, 8, 3)], data);
				m_r[BIT(op, 8, 3)]++;
				m_icount -= 2; // TODO: repeat timing
			}
			else if ((op & 0xf81f) == 0x581b)
			{
				m_data.write_word(m_r[BIT(op, 8, 3)], m_r[BIT(op, 5, 3)]);
				m_icount -= 1;
			}
			else if ((op & 0xf81f) == 0x581d)
			{
				m_data.write_word(m_r[BIT(op, 8, 3)], m_r[BIT(op, 5, 3)]);
				m_r[BIT(op, 8, 3)]++;
				m_icount -= 1;
			}
			else if ((op & 0xf8ff) == 0x581e)
			{
				// PUSH
				m_data.write_word(m_sp, m_r[BIT(op, 8, 3)]);
				m_sp--;
				m_icount -= 1;
			}
			else if ((op & 0xf8ff) == 0x583e)
			{
				// POP
				m_sp++;
				m_r[BIT(op, 8, 3)] = m_data.read_word(m_sp);
				m_icount -= 1;
			}
			else if ((op & 0xf8ff) == 0x585e)
			{
				m_pc = m_r[BIT(op, 8, 3)];
				m_icount -= 2;
			}
			else if ((op & 0xf8ff) == 0x589e)
			{
				m_r[BIT(op, 8, 3)] = m_data.read_word(m_cache.read_word(m_pc));
				m_pc++;
				m_icount -= 2;
			}
			else if ((op & 0xf8ff) == 0x58be)
			{
				m_data.write_word(m_cache.read_word(m_pc), m_r[BIT(op, 8, 3)]);
				m_pc++;
				m_icount -= 2;
			}
			else if ((op & 0xf8ff) == 0x58de)
			{
				m_data.write_word(m_r[BIT(op, 8, 3)], m_cache.read_word(m_pc));
				m_pc++;
				m_icount -= 2;
			}
			else if ((op & 0xf800) == 0x6000)
			{
				// MOV low immediate
				m_r[BIT(op, 8, 3)] = BIT(op, 0, 8);
				m_icount -= 1;
			}
			else if ((op & 0xf800) == 0x6800)
			{
				// MOV high immediate
				m_r[BIT(op, 8, 3)] = (m_r[BIT(op, 8, 3)] & 0x00ff) | BIT(op, 0, 8) << 8;
				m_icount -= 1;
			}
			else if ((op & 0xe000) == 0x8000)
			{
				// IF cond JMP Short_addr
				if (test_condition(BIT(op, 9, 4)))
				{
					m_pc += util::sext(op, 9);
					m_icount -= 2;
				}
				else
					m_icount -= 1;
			}
			else if ((op & 0xff00) == 0xa000)
			{
				const u16 data = m_data.read_word(m_r[3] - BIT(op, 0, 5));
				m_r[BIT(op, 5, 3)] = data;
				m_icount -= 1;
			}
			else if ((op & 0xff00) == 0xa400)
			{
				m_data.write_word(m_r[3] - BIT(op, 0, 5), m_r[BIT(op, 5, 3)]);
				m_icount -= 1;
			}
			else if ((op & 0xff80) == 0xa800)
			{
				m_r[BIT(op, 0, 3)] |= u16(1) << BIT(op, 3, 4);
				m_icount -= 1;
			}
			else if ((op & 0xff00) == 0xa900)
			{
				const u8 port = BIT(op, 7) << 3 | BIT(op, 0, 3);
				m_io.write_word(port, m_io.read_word(port) | u16(1) << BIT(op, 3, 4));
				m_icount -= 1;
			}
			else if ((op & 0xff80) == 0xaa00)
			{
				m_r[BIT(op, 0, 3)] &= ~(u16(1) << BIT(op, 3, 4));
				m_icount -= 1;
			}
			else if ((op & 0xff00) == 0xab00)
			{
				const u8 port = BIT(op, 7) << 3 | BIT(op, 0, 3);
				m_io.write_word(port, m_io.read_word(port) & ~(u16(1) << BIT(op, 3, 4)));
				m_icount -= 1;
			}
			else if ((op & 0xff80) == 0xac00)
			{
				if (BIT(m_r[BIT(op, 0, 3)], BIT(op, 3, 4)))
					m_sr |= 0x0010;
				else
					m_sr &= 0xffef;
				m_icount -= 1;
			}
			else if ((op & 0xf880) == 0xb000)
			{
				// IN
				m_r[BIT(op, 8, 3)] = m_io.read_word(BIT(op, 0, 7));
				m_icount -= 1;
			}
			else if ((op & 0xf880) == 0xb080)
			{
				// OUT
				m_io.write_word(BIT(op, 0, 7), m_r[BIT(op, 8, 3)]);
				m_icount -= 1;
			}
			else if ((op & 0xff80) == 0xb800)
			{
				// PUSH IO[addr]
				m_data.write_word(m_sp, m_io.read_word(BIT(op, 0, 7)));
				m_sp--;
				m_icount -= 1;
			}
			else if ((op & 0xff80) == 0xb880)
			{
				// POP IO[addr]
				m_sp++;
				m_io.write_word(BIT(op, 0, 7), m_data.read_word(m_sp));
				m_icount -= 1;
			}
			else if (op >= 0xc000)
			{
				// CALL Short_addr
				m_data.write_word(m_sp, m_pc);
				m_sp--;
				m_pc = BIT(op, 0, 14);
				m_icount -= 2;
			}
			else
			{
				logerror("Unemulated/undefined instruction 0x%04X @ PC = 0x%04X\n", op, m_ppc);
				m_icount -= 1;
			}
		}
	}
	while (m_icount > 0);
}
