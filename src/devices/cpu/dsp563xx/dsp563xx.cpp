// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx base emulation

#include "emu.h"
#include "dsp563xx.h"
#include "dsp563xxd.h"

dsp563xx_device::dsp563xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
								 address_map_constructor map_p, address_map_constructor map_x, address_map_constructor map_y) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_hi08(*this, "hi08"),
	m_shi(*this, "shi"),
	m_p_config("p", ENDIANNESS_LITTLE, 32, 24, -2, map_p),
	m_x_config("x", ENDIANNESS_LITTLE, 32, 24, -2, map_x),
	m_y_config("y", ENDIANNESS_LITTLE, 32, 24, -2, map_y)
{
}

void dsp563xx_device::device_start()
{
	space(AS_P).cache(m_p);
	space(AS_X).specific(m_x);
	space(AS_Y).specific(m_y);

	using namespace std::placeholders;
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_ccr).formatstr("%8s").noshow();
	state_add(DSP563XX_PC,     "PC",        m_pc).formatstr("%06X");
	state_add(DSP563XX_LA, "LA", m_la).formatstr("%06X");
	state_add(DSP563XX_LC, "LC", m_lc).formatstr("%06X");
	state_add<u32>(DSP563XX_SR, "SR", std::bind(&dsp563xx_device::get_sr, this), std::bind(&dsp563xx_device::set_sr, this, _1)).formatstr("%06X");
	state_add(DSP563XX_EMR, "EMR", m_emr).noshow();
	state_add(DSP563XX_MR, "MR", m_mr).noshow();
	state_add(DSP563XX_CCR, "CCR", m_ccr).noshow();
	state_add(DSP563XX_OMR, "OMR", m_omr).formatstr("%06X");
	state_add<u32>(DSP563XX_SSH, "SSH", std::bind(&dsp563xx_device::get_ssh, this), std::bind(&dsp563xx_device::set_ssh, this, _1)).formatstr("%06X");
	state_add<u32>(DSP563XX_SSL, "SSL", std::bind(&dsp563xx_device::get_ssl, this), std::bind(&dsp563xx_device::set_ssl, this, _1)).formatstr("%06X");
	state_add(DSP563XX_A, "A", m_a).formatstr("%014X");
	state_add(DSP563XX_B, "B", m_b).formatstr("%014X");
	state_add(DSP563XX_X0, "X0", m_x0).formatstr("%06X");
	state_add(DSP563XX_X1, "X1", m_x1).formatstr("%06X");
	state_add(DSP563XX_Y0, "Y0", m_y0).formatstr("%06X");
	state_add(DSP563XX_Y1, "Y1", m_y1).formatstr("%06X");
	state_add(DSP563XX_R0, "R0", m_r[0]).formatstr("%06X");
	state_add(DSP563XX_R1, "R1", m_r[1]).formatstr("%06X");
	state_add(DSP563XX_R2, "R2", m_r[2]).formatstr("%06X");
	state_add(DSP563XX_R3, "R3", m_r[3]).formatstr("%06X");
	state_add(DSP563XX_R4, "R4", m_r[4]).formatstr("%06X");
	state_add(DSP563XX_R5, "R5", m_r[5]).formatstr("%06X");
	state_add(DSP563XX_R6, "R6", m_r[6]).formatstr("%06X");
	state_add(DSP563XX_R7, "R7", m_r[7]).formatstr("%06X");
	state_add(DSP563XX_N0, "N0", m_n[0]).formatstr("%06X");
	state_add(DSP563XX_N1, "N1", m_n[1]).formatstr("%06X");
	state_add(DSP563XX_N2, "N2", m_n[2]).formatstr("%06X");
	state_add(DSP563XX_N3, "N3", m_n[3]).formatstr("%06X");
	state_add(DSP563XX_N4, "N4", m_n[4]).formatstr("%06X");
	state_add(DSP563XX_N5, "N5", m_n[5]).formatstr("%06X");
	state_add(DSP563XX_N6, "N6", m_n[6]).formatstr("%06X");
	state_add(DSP563XX_N7, "N7", m_n[7]).formatstr("%06X");
	state_add(DSP563XX_M0, "M0", m_m[0]).formatstr("%06X");
	state_add(DSP563XX_M1, "M1", m_m[1]).formatstr("%06X");
	state_add(DSP563XX_M2, "M2", m_m[2]).formatstr("%06X");
	state_add(DSP563XX_M3, "M3", m_m[3]).formatstr("%06X");
	state_add(DSP563XX_M4, "M4", m_m[4]).formatstr("%06X");
	state_add(DSP563XX_M5, "M5", m_m[5]).formatstr("%06X");
	state_add(DSP563XX_M6, "M6", m_m[6]).formatstr("%06X");
	state_add(DSP563XX_M7, "M7", m_m[7]).formatstr("%06X");

	save_item(NAME(m_icount));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_tmp1));
	save_item(NAME(m_tmp2));
	save_item(NAME(m_stackh));
	save_item(NAME(m_stackl));
	save_item(NAME(m_pc));
	save_item(NAME(m_la));
	save_item(NAME(m_lc));
	save_item(NAME(m_temp_lc));
	save_item(NAME(m_vba));
	save_item(NAME(m_x0));
	save_item(NAME(m_x1));
	save_item(NAME(m_y0));
	save_item(NAME(m_y1));
	save_item(NAME(m_ep));
	save_item(NAME(m_omr));
	save_item(NAME(m_sp));
	save_item(NAME(m_sz));
	save_item(NAME(m_r));
	save_item(NAME(m_m));
	save_item(NAME(m_n));
	save_item(NAME(m_sc));
	save_item(NAME(m_emr));
	save_item(NAME(m_mr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_rep));

	std::fill(m_stackh.begin(), m_stackh.end(), 0);
	std::fill(m_stackl.begin(), m_stackl.end(), 0);
	std::fill(m_r.begin(), m_r.end(), 0);
	std::fill(m_n.begin(), m_n.end(), 0);
	m_a = 0;
	m_b = 0;
	m_tmp1 = 0;
	m_tmp2 = 0;
	m_x0 = 0;
	m_x1 = 0;
	m_y0 = 0;
	m_y1 = 0;
	m_sz = 0;
	m_ep = 0;
	m_npc = 0;
	m_skip = false;
	m_temp_lc = 0;

	set_icountptr(m_icount);
}

void dsp563xx_device::set_hard_omr(u8 mode)
{
	m_hard_omr = mode;
}

void dsp563xx_device::device_reset()
{
	std::fill(m_m.begin(), m_m.end(), 0xffffff);
	m_omr = 0x000300 | m_hard_omr;
	m_emr = 0xc0;
	m_mr = 0x03;
	m_ccr = 0x00;
	m_sp = 0;
	m_sc = 0;
	m_la = 0;
	m_lc = 0;
	m_vba = 0;
	m_pc = get_reset_vector();
	m_rep = false;
}

void dsp563xx_device::unhandled(const char *inst)
{
	logerror("Unhandled instruction %s\n", inst);
}

void dsp563xx_device::execute_run()
{
	while(m_icount > 0) {
		debugger_instruction_hook(m_pc);
		u32 opcode = m_p.read_dword(m_pc);
		bool loop = (m_mr & MR_LF) && m_pc == m_la;
		u16 kmove = t_move[opcode >> 8];
		u16 knpar = kmove || opcode >= 0x100000 ? 0 : t_npar[opcode];
		u16 kipar = knpar ? 0 : t_ipar[opcode & 0xff];
		bool ex = BIT(t_move_ex[kmove >> 6], kmove & 0x3f) || BIT(t_npar_ex[knpar >> 6], knpar & 0x3f);
		u32 exv;
		if(ex) {
			exv = m_p.read_dword(m_pc+1);
			loop = loop || ((m_mr & MR_LF) && m_pc+1 == m_la);
		} else
			exv = 0;
		if(m_rep) {
			if(m_lc != 1)
				m_lc = m_lc ? m_lc-1 : 0xffff;
			else {
				m_lc = m_temp_lc;
				m_npc = (m_pc + (ex ? 2 : 1)) & 0xffffff;
				m_rep = false;
			}
		}
		else {
			m_npc = (m_pc + (ex ? 2 : 1)) & 0xffffff;
			if(loop) {
				if(m_lc != 1 || (m_emr & EMR_FV)) {
					m_lc = m_lc ? m_lc-1 : 0xffff;
					m_npc = get_ssh();
				} else {
					set_sr(get_ssl());
					dec_sp();
					set_la(get_ssh());
					set_lc(get_ssl());
					dec_sp();
				}
			}
		}
		m_skip = false;
		if(kmove)
			execute_pre_move(kmove, opcode, exv);
		if(kipar && !m_skip)
			execute_ipar(kipar);
		if(kmove)
			execute_post_move(kmove, opcode, exv);
		if(knpar)
			execute_npar(knpar, opcode, exv);
		m_icount --;
		m_pc = m_npc;
	}
}

device_memory_interface::space_config_vector dsp563xx_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_P, &m_p_config),
		std::make_pair(AS_X, &m_x_config),
		std::make_pair(AS_Y, &m_y_config)
	};

}

void dsp563xx_device::state_import(const device_state_entry &entry)
{
}

void dsp563xx_device::state_export(const device_state_entry &entry)
{
}

void dsp563xx_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c",
							(m_ccr & CCR_S) ? 'S' : '-',
							(m_ccr & CCR_L) ? 'L' : '-',
							(m_ccr & CCR_E) ? 'E' : '-',
							(m_ccr & CCR_U) ? 'U' : '-',
							(m_ccr & CCR_N) ? 'N' : '-',
							(m_ccr & CCR_Z) ? 'Z' : '-',
							(m_ccr & CCR_V) ? 'V' : '-',
							(m_ccr & CCR_C) ? 'C' : '-');
		break;
	}
}

std::unique_ptr<util::disasm_interface> dsp563xx_device::create_disassembler()
{
	return std::make_unique<dsp563xx_disassembler>();
}

void dsp563xx_device::hi08_w(offs_t offset, u8 data)
{
	m_hi08->write(offset, data);
}

u8 dsp563xx_device::hi08_r(offs_t offset)
{
	return m_hi08->read(offset);
}


u64 dsp563xx_device::do_add56(u64 v1, u64 v2)
{
	m_ccr &= ~(CCR_E|CCR_U|CCR_N|CCR_Z|CCR_V|CCR_C);

	s64 r = util::sext(v2, 56) + v1;
	switch(m_mr & (MR_S0|MR_S1))
	{
	case 0:
		if(BIT(r, 47, 9) != 0 && BIT(r, 47, 9) != 0x1ff)
			m_ccr |= CCR_E;
		if(BIT(r, 47) == BIT(r, 46))
			m_ccr |= CCR_U;
		break;

	case MR_S0:
		if(BIT(r, 48, 8) != 0 && BIT(r, 48, 8) != 0xff)
			m_ccr |= CCR_E;
		if(BIT(r, 48) == BIT(r, 47))
			m_ccr |= CCR_U;
		break;

	case MR_S1:
		if(BIT(r, 46, 10) != 0 && BIT(r, 46, 10) != 0x3ff)
			m_ccr |= CCR_E;
		if(BIT(r, 46) == BIT(r, 45))
			m_ccr |= CCR_U;
		break;
	}

	if(BIT(r, 55))
		m_ccr |= CCR_N;
	if(!r)
		m_ccr |= CCR_Z;
	if(BIT(r, 56) != BIT(r, 55))
		m_ccr |= CCR_L|CCR_V;
	if(BIT(v2 + (v1 & 0xffffffffffffff), 56))
		m_ccr |= CCR_C;

	return r;
}

u64 dsp563xx_device::do_sub56(u64 v1, u64 v2)
{
	m_ccr &= ~(CCR_E|CCR_U|CCR_N|CCR_Z|CCR_V|CCR_C);

	s64 r = util::sext(v2, 56) - v1;
	switch(m_mr & (MR_S0|MR_S1))
	{
	case 0:
		if(BIT(r, 47, 9) != 0 && BIT(r, 47, 9) != 0x1ff)
			m_ccr |= CCR_E;
		if(BIT(r, 47) == BIT(r, 46))
			m_ccr |= CCR_U;
		break;

	case MR_S0:
		if(BIT(r, 48, 8) != 0 && BIT(r, 48, 8) != 0xff)
			m_ccr |= CCR_E;
		if(BIT(r, 48) == BIT(r, 47))
			m_ccr |= CCR_U;
		break;

	case MR_S1:
		if(BIT(r, 46, 10) != 0 && BIT(r, 46, 10) != 0x3ff)
			m_ccr |= CCR_E;
		if(BIT(r, 46) == BIT(r, 45))
			m_ccr |= CCR_U;
		break;
	}

	if(BIT(r, 55))
		m_ccr |= CCR_N;
	if(!r)
		m_ccr |= CCR_Z;
	if(BIT(r, 56) != BIT(r, 55))
		m_ccr |= CCR_L|CCR_V;
	if(BIT(v2 - (v1 & 0xffffffffffffff), 56))
		m_ccr |= CCR_C;

	return r;
}

u64 dsp563xx_device::do_asl56(u8 n, u64 v)
{
	m_ccr &= ~(CCR_E|CCR_U|CCR_N|CCR_Z|CCR_V|CCR_C);

	u64 r = (v << n) & 0xffffffffffffff;
	switch(m_mr & (MR_S0|MR_S1))
	{
	case 0:
		if(BIT(r, 47, 9) != 0 && BIT(r, 47, 9) != 0x1ff)
			m_ccr |= CCR_E;
		if(BIT(r, 47) == BIT(r, 46))
			m_ccr |= CCR_U;
		break;

	case MR_S0:
		if(BIT(r, 48, 8) != 0 && BIT(r, 48, 8) != 0xff)
			m_ccr |= CCR_E;
		if(BIT(r, 48) == BIT(r, 47))
			m_ccr |= CCR_U;
		break;

	case MR_S1:
		if(BIT(r, 46, 10) != 0 && BIT(r, 46, 10) != 0x3ff)
			m_ccr |= CCR_E;
		if(BIT(r, 46) == BIT(r, 45))
			m_ccr |= CCR_U;
		break;
	}

	if(BIT(r, 55))
		m_ccr |= CCR_N;
	if(!r)
		m_ccr |= CCR_Z;
	if((BIT(v, 55) ? v << 8 : ~v << 8) >> (64-n))
		m_ccr |= CCR_L|CCR_V;
	if(n != 0 && BIT(v, 56-n))
		m_ccr |= CCR_C;

	return r;
}

u64 dsp563xx_device::do_asr56(u8 n, u64 v)
{
	m_ccr &= ~(CCR_V|CCR_C);
	if(n != 0 && BIT(v, n-1))
		m_ccr |= CCR_C;

	return util::sext(v, BIT(m_emr, 1) ? 40 : 56) >> n;
}

void dsp563xx_device::do_tst56(u64 v)
{
	m_ccr &= ~(CCR_E|CCR_U|CCR_N|CCR_Z|CCR_V);

	switch(m_mr & (MR_S0|MR_S1))
	{
	case 0:
		if(BIT(v, 47, 9) != 0 && BIT(v, 47, 9) != 0x1ff)
			m_ccr |= CCR_E;
		if(BIT(v, 47) == BIT(v, 46))
			m_ccr |= CCR_U;
		break;

	case MR_S0:
		if(BIT(v, 48, 8) != 0 && BIT(v, 48, 8) != 0xff)
			m_ccr |= CCR_E;
		if(BIT(v, 48) == BIT(v, 47))
			m_ccr |= CCR_U;
		break;

	case MR_S1:
		if(BIT(v, 46, 10) != 0 && BIT(v, 46, 10) != 0x3ff)
			m_ccr |= CCR_E;
		if(BIT(v, 46) == BIT(v, 45))
			m_ccr |= CCR_U;
		break;
	}

	if(BIT(v, 55))
		m_ccr |= CCR_N;
	if(!v)
		m_ccr |= CCR_Z;
}
