// license:GPL-2.0+
// copyright-holders:byuu, Nach
/***************************************************************************

    cx4fn.c

    Code based on original work by zsKnight, anomie and Nach.
    This implementation is based on C++ "cx4*.cpp" by byuu.
    (up to date with source v 0.49).

***************************************************************************/

#include <math.h>
#define CX4_Tan(a) (CX4_CosTable[a] ? ((((INT32)CX4_SinTable[a]) << 16) / CX4_CosTable[a]) : 0x80000000)
#define CX4_sar(b, n) ((b) >> (n))
#ifdef PI
#undef PI
#endif
#define PI 3.1415926535897932384626433832795

//Wireframe Helpers
static void CX4_C4TransfWireFrame(void)
{
	cx4.c4x = (double)cx4.C4WFXVal;
	cx4.c4y = (double)cx4.C4WFYVal;
	cx4.c4z = (double)cx4.C4WFZVal - 0x95;

	//Rotate X
	cx4.tanval = -(double)cx4.C4WFX2Val * PI * 2 / 128;
	cx4.c4y2   = cx4.c4y * cos(cx4.tanval) - cx4.c4z * sin(cx4.tanval);
	cx4.c4z2   = cx4.c4y * sin(cx4.tanval) + cx4.c4z * cos(cx4.tanval);

	//Rotate Y
	cx4.tanval = -(double)cx4.C4WFY2Val * PI * 2 / 128;
	cx4.c4x2   = cx4.c4x * cos(cx4.tanval)  + cx4.c4z2 * sin(cx4.tanval);
	cx4.c4z    = cx4.c4x * -sin(cx4.tanval) + cx4.c4z2 * cos(cx4.tanval);

	//Rotate Z
	cx4.tanval = -(double)cx4.C4WFDist * PI * 2 / 128;
	cx4.c4x    = cx4.c4x2 * cos(cx4.tanval) - cx4.c4y2 * sin(cx4.tanval);
	cx4.c4y    = cx4.c4x2 * sin(cx4.tanval) + cx4.c4y2 * cos(cx4.tanval);

	//Scale
	cx4.C4WFXVal = (INT16)(cx4.c4x * cx4.C4WFScale / (0x90 * (cx4.c4z + 0x95)) * 0x95);
	cx4.C4WFYVal = (INT16)(cx4.c4y * cx4.C4WFScale / (0x90 * (cx4.c4z + 0x95)) * 0x95);
}

static void CX4_C4CalcWireFrame(void)
{
	cx4.C4WFXVal = cx4.C4WFX2Val - cx4.C4WFXVal;
	cx4.C4WFYVal = cx4.C4WFY2Val - cx4.C4WFYVal;

	if(abs(cx4.C4WFXVal) > abs(cx4.C4WFYVal))
	{
		cx4.C4WFDist = abs(cx4.C4WFXVal) + 1;
		cx4.C4WFYVal = (256 * (long)cx4.C4WFYVal) / abs(cx4.C4WFXVal);
		cx4.C4WFXVal = (cx4.C4WFXVal < 0) ? -256 : 256;
	}
	else if(cx4.C4WFYVal != 0)
	{
		cx4.C4WFDist = abs(cx4.C4WFYVal) + 1;
		cx4.C4WFXVal = (256 * (long)cx4.C4WFXVal) / abs(cx4.C4WFYVal);
		cx4.C4WFYVal = (cx4.C4WFYVal < 0) ? -256 : 256;
	}
	else
	{
		cx4.C4WFDist = 0;
	}
}

static void CX4_C4TransfWireFrame2(void)
{
	cx4.c4x = (double)cx4.C4WFXVal;
	cx4.c4y = (double)cx4.C4WFYVal;
	cx4.c4z = (double)cx4.C4WFZVal;

	//Rotate X
	cx4.tanval = -(double)cx4.C4WFX2Val * PI * 2 / 128;
	cx4.c4y2   = cx4.c4y * cos(cx4.tanval) - cx4.c4z * sin(cx4.tanval);
	cx4.c4z2   = cx4.c4y * sin(cx4.tanval) + cx4.c4z * cos(cx4.tanval);

	//Rotate Y
	cx4.tanval = -(double)cx4.C4WFY2Val * PI * 2 / 128;
	cx4.c4x2   = cx4.c4x * cos(cx4.tanval)  + cx4.c4z2 * sin(cx4.tanval);
	cx4.c4z    = cx4.c4x * -sin(cx4.tanval) + cx4.c4z2 * cos(cx4.tanval);

	//Rotate Z
	cx4.tanval = -(double)cx4.C4WFDist * PI * 2 / 128;
	cx4.c4x    = cx4.c4x2 * cos(cx4.tanval) - cx4.c4y2 * sin(cx4.tanval);
	cx4.c4y    = cx4.c4x2 * sin(cx4.tanval) + cx4.c4y2 * cos(cx4.tanval);

	//Scale
	cx4.C4WFXVal = (INT16)(cx4.c4x * cx4.C4WFScale / 0x100);
	cx4.C4WFYVal = (INT16)(cx4.c4y * cx4.C4WFScale / 0x100);
}

static void CX4_C4DrawWireFrame(running_machine &machine)
{
	UINT32 line = CX4_readl(0x1f80);
	UINT32 point1, point2;
	INT16 X1, Y1, Z1;
	INT16 X2, Y2, Z2;
	UINT8 Color;
	INT32 i;

	address_space &space = machine.device<cpu_device>("maincpu")->space(AS_PROGRAM);
	for(i = cx4.ram[0x0295]; i > 0; i--, line += 5)
	{
		if(space.read_byte(line) == 0xff &&
			space.read_byte(line + 1) == 0xff)
		{
			INT32 tmp = line - 5;
			while(space.read_byte(tmp + 2) == 0xff &&
					space.read_byte(tmp + 3) == 0xff &&
					(tmp + 2) >= 0)
			{
				tmp -= 5;
			}
			point1 = (CX4_read(0x1f82) << 16) |
						(space.read_byte(tmp + 2) << 8) |
						space.read_byte(tmp + 3);
		}
		else
		{
			point1 = (CX4_read(0x1f82) << 16) |
						(space.read_byte(line) << 8) |
						space.read_byte(line + 1);
		}
		point2 = (CX4_read(0x1f82) << 16) |
					(space.read_byte(line + 2) << 8) |
					space.read_byte(line + 3);

		X1=(space.read_byte(point1 + 0) << 8) |
			space.read_byte(point1 + 1);
		Y1=(space.read_byte(point1 + 2) << 8) |
			space.read_byte(point1 + 3);
		Z1=(space.read_byte(point1 + 4) << 8) |
			space.read_byte(point1 + 5);
		X2=(space.read_byte(point2 + 0) << 8) |
			space.read_byte(point2 + 1);
		Y2=(space.read_byte(point2 + 2) << 8) |
			space.read_byte(point2 + 3);
		Z2=(space.read_byte(point2 + 4) << 8) |
			space.read_byte(point2 + 5);
		Color = space.read_byte(line + 4);
		CX4_C4DrawLine(X1, Y1, Z1, X2, Y2, Z2, Color);
	}
}

static void CX4_C4DrawLine(INT32 X1, INT32 Y1, INT16 Z1, INT32 X2, INT32 Y2, INT16 Z2, UINT8 Color)
{
	INT32 i;

	//Transform coordinates
	cx4.C4WFXVal  = (INT16)X1;
	cx4.C4WFYVal  = (INT16)Y1;
	cx4.C4WFZVal  = Z1;
	cx4.C4WFScale = CX4_read(0x1f90);
	cx4.C4WFX2Val = CX4_read(0x1f86);
	cx4.C4WFY2Val = CX4_read(0x1f87);
	cx4.C4WFDist  = CX4_read(0x1f88);
	CX4_C4TransfWireFrame2();
	X1 = (cx4.C4WFXVal + 48) << 8;
	Y1 = (cx4.C4WFYVal + 48) << 8;

	cx4.C4WFXVal = (INT16)X2;
	cx4.C4WFYVal = (INT16)Y2;
	cx4.C4WFZVal = Z2;
	CX4_C4TransfWireFrame2();
	X2 = (cx4.C4WFXVal + 48) << 8;
	Y2 = (cx4.C4WFYVal + 48) << 8;

	//Get line info
	cx4.C4WFXVal  = (INT16)(X1 >> 8);
	cx4.C4WFYVal  = (INT16)(Y1 >> 8);
	cx4.C4WFX2Val = (INT16)(X2 >> 8);
	cx4.C4WFY2Val = (INT16)(Y2 >> 8);
	CX4_C4CalcWireFrame();
	X2 = (INT16)cx4.C4WFXVal;
	Y2 = (INT16)cx4.C4WFYVal;

	//Render line
	for(i = cx4.C4WFDist ? cx4.C4WFDist : 1; i > 0; i--)
	{
		if(X1 > 0xff && Y1 > 0xff && X1 < 0x6000 && Y1 < 0x6000)
		{
			UINT16 addr = (((Y1 >> 8) >> 3) << 8) - (((Y1 >> 8) >> 3) << 6) + (((X1 >> 8) >> 3) << 4) + ((Y1 >> 8) & 7) * 2;
			UINT8 bit = 0x80 >> ((X1 >> 8) & 7);
			cx4.ram[addr + 0x300] &= ~bit;
			cx4.ram[addr + 0x301] &= ~bit;
			if(Color & 1)
			{
				cx4.ram[addr + 0x300] |= bit;
			}
			if(Color & 2)
			{
				cx4.ram[addr + 0x301] |= bit;
			}
		}
		X1 += X2;
		Y1 += Y2;
	}
}

static void CX4_C4DoScaleRotate(int row_padding)
{
	INT16 A, B, C, D;
	INT32 x, y;

	//Calculate Pixel Resolution
	UINT8 w = CX4_read(0x1f89) & ~7;
	UINT8 h = CX4_read(0x1f8c) & ~7;

	INT32 Cx = (INT16)CX4_readw(0x1f83);
	INT32 Cy = (INT16)CX4_readw(0x1f86);

	INT32 LineX, LineY;
	UINT32 X, Y;
	UINT8 byte;
	INT32 outidx = 0;
	UINT8 bit    = 0x80;

	//Calculate matrix
	INT32 XScale = CX4_readw(0x1f8f);
	INT32 YScale = CX4_readw(0x1f92);

	if(XScale & 0x8000)
	{
		XScale = 0x7fff;
	}
	if(YScale & 0x8000)
	{
		YScale = 0x7fff;
	}

	if(CX4_readw(0x1f80) == 0)
	{ //no rotation
		A = (INT16)XScale;
		B = 0;
		C = 0;
		D = (INT16)YScale;
	}
	else if(CX4_readw(0x1f80) == 128)
	{ //90 degree rotation
		A = 0;
		B = (INT16)(-YScale);
		C = (INT16)XScale;
		D = 0;
	}
	else if(CX4_readw(0x1f80) == 256)
	{ //180 degree rotation
		A = (INT16)(-XScale);
		B = 0;
		C = 0;
		D = (INT16)(-YScale);
	}
	else if(CX4_readw(0x1f80) == 384)
	{ //270 degree rotation
		A = 0;
		B = (INT16)YScale;
		C = (INT16)(-XScale);
		D = 0;
	}
	else
	{
		A = (INT16)  CX4_sar(CX4_CosTable[CX4_readw(0x1f80) & 0x1ff] * XScale, 15);
		B = (INT16)(-CX4_sar(CX4_SinTable[CX4_readw(0x1f80) & 0x1ff] * YScale, 15));
		C = (INT16)  CX4_sar(CX4_SinTable[CX4_readw(0x1f80) & 0x1ff] * XScale, 15);
		D = (INT16)  CX4_sar(CX4_CosTable[CX4_readw(0x1f80) & 0x1ff] * YScale, 15);
	}

	//Clear the output RAM
	memset(cx4.ram, 0, (w + row_padding / 4) * h / 2);

	//Calculate start position (i.e. (Ox, Oy) = (0, 0))
	//The low 12 bits are fractional, so (Cx<<12) gives us the Cx we want in
	//the function. We do Cx*A etc normally because the matrix parameters
	//already have the fractional parts.
	LineX = (Cx << 12) - Cx * A - Cx * B;
	LineY = (Cy << 12) - Cy * C - Cy * D;

	//Start loop
	for(y = 0; y < h; y++)
	{
		X = LineX;
		Y = LineY;
		for(x = 0; x < w; x++)
		{
			if((X >> 12) >= w || (Y >> 12) >= h)
			{
				byte = 0;
			}
			else
			{
				UINT32 addr = (Y >> 12) * w + (X >> 12);
				byte = CX4_read(0x600 + (addr >> 1));
				if(addr & 1)
				{
					byte >>= 4;
				}
			}

			//De-bitplanify
			if(byte & 1) { cx4.ram[outidx     ] |= bit; }
			if(byte & 2) { cx4.ram[outidx +  1] |= bit; }
			if(byte & 4) { cx4.ram[outidx + 16] |= bit; }
			if(byte & 8) { cx4.ram[outidx + 17] |= bit; }

			bit >>= 1;
			if(!bit)
			{
				bit     = 0x80;
				outidx += 32;
			}

			X += A; //Add 1 to output x => add an A and a C
			Y += C;
		}
		outidx += 2 + row_padding;
		if(outidx & 0x10)
		{
			outidx &= ~0x10;
		}
		else
		{
			outidx -= w * 4 + row_padding;
		}
		LineX += B; //Add 1 to output y => add a B and a D
		LineY += D;
	}
}
