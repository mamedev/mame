/*
 * Note: Original Java source written by:
 *
 * Barry Silverman mailto:barry@disus.com or mailto:bss@media.mit.edu
 * Vadim Gerasimov mailto:vadim@media.mit.edu
 *
 * Basically, it has been rewritten entirely in order to perform cycle-level simulation
 * (with only a few flip-flops being set one cycle too early or too late).  I don't know if
 * it is a good thing or a bad thing (it makes emulation more accurate, but slower, and
 * code is more complex and less readable), but it appears to be the only way we could emulate
 * mid-instruction sequence break.  And it enables us to emulate the control panel fairly
 * accurately.
 *
 * Additionnally, IOT functions have been modified to be external: IOT callback pointers are set
 * at emulation initiation, and most IOT callback functions are part of the machine emulation.
 *
 *
 * for the runnable java applet, with applet and Spacewar! source, go to:
 * http://lcs.www.media.mit.edu/groups/el/projects/spacewar/
 *
 * for a complete html version of the pdp1 handbook go to:
 * http://www.dbit.com/~greeng3/pdp1/index.html
 *
 * there is another java simulator (by the same people) which runs the
 * original pdp1 LISP interpreter, go to:
 * http://lcs.www.media.mit.edu/groups/el/projects/pdp1
 *
 * Another PDP1 emulator (or simulator) is at:
 * ftp://minnie.cs.adfa.oz.au/pub/PDP-11/Sims/Supnik_2.3
 * It seems to emulate pdp1 I/O more accurately than we do.
 * However, there is no CRT emulation.
 *
 * and finally, there is a nice article about SPACEWAR!, go to:
 * http://ars-www.uchicago.edu/~eric/lore/spacewar/spacewar.html
 *
 * some extra documentation is available on spies:
 * http://www.spies.com/~aek/pdf/dec/pdp1/
 * The file "F17_PDP1Maint.pdf" explains operation procedures and much of the internals of pdp-1.
 * It was the main reference for this emulator.
 * The file "F25_PDP1_IO.pdf" has interesting information on the I/O system, too.
 *
 * Following is an extract from the handbook:
 *
 * INTRODUCTION
 *
 * The Programmed Data Processor (PDP-1) is a high speed, solid state digital computer designed to
 * operate with many types of input-output devices with no internal machine changes. It is a single
 * address, single instruction, stored program computer with powerful program features. Five-megacycle
 * circuits, a magnetic core memory and fully parallel processing make possible a computation rate of
 * 100,000 additions per second. The PDP-1 is unusually versatile. It is easy to install, operate and
 * maintain. Conventional 110-volt power is used, neither air conditioning nor floor reinforcement is
 * necessary, and preventive maintenance is provided for by built-in marginal checking circuits.
 *
 * PDP-1 circuits are based on the designs of DEC's highly successful and reliable System Modules.
 * Flip-flops and most switches use saturating transistors. Primary active elements are
 * Micro-Alloy-Diffused transistors.
 *
 * The entire computer occupies only 17 square feet of floor space. It consists of four equipment frames,
 * one of which is used as the operating station.
 *
 * CENTRAL PROCESSOR
 *
 * The Central Processor contains the control, arithmetic and memory addressing elements, and the memory
 * buffer register. The word length is 18 binary digits. Instructions are performed in multiples of the
 * memory cycle time of five microseconds. Add, subtract, deposit, and load, for example, are two-cycle
 * instructions requiring 10 microseconds. Multiplication requires and average of 20 microseconds.
 * Program features include: single address instructions, multiple step indirect addressing and logical
 * arithmetic commands. Console features include: flip-flop indicators grouped for convenient octal
 * reading, six program flags for automatic setting and computer sensing, and six sense switches for
 * manual setting and computer sensing.
 *
 * MEMORY SYSTEM
 *
 * The coincident-current, magnetic core memory of a standard PDP-1 holds 4096 words of 18 bits each.
 * Memory capacity may be readily expanded, in increments of 4096 words, to a maximum of 65,536 words.
 * The read-rewrite time of the memory is five microseconds, the basic computer rate. Driving currents
 * are automatically adjusted to compensate for temperature variations between 50 and 110 degrees
 * Fahrenheit. The core memory storage may be supplemented by up to 24 magnetic tape transports.
 *
 * INPUT-OUTPUT
 *
 * PDP-1 is designed to operate a variety of buffered input-output devices. Standard equipment consistes
 * of a perforated tape reader with a read speed of 400 lines per second, and alphanuermic typewriter for
 * on-line operation in both input and output, and a perforated tape punch (alphanumeric or binary) with
 * a speed of 63 lines per second. A variety of optional equipment is available, including the following:
 *
 *     Precision CRT Display Type 30
 *     Ultra-Precision CRT Display Type 31
 *     Symbol Generator Type 33
 *     Light Pen Type 32
 *     Oscilloscope Display Type 34
 *     Card Punch Control Type 40-1
 *     Card Reader and Control Type 421
 *     Magnetic Tape Transport Type 50
 *     Programmed Magnetic Tape Control Type 51
 *     Automatic Magnetic Tape Control Type 52
 *     Automatic Magnetic Tape Control Type 510
 *     Parallel Drum Type 23
 *     Automatic Line Printer and Control Type 64
 *     18-bit Real Time Clock
 *     18-bit Output Relay Buffer Type 140
 *     Multiplexed A-D Converter Type 138/139
 *
 * All in-out operations are performed through the In-Out Register or through the high speed input-output
 * channels.
 *
 * The PDP-1 is also available with the optional Sequence Break System. This is a multi-channel priority
 * interrupt feature which permits concurrent operation of several in-out devices. A one-channel Sequence
 * Break System is included in the standard PDP-1. Optional Sequence Break Systems consist of 16, 32, 64,
 * 128, and 256 channels.
 *
 * ...
 *
 * BASIC INSTRUCTIONS
 *
 *                                                                    OPER. TIME
 * INSTRUCTION  CODE #  EXPLANATION                                     (usec)
 * ------------------------------------------------------------------------------
 * add Y        40      Add C(Y) to C(AC)                                 10
 * and Y        02      Logical AND C(Y) with C(AC)                       10
 * cal Y        16      Equals jda 100                                    10
 * dac Y        24      Deposit C(AC) in Y                                10
 * dap Y        26      Deposit contents of address part of AC in Y       10
 * dio Y        32      Deposit C(IO) in Y                                10
 * dip Y        30      Deposit contents of instruction part of AC in Y   10
 * div Y        56      Divide                                          40 max
 * dzm Y        34      Deposit zero in Y                                 10
 * idx Y        44      Index (add one) C(Y), leave in Y & AC             10
 * ior Y        04      Inclusive OR C(Y) with C(AC)                      10
 * iot Y        72      In-out transfer, see below
 * isp Y        46      Index and skip if result is positive              10
 * jda Y        17      Equals dac Y and jsp Y+1                          10
 * jmp Y        60      Take next instruction from Y                      5
 * jsp Y        62      Jump to Y and save program counter in AC          5
 * lac Y        20      Load the AC with C(Y)                             10
 * law N        70      Load the AC with the number N                     5
 * law-N        71      Load the AC with the number -N                    5
 * lio Y        22      Load IO with C(Y)                                 10
 * mul Y        54      Multiply                                        25 max
 * opr          76      Operate, see below                                5
 * sad Y        50      Skip next instruction if C(AC) <> C(Y)            10
 * sas Y        52      Skip next instruction if C(AC) = C(Y)             10
 * sft          66      Shift, see below                                  5
 * skp          64      Skip, see below                                   5
 * sub Y        42      Subtract C(Y) from C(AC)                          10
 * xct Y        10      Execute instruction in Y                          5+
 * xor Y        06      Exclusive OR C(Y) with C(AC)                      10
 *
 * OPERATE GROUP
 *
 *                                                                    OPER. TIME
 * INSTRUCTION  CODE #   EXPLANATION                                    (usec)
 * ------------------------------------------------------------------------------
 * cla        760200     Clear AC                                         5
 * clf        76000f     Clear selected Program Flag (f = flag #)         5
 * cli        764000     Clear IO                                         5
 * cma        761000     Complement AC                                    5
 * hlt        760400     Halt                                             5
 * lap        760100     Load AC with Program Counter                     5
 * lat        762200     Load AC from Test Word switches                  5
 * nop        760000     No operation                                     5
 * stf        76001f     Set selected Program Flag                        5
 *
 * IN-OUT TRANSFER GROUP
 *
 * PERFORATED TAPE READER
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * rpa        720001     Read Perforated Tape Alphanumeric
 * rpb        720002     Read Perforated Tape Binary
 * rrb        720030     Read Reader Buffer
 *
 * PERFORATED TAPE PUNCH
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * ppa        720005     Punch Perforated Tape Alphanumeric
 * ppb        720006     Punch Perforated Tape Binary
 *
 * ALPHANUMERIC ON-LINE TYPEWRITER
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * tyo        720003     Type Out
 * tyi        720004     Type In
 *
 * SEQUENCE BREAK SYSTEM TYPE 120
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * esm        720055     Enter Sequence Break Mode
 * lsm        720054     Leave Sequence Break Mode
 * cbs        720056     Clear Sequence Break System
 * dsc        72kn50     Deactivate Sequence Break Channel
 * asc        72kn51     Activate Sequence Break Channel
 * isb        72kn52     Initiate Sequence Break
 * cac        720053     Clear All Channels
 *
 * HIGH SPEED DATA CONTROL TYPE 131
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * swc        72x046     Set Word Counter
 * sia        720346     Set Location Counter
 * sdf        720146     Stop Data Flow
 * rlc        720366     Read Location Counter
 * shr        720446     Set High Speed Channel Request
 *
 * PRECISION CRT DISPLAY TYPE 30
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * dpy        720007     Display One Point
 *
 * SYMBOL GENERATOR TYPE 33
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * gpl        722027     Generator Plot Left
 * gpr        720027     Generator Plot Right
 * glf        722026     Load Format
 * gsp        720026     Space
 * sdb        722007     Load Buffer, No Intensity
 *
 * ULTRA-PRECISION CRT DISPLAY TYPE 31
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * dpp        720407     Display One Point on Ultra Precision CRT
 *
 * CARD PUNCH CONTROL TYPE 40-1
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * lag        720044     Load a Group
 * pac        720043     Punch a Card
 *
 * CARD READER TYPE 421
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * rac        720041     Read Card Alpha
 * rbc        720042     Read Card Binary
 * rcc        720032     Read Card Column
 *
 * PROGRAMMED MAGNETIC TAPE CONTROL TYPE 51
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * msm        720073     Select Mode
 * mcs        720034     Check Status
 * mcb        720070     Clear Buffer
 * mwc        720071     Write a Character
 * mrc        720072     Read Character
 *
 * AUTOMATIC MAGNETIC TAPE CONTROL TYPE 52
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * muf        72ue76     Tape Unit and FinalT
 * mic        72ue75     Initial and Command
 * mrf        72u067     Reset Final
 * mri        72ug66     Reset Initial
 * mes        72u035     Examine States
 * mel        72u036     Examine Location
 * inr        72ur67     Initiate a High Speed Channel Request
 * ccr        72s067     Clear Command Register
 *
 * AUTOMATIC MAGNETIC TAPE CONTROL TYPE 510
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * sfc        720072     Skip if Tape Control Free
 * rsr        720172     Read State Register
 * crf        720272     Clear End-of-Record Flip-Flop
 * cpm        720472     Clear Proceed Mode
 * dur        72xx70     Load Density, Unit, Rewind
 * mtf        73xx71     Load Tape Function Register
 * cgo        720073     Clear Go
 *
 * MULTIPLEXED A-D CONVERTER TYPE 138/139
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * rcb        720031     Read Converter Buffer
 * cad        720040     Convert a Voltage
 * scv        72mm47     Select Multiplexer (1 of 64 Channels)
 * icv        720060     Index Multiplexer
 *
 * AUTOMATIC LINE PRINTER TYPE 64
 *
 * INSTRUCTION  CODE #   EXPLANATION
 * ------------------------------------------------------------------------------
 * clrbuf     722045     Clear Buffer
 * lpb        720045     Load Printer Buffer
 * pas        721x45     Print and Space
 *
 * SKIP GROUP
 *
 *                                                                    OPER. TIME
 * INSTRUCTION  CODE #   EXPLANATION                                    (usec)
 * ------------------------------------------------------------------------------
 * sma        640400     Dkip on minus AC                                 5
 * spa        640200     Skip on plus AC                                  5
 * spi        642000     Skip on plus IO                                  5
 * sza        640100     Skip on ZERO (+0) AC                             5
 * szf        6400f      Skip on ZERO flag                                5
 * szo        641000     Skip on ZERO overflow (and clear overflow)       5
 * szs        6400s0     Skip on ZERO sense switch                        5
 *
 * SHIFT/ROTATE GROUP
 *
 *                                                                      OPER. TIME
 * INSTRUCTION  CODE #   EXPLANATION                                      (usec)
 * ------------------------------------------------------------------------------
 *   ral        661      Rotate AC left                                     5
 *   rar        671      Rotate AC right                                    5
 *   rcl        663      Rotate Combined AC & IO left                       5
 *   rcr        673      Rotate Combined AC & IO right                      5
 *   ril        662      Rotate IO left                                     5
 *   rir        672      Rotate IO right                                    5
 *   sal        665      Shift AC left                                      5
 *   sar        675      Shift AC right                                     5
 *   scl        667      Shift Combined AC & IO left                        5
 *   scr        677      Shift Combined AC & IO right                       5
 *   sil        666      Shift IO left                                      5
 *   sir        676      Shift IO right                                     5
 */


/*
    TODO:
    * support other extensions as time permits
*/


#include "cpuintrf.h"
#include "debugger.h"
#include "pdp1.h"

#define LOG 0
#define LOG_EXTRA 0

#ifndef LOG_IOT_EXTRA
#define LOG_IOT_EXTRA 0
#endif


static void execute_instruction(void);
static void null_iot (int op2, int nac, int mb, int *io, int ac);
static void lem_eem_iot(int op2, int nac, int mb, int *io, int ac);
static void sbs_iot(int op2, int nac, int mb, int *io, int ac);
static void type_20_sbs_iot(int op2, int nac, int mb, int *io, int ac);
static void pulse_start_clear(void);


/* PDP1 Registers */
typedef struct
{
	/* processor registers */
	UINT32 pc;		/* program counter (12, 15 or 16 bits) */
	int ir;			/* basic operation code of current instruction (5 bits) */
	int mb;			/* memory buffer (used for holding the current instruction only) (18 bits) */
	int ma;			/* memory address (12, 15 or 16 bits) */
	int ac;			/* accumulator (18 bits) */
	int io;			/* i/o register (18 bits) */
	int pf;			/* program flag register (6 bits) */

	/* operator panel switches */
	int ta;			/* current state of the 12 or 16 address switches */
	int tw;			/* current state of the 18 test word switches */
	int ss;			/* current state of the 6 sense switches on the operator panel (6 bits) */
	unsigned int sngl_step : 1;	/* stop every memory cycle */
	unsigned int sngl_inst : 1;	/* stop every instruction */
	unsigned int extend_sw : 1;	/* extend switch (loaded into the extend flip-flop on start/read-in) */

	/* processor state flip-flops */
	unsigned int run : 1;		/* processor is running */
	unsigned int cycle : 1;		/* processor is in the midst of an instruction */
	unsigned int defer : 1;		/* processor is handling deferred (i.e. indirect) addressing */
	unsigned int brk_ctr : 2;	/* break counter */
	unsigned int ov;			/* overflow flip-flop */
	unsigned int rim : 1;		/* processor is in read-in mode */

	unsigned int sbm : 1;		/* processor is in sequence break mode (i.e. interrupts are enabled) */

	unsigned int exd : 1;		/* extend mode: processor is in extend mode */
	unsigned int exc : 1;		/* extend-mode cycle: current instruction cycle is done in extend mode */
	unsigned int ioc : 1;		/* i-o commands: seems to be equivalent to (! ioh) */
	unsigned int ioh : 1;		/* i-o halt: processor is executing an Input-Output Transfer wait */
	unsigned int ios : 1;		/* i-o synchronizer: set on i-o operation completion */

	/* sequence break system */
	unsigned int irq_state : 16;	/* mirrors the state of the interrupt pins */
	unsigned int b1 : 16;			/* interrupt enable */
	unsigned int b2 : 16;			/* interrupt pulse request pending - asynchronous with computer operation (set by pulses on irq_state, cleared when interrupt is taken) */
	/*unsigned int b3 : 16;*/			/* interrupt request pending - synchronous with computer operation (logical or of irq_state and b2???) */
	unsigned int b4 : 16;			/* interrupt in progress */

	/* additional emulator state variables */
	int rim_step;			/* current step in rim execution */
	int sbs_request;		/* interrupt request (i.e. (b3 & (~ b4)) && (! sbm)) */
	int sbs_level;			/* interrupt request level (first bit in (b3 & (~ b4)) */
	int sbs_restore;		/* set when a jump instruction is an interrupt return */
	int no_sequence_break;	/* disable sequence break recognition for one cycle */

	/* callback for the iot instruction */
	void (*extern_iot[64])(int op2, int nac, int mb, int *io, int ac);
	/* read a word from the perforated tape reader */
	void (*read_binary_word)(void);
	/* called when sc is pulsed: IO devices should reset */
	void (*io_sc_callback)(void);

	/* 0: no extend support, 1: extend with 15-bit address, 2: extend with 16-bit address */
	int extend_support;

	int extended_address_mask;	/* 07777 with no extend support, 077777 or 0177777 with extend support */
	int address_extension_mask;	/* 00000 with no extend support, 070000 or 0170000 with extend support */

	/* 1 to use hardware multiply/divide (MUL, DIV) instead of MUS, DIS */
	int hw_mul_div;

	/* 1 for 16-line sequence break system, 0 for default break system */
	int type_20_sbs;
}
pdp1_Regs;


static pdp1_Regs pdp1;

#define PC		pdp1.pc
#define IR		pdp1.ir
#define MB		pdp1.mb
#define MA		pdp1.ma
#define AC		pdp1.ac
#define IO		pdp1.io
#define OV		pdp1.ov
#define EXD		pdp1.exd
/* note that we start counting flags/sense switches at 1, therefore n is in [1,6] */
#define FLAGS	pdp1.pf
#define READFLAG(n)	((pdp1.pf >> (6-(n))) & 1)
#define WRITEFLAG(n, data)	(pdp1.pf = (pdp1.pf & ~(1 << (6-(n)))) | (((data) & 1) << (6-(n))))
#define SENSE_SW	pdp1.ss
#define READSENSE(n)	((pdp1.ss >> (6-(n))) & 1)
#define WRITESENSE(n, data)	(pdp1.ss = (pdp1.ss & ~(1 << (6-(n)))) | (((data) & 1) << (6-(n))))

#define EXTENDED_ADDRESS_MASK	pdp1.extended_address_mask
#define ADDRESS_EXTENSION_MASK	pdp1.address_extension_mask
#define BASE_ADDRESS_MASK		0007777

#define INCREMENT_PC	(PC = (PC & ADDRESS_EXTENSION_MASK) | ((PC+1) & BASE_ADDRESS_MASK))
#define DECREMENT_PC	(PC = (PC & ADDRESS_EXTENSION_MASK) | ((PC-1) & BASE_ADDRESS_MASK))
#define INCREMENT_MA	(MA = (MA & ADDRESS_EXTENSION_MASK) | ((MA+1) & BASE_ADDRESS_MASK))
#define PREVIOUS_PC		((PC & ADDRESS_EXTENSION_MASK) | ((PC-1) & BASE_ADDRESS_MASK))

/* public globals */
static signed int pdp1_ICount;



/*
    Interrupts are called "sequence break" in pdp1, but the general idea is the same.

    There are several interrupt lines.  With the standard sequence break system, all lines
    are logically or'ed to trigger a single interrupt level.  Interrupts can be triggered
    by either a pulse or a level on the interrupt lines.  With the optional type 120 sequence
    break system, each of 16 lines triggers is wired to a different priority level: additionnally,
    each interrupt line can be masked out, and interrupt can be triggered through software.

    Also, instructions can be interrupted in the middle of execution.  This is done by
    decrementing the PC register: therefore the instruction is re-executed from start.

    Interrupt routines should not execute most IOT, as the interrupt may interrupt another.

    More details can be found in the handbook and the maintenance manual.
*/
/*
    This function MUST be called every time pdp1.sbm, pdp1.b4, pdp1.irq_state or pdp1.b2 change.
*/
static void field_interrupt(void)
{
	/* current_irq: 1 bit for each active pending interrupt request
    Pending interrupts are in b3 (simulated by (pdp1.irq_state & pdp1.b1) | pdp1.b2)), but they
    are only honored if no higher priority interrupt routine is in execution (one bit set in b4
    for each routine in execution).  The revelant mask is created with (pdp1.b4 | (- pdp1.b4)),
    as the carry chain (remember that -b4 = (~ b4) + 1) does precisely what we want.
    b4:    0001001001000
    -b4:   1110110111000
    b4|-b4:1111111111000
    Neat, uh?
     */
	int current_irq = ((pdp1.irq_state & pdp1.b1) | pdp1.b2) & ~ (pdp1.b4 | (- pdp1.b4));
	int i;

	if (pdp1.sbm && current_irq)
	{
		pdp1.sbs_request = 1;
		for (i=0; /*i<16 &&*/ (! ((current_irq >> i) & 1)); i++)
			;
		pdp1.sbs_level = i;
	}
	else
		pdp1.sbs_request = 0;
}

static void pdp1_set_irq_line (int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* no specific NMI line */
	}
	else if ((irqline >= 0) && (irqline < (pdp1.type_20_sbs ? 1 : 16)))
	{
		unsigned int new_state = state ? 1 : 0;

		if (((pdp1.irq_state >> irqline) & 1) != new_state)
		{
			pdp1.irq_state = (pdp1.irq_state & ~ (1 << irqline)) | (new_state << irqline);

			if ((new_state) && ((pdp1.b1 >> irqline) & 1))
				pdp1.b2 |= (new_state << irqline);

			/*pdp1.b3 = pdp1.irq_state | pdp1.b2;*/

			field_interrupt();  /* interrupt state has changed */
		}
	}
}


static void pdp1_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const pdp1_reset_param_t *param = config;
	int i;

	/* clean-up */
	memset (&pdp1, 0, sizeof (pdp1));

	/* set up params and callbacks */
	for (i=0; i<64; i++)
	{
		pdp1.extern_iot[i] = (param && param->extern_iot[i])
										? param->extern_iot[i]
										: null_iot;
	}
	pdp1.read_binary_word = (param) ? param->read_binary_word : NULL;
	pdp1.io_sc_callback = (param) ? param->io_sc_callback : NULL;
	pdp1.extend_support = (param) ? param->extend_support : 0;
	pdp1.hw_mul_div = (param) ? param->hw_mul_div : 0;
	pdp1.type_20_sbs = (param) ? param->type_20_sbs : 0;

	switch (pdp1.extend_support)
	{
	default:
		pdp1.extend_support = 0;
	case 0:		/* no extension */
		pdp1.extended_address_mask = 07777;
		pdp1.address_extension_mask = 00000;
		break;
	case 1:		/* 15-bit extension */
		pdp1.extended_address_mask = 077777;
		pdp1.address_extension_mask = 070000;
		break;
	case 2:		/* 16-bit extension */
		pdp1.extended_address_mask = 0177777;
		pdp1.address_extension_mask = 0170000;
		break;
	}

	if (pdp1.extend_support)
	{
		pdp1.extern_iot[074] = lem_eem_iot;
	}
	pdp1.extern_iot[054] = pdp1.extern_iot[055] = pdp1.extern_iot[056] = sbs_iot;
	if (pdp1.type_20_sbs)
	{
		pdp1.extern_iot[050] = pdp1.extern_iot[051] = pdp1.extern_iot[052] = pdp1.extern_iot[053]
				= type_20_sbs_iot;
	}

	/* reset CPU flip-flops */
	pulse_start_clear();
}

static void pdp1_reset (void)
{
	/* nothing to do */
}

static void pdp1_get_context (void *dst)
{
	if (dst)
		*(pdp1_Regs *) dst = pdp1;
}

static void pdp1_set_context (void *src)
{
	if (src)
		pdp1 = *(pdp1_Regs *) src;
}


/*
    flags:
    * 1 for each instruction which supports indirect addressing (memory reference instructions,
      except cal and jda, and with the addition of jmp and jsp)
    * 2 for memory reference instructions
*/
static const char instruction_kind[32] =
{
/*      and ior xor xct         cal/jda */
	0,	3,	3,	3,	3,	0,	0,	2,
/*  lac lio dac dap dip dio dzm     */
	3,	3,	3,	3,	3,	3,	3,	0,
/*  add sub idx isp sad sas mus dis */
	3,	3,	3,	3,	3,	3,	3,	3,
/*  jmp jsp skp sft law iot     opr */
	1,	1,	0,	0,	0,	0,	0,	0
};


/* execute instructions on this CPU until icount expires */
static int pdp1_execute(int cycles)
{
	pdp1_ICount = cycles;

	do
	{
		CALL_MAME_DEBUG;


		/* ioh should be cleared at the end of the instruction cycle, and ios at the
        start of next instruction cycle, but who cares? */
		if (pdp1.ioh && pdp1.ios)
		{
			pdp1.ioh = 0;
			pdp1.ios = 0;
		}


		if ((! pdp1.run) && (! pdp1.rim))
			pdp1_ICount = 0;	/* if processor is stopped, just burn cycles */
		else if (pdp1.rim)
		{
			switch (pdp1.rim_step)
			{
			case 0:
				/* read first word as instruction */
				if (pdp1.read_binary_word)
					(*pdp1.read_binary_word)();		/* data will be transferred to IO register */
				pdp1.rim_step = 1;
				break;

			case 1:
				if (! pdp1.ios)
				{	/* transfer incomplete: wait some more */
					pdp1_ICount = 0;
				}
				else
				{	/* data transfer complete */
					pdp1.ios = 0;

					MB = IO;
					IR = MB >> 13;		/* basic opcode */
					if (IR == JMP)		/* jmp instruction ? */
					{
						PC = (MA & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);
						pdp1.rim = 0;	/* exit read-in mode */
						pdp1.run = 1;
						pdp1.rim_step = 0;
					}
					else if ((IR == DIO) || (IR == DAC))	/* dio or dac instruction ? */
					{	/* there is a discrepancy: the pdp1 handbook tells that only dio should be used,
                        but the lisp tape uses the dac instruction instead */
						/* Yet maintainance manual p. 6-25 states clearly that the data is located
                        in IO and transfered to MB, so DAC is likely to be a mistake. */
						pdp1.rim_step = 2;
					}
					else
					{
						/* what the heck? */
						if (LOG)
							logerror("It seems this tape should not be operated in read-in mode\n");

						pdp1.rim = 0;		/* exit read-in mode (right???) */
						pdp1.rim_step = 0;
					}
				}
				break;

			case 2:
				/* read second word as data */
				if (pdp1.read_binary_word)
					(*pdp1.read_binary_word)();		/* data will be transferred to IO register */
				pdp1.rim_step = 3;
				break;

			case 3:
				if (! pdp1.ios)
				{	/* transfer incomplete: wait some more */
					pdp1_ICount = 0;
				}
				else
				{	/* data transfer complete */
					pdp1.ios = 0;

					MA = (PC & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);

					MB = IO;
					WRITE_PDP_18BIT(MA, MB);

					pdp1.rim_step = 0;
				}
				break;
			}
		}
		else
		{
			/* yes, interrupt can occur in the midst of an instruction (impressing, huh?) */
			/* Note that break cannot occur during a one-cycle jump that is deferred only once,
            or another break cycle.  Also, it cannot interrupt the long cycle 1 of automatic
            multiply/divide.  (maintainance manual 6-19) */
			if (pdp1.sbs_request && (! pdp1.no_sequence_break) && (! pdp1.brk_ctr))
			{	/* begin sequence break */
				pdp1.brk_ctr = 1;
			}
			if (pdp1.brk_ctr)
			{	/* sequence break in progress */
				switch (pdp1.brk_ctr)
				{
				case 1:
					if (pdp1.cycle)
						DECREMENT_PC;	/* set PC to point to aborted instruction, so that it can be re-run */

					pdp1.b4 |= (1 << pdp1.sbs_level);	/* set "interrupt in progress" flag */
					pdp1.b2 &= ~(1 << pdp1.sbs_level);	/* clear interrupt request */
					field_interrupt();
					MA = pdp1.sbs_level << 2;	/* always 0 with standard sequence break system */
					MB = AC;			/* save AC to MB */
					AC = (OV << 17) | (EXD << 16) | PC;	/* save OV/EXD/PC to AC */
					EXD = OV = 0;		/* according to maintainance manual p. 8-17 and ?-?? */
					pdp1.cycle = pdp1.defer = pdp1.exc = 0;	/* mere guess */
					WRITE_PDP_18BIT(MA, MB);	/* save former AC to memory */
					INCREMENT_MA;
					pdp1_ICount -= 5;
					pdp1.brk_ctr++;
					break;

				case 2:
					WRITE_PDP_18BIT(MA, MB = AC);	/* save former OV/EXD/PC to memory */
					INCREMENT_MA;
					pdp1_ICount -= 5;
					pdp1.brk_ctr++;
					break;

				case 3:
					WRITE_PDP_18BIT(MA, MB = IO);	/* save IO to memory */
					INCREMENT_MA;
					PC = MA;
					pdp1_ICount -= 5;
					pdp1.brk_ctr = 0;
					break;
				}
			}
			else
			{
				if (pdp1.no_sequence_break)
					pdp1.no_sequence_break = 0;

				if (! pdp1.cycle)
				{	/* no instruction in progress: time to fetch a new instruction, I guess */
					MB = READ_PDP_18BIT(MA = PC);
					INCREMENT_PC;
					IR = MB >> 13;		/* basic opcode */

					if ((instruction_kind[IR] & 1) && (MB & 010000))
					{
						pdp1.defer = 1;
						pdp1.cycle = 1;			/* instruction shall be executed later */

						/* detect deferred one-cycle jumps */
						if ((IR == JMP) || (IR == JSP))
						{
							pdp1.no_sequence_break = 1;
							/* detect JMP *(4*n+1) to memory module 0 if in sequence break mode */
							if (((MB & 0777703) == 0610001) && (pdp1.sbm) && ! (MA & 0170000))
							{
								int level = (MB & 0000074) >> 2;

								if ((pdp1.type_20_sbs) || (level == 0))
								{
									pdp1.b4 &= ~(1 << level);
									field_interrupt();
									if (pdp1.extend_support)
										EXD = 1;	/* according to maintainance manual p. 6-33 */
									pdp1.sbs_restore = 1;
								}
							}
						}
					}
					else if (instruction_kind[IR] & 2)
						pdp1.cycle = 1;			/* instruction shall be executed later */
					else
						execute_instruction();	/* execute instruction at once */

					pdp1_ICount -= 5;
				}
				else if (pdp1.defer)
				{	/* defer cycle : handle indirect addressing */
					MA = (PC & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);

					MB = READ_PDP_18BIT(MA);

					/* determinate new value of pdp1.defer */
					if (EXD)
					{
						pdp1.defer = 0;
						pdp1.exc = 1;
					}
					else
						pdp1.defer = (MB & 010000) ? 1 : 0;

					/* execute JMP and JSP immediately if applicable */
					if ((! pdp1.defer) && (! (instruction_kind[IR] & 2)))
					{
						execute_instruction();	/* execute instruction at once */
						/*pdp1.cycle = 0;*/
						pdp1.exc = 0;

						if (pdp1.sbs_restore)
						{	/* interrupt return: according to maintainance manual p. 6-33 */
							if (pdp1.extend_support)
								EXD = (MB >> 16) & 1;
							OV = (MB >> 17) & 1;
							pdp1.sbs_restore = 0;
						}
					}

					pdp1_ICount -= 5;
				}
				else
				{	/* memory reference instruction in cycle 1 */
					if (pdp1.exc)
					{
						MA = MB & EXTENDED_ADDRESS_MASK;
						pdp1.exc = 0;
					}
					else
						MA = (PC & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);

					execute_instruction();	/* execute instruction */

					pdp1_ICount -= 5;
				}

				if ((pdp1.sngl_inst) && (! pdp1.cycle))
					pdp1.run = 0;
			}
			if (pdp1.sngl_step)
				pdp1.run = 0;
		}
	}
	while (pdp1_ICount > 0);

	return cycles - pdp1_ICount;
}


static void pdp1_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:
	case CPUINFO_INT_INPUT_STATE + 5:
	case CPUINFO_INT_INPUT_STATE + 6:
	case CPUINFO_INT_INPUT_STATE + 7:
	case CPUINFO_INT_INPUT_STATE + 8:
	case CPUINFO_INT_INPUT_STATE + 9:
	case CPUINFO_INT_INPUT_STATE + 10:
	case CPUINFO_INT_INPUT_STATE + 11:
	case CPUINFO_INT_INPUT_STATE + 12:
	case CPUINFO_INT_INPUT_STATE + 13:
	case CPUINFO_INT_INPUT_STATE + 14:
	case CPUINFO_INT_INPUT_STATE + 15:			pdp1_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i); break;

	case CPUINFO_INT_SP:						(void) info->i;	/* no SP */					break;
	case CPUINFO_INT_PC:
	case CPUINFO_INT_REGISTER + PDP1_PC:		PC = info->i & EXTENDED_ADDRESS_MASK;		break;
	case CPUINFO_INT_REGISTER + PDP1_IR:		IR = info->i & 037; /* weird idea */		break;
	case CPUINFO_INT_REGISTER + PDP1_MB:		MB = info->i & 0777777;						break;
	case CPUINFO_INT_REGISTER + PDP1_MA:		MA = info->i & EXTENDED_ADDRESS_MASK;		break;
	case CPUINFO_INT_REGISTER + PDP1_AC:		AC = info->i & 0777777;						break;
	case CPUINFO_INT_REGISTER + PDP1_IO:		IO = info->i & 0777777;						break;
	case CPUINFO_INT_REGISTER + PDP1_OV:		OV = info->i ? 1 : 0;						break;
	case CPUINFO_INT_REGISTER + PDP1_PF:		FLAGS = info->i & 077;						break;
	case CPUINFO_INT_REGISTER + PDP1_PF1:		WRITEFLAG(1, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_PF2:		WRITEFLAG(2, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_PF3:		WRITEFLAG(3, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_PF4:		WRITEFLAG(4, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_PF5:		WRITEFLAG(5, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_PF6:		WRITEFLAG(6, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_TA:		pdp1.ta = info->i & 0177777 /*07777 with simpler control panel*/; break;
	case CPUINFO_INT_REGISTER + PDP1_TW:		pdp1.tw = info->i & 0777777;				break;
	case CPUINFO_INT_REGISTER + PDP1_SS:		SENSE_SW = info->i & 077;					break;
	case CPUINFO_INT_REGISTER + PDP1_SS1:		WRITESENSE(1, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SS2:		WRITESENSE(2, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SS3:		WRITESENSE(3, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SS4:		WRITESENSE(4, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SS5:		WRITESENSE(5, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SS6:		WRITESENSE(6, info->i ? 1 : 0);				break;
	case CPUINFO_INT_REGISTER + PDP1_SNGL_STEP:	pdp1.sngl_step = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + PDP1_SNGL_INST:	pdp1.sngl_inst = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + PDP1_EXTEND_SW:	pdp1.extend_sw = info->i ? 1 : 0;			break;
	case CPUINFO_INT_REGISTER + PDP1_RUN:		pdp1.run = info->i ? 1 : 0;					break;
#if LOG
	case CPUINFO_INT_REGISTER + PDP1_CYC:		logerror("pdp1_set_reg to cycle flip-flop ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + PDP1_DEFER:		logerror("pdp1_set_reg to defer flip-flop ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + PDP1_BRK_CTR:	logerror("pdp1_set_reg to break counter ignored\n");/* no way!*/ break;
#endif
	case CPUINFO_INT_REGISTER + PDP1_RIM:		pdp1.rim = info->i ? 1 : 0;					break;
	case CPUINFO_INT_REGISTER + PDP1_SBM:		pdp1.sbm = info->i ? 1 : 0;					break;
	case CPUINFO_INT_REGISTER + PDP1_EXD:		EXD = (pdp1.extend_support && info->i) ? 1 : 0; break;
#if LOG
	case CPUINFO_INT_REGISTER + PDP1_IOC:		logerror("pdp1_set_reg to ioc flip-flop ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + PDP1_IOH:		logerror("pdp1_set_reg to ioh flip-flop ignored\n");/* no way!*/ break;
	case CPUINFO_INT_REGISTER + PDP1_IOS:		logerror("pdp1_set_reg to ios flip-flop ignored\n");/* no way!*/ break;
#endif
	case CPUINFO_INT_REGISTER + PDP1_START_CLEAR:	pulse_start_clear();					break;
	case CPUINFO_INT_REGISTER + PDP1_IO_COMPLETE:	pdp1.ios = 1;							break;
	}
}


void pdp1_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(pdp1);					break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 16;							break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
	case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;	/*don't care*/	break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 5;	/* 5us cycle time */	break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 31;	/* we emulate individual 5us cycle, but MUL/DIV have longer timings */	break;

	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 18;	/*16+2 ignored bits to make double word address*/	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

	case CPUINFO_INT_SP:							info->i = 0;	/* no SP */				break;
	case CPUINFO_INT_PC:							info->i = PC;							break;
	case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* TODO??? */			break;

	case CPUINFO_INT_INPUT_STATE + 0:
	case CPUINFO_INT_INPUT_STATE + 1:
	case CPUINFO_INT_INPUT_STATE + 2:
	case CPUINFO_INT_INPUT_STATE + 3:
	case CPUINFO_INT_INPUT_STATE + 4:
	case CPUINFO_INT_INPUT_STATE + 5:
	case CPUINFO_INT_INPUT_STATE + 6:
	case CPUINFO_INT_INPUT_STATE + 7:
	case CPUINFO_INT_INPUT_STATE + 8:
	case CPUINFO_INT_INPUT_STATE + 9:
	case CPUINFO_INT_INPUT_STATE + 10:
	case CPUINFO_INT_INPUT_STATE + 11:
	case CPUINFO_INT_INPUT_STATE + 12:
	case CPUINFO_INT_INPUT_STATE + 13:
	case CPUINFO_INT_INPUT_STATE + 14:
	case CPUINFO_INT_INPUT_STATE + 15:				info->i = (pdp1.irq_state >> (state-CPUINFO_INT_INPUT_STATE)) & 1;	break;

	case CPUINFO_INT_REGISTER + PDP1_PC:			info->i = PC;							break;
	case CPUINFO_INT_REGISTER + PDP1_IR:			info->i = IR;							break;
	case CPUINFO_INT_REGISTER + PDP1_MB:			info->i = MB;							break;
	case CPUINFO_INT_REGISTER + PDP1_MA:			info->i = MA;							break;
	case CPUINFO_INT_REGISTER + PDP1_AC:			info->i = AC;							break;
	case CPUINFO_INT_REGISTER + PDP1_IO:			info->i = IO;							break;
	case CPUINFO_INT_REGISTER + PDP1_OV:			info->i = OV;							break;
	case CPUINFO_INT_REGISTER + PDP1_PF:			info->i = FLAGS;						break;
	case CPUINFO_INT_REGISTER + PDP1_PF1:			info->i = READFLAG(1);					break;
	case CPUINFO_INT_REGISTER + PDP1_PF2:			info->i = READFLAG(2);					break;
	case CPUINFO_INT_REGISTER + PDP1_PF3:			info->i = READFLAG(3);					break;
	case CPUINFO_INT_REGISTER + PDP1_PF4:			info->i = READFLAG(4);					break;
	case CPUINFO_INT_REGISTER + PDP1_PF5:			info->i = READFLAG(5);					break;
	case CPUINFO_INT_REGISTER + PDP1_PF6:			info->i = READFLAG(6);					break;
	case CPUINFO_INT_REGISTER + PDP1_TA:			info->i = pdp1.ta;						break;
	case CPUINFO_INT_REGISTER + PDP1_TW:			info->i = pdp1.tw;						break;
	case CPUINFO_INT_REGISTER + PDP1_SS:			info->i = SENSE_SW;						break;
	case CPUINFO_INT_REGISTER + PDP1_SS1:			info->i = READSENSE(1);					break;
	case CPUINFO_INT_REGISTER + PDP1_SS2:			info->i = READSENSE(2);					break;
	case CPUINFO_INT_REGISTER + PDP1_SS3:			info->i = READSENSE(3);					break;
	case CPUINFO_INT_REGISTER + PDP1_SS4:			info->i = READSENSE(4);					break;
	case CPUINFO_INT_REGISTER + PDP1_SS5:			info->i = READSENSE(5);					break;
	case CPUINFO_INT_REGISTER + PDP1_SS6:			info->i = READSENSE(6);					break;
	case CPUINFO_INT_REGISTER + PDP1_SNGL_STEP:		info->i = pdp1.sngl_step;				break;
	case CPUINFO_INT_REGISTER + PDP1_SNGL_INST:		info->i = pdp1.sngl_inst;				break;
	case CPUINFO_INT_REGISTER + PDP1_EXTEND_SW:		info->i = pdp1.extend_sw;				break;
	case CPUINFO_INT_REGISTER + PDP1_RUN:			info->i = pdp1.run;						break;
	case CPUINFO_INT_REGISTER + PDP1_CYC:			info->i = pdp1.cycle;					break;
	case CPUINFO_INT_REGISTER + PDP1_DEFER:			info->i = pdp1.defer;					break;
	case CPUINFO_INT_REGISTER + PDP1_BRK_CTR:		info->i = pdp1.brk_ctr;					break;
	case CPUINFO_INT_REGISTER + PDP1_RIM:			info->i = pdp1.rim;						break;
	case CPUINFO_INT_REGISTER + PDP1_SBM:			info->i = pdp1.sbm;						break;
	case CPUINFO_INT_REGISTER + PDP1_EXD:			info->i = EXD;							break;
	case CPUINFO_INT_REGISTER + PDP1_IOC:			info->i = pdp1.ioc;						break;
	case CPUINFO_INT_REGISTER + PDP1_IOH:			info->i = pdp1.ioh;						break;
	case CPUINFO_INT_REGISTER + PDP1_IOS:			info->i = pdp1.ios;						break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_PTR_SET_INFO:						info->setinfo = pdp1_set_info;			break;
	case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = pdp1_get_context;	break;
	case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = pdp1_set_context;	break;
	case CPUINFO_PTR_INIT:							info->init = pdp1_init;					break;
	case CPUINFO_PTR_RESET:							info->reset = pdp1_reset;				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = pdp1_execute;			break;
	case CPUINFO_PTR_BURN:							info->burn = NULL;						break;

#ifdef MAME_DEBUG
	case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = pdp1_dasm;			break;
#endif /* MAME_DEBUG */
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &pdp1_ICount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME: 							strcpy(info->s = cpuintrf_temp_str(), "PDP1");	break;
	case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s = cpuintrf_temp_str(), "DEC PDP-1");	break;
	case CPUINFO_STR_CORE_VERSION:					strcpy(info->s = cpuintrf_temp_str(), "2.0");	break;
	case CPUINFO_STR_CORE_FILE:						strcpy(info->s = cpuintrf_temp_str(), __FILE__);	break;
	case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s = cpuintrf_temp_str(),
															"Brian Silverman (original Java Source)\n"
															"Vadim Gerasimov (original Java Source)\n"
															"Chris Salomon (MESS driver)\n"
															"Raphael Nabet (MESS driver)\n");
													break;

    case CPUINFO_STR_FLAGS:
		sprintf(info->s = cpuintrf_temp_str(), "%c%c%c%c%c%c-%c%c%c%c%c%c",
				 (FLAGS & 040) ? '1' : '.',
				 (FLAGS & 020) ? '2' : '.',
				 (FLAGS & 010) ? '3' : '.',
				 (FLAGS & 004) ? '4' : '.',
				 (FLAGS & 002) ? '5' : '.',
				 (FLAGS & 001) ? '6' : '.',
				 (SENSE_SW & 040) ? '1' : '.',
				 (SENSE_SW & 020) ? '2' : '.',
				 (SENSE_SW & 010) ? '3' : '.',
				 (SENSE_SW & 004) ? '4' : '.',
				 (SENSE_SW & 002) ? '5' : '.',
				 (SENSE_SW & 001) ? '6' : '.');
		break;


	case CPUINFO_STR_REGISTER + PDP1_PC:			sprintf(info->s = cpuintrf_temp_str(), "PC:0%06o", PC); break;
	case CPUINFO_STR_REGISTER + PDP1_IR:			sprintf(info->s = cpuintrf_temp_str(), "IR:0%02o", IR); break;
	case CPUINFO_STR_REGISTER + PDP1_MB:			sprintf(info->s = cpuintrf_temp_str(), "MB:0%06o", MB); break;
	case CPUINFO_STR_REGISTER + PDP1_MA:			sprintf(info->s = cpuintrf_temp_str(), "MA:0%06o", MA); break;
	case CPUINFO_STR_REGISTER + PDP1_AC:			sprintf(info->s = cpuintrf_temp_str(), "AC:0%06o", AC); break;
	case CPUINFO_STR_REGISTER + PDP1_IO:			sprintf(info->s = cpuintrf_temp_str(), "IO:0%06o", IO); break;
	case CPUINFO_STR_REGISTER + PDP1_OV:			sprintf(info->s = cpuintrf_temp_str(), "OV:%X", OV); break;
	case CPUINFO_STR_REGISTER + PDP1_PF:			sprintf(info->s = cpuintrf_temp_str(), "FLAGS:0%02o", FLAGS); break;
	case CPUINFO_STR_REGISTER + PDP1_PF1:			sprintf(info->s = cpuintrf_temp_str(), "FLAG1:%X", READFLAG(1)); break;
	case CPUINFO_STR_REGISTER + PDP1_PF2:			sprintf(info->s = cpuintrf_temp_str(), "FLAG2:%X", READFLAG(2)); break;
	case CPUINFO_STR_REGISTER + PDP1_PF3:			sprintf(info->s = cpuintrf_temp_str(), "FLAG3:%X", READFLAG(3)); break;
	case CPUINFO_STR_REGISTER + PDP1_PF4:			sprintf(info->s = cpuintrf_temp_str(), "FLAG4:%X", READFLAG(4)); break;
	case CPUINFO_STR_REGISTER + PDP1_PF5:			sprintf(info->s = cpuintrf_temp_str(), "FLAG5:%X", READFLAG(5)); break;
	case CPUINFO_STR_REGISTER + PDP1_PF6:			sprintf(info->s = cpuintrf_temp_str(), "FLAG6:%X", READFLAG(6)); break;
	case CPUINFO_STR_REGISTER + PDP1_TA:			sprintf(info->s = cpuintrf_temp_str(), "TA:0%06o", pdp1.ta); break;
	case CPUINFO_STR_REGISTER + PDP1_TW:			sprintf(info->s = cpuintrf_temp_str(), "TW:0%06o", pdp1.tw); break;
	case CPUINFO_STR_REGISTER + PDP1_SS:			sprintf(info->s = cpuintrf_temp_str(), "SS:0%02o", SENSE_SW); break;
	case CPUINFO_STR_REGISTER + PDP1_SS1:			sprintf(info->s = cpuintrf_temp_str(), "SENSE1:%X", READSENSE(1)); break;
	case CPUINFO_STR_REGISTER + PDP1_SS2:			sprintf(info->s = cpuintrf_temp_str(), "SENSE2:%X", READSENSE(2)); break;
	case CPUINFO_STR_REGISTER + PDP1_SS3:			sprintf(info->s = cpuintrf_temp_str(), "SENSE3:%X", READSENSE(3)); break;
	case CPUINFO_STR_REGISTER + PDP1_SS4:			sprintf(info->s = cpuintrf_temp_str(), "SENSE4:%X", READSENSE(4)); break;
	case CPUINFO_STR_REGISTER + PDP1_SS5:			sprintf(info->s = cpuintrf_temp_str(), "SENSE5:%X", READSENSE(5)); break;
	case CPUINFO_STR_REGISTER + PDP1_SS6:			sprintf(info->s = cpuintrf_temp_str(), "SENSE6:%X", READSENSE(6)); break;
	case CPUINFO_STR_REGISTER + PDP1_SNGL_STEP:		sprintf(info->s = cpuintrf_temp_str(), "SNGLSTEP:%X", pdp1.sngl_step); break;
	case CPUINFO_STR_REGISTER + PDP1_SNGL_INST:		sprintf(info->s = cpuintrf_temp_str(), "SNGLINST:%X", pdp1.sngl_inst); break;
	case CPUINFO_STR_REGISTER + PDP1_EXTEND_SW:		sprintf(info->s = cpuintrf_temp_str(), "EXS:%X", pdp1.extend_sw); break;
	case CPUINFO_STR_REGISTER + PDP1_RUN:			sprintf(info->s = cpuintrf_temp_str(), "RUN:%X", pdp1.run); break;
	case CPUINFO_STR_REGISTER + PDP1_CYC:			sprintf(info->s = cpuintrf_temp_str(), "CYC:%X", pdp1.cycle); break;
	case CPUINFO_STR_REGISTER + PDP1_DEFER:			sprintf(info->s = cpuintrf_temp_str(), "DF:%X", pdp1.defer); break;
	case CPUINFO_STR_REGISTER + PDP1_BRK_CTR:		sprintf(info->s = cpuintrf_temp_str(), "BRKCTR:%X", pdp1.brk_ctr); break;
	case CPUINFO_STR_REGISTER + PDP1_RIM:			sprintf(info->s = cpuintrf_temp_str(), "RIM:%X", pdp1.rim); break;
	case CPUINFO_STR_REGISTER + PDP1_SBM:			sprintf(info->s = cpuintrf_temp_str(), "SBM:%X", pdp1.sbm); break;
	case CPUINFO_STR_REGISTER + PDP1_EXD:			sprintf(info->s = cpuintrf_temp_str(), "EXD:%X", EXD); break;
	case CPUINFO_STR_REGISTER + PDP1_IOC:			sprintf(info->s = cpuintrf_temp_str(), "IOC:%X", pdp1.ioc); break;
	case CPUINFO_STR_REGISTER + PDP1_IOH:			sprintf(info->s = cpuintrf_temp_str(), "IOH:%X", pdp1.ioh); break;
	case CPUINFO_STR_REGISTER + PDP1_IOS:			sprintf(info->s = cpuintrf_temp_str(), "IOS:%X", pdp1.ios); break;
	}
}


/* execute one instruction */
static void execute_instruction(void)
{
	switch (IR)
	{
	case AND:		/* Logical And */
		AC &= (MB = READ_PDP_18BIT(MA));
		break;
	case IOR:		/* Inclusive Or */
		AC |= (MB = READ_PDP_18BIT(MA));
		break;
	case XOR:		/* Exclusive Or */
		AC ^= (MB = READ_PDP_18BIT(MA));
		break;
	case XCT:		/* Execute */
		MB = READ_PDP_18BIT(MA);
		IR = MB >> 13;		/* basic opcode */
		if ((instruction_kind[IR] & 1) && (MB & 010000))
		{
			pdp1.defer = 1;
			/*pdp1.cycle = 1;*/			/* instruction shall be executed later */
			goto no_fetch;			/* fall through to next instruction */
		}
		else if (instruction_kind[IR] & 2)
		{
			/*pdp1.cycle = 1;*/			/* instruction shall be executed later */
			goto no_fetch;			/* fall through to next instruction */
		}
		else
			execute_instruction();	/* execute instruction at once */
		break;
	case CALJDA:	/* Call subroutine and Jump and Deposit Accumulator instructions */
		if (MB & 010000)
			/* JDA */
			MA = (PC & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);
		else
			/* CAL: equivalent to JDA 100 */
			/* Note that I cannot tell for sure what happens to extension bits, but I did notice
            that setting the extension bits to 0 would make cal basically useless, since
            there would be no simple way the call routine could return to the callee
            if it were located in another module with extend mode off (i.e. exd == 0). */
			MA = (PC & ADDRESS_EXTENSION_MASK) | 0100;

		WRITE_PDP_18BIT(MA, (MB = AC));
		INCREMENT_MA;
		AC = (OV << 17) | (EXD << 16) | PC;
		PC = MA;
		break;
	case LAC:		/* Load Accumulator */
		AC = (MB = READ_PDP_18BIT(MA));
		break;
	case LIO:		/* Load i/o register */
		IO = (MB = READ_PDP_18BIT(MA));
		break;
	case DAC:		/* Deposit Accumulator */
		WRITE_PDP_18BIT(MA, (MB = AC));
		break;
	case DAP:		/* Deposit Address Part */
		WRITE_PDP_18BIT(MA, (MB = ((READ_PDP_18BIT(MA) & 0770000) | (AC & 0007777))));
		break;
	case DIP:		/* Deposit Instruction Part */
		WRITE_PDP_18BIT(MA, (MB = ((READ_PDP_18BIT(MA) & 0007777) | (AC & 0770000))));
		break;
	case DIO:		/* Deposit I/O Register */
		WRITE_PDP_18BIT(MA, (MB = IO));
		break;
	case DZM:		/* Deposit Zero in Memory */
		WRITE_PDP_18BIT(MA, (MB = 0));
		break;
	case ADD:		/* Add */
		{
			/* overflow is set if the 2 operands have the same sign and the final result has another */
			int ov2;	/* 1 if the operands have the same sign*/

			MB = READ_PDP_18BIT(MA);

			ov2 = ((AC & 0400000) == (MB & 0400000));

			AC = AC + MB;
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

			/* I think we need to check for overflow before checking for -0,
            because the sum -0+-0 = -0 = +0 would generate an overflow
            otherwise. */
			if (ov2 && ((AC & 0400000) != (MB & 0400000)))
				OV = 1;

			if (AC == 0777777)		/* check for -0 */
				AC = 0;

			break;
		}
	case SUB:		/* Subtract */
		{	/* maintainance manual 7-14 seems to imply that substract does not test for -0.
              The sim 2.3 source says so explicitely, though they do not give a reference.
              It sounds a bit weird, but the reason is probably that doing so would
              require additionnal logic that does not exist. */
			/* overflow is set if the 2 operands have the same sign and the final result has another */
			int ov2;	/* 1 if the operands have the same sign*/

			AC ^= 0777777;

			MB = READ_PDP_18BIT(MA);

			ov2 = ((AC & 0400000) == (MB & 0400000));

			AC = AC + MB;
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */

			if (ov2 && ((AC & 0400000) != (MB & 0400000)))
				OV = 1;

			AC ^= 0777777;

			break;
		}
	case IDX:		/* Index */
		AC = READ_PDP_18BIT(MA) + 1;

		#if 0
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
			if (AC == 0777777)		/* check for -0 */
				AC = 0;
		#else
			if (AC >= 0777777)
				AC = (AC + 1) & 0777777;
		#endif

		WRITE_PDP_18BIT(MA, (MB = AC));
		break;
	case ISP:		/* Index and Skip if Positive */
		AC = READ_PDP_18BIT(MA) + 1;

		#if 0
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
			if (AC == 0777777)		/* check for -0 */
				AC = 0;
		#else
			if (AC >= 0777777)
				AC = (AC + 1) & 0777777;
		#endif

		WRITE_PDP_18BIT(MA, (MB = AC));
		if ((AC & 0400000) == 0)
			INCREMENT_PC;
		break;
	case SAD:		/* Skip if Accumulator and Y differ */
		if (AC != (MB = READ_PDP_18BIT(MA)))
			INCREMENT_PC;
		break;
	case SAS:		/* Skip if Accumulator and Y are the same */
		if (AC == (MB = READ_PDP_18BIT(MA)))
			INCREMENT_PC;
		break;
	case MUS_MUL:	/* Multiply Step or Multiply */
		if (pdp1.hw_mul_div)
		{	/* MUL */
			int scr;
			int smb, srm;
			double etime = 4.;		/* approximative */

			IO = MB = AC;
			MB = READ_PDP_18BIT(MA);
			scr = 0;
			if (MB & 0400000)
			{
				smb = 1;
				MB = MB ^ 0777777;
			}
			else
				smb = 0;
			if (IO & 0400000)
			{
				srm = 1;
				IO = IO ^ 0777777;
			}
			else
				srm = 0;
			AC = 0;
			scr++;
			while (scr < 022)
			{
				if (IO & 1)
				{
					/*assert(! (AC & 0400000));*/
					AC = AC + MB;
					/* we can save carry around since both numbers are positive */
					/*AC = (AC + (AC >> 18)) & 0777777;*/
					etime += .65;		/* approximative */
				}
				IO = (IO >> 1) | ((AC & 1) << 17);
				AC = AC >> 1;
				scr++;
			}
			if (smb ^ srm)
			{
				AC = AC ^ 0777777;
				IO = IO ^ 0777777;
			}

			pdp1_ICount -= etime+.5;	/* round to closest */
		}
		else
		{	/* MUS */
			/* should we check for -0??? (Maintainance manual 7-14 seems to imply we should not:
            as a matter of fact, since the MUS instruction is supposed to have positive operands,
            there is no need to check for -0, therefore such a simplification does not sound
            absurd.) */
			if ((IO & 1) == 1)
			{
				AC = AC + (MB = READ_PDP_18BIT(MA));
				AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
			}
			IO = (IO >> 1 | AC << 17) & 0777777;
			AC >>= 1;
		}
		break;
	case DIS_DIV:	/* Divide Step or Divide */
		if (pdp1.hw_mul_div)
		{	/* DIV */
			/* As a side note, the order of -0 detection and overflow checking does not matter,
            because the sum of two positive number cannot give 0777777 (since positive
            numbers are 0377777 at most, their sum is 0777776 at most).
            Additionnally, we cannot have carry set and a result equal to 0777777  (since numbers
            are 0777777 at most, their sum is 01777776 at most): this is nice, because it makes
            the sequence:
                AC = (AC + (AC >> 18)) & 0777777;   // propagate carry around
                if (AC == 0777777)          // check for -0
                    AC = 0;
            equivalent to:
                if (AC >= 0777777)
                    AC = (AC + 1) & 0777777;
            which is a bit more efficient. */
			int acl;
			int scr;
			int smb, srm;
			double etime = 0;		/* approximative */

			MB = READ_PDP_18BIT(MA);
			scr = 0;
			if (MB & 0400000)
			{
				smb = 1;
			}
			else
			{
				smb = 0;
				MB = MB ^ 0777777;
			}
			if (AC & 0400000)
			{
				srm = 1;
				AC = AC ^ 0777777;
				IO = IO ^ 0777777;
			}
			else
				srm = 0;
			while (1)
			{
				AC = (AC + MB);
				#if 1
					AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
					if (AC == 0777777)		/* check for -0 */
						AC = 0;
				#else
					if (AC >= 0777777)
						AC = (AC + 1) & 0777777;
				#endif
				if (MB & 0400000)
					MB = MB ^ 0777777;

				if (((scr == 0) && ! (AC & 0400000))
					|| (scr == 022))
					break;

				scr++;

				if (! (AC & 0400000))
					MB = MB ^ 0777777;

				acl = AC >> 17;
				AC = (AC << 1 | IO >> 17) & 0777777;
				IO = ((IO << 1 | acl) & 0777777) ^ 1;
				if (acl)
				{
					AC++;
					AC = (AC + (AC >> 18)) & 0777777;
					etime += .6;		/* approximative */
				}
			}

			AC = (AC + MB);
			#if 1
				AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
				if (AC == 0777777)		/* check for -0 */
					AC = 0;
			#else
				if (AC >= 0777777)
					AC = (AC + 1) & 0777777;
			#endif

			if (scr)
			{
				INCREMENT_PC;
				AC = AC >> 1;
			}

			if (srm && (AC != 0))
				AC = AC ^ 0777777;

			if (((! scr) && (srm))
					|| (scr && (srm ^ smb) && (IO != 0)))
				IO = IO ^ 0777777;

			if (scr)
			{
				MB = AC;
				AC = IO;
				IO = MB;
			}
			if (scr)
				etime += 20;		/* approximative */
			else
				etime += 2;			/* approximative */

			pdp1_ICount -= etime+.5;	/* round to closest */
		}
		else
		{	/* DIS */
			int acl;

			acl = AC >> 17;
			AC = (AC << 1 | IO >> 17) & 0777777;
			IO = ((IO << 1 | acl) & 0777777) ^ 1;
			MB = READ_PDP_18BIT(MA);
			if (IO & 1)
				AC += (MB ^ 0777777);
			else
				/* Note that if AC+MB = 0777777, we are in trouble.  I don't
                know how a real PDP-1 behaves in this case. */
				AC += MB + 1;
			AC = (AC + (AC >> 18)) & 0777777;	/* propagate carry around */
			if (AC == 0777777)		/* check for -0 */
				AC = 0;
		}
		break;
	case JMP:		/* Jump */
		if (pdp1.exc)
			PC = MB & EXTENDED_ADDRESS_MASK;
		else
			PC = (MA & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);
		break;
	case JSP:		/* Jump and Save Program Counter */
		AC = (OV << 17) | (EXD << 16) | PC;
		if (pdp1.exc)
			PC = MB & EXTENDED_ADDRESS_MASK;
		else
			PC = (MA & ADDRESS_EXTENSION_MASK) | (MB & BASE_ADDRESS_MASK);
		break;
	case SKP:		/* Skip Instruction Group */
		{
			int cond = ((MB & 0100) && (AC == 0))	/* ZERO Accumulator */
				|| ((MB & 0200) && (AC >> 17 == 0))	/* Plus Accumulator */
				|| ((MB & 0400) && (AC >> 17 == 1))	/* Minus Accumulator */
				|| ((MB & 01000) && (OV == 0))		/* ZERO Overflow */
				|| ((MB & 02000) && (IO >> 17 == 0))	/* Plus I/O Register */
				|| (((MB & 7) != 0) && (((MB & 7) == 7) ? ! FLAGS : ! READFLAG(MB & 7)))	/* ZERO Flag (deleted by mistake in PDP-1 handbook) */
				|| (((MB & 070) != 0) && (((MB & 070) == 070) ? ! SENSE_SW : ! READSENSE((MB & 070) >> 3)));	/* ZERO Switch */

			if (! (MB & 010000))
			{
				if (cond)
					INCREMENT_PC;
			}
			else
			{
				if (!cond)
					INCREMENT_PC;
			}
			if (MB & 01000)
				OV = 0;
			break;
		}
	case SFT:		/* Shift Instruction Group */
		{
			/* Bit 5 specifies direction of shift, Bit 6 specifies the character of the shift
            (arithmetic or logical), Bits 7 and 8 enable the registers (01 = AC, 10 = IO,
            and 11 = both) and Bits 9 through 17 specify the number of steps. */
			int nshift = 0;
			int mask = MB & 0777;

			while (mask != 0)
			{
				nshift += mask & 1;
				mask >>= 1;
			}
			switch ((MB >> 9) & 017)
			{
				int i;

			case 1:		/* ral rotate accumulator left */
				for (i = 0; i < nshift; i++)
					AC = (AC << 1 | AC >> 17) & 0777777;
				break;
			case 2:		/* ril rotate i/o register left */
				for (i = 0; i < nshift; i++)
					IO = (IO << 1 | IO >> 17) & 0777777;
				break;
			case 3:		/* rcl rotate AC and IO left */
				for (i = 0; i < nshift; i++)
				{
					int tmp = AC;

					AC = (AC << 1 | IO >> 17) & 0777777;
					IO = (IO << 1 | tmp >> 17) & 0777777;
				}
				break;
			case 5:		/* sal shift accumulator left */
				for (i = 0; i < nshift; i++)
					AC = ((AC << 1 | AC >> 17) & 0377777) + (AC & 0400000);
				break;
			case 6:		/* sil shift i/o register left */
				for (i = 0; i < nshift; i++)
					IO = ((IO << 1 | IO >> 17) & 0377777) + (IO & 0400000);
				break;
			case 7:		/* scl shift AC and IO left */
				for (i = 0; i < nshift; i++)
				{
					int tmp = AC;

					AC = ((AC << 1 | IO >> 17) & 0377777) + (AC & 0400000);		/* shouldn't that be IO?, no it is the sign! */
					IO = (IO << 1 | tmp >> 17) & 0777777;
				}
				break;
			case 9:		/* rar rotate accumulator right */
				for (i = 0; i < nshift; i++)
					AC = (AC >> 1 | AC << 17) & 0777777;
				break;
			case 10:	/* rir rotate i/o register right */
				for (i = 0; i < nshift; i++)
					IO = (IO >> 1 | IO << 17) & 0777777;
				break;
			case 11:	/* rcr rotate AC and IO right */
				for (i = 0; i < nshift; i++)
				{
					int tmp = AC;

					AC = (AC >> 1 | IO << 17) & 0777777;
					IO = (IO >> 1 | tmp << 17) & 0777777;
				}
				break;
			case 13:	/* sar shift accumulator right */
				for (i = 0; i < nshift; i++)
					AC = (AC >> 1) + (AC & 0400000);
				break;
			case 14:	/* sir shift i/o register right */
				for (i = 0; i < nshift; i++)
					IO = (IO >> 1) + (IO & 0400000);
				break;
			case 15:	/* scr shift AC and IO right */
				for (i = 0; i < nshift; i++)
				{
					int tmp = AC;

					AC = (AC >> 1) + (AC & 0400000);	/* shouldn't that be IO, no it is the sign */
					IO = (IO >> 1 | tmp << 17) & 0777777;
				}
				break;
			default:
				if (LOG)
					logerror("Undefined shift: 0%06o at 0%06o\n", MB, PREVIOUS_PC);
				break;
			}
			break;
		}
	case LAW:		/* Load Accumulator with N */
		AC = MB & 07777;
		if (MB & 010000)
			AC ^= 0777777;
		break;
	case IOT:		/* In-Out Transfer Instruction Group */
		/*
            The variations within this group of instructions perform all the in-out control
            and information transfer functions.  If Bit 5 (normally the Indirect Address bit)
            is a ONE, the computer will enter a special waiting state until the completion pulse
            from the activated device has returned.  When this device delivers its completion,
            the computer will resume operation of the instruction sequence.

            The computer may be interrupted from the special waiting state to serve a sequence
            break request or a high speed channel request.

            Most in-out operations require a known minimum time before completion.  This time
            may be utilized for programming.  The appropriate In-Out Transfer can be given with
            no in-out wait (Bit 5 a ZERO and Bit 6 a ONE).  The instruction sequence then
            continues.  This sequence must include an iot instruction 730000 which performs
            nothing but the in-out wait. The computer will then enter the special waiting state
            until the device returns the in-out restart pulse.  If the device has already
            returned the completion pulse before the instruction 730000, the computer will
            proceed immediately.

            Bit 6 determines whether a completion pulse will or will not be received from
            the in-out device.  When it is different than Bit 5, a completion pulse will be
            received.  When it is the same as Bit 5, a completion pulse will not be received.

            In addition to the control function of Bits 5 and 6, Bits 7 through 11 are also
            used as control bits serving to extend greatly the power of the iot instructions.
            For example, Bits 12 through 17, which are used to designate a class of input or
            output devices such as typewriters, may be further defined by Bits 7 through 11
            as referring to Typewriter 1, 2, 3, etc.  In several of the optional in-out devices,
            in particular the magnetic tape, Bits 7 through 11 specify particular functions
            such as forward, backward etc.  If a large number of specialized devices are to
            be attached, these bits may be used to further the in-out transfer instruction
            to perform totally distinct functions.

            Note that ioc is supposed to be set at the beggining of the memory cycle after
            ioh is cleared.
            However, we cannot set ioc at the beggining of every memory cycle as we
            did before, because it breaks in the following case:
            a) IOT instruction enables IO wait
            b) sequence break in the middle of IO-halt
            c) ioh is cleared in middle of sequence break routine
            d) re-execute IOT instruction.  Unfortunately, ioc has been cleared, therefore
              we perform an IOT command pulse and IO wait again, which is completely WRONG.
            Therefore ioc is cleared only after a IOT with wait is executed.
        */
		if (MB & 010000)
		{	/* IOT with IO wait */
			if (pdp1.ioc)
			{	/* the iot command line is pulsed only if ioc is asserted */
				(*pdp1.extern_iot[MB & 0000077])(MB & 0000077, (MB & 0004000) == 0, MB, &IO, AC);

				pdp1.ioh = 1;	/* enable io wait */

				pdp1.ioc = 0;	/* actually happens at the start of next memory cycle */

				/* test ios now in case the IOT callback has sent a completion pulse immediately */
				if (pdp1.ioh && pdp1.ios)
				{
					/* ioh should be cleared at the end of the instruction cycle, and ios at the
                    start of next instruction cycle, but who cares? */
					pdp1.ioh = 0;
					pdp1.ios = 0;
				}
			}

			if (pdp1.ioh)
				DECREMENT_PC;
			else
				pdp1.ioc = 1;	/* actually happens at the start of next memory cycle */
		}
		else
		{	/* IOT with no IO wait */
			(*pdp1.extern_iot[MB & 0000077])(MB & 0000077, (MB & 0004000) != 0, MB, &IO, AC);
		}
		break;
	case OPR:		/* Operate Instruction Group */
		{
			int nflag;

			if (MB & 00200)		/* clear AC */
				AC = 0;
			if (MB & 04000)		/* clear I/O register */
				IO = 0;
			if (MB & 02000)		/* load Accumulator from Test Word */
				AC |= pdp1.tw;
			if (MB & 00100)		/* load Accumulator with Program Counter */
				AC |= (OV << 17) | (EXD << 16) | PC;
			nflag = MB & 7;
			if (nflag)
			{
				if (nflag == 7)
					FLAGS = (MB & 010) ? 077 : 000;
				else
					WRITEFLAG(nflag, (MB & 010) ? 1 : 0);
			}
			if (MB & 01000)		/* Complement AC */
				AC ^= 0777777;
			if (MB & 00400)		/* Halt */
			{
				if (LOG_EXTRA)
					logerror("PDP1 Program executed HALT: at 0%06o\n", PREVIOUS_PC);

				pdp1.run = 0;
			}
			break;
		}
	default:
		if (LOG)
			logerror("Illegal instruction: 0%06o at 0%06o\n", MB, PREVIOUS_PC);

		/* let us stop the CPU, like a real pdp-1 */
		pdp1.run = 0;

		break;
	}
	pdp1.cycle = 0;
no_fetch:
	;
}


/*
    Handle unimplemented IOT
*/
static void null_iot(int op2, int nac, int mb, int *io, int ac)
{
	/* Note that the dummy IOT 0 is used to wait for the completion pulse
    generated by the a pending IOT (IOT with completion pulse but no IO wait) */
	if (LOG_IOT_EXTRA)
	{
		if (op2 == 000)
			logerror("IOT sync instruction: mb=0%06o, pc=0%06o\n", (unsigned) mb, (unsigned) cpunum_get_reg(0, PDP1_PC));
	}
	if (LOG)
	{
		if (op2 != 000)
			logerror("Not supported IOT command (no external IOT function given) 0%06o at 0%06o\n", mb, PREVIOUS_PC);
	}
}


/*
    Memory expansion control (type 15)

    IOT 74: LEM/EEM
*/
static void lem_eem_iot(int op2, int nac, int mb, int *io, int ac)
{
	if (! pdp1.extend_support)	/* extend mode supported? */
	{
		if (LOG)
			logerror("Ignoring internal error in file " __FILE__ " line %d.\n", __LINE__);
		return;
	}
	if (LOG_EXTRA)
	{
		logerror("EEM/LEM instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);
	}
	EXD = (mb & 0004000) ? 1 : 0;
}


/*
    Standard sequence break system

    IOT 54: lsm
    IOT 55: esm
    IOT 56: cbs
*/
static void sbs_iot(int op2, int nac, int mb, int *io, int ac)
{
	switch (op2)
	{
	case 054:	/* LSM */
		if (LOG_EXTRA)
			logerror("LSM instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.sbm = 0;
		field_interrupt();
		break;
	case 055:	/* ESM */
		if (LOG_EXTRA)
			logerror("ESM instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.sbm = 1;
		field_interrupt();
		break;
	case 056:	/* CBS */
		if (LOG_EXTRA)
			logerror("CBS instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		/*pdp1.b3 = 0;*/
		pdp1.b4 = 0;
		field_interrupt();
		break;
	default:
		if (LOG)
			logerror("Ignoring internal error in file " __FILE__ " line %d.\n", __LINE__);

		break;
	}
}


/*
    type 20 sequence break system

    IOT 50: dsc
    IOT 51: asc
    IOT 52: isb
    IOT 53: cac
*/
static void type_20_sbs_iot(int op2, int nac, int mb, int *io, int ac)
{
	int channel, mask;
	if (! pdp1.type_20_sbs)	/* type 20 sequence break system supported? */
	{
		if (LOG)
			logerror("Ignoring internal error in file " __FILE__ " line %d.\n", __LINE__);
		return;
	}
	channel = (mb >> 6) & 017;
	mask = 1 << channel;
	switch (op2)
	{
	case 050:	/* DSC */
		if (LOG_EXTRA)
			logerror("DSC instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.b1 &= ~mask;
		field_interrupt();
		break;
	case 051:	/* ASC */
		if (LOG_EXTRA)
			logerror("ASC instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.b1 |= mask;
		field_interrupt();
		break;
	case 052:	/* ISB */
		if (LOG_EXTRA)
			logerror("ISB instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.b2 |= mask;
		field_interrupt();
		break;
	case 053:	/* CAC */
		if (LOG_EXTRA)
			logerror("CAC instruction: mb=0%06o, pc=0%06o\n", mb, pdp1.pc);

		pdp1.b1 = 0;
		field_interrupt();
		break;
	default:
		if (LOG)
			logerror("Ignoring internal error in file " __FILE__ " line %d.\n", __LINE__);

		break;
	}

}


/*
    Simulate a pulse on start/clear line:
    reset most registers and flip-flops, and initialize a few emulator state
    variables.
*/
static void pulse_start_clear(void)
{
	/* processor registers */
	PC = 0;			/* according to maintainance manual p. 6-17 */
	IR = 0;			/* according to maintainance manual p. 6-13 */
	/*MB = 0;*/		/* ??? */
	/*MA = 0;*/		/* ??? */
	/*AC = 0;*/		/* ??? */
	/*IO = 0;*/		/* ??? */
	/*PF = 0;*/		/* ??? */

	/* processor state flip-flops */
	pdp1.run = 0;		/* ??? */
	pdp1.cycle = 0;		/* mere guess */
	pdp1.defer = 0;		/* mere guess */
	pdp1.brk_ctr = 0;	/* mere guess */
	pdp1.ov = 0;		/* according to maintainance manual p. 7-18 */
	pdp1.rim = 0;		/* ??? */
	pdp1.sbm = 0;		/* ??? */
	EXD = 0;			/* according to maintainance manual p. 8-16 */
	pdp1.exc = 0;		/* according to maintainance manual p. 8-16 */
	pdp1.ioc = 1;		/* according to maintainance manual p. 6-10 */
	pdp1.ioh = 0;		/* according to maintainance manual p. 6-10 */
	pdp1.ios = 0;		/* according to maintainance manual p. 6-10 */

	pdp1.b1 = pdp1.type_20_sbs ? 0 : 1;	/* mere guess */
	pdp1.b2 = 0;		/* mere guess */
	pdp1.b4 = 0;		/* mere guess */


	pdp1.rim_step = 0;
	pdp1.sbs_restore = 0;		/* mere guess */
	pdp1.no_sequence_break = 0;	/* mere guess */

	field_interrupt();

	/* now, we kindly ask IO devices to reset, too */
	if (pdp1.io_sc_callback)
		(*pdp1.io_sc_callback)();
}
