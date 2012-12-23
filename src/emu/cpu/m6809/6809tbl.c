const UINT8 m6809_base_device::m_flags8i[256] = /* increment */
{
	CC_Z,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	CC_N|CC_V,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,	  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

const UINT8 m6809_base_device::m_flags8d[256] = /* decrement */
{
	CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CC_V,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
	CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

/* FIXME: Cycles differ slighly from hd6309 emulation */
const UINT8 m6809_base_device::m_index_cycle_em[256] = {        /* Index Loopup cycle counts */
/*           0xX0, 0xX1, 0xX2, 0xX3, 0xX4, 0xX5, 0xX6, 0xX7, 0xX8, 0xX9, 0xXA, 0xXB, 0xXC, 0xXD, 0xXE, 0xXF */

/* 0x0X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x1X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x2X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x3X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x4X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x5X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x6X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x7X */      1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
/* 0x8X */      2,    3,    2,    3,    0,    1,    1,    0,    1,    4,    0,    4,    1,    5,    0,    5,
/* 0x9X */      5,    6,    5,    6,    3,    4,    4,    0,    4,    7,    0,    7,    4,    8,    0,    8,
/* 0xAX */      2,    3,    2,    3,    0,    1,    1,    0,    1,    4,    0,    4,    1,    5,    0,    5,
/* 0xBX */      5,    6,    5,    6,    3,    4,    4,    0,    4,    7,    0,    7,    4,    8,    0,    8,
/* 0xCX */      2,    3,    2,    3,    0,    1,    1,    0,    1,    4,    0,    4,    1,    5,    0,    3,
/* 0xDX */      5,    6,    5,    6,    3,    4,    4,    0,    4,    7,    0,    7,    4,    8,    0,    8,
/* 0xEX */      2,    3,    2,    3,    0,    1,    1,    0,    1,    4,    0,    4,    1,    5,    0,    5,
/* 0xFX */      4,    6,    5,    6,    3,    4,    4,    0,    4,    7,    0,    7,    4,    8,    0,    8
};

/* timings for 1-byte opcodes */
const UINT8 m6809_base_device::m_cycles1[256] =
{
	/*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
  /*0*/  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
  /*1*/  0, 0, 2, 4, 2, 2, 5, 9, 2, 2, 3, 2, 3, 2, 8, 6,
  /*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  /*3*/  4, 4, 4, 4, 5, 5, 5, 5, 2, 5, 3, 6,20,11, 2,19,
  /*4*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  /*5*/  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  /*6*/  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
  /*7*/  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 7,
  /*8*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 7, 3, 2,
  /*9*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
  /*A*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
  /*B*/  5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 8, 6, 6,
  /*C*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 3,
  /*D*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
  /*E*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
  /*F*/  5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6
};
