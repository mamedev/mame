#ifndef __DSP56K_PARALLEL_MOVE_H__
#define __DSP56K_PARALLEL_MOVE_H__

#include <string>

#include "emu.h"
#include "opcode.h"
#include "tables.h"

//
// A ParallelMove Object is what all parallel move classes inherit from.
//
namespace DSP56K
{

class Opcode;

class ParallelMove
{
public:
	ParallelMove(const Opcode* oco) : m_valid(false), m_oco(oco) { }
	virtual ~ParallelMove() {}

	virtual bool decode(const UINT16 word0, const UINT16 word1) = 0;
	virtual void disassemble(std::string& retString) const = 0;
	virtual void evaluate() = 0;

	static ParallelMove* decodeParallelMove(const Opcode* opc, const UINT16 word0, const UINT16 word1);

	const bool valid() const { return m_valid; }

	// Peek through the opcode to see the instruction
	const std::string& opSource() const;
	const std::string& opDestination() const;
	const size_t opAccumulatorBitsModified() const;

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
	XMemoryDataMove(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		decode_RR_table(BITSn(word0,0x3000), rNum);

		std::string SD;
		decode_HHH_table(BITSn(word0,0x0e00), SD);

		std::string ea;
		assemble_ea_from_m_table(BITSn(word0,0x4000), rNum, ea);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source, m_destination);

		// If the destination of the instruction overlaps with our destination, abort.
		if (registerOverlap(opDestination(), opAccumulatorBitsModified(), m_destination))
			return false;

		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_source + "," + m_destination;
	}
	void evaluate() {}

private:
	std::string m_source;
	std::string m_destination;
};


/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
class XMemoryDataMove_2: public ParallelMove
{
public:
	XMemoryDataMove_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string ea;
		if (opDestination() == "B")
			ea = "(A1)";
		else if (opDestination() == "A")
			ea = "(B1)";
		else
			ea = "(A1)";

		std::string SD;
		decode_HHH_table(BITSn(word0,0x0e00), SD);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source, m_destination);

		// If the destination of the instruction overlaps with our destination, abort.
		if (registerOverlap(opDestination(), opAccumulatorBitsModified(), m_destination))
			return false;

		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_source + "," + m_destination;
	}
	void evaluate() {}

private:
	std::string m_source;
	std::string m_destination;
};


/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
class DualXMemoryDataRead: public ParallelMove
{
public:
	DualXMemoryDataRead(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		std::string D1 = "";
		std::string D2 = "";
		std::string ea1 = "";
		std::string ea2 = "";

		decode_rr_table(BITSn(word0,0x0060), rNum);
		decode_KKK_table(BITSn(word0,0x0700), D1, D2);
		assemble_eas_from_mm_table(BITSn(word0,0x1800), rNum, 3, ea1, ea2);

		/* Not documented, but extrapolated from docs on page A-133 */
		if (D1 == "^F")
		{
			if (opDestination() == "B")
				D1 = "A";
			else if (opDestination() == "A")
				D1 = "B";
			else
				D1 = "A";   /* In the case of no data ALU instruction */
		}

		/* D1 and D2 may not specify the same register : A-142 */
		if (rNum == 3) return false;

		char temp[32];
		sprintf(temp,  "X:%s,%s", ea1.c_str(), D1.c_str());
		parallelMove = temp;
		sprintf(temp, "X:%s,%s", ea2.c_str(), D2.c_str());
		parallelMove2 = temp;

		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = parallelMove + " " + parallelMove2;
	}
	void evaluate() {}

private:
	std::string parallelMove;
	std::string parallelMove2;
};


/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
class RegisterToRegisterDataMove: public ParallelMove
{
public:
	RegisterToRegisterDataMove(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_IIIIx_table(BITSn(word0,0x0f00), BITSn(word0,0x0008),
						   m_source, m_destination);

		if (m_source == "!")
			return false;

		if (m_source == "F")
			m_source = opDestination();

		if (m_destination == "^F")
		{
			if (opDestination() == "B")
				m_destination = "A";
			else if (opDestination() == "A")
				m_destination = "B";
			else
				m_destination = "A";	/* In the case of no data ALU instruction */
		}

		// Don't return a failure, just let everything fall through (nop).
		//if (m_source == "?" && m_destination == "?")
		//  return false;

		return true;
	}
	void disassemble(std::string& retString) const
	{
		// (?,?) is a parallel nop
		if (m_source == "?" && m_destination == "?")
			retString = "";
		else
			retString = m_source + "," + m_destination;
	}
	void evaluate() {}

private:
	std::string m_source;
	std::string m_destination;
};


/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
class XMemoryDataWriteAndRegisterDataMove: public ParallelMove
{
public:
	XMemoryDataWriteAndRegisterDataMove(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		pms = "";
		pms2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		std::string S;
		std::string Dnot;
		char parallel_move_str[128];
		char parallel_move_str2[128];

		if (opDestination() == "A") Dnot = "B";
		else                        Dnot = "A";

		// NEW // decode_k_table(BITSn(word0,0x0100), Dnot);
		decode_RR_table(BITSn(word0,0x00c0), rNum);
		decode_DD_table(BITSn(word0,0x0030), S);

		sprintf(parallel_move_str,  "%s,X:(R%d)+N%d", Dnot.c_str(), rNum, rNum);
		sprintf(parallel_move_str2, "%s,%s", S.c_str(), Dnot.c_str());
		pms = parallel_move_str;
		pms2 = parallel_move_str2;
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = pms + " " + pms2;
	}
	void evaluate() {}

private:
	std::string pms;    // TODO
	std::string pms2;
};


/* Address Register Update : 0011 0zRR .... .... : A-135 */
class AddressRegisterUpdate: public ParallelMove
{
public:
	AddressRegisterUpdate(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_ea = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		decode_RR_table(BITSn(word0,0x0300), rNum);
		assemble_ea_from_z_table(BITSn(word0,0x0400), rNum, m_ea);

		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_ea;
	}
	void evaluate() {}

private:
	std::string m_ea;
};


/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
class XMemoryDataMoveWithShortDisplacement: public ParallelMove
{
public:
	XMemoryDataMoveWithShortDisplacement(const Opcode* oco, const UINT16 word0, const UINT16 word1) : ParallelMove(oco)
	{
		m_source = "";
		m_destination = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 b;
		std::string SD;
		std::string args;

		b = (char)(word0 & 0x00ff);
		decode_HHH_table(BITSn(word1,0x0e00), SD);
		assemble_reg_from_W_table(BITSn(word1,0x0100), 'X', SD, b, m_source, m_destination);

		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_source + "," + m_destination;
	}
	void evaluate() {}

private:
	std::string m_source;
	std::string m_destination;
};

}
#endif
