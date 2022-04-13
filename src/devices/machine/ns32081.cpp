// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor 32081 Floating-Point Unit.
 *
 * Sources:
 *   - http://bitsavers.org/components/national/_dataBooks/1989_National_Microprocessor_Databook_32000_NSC800.pdf
 *
 * TODO:
 *   - testing
 */

#include "emu.h"
#include "ns32081.h"

#include "softfloat3/source/include/softfloat.h"

#define LOG_GENERAL (1U << 0)
//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32081, ns32081_device, "ns32081", "National Semiconductor 32081 Floating-Point Unit")

enum fsr_mask : u32
{
	FSR_TT  = 0x00000007, // trap type
	FSR_UEN = 0x00000008, // underflow trap enable
	FSR_UF  = 0x00000010, // underflow flag
	FSR_IEN = 0x00000020, // inexact result trap enable
	FSR_IF  = 0x00000040, // inexact result flag
	FSR_RM  = 0x00000180, // rounding mode
	FSR_SWF = 0x0000fe00, // software field
};

enum rm_mask : u32
{
	RM_N = 0x00000000, // round to nearest value
	RM_Z = 0x00000080, // round toward zero
	RM_U = 0x00000100, // round toward positive infinity
	RM_D = 0x00000180, // round toward negative infinity
};

enum tt_mask : u32
{
	TT_UND = 0x00000001, // underflow
	TT_OVF = 0x00000002, // overflow
	TT_DVZ = 0x00000003, // divide by zero
	TT_ILL = 0x00000004, // illegal instruction
	TT_INV = 0x00000005, // invalid operation
	TT_INX = 0x00000006, // inexact result
	TT_RSV = 0x00000007, // reserved
};

enum state : unsigned
{
	IDLE      = 0,
	OPERATION = 1, // awaiting operation word
	OPERAND   = 2, // awaiting operands
	STATUS    = 4, // status word available
	RESULT    = 5, // result word available
};

enum idbyte : u8
{
	FORMAT_9  = 0x3e,
	FORMAT_11 = 0xbe,
};

enum operand_length : unsigned
{
	LENGTH_F = 4, // single precision
	LENGTH_L = 8, // double precision
};

enum size_code : unsigned
{
	SIZE_B = 0,
	SIZE_W = 1,
	SIZE_D = 3,
};

ns32081_device::ns32081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NS32081, tag, owner, clock)
	, ns32000_slow_slave_interface(mconfig, *this)
{
}

void ns32081_device::device_start()
{
	save_item(NAME(m_fsr));
	save_item(NAME(m_f));

	save_item(NAME(m_idbyte));
	save_item(NAME(m_opword));
	save_item(STRUCT_MEMBER(m_op, expected));
	save_item(STRUCT_MEMBER(m_op, issued));
	save_item(STRUCT_MEMBER(m_op, value));
	save_item(NAME(m_status));

	save_item(NAME(m_state));
	save_item(NAME(m_tcy));

	m_complete = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ns32081_device::complete), this));
}

void ns32081_device::device_reset()
{
	m_fsr = 0;
	std::fill(std::begin(m_f), std::end(m_f), 0);

	m_state = IDLE;
}

void ns32081_device::state_add(device_state_interface &parent, int &index)
{
	parent.state_add(index++, "FSR", m_fsr).formatstr("%04X");

	for (unsigned i = 0; i < 8; i++)
		parent.state_add(index++, util::string_format("F%d", i).c_str(), m_f[i]).formatstr("%08X");
}

u16 ns32081_device::read_st(int *icount)
{
	if (m_state == STATUS)
	{
		m_state = (m_op[2].issued == m_op[2].expected) ? IDLE : RESULT;

		if (icount)
			*icount -= m_tcy;

		LOG("read_st status 0x%04x tcy %d %s (%s)\n", m_status, m_tcy,
			(m_state == RESULT ? "results pending" : "complete"), machine().describe_context());

		return m_status;
	}

	logerror("protocol error reading status word (%s)\n", machine().describe_context());
	return 0;
}

u16 ns32081_device::read_op()
{
	if (m_state == RESULT && m_op[2].issued < m_op[2].expected)
	{
		u16 const data = u16(m_op[2].value >> (m_op[2].issued * 8));
		LOG("read_op word %d data 0x%04x (%s)\n", m_op[2].issued >> 1, data, machine().describe_context());

		m_op[2].issued += 2;

		if (m_op[2].issued == m_op[2].expected)
		{
			LOG("read_op last result word issued\n");
			m_state = IDLE;
		}

		return data;
	}

	logerror("protocol error reading result word (%s)\n", machine().describe_context());
	return 0;
}

void ns32081_device::write_id(u16 data)
{
	bool const match = (data == FORMAT_9) || (data == FORMAT_11);

	if (match)
	{
		LOG("write_id match 0x%04x (%s)\n", data, machine().describe_context());
		m_state = OPERATION;
	}
	else
	{
		LOG("write_id ignore 0x%04x (%s)\n", data, machine().describe_context());
		m_state = IDLE;
	}

	m_idbyte = u8(data);
}

void ns32081_device::write_op(u16 data)
{
	switch (m_state)
	{
	case OPERATION:
		m_opword = swapendian_int16(data);
		LOG("write_op opword 0x%04x (%s)\n", m_opword, machine().describe_context());

		// initialize operands
		for (operand &op : m_op)
		{
			op.expected = 0;
			op.issued = 0;
			op.value = 0;
		}

		// decode operands
		if (m_idbyte == FORMAT_9)
		{
			// format 9: 1111 1222 22oo ofii
			unsigned const f_length = BIT(m_opword, 2) ? LENGTH_F : LENGTH_L;
			unsigned const size = m_opword & 3;

			switch ((m_opword >> 3) & 7)
			{
			case 0: // movif
				m_op[0].expected = size + 1;
				m_op[2].expected = f_length;
				break;
			case 1: // lfsr
				m_op[0].expected = 4;
				break;
			case 2: // movlf
				m_op[0].expected = LENGTH_L;
				m_op[2].expected = f_length;
				break;
			case 3: // movfl
				m_op[0].expected = LENGTH_F;
				m_op[2].expected = f_length;
				break;
			case 4: // roundfi
			case 5: // truncfi
			case 7: // floorfi
				m_op[0].expected = f_length;
				m_op[2].expected = size + 1;
				break;
			case 6: // sfsr
				m_op[2].expected = 4;
				break;
			}
		}
		else if (m_idbyte == FORMAT_11)
		{
			// format 11: 1111 1222 22oo oo0f
			unsigned const opcode = (m_opword >> 2) & 15;
			unsigned const f_length = BIT(m_opword, 0) ? LENGTH_F : LENGTH_L;

			m_op[0].expected = f_length;

			// even opcodes have two input operands
			if (!BIT(opcode, 0))
				m_op[1].expected = f_length;

			// all operations except CMPf issue a result
			if (opcode != 2)
				m_op[2].expected = f_length;
		}

		// operand 1 in register
		if (m_op[0].expected && !(m_opword & 0xc000))
		{
			// exclude integer operands
			if (m_idbyte == FORMAT_11 || ((m_opword >> 3) & 7) > 1)
			{
				unsigned const reg = (m_opword >> 11) & 7;
				LOG("write_op read f%d\n", reg);

				m_op[0].value = m_f[reg ^ 0];
				if (m_op[0].expected == 8)
					m_op[0].value |= u64(m_f[reg ^ 1]) << 32;

				m_op[0].issued = m_op[0].expected;
			}
		}

		// operand 2 in register
		if (m_op[1].expected && !(m_opword & 0x0600))
		{
			unsigned const reg = (m_opword >> 6) & 7;
			LOG("write_op read f%d\n", reg);

			m_op[1].value = m_f[reg ^ 0];
			if (m_op[1].expected == 8)
				m_op[1].value |= u64(m_f[reg ^ 1]) << 32;

			m_op[1].issued = m_op[1].expected;
		}

		m_state = OPERAND;
		break;

	case OPERAND:
		// check awaiting operand word
		if (m_op[0].issued < m_op[0].expected || m_op[1].issued < m_op[1].expected)
		{
			unsigned const n = (m_op[0].issued < m_op[0].expected) ? 0 : 1;
			operand &op = m_op[n];

			LOG("write_op op%d data 0x%04x (%s)\n", n, data, machine().describe_context());

			// insert word into operand value
			op.value |= u64(data) << (op.issued * 8);
			op.issued += 2;
		}
		else
			logerror("protocol error unexpected operand word 0x%04x (%s)\n", data, machine().describe_context());
		break;
	}

	// start execution when all operands are available
	if (m_state == OPERAND && m_op[0].issued >= m_op[0].expected && m_op[1].issued >= m_op[1].expected)
		execute();
}

void ns32081_device::execute()
{
	softfloat_exceptionFlags = 0;
	m_fsr &= ~FSR_TT;

	m_status = 0;
	m_tcy = 0;

	switch (m_idbyte)
	{
	case FORMAT_9:
		// format 9: 1111 1222 22oo ofii
		{
			bool const single = BIT(m_opword, 2);
			unsigned const f_length = single ? LENGTH_F : LENGTH_L;
			unsigned const size = m_opword & 3;

			switch ((m_opword >> 3) & 7)
			{
			case 0:
				// MOVif src,dest
				//       gen,gen
				//       read.i,write.f
				{
					s32 const src =
						(size == SIZE_D) ? s32(m_op[0].value) :
						(size == SIZE_W) ? s16(m_op[0].value) :
						s8(m_op[0].value);

					if (single)
						m_op[2].value = i32_to_f32(src).v;
					else
						m_op[2].value = i32_to_f64(src).v;
					m_op[2].expected = f_length;
					m_tcy = 53;
				}
				break;
			case 1:
				// LFSR src
				//      gen
				//      read.D
				m_fsr = u16(m_op[0].value);

				switch (m_fsr & FSR_RM)
				{
				case RM_N: softfloat_roundingMode = softfloat_round_near_even; break;
				case RM_Z: softfloat_roundingMode = softfloat_round_minMag; break;
				case RM_U: softfloat_roundingMode = softfloat_round_max; break;
				case RM_D: softfloat_roundingMode = softfloat_round_min; break;
				}
				m_tcy = 18;
				break;
			case 2:
				// MOVLF src,dest
				//       gen,gen
				//       read.L,write.F
				m_op[2].value = f64_to_f32(float64_t{ m_op[0].value }).v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc000) ? 23 : 27;
				break;
			case 3:
				// MOVFL src,dest
				//       gen,gen
				//       read.F,write.L
				m_op[2].value = f32_to_f64(float32_t{ u32(m_op[0].value) }).v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc000) ? 22 : 26;
				break;
			case 4:
				// ROUNDfi src,dest
				//         gen,gen
				//         read.f,write.i
				if (single)
					m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_near_even, true);
				else
					m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_near_even, true);

				if ((size == SIZE_D && s64(m_op[2].value) != s32(m_op[2].value))
				|| (size == SIZE_W && s64(m_op[2].value) != s16(m_op[2].value))
				|| (size == SIZE_B && s64(m_op[2].value) != s8(m_op[2].value)))
					softfloat_exceptionFlags |= softfloat_flag_overflow;

				m_op[2].expected = size + 1;
				m_tcy = (m_opword & 0xc000) ? 53 : 66;
				break;
			case 5:
				// TRUNCfi src,dest
				//         gen,gen
				//         read.f,write.i
				if (single)
					m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_minMag, true);
				else
					m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_minMag, true);

				if ((size == SIZE_D && s64(m_op[2].value) != s32(m_op[2].value))
				|| (size == SIZE_W && s64(m_op[2].value) != s16(m_op[2].value))
				|| (size == SIZE_B && s64(m_op[2].value) != s8(m_op[2].value)))
					softfloat_exceptionFlags |= softfloat_flag_overflow;

				m_op[2].expected = size + 1;
				m_tcy = (m_opword & 0xc000) ? 53 : 66;
				break;
			case 6:
				// SFSR dest
				//      gen
				//      write.D
				m_op[2].value = m_fsr;
				m_op[2].expected = 4;
				m_tcy = 13;
				break;
			case 7:
				// FLOORfi src,dest
				//         gen,gen
				//         read.f,write.i
				if (single)
					m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_min, true);
				else
					m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_min, true);

				if ((size == SIZE_D && s64(m_op[2].value) != s32(m_op[2].value))
				|| (size == SIZE_W && s64(m_op[2].value) != s16(m_op[2].value))
				|| (size == SIZE_B && s64(m_op[2].value) != s8(m_op[2].value)))
					softfloat_exceptionFlags |= softfloat_flag_overflow;

				m_op[2].expected = size + 1;
				m_tcy = (m_opword & 0xc000) ? 53 : 66;
				break;
			}
		}
		break;

	case FORMAT_11:
		// format 11: 1111122222oooo0f
		{
			bool const single = BIT(m_opword, 0);
			unsigned const f_length = single ? LENGTH_F : LENGTH_L;

			switch ((m_opword >> 2) & 15)
			{
			case 0x0:
				// ADDf src,dest
				//      gen,gen
				//      read.f,rmw.f
				if (single)
					m_op[2].value = f32_add(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
				else
					m_op[2].value = f64_add(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc600) ? 70 : 74;
				break;
			case 0x1:
				// MOVf src,dest
				//      gen,gen
				//      read.f,write.f
				m_op[2].value = m_op[0].value;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc000) ? 23 : 27;
				break;
			case 0x2:
				// CMPf src1,src2
				//      gen,gen
				//      read.f,read.f
				if (m_op[0].value == m_op[1].value)
					m_status |= SLAVE_Z;
				if ((single && f32_le(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }))
				|| (!single && f64_le(float64_t{ m_op[1].value }, float64_t{ m_op[0].value })))
					m_status |= SLAVE_N;
				m_tcy = (m_opword & 0xc600) ? 45 : 49;
				break;
			case 0x3:
				// Trap(SLAVE)
				m_fsr |= TT_ILL;
				m_status = SLAVE_Q;
				break;
			case 0x4:
				// SUBf src,dest
				//      gen,gen
				//      read.f,rmw.f
				if (single)
					m_op[2].value = f32_sub(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
				else
					m_op[2].value = f64_sub(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc600) ? 70 : 74;
				break;
			case 0x5:
				// NEGf src,dest
				//      gen,gen
				//      read.f,write.f
				if (single)
					m_op[2].value = f32_mul(float32_t{ u32(m_op[0].value) }, i32_to_f32(-1)).v;
				else
					m_op[2].value = f64_mul(float64_t{ m_op[0].value }, i32_to_f64(-1)).v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc000) ? 20 : 24;
				break;
			case 0x8:
				// DIVf src,dest
				//      gen,gen
				//      read.f,rmw.f
				if (single)
					m_op[2].value = f32_div(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
				else
					m_op[2].value = f64_div(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;
				m_op[2].expected = f_length;
				m_tcy = ((m_opword & 0xc600) ? 55 : 59) + (single ? 30 : 60);
				break;
			case 0x9:
				// Trap(SLAVE)
				m_fsr |= TT_ILL;
				m_status = SLAVE_Q;
				break;
			case 0xc:
				// MULf src,dest
				//      gen,gen
				//      read.f,rmw.f
				if (single)
					m_op[2].value = f32_mul(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
				else
					m_op[2].value = f64_mul(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;
				m_op[2].expected = f_length;
				m_tcy = ((m_opword & 0xc600) ? 30 : 34) + (single ? 14 : 28);
				break;
			case 0xd:
				// ABSf src,dest
				//      gen,gen
				//      read.f,write.f
				if (single)
					if (f32_lt(float32_t{ u32(m_op[0].value) }, float32_t{ 0 }))
						m_op[2].value = f32_mul(float32_t{ u32(m_op[0].value) }, i32_to_f32(-1)).v;
					else
						m_op[2].value = float32_t{ u32(m_op[0].value) }.v;
				else
					if (f64_lt(float64_t{ m_op[0].value }, float64_t{ 0 }))
						m_op[2].value = f64_mul(float64_t{ m_op[0].value }, i32_to_f64(-1)).v;
					else
						m_op[2].value = float64_t{ m_op[0].value }.v;
				m_op[2].expected = f_length;
				m_tcy = (m_opword & 0xc000) ? 20 : 24;
				break;
			}
		}
		break;
	}

	// check for exceptions
	if (softfloat_exceptionFlags & softfloat_flag_underflow)
	{
		m_fsr |= FSR_UF | TT_UND;

		if (m_fsr & FSR_UEN)
			m_status |= SLAVE_Q;
		else
			m_op[2].value = 0;
	}
	else if (softfloat_exceptionFlags & softfloat_flag_overflow)
	{
		m_fsr |= TT_OVF;
		m_status |= SLAVE_Q;
	}
	else if (softfloat_exceptionFlags & softfloat_flag_infinite)
	{
		m_fsr |= TT_DVZ;
		m_status |= SLAVE_Q;
	}
	else if (softfloat_exceptionFlags & softfloat_flag_invalid)
		m_fsr |= TT_INV;
	else if (softfloat_exceptionFlags & softfloat_flag_inexact)
	{
		m_fsr |= FSR_IF | TT_INX;

		if (m_fsr & FSR_IEN)
			m_status |= SLAVE_Q;
	}

	// exceptions suppress result issue
	if (m_status & SLAVE_Q)
		m_op[2].expected = 0;

	if (VERBOSE & LOG_GENERAL)
	{
		static char const *format9[] = { "movif", "lfsr", "movlf", "movfl", "roundfi", "truncfi", "sfsr", "floorfi" };
		static char const *format11[] =
		{
			"addf", "movf", "cmpf", nullptr, "subf", "negf", nullptr, nullptr,
			"divf", nullptr, nullptr, nullptr, "mulf", "absf", nullptr, nullptr
		};

		if (m_status & SLAVE_Q)
			LOG("execute %s 0x%x,0x%x exception\n",
				(m_idbyte == FORMAT_9)
					? format9[(m_opword >> 3) & 7]
					: format11[(m_opword >> 2) & 15],
				m_op[0].value, m_op[1].value);
		else
			LOG("execute %s 0x%x,0x%x result 0x%x\n",
				(m_idbyte == FORMAT_9)
					? format9[(m_opword >> 3) & 7]
					: format11[(m_opword >> 2) & 15],
				m_op[0].value, m_op[1].value, m_op[2].value);
	}

	// write-back floating point register results
	if (m_op[2].expected && !(m_opword & 0x0600))
	{
		// exclude integer results (roundfi, truncfi, sfsr, floorfi)
		if (m_idbyte == FORMAT_11 || ((m_opword >> 3) & 7) < 4)
		{
			unsigned const reg = (m_opword >> 6) & 7;

			LOG("execute write-back f%d\n", reg);

			m_f[reg ^ 0] = u32(m_op[2].value >> 0);
			if (m_op[2].expected == 8)
				m_f[reg ^ 1] = u32(m_op[2].value >> 32);

			m_op[2].issued = m_op[2].expected;
		}
	}

	m_state = STATUS;

	if (m_out_scb)
		m_complete->adjust(attotime::from_ticks(m_tcy, clock()));
}

void ns32081_device::complete(s32 param)
{
	m_out_scb(0);
	m_out_scb(1);
}
