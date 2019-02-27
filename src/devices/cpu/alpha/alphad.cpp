// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * TODO
 *   - enforce "should be zero" fields
 *   - enforce "Ra must be R31"
 *   - assembly and register naming preferences
 *   - implementation-dependent instructions for EV5 and EV6
 */

#include "emu.h"
#include "alphad.h"
#include "common.h"

 // instruction formats
#define OPERATE_RRR(opcode, ra, rb, rc)  util::stream_format(stream, "%-10s %s, %s, %s",   opcode, R[ra], R[rb], R[rc])
#define OPERATE_FFF(opcode, ra, rb, rc)  util::stream_format(stream, "%-10s %s, %s, %s",   opcode, F[ra], F[rb], F[rc])
#define OPERATE_RIR(opcode, ra, im, rc)  util::stream_format(stream, "%-10s %s, #%d, %s",  opcode, R[ra], im, R[rc])
#define OPERATE_RR(opcode, rb, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, R[rb], R[rc])
#define OPERATE_FF(opcode, rb, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, F[rb], F[rc])
#define OPERATE_RF(opcode, ra, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, R[ra], F[rc])
#define OPERATE_FR(opcode, ra, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, F[ra], R[rc])
#define OPERATE_IR(opcode, im, rc)       util::stream_format(stream, "%-10s #%d, %s",      opcode, im, R[rc])
#define OPERATE_R(opcode, rc)            util::stream_format(stream, "%-10s %s",           opcode, R[rc])
#define OPERATE_F(opcode, rc)            util::stream_format(stream, "%-10s %s",           opcode, F[rc])

// TODO: verify syntax for multiple register formats
#define OPERATE_RI(opcode, rb, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, R[rb], IBX[rc])
#define OPERATE_RA(opcode, rb, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, R[rb], ABX[rc])
#define OPERATE_RAI(opcode, rb, rc)      util::stream_format(stream, "%-10s %s, %s:%s",    opcode, R[rb], ABX[rc], IBX[rc])
#define OPERATE_RP(opcode, rb, rc)       util::stream_format(stream, "%-10s %s, %s",       opcode, R[rb], PT[rc])
#define OPERATE_RPI(opcode, rb, rc)      util::stream_format(stream, "%-10s %s, %s:%s",    opcode, R[rb], PT[rc], IBX[rc])
#define OPERATE_RPA(opcode, rb, rc)      util::stream_format(stream, "%-10s %s, %s:%s",    opcode, R[rb], PT[rc], ABX[rc])
#define OPERATE_RPAI(opcode, rb, rc)     util::stream_format(stream, "%-10s %s, %s:%s:%s", opcode, R[rb], PT[rc], ABX[rc], IBX[rc])

#define MEMORY_R(opcode, ra, disp, rb)   util::stream_format(stream, "%-10s %s, %d(%s)",   opcode, R[ra], disp, R[rb])
#define MEMORY_F(opcode, ra, disp, rb)   util::stream_format(stream, "%-10s %s, %d(%s)",   opcode, F[ra], disp, R[rb])
#define BRANCH_R(opcode, ra, offset)     util::stream_format(stream, "%-10s %s, 0x%x",     opcode, R[ra], pc + 4 + offset)
#define BRANCH_F(opcode, ra, offset)     util::stream_format(stream, "%-10s %s, 0x%x",     opcode, F[ra], pc + 4 + offset)
#define BRANCH(opcode, offset)           util::stream_format(stream, "%-10s 0x%x",         opcode, pc + 4 + offset)

#define JUMP(opcode, ra, rb)             util::stream_format(stream, "%-10s %s, (%s)",     opcode, R[ra], R[rb])

#define MISC(opcode)                     util::stream_format(stream, "%-10s",              opcode)
#define MISC_R(opcode, ra)               util::stream_format(stream, "%-10s %s",           opcode, R[ra])
#define MISC_M(opcode, rb)               util::stream_format(stream, "%-10s (%s)",         opcode, R[rb])

#define CALL_PAL(fnc)                    util::stream_format(stream, "%-10s %s",           "call_pal", fnc)

#define RESERVED(opcode)                 util::stream_format(stream, "%-10s",              opcode)
#define UNKNOWN(type)                    util::stream_format(stream, "unknown %s",         type)

// register names
const char *const alpha_disassembler::R[] =
{
	 "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
	 "r8",  "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

const char *const alpha_disassembler::F[] =
{
	 "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
	 "f8",  "f9", "f10", "f11", "f12", "f13", "f14", "f15",
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
};

const char *const alpha_disassembler::PT[] =
{
	 "pt0",  "pt1",  "pt2",  "pt3",  "pt4",  "pt5",  "pt6",  "pt7",
	 "pt8",  "pt9", "pt10", "pt11", "pt12", "pt13", "pt14", "pt15",
	"pt16", "pt17", "pt18", "pt19", "pt20", "pt21", "pt22", "pt23",
	"pt24", "pt25", "pt26", "pt27", "pt28", "pt29", "pt30", "pt31"
};

// NOTE: ABXn and IBXn are placeholders for undefined/unused registers
const char *const alpha_disassembler::ABX[] =
{
	"TB_CTL", "ABX1", "DTB_PTE", "DTB_PTE_TEMP", "MM_CSR", "VA", "DTBZAP", "DTBASM",
	"DTBIS", "BIU_ADDR", "BIU_STAT", "ABX12", "DC_STAT", "FILL_ADDR", "ABOX_CTL", "ALT_MODE",
	"CC", "CC_CTL", "BIU_CTL", "FILL_SYNDROME", "BC_TAG", "FLUSH_IC", "ABX22", "FLUSH_IC_ASM",
	"ABX24", "ABX25", "ABX26", "ABX27", "ABX28", "ABX29", "ABX30", "ABX31"
};

const char *const alpha_disassembler::IBX[] =
{
	"TB_TAG", "ITB_PTE", "ICCSR", "ITB_PTE_TEMP", "EXC_ADDR", "SL_RCV", "ITBZAP", "ITBASM",
	"ITBIS", "PS", "EXC_SUM", "PAL_BASE", "HIRR", "SIRR", "ASTRR", "IBX15",
	"HIERR", "SIER", "ASTER", "SL_CLR", "IBX20", "IBX21", "SL_XMIT", "IBX23",
	"IBX24", "IBX25", "IBX26", "IBX27", "IBX28", "IBX29", "IBX30", "IBX31"
};

offs_t alpha_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t const bytes = 4;
	u32 flags = SUPPORTED;

	u32 const instruction = opcodes.r32(pc);

	switch (instruction >> 26)
	{
		// pal format
		case 0x00:
			switch (m_dasm_type)
			{
			case TYPE_UNKNOWN:
				switch (instruction & 0x03ffffff)
				{
				case 0x0000: CALL_PAL("halt"); break;
				case 0x0002: CALL_PAL("draina"); break;
				case 0x0086: CALL_PAL("imb"); break;

				default: UNKNOWN("call_pal"); break;
				}
				break;

			case TYPE_NT:
				switch (instruction & 0x03ffffff)
				{
					// Windows NT unprivileged PALcode
				case 0x0080: CALL_PAL("bpt");        break; // breakpoint trap
				case 0x0083: CALL_PAL("callsys");    break; // call system service
				case 0x0086: CALL_PAL("imb");        break; // instruction memory barrier
				case 0x00aa: CALL_PAL("gentrap");    break; // generate trap
				case 0x00ab: CALL_PAL("rdteb");      break; // read TEB internal processor register
				case 0x00ac: CALL_PAL("kbpt");       break; // kernel breakpoint trap
				case 0x00ad: CALL_PAL("callkd");     break; // call kernel debugger

					// Windows NT privileged PALcode
				case 0x0000: CALL_PAL("halt");       break; // trap to illegal instruction
				case 0x0001: CALL_PAL("restart");    break; // restart the processor
				case 0x0002: CALL_PAL("draina");     break; // drain aborts
				case 0x0003: CALL_PAL("reboot");     break; // transfer to console firmware
				case 0x0004: CALL_PAL("initpal");    break; // initialize the PALcode
				case 0x0005: CALL_PAL("wrentry");    break; // write system entry
				case 0x0006: CALL_PAL("swpirql");    break; // swap IRQL
				case 0x0007: CALL_PAL("rdirql");     break; // read current IRQL
				case 0x0008: CALL_PAL("di");         break; // disable interrupts
				case 0x0009: CALL_PAL("ei");         break; // enable interrupts
				case 0x000a: CALL_PAL("swppal");     break; // swap PALcode
				case 0x000c: CALL_PAL("ssir");       break; // set software interrupt request
				case 0x000d: CALL_PAL("csir");       break; // clear software interrupt request
				case 0x000e: CALL_PAL("rfe");        break; // return from exception
				case 0x000f: CALL_PAL("retsys");     break; // return from system service call
				case 0x0010: CALL_PAL("swpctx");     break; // swap privileged thread context
				case 0x0011: CALL_PAL("swpprocess"); break; // swap privileged process context
				case 0x0012: CALL_PAL("rdmces");     break; // read machine check error summary
				case 0x0013: CALL_PAL("wrmces");     break; // write machine check error summary
				case 0x0014: CALL_PAL("tbia");       break; // translation buffer invalidate all
				case 0x0015: CALL_PAL("tbis");       break; // translation buffer invalidate single
				case 0x0016: CALL_PAL("dtbis");      break; // data translation buffer invalidate single
				case 0x0017: CALL_PAL("tbisasn");    break; // translation buffer invalidate single ASN
				case 0x0018: CALL_PAL("rdksp");      break; // read initial kernel stack
				case 0x0019: CALL_PAL("swpksp");     break; // swap initial kernel stack
				case 0x001a: CALL_PAL("rdpsr");      break; // read processor status register
				case 0x001c: CALL_PAL("rdpcr");      break; // read PCR (processor control registers)
				case 0x001e: CALL_PAL("rdthread");   break; // read the current thread value
				case 0x0020: CALL_PAL("wrperfmon");  break; // write performance monitoring values
				case 0x0030: CALL_PAL("rdcounters"); break; // read PALcode event counters
				case 0x0031: CALL_PAL("rdstate");    break; // read internal processor state

				default: UNKNOWN("call_pal"); break;
				}
				break;

			case TYPE_UNIX:
				switch (instruction & 0x03ffffff)
				{
					// Digital UNIX unprivileged PALcode
				case 0x0080: CALL_PAL("bpt");        break; // breakpoint trap
				case 0x0081: CALL_PAL("bugchk");     break; // bugcheck
				case 0x0083: CALL_PAL("callsys");    break; // system call
				case 0x0086: CALL_PAL("imb");        break; // i-stream memory barrier
				case 0x0092: CALL_PAL("urti");       break; // return from user mode trap
				case 0x009e: CALL_PAL("rdunique");   break; // read unique value
				case 0x009f: CALL_PAL("wrunique");   break; // write unique value
				case 0x00aa: CALL_PAL("gentrap");    break; // generate software trap
				case 0x00ae: CALL_PAL("clrfen");     break; // clear floating-point enable

					// Digital UNIX privileged PALcode
				case 0x0000: CALL_PAL("halt");       break; // halt the processor
				case 0x0001: CALL_PAL("cflush");     break; // cache flush
				case 0x0002: CALL_PAL("draina");     break; // drain aborts
				case 0x0009: CALL_PAL("cserve");     break; // console service
				case 0x000a: CALL_PAL("swppal");     break; // swap PALcode image
				case 0x000d: CALL_PAL("wripir");     break; // write interprocessor interrupt request
				case 0x0010: CALL_PAL("rdmces");     break; // read machine check error summary register
				case 0x0011: CALL_PAL("wrmces");     break; // write machine check error summary register
				case 0x002b: CALL_PAL("wrfen");      break; // write floating-point enable
				case 0x002d: CALL_PAL("wrvptptr");   break; // write virtual page table pointer
				case 0x0030: CALL_PAL("swpctx");     break; // swap privileged context
				case 0x0031: CALL_PAL("wrval");      break; // write system value
				case 0x0032: CALL_PAL("rdval");      break; // read system value
				case 0x0033: CALL_PAL("tbi");        break; // translation buffer invalidate
				case 0x0034: CALL_PAL("wrent");      break; // write system entry address
				case 0x0035: CALL_PAL("swpipl");     break; // swap interrupt priority level
				case 0x0036: CALL_PAL("rdps");       break; // read processor status
				case 0x0037: CALL_PAL("wrkgp");      break; // write kernel global pointer
				case 0x0038: CALL_PAL("wrusp");      break; // write user stack pointer
				case 0x0039: CALL_PAL("wrperfmon");  break; // performance monitoring function
				case 0x003a: CALL_PAL("rdusp");      break; // read user stack pointer
				case 0x003c: CALL_PAL("whami");      break; // who am I
				case 0x003d: CALL_PAL("retsys");     break; // return from system call
				case 0x003e: CALL_PAL("wtint");      break; // wait for interrupt
				case 0x003f: CALL_PAL("rti");        break; // return from trap or interrupt

				default: UNKNOWN("call_pal"); break;
				}
				break;

			case TYPE_VMS:
				switch (instruction & 0x03ffffff)
				{
					// OpenVMS unprivileged PALcode
				case 0x0080: CALL_PAL("bpt");          break; // breakpoint
				case 0x0081: CALL_PAL("bugchk");       break; // bugcheck
				case 0x0082: CALL_PAL("chme");         break; // change mode to executive
				case 0x0083: CALL_PAL("chmk");         break; // change mode to kernel
				case 0x0084: CALL_PAL("chms");         break; // change mode to supervisor
				case 0x0085: CALL_PAL("chmu");         break; // change mode to user
				case 0x0086: CALL_PAL("imb");          break; // i-stream memory barrier
				case 0x0087: CALL_PAL("insqhil");      break; // insert into longword queue at head interlocked
				case 0x0088: CALL_PAL("insqtil");      break; // insert into longword queue at tail interlocked
				case 0x0089: CALL_PAL("insqhiq");      break; // insert into quadword queue at head interlocked
				case 0x008a: CALL_PAL("insqtiq");      break; // insert into quadword queue at tail interlocked
				case 0x008b: CALL_PAL("insquel");      break; // insert entry into longword queue
				case 0x008c: CALL_PAL("insqueq");      break; // insert entry into quadword queue
				case 0x008d: CALL_PAL("insquel/d");    break; // insert entry into longword queue deferred
				case 0x008e: CALL_PAL("insqueq/d");    break; // insert entry into quadword queue deferred
				case 0x008f: CALL_PAL("prober");       break; // probe for read access
				case 0x0090: CALL_PAL("probew");       break; // probe for write access
				case 0x0091: CALL_PAL("rd_ps");        break; // move processor status
				case 0x0092: CALL_PAL("rei");          break; // return from exception or interrupt
				case 0x0093: CALL_PAL("remqhil");      break; // remove from longword queue at head interlocked
				case 0x0094: CALL_PAL("remqtil");      break; // remove from longword queue at tail interlocked
				case 0x0095: CALL_PAL("remqhiq");      break; // remove from quadword queue at head interlocked
				case 0x0096: CALL_PAL("remqtiq");      break; // remove from quadword queue at tail interlocked
				case 0x0097: CALL_PAL("remquel");      break; // remove entry from longword queue
				case 0x0098: CALL_PAL("remqueq");      break; // remove entry from quadword queue
				case 0x0099: CALL_PAL("remquel/d");    break; // remove entry from longword queue deferred
				case 0x009a: CALL_PAL("remqueq/d");    break; // remove entry from quadword queue deferred
				case 0x009b: CALL_PAL("swasten");      break; // swap AST enable for current mode
				case 0x009c: CALL_PAL("wr_ps_sw");     break; // write processor status software field
				case 0x009d: CALL_PAL("rscc");         break; // read system cycle counter
				case 0x009e: CALL_PAL("read_unq");     break; // read unique context
				case 0x009f: CALL_PAL("write_unq");    break; // write unique context
				case 0x00a0: CALL_PAL("amovrr");       break; // atomic move from register to register
				case 0x00a1: CALL_PAL("amovrm");       break; // atomic move from register to memory
				case 0x00a2: CALL_PAL("insqhilr");     break; // insert into longword queue at head interlocked resident
				case 0x00a3: CALL_PAL("insqtilr");     break; // insert into longword queue at tail interlocked resident
				case 0x00a4: CALL_PAL("insqhiqr");     break; // insert into quadword queue at head interlocked resident
				case 0x00a5: CALL_PAL("insqtiqr");     break; // insert into quadword queue at tail interlocked resident
				case 0x00a6: CALL_PAL("remqhilr");     break; // remove from longword queue at head interlocked resident
				case 0x00a7: CALL_PAL("remqtilr");     break; // remove from longword queue at tail interlocked resident
				case 0x00a8: CALL_PAL("remqhiqr");     break; // remove from quadword queue at head interlocked resident
				case 0x00a9: CALL_PAL("remqtiqr");     break; // remove from quadword queue at tail interlocked resident
				case 0x00aa: CALL_PAL("gentrap");      break; // generate software trap
				case 0x00ae: CALL_PAL("clrfen");       break; // clear floating-point enable

					// OpenVMS privileged PALcode
				case 0x0000: CALL_PAL("halt");         break; // halt processor
				case 0x0001: CALL_PAL("cflush");       break; // cache flush
				case 0x0002: CALL_PAL("draina");       break; // drain aborts
				case 0x0003: CALL_PAL("ldqp");         break; // load quadword physical
				case 0x0004: CALL_PAL("stqp");         break; // store quadword physical
				case 0x0005: CALL_PAL("swpctx");       break; // swap privileged context
				case 0x0006: CALL_PAL("mfpr_asn");     break; // move from processor register ASN
				case 0x0009: CALL_PAL("cserve");       break; // console service
				case 0x000a: CALL_PAL("swppal");       break; // swap PALcode image
				case 0x000b: CALL_PAL("mfpr_fen");     break; // move from processor register FEN
				case 0x000c: CALL_PAL("mtpr_fen");     break; // move to processor register FEN
				case 0x000d: CALL_PAL("mtpr_ipir");    break; // move to processor register IPRI
				case 0x000e: CALL_PAL("mfpr_ipl");     break; // move from processor register IPL
				case 0x000f: CALL_PAL("mtpr_ipl");     break; // move to processor register IPL
				case 0x0010: CALL_PAL("mfpr_mces");    break; // move from processor register MCES
				case 0x0011: CALL_PAL("mtpr_mces");    break; // move to processor register MCES
				case 0x0012: CALL_PAL("mfpr_pcbb");    break; // move from processor register PCBB
				case 0x0013: CALL_PAL("mfpr_prbr");    break; // move from processor register PRBR
				case 0x0014: CALL_PAL("mtpr_prbr");    break; // move to processor register PRBR
				case 0x0015: CALL_PAL("mfpr_ptbr");    break; // move from processor register PTBR
				case 0x0016: CALL_PAL("mfpr_scbb");    break; // move from processor register SCBB
				case 0x0017: CALL_PAL("mtpr_scbb");    break; // move to processor register SCBB
				case 0x0018: CALL_PAL("mtpr_sirr");    break; // move to processor register SIRR
				case 0x0019: CALL_PAL("mfpr_sisr");    break; // move from processor register SISR
				case 0x001a: CALL_PAL("mfpr_tbchk");   break; // move from processor register TBCHK
				case 0x001b: CALL_PAL("mtpr_tbia");    break; // move to processor register TBIA
				case 0x001c: CALL_PAL("mtpr_tbiap");   break; // move to processor register TBIAP
				case 0x001d: CALL_PAL("mtpr_tbis");    break; // move to processor register TBIS
				case 0x001e: CALL_PAL("mfpr_esp");     break; // move from processor register ESP
				case 0x001f: CALL_PAL("mtpr_esp");     break; // move to processor register ESP
				case 0x0020: CALL_PAL("mfpr_ssp");     break; // move from processor register SSP
				case 0x0021: CALL_PAL("mtpr_ssp");     break; // move to processor register SSP
				case 0x0022: CALL_PAL("mfpr_usp");     break; // move from processor register USP
				case 0x0023: CALL_PAL("mtpr_usp");     break; // move to processor register USP
				case 0x0024: CALL_PAL("mtpr_tbisd");   break; // move to processor register TBISD
				case 0x0025: CALL_PAL("mtpr_tbisi");   break; // move to processor register TBISI
				case 0x0026: CALL_PAL("mtpr_asten");   break; // move to processor register ASTEN
				case 0x0027: CALL_PAL("mtpr_astsr");   break; // move to processor register ASTSR
				case 0x0029: CALL_PAL("mfpr_vptb");    break; // move from processor register VPTB
				case 0x002a: CALL_PAL("mtpr_vptb");    break; // move to processor register VPTB
				case 0x002b: CALL_PAL("mtpr_perfmon"); break; // move to processor register PERFMON
				case 0x002e: CALL_PAL("mtpr_datfx");   break; // move to processor register DATFX
				case 0x003e: CALL_PAL("wtint");        break; // wait for interrupt
				case 0x003f: CALL_PAL("mfpr_whami");   break; // move from processor register WHAMI

				default: UNKNOWN("call_pal"); break;
				}
				break;
			}
			break;

		// reserved opcodes
		case 0x01: RESERVED("opc01"); break; // OPC01
		case 0x02: RESERVED("opc02"); break; // OPC02
		case 0x03: RESERVED("opc03"); break; // OPC03
		case 0x04: RESERVED("opc04"); break; // OPC04
		case 0x05: RESERVED("opc05"); break; // OPC05
		case 0x06: RESERVED("opc06"); break; // OPC06
		case 0x07: RESERVED("opc07"); break; // OPC07

		// memory format
		case 0x08:
			if (Rb(instruction) == 31)
				OPERATE_IR("mov", Disp_M(instruction), Ra(instruction));
			else
				MEMORY_R("lda",   Ra(instruction), Disp_M(instruction), Rb(instruction)); // LDA
			break;
		case 0x09: MEMORY_R("ldah",  Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDAH
		case 0x0a: MEMORY_R("ldbu",  Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDBU (BWX)
		case 0x0b:
			if (Ra(instruction) == 31 && Disp_M(instruction) == 0)
				MISC("unop");
			else
				MEMORY_R("ldq_u", Ra(instruction), Disp_M(instruction), Rb(instruction)); // LDQ_U
			break;
		case 0x0c: MEMORY_R("ldwu",  Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDWU (BWX)
		case 0x0d: MEMORY_R("stw",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STW (BWX)
		case 0x0e: MEMORY_R("stb",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STB (BWX)
		case 0x0f: MEMORY_R("stq_u", Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STQ_U

		// operate format
		case 0x10: // INTA* (integer arithmetic)
			switch ((instruction >> 5) & 0xff)
			{
				// register variants
				case 0x00:
					if (Ra(instruction) == 31)
						OPERATE_RR("sextl", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("addl", Ra(instruction), Rb(instruction), Rc(instruction)); // ADDL
					break;
				case 0x02: OPERATE_RRR("s4addl", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S4ADDL
				case 0x09:
					if (Ra(instruction) == 31)
						OPERATE_RR("negl", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("subl", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBL
					break;
				case 0x0b: OPERATE_RRR("s4subl", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S4SUBL
				case 0x0f: OPERATE_RRR("cmpbge", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPBGE
				case 0x12: OPERATE_RRR("s8addl", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S8ADDL
				case 0x1b: OPERATE_RRR("s8subl", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S8SUBL
				case 0x1d: OPERATE_RRR("cmpult", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPULT
				case 0x20: OPERATE_RRR("addq",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDQ
				case 0x22: OPERATE_RRR("s4addq", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S4ADDQ
				case 0x29:
					if (Ra(instruction) == 31)
						OPERATE_RR("negq", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("subq", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBQ
					break;
				case 0x2b: OPERATE_RRR("s4subq", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S4SUBQ
				case 0x2d: OPERATE_RRR("cmpeq",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPEQ
				case 0x32: OPERATE_RRR("s8addq", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S8ADDQ
				case 0x3b: OPERATE_RRR("s8subq", Ra(instruction), Rb(instruction), Rc(instruction)); break; // S8SUBQ
				case 0x3d: OPERATE_RRR("cmpule", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPULE
				case 0x40: OPERATE_RRR("addl/v", Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDL/V
				case 0x49:
					if (Ra(instruction) == 31)
						OPERATE_RR("negl/v", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("subl/v", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBL/V
					break;
				case 0x4d: OPERATE_RRR("cmplt",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPLT
				case 0x60: OPERATE_RRR("addq/v", Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDQ/V
				case 0x69:
					if (Ra(instruction) == 31)
						OPERATE_RR("negq/v", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("subq/v", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBQ/V
					break;
				case 0x6d: OPERATE_RRR("cmple",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPLE

				// immediate variants
				case 0x80:
					if (Ra(instruction) == 31)
						OPERATE_IR("sextl", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("addl", Ra(instruction), Im(instruction), Rc(instruction)); // ADDL
					break;
				case 0x82: OPERATE_RIR("s4addl", Ra(instruction), Im(instruction), Rc(instruction)); break; // S4ADDL
				case 0x89:
					if (Ra(instruction) == 31)
						OPERATE_IR("negl", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("subl", Ra(instruction), Im(instruction), Rc(instruction)); // SUBL
					break;
				case 0x8b: OPERATE_RIR("s4subl", Ra(instruction), Im(instruction), Rc(instruction)); break; // S4SUBL
				case 0x8f: OPERATE_RIR("cmpbge", Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPBGE
				case 0x92: OPERATE_RIR("s8addl", Ra(instruction), Im(instruction), Rc(instruction)); break; // S8ADDL
				case 0x9b: OPERATE_RIR("s8subl", Ra(instruction), Im(instruction), Rc(instruction)); break; // S8SUBL
				case 0x9d: OPERATE_RIR("cmpult", Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPULT
				case 0xa0: OPERATE_RIR("addq",   Ra(instruction), Im(instruction), Rc(instruction)); break; // ADDQ
				case 0xa2: OPERATE_RIR("s4addq", Ra(instruction), Im(instruction), Rc(instruction)); break; // S4ADDQ
				case 0xa9:
					if (Ra(instruction) == 31)
						OPERATE_IR("negq", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("subq", Ra(instruction), Im(instruction), Rc(instruction)); // SUBQ
					break;
				case 0xab: OPERATE_RIR("s4subq", Ra(instruction), Im(instruction), Rc(instruction)); break; // S4SUBQ
				case 0xad: OPERATE_RIR("cmpeq",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPEQ
				case 0xb2: OPERATE_RIR("s8addq", Ra(instruction), Im(instruction), Rc(instruction)); break; // S8ADDQ
				case 0xbb: OPERATE_RIR("s8subq", Ra(instruction), Im(instruction), Rc(instruction)); break; // S8SUBQ
				case 0xbd: OPERATE_RIR("cmpule", Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPULE
				case 0xc0: OPERATE_RIR("addl/v", Ra(instruction), Im(instruction), Rc(instruction)); break; // ADDL/V
				case 0xc9:
					if (Ra(instruction) == 31)
						OPERATE_IR("negl/v", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("subl/v", Ra(instruction), Im(instruction), Rc(instruction)); // SUBL/V
					break;
				case 0xcd: OPERATE_RIR("cmplt",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPLT
				case 0xe0: OPERATE_RIR("addq/v", Ra(instruction), Im(instruction), Rc(instruction)); break; // ADDQ/V
				case 0xe9:
					if (Ra(instruction) == 31)
						OPERATE_IR("negq/v", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("subq/v", Ra(instruction), Im(instruction), Rc(instruction)); // SUBQ/V
					break;
				case 0xed: OPERATE_RIR("cmple",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMPLE

				default: UNKNOWN("inta*"); break;
			}
			break;
		case 0x11: // INTL* (integer logical)
			switch ((instruction >> 5) & 0xff)
			{
				// register variants
				case 0x00: OPERATE_RRR("and",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // AND
				case 0x08: OPERATE_RRR("bic",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // BIC
				case 0x14: OPERATE_RRR("cmovlbs", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVLBS
				case 0x16: OPERATE_RRR("cmovlbc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVLBC
				case 0x20:
					if (Ra(instruction) == 31 && Rb(instruction) == 31 && Rc(instruction) == 31)
						MISC("nop");
					else if (Ra(instruction) == 31 && Rb(instruction) == 31)
						OPERATE_R("clr", Rc(instruction));
					else if (Ra(instruction) == Rb(instruction))
						OPERATE_RR("mov", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("bis", Ra(instruction), Rb(instruction), Rc(instruction)); // BIS
					break;
				case 0x24: OPERATE_RRR("cmoveq",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVEQ
				case 0x26: OPERATE_RRR("cmovne",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVNE
				case 0x28:
					if (Ra(instruction) == 31)
						OPERATE_RR("not", Rb(instruction), Rc(instruction));
					else
						OPERATE_RRR("ornot", Ra(instruction), Rb(instruction), Rc(instruction)); // ORNOT
					break;
				case 0x40: OPERATE_RRR("xor",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // XOR
				case 0x44: OPERATE_RRR("cmovlt",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVLT
				case 0x46: OPERATE_RRR("cmovge",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVGE
				case 0x48: OPERATE_RRR("eqv",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // EQV
				case 0x61: OPERATE_RR("amask",    Rb(instruction), Rc(instruction)); break; // AMASK
				case 0x64: OPERATE_RRR("cmovle",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVLE
				case 0x66: OPERATE_RRR("cmovgt",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMOVGT

				// immediate variants
				case 0x80: OPERATE_RIR("and",     Ra(instruction), Im(instruction), Rc(instruction)); break; // AND
				case 0x88: OPERATE_RIR("bic",     Ra(instruction), Im(instruction), Rc(instruction)); break; // BIC
				case 0x94: OPERATE_RIR("cmovlbs", Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVLBS
				case 0x96: OPERATE_RIR("cmovlbc", Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVLBC
				case 0xa0:
					if (Ra(instruction) == 31)
						OPERATE_IR("mov", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("bis", Ra(instruction), Im(instruction), Rc(instruction)); // BIS
					break;
				case 0xa4: OPERATE_RIR("cmoveq",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVEQ
				case 0xa6: OPERATE_RIR("cmovne",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVNE
				case 0xa8:
					if (Ra(instruction) == 31)
						OPERATE_IR("not", Im(instruction), Rc(instruction));
					else
						OPERATE_RIR("ornot", Ra(instruction), Im(instruction), Rc(instruction)); // ORNOT
					break;
				case 0xc0: OPERATE_RIR("xor",     Ra(instruction), Im(instruction), Rc(instruction)); break; // XOR
				case 0xc4: OPERATE_RIR("cmovlt",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVLT
				case 0xc6: OPERATE_RIR("cmovge",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVGE
				case 0xc8: OPERATE_RIR("eqv",     Ra(instruction), Im(instruction), Rc(instruction)); break; // EQV
				case 0xe1: OPERATE_IR("amask",    Im(instruction), Rc(instruction)); break; // AMASK
				case 0xe4: OPERATE_RIR("cmovle",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVLE
				case 0xe6: OPERATE_RIR("cmovgt",  Ra(instruction), Im(instruction), Rc(instruction)); break; // CMOVGT

				// TODO: Rb must be literal #1?
				case 0xec: OPERATE_R("implver", Rc(instruction)); break; // IMPLVER

				default: UNKNOWN("intl*"); break;
			}
			break;
		case 0x12: // INTS* (integer shift)
			switch ((instruction >> 5) & 0xff)
			{
				// register variants
				case 0x02: OPERATE_RRR("mskbl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKBL
				case 0x06: OPERATE_RRR("extbl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTBL
				case 0x0b: OPERATE_RRR("insbl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSBL
				case 0x12: OPERATE_RRR("mskwl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKWL
				case 0x16: OPERATE_RRR("extwl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTWL
				case 0x1b: OPERATE_RRR("inswl",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSWL
				case 0x22: OPERATE_RRR("mskll",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKLL
				case 0x26: OPERATE_RRR("extll",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTLL
				case 0x2b: OPERATE_RRR("insll",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSLL
				case 0x30: OPERATE_RRR("zap",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ZAP
				case 0x31: OPERATE_RRR("zapnot",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ZAPNOT
				case 0x32: OPERATE_RRR("mskql",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKQL
				case 0x34: OPERATE_RRR("srl",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SRL
				case 0x36: OPERATE_RRR("extql",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTQL
				case 0x39: OPERATE_RRR("sll",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SLL
				case 0x3b: OPERATE_RRR("insql",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSQL
				case 0x3c: OPERATE_RRR("sra",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SRA
				case 0x52: OPERATE_RRR("mskwh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKWH
				case 0x57: OPERATE_RRR("inswh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSWH
				case 0x5a: OPERATE_RRR("extwh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTWH
				case 0x62: OPERATE_RRR("msklh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKLH
				case 0x67: OPERATE_RRR("inslh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSLH
				case 0x6a: OPERATE_RRR("extlh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTLH
				case 0x72: OPERATE_RRR("mskqh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MSKQH
				case 0x77: OPERATE_RRR("insqh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // INSQH
				case 0x7a: OPERATE_RRR("extqh",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // EXTQH

				// immediate variants
				case 0x82: OPERATE_RIR("mskbl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKBL
				case 0x86: OPERATE_RIR("extbl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTBL
				case 0x8b: OPERATE_RIR("insbl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSBL
				case 0x92: OPERATE_RIR("mskwl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKWL
				case 0x96: OPERATE_RIR("extwl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTWL
				case 0x9b: OPERATE_RIR("inswl",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSWL
				case 0xa2: OPERATE_RIR("mskll",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKLL
				case 0xa6: OPERATE_RIR("extll",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTLL
				case 0xab: OPERATE_RIR("insll",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSLL
				case 0xb0: OPERATE_RIR("zap",     Ra(instruction), Im(instruction), Rc(instruction)); break; // ZAP
				case 0xb1: OPERATE_RIR("zapnot",  Ra(instruction), Im(instruction), Rc(instruction)); break; // ZAPNOT
				case 0xb2: OPERATE_RIR("mskql",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKQL
				case 0xb4: OPERATE_RIR("srl",     Ra(instruction), Im(instruction), Rc(instruction)); break; // SRL
				case 0xb6: OPERATE_RIR("extql",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTQL
				case 0xb9: OPERATE_RIR("sll",     Ra(instruction), Im(instruction), Rc(instruction)); break; // SLL
				case 0xbb: OPERATE_RIR("insql",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSQL
				case 0xbc: OPERATE_RIR("sra",     Ra(instruction), Im(instruction), Rc(instruction)); break; // SRA
				case 0xd2: OPERATE_RIR("mskwh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKWH
				case 0xd7: OPERATE_RIR("inswh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSWH
				case 0xda: OPERATE_RIR("extwh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTWH
				case 0xe2: OPERATE_RIR("msklh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKLH
				case 0xe7: OPERATE_RIR("inslh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSLH
				case 0xea: OPERATE_RIR("extlh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTLH
				case 0xf2: OPERATE_RIR("mskqh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MSKQH
				case 0xf7: OPERATE_RIR("insqh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // INSQH
				case 0xfa: OPERATE_RIR("extqh",   Ra(instruction), Im(instruction), Rc(instruction)); break; // EXTQH

				default: UNKNOWN("ints*"); break;
			}
			break;
		case 0x13: // INTM* (integer multiply)
			switch ((instruction >> 5) & 0xff)
			{
				// register variants
				case 0x00: OPERATE_RRR("mull",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULL
				case 0x20: OPERATE_RRR("mulq",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULQ
				case 0x30: OPERATE_RRR("umulh",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // UMULH
				case 0x40: OPERATE_RRR("mull/v", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULL/V
				case 0x60: OPERATE_RRR("mulq/v", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULQ/V

				// immediate variants
				case 0x80: OPERATE_RIR("mull",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MULL
				case 0xa0: OPERATE_RIR("mulq",   Ra(instruction), Im(instruction), Rc(instruction)); break; // MULQ
				case 0xb0: OPERATE_RIR("umulh",  Ra(instruction), Im(instruction), Rc(instruction)); break; // UMULH
				case 0xc0: OPERATE_RIR("mull/v", Ra(instruction), Im(instruction), Rc(instruction)); break; // MULL/V
				case 0xe0: OPERATE_RIR("mulq/v", Ra(instruction), Im(instruction), Rc(instruction)); break; // MULQ/V

				default: UNKNOWN("intm*"); break;
			}
		case 0x14: // ITFP* (integer to floating)
			switch ((instruction >> 5) & 0x7ff)
			{
				case 0x004: OPERATE_RF("itofs",      Ra(instruction), Rc(instruction)); break; // ITOFS
				case 0x00a: OPERATE_FF("sqrtf/c",    Rb(instruction), Rc(instruction)); break; // SQRTF/C
				case 0x00b: OPERATE_FF("sqrts/c",    Rb(instruction), Rc(instruction)); break; // SQRTS/C
				case 0x014: OPERATE_RF("itoff",      Ra(instruction), Rc(instruction)); break; // ITOFF
				case 0x024: OPERATE_RF("itoft",      Ra(instruction), Rc(instruction)); break; // ITOFT
				case 0x02a: OPERATE_FF("sqrtg/c",    Rb(instruction), Rc(instruction)); break; // SQRTG/C
				case 0x02b: OPERATE_FF("sqrtt/c",    Rb(instruction), Rc(instruction)); break; // SQRTT/C
				case 0x04b: OPERATE_FF("sqrts/m",    Rb(instruction), Rc(instruction)); break; // SQRTS/M
				case 0x06b: OPERATE_FF("sqrtt/m",    Rb(instruction), Rc(instruction)); break; // SQRTT/M
				case 0x08a: OPERATE_FF("sqrtf",      Rb(instruction), Rc(instruction)); break; // SQRTF
				case 0x08b: OPERATE_FF("sqrts",      Rb(instruction), Rc(instruction)); break; // SQRTS
				case 0x0aa: OPERATE_FF("sqrtg",      Rb(instruction), Rc(instruction)); break; // SQRTG
				case 0x0ab: OPERATE_FF("sqrtt",      Rb(instruction), Rc(instruction)); break; // SQRTT
				case 0x0cb: OPERATE_FF("sqrts/d",    Rb(instruction), Rc(instruction)); break; // SQRTS/D
				case 0x0eb: OPERATE_FF("sqrtt/d",    Rb(instruction), Rc(instruction)); break; // SQRTT/D
				case 0x10a: OPERATE_FF("sqrtf/uc",   Rb(instruction), Rc(instruction)); break; // SQRTF/UC
				case 0x10b: OPERATE_FF("sqrts/uc",   Rb(instruction), Rc(instruction)); break; // SQRTS/UC
				case 0x12a: OPERATE_FF("sqrtg/uc",   Rb(instruction), Rc(instruction)); break; // SQRTG/UC
				case 0x12b: OPERATE_FF("sqrtt/uc",   Rb(instruction), Rc(instruction)); break; // SQRTT/UC
				case 0x14b: OPERATE_FF("sqrts/um",   Rb(instruction), Rc(instruction)); break; // SQRTS/UM
				case 0x16b: OPERATE_FF("sqrtt/um",   Rb(instruction), Rc(instruction)); break; // SQRTT/UM
				case 0x18a: OPERATE_FF("sqrtf/u",    Rb(instruction), Rc(instruction)); break; // SQRTF/U
				case 0x1aa: OPERATE_FF("sqrtg/u",    Rb(instruction), Rc(instruction)); break; // SQRTG/U
				case 0x1ab: OPERATE_FF("sqrtt/u",    Rb(instruction), Rc(instruction)); break; // SQRTT/U
				case 0x1cb: OPERATE_FF("sqrts/ud",   Rb(instruction), Rc(instruction)); break; // SQRTS/UD
				case 0x1eb: OPERATE_FF("sqrtt/ud",   Rb(instruction), Rc(instruction)); break; // SQRTT/UD
				case 0x40a: OPERATE_FF("sqrtf/sc",   Rb(instruction), Rc(instruction)); break; // SQRTF/SC
				case 0x42a: OPERATE_FF("sqrtg/sc",   Rb(instruction), Rc(instruction)); break; // SQRTG/SC
				case 0x48a: OPERATE_FF("sqrtf/s",    Rb(instruction), Rc(instruction)); break; // SQRTF/S
				case 0x4aa: OPERATE_FF("sqrtg/s",    Rb(instruction), Rc(instruction)); break; // SQRTG/S
				case 0x50a: OPERATE_FF("sqrtf/suc",  Rb(instruction), Rc(instruction)); break; // SQRTF/SUC
				case 0x50b: OPERATE_FF("sqrts/suc",  Rb(instruction), Rc(instruction)); break; // SQRTS/SUC
				case 0x52a: OPERATE_FF("sqrtg/suc",  Rb(instruction), Rc(instruction)); break; // SQRTG/SUC
				case 0x52b: OPERATE_FF("sqrtt/suc",  Rb(instruction), Rc(instruction)); break; // SQRTT/SUC
				case 0x54b: OPERATE_FF("sqrts/sum",  Rb(instruction), Rc(instruction)); break; // SQRTS/SUM
				case 0x56b: OPERATE_FF("sqrtt/sum",  Rb(instruction), Rc(instruction)); break; // SQRTT/SUM
				case 0x58a: OPERATE_FF("sqrtf/su",   Rb(instruction), Rc(instruction)); break; // SQRTF/SU
				case 0x58b: OPERATE_FF("sqrts/su",   Rb(instruction), Rc(instruction)); break; // SQRTS/SU
				case 0x5aa: OPERATE_FF("sqrtg/su",   Rb(instruction), Rc(instruction)); break; // SQRTG/SU
				case 0x5ab: OPERATE_FF("sqrtt/su",   Rb(instruction), Rc(instruction)); break; // SQRTT/SU
				case 0x5cb: OPERATE_FF("sqrts/sud",  Rb(instruction), Rc(instruction)); break; // SQRTS/SUD
				case 0x5eb: OPERATE_FF("sqrtt/sud",  Rb(instruction), Rc(instruction)); break; // SQRTT/SUD
				case 0x70b: OPERATE_FF("sqrts/suic", Rb(instruction), Rc(instruction)); break; // SQRTS/SUIC
				case 0x72b: OPERATE_FF("sqrtt/suic", Rb(instruction), Rc(instruction)); break; // SQRTT/SUIC
				case 0x74b: OPERATE_FF("sqrts/suim", Rb(instruction), Rc(instruction)); break; // SQRTS/SUIM
				case 0x76b: OPERATE_FF("sqrtt/suim", Rb(instruction), Rc(instruction)); break; // SQRTT/SUIM
				case 0x78b: OPERATE_FF("sqrts/sui",  Rb(instruction), Rc(instruction)); break; // SQRTS/SUI
				case 0x7ab: OPERATE_FF("sqrtt/sui",  Rb(instruction), Rc(instruction)); break; // SQRTT/SUI
				case 0x7cb: OPERATE_FF("sqrts/suid", Rb(instruction), Rc(instruction)); break; // SQRTS/SUID
				case 0x7eb: OPERATE_FF("sqrtt/suid", Rb(instruction), Rc(instruction)); break; // SQRTT/SUID

				default: UNKNOWN("itfp*"); break;
			}
			break;
		case 0x15: // FLTV* (vax floating)
			switch ((instruction >> 5) & 0x7ff)
			{
				case 0x000: OPERATE_FFF("addf/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/C
				case 0x001: OPERATE_FFF("subf/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/C
				case 0x002: OPERATE_FFF("mulf/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/C
				case 0x003: OPERATE_FFF("divf/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/C
				case 0x01e: OPERATE_FF("cvtdg/c",   Rb(instruction), Rc(instruction)); break; // CVTDG/C
				case 0x020: OPERATE_FFF("addg/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/C
				case 0x021: OPERATE_FFF("subg/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/C
				case 0x022: OPERATE_FFF("mulg/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/C
				case 0x023: OPERATE_FFF("divg/c",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/C
				case 0x02c: OPERATE_FF("cvtgf/c",   Rb(instruction), Rc(instruction)); break; // CVTGF/C
				case 0x02d: OPERATE_FF("cvtgd/c",   Rb(instruction), Rc(instruction)); break; // CVTGD/C
				case 0x02f: OPERATE_FF("cvtgq/c",   Rb(instruction), Rc(instruction)); break; // CVTGQ/C
				case 0x03c: OPERATE_FF("cvtqf/c",   Rb(instruction), Rc(instruction)); break; // CVTQF/C
				case 0x03e: OPERATE_FF("cvtqg/c",   Rb(instruction), Rc(instruction)); break; // CVTQG/C
				case 0x080: OPERATE_FFF("addf",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF
				case 0x081:
					if (Ra(instruction) == 31)
						OPERATE_FF("negf", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subf", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBF
					break;
				case 0x082: OPERATE_FFF("mulf",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF
				case 0x083: OPERATE_FFF("divf",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF
				case 0x09e: OPERATE_FF("cvtdg",     Rb(instruction), Rc(instruction)); break; // CVTDG
				case 0x0a0: OPERATE_FFF("addg",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG
				case 0x0a1:
					if (Ra(instruction) == 31)
						OPERATE_FF("negg", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subg", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBG
					break;
				case 0x0a2: OPERATE_FFF("mulg",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG
				case 0x0a3: OPERATE_FFF("divg",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG
				case 0x0a5: OPERATE_FFF("cmpgeq",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGEQ
				case 0x0a6: OPERATE_FFF("cmpglt",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGLT
				case 0x0a7: OPERATE_FFF("cmpgle",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGLE
				case 0x0ac: OPERATE_FF("cvtgf",     Rb(instruction), Rc(instruction)); break; // CVTGF
				case 0x0ad: OPERATE_FF("cvtgd",     Rb(instruction), Rc(instruction)); break; // CVTGD
				case 0x0af: OPERATE_FF("cvtgq",     Rb(instruction), Rc(instruction)); break; // CVTGQ
				case 0x0bc: OPERATE_FF("cvtqf",     Rb(instruction), Rc(instruction)); break; // CVTQF
				case 0x0be: OPERATE_FF("cvtqg",     Rb(instruction), Rc(instruction)); break; // CVTQG
				case 0x100: OPERATE_FFF("addf/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/UC
				case 0x101: OPERATE_FFF("subf/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/UC
				case 0x102: OPERATE_FFF("mulf/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/UC
				case 0x103: OPERATE_FFF("divf/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/UC
				case 0x11e: OPERATE_FF("cvtdg/uc",  Rb(instruction), Rc(instruction)); break; // CVTDG/UC
				case 0x120: OPERATE_FFF("addg/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/UC
				case 0x121: OPERATE_FFF("subg/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/UC
				case 0x122: OPERATE_FFF("mulg/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/UC
				case 0x123: OPERATE_FFF("divg/uc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/UC
				case 0x12c: OPERATE_FF("cvtgf/uc",  Rb(instruction), Rc(instruction)); break; // CVTGF/UC
				case 0x12d: OPERATE_FF("cvtgd/uc",  Rb(instruction), Rc(instruction)); break; // CVTGD/UC
				case 0x12f: OPERATE_FF("cvtgq/vc",  Rb(instruction), Rc(instruction)); break; // CVTGQ/VC
				case 0x180: OPERATE_FFF("addf/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/U
				case 0x181: OPERATE_FFF("subf/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/U
				case 0x182: OPERATE_FFF("mulf/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/U
				case 0x183: OPERATE_FFF("divf/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/U
				case 0x19e: OPERATE_FF("cvtdg/u",   Rb(instruction), Rc(instruction)); break; // CVTDG/U
				case 0x1a0: OPERATE_FFF("addg/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/U
				case 0x1a1: OPERATE_FFF("subg/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/U
				case 0x1a2: OPERATE_FFF("mulg/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/U
				case 0x1a3: OPERATE_FFF("divg/u",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/U
				case 0x1ac: OPERATE_FF("cvtgf/u",   Rb(instruction), Rc(instruction)); break; // CVTGF/U
				case 0x1ad: OPERATE_FF("cvtgd/u",   Rb(instruction), Rc(instruction)); break; // CVTGD/U
				case 0x1af: OPERATE_FF("cvtgq/v",   Rb(instruction), Rc(instruction)); break; // CVTGQ/V
				case 0x400: OPERATE_FFF("addf/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/SC
				case 0x401: OPERATE_FFF("subf/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/SC
				case 0x402: OPERATE_FFF("mulf/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/SC
				case 0x403: OPERATE_FFF("divf/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/SC
				case 0x41e: OPERATE_FF("cvtdg/sc",  Rb(instruction), Rc(instruction)); break; // CVTDG/SC
				case 0x420: OPERATE_FFF("addg/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/SC
				case 0x421: OPERATE_FFF("subg/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/SC
				case 0x422: OPERATE_FFF("mulg/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/SC
				case 0x423: OPERATE_FFF("divg/sc",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/SC
				case 0x42c: OPERATE_FF("cvtgf/sc",  Rb(instruction), Rc(instruction)); break; // CVTGF/SC
				case 0x42d: OPERATE_FF("cvtgd/sc",  Rb(instruction), Rc(instruction)); break; // CVTGD/SC
				case 0x42f: OPERATE_FF("cvtgq/sc",  Rb(instruction), Rc(instruction)); break; // CVTGQ/SC
				case 0x480: OPERATE_FFF("addf/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/S
				case 0x481:
					if (Ra(instruction) == 31)
						OPERATE_FF("negf/s", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subf/s", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBF/S
					break;
				case 0x482: OPERATE_FFF("mulf/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/S
				case 0x483: OPERATE_FFF("divf/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/S
				case 0x49e: OPERATE_FF("cvtdg/s",   Rb(instruction), Rc(instruction)); break; // CVTDG/S
				case 0x4a0: OPERATE_FFF("addg/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/S
				case 0x4a1:
					if (Ra(instruction) == 31)
						OPERATE_FF("negg/s", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subg/s", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBG/S
					break;
				case 0x4a2: OPERATE_FFF("mulg/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/S
				case 0x4a3: OPERATE_FFF("divg/s",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/S
				case 0x4a5: OPERATE_FFF("cmpgeq/s", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGEQ/S
				case 0x4a6: OPERATE_FFF("cmpglt/s", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGLT/S
				case 0x4a7: OPERATE_FFF("cmpgle/s", Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPGLE/S
				case 0x4ac: OPERATE_FF("cvtgf/s",   Rb(instruction), Rc(instruction)); break; // CVTGF/S
				case 0x4ad: OPERATE_FF("cvtgd/s",   Rb(instruction), Rc(instruction)); break; // CVTGD/S
				case 0x4af: OPERATE_FF("cvtgq/s",   Rb(instruction), Rc(instruction)); break; // CVTGQ/S
				case 0x500: OPERATE_FFF("addf/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/SUC
				case 0x501: OPERATE_FFF("subf/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/SUC
				case 0x502: OPERATE_FFF("mulf/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/SUC
				case 0x503: OPERATE_FFF("divf/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/SUC
				case 0x51e: OPERATE_FF("cvtdg/suc", Rb(instruction), Rc(instruction)); break; // CVTDG/SUC
				case 0x520: OPERATE_FFF("addg/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/SUC
				case 0x521: OPERATE_FFF("subg/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/SUC
				case 0x522: OPERATE_FFF("mulg/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/SUC
				case 0x523: OPERATE_FFF("divg/suc", Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/SUC
				case 0x52c: OPERATE_FF("cvtgf/suc", Rb(instruction), Rc(instruction)); break; // CVTGF/SUC
				case 0x52d: OPERATE_FF("cvtgd/suc", Rb(instruction), Rc(instruction)); break; // CVTGD/SUC
				case 0x52f: OPERATE_FF("cvtgq/svc", Rb(instruction), Rc(instruction)); break; // CVTGQ/SVC
				case 0x580: OPERATE_FFF("addf/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDF/SU
				case 0x581: OPERATE_FFF("subf/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBF/SU
				case 0x582: OPERATE_FFF("mulf/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULF/SU
				case 0x583: OPERATE_FFF("divf/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVF/SU
				case 0x59e: OPERATE_FF("cvtdg/su",  Rb(instruction), Rc(instruction)); break; // CVTDG/SU
				case 0x5a0: OPERATE_FFF("addg/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDG/SU
				case 0x5a1: OPERATE_FFF("subg/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBG/SU
				case 0x5a2: OPERATE_FFF("mulg/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULG/SU
				case 0x5a3: OPERATE_FFF("divg/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVG/SU
				case 0x5ac: OPERATE_FF("cvtgf/su",  Rb(instruction), Rc(instruction)); break; // CVTGF/SU
				case 0x5ad: OPERATE_FF("cvtgd/su",  Rb(instruction), Rc(instruction)); break; // CVTGD/SU
				case 0x5af: OPERATE_FF("cvtgq/sv",  Rb(instruction), Rc(instruction)); break; // CVTGQ/SV

				default: UNKNOWN("fltv*"); break;
			}
			break;
		case 0x16: // FLTI* (ieee floating)
			switch ((instruction >> 5) & 0x7ff)
			{
				case 0x000: OPERATE_FFF("adds/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/C
				case 0x001: OPERATE_FFF("subs/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/C
				case 0x002: OPERATE_FFF("muls/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/C
				case 0x003: OPERATE_FFF("divs/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/C
				case 0x020: OPERATE_FFF("addt/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/C
				case 0x021: OPERATE_FFF("subt/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/C
				case 0x022: OPERATE_FFF("mult/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/C
				case 0x023: OPERATE_FFF("divt/c",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/C
				case 0x02c: OPERATE_FF("cvttts/c",    Rb(instruction), Rc(instruction)); break; // CVTTS/C
				case 0x02f: OPERATE_FF("cvttq/c",     Rb(instruction), Rc(instruction)); break; // CVTTQ/C
				case 0x03c: OPERATE_FF("cvtqs/c",     Rb(instruction), Rc(instruction)); break; // CVTQS/C
				case 0x03e: OPERATE_FF("cvtqt/c",     Rb(instruction), Rc(instruction)); break; // CVTQT/C
				case 0x040: OPERATE_FFF("adds/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/M
				case 0x041: OPERATE_FFF("subs/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/M
				case 0x042: OPERATE_FFF("muls/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/M
				case 0x043: OPERATE_FFF("divs/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/M
				case 0x060: OPERATE_FFF("addt/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/M
				case 0x061: OPERATE_FFF("subt/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/M
				case 0x062: OPERATE_FFF("mult/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/M
				case 0x063: OPERATE_FFF("divt/m",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/M
				case 0x06c: OPERATE_FF("cvttts/m",    Rb(instruction), Rc(instruction)); break; // CVTTS/M
				case 0x06f: OPERATE_FF("cvttq/m",     Rb(instruction), Rc(instruction)); break; // CVTTQ/M
				case 0x07c: OPERATE_FF("cvtqs/m",     Rb(instruction), Rc(instruction)); break; // CVTQS/M
				case 0x07e: OPERATE_FF("cvtqt/m",     Rb(instruction), Rc(instruction)); break; // CVTQT/M
				case 0x080: OPERATE_FFF("adds",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS
				case 0x081:
					if (Ra(instruction) == 31)
						OPERATE_FF("negs", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subs", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBS
					break;
				case 0x082: OPERATE_FFF("muls",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS
				case 0x083: OPERATE_FFF("divs",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS
				case 0x0a0: OPERATE_FFF("addt",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT
				case 0x0a1:
					if (Ra(instruction) == 31)
						OPERATE_FF("negt", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subt", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBT
					break;
				case 0x0a2: OPERATE_FFF("mult",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT
				case 0x0a3: OPERATE_FFF("divt",       Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT
				case 0x0a4: OPERATE_FFF("cmptun",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTUN
				case 0x0a5: OPERATE_FFF("cmpteq",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTEQ
				case 0x0a6: OPERATE_FFF("cmptlt",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTLT
				case 0x0a7: OPERATE_FFF("cmptle",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTLE
				case 0x0ac: OPERATE_FF("cvttts",      Rb(instruction), Rc(instruction)); break; // CVTTS
				case 0x0af: OPERATE_FF("cvttq",       Rb(instruction), Rc(instruction)); break; // CVTTQ
				case 0x0bc: OPERATE_FF("cvtqs",       Rb(instruction), Rc(instruction)); break; // CVTQS
				case 0x0be: OPERATE_FF("cvtqt",       Rb(instruction), Rc(instruction)); break; // CVTQT
				case 0x0c0: OPERATE_FFF("adds/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/D
				case 0x0c1: OPERATE_FFF("subs/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/D
				case 0x0c2: OPERATE_FFF("muls/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/D
				case 0x0c3: OPERATE_FFF("divs/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/D
				case 0x0e0: OPERATE_FFF("addt/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/D
				case 0x0e1: OPERATE_FFF("subt/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/D
				case 0x0e2: OPERATE_FFF("mult/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/D
				case 0x0e3: OPERATE_FFF("divt/d",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/D
				case 0x0ec: OPERATE_FF("cvttts/d",    Rb(instruction), Rc(instruction)); break; // CVTTS/D
				case 0x0ef: OPERATE_FF("cvttq/d",     Rb(instruction), Rc(instruction)); break; // CVTTQ/D
				case 0x0fc: OPERATE_FF("cvtqs/d",     Rb(instruction), Rc(instruction)); break; // CVTQS/D
				case 0x0fe: OPERATE_FF("cvtqt/d",     Rb(instruction), Rc(instruction)); break; // CVTQT/D
				case 0x100: OPERATE_FFF("adds/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/UC
				case 0x101: OPERATE_FFF("subs/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/UC
				case 0x102: OPERATE_FFF("muls/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/UC
				case 0x103: OPERATE_FFF("divs/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/UC
				case 0x120: OPERATE_FFF("addt/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/UC
				case 0x121: OPERATE_FFF("subt/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/UC
				case 0x122: OPERATE_FFF("mult/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/UC
				case 0x123: OPERATE_FFF("divt/uc",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/UC
				case 0x12c: OPERATE_FF("cvttts/uc",   Rb(instruction), Rc(instruction)); break; // CVTTS/UC
				case 0x12f: OPERATE_FF("cvttq/vc",    Rb(instruction), Rc(instruction)); break; // CVTTQ/VC
				case 0x140: OPERATE_FFF("adds/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/UM
				case 0x141: OPERATE_FFF("subs/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/UM
				case 0x142: OPERATE_FFF("muls/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/UM
				case 0x143: OPERATE_FFF("divs/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/UM
				case 0x160: OPERATE_FFF("addt/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/UM
				case 0x161: OPERATE_FFF("subt/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/UM
				case 0x162: OPERATE_FFF("mult/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/UM
				case 0x163: OPERATE_FFF("divt/um",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/UM
				case 0x16c: OPERATE_FF("cvttts/um",   Rb(instruction), Rc(instruction)); break; // CVTTS/UM
				case 0x16f: OPERATE_FF("cvttq/vm",    Rb(instruction), Rc(instruction)); break; // CVTTQ/VM
				case 0x180: OPERATE_FFF("adds/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/U
				case 0x181: OPERATE_FFF("subs/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/U
				case 0x182: OPERATE_FFF("muls/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/U
				case 0x183: OPERATE_FFF("divs/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/U
				case 0x1a0: OPERATE_FFF("addt/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/U
				case 0x1a1: OPERATE_FFF("subt/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/U
				case 0x1a2: OPERATE_FFF("mult/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/U
				case 0x1a3: OPERATE_FFF("divt/u",     Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/U
				case 0x1ac: OPERATE_FF("cvttts/u",    Rb(instruction), Rc(instruction)); break; // CVTTS/U
				case 0x1af: OPERATE_FF("cvttq/v",     Rb(instruction), Rc(instruction)); break; // CVTTQ/V
				case 0x1c0: OPERATE_FFF("adds/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/UD
				case 0x1c1: OPERATE_FFF("subs/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/UD
				case 0x1c2: OPERATE_FFF("muls/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/UD
				case 0x1c3: OPERATE_FFF("divs/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/UD
				case 0x1e0: OPERATE_FFF("addt/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/UD
				case 0x1e1: OPERATE_FFF("subs/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/UD
				case 0x1e2: OPERATE_FFF("mult/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/UD
				case 0x1e3: OPERATE_FFF("divt/ud",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/UD
				case 0x1ec: OPERATE_FF("cvttts/ud",   Rb(instruction), Rc(instruction)); break; // CVTTS/UD
				case 0x1ef: OPERATE_FF("cvttq/vd",    Rb(instruction), Rc(instruction)); break; // CVTTQ/VD
				case 0x2ac: OPERATE_FF("cvtst",       Rb(instruction), Rc(instruction)); break; // CVTST
				case 0x500: OPERATE_FFF("adds/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUC
				case 0x501: OPERATE_FFF("subs/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUC
				case 0x502: OPERATE_FFF("muls/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUC
				case 0x503: OPERATE_FFF("divs/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUC
				case 0x520: OPERATE_FFF("addt/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUC
				case 0x521: OPERATE_FFF("subt/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUC
				case 0x522: OPERATE_FFF("mult/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUC
				case 0x523: OPERATE_FFF("divt/suc",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUC
				case 0x52c: OPERATE_FF("cvttts/suc",  Rb(instruction), Rc(instruction)); break; // CVTTS/SUC
				case 0x52f: OPERATE_FF("cvttq/svc",   Rb(instruction), Rc(instruction)); break; // CVTTQ/SVC
				case 0x540: OPERATE_FFF("adds/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUM
				case 0x541: OPERATE_FFF("subs/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUM
				case 0x542: OPERATE_FFF("muls/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUM
				case 0x543: OPERATE_FFF("divs/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUM
				case 0x560: OPERATE_FFF("addt/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUM
				case 0x561: OPERATE_FFF("subt/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUM
				case 0x562: OPERATE_FFF("mult/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUM
				case 0x563: OPERATE_FFF("divt/sum",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUM
				case 0x56c: OPERATE_FF("cvttts/sum",  Rb(instruction), Rc(instruction)); break; // CVTTS/SUM
				case 0x56f: OPERATE_FF("cvttq/svm",   Rb(instruction), Rc(instruction)); break; // CVTTQ/SVM
				case 0x580: OPERATE_FFF("adds/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SU
				case 0x581:
					if (Ra(instruction) == 31)
						OPERATE_FF("negs/su", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subs/su", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBS/SU
					break;
				case 0x582: OPERATE_FFF("muls/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SU
				case 0x583: OPERATE_FFF("divs/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SU
				case 0x5a0: OPERATE_FFF("addt/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SU
				case 0x5a1:
					if (Ra(instruction) == 31)
						OPERATE_FF("negt/su", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subt/su", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBT/SU
					break;
				case 0x5a2: OPERATE_FFF("mult/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SU
				case 0x5a3: OPERATE_FFF("divt/su",    Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SU
				case 0x5a4: OPERATE_FFF("cmptun/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTUN/SU
				case 0x5a5: OPERATE_FFF("cmpteq/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTEQ/SU
				case 0x5a6: OPERATE_FFF("cmptlt/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTLT/SU
				case 0x5a7: OPERATE_FFF("cmptle/su",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // CMPTLE/SU
				case 0x5ac: OPERATE_FF("cvttts/su",   Rb(instruction), Rc(instruction)); break; // CVTTS/SU
				case 0x5af: OPERATE_FF("cvttq/sv",    Rb(instruction), Rc(instruction)); break; // CVTTQ/SV
				case 0x5c0: OPERATE_FFF("adds/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUD
				case 0x5c1: OPERATE_FFF("subs/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUD
				case 0x5c2: OPERATE_FFF("muls/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUD
				case 0x5c3: OPERATE_FFF("divs/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUD
				case 0x5e0: OPERATE_FFF("addt/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUD
				case 0x5e1: OPERATE_FFF("subt/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUD
				case 0x5e2: OPERATE_FFF("mult/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUD
				case 0x5e3: OPERATE_FFF("divt/sud",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUD
				case 0x5ec: OPERATE_FF("cvttts/sud",  Rb(instruction), Rc(instruction)); break; // CVTTS/SUD
				case 0x5ef: OPERATE_FF("cvttq/svd",   Rb(instruction), Rc(instruction)); break; // CVTTQ/SVD
				case 0x6ac: OPERATE_FF("cvtst/s",     Rb(instruction), Rc(instruction)); break; // CVTST/S
				case 0x700: OPERATE_FFF("adds/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUIC
				case 0x701: OPERATE_FFF("subs/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUIC
				case 0x702: OPERATE_FFF("muls/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUIC
				case 0x703: OPERATE_FFF("divs/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUIC
				case 0x720: OPERATE_FFF("addt/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUIC
				case 0x721: OPERATE_FFF("subt/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUIC
				case 0x722: OPERATE_FFF("mult/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUIC
				case 0x723: OPERATE_FFF("divt/suic",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUIC
				case 0x72c: OPERATE_FF("cvttts/suic", Rb(instruction), Rc(instruction)); break; // CVTTS/SUIC
				case 0x72f: OPERATE_FF("cvttq/svic",  Rb(instruction), Rc(instruction)); break; // CVTTQ/SVIC
				case 0x73c: OPERATE_FF("cvtqs/suic",  Rb(instruction), Rc(instruction)); break; // CVTQS/SUIC
				case 0x73e: OPERATE_FF("cvtqt/suic",  Rb(instruction), Rc(instruction)); break; // CVTQT/SUIC
				case 0x740: OPERATE_FFF("adds/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUIM
				case 0x741: OPERATE_FFF("subs/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUIM
				case 0x742: OPERATE_FFF("muls/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUIM
				case 0x743: OPERATE_FFF("divs/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUIM
				case 0x760: OPERATE_FFF("addt/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUIM
				case 0x761: OPERATE_FFF("subt/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUIM
				case 0x762: OPERATE_FFF("mult/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUIM
				case 0x763: OPERATE_FFF("divt/suim",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUIM
				case 0x76c: OPERATE_FF("cvttts/suim", Rb(instruction), Rc(instruction)); break; // CVTTS/SUIM
				case 0x76f: OPERATE_FF("cvttq/svim",  Rb(instruction), Rc(instruction)); break; // CVTTQ/SVIM
				case 0x77c: OPERATE_FF("cvtqs/suim",  Rb(instruction), Rc(instruction)); break; // CVTQS/SUIM
				case 0x77e: OPERATE_FF("cvtqt/suim",  Rb(instruction), Rc(instruction)); break; // CVTQT/SUIM
				case 0x780: OPERATE_FFF("adds/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUI
				case 0x781:
					if (Ra(instruction) == 31)
						OPERATE_FF("negs/sui", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subs/sui", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBS/SUI
					break;
				case 0x782: OPERATE_FFF("muls/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUI
				case 0x783: OPERATE_FFF("divs/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUI
				case 0x7a0: OPERATE_FFF("addt/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUI
				case 0x7a1:
					if (Ra(instruction) == 31)
						OPERATE_FF("negt/sui", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("subt/sui", Ra(instruction), Rb(instruction), Rc(instruction)); // SUBT/SUI
					break;
				case 0x7a2: OPERATE_FFF("mult/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUI
				case 0x7a3: OPERATE_FFF("divt/sui",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUI
				case 0x7ac: OPERATE_FF("cvttts/sui",  Rb(instruction), Rc(instruction)); break; // CVTTS/SUI
				case 0x7af: OPERATE_FF("cvttq/svi",   Rb(instruction), Rc(instruction)); break; // CVTTQ/SVI
				case 0x7bc: OPERATE_FF("cvtqs/sui",   Rb(instruction), Rc(instruction)); break; // CVTQS/SUI
				case 0x7be: OPERATE_FF("cvtqt/sui",   Rb(instruction), Rc(instruction)); break; // CVTQT/SUI
				case 0x7c0: OPERATE_FFF("adds/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDS/SUID
				case 0x7c1: OPERATE_FFF("subs/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBS/SUID
				case 0x7c2: OPERATE_FFF("muls/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULS/SUID
				case 0x7c3: OPERATE_FFF("divs/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVS/SUID
				case 0x7e0: OPERATE_FFF("addt/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // ADDT/SUID
				case 0x7e1: OPERATE_FFF("subt/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // SUBT/SUID
				case 0x7e2: OPERATE_FFF("mult/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // MULT/SUID
				case 0x7e3: OPERATE_FFF("divt/suid",  Ra(instruction), Rb(instruction), Rc(instruction)); break; // DIVT/SUID
				case 0x7ec: OPERATE_FF("cvttts/suid", Rb(instruction), Rc(instruction)); break; // CVTTS/SUID
				case 0x7ef: OPERATE_FF("cvttq/svid",  Rb(instruction), Rc(instruction)); break; // CVTTQ/SVID
				case 0x7fc: OPERATE_FF("cvtqs/suid",  Rb(instruction), Rc(instruction)); break; // CVTQS/SUID
				case 0x7fe: OPERATE_FF("cvtqt/suid",  Rb(instruction), Rc(instruction)); break; // CVTQT/SUID

				default: UNKNOWN("flti*"); break;
			}
			break;
		case 0x17: // FLTL* (floating)
			switch ((instruction >> 5) & 0x7ff)
			{
				case 0x010: OPERATE_FF("cvtlq",    Rb(instruction), Rc(instruction)); break; // CVTLQ
				case 0x020:
					if (Ra(instruction) == 31 && Rb(instruction) == 31 && Rc(instruction) == 31)
						MISC("fnop");
					else if (Ra(instruction) == 31 && Rb(instruction) == 31)
						OPERATE_F("fclr", Rc(instruction));
					else if (Ra(instruction) == 31)
						OPERATE_FF("fabs", Rb(instruction), Rc(instruction));
					else if (Ra(instruction) == Rb(instruction))
						OPERATE_FF("fmov", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("cpys", Ra(instruction), Rb(instruction), Rc(instruction)); // CPYS
					break;
				case 0x021:
					if (Ra(instruction) == Rb(instruction))
						OPERATE_FF("fneg", Rb(instruction), Rc(instruction));
					else
						OPERATE_FFF("cpysn", Ra(instruction), Rb(instruction), Rc(instruction)); // CPYSN
					break;
				case 0x022: OPERATE_FFF("cpyse",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // CPYSE
				case 0x024:
					if (Ra(instruction) == Rb(instruction) && Rb(instruction) == Rc(instruction))
						OPERATE_F("mt_fpcr", Rc(instruction));
					else
						OPERATE_FFF("mt_fpcr", Ra(instruction), Rb(instruction), Rc(instruction)); // MT_FPCR
					break;
				case 0x025:
					if (Ra(instruction) == Rb(instruction) && Rb(instruction) == Rc(instruction))
						OPERATE_F("mf_fpcr", Rc(instruction));
					else
						OPERATE_FFF("mf_fpcr", Ra(instruction), Rb(instruction), Rc(instruction)); // MF_FPCR
					break;
				case 0x02a: OPERATE_FFF("fcmoveq", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVEQ
				case 0x02b: OPERATE_FFF("fcmovne", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVNE
				case 0x02c: OPERATE_FFF("fcmovlt", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVLT
				case 0x02d: OPERATE_FFF("fcmovge", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVGE
				case 0x02e: OPERATE_FFF("fcmovle", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVLE
				case 0x02f: OPERATE_FFF("fcmovgt", Ra(instruction), Rb(instruction), Rc(instruction)); break; // FCMOVGT
				case 0x030: OPERATE_FF("cvtql",    Rb(instruction), Rc(instruction)); break; // CVTQL
				case 0x130: OPERATE_FF("cvtql/v",  Rb(instruction), Rc(instruction)); break; // CVTQL/V
				case 0x530: OPERATE_FF("cvtql/sv", Rb(instruction), Rc(instruction)); break; // CVTQL/SV

				default: UNKNOWN("fltl*"); break;
			}
			break;
		case 0x18: // MISC* (miscellaneous)
			switch (u16(instruction))
			{
				case 0x0000: MISC("trapb");    break; // TRAPB
				case 0x0400: MISC("excb");     break; // EXCB
				case 0x4000: MISC("mb");       break; // MB
				case 0x4400: MISC("wmb");      break; // WMB
				case 0x8000: MISC_M("fetch",   Rb(instruction)); break; // FETCH
				case 0xa000: MISC_M("fetch_m", Rb(instruction)); break; // FETCH_M
				case 0xc000: MISC_R("rpcc",    Ra(instruction)); break; // RPCC
				case 0xe000: MISC_R("rc",      Ra(instruction)); break; // RC
				case 0xe800: MISC_M("ecb",     Rb(instruction)); break; // ECB
				case 0xf000: MISC_R("rs",      Ra(instruction)); break; // RS
				case 0xf800: MISC_M("wh64",    Rb(instruction)); break; // WH64

				default: UNKNOWN("misc*"); break;
			}
			break;
		case 0x19: // PAL19
			switch ((instruction >> 5) & 0x7)
			{
			case 0x0: MISC("nop"); break;
			case 0x1: OPERATE_RI("hw_mfpr/i",     Rb(instruction), Rc(instruction)); break;
			case 0x2: OPERATE_RA("hw_mfpr/a",     Rb(instruction), Rc(instruction)); break;
			case 0x3: OPERATE_RAI("hw_mfpr/ai",   Rb(instruction), Rc(instruction)); break;
			case 0x4: OPERATE_RP("hw_mfpr/p",     Rb(instruction), Rc(instruction)); break;
			case 0x5: OPERATE_RPI("hw_mfpr/pi",   Rb(instruction), Rc(instruction)); break;
			case 0x6: OPERATE_RPA("hw_mfpr/pa",   Rb(instruction), Rc(instruction)); break;
			case 0x7: OPERATE_RPAI("hw_mfpr/pai", Rb(instruction), Rc(instruction)); break;
			}
			break;

		case 0x1a: // JSR* (jump)
			switch ((instruction >> 14) & 3)
			{
				case 0: JUMP("jmp",   Ra(instruction), Rb(instruction)); break; // JMP
				case 1: JUMP("jsr",   Ra(instruction), Rb(instruction)); flags |= STEP_OVER; break; // JSR
				case 2: JUMP("ret",   Ra(instruction), Rb(instruction)); flags |= STEP_OUT; break; // RET
				case 3: JUMP("jsr_c", Ra(instruction), Rb(instruction)); break; // JSR_COROUTINE
			}
			break;
		case 0x1b: // PAL1B
			switch ((instruction >> 12) & 0xf)
			{
			case 0x0: MEMORY_R("hw_ldl",     Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x1: MEMORY_R("hw_ldq",     Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x2: MEMORY_R("hw_ldl/r",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x3: MEMORY_R("hw_ldq/r",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x4: MEMORY_R("hw_ldl/a",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x5: MEMORY_R("hw_ldq/a",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x6: MEMORY_R("hw_ldl/ar",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x7: MEMORY_R("hw_ldq/ar",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x8: MEMORY_R("hw_ldl/p",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x9: MEMORY_R("hw_ldq/p",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xa: MEMORY_R("hw_ldl/pr",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xb: MEMORY_R("hw_ldq/pr",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xc: MEMORY_R("hw_ldl/pa",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xd: MEMORY_R("hw_ldq/pa",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xe: MEMORY_R("hw_ldl/par", Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xf: MEMORY_R("hw_ldq/par", Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			}
			break;
		case 0x1c: // FPTI* (floating to integer)
			switch ((instruction >> 5) & 0xff)
			{
				// register variants
				case 0x00: OPERATE_RR("sextb",   Rb(instruction), Rc(instruction)); break; // SEXTB (BWX)
				case 0x01: OPERATE_RR("sextw",   Rb(instruction), Rc(instruction)); break; // SEXTW (BWX)
				case 0x30: OPERATE_RR("ctpop",   Rb(instruction), Rc(instruction)); break; // CTPOP (CIX)
				case 0x31: OPERATE_RRR("perr",   Ra(instruction), Rb(instruction), Rc(instruction)); break; // PERR (MVI)
				case 0x32: OPERATE_RR("ctlz",    Rb(instruction), Rc(instruction)); break; // CTLZ (CIX)
				case 0x33: OPERATE_RR("cttz",    Rb(instruction), Rc(instruction)); break; // CTTZ (CIX)
				case 0x34: OPERATE_RR("unpkbw",  Rb(instruction), Rc(instruction)); break; // UNPKBW (MVI)
				case 0x35: OPERATE_RR("unpkbl",  Rb(instruction), Rc(instruction)); break; // UNPKBL (MVI)
				case 0x36: OPERATE_RR("pkwb",    Rb(instruction), Rc(instruction)); break; // PKWB (MVI)
				case 0x37: OPERATE_RR("pklb",    Rb(instruction), Rc(instruction)); break; // PKLB (MVI)
				case 0x38: OPERATE_RRR("minsb8", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MINSB8 (MVI)
				case 0x39: OPERATE_RRR("minsw4", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MINSW4 (MVI)
				case 0x3a: OPERATE_RRR("minub8", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MINUB8 (MVI)
				case 0x3b: OPERATE_RRR("minuw4", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MINUW4 (MVI)
				case 0x3c: OPERATE_RRR("maxub8", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MAXUB8 (MVI)
				case 0x3d: OPERATE_RRR("maxuw4", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MAXUW4 (MVI)
				case 0x3e: OPERATE_RRR("maxsb8", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MAXSB8 (MVI)
				case 0x3f: OPERATE_RRR("maxsw4", Ra(instruction), Rb(instruction), Rc(instruction)); break; // MAXSW4 (MVI)
				case 0x70: OPERATE_FR("ftoit",   Ra(instruction), Rc(instruction)); break; // FTOIT (FIX)
				case 0x78: OPERATE_FR("ftois",   Ra(instruction), Rc(instruction)); break; // FTOIS (FIX)

				// immediate variants
				case 0x80: OPERATE_IR("sextb",   Im(instruction), Rc(instruction)); break; // SEXTB (BWX)
				case 0x81: OPERATE_IR("sextw",   Im(instruction), Rc(instruction)); break; // SEXTW (BWX)
				case 0xb8: OPERATE_RIR("minsb8", Ra(instruction), Im(instruction), Rc(instruction)); break; // MINSB8 (MVI)
				case 0xb9: OPERATE_RIR("minsw4", Ra(instruction), Im(instruction), Rc(instruction)); break; // MINSW4 (MVI)
				case 0xba: OPERATE_RIR("minub8", Ra(instruction), Im(instruction), Rc(instruction)); break; // MINUB8 (MVI)
				case 0xbb: OPERATE_RIR("minuw4", Ra(instruction), Im(instruction), Rc(instruction)); break; // MINUW4 (MVI)
				case 0xbc: OPERATE_RIR("maxub8", Ra(instruction), Im(instruction), Rc(instruction)); break; // MAXUB8 (MVI)
				case 0xbd: OPERATE_RIR("maxuw4", Ra(instruction), Im(instruction), Rc(instruction)); break; // MAXUW4 (MVI)
				case 0xbe: OPERATE_RIR("maxsb8", Ra(instruction), Im(instruction), Rc(instruction)); break; // MAXSB8 (MVI)
				case 0xbf: OPERATE_RIR("maxsw4", Ra(instruction), Im(instruction), Rc(instruction)); break; // MAXSW4 (MVI)

				default: UNKNOWN("fpti*"); break;
			}
			break;
		case 0x1d: // PAL1D
			switch ((instruction >> 5) & 0x7)
			{
			case 0x0: MISC("nop"); break;
			case 0x1: OPERATE_RI("hw_mtpr/i",     Rb(instruction), Rc(instruction)); break;
			case 0x2: OPERATE_RA("hw_mtpr/a",     Rb(instruction), Rc(instruction)); break;
			case 0x3: OPERATE_RAI("hw_mtpr/ai",   Rb(instruction), Rc(instruction)); break;
			case 0x4: OPERATE_RP("hw_mtpr/p",     Rb(instruction), Rc(instruction)); break;
			case 0x5: OPERATE_RPI("hw_mtpr/pi",   Rb(instruction), Rc(instruction)); break;
			case 0x6: OPERATE_RPA("hw_mtpr/pa",   Rb(instruction), Rc(instruction)); break;
			case 0x7: OPERATE_RPAI("hw_mtpr/pai", Rb(instruction), Rc(instruction)); break;
			}
			break;
		case 0x1e: // PAL1E
			switch (instruction & 0x03ffffff)
			{
			case 0x03ff8000: MISC("hw_rei"); break;

			default: UNKNOWN("pal1e"); break;
			}
			break;
		case 0x1f: // PAL1F
			switch ((instruction >> 12) & 0xf)
			{
			case 0x0: MEMORY_R("hw_stl",     Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x1: MEMORY_R("hw_stq",     Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x2: MEMORY_R("hw_stl/r",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x3: MEMORY_R("hw_stq/r",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x4: MEMORY_R("hw_stl/a",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x5: MEMORY_R("hw_stq/a",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x6: MEMORY_R("hw_stl/ar",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x7: MEMORY_R("hw_stq/ar",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x8: MEMORY_R("hw_stl/p",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0x9: MEMORY_R("hw_stq/p",   Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xa: MEMORY_R("hw_stl/pr",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xb: MEMORY_R("hw_stq/pr",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xc: MEMORY_R("hw_stl/pa",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xd: MEMORY_R("hw_stq/pa",  Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xe: MEMORY_R("hw_stl/par", Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			case 0xf: MEMORY_R("hw_stq/par", Ra(instruction), Disp_P(instruction), Rb(instruction)); break;
			}
			break;
			// memory format
		case 0x20: MEMORY_F("ldf",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDF
		case 0x21: MEMORY_F("ldg",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDG
		case 0x22: MEMORY_F("lds",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDS
		case 0x23: MEMORY_F("ldt",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDT
		case 0x24: MEMORY_F("stf",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STF
		case 0x25: MEMORY_F("stg",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STG
		case 0x26: MEMORY_F("sts",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STS
		case 0x27: MEMORY_F("stt",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STT
		case 0x28: MEMORY_R("ldl",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDL
		case 0x29: MEMORY_R("ldq",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDQ
		case 0x2a: MEMORY_R("ldl_l", Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDL_L
		case 0x2b: MEMORY_R("ldq_l", Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // LDQ_L
		case 0x2c: MEMORY_R("stl",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STL
		case 0x2d: MEMORY_R("stq",   Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STQ
		case 0x2e: MEMORY_R("stl_c", Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STL_C
		case 0x2f: MEMORY_R("stq_c", Ra(instruction), Disp_M(instruction), Rb(instruction)); break; // STQ_C

			// branch format
		case 0x30:
			if (Ra(instruction) == 31)
				BRANCH("br", Disp_B(instruction));
			else
				BRANCH_R("br", Ra(instruction), Disp_B(instruction)); // BR
			break;
		case 0x31: BRANCH_F("fbeq", Ra(instruction), Disp_B(instruction)); break; // FBEQ
		case 0x32: BRANCH_F("fblt", Ra(instruction), Disp_B(instruction)); break; // FBLT
		case 0x33: BRANCH_F("fble", Ra(instruction), Disp_B(instruction)); break; // FBLE
		case 0x34: BRANCH_R("bsr",  Ra(instruction), Disp_B(instruction)); break; // BSR
		case 0x35: BRANCH_F("fbne", Ra(instruction), Disp_B(instruction)); break; // FBNE
		case 0x36: BRANCH_F("fbge", Ra(instruction), Disp_B(instruction)); break; // FBGE
		case 0x37: BRANCH_F("fbgt", Ra(instruction), Disp_B(instruction)); break; // FBGT
		case 0x38: BRANCH_R("blbc", Ra(instruction), Disp_B(instruction)); break; // BLBC
		case 0x39: BRANCH_R("beq",  Ra(instruction), Disp_B(instruction)); break; // BEQ
		case 0x3a: BRANCH_R("blt",  Ra(instruction), Disp_B(instruction)); break; // BLT
		case 0x3b: BRANCH_R("ble",  Ra(instruction), Disp_B(instruction)); break; // BLE
		case 0x3c: BRANCH_R("blbs", Ra(instruction), Disp_B(instruction)); break; // BLBS
		case 0x3d: BRANCH_R("bne",  Ra(instruction), Disp_B(instruction)); break; // BNE
		case 0x3e: BRANCH_R("bge",  Ra(instruction), Disp_B(instruction)); break; // BGE
		case 0x3f: BRANCH_R("bgt",  Ra(instruction), Disp_B(instruction)); break; // BGT
	}

	return bytes | flags;
}
