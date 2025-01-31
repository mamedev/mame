// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VAX disassembler

    TODO:
    * Floating point literal formats
    * Vector instruction notation

***************************************************************************/

#include "emu.h"
#include "vaxdasm.h"

vax_disassembler::vax_disassembler()
	: util::disasm_interface()
{
}

u32 vax_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

using namespace std::literals;
using mode = vax_disassembler::mode;

static constexpr bool is_address_mode(mode m)
{
	return m == mode::ab || m == mode::aw || m == mode::al || m == mode::aq || m == mode::ao;
}

static const std::string_view s_gpr_names[0x10] =
{
	"R0"sv,
	"R1"sv,
	"R2"sv,
	"R3"sv,
	"R4"sv,
	"R5"sv,
	"R6"sv,
	"R7"sv,
	"R8"sv,
	"R9"sv,
	"R10"sv,
	"R11"sv,
	"AP"sv,
	"FP"sv,
	"SP"sv,
	"PC"sv
};

static const std::string_view s_pr_names[0xf3] =
{
	"KSP"sv,
	"ESP"sv,
	"SSP"sv,
	"USP"sv,
	"ISP"sv,
	{},
	"ASN"sv,
	"SPTEP"sv,

	"P0BR"sv,
	"P0LR"sv,
	"P1BR"sv,
	"P1LR"sv,
	"SBR"sv,
	"SLR"sv,
	"CPUID"sv,
	"WHAMI"sv,

	"PCBB"sv,
	"SCBB"sv,
	"IPL"sv,
	"ASTLVL"sv,
	"SIRR"sv,
	"SISR"sv,
	"IPIR"sv,
	"CMIERR"sv,

	"ICCS"sv,
	"NICR"sv,
	"ICR"sv,
	"TODR"sv,
	"CSRS"sv,
	"CSRD"sv,
	"CSTS"sv,
	"CSTD"sv,

	"RXCS"sv,
	"RXDB"sv,
	"TXCS"sv,
	"TXDB"sv,
	"TBDR"sv,
	"CADR"sv,
	"MCESR"sv,
	"CAER"sv,

	"ACCS"sv,
	"SAVGPR"sv, // or ACCR or SAVISP
	"SAVPC"sv,
	"SAVPSL"sv,
	"WCSA"sv, // or MCESR
	"WCSD"sv, // or BCWBLK
	"WCSL"sv, // or PCIBLK
	"TBTAG"sv,

	"SBIFS"sv,
	"SBIS"sv,
	"SBISC"sv,
	"SBIMT"sv,
	"SBIER"sv,
	"SBITA"sv,
	"SBIQC"sv,
	"IORESET"sv,

	"MAPEN"sv,
	"TBIA"sv,
	"TBIS"sv,
	"TBDATA"sv, // or TBIASN
	"MBRK"sv, // or TBISYS
	"PME"sv,
	"SID"sv,
	"TBCHK"sv,

	"PAMACC"sv,
	"PAMLOC"sv,
	"CSWP"sv,
	"MDECC"sv,
	"MENA"sv,
	"MDCTL"sv,
	"MCCTL"sv,
	"MERG"sv,

	"CRBT"sv,
	"DFI"sv,
	"EHSR"sv,
	{},
	"STXCS"sv,
	"STXDB"sv,
	"ESPA"sv,
	"ESPD"sv,

	"RXCS1"sv,
	"RXDB1"sv,
	"TXCS1"sv,
	"TXDB1"sv,
	"RXCS2"sv,
	"RXDB2"sv,
	"TXCS2"sv,
	"TXDB2"sv,

	"RXCS3"sv,
	"RXDB3"sv,
	"TXCS3"sv,
	"TXDB3"sv,
	"RXCD"sv,
	"CACHEX"sv,
	"BINID"sv,
	"BISTOP"sv,

	{},
	{},
	{},
	{},
	"MEMSIZE"sv,
	"KCALL"sv,
	"VMPSL"sv,
	{},

	{},
	{},
	"CPUCNF"sv,
	"ICIR"sv,
	"RXFCT"sv,
	"RXPRM"sv,
	"TXFCT"sv,
	"TXPRM"sv,

	"BCIDX"sv,
	"BCBTS"sv, // or BCSTS
	"BCP1TS"sv, // or BCCTL
	"BCP2TS"sv, // or BCERA
	"BCRFR"sv, // or BCBTS
	"BCIDX"sv, // or BCDET
	"BCSTS"sv, // or BCERT
	"BCCTL"sv,

	"BCERR"sv,
	"BCFBTS"sv,
	"BCFPTS"sv,
	"VINTSR"sv,
	"PCTAG"sv, // or JTAGCR
	"PCIDX"sv, // or ECR
	"PCERR"sv,
	"PCSTS"sv,

	"NMICTL"sv,
	"INOP"sv,
	"NMIFSR"sv,
	"NMISILO"sv,
	"NMIEAR"sv,
	"COR"sv,
	"REVR1"sv,
	"REVR2"sv, // or VPAMODE

	"CLRTOSTS"sv,
	{},
	{},
	{},
	{},
	{},
	{},
	{},

	"VPSR"sv,
	"VAER"sv,
	"VMAC"sv,
	"VTBIA"sv,
	"VSAR"sv,
	{},
	{},
	{},

	{},
	{},
	{},
	{},
	{},
	"VIADR"sv,
	"VIDLO"sv,
	"VIDHI"sv,

	"CCTL"sv,
	{},
	"BDCECC"sv,
	"BCETSTS"sv,
	"BCETIDX"sv,
	"BCETAG"sv,
	"BCEDSTS"sv,
	"BCEDIDX"sv,

	"BCEDHI"sv,
	"BCEDLO"sv,
	"BCEDECC"sv,
	"CEFADR"sv,
	"CEFSTS"sv,
	{},
	{},
	{},

	"NESTS"sv,
	"NEOADR"sv,
	"NEOCMD"sv,
	"NEDATHI"sv,
	"NEDATLO"sv,
	"NEICMD"sv,
	{},
	{},

	"VADR"sv,
	"VCMD"sv,
	"VQWLO"sv,
	"VQWHI"sv,
	{},
	{},
	{},
	{},

	{}, {}, {}, {}, {}, {}, {}, {},
	{}, {}, {}, {}, {}, {}, {}, {},

	"VMAR"sv,
	"VTAG"sv,
	"VDATA"sv,
	"ICSR"sv,
	{},
	{},
	{},
	{},

	{}, {}, {}, {}, {}, {}, {}, {},

	{},
	{},
	{},
	{},
	{},
	{},
	{},
	"PAMODE"sv,

	"MMEADR"sv,
	"MMEPTE"sv,
	"MMESTS"sv,
	{},
	"TBADR"sv,
	"TBSTS"sv,
	{},
	{},

	"PCADR"sv,
	"PCSTS"sv,
	"PCCTL"sv
};

static const vax_disassembler::opdef s_nonprefix_ops[0x100 - 3] =
{
	{ "HALT"sv,   { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "NOP"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "REI"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OUT },
	{ "BPT"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "RET"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OUT },
	{ "RSB"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OUT },
	{ "LDPCTX"sv, { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OUT },
	{ "SVPCTX"sv, { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTPS"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "CVTSP"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "INDEX"sv,  { mode::rl,   mode::rl,   mode::rl,   mode::rl,   mode::rl,   mode::wl   }, 0 },
	{ "CRC"sv,    { mode::ab,   mode::rl,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "PROBER"sv, { mode::rl,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "PROBEW"sv, { mode::rl,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "INSQUE"sv, { mode::ab,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "REMQUE"sv, { mode::ab,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "BSBB"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OUT },
	{ "BRB"sv,    { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BNEQ"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND }, // or BNEQU
	{ "BEQL"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND }, // or BEQLU
	{ "BGTR"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BLEQ"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "JSB"sv,    { mode::ab,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OVER },
	{ "JMP"sv,    { mode::ab,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BGEQ"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BLSS"sv,   { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BGTRU"sv,  { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BLEQU"sv,  { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BVC"sv,    { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BVS"sv,    { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BCC"sv,    { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND }, // or BGEQU
	{ "BCS"sv,    { mode::bb,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND }, // or BLSSU

	{ "ADDP4"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "ADDP6"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::rw,   mode::ab   }, 0 },
	{ "SUBP4"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "SUBP6"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::rw,   mode::ab   }, 0 },
	{ "CVTPT"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::rw,   mode::ab,   mode::none }, 0 },
	{ "MULP"sv,   { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::rw,   mode::ab   }, 0 },
	{ "CVTTP"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::rw,   mode::ab,   mode::none }, 0 },
	{ "DIVP"sv,   { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::rw,   mode::ab   }, 0 },
	{ "MOVC3"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CMPC3"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "SCANC"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::urb,  mode::none, mode::none }, 0 },
	{ "SPANC"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::urb,  mode::none, mode::none }, 0 },
	{ "MOVC5"sv,  { mode::rw,   mode::ab,   mode::urb,  mode::rw,   mode::ab,   mode::none }, 0 },
	{ "CMPC5"sv,  { mode::rw,   mode::ab,   mode::urb,  mode::rw,   mode::ab,   mode::none }, 0 },
	{ "MOVTC"sv,  { mode::rw,   mode::ab,   mode::urb,  mode::ab,   mode::rw,   mode::ab   }, 0 },
	{ "MOVTUC"sv, { mode::rw,   mode::ab,   mode::urb,  mode::ab,   mode::rw,   mode::ab   }, 0 },

	{ "BSBW"sv,   { mode::bw,   mode::none, mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OVER },
	{ "BRW"sv,    { mode::bw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWL"sv,  { mode::srw,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWB"sv,  { mode::srw,  mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVP"sv,   { mode::rw,   mode::ab,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CMPP3"sv,  { mode::rw,   mode::ab,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTPL"sv,  { mode::rw,   mode::ab,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "CMPP4"sv,  { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "EDITPC"sv, { mode::rw,   mode::ab,   mode::ab,   mode::ab,   mode::none, mode::none }, 0 },
	{ "MATCHC"sv, { mode::rw,   mode::ab,   mode::rw,   mode::ab,   mode::none, mode::none }, 0 },
	{ "LOCC"sv,   { mode::urb,  mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "SKPC"sv,   { mode::urb,  mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "MOVZWL"sv, { mode::rw,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ACBW"sv,   { mode::srw,  mode::srw,  mode::mw,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "MOVAW"sv,  { mode::aw,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PUSHAW"sv, { mode::aw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDF2"sv,  { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDF3"sv,  { mode::rl,   mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBF2"sv,  { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBF3"sv,  { mode::rl,   mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "MULF2"sv,  { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULF3"sv,  { mode::rl,   mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVF2"sv,  { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVF3"sv,  { mode::rl,   mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTFB"sv,  { mode::rl,   mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTFW"sv,  { mode::rl,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTFL"sv,  { mode::rl,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTRFL"sv, { mode::rl,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBF"sv,  { mode::srb,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWF"sv,  { mode::srw,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTLF"sv,  { mode::srl,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ACBF"sv,   { mode::rl,   mode::rl,   mode::ml,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },

	{ "MOVF"sv,   { mode::rl,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPF"sv,   { mode::rl,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MNEGF"sv,  { mode::rl,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTF"sv,   { mode::rl,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "EMODF"sv,  { mode::rl,   mode::rb,   mode::rl,   mode::wl,   mode::wl,   mode::none }, 0 },
	{ "POLYF"sv,  { mode::rl,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTFD"sv,  { mode::rl,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADAWI"sv,  { mode::rw,   mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "INSQHI"sv, { mode::ab,   mode::aq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "INSQTI"sv, { mode::ab,   mode::aq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "REMQHI"sv, { mode::aq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "REMQTI"sv, { mode::aq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDD2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDD3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBD2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBD3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "MULD2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULD3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVD2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVD3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTDB"sv,  { mode::rq,   mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTDW"sv,  { mode::rq,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTDL"sv,  { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTRDL"sv, { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBD"sv,  { mode::srb,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWD"sv,  { mode::srw,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTLD"sv,  { mode::srl,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ACBD"sv,   { mode::rq,   mode::rq,   mode::mq,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },

	{ "MOVD"sv,   { mode::rq,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPD"sv,   { mode::rq,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MNEGD"sv,  { mode::rq,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTD"sv,   { mode::rq,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "EMODD"sv,  { mode::rq,   mode::rb,   mode::rq,   mode::wl,   mode::wq,   mode::none }, 0 },
	{ "POLYD"sv,  { mode::rq,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTDF"sv,  { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ASHL"sv,   { mode::cntb, mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "ASHQ"sv,   { mode::cntb, mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "EMUL"sv,   { mode::rl,   mode::rl,   mode::rl,   mode::wq,   mode::none, mode::none }, 0 },
	{ "EDIV"sv,   { mode::rl,   mode::rq,   mode::wl,   mode::wl,   mode::none, mode::none }, 0 },
	{ "CLRQ"sv,   { mode::wq,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or CLRD or CLRG
	{ "MOVQ"sv,   { mode::urq,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVAQ"sv,  { mode::aq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 }, // or MOVAD or MOVAG
	{ "PUSHAQ"sv, { mode::aq,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or PUSHAD or PUSHAG

	{ "ADDB2"sv,  { mode::srb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDB3"sv,  { mode::srb,  mode::srb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBB2"sv,  { mode::srb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBB3"sv,  { mode::srb,  mode::srb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "MULB2"sv,  { mode::srb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULB3"sv,  { mode::srb,  mode::srb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVB2"sv,  { mode::srb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVB3"sv,  { mode::srb,  mode::srb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "BISB2"sv,  { mode::urb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BISB3"sv,  { mode::urb,  mode::urb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "BICB2"sv,  { mode::urb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BICB3"sv,  { mode::urb,  mode::urb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "XORB2"sv,  { mode::urb,  mode::mb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "XORB3"sv,  { mode::urb,  mode::urb,  mode::wb,   mode::none, mode::none, mode::none }, 0 },
	{ "MNEGB"sv,  { mode::srb,  mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CASEB"sv,  { mode::rb,   mode::rb,   mode::rb,   mode::none, mode::none, mode::none }, 0 },

	{ "MOVB"sv,   { mode::urb,  mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPB"sv,   { mode::srb,  mode::srb,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MCOMB"sv,  { mode::rb,   mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BITB"sv,   { mode::urb,  mode::urb,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CLRB"sv,   { mode::wb,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTB"sv,   { mode::rb,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "INCB"sv,   { mode::mb,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DECB"sv,   { mode::mb,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBL"sv,  { mode::srb,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBW"sv,  { mode::srb,  mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVZBL"sv, { mode::rb,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVZBW"sv, { mode::rb,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ROTL"sv,   { mode::cntb, mode::rl,   mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "ACBB"sv,   { mode::srb,  mode::srb,  mode::mb,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "MOVAB"sv,  { mode::ab,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PUSHAB"sv, { mode::ab,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDW2"sv,  { mode::srw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDW3"sv,  { mode::srw,  mode::srw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBW2"sv,  { mode::srw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBW3"sv,  { mode::srw,  mode::srw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "MULW2"sv,  { mode::srw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULW3"sv,  { mode::srw,  mode::srw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVW2"sv,  { mode::srw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVW3"sv,  { mode::srw,  mode::srw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "BISW2"sv,  { mode::urw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BISW3"sv,  { mode::urw,  mode::urw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "BICW2"sv,  { mode::urw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BICW3"sv,  { mode::urw,  mode::urw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "XORW2"sv,  { mode::urw,  mode::mw,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "XORW3"sv,  { mode::urw,  mode::urw,  mode::ww,   mode::none, mode::none, mode::none }, 0 },
	{ "MNEGW"sv,  { mode::srw,  mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CASEW"sv,  { mode::rw,   mode::rw,   mode::rw,   mode::none, mode::none, mode::none }, 0 },

	{ "MOVW"sv,   { mode::urw,  mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPW"sv,   { mode::srw,  mode::srw,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MCOMW"sv,  { mode::rw,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BITW"sv,   { mode::urw,  mode::urw,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CLRW"sv,   { mode::ww,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTW"sv,   { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "INCW"sv,   { mode::mw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DECW"sv,   { mode::mw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BISPSW"sv, { mode::urw,  mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BICPSW"sv, { mode::urw,  mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "POPR"sv,   { mode::mskw, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PUSHR"sv,  { mode::mskw, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CHMK"sv,   { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CHME"sv,   { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CHMS"sv,   { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CHMU"sv,   { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDL2"sv,  { mode::srl,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDL3"sv,  { mode::srl,  mode::srl,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBL2"sv,  { mode::srl,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBL3"sv,  { mode::srl,  mode::srl,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "MULL2"sv,  { mode::srl,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULL3"sv,  { mode::srl,  mode::srl,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVL2"sv,  { mode::srl,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVL3"sv,  { mode::srl,  mode::srl,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "BISL2"sv,  { mode::url,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BISL3"sv,  { mode::url,  mode::url,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "BICL2"sv,  { mode::url,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BICL3"sv,  { mode::url,  mode::url,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "XORL2"sv,  { mode::url,  mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "XORL3"sv,  { mode::url,  mode::url,  mode::wl,   mode::none, mode::none, mode::none }, 0 },
	{ "MNEGL"sv,  { mode::srl,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CASEL"sv,  { mode::rl,   mode::rl,   mode::rl,   mode::none, mode::none, mode::none }, 0 },

	{ "MOVL"sv,   { mode::url,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPL"sv,   { mode::srl,  mode::srl,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MCOML"sv,  { mode::rl,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "BITL"sv,   { mode::url,  mode::url,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CLRL"sv,   { mode::wl,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or CLRF
	{ "TSTL"sv,   { mode::rl,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "INCL"sv,   { mode::ml,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DECL"sv,   { mode::ml,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADWC"sv,   { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SBWC"sv,   { mode::rl,   mode::ml,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MTPR"sv,   { mode::url,  mode::prl,  mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MFPR"sv,   { mode::prl,  mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVPSL"sv, { mode::wl,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PUSHL"sv,  { mode::rl,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVAL"sv,  { mode::al,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 }, // or MOVAF
	{ "PUSHAL"sv, { mode::al,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or PUSHAF

	{ "BBS"sv,    { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBC"sv,    { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBSS"sv,   { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBCS"sv,   { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBSC"sv,   { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBCC"sv,   { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBSSI"sv,  { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BBCCI"sv,  { mode::posl, mode::vb,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BLBS"sv,   { mode::posl, mode::bb,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "BLBC"sv,   { mode::posl, mode::bb,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "FFS"sv,    { mode::posl, mode::cntb, mode::vb,   mode::wl,   mode::none, mode::none }, 0 },
	{ "FFC"sv,    { mode::posl, mode::cntb, mode::vb,   mode::wl,   mode::none, mode::none }, 0 },
	{ "CMPV"sv,   { mode::posl, mode::cntb, mode::vb,   mode::rl,   mode::none, mode::none }, 0 },
	{ "CMPZV"sv,  { mode::posl, mode::cntb, mode::vb,   mode::rl,   mode::none, mode::none }, 0 },
	{ "EXTV"sv,   { mode::posl, mode::cntb, mode::vb,   mode::wl,   mode::none, mode::none }, 0 },
	{ "EXTZV"sv,  { mode::posl, mode::cntb, mode::vb,   mode::wl,   mode::none, mode::none }, 0 },

	{ "INSV"sv,   { mode::rl,   mode::posl, mode::cntb, mode::vb,   mode::none, mode::none }, 0 },
	{ "ACBL"sv,   { mode::srl,  mode::srl,  mode::ml,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "AOBLSS"sv, { mode::srl,  mode::ml,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "AOBLEQ"sv, { mode::srl,  mode::ml,   mode::bb,   mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "SOBGEQ"sv, { mode::ml,   mode::bb,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "SOBGTR"sv, { mode::ml,   mode::bb,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_COND },
	{ "CVTLB"sv,  { mode::srl,  mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTLW"sv,  { mode::srl,  mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ASHP"sv,   { mode::cntb, mode::rw,   mode::ab,   mode::rb,   mode::rw,   mode::ab   }, 0 },
	{ "CVTLP"sv,  { mode::srb,  mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CALLG"sv,  { mode::ab,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OVER },
	{ "CALLS"sv,  { mode::rl,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, vax_disassembler::STEP_OVER },
	{ "XFC"sv,    { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }
};

static const vax_disassembler::opdef s_fdprefix_ops[0x100 - 0x31] =
{
	{ "MFVP"sv,   { mode::rw,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTDH"sv,  { mode::rq,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTGF"sv,  { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VLDL"sv,   { mode::rw,   mode::ab,   mode::rl,   mode::none, mode::none, mode::none }, 0 },
	{ "VGATHL"sv, { mode::rw,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VLDQ"sv,   { mode::rw,   mode::ab,   mode::rl,   mode::none, mode::none, mode::none }, 0 },
	{ "VLGATHQ"sv,{ mode::rw,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDG2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDG3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBG2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBG3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "MULG2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULG3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVG2"sv,  { mode::rq,   mode::mq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVG3"sv,  { mode::rq,   mode::rq,   mode::wq,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTGB"sv,  { mode::rq,   mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTGW"sv,  { mode::rq,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTGL"sv,  { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTRGL"sv, { mode::rq,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBG"sv,  { mode::srb,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWG"sv,  { mode::srw,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTLG"sv,  { mode::srl,  mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ACBG"sv,   { mode::rq,   mode::rq,   mode::mq,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },

	{ "MOVG"sv,   { mode::rq,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPG"sv,   { mode::rq,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MNEGG"sv,  { mode::rq,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTG"sv,   { mode::rq,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "EMODG"sv,  { mode::rq,   mode::rb,   mode::rq,   mode::wl,   mode::wq,   mode::none }, 0 },
	{ "POLYG"sv,  { mode::rq,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTGH"sv,  { mode::rq,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "ADDH2"sv,  { mode::ro,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ADDH3"sv,  { mode::ro,   mode::ro,   mode::wo,   mode::none, mode::none, mode::none }, 0 },
	{ "SUBH2"sv,  { mode::ro,   mode::mo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "SUBH3"sv,  { mode::ro,   mode::ro,   mode::wo,   mode::none, mode::none, mode::none }, 0 },
	{ "MULH2"sv,  { mode::ro,   mode::mo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MULH3"sv,  { mode::ro,   mode::ro,   mode::wo,   mode::none, mode::none, mode::none }, 0 },
	{ "DIVH2"sv,  { mode::ro,   mode::mo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "DIVH3"sv,  { mode::ro,   mode::ro,   mode::wo,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTHB"sv,  { mode::ro,   mode::wb,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTHW"sv,  { mode::ro,   mode::ww,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTHL"sv,  { mode::ro,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTRHL"sv, { mode::ro,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTBH"sv,  { mode::srb,  mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTWH"sv,  { mode::srw,  mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTLH"sv,  { mode::srl,  mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "ACBH"sv,   { mode::ro,   mode::ro,   mode::mq,   mode::bw,   mode::none, mode::none }, vax_disassembler::STEP_COND },

	{ "MOVH"sv,   { mode::ro,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CMPH"sv,   { mode::ro,   mode::ro,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MNEGH"sv,  { mode::ro,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "TSTH"sv,   { mode::ro,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "EMODH"sv,  { mode::ro,   mode::rb,   mode::ro,   mode::wl,   mode::wo,   mode::none }, 0 },
	{ "POLYH"sv,  { mode::ro,   mode::rw,   mode::ab,   mode::none, mode::none, mode::none }, 0 },
	{ "CVTHG"sv,  { mode::ro,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CLRO"sv,   { mode::wo,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or CLRH
	{ "MOVO"sv,   { mode::uro,  mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "MOVAO"sv,  { mode::ao,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 }, // or MOVAH
	{ "PUSHAO"sv, { mode::ao,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }, // or PUSHAH

	{ "VVADDL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSADDL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVADDG"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSADDG"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVADDF"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSADDF"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVADDD"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSADDD"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVSUBL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSUBL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVSUBG"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSUBG"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVSUBF"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSUBF"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVSUBD"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSUBD"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTFH"sv,  { mode::rl,   mode::wo,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTFG"sv,  { mode::rl,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PROBEVMR"sv, { mode::rb, mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "PROBEVMW"sv, { mode::rb, mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSTL"sv,   { mode::rw,   mode::ab,   mode::rl,   mode::none, mode::none, mode::none }, 0 },
	{ "VSCATL"sv, { mode::rw,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSTQ"sv,   { mode::rw,   mode::ab,   mode::rl,   mode::none, mode::none, mode::none }, 0 },
	{ "VSCATQ"sv, { mode::rw,   mode::ab,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "VVMULL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSMULL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVMULG"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSMULG"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVMULF"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSMULF"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVMULD"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSMULD"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVDIVL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSDIVL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVDIVG"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSDIVG"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVDIVF"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSDIVF"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVDIVD"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSDIVD"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "VVCMPL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSCMPL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVCMPG"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSCMPG"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVCMPF"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSCMPF"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVCMPD"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSCMPD"sv, { mode::rw,   mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVBISL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSBISL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVBICL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSBICL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },

	{ "VVSRLL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSRLL"sv, { mode::rw,   mode::posl, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVSLLL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSSLLL"sv, { mode::rw,   mode::posl, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVXORL"sv, { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSXORL"sv, { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVCNT"sv,  { mode::rw,   mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "IOTA"sv,   { mode::rw,   mode::rl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VVMERGE"sv, { mode::rw,  mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "VSMERGE"sv, { mode::rw,  mode::rq,   mode::none, mode::none, mode::none, mode::none }, 0 },

	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTHF"sv,  { mode::ro,   mode::wl,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ "CVTHD"sv,  { mode::ro,   mode::wq,   mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 },
	{ {},         { mode::none, mode::none, mode::none, mode::none, mode::none, mode::none }, 0 }
};

} // anonymous namespace

void vax_disassembler::format_immediate(std::ostream &stream, u32 i) const
{
	stream << '#';
	if (i > 9)
		stream << "^X";
	util::stream_format(stream, "%X", i);
}

void vax_disassembler::format_signed(std::ostream &stream, s32 value) const
{
	if (value < 0)
	{
		stream << '-';
		value = -value;
	}
	if (u32(value) > 9)
		stream << "^X";
	util::stream_format(stream, "%X", u32(value));
}

void vax_disassembler::format_relative(std::ostream &stream, u32 pc, s32 displ) const
{
	util::stream_format(stream, "^X%08X", pc + displ);
}

void vax_disassembler::format_register_mask(std::ostream &stream, u16 mask) const
{
	stream << "^M<";

	for (int n = 0; mask != 0; n++)
	{
		assert(n < 16);
		if (BIT(mask, n))
		{
			stream << s_gpr_names[n];
			mask &= ~(1 << n);
			if (mask != 0)
				stream << ',';
		}
	}

	stream << '>';
}

offs_t vax_disassembler::disassemble_inst(std::ostream &stream, const vax_disassembler::opdef &inst, offs_t &pc, offs_t ppc, const vax_disassembler::data_buffer &opcodes) const
{
	if (inst.operand[0] == mode::none)
		stream << inst.mnemonic;
	else
		util::stream_format(stream, "%-7s ", inst.mnemonic);
	for (int n = 0; n < 6 && inst.operand[n] != mode::none; n++)
	{
		if (n != 0)
			stream << ", ";
		if (inst.operand[n] == mode::bb)
		{
			s8 displacement = opcodes.r8(pc++);
			format_relative(stream, pc, s32(displacement));
		}
		else if (inst.operand[n] == mode::bw)
		{
			s16 displacement = opcodes.r16(pc);
			pc += 2;
			format_relative(stream, pc, s32(displacement));
		}
		else
		{
			u8 opmode = opcodes.r8(pc++);
			u8 idxmode = 0;
			if (opmode >= 0x40 && opmode < 0x4f)
			{
				// Indexed mode
				idxmode = opmode;
				opmode = opcodes.r8(pc++);
			}
			if (opmode < 0x40 && is_read_mode(inst.operand[n]) && idxmode == 0)
			{
				// Literal mode
				if (inst.operand[n] == mode::mskw)
					format_register_mask(stream, opmode);
				else if (inst.operand[n] == mode::prl && !s_pr_names[opmode].empty())
					stream << s_pr_names[opmode];
				else if (inst.operand[n] == mode::urb)
					util::stream_format(stream, "#^X%02X", opmode);
				else if (inst.operand[n] == mode::cntb || inst.operand[n] == mode::posl)
					util::stream_format(stream, "#%d", opmode);
				else
					format_immediate(stream, opmode);
			}
			else if (opmode < 0x50)
				stream << "<invalid>";
			else if (opmode < 0x60)
			{
				// Register mode
				if (idxmode != 0 || is_address_mode(inst.operand[n]))
					stream << "<invalid>";
				else
					stream << s_gpr_names[opmode & 0x0f];
			}
			else
			{
				if (opmode < 0x80)
				{
					// Register deferred or autodecrement mode
					if (opmode >= 0x70)
						stream << '-';
					util::stream_format(stream, "(%s)", s_gpr_names[opmode & 0x0f]);
				}
				else if (opmode < 0xa0)
				{
					// Autoincrement mode
					if (BIT(opmode, 4))
						stream << '@';
					if (opmode == 0x9f)
					{
						util::stream_format(stream, "#^X%08X", opcodes.r32(pc));
						pc += 4;
					}
					else if (opmode != 0x8f)
						util::stream_format(stream, "(%s)+", s_gpr_names[opmode & 0x0f]);
					else switch (inst.operand[n])
					{
					case mode::rb:
					case mode::ab: // Immediate bytes may be used as MOVCn sources, at least with a length of 0
						format_immediate(stream, opcodes.r8(pc++));
						break;

					case mode::srb:
						stream << '#';
						format_signed(stream, s8(opcodes.r8(pc++)));
						break;

					case mode::urb:
						util::stream_format(stream, "#^X%02X", opcodes.r8(pc++));
						break;

					case mode::cntb:
						util::stream_format(stream, "#%d", s8(opcodes.r8(pc++)));
						break;

					case mode::rw:
						format_immediate(stream, opcodes.r16(pc));
						pc += 2;
						break;

					case mode::srw:
						stream << '#';
						format_signed(stream, s16(opcodes.r16(pc)));
						pc += 2;
						break;

					case mode::urw:
						util::stream_format(stream, "#^X%04X", opcodes.r16(pc));
						pc += 2;
						break;

					case mode::mskw:
						format_register_mask(stream, opcodes.r16(pc));
						pc += 2;
						break;

					case mode::rl: case mode::posl:
						format_immediate(stream, opcodes.r32(pc));
						pc += 4;
						break;

					case mode::srl:
						stream << '#';
						format_signed(stream, s32(opcodes.r32(pc)));
						pc += 4;
						break;

					case mode::url:
						util::stream_format(stream, "#^X%08X", opcodes.r32(pc));
						pc += 4;
						break;

					case mode::prl:
					{
						u32 i = opcodes.r32(pc);
						pc += 4;
						if (i < std::size(s_pr_names) && !s_pr_names[i].empty())
							stream << s_pr_names[i];
						else
							util::stream_format(stream, "#^X%X", i);
						break;
					}

					case mode::rq: case mode::urq:
						util::stream_format(stream, "#^X%016X", opcodes.r64(pc));
						pc += 8;
						break;

					case mode::ro: case mode::uro:
						util::stream_format(stream, "#^X%016X%016X", opcodes.r64(pc), opcodes.r64(pc + 4));
						pc += 16;
						break;

					default:
						stream << "(PC)+";
						break;
					}
				}
				else
				{
					if (BIT(opmode, 4))
						stream << '@';

					s32 displ;
					if (opmode < 0xc0)
					{
						// Byte displacement mode
						displ = s32(s8(opcodes.r8(pc++)));
					}
					else if (opmode < 0xe0)
					{
						// Word displacement mode
						displ = s32(s16(opcodes.r16(pc)));
						pc += 2;
					}
					else
					{
						// Long displacement mode
						displ = s32(opcodes.r32(pc));
						pc += 4;
					}

					if ((opmode & 0x0f) != 0x0f)
					{
						if (opmode >= 0xe0)
							util::stream_format(stream, "^X%X", displ);
						else
							format_signed(stream, displ);
						util::stream_format(stream, "(%s)", s_gpr_names[opmode & 0x0f]);
					}
					else
						format_relative(stream, pc, displ);
				}
				if (idxmode != 0)
					util::stream_format(stream, "[%s]", s_gpr_names[idxmode & 0x0f]);
			}
		}
	}
	return (pc - ppc) | inst.flags | SUPPORTED;
}

offs_t vax_disassembler::disassemble(std::ostream &stream, offs_t pc, const vax_disassembler::data_buffer &opcodes, const vax_disassembler::data_buffer &params)
{
	offs_t ppc = pc;
	u8 op = opcodes.r8(pc++);

	if (op >= 0xfd)
	{
		u8 op2 = opcodes.r8(pc++);
		if (op == 0xfd)
		{
			if (op2 == 0x02)
			{
				stream << "WAIT";
				return 2 | SUPPORTED;
			}
			else if (op2 >= 0x31 && !s_fdprefix_ops[op2 - 0x31].mnemonic.empty())
				return disassemble_inst(stream, s_fdprefix_ops[op2 - 0x31], pc, ppc, opcodes);
		}
		else if (op == 0xff)
		{
			if (op2 == 0xfe)
			{
				stream << "BUGW";
				return 2 | SUPPORTED;
			}
			else if (op2 == 0xfd)
			{
				stream << "BUGL";
				return 2 | SUPPORTED;
			}
		}
		util::stream_format(stream, "%-8s^X%02X%02X", ".WORD", op2, op);
		return 2 | SUPPORTED;
	}
	else if (s_nonprefix_ops[op].mnemonic.empty())
	{
		util::stream_format(stream, "%-8s^X%02X", ".BYTE", op);
		return 1 | SUPPORTED;
	}
	else
		return disassemble_inst(stream, s_nonprefix_ops[op], pc, ppc, opcodes);
}

bool vax_disassembler::is_read_mode(vax_disassembler::mode m)
{
	switch (m)
	{
	case mode::rb: case mode::srb: case mode::urb: case mode::cntb:
	case mode::rw: case mode::srw: case mode::urw: case mode::mskw:
	case mode::rl: case mode::srl: case mode::url: case mode::posl: case mode::prl:
	case mode::rq: case mode::urq:
	case mode::ro: case mode::uro:
		return true;
	default:
		return false;
	}
}

const vax_disassembler::mode *vax_disassembler::get_operands(u8 op)
{
	return s_nonprefix_ops[op].operand;
}
