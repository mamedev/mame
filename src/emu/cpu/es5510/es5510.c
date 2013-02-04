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
  INT32 aSign = a & 0x00800000;
  INT32 bSign = (b & 0x00800000) == 0;
  INT32 result = a + b;
  INT32 resultSign = result & 0x00800000;
  bool overflow = (aSign == bSign) && (aSign != resultSign);
  bool carry = result & 0x01000000;
  bool negative = resultSign != 0;
  bool lessThan = (overflow && !negative) || (!overflow && negative);
  flags = setFlagTo(flags, FLAG_C, carry);
  flags = setFlagTo(flags, FLAG_N, negative);
  flags = setFlagTo(flags, FLAG_Z, result == 0);
  flags = setFlagTo(flags, FLAG_V, overflow);
  flags = setFlagTo(flags, FLAG_LT, lessThan);
  return result;
}

inline static INT32 saturate(INT32 value, UINT8 &flags) {
  if (isFlagSet(flags, FLAG_V)) {
    return isFlagSet(flags, FLAG_N) ? 0x00800000 : 0x007fffff;
  } else {
    return value;
  }
}

inline static INT32 negate(INT32 value) {
  return (value ^ 0x00ffffff) + 1;
}

inline static INT32 asl(INT32 value, int shift, UINT8 &flags) {
  INT32 signBefore = value & 0x00800000;
  INT32 result = (value << shift) & 0x00ffffff;
  INT32 signAfter = result & 0x00800000;
  bool overflow = signBefore != signAfter;
  flags = setFlagTo(flags, FLAG_V, overflow);
  return saturate(result, flags);
}


es5510_device::es5510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
  : cpu_device(mconfig, ES5510, "ES5510", tag, owner, clock)
{
  // Initialize ESP to mostly zeroed, configured for 64k samples of delay line memory, running (not halted)
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

  memset(&alu, 0, sizeof(alu));
  memset(&mulacc, 0, sizeof(mulacc));
}

typedef es5510_device::alu_op_t alu_op_t;
typedef es5510_device::op_select_t op_select_t;
typedef es5510_device::op_src_dst_t op_src_dst_t;

static inline INT32 SX(INT32 x) { return (x & 0x00800000) ? x | 0xff000000 : x & 0x00ffffff; }
static inline INT32 SC(INT32 x) { return x & 0x00ffffff; }
static inline INT64 SX64(INT64 x) { return (x & U64(0x0000800000000000)) ? x | U64(0xffff000000000000) : x & U64(0x0000ffffffffffff); }
static inline INT64 SC64(INT64 x) { return x & U64(0x0000ffffffffffff); }

static inline const char * const REGNAME(UINT8 r) {
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
  return NULL;
}

static inline char * DESCRIBE_REG(char *s, UINT8 r) {
  return stpcpy_int(s, REGNAME(r));
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
  { 2, "DIFF" },
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

static inline char * DESCRIBE_SRC_DST(char *s, UINT8 reg, op_src_dst_t src_dst) {
  switch (src_dst) {
  case es5510_device::SRC_DST_REG:
    return DESCRIBE_REG(s, reg);
  case es5510_device::SRC_DST_DELAY:
    return stpcpy_int(s, "Delay");
  case es5510_device::SRC_DST_BOTH:
    s = DESCRIBE_REG(s, reg);
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
  { es5510_device::RAM_CYCLE_READ,      es5510_device::RAM_CONTROL_IO,      "Read I/O at %06x" },
  { es5510_device::RAM_CYCLE_WRITE,     es5510_device::RAM_CONTROL_IO,      "Write I/o %06x" },
};

static inline char * DESCRIBE_RAM(char *s, UINT8 ramControl, UINT32 gprContents) {
  return s + sprintf(s, es5510_device::RAM_CONTROL[ramControl].description, SC(gprContents));
}

static inline char * DESCRIBE_ALU(char *s, UINT8 opcode, UINT8 aReg, UINT8 bReg, const op_select_t &opSelect) {
  const alu_op_t &op = es5510_device::ALU_OPS[opcode];

  switch (op.operands) {
  case 0:
    return stpcpy_int(s, op.opcode);

  case 1:
    s += sprintf(s, "%s %s >", op.opcode, REGNAME(bReg));
    return DESCRIBE_SRC_DST(s, aReg, opSelect.alu_dst);
    
  case 2:
    s += sprintf(s, "%s %s,", op.opcode, REGNAME(bReg));
    s = DESCRIBE_SRC_DST(s, aReg, opSelect.alu_src);
    s += sprintf(s, " >");
    return DESCRIBE_SRC_DST(s, aReg, opSelect.alu_dst);
  }
  return s;
}

static inline char * DESCRIBE_MAC(char *s, UINT8 mac, UINT8 cReg, UINT8 dReg, const op_select_t &opSelect)
{
  if (mac)
  {
    s += sprintf(s, "MAC + ");
  }
  s = DESCRIBE_REG(s, dReg);
  s += sprintf(s, " * ");
  s = DESCRIBE_SRC_DST(s, cReg, opSelect.mac_src);
  s += sprintf(s, " >");
  return DESCRIBE_SRC_DST(s, cReg, opSelect.mac_dst);
}

static inline char * DESCRIBE_INSTR(char *s, UINT64 instr, UINT32 gpr)
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

  s = DESCRIBE_ALU(s, aluOpcode, aReg, bReg, opSelect);
  s += sprintf(s, "; ");
  s = DESCRIBE_MAC(s, mac, cReg, dReg, opSelect);
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
  if (mame_stricmp(space.machine().system().name, "vfx") == 0)
  {
    if (space.device().safe_pc() == 0xc091f0)
    {
      return space.device().state().state_int(M68K_D2);
    }
  }
  
  switch(offset)
  {
  case 0x00: logerror("ES5510: Read GPR latch[2]: %02x\n", (gpr_latch >> 16) & 0xff); return (gpr_latch >> 16) & 0xff;
  case 0x01: logerror("ES5510: Read GPR latch[1]: %02x\n", (gpr_latch >>  8) & 0xff); return (gpr_latch >>  8) & 0xff;
  case 0x02: logerror("ES5510: Read GPR latch[0]: %02x\n", (gpr_latch >>  0) & 0xff); return (gpr_latch >>  0) & 0xff;
    
  case 0x03: logerror("ES5510: Read INSTR latch[5]: %02x\n", (UINT8)((instr_latch >> 40) & 0xff)); return (instr_latch >> 40) & 0xff;
  case 0x04: logerror("ES5510: Read INSTR latch[4]: %02x\n", (UINT8)((instr_latch >> 32) & 0xff)); return (instr_latch >> 32) & 0xff;
  case 0x05: logerror("ES5510: Read INSTR latch[3]: %02x\n", (UINT8)((instr_latch >> 24) & 0xff)); return (instr_latch >> 24) & 0xff;
  case 0x06: logerror("ES5510: Read INSTR latch[2]: %02x\n", (UINT8)((instr_latch >> 16) & 0xff)); return (instr_latch >> 16) & 0xff;
  case 0x07: logerror("ES5510: Read INSTR latch[1]: %02x\n", (UINT8)((instr_latch >>  8) & 0xff)); return (instr_latch >>  8) & 0xff;
  case 0x08: logerror("ES5510: Read INSTR latch[0]: %02x\n", (UINT8)((instr_latch >>  0) & 0xff)); return (instr_latch >>  0) & 0xff;
    
  case 0x09: logerror("ES5510: Read DIL latch[2]: %02x\n", (dil_latch >> 16) & 0xff); return (dil_latch >> 16) & 0xff;
  case 0x0a: logerror("ES5510: Read DIL latch[1]: %02x\n", (dil_latch >>  8) & 0xff); return (dil_latch >>  8) & 0xff;
  case 0x0b: logerror("ES5510: Read DIL latch[0]: %02x\n", (dil_latch >>  0) & 0xff); return (dil_latch >>  0) & 0xff; //TODO: docs says that this always returns 0
    
  case 0x0c: logerror("ES5510: Read DOL latch[2]: %02x\n", (dol_latch >> 16) & 0xff); return (dol_latch >> 16) & 0xff;
  case 0x0d: logerror("ES5510: Read DOL latch[1]: %02x\n", (dol_latch >>  8) & 0xff); return (dol_latch >>  8) & 0xff;
  case 0x0e: logerror("ES5510: Read DOL latch[0]: %02x\n", (dol_latch >>  0) & 0xff); return (dol_latch >>  0) & 0xff; //TODO: docs says that this always returns 0
    
  case 0x0f: logerror("ES5510: Read DADR latch[2]: %02x\n", (dadr_latch >> 16) & 0xff); return (dadr_latch >> 16) & 0xff;
  case 0x10: logerror("ES5510: Read DADR latch[1]: %02x\n", (dadr_latch >>  8) & 0xff); return (dadr_latch >>  8) & 0xff;
  case 0x11: logerror("ES5510: Read DADR latch[0]: %02x\n", (dadr_latch >>  0) & 0xff); return (dadr_latch >>  0) & 0xff;
    
  case 0x12: logerror("ES5510: Reading Host Control\n"); return 0; // Host Control
    
  case 0x16: return 0x27; // Program Counter, for test purposes only
  }
  
  // default: 0.
  return 0x00;
}

WRITE8_MEMBER(es5510_device::host_w)
{
  static char buf[1024];
  switch (offset) {
  case 0x00: 
    gpr_latch = (gpr_latch&0x00ffff) | ((data&0xff)<<16); 
    logerror("ES5510: Write GPR latch[2] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch)); 
    break;
  case 0x01:
    gpr_latch = (gpr_latch&0xff00ff) | ((data&0xff)<< 8); 
    logerror("ES5510: Write GPR latch[1] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch)); 
    break;
  case 0x02:
    gpr_latch = (gpr_latch&0xffff00) | ((data&0xff)<< 0); 
    logerror("ES5510: Write GPR latch[0] = %02x -> %06x (%d)\n", data, gpr_latch, SX(gpr_latch));
    break;

    /* 0x03 to 0x08 INSTR Register */
  case 0x03: instr_latch = ((instr_latch&U64(0x00ffffffffff)) | ((INT64)data&0xff)<<40); logerror("ES5510: Write INSTR latch[5] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;
  case 0x04: instr_latch = ((instr_latch&U64(0xff00ffffffff)) | ((INT64)data&0xff)<<32); logerror("ES5510: Write INSTR latch[4] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;
  case 0x05: instr_latch = ((instr_latch&U64(0xffff00ffffff)) | ((INT64)data&0xff)<<24); logerror("ES5510: Write INSTR latch[3] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;
  case 0x06: instr_latch = ((instr_latch&U64(0xffffff00ffff)) | ((INT64)data&0xff)<<16); logerror("ES5510: Write INSTR latch[2] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;
  case 0x07: instr_latch = ((instr_latch&U64(0xffffffff00ff)) | ((INT64)data&0xff)<< 8); logerror("ES5510: Write INSTR latch[1] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;
  case 0x08: instr_latch = ((instr_latch&U64(0xffffffffff00)) | ((INT64)data&0xff)<< 0); logerror("ES5510: Write INSTR latch[0] = %02x -> %012" I64FMT "x\n", data, instr_latch); break;

    /* 0x09 to 0x0b DIL Register (r/o) */

  case 0x0c: dol_latch = (dol_latch&0x00ffff) | ((data&0xff)<<16); logerror("ES5510: Write DOL latch[2] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch)); break;
  case 0x0d: dol_latch = (dol_latch&0xff00ff) | ((data&0xff)<< 8); logerror("ES5510: Write DOL latch[1] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch)); break;
  case 0x0e: dol_latch = (dol_latch&0xffff00) | ((data&0xff)<< 0); logerror("ES5510: Write DOL latch[0] = %02x -> %06x (%d)\n", data, dol_latch, SX(dol_latch)); break; //TODO: docs says that this always returns 0xff

  case 0x0f:
    dadr_latch = (dadr_latch&0x00ffff) | ((data&0xff)<<16);
    if (ram_sel) 
    {
      dil_latch = dram[dadr_latch];
    }
    else
    {
      dram[dadr_latch] = dol_latch;
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
    logerror("ES5510: Write Host Serial control %02x: %s, %s, ser3 %s, ser2 %s, ser1 %s, ser0 %s\n", data,
	     data&0x80 ? "Master" : "Slave",
	     data&0x40 ? "Sony" : "I2S",
	     data & 0x20 ? "Out" : "In", 
	     data & 0x10 ? "Out" : "In", 
	     data & 0x08 ? "Out" : "In", 
	     data & 0x04 ? "Out" : "In");
    break;

    /* 0x1f Halt enable (w) / Frame Counter (r) */
  case 0x1F: 
    logerror("ES5510: Write Halt Enable %02x; HALT line is %d\n", data, input_state(ES5510_HALT));
    if (input_state(ES5510_HALT)) {
      logerror("ES5510: Write to Halt Enable while HALT line is asserted: Halting!\n");
      state = STATE_HALTED;
    }
    break;

  case 0x80: /* Read select - GPR + INSTR */
    logerror("ES5510: Read INSTR+GPR %02x (%s): %012" I64FMT "x %06x (%d)\n", data, REGNAME(data & 0xff), instr[data] & 0xffffffffffffL, gpr[data] & 0xffffff, gpr[data]);

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
    logerror("ES5510: Write GPR %02x (%s): %06x (%d)\n",data, REGNAME(data&0xff), gpr_latch, SX(gpr_latch));
    write_reg(data, gpr_latch);
    break;

  case 0xc0: /* Write select - INSTR */
    DESCRIBE_INSTR(buf, instr_latch, gpr[data]);
    logerror("ES5510: Write INSTR %02x %012" I64FMT "x: %s\n",data, instr_latch&0xffffffffffffL, buf);
    if (data < 0xa0) {
      instr[data] = instr_latch&0xffffffffffffL;
    }
    break;

  case 0xe0: /* Write select - GPR + INSTR */
    DESCRIBE_INSTR(buf, instr_latch, gpr_latch);
    logerror("ES5510:  Write INSTR+GPR %02x (%s): %012" I64FMT "x %06x (%d): %s\n",data, REGNAME(data&0xff), instr_latch, gpr_latch, SX(gpr_latch), buf);
    if (data < 0xa0) {
      instr[data] = instr_latch;
    }
    write_reg(data, gpr_latch);
    break;
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
}

const address_space_config *es5510_device::memory_space_config(address_spacenum spacenum) const {
  return 0;
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

void es5510_device::execute_run() {
  while (icount > 0) {
    if (state == STATE_HALTED) {
      // Currently halted, sample the HALT line
      if (input_state(ES5510_HALT)) {
	// remain halted
	host_control |= 0x04; // Signal Host Access OK
      } else {
	logerror("ES5501: Starting!\n");
	state = STATE_RUNNING;
	
	UINT8 addr;
	char buf[1024];
	for (addr = 0; addr < 0xa0; addr++) {
	  DESCRIBE_INSTR(buf, instr[addr], gpr[addr]);
	  logerror("%02x: %012" I64FMT "x %06x  %s\n", addr, instr[addr], gpr[addr]&0xffffff, buf);
	}
	for (; addr < 0xc0; addr++) {
	  logerror("%02x: %06x (%d)\n", addr, gpr[addr]&0xffffff, gpr[addr]);
	}
      }
    } else {
      // currently running, execute one instruction.

      ram_pp = ram_p;
      ram_p = ram;
      
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
	  dil = dram[ram_pp.address];
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
	ram.address = (((dbase + offset) % (dlength + 1)) & memmask) >> memshift;
	break;
      case RAM_CONTROL_TABLE_A:
	ram.address = ((abase + offset) & memmask) >> memshift;
	break;
      case RAM_CONTROL_TABLE_B:
	ram.address = ((bbase + offset) & memmask) >> memshift;
	break;
      case RAM_CONTROL_IO:
	ram.address = offset & 0x00fffff0; // mask off the low 4 bits
	break;
      }
      
      // *** T1, clock high
      // --- Decode instruction N;
      //     we will do this both here and in stages as the different parts of the instruction complete & recommence.

      UINT8 operandSelect = (UINT8)((instr >> 8) & 0x0f);
      const op_select_t &opSelect = OPERAND_SELECT[operandSelect];
      bool skip = false;
      bool skippable = ((instr >> 7) & 0x01) != 0; // aka the 'SKIP' bit in the instruction word
      if (skippable) {
	bool skipConditionSatisfied = (ccr & cmr & FLAG_MASK) != 0;
	if (isFlagSet(cmr, FLAG_NOT)) {
	  skipConditionSatisfied = !skipConditionSatisfied;
	}
	skip = skipConditionSatisfied;
      }

      // --- Write Multiplier result N-1
      if (mulacc.write_result) {
	mulacc.product = (mulacc.cValue * mulacc.dValue) << mulshift;
	mulacc.result = (mulacc.accumulate ? machl : 0) + mulacc.product;
	INT32 tmp = (mulacc.result & 0x0000ffffff000000) >> 24;
	if (mulacc.dst & SRC_DST_REG) {
	  machl = mulacc.result;
	  write_reg(mulacc.cReg, tmp);
	}
	if (mulacc.dst & SRC_DST_DELAY) {
	  write_to_dol(tmp);
	}
      }

      // *** T1, clock low
      // --- Start of multiplier cycle N
      mulacc.cReg = (UINT8)((instr >> 32) & 0xff);
      mulacc.dReg = (UINT8)((instr >> 40) & 0xff);
      mulacc.src = opSelect.mac_src;
      mulacc.dst = opSelect.mac_dst;
      mulacc.accumulate = ((instr >> 6) & 0x01) != 0;
      mulacc.write_result = skip;

      // --- Read Multiplier Operands N
      if (mulacc.src == SRC_DST_REG) {
	mulacc.cValue = read_reg(mulacc.cReg);
      } else { // must be SRC_DST_DELAY 
	mulacc.cValue = dil;
      }
      mulacc.dValue = read_reg(mulacc.dReg);
      
      // *** T2, clock high
      // --- Write ALU Result N-1
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
      // --- Start of ALU cycle N
      alu.aReg = (instr >> 16) & 0xff;
      alu.bReg = (instr >> 24) & 0xff;
      alu.op = (instr >> 12) & 0x0f;
      alu.src = opSelect.alu_src;
      alu.dst = opSelect.alu_dst;
      alu.write_result = skip;
      alu.update_ccr = !skippable || (alu.op == OP_CMP);

      // --- Read ALU Operands N
      alu_op_t aluOp = ALU_OPS[alu.op];
      if (aluOp.operands == 2) {
	if (alu.src == SRC_DST_REG) {
	  alu.aValue = read_reg(alu.aReg);
	} else { // must be SRC_DST_DELAY
	  alu.aValue = dil;
	}
      }
      if (aluOp.operands >= 1) {
	alu.bValue = read_reg(alu.bReg);
      }

      // --- RAM cycle N-1
      if (ram_p.cycle != RAM_CYCLE_READ) {
	if (ram_p.cycle == RAM_CYCLE_WRITE) {
	  // If this is a write cycle, write the frontmost DOL value to RAM or I/O
	  if (ram_p.io) {
	    // write_io(ram_p.io, dol[0]);
	  } else {
	    dram[ram_p.address] = dol[0];
	  }
	}
	// If this is a Write or Dump cycle, eject the frontmost DL value.
	dol[0] = dol[1];
	if (dol_count > 0) {
	  --dol_count;
	}
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

INT32 es5510_device::read_reg(UINT8 reg)
{
  if (reg < 0xc0) {
    return gpr[reg];
  } else {
    switch(reg)
    {
    case 234: return ser0r;
    case 235: return ser0l;
    case 236: return ser1r;
    case 237: return ser1l;
    case 238: return ser2r;
    case 239: return ser2l;
    case 240: return ser3r;
    case 241: return ser3l;
    case 242: return (machl >>  0) & 0x00ffffff;
    case 243: return (machl >> 24) & 0x00ffffff;
    case 244: return dil; // DIL when reading
    case 245: return dlength;
    case 246: return abase;
    case 247: return bbase;
    case 248: return dbase;
    case 249: return sigreg;
    case 250: return ccr;
    case 251: return cmr;
    case 252: return 0x00ffffff;
    case 253: return 0x00800000;
    case 254: return 0x007fffff;
    case 255: return 0x00000000;
    default:
      // unknown SPR
      return 0;
    }
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

void es5510_device::write_reg(UINT8 reg, INT32 value)
{
  value &= 0x00ffffff;
  if (reg < 0xc0) {
    gpr[reg] = value;
  } else {
    switch(reg)
    {
    case 234: ser0r = value;
    case 235: ser0l = value;
    case 236: ser1r = value;
    case 237: ser1l = value;
    case 238: ser2r = value;
    case 239: ser2l = value;
    case 240: ser3r = value;
    case 241: ser3l = value;
    case 242: machl = (machl & ~((INT64)0x00ffffff <<  0)) | (value <<  0);
    case 243: machl = (machl & ~((INT64)0x00ffffff << 24)) | (value << 24);
    case 244: 
      memshift = countLowOnes(value);
      memsiz = 0x00ffffff >> (24 - memshift);
      memmask = 0x00ffffff & ~memsiz;
      memincrement = 1 << memshift;
    case 245: dlength = value;
    case 246: abase = value;
    case 247: bbase = value;
    case 248: dbase = value;
    case 249: sigreg = (value != 0);
    case 250: ccr = (value >> 16) & FLAG_MASK;
    case 251: cmr = (value >> 16) & (FLAG_MASK | FLAG_NOT);
    case 252: // no-op
    case 253: // no-op
    case 254: // no-op
    case 255: // no-op
    default:
      // unknown register
      ;
    }
  }
}

void es5510_device::write_to_dol(INT32 value) {
  if (dol_count >= 2) {
    dol[0] = dol[1];
    dol[1] = value;
  } else {
    dol[dol_count++] = value;
  }
}

INT32 es5510_device::alu_operation(UINT8 op, INT32 a, INT32 b, UINT8 &flags) {
  switch(op) {
  case 0x0: // ADD
    return saturate(add(a, b, flags), flags);
    
  case 0x1: // SUB
    return saturate(add(a, negate(b), flags), flags);

  case 0x2: // ADDU
    return add(a, b, flags);

  case 0x3: // SUBU
    return add(a, negate(b), flags);

  case 0x4: // CMP
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
    if (isNegative) {
      a = (a ^ 0x00ffffff) + 1;
    }
    return a;
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

  case 0xF: // END
    // sample the HALT line
    if (input_state(ES5510_HALT)) {
      // halt
      state = STATE_HALTED;      
      host_control |= 0x04; // Signal Host Access OK
    }
    // update the delay line base pointer
    dbase -= memincrement;
    if (dbase < 0) {
      dbase = dlength;
    }

  default:
    return 0;
  }
}
