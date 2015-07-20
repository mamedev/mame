// license:GPL-2.0+
// copyright-holders:Fabio Priuli, byuu, Nach
/***************************************************************************

    snescx4.h

    Code based on original work by zsKnight, anomie and Nach.
    This implementation is based on C++ "cx4*.cpp" by byuu
    (up to date with source v 0.49).

***************************************************************************/

struct CX4
{
		UINT8 ram[0x0c00];
		UINT8 reg[0x0100];
		UINT32 r0, r1, r2,  r3,  r4,  r5,  r6,  r7,
					r8, r9, r10, r11, r12, r13, r14, r15;

		INT16 C4WFXVal, C4WFYVal, C4WFZVal, C4WFX2Val, C4WFY2Val, C4WFDist, C4WFScale;
		INT16 C41FXVal, C41FYVal, C41FAngleRes, C41FDist, C41FDistVal;

		double tanval;
		double c4x, c4y, c4z;
		double c4x2, c4y2, c4z2;
};

UINT8 CX4_read(UINT32 addr);
void CX4_write(running_machine &machine, UINT32 addr, UINT8 data);
