// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Couriersud, Jonathan Gevaryahu
/* TMS51xx and TMS52xx ROM Tables */

/* The following table is assumed to be for TMS5100
 *
 * US Patent 4209836
 *           4331836
 *           4304964
 *           4234761
 *           4189779
 *           4449233
 *
 * All patents give interpolation coefficients
 *  { 1, 8, 8, 8, 4, 4, 2, 2 }
 *  This sequence will not calculate the published
 *  fractions:
 * 1 8 0.125
 * 2 8 0.234
 * 3 8 0.330
 * 4 4 0.498
 * 5 4 0.623
 * 6 2 0.717
 * 7 2 0.859
 * 0 1 1.000
 * (remember, 1 is the FIRST entry!)
 *
 * Instead,  { 1, 8, 8, 8, 4, 4, 4, 2 }
 * will calculate those coefficients.
 * Howeever, after simulating the actual circuit from the patent in pspice,
 * the { 1, 8, 8, 8, 4, 4, 2, 2 } pattern is revealed as the correct one.
 * Since the real chip uses shifters and not true division to achieve those
 * factors, they have been replaced by the shifting coefficients:
 * { 0, 3, 3, 3, 2, 2, 1, 1 }
 */

	/* quick note on derivative analysis:
	Judging by all the TI chips I (Lord Nightmare) have done this test on, the first derivative between successive values of the LPC tables should follow a roughly triangular or sine shaped curve, the second derivative should start at a value, increase slightly, then decrease smoothly and become negative right around where the LPC curve passes 0, finally increase slightly right near the end. If it doesn't do this, there is probably a wrong value in there somewhere. The pitch and energy tables follow similar patterns but aren't the same since they never cross 0. The chirp table doesn't follow this pattern at all.
	*/

	/* Chip types based on die marks from decap:
	chip type
	|||||| rom number
	|||||| |||||
	VVVVVV VVVVV
	T0280A 0281  = 1978 speak & spell, unknown difference to below, assumed same? uses old chirp
	T0280B 0281A = 1979 speak & spell, also == TMS5100, uses old chirp
	?????? ????? (no decap; likely 'T0280D 0281D') = 1980 speak & spell, 1981 speak & spell compact, changed energy table, otherwise same as above, uses old chirp
	T0280F 2801A = 1980 speak & math, 1980 speak and read, uses old chirp
	T0280F 2802  = touch and tell, language translator; uses a unique chirp rom.
	?????? ????? = TMS5110
	T0280F 5110A = TMS5110AN2L


	*/

/* chip rom contents defines */
#define SUBTYPE_0281A           1
#define SUBTYPE_0281D           2
#define SUBTYPE_2801A           4
#define SUBTYPE_M58817          8
#define SUBTYPE_2802            16
#define SUBTYPE_5110            32
#define SUBTYPE_2501E           64
#define SUBTYPE_5220            128
#define SUBTYPE_PAT4335277      256
#define SUBTYPE_VLM5030         512

/* coefficient defines */
#define MAX_K                   10
#define MAX_SCALE_BITS          6
#define MAX_SCALE               (1<<MAX_SCALE_BITS)
#define MAX_CHIRP_SIZE          52

struct tms5100_coeffs
{
	int             subtype;
	int             num_k;
	int             energy_bits;
	int             pitch_bits;
	int             kbits[MAX_K];
	unsigned short  energytable[MAX_SCALE];
	unsigned short  pitchtable[MAX_SCALE];
	int             ktable[MAX_K][MAX_SCALE];
	INT16           chirptable[MAX_CHIRP_SIZE];
	INT8            interp_coeff[8];
};

/* common, shared coefficients */
/* energy */
#define TI_0280_PATENT_ENERGY \
		/* E  */\
		{   0,  0,  1,  1,  2,  3,  5,  7, \
			10, 15, 21, 30, 43, 61, 86, 0 },

#define TI_028X_LATER_ENERGY \
		/* E  */\
		{   0,  1,  2,  3,  4,  6,  8, 11, \
			16, 23, 33, 47, 63, 85,114, 0 },

/* pitch */
#define TI_0280_2801_PATENT_PITCH \
	/* P  */\
	{  0,   41,  43,  45,  47,  49,  51,  53,  \
		55,  58,  60,  63,  66,  70,  73,  76,  \
		79,  83,  87,  90,  94,  99,  103, 107,  \
		112, 118, 123, 129, 134, 140, 147, 153 },

#define TI_2802_PITCH \
	/* P */\
	{   0,  16,  18,  19,  21,  24,  26,  28,  \
		31,  35,  37,  42,  44,  47,  50,  53,  \
		56,  59,  63,  67,  71,  75,  79,  84,  \
		89,  94, 100, 106, 112, 126, 141, 150},

#define TI_5110_PITCH \
	/* P */\
	{   0,  15,  16,  17,  19,  21,  22,  25,  \
		26,  29,  32,  36,  40,  42,  46,  50,  \
		55,  60,  64,  68,  72,  76,  80,  84,  \
		86,  93, 101, 110, 120, 132, 144, 159},

#define TI_2501E_PITCH \
	/* P */\
	{   0,  14,  15,  16,  17,  18,  19,  20,  \
		21,  22,  23,  24,  25,  26,  27,  28,  \
		29,  30,  31,  32,  34,  36,  38,  40,  \
		41,  43,  45,  48,  49,  51,  54,  55,  \
		57,  60,  62,  64,  68,  72,  74,  76,  \
		81,  85,  87,  90,  96,  99, 103, 107, \
		112, 117, 122, 127, 133, 139, 145, 151, \
		157, 164, 171, 178, 186, 194, 202, 211},

#define TI_5220_PITCH \
	/* P */\
	{   0,  15,  16,  17,  18,  19,  20,  21,  \
		22,  23,  24,  25,  26,  27,  28,  29,  \
		30,  31,  32,  33,  34,  35,  36,  37,  \
		38,  39,  40,  41,  42,  44,  46,  48,  \
		50,  52,  53,  56,  58,  60,  62,  65,  \
		68,  70,  72,  76,  78,  80,  84,  86,  \
		91,  94,  98, 101, 105, 109, 114, 118, \
		122, 127, 132, 137, 142, 148, 153, 159},

/* LPC */
#define TI_0280_PATENT_LPC \
		/* K1  */\
		{ -501, -497, -493, -488, -480, -471, -460, -446,\
			-427, -405, -378, -344, -305, -259, -206, -148,\
			-86,  -21,   45,  110,  171,  227,  277,  320,\
			357,  388,  413,  434,  451,  464,  474,  498 },\
		/* K2  */\
		{ -349, -328, -305, -280, -252, -223, -192, -158,\
			-124,  -88,  -51,  -14,  23,    60,   97,  133,\
			167,  199,  230,  259,  286,  310,  333,  354,\
			372,  389,  404,  417,  429,  439,  449,  506 },\
		/* K3  */\
		{ -397, -365, -327, -282, -229, -170, -104, -36,\
			35,  104,  169,  228,  281,  326,  364, 396 },\
		/* K4  */\
		{ -369, -334, -293, -245, -191, -131, -67,  -1,\
			64,  128,  188,  243,  291,  332, 367, 397 },\
		/* K5  */\
		{ -319, -286, -250, -211, -168, -122, -74, -25,\
			24,   73,  121,  167,  210,  249, 285, 318 },\
		/* K6  */\
		{ -290, -252, -209, -163, -114,  -62,  -9,  44,\
			97,  147,  194,  238,  278,  313, 344, 371 },\
		/* K7  */\
		{ -291, -256, -216, -174, -128, -80, -31,  19,\
			69,  117,  163,  206,  246, 283, 316, 345 },\
		/* K8  */\
		{ -218, -133,  -38,  59,  152,  235, 305, 361 },\
		/* K9  */\
		{ -226, -157,  -82,  -3,   76,  151, 220, 280 },\
		/* K10 */\
		{ -179, -122,  -61,    1,   62,  123, 179, 231 },

#define TI_2801_2501E_LPC \
		/* K1  */\
		{ -501, -498, -495, -490, -485, -478, -469, -459,\
			-446, -431, -412, -389, -362, -331, -295, -253,\
			-207, -156, -102,  -45,   13,   70,  126,  179,\
			228,  272,  311,  345,  374,  399,  420,  437 },\
		/* K2  */\
		{ -376, -357, -335, -312, -286, -258, -227, -195,\
			-161, -124,  -87,  -49,  -10,   29,   68,  106,\
			143,  178,  212,  243,  272,  299,  324,  346,\
			366,  384,  400,  414,  427,  438,  448,  506 },\
		/* K3  */\
		{ -407, -381, -349, -311, -268, -218, -162, -102,\
			-39,   25,   89,  149,  206,  257,  302,  341 },\
		/* K4  */\
		{ -290, -252, -209, -163, -114,  -62,   -9,   44,\
			97,  147,  194,  238,  278,  313,  344,  371 },\
		/* K5  */\
		{ -318, -283, -245, -202, -156, -107,  -56,   -3,\
			49,  101,  150,  196,  239,  278,  313,  344 },\
		/* K6  */\
		{ -193, -152, -109,  -65,  -20,   26,   71,  115,\
			158,  198,  235,  270,  301,  330,  355,  377 },\
		/* K7  */\
		{ -254, -218, -180, -140,  -97,  -53,   -8,   36,\
			81,  124,  165,  204,  240,  274,  304,  332 },\
		/* K8  */\
		{ -205, -112,  -10,   92,  187,  269,  336,  387 },\
		/* K9  */\
		{ -249, -183, -110,  -32,   48,  126,  198,  261 }, /* on patents 4,403,965 and 4,946,391 the 4th entry is 0x3ED (-19) which is a typo of the correct value of 0x3E0 (-32)*/\
		/* K10 */\
		{ -190, -133,  -73,  -10,   53,  115,  173,  227 },

// below is the same as 2801/2501E above EXCEPT for K4 which is completely different.
#define TI_2802_LPC \
		/* K1 */\
		{ -501, -498, -495, -490, -485, -478, -469, -459,\
			-446, -431, -412, -389, -362, -331, -295, -253,\
			-207, -156, -102,  -45,   13,   70,  126,  179,\
			228,  272,  311,  345,  374,  399,  420,  437},\
		/* K2 */\
		{ -376, -357, -335, -312, -286, -258, -227, -195,\
			-161, -124,  -87,  -49,  -10,   29,   68,  106,\
			143,  178,  212,  243,  272,  299,  324,  346,\
			366,  384,  400,  414,  427,  438,  448,  506},\
		/* K3 */\
		{ -407, -381, -349, -311, -268, -218, -162, -102,\
			-39,   25,   89,  149,  206,  257,  302,  341},\
		/* K4 */\
		{ -289, -248, -202, -152,  -98,  -43,   14,   71,\
			125,  177,  225,  269,  307,  341,  371,  506},\
		/* K5 */\
		{ -318, -283, -245, -202, -156, -107,  -56,   -3,\
			49,  101,  150,  196,  239,  278,  313,  344},\
		/* K6 */\
		{ -193, -152, -109,  -65,  -20,   26,   71,  115,\
			158,  198,  235,  270,  301,  330,  355,  377},\
		/* K7 */\
		{ -254, -218, -180, -140,  -97,  -53,   -8,   36,\
			81,  124,  165,  204,  240,  274,  304,  332},\
		/* K8 */\
		{ -205, -112,  -10,   92,  187,  269,  336,  387},\
		/* K9 */\
		{ -249, -183, -110,  -32,   48,  126,  198,  261},\
		/* K10 */\
		{ -190, -133,  -73,  -10,   53,  115,  173,  227},

#define TI_5110_5220_LPC \
		/* K1  */\
		{ -501, -498, -497, -495, -493, -491, -488, -482,\
			-478, -474, -469, -464, -459, -452, -445, -437,\
			-412, -380, -339, -288, -227, -158,  -81,   -1,\
			80,  157,  226,  287,  337,  379,  411,  436 },\
		/* K2  */\
		{ -328, -303, -274, -244, -211, -175, -138,  -99,\
			-59,  -18,   24,   64,  105,  143,  180,  215,\
			248,  278,  306,  331,  354,  374,  392,  408,\
			422,  435,  445,  455,  463,  470,  476,  506 },\
		/* K3  */\
		{ -441, -387, -333, -279, -225, -171, -117,  -63,\
			-9,   45,   98,  152,  206,  260,  314,  368  },\
		/* K4  */\
		{ -328, -273, -217, -161, -106,  -50,    5,   61,\
			116,  172,  228,  283,  339,  394,  450,  506  },\
		/* K5  */\
		{ -328, -282, -235, -189, -142,  -96,  -50,   -3,\
			43,   90,  136,  182,  229,  275,  322,  368  },\
		/* K6  */\
		{ -256, -212, -168, -123,  -79,  -35,   10,   54,\
			98,  143,  187,  232,  276,  320,  365,  409  },\
		/* K7  */\
		{ -308, -260, -212, -164, -117,  -69,  -21,   27,\
			75,  122,  170,  218,  266,  314,  361,  409  },\
		/* K8  */\
		{ -256, -161,  -66,   29,  124,  219,  314,  409  },\
		/* K9  */\
		{ -256, -176,  -96,  -15,   65,  146,  226,  307  },\
		/* K10 */\
		{ -205, -132,  -59,   14,   87,  160,  234,  307  },

/* chirp */
#define TI_0280_PATENT_CHIRP \
	/* Chirp table */\
	{   0x00, 0x2a, 0xd4, 0x32, 0xb2, 0x12, 0x25, 0x14,\
		0x02, 0xe1, 0xc5, 0x02, 0x5f, 0x5a, 0x05, 0x0f,\
		0x26, 0xfc, 0xa5, 0xa5, 0xd6, 0xdd, 0xdc, 0xfc,\
		0x25, 0x2b, 0x22, 0x21, 0x0f, 0xff, 0xf8, 0xee,\
		0xed, 0xef, 0xf7, 0xf6, 0xfa, 0x00, 0x03, 0x02,\
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00 },

// almost, but not exactly the same as the patent chirp above (25 bits differ)
#define TI_2801_CHIRP \
	/* Chirp table */\
	{   0x00, 0x2b, 0xd4, 0x33, 0xb3, 0x12, 0x25, 0x14,\
		0x02, 0xe2, 0xc6, 0x03, 0x60, 0x5b, 0x05, 0x0f,\
		0x26, 0xfc, 0xa6, 0xa5, 0xd6, 0xdd, 0xdd, 0xfd,\
		0x25, 0x2b, 0x23, 0x22, 0x0f, 0xff, 0xf8, 0xef,\
		0xed, 0xef, 0xf7, 0xf7, 0xfa, 0x01, 0x04, 0x03,\
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00 },

#define TI_2802_CHIRP \
	/* Chirp table */\
	{   0x00, 0xa5, 0xbd, 0xee, 0x34, 0x73, 0x7e, 0x3d,\
		0xe8, 0xea, 0x34, 0x24, 0xd1, 0x01, 0x13, 0xc3,\
		0x0c, 0xd2, 0xe7, 0xdd, 0xd9, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00 },

#define TI_LATER_CHIRP \
	/* Chirp table */\
	{   0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50,\
		0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13,\
		0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00 },

/* Interpolation Table */
#define TI_INTERP \
	/* interpolation shift coefficients */\
	{ 0, 3, 3, 3, 2, 2, 1, 1 }

/* TMS5100/TMC0281:
   (Die revs A, B; 1977?-1981?)
   The TMS5100NL was decapped and imaged by digshadow in April, 2013.
    The LPC table is verified to match the decap.
    It also matches the intended contents of US Patent 4,209,836 and several others.
    The chirp table is verified to match the decap, and also matches the patents.
   In April, 2013, digshadow decapped a TMS5100 chip from 1980: http://siliconpr0n.org/map/ti/tms5100nl/
    The TMS5100 had the die markings: "0281 B  281A"
   In December 2014, Sean riddle decapped a TMC0281 chip from 1978 from an early speak
   and spell.
    The TMC0281 die had the die markings "0281 A  281"
    The chirp table matches what digshadow had decapped earlier.
    The LPC table hasn't been fully typed yet.
   Digitally dumped via PROMOUT by PlgDavid in 2014 for verification.
*/
static const struct tms5100_coeffs T0280B_0281A_coeff =
{
	/* subtype */
	SUBTYPE_0281A,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_0280_PATENT_ENERGY
	TI_0280_2801_PATENT_PITCH
	{
	TI_0280_PATENT_LPC
	},
	TI_0280_PATENT_CHIRP
	TI_INTERP
};

/* TMS5110A/TMC0281D:
   This chip is used on the later speak & spell, and speak & spell compact;
   The energy table differs from the original tmc0281/tms5100, as does the interpolation behavior,
   which is the 'alternate' behavior.
   The chips have datecodes in the 1983-1984 range, probably 1982 also.
   Digitally dumped via PROMOUT by PlgDavid in 2014
   */
static const struct tms5100_coeffs T0280D_0281D_coeff =
{
	/* subtype */
	SUBTYPE_0281D,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_0280_2801_PATENT_PITCH
	{
	TI_0280_PATENT_LPC
	},
	TI_0280_PATENT_CHIRP
	TI_INTERP
};

/* TMC0280/CD2801:
   Used in the Speak & Math, Speak & Read, and Language Translator
   Decapped by Digshadow in 2014 http://siliconpr0n.org/map/ti/tmc0280fnl/
   Digitally dumped via PROMOUT by PlgDavid in 2014
   The coefficients are exactly the same as the TMS5200.
   The coefficients also come from US Patents 4,403,965 and 4,946,391 (with
   one typo in the patent).
   The chirp table is very slightly different from the 4,209,836 patent one,
   but matches the table in the 4,403,965 and 4,946,391 patents.
   The Mitsubishi M58817 also seems to work best with these coefficients, so
   it is possible the engineers of that chip copied them from the TI patents.
   ***TODO: there are 2 versions of this chip, and the interpolation
      behavior between the two differs slightly:
   * TMC0280NLP // CD2801 with datecodes around 1980 has the same
     interpolation inhibit behavior as 5100/TMC0281 on unvoiced->silent
     transition.
   * CD2801A-NL with datecodes around 1982 have the 'alternate behavior'
   */
static const struct tms5100_coeffs T0280F_2801A_coeff =
{
	/* subtype */
	SUBTYPE_2801A,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_0280_2801_PATENT_PITCH
	{
	TI_2801_2501E_LPC
	},
	TI_2801_CHIRP
	TI_INTERP
};

/* Mitsubishi M58817
The Mitsubishi M58817 seems to have (partly?) copied the coefficients from the
TMC0280/CD2801 above, but has some slight differences to it within the chip:
the main accumulator seems to have 1 extra bit and the digital values are
tapped 1 bit higher than on the TI chips. This is emulated within tms5110.c
   */
static const struct tms5100_coeffs M58817_coeff =
{
	/* subtype */
	SUBTYPE_M58817,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_0280_2801_PATENT_PITCH
	{
	TI_2801_2501E_LPC
	},
	TI_2801_CHIRP
	TI_INTERP
};

/* CD2802:
   (1984 era?)
   Used in Touch and Tell only (and Vocaid?), this chip has a unique pitch, LPC and chirp table.
   Has the 'alternate' interpolation behavior.
   Digitally dumped via PROMOUT by PlgDavid in 2014
   Decapped by Sean Riddle in 2015
   */
static const struct tms5100_coeffs T0280F_2802_coeff =
{
	/* subtype */
	SUBTYPE_2802,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_2802_PITCH
	{
	TI_2802_LPC
	},
	TI_2802_CHIRP
	TI_INTERP
};

/* TMS5110A:
   (1984-90 era? early chips may be called TMS5110C; later chips past 1988 or so may be called TSP5110A)
   The TMS5110A LPC coefficients were originally read from an actual TMS5110A
   chip by Jarek Burczynski using the PROMOUT pin, later verified/redumped
   by PlgDavid.
   NullMoogleCable decapped a TMS5110AN2L in 2015: http://wtfmoogle.com/wp-content/uploads/2015/03/0317_1.jpg
   which was used to verify the chirp table.
   The slightly older but otherwise identical TMS5111NLL was decapped and imaged by digshadow in April, 2013.
   The die is marked "TMS5110AJ"
   The LPC table is verified from decap to match the values from Jarek and PlgDavid's PROMOUT dumps of the TMS5110.
   The LPC table matches that of the TMS5220.
   It uses the 'newer' 5200-style chirp table.
   It has the 'alternate' interpolation behavor (tested on 5110a; 5111 behavior is unknown)
*/
static const struct tms5100_coeffs tms5110a_coeff =
{
	/* subtype */
	SUBTYPE_5110,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_5110_PITCH
	{
	TI_5110_5220_LPC
	},
	TI_LATER_CHIRP
	TI_INTERP
};

/* The following coefficients come from US Patent 4,335,277 and 4,581,757.
However, the K10 row of coefficients are entirely missing from both of those
patents.
The K values don't match the values read from any of the TI chips so far, but
might match some other undiscovered chip? Or may be complete garbage put as a red
herring in the patent?
*/
	// k* is followed by d if done transcription, c if checked for derivative aberrations
static const struct tms5100_coeffs pat4335277_coeff =
{
	/* subtype */
	SUBTYPE_PAT4335277,
	10,
	4,
	6,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_2501E_PITCH
	{
		/* K1dc  */
		{ -507, -505, -503, -501, -497, -493, -488, -481,
			-473, -463, -450, -434, -414, -390, -362, -328,
			-288, -242, -191, -135,  -75,  -13,   49,  110,
			168,  221,  269,  311,  348,  379,  404,  426 },
		/* K2dc  */
		{ -294, -266, -235, -202, -167, -130,  -92,  -52,
			-12,   28,   68,  108,  145,  182,  216,  248,
			278,  305,  330,  352,  372,  390,  406,  420,
			432,  443,  453,  461,  468,  474,  479,  486 },
		/* K3dc  */
		{ -449, -432, -411, -385, -354, -317, -273, -223,
			-167, -107,  -43,   22,   87,  148,  206,  258 },
		/* K4dc (first 4-5 values are probably wrong but close) */
		{ -321, -270, -220, -157,  -97,  -40,   25,   89,
			150,  207,  259,  304,  343,  376,  403,  425 },
		/* K5dc  */
		{ -373, -347, -318, -284, -247, -206, -162, -115,
			-65,  -15,   36,   86,  135,  181,  224,  263 },
		/* K6dc  */
		{ -213, -176, -137,  -96,  -54,  -11,   33,   75,
			117,  157,  195,  231,  264,  294,  322,  347 },
		/* K7dc  */
		{ -294, -264, -232, -198, -161, -122,  -82,  -41,
				1,   43,   84,  125,  163,  200,  234,  266 },
		/* K8dc  */
		{ -195, -117,  -32,   54,  137,  213,  279,  335 },
		/* K9dc  */
		{ -122,  -55,   15,   83, 149,  210,  264,  311  },
		/* K10  - this was entirely missing from the patent, and I've simply copied the real TMS5220 one, which is wrong */
		{ -205, -132,  -59,   14,  87,  160,  234,  307  },
	},
	TI_0280_PATENT_CHIRP
	TI_INTERP
};

/* TMS5200/CD2501E
 (1979-1983 era)
The TMS5200NL was decapped and imaged by digshadow in March, 2013.
It is equivalent to the CD2501E (internally: "TMC0285") chip used
 on the TI 99/4(A) speech module.
The LPC table is verified to match the decap.
 (It was previously dumped with PROMOUT which matches as well)
The chirp table is verified to match the decap. (sum = 0x3da)
Note that the K coefficients are VERY different from the coefficients given
 in the US 4,335,277 patent, which may have been for some sort of prototype or
 otherwise intentionally scrambled. The energy and pitch tables, however, are
 identical to that patent.
Also note, that the K coefficients are identical to the coefficients from the
 CD2801 (which itself is almost identical to the CD2802).
NOTE FROM DECAP: immediately to the left of each of the K1,2,3,4,5,and 6
 coefficients in the LPC rom are extra columns containing the constants
 -510, -502, 313, 318, or in hex 0x202, 0x20A, 0x139, 0x13E.
 Those EXACT constants DO appear (rather nonsensically) on the lpc table in US
 patent 4,335,277. They are likely related to the multiplicative interpolator
 described in us patent 4,419,540; whether the 5200/2501E and the 5220 or 5220C
 actually implement this interpolator or not is unclear. This interpolator
 seems intended for chips with variable frame rate, so it may only exist
 on the TMS/TSP5220C and CD2501ECD.
*/

static const struct tms5100_coeffs T0285_2501E_coeff =
{
	/* subtype */
	SUBTYPE_2501E,
	10,
	4,
	6,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_2501E_PITCH
	{
	TI_2801_2501E_LPC
	},
	TI_LATER_CHIRP
	TI_INTERP
};

/* TMS5220/5220C:
(1983 era for 5220, 1986-1992 era for 5220C; 5220C may also be called TSP5220C)
The TMS5220NL was decapped and imaged by digshadow in April, 2013.
The LPC table table is verified to match the decap.
The chirp table is verified to match the decap. (sum = 0x3da)
Note that all the LPC K* values match the TMS5110a table (as read via PROMOUT)
exactly.
The TMS5220CNL was decapped and imaged by digshadow in April, 2013.
The LPC table table is verified to match the decap and exactly matches TMS5220NL.
The chirp table is verified to match the decap. (sum = 0x3da)
*/
static const struct tms5100_coeffs tms5220_coeff =
{
	/* subtype */
	SUBTYPE_5220,
	10,
	4,
	6,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 },
	TI_028X_LATER_ENERGY
	TI_5220_PITCH
	{
	TI_5110_5220_LPC
	},
	TI_LATER_CHIRP
	TI_INTERP
};

/* The following Sanyo VLM5030 coefficients are derived from decaps of the chip
done by ogoun, plus image stitching done by John McMaster. The organization of
coefficients beyond k2 is derived from work by Tatsuyuki Satoh.
The actual coefficient rom on the chip die has 5 groups of bits only:
Address |   K1A   |   K1B   |   K2   | Energy | Pitch |
Decoder |   K1A   |   K1B   |   K2   | Energy | Pitch |
K1A, K1B and K2 are 10 bits wide, 32 bits long each.
Energy and pitch are both 7 bits wide, 32 bits long each.
K1A holds odd values of K1, K1B holds even values.
K2 holds values for K2 only
K3 and K4 are actually the table index values <<6
K5 thru K10 are actually the table index values <<7
The concept of only having non-binary weighted reflection coefficients for the
first two k stages is mentioned in Markel & Gray "Linear Prediction of Speech"
and in Thomas Parsons' "Voice and Speech Processing"
 */
static const struct tms5100_coeffs vlm5030_coeff =
{
	/* subtype */
	SUBTYPE_VLM5030,
	10,
	5,
	5,
	{ 6, 5, 4, 4, 3, 3, 3, 3, 3, 3 },
	/* E   */
	{ 0,  1,  2,  3,  5,  6,  7,  9,
		11, 13, 15, 17, 19, 22, 24, 27,
		31, 34, 38, 42, 47, 51, 57, 62,
		68, 75, 82, 89, 98,107,116,127},
	/* P   */
	{   0,  21,  22,  23,  24,  25,  26,  27,
		28,  29,  31,  33,  35,  37,  39,  41,
		43,  45,  49,  53,  57,  61,  65,  69,
		73,  77,  85,  93, 101, 109, 117, 125 },
	{
		/* K1  */
		/* (NOTE: the order of each table is correct, despite that the index MSb
		looks backwards) */
		{  390, 403, 414, 425, 434, 443, 450, 457,
			463, 469, 474, 478, 482, 485, 488, 491,
			494, 496, 498, 499, 501, 502, 503, 504,
			505, 506, 507, 507, 508, 508, 509, 509,
			-390,-376,-360,-344,-325,-305,-284,-261,
			-237,-211,-183,-155,-125, -95, -64, -32,
				0,  32,  64,  95, 125, 155, 183, 211,
			237, 261, 284, 305, 325, 344, 360, 376 },
		/* K2  */
		{    0,  50, 100, 149, 196, 241, 284, 325,
			362, 396, 426, 452, 473, 490, 502, 510,
				0,-510,-502,-490,-473,-452,-426,-396, /* entry 16(0x10) either has some special function, purpose unknown, or is a manufacturing error and should have been -512 */
			-362,-325,-284,-241,-196,-149,-100, -50 },
		/* K3  */
		/*{    0, 100, 196, 284, 362, 426, 473, 502,
		  -510,-490,-452,-396,-325,-241,-149, -50 },*/
		{    0, 64, 128, 192, 256, 320, 384, 448,
			-512,-448,-384,-320,-256,-192,-128, -64 },
		/* K4  */
		/*{    0, 100, 196, 284, 362, 426, 473, 502,
		  -510,-490,-452,-396,-325,-241,-149, -50 },*/
		{    0, 64, 128, 192, 256, 320, 384, 448,
			-512,-448,-384,-320,-256,-192,-128, -64 },
		/* K5  */
		{    0, 128, 256, 384,-512,-384,-256,-128 },
		/* K6  */
		{    0, 128, 256, 384,-512,-384,-256,-128 },
		/* K7  */
		{    0, 128, 256, 384,-512,-384,-256,-128 },
		/* K8  */
		{    0, 128, 256, 384,-512,-384,-256,-128 },
		/* K9  */
		{    0, 128, 256, 384,-512,-384,-256,-128 },
		/* K10 */
		/*{    0, 196, 362, 473,-510,-452,-325,-149 },*/
		{    0, 128, 256, 384,-512,-384,-256,-128 },
	},
	/* Chirp table */
	{   0,127,127,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0 },
	/* interpolation coefficients */
	{ 3, 3, 3, 2, 2, 1, 1, 0 }
};
