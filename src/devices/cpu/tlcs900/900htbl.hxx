// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

TLCS-900/H instruction set

*******************************************************************/


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_80[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 14 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 14 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDI, 0, 0, 8 }, { &tlcs900h_device::op_LDIR, 0, 0, 8 }, { &tlcs900h_device::op_LDD, 0, 0, 8 }, { &tlcs900h_device::op_LDDR, 0, 0, 8 },
	{ &tlcs900h_device::op_CPI, 0, 0, 6 }, { &tlcs900h_device::op_CPIR, 0, 0, 7 }, { &tlcs900h_device::op_CPD, 0, 0, 6 }, { &tlcs900h_device::op_CPDR, 0, 0, 7 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RLBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 6 }, { &tlcs900h_device::op_SRABM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_88[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 14 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 14 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RLBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 6 }, { &tlcs900h_device::op_SRABM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_90[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDIW, 0, 0, 8 }, { &tlcs900h_device::op_LDIRW, 0, 0, 8 }, { &tlcs900h_device::op_LDDW, 0, 0, 8 }, { &tlcs900h_device::op_LDDRW, 0, 0, 8 },
	{ &tlcs900h_device::op_CPIW, 0, 0, 6 }, { &tlcs900h_device::op_CPIRW, 0, 0, 7 }, { &tlcs900h_device::op_CPDW, 0, 0, 6 }, { &tlcs900h_device::op_CPDRW, 0, 0, 7 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RLWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_98[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RLWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 6 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 4 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 6 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 14 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 14 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 13 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 11 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 16 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 19 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 6 }, { &tlcs900h_device::op_RLBM, p_M, 0, 6 }, { &tlcs900h_device::op_RRBM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 6 }, { &tlcs900h_device::op_SRABM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBRI, p_R, p_I8, 3 },
	{ &tlcs900h_device::op_PUSHBR, p_R, 0, 4 }, { &tlcs900h_device::op_POPBR, p_R, 0, 5 }, { &tlcs900h_device::op_CPLBR, p_R, 0, 2 }, { &tlcs900h_device::op_NEGBR, p_R, 0, 2 },
	{ &tlcs900h_device::op_MULBRI, p_R, p_I8, 12 }, { &tlcs900h_device::op_MULSBRI, p_R, p_I8, 10 }, { &tlcs900h_device::op_DIVBRI, p_R, p_I8, 15 }, { &tlcs900h_device::op_DIVSBRI, p_R, p_I8, 18 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DAABR, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DJNZB, p_R, p_D8, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_ANDCFBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_ORCFBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_XORCFBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_LDCFBIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_STCFBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_ANDCFBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_ORCFBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_XORCFBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_LDCFBRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_STCFBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCBRR, p_CR8, p_R, 3 }, { &tlcs900h_device::op_LDCBRR, p_R, p_CR8, 3 },
	{ &tlcs900h_device::op_RESBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SETBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_CHGBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_BITBIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_TSETBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 },
	{ &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 11 },
	{ &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 },
	{ &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 9 },
	{ &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 },
	{ &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 15 },
	{ &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 },
	{ &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 2 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 },
	{ &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 2 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 },
	{ &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 3 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ADDBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_ADCBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_SUBBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_SBCBRI, p_R, p_I8, 3 },
	{ &tlcs900h_device::op_ANDBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_XORBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_ORBRI, p_R, p_I8, 3 }, { &tlcs900h_device::op_CPBRI, p_R, p_I8, 3 },
	{ &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 2 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_RLCBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRCBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RLBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRBIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_SLABIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRABIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SLLBIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRLBIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 2 },
	{ &tlcs900h_device::op_RLCBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRCBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RLBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRBRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_SLABRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRABRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SLLBRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRLBRR, p_A, p_R, 3 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 16 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 14 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 24 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 27 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 6 }, { &tlcs900h_device::op_RLWM, p_M, 0, 6 }, { &tlcs900h_device::op_RRWM, p_M, 0, 6 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 6 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 6 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 4 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_PUSHWR, p_R, 0, 4 }, { &tlcs900h_device::op_POPWR, p_R, 0, 5 }, { &tlcs900h_device::op_CPLWR, p_R, 0, 2 }, { &tlcs900h_device::op_NEGWR, p_R, 0, 2 },
	{ &tlcs900h_device::op_MULWRI, p_R, p_I16, 15 }, { &tlcs900h_device::op_MULSWRI, p_R, p_I16, 13 }, { &tlcs900h_device::op_DIVWRI, p_R, p_I16, 23 }, { &tlcs900h_device::op_DIVSWRI, p_R, p_I16, 26 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_BS1FRR, p_A, p_R, 3 }, { &tlcs900h_device::op_BS1BRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_EXTZWR, p_R, 0, 3 }, { &tlcs900h_device::op_EXTSWR, p_R, 0, 3 },
	{ &tlcs900h_device::op_PAAWR, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_MIRRW, p_R, 0, 3 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_MULAR, p_R, 0, 19 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DJNZW, p_R, p_D8, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_ANDCFWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_ORCFWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_XORCFWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_LDCFWIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_STCFWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_ANDCFWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_ORCFWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_XORCFWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_LDCFWRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_STCFWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCWRR, p_CR16, p_R, 3 }, { &tlcs900h_device::op_LDCWRR, p_R, p_CR16, 3 },
	{ &tlcs900h_device::op_RESWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SETWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_CHGWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_BITWIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_TSETWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_MINC1, p_I16, p_R, 5 }, { &tlcs900h_device::op_MINC2, p_I16, p_R, 5 }, { &tlcs900h_device::op_MINC4, p_I16, p_R, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_MDEC1, p_I16, p_R, 4 }, { &tlcs900h_device::op_MDEC2, p_I16, p_R, 4 }, { &tlcs900h_device::op_MDEC4, p_I16, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 },
	{ &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 14 },
	{ &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 },
	{ &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 12 },
	{ &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 },
	{ &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 23 },
	{ &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 },
	{ &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 26 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 2 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 },
	{ &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 2 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 },
	{ &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 3 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ADDWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_ADCWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_SUBWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_SBCWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_ANDWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_XORWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_ORWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 2 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_RLCWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRCWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RLWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRWIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_SLAWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRAWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SLLWIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRLWIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 2 },
	{ &tlcs900h_device::op_RLCWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRCWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RLWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRWRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_SLAWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRAWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SLLWRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRLWRR, p_A, p_R, 3 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDLRI, p_R, p_I32, 6 },
	{ &tlcs900h_device::op_PUSHLR, p_R, 0, 6 }, { &tlcs900h_device::op_POPLR, p_R, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LINK, p_R, p_I16, 8 }, { &tlcs900h_device::op_UNLK, p_R, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_EXTZLR, p_R, 0, 3 }, { &tlcs900h_device::op_EXTSLR, p_R, 0, 3 },
	{ &tlcs900h_device::op_PAALR, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCLRR, p_CR32, p_R, 3 }, { &tlcs900h_device::op_LDCLRR, p_R, p_CR32, 3 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 2 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 },
	{ &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 2 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ADDLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_ADCLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_SUBLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_SBCLRI, p_R, p_I32, 6 },
	{ &tlcs900h_device::op_ANDLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_XORLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_ORLRI, p_R, p_I32, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I32, 6 },
	{ &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 },
	{ &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 2 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_RLCLIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRCLIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RLLIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_RRLIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_SLALIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRALIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SLLLIR, p_I8, p_R, 3 }, { &tlcs900h_device::op_SRLLIR, p_I8, p_R, 3 },
	{ &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 2 },
	{ &tlcs900h_device::op_RLCLRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRCLRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RLLRR, p_A, p_R, 3 }, { &tlcs900h_device::op_RRLRR, p_A, p_R, 3 },
	{ &tlcs900h_device::op_SLALRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRALRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SLLLRR, p_A, p_R, 3 }, { &tlcs900h_device::op_SRLLRR, p_A, p_R, 3 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 6 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 6 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 7 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 7 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_NOP, 0, 0, 2 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_PUSHWR, p_SR, 0, 3 }, { &tlcs900h_device::op_POPWSR, p_SR, 0, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_HALT, 0, 0, 6 }, { &tlcs900h_device::op_EI, p_I8, 0, 3 }, { &tlcs900h_device::op_RETI, 0, 0, 12 },
	{ &tlcs900h_device::op_LDBMI, p_M8, p_I8, 5 }, { &tlcs900h_device::op_PUSHBI, p_I8, 0, 4 }, { &tlcs900h_device::op_LDWMI, p_M8, p_I16, 6 }, { &tlcs900h_device::op_PUSHWI, p_I16, 0, 5 },
	{ &tlcs900h_device::op_INCF, 0, 0, 2 }, { &tlcs900h_device::op_DECF, 0, 0, 2 }, { &tlcs900h_device::op_RET, 0, 0, 9 }, { &tlcs900h_device::op_RETD, p_I16, 0, 11 },
	{ &tlcs900h_device::op_RCF, 0, 0, 2 }, { &tlcs900h_device::op_SCF, 0, 0, 2 }, { &tlcs900h_device::op_CCF, 0, 0, 2 }, { &tlcs900h_device::op_ZCF, 0, 0, 2 },
	{ &tlcs900h_device::op_PUSHBR, p_A, 0, 3 }, { &tlcs900h_device::op_POPBR, p_A, 0, 4 }, { &tlcs900h_device::op_EXBRR, p_F, p_F, 2 }, { &tlcs900h_device::op_LDF, p_I8, 0, 2 },
	{ &tlcs900h_device::op_PUSHBR, p_F, 0, 3 }, { &tlcs900h_device::op_POPBR, p_F, 0, 4 }, { &tlcs900h_device::op_JPI, p_I16, 0, 5 }, { &tlcs900h_device::op_JPI, p_I24, 0, 6 },
	{ &tlcs900h_device::op_CALLI, p_I16, 0, 9 }, { &tlcs900h_device::op_CALLI, p_I24, 0, 10 }, { &tlcs900h_device::op_CALR, p_D16, 0, 10 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 },
	{ &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 },
	{ &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 2 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 2 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 },
	{ &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 },
	{ &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 },
	{ &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 },
	{ &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 },
	{ &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 },
	{ &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 },
	{ &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 },

	/* A0 - BF */
	{ &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 },
	{ &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 },
	{ &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 },
	{ &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 },
	{ &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 },
	{ &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 },
	{ &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 },
	{ &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 },

	/* C0 - DF */
	{ &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 },
	{ &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 },
	{ &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },

	/* E0 - FF */
	{ &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 },
	{ &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 },
	{ &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::op_LDX, 0, 0, 8 },
	{ &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 },
	{ &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 }, { &tlcs900h_device::op_SWI, p_I3, 0, 19 }
};
