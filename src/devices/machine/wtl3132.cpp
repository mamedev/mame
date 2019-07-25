// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the Weitek 3132 floating point data path.
 *
 * Sources:
 *   - http://www.bitsavers.org/components/weitek/dataSheets/WTL-3132_3332_XL-3132_32-Bit_Floating_Point_Data_Path_Oct88.pdf
 *
 * TODO:
 *   - reciprocal via lookup table
 *   - fully analyze reversed C bus case
 *   - testing/logging undefined cases
 */

#include "emu.h"
#include "wtl3132.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REGS    (1U << 1)
#define LOG_IO      (1U << 2)
#define LOG_BYPASS  (1U << 3)

#define VERBOSE (LOG_GENERAL|LOG_REGS|LOG_IO|LOG_BYPASS)
#include "logmacro.h"

ALLOW_SAVE_TYPE(float32_t);

// helper for extracting opcode fields
#define OPF(c, x) ((c & M_##x) >> S_##x)

DEFINE_DEVICE_TYPE(WTL3132, wtl3132_device, "wtl3132", "Weitek 3132")

wtl3132_device::wtl3132_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WTL3132, tag, owner, clock)
	, m_fpcn_cb(*this)
	, m_fpex_cb(*this)
	, m_zero_cb(*this)
{
}

void wtl3132_device::device_start()
{
	m_fpcn_cb.resolve_safe();
	m_fpex_cb.resolve_safe();
	m_zero_cb.resolve_safe();

	save_item(NAME(m_fpcn_state));
	save_item(NAME(m_fpex_state));
	save_item(NAME(m_zero_state));

	save_item(NAME(m_c_port));
	save_item(NAME(m_x_port));
	save_item(NAME(m_x_in));
	save_item(NAME(m_x_out));

	save_item(NAME(m_slot));
	save_item(NAME(m_head));

	save_item(NAME(m_mode));
	save_item(NAME(m_f));
	save_item(NAME(m_t1));
	save_item(NAME(m_t2));
	save_item(NAME(m_t3));
	save_item(NAME(m_cr));
	save_item(NAME(m_sr));
	save_item(NAME(m_zr));

	save_item(NAME(m_c_bus_data));
	save_item(NAME(m_c_bus_addr));
	save_item(NAME(m_c_bus_cwen));

	save_item(NAME(m_ma_in));
	save_item(NAME(m_mb_in));
	save_item(NAME(m_m_out));

	save_item(NAME(m_aa_in));
	save_item(NAME(m_ab_in));
	save_item(NAME(m_a_out));
}

void wtl3132_device::device_reset()
{
	m_abort = 1;
	m_neut = 1;
	m_stall = 1;

	m_fpcn_state = 1;
	m_fpex_state = 1;
	m_zero_state = 1;

	// initialize pipeline with fnop to minimize logging noise
	for (u64 &slot : m_slot)
		slot = (F_FSUB << S_F) | M_CWEN | M_ABIN | M_ADST;
	m_head = 0;

	m_mode = 0;
}

void wtl3132_device::clk_w(int state)
{
	// ignore falling edges for now
	if (!state)
		return;

	// precompute pipeline slot indixes
	unsigned const index0 = (m_head + 4) & 3;
	unsigned const index1 = (m_head + 3) & 3;
	unsigned const index2 = (m_head + 2) & 3;
	unsigned const index3 = (m_head + 1) & 3;

	LOG("code 0x%09x slot %d %s\n", m_c_port, index0, disassemble(m_c_port));

	// store the next instruction in the pipeline
	m_slot[index0] = m_c_port;

	// STALL- and ABORT- prevent all processing of the next instruction
	if (!m_stall || !m_abort)
	{
		LOG("slot %d stage instruction %s\n", index0, !m_stall ? "stalled" : "aborted");
		m_slot[index0] |= M_CANCEL;
	}

	// retire pipeline stages in fifo order simplifying dependency forwarding
	if (!(m_slot[index3] & M_CANCEL))
		stage4(index3);

	if (!(m_slot[index2] & M_CANCEL))
		stage3(index2);

	if (!(m_slot[index1] & M_CANCEL))
		stage2(index1);

	if (!(m_slot[index0] & M_CANCEL))
		stage1(index0);

	// write to the register file after the first pipeline stage has completed
	// to ensure the effective 4 cycle delay for non-bypassed register access
	if (!m_c_bus_cwen && !(m_slot[index3] & M_CANCEL))
	{
		m_f[m_c_bus_addr] = m_c_bus_data;
		LOGMASKED(LOG_REGS, "slot %d stage 4 f%d = %f\n", index3, m_c_bus_addr, u2f(m_c_bus_data.v));
	}

	// NEUT- and ABORT- prevent writeback of the previous instruction, but
	// allow i/o to complete normally
	if (!m_neut || !m_abort)
	{
		LOG("slot %d stage instruction %s\n", index1, !m_neut ? "neutralized" : "aborted");
		m_slot[index1] |= M_CANCEL;
	}

	m_head = index3;
}

void wtl3132_device::stage1(unsigned const index)
{
	u64 const code = m_slot[index];

	// A and B buses are loaded from registers by default
	float32_t a_bus = m_f[OPF(code, AADD)];
	float32_t b_bus = m_f[OPF(code, BADD)];

	// C-to-A bus internal bypass
	if (((m_mode & MODE_IBA) && !m_c_bus_cwen && OPF(code, AADD) == m_c_bus_addr))
	{
		a_bus = m_c_bus_data;
		LOGMASKED(LOG_BYPASS, "slot %d stage 1 C-to-A internal bypass %f\n", index, u2f(m_c_bus_data.v));
	}

	// C-to-B bus internal bypass
	if (((m_mode & MODE_IBB) && !m_c_bus_cwen && OPF(code, BADD) == m_c_bus_addr))
	{
		b_bus = m_c_bus_data;
		LOGMASKED(LOG_BYPASS, "slot %d stage 1 C-to-B internal bypass %f\n", index, u2f(m_c_bus_data.v));
	}

	// perform i/o
	switch (OPF(code, IOCT))
	{
	case 1: // floadrc
		m_f[OPF(code, DADD)] = m_x_port;
		// FIXME: floadrc uses c bus, preventing result writeback except for flut
		m_x_in = m_x_port;
		LOGMASKED(LOG_IO, "slot %d stage 1 floadrc f%d = %f\n", index, OPF(code, DADD), u2f(m_x_port.v));
		break;
	case 2: // fstore
		// C-to-X output bypass
		if ((m_mode & MODE_OBP) && !m_c_bus_cwen && OPF(code, DADD) == m_c_bus_addr)
		{
			m_x_out = m_c_bus_data;
			LOGMASKED(LOG_BYPASS, "slot %d stage 1 C-to-X output bypass %f\n", index, u2f(m_c_bus_data.v));
		}
		else
			m_x_out = m_f[OPF(code, DADD)];
		break;
	case 3: // fload
		m_f[OPF(code, DADD)] = m_x_port;
		LOGMASKED(LOG_IO, "slot %d stage 1 fload f%d = %f\n", index, OPF(code, DADD), u2f(m_x_port.v));

		// X-to-A bus input bypass
		if ((m_mode & MODE_IBP) && OPF(code, AADD) == OPF(code, DADD))
		{
			a_bus = m_x_port;
			LOGMASKED(LOG_BYPASS, "slot %d stage 1 X-to-A input bypass %f\n", index, u2f(m_x_port.v));
		}
		break;
	}

	// perform status and mode register operations
	switch (OPF(code, F))
	{
	case F_MISC:
		switch (OPF(code, BADD))
		{
		case MF_FCLSR:
			m_sr = false;
			break;
		case MF_FSTSR:
			m_x_port.v = m_sr;
			break;

		case MF_FMODE:
			{
				// check polarity inversion
				bool const polarity = bool(m_mode & MODE_FXH) ^ bool((OPF(code, CADD) << 5) & MODE_FXH);

				// update mode
				m_mode = (OPF(code, ABIN) << 10) | (OPF(code, CADD) << 5) | OPF(code, AADD);
				if (!(m_mode & MODE_FXO))
					m_sr = false;

				// force line update
				if (polarity)
					m_fpex_state = !m_sr;
			}
			break;
		}
	}

	// select and load multiplier inputs
	m_ma_in = a_bus;
	m_mb_in = OPF(code, MBIN) ? m_x_in : b_bus;

	// buffer accumulator inputs
	m_aa_in[0] = a_bus;
	m_ab_in[0] = b_bus;
}

void wtl3132_device::stage2(unsigned const index)
{
	u64 const code = m_slot[index];

	// load AAin from multiplier output or buffered A bus
	m_aa_in[1] = (OPF(code, F) & 4) ? m_m_out : m_aa_in[0];

	// load ABin from source specified by ABIN opcode field
	switch (OPF(code, ABIN))
	{
	case 0: m_ab_in[1] = m_x_in; break;
	case 1: m_ab_in[1] = m_ab_in[0]; break;
	case 2: m_ab_in[1] = m_t2; break;
	case 3: m_ab_in[1] = m_t1; break;
	case 4: m_ab_in[1] = m_t3; break;

	case 6: m_ab_in[1] = i32_to_f32(2); break;
	case 7: m_ab_in[1] = i32_to_f32(0); break;
	}

	// execute multiply and optional negate
	switch (OPF(code, F))
	{
	case F_FMNA:
	case F_FMNS:
		// multiply and negate
		m_m_out = f32_mul(f32_mul(m_ma_in, m_mb_in), i32_to_f32(-1));
		LOG("slot %d stage 2 -(%f * %f) == %f\n", index, u2f(m_ma_in.v), u2f(m_mb_in.v), u2f(m_m_out.v));
		break;
	case F_FMAC:
		// multiply
		m_m_out = f32_mul(m_ma_in, m_mb_in);
		LOG("slot %d stage 2 %f * %f == %f\n", index, u2f(m_ma_in.v), u2f(m_mb_in.v), u2f(m_m_out.v));
		break;
	}

	// complete fstore
	if (OPF(code, IOCT) == 2)
	{
		m_x_port = m_x_out;
		LOGMASKED(LOG_IO, "slot %d stage 2 fstore %f\n", index, u2f(m_x_out.v));
	}
}

void wtl3132_device::stage3(unsigned const index)
{
	u64 const code = m_slot[index];

	softfloat_exceptionFlags = 0;

	// execute accumulator operation
	switch (OPF(code, F))
	{
	case F_MISC:
		switch (OPF(code, BADD))
		{
		case MF_FABS:
			if (f32_lt(m_aa_in[1], i32_to_f32(0)))
				m_a_out = f32_mul(m_aa_in[1], i32_to_f32(-1));
			else
				m_a_out = m_aa_in[1];

			if (m_mode & MODE_RTN)
				m_cr = f32_eq(m_a_out, i32_to_f32(0));
			LOG("slot %d stage 3 fabs %f == %f\n", index, u2f(m_aa_in[1].v), u2f(m_a_out.v));
			break;
		case MF_FLOAT:
			if ((m_mode & MODE_RTN) && OPF(code, ENCN) == 1)
				m_cr = (m_aa_in[1].v & 0xff000000) && (~m_aa_in[1].v & 0xff000000);

			m_a_out = i32_to_f32(m_aa_in[1].v);
			LOG("slot %d stage 3 float 0x%08x == %f\n", index, m_aa_in[1].v, u2f(m_a_out.v));
			break;
		case MF_FIX:
			if ((m_mode & MODE_RTN) && OPF(code, ENCN) == 1)
				m_cr = f32_lt(m_aa_in[1], i32_to_f32(-4194304)) || f32_lt(i32_to_f32(4194304), m_aa_in[1]);

			m_a_out.v = (f32_to_i32(m_aa_in[1], (m_mode & MODE_RTN) ?
				softfloat_round_near_even : softfloat_round_min, false) << 8) >> 8;
			LOG("slot %d stage 3 fix %f == 0x%08x\n", index, u2f(m_aa_in[1].v), m_a_out.v);
			break;
		case MF_FLUT:
			// TODO: replace with lookup table
			m_a_out = f32_div(i32_to_f32(1), m_aa_in[1]);
			LOG("slot %d stage 3 flut %f == %f\n", index, u2f(m_aa_in[1].v), u2f(m_a_out.v));
			break;
		}
		break;
	case F_FSUBR:
		m_a_out = f32_sub(m_ab_in[1], m_aa_in[1]);
		LOG("slot %d stage 3 fsubr %f - %f == %f\n", index, u2f(m_ab_in[1].v), u2f(m_aa_in[1].v), u2f(m_a_out.v));
		break;
	case F_FSUB:
		m_a_out = f32_sub(m_aa_in[1], m_ab_in[1]);
		LOG("slot %d stage 3 fsub %f - %f == %f\n", index, u2f(m_aa_in[1].v), u2f(m_ab_in[1].v), u2f(m_a_out.v));
		break;
	case F_FADD:
		m_a_out = f32_add(m_aa_in[1], m_ab_in[1]);
		LOG("slot %d stage 3 fadd %f + %f == %f\n", index, u2f(m_aa_in[1].v), u2f(m_ab_in[1].v), u2f(m_a_out.v));
		break;

	case F_FMNA:
		m_a_out = f32_add(m_m_out, m_ab_in[1]);
		LOG("slot %d stage 3 fmna %f + %f == %f\n", index, u2f(m_m_out.v), u2f(m_ab_in[1].v), u2f(m_a_out.v));
		break;
	case F_FMNS:
		m_a_out = f32_sub(m_m_out, m_ab_in[1]);
		LOG("slot %d stage 3 fmns %f - %f == %f\n", index, u2f(m_m_out.v), u2f(m_ab_in[1].v), u2f(m_a_out.v));
		break;
	case F_FMAC:
		m_a_out = f32_add(m_m_out, m_ab_in[1]);
		LOG("slot %d stage 3 fmac %f + %f == %f\n", index, u2f(m_m_out.v), u2f(m_ab_in[1].v), u2f(m_a_out.v));
		break;
	}

	// update condition, zero and status registers
	if (OPF(code, F))
	{
		switch (OPF(code, ENCN))
		{
		case 1:
			m_cr = f32_le(m_a_out, i32_to_f32(0));
			m_zr = f32_eq(m_a_out, i32_to_f32(0));
			break;
		case 2:
			m_cr = f32_lt(m_a_out, i32_to_f32(0));
			m_zr = f32_eq(m_a_out, i32_to_f32(0));
			break;
		case 3:
			m_cr = f32_eq(m_a_out, i32_to_f32(0));
			m_zr = f32_eq(m_a_out, i32_to_f32(0));
			break;
		}

		if ((m_mode & MODE_FXH) || !m_sr)
			m_sr = softfloat_exceptionFlags & softfloat_flag_overflow;
	}
}

void wtl3132_device::stage4(unsigned const index)
{
	u64 const code = m_slot[index];

	// update C bus
	m_c_bus_data = m_a_out;
	m_c_bus_addr = OPF(code, CADD);
	m_c_bus_cwen = OPF(code, CWEN);

	// update temporary registers
	switch (OPF(code, ADST))
	{
	case 0: m_t3 = m_a_out; LOGMASKED(LOG_REGS, "slot %d stage 4 t3 = %f\n", index, u2f(m_t3.v)); break;
	case 1: m_t2 = m_a_out; LOGMASKED(LOG_REGS, "slot %d stage 4 t2 = %f\n", index, u2f(m_t2.v)); break;
	case 2: m_t1 = m_a_out; LOGMASKED(LOG_REGS, "slot %d stage 4 t1 = %f\n", index, u2f(m_t1.v)); break;
	}

	// update output lines
	if (m_cr ^ m_fpcn_state)
	{
		m_fpcn_state = m_cr;
		m_fpcn_cb(m_fpcn_state);
	}

	if (m_sr ^ m_fpex_state)
	{
		m_fpex_state = m_sr;
		m_fpex_cb(!(m_fpex_state ^ bool(m_mode & MODE_FXH)));
	}

	if (m_zr ^ m_zero_state)
	{
		m_zero_state = m_zr;
		m_zero_cb(m_zero_state);
	}
}

std::string wtl3132_device::disassemble(u64 const code)
{
	std::string alu = "";

	switch (OPF(code, F))
	{
	case F_MISC:
		switch (OPF(code, BADD))
		{
		case MF_FCLSR: alu = std::string("fclsr"); break;
		case MF_FSTSR: alu = std::string("fstsr"); break;

		case MF_FMODE: alu = util::string_format("fmode   0x%04x", (OPF(code, ABIN) << 10) | (OPF(code, CADD) << 5) | OPF(code, AADD)); break;
		case MF_FABS:  alu = util::string_format("fabs    %4s, %-12s", reg(OPF(code, AADD)), adst(code)); break;
		case MF_FLOAT: alu = util::string_format("float   %4s, %-12s", reg(OPF(code, AADD)), adst(code)); break;
		case MF_FIX:   alu = util::string_format("fix     %4s, %-12s", reg(OPF(code, AADD)), adst(code)); break;
		case MF_FLUT:  alu = util::string_format("flut    %4s, %-4s", reg(OPF(code, AADD)), reg(OPF(code, CADD))); break;

		}
		break;
	case F_FSUBR: alu = util::string_format("fsubr   %4s, %4s, %-12s", reg(OPF(code, AADD)), abin(code), adst(code)); break;
	case F_FSUB:
		if (!OPF(code, CWEN) || OPF(code, ADST) != 3 || OPF(code, ENCN))
			alu = util::string_format("fsub    %4s, %4s, %-12s", reg(OPF(code, AADD)), abin(code), adst(code));
		else
			alu = std::string("fnop");
		break;
	case F_FADD:
		if (!OPF(code, CWEN) || OPF(code, ADST) != 3 || OPF(code, ENCN) || OPF(code, ABIN) != 7)
			alu = util::string_format("fadd    %4s, %4s, %-12s", reg(OPF(code, AADD)), abin(code), adst(code));
		else
			alu = std::string("fnop");
		break;

	case F_FMNA: alu = util::string_format("fmna    %4s, %4s, %4s, %-12s", reg(OPF(code, AADD)), mbin(code), abin(code), adst(code)); break;
	case F_FMNS: alu = util::string_format("fmns    %4s, %4s, %4s, %-12s", reg(OPF(code, AADD)), mbin(code), abin(code), adst(code)); break;
	case F_FMAC: alu = util::string_format("fmac    %4s, %4s, %4s, %-12s", reg(OPF(code, AADD)), mbin(code), abin(code), adst(code)); break;
	}

	switch (OPF(code, IOCT))
	{
	case 1: return util::string_format("%-38s ; floadrc %4s", alu, reg(OPF(code, DADD))); break;
	case 2: return util::string_format("%-38s ; fstore  %4s", alu, reg(OPF(code, DADD))); break;
	case 3: return util::string_format("%-38s ; fload   %4s", alu, reg(OPF(code, DADD))); break;
	default:
		return alu;
	}
}

std::string wtl3132_device::reg(unsigned const reg)
{
	return util::string_format(".f%d", reg);
}

std::string wtl3132_device::mbin(u64 const code)
{
	switch (OPF(code, MBIN))
	{
	case 0: return util::string_format(".f%d", OPF(code, BADD));
	case 1: return util::string_format("Cbus");
	}

	// can't happen
	return std::string();
}

std::string wtl3132_device::abin(u64 const code)
{
	switch (OPF(code, ABIN))
	{
	case 0: return util::string_format("Cbus");
	case 1: return util::string_format(".f%d", OPF(code, BADD));
	case 2: return util::string_format(".t2");
	case 3: return util::string_format(".t1");
	case 4: return util::string_format(".t3");

	case 6: return util::string_format("2");
	case 7: return util::string_format("0");
	}

	return std::string();
}

std::string wtl3132_device::adst(u64 const code)
{
	switch (OPF(code, ADST))
	{
	case 0: return OPF(code, CWEN) ? util::string_format(".t3") : util::string_format(".f%d and .t3", OPF(code, CADD));
	case 1: return OPF(code, CWEN) ? util::string_format(".t2") : util::string_format(".f%d and .t2", OPF(code, CADD));
	case 2: return OPF(code, CWEN) ? util::string_format(".t1") : util::string_format(".f%d and .t1", OPF(code, CADD));
	case 3: return OPF(code, CWEN) ? std::string() : util::string_format(".f%d", OPF(code, CADD));
	}

	// can't happen
	return std::string();
}
