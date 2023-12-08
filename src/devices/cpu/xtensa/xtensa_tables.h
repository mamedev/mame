// license:BSD-3-Clause
// copyright-holders:AJR

namespace {

static const char *const s_special_regs[256] =
{
	"lbeg", "lend", "lcount", // Loop Option (0-2)
	"sar", // Core Architecture (3)
	"br", // Boolean Option (4)
	"litbase", // Extended L32R Option (5)
	"", "", "", "", "", "",
	"scompare1", // Conditional Store Option (12)
	"", "", "",
	"acclo", "acchi", // MAC16 Option (16-17)
	"", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"m0", "m1", "m2", "m3", // MAC16 Option (32-35)
	"", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"WindowBase", "WindowStart", // Windowed Register Option (72-73)
	"", "", "", "", "", "", "", "", "",
	"ptevaddr", // MMU Option (83)
	"", "", "", "", "",
	"mmid", // Trace Port Option (89)
	"rasid", "itlbcfg", "dtlbcfg", // MMU Option (90-92)
	"", "", "",
	"ibreakenable", // Debug Option (96)
	"",
	"cacheattr", // XEA1 Only (98)
	"atomctl", // Conditional Store Option (99)
	"", "", "", "",
	"ddr", // Debug Option (104)
	"",
	"mepc", "meps", "mesave", "mesr", "mecr", "mevaddr", // Memory ECC/Parity Option (106-111)
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"ibreaka0", "ibreaka1", // Debug Option (128-129)
	"", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"dbreaka0", "dbreaka1", // Debug Option (144-145)
	"", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"dbreakc0", "dbreakc1", // Debug Option (160-161)
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"epc1", // Exception Option (177)
	"epc2", "epc3", "epc4", "epc5", "epc6", "epc7", // High-Priority Interrupt Option (178-183)
	"", "", "", "", "", "", "", "",
	"depc", // Exception Option (192)
	"",
	"eps2", "eps3", "eps4", "eps5", "eps6", "eps7", // High-Priority Interrupt Option (194-199)
	"", "", "", "", "", "", "", "", "",
	"excsave1", // Exception Option (209)
	"excsave2", "excsave3", "excsave4", "excsave5", "excsave6", "excsave7", // High-Priority Interrupt Option (210-215)
	"", "", "", "", "", "", "", "",
	"cpenable", // Coprocessor Option (224)
	"",
	"intset", "intclr", "intenable", // Interrupt Option (226-228)
	"",
	"ps", // various options (230)
	"vecbase", // Relocatable Vector Option (231)
	"exccause", // Exception Option (232)
	"debugcause", // Debug Option (233)
	"ccount", // Timer Interrupt Option (234)
	"prid", // Processor ID Option (235)
	"icount", "icountlevel", // Debug Option (236-237)
	"excvaddr", // Exception Option (238)
	"",
	"ccompare0", "ccompare1", "ccompare2", // Timer Interrupt Option (240-242)
	"",
	"misc0", "misc1", "misc2", "misc3", // Miscellaneous Special Registers Option (244-247)
	"", "", "", "", "", "", "", ""
};

static const char *const s_st1_ops[16] =
{
	"ssr", "ssl",
	"ssa8l", "ssa8b",
	"ssai", "",
	"rer", "wer",
	"rotw", "",
	"", "",
	"", "",
	"nsa", "nsau"
};

static const char *const s_tlb_ops[16] =
{
	"", "", "", "ritlb0",
	"iitlb", "pitlb", "witlb", "ritlb1",
	"", "", "", "rdtlb0",
	"idtlb", "pdtlb", "wdtlb", "rdtlb1"
};

static const char *const s_rst2_ops[16] =
{
	"andb", "andbc", "orb", "orbc", "xorb", "", "", "",
	"mull", "", "muluh", "mulsh",
	"quou", "quos", "remu", "rems"
};

static const char *const s_rst3_ops[16] =
{
	"rsr", "wsr",
	"sext", "clamps",
	"min", "max",
	"minu", "maxu",
	"moveqz", "movnez",
	"movltz", "movgez",
	"movf", "movt",
	"rur", "wur"
};

static const char *const s_fp0_ops[16] =
{
	"add.s", "sub.s", "mul.s", "",
	"madd.s", "msub.s", "", "",
	"round.s", "trunc.s", "floor.s", "ceil.s",
	"float.s", "ufloat.s", "utrunc.s", ""
};

static const char *const s_fp1_ops[16] =
{
	"", "un.s",
	"oeq.s", "ueq.s",
	"olt.s", "ult.s",
	"ole.s", "ule.s",
	"moveqz.s", "movltz.s",
	"movltz.s", "movgez.s",
	"movf.s", "movt.s",
	"", ""
};

static const char *const s_lsai_ops[16] =
{
	"l8ui", "l16ui", "l32i", "",
	"s8i", "s16i", "s32i", "",
	"", "l16si", "movi", "l32ai",
	"addi", "addmi", "s32c1i", "s32ri"
};

static const char *const s_cache_ops[16] =
{
	"dpfr", "dpfw",
	"dpfro", "dpfwo",
	"dhwb", "dhwbi",
	"dhi", "dii",
	"", "",
	"", "",
	"ipf", "",
	"ihi", "iii"
};

static const char *const s_lsci_ops[4] =
{
	"lsi", "ssi", "lsiu", "ssiu"
};

static const char *const s_mac16_ops[4] =
{
	"umul", "mul", "mula", "muls"
};

static const char *const s_mac16_half[4] =
{
	"ll", "hl", "lh", "hh"
};

static const char *const s_bz_ops[4] =
{
	"beqz", "bnez", "bltz", "bgez"
};

static const char *const s_bi0_ops[4] =
{
	"beqi", "bnei", "blti", "bgei"
};

static const s32 s_b4const[16] =
{
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 16, 32, 64, 128, 256
};

static const u32 s_b4constu[16] =
{
	32768, 65536, 2, 3, 4, 5, 6, 7, 8, 10, 12, 16, 32, 64, 128, 256
};

static const char *const s_b_ops[16] =
{
	"bnone", "beq", "blt", "bltu", "ball", "bbc", "bbci", "bbci",
	"bany", "bne", "bge", "bgeu", "bnall", "bbs", "bbsi", "bbsih"
};

static std::string format_imm(u32 imm)
{
	if (s32(imm) < 0)
	{
		if (s32(imm < -9))
		{
			return util::string_format("-0x%X", -imm);
		}
		else
		{
			return util::string_format("-%X", -imm);
		}
	}
	else
	{
		if (imm > 9)
		{
			return util::string_format("0x%X", imm);
		}
		else
		{
			return util::string_format("%X", imm);
		}
	}
}


static std::string special_reg(u8 n, bool wsr)
{
	if (n == 226 && !wsr)
		return "interrupt";

	const char *s = s_special_regs[n];
	if (s[0] == '\0')
		return util::string_format("s%u", n);
	else
		return s;
}

} // anonymous namespace
