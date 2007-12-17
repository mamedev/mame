/**************************************************************************
 *               National Semiconductor COP420 Emulator                   *
 *                                                                        *
 *                   Copyright (C) 2006 MAME Team                         *
 **************************************************************************/

#define SC				R.SC

#define PUSH(addr) 		{ SC = SB; SB = SA; SA = addr; }
#define POP() 			{ PC = SA; SA = SB; SB = SC; }

#include "410ops.c"

#define IN_IN()			IN(COP400_PORT_IN)

INLINE void adt(void) { A = (A + 10) & 0x0F; }

INLINE void casc(void)
{
	A = (A ^ 0xF) + RAM_R(B) + C;

	if (A > 0xF)
	{
		C = 1;
		skip = 1;
		A &= 0xF;
	}
	else
	{
		C = 0;
	}
}

INLINE void cqma(void)
{
	WRITE_M(Q >> 4);
	A = Q & 0xF;
}

INLINE void inil(void)
{
	// NOT IMPLEMENTED
	A = IN_IN();
}

INLINE void cop421_inil(void)
{
	// NOT IMPLEMENTED
}

INLINE void inin(void) { A = IN_IN(); }

INLINE void cop402m_inin(void) { A = IN_IN() | 0x02; }

INLINE void ldd(void)
{
	A = RAM_R(ROM(PC++) & 0x3f);
}

INLINE void ogi0(void) { WRITE_G(0); }
INLINE void ogi1(void) { WRITE_G(1); }
INLINE void ogi2(void) { WRITE_G(2); }
INLINE void ogi3(void) { WRITE_G(3); }
INLINE void ogi4(void) { WRITE_G(4); }
INLINE void ogi5(void) { WRITE_G(5); }
INLINE void ogi6(void) { WRITE_G(6); }
INLINE void ogi7(void) { WRITE_G(7); }
INLINE void ogi8(void) { WRITE_G(8); }
INLINE void ogi9(void) { WRITE_G(9); }
INLINE void ogi10(void) { WRITE_G(10); }
INLINE void ogi11(void) { WRITE_G(11); }
INLINE void ogi12(void) { WRITE_G(12); }
INLINE void ogi13(void) { WRITE_G(13); }
INLINE void ogi14(void) { WRITE_G(14); }
INLINE void ogi15(void) { WRITE_G(15); }

INLINE void skt(void)
{
	if (R.timerlatch == 1)
	{
		R.timerlatch = 0;
		skip = 1;
	}
}

INLINE void xabr(void)
{
	UINT8 Br = A & 0x03;
	UINT8 Bd = B & 0x0f;

	A = (B & 0x30) >> 4;
	B = (Br << 4) + Bd;
}
