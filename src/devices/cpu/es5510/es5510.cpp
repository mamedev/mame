// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
 *
 *   es5510.c - Ensoniq ES5510 (ESP) emulation
 *   by Christian Brunschen
 *
 ***************************************************************************/

#include <cstdio>
#include "emu.h"
#include "debugger.h"
#include "es5510.h"
#include "cpu/m68000/m68000.h"

static const INT32 MIN_24 = -(1 << 23);
static const INT32 MAX_24 = (1 << 23) - 1;

static const INT64 MIN_48 = -(S64(1) << 47);
static const INT64 MAX_48 = (S64(1) << 47) - 1;

#define SIGN_BIT_24 (0x00800000)
#define GET_SIGN_BIT_24(x) ((x) & SIGN_BIT_24)
#define IS_NEGATIVE(x) (((x) & SIGN_BIT_24) != 0)

#define CARRY_OUT_24 (0x01000000)

static inline INT32 SX(INT32 x) { return IS_NEGATIVE(x) ? x | 0xff000000 : x & 0x00ffffff; }
static inline INT32 SC(INT32 x) { return x & 0x00ffffff; }
static inline INT64 SX64(INT64 x) { return (x & S64(0x0000800000000000)) ? x | S64(0xffff000000000000) : x & S64(0x0000ffffffffffff); }
//static inline INT64 SC64(INT64 x) { return x & S64(0x0000ffffffffffff); }

#define VERBOSE 0
#define VERBOSE_EXEC 0

#if VERBOSE
static inline void ATTR_PRINTF(1,2) log_to_stderr(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

#define LOG(x) do { log_to_stderr x; } while(0)
#else
#define LOG(x)
#endif


#if VERBOSE_EXEC
static int exec_cc = 0;
#define LOG_EXEC(x) do { if (!exec_cc) LOG(x); } while(0)
#else
#define LOG_EXEC(x)
#endif

const device_type ES5510 = &device_creator<es5510_device>;

#define FLAG_N (1 << 7)
#define FLAG_C (1 << 6)
#define FLAG_V (1 << 5)
#define FLAG_LT (1 << 4)
#define FLAG_Z (1 << 3)
#define FLAG_NOT (1 << 2)

#define FLAG_MASK (FLAG_N | FLAG_C | FLAG_V | FLAG_LT | FLAG_Z)

char *stpcpy_int (char *dst, const char *src)
{
	const size_t len = strlen (src);
	return (char *) memcpy (dst, src, len + 1) + len;
}

inline static UINT8 setFlag(UINT8 ccr, UINT8 flag) {
	return ccr | flag;
}

inline static UINT8 clearFlag(UINT8 ccr, UINT8 flag) {
	return ccr & ~flag;
}

inline static UINT8 setFlagTo(UINT8 ccr, UINT8 flag, bool set) {
	return set ? setFlag(ccr, flag) : clearFlag(ccr, flag);
}

inline static bool isFlagSet(UINT8 ccr, UINT8 flag) {
	return (ccr & flag) != 0;
}

inline static INT32 add(INT32 a, INT32 b, UINT8 &flags) {
	INT32 aSign = a & SIGN_BIT_24;
	INT32 bSign = b & SIGN_BIT_24;
	INT32 result = a + b;
	INT32 resultSign = result & SIGN_BIT_24;
	bool overflow = (aSign == bSign) && (aSign != resultSign);
	bool carry = result & CARRY_OUT_24;
	bool negative = resultSign != 0;
	bool lessThan = (overflow && !negative) || (!overflow && negative);
	flags = setFlagTo(flags, FLAG_C, carry);
	flags = setFlagTo(flags, FLAG_N, negative);
	flags = setFlagTo(flags, FLAG_Z, result == 0);
	flags = setFlagTo(flags, FLAG_V, overflow);
	flags = setFlagTo(flags, FLAG_LT, lessThan);
	return SC(result);
}

inline static INT32 saturate(INT32 value, UINT8 &flags, bool negative) {
	if (isFlagSet(flags, FLAG_V)) {
		setFlagTo(flags, FLAG_N, negative);
		return negative ? MIN_24 : MAX_24;
	} else {
		return value;
	}
}

inline static INT32 negate(INT32 value) {
	return ((value ^ 0x00ffffff) + 1) & 0x00ffffff;
}

inline static INT32 asl(INT32 value, int shift, UINT8 &flags) {
	INT32 signBefore = value & SIGN_BIT_24;
	INT32 result = value << shift;
	INT32 signAfter = result & SIGN_BIT_24;
	bool overflow = signBefore != signAfter;
	flags = setFlagTo(flags, FLAG_V, overflow);
	return saturate(result, flags, signBefore != 0);
}

es5510_device::es5510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: cpu_device(mconfig, ES5510, "ES5510", tag, owner, clock, "es5510", __FILE__)
{
	// Initialize ESP to mostly zeroed, configured for 64k samples of delay line memory, running (not halted)
	halt_asserted = false;
	icount = 0;
	pc = 0;
	state = STATE_HALTED;
	memset(gpr, 0, 0xc0 * sizeof(gpr[0]));
	ser0r = 0;
	ser0l = 0;
	ser1r = 0;
	ser1l = 0;
	ser2r = 0;
	ser2l = 0;
	ser3r = 0;
	ser3l = 0;
	machl = 0;
	dil   = 0;
	memsiz       = 0x00ffffff;
	memmask      = 0x00000000;
	memincrement = 0x01000000;
	memshift     = 24;
	dlength      = 0;
	abase        = 0;
	bbase        = 0;
	dbase        = 0;
	sigreg       = 1;
	mulshift     = 1;
	ccr = 0;
	cmr = 0;
	dol[0] = dol[1] = 0;
	dol_count = 0;

	memset(instr, 0, 160 * sizeof(instr[0]));
	memset(dram, 0, (1<<20) * sizeof(dram[0]));

	dol_latch = 0;
	dil_latch = 0;
	dadr_latch = 0;
	gpr_latch = 0;
	instr_latch = 0;
	ram_sel = 0;
	host_control = 0;

	pc = 0;
	memset(&alu, 0, sizeof(alu));
	memset(&mulacc, 0, sizeof(mulacc));
}

typedef es5510_device::alu_op_t alu_op_t;
typedef es5510_device::op_select_t op_select_t;
typedef es5510_device::op_src_dst_t op_src_dst_t;

static inline const char * REGNAME(UINT8 r) {
	static char rn[8];
	if (r < 234) { sprintf(rn, "GPR_%02x", r); return rn; }
	switch(r) {
	case 234: return "SER0R";
	case 235: return "SER0L";
	case 236: return "SER1R";
	case 237: return "SER1L";
	case 238: return "SER2R";
	case 239: return "SER2L";
	case 240: return "SER3R";
	case 241: return "SER3L";
	case 242: return "MACL";
	case 243: return "MACH";
	case 244: return "DIL/MEMSIZ";
	case 245: return "DLENGTH";
	case 246: return "ABASE";
	case 247: return "BBASE";
	case 248: return "DBASE";
	case 249: return "SIGREG";
	case 250: return "CCR";
	case 251: return "CMR";
	case 252: return "MINUS1";
	case 253: return "MIN";
	case 254: return "MAX";
	case 255: return "ZERO";
	}
	return nullptr;
}

static inline char * DESCRIBE_REG(char *s, UINT8 r, const char *name) {
	if (name && *name) {
		return s + sprintf(s, "%s/%s", REGNAME(r), name);
	} else {
		return stpcpy_int(s, REGNAME(r));
	}

	// never executed
	//return 0;
}

const alu_op_t es5510_device::ALU_OPS[16] = {
	{ 2, "ADD" },
	{ 2, "SUB" },
	{ 2, "ADDU" },
	{ 2, "SUBU" },
	{ 2, "CMP" },
	{ 2, "AND" },
	{ 2, "OR" },
	{ 2, "XOR" },
	{ 1, "ABS" },
	{ 1, "MOV" },
	{ 1, "ASL2" },
	{ 1, "ASL8" },
	{ 1, "LS15" },
	{ 1, "DIFF" },
	{ 1, "ASR" },
	{ 0, "END" },
};

// The CMP operation is not affected by being skippable
#define OP_CMP (4)

const op_select_t es5510_device::OPERAND_SELECT[16] = {
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_BOTH },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_BOTH },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_BOTH, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_REG, es5510_device::SRC_DST_BOTH, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_BOTH },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_BOTH },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_BOTH, es5510_device::SRC_DST_REG, es5510_device::SRC_DST_REG },
	{ es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_BOTH, es5510_device::SRC_DST_DELAY, es5510_device::SRC_DST_REG },
};

static inline char * DESCRIBE_SRC_DST(char *s, UINT8 reg, const char *regname, op_src_dst_t src_dst) {
	switch (src_dst) {
	case es5510_device::SRC_DST_REG:
		return DESCRIBE_REG(s, reg, regname);
	case es5510_device::SRC_DST_DELAY:
		return stpcpy_int(s, "Delay");
	case es5510_device::SRC_DST_BOTH:
		s = DESCRIBE_REG(s, reg, regname);
		return stpcpy_int(s, ",Delay");
	}
	// should never happen!
	return s;
}

const es5510_device::ram_control_t es5510_device::RAM_CONTROL[8] = {
	{ es5510_device::RAM_CYCLE_READ,      es5510_device::RAM_CONTROL_DELAY,   "Read Delay+%06x" },
	{ es5510_device::RAM_CYCLE_WRITE,     es5510_device::RAM_CONTROL_DELAY,   "Write Delay+%06x" },
	{ es5510_device::RAM_CYCLE_READ,      es5510_device::RAM_CONTROL_TABLE_A, "Read Table A+%06x" },
	{ es5510_device::RAM_CYCLE_WRITE,     es5510_device::RAM_CONTROL_TABLE_A, "Write Table A+%06x" },
	{ es5510_device::RAM_CYCLE_READ,      es5510_device::RAM_CONTROL_TABLE_B, "Read Table B+%06x" },
	{ es5510_device::RAM_CYCLE_DUMP_FIFO, es5510_device::RAM_CONTROL_DELAY,   "Read Delay+%06x and Dump FIFO" },
	{ es5510_device::RAM_CYCLE_READ,      es5510_device::RAM_CONTROL_IO,      "Read from I/O at %06x" },
	{ es5510_device::RAM_CYCLE_WRITE,     es5510_device::RAM_CONTROL_IO,      "Write to I/O at %06x" },
};

static inline char * DESCRIBE_RAM(char *s, UINT8 ramControl, UINT32 gprContents) {
	return s + sprintf(s, es5510_device::RAM_CONTROL[ramControl].description, SC(gprContents));
}

static inline char * DESCRIBE_ALU(char *s, UINT8 opcode, UINT8 aReg, const char *aName, UINT8 bReg, const char *bName, const op_select_t &opSelect) {
	const alu_op_t &op = es5510_device::ALU_OPS[opcode];

	switch (op.operands) {
	case 0:
		return stpcpy_int(s, op.opcode);

	case 1:
		s += sprintf(s, "%s ", op.opcode);
		s = DESCRIBE_SRC_DST(s, bReg, bName, opSelect.alu_src);
		s += sprintf(s, " >");
		return DESCRIBE_SRC_DST(s, aReg, aName, opSelect.alu_dst);

	case 2:
		s += sprintf(s, "%s ", op.opcode);
		s = DESCRIBE_SRC_DST(s, aReg, aName, opSelect.alu_src);
		s += sprintf(s, " ");
		s = DESCRIBE_REG(s, bReg, bName);
		s += sprintf(s, " >");
		return DESCRIBE_SRC_DST(s, aReg, aName, opSelect.alu_dst);
	}
	return s;
}

static inline char * DESCRIBE_MAC(char *s, UINT8 mac, UINT8 cReg, const char *cName, UINT8 dReg, const char *dName, const op_select_t &opSelect)
{
	if (mac)
	{
		s += sprintf(s, "MAC + ");
	}
	s = DESCRIBE_SRC_DST(s, cReg, cName, opSelect.mac_src);
	s += sprintf(s, " * ");
	s = DESCRIBE_REG(s, dReg, dName);
	s += sprintf(s, " >");
	return DESCRIBE_SRC_DST(s, cReg, cName, opSelect.mac_dst);
}

static inline char * DESCRIBE_INSTR(char *s, UINT64 instr, UINT32 gpr, const char *aName, const char *bName, const char *cName, const char *dName)
{
	UINT8 dReg = (UINT8)((instr >> 40) & 0xff);
	UINT8 cReg = (UINT8)((instr >> 32) & 0xff);
	UINT8 bReg = (UINT8)((instr >> 24) & 0xff);
	UINT8 aReg = (UINT8)((instr >> 16) & 0xff);
	UINT8 aluOpcode = (UINT8)((instr >> 12) & 0x0f);
	UINT8 operandSelect = (UINT8)((instr >> 8) & 0x0f);
	UINT8 skip = (UINT8)((instr >> 7) & 0x01);
	UINT8 mac = (UINT8)((instr >> 6) & 0x01);
	UINT8 ramControl = (UINT8)((instr >> 3) & 0x07);

	const op_select_t &opSelect = es5510_device::OPERAND_SELECT[operandSelect];

	s = DESCRIBE_ALU(s, aluOpcode, aReg, aName, bReg, bName, opSelect);
	s += sprintf(s, "; ");
	s = DESCRIBE_MAC(s, mac, cReg, cName, dReg, dName, opSelect);
	s += sprintf(s, "; ");
	s = DESCRIBE_RAM(s, ramControl, gpr);
	if (skip) {
		s += sprintf(s, "; skippable");
	}

	return s;
}


READ8_MEMBER(es5510_device::host_r)
{
	//  printf("%06x: DSP read offset %04x (data is %04x)\n",space.device().safe_pc(),offset,dsp_ram[offset]);

	// VFX hack
	if (core_stricmp(space.machine().system().name, "vfx") == 0)
	{
		if (space.device().safe_pc() == 0xc091f0)
		{
			return space.device().state().state_int(M68K_D2);
		}
	}

	switch(offset)
	{
	case 0x00: LOG(("ES5510: Host Read GPR latch[2]: %02x\n", (gpr_latch >> 16) & 0xff)); return (gpr_latch >> 16) & 0xff;
	case 0x01: LOG(("ES5510: Host Read GPR latch[1]: %02x\n", (gpr_latch >>  8) & 0xff)); return (gpr_latch >>  8) & 0xff;
	case 0x02: LOG(("ES5510: Host Read GPR latch[0]: %02x\n", (gpr_latch >>  0) & 0xff)); return (gpr_latch >>  0) & 0xff;

	case 0x03: LOG(("ES5510: Host Read INSTR latch[5]: %02x\n", (UINT8)((instr_latch >> 40) & 0xff))); return (instr_latch >> 40) & 0xff;
	case 0x04: LOG(("ES5510: Host Read INSTR latch[4]: %02x\n", (UINT8)((instr_latch >> 32) & 0xff))); return (instr_latch >> 32) & 0xff;
	case 0x05: LOG(("ES5510: Host Read INSTR latch[3]: %02x\n", (UINT8)((instr_latch >> 24) & 0xff))); return (instr_latch >> 24) & 0xff;
	case 0x06: LOG(("ES5510: Host Read INSTR latch[2]: %02x\n", (UINT8)((instr_latch >> 16) & 0xff))); return (instr_latch >> 16) & 0xff;
	case 0x07: LOG(("ES5510: Host Read INSTR latch[1]: %02x\n", (UINT8)((instr_latch >>  8) & 0xff))); return (instr_latch >>  8) & 0xff;
	case 0x08: LOG(("ES5510: Host Read INSTR latch[0]: %02x\n", (UINT8)((instr_latch >>  0) & 0xff))); return (instr_latch >>  0) & 0xff;

	case 0x09: LOG(("ES5510: Host Read DIL latch[2]: %02x\n", (dil_latch >> 16) & 0xff)); return (dil_latch >> 16) & 0xff;
	case 0x0a: LOG(("ES5510: Host Read DIL latch[1]: %02x\n", (dil_latch >>  8) & 0xff)); return (dil_latch >>  8) & 0xff;
	case 0x0b: LOG(("ES5510: Host Read DIL latch[0]: %02x\n", 0)); return 0;

	case 0x0c: LOG(("ES5510: Host Read DOL latch[2]: %02x\n", (dol_latch >> 16) & 0xff)); return (dol_latch >> 16) & 0xff;
	case 0x0d: LOG(("ES5510: Host Read DOL latch[1]: %02x\n", (dol_latch >>  8) & 0xff)); return (dol_latch >>  8) & 0xff;
	case 0x0e: LOG(("ES5510: Host Read DOL latch[0]: %02x\n", 0xff)); return 0xff;

	case 0x0f: LOG(("ES5510: Host Read DADR latch[2]: %02x\n", (dadr_latch >> 16) & 0xff)); return (dadr_latch >> 16) & 0xff;
	case 0x10: LOG(("ES5510: Host Read DADR latch[1]: %02x\n", (dadr_latch >>  8) & 0xff)); return (dadr_latch >>  8) & 0xff;
	case 0x11: LOG(("ES5510: Host Read DADR latch[0]: %02x\n", (dadr_latch >>  0) & 0xff)); return (dadr_latch >>  0) & 0xff;

	case 0x12: LOG(("ES5510: Host Reading Host Control\n")); return 0; // Host Control

	case 0x16: return 0x27; // Program Counter, for test purposes only
	}

	// default: 0.
	return 0x00;
}

WRITE8_MEMBER(es5510_device::host_w)
{
#if VERBOSE
	static char buf[1024];
#endif
	switch (offset) {
	case 0x00:
		gpr_latch = (gpr_latch&0x00ffff) | ((data&0xff)<<16);
		LOG(("ES5510: Host Write GPR latch[2] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch)));
		break;
	case 0x01:
		gpr_latch = (gpr_latch&0xff00ff) | ((data&0xff)<< 8);
		LOG(("ES5510: Host Write GPR latch[1] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch)));
		break;
	case 0x02:
		gpr_latch = (gpr_latch&0xffff00) | ((data&0xff)<< 0);
		LOG(("ES5510: Host Write GPR latch[0] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch)));
		break;

		/* 0x03 to 0x08 INSTR Register */
	case 0x03: instr_latch = ((instr_latch&U64(0x00ffffffffff)) | ((INT64)data&0xff)<<40); LOG(("%s",string_format("ES5510: Host Write INSTR latch[5] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;
	case 0x04: instr_latch = ((instr_latch&U64(0xff00ffffffff)) | ((INT64)data&0xff)<<32); LOG(("%s",string_format("ES5510: Host Write INSTR latch[4] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;
	case 0x05: instr_latch = ((instr_latch&U64(0xffff00ffffff)) | ((INT64)data&0xff)<<24); LOG(("%s",string_format("ES5510: Host Write INSTR latch[3] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;
	case 0x06: instr_latch = ((instr_latch&U64(0xffffff00ffff)) | ((INT64)data&0xff)<<16); LOG(("%s",string_format("ES5510: Host Write INSTR latch[2] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;
	case 0x07: instr_latch = ((instr_latch&U64(0xffffffff00ff)) | ((INT64)data&0xff)<< 8); LOG(("%s",string_format("ES5510: Host Write INSTR latch[1] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;
	case 0x08: instr_latch = ((instr_latch&U64(0xffffffffff00)) | ((INT64)data&0xff)<< 0); LOG(("%s",string_format("ES5510: Host Write INSTR latch[0] = %02x -> %012I64x\n", data, instr_latch).c_str())); break;

		/* 0x09 to 0x0b DIL Register (r/o) */

	case 0x0c: dol_latch = (dol_latch&0x00ffff) | ((data&0xff)<<16); LOG(("ES5510: Host Write DOL latch[2] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch))); break;
	case 0x0d: dol_latch = (dol_latch&0xff00ff) | ((data&0xff)<< 8); LOG(("ES5510: Host Write DOL latch[1] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch))); break;
	case 0x0e: dol_latch = (dol_latch&0xffff00) | ((data&0xff)<< 0); LOG(("ES5510: Host Write DOL latch[0] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch))); break; //TODO: docs says that this always returns 0xff

	case 0x0f:
		dadr_latch = (dadr_latch&0x00ffff) | ((data&0xff)<<16);
		if (ram_sel)
		{
			dil_latch = dram[dadr_latch] << 8;
		}
		else
		{
			dram[dadr_latch] = dol_latch >> 8;
		}
		break;

	case 0x10: dadr_latch = (dadr_latch&0xff00ff) | ((data&0xff)<< 8); break;
	case 0x11: dadr_latch = (dadr_latch&0xffff00) | ((data&0xff)<< 0); break;

		/* 0x12 Host Control */

	case 0x14: ram_sel = data & 0x80; /* bit 6 is i/o select, everything else is undefined */break;

		/* 0x16 Program Counter (test purpose, r/o?) */
		/* 0x17 Internal Refresh counter (test purpose) */
		/* 0x18 Host Serial Control */
	case 0x18:
		LOG(("ES5510: Host Write Host Serial control %02x: %s, %s, ser3 %s, ser2 %s, ser1 %s, ser0 %s\n", data,
			data&0x80 ? "Master" : "Slave",
			data&0x40 ? "Sony" : "I2S",
			data & 0x20 ? "Out" : "In",
			data & 0x10 ? "Out" : "In",
			data & 0x08 ? "Out" : "In",
			data & 0x04 ? "Out" : "In"));
		break;

		/* 0x1f Halt enable (w) / Frame Counter (r) */
	case 0x1F:
		LOG(("ES5510: Host Write Halt Enable %02x; HALT line is %d\n", data, halt_asserted));
		if (halt_asserted) {
			LOG(("ES5510: Host Write to Halt Enable while HALT line is asserted: Halting!\n"));
			state = STATE_HALTED;
		}
		break;

	case 0x80: /* Read select - GPR + INSTR */
		LOG(("%s",string_format("ES5510: Host Read INSTR+GPR %02x (%s): %012I64x %06x (%d)\n", data, REGNAME(data & 0xff), instr[data] & U64(0xffffffffffff), gpr[data] & 0xffffff, gpr[data]).c_str()));

		/* Check if an INSTR address is selected */
		if (data < 0xa0) {
			instr_latch = instr[data];
		}
		if (data < 0xc0) {
			gpr_latch = gpr[data] & 0xffffff;
		} else if (data >= 0xea) {
			gpr_latch = read_reg(data);
		}
		break;

	case 0xa0: /* Write select - GPR */
		LOG(("ES5510: Host Write GPR %02x (%s): %06x (%d)\n", data, REGNAME(data&0xff), gpr_latch, SX(gpr_latch)));
		write_reg(data, gpr_latch);
		break;

	case 0xc0: /* Write select - INSTR */
#if VERBOSE
		DESCRIBE_INSTR(buf, instr_latch, gpr[data], NULL, NULL, NULL, NULL);
		LOG(("%s",string_format("ES5510: Host Write INSTR %02x %012I64x: %s\n", data, instr_latch&U64(0xffffffffffff), buf).c_str()));
#endif
		if (data < 0xa0) {
			instr[data] = instr_latch&U64(0xffffffffffff);
		}
		break;

	case 0xe0: /* Write select - GPR + INSTR */
#if VERBOSE
		DESCRIBE_INSTR(buf, instr_latch, gpr_latch, NULL, NULL, NULL, NULL);
		LOG(("%s",string_format("ES5510: Host Write INSTR+GPR %02x (%s): %012I64x %06x (%d): %s\n", data, REGNAME(data&0xff), instr_latch, gpr_latch, SX(gpr_latch), buf).c_str()));
#endif
		if (data < 0xa0) {
			instr[data] = instr_latch;
		}
		write_reg(data, gpr_latch);
		break;
	}
}

INT16 es5510_device::ser_r(int offset)
{
	switch(offset)
	{
	case 0: return ser0l;
	case 1: return ser0r;
	case 2: return ser1l;
	case 3: return ser1r;
	case 4: return ser2l;
	case 5: return ser2r;
	case 6: return ser3l;
	case 7: return ser3r;
	}
	return 0;
}

void es5510_device::ser_w(int offset, INT16 data)
{
	switch(offset)
	{
	case 0: ser0l = data; break;
	case 1: ser0r = data; break;
	case 2: ser1l = data; break;
	case 3: ser1r = data; break;
	case 4: ser2l = data; break;
	case 5: ser2r = data; break;
	case 6: ser3l = data; break;
	case 7: ser3r = data; break;
	}
}

void es5510_device::device_start() {
	m_icountptr = &icount;
	state_add(STATE_GENPC,"GENPC", pc).noshow();
}

void es5510_device::device_reset() {
	pc = 0x00;
	memset(gpr, 0, sizeof(*gpr) * 0xc0);
	memset(instr, 0, sizeof(*instr) * 0xa0);
	memset(dram, 0, sizeof(*dram) * (1<<20));
	state = STATE_RUNNING;
	dil_latch = dol_latch = dadr_latch = gpr_latch = 0;
	instr_latch = UINT64(0);
	ram_sel = 0;
	host_control = 0;
	memset(&ram, 0, sizeof(ram_t));
	memset(&ram_p, 0, sizeof(ram_t));
	memset(&ram_pp, 0, sizeof(ram_t));
}

const address_space_config *es5510_device::memory_space_config(address_spacenum spacenum) const {
	return nullptr;
}

UINT64 es5510_device::execute_clocks_to_cycles(UINT64 clocks) const {
	return clocks / 3;
}

UINT64 es5510_device::execute_cycles_to_clocks(UINT64 cycles) const {
	return cycles * 3;
}

UINT32 es5510_device::execute_min_cycles() const {
	return 1;
}

UINT32 es5510_device::execute_max_cycles() const {
	return 1;
}

UINT32 es5510_device::execute_input_lines() const {
	return 1;
}

void es5510_device::execute_set_input(int linenum, int state) {
	if (linenum == ES5510_HALT) {
		halt_asserted = (state == ASSERT_LINE);
	}
}

void es5510_device::list_program(void(p)(const char *, ...)) {
	LOG(("ES5501: Starting!\n"));

	char buf[1024];
	bool is_written[0x100], is_read[0x100];
	char name[0x100][16];
	int addr;

	for (int i = 0; i < 0x100; i++) {
		is_written[i] = is_read[i] = false;
		name[i][0] = '\0';
	}

	for (addr = 0; addr < 0xa0; addr++) {
		DESCRIBE_INSTR(buf, instr[addr], gpr[addr], nullptr, nullptr, nullptr, nullptr);
		UINT64 inst = instr[addr];
		UINT8 aReg = (UINT8)((inst >> 16) & 0xff);
		UINT8 bReg = (UINT8)((inst >> 24) & 0xff);
		UINT8 cReg = (UINT8)((inst >> 32) & 0xff);
		UINT8 dReg = (UINT8)((inst >> 40) & 0xff);
		UINT8 alu_op = (inst >> 12) & 0x0f;
		if (alu_op == 0x0f) {
			// END!
			break;
		}

		UINT8 operandSelect = (UINT8)((inst >> 8) & 0x0f);
		const op_select_t &opSelect = OPERAND_SELECT[operandSelect];

		if (opSelect.mac_src == SRC_DST_REG) {
			is_read[cReg] = true;
		}
		is_read[dReg] = true;
		if (opSelect.mac_dst != SRC_DST_DELAY) { // either REG or BOTH
			is_written[cReg] = true;
		}

		alu_op_t aluOp = ALU_OPS[alu_op];
		if (aluOp.operands == 1) {
			if (opSelect.alu_src == SRC_DST_REG) {
				is_read[bReg] = true;
			}
		} else if (aluOp.operands == 2) {
			if (opSelect.alu_src == SRC_DST_REG) {
				is_read[aReg] = true;
			}
			is_read[bReg] = true;
		}
		if (opSelect.mac_dst != SRC_DST_DELAY) { // either REG or BOTH
			is_written[aReg] = true;
		}
	}

	int varIndex = 1;
	int constIndex = 1;
	for (int i = 0; i < 0xc0; i++) {
		if (is_written[i]) {
			// this is a variable
			sprintf(name[i], "v_%03d", varIndex++);
		} else if (is_read[i]) {
			// this is only read, so a constant - or possibly something updated by the CPU
			sprintf(name[i], "c_%03d", constIndex++);
		} else {
			name[i][0] = 0;
		}
	}
	for (int i = 0xc0; i < 0x100; i++) {
		name[i][0] = 0;
	}

	for (addr = 0; addr < 0xa0; addr++) {
		UINT8 aReg = (UINT8)((instr[addr] >> 16) & 0xff);
		UINT8 bReg = (UINT8)((instr[addr] >> 24) & 0xff);
		UINT8 cReg = (UINT8)((instr[addr] >> 32) & 0xff);
		UINT8 dReg = (UINT8)((instr[addr] >> 40) & 0xff);
		DESCRIBE_INSTR(buf, instr[addr], gpr[addr], name[aReg], name[bReg], name[cReg], name[dReg]);
		p("%s",string_format("%02x: %012I64x %06x (%8d) %s\n", addr, instr[addr], gpr[addr]&0xffffff, SX(gpr[addr]&0xffffff), buf).c_str());
	}
	for (; addr < 0xc0; addr++) {
		p("%02x: %06x (%d)\n", addr, gpr[addr]&0xffffff, SX(gpr[addr]&0xffffff));
	}
}

void es5510_device::execute_run() {
	while (icount > 0) {
		if (state == STATE_HALTED) {
			// Currently halted, sample the HALT line
			if (halt_asserted) {
				// remain halted
				host_control |= 0x04; // Signal Host Access OK
			} else {
				// start from the beginning at PC 0
				state = STATE_RUNNING;
				host_control &= ~0x04; // Signal Host Access not OK
				pc = 0;
			}
		} else {
			// currently running, execute one instruction.

#if VERBOSE_EXEC
			char buf[1024];
			DESCRIBE_INSTR(buf, instr[pc], gpr[pc], NULL, NULL, NULL, NULL);
			LOG_EXEC(("%s",string_format("EXECUTING %02x: %012I64x %06x  %s\n", pc, instr[pc], gpr[pc]&0xffffff, buf).c_str()));
#endif

			ram_pp = ram_p;
			ram_p = ram;

			LOG_EXEC(("- T0\n"));

			// *** T0, clock high
			// --- nothing to do!

			// *** T0, clock low
			// --- Read instruction N
			UINT64 instr = this->instr[pc];

			// --- RAM cycle N-2 (if a Read cycle): data read from bus is stored in DIL
			if (ram_pp.cycle != RAM_CYCLE_WRITE) {
				if (ram_pp.io) { // read from I/O and store into DIL
					dil = 0; // read_io(ram_pp.address);;
				} else { // read from DRAM and store into DIL
					dil = dram[ram_pp.address] << 8;
					LOG_EXEC(("  . RAM: read %x (%d) from address %x\n", dil, dil, ram_pp.address));
				}
			}

			// --- start of RAM cycle N
			ram_control_t ramControl = RAM_CONTROL[((instr >> 3) & 0x07)];
			ram.cycle = ramControl.cycle;
			ram.io = ramControl.access == RAM_CONTROL_IO;

			// --- RAM cycle N: read offset N
			INT32 offset = gpr[pc];
			switch(ramControl.access) {
			case RAM_CONTROL_DELAY:
				ram.address = (((dbase + offset) % (dlength + memincrement)) & memmask) >> memshift;
				LOG_EXEC((". Ram Control: Delay, base=%x, offset=%x, length=%x => address=%x\n", dbase >> memshift, offset >> memshift, (dlength + memincrement) >> memshift, ram.address));
				break;
			case RAM_CONTROL_TABLE_A:
				ram.address = ((abase + offset) & memmask) >> memshift;
				LOG_EXEC((". Ram Control: table A = %x, offset=%x => address=%x\n", abase >> memshift, offset >> memshift, ram.address));
				break;
			case RAM_CONTROL_TABLE_B:
				ram.address = ((bbase + offset) & memmask) >> memshift;
				LOG_EXEC((". Ram Control: table B = %x, offset=%x => address=%x\n", bbase >> memshift, offset >> memshift, ram.address));
				break;
			case RAM_CONTROL_IO:
				ram.address = offset & 0x00fffff0; // mask off the low 4 bits
				LOG_EXEC((". Ram Control: I/O at address=%x\n", ram.address));
				break;
			}

			// *** T1, clock high
			// --- Decode instruction N;
			//     we will do this both here and in stages as the different parts of the instruction complete & recommence.

			LOG_EXEC(("- T1.1\n"));

			UINT8 operandSelect = (UINT8)((instr >> 8) & 0x0f);
			const op_select_t &opSelect = OPERAND_SELECT[operandSelect];
			bool skip;
			bool skippable = (instr & (0x01 << 7)) != 0; // aka the 'SKIP' bit in the instruction word
			if (skippable) {
				bool skipConditionSatisfied = (ccr & cmr & FLAG_MASK) != 0;
				if (isFlagSet(cmr, FLAG_NOT)) {
					skipConditionSatisfied = !skipConditionSatisfied;
				}
				skip = skipConditionSatisfied;
				LOG_EXEC((". skippable: %x vs %x => skippable = %d\n", ccr, cmr, skip));
			} else {
				skip = false;
			}

			// --- Write Multiplier result N-1
			LOG_EXEC((". write mulacc:\n"));
			if (mulacc.write_result) {
				mulacc.product = ((INT64)SX(mulacc.cValue) * (INT64)SX(mulacc.dValue)) << mulshift;
				if (mulacc.accumulate) {
					mulacc.result = mulacc.product + machl;
				} else {
					mulacc.result = mulacc.product;
				}

				if (mulacc.result < MIN_48 || mulacc.result > MAX_48) {
					mac_overflow = true;
				} else {
					mac_overflow = false;
				}
#if VERBOSE_EXEC
				if (mulacc.cValue || mulacc.dValue || (mulacc.accumulate && machl)) {
					LOG_EXEC((". mulacc: %x (%d) * %x (%d) << %d", SX(mulacc.cValue), SX(mulacc.cValue), SX(mulacc.dValue), SX(mulacc.dValue), mulshift));
					if (mulacc.accumulate) LOG_EXEC((" + %llx (%lld) ", machl, machl));
					LOG_EXEC((" = %llx (%lld)", mulacc.result, mulacc.result));
					if (mac_overflow) {
						LOG_EXEC((" overflow!\n"));
					} else {
						LOG_EXEC(("\n"));
					}
				}
#endif
				machl = mulacc.result;
				INT32 tmp = mac_overflow ? (machl < 0 ? MIN_24 : MAX_24) : (mulacc.result & U64(0x0000ffffff000000)) >> 24;
				if (mulacc.dst & SRC_DST_REG) {
					write_reg(mulacc.cReg, tmp);
				}
				if (mulacc.dst & SRC_DST_DELAY) {
					write_to_dol(tmp);
				}
			}

			// *** T1, clock low

			LOG_EXEC(("- T1.0\n"));

			// --- Start of multiplier cycle N
			LOG_EXEC((". start mulacc:\n"));
			mulacc.cReg = (UINT8)((instr >> 32) & 0xff);
			mulacc.dReg = (UINT8)((instr >> 40) & 0xff);
			mulacc.src = opSelect.mac_src;
			mulacc.dst = opSelect.mac_dst;
			mulacc.accumulate = ((instr >> 6) & 0x01) != 0;
			mulacc.write_result = !skip;

			// --- Read Multiplier Operands N
			if (mulacc.src == SRC_DST_REG) {
				mulacc.cValue = read_reg(mulacc.cReg);
			} else { // must be SRC_DST_DELAY
				LOG_EXEC(("  . reading %x (%d) from dil\n", dil, SX(dil)));
				mulacc.cValue = dil;
			}
			mulacc.dValue = read_reg(mulacc.dReg);

			// *** T2, clock high

			LOG_EXEC(("- T2.1\n"));

			// --- Write ALU Result N-1
			LOG_EXEC((". write ALU:\n"));
			if (alu.write_result) {
				UINT8 flags = ccr;
				alu.result = alu_operation(alu.op, alu.aValue, alu.bValue, flags);
				if (alu.dst & SRC_DST_REG) {
					write_reg(alu.aReg, alu.result);
				}
				if (alu.dst & SRC_DST_DELAY) {
					write_to_dol(alu.result);
				}
				if (alu.update_ccr) {
					ccr = flags;
				}
			}

			// *** T2, clock low

			LOG_EXEC(("- T2.0\n"));

			// --- Start of ALU cycle N
			LOG_EXEC((". start ALU:\n"));
			alu.aReg = (instr >> 16) & 0xff;
			alu.bReg = (instr >> 24) & 0xff;
			alu.op = (instr >> 12) & 0x0f;
			alu.src = opSelect.alu_src;
			alu.dst = opSelect.alu_dst;
			alu.write_result = !skip;
			alu.update_ccr = !skippable || (alu.op == OP_CMP);

			if (alu.op == 0xF) {
				alu_operation_end();
			} else {
				// --- Read ALU Operands N
				alu_op_t aluOp = ALU_OPS[alu.op];
				if (aluOp.operands == 1) {
					if (alu.src == SRC_DST_REG) {
						alu.bValue = read_reg(alu.bReg);
					} else { // must be SRC_DST_DELAY
						alu.bValue = dil;
					}
				} else {
					if (alu.src == SRC_DST_REG) {
						alu.aValue = read_reg(alu.aReg);
					} else { // must be SRC_DST_DELAY
						alu.aValue = dil;
					}
					alu.bValue = read_reg(alu.bReg);
				}
			}

			// --- RAM cycle N-1
			if (ram_p.cycle != RAM_CYCLE_READ) {
				if (ram_p.cycle == RAM_CYCLE_WRITE) {
					// If this is a write cycle, write the frontmost DOL value to RAM or I/O
					if (ram_p.io) {
						// write_io(ram_p.io, dol[0]);
					} else {
						dram[ram_p.address] = dol[0] >> 8;
						LOG_EXEC(("  . RAM: writing %x (%d) [of %x (%d)] to address %x\n", dol[0]&0xffff00, SX(dol[0]&0xffff00), dol[0], SX(dol[0]), ram_p.address));
					}
				}
				// If this is a Write or Dump cycle, eject the frontmost DL value.
#if VERBOSE_EXEC
				LOG_EXEC(("  . ejecting from DOL: [ "));
				if (dol_count >= 1) LOG_EXEC(("{ %x (%d) }", dol[0], SX(dol[0])));
				if (dol_count == 2) LOG_EXEC((", { %x (%d) }", dol[1], SX(dol[1])));
				LOG_EXEC((" ] -> [ "));
#endif
				dol[0] = dol[1];
				if (dol_count > 0) {
					--dol_count;
				}
#if VERBOSE_EXEC
				if (dol_count >= 1) LOG_EXEC(("{ %x (%d) }", dol[0], SX(dol[0])));
				if (dol_count == 2) LOG_EXEC((", { %x (%d) }", dol[1], SX(dol[1])));
				LOG_EXEC((" ]\n"));
#endif
			}

			++pc;
		}
		--icount;
	}
}

UINT32 es5510_device::disasm_min_opcode_bytes() const
{
	return 6;
}

UINT32 es5510_device::disasm_max_opcode_bytes() const
{
	return 6;
}

offs_t es5510_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return pc;
}

#if VERBOSE_EXEC
#define RETURN_GPR(r, x) do { INT32 v = (x); LOG_EXEC(("  . reading %x (%d) from gpr_%02x\n", v, SX(v), r)); return v; } while(0)
#define RETURN(r, x) do { INT32 v = (x); LOG_EXEC(("  . reading %x (%d) from " #r "\n", v, SX(v))); return v; } while(0)
#define RETURN16(r, x) do { INT16 vv = (x); INT32 v = vv << 8; LOG_EXEC(("  . reading %x (%d) as %x (%d) from " #r "\n", vv, vv, v, SX(v))); return v; } while(0)
#else
#define RETURN_GPR(r, x) return x
#define RETURN(r, x) return x
#define RETURN16(r, x) return (x) << 8
#endif

INT32 es5510_device::read_reg(UINT8 reg)
{
	if (reg < 0xc0) {
		RETURN_GPR(reg, gpr[reg]);
	} else {
		switch(reg)
		{
		case 234: RETURN16(ser0r, ser0r);
		case 235: RETURN16(ser0l, ser0l);
		case 236: RETURN16(ser1r, ser1r);
		case 237: RETURN16(ser1l, ser1l);
		case 238: RETURN16(ser2r, ser2r);
		case 239: RETURN16(ser2l, ser2l);
		case 240: RETURN16(ser3r, ser3r);
		case 241: RETURN16(ser3l, ser3l);
		case 242: /* macl */ RETURN(macl, mac_overflow ? (machl < 0 ? 0x00ffffff : 0x00000000) : (machl >>  0) & 0x00ffffff);
		case 243: /* mach */ RETURN(mach, mac_overflow ? (machl < 0 ? MIN_24 : MAX_24) : (machl >> 24) & 0x00ffffff);
		case 244: RETURN(dil, dil); // DIL when reading
		case 245: RETURN(dlength, dlength);
		case 246: RETURN(abase, abase);
		case 247: RETURN(bbase, bbase);
		case 248: RETURN(dbase, dbase);
		case 249: RETURN(sigreg, sigreg);
		case 250: RETURN(ccr, ccr);
		case 251: RETURN(cmr, cmr);
		case 252: RETURN(minus_one, 0x00ffffff);
		case 253: RETURN(min, MIN_24);
		case 254: RETURN(max, MAX_24);
		case 255: RETURN(zero, 0);
		default:
			// unknown SPR
			RETURN(unknown, 0);
		}
	}
}

void es5510_device::run_once()
{
	// turn HALT off
	set_HALT(false);

	// run for one instruction
	icount = 1;
	execute_run();

	// turn HALT on again
	set_HALT(true);

	// run ESP to the end of its program, a few instructions at a time
	while (state != STATE_HALTED) {
		icount = 1;
		execute_run();
	}
}

INT8 countLowOnes(INT32 x) {
	INT8 n = 0;
	while ((x & 1) == 1) {
		++n;
		x >>= 1;
	}
	return n;
}

#if VERBOSE_EXEC
#define WRITE_REG(r, x) do { r = value; LOG_EXEC(("  . writing %x (%d) to " #r "\n", r, SX(r))); } while(0)
#define WRITE_REG16(r, x) do { r = ((value >> 8) & 0xffff); LOG_EXEC(("  . writing %x (%d) as %x (%d) to " #r "\n", value, SX(value), r, r)); } while(0)
#else
#define WRITE_REG(r, x) do { r = value; } while(0)
#define WRITE_REG16(r, x) do { r = ((value >> 8) & 0xffff); } while(0)
#endif

void es5510_device::write_reg(UINT8 reg, INT32 value)
{
	#if VERBOSE_EXEC
	INT64 old;
	#endif
	value &= 0x00ffffff;
	if (reg < 0xc0) {
		LOG_EXEC(("  . writing %x (%d) to gpr_%02x\n", value, SX(value), reg));
		gpr[reg] = value;
	} else {
		switch(reg)
		{
		case 234: WRITE_REG16(ser0r, value);
			break;
		case 235: WRITE_REG16(ser0l, value);
			break;
		case 236: WRITE_REG16(ser1r, value);
			break;
		case 237: WRITE_REG16(ser1l, value);
			break;
		case 238: WRITE_REG16(ser2r, value);
			break;
		case 239: WRITE_REG16(ser2l, value);
			break;
		case 240: WRITE_REG16(ser3r, value);
			break;
		case 241: WRITE_REG16(ser3l, value);
			break;
		case 242: /* macl */ {
			#if VERBOSE_EXEC
			old = machl;
			#endif
			INT64 masked = machl & (S64(0x00ffffff) << 24);
			INT64 shifted = (INT64)(value & 0x00ffffff) << 0;
			machl = SX64(masked | shifted);
			#if VERBOSE_EXEC
			LOG_EXEC(("  . writing machl: l -> %06x => %llx -> %llx\n", value, old, machl));
			#endif
			break;
		}
		case 243: /* mach */ {
			#if VERBOSE_EXEC
			old = machl;
			#endif
			INT64 masked = machl & (S64(0x00ffffff) << 0);
			INT64 shifted = (INT64)(value & 0x00ffffff) << 24;
			machl = SX64(masked | shifted);
			mac_overflow = false;
			#if VERBOSE_EXEC
			LOG_EXEC(("  . writing machl: h -> %06x => %llx -> %llx\n", value, old, machl));
			#endif
			break;
		}
		case 244: /* MEMSIZ when writing */
			memshift = countLowOnes(value);
			memsiz = 0x00ffffff >> (24 - memshift);
			memmask = 0x00ffffff & ~memsiz;
			memincrement = 1 << memshift;
			LOG_EXEC(("  . writing %x (%d) to memsiz => memsiz=%x, shift=%d, mask=%x, increment=%x\n", value, SX(value), memsiz, memshift, memmask, memincrement));
			break;
		case 245: WRITE_REG(dlength, value);
			break;
		case 246: WRITE_REG(abase, value);
			break;
		case 247: WRITE_REG(bbase, value);
			break;
		case 248: WRITE_REG(dbase, value);
			break;
		case 249: WRITE_REG(sigreg, (value != 0));
			break;
		case 250: WRITE_REG(ccr, (value >> 16) & FLAG_MASK);
			break;
		case 251: WRITE_REG(cmr, (value >> 16) & (FLAG_MASK | FLAG_NOT));
			break;
		case 252: LOG_EXEC((". not writing %x (%d) to minus_one\n", value, SX(value))); // no-op
			break;
		case 253: LOG_EXEC((". not writing %x (%d) to min\n", value, SX(value))); // no-op
			break;
		case 254: LOG_EXEC((". not writing %x (%d) to max\n", value, SX(value))); // no-op
			break;
		case 255: LOG_EXEC((". not writing %x (%d) to zero\n", value, SX(value))); // no-op
			break;
		default: // unknown register
			break;
		}
	}
}

void es5510_device::write_to_dol(INT32 value) {
#if VERBOSE_EXEC
	LOG_EXEC((". writing %x (%d) to DOL: [ ", value, value));
	if (dol_count >= 1) LOG_EXEC(("{ %x (%d) }", dol[0], SX(dol[0])));
	if (dol_count == 2) LOG_EXEC((", { %x (%d) }", dol[1], SX(dol[1])));
	LOG_EXEC((" ] -> [ "));
#endif
	if (dol_count >= 2) {
		dol[0] = dol[1];
		dol[1] = value;
	} else {
		dol[dol_count++] = value;
	}
#if VERBOSE_EXEC
	LOG_EXEC(("{%x (%d)}", dol[0], SX(dol[0])));
	if (dol_count == 2) LOG_EXEC((", {%x (%d)}", dol[1], SX(dol[1])));
	LOG_EXEC((" ]\n"));
#endif
}

void es5510_device::alu_operation_end() {
	// Handle the END instruction separately
	LOG_EXEC(("ES5510: END\n"));
	// sample the HALT line
	if (halt_asserted) {
		// halt
		state = STATE_HALTED;
		host_control |= 0x04; // Signal Host Access OK
	}
	// update the delay line base pointer
	dbase -= memincrement;
	if (dbase < 0) {
		dbase = dlength;
	}
	// Possibly reset the PC
	if (state == STATE_RUNNING) {
		pc = 0;
	}

#if VERBOSE_EXEC
	// update the verbose-execution counter.
	exec_cc = (exec_cc + 1) % 30000;
#endif
}

INT32 es5510_device::alu_operation(UINT8 op, INT32 a, INT32 b, UINT8 &flags) {
	INT32 tmp;
	switch(op) {
	case 0x0: // ADD
		tmp = add(a, b, flags);
		return saturate(tmp, flags, (a & 0x00800000) != 0);

	case 0x1: // SUB
		tmp = add(a, negate(b), flags);
		return saturate(tmp, flags, (a & 0x00800000) != 0);

	case 0x2: // ADDU
		return add(a, b, flags);

	case 0x3: // SUBU
		return add(a, negate(b), flags);

	case 0x4: // CMP
		// perform the subtraction, only for its effect on the flags
		add(a, negate(b), flags);
		return a;

	case 0x5: // AND
		a &= b;
		setFlagTo(flags, FLAG_N, (a & 0x0080000000) != 0);
		setFlagTo(flags, FLAG_Z, a == 0);
		return a;

	case 0x6: // OR
		a |= b;
		setFlagTo(flags, FLAG_N, (a & 0x0080000000) != 0);
		setFlagTo(flags, FLAG_Z, a == 0);
		return a;

	case 0x7: // XOR
		a ^= b;
		setFlagTo(flags, FLAG_N, (a & 0x0080000000) != 0);
		setFlagTo(flags, FLAG_Z, a == 0);
		return a;

	case 0x8: // ABS
	{
		clearFlag(flags, FLAG_N);
		bool isNegative = (a & 0x00800000) != 0;
		setFlagTo(flags, FLAG_C, isNegative);
		// Note: the absolute value is calculated by one's complement!
		return isNegative ? (0x00ffffff ^ a) : a;
	}

	case 0x9: // MOV
		return b;

	case 0xA: // ASL2
		return asl(b, 2, flags);

	case 0xB: // ASL8
		return asl(b, 8, flags);

	case 0xC: // LS15
		return (b << 15) & 0x007fffff;

	case 0xD: // DIFF
		return add(0x007fffff, negate(b), flags);

	case 0xE: // ASR
		return (b >> 1) | (b & 0x00800000);

	case 0xF: // END - handled separately in alu_operation_end()
	default:
		return 0;
	}
}
