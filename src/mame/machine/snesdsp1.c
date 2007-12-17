/***************************************************************************

  snesdsp1.c

  File to handle emulation of the SNES "DSP-1" add-on chip.

  R. Belmont
  Code from http://users.tpg.com.au/trauma/dsp/dsp1.html
  By The Dumper (Electrical Engineering), Neviksti (Physics), Overload (Computer Science), Andreas Naive (Mathematics)

  This is up to date with the source version that was posted May 29, 2006.

***************************************************************************/

static INT16 dsp1_waitcmd, dsp1_in_cnt, dsp1_out_cnt, dsp1_in_idx, dsp1_out_idx, dsp1_first_parm, dsp1_cur_cmd;
static int has_dsp1;
static UINT8 dsp1_in[32], dsp1_out[16];

#define DSP1_VERSION 0x0102

// 1FH - Memory Dump (DSP1B Data ROM)
// Input    byte(1FH) integer(Undefined)
// Output   integer[1024]

static UINT16 DSP1ROM[1024];

// Command Translation Table

static const INT16 DSP1_CmdTable[64] = {
	  0x0000,  0x0001,  0x0002,  0x0003,  0x0004,  0x0001,  0x0006,  0x000f,
	  0x0008,  0x000d,  0x000a,  0x000b,  0x000c,  0x000d,  0x000e,  0x000f,
	  0x0010,  0x0011,  0x0002,  0x0013,  0x0014,  0x0011,  0x0006,  0x001f,
	  0x0018,  0x001d,  0x001a,  0x001b,  0x001c,  0x001d,  0x000e,  0x001f,
	  0x0020,  0x0021,  0x0002,  0x0023,  0x0004,  0x0021,  0x0006,  0x002f,
	  0x0028,  0x002d,  0x001a,  0x002b,  0x000c,  0x002d,  0x000e,  0x002f,
	  0x0010,  0x0001,  0x0002,  0x0003,  0x0014,  0x0001,  0x0006,  0x001f,
	  0x0038,  0x000d,  0x001a,  0x000b,  0x001c,  0x000d,  0x000e,  0x001f};

// Optimised for Performance

static const INT16 DSP1_MulTable[256] = {
	  0x0000,  0x0003,  0x0006,  0x0009,  0x000c,  0x000f,  0x0012,  0x0015,
	  0x0019,  0x001c,  0x001f,  0x0022,  0x0025,  0x0028,  0x002b,  0x002f,
	  0x0032,  0x0035,  0x0038,  0x003b,  0x003e,  0x0041,  0x0045,  0x0048,
	  0x004b,  0x004e,  0x0051,  0x0054,  0x0057,  0x005b,  0x005e,  0x0061,
	  0x0064,  0x0067,  0x006a,  0x006d,  0x0071,  0x0074,  0x0077,  0x007a,
	  0x007d,  0x0080,  0x0083,  0x0087,  0x008a,  0x008d,  0x0090,  0x0093,
	  0x0096,  0x0099,  0x009d,  0x00a0,  0x00a3,  0x00a6,  0x00a9,  0x00ac,
	  0x00af,  0x00b3,  0x00b6,  0x00b9,  0x00bc,  0x00bf,  0x00c2,  0x00c5,
	  0x00c9,  0x00cc,  0x00cf,  0x00d2,  0x00d5,  0x00d8,  0x00db,  0x00df,
	  0x00e2,  0x00e5,  0x00e8,  0x00eb,  0x00ee,  0x00f1,  0x00f5,  0x00f8,
	  0x00fb,  0x00fe,  0x0101,  0x0104,  0x0107,  0x010b,  0x010e,  0x0111,
	  0x0114,  0x0117,  0x011a,  0x011d,  0x0121,  0x0124,  0x0127,  0x012a,
	  0x012d,  0x0130,  0x0133,  0x0137,  0x013a,  0x013d,  0x0140,  0x0143,
	  0x0146,  0x0149,  0x014d,  0x0150,  0x0153,  0x0156,  0x0159,  0x015c,
	  0x015f,  0x0163,  0x0166,  0x0169,  0x016c,  0x016f,  0x0172,  0x0175,
	  0x0178,  0x017c,  0x017f,  0x0182,  0x0185,  0x0188,  0x018b,  0x018e,
	  0x0192,  0x0195,  0x0198,  0x019b,  0x019e,  0x01a1,  0x01a4,  0x01a8,
	  0x01ab,  0x01ae,  0x01b1,  0x01b4,  0x01b7,  0x01ba,  0x01be,  0x01c1,
	  0x01c4,  0x01c7,  0x01ca,  0x01cd,  0x01d0,  0x01d4,  0x01d7,  0x01da,
	  0x01dd,  0x01e0,  0x01e3,  0x01e6,  0x01ea,  0x01ed,  0x01f0,  0x01f3,
	  0x01f6,  0x01f9,  0x01fc,  0x0200,  0x0203,  0x0206,  0x0209,  0x020c,
	  0x020f,  0x0212,  0x0216,  0x0219,  0x021c,  0x021f,  0x0222,  0x0225,
	  0x0228,  0x022c,  0x022f,  0x0232,  0x0235,  0x0238,  0x023b,  0x023e,
	  0x0242,  0x0245,  0x0248,  0x024b,  0x024e,  0x0251,  0x0254,  0x0258,
	  0x025b,  0x025e,  0x0261,  0x0264,  0x0267,  0x026a,  0x026e,  0x0271,
	  0x0274,  0x0277,  0x027a,  0x027d,  0x0280,  0x0284,  0x0287,  0x028a,
	  0x028d,  0x0290,  0x0293,  0x0296,  0x029a,  0x029d,  0x02a0,  0x02a3,
	  0x02a6,  0x02a9,  0x02ac,  0x02b0,  0x02b3,  0x02b6,  0x02b9,  0x02bc,
	  0x02bf,  0x02c2,  0x02c6,  0x02c9,  0x02cc,  0x02cf,  0x02d2,  0x02d5,
	  0x02d8,  0x02db,  0x02df,  0x02e2,  0x02e5,  0x02e8,  0x02eb,  0x02ee,
	  0x02f1,  0x02f5,  0x02f8,  0x02fb,  0x02fe,  0x0301,  0x0304,  0x0307,
	  0x030b,  0x030e,  0x0311,  0x0314,  0x0317,  0x031a,  0x031d,  0x0321};

static const INT16 DSP1_SinTable[256] = {
	  0x0000,  0x0324,  0x0647,  0x096a,  0x0c8b,  0x0fab,  0x12c8,  0x15e2,
	  0x18f8,  0x1c0b,  0x1f19,  0x2223,  0x2528,  0x2826,  0x2b1f,  0x2e11,
	  0x30fb,  0x33de,  0x36ba,  0x398c,  0x3c56,  0x3f17,  0x41ce,  0x447a,
	  0x471c,  0x49b4,  0x4c3f,  0x4ebf,  0x5133,  0x539b,  0x55f5,  0x5842,
	  0x5a82,  0x5cb4,  0x5ed7,  0x60ec,  0x62f2,  0x64e8,  0x66cf,  0x68a6,
	  0x6a6d,  0x6c24,  0x6dca,  0x6f5f,  0x70e2,  0x7255,  0x73b5,  0x7504,
	  0x7641,  0x776c,  0x7884,  0x798a,  0x7a7d,  0x7b5d,  0x7c29,  0x7ce3,
	  0x7d8a,  0x7e1d,  0x7e9d,  0x7f09,  0x7f62,  0x7fa7,  0x7fd8,  0x7ff6,
	  0x7fff,  0x7ff6,  0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,
	  0x7d8a,  0x7ce3,  0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,
	  0x7641,  0x7504,  0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,
	  0x6a6d,  0x68a6,  0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,
	  0x5a82,  0x5842,  0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,
	  0x471c,  0x447a,  0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,
	  0x30fb,  0x2e11,  0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,
	  0x18f8,  0x15e2,  0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,
	 -0x0000, -0x0324, -0x0647, -0x096a, -0x0c8b, -0x0fab, -0x12c8, -0x15e2,
	 -0x18f8, -0x1c0b, -0x1f19, -0x2223, -0x2528, -0x2826, -0x2b1f, -0x2e11,
	 -0x30fb, -0x33de, -0x36ba, -0x398c, -0x3c56, -0x3f17, -0x41ce, -0x447a,
	 -0x471c, -0x49b4, -0x4c3f, -0x4ebf, -0x5133, -0x539b, -0x55f5, -0x5842,
	 -0x5a82, -0x5cb4, -0x5ed7, -0x60ec, -0x62f2, -0x64e8, -0x66cf, -0x68a6,
	 -0x6a6d, -0x6c24, -0x6dca, -0x6f5f, -0x70e2, -0x7255, -0x73b5, -0x7504,
	 -0x7641, -0x776c, -0x7884, -0x798a, -0x7a7d, -0x7b5d, -0x7c29, -0x7ce3,
	 -0x7d8a, -0x7e1d, -0x7e9d, -0x7f09, -0x7f62, -0x7fa7, -0x7fd8, -0x7ff6,
	 -0x7fff, -0x7ff6, -0x7fd8, -0x7fa7, -0x7f62, -0x7f09, -0x7e9d, -0x7e1d,
	 -0x7d8a, -0x7ce3, -0x7c29, -0x7b5d, -0x7a7d, -0x798a, -0x7884, -0x776c,
	 -0x7641, -0x7504, -0x73b5, -0x7255, -0x70e2, -0x6f5f, -0x6dca, -0x6c24,
	 -0x6a6d, -0x68a6, -0x66cf, -0x64e8, -0x62f2, -0x60ec, -0x5ed7, -0x5cb4,
	 -0x5a82, -0x5842, -0x55f5, -0x539b, -0x5133, -0x4ebf, -0x4c3f, -0x49b4,
	 -0x471c, -0x447a, -0x41ce, -0x3f17, -0x3c56, -0x398c, -0x36ba, -0x33de,
	 -0x30fb, -0x2e11, -0x2b1f, -0x2826, -0x2528, -0x2223, -0x1f19, -0x1c0b,
	 -0x18f8, -0x15e2, -0x12c8, -0x0fab, -0x0c8b, -0x096a, -0x0647, -0x0324
};

#define INCR 2048
static double CosTable2[INCR];
static double SinTable2[INCR];

#define Angle(x) (((x)/(65536/INCR)) & (INCR-1))
#define Cos(x) ((double) CosTable2[x])
#define Sin(x) ((double) SinTable2[x])

// init DSP1 math tables
static void InitDSP1(void)
{
	UINT32 i;
	UINT8 *dspin = memory_region(REGION_USER6);

	dsp1_waitcmd = dsp1_first_parm = 1;

	for (i=0; i<INCR; i++)
	{
		CosTable2[i] = (cos((double)(2*M_PI*i/INCR)));
		SinTable2[i] = (sin((double)(2*M_PI*i/INCR)));
	}

	// expand the DSP-1 data ROM
	for (i = 0; i < 2048; i+=2)
	{
		DSP1ROM[i/2] = dspin[i]<<8 | dspin[i+1];
	}
}

static INT16 Gx, Nx, CentreX;
static INT16 Gy, Ny, CentreY;
static INT16 Gz, Nz, CentreZ;
static INT16 Les_C, Les_E, Les_G;

static INT16 VOffset;

static INT16 VPlane_C, VPlane_E;

// Azimuth and Zenith angles
static INT16 SinAas, CosAas, SinAzs, CosAzs;

// Clipped Zenith angle
static INT16 SinAzsB, CosAzsB;
static INT16 SecAZS_C1;
static INT16 SecAZS_E1;
static INT16 SecAZS_C2;
static INT16 SecAZS_E2;

static INT16 DSP1_Sin(INT16 Angle)
{
	int S;

	if (Angle < 0) {
		if (Angle == -32768) return 0;
		return -DSP1_Sin(-Angle);
	}

	S = DSP1_SinTable[Angle >> 8] + (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[0x40 + (Angle >> 8)] >> 15);
	if (S > 32767) S = 32767;
	return (INT16) S;
}

static INT16 DSP1_Cos(INT16 Angle)
{
	int S;

	if (Angle < 0) {
		if (Angle == -32768) return -32768;
		Angle = -Angle;
	}
	S = DSP1_SinTable[0x40 + (Angle >> 8)] - (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[Angle >> 8] >> 15);
	if (S < -32768) S = -32767;
	return (INT16) S;
}

static void DSP1_Normalize(INT16 m, INT16 *Coefficient, INT16 *Exponent)
{
	INT16 i = 0x4000;
	INT16 e = 0;

	if (m < 0)
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	else
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}

	if (e > 0)
		*Coefficient = m * DSP1ROM[0x0021 + e] << 1;
	else
		*Coefficient = m;

	*Exponent -= e;
}

static void DSP1_NormalizeDouble(int Product, INT16 *Coefficient, INT16 *Exponent)
{
	INT16 n = Product & 0x7fff;
	INT16 m = Product >> 15;
	INT16 i = 0x4000;
	INT16 e = 0;

	if (m < 0)
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	else
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}

	if (e > 0)
	{
		*Coefficient = m * DSP1ROM[0x0021 + e] << 1;

		if (e < 15)
			*Coefficient += n * DSP1ROM[0x0040 - e] >> 15;
		else
		{
			i = 0x4000;

			if (m < 0)
				while ((n & i) && i)
				{
					i >>= 1;
					e++;
				}
			else
				while (!(n & i) && i)
				{
					i >>= 1;
					e++;
				}

			if (e > 15)
				*Coefficient = n * DSP1ROM[0x0012 + e] << 1;
			else
				*Coefficient += n;
		}
	}
	else
		*Coefficient = m;

	*Exponent = e;
}

static INT16 DSP1_Truncate(INT16 C, INT16 E)
{
	if (E > 0) {
		if (C > 0) return 32767; else if (C < 0) return -32767;
	} else {
		if (E < 0) return C * DSP1ROM[0x0031 + E] >> 15;
	}
	return C;
}

// 00H - 16-bit Multiplication (Bit Perfect)
// Input    byte(00H) integer(Multiplicand) integer(Multiplier)
// Output   integer(Product)

static void DSP1_Multiply(INT16 Multiplicand, INT16 Multiplier, INT16 *Product)
{
	*Product = (Multiplicand * Multiplier >> 15);
}

// 20H - 16-bit Multiplication (Bit Perfect)
// Input    byte(20H) integer(Multiplicand) integer(Multiplier)
// Output   integer(Product)

#ifdef UNUSED_FUNCTION
static void DSP1_Multiply1(INT16 Multiplicand, INT16 Multiplier, INT16 *Product)
{
	*Product = (Multiplicand * Multiplier >> 15) + 1;
}
#endif

// 10H - Inverse Calculation (Bit Perfect)
// Input    byte(10H) integer(Coefficient) integer(Exponent)
// Output   integer(Coefficient) integer(Exponent)

static void DSP1_Inverse(INT16 Coefficient, INT16 Exponent, INT16 *iCoefficient, INT16 *iExponent)
{
	// Step One: Division by Zero
	if (Coefficient == 0x0000)
	{
		*iCoefficient = 0x7fff;
		*iExponent = 0x002f;
	}
	else
	{
		INT16 Sign = 1;

		// Step Two: Remove Sign
		if (Coefficient < 0)
		{
			if (Coefficient < -32767) Coefficient = -32767;
			Coefficient = -Coefficient;
			Sign = -1;
		}

		// Step Three: Normalize
		while (Coefficient <  0x4000)
		{
			Coefficient <<= 1;
			Exponent--;
		}

		// Step Four: Special Case
		if (Coefficient == 0x4000)
			if (Sign == 1) *iCoefficient = 0x7fff;
			else  {
				*iCoefficient = -0x4000;
				Exponent--;
			}
		else {
			// Step Five: Initial Guess
			INT16 i = DSP1ROM[((Coefficient - 0x4000) >> 7) + 0x0065];

			// Step Six: Iterate Newton's Method
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;

			*iCoefficient = i * Sign;
		}

		*iExponent = 1 - Exponent;
	}
}

// 04H - Trigonometric Calculation (Bit Perfect)
// Input    byte(04H) integer(Angle) integer(Radius)
// Output   integer(Sine) integer(Cosine)

static void DSP1_Triangle(INT16 Angle, INT16 Radius, INT16 *S, INT16 *C)
{
	*S = DSP1_Sin(Angle) * Radius >> 15;
	*C = DSP1_Cos(Angle) * Radius >> 15;
}

// 08H - Vector Size Calculation (Bit Perfect)
// Input    byte(08H) integer(X) integer(Y) integer(Z)
// Output   double(Radius)

static void DSP1_Radius(INT16 X, INT16 Y, INT16 Z, int *Radius)
{
	*Radius = (X * X + Y * Y + Z * Z) << 1;
}

// 18H - Vector Size Comparison (Bit Perfect)
// Input    byte(18H) integer(X) integer(Y) integer(Z) integer(Radius)
// Output   integer(Range)

static void DSP1_Range(INT16 X, INT16 Y, INT16 Z, INT16 Radius, INT16 *Range)
{
	*Range = ((X * X + Y * Y + Z * Z - Radius * Radius) >> 15);
}

// 38H - Vector Size Comparison (Bit Perfect)
// Input    byte(38H) integer(X) integer(Y) integer(Z) integer(Radius)
// Output   integer(Range)

#ifdef UNUSED_FUNCTION
static void DSP1_Range1(INT16 X, INT16 Y, INT16 Z, INT16 Radius, INT16 *Range)
{
	*Range = ((X * X + Y * Y + Z * Z - Radius * Radius) >> 15) + 1;
}
#endif

// 28H - Vector Absolute Value Calculation (Bit Perfect)
// Input    byte(28H) integer(X) integer(Y) integer(Z)
// Output   integer(Distance)

static void DSP1_Distance(INT16 X, INT16 Y, INT16 Z, INT16 *Distance)
{
	int Radius = X * X + Y * Y + Z * Z;
	INT16 C, E, Pos, Node1, Node2;

	if (Radius == 0) *Distance = 0;
	else
	{
		DSP1_NormalizeDouble(Radius, &C, &E);
		if (E & 1) C = C * 0x4000 >> 15;

		Pos = C * 0x0040 >> 15;

		Node1 = DSP1ROM[0x00d5 + Pos];
		Node2 = DSP1ROM[0x00d6 + Pos];

		*Distance = ((Node2 - Node1) * (C & 0x1ff) >> 9) + Node1;

#if DSP1_VERSION < 0x0102
		if (Pos & 1) *Distance -= (Node2 - Node1);
#endif
		*Distance >>= (E >> 1);
	}
}

// 0CH - 2D Coordinate Rotation (Bit Perfect)
// Input    byte(0CH) integer(Angle) integer(X) integer(Y)
// Output   integer(X) integer(Y)

static void DSP1_Rotate(INT16 Angle, INT16 X1, INT16 Y1, INT16 *X2, INT16 *Y2)
{
	*X2 = (Y1 * DSP1_Sin(Angle) >> 15) + (X1 * DSP1_Cos(Angle) >> 15);
	*Y2 = (Y1 * DSP1_Cos(Angle) >> 15) - (X1 * DSP1_Sin(Angle) >> 15);
}

// 1CH - 3D Coordinate Rotation (Bit Perfect)
// Input    byte(1CH) integer(Az) integer(Ay) integer(Ax) integer(X) integer(Y) integer(Z)
// Output   integer(X) integer(Y) integer(Z)

#ifdef UNUSED_FUNCTION
static void DSP1_Polar(INT16 Az, INT16 Ay, INT16 Ax, INT16 X1, INT16 Y1, INT16 Z1, INT16 *X2, INT16 *Y2, INT16 *Z2)
{
	INT16 X, Y, Z;

	// Rotate Around Z
	X = (Y1 * DSP1_Sin(Az) >> 15) + (X1 * DSP1_Cos(Az) >> 15);
	Y = (Y1 * DSP1_Cos(Az) >> 15) - (X1 * DSP1_Sin(Az) >> 15);
	X1 = X; Y1 = Y;

	// Rotate Around Y
	Z = (X1 * DSP1_Sin(Ay) >> 15) + (Z1 * DSP1_Cos(Ay) >> 15);
	X = (X1 * DSP1_Cos(Ay) >> 15) - (Z1 * DSP1_Sin(Ay) >> 15);
	*X2 = X; Z1 = Z;

	// Rotate Around X
	Y = (Z1 * DSP1_Sin(Ax) >> 15) + (Y1 * DSP1_Cos(Ax) >> 15);
	Z = (Z1 * DSP1_Cos(Ax) >> 15) - (Y1 * DSP1_Sin(Ax) >> 15);
	*Y2 = Y; *Z2 = Z;
}
#endif

// 02H - Projection Parameter Setting (Bit Perfect)
// Input    byte(02H) integer(Fx) integer(Fy) integer(Fz) integer(Lfe) integer(Les) integer(Aas) integer(Azs)
// Output   integer(Vof) integer(Vva) integer(Cx) integer(Cy)

static const INT16 MaxAZS_Exp[16] = {
	0x38b4, 0x38b7, 0x38ba, 0x38be, 0x38c0, 0x38c4, 0x38c7, 0x38ca,
	0x38ce, 0x38d0, 0x38d4, 0x38d7, 0x38da, 0x38dd, 0x38e0, 0x38e4
};

static INT16 FxParm, FyParm, FzParm, AasParm, AzsParm, LfeParm, LesParm;

static void DSP1_Parameter(INT16 Fx, INT16 Fy, INT16 Fz, INT16 Lfe, INT16 Les, INT16 Aas, INT16 Azs, INT16 *Vof, INT16 *Vva, INT16 *Cx, INT16 *Cy)
{
	INT16 CSec, C, E;

	// Copy Zenith angle for clipping
	INT16 AzsB = Azs;

	INT16 MaxAZS;

	// store off parameters for use by the Project operation
	FxParm = Fx;
	FyParm = Fy;
	FzParm = Fz;
	AasParm = Aas;
	AzsParm = Azs;
	LfeParm = Lfe;
	LesParm = Les;

	// Store Sine and Cosine of Azimuth and Zenith angle
	SinAas = DSP1_Sin(Aas);
	CosAas = DSP1_Cos(Aas);
	SinAzs = DSP1_Sin(Azs);
	CosAzs = DSP1_Cos(Azs);

	Nx = SinAzs * -SinAas >> 15;
	Ny = SinAzs * CosAas >> 15;
	Nz = CosAzs * 0x7fff >> 15;

	// Center of Projection
	CentreX = Fx + (Lfe * Nx >> 15);
	CentreY = Fy + (Lfe * Ny >> 15);
	CentreZ = Fz + (Lfe * Nz >> 15);

	Gx = CentreX - (Les * Nx >> 15);
	Gy = CentreY - (Les * Ny >> 15);
	Gz = CentreZ - (Les * Nz >> 15);

	Les_E = 0;
	DSP1_Normalize(Les, &Les_C, &Les_E);
	Les_G = Les;

	E = 0;
	DSP1_Normalize(CentreZ, &C, &E);

	VPlane_C = C;
	VPlane_E = E;

	// Determine clip boundary and clip Zenith angle if necessary
	MaxAZS = MaxAZS_Exp[-E];

	if (AzsB < 0) {
		MaxAZS = -MaxAZS;
		if (AzsB < MaxAZS + 1) AzsB = MaxAZS + 1;
	} else {
		if (AzsB > MaxAZS) AzsB = MaxAZS;
	}

	// Store Sine and Cosine of clipped Zenith angle
	SinAzsB = DSP1_Sin(AzsB);
	CosAzsB = DSP1_Cos(AzsB);

	DSP1_Inverse(CosAzsB, 0, &SecAZS_C1, &SecAZS_E1);
	DSP1_Normalize(C * SecAZS_C1 >> 15, &C, &E);
	E += SecAZS_E1;

	C = DSP1_Truncate(C, E) * SinAzsB >> 15;

	CentreX += C * SinAas >> 15;
	CentreY -= C * CosAas >> 15;

	*Cx = CentreX;
	*Cy = CentreY;

	// Raster number of imaginary center and horizontal line
	*Vof = 0;

	if ((Azs != AzsB) || (Azs == MaxAZS))
	{
		INT16 Aux;

		if (Azs == -32768) Azs = -32767;

		C = Azs - MaxAZS;
		if (C >= 0) C--;
		Aux = ~(C << 2);

		C = Aux * DSP1ROM[0x0328] >> 15;
		C = (C * Aux >> 15) + DSP1ROM[0x0327];
		*Vof -= (C * Aux >> 15) * Les >> 15;

		C = Aux * Aux >> 15;
		Aux = (C * DSP1ROM[0x0324] >> 15) + DSP1ROM[0x0325];
		CosAzsB += (C * Aux >> 15) * CosAzsB >> 15;
	}

	VOffset = Les * CosAzsB >> 15;

	DSP1_Inverse(SinAzsB, 0, &CSec, &E);
	DSP1_Normalize(VOffset, &C, &E);
	DSP1_Normalize(C * CSec >> 15, &C, &E);

	if (C == -32768) { C >>= 1; E++; }

	*Vva = DSP1_Truncate(-C, E);

	// Store Secant of clipped Zenith angle
	DSP1_Inverse(CosAzsB, 0, &SecAZS_C2, &SecAZS_E2);
}

// 06H - Object Projection Calculation
// Input    byte(06H) integer(X) integer(Y) integer(Z)
// Output   integer(H) integer(V) integer(M)

static void DSP1_Project(INT16 X, INT16 Y, INT16 Z, INT16 *H, INT16 *V, UINT16 *M)
{
	double Px, Py, Pz, Px1, Py1, Pz1, Px2, Py2, Pz2;
	INT32 dTan;

	Px = X - FxParm;
	Py = Y - FyParm;
	Pz = Z - FzParm;

	// rotate around the Z axis
	dTan = Angle(-AasParm+32768);
	Px1 = (Px*Cos(dTan)+Py*-Sin(dTan));
	Py1 = (Px*Sin(dTan)+Py*Cos(dTan));
	Pz1 = Pz;

	// rotate around the X axis
	dTan = Angle(-AzsParm);
	Px2 = Px1;
	Py2 = (Py1*Cos(dTan)+Pz1*-Sin(dTan));
	Pz2 = (Py1*Sin(dTan)+Pz1*Cos(dTan));

	Pz2 = Pz2-LfeParm;

	if (Pz2 < 0)
	{
		double dM;

		*H = (INT16)(-Px2*LesParm/-Pz2);
		*V = (INT16)(-Py2*LesParm/-Pz2);

		dM = (double)LesParm;
		dM *= 256.0;
		dM /= -Pz2;

		if (dM > 65535.0)
		{
			dM = 65535.0;
		}
		else if (dM < 0.0)
		{
			dM = 0.0;
		}

		*M = (UINT16)dM;
	}
	else
	{
		*H = 0;
		*V = 224;
		*M = 65535;
	}
}

// 0AH - Raster Data Calculation (Bit Perfect)
// Input    byte(0AH) integer(Vs)
// Output   integer(An) integer(Bn) integer(Cn) integer(Dn)

static void DSP1_Raster(INT16 Vs, INT16 *An, INT16 *Bn, INT16 *Cn, INT16 *Dn)
{
	INT16 C, E, C1, E1;

	DSP1_Inverse((Vs * SinAzs >> 15) + VOffset, 7, &C, &E);
	E += VPlane_E;

	C1 = C * VPlane_C >> 15;
	E1 = E + SecAZS_E2;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E);

	*An = C * CosAas >> 15;
	*Cn = C * SinAas >> 15;

	DSP1_Normalize(C1 * SecAZS_C2 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1);

	*Bn = C * -SinAas >> 15;
	*Dn = C * CosAas >> 15;
}

// 0EH - Coordinate Calculation of a Selected Point on the Screen (Bit Perfect)
// Input    byte(0EH) integer(H) integer(V)
// Output   integer(X) integer(Y)

static void DSP1_Target(INT16 H, INT16 V, INT16 *X, INT16 *Y)
{
	INT16 C, E, C1, E1;

	DSP1_Inverse((V * SinAzs >> 15) + VOffset, 8, &C, &E);
	E += VPlane_E;

	C1 = C * VPlane_C >> 15;
	E1 = E + SecAZS_E1;

	H <<= 8;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E) * H >> 15;

	*X = CentreX + (C * CosAas >> 15);
	*Y = CentreY - (C * SinAas >> 15);

	V <<= 8;

	DSP1_Normalize(C1 * SecAZS_C1 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1) * V >> 15;

	*X += C * -SinAas >> 15;
	*Y += C * CosAas >> 15;
}

// 01H - Set Attitude A (Bit Perfect)
// Input    byte(01H) integer(M) integer(Az) integer(Ay) integer(Ax)
// Output   None

static INT16 MatrixA[3][3];

static void DSP1_Attitude_A(INT16 M, INT16 Az, INT16 Ay, INT16 Ax)
{
	INT16 SinAz = DSP1_Sin(Az);
	INT16 CosAz = DSP1_Cos(Az);
	INT16 SinAy = DSP1_Sin(Ay);
	INT16 CosAy = DSP1_Cos(Ay);
	INT16 SinAx = DSP1_Sin(Ax);
	INT16 CosAx = DSP1_Cos(Ax);

	M >>= 1;

	MatrixA[0][0] = (M * CosAz >> 15) * CosAy >> 15;
	MatrixA[0][1] = -((M * SinAz >> 15) * CosAy >> 15);
	MatrixA[0][2] = M * SinAy >> 15;

	MatrixA[1][0] = ((M * SinAz >> 15) * CosAx >> 15) + (((M * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixA[1][1] = ((M * CosAz >> 15) * CosAx >> 15) - (((M * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixA[1][2] = -((M * SinAx >> 15) * CosAy >> 15);

	MatrixA[2][0] = ((M * SinAz >> 15) * SinAx >> 15) - (((M * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixA[2][1] = ((M * CosAz >> 15) * SinAx >> 15) + (((M * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixA[2][2] = (M * CosAx >> 15) * CosAy >> 15;
}

// 11H - Set Attitude B (Bit Perfect)
// Input    byte(11H) integer(M) integer(Az) integer(Ay) integer(Ax)
// Output   None

static INT16 MatrixB[3][3];

static void DSP1_Attitude_B(INT16 M, INT16 Az, INT16 Ay, INT16 Ax)
{
	INT16 SinAz = DSP1_Sin(Az);
	INT16 CosAz = DSP1_Cos(Az);
	INT16 SinAy = DSP1_Sin(Ay);
	INT16 CosAy = DSP1_Cos(Ay);
	INT16 SinAx = DSP1_Sin(Ax);
	INT16 CosAx = DSP1_Cos(Ax);

	M >>= 1;

	MatrixB[0][0] = (M * CosAz >> 15) * CosAy >> 15;
	MatrixB[0][1] = -((M * SinAz >> 15) * CosAy >> 15);
	MatrixB[0][2] = M * SinAy >> 15;

	MatrixB[1][0] = ((M * SinAz >> 15) * CosAx >> 15) + (((M * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixB[1][1] = ((M * CosAz >> 15) * CosAx >> 15) - (((M * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixB[1][2] = -((M * SinAx >> 15) * CosAy >> 15);

	MatrixB[2][0] = ((M * SinAz >> 15) * SinAx >> 15) - (((M * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixB[2][1] = ((M * CosAz >> 15) * SinAx >> 15) + (((M * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixB[2][2] = (M * CosAx >> 15) * CosAy >> 15;
}

// 21H - Set Attitude C (Bit Perfect)
// Input    byte(21H) integer(M) integer(Az) integer(Ay) integer(Ax)
// Output   None

static INT16 MatrixC[3][3];

#ifdef UNUSED_FUNCTION
static void DSP1_Attitude_C(INT16 M, INT16 Az, INT16 Ay, INT16 Ax)
{
	INT16 SinAz = DSP1_Sin(Az);
	INT16 CosAz = DSP1_Cos(Az);
	INT16 SinAy = DSP1_Sin(Ay);
	INT16 CosAy = DSP1_Cos(Ay);
	INT16 SinAx = DSP1_Sin(Ax);
	INT16 CosAx = DSP1_Cos(Ax);

	M >>= 1;

	MatrixC[0][0] = (M * CosAz >> 15) * CosAy >> 15;
	MatrixC[0][1] = -((M * SinAz >> 15) * CosAy >> 15);
	MatrixC[0][2] = M * SinAy >> 15;

	MatrixC[1][0] = ((M * SinAz >> 15) * CosAx >> 15) + (((M * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixC[1][1] = ((M * CosAz >> 15) * CosAx >> 15) - (((M * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	MatrixC[1][2] = -((M * SinAx >> 15) * CosAy >> 15);

	MatrixC[2][0] = ((M * SinAz >> 15) * SinAx >> 15) - (((M * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixC[2][1] = ((M * CosAz >> 15) * SinAx >> 15) + (((M * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	MatrixC[2][2] = (M * CosAx >> 15) * CosAy >> 15;
}
#endif

// 0DH - Convert from Global to Object Coordinates A (Bit Perfect)
// Input    byte(0DH) integer(X) integer(Y) integer(Z)
// Output   integer(F) integer(L) integer(U)

static void DSP1_Objective_A(INT16 X, INT16 Y, INT16 Z, INT16 *F, INT16 *L, INT16 *U)
{
	*F = (X * MatrixA[0][0] >> 15) + (Y * MatrixA[0][1] >> 15) + (Z * MatrixA[0][2] >> 15);
	*L = (X * MatrixA[1][0] >> 15) + (Y * MatrixA[1][1] >> 15) + (Z * MatrixA[1][2] >> 15);
	*U = (X * MatrixA[2][0] >> 15) + (Y * MatrixA[2][1] >> 15) + (Z * MatrixA[2][2] >> 15);
}

// 1DH - Convert from Global to Object Coordinates B (Bit Perfect)
// Input    byte(1DH) integer(X) integer(Y) integer(Z)
// Output   integer(F) integer(L) integer(U)

void DSP1_Objective_B(INT16 X, INT16 Y, INT16 Z, INT16 *F, INT16 *L, INT16 *U)
{
	*F = (X * MatrixB[0][0] >> 15) + (Y * MatrixB[0][1] >> 15) + (Z * MatrixB[0][2] >> 15);
	*L = (X * MatrixB[1][0] >> 15) + (Y * MatrixB[1][1] >> 15) + (Z * MatrixB[1][2] >> 15);
	*U = (X * MatrixB[2][0] >> 15) + (Y * MatrixB[2][1] >> 15) + (Z * MatrixB[2][2] >> 15);
}

// 2DH - Convert from Global to Object Coordinates C (Bit Perfect)
// Input    byte(2DH) integer(X) integer(Y) integer(Z)
// Output   integer(F) integer(L) integer(U)

static void DSP1_Objective_C(INT16 X, INT16 Y, INT16 Z, INT16 *F, INT16 *L, INT16 *U)
{
	*F = (X * MatrixC[0][0] >> 15) + (Y * MatrixC[0][1] >> 15) + (Z * MatrixC[0][2] >> 15);
	*L = (X * MatrixC[1][0] >> 15) + (Y * MatrixC[1][1] >> 15) + (Z * MatrixC[1][2] >> 15);
	*U = (X * MatrixC[2][0] >> 15) + (Y * MatrixC[2][1] >> 15) + (Z * MatrixC[2][2] >> 15);
}

// 03H - Conversion from Object to Global Coordinates A (Bit Perfect)
// Input    byte(03H) integer(F) integer(L) integer(U)
// Output   integer(X) integer(Y) integer(Z)

static void DSP1_Subjective_A(INT16 F, INT16 L, INT16 U, INT16 *X, INT16 *Y, INT16 *Z)
{
	*X = (F * MatrixA[0][0] >> 15) + (L * MatrixA[1][0] >> 15) + (U * MatrixA[2][0] >> 15);
	*Y = (F * MatrixA[0][1] >> 15) + (L * MatrixA[1][1] >> 15) + (U * MatrixA[2][1] >> 15);
	*Z = (F * MatrixA[0][2] >> 15) + (L * MatrixA[1][2] >> 15) + (U * MatrixA[2][2] >> 15);
}

// 13H - Conversion from Object to Global Coordinates B (Bit Perfect)
// Input    byte(13H) integer(F) integer(L) integer(U)
// Output   integer(X) integer(Y) integer(Z)

static void DSP1_Subjective_B(INT16 F, INT16 L, INT16 U, INT16 *X, INT16 *Y, INT16 *Z)
{
	*X = (F * MatrixB[0][0] >> 15) + (L * MatrixB[1][0] >> 15) + (U * MatrixB[2][0] >> 15);
	*Y = (F * MatrixB[0][1] >> 15) + (L * MatrixB[1][1] >> 15) + (U * MatrixB[2][1] >> 15);
	*Z = (F * MatrixB[0][2] >> 15) + (L * MatrixB[1][2] >> 15) + (U * MatrixB[2][2] >> 15);
}

// 23H - Conversion from Object to Global Coordinates C (Bit Perfect)
// Input    byte(23H) integer(F) integer(L) integer(U)
// Output   integer(X) integer(Y) integer(Z)

static void DSP1_Subjective_C(INT16 F, INT16 L, INT16 U, INT16 *X, INT16 *Y, INT16 *Z)
{
	*X = (F * MatrixC[0][0] >> 15) + (L * MatrixC[1][0] >> 15) + (U * MatrixC[2][0] >> 15);
	*Y = (F * MatrixC[0][1] >> 15) + (L * MatrixC[1][1] >> 15) + (U * MatrixC[2][1] >> 15);
	*Z = (F * MatrixC[0][2] >> 15) + (L * MatrixC[1][2] >> 15) + (U * MatrixC[2][2] >> 15);
}

// 0BH - Calculation of Inner Product with Forward Attitude A and a Vector (Bit Perfect)
// Input    byte(0BH) integer(X) integer(Y) integer(Z)
// Output   integer(S)

static void DSP1_Scalar_A(INT16 X, INT16 Y, INT16 Z, INT16 *S)
{
	*S = ((X * MatrixA[0][0]) + (Y * MatrixA[0][1]) + (Z * MatrixA[0][2])) >> 15;
}

// 1BH - Calculation of Inner Product with Forward Attitude B and a Vector (Bit Perfect)
// Input    byte(1BH) integer(X) integer(Y) integer(Z)
// Output   integer(S)

static void DSP1_Scalar_B(INT16 X, INT16 Y, INT16 Z, INT16 *S)
{
	*S = ((X * MatrixB[0][0]) + (Y * MatrixB[0][1]) + (Z * MatrixB[0][2])) >> 15;
}

// 2BH - Calculation of Inner Product with Forward Attitude C and a Vector (Bit Perfect)
// Input    byte(2BH) integer(X) integer(Y) integer(Z)
// Output   integer(S)

static void DSP1_Scalar_C(INT16 X, INT16 Y, INT16 Z, INT16 *S)
{
	*S = ((X * MatrixC[0][0]) + (Y * MatrixC[0][1]) + (Z * MatrixC[0][2])) >> 15;
}

// 14H - 3D Angle Rotation (Bit Perfect)
// Input    byte(14H) integer(Az) integer(Ax) integer(Ay) integer(U) integer(F) integer(L)
// Output   integer(Rz) integer(Rx) integer(Ry)

static void DSP1_Gyrate(INT16 Az, INT16 Ax, INT16 Ay, INT16 U, INT16 F, INT16 L, INT16 *Rz, INT16 *Rx, INT16 *Ry)
{
	INT16 CSec, ESec, CSin, C, E;

	DSP1_Inverse(DSP1_Cos(Ax), 0, &CSec, &ESec);

	// Rotation Around Z
	DSP1_NormalizeDouble(U * DSP1_Cos(Ay) - F * DSP1_Sin(Ay), &C, &E);

	E = ESec - E;

	DSP1_Normalize(C * CSec >> 15, &C, &E);

	*Rz = Az + DSP1_Truncate(C, E);

	// Rotation Around X
	*Rx = Ax + (U * DSP1_Sin(Ay) >> 15) + (F * DSP1_Cos(Ay) >> 15);

	// Rotation Around Y
	DSP1_NormalizeDouble(U * DSP1_Cos(Ay) + F * DSP1_Sin(Ay), &C, &E);

	E = ESec - E;

	DSP1_Normalize(DSP1_Sin(Ax), &CSin, &E);

	DSP1_Normalize(-(C * (CSec * CSin >> 15) >> 15), &C, &E);

	*Ry = Ay + DSP1_Truncate(C, E) + L;
}

// 0FH - Memory Test (Bit Perfect)
// Input    byte(0FH) integer(Size)
// Output   integer(Result)

static void DSP1_MemoryTest(INT16 Size, INT16 *Result)
{
	*Result = 0x0000;
}

// 2FH - Memory Size Calculation (Bit Perfect)
// Input    byte(2FH) integer(Undefined)
// Output   integer(Size)

static void DSP1_MemorySize(INT16 *Size)
{
	*Size = 0x0100;
}

// ********************************************************************************************
// MAME/MESS interface for above DSP1 code
// ********************************************************************************************

// DSP1 interface
static UINT8 dsp1_read(UINT16 address)
{
	// check data vs. status
	if (((address & 0xf000) == 0x6000) || ((address & 0x7fff) < 0x4000))
	{
		// data reg
		if (dsp1_out_cnt)
		{
			UINT8 temp = (UINT8)dsp1_out[dsp1_out_idx++];
			if (--dsp1_out_cnt == 0)
			{
				if ((dsp1_cur_cmd == 0x1a) || (dsp1_cur_cmd == 0x0a))
				{
					INT16 tr, tr1, tr2, tr3;

					DSP1_Raster(dsp1_in[0]|dsp1_in[1]<<8, &tr, &tr1, &tr2, &tr3);
					dsp1_in[0]++;
					if (dsp1_in[0] == 0) dsp1_in[1]++;
					dsp1_out_cnt = 8;
					dsp1_out[0] = tr&0xff;
					dsp1_out[1] = (tr>>8)&0xff;
					dsp1_out[2] = tr1&0xff;
					dsp1_out[3] = (tr1>>8)&0xff;
					dsp1_out[4] = tr2&0xff;
					dsp1_out[5] = (tr2>>8)&0xff;
					dsp1_out[6] = tr3&0xff;
					dsp1_out[7] = (tr3>>8)&0xff;
					dsp1_out_idx = 0;
				}
				else if (dsp1_cur_cmd == 0x1f)
				{
					if ((dsp1_out_idx % 2) != 0)
					{
						return DSP1ROM[dsp1_out_idx>>1] & 0xff;
					}
					else
					{
						return (DSP1ROM[dsp1_out_idx>>1] >> 8) & 0xff;
					}
				}
			}
			dsp1_waitcmd = 1;
//          printf("dsp_r: %02x\n", temp);
			return temp;
		}
		else
		{
//          printf("dsp_r: %02x\n", 0xff);
			return 0xff;	// indicate "no data"
		}
	}

	// status register
//  printf("dsp_r: %02x\n", 0x80);
	return 0x80;
}

static void dsp1_write(UINT16 address, UINT8 data)
{
	// check data vs. status
	if (((address & 0xf000) == 0x6000) || ((address & 0x7fff) < 0x4000))
	{
//      printf("DSP_w: %02x cmd %02x wait %d dsp1_in_cnt %d\n", data, dsp1_cur_cmd, dsp1_waitcmd, dsp1_in_cnt);

		if (((dsp1_cur_cmd == 0x0a) || (dsp1_cur_cmd == 0x1a)) && (dsp1_out_cnt != 0))
		{
			dsp1_out_cnt--;
			dsp1_out_idx++;
			return;
		}
		else if (dsp1_waitcmd)
		{
			dsp1_cur_cmd = data;
			dsp1_in_idx = 0;
			dsp1_waitcmd = 0;
			dsp1_first_parm = 1;
			switch (data)
			{
				case 0x00: dsp1_in_cnt = 2; break;
				case 0x10: case 0x30: dsp1_in_cnt = 2; break;
				case 0x20: dsp1_in_cnt = 2; break;
				case 0x04: case 0x24: dsp1_in_cnt = 2; break;
				case 0x08: dsp1_in_cnt = 3; break;
				case 0x18: dsp1_in_cnt = 4; break;
				case 0x28: dsp1_in_cnt = 3; break;
				case 0x38: dsp1_in_cnt = 4; break;
				case 0x0c: case 0x2c: dsp1_in_cnt = 3; break;
				case 0x1c: case 0x3c: dsp1_in_cnt = 6; break;
				case 0x02: case 0x12: case 0x22: case 0x32: dsp1_in_cnt = 7; break;
				case 0x0a: dsp1_in_cnt = 1; break;
				case 0x1a: case 0x2a: case 0x3a:  dsp1_cur_cmd = 0x1a; dsp1_in_cnt = 1; break;
				case 0x06: case 0x16: case 0x26: case 0x36: dsp1_in_cnt = 3; break;
				case 0x0e: case 0x1e: case 0x2e: case 0x3e: dsp1_in_cnt = 2; break;
				case 0x01: case 0x05: case 0x31: case 0x35: dsp1_in_cnt = 4; break;
				case 0x11: case 0x15: dsp1_in_cnt = 4; break;
				case 0x21: case 0x25: dsp1_in_cnt = 4; break;
				case 0x0d: case 0x3d: case 0x09: case 0x39: dsp1_in_cnt = 3; break;
				case 0x19: case 0x1d: dsp1_in_cnt = 3; break;
				case 0x29: case 0x2d: dsp1_in_cnt = 3;	break;
				case 0x03: case 0x33: dsp1_in_cnt = 3;	break;
				case 0x13: dsp1_in_cnt = 3;	break;
				case 0x23: dsp1_in_cnt = 3;	break;
				case 0x0b: case 0x3b: dsp1_in_cnt = 3;	break;
				case 0x1b: dsp1_in_cnt = 3;	break;
				case 0x2b: dsp1_in_cnt = 3;	break;
				case 0x14: case 0x34: dsp1_in_cnt = 6;	break;
				case 0x07: case 0x0f: dsp1_in_cnt = 1;	break;
				case 0x27: case 0x2F: dsp1_in_cnt=1; break;
				case 0x17: case 0x37: case 0x3F: dsp1_cur_cmd = 0x1f; dsp1_in_cnt = 1; break;
				case 0x1f: dsp1_in_cnt = 1;	break;

				case 0x80:
					dsp1_in_cnt = 0;
					dsp1_waitcmd = 1;
					dsp1_first_parm = 1;
					break;
			}

			// that gives us parameter lengths in words, convert to bytes
			dsp1_in_cnt *= 2;
		}
		else
		{
			dsp1_in[dsp1_in_idx++] = data;
			dsp1_first_parm = 0;
		}

		if ((dsp1_waitcmd) || ((dsp1_first_parm) && (data == 0x80)))
		{
			dsp1_waitcmd = 1;
			dsp1_first_parm = 0;
		}
		else if (((dsp1_first_parm) && (dsp1_in_cnt != 0)) || ((dsp1_in_cnt == 0) && (dsp1_in_idx == 0)))
		{
		}
		else
		{
			if (dsp1_in_cnt)
			{
				if (--dsp1_in_cnt == 0)
				{
					INT16 tr = 0, tr1 = 0, tr2 = 0, tr3 = 0;
					INT32 ltr;

					// time to run the command
					dsp1_waitcmd = 1;
					dsp1_out_idx = 0;
					switch (dsp1_cur_cmd)
					{
						case 0x00:
							DSP1_Multiply(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x01:
						case 0x05:
						case 0x31:
						case 0x35:
							DSP1_Attitude_A(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, dsp1_in[6]|dsp1_in[7]<<8);
							dsp1_out_cnt = 0;
							break;

						case 0x02:
						case 0x12:
						case 0x22:
						case 0x32:
							DSP1_Parameter(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, dsp1_in[6]|dsp1_in[7]<<8,
								       dsp1_in[8]|dsp1_in[9]<<8, dsp1_in[10]|dsp1_in[11]<<8, dsp1_in[12]|dsp1_in[13]<<8, &tr, &tr1, &tr2, &tr3);
							dsp1_out_cnt = 8;
							break;

						case 0x03:
						case 0x33:
							DSP1_Subjective_A(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x04:
						case 0x24:
							DSP1_Triangle(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, &tr, &tr1);
							dsp1_out_cnt = 4;
							break;

						case 0x06:
						case 0x16:
						case 0x26:
						case 0x36:
							DSP1_Project(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, (UINT16 *) &tr2);
						       	dsp1_out_cnt = 6;
							break;

						case 0x0a:
						case 0x1a:
						case 0x2a:
						case 0x3a:
							DSP1_Raster(dsp1_in[0]|dsp1_in[1]<<8, &tr, &tr1, &tr2, &tr3);
							dsp1_in[0]++;
							if (dsp1_in[0] == 0) dsp1_in[1]++;
							dsp1_out_cnt = 8;
							dsp1_in_idx = 0;
							break;

						case 0x08:
							DSP1_Radius(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &ltr);
							tr = (ltr & 0xffff);
							tr1 = (ltr >> 16) & 0xffff;
							dsp1_out_cnt = 4;
							break;

						case 0x09:
						case 0x0d:
						case 0x39:
						case 0x3d:
							DSP1_Objective_A(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x0b:
						case 0x3b:
							DSP1_Scalar_A(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x0c:
						case 0x2c:
							DSP1_Rotate(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1);
							dsp1_out_cnt = 4;
							break;

						case 0x0e:
						case 0x1e:
						case 0x2e:
						case 0x3e:
							DSP1_Target(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, &tr, &tr1);
							dsp1_out_cnt = 4;
							break;

						case 0x07:
						case 0x0f:
							DSP1_MemoryTest(dsp1_in[0]|dsp1_in[1]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x10:
						case 0x30:
							DSP1_Inverse(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, &tr, &tr1);
							dsp1_out_cnt = 4;
							break;

						case 0x11:
						case 0x15:
							DSP1_Attitude_B(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, dsp1_in[6]|dsp1_in[7]<<8);
							dsp1_out_cnt = 0;
							break;

						case 0x13:
							DSP1_Subjective_B(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x14:
						case 0x34:
							DSP1_Gyrate(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, dsp1_in[6]|dsp1_in[7]<<8,
								       dsp1_in[8]|dsp1_in[9]<<8, dsp1_in[10]|dsp1_in[11]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x18:
							DSP1_Range(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, dsp1_in[6]|dsp1_in[7]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x19:
						case 0x1d:
							DSP1_Objective_B(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x1b:
							DSP1_Scalar_B(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x1f:
							dsp1_out_cnt = 2048;
							break;

						case 0x23:
							DSP1_Subjective_C(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x28:
							DSP1_Distance(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x29:
						case 0x2d:
							DSP1_Objective_C(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr, &tr1, &tr2);
							dsp1_out_cnt = 6;
							break;

						case 0x2b:
							DSP1_Scalar_C(dsp1_in[0]|dsp1_in[1]<<8, dsp1_in[2]|dsp1_in[3]<<8, dsp1_in[4]|dsp1_in[5]<<8, &tr);
							dsp1_out_cnt = 2;
							break;

						case 0x27:
						case 0x2f:
							DSP1_MemorySize(&tr);
							dsp1_out_cnt = 2;
							break;

						default:
							printf("Unhandled DSP1 command: %02x\n", dsp1_cur_cmd);
							break;

					}

					// copy up to 8 bytes of result into the buffer
					dsp1_out[0] = tr&0xff;
					dsp1_out[1] = (tr>>8)&0xff;
					dsp1_out[2] = tr1&0xff;
					dsp1_out[3] = (tr1>>8)&0xff;
					dsp1_out[4] = tr2&0xff;
					dsp1_out[5] = (tr2>>8)&0xff;
					dsp1_out[6] = tr3&0xff;
					dsp1_out[7] = (tr3>>8)&0xff;
				}
			}
		}
	}
}

