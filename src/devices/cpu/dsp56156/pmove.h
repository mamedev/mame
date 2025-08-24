// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_CPU_DSP56156_PMOVE_H
#define MAME_CPU_DSP56156_PMOVE_H

#pragma once

#include "opcode.h"
#include "tables.h"

//
// A ParallelMove Object is what all parallel move classes inherit from.
//
namespace DSP_56156 {

class Opcode;

class ParallelMove
{
public:
	ParallelMove(const Opcode* oco) : m_valid(false), m_oco(oco) { }
	virtual ~ParallelMove() {}

	virtual bool decode(const uint16_t word0, const uint16_t word1) = 0;
	virtual std::string disassemble() const = 0;

	static std::unique_ptr<ParallelMove> decodeParallelMove(const Opcode* opc, const uint16_t word0, const uint16_t word1);

	bool valid() const { return m_valid; }

	// Peek through the opcode to see the instruction
	const reg_id& opSource() const;
	const reg_id& opDestination() const;
	size_t opAccumulatorBitsModified() const;

protected:
	bool m_valid;
	const Opcode* m_oco;
};


////////////////////////////////////////////////////////////////////////////////
//  PARALLEL MOVES                  ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
class XMemoryDataMove: public ParallelMove
{
public:
	XMemoryDataMove(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		reg_id r;
		decode_RR_table(BITSn(word0,0x3000), r);

		reg_id SD;
		decode_HHH_table(BITSn(word0,0x0e00), SD);

		std::string ea;
		assemble_ea_from_m_table(BITSn(word0,0x4000), regIDAsNum(r), ea);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source, m_destination);

		// If the destination of the instruction overlaps with our destination, abort.
		if (registerOverlap(opDestination(), opAccumulatorBitsModified(), stringAsRegID(m_destination)))
			return false;

		return true;
	}
	std::string disassemble() const override
	{
		return m_source + "," + m_destination;
	}

private:
	std::string m_source;
	std::string m_destination;
};


/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
class XMemoryDataMove_2: public ParallelMove
{
public:
	XMemoryDataMove_2(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		std::string ea;
		if (opDestination() == iB)
			ea = "(A1)";
		else if (opDestination() == iA)
			ea = "(B1)";
		else
			ea = "(A1)";

		reg_id SD;
		decode_HHH_table(BITSn(word0,0x0e00), SD);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source, m_destination);

		// If the destination of the instruction overlaps with our destination, abort.
		if (registerOverlap(opDestination(), opAccumulatorBitsModified(), stringAsRegID(m_destination)))
			return false;

		return true;
	}
	std::string disassemble() const override
	{
		return m_source + "," + m_destination;
	}

private:
	std::string m_source;
	std::string m_destination;
};


/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
class DualXMemoryDataRead: public ParallelMove
{
public:
	DualXMemoryDataRead(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		reg_id r;
		reg_id D1;
		reg_id D2;
		std::string ea1 = "";
		std::string ea2 = "";

		decode_rr_table(BITSn(word0,0x0060), r);
		decode_KKK_table(BITSn(word0,0x0700), D1, D2);
		assemble_eas_from_mm_table(BITSn(word0,0x1800), regIDAsNum(r), 3, ea1, ea2);

		/* Not documented, but extrapolated from docs on page A-133 */
		if (D1 == iFHAT)
		{
			if (opDestination() == iB)
				D1 = iA;
			else if (opDestination() == iA)
				D1 = iB;
			else
				D1 = iA;   /* In the case of no data ALU instruction */
		}

		/* D1 and D2 may not specify the same register : A-142 */
		if (r == iR3) return false;

		parallelMove = util::string_format("X:%s,%s", ea1, regIdAsString(D1));
		parallelMove2 = util::string_format("X:%s,%s", ea2, regIdAsString(D2));

		return true;
	}
	std::string disassemble() const override
	{
		return parallelMove + " " + parallelMove2;
	}

private:
	std::string parallelMove;
	std::string parallelMove2;
};


/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
class RegisterToRegisterDataMove: public ParallelMove
{
public:
	RegisterToRegisterDataMove(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		decode_IIIIx_table(BITSn(word0,0x0f00), BITSn(word0,0x0008),
							m_source, m_destination);

		if (m_source == iINVALID)
			return false;

		if (m_source == iF)
			m_source = opDestination();

		if (m_destination == iFHAT)
		{
			if (opDestination() == iB)
				m_destination = iA;
			else if (opDestination() == iA)
				m_destination = iB;
			else
				m_destination = iA; /* In the case of no data ALU instruction */
		}

		// Don't return a failure, just let everything fall through (nop).
		//if (m_source == "?" && m_destination == "?")
		//  return false;

		return true;
	}
	std::string disassemble() const override
	{
		// (?,?) is a parallel nop
		if (m_source == iWEIRD && m_destination == iWEIRD)
			return "";
		else
			return regIdAsString(m_source) + "," + regIdAsString(m_destination);
	}

private:
	reg_id m_source;
	reg_id m_destination;
};


/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
class XMemoryDataWriteAndRegisterDataMove: public ParallelMove
{
public:
	XMemoryDataWriteAndRegisterDataMove(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		pms = "";
		pms2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		reg_id r;
		reg_id S;
		reg_id Dnot;

		if (opDestination() == iA) Dnot = iB;
		else                       Dnot = iA;

		// NEW // decode_k_table(BITSn(word0,0x0100), Dnot);
		decode_RR_table(BITSn(word0,0x00c0), r);
		decode_DD_table(BITSn(word0,0x0030), S);

		pms = util::string_format("%s,X:(R%d)+N%d", regIdAsString(Dnot), regIDAsNum(r), regIDAsNum(r));
		pms2 = util::string_format("%s,%s", regIdAsString(S), regIdAsString(Dnot));
		return true;
	}
	std::string disassemble() const override
	{
		return pms + " " + pms2;
	}

private:
	std::string pms;    // TODO
	std::string pms2;
};


/* Address Register Update : 0011 0zRR .... .... : A-135 */
class AddressRegisterUpdate: public ParallelMove
{
public:
	AddressRegisterUpdate(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_ea = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		reg_id r;
		decode_RR_table(BITSn(word0,0x0300), r);
		assemble_ea_from_z_table(BITSn(word0,0x0400), regIDAsNum(r), m_ea);

		return true;
	}
	std::string disassemble() const override
	{
		return m_ea;
	}

private:
	std::string m_ea;
};


/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
class XMemoryDataMoveWithShortDisplacement: public ParallelMove
{
public:
	XMemoryDataMoveWithShortDisplacement(const Opcode* oco, const uint16_t word0, const uint16_t word1) : ParallelMove(oco)
	{
		m_source = "";
		m_destination = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const uint16_t word0, const uint16_t word1) override
	{
		int8_t b;
		reg_id SD;
		b = (char)(word0 & 0x00ff);
		decode_HHH_table(BITSn(word1,0x0e00), SD);
		assemble_reg_from_W_table(BITSn(word1,0x0100), 'X', SD, b, m_source, m_destination);

		return true;
	}
	std::string disassemble() const override
	{
		return m_source + "," + m_destination;
	}

private:
	std::string m_source;
	std::string m_destination;
};

} // namespace DSP_56156

#endif // MAME_CPU_DSP56156_PMOVE_H
