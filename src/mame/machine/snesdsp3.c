/************************************************************************

    DSP-3 emulator code

    Copyright (c) 2003-2009 John Weidman, Kris Bleakley, Lancer, Nach,
    z80 gaiden, and Jonas Quinn

    This code is released by ZSNES Team under GNU General Public License
    version 2 as published by the Free Software Foundation.
    The implementation below is released under the MAME license for use
    in MAME, MESS and derivatives by permission of the authors.

************************************************************************/

static UINT16 DSP3_DataROM[1024];

static void(*SetDSP3)(void);
static void DSP3_Command(void);

static UINT16 DSP3_DR;
static UINT16 DSP3_SR;
static UINT16 DSP3_MemoryIndex;

static void DSP3_Reset(void)
{
	DSP3_DR = 0x0080;
	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_Command;
}

#ifdef UNUSED_FUNCTION
void DSP3_MemorySize(void)
{
	DSP3_DR = 0x0300;
	SetDSP3 = &DSP3_Reset;
}
#endif

static void DSP3_TestMemory(void)
{
	DSP3_DR = 0x0000;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_DumpDataROM(void)
{
	DSP3_DR = DSP3_DataROM[DSP3_MemoryIndex++];
	if (DSP3_MemoryIndex == 1024)
		SetDSP3 = &DSP3_Reset;
}

static void DSP3_MemoryDump(void)
{
	DSP3_MemoryIndex = 0;
	SetDSP3 = &DSP3_DumpDataROM;
	DSP3_DumpDataROM();
}

static INT16 DSP3_WinLo;
static INT16 DSP3_WinHi;

static void DSP3_OP06(void)
{
	DSP3_WinLo = (UINT8)(DSP3_DR);
	DSP3_WinHi = (UINT8)(DSP3_DR >> 8);
	DSP3_Reset();
}

static void DSP3_OP03(void)
{
	INT16 Lo = (UINT8)(DSP3_DR);
	INT16 Hi = (UINT8)(DSP3_DR >> 8);
	INT16 Ofs = (DSP3_WinLo * Hi << 1) + (Lo << 1);
	DSP3_DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static INT16 DSP3_AddLo;
static INT16 DSP3_AddHi;

static void DSP3_OP07_B(void)
{
	INT16 Ofs = (DSP3_WinLo * DSP3_AddHi << 1) + (DSP3_AddLo << 1);
	DSP3_DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP07_A(void)
{
	INT16 Lo = (UINT8)(DSP3_DR);
	INT16 Hi = (UINT8)(DSP3_DR >> 8);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	if (DSP3_AddLo < 0)
		DSP3_AddLo += DSP3_WinLo;
	else
		if (DSP3_AddLo >= DSP3_WinLo)
			DSP3_AddLo -= DSP3_WinLo;

	if (DSP3_AddHi < 0)
		DSP3_AddHi += DSP3_WinHi;
	else
		if (DSP3_AddHi >= DSP3_WinHi)
			DSP3_AddHi -= DSP3_WinHi;

	DSP3_DR = DSP3_AddLo | (DSP3_AddHi << 8) | ((DSP3_AddHi >> 8) & 0xff);
	SetDSP3 = &DSP3_OP07_B;
}

static void DSP3_OP07(void)
{
	UINT32 dataOfs = ((DSP3_DR << 1) + 0x03b2) & 0x03ff;

	DSP3_AddHi = DSP3_DataROM[dataOfs];
	DSP3_AddLo = DSP3_DataROM[dataOfs + 1];

	SetDSP3 = &DSP3_OP07_A;
	DSP3_SR = 0x0080;
}

static UINT16 DSP3_Codewords;
static UINT16 DSP3_Outwords;
static UINT16 DSP3_Symbol;
static UINT16 DSP3_BitCount;
static UINT16 DSP3_Index;
static UINT16 DSP3_Codes[512];
static UINT16 DSP3_BitsLeft;
static UINT16 DSP3_ReqBits;
static UINT16 DSP3_ReqData;
static UINT16 DSP3_BitCommand;
static UINT8  DSP3_BaseLength;
static UINT16 DSP3_BaseCodes;
static UINT16 DSP3_BaseCode;
static UINT8  DSP3_CodeLengths[8];
static UINT16 DSP3_CodeOffsets[8];
static UINT16 DSP3_LZCode;
static UINT8  DSP3_LZLength;

static UINT16 DSP3_X;
static UINT16 DSP3_Y;

static void DSP3_Coordinate(void)
{
	DSP3_Index++;

	switch (DSP3_Index)
	{
	case 3:
		{
			if (DSP3_DR == 0xffff)
				DSP3_Reset();
			break;
		}
	case 4:
		{
			DSP3_X = DSP3_DR;
			break;
		}
	case 5:
		{
			DSP3_Y = DSP3_DR;
			DSP3_DR = 1;
			break;
		}
	case 6:
		{
			DSP3_DR = DSP3_X;
			break;
		}
	case 7:
		{
			DSP3_DR = DSP3_Y;
			DSP3_Index = 0;
			break;
		}
	}
}

static UINT8  DSP3_Bitmap[8];
static UINT8  DSP3_Bitplane[8];
static UINT16 DSP3_BMIndex;
static UINT16 DSP3_BPIndex;
static UINT16 DSP3_Count;

static void DSP3_Convert_A(void)
{
	if (DSP3_BMIndex < 8)
	{
		DSP3_Bitmap[DSP3_BMIndex++] = (UINT8) (DSP3_DR);
		DSP3_Bitmap[DSP3_BMIndex++] = (UINT8) (DSP3_DR >> 8);

		if (DSP3_BMIndex == 8)
		{
      short i, j;
			for (i=0; i < 8; i++)
				for (j=0; j < 8; j++)
				{
					DSP3_Bitplane[j] <<= 1;
					DSP3_Bitplane[j] |= (DSP3_Bitmap[i] >> j) & 1;
				}

			DSP3_BPIndex = 0;
			DSP3_Count--;
		}
	}

	if (DSP3_BMIndex == 8)
	{
		if (DSP3_BPIndex == 8)
		{
			if (!DSP3_Count) DSP3_Reset();
			DSP3_BMIndex = 0;
		}
		else
		{
			DSP3_DR = DSP3_Bitplane[DSP3_BPIndex++];
			DSP3_DR |= DSP3_Bitplane[DSP3_BPIndex++] << 8;
		}
	}
}

static void DSP3_Convert(void)
{
	DSP3_Count = DSP3_DR;
	DSP3_BMIndex = 0;
	SetDSP3 = &DSP3_Convert_A;
}

static UINT8 DSP3_GetBits(UINT8 Count)
{
	if (!DSP3_BitsLeft)
	{
		DSP3_BitsLeft = Count;
		DSP3_ReqBits = 0;
	}

	do {
		if (!DSP3_BitCount)
		{
			DSP3_SR = 0xC0;
			return 0;
		}

		DSP3_ReqBits <<= 1;
		if (DSP3_ReqData & 0x8000) DSP3_ReqBits++;
		DSP3_ReqData <<= 1;

		DSP3_BitCount--;
		DSP3_BitsLeft--;

	} while (DSP3_BitsLeft);

	return 1;
}

static void DSP3_Decode_Data(void)
{
	if (!DSP3_BitCount)
	{
		if (DSP3_SR & 0x40)
		{
			DSP3_ReqData = DSP3_DR;
			DSP3_BitCount += 16;
		}
		else
		{
			DSP3_SR = 0xC0;
			return;
		}
	}

	if (DSP3_LZCode == 1)
	{
		if (!DSP3_GetBits(1))
			return;

		if (DSP3_ReqBits)
			DSP3_LZLength = 12;
		else
			DSP3_LZLength = 8;

		DSP3_LZCode++;
	}

	if (DSP3_LZCode == 2)
	{
		if (!DSP3_GetBits(DSP3_LZLength))
			return;

		DSP3_LZCode = 0;
		DSP3_Outwords--;
		if (!DSP3_Outwords) SetDSP3 = &DSP3_Reset;

		DSP3_SR = 0x80;
		DSP3_DR = DSP3_ReqBits;
		return;
	}

	if (DSP3_BaseCode == 0xffff)
	{
		if (!DSP3_GetBits(DSP3_BaseLength))
			return;

		DSP3_BaseCode = DSP3_ReqBits;
	}

	if (!DSP3_GetBits(DSP3_CodeLengths[DSP3_BaseCode]))
		return;

	DSP3_Symbol = DSP3_Codes[DSP3_CodeOffsets[DSP3_BaseCode] + DSP3_ReqBits];
	DSP3_BaseCode = 0xffff;

	if (DSP3_Symbol & 0xff00)
	{
		DSP3_Symbol += 0x7f02;
		DSP3_LZCode++;
	}
	else
	{
		DSP3_Outwords--;
		if (!DSP3_Outwords)
			SetDSP3 = &DSP3_Reset;
	}

	DSP3_SR = 0x80;
	DSP3_DR = DSP3_Symbol;
}

static void DSP3_Decode_Tree(void)
{
	if (!DSP3_BitCount)
	{
		DSP3_ReqData = DSP3_DR;
		DSP3_BitCount += 16;
	}

	if (!DSP3_BaseCodes)
	{
		DSP3_GetBits(1);
		if (DSP3_ReqBits)
		{
			DSP3_BaseLength = 3;
			DSP3_BaseCodes = 8;
		}
		else
		{
			DSP3_BaseLength = 2;
			DSP3_BaseCodes = 4;
		}
	}

	while (DSP3_BaseCodes)
	{
		if (!DSP3_GetBits(3))
			return;

		DSP3_ReqBits++;

		DSP3_CodeLengths[DSP3_Index] = (UINT8) DSP3_ReqBits;
		DSP3_CodeOffsets[DSP3_Index] = DSP3_Symbol;
		DSP3_Index++;

		DSP3_Symbol += 1 << DSP3_ReqBits;
		DSP3_BaseCodes--;
	}

	DSP3_BaseCode = 0xffff;
	DSP3_LZCode = 0;

	SetDSP3 = &DSP3_Decode_Data;
	if (DSP3_BitCount) DSP3_Decode_Data();
}

static void DSP3_Decode_Symbols(void)
{
	DSP3_ReqData = DSP3_DR;
	DSP3_BitCount += 16;

	do {

		if (DSP3_BitCommand == 0xffff)
		{
			if (!DSP3_GetBits(2)) return;
			DSP3_BitCommand = DSP3_ReqBits;
		}

		switch (DSP3_BitCommand)
		{
		case 0:
			{
				if (!DSP3_GetBits(9)) return;
				DSP3_Symbol = DSP3_ReqBits;
				break;
			}
		case 1:
			{
				DSP3_Symbol++;
				break;
			}
		case 2:
			{
				if (!DSP3_GetBits(1)) return;
				DSP3_Symbol += 2 + DSP3_ReqBits;
				break;
			}
		case 3:
			{
				if (!DSP3_GetBits(4)) return;
				DSP3_Symbol += 4 + DSP3_ReqBits;
				break;
			}
		}

		DSP3_BitCommand = 0xffff;

		DSP3_Codes[DSP3_Index++] = DSP3_Symbol;
		DSP3_Codewords--;

	} while (DSP3_Codewords);

	DSP3_Index = 0;
	DSP3_Symbol = 0;
	DSP3_BaseCodes = 0;

	SetDSP3 = &DSP3_Decode_Tree;
	if (DSP3_BitCount) DSP3_Decode_Tree();
}

static void DSP3_Decode_A(void)
{
	DSP3_Outwords = DSP3_DR;
	SetDSP3 = &DSP3_Decode_Symbols;
	DSP3_BitCount = 0;
	DSP3_BitsLeft = 0;
	DSP3_Symbol = 0;
	DSP3_Index = 0;
	DSP3_BitCommand = 0xffff;
	DSP3_SR = 0xC0;
}

static void DSP3_Decode(void)
{
	DSP3_Codewords = DSP3_DR;
	SetDSP3 = &DSP3_Decode_A;
}


// Opcodes 1E/3E bit-perfect to 'dsp3-intro' log
// src: adapted from SD Gundam X/G-Next

static INT16 op3e_x;
static INT16 op3e_y;

static INT16 op1e_terrain[0x2000];
static INT16 op1e_cost[0x2000];
static INT16 op1e_weight[0x2000];

static INT16 op1e_cell;
static INT16 op1e_turn;
static INT16 op1e_search;

static INT16 op1e_x;
static INT16 op1e_y;

static INT16 op1e_min_radius;
static INT16 op1e_max_radius;

static INT16 op1e_max_search_radius;
static INT16 op1e_max_path_radius;

static INT16 op1e_lcv_radius;
static INT16 op1e_lcv_steps;
static INT16 op1e_lcv_turns;

static void DSP3_OP3E(void)
{
	op3e_x = (UINT8)(DSP3_DR & 0x00ff);
	op3e_y = (UINT8)((DSP3_DR & 0xff00)>>8);

	DSP3_OP03();

	op1e_terrain[ DSP3_DR ] = 0x00;
	op1e_cost[ DSP3_DR ] = 0xff;
	op1e_weight[ DSP3_DR ] = 0;

	op1e_max_search_radius = 0;
	op1e_max_path_radius = 0;
}

static void DSP3_OP1E_A(void);
static void DSP3_OP1E_A1(void);
static void DSP3_OP1E_A2(void);
static void DSP3_OP1E_A3(void);

static void DSP3_OP1E_B(void);
static void DSP3_OP1E_B1(void);
static void DSP3_OP1E_B2(void);

static void DSP3_OP1E_C(void);
static void DSP3_OP1E_C1(void);
static void DSP3_OP1E_C2(void);

static void DSP3_OP1E_D( INT16, INT16 *, INT16 * );
static void DSP3_OP1E_D1( INT16 move, INT16 *lo, INT16 *hi );

static void DSP3_OP1E(void)
{
	int lcv;

	op1e_min_radius = (UINT8)(DSP3_DR & 0x00ff);
	op1e_max_radius = (UINT8)((DSP3_DR & 0xff00)>>8);

	if( op1e_min_radius == 0 )
		op1e_min_radius++;

	if( op1e_max_search_radius >= op1e_min_radius )
		op1e_min_radius = op1e_max_search_radius+1;

	if( op1e_max_radius > op1e_max_search_radius )
		op1e_max_search_radius = op1e_max_radius;

	op1e_lcv_radius = op1e_min_radius;
	op1e_lcv_steps = op1e_min_radius;

	op1e_lcv_turns = 6;
	op1e_turn = 0;

	op1e_x = op3e_x;
	op1e_y = op3e_y;

	for( lcv = 0; lcv < op1e_min_radius; lcv++ )
		DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );

	DSP3_OP1E_A();
}

static void DSP3_OP1E_A(void)
{
	int lcv;

	if( op1e_lcv_steps == 0 ) {
		op1e_lcv_radius++;

		op1e_lcv_steps = op1e_lcv_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_lcv_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_radius > op1e_max_radius ) {
		op1e_turn++;
		op1e_lcv_turns--;

		op1e_lcv_radius = op1e_min_radius;
		op1e_lcv_steps = op1e_min_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_min_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_turns == 0 ) {
		DSP3_DR = 0xffff;
		DSP3_SR = 0x0080;
		SetDSP3 = &DSP3_OP1E_B;
		return;
	}

	DSP3_DR = (UINT8)(op1e_x) | ((UINT8)(op1e_y)<<8);
	DSP3_OP03();

	op1e_cell = DSP3_DR;

	DSP3_SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_A1;
}

static void DSP3_OP1E_A1(void)
{
	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A2;
}

static void DSP3_OP1E_A2(void)
{
	op1e_terrain[ op1e_cell ] = (UINT8)(DSP3_DR & 0x00ff);

	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A3;
}

static void DSP3_OP1E_A3(void)
{
	op1e_cost[ op1e_cell ] = (UINT8)(DSP3_DR & 0x00ff);

	if( op1e_lcv_radius == 1 ) {
		if( op1e_terrain[ op1e_cell ] & 1 ) {
			op1e_weight[ op1e_cell ] = 0xff;
		} else {
			op1e_weight[ op1e_cell ] = op1e_cost[ op1e_cell ];
		}
	}
	else {
		op1e_weight[ op1e_cell ] = 0xff;
	}

	DSP3_OP1E_D( (INT16)(op1e_turn+2), &op1e_x, &op1e_y );
	op1e_lcv_steps--;

	DSP3_SR = 0x0080;
	DSP3_OP1E_A();
}


static void DSP3_OP1E_B(void)
{
	op1e_x = op3e_x;
	op1e_y = op3e_y;
	op1e_lcv_radius = 1;

	op1e_search = 0;

	DSP3_OP1E_B1();

	SetDSP3 = &DSP3_OP1E_C;
}


static void DSP3_OP1E_B1(void)
{
	while( op1e_lcv_radius < op1e_max_radius ) {
		op1e_y--;

		op1e_lcv_turns = 6;
		op1e_turn = 5;

		while( op1e_lcv_turns ) {
			op1e_lcv_steps = op1e_lcv_radius;

			while( op1e_lcv_steps ) {
				DSP3_OP1E_D1( op1e_turn, &op1e_x, &op1e_y );

				if( 0 <= op1e_y && op1e_y < DSP3_WinHi &&
						0 <= op1e_x && op1e_x < DSP3_WinLo ) {
					DSP3_DR = (UINT8)(op1e_x) | ((UINT8)(op1e_y)<<8);
					DSP3_OP03();

					op1e_cell = DSP3_DR;
					if( op1e_cost[ op1e_cell ] < 0x80 &&
							op1e_terrain[ op1e_cell ] < 0x40 ) {
						DSP3_OP1E_B2();
					} // end cell perimeter
				}

				op1e_lcv_steps--;
			} // end search line

			op1e_turn--;
			if( op1e_turn == 0 ) op1e_turn = 6;

			op1e_lcv_turns--;
		} // end circle search

		op1e_lcv_radius++;
	} // end radius search
}


static void DSP3_OP1E_B2(void)
{
	INT16 cell;
	INT16 path;
	INT16 x,y;
	INT16 lcv_turns;

	path = 0xff;
	lcv_turns = 6;

	while( lcv_turns ) {
		x = op1e_x;
		y = op1e_y;

		DSP3_OP1E_D1( lcv_turns, &x, &y );

		DSP3_DR = (UINT8)(x) | ((UINT8)(y)<<8);
		DSP3_OP03();

		cell = DSP3_DR;

		if( 0 <= y && y < DSP3_WinHi &&
				0 <= x && x < DSP3_WinLo  ) {

			if( op1e_terrain[ cell ] < 0x80 || op1e_weight[ cell ] == 0 ) {
				if( op1e_weight[ cell ] < path ) {
					path = op1e_weight[ cell ];
				}
			}
		} // end step travel

		lcv_turns--;
	} // end while turns

	if( path != 0xff ) {
		op1e_weight[ op1e_cell ] = path + op1e_cost[ op1e_cell ];
	}
}


static void DSP3_OP1E_C(void)
{
	int lcv;

	op1e_min_radius = (UINT8)(DSP3_DR & 0x00ff);
	op1e_max_radius = (UINT8)((DSP3_DR & 0xff00)>>8);

	if( op1e_min_radius == 0 )
		op1e_min_radius++;

	if( op1e_max_path_radius >= op1e_min_radius )
		op1e_min_radius = op1e_max_path_radius+1;

	if( op1e_max_radius > op1e_max_path_radius )
		op1e_max_path_radius = op1e_max_radius;

	op1e_lcv_radius = op1e_min_radius;
	op1e_lcv_steps = op1e_min_radius;

	op1e_lcv_turns = 6;
	op1e_turn = 0;

	op1e_x = op3e_x;
	op1e_y = op3e_y;

	for( lcv = 0; lcv < op1e_min_radius; lcv++ )
		DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );

	DSP3_OP1E_C1();
}


static void DSP3_OP1E_C1(void)
{
	int lcv;

	if( op1e_lcv_steps == 0 ) {
		op1e_lcv_radius++;

		op1e_lcv_steps = op1e_lcv_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_lcv_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_radius > op1e_max_radius ) {
		op1e_turn++;
		op1e_lcv_turns--;

		op1e_lcv_radius = op1e_min_radius;
		op1e_lcv_steps = op1e_min_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_min_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_turns == 0 ) {
		DSP3_DR = 0xffff;
		DSP3_SR = 0x0080;
		SetDSP3 = &DSP3_Reset;
		return;
	}

	DSP3_DR = (UINT8)(op1e_x) | ((UINT8)(op1e_y)<<8);
	DSP3_OP03();

	op1e_cell = DSP3_DR;

	DSP3_SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_C2;
}


static void DSP3_OP1E_C2(void)
{
	DSP3_DR = op1e_weight[ op1e_cell ];

	DSP3_OP1E_D( (INT16)(op1e_turn+2), &op1e_x, &op1e_y );
	op1e_lcv_steps--;

	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_C1;
}


static void DSP3_OP1E_D( INT16 move, INT16 *lo, INT16 *hi )
{
	UINT32 dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	INT16 Lo;
	INT16 Hi;

	DSP3_AddHi = DSP3_DataROM[dataOfs];
	DSP3_AddLo = DSP3_DataROM[dataOfs + 1];

	Lo = (UINT8)(*lo);
	Hi = (UINT8)(*hi);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	if (DSP3_AddLo < 0)
		DSP3_AddLo += DSP3_WinLo;
	else
		if (DSP3_AddLo >= DSP3_WinLo)
			DSP3_AddLo -= DSP3_WinLo;

	if (DSP3_AddHi < 0)
		DSP3_AddHi += DSP3_WinHi;
	else
		if (DSP3_AddHi >= DSP3_WinHi)
			DSP3_AddHi -= DSP3_WinHi;

	*lo = DSP3_AddLo;
	*hi = DSP3_AddHi;
}


static void DSP3_OP1E_D1( INT16 move, INT16 *lo, INT16 *hi )
{
	//UINT32 dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	INT16 Lo;
	INT16 Hi;

	const unsigned short HiAdd[] = {
		0x00, 0xFF, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
		0x00, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF, 0x00
	};
	const unsigned short LoAdd[] = {
		0x00, 0x00, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x00
	};

	if( (*lo) & 1 )
		DSP3_AddHi = HiAdd[ move + 8 ];
	else
		DSP3_AddHi = HiAdd[ move + 0 ];
	DSP3_AddLo = LoAdd[ move ];

	Lo = (UINT8)(*lo);
	Hi = (UINT8)(*hi);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	*lo = DSP3_AddLo;
	*hi = DSP3_AddHi;
}


static void DSP3_OP10(void)
{
	if( DSP3_DR == 0xffff ) {
		DSP3_Reset();
	} else {
		// absorb 2 bytes
		DSP3_DR = DSP3_DR;
	}
}

#ifdef UNUSED_FUNCTION
static void DSP3_OP0C_A(void)
{
	// absorb 2 bytes

	DSP3_DR = 0;
	SetDSP3 = &DSP3_Reset;
}
#endif

static void DSP3_OP0C(void)
{
	// absorb 2 bytes

	DSP3_DR = 0;
	//SetDSP3 = &DSP3_OP0C_A;
	SetDSP3 = &DSP3_Reset;
}


static void DSP3_OP1C_C(void)
{
	// return 2 bytes
	DSP3_DR = 0;
	SetDSP3 = &DSP3_Reset;
}


static void DSP3_OP1C_B(void)
{
	// absorb 2 bytes

	// return 2 bytes
	DSP3_DR = 0;
	SetDSP3 = &DSP3_OP1C_C;
}


static void DSP3_OP1C_A(void)
{
	// absorb 2 bytes

	SetDSP3 = &DSP3_OP1C_B;
}


static void DSP3_OP1C(void)
{
	// absorb 2 bytes

	SetDSP3 = &DSP3_OP1C_A;
}


static void DSP3_Command(void)
{
	if (DSP3_DR < 0x40)
	{
		switch (DSP3_DR)
		{
    case 0x02: SetDSP3 = &DSP3_Coordinate; break;
    case 0x03: SetDSP3 = &DSP3_OP03; break;
    case 0x06: SetDSP3 = &DSP3_OP06; break;
    case 0x07: SetDSP3 = &DSP3_OP07; return;
    case 0x0c: SetDSP3 = &DSP3_OP0C; break;
    case 0x0f: SetDSP3 = &DSP3_TestMemory; break;
    case 0x10: SetDSP3 = &DSP3_OP10; break;
    case 0x18: SetDSP3 = &DSP3_Convert; break;
    case 0x1c: SetDSP3 = &DSP3_OP1C; break;
    case 0x1e: SetDSP3 = &DSP3_OP1E; break;
    case 0x1f: SetDSP3 = &DSP3_MemoryDump; break;
    case 0x38: SetDSP3 = &DSP3_Decode; break;
    case 0x3e: SetDSP3 = &DSP3_OP3E; break;
    default:
     return;
		}
		DSP3_SR = 0x0080;
		DSP3_Index = 0;
	}
}


static void DSP3_write( UINT16 dsp3_address, UINT8 dsp3_byte )
{
  if (dsp3_address < 0xC000)
  {
		if (DSP3_SR & 0x04)
		{
			DSP3_DR = (DSP3_DR & 0xff00) + dsp3_byte;
			(*SetDSP3)();
		}
		else
		{
			DSP3_SR ^= 0x10;

			if (DSP3_SR & 0x10)
				DSP3_DR = (DSP3_DR & 0xff00) + dsp3_byte;
			else
			{
				DSP3_DR = (DSP3_DR & 0x00ff) + (dsp3_byte << 8);
				(*SetDSP3)();
			}
		}
  }
}

static UINT8 DSP3_read( UINT16 dsp3_address )
{
  UINT8 value;
  if (dsp3_address < 0xC000)
  {
		if (DSP3_SR & 0x04)
		{
			value = (UINT8) DSP3_DR;
			(*SetDSP3)();
		}
		else
		{
			DSP3_SR ^= 0x10;

			if (DSP3_SR & 0x10)
				value = (UINT8) (DSP3_DR);
			else
			{
				value = (UINT8) (DSP3_DR >> 8);
				(*SetDSP3)();
			}
		}

  }
  else
  {
    value = (UINT8) DSP3_SR;
  }

  return value;
}

static void InitDSP3( running_machine *machine )
{
	UINT32 i;
	UINT8 *dspin = memory_region(machine, "addons");

	DSP3_Reset();

	// expand the DSP-3 data ROM
	for (i = 0; i < 2048; i+=2)
	{
		DSP3_DataROM[i/2] = dspin[0x800 + i] << 8 | dspin[0x800 + i + 1];
	}
}
