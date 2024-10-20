// license:BSD-3-Clause
// copyright-holders:Ramiro Polla
//============================================================
//
//  debuggdbstub.cpp - GDB stub debugger
//
//============================================================

#include "emu.h"
#include "debug_module.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"
#include "debug/textbuf.h"
#include "debugger.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"

#include "fileio.h"

#include <cinttypes>
#include <string_view>


namespace osd {

namespace {

//-------------------------------------------------------------------------
#define MAX_PACKET_SIZE 16384

//-------------------------------------------------------------------------
enum gdb_register_type
{
	TYPE_INT,
	TYPE_CODE_POINTER,
	TYPE_DATA_POINTER,
	TYPE_I387_EXT,
};
static const char *const gdb_register_type_str[] = {
	"int",
	"code_ptr",
	"data_ptr",
	"i387_ext",
};
struct gdb_register_map
{
	const char *arch;
	const char *feature;
	struct gdb_register_description
	{
		const char *state_name;
		const char *gdb_name;
		bool stop_packet;
		gdb_register_type gdb_type;
		int override_bitsize;

		gdb_register_description(const char *_state_name=nullptr, const char *_gdb_name=nullptr, bool _stop_packet=false, gdb_register_type _gdb_type=TYPE_INT, int _override_bitsize=-1)
		: state_name(_state_name)
		, gdb_name(_gdb_name)
		, stop_packet(_stop_packet)
		, gdb_type(_gdb_type)
		, override_bitsize(_override_bitsize)
		{
		}
	};
	std::vector<gdb_register_description> registers;
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_i486 =
{
	"i386",
	"org.gnu.gdb.i386.core",
	{
		{ "EAX",     "eax",    false, TYPE_INT },
		{ "ECX",     "ecx",    false, TYPE_INT },
		{ "EDX",     "edx",    false, TYPE_INT },
		{ "EBX",     "ebx",    false, TYPE_INT },
		{ "ESP",     "esp",    true,  TYPE_DATA_POINTER },
		{ "EBP",     "ebp",    true,  TYPE_DATA_POINTER },
		{ "ESI",     "esi",    false, TYPE_INT },
		{ "EDI",     "edi",    false, TYPE_INT },
		{ "EIP",     "eip",    true,  TYPE_CODE_POINTER },
		{ "EFLAGS",  "eflags", false, TYPE_INT }, // TODO describe bitfield
		{ "CS",      "cs",     false, TYPE_INT },
		{ "SS",      "ss",     false, TYPE_INT },
		{ "DS",      "ds",     false, TYPE_INT },
		{ "ES",      "es",     false, TYPE_INT },
		{ "FS",      "fs",     false, TYPE_INT },
		{ "GS",      "gs",     false, TYPE_INT },
		// TODO fix x87 registers!
		// The x87 registers are just plain wrong for a few reasons:
		//  - The st* registers use a dummy variable in i386_device, so we
		//    don't retrieve the real value (also the bitsize is wrong);
		//  - The seg/off/op registers don't seem to be exported in the
		//    state.
		{ "ST0",     "st0",    false, TYPE_I387_EXT },
		{ "ST1",     "st1",    false, TYPE_I387_EXT },
		{ "ST2",     "st2",    false, TYPE_I387_EXT },
		{ "ST3",     "st3",    false, TYPE_I387_EXT },
		{ "ST4",     "st4",    false, TYPE_I387_EXT },
		{ "ST5",     "st5",    false, TYPE_I387_EXT },
		{ "ST6",     "st6",    false, TYPE_I387_EXT },
		{ "ST7",     "st7",    false, TYPE_I387_EXT },
		{ "x87_CW",  "fctrl",  false, TYPE_INT },
		{ "x87_SW",  "fstat",  false, TYPE_INT },
		{ "x87_TAG", "ftag",   false, TYPE_INT },
		{ "EAX",     "fiseg",  false, TYPE_INT },
		{ "EAX",     "fioff",  false, TYPE_INT },
		{ "EAX",     "foseg",  false, TYPE_INT },
		{ "EAX",     "fooff",  false, TYPE_INT },
		{ "EAX",     "fop",    false, TYPE_INT },
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_arm7 =
{
	"arm",
	"org.gnu.gdb.arm.core",
	{
		{ "R0",   "r0",   false, TYPE_INT },
		{ "R1",   "r1",   false, TYPE_INT },
		{ "R2",   "r2",   false, TYPE_INT },
		{ "R3",   "r3",   false, TYPE_INT },
		{ "R4",   "r4",   false, TYPE_INT },
		{ "R5",   "r5",   false, TYPE_INT },
		{ "R6",   "r6",   false, TYPE_INT },
		{ "R7",   "r7",   false, TYPE_INT },
		{ "R8",   "r8",   false, TYPE_INT },
		{ "R9",   "r9",   false, TYPE_INT },
		{ "R10",  "r10",  false, TYPE_INT },
		{ "R11",  "r11",  false, TYPE_INT },
		{ "R12",  "r12",  false, TYPE_INT },
		{ "R13",  "sp",   true,  TYPE_DATA_POINTER },
		{ "R14",  "lr",   true,  TYPE_INT },
		{ "R15",  "pc",   true,  TYPE_CODE_POINTER },
		{ "CPSR", "cpsr", false, TYPE_INT }, // TODO describe bitfield
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_ppc601 =
{
	"powerpc:common",
	"org.gnu.gdb.power.core",
	{
		{ "R0",   "r0",   false, TYPE_INT },
		{ "R1",   "r1",   false, TYPE_INT },
		{ "R2",   "r2",   false, TYPE_INT },
		{ "R3",   "r3",   false, TYPE_INT },
		{ "R4",   "r4",   false, TYPE_INT },
		{ "R5",   "r5",   false, TYPE_INT },
		{ "R6",   "r6",   false, TYPE_INT },
		{ "R7",   "r7",   false, TYPE_INT },
		{ "R8",   "r8",   false, TYPE_INT },
		{ "R9",   "r9",   false, TYPE_INT },
		{ "R10",  "r10",  false, TYPE_INT },
		{ "R11",  "r11",  false, TYPE_INT },
		{ "R12",  "r12",  false, TYPE_INT },
		{ "R13",  "r13",  false, TYPE_INT },
		{ "R14",  "r14",  false, TYPE_INT },
		{ "R15",  "r15",  false, TYPE_INT },
		{ "R16",  "r16",  false, TYPE_INT },
		{ "R17",  "r17",  false, TYPE_INT },
		{ "R18",  "r18",  false, TYPE_INT },
		{ "R19",  "r19",  false, TYPE_INT },
		{ "R20",  "r20",  false, TYPE_INT },
		{ "R21",  "r21",  false, TYPE_INT },
		{ "R22",  "r22",  false, TYPE_INT },
		{ "R23",  "r23",  false, TYPE_INT },
		{ "R24",  "r24",  false, TYPE_INT },
		{ "R25",  "r25",  false, TYPE_INT },
		{ "R26",  "r26",  false, TYPE_INT },
		{ "R27",  "r27",  false, TYPE_INT },
		{ "R28",  "r28",  false, TYPE_INT },
		{ "R29",  "r29",  false, TYPE_INT },
		{ "R30",  "r30",  false, TYPE_INT },
		{ "R31",  "r31",  false, TYPE_INT },
		{ "PC",   "pc",   true,  TYPE_CODE_POINTER },
		{ "MSR",  "msr",  false, TYPE_INT },
		{ "CR",   "cr",   false, TYPE_INT },
		{ "LR",   "lr",   true,  TYPE_CODE_POINTER },
		{ "CTR",  "ctr",  false, TYPE_INT },
		{ "XER",  "xer",  false, TYPE_INT },
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_r4600 =
{
	"mips",
	"org.gnu.gdb.mips.cpu",
	{
		{ "zero", "r0",   false, TYPE_INT, 32 },
		{ "at",   "r1",   false, TYPE_INT, 32 },
		{ "v0",   "r2",   false, TYPE_INT, 32 },
		{ "v1",   "r3",   false, TYPE_INT, 32 },
		{ "a0",   "r4",   false, TYPE_INT, 32 },
		{ "a1",   "r5",   false, TYPE_INT, 32 },
		{ "a2",   "r6",   false, TYPE_INT, 32 },
		{ "a3",   "r7",   false, TYPE_INT, 32 },
		{ "t0",   "r8",   false, TYPE_INT, 32 },
		{ "t1",   "r9",   false, TYPE_INT, 32 },
		{ "t2",   "r10",  false, TYPE_INT, 32 },
		{ "t3",   "r11",  false, TYPE_INT, 32 },
		{ "t4",   "r12",  false, TYPE_INT, 32 },
		{ "t5",   "r13",  false, TYPE_INT, 32 },
		{ "t6",   "r14",  false, TYPE_INT, 32 },
		{ "t7",   "r15",  false, TYPE_INT, 32 },
		{ "s0",   "r16",  false, TYPE_INT, 32 },
		{ "s1",   "r17",  false, TYPE_INT, 32 },
		{ "s2",   "r18",  false, TYPE_INT, 32 },
		{ "s3",   "r19",  false, TYPE_INT, 32 },
		{ "s4",   "r20",  false, TYPE_INT, 32 },
		{ "s5",   "r21",  false, TYPE_INT, 32 },
		{ "s6",   "r22",  false, TYPE_INT, 32 },
		{ "s7",   "r23",  false, TYPE_INT, 32 },
		{ "t8",   "r24",  false, TYPE_INT, 32 },
		{ "t9",   "r25",  false, TYPE_INT, 32 },
		{ "k0",   "r26",  false, TYPE_INT, 32 },
		{ "k1",   "r27",  false, TYPE_INT, 32 },
		{ "gp",   "r28",  false, TYPE_INT, 32 },
		{ "sp",   "r29",  false, TYPE_INT, 32 },
		{ "fp",   "r30",  false, TYPE_INT, 32 },
		{ "ra",   "r31",  false, TYPE_INT, 32 },
		{ "LO",   "lo",   false, TYPE_INT, 32 },
		{ "HI",   "hi",   false, TYPE_INT, 32 },
		{ "PC",   "pc",   true,  TYPE_CODE_POINTER, 32 },
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_m68020pmmu =
{
	"m68k",
	"org.gnu.gdb.m68k.core",
	{
		{ "D0", "d0", false, TYPE_INT },
		{ "D1", "d1", false, TYPE_INT },
		{ "D2", "d2", false, TYPE_INT },
		{ "D3", "d3", false, TYPE_INT },
		{ "D4", "d4", false, TYPE_INT },
		{ "D5", "d5", false, TYPE_INT },
		{ "D6", "d6", false, TYPE_INT },
		{ "D7", "d7", false, TYPE_INT },
		{ "A0", "a0", false, TYPE_INT },
		{ "A1", "a1", false, TYPE_INT },
		{ "A2", "a2", false, TYPE_INT },
		{ "A3", "a3", false, TYPE_INT },
		{ "A4", "a4", false, TYPE_INT },
		{ "A5", "a5", false, TYPE_INT },
		{ "A6", "fp", true,  TYPE_INT },
		{ "SP", "sp", true,  TYPE_INT },
		{ "SR", "ps", false, TYPE_INT }, // NOTE GDB named it ps, but it's actually sr
		{ "CURPC","pc", true,  TYPE_CODE_POINTER },
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_m68000 =
{
	"m68k",
	"org.gnu.gdb.m68k.core",
	{
		{ "D0", "d0", false, TYPE_INT },
		{ "D1", "d1", false, TYPE_INT },
		{ "D2", "d2", false, TYPE_INT },
		{ "D3", "d3", false, TYPE_INT },
		{ "D4", "d4", false, TYPE_INT },
		{ "D5", "d5", false, TYPE_INT },
		{ "D6", "d6", false, TYPE_INT },
		{ "D7", "d7", false, TYPE_INT },
		{ "A0", "a0", false, TYPE_INT },
		{ "A1", "a1", false, TYPE_INT },
		{ "A2", "a2", false, TYPE_INT },
		{ "A3", "a3", false, TYPE_INT },
		{ "A4", "a4", false, TYPE_INT },
		{ "A5", "a5", false, TYPE_INT },
		{ "A6", "fp", true,  TYPE_INT },
		{ "SP", "sp", true,  TYPE_INT },
		{ "SR", "ps", false, TYPE_INT }, // NOTE GDB named it ps, but it's actually sr
		{ "CURPC","pc", true,  TYPE_CODE_POINTER },
		//NOTE m68-elf-gdb complains about fpcontrol register not present but 68000 doesn't have floating point so...
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_z80 =
{
	"z80",
	"mame.z80",
	{
		{ "AF",  "af",  false, TYPE_INT },
		{ "BC",  "bc",  false, TYPE_INT },
		{ "DE",  "de",  false, TYPE_INT },
		{ "HL",  "hl",  false, TYPE_INT },
		{ "AF2", "af'", false, TYPE_INT },
		{ "BC2", "bc'", false, TYPE_INT },
		{ "DE2", "de'", false, TYPE_INT },
		{ "HL2", "hl'", false, TYPE_INT },
		{ "IX",  "ix",  false, TYPE_INT },
		{ "IY",  "iy",  false, TYPE_INT },
		{ "SP",  "sp",  true,  TYPE_DATA_POINTER },
		{ "PC",  "pc",  true,  TYPE_CODE_POINTER },
	}
};

//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_m6502 =
{
	"m6502",
	"mame.m6502",
	{
		{ "A",  "a",   false, TYPE_INT },
		{ "X",  "x",   false, TYPE_INT },
		{ "Y",  "y",   false, TYPE_INT },
		{ "P",  "p",   false, TYPE_INT },
		{ "SP", "sp",  true,  TYPE_DATA_POINTER },
		{ "PC", "pc",  true,  TYPE_CODE_POINTER },
	}
};


//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_m6809 =
{
	"m6809",
	"mame.m6809",
	{
		{ "A",  "a",   false, TYPE_INT },
		{ "B",  "b",   false, TYPE_INT },
		{ "D",  "d",   false, TYPE_INT },
		{ "X",  "x",   false, TYPE_INT },
		{ "Y",  "y",   false, TYPE_INT },
		{ "U",  "u",   true,  TYPE_DATA_POINTER },
		{ "PC", "pc",  true,  TYPE_CODE_POINTER },
		{ "S",  "s",   true,  TYPE_DATA_POINTER },
		{ "CC", "cc",  false, TYPE_INT }, // TODO describe bitfield
		{ "DP", "dp",  false, TYPE_INT },
	}
};


//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_score7 =
{
	"score7",
	"mame.score7",
	{
		{ "r0",      "r0",      true,  TYPE_DATA_POINTER },
		{ "r1",      "r1",      false, TYPE_INT },
		{ "r2",      "r2",      false, TYPE_INT },
		{ "r3",      "r3",      false, TYPE_INT },
		{ "r4",      "r4",      false, TYPE_INT },
		{ "r5",      "r5",      false, TYPE_INT },
		{ "r6",      "r6",      false, TYPE_INT },
		{ "r7",      "r7",      false, TYPE_INT },
		{ "r8",      "r8",      false, TYPE_INT },
		{ "r9",      "r9",      false, TYPE_INT },
		{ "r10",     "r10",     false, TYPE_INT },
		{ "r11",     "r11",     false, TYPE_INT },
		{ "r12",     "r12",     false, TYPE_INT },
		{ "r13",     "r13",     false, TYPE_INT },
		{ "r14",     "r14",     false, TYPE_INT },
		{ "r15",     "r15",     false, TYPE_INT },
		{ "r16",     "r16",     false, TYPE_INT },
		{ "r17",     "r17",     false, TYPE_INT },
		{ "r18",     "r18",     false, TYPE_INT },
		{ "r19",     "r19",     false, TYPE_INT },
		{ "r20",     "r20",     false, TYPE_INT },
		{ "r21",     "r21",     false, TYPE_INT },
		{ "r22",     "r22",     false, TYPE_INT },
		{ "r23",     "r23",     false, TYPE_INT },
		{ "r24",     "r24",     false, TYPE_INT },
		{ "r25",     "r25",     false, TYPE_INT },
		{ "r26",     "r26",     false, TYPE_INT },
		{ "r27",     "r27",     false, TYPE_INT },
		{ "r28",     "r28",     false, TYPE_INT },
		{ "r29",     "r29",     false, TYPE_INT },
		{ "r30",     "r30",     false, TYPE_INT },
		{ "r31",     "r31",     false, TYPE_INT },
		{ "cr0",     "PSR",     false, TYPE_INT },
		{ "cr1",     "COND",    false, TYPE_INT },
		{ "cr2",     "ECR",     false, TYPE_INT },
		{ "cr3",     "EXCPVEC", false, TYPE_INT },
		{ "cr4",     "CCR",     false, TYPE_INT },
		{ "cr5",     "EPC",     false, TYPE_INT },
		{ "cr6",     "EMA",     false, TYPE_INT },
		{ "cr7",     "TLBLOCK", false, TYPE_INT },
		{ "cr8",     "TLBPT",   false, TYPE_INT },
		{ "cr9",     "PEADDR",  false, TYPE_INT },
		{ "cr10",    "TLBRPT",  false, TYPE_INT },
		{ "cr11",    "PEVN",    false, TYPE_INT },
		{ "cr12",    "PECTX",   false, TYPE_INT },
		{ "cr15",    "LIMPFN",  false, TYPE_INT },
		{ "cr16",    "LDMPFN",  false, TYPE_INT },
		{ "cr18",    "PREV",    false, TYPE_INT },
		{ "cr29",    "DREG",    false, TYPE_INT },
		{ "PC",      "PC",      true,  TYPE_CODE_POINTER }, // actually Debug exception program counter (DEPC)
		{ "cr31",    "DSAVE",   false, TYPE_INT },
		{ "sr0",     "COUNTER", false, TYPE_INT },
		{ "sr1",     "LDCR",    false, TYPE_INT },
		{ "sr2",     "STCR",    false, TYPE_INT },
		{ "ceh",     "CEH",     false, TYPE_INT },
		{ "cel",     "CEL",     false, TYPE_INT },
	}
};


//-------------------------------------------------------------------------
static const gdb_register_map gdb_register_map_nios2 =
{
	"nios2",
	"org.gnu.gdb.nios2.cpu",
	{
		{ "zero",     "zero",     false, TYPE_INT },
		{ "at",       "at",       false, TYPE_INT },
		{ "r2",       "r2",       false, TYPE_INT },
		{ "r3",       "r3",       false, TYPE_INT },
		{ "r4",       "r4",       false, TYPE_INT },
		{ "r5",       "r5",       false, TYPE_INT },
		{ "r6",       "r6",       false, TYPE_INT },
		{ "r7",       "r7",       false, TYPE_INT },
		{ "r8",       "r8",       false, TYPE_INT },
		{ "r9",       "r9",       false, TYPE_INT },
		{ "r10",      "r10",      false, TYPE_INT },
		{ "r11",      "r11",      false, TYPE_INT },
		{ "r12",      "r12",      false, TYPE_INT },
		{ "r13",      "r13",      false, TYPE_INT },
		{ "r14",      "r14",      false, TYPE_INT },
		{ "r15",      "r15",      false, TYPE_INT },
		{ "r16",      "r16",      false, TYPE_INT },
		{ "r17",      "r17",      false, TYPE_INT },
		{ "r18",      "r18",      false, TYPE_INT },
		{ "r19",      "r19",      false, TYPE_INT },
		{ "r20",      "r20",      false, TYPE_INT },
		{ "r21",      "r21",      false, TYPE_INT },
		{ "r22",      "r22",      false, TYPE_INT },
		{ "r23",      "r23",      false, TYPE_INT },
		{ "et",       "et",       false, TYPE_INT },
		{ "bt",       "bt",       false, TYPE_INT },
		{ "gp",       "gp",       false, TYPE_DATA_POINTER },
		{ "sp",       "sp",       true,  TYPE_DATA_POINTER },
		{ "fp",       "fp",       false, TYPE_DATA_POINTER },
		{ "ea",       "ea",       false, TYPE_CODE_POINTER },
		{ "ba",       "sstatus",  false, TYPE_INT }, // this is Altera's fault
		{ "ra",       "ra",       false, TYPE_CODE_POINTER },
		{ "status",   "status",   false, TYPE_INT },
		{ "estatus",  "estatus",  false, TYPE_INT },
		{ "bstatus",  "bstatus",  false, TYPE_INT },
		{ "ienable",  "ienable",  false, TYPE_INT },
		{ "ipending", "ipending", false, TYPE_INT },
		{ "cpuid",    "cpuid",    false, TYPE_INT },
		{ "ctl6",     "ctl6",     false, TYPE_INT },
		{ "exception","exception",false, TYPE_INT },
		{ "pteaddr",  "pteaddr",  false, TYPE_INT },
		{ "tlbacc",   "tlbacc",   false, TYPE_INT },
		{ "tlbmisc",  "tlbmisc",  false, TYPE_INT },
		{ "eccinj",   "eccinj",   false, TYPE_INT },
		{ "badaddr",  "badaddr",  false, TYPE_INT },
		{ "config",   "config",   false, TYPE_INT },
		{ "mpubase",  "mpubase",  false, TYPE_INT },
		{ "mpuacc",   "mpuacc",   false, TYPE_INT },
		{ "PC",       "pc",       true,  TYPE_CODE_POINTER },
	}
};

//-------------------------------------------------------------------------
static const std::map<std::string, const gdb_register_map &> gdb_register_maps = {
	{ "i486",       gdb_register_map_i486 },
	{ "arm7_le",    gdb_register_map_arm7 },
	{ "r4600",      gdb_register_map_r4600 },
	{ "ppc601",     gdb_register_map_ppc601 },
	{ "m68020pmmu", gdb_register_map_m68020pmmu },
	{ "m68000",     gdb_register_map_m68000 },
	{ "z80",        gdb_register_map_z80 },
	{ "z80n",       gdb_register_map_z80 },
	{ "z84c015",    gdb_register_map_z80 },
	{ "m6502",      gdb_register_map_m6502 },
	{ "m6507",      gdb_register_map_m6502 },
	{ "m6510",      gdb_register_map_m6502 },
	{ "m65c02",     gdb_register_map_m6502 },
	{ "m65ce02",    gdb_register_map_m6502 },
	{ "rp2a03",     gdb_register_map_m6502 },
	{ "w65c02s",    gdb_register_map_m6502 },
	{ "m6809",      gdb_register_map_m6809 },
	{ "score7",     gdb_register_map_score7 },
	{ "nios2",      gdb_register_map_nios2 },
};

//-------------------------------------------------------------------------
class debug_gdbstub : public osd_module, public debug_module
{
public:
	debug_gdbstub() :
		osd_module(OSD_DEBUG_PROVIDER, "gdbstub"), debug_module(),
		m_readbuf_state(PACKET_START),
		m_machine(nullptr),
		m_maincpu(nullptr),
		m_state(nullptr),
		m_memory(nullptr),
		m_address_space(nullptr),
		m_debugger_cpu(nullptr),
		m_debugger_console(nullptr),
		m_debugger_host(),
		m_debugger_port(0),
		m_socket(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE),
		m_is_be(false),
		m_initialized(false),
		m_dettached(false),
		m_extended_mode(false),
		m_send_stop_packet(false),
		m_target_xml_sent(false),
		m_triggered_breakpoint(nullptr),
		m_triggered_watchpoint(nullptr),
		m_readbuf_len(0),
		m_readbuf_offset(0),
		m_packet_len(0),
		m_packet_checksum(0),
		m_recv_checksum(0)
	{
	}

	virtual ~debug_gdbstub() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

	std::string get_register_string(int gdb_regnum);
	bool parse_register_string(uint64_t *pvalue, const char *buf, int gdb_regnum);

	bool parse_zZ(int *ptype, uint64_t *paddress, int *pkind, const char *buf);

	void set_register_value(int gdb_regnum, uint64_t value);

	bool is_thread_id_ok(const char *buf);

	void handle_character(char ch);
	void send_nack();
	void send_ack();
	void handle_packet();

	enum cmd_reply
	{
		REPLY_NONE,
		REPLY_OK,
		REPLY_ENN,
		REPLY_UNSUPPORTED,
	};

	cmd_reply handle_exclamation(const char *buf);
	cmd_reply handle_question(const char *buf);
	cmd_reply handle_c(const char *buf);
	cmd_reply handle_D(const char *buf);
	cmd_reply handle_g(const char *buf);
	cmd_reply handle_G(const char *buf);
	cmd_reply handle_H(const char *buf);
	cmd_reply handle_i(const char *buf);
	cmd_reply handle_I(const char *buf);
	cmd_reply handle_k(const char *buf);
	cmd_reply handle_m(const char *buf);
	cmd_reply handle_M(const char *buf);
	cmd_reply handle_p(const char *buf);
	cmd_reply handle_P(const char *buf);
	cmd_reply handle_q(const char *buf);
	cmd_reply handle_s(const char *buf);
	cmd_reply handle_T(const char *buf);
	cmd_reply handle_z(const char *buf);
	cmd_reply handle_Z(const char *buf);

	enum readbuf_state
	{
		PACKET_START,
		PACKET_DATA,
		PACKET_CHECKSUM1,
		PACKET_CHECKSUM2,
	};

	readbuf_state m_readbuf_state;

	void generate_target_xml();

	int readchar();

	void send_reply(std::string_view str);
	void send_stop_packet();

private:
	running_machine *m_machine;
	device_t *m_maincpu;
	device_state_interface *m_state;
	device_memory_interface *m_memory;
	address_space *m_address_space;
	debugger_cpu *m_debugger_cpu;
	debugger_console *m_debugger_console;
	std::string m_debugger_host;
	int m_debugger_port;
	emu_file m_socket;
	bool m_is_be;
	bool m_initialized;
	bool m_dettached;
	bool m_extended_mode;
	bool m_send_stop_packet;
	bool m_target_xml_sent;     // the 'g', 'G', 'p', and 'P' commands only work once target.xml has been sent

	struct gdb_register
	{
		std::string gdb_name;
		int gdb_regnum;
		gdb_register_type gdb_type;
		int gdb_bitsize;
		int state_index;
	};
	std::vector<gdb_register> m_gdb_registers;
	std::set<int> m_stop_reply_registers;
	std::string m_gdb_arch;
	std::string m_gdb_feature;

	std::map<offs_t, uint64_t> m_address_map;

	debug_breakpoint *m_triggered_breakpoint;
	debug_watchpoint *m_triggered_watchpoint;

	std::string m_target_xml;

	uint8_t  m_readbuf[512];
	uint32_t m_readbuf_len;
	uint32_t m_readbuf_offset;

	uint8_t m_packet_buf[MAX_PACKET_SIZE+1];
	int     m_packet_len;
	uint8_t m_packet_checksum;
	uint8_t m_recv_checksum;
};

//-------------------------------------------------------------------------
int debug_gdbstub::init(osd_interface &osd, const osd_options &options)
{
	m_debugger_host = options.debugger_host();
	m_debugger_port = options.debugger_port();
	return 0;
}

//-------------------------------------------------------------------------
void debug_gdbstub::exit()
{
}

//-------------------------------------------------------------------------
void debug_gdbstub::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

//-------------------------------------------------------------------------
int debug_gdbstub::readchar()
{
	// NOTE: we don't use m_socket.getc() because it does not work with
	//       sockets (it assumes seeking is possible).

	if ( !m_socket.is_open() )
		return -1;

	if ( m_readbuf_offset == m_readbuf_len )
	{
		m_readbuf_offset = 0;
		m_readbuf_len = m_socket.read(m_readbuf, sizeof(m_readbuf));
		if ( m_readbuf_len == 0 )
			return -1;
	}

	return (int) m_readbuf[m_readbuf_offset++];
}

//-------------------------------------------------------------------------
static std::string escape_packet(std::string_view src)
{
	std::string result;
	result.reserve(src.length());
	for ( char ch : src )
	{
		if ( ch == '#' || ch == '$' || ch == '}' )
		{
			result += '}';
			ch ^= 0x20;
		}
		result += ch;
	}
	return result;
}

//-------------------------------------------------------------------------
void debug_gdbstub::generate_target_xml()
{
	// Note: we do not attempt to replicate the regnum values from old
	//       GDB clients that did not support target.xml.
	std::string target_xml;
	target_xml += "<?xml version=\"1.0\"?>\n";
	target_xml += "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">\n";
	target_xml += "<target version=\"1.0\">\n";
	target_xml += string_format("<architecture>%s</architecture>\n", m_gdb_arch);
	target_xml += string_format("  <feature name=\"%s\">\n", m_gdb_feature);
	for ( const auto &reg: m_gdb_registers )
		target_xml += string_format("    <reg name=\"%s\" bitsize=\"%d\" type=\"%s\"/>\n", reg.gdb_name, reg.gdb_bitsize, gdb_register_type_str[reg.gdb_type]);
	target_xml += "  </feature>\n";
	target_xml += "</target>\n";
	m_target_xml = escape_packet(target_xml);
}

//-------------------------------------------------------------------------
void debug_gdbstub::wait_for_debugger(device_t &device, bool firststop)
{
	if ( m_dettached )
		return;

	if ( firststop && !m_initialized )
	{
		// find the "main" CPU, which is the first CPU (gdbstub doesn't have any notion of switching CPUs)
		m_maincpu = device_interface_enumerator<cpu_device>(m_machine->root_device()).first();
		if (!m_maincpu)
			fatalerror("gdbstub: cannot find any CPUs\n");

		const char *cpuname = m_maincpu->shortname();
		auto it = gdb_register_maps.find(cpuname);
		if ( it == gdb_register_maps.end() )
			fatalerror("gdbstub: cpuname %s not found in gdb stub descriptions\n", cpuname);

		m_maincpu->interface(m_state);
		assert(m_state != nullptr);
		m_memory = &m_maincpu->memory();
		m_address_space = &m_memory->space(AS_PROGRAM);
		m_debugger_cpu = &m_machine->debugger().cpu();
		m_debugger_console = &m_machine->debugger().console();

		m_is_be = m_address_space->endianness() == ENDIANNESS_BIG;

#if 0
		for ( const auto &entry: m_state->state_entries() )
		{
			const char *symbol = entry->symbol();
			uint8_t datasize = entry->datasize();
			uint64_t datamask = entry->datamask();
			int index = entry->index();
			const std::string &format_string = entry->format_string();
			osd_printf_info("[%3d] datasize %d mask %016" PRIx64 " [%s] [%s]\n", index, datasize, datamask, symbol, format_string);
		}
#endif

		const gdb_register_map &register_map = it->second;
		m_gdb_arch = register_map.arch;
		m_gdb_feature = register_map.feature;
		int cur_gdb_regnum = 0;
		for ( const auto &reg: register_map.registers )
		{
			bool added = false;
			for ( const auto &entry: m_state->state_entries() )
			{
				const char *symbol = entry->symbol();
				if ( strcmp(symbol, reg.state_name) == 0 )
				{
					gdb_register new_reg;
					new_reg.gdb_name = reg.gdb_name;
					new_reg.gdb_regnum = cur_gdb_regnum;
					new_reg.gdb_type = reg.gdb_type;
					if ( reg.override_bitsize != -1 )
						new_reg.gdb_bitsize = reg.override_bitsize;
					else
						new_reg.gdb_bitsize = entry->datasize() * 8;
					new_reg.state_index = entry->index();
					m_gdb_registers.push_back(std::move(new_reg));
					if ( reg.stop_packet )
						m_stop_reply_registers.insert(cur_gdb_regnum);
					added = true;
					cur_gdb_regnum++;
					break;
				}
			}
			if ( !added )
				osd_printf_info("gdbstub: could not find register [%s]\n", reg.gdb_name);
		}

#if 0
		for ( const auto &reg: m_gdb_registers )
			osd_printf_info(" %3d (%d) %d %d [%s]\n", reg.gdb_regnum, reg.state_index, reg.gdb_bitsize, reg.gdb_type, reg.gdb_name);
#endif

		std::string socket_name = string_format("socket.%s:%d", m_debugger_host, m_debugger_port);
		std::error_condition const filerr = m_socket.open(socket_name);
		if ( filerr )
			fatalerror("gdbstub: failed to start listening on address %s port %d\n", m_debugger_host, m_debugger_port);
		osd_printf_info("gdbstub: listening on address %s port %d\n", m_debugger_host, m_debugger_port);

		m_initialized = true;
	}
	else
	{
		device_debug *debug = m_debugger_console->get_visible_cpu()->debug();
		m_triggered_watchpoint = debug->triggered_watchpoint();
		m_triggered_breakpoint = debug->triggered_breakpoint();
		if ( m_send_stop_packet )
		{
			send_stop_packet();
			m_send_stop_packet = false;
		}
	}

	while ( m_debugger_cpu->is_stopped() )
	{
		int ch = readchar();
		if ( ch < 0 )
		{
			// TODO add support for timeout in *_osd_socket.
			// To prevent 100% cpu usage while waiting for data to
			// arrive from the socket, we sleep for 1 millisecond.
			osd_sleep(osd_ticks_per_second() / 1000);
			continue;
		}
		handle_character((char) ch);
	}
}

//-------------------------------------------------------------------------
void debug_gdbstub::debugger_update()
{
	while ( true )
	{
		int ch = readchar();
		if ( ch < 0 )
			break;
		handle_character((char) ch);
	}
}

//-------------------------------------------------------------------------
void debug_gdbstub::send_nack()
{
	m_socket.write("-", 1);
}

//-------------------------------------------------------------------------
void debug_gdbstub::send_ack()
{
	m_socket.write("+", 1);
}

//-------------------------------------------------------------------------
void debug_gdbstub::send_reply(std::string_view str)
{
	uint8_t checksum = 0;
	for ( char ch : str )
		checksum += ch;

	std::string reply = string_format("$%s#%02x", str, checksum);
	m_socket.write(reply.c_str(), reply.length());
}


//-------------------------------------------------------------------------
// Enable extended mode.
debug_gdbstub::cmd_reply debug_gdbstub::handle_exclamation(const char *buf)
{
	m_extended_mode = true;
	return REPLY_OK;
}

//-------------------------------------------------------------------------
// Indicate the reason the target halted.
debug_gdbstub::cmd_reply debug_gdbstub::handle_question(const char *buf)
{
	send_stop_packet();
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Continue at addr.
debug_gdbstub::cmd_reply debug_gdbstub::handle_c(const char *buf)
{
	// We don't support continuing with addr.
	if ( *buf != '\0' )
		return REPLY_UNSUPPORTED;

	m_debugger_console->get_visible_cpu()->debug()->go();
	m_send_stop_packet = true;
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Detach.
debug_gdbstub::cmd_reply debug_gdbstub::handle_D(const char *buf)
{
	// We don't support dettaching with pid.
	if ( *buf != '\0' )
		return REPLY_UNSUPPORTED;

	m_debugger_console->get_visible_cpu()->debug()->go();
	m_dettached = true;

	return REPLY_OK;
}

//-------------------------------------------------------------------------
// Read general registers.
debug_gdbstub::cmd_reply debug_gdbstub::handle_g(const char *buf)
{
	if ( !m_target_xml_sent )
		return REPLY_ENN;
	if ( *buf != '\0' )
		return REPLY_UNSUPPORTED;
	std::string reply;
	for ( const auto &reg: m_gdb_registers )
		reply += get_register_string(reg.gdb_regnum);
	send_reply(reply);
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Write general registers.
debug_gdbstub::cmd_reply debug_gdbstub::handle_G(const char *buf)
{
	if ( !m_target_xml_sent )
		return REPLY_ENN;
	for ( const auto &reg: m_gdb_registers )
	{
		uint64_t value;
		if ( !parse_register_string(&value, buf, reg.gdb_regnum) )
			return REPLY_ENN;
		set_register_value(reg.gdb_regnum, value);
		buf += (reg.gdb_bitsize / 8) * 2;
	}
	if ( *buf != '\0' )
		return REPLY_ENN;
	return REPLY_OK;
}

//-------------------------------------------------------------------------
// Set thread for subsequent operations.
debug_gdbstub::cmd_reply debug_gdbstub::handle_H(const char *buf)
{
	// accept threads 'any', 1, and 'all'
	if ( (buf[0] == 'c' || buf[0] == 'g') && is_thread_id_ok(buf + 1) )
		return REPLY_OK;
	// otherwise silently ignore
	return REPLY_UNSUPPORTED;
}

//-------------------------------------------------------------------------
// Kill request.
debug_gdbstub::cmd_reply debug_gdbstub::handle_k(const char *buf)
{
	m_machine->schedule_exit();
	m_debugger_console->get_visible_cpu()->debug()->go();
	m_dettached = true;
	m_socket.close();
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Read memory.
debug_gdbstub::cmd_reply debug_gdbstub::handle_m(const char *buf)
{
	uint64_t address;
	uint64_t length;
	if ( sscanf(buf, "%" PRIx64 ",%" PRIx64, &address, &length) != 2 )
		return REPLY_ENN;

	offs_t offset = address;
	address_space *tspace;
	if ( !m_memory->translate(m_address_space->spacenum(), device_memory_interface::TR_READ, offset, tspace) )
		return REPLY_ENN;

	// Disable side effects while reading memory.
	auto dis = m_machine->disable_side_effects();

	std::string reply;
	reply.reserve(length * 2);
	for ( int i = 0; i < length; i++ )
	{
		uint8_t value = tspace->read_byte(offset + i);
		reply += string_format("%02x", value);
	}
	send_reply(reply);

	return REPLY_NONE;
}

//-------------------------------------------------------------------------
static bool hex_decode(std::vector<uint8_t> *_data, const char *buf, size_t length)
{
	std::vector<uint8_t> &data = *_data;
	data.resize(length);
	for ( int i = 0; i < length; i++ )
	{
		if ( sscanf(buf, "%02hhx", &data[i]) != 1 )
			return false;
		buf += 2;
	}
	if ( *buf != '\0' )
		return false;
	return true;
}

//-------------------------------------------------------------------------
// Write memory.
debug_gdbstub::cmd_reply debug_gdbstub::handle_M(const char *buf)
{
	uint64_t address;
	uint64_t length;
	int buf_offset;
	if ( sscanf(buf, "%" PRIx64 ",%" PRIx64 ":%n", &address, &length, &buf_offset) != 2 )
		return REPLY_ENN;

	offs_t offset = address;
	address_space *tspace;
	if ( !m_memory->translate(m_address_space->spacenum(), device_memory_interface::TR_READ, offset, tspace) )
		return REPLY_ENN;

	std::vector<uint8_t> data;
	if ( !hex_decode(&data, buf + buf_offset, length) )
		return REPLY_ENN;

	for ( int i = 0; i < length; i++ )
		tspace->write_byte(offset + i, data[i]);

	return REPLY_OK;
}

//-------------------------------------------------------------------------
// Read the value of register n.
debug_gdbstub::cmd_reply debug_gdbstub::handle_p(const char *buf)
{
	if ( !m_target_xml_sent )
		return REPLY_ENN;
	int gdb_regnum;
	if ( sscanf(buf, "%x", &gdb_regnum) != 1 || gdb_regnum >= m_gdb_registers.size() )
		return REPLY_ENN;
	std::string reply = get_register_string(gdb_regnum);
	send_reply(reply);
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Write register n… with value r….
debug_gdbstub::cmd_reply debug_gdbstub::handle_P(const char *buf)
{
	if ( !m_target_xml_sent )
		return REPLY_ENN;
	int gdb_regnum;
	int buf_offset;
	if ( sscanf(buf, "%x=%n", &gdb_regnum, &buf_offset) != 1 || gdb_regnum >= m_gdb_registers.size() )
		return REPLY_ENN;
	buf += buf_offset;
	uint64_t value;
	if ( !parse_register_string(&value, buf, gdb_regnum) )
		return REPLY_ENN;
	set_register_value(gdb_regnum, value);
	return REPLY_OK;
}

//-------------------------------------------------------------------------
// General query.
debug_gdbstub::cmd_reply debug_gdbstub::handle_q(const char *buf)
{
	// First check for packets that predate the qname:params convention.
	if ( *buf == 'C' )
	{
		// Return the current thread ID.
		send_reply("QC1");
		return REPLY_NONE;
	}
	else if ( *buf == 'P' )
	{
		// Returns information on thread-id.
		return REPLY_UNSUPPORTED;
	}
	else if ( *buf == 'L' )
	{
		// Obtain thread information from RTOS.
		return REPLY_UNSUPPORTED;
	}

	// Check packets that use qname,params convention.
	if ( strncmp(buf, "Rcmd,", 5) == 0 )
	{
		buf += 5;
		std::vector<uint8_t> data;
		if ( !hex_decode(&data, buf, strlen(buf) / 2) )
			return REPLY_ENN;
		std::string command(data.begin(), data.end());
		text_buffer &textbuf = m_debugger_console->get_console_textbuf();
		text_buffer_clear(textbuf);
		m_debugger_console->execute_command(command, false);
		uint32_t nlines = text_buffer_num_lines(textbuf);
		if ( nlines == 0 )
			return REPLY_OK;
		std::string reply;
		for ( uint32_t i = 0; i < nlines; i++ )
		{
			const char *line = text_buffer_get_seqnum_line(textbuf, i);
			reply.reserve(reply.length() + (strlen(line)+1)*2);
			while ( *line != '\0' )
				reply += string_format("%02x", *line++);
			reply += "0A";
		}
		send_reply(reply);
		return REPLY_NONE;
	}

	// Split name and parameters
	const char *ptr = buf;
	while ( *ptr != '\0' && *ptr != ':' )
		ptr++;
	std::string name(buf, ptr-buf);
	std::string params;
	if ( *ptr != '\0' )
		params = ptr+1;

	if ( name == "Supported" )
	{
		std::string reply = string_format("PacketSize=%x", MAX_PACKET_SIZE);
		reply += ";qXfer:features:read+";
		send_reply(reply);
		return REPLY_NONE;
	}
	else if ( name == "Xfer" )
	{
		// "features:read:target.xml:0,3fff"
		if ( strncmp(params.c_str(), "features:read:", 14) == 0 )
		{
			int offset = 0;
			int length = 0;
			if ( sscanf(params.c_str() + 14, "target.xml:%x,%x", &offset, &length) == 2 )
			{
				if ( m_target_xml.empty() )
					generate_target_xml();
				length = std::min(length, (int) m_target_xml.length()-offset);
				std::string reply;
				if ( offset + length < m_target_xml.length() )
					reply += 'm';
				else
					reply += 'l';
				reply += m_target_xml.substr(offset, length);
				send_reply(reply);
				m_target_xml_sent = true;
				return REPLY_NONE;
			}
		}
	}
	else if ( name == "fThreadInfo" )
	{
		send_reply("m1");
		return REPLY_NONE;
	}
	else if ( name == "sThreadInfo" )
	{
		send_reply("l");
		return REPLY_NONE;
	}

	return REPLY_UNSUPPORTED;
}

//-------------------------------------------------------------------------
// Single step, resuming at addr.
debug_gdbstub::cmd_reply debug_gdbstub::handle_s(const char *buf)
{
	// We don't support stepping with addr.
	if ( *buf != '\0' )
		return REPLY_UNSUPPORTED;

	m_debugger_console->get_visible_cpu()->debug()->single_step();
	m_send_stop_packet = true;
	return REPLY_NONE;
}

//-------------------------------------------------------------------------
// Find out if the thread XX is alive.
debug_gdbstub::cmd_reply debug_gdbstub::handle_T(const char *buf)
{
	if ( is_thread_id_ok(buf) )
		return REPLY_OK;

	// thread is dead
	return REPLY_ENN;
}

//-------------------------------------------------------------------------
static bool remove_breakpoint(device_debug *debug, uint64_t address, int /*kind*/)
{
	const debug_breakpoint *bp = debug->breakpoint_find(address);
	if (bp != nullptr)
		return debug->breakpoint_clear(bp->index());
	return false;
}

//-------------------------------------------------------------------------
static bool remove_watchpoint(device_debug *debug, read_or_write type, uint64_t address, int kind)
{
	const auto &watchpoints = debug->watchpoint_vector(AS_PROGRAM);
	for ( const auto &wp: watchpoints )
		if ( wp->type() == type && wp->address() == address && wp->length() == kind )
			return debug->watchpoint_clear(wp->index());
	return false;
}

//-------------------------------------------------------------------------
bool debug_gdbstub::parse_zZ(int *ptype, uint64_t *paddress, int *pkind, const char *buf)
{
	int buf_offset;
	if ( sscanf(buf, "%d,%" PRIx64 ",%x%n", ptype, paddress, pkind, &buf_offset) != 3 || buf[buf_offset] != '\0' )
		return false;
	return true;
}

//-------------------------------------------------------------------------
// Remove breakpoint or watchpoint.
debug_gdbstub::cmd_reply debug_gdbstub::handle_z(const char *buf)
{
	int type;
	uint64_t address;
	int kind;
	if ( !parse_zZ(&type, &address, &kind, buf) )
		return REPLY_ENN;

	// watchpoints
	offs_t offset = address;
	address_space *tspace;
	if ( type == 2 || type == 3 || type == 4 )
	{
		if ( !m_memory->translate(m_address_space->spacenum(), device_memory_interface::TR_READ, offset, tspace) )
			return REPLY_ENN;
		m_address_map.erase(offset);
	}

	device_debug *debug = m_debugger_console->get_visible_cpu()->debug();
	switch ( type )
	{
		// Note: software and hardware breakpoints are treated both the
		//       same way, and the 'kind' parameter is ignored.
		case 0: // software breakpoint
		case 1: // hardware breakpoint
			return remove_breakpoint(debug, address, kind) ? REPLY_OK : REPLY_ENN;
		case 2:
			// write watchpoint
			return remove_watchpoint(debug, read_or_write::WRITE, offset, kind) ? REPLY_OK : REPLY_ENN;
		case 3:
			// read watchpoint
			return remove_watchpoint(debug, read_or_write::READ, offset, kind) ? REPLY_OK : REPLY_ENN;
		case 4:
			// access watchpoint
			return remove_watchpoint(debug, read_or_write::READWRITE, offset, kind) ? REPLY_OK : REPLY_ENN;
	}

	return REPLY_UNSUPPORTED;
}

//-------------------------------------------------------------------------
// Insert breakpoint or watchpoint.
debug_gdbstub::cmd_reply debug_gdbstub::handle_Z(const char *buf)
{
	int type;
	uint64_t address;
	int kind;
	if ( !parse_zZ(&type, &address, &kind, buf) )
		return REPLY_ENN;

	// watchpoints
	offs_t offset = address;
	address_space *tspace;
	if ( type == 2 || type == 3 || type == 4 )
	{
		if ( !m_memory->translate(m_address_space->spacenum(), device_memory_interface::TR_READ, offset, tspace) )
			return REPLY_ENN;
		m_address_map[offset] = address;
	}

	device_debug *debug = m_debugger_console->get_visible_cpu()->debug();
	switch ( type )
	{
		// Note: software and hardware breakpoints are treated both the
		//       same way, and the 'kind' parameter is ignored.
		case 0: // software breakpoint
		case 1: // hardware breakpoint
			debug->breakpoint_set(address);
			return REPLY_OK;
		case 2:
			// write watchpoint
			debug->watchpoint_set(*m_address_space, read_or_write::WRITE, offset, kind);
			return REPLY_OK;
		case 3:
			// read watchpoint
			debug->watchpoint_set(*m_address_space, read_or_write::READ, offset, kind);
			return REPLY_OK;
		case 4:
			// access watchpoint
			debug->watchpoint_set(*m_address_space, read_or_write::READWRITE, offset, kind);
			return REPLY_OK;
	}

	return REPLY_UNSUPPORTED;
}


//-------------------------------------------------------------------------
void debug_gdbstub::send_stop_packet()
{
	int signal = 5; // GDB_SIGNAL_TRAP
	std::string reply = string_format("T%02x", signal);
	if ( m_triggered_watchpoint != nullptr )
	{
		switch ( m_triggered_watchpoint->type() )
		{
			case read_or_write::WRITE:
				reply += "watch";
				break;
			case read_or_write::READ:
				reply += "rwatch";
				break;
			case read_or_write::READWRITE:
				reply += "awatch";
				break;
		}
		offs_t offset = m_triggered_watchpoint->address();
		uint64_t address = m_address_map[offset];
		reply += string_format(":%" PRIx64 ";", address);
	}
	if ( m_target_xml_sent )
		for ( const auto &gdb_regnum: m_stop_reply_registers )
			reply += string_format("%02x:%s;", gdb_regnum, get_register_string(gdb_regnum));
	send_reply(reply);
}

//-------------------------------------------------------------------------
void debug_gdbstub::handle_packet()
{
	// For any command not supported by the stub, an empty response
	// (‘$#00’) should be returned. That way it is possible to extend
	// the protocol. A newer GDB can tell if a packet is supported
	// based on that response.
	cmd_reply reply = REPLY_UNSUPPORTED;

	const char *buf = (const char *) m_packet_buf+1;
	switch ( m_packet_buf[0] )
	{
		case '!': reply = handle_exclamation(buf); break;
		case '?': reply = handle_question(buf); break;
		case 'c': reply = handle_c(buf); break;
		case 'D': reply = handle_D(buf); break;
		case 'g': reply = handle_g(buf); break;
		case 'G': reply = handle_G(buf); break;
		case 'H': reply = handle_H(buf); break;
		case 'k': reply = handle_k(buf); break;
		case 'm': reply = handle_m(buf); break;
		case 'M': reply = handle_M(buf); break;
		case 'p': reply = handle_p(buf); break;
		case 'P': reply = handle_P(buf); break;
		case 'q': reply = handle_q(buf); break;
		case 's': reply = handle_s(buf); break;
		case 'T': reply = handle_T(buf); break;
		case 'z': reply = handle_z(buf); break;
		case 'Z': reply = handle_Z(buf); break;
	}
	if ( reply == REPLY_OK )
		send_reply("OK");
	else if ( reply == REPLY_ENN )
		send_reply("E01");
	else if ( reply == REPLY_UNSUPPORTED )
		send_reply("");
}

//-------------------------------------------------------------------------
std::string debug_gdbstub::get_register_string(int gdb_regnum)
{
	const gdb_register &reg = m_gdb_registers[gdb_regnum];
	const char *fmt = (reg.gdb_bitsize == 64) ? "%016" PRIx64
					: (reg.gdb_bitsize == 32) ? "%08"  PRIx64
					: (reg.gdb_bitsize == 16) ? "%04"  PRIx64
					:                           "%02"  PRIx64;
	uint64_t value = m_state->state_int(reg.state_index);
	if ( reg.gdb_bitsize < 64 )
		value &= (1ULL << reg.gdb_bitsize) - 1;
	if ( !m_is_be )
	{
		value = (reg.gdb_bitsize == 64) ? swapendian_int64(value)
			  : (reg.gdb_bitsize == 32) ? swapendian_int32(value)
			  : (reg.gdb_bitsize == 16) ? swapendian_int16(value)
			  :                           value;
	}
	return string_format(fmt, value);
}

//-------------------------------------------------------------------------
bool debug_gdbstub::parse_register_string(uint64_t *pvalue, const char *buf, int gdb_regnum)
{
	const gdb_register &reg = m_gdb_registers[gdb_regnum];
	const char *fmt = (reg.gdb_bitsize == 64) ? "%016" PRIx64
					: (reg.gdb_bitsize == 32) ? "%08"  PRIx64
					: (reg.gdb_bitsize == 16) ? "%04"  PRIx64
					:                           "%02"  PRIx64;
	uint64_t value;
	if ( sscanf(buf, fmt, &value) != 1 )
		return false;
	if ( !m_is_be )
	{
		value = (reg.gdb_bitsize == 64) ? swapendian_int64(value)
			  : (reg.gdb_bitsize == 32) ? swapendian_int32(value)
			  : (reg.gdb_bitsize == 16) ? swapendian_int16(value)
			  :                           value;
	}
	*pvalue = value;
	return true;
}

//-------------------------------------------------------------------------
void debug_gdbstub::set_register_value(int gdb_regnum, uint64_t value)
{
	const gdb_register &reg = m_gdb_registers[gdb_regnum];
	m_state->set_state_int(reg.state_index, value);
}

//-------------------------------------------------------------------------
bool debug_gdbstub::is_thread_id_ok(const char *buf)
{
	// 'any'
	if ( buf[0] == '0' && buf[1] == '\0' )
		return true;
	// The thread id we reported.
	if ( buf[0] == '1' && buf[1] == '\0' )
		return true;
	// 'all'
	if ( buf[0] == '-' && buf[1] == '1' && buf[2] == '\0' )
		return true;
	return false;
}

//-------------------------------------------------------------------------
void debug_gdbstub::handle_character(char ch)
{
	int8_t nibble;
	switch ( m_readbuf_state )
	{
		case PACKET_START:
			if ( ch == '$' )
			{
				m_packet_len = 0;
				m_packet_checksum = 0;
				m_readbuf_state = PACKET_DATA;
			}
			else if ( ch == '\x03' )
			{
				m_debugger_cpu->set_execution_stopped();
			}
			break;
		case PACKET_DATA:
			if ( ch == '#' )
			{
				m_readbuf_state = PACKET_CHECKSUM1;
			}
			else if ( m_packet_len >= MAX_PACKET_SIZE )
			{
				osd_printf_info("gdbstub: packet buffer overflow!\n");
				m_readbuf_state = PACKET_START;
			}
			else
			{
				m_packet_buf[m_packet_len++] = ch;
				m_packet_checksum += ch;
			}
			break;
		case PACKET_CHECKSUM1:
			if ( sscanf(&ch, "%01hhx", &nibble) != 1 )
			{
				osd_printf_info("gdbstub: invalid checksum!\n");
				m_readbuf_state = PACKET_START;
				break;
			}
			m_recv_checksum = nibble;
			m_readbuf_state = PACKET_CHECKSUM2;
			break;
		case PACKET_CHECKSUM2:
			if ( sscanf(&ch, "%01hhx", &nibble) != 1 )
			{
				osd_printf_info("gdbstub: invalid checksum!\n");
				m_readbuf_state = PACKET_START;
				break;
			}
			m_recv_checksum <<= 4;
			m_recv_checksum |= nibble;
			if ( m_recv_checksum != m_packet_checksum )
			{
				osd_printf_info("gdbstub: bad checksum!\n");
				send_nack();
				m_readbuf_state = PACKET_START;
				break;
			}
			m_packet_buf[m_packet_len] = '\0';
			send_ack();
			handle_packet();
			m_readbuf_state = PACKET_START;
			break;
	}
}

} // anonymous namespace

} // namespace osd

//-------------------------------------------------------------------------
MODULE_DEFINITION(DEBUG_GDBSTUB, osd::debug_gdbstub)
