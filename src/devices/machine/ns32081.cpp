// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor NS32081 Floating-Point Unit.
 *
 * Sources:
 *  - Microprocessor Databook, Series 32000, NSC800, 1989 Edition, National Semiconductor
 *
 * TODO:
 *  - poly/scalb/logb/dot
 *  - ns32381 timing
 *  - no-result operations
 */

#include "emu.h"
#include "ns32081.h"

#include "softfloat3/source/include/softfloat.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32081, ns32081_device, "ns32081", "National Semiconductor NS32081 Floating-Point Unit")
DEFINE_DEVICE_TYPE(NS32381, ns32381_device, "ns32381", "National Semiconductor NS32381 Floating-Point Unit")

enum fsr_mask : u32
{
	FSR_TT  = 0x00000007, // trap type
	FSR_UEN = 0x00000008, // underflow trap enable
	FSR_UF  = 0x00000010, // underflow flag
	FSR_IEN = 0x00000020, // inexact result trap enable
	FSR_IF  = 0x00000040, // inexact result flag
	FSR_RM  = 0x00000180, // rounding mode
	FSR_SWF = 0x0000fe00, // software field
	FSR_RMB = 0x00010000, // (32381 only) register modify bit
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

enum operand_length : unsigned
{
	LENGTH_F = 4, // single precision
	LENGTH_L = 8, // double precision
};

ns32081_device_base::ns32081_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, ns32000_fpu_interface(mconfig, *this)
{
}

void ns32081_device_base::device_start()
{
	save_item(NAME(m_fsr));

	save_item(NAME(m_state));
	save_item(NAME(m_idbyte));
	save_item(NAME(m_opword));

	save_item(STRUCT_MEMBER(m_op, expected));
	save_item(STRUCT_MEMBER(m_op, issued));
	save_item(STRUCT_MEMBER(m_op, value));

	save_item(NAME(m_status));
	save_item(NAME(m_tcy));

	m_complete = timer_alloc(FUNC(ns32081_device::complete), this);
}

void ns32081_device_base::device_reset()
{
	m_fsr = 0;
	m_state = IDLE;
	m_idbyte = 0;
	m_opword = 0;
	m_status = 0;
	m_tcy = 0;
}

void ns32081_device_base::state_add(device_state_interface &parent, int &index)
{
	parent.state_add(index++, "FSR", m_fsr).formatstr("%04X");
}

template <typename T> T ns32081_device_base::read()
{
	if (m_state == RESULT && m_op[2].issued < m_op[2].expected)
	{
		T const data = m_op[2].value >> (m_op[2].issued * 8);

		m_op[2].issued += sizeof(T);

		LOG("read %d data 0x%0*x (%s)\n",
			m_op[2].issued / sizeof(T), sizeof(T) * 2, data, machine().describe_context());

		if (m_op[2].issued == m_op[2].expected)
		{
			LOG("read complete\n");
			m_state = IDLE;
		}

		return data;
	}

	logerror("read protocol error (%s)\n", machine().describe_context());
	return 0;
}

template <typename T> void ns32081_device_base::write(T data)
{
	switch (m_state)
	{
	case IDLE:
		if (sizeof(T) == 4)
		{
			// decode instruction
			if (!decode(BIT(data, 24, 8), swapendian_int16(BIT(data, 8, 16))))
				return;

			m_state = OPERAND;
		}
		else
		{
			LOG("write idbyte 0x%04x (%s)\n", data, machine().describe_context());
			if ((data == FORMAT_9) || (data == FORMAT_11) || (type() == NS32381 && data == FORMAT_12))
			{
				// record idbyte
				m_idbyte = data;

				m_state = OPERATION;
			}
		}
		break;

	case OPERATION:
		LOG("write opword 0x%0*x (%s)\n", sizeof(T) * 2, data, machine().describe_context());

		// decode instruction
		decode(m_idbyte, swapendian_int16(data));

		m_state = OPERAND;
		break;

	case OPERAND:
		// check awaiting operand data
		if (m_op[0].issued < m_op[0].expected || m_op[1].issued < m_op[1].expected)
		{
			unsigned const n = (m_op[0].issued < m_op[0].expected) ? 0 : 1;
			operand &op = m_op[n];

			LOG("write operand %d data 0x%0*x (%s)\n",
				n, sizeof(T) * 2, data, machine().describe_context());

			// insert data into operand value
			op.value |= u64(data) << (op.issued * 8);
			op.issued += sizeof(T);
		}
		else
			logerror("write protocol error unexpected operand data 0x%0*x (%s)\n",
				sizeof(T) * 2, data, machine().describe_context());
		break;
	}

	// start execution when all operands are available
	if (m_state == OPERAND && m_op[0].issued >= m_op[0].expected && m_op[1].issued >= m_op[1].expected)
		execute();
}

bool ns32081_device_base::decode(u8 const idbyte, u16 const opword)
{
	LOG("decode idbyte 0x%02x opword 0x%04x (%s)\n", idbyte, opword, machine().describe_context());

	m_idbyte = idbyte;
	m_opword = opword;

	// initialize operands
	for (operand &op : m_op)
	{
		op.expected = 0;
		op.issued = 0;
		op.value = 0;
	}

	switch (m_idbyte)
	{
	case FORMAT_9:
		{
			// format 9: 1111 1222 22oo ofii
			unsigned const f_length = BIT(m_opword, 2) ? LENGTH_F : LENGTH_L;
			unsigned const size = m_opword & 3;

			switch (BIT(m_opword, 3, 3))
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
		break;
	case FORMAT_11:
		{
			// format 11: 1111 1222 22oo oo0f
			unsigned const opcode = BIT(m_opword, 2, 4);
			unsigned const f_length = BIT(m_opword, 0) ? LENGTH_F : LENGTH_L;

			m_op[0].expected = f_length;

			// even opcodes have two input operands
			if (!BIT(opcode, 0))
				m_op[1].expected = f_length;

			// all operations except CMPf issue a result
			if (opcode != 2)
				m_op[2].expected = f_length;
		}
		break;
	case FORMAT_12:
		{
			// format 12: 1111 1222 22oo oo0f
			unsigned const f_length = BIT(m_opword, 0) ? LENGTH_F : LENGTH_L;

			switch (BIT(m_opword, 2, 4))
			{
			case 2: // polyf
			case 3: // dotf
				m_op[0].expected = f_length;
				m_op[1].expected = f_length;
				break;
			case 4: // scalbf
				m_op[0].expected = f_length;
				m_op[1].expected = f_length;
				m_op[2].expected = f_length;
				break;
			case 5: // logbf
				m_op[0].expected = f_length;
				m_op[2].expected = f_length;
				break;
			}
		}
		break;
	default:
		LOG("decode idbyte 0x%02x unknown (%s)\n", m_idbyte, machine().describe_context());
		return false;
	}

	// operand 1 in register
	if (m_op[0].expected && !BIT(m_opword, 14, 2))
	{
		// exclude integer operands
		if (m_idbyte != FORMAT_9 || (BIT(m_opword, 3, 3) > 1))
		{
			reg_get(m_op[0].expected, m_op[0].value, BIT(m_opword, 11, 3));

			m_op[0].issued = m_op[0].expected;
		}
	}

	// operand 2 in register
	if (m_op[1].expected && !BIT(m_opword, 9, 2))
	{
		reg_get(m_op[1].expected, m_op[1].value, BIT(m_opword, 6, 3));

		m_op[1].issued = m_op[1].expected;
	}

	return true;
}

void ns32081_device_base::execute()
{
	u32 const fsr = m_fsr;
	m_fsr &= ~FSR_TT;

	softfloat_exceptionFlags = 0;
	m_status = 0;
	m_tcy = 0;

	switch (m_idbyte)
	{
	case FORMAT_9:
		// format 9: 1111 1222 22oo ofii
		switch (BIT(m_opword, 3, 3))
		{
		case 0:
			// MOVif src,dest
			//       gen,gen
			//       read.i,write.f
			{
				s32 const src = util::sext<s32>(m_op[0].value, m_op[0].expected * 8);

				if (m_op[2].expected == LENGTH_F)
					m_op[2].value = i32_to_f32(src).v;
				else
					m_op[2].value = i32_to_f64(src).v;

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

			m_tcy = BIT(m_opword, 14, 2) ? 23 : 27;
			break;
		case 3:
			// MOVFL src,dest
			//       gen,gen
			//       read.F,write.L
			m_op[2].value = f32_to_f64(float32_t{ u32(m_op[0].value) }).v;

			m_tcy = BIT(m_opword, 14, 2) ? 22 : 26;
			break;
		case 4:
			// ROUNDfi src,dest
			//         gen,gen
			//         read.f,write.i
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_near_even, true);
			else
				m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_near_even, true);

			if (s64(m_op[2].value) != util::sext<s64>(m_op[2].value, m_op[2].expected * 8))
				softfloat_exceptionFlags |= softfloat_flag_overflow;

			m_tcy = BIT(m_opword, 14, 2) ? 53 : 66;
			break;
		case 5:
			// TRUNCfi src,dest
			//         gen,gen
			//         read.f,write.i
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_minMag, true);
			else
				m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_minMag, true);

			if (s64(m_op[2].value) != util::sext<s64>(m_op[2].value, m_op[2].expected * 8))
				softfloat_exceptionFlags |= softfloat_flag_overflow;

			m_tcy = BIT(m_opword, 14, 2) ? 53 : 66;
			break;
		case 6:
			// SFSR dest
			//      gen
			//      write.D
			m_op[2].value = fsr;

			m_tcy = 13;
			break;
		case 7:
			// FLOORfi src,dest
			//         gen,gen
			//         read.f,write.i
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_to_i64(float32_t{ u32(m_op[0].value) }, softfloat_round_min, true);
			else
				m_op[2].value = f64_to_i64(float64_t{ m_op[0].value }, softfloat_round_min, true);

			if (s64(m_op[2].value) != util::sext<s64>(m_op[2].value, m_op[2].expected * 8))
				softfloat_exceptionFlags |= softfloat_flag_overflow;

			m_tcy = BIT(m_opword, 14, 2) ? 53 : 66;
			break;
		}
		break;

	case FORMAT_11:
		// format 11: 1111 1222 22oo oo0f
		switch (BIT(m_opword, 2, 4))
		{
		case 0x0:
			// ADDf src,dest
			//      gen,gen
			//      read.f,rmw.f
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_add(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
			else
				m_op[2].value = f64_add(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;

			m_tcy = (m_opword & 0xc600) ? 70 : 74;
			break;
		case 0x1:
			// MOVf src,dest
			//      gen,gen
			//      read.f,write.f
			m_op[2].value = m_op[0].value;

			m_tcy = BIT(m_opword, 14, 2) ? 23 : 27;
			break;
		case 0x2:
			// CMPf src1,src2
			//      gen,gen
			//      read.f,read.f
			if (m_op[0].value == m_op[1].value)
				m_status |= SLAVE_Z;
			if ((m_op[0].expected == LENGTH_F && f32_le(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }))
			|| (m_op[0].expected == LENGTH_L && f64_le(float64_t{ m_op[1].value }, float64_t{ m_op[0].value })))
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
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_sub(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
			else
				m_op[2].value = f64_sub(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;

			m_tcy = (m_opword & 0xc600) ? 70 : 74;
			break;
		case 0x5:
			// NEGf src,dest
			//      gen,gen
			//      read.f,write.f
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_mul(float32_t{ u32(m_op[0].value) }, i32_to_f32(-1)).v;
			else
				m_op[2].value = f64_mul(float64_t{ m_op[0].value }, i32_to_f64(-1)).v;

			m_tcy = BIT(m_opword, 14, 2) ? 20 : 24;
			break;
		case 0x8:
			// DIVf src,dest
			//      gen,gen
			//      read.f,rmw.f
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_div(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
			else
				m_op[2].value = f64_div(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;

			m_tcy = ((m_opword & 0xc600) ? 55 : 59) + (m_op[0].expected == LENGTH_F ? 30 : 60);
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
			if (m_op[0].expected == LENGTH_F)
				m_op[2].value = f32_mul(float32_t{ u32(m_op[1].value) }, float32_t{ u32(m_op[0].value) }).v;
			else
				m_op[2].value = f64_mul(float64_t{ m_op[1].value }, float64_t{ m_op[0].value }).v;

			m_tcy = ((m_opword & 0xc600) ? 30 : 34) + (m_op[0].expected == LENGTH_F ? 14 : 28);
			break;
		case 0xd:
			// ABSf src,dest
			//      gen,gen
			//      read.f,write.f
			if (m_op[0].expected == LENGTH_F)
				if (f32_lt(float32_t{ u32(m_op[0].value) }, float32_t{ 0 }))
					m_op[2].value = f32_mul(float32_t{ u32(m_op[0].value) }, i32_to_f32(-1)).v;
				else
					m_op[2].value = float32_t{ u32(m_op[0].value) }.v;
			else
				if (f64_lt(float64_t{ m_op[0].value }, float64_t{ 0 }))
					m_op[2].value = f64_mul(float64_t{ m_op[0].value }, i32_to_f64(-1)).v;
				else
					m_op[2].value = float64_t{ m_op[0].value }.v;

			m_tcy = BIT(m_opword, 14, 2) ? 20 : 24;
			break;
		}
		break;

	case FORMAT_12:
		// format 12: 1111 1222 22oo oo0f
		switch (BIT(m_opword, 2, 4))
		{
		case 0x2:
			// POLYf src1,src2
			//       gen,gen
			//       read.f,read.f
			m_fsr |= FSR_RMB;
			break;
		case 0x3:
			// DOTf src1,src2
			//      gen,gen
			//      read.f,read.f
			m_fsr |= FSR_RMB;
			break;
		case 0x4:
			// SCALBf src,dest
			//        gen,gen
			//        read.f,rmw.f
			break;
		case 0x5:
			// LOGBf src,dest
			//       gen,gen
			//       read.f,write.f
			break;
		default:
			// Trap(SLAVE)
			m_fsr |= TT_ILL;
			m_status = SLAVE_Q;
			break;
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
	{
		m_fsr |= TT_INV;
		m_status |= SLAVE_Q;
	}
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
		static char const *format12[] =
		{
			nullptr, nullptr, "polyf", "dotf", "scalbf", "logbf", nullptr, nullptr,
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
		};

		char const *operation = nullptr;
		switch (m_idbyte)
		{
		case FORMAT_9: operation = format9[BIT(m_opword, 3, 3)]; break;
		case FORMAT_11: operation = format11[BIT(m_opword, 2, 4)]; break;
		case FORMAT_12: operation = format12[BIT(m_opword, 2, 4)]; break;
		}

		if (m_status & SLAVE_Q)
			LOG("execute %s 0x%x,0x%x exception\n", operation, m_op[0].value, m_op[1].value);
		else
			LOG("execute %s 0x%x,0x%x result 0x%x\n", operation, m_op[0].value, m_op[1].value, m_op[2].value);
	}

	// write-back floating point register results
	if (m_op[2].expected && !BIT(m_opword, 9, 2))
	{
		// exclude integer results (roundfi, truncfi, sfsr, floorfi)
		if (m_idbyte != FORMAT_9 || (BIT(m_opword, 3, 3) < 4))
		{
			reg_set(BIT(m_opword, 6, 3), m_op[2].expected, m_op[2].value);

			if (type() == NS32381)
				m_fsr |= FSR_RMB;

			m_op[2].issued = m_op[2].expected;
		}
	}

	if (!m_out_spc.isunset())
		m_complete->adjust(attotime::from_ticks(m_tcy, clock()));

	m_state = STATUS;
}

u16 ns32081_device_base::status(int *icount)
{
	if (m_state == STATUS)
	{
		m_state = (m_op[2].issued == m_op[2].expected) ? IDLE : RESULT;

		if (icount)
			*icount -= m_tcy;

		LOG("status 0x%04x tcy %d %s (%s)\n", m_status, m_tcy,
			(m_state == RESULT ? "results pending" : "complete"), machine().describe_context());

		return m_status;
	}

	logerror("status protocol error (%s)\n", machine().describe_context());
	return 0;
}

void ns32081_device_base::complete(s32 param)
{
	m_out_spc(0);
	m_out_spc(1);
}

ns32081_device::ns32081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ns32081_device_base(mconfig, NS32081, tag, owner, clock)
	, ns32000_slow_slave_interface(mconfig, *this)
{
}

void ns32081_device::device_start()
{
	ns32081_device_base::device_start();

	save_item(NAME(m_f));
}

void ns32081_device::device_reset()
{
	ns32081_device_base::device_reset();

	std::fill(std::begin(m_f), std::end(m_f), 0);
}

void ns32081_device::state_add(device_state_interface &parent, int &index)
{
	ns32081_device_base::state_add(parent, index);

	for (unsigned i = 0; i < 8; i++)
		parent.state_add(index++, util::string_format("F%d", i).c_str(), m_f[i]).formatstr("%08X");
}

void ns32081_device::reg_get(unsigned const op_size, u64 &op_value, unsigned const reg) const
{
	op_value = m_f[reg ^ 0];
	if (op_size == LENGTH_L)
		op_value |= u64(m_f[reg ^ 1]) << 32;

	if (op_size == LENGTH_L)
		LOG("reg_get f%d:%d data 0x%016x\n", reg ^ 1, reg ^ 0, op_value);
	else
		LOG("reg_get f%d data 0x%08x\n", reg, op_value);
}

void ns32081_device::reg_set(unsigned const reg, unsigned const op_size, u64 const op_value)
{
	if (op_size == LENGTH_L)
		LOG("reg_set f%d:%d data 0x%016x\n", reg ^ 1, reg ^ 0, op_value);
	else
		LOG("reg_set f%d data 0x%08x\n", reg, op_value);

	m_f[reg ^ 0] = u32(op_value >> 0);
	if (op_size == LENGTH_L)
		m_f[reg ^ 1] = u32(op_value >> 32);
}

u16 ns32081_device::slow_status(int *icount)
{
	return status(icount);
}

u16 ns32081_device::slow_read()
{
	return read<u16>();
}

void ns32081_device::slow_write(u16 data)
{
	write<u16>(data);
}

ns32381_device::ns32381_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ns32081_device_base(mconfig, NS32381, tag, owner, clock)
	, ns32000_slow_slave_interface(mconfig, *this)
	, ns32000_fast_slave_interface(mconfig, *this)
{
}

void ns32381_device::device_start()
{
	ns32081_device_base::device_start();

	save_item(NAME(m_l));
}

void ns32381_device::device_reset()
{
	std::fill(std::begin(m_l), std::end(m_l), 0);
}

void ns32381_device::state_add(device_state_interface &parent, int &index)
{
	ns32081_device_base::state_add(parent, index);

	for (unsigned i = 0; i < 8; i++)
		parent.state_add(index++, util::string_format("L%d", i).c_str(), m_l[i]).formatstr("%016X");
}

void ns32381_device::reg_get(unsigned const op_size, u64 &op_value, unsigned const reg) const
{
	if (op_size == LENGTH_L)
		op_value = m_l[reg];
	else if (reg & 1)
		op_value = m_l[reg & 6] >> 32;
	else
		op_value = u32(m_l[reg & 6]);

	if (op_size == LENGTH_L)
		LOG("reg_get l%d data 0x%016x\n", reg, op_value);
	else
		LOG("reg_get f%d data 0x%08x\n", reg, op_value);
}

void ns32381_device::reg_set(unsigned const reg, unsigned const op_size, u64 const op_value)
{
	if (op_size == LENGTH_L)
		LOG("reg_set l%d data 0x%016x\n", reg, op_value);
	else
		LOG("reg_set f%d data 0x%08x\n", reg, op_value);

	if (op_size == LENGTH_L)
		m_l[reg] = op_value;
	else if (reg & 1)
		m_l[reg & 6] = (op_value << 32) | u32(m_l[reg & 6]);
	else
		m_l[reg & 6] = (m_l[reg & 6] & 0xffff'ffff'0000'0000ULL) | u32(op_value);
}

u16 ns32381_device::slow_status(int *icount)
{
	return status(icount);
}

u16 ns32381_device::slow_read()
{
	return read<u16>();
}

void ns32381_device::slow_write(u16 data)
{
	write<u16>(data);
}

u32 ns32381_device::fast_status(int *icount)
{
	return status(icount);
}

u32 ns32381_device::fast_read()
{
	return read<u32>();
}

void ns32381_device::fast_write(u32 data)
{
	write<u32>(data);
}
