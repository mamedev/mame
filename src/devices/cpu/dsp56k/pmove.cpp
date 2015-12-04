// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "pmove.h"

namespace DSP56K
{
const reg_id& ParallelMove::opSource() const { return m_oco->instSource(); }
const reg_id& ParallelMove::opDestination() const { return m_oco->instDestination(); }
size_t ParallelMove::opAccumulatorBitsModified() const { return m_oco->instAccumulatorBitsModified(); }


std::unique_ptr<ParallelMove> ParallelMove::decodeParallelMove(const Opcode* opc, const UINT16 word0, const UINT16 word1)
{
	const UINT16 w0 = word0;
	const UINT16 w1 = word1;

	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
	if ((w0 & 0xe000) == 0x6000)
	{
		return std::make_unique<DualXMemoryDataRead>(opc, w0, w1);
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	else if ((w0 & 0xfe00) == 0x1600)
	{
		return std::make_unique<XMemoryDataWriteAndRegisterDataMove>(opc, w0, w1);
	}
	else
	{
		/* 32 General parallel move operations */
		/* Note: It's important that NPDM comes before RtRDM */

		/* No Parallel Data Move : 0100 1010 .... .... : A-131 */
		if ((w0 & 0xff00) == 0x4a00)
		{
			return nullptr;
		}
		/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
		else if ((w0 & 0xf000) == 0x4000)
		{
			return std::make_unique<RegisterToRegisterDataMove>(opc, w0, w1);
		}
		/* Address Register Update : 0011 0zRR .... .... : A-135 */
		else if ((w0 & 0xf800) == 0x3000)
		{
			return std::make_unique<AddressRegisterUpdate>(opc, w0, w1);
		}
		/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
		else if ((w0 & 0x8000) == 0x8000)
		{
			return std::make_unique<XMemoryDataMove>(opc, w0, w1);
		}
		/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
		else if ((w0 & 0xf000) == 0x5000)
		{
			return std::make_unique<XMemoryDataMove_2>(opc, w0, w1);
		}
		/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
		else if ((w0 & 0xff00) == 0x0500)
		{
			// Now check it against all potential double-ups.
			// These operations can't have an additional parallel move.
			//
			// MOVE(M) :   0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152
			// MOVE(C) :   0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144
			// MOVE :      0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128
			//
			if (((w1 & 0xfe20) != 0x0200) &&
				((w1 & 0xf810) != 0x3800) &&
				((w1 & 0x00ff) != 0x0011))
			{
				return std::make_unique<XMemoryDataMoveWithShortDisplacement>(opc, w0, w1);
			}
		}
	}

	return nullptr;
}

}
