/************************************************************************

    DSP-3 emulator code

    Copyright (c) 2003-2009 John Weidman, Kris Bleakley, Lancer, Nach,
    z80 gaiden, and Jonas Quinn

    This code is released by ZSNES Team under GNU General Public License
    version 2 as published by the Free Software Foundation.
    The implementation below is released under the MAME license for use
    in MAME, MESS and derivatives by permission of the authors.

************************************************************************/

static void dsp3_Command(void);

struct _snes_dsp3_state
{
	UINT16  DR;
	UINT16  SR;
	UINT16  MemoryIndex;

	INT16   AddLo;
	INT16   AddHi;
	INT16   WinLo;
	INT16   WinHi;
	UINT16  Codewords;
	UINT16  Outwords;
	UINT16  Symbol;
	UINT16  BitCount;
	UINT16  Index;
	UINT16  Codes[512];
	UINT16  BitsLeft;
	UINT16  ReqBits;
	UINT16  ReqData;
	UINT16  BitCommand;
	UINT8   BaseLength;
	UINT16  BaseCodes;
	UINT16  BaseCode;
	UINT8   CodeLengths[8];
	UINT16  CodeOffsets[8];
	UINT16  LZCode;
	UINT8   LZLength;

	UINT16  X;
	UINT16  Y;

	UINT8   Bitmap[8];
	UINT8   Bitplane[8];
	UINT16  BMIndex;
	UINT16  BPIndex;
	UINT16  Count;

	INT16 op3e_x;
	INT16 op3e_y;

	INT16 op1e_terrain[0x2000];
	INT16 op1e_cost[0x2000];
	INT16 op1e_weight[0x2000];

	INT16 op1e_cell;
	INT16 op1e_turn;
	INT16 op1e_search;

	INT16 op1e_x;
	INT16 op1e_y;

	INT16 op1e_min_radius;
	INT16 op1e_max_radius;

	INT16 op1e_max_search_radius;
	INT16 op1e_max_path_radius;

	INT16 op1e_lcv_radius;
	INT16 op1e_lcv_steps;
	INT16 op1e_lcv_turns;

	void(*SetDSP3)(void);

	UINT16 DataROM[1024];
};

static struct _snes_dsp3_state  dsp3_state;


static void dsp3_Reset(void)
{
	dsp3_state.DR = 0x0080;
	dsp3_state.SR = 0x0084;
	dsp3_state.SetDSP3 = &dsp3_Command;
}

#ifdef UNUSED_FUNCTION
void dsp3_MemorySize(void)
{
	dsp3_state.DR = 0x0300;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}
#endif

static void dsp3_TestMemory(void)
{
	dsp3_state.DR = 0x0000;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}

static void dsp3_DumpDataROM(void)
{
	dsp3_state.DR = dsp3_state.DataROM[dsp3_state.MemoryIndex++];
	if (dsp3_state.MemoryIndex == 1024)
		dsp3_state.SetDSP3 = &dsp3_Reset;
}

static void dsp3_MemoryDump(void)
{
	dsp3_state.MemoryIndex = 0;
	dsp3_state.SetDSP3 = &dsp3_DumpDataROM;
	dsp3_DumpDataROM();
}

static void dsp3_OP06(void)
{
	dsp3_state.WinLo = (UINT8)(dsp3_state.DR);
	dsp3_state.WinHi = (UINT8)(dsp3_state.DR >> 8);
	dsp3_Reset();
}

static void dsp3_OP03(void)
{
	INT16 Lo = (UINT8)(dsp3_state.DR);
	INT16 Hi = (UINT8)(dsp3_state.DR >> 8);
	INT16 Ofs = (dsp3_state.WinLo * Hi << 1) + (Lo << 1);
	dsp3_state.DR = Ofs >> 1;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}

static void dsp3_OP07_B(void)
{
	INT16 Ofs = (dsp3_state.WinLo * dsp3_state.AddHi << 1) + (dsp3_state.AddLo << 1);
	dsp3_state.DR = Ofs >> 1;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}

static void dsp3_OP07_A(void)
{
	INT16 Lo = (UINT8)(dsp3_state.DR);
	INT16 Hi = (UINT8)(dsp3_state.DR >> 8);

	if (Lo & 1)
		Hi += (dsp3_state.AddLo & 1);

	dsp3_state.AddLo += Lo;
	dsp3_state.AddHi += Hi;

	if (dsp3_state.AddLo < 0)
		dsp3_state.AddLo += dsp3_state.WinLo;
	else if (dsp3_state.AddLo >= dsp3_state.WinLo)
		dsp3_state.AddLo -= dsp3_state.WinLo;

	if (dsp3_state.AddHi < 0)
		dsp3_state.AddHi += dsp3_state.WinHi;
	else if (dsp3_state.AddHi >= dsp3_state.WinHi)
		dsp3_state.AddHi -= dsp3_state.WinHi;

	dsp3_state.DR = dsp3_state.AddLo | (dsp3_state.AddHi << 8) | ((dsp3_state.AddHi >> 8) & 0xff);
	dsp3_state.SetDSP3 = &dsp3_OP07_B;
}

static void dsp3_OP07(void)
{
	UINT32 dataOfs = ((dsp3_state.DR << 1) + 0x03b2) & 0x03ff;

	dsp3_state.AddHi = dsp3_state.DataROM[dataOfs];
	dsp3_state.AddLo = dsp3_state.DataROM[dataOfs + 1];

	dsp3_state.SetDSP3 = &dsp3_OP07_A;
	dsp3_state.SR = 0x0080;
}

static void dsp3_Coordinate(void)
{
	dsp3_state.Index++;

	switch (dsp3_state.Index)
	{
	case 3:
		if (dsp3_state.DR == 0xffff)
			dsp3_Reset();
		break;
	case 4:
		dsp3_state.X = dsp3_state.DR;
		break;
	case 5:
		dsp3_state.Y = dsp3_state.DR;
		dsp3_state.DR = 1;
		break;
	case 6:
		dsp3_state.DR = dsp3_state.X;
		break;
	case 7:
		dsp3_state.DR = dsp3_state.Y;
		dsp3_state.Index = 0;
		break;
	}
}

static void dsp3_Convert_A(void)
{
	if (dsp3_state.BMIndex < 8)
	{
		dsp3_state.Bitmap[dsp3_state.BMIndex++] = (UINT8) (dsp3_state.DR);
		dsp3_state.Bitmap[dsp3_state.BMIndex++] = (UINT8) (dsp3_state.DR >> 8);

		if (dsp3_state.BMIndex == 8)
		{
			short i, j;
			for (i = 0; i < 8; i++)
				for (j = 0; j < 8; j++)
				{
					dsp3_state.Bitplane[j] <<= 1;
					dsp3_state.Bitplane[j] |= (dsp3_state.Bitmap[i] >> j) & 1;
				}

			dsp3_state.BPIndex = 0;
			dsp3_state.Count--;
		}
	}

	if (dsp3_state.BMIndex == 8)
	{
		if (dsp3_state.BPIndex == 8)
		{
			if (!dsp3_state.Count)
				dsp3_Reset();
			dsp3_state.BMIndex = 0;
		}
		else
		{
			dsp3_state.DR =  dsp3_state.Bitplane[dsp3_state.BPIndex++];
			dsp3_state.DR |= dsp3_state.Bitplane[dsp3_state.BPIndex++] << 8;
		}
	}
}

static void dsp3_Convert(void)
{
	dsp3_state.Count = dsp3_state.DR;
	dsp3_state.BMIndex = 0;
	dsp3_state.SetDSP3 = &dsp3_Convert_A;
}

static UINT8 dsp3_GetBits(UINT8 Count)
{
	if (!dsp3_state.BitsLeft)
	{
		dsp3_state.BitsLeft = Count;
		dsp3_state.ReqBits = 0;
	}

	do {
		if (!dsp3_state.BitCount)
		{
			dsp3_state.SR = 0xC0;
			return 0;
		}

		dsp3_state.ReqBits <<= 1;
		if (dsp3_state.ReqData & 0x8000)
			dsp3_state.ReqBits++;
		dsp3_state.ReqData <<= 1;

		dsp3_state.BitCount--;
		dsp3_state.BitsLeft--;

	} while (dsp3_state.BitsLeft);

	return 1;
}

static void dsp3_Decode_Data(void)
{
	if (!dsp3_state.BitCount)
	{
		if (dsp3_state.SR & 0x40)
		{
			dsp3_state.ReqData = dsp3_state.DR;
			dsp3_state.BitCount += 16;
		}
		else
		{
			dsp3_state.SR = 0xC0;
			return;
		}
	}

	if (dsp3_state.LZCode == 1)
	{
		if (!dsp3_GetBits(1))
			return;

		if (dsp3_state.ReqBits)
			dsp3_state.LZLength = 12;
		else
			dsp3_state.LZLength = 8;

		dsp3_state.LZCode++;
	}

	if (dsp3_state.LZCode == 2)
	{
		if (!dsp3_GetBits(dsp3_state.LZLength))
			return;

		dsp3_state.LZCode = 0;
		dsp3_state.Outwords--;
		if (!dsp3_state.Outwords) dsp3_state.SetDSP3 = &dsp3_Reset;

		dsp3_state.SR = 0x80;
		dsp3_state.DR = dsp3_state.ReqBits;
		return;
	}

	if (dsp3_state.BaseCode == 0xffff)
	{
		if (!dsp3_GetBits(dsp3_state.BaseLength))
			return;

		dsp3_state.BaseCode = dsp3_state.ReqBits;
	}

	if (!dsp3_GetBits(dsp3_state.CodeLengths[dsp3_state.BaseCode]))
		return;

	dsp3_state.Symbol = dsp3_state.Codes[dsp3_state.CodeOffsets[dsp3_state.BaseCode] + dsp3_state.ReqBits];
	dsp3_state.BaseCode = 0xffff;

	if (dsp3_state.Symbol & 0xff00)
	{
		dsp3_state.Symbol += 0x7f02;
		dsp3_state.LZCode++;
	}
	else
	{
		dsp3_state.Outwords--;
		if (!dsp3_state.Outwords)
			dsp3_state.SetDSP3 = &dsp3_Reset;
	}

	dsp3_state.SR = 0x80;
	dsp3_state.DR = dsp3_state.Symbol;
}

static void dsp3_Decode_Tree(void)
{
	if (!dsp3_state.BitCount)
	{
		dsp3_state.ReqData = dsp3_state.DR;
		dsp3_state.BitCount += 16;
	}

	if (!dsp3_state.BaseCodes)
	{
		dsp3_GetBits(1);
		if (dsp3_state.ReqBits)
		{
			dsp3_state.BaseLength = 3;
			dsp3_state.BaseCodes = 8;
		}
		else
		{
			dsp3_state.BaseLength = 2;
			dsp3_state.BaseCodes = 4;
		}
	}

	while (dsp3_state.BaseCodes)
	{
		if (!dsp3_GetBits(3))
			return;

		dsp3_state.ReqBits++;

		dsp3_state.CodeLengths[dsp3_state.Index] = (UINT8) dsp3_state.ReqBits;
		dsp3_state.CodeOffsets[dsp3_state.Index] = dsp3_state.Symbol;
		dsp3_state.Index++;

		dsp3_state.Symbol += 1 << dsp3_state.ReqBits;
		dsp3_state.BaseCodes--;
	}

	dsp3_state.BaseCode = 0xffff;
	dsp3_state.LZCode = 0;

	dsp3_state.SetDSP3 = &dsp3_Decode_Data;
	if (dsp3_state.BitCount)
		dsp3_Decode_Data();
}

static void dsp3_Decode_Symbols(void)
{
	dsp3_state.ReqData = dsp3_state.DR;
	dsp3_state.BitCount += 16;

	do {

		if (dsp3_state.BitCommand == 0xffff)
		{
			if (!dsp3_GetBits(2)) return;
			dsp3_state.BitCommand = dsp3_state.ReqBits;
		}

		switch (dsp3_state.BitCommand)
		{
		case 0:
			{
				if (!dsp3_GetBits(9)) return;
				dsp3_state.Symbol = dsp3_state.ReqBits;
				break;
			}
		case 1:
			{
				dsp3_state.Symbol++;
				break;
			}
		case 2:
			{
				if (!dsp3_GetBits(1)) return;
				dsp3_state.Symbol += 2 + dsp3_state.ReqBits;
				break;
			}
		case 3:
			{
				if (!dsp3_GetBits(4)) return;
				dsp3_state.Symbol += 4 + dsp3_state.ReqBits;
				break;
			}
		}

		dsp3_state.BitCommand = 0xffff;

		dsp3_state.Codes[dsp3_state.Index++] = dsp3_state.Symbol;
		dsp3_state.Codewords--;

	} while (dsp3_state.Codewords);

	dsp3_state.Index = 0;
	dsp3_state.Symbol = 0;
	dsp3_state.BaseCodes = 0;

	dsp3_state.SetDSP3 = &dsp3_Decode_Tree;
	if (dsp3_state.BitCount) dsp3_Decode_Tree();
}

static void dsp3_Decode_A(void)
{
	dsp3_state.Outwords = dsp3_state.DR;
	dsp3_state.SetDSP3 = &dsp3_Decode_Symbols;
	dsp3_state.BitCount = 0;
	dsp3_state.BitsLeft = 0;
	dsp3_state.Symbol = 0;
	dsp3_state.Index = 0;
	dsp3_state.BitCommand = 0xffff;
	dsp3_state.SR = 0xC0;
}

static void dsp3_Decode(void)
{
	dsp3_state.Codewords = dsp3_state.DR;
	dsp3_state.SetDSP3 = &dsp3_Decode_A;
}


// Opcodes 1E/3E bit-perfect to 'dsp3-intro' log
// src: adapted from SD Gundam X/G-Next

static void dsp3_OP3E(void)
{
	dsp3_state.op3e_x = (UINT8)(dsp3_state.DR & 0x00ff);
	dsp3_state.op3e_y = (UINT8)((dsp3_state.DR & 0xff00) >> 8);

	dsp3_OP03();

	dsp3_state.op1e_terrain[dsp3_state.DR] = 0x00;
	dsp3_state.op1e_cost[dsp3_state.DR] = 0xff;
	dsp3_state.op1e_weight[dsp3_state.DR] = 0;

	dsp3_state.op1e_max_search_radius = 0;
	dsp3_state.op1e_max_path_radius = 0;
}

static void dsp3_OP1E_A(void);
static void dsp3_OP1E_A1(void);
static void dsp3_OP1E_A2(void);
static void dsp3_OP1E_A3(void);

static void dsp3_OP1E_B(void);
static void dsp3_OP1E_B1(void);
static void dsp3_OP1E_B2(void);

static void dsp3_OP1E_C(void);
static void dsp3_OP1E_C1(void);
static void dsp3_OP1E_C2(void);

static void dsp3_OP1E_D(INT16, INT16 *, INT16 *);
static void dsp3_OP1E_D1(INT16 move, INT16 *lo, INT16 *hi);

static void dsp3_OP1E(void)
{
	int lcv;

	dsp3_state.op1e_min_radius = (UINT8)(dsp3_state.DR & 0x00ff);
	dsp3_state.op1e_max_radius = (UINT8)((dsp3_state.DR & 0xff00)>>8);

	if (dsp3_state.op1e_min_radius == 0)
		dsp3_state.op1e_min_radius++;

	if (dsp3_state.op1e_max_search_radius >= dsp3_state.op1e_min_radius)
		dsp3_state.op1e_min_radius = dsp3_state.op1e_max_search_radius + 1;

	if (dsp3_state.op1e_max_radius > dsp3_state.op1e_max_search_radius)
		dsp3_state.op1e_max_search_radius = dsp3_state.op1e_max_radius;

	dsp3_state.op1e_lcv_radius = dsp3_state.op1e_min_radius;
	dsp3_state.op1e_lcv_steps = dsp3_state.op1e_min_radius;

	dsp3_state.op1e_lcv_turns = 6;
	dsp3_state.op1e_turn = 0;

	dsp3_state.op1e_x = dsp3_state.op3e_x;
	dsp3_state.op1e_y = dsp3_state.op3e_y;

	for (lcv = 0; lcv < dsp3_state.op1e_min_radius; lcv++)
		dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);

	dsp3_OP1E_A();
}

static void dsp3_OP1E_A(void)
{
	int lcv;

	if (dsp3_state.op1e_lcv_steps == 0)
	{
		dsp3_state.op1e_lcv_radius++;

		dsp3_state.op1e_lcv_steps = dsp3_state.op1e_lcv_radius;

		dsp3_state.op1e_x = dsp3_state.op3e_x;
		dsp3_state.op1e_y = dsp3_state.op3e_y;

		for (lcv = 0; lcv < dsp3_state.op1e_lcv_radius; lcv++)
			dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	}

	if (dsp3_state.op1e_lcv_radius > dsp3_state.op1e_max_radius)
	{
		dsp3_state.op1e_turn++;
		dsp3_state.op1e_lcv_turns--;

		dsp3_state.op1e_lcv_radius = dsp3_state.op1e_min_radius;
		dsp3_state.op1e_lcv_steps = dsp3_state.op1e_min_radius;

		dsp3_state.op1e_x = dsp3_state.op3e_x;
		dsp3_state.op1e_y = dsp3_state.op3e_y;

		for (lcv = 0; lcv < dsp3_state.op1e_min_radius; lcv++)
			dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	}

	if (dsp3_state.op1e_lcv_turns == 0)
	{
		dsp3_state.DR = 0xffff;
		dsp3_state.SR = 0x0080;
		dsp3_state.SetDSP3 = &dsp3_OP1E_B;
		return;
	}

	dsp3_state.DR = (UINT8)(dsp3_state.op1e_x) | ((UINT8)(dsp3_state.op1e_y) << 8);
	dsp3_OP03();

	dsp3_state.op1e_cell = dsp3_state.DR;

	dsp3_state.SR = 0x0080;
	dsp3_state.SetDSP3 = &dsp3_OP1E_A1;
}

static void dsp3_OP1E_A1(void)
{
	dsp3_state.SR = 0x0084;
	dsp3_state.SetDSP3 = &dsp3_OP1E_A2;
}

static void dsp3_OP1E_A2(void)
{
	dsp3_state.op1e_terrain[dsp3_state.op1e_cell] = (UINT8)(dsp3_state.DR & 0x00ff);

	dsp3_state.SR = 0x0084;
	dsp3_state.SetDSP3 = &dsp3_OP1E_A3;
}

static void dsp3_OP1E_A3(void)
{
	dsp3_state.op1e_cost[dsp3_state.op1e_cell] = (UINT8)(dsp3_state.DR & 0x00ff);

	if (dsp3_state.op1e_lcv_radius == 1)
	{
		if (dsp3_state.op1e_terrain[dsp3_state.op1e_cell] & 1)
		{
			dsp3_state.op1e_weight[dsp3_state.op1e_cell] = 0xff;
		}
		else
		{
			dsp3_state.op1e_weight[dsp3_state.op1e_cell] = dsp3_state.op1e_cost[dsp3_state.op1e_cell];
		}
	}
	else
	{
		dsp3_state.op1e_weight[dsp3_state.op1e_cell] = 0xff;
	}

	dsp3_OP1E_D((INT16)(dsp3_state.op1e_turn + 2), &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	dsp3_state.op1e_lcv_steps--;

	dsp3_state.SR = 0x0080;
	dsp3_OP1E_A();
}


static void dsp3_OP1E_B(void)
{
	dsp3_state.op1e_x = dsp3_state.op3e_x;
	dsp3_state.op1e_y = dsp3_state.op3e_y;
	dsp3_state.op1e_lcv_radius = 1;

	dsp3_state.op1e_search = 0;

	dsp3_OP1E_B1();

	dsp3_state.SetDSP3 = &dsp3_OP1E_C;
}


static void dsp3_OP1E_B1(void)
{
	while (dsp3_state.op1e_lcv_radius < dsp3_state.op1e_max_radius)
	{
		dsp3_state.op1e_y--;

		dsp3_state.op1e_lcv_turns = 6;
		dsp3_state.op1e_turn = 5;

		while (dsp3_state.op1e_lcv_turns)
		{
			dsp3_state.op1e_lcv_steps = dsp3_state.op1e_lcv_radius;

			while (dsp3_state.op1e_lcv_steps)
			{
				dsp3_OP1E_D1(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);

				if (0 <= dsp3_state.op1e_y && dsp3_state.op1e_y < dsp3_state.WinHi &&
						0 <= dsp3_state.op1e_x && dsp3_state.op1e_x < dsp3_state.WinLo )
				{
					dsp3_state.DR = (UINT8)(dsp3_state.op1e_x) | ((UINT8)(dsp3_state.op1e_y) << 8);
					dsp3_OP03();

					dsp3_state.op1e_cell = dsp3_state.DR;
					if (dsp3_state.op1e_cost[dsp3_state.op1e_cell ] < 0x80 && dsp3_state.op1e_terrain[dsp3_state.op1e_cell] < 0x40)
					{
						dsp3_OP1E_B2();
					} // end cell perimeter
				}

				dsp3_state.op1e_lcv_steps--;
			} // end search line

			dsp3_state.op1e_turn--;
			if (dsp3_state.op1e_turn == 0)
				dsp3_state.op1e_turn = 6;

			dsp3_state.op1e_lcv_turns--;
		} // end circle search

		dsp3_state.op1e_lcv_radius++;
	} // end radius search
}


static void dsp3_OP1E_B2(void)
{
	INT16 cell;
	INT16 path;
	INT16 x,y;
	INT16 lcv_turns;

	path = 0xff;
	lcv_turns = 6;

	while (lcv_turns)
	{
		x = dsp3_state.op1e_x;
		y = dsp3_state.op1e_y;

		dsp3_OP1E_D1(lcv_turns, &x, &y);

		dsp3_state.DR = (UINT8)(x) | ((UINT8)(y)<<8);
		dsp3_OP03();

		cell = dsp3_state.DR;

		if (0 <= y && y < dsp3_state.WinHi && 0 <= x && x < dsp3_state.WinLo)
		{
			if (dsp3_state.op1e_terrain[cell] < 0x80 || dsp3_state.op1e_weight[cell] == 0)
			{
				if (dsp3_state.op1e_weight[cell] < path)
					path = dsp3_state.op1e_weight[cell];
			}
		} // end step travel

		lcv_turns--;
	} // end while turns

	if (path != 0xff)
		dsp3_state.op1e_weight[dsp3_state.op1e_cell] = path + dsp3_state.op1e_cost[dsp3_state.op1e_cell];
}


static void dsp3_OP1E_C(void)
{
	int lcv;

	dsp3_state.op1e_min_radius = (UINT8)(dsp3_state.DR & 0x00ff);
	dsp3_state.op1e_max_radius = (UINT8)((dsp3_state.DR & 0xff00) >> 8);

	if (dsp3_state.op1e_min_radius == 0)
		dsp3_state.op1e_min_radius++;

	if (dsp3_state.op1e_max_path_radius >= dsp3_state.op1e_min_radius)
		dsp3_state.op1e_min_radius = dsp3_state.op1e_max_path_radius + 1;

	if (dsp3_state.op1e_max_radius > dsp3_state.op1e_max_path_radius)
		dsp3_state.op1e_max_path_radius = dsp3_state.op1e_max_radius;

	dsp3_state.op1e_lcv_radius = dsp3_state.op1e_min_radius;
	dsp3_state.op1e_lcv_steps = dsp3_state.op1e_min_radius;

	dsp3_state.op1e_lcv_turns = 6;
	dsp3_state.op1e_turn = 0;

	dsp3_state.op1e_x = dsp3_state.op3e_x;
	dsp3_state.op1e_y = dsp3_state.op3e_y;

	for (lcv = 0; lcv < dsp3_state.op1e_min_radius; lcv++)
		dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);

	dsp3_OP1E_C1();
}


static void dsp3_OP1E_C1(void)
{
	int lcv;

	if (dsp3_state.op1e_lcv_steps == 0)
	{
		dsp3_state.op1e_lcv_radius++;

		dsp3_state.op1e_lcv_steps = dsp3_state.op1e_lcv_radius;

		dsp3_state.op1e_x = dsp3_state.op3e_x;
		dsp3_state.op1e_y = dsp3_state.op3e_y;

		for (lcv = 0; lcv < dsp3_state.op1e_lcv_radius; lcv++)
			dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	}

	if (dsp3_state.op1e_lcv_radius > dsp3_state.op1e_max_radius)
	{
		dsp3_state.op1e_turn++;
		dsp3_state.op1e_lcv_turns--;

		dsp3_state.op1e_lcv_radius = dsp3_state.op1e_min_radius;
		dsp3_state.op1e_lcv_steps = dsp3_state.op1e_min_radius;

		dsp3_state.op1e_x = dsp3_state.op3e_x;
		dsp3_state.op1e_y = dsp3_state.op3e_y;

		for (lcv = 0; lcv < dsp3_state.op1e_min_radius; lcv++)
			dsp3_OP1E_D(dsp3_state.op1e_turn, &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	}

	if (dsp3_state.op1e_lcv_turns == 0)
	{
		dsp3_state.DR = 0xffff;
		dsp3_state.SR = 0x0080;
		dsp3_state.SetDSP3 = &dsp3_Reset;
		return;
	}

	dsp3_state.DR = (UINT8)(dsp3_state.op1e_x) | ((UINT8)(dsp3_state.op1e_y) << 8);
	dsp3_OP03();

	dsp3_state.op1e_cell = dsp3_state.DR;

	dsp3_state.SR = 0x0080;
	dsp3_state.SetDSP3 = &dsp3_OP1E_C2;
}


static void dsp3_OP1E_C2(void)
{
	dsp3_state.DR = dsp3_state.op1e_weight[dsp3_state.op1e_cell];

	dsp3_OP1E_D((INT16)(dsp3_state.op1e_turn + 2), &dsp3_state.op1e_x, &dsp3_state.op1e_y);
	dsp3_state.op1e_lcv_steps--;

	dsp3_state.SR = 0x0084;
	dsp3_state.SetDSP3 = &dsp3_OP1E_C1;
}


static void dsp3_OP1E_D( INT16 move, INT16 *lo, INT16 *hi )
{
	UINT32 dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	INT16 Lo;
	INT16 Hi;

	dsp3_state.AddHi = dsp3_state.DataROM[dataOfs];
	dsp3_state.AddLo = dsp3_state.DataROM[dataOfs + 1];

	Lo = (UINT8)(*lo);
	Hi = (UINT8)(*hi);

	if (Lo & 1)	Hi += (dsp3_state.AddLo & 1);

	dsp3_state.AddLo += Lo;
	dsp3_state.AddHi += Hi;

	if (dsp3_state.AddLo < 0)
		dsp3_state.AddLo += dsp3_state.WinLo;
	else if (dsp3_state.AddLo >= dsp3_state.WinLo)
		dsp3_state.AddLo -= dsp3_state.WinLo;

	if (dsp3_state.AddHi < 0)
		dsp3_state.AddHi += dsp3_state.WinHi;
	else if (dsp3_state.AddHi >= dsp3_state.WinHi)
		dsp3_state.AddHi -= dsp3_state.WinHi;

	*lo = dsp3_state.AddLo;
	*hi = dsp3_state.AddHi;
}


static void dsp3_OP1E_D1( INT16 move, INT16 *lo, INT16 *hi )
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

	if ((*lo) & 1)
		dsp3_state.AddHi = HiAdd[move + 8];
	else
		dsp3_state.AddHi = HiAdd[move + 0];
	dsp3_state.AddLo = LoAdd[move];

	Lo = (UINT8)(*lo);
	Hi = (UINT8)(*hi);

	if (Lo & 1)
		Hi += (dsp3_state.AddLo & 1);

	dsp3_state.AddLo += Lo;
	dsp3_state.AddHi += Hi;

	*lo = dsp3_state.AddLo;
	*hi = dsp3_state.AddHi;
}


static void dsp3_OP10(void)
{
	if (dsp3_state.DR == 0xffff )
		dsp3_Reset();
	else
	{
		// absorb 2 bytes
		dsp3_state.DR = dsp3_state.DR;
	}
}

#ifdef UNUSED_FUNCTION
static void dsp3_OP0C_A(void)
{
	// absorb 2 bytes

	dsp3_state.DR = 0;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}
#endif

static void dsp3_OP0C(void)
{
	// absorb 2 bytes

	dsp3_state.DR = 0;
	//dsp3_state.SetDSP3 = &dsp3_OP0C_A;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}


static void dsp3_OP1C_C(void)
{
	// return 2 bytes
	dsp3_state.DR = 0;
	dsp3_state.SetDSP3 = &dsp3_Reset;
}


static void dsp3_OP1C_B(void)
{
	// absorb 2 bytes

	// return 2 bytes
	dsp3_state.DR = 0;
	dsp3_state.SetDSP3 = &dsp3_OP1C_C;
}


static void dsp3_OP1C_A(void)
{
	// absorb 2 bytes

	dsp3_state.SetDSP3 = &dsp3_OP1C_B;
}


static void dsp3_OP1C(void)
{
	// absorb 2 bytes

	dsp3_state.SetDSP3 = &dsp3_OP1C_A;
}


static void dsp3_Command(void)
{
	if (dsp3_state.DR < 0x40)
	{
		switch (dsp3_state.DR)
		{
		case 0x02: dsp3_state.SetDSP3 = &dsp3_Coordinate; break;
		case 0x03: dsp3_state.SetDSP3 = &dsp3_OP03; break;
		case 0x06: dsp3_state.SetDSP3 = &dsp3_OP06; break;
		case 0x07: dsp3_state.SetDSP3 = &dsp3_OP07; return;
		case 0x0c: dsp3_state.SetDSP3 = &dsp3_OP0C; break;
		case 0x0f: dsp3_state.SetDSP3 = &dsp3_TestMemory; break;
		case 0x10: dsp3_state.SetDSP3 = &dsp3_OP10; break;
		case 0x18: dsp3_state.SetDSP3 = &dsp3_Convert; break;
		case 0x1c: dsp3_state.SetDSP3 = &dsp3_OP1C; break;
		case 0x1e: dsp3_state.SetDSP3 = &dsp3_OP1E; break;
		case 0x1f: dsp3_state.SetDSP3 = &dsp3_MemoryDump; break;
		case 0x38: dsp3_state.SetDSP3 = &dsp3_Decode; break;
		case 0x3e: dsp3_state.SetDSP3 = &dsp3_OP3E; break;
		default: return;
		}

		dsp3_state.SR = 0x0080;
		dsp3_state.Index = 0;
	}
}


static void dsp3_write( UINT16 dsp3_address, UINT8 dsp3_byte )
{
	if (dsp3_address < 0xC000)
	{
		if (dsp3_state.SR & 0x04)
		{
			dsp3_state.DR = (dsp3_state.DR & 0xff00) + dsp3_byte;
			(*dsp3_state.SetDSP3)();
		}
		else
		{
			dsp3_state.SR ^= 0x10;

			if (dsp3_state.SR & 0x10)
				dsp3_state.DR = (dsp3_state.DR & 0xff00) + dsp3_byte;
			else
			{
				dsp3_state.DR = (dsp3_state.DR & 0x00ff) + (dsp3_byte << 8);
				(*dsp3_state.SetDSP3)();
			}
		}
	}
}

static UINT8 dsp3_read( UINT16 dsp3_address )
{
	UINT8 value;
	if (dsp3_address < 0xC000)
	{
		if (dsp3_state.SR & 0x04)
		{
			value = (UINT8) dsp3_state.DR;
			(*dsp3_state.SetDSP3)();
		}
		else
		{
			dsp3_state.SR ^= 0x10;

			if (dsp3_state.SR & 0x10)
				value = (UINT8) (dsp3_state.DR);
			else
			{
				value = (UINT8) (dsp3_state.DR >> 8);
				(*dsp3_state.SetDSP3)();
			}
		}

	}
	else
	{
		value = (UINT8) dsp3_state.SR;
	}

	return value;
}

static void dsp3_register_save( running_machine *machine )
{
	state_save_register_global(machine, dsp3_state.DR);
	state_save_register_global(machine, dsp3_state.SR);
	state_save_register_global(machine, dsp3_state.MemoryIndex);

	state_save_register_global(machine, dsp3_state.AddLo);
	state_save_register_global(machine, dsp3_state.AddHi);
	state_save_register_global(machine, dsp3_state.WinLo);
	state_save_register_global(machine, dsp3_state.WinHi);
	state_save_register_global(machine, dsp3_state.Codewords);
	state_save_register_global(machine, dsp3_state.Outwords);
	state_save_register_global(machine, dsp3_state.Symbol);
	state_save_register_global(machine, dsp3_state.BitCount);
	state_save_register_global(machine, dsp3_state.Index);
	state_save_register_global_array(machine, dsp3_state.Codes);
	state_save_register_global(machine, dsp3_state.BitsLeft);
	state_save_register_global(machine, dsp3_state.ReqBits);
	state_save_register_global(machine, dsp3_state.ReqData);
	state_save_register_global(machine, dsp3_state.BitCommand);
	state_save_register_global(machine, dsp3_state.BaseLength);
	state_save_register_global(machine, dsp3_state.BaseCodes);
	state_save_register_global(machine, dsp3_state.BaseCode);
	state_save_register_global_array(machine, dsp3_state.CodeLengths);
	state_save_register_global_array(machine, dsp3_state.CodeOffsets);
	state_save_register_global(machine, dsp3_state.LZCode);
	state_save_register_global(machine, dsp3_state.LZLength);

	state_save_register_global(machine, dsp3_state.X);
	state_save_register_global(machine, dsp3_state.Y);

	state_save_register_global_array(machine, dsp3_state.Bitmap);
	state_save_register_global_array(machine, dsp3_state.Bitplane);
	state_save_register_global(machine, dsp3_state.BMIndex);
	state_save_register_global(machine, dsp3_state.BPIndex);
	state_save_register_global(machine, dsp3_state.Count);

	state_save_register_global(machine, dsp3_state.op3e_x);
	state_save_register_global(machine, dsp3_state.op3e_y);

	state_save_register_global_array(machine, dsp3_state.op1e_terrain);
	state_save_register_global_array(machine, dsp3_state.op1e_cost);
	state_save_register_global_array(machine, dsp3_state.op1e_weight);

	state_save_register_global(machine, dsp3_state.op1e_cell);
	state_save_register_global(machine, dsp3_state.op1e_turn);
	state_save_register_global(machine, dsp3_state.op1e_search);

	state_save_register_global(machine, dsp3_state.op1e_x);
	state_save_register_global(machine, dsp3_state.op1e_y);

	state_save_register_global(machine, dsp3_state.op1e_min_radius);
	state_save_register_global(machine, dsp3_state.op1e_max_radius);

	state_save_register_global(machine, dsp3_state.op1e_max_search_radius);
	state_save_register_global(machine, dsp3_state.op1e_max_path_radius);

	state_save_register_global(machine, dsp3_state.op1e_lcv_radius);
	state_save_register_global(machine, dsp3_state.op1e_lcv_steps);
	state_save_register_global(machine, dsp3_state.op1e_lcv_turns);

}

static void dsp3_init( running_machine *machine )
{
	UINT32 i;
	UINT8 *dspin = memory_region(machine, "addons");


	// expand the DSP-3 data ROM
	for (i = 0; i < 2048; i += 2)
	{
		dsp3_state.DataROM[i / 2] = dspin[0x800 + i] << 8 | dspin[0x800 + i + 1];
	}

	dsp3_register_save(machine);
}
