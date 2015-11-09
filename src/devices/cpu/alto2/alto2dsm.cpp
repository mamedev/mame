// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 *   Xerox AltoII disassembler
 *
 **********************************************************/
#include "alto2cpu.h"

#define loc_DASTART     0000420 // display list header
#define loc_DVIBITS     0000421 // display vertical field interrupt bitword
#define loc_ITQUAN      0000422 // interval timer stored quantity
#define loc_ITBITS      0000423 // interval timer bitword
#define loc_MOUSEX      0000424 // mouse X coordinate
#define loc_MOUSEY      0000425 // mouse Y coordinate
#define loc_CURSORX     0000426 // cursor X coordinate
#define loc_CURSORY     0000427 // cursor Y coordinate
#define loc_RTC         0000430 // real time clock
#define loc_CURMAP      0000431 // cursor bitmap (16 words up to 00450)
#define loc_WW          0000452 // interrupt wakeups waiting
#define loc_ACTIVE      0000453 // active interrupt bitword
#define loc_MASKTAB     0000460 // mask table for convert
#define loc_PCLOC       0000500 // saved interrupt PC
#define loc_INTVEC      0000501 // interrupt transfer vector (15 words up to 00517)
#define loc_KBLK        0000521 // disk command block address
#define loc_KSTAT       0000522 // disk status at start of current sector
#define loc_KADDR       0000523 // disk address of latest disk command
#define loc_KSIBITS     0000524 // sector interrupt bit mask
#define loc_ITTIME      0000525 // interval timer timer
#define loc_TRAPPC      0000527 // trap saved PC
#define loc_TRAPVEC     0000530 // trap vectors (up to 0567)
#define loc_TIMERDATA   0000570 // timer data (OS; up to 0577)
#define loc_EPLOC       0000600 // ethernet post location
#define loc_EBLOC       0000601 // ethernet interrupt bitmask
#define loc_EELOC       0000602 // ethernet ending count
#define loc_ELLOC       0000603 // ethernet load location
#define loc_EICLOC      0000604 // ethernet input buffer count
#define loc_EIPLOC      0000605 // ethernet input buffer pointer
#define loc_EOCLOC      0000606 // ethernet output buffer count
#define loc_EOPLOC      0000607 // ethernet output buffer pointer
#define loc_EHLOC       0000610 // ethernet host address
#define loc_ERSVD       0000611 // reserved for ethernet expansion (up to 00612)
#define loc_ALTOV       0000613 // Alto I/II indication that microcode caninterrogate (0 = Alto I, -1 = Alto II)
#define loc_DCBR        0000614 // posted by parity task (main memory parity error)
#define loc_KNMAR       0000615 // -"-
#define loc_DWA         0000616 // -"-
#define loc_CBA         0000617 // -"-
#define loc_PC          0000620 // -"-
#define loc_SAD         0000621 // -"-
#define loc_SWATR       0000700 // saved registers (Swat; up to 00707)
#define loc_UTILOUT     0177016 // printer output (up to 177017)
#define loc_XBUS        0177020 // untility input bus (up to 177023)
#define loc_MEAR        0177024 // memory error address register
#define loc_MESR        0177025 // memory error status register
#define loc_MECR        0177026 // memory error control register
#define loc_UTILIN      0177030 // printer status, mouse keyset
#define loc_KBDAD       0177034 // undecoded keyboard (up to 177037)
#define loc_BANKREGS    0177740 // extended memory option bank registers

/**
 * @brief Microcode and constants PROM size
 */
#define MCODE_PAGE  1024
#define MCODE_SIZE  (2*MCODE_PAGE)      /* Alto II may have 2 pages (or even 4?) */
#define MCODE_MASK  (MCODE_SIZE - 1)
#define PROM_SIZE   256

/**
 * @brief short names for the 16 tasks
 */
static const char *taskname[16] = {
	"EMU",      // emulator task
	"T01",
	"T02",
	"T03",
	"DSC",      // disk sector task
	"T05",
	"T06",
	"ETH",      // ethernet task
	"MRT",      // memory refresh task
	"DWT",      // display word task
	"CUR",      // cursor task
	"DHT",      // display horizontal task
	"DVT",      // display vertical task
	"PAR",      // parity task
	"DWD",      // disk word task
	"T17"
};

/**
 * @brief names for the 32 R registers
 */
static const char *regname[32] = {
	"AC(3)",    // emulator accu 3
	"AC(2)",    // emulator accu 2
	"AC(1)",    // emulator accu 1
	"AC(0)",    // emulator accu 0
	"R04",
	"R05",
	"PC",       // emulator program counter
	"R07",
	"R10",
	"R11",
	"R12",
	"R13",
	"R14",
	"R15",
	"R16",
	"R17",
	"R20",
	"R21",
	"CBA",      // address of the currently active DCB+1
	"AECL",     // address of end of current scanline's bitmap
	"SLC",      // scan line count
	"HTAB",     // number of tab words remaining on current scanline
	"DWA",      // address of the bit map double word being fetched
	"MTEMP",    // temporary cell
	"R30",
	"R31",
	"R32",
	"R33",
	"R34",
	"R35",
	"R36",
	"R37"
};

//! for ALUF which is the value loaded into T, if t flags is set
static const char* t_bus_alu[16] = {
	"ALU",
	"BUS",
	"ALU",
	"BUS",
	"BUS",
	"ALU",
	"ALU",
	"BUS",
	"BUS",
	"BUS",
	"ALU",
	"ALU",
	"ALU",
	"BUS",
	"BUS",
	"BUS",
};

/**
 * @brief copy of the constant PROM, which this disassembler may not have access to
 */
static UINT16 const_prom[PROM_SIZE] = {
	/* 0000 */  0x0000, 0x0001, 0x0002, 0xfffe, 0xffff, 0xffff, 0x000f, 0xffff,
	/* 0008 */  0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0xfff8, 0xfff8,
	/* 0010 */  0x0010, 0x001f, 0x0020, 0x003f, 0x0040, 0x007f, 0x0080, 0x0007,
	/* 0018 */  0x00ff, 0xff00, 0x0400, 0x0100, 0x0110, 0x0151, 0x0114, 0x000f,
	/* 0020 */  0x0116, 0x0118, 0x0ffa, 0xf000, 0x4000, 0xfffc, 0xfff6, 0xffeb,
	/* 0028 */  0x4800, 0x6c00, 0x0800, 0x1000, 0xfe00, 0x7fff, 0x7fe0, 0x7f00,
	/* 0030 */  0xffbd, 0x0f00, 0x0f0f, 0xf0f0, 0x6048, 0x3000, 0x7159, 0x2109,
	/* 0038 */  0x6a3c, 0x4213, 0xa5a5, 0xfe1c, 0x3f00, 0xffc0, 0x012a, 0x0140,
	/* 0040 */  0x8000, 0xffe0, 0x00bf, 0xfff9, 0xfff0, 0xfffd, 0x0970, 0x5d20,
	/* 0048 */  0x3844, 0x6814, 0xfc00, 0xfe20, 0xfe22, 0x0083, 0x00f0, 0xff80,
	/* 0050 */  0xf800, 0xe000, 0xc000, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff,
	/* 0058 */  0x3fff, 0x0200, 0x2000, 0xfff1, 0x0156, 0x0157, 0x0138, 0x0c00,
	/* 0060 */  0x0130, 0x1813, 0x0180, 0x0181, 0x0182, 0x0183, 0x0184, 0x0185,
	/* 0068 */  0x0186, 0x0187, 0x0188, 0x018a, 0x0112, 0x0113, 0x0102, 0xfff0,
	/* 0070 */  0x0153, 0x0154, 0xffef, 0xffe4, 0xfffb, 0x000a, 0xffc1, 0x001f,
	/* 0078 */  0x0e00, 0x007e, 0xff7e, 0x0018, 0x000d, 0x03f8, 0x83f9, 0xffe0,
	/* 0080 */  0xfbff, 0x0009, 0x000b, 0x000c, 0x000e, 0x0030, 0x01fe, 0xff7f,
	/* 0088 */  0x81ff, 0xffbf, 0xffcc, 0x0557, 0x0041, 0x0198, 0x0199, 0x01a2,
	/* 0090 */  0xfe72, 0xfe58, 0x0012, 0x0014, 0x00dd, 0x02ff, 0x0101, 0x0001,
	/* 0098 */  0x0401, 0x0011, 0x0013, 0x0015, 0x0016, 0x0017, 0x0019, 0x0003,
	/* 00a0 */  0x03bd, 0x01de, 0xfe50, 0x00c0, 0x0c01, 0x6200, 0x6300, 0x0008,
	/* 00a8 */  0x6400, 0x6500, 0x6e00, 0x6700, 0x6900, 0x6d00, 0x6600, 0x000c,
	/* 00b0 */  0x6b00, 0x6b01, 0x6b02, 0x6b03, 0x6b04, 0x6b05, 0x6b06, 0x0010,
	/* 00b8 */  0x6b07, 0x6b08, 0x6b09, 0x6b0a, 0x6b0b, 0x6b0c, 0x6b0d, 0x0020,
	/* 00c0 */  0x6b0e, 0x6b0f, 0xfff3, 0xfe14, 0xfe15, 0xfe16, 0x0ffc, 0x0040,
	/* 00c8 */  0x04ff, 0x05ff, 0x06ff, 0x013f, 0x017e, 0xfe7d, 0xffff, 0x0080,
	/* 00d0 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00c0,
	/* 00d8 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00ff,
	/* 00e0 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 00e8 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 00f0 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 00f8 */  0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

/**
 * @brief print a symbolic name for an mpc address
 *
 * @param a microcode address (mpc)
 * @return pointer to const string with the address or symbolic name
 */
static const char *addrname(int a)
{
	static char buffer[4][32];
	static int which = 0;
	char *dst;

	which = (which + 1) % 4;
	dst = buffer[which];

	if (a < 020) {
		// start value for mpc per task is the task number
		snprintf(dst, sizeof(buffer[0]), "*%s", taskname[a]);
	} else {
		snprintf(dst, sizeof(buffer[0]), "%04o", a);
	}
	return dst;
}

offs_t alto2_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	size_t len = 128;

	UINT32 mir = (static_cast<UINT32>(oprom[0]) << 24) |
			(static_cast<UINT32>(oprom[1]) << 16) |
			(static_cast<UINT32>(oprom[2]) << 8) |
			(static_cast<UINT32>(oprom[3]) << 0);
	UINT8 rsel = static_cast<UINT8>((mir >> 27) & 31);
	UINT8 aluf = static_cast<UINT8>((mir >> 23) & 15);
	UINT8 bs = static_cast<UINT8>((mir >> 20) & 7);
	UINT8 f1 = static_cast<UINT8>((mir >> 16) & 15);
	UINT8 f2 = static_cast<UINT8>((mir >> 12) & 15);
	UINT8 t = static_cast<UINT8>((mir >> 11) & 1);
	UINT8 l = static_cast<UINT8>((mir >> 10) & 1);
	offs_t next = static_cast<offs_t>(mir & 1023);
	const UINT8* src = oprom - 4 * pc + 4 * next;
	UINT32 next2 =  (static_cast<UINT32>(src[0]) << 24) |
			(static_cast<UINT32>(src[1]) << 16) |
			(static_cast<UINT32>(src[2]) << 8) |
			(static_cast<UINT32>(src[3]) << 0);
	UINT16 prefetch = next2 & 1023;
	char *dst = buffer;
	offs_t result = 1 | DASMFLAG_SUPPORTED;
	UINT8 pa;

	if (next != pc + 1)
		result |= DASMFLAG_STEP_OUT;

	if (t)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "T<-%s ", t_bus_alu[aluf]);
	if (l)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "L<- ");
	if (bs == 1)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "%s<- ", regname[rsel]);
	switch (aluf) {
	case  0: // T?: BUS
		// this is somehow redundant and just wasting space
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS) ");
		break;
	case  1: //   : T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(T) ");
		break;
	case  2: // T?: BUS OR T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS|T) ");
		break;
	case  3: //   : BUS AND T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS&T) ");
		break;
	case  4: //   : BUS XOR T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS^T) ");
		break;
	case  5: // T?: BUS + 1
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS+1) ");
		break;
	case  6: // T?: BUS - 1
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS-1) ");
		break;
	case  7: //   : BUS + T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS+T) ");
		break;
	case  8: //   : BUS - T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS-T) ");
		break;
	case  9: //   : BUS - T - 1
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS-T-1) ");
		break;
	case 10: // T?: BUS + T + 1
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS+T+1) ");
		break;
	case 11: // T?: BUS + SKIP
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS+SKIP) ");
		break;
	case 12: // T?: BUS, T (AND)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS,T) ");
		break;
	case 13: //   : BUS AND NOT T
		dst += snprintf(dst, len - (size_t)(dst - buffer), "ALUF(BUS&~T) ");
		break;
	case 14: //   : undefined
		dst += snprintf(dst, len - (size_t)(dst - buffer), "*ALUF(BUS) ");
		break;
	case 15: //   : undefined
		dst += snprintf(dst, len - (size_t)(dst - buffer), "*ALUF(BUS) ");
		break;
	}

	switch (bs) {
	case 0: // read R
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-%s ", regname[rsel]);
		break;
	case 1: // load R from shifter output
		// dst += snprintf(dst, len - (size_t)(dst - buffer), "; %s<-", regname[rsel]);
		break;
	case 2: // enables no source to the BUS, leaving it all ones
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-177777 ");
		break;
	case 3: // performs different functions in different tasks
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-BS3 ");
		break;
	case 4: // performs different functions in different tasks
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-BS4 ");
		break;
	case 5: // memory data
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-MD ");
		break;
	case 6: // BUS[3-0] <- MOUSE; BUS[15-4] <- -1
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-MOUSE ");
		break;
	case 7: // IR[7-0], possibly sign extended
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-DISP ");
		break;
	}


	switch (f1) {
	case 0: // no operation
		break;
	case 1: // load MAR from ALU output; start main memory reference
		dst += snprintf(dst, len - (size_t)(dst - buffer), "MAR<-ALU ");
		break;
	case 2: // switch tasks if higher priority wakeup is pending
		dst += snprintf(dst, len - (size_t)(dst - buffer), "TASK ");
		break;
	case 3: // disable the current task until re-enabled by a hardware-generated condition
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BLOCK ");
		break;
	case 4: // SHIFTER output will be L shifted left one place
		dst += snprintf(dst, len - (size_t)(dst - buffer), "SHIFTER<-L(LSH1) ");
		break;
	case 5: // SHIFTER output will be L shifted right one place
		dst += snprintf(dst, len - (size_t)(dst - buffer), "SHIFTER<-L(RSH1) ");
		break;
	case 6: // SHIFTER output will be L rotated left 8 places
		dst += snprintf(dst, len - (size_t)(dst - buffer), "SHIFTER<-L(LCY8) ");
		break;
	case 7: // put the constant from PROM (RSELECT,BS) on the bus
		pa = (rsel << 3) | bs;
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-%05o CONST[%03o]", const_prom[pa], pa);
		break;
	default:
		dst += snprintf(dst, len - (size_t)(dst - buffer), "F1_%02o ", f1);
		break;
	}

	switch (f2) {
	case 0: // no operation
		break;
	case 1: // NEXT <- NEXT OR (BUS==0 ? 1 : 0)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "[BUS==0 ? %s:%s] ",
			addrname((prefetch | 1) & MCODE_MASK), addrname(prefetch & MCODE_MASK));
		break;
	case 2: // NEXT <- NEXT OR (SHIFTER==0 ? 1 : 0)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "[SH==0 ? %s:%s] ",
			addrname((prefetch | 1) & MCODE_MASK), addrname(prefetch & MCODE_MASK));
		break;
	case 3: // NEXT <- NEXT OR (SHIFTER<0 ? 1 : 0)
		dst += snprintf(dst, len - (size_t)(dst - buffer), "[SH<0 ? %s:%s] ",
			addrname((prefetch | 1) & MCODE_MASK), addrname(prefetch & MCODE_MASK));
		break;
	case 4: // NEXT <- NEXT OR BUS
		dst += snprintf(dst, len - (size_t)(dst - buffer), "NEXT<-BUS ");
		break;
	case 5: // NEXT <- NEXT OR ALUC0. ALUC0 is the carry produced by last L loading microinstruction.
		dst += snprintf(dst, len - (size_t)(dst - buffer), "[ALUC0 ? %s:%s] ",
			addrname((prefetch | 1) & MCODE_MASK), addrname(prefetch & MCODE_MASK));
		break;
	case 6: // deliver BUS data to memory
		dst += snprintf(dst, len - (size_t)(dst - buffer), "MD<-BUS ");
		break;
	case 7: // put on the bus the constant from PROM (RSELECT,BS)
		if (f1 != 7) {
			pa = 8 * rsel + bs;
			dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-%05o CONST[%03o]", const_prom[pa], pa);
		}
		break;
	default:
		dst += snprintf(dst, len - (size_t)(dst - buffer), "BUS<-F2_%02o ", f2);
		break;
	}
	return result;
}
