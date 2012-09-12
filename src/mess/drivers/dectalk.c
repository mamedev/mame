/******************************************************************************
*
*  Dectalk DTC-01 Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with major help (dumping, tech questions answered, etc)
*  from Kevin 'kevtris' Horton, without whom this driver would
*  have been impossible.
*  Special thanks to Al Kossow for archiving the DTC-01 schematic at bitsavers,
*  http://bitsavers.org/pdf/dec/dectalk/MP-01820_DTC01_EngrDrws_Nov83.pdf
*  which has been invaluable for work on this driver.
*  Special thanks to leeeeee for helping figure out what the led selftest codes actually mean
*
*  TODO:
*  * DUART:
*    * <DONE> DUART needs to be reset on reset line activation. as is it works ok, but it should be done anyway.
*    * DUART needs its i/o pins connected as well:
*    * pins IP0, IP2, and IP3 are connected to the primary serial port:
*      * IP0 is CTS
*      * IP2 is DSR
*      * IP3 is RLS (received line signal, this pin is rarely used on rs232)
*    * <DONE> pins IP4, IP5 and IP6 are on jumpers on the DUART, tied high normally but jumperable low, should be handled as dipswitches:
*      * IP4 low: skip hardware self tests
*      * IP5 low: unknown
*      * IP6 low: unknown
*    * pins OP0 and OP2 are connected to the primary serial port:
*      * OP0 is RTS
*      * OP2 is DTR
*  * Actually store the X2212 nvram's eeprom data to disk rather than throwing it out on exit
*    * Is there some way I can hook this up using &generic_nvram? Right now signs point to no.
*  * emulate/simulate the MT8060 dtmf decoder as a 16-key input device? or hook it to some simple fft code? Francois Javier's fftmorse code ran full speed on a 6mhz 80286, maybe use that?
*  * discuss and figure out how to have an external application send data to the two serial ports to be spoken (maybe using paste from clipboard?)
*
* LED error code list (found by experimentation and help from leeeeee):
*    Startup Self tests:
*    FF 00 - M68k address register check fail or data register check fail (test code at $21E)
*     (for some data register failures the processor just spins forever and no led code is generated)
*    FF 01 - ROM check fail @ 0x00000, rom at E8 or E22 (test code at $278)
*    FF 02 - ROM check fail @ 0x08000, rom at E7 or E21 "
*    FF 03 - ROM check fail @ 0x10000, rom at E6 or E20 "
*    FF 04 - ROM check fail @ 0x18000, rom at E5 or E19 "
*    FF 05 - ROM check fail @ 0x20000, rom at E4 or E18 "
*    FF 06 - ROM check fail @ 0x28000, rom at E3 or E17 "
*    FF 07 - ROM check fail @ 0x30000, rom at E2 or E16 "
*    FF 08 - ROM check fail @ 0x38000, rom at E1 or E15 "
*    FF 0F - ROM check fail at multiple addresses
*    FE 01 - RAM check fail @ 0x80000-0x83fff, ram at E36 or E49 (test code at $338)
*    FE 02 - RAM check fail @ 0x84000-0x87fff, ram at E35 or E48 "
*    FE 03 - RAM check fail @ 0x88000-0x8bfff, ram at E34 or E47 "
*    FE 04 - RAM check fail @ 0x8c000-0x8ffff, ram at E33 or E46 "
*    FE 05 - RAM check fail @ 0x90000-0x93fff, ram at E32 or E44 "
*    FD 00 - DUART test & DUART interrupt test (test code at $046C)
*    FC 00 - This test doesn't exist. Some vestiges of it may remain in code for the FD and FB tests.
*    FB 00 - TMS32010 extensive tests (test code at $051E): test spc interrupt [works] and make dtmf tone to test tlc interrupt [fails in mess, requires dtmf detection on output]
*    Jump to $102C to skip the self tests
*    During normal operation:
(table taken from http://www3.sympatico.ca/n.rieck/docs/DECtalk_notes.html )
DTC-01 LEDs

   76543210         LEDs
   ||||||||
   |||||||+-------- set if host asserting CTS
   ||||||+--------- set if DECtalk asserting RTS
   |||||+---------- set if host asserting DSR
   ||||+----------- set if host asserting DCD
   |||+------------ set if DECtalk asserting DTR
   +++------------- 3 bit state code

   000              in state 5, first 500 ms; waiting for CD & CTS in first 500 ms when DECtalk is on-line
   001              timing 2 second CD=0 while in state 6 (moving data)
   010              waiting for DSR=0 while disconnecting (part of state 7)
   011              waiting for DSR=1 while connecting (state 3)
   100              delaying for UK modems during disconnect (part of state 7)
   101              waiting for CD and CTS (main part of state 5)
   110              moving data (state 6)
   111              disconnecting (start of state 7)
*    Dectalk dtc-01 hardware and rom version history, from dtc-01 schematic at bitsavers and with additional info from
     http://americanhistory.si.edu/archives/speechsynthesis/ss_dec1.htm
*    August 1983: Hardware version A done
*    unknown date: Version 1.0 roms finalized
*    unknown date: Version 1.1 roms released to fix a bug with insufficient stack space, see ss_dec1 above
*    March 1984: Hardware version B done (integrates the output fifo sync error check onto the pcb; Version A units are retrofitted when sent in for firmware upgrades) (most of the schematics come from this time)
     <there might be a v1.2 or v1.3 in this period>
*    July 02 1984: Second half of Version 2.0 rom finalized
*    July 23 1984: First half of Version 2.0 rom finalized
*    October 1984 (the rest of the schematics come from this time)
*    unknown date: version 2.1 rom finalized
*
* NVRAM related stuff found by leeeeee:
* $10402 - nvram recall/reset based on byte on stack (0 = recall, 1 = write default to nvram)
* $10f2E - nvram recall
* $10f52 - entry point for nvram check routine
* $11004(entry point)-$11030, $11032-$111B4 - nvram store routine:
*    pass with a blank word on stack and be sure the test and branch at 11016 & 1101a passes (jumps to 1106a)
*    It will then write currently loaded settings to nvram and execute a store, so do it after the defaults are loaded
* $1a7ae - default nvram image, without checksum (0x80 bytes)
*******************************************************************************/
/*the 68k memory map is such:
0x000000-0x007fff: E22 low, E8 high
0x008000-0x00ffff: E21 low, E7 high
0x010000-0x017fff: E20 low, E6 high
0x018000-0x01ffff: E19 low, E5 high
0x020000-0x027fff: E18 low, E4 high
0x028000-0x02ffff: E17 low, E3 high
0x030000-0x037fff: E16 low, E2 high
0x038000-0x03ffff: E15 low, E1 high
mirrrored at 0x040000-0x07ffff
ram/nvram/speech mapping:
0x080000-0x083fff: e36 low, e49 high
0x084000-0x087fff: e35 low, e48 high
0x088000-0x08bfff: e34 low, e47 high
0x08c000-0x08ffff: e33 low, e46 high
0x090000-0x093fff: e32 low, e45 high
0x094000-0x097fff: LED/SW/NVR
0x098000-0x09bfff: DUART
0x09c000-0x09ffff: DTMF and TMS32010 (TLC, SPC)
mirrored at 0x0a0000-0x0fffff x3
entire space mirrored at 0x100000-0x7fffff
0x800000-0xffffff is open bus?

interrupts:
68k has an interrupt priority decoder attached to it:
TLC is INT level 4
SPC is INT level 5
DUART is INT level 6
*/
// USE_LOOSE_TIMING makes the cpu interleave much lower and boosts it on fifo and flag writes by the 68k and semaphore sets by the dsp
#define USE_LOOSE_TIMING 1
// generic logs like led state, and common writes for dsp and spc such as the speech int
#undef VERBOSE
// logs reads and writes to nvram, and nvram store/recall flag messages
#undef NVRAM_LOG
// logs reads and writes to TLC regs
#undef TLC_LOG
// logs reads and writes to SPC regs, 68k side only
#undef SPC_LOG_68K
// logs reads and writes to SPC regs, dsp side only
#undef SPC_LOG_DSP
// logs txd and related serial lines to stderr
#undef SERIAL_TO_STDERR

/* Core includes */
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "dectalk.lh" //  hack to avoid screenless system crash
#include "machine/68681.h"
#include "sound/dac.h"
#include "machine/terminal.h"


class dectalk_state : public driver_device
{
public:
	dectalk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_terminal(*this, TERMINAL_TAG) { }

	UINT8 m_data[8]; // hack to prevent gcc bitching about struct pointers. not used.
	UINT8 m_x2214_sram[256]; // NVRAM chip's temp sram space
	UINT8 m_statusLED;
	// input fifo, between m68k and tms32010
	UINT16 m_infifo[32]; // technically eight 74LS224 4bit*16stage FIFO chips, arranged as a 32 stage, 16-bit wide fifo
	UINT8 m_infifo_tail_ptr;
	UINT8 m_infifo_head_ptr;
	// output fifo, between tms32010 and 10khz sample latch for dac
	UINT16 m_outfifo[16]; // technically three 74LS224 4bit*16stage FIFO chips, arranged as a 16 stage, 12-bit wide fifo
	UINT8 m_outfifo_tail_ptr;
	UINT8 m_outfifo_head_ptr;
	UINT8 m_infifo_semaphore; // latch for status of output fifo, d-latch 74ls74 @ E64 'lower half'
	UINT8 m_spc_error_latch; // latch for error status of speech dsp, d-latch 74ls74 @ E64 'upper half'
	UINT8 m_m68k_spcflags_latch; // latch for initializing the speech dsp, d-latch 74ls74 @ E29 'lower half', AND latch for spc irq enable, d-latch 74ls74 @ E29 'upper half'; these are stored in bits 0 and 6 respectively, the rest of the bits stored here MUST be zeroed!
	UINT8 m_m68k_tlcflags_latch; // latch for telephone interface stuff, d-latches 74ls74 @ E93 'upper half' and @ 103 'upper and lower halves'
	UINT8 m_simulate_outfifo_error; // simulate an error on the outfifo, which does something unusual to the dsp latches
	UINT8 m_tlc_tonedetect;
	UINT8 m_tlc_ringdetect;
	UINT8 m_tlc_dtmf; // dtmf holding reg
	UINT8 m_duart_inport; // low 4 bits of duart input
	UINT8 m_duart_outport; // most recent duart output
	UINT8 m_hack_self_test; // temp variable for hack below

	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(nvram_read);
	DECLARE_WRITE8_MEMBER(led_write);
	DECLARE_WRITE8_MEMBER(nvram_write);
	DECLARE_WRITE16_MEMBER(m68k_infifo_w);
	DECLARE_READ16_MEMBER(m68k_spcflags_r);
	DECLARE_WRITE16_MEMBER(m68k_spcflags_w);
	DECLARE_READ16_MEMBER(m68k_tlcflags_r);
	DECLARE_WRITE16_MEMBER(m68k_tlcflags_w);
	DECLARE_READ16_MEMBER(m68k_tlc_dtmf_r);
	DECLARE_WRITE16_MEMBER(spc_latch_outfifo_error_stats);
	DECLARE_READ16_MEMBER(spc_infifo_data_r);
	DECLARE_WRITE16_MEMBER(spc_outfifo_data_w);
	DECLARE_READ16_MEMBER(spc_semaphore_r);
	DECLARE_DRIVER_INIT(dectalk);
};


/* Devices */
static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	device->machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_6, state, M68K_INT_ACK_AUTOVECTOR);
	//device->machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_6, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
	//device->machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_6, HOLD_LINE, vector);
};

static UINT8 duart_input(device_t *device)
{
	dectalk_state *state = device->machine().driver_data<dectalk_state>();
	UINT8 data = 0;
	data |= state->m_duart_inport&0xF;
	data |= (device->machine().root_device().ioport("duart_in")->read()&0xF0);
	if ((state->m_hack_self_test == 1) && (state->ioport("hacks")->read()&0x01)) data |= 0x10; // hack to prevent hang if selftest disable bit is kept low past the first read; i suppose the proper use of this bit was an incremental switch, or perhaps its expecting an interrupt later from serial in or tone in? added a dipswitch to disable the hack for testing
		state->m_hack_self_test = 1;
	return data;
}

static void duart_output(device_t *device, UINT8 data)
{
	dectalk_state *state = device->machine().driver_data<dectalk_state>();
	state->m_duart_outport = data;
#ifdef SERIAL_TO_STDERR
	fprintf(stderr, "RTS: %01X, DTR: %01X\n", data&1, (data&4)>>2);
#endif
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
	device_t *devconf = device->machine().device(TERMINAL_TAG);
	dynamic_cast<generic_terminal_device *>(devconf)->write(*devconf->machine().memory().first_space(), 0, data);
#ifdef SERIAL_TO_STDERR
	fprintf(stderr, "%02X ",data);
#endif
}

static const duart68681_config dectalk_duart68681_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	duart_output
};

#define SPC_INITIALIZE state->m_m68k_spcflags_latch&0x1 // speech initialize flag
#define SPC_IRQ_ENABLED ((state->m_m68k_spcflags_latch&0x40)>>6) // irq enable flag

static void dectalk_outfifo_check (running_machine &machine)
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	// check if output fifo is full; if it isn't, set the int on the dsp
	if (((state->m_outfifo_head_ptr-1)&0xF) != state->m_outfifo_tail_ptr)
	machine.device("dsp")->execute().set_input_line(0, ASSERT_LINE); // TMS32010 INT
	else
	machine.device("dsp")->execute().set_input_line(0, CLEAR_LINE); // TMS32010 INT
}

static void dectalk_clear_all_fifos( running_machine &machine )
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	// clear fifos (TODO: memset would work better here...)
	int i;
	for (i=0; i<16; i++) state->m_outfifo[i] = 0;
	for (i=0; i<32; i++) state->m_infifo[i] = 0;
	state->m_outfifo_tail_ptr = state->m_outfifo_head_ptr = 0;
	state->m_infifo_tail_ptr = state->m_infifo_head_ptr = 0;
	dectalk_outfifo_check(machine);
}

static void dectalk_x2212_store( running_machine &machine )
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	UINT8 *nvram = state->memregion("nvram")->base();
	int i;
	for (i = 0; i < 256; i++)
	nvram[i] = state->m_x2214_sram[i];
#ifdef NVRAM_LOG
	logerror("nvram store done\n");
#endif
}

static void dectalk_x2212_recall( running_machine &machine )
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	UINT8 *nvram = state->memregion("nvram")->base();
	int i;
	for (i = 0; i < 256; i++)
	state->m_x2214_sram[i] = nvram[i];
#ifdef NVRAM_LOG
	logerror("nvram recall done\n");
#endif
}

// helper for dsp infifo_semaphore flag to make dealing with interrupts easier
static void dectalk_semaphore_w ( running_machine &machine, UINT16 data )
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	state->m_infifo_semaphore = data&1;
	if ((state->m_infifo_semaphore == 1) && (state->m_m68k_spcflags_latch&0x40))
	{
#ifdef VERBOSE
		logerror("speech int fired!\n");
#endif
		machine.device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	}
	else
	machine.device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_5, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

// read the output fifo and set the interrupt line active on the dsp
static UINT16 dectalk_outfifo_r ( running_machine &machine )
{
	dectalk_state *state = machine.driver_data<dectalk_state>();
	UINT16 data = 0xFFFF;
	data = state->m_outfifo[state->m_outfifo_tail_ptr];
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	//if (state->m_outfifo_tail_ptr != state->m_outfifo_head_ptr) state->m_outfifo_tail_ptr++; // technically correct but doesn't match sn74ls224 sheet
	if (((state->m_outfifo_head_ptr-1)&0xF) != state->m_outfifo_tail_ptr) state->m_outfifo_tail_ptr++; // matches sn74ls224 sheet
	state->m_outfifo_tail_ptr&=0xF;
	dectalk_outfifo_check(machine);
	return ((data&0xfff0)^0x8000); // yes this is right, top bit is inverted and bottom 4 are ignored
	//return data; // not right but want to get it working first
}

/* Machine reset and friends: stuff that needs setting up which IS directly affected by reset */
static void dectalk_reset(device_t *device)
{
	dectalk_state *state = device->machine().driver_data<dectalk_state>();
	state->m_hack_self_test = 0; // hack
	// stuff that is DIRECTLY affected by the RESET line
	state->m_statusLED = 0; // clear status led latch
	dectalk_x2212_recall(device->machine()); // nvram recall
	state->m_m68k_spcflags_latch = 1; // initial status is speech reset(d0) active and spc int(d6) disabled
	state->m_m68k_tlcflags_latch = 0; // initial status is tone detect int(d6) off, answer phone(d8) off, ring detect int(d14) off
	devtag_reset(device->machine(), "duart68681"); // reset the DUART
	// stuff that is INDIRECTLY affected by the RESET line
	dectalk_clear_all_fifos(device->machine()); // speech reset clears the fifos, though we have to do it explicitly here since we're not actually in the m68k_spcflags_w function.
	dectalk_semaphore_w(device->machine(), 0); // on the original state->m_dectalk pcb revision, this is a semaphore for the INPUT fifo, later dec hacked on a check for the 3 output fifo chips to see if they're in sync, and set both of these latches if true.
	state->m_spc_error_latch = 0; // spc error latch is cleared on /reset
	device->machine().device("dsp")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // speech reset forces the CLR line active on the tms32010
	state->m_tlc_tonedetect = 0; // TODO, needed for selftest pass
	state->m_tlc_ringdetect = 0; // TODO
	state->m_tlc_dtmf = 0; // TODO
	state->m_duart_inport = 0xF;
	state->m_duart_outport = 0;
}

static MACHINE_RESET( dectalk )
{
	/* hook the RESET line, which resets a slew of other components */
	m68k_set_reset_callback(machine.device("maincpu"), dectalk_reset);
}

/* Begin 68k i/o handlers */
READ8_MEMBER(dectalk_state::nvram_read)// read from x2212 nvram chip and possibly do recall
{
	UINT8 data = 0xFF;
	data = m_x2214_sram[offset&0xff]; // TODO: should this be before or after a possible /RECALL? I'm guessing before.
#ifdef NVRAM_LOG
		logerror("m68k: nvram read at %08X: %02X\n", offset, data);
#endif
	if (offset&0x200) // if a9 is set, do a /RECALL
	dectalk_x2212_recall(machine());
	return data;
}

WRITE8_MEMBER(dectalk_state::led_write)
{
	m_statusLED = data&0xFF;
	popmessage("LED status: %02X\n", data&0xFF);
#ifdef VERBOSE
	logerror("m68k: LED status: %02X\n", data&0xFF);
#endif
	//popmessage("LED status: %x %x %x %x %x %x %x %x\n", data&0x80, data&0x40, data&0x20, data&0x10, data&0x8, data&0x4, data&0x2, data&0x1);
}

WRITE8_MEMBER(dectalk_state::nvram_write)// write to X2212 NVRAM chip and possibly do store
{
#ifdef NVRAM_LOG
	logerror("m68k: nvram write at %08X: %02X\n", offset, data&0x0f);
#endif
	m_x2214_sram[offset&0xff] = (UINT8)data&0x0f; // TODO: should this be before or after a possible /STORE? I'm guessing before.
	if (offset&0x200) // if a9 is set, do a /STORE
	dectalk_x2212_store(machine());
}

WRITE16_MEMBER(dectalk_state::m68k_infifo_w)// 68k write to the speech input fifo
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_68K
	logerror("m68k: SPC infifo written with data = %04X, fifo head was: %02X; fifo tail: %02X\n",data, m_infifo_head_ptr, m_infifo_tail_ptr);
#endif
	// if fifo is full (head ptr = tail ptr-1), do not increment the head ptr and do not store the data
	if (((m_infifo_tail_ptr-1)&0x1F) == m_infifo_head_ptr)
	{
#ifdef SPC_LOG_68K
		logerror("infifo was full, write ignored!\n");
#endif
		return;
	}
	m_infifo[m_infifo_head_ptr] = data;
	m_infifo_head_ptr++;
	m_infifo_head_ptr&=0x1F;
}

READ16_MEMBER(dectalk_state::m68k_spcflags_r)// 68k read from the speech flags
{
	UINT8 data = 0;
	data |= m_m68k_spcflags_latch; // bits 0 and 6
	data |= m_spc_error_latch<<5; // bit 5
	data |= m_infifo_semaphore<<7; // bit 7
#ifdef SPC_LOG_68K
	logerror("m68k: SPC flags read, returning data = %04X\n",data);
#endif
	return data;
}

WRITE16_MEMBER(dectalk_state::m68k_spcflags_w)// 68k write to the speech flags (only 3 bits do anything)
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_68K
	logerror("m68k: SPC flags written with %04X, only storing %04X\n",data, data&0x41);
#endif
	m_m68k_spcflags_latch = data&0x41; // ONLY store bits 6 and 0!
	// d0: initialize speech flag (reset tms32010 and clear infifo and outfifo if high)
	if ((data&0x1) == 0x1) // bit 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x01: initialize speech: fifos reset, clear error+semaphore latches and dsp reset\n");
#endif
		dectalk_clear_all_fifos(machine());
		machine().device("dsp")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // speech reset forces the CLR line active on the tms32010
		// clear the two speech side latches
		m_spc_error_latch = 0;
		dectalk_semaphore_w(machine(), 0);
	}
	else // (data&0x1) == 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x01 = 0: initialize speech off, dsp running\n");
#endif
		machine().device("dsp")->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE); // speech reset deassert clears the CLR line on the tms32010
	}
	if ((data&0x2) == 0x2) // bit 1 - clear error and semaphore latches
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x02: clear error+semaphore latches\n");
#endif
		// clear the two speech side latches
		m_spc_error_latch = 0;
		dectalk_semaphore_w(machine(), 0);
	}
	if ((data&0x40) == 0x40) // bit 6 - spc irq enable
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x40: speech int enabled\n");
#endif
		if (m_infifo_semaphore == 1)
		{
#ifdef SPC_LOG_68K
			logerror("    speech int fired!\n");
#endif
			machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because semaphore was set
		}
	}
	else // data&0x40 == 0
	{
#ifdef SPC_LOG_68K
		logerror(" | 0x40 = 0: speech int disabled\n");
#endif
		machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_5, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
}

READ16_MEMBER(dectalk_state::m68k_tlcflags_r)// dtmf flags read
{
	UINT16 data = 0;
	data |= m_m68k_tlcflags_latch; // bits 6, 8, 14;
	data |= m_tlc_tonedetect<<7; // bit 7 is tone detect
	data |= m_tlc_ringdetect<<14; // bit 15 is ring detect
#ifdef TLC_LOG
	logerror("m68k: TLC flags read, returning data = %04X\n",data);
#endif
	return data;
}

WRITE16_MEMBER(dectalk_state::m68k_tlcflags_w)// dtmf flags write
{
#ifdef TLC_LOG
	logerror("m68k: TLC flags written with %04X, only storing %04X\n",data, data&0x4140);
#endif
	m_m68k_tlcflags_latch = data&0x4140; // ONLY store bits 6 8 and 14!
	if ((data&0x40) == 0x40) // bit 6: tone detect interrupt enable
	{
#ifdef TLC_LOG
		logerror(" | 0x40: tone detect int enabled\n");
#endif
		if (m_tlc_tonedetect == 1)
		{
#ifdef TLC_LOG
			logerror("    TLC int fired!\n");
#endif
			machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because tone detect was set
		}
	}
	else // data&0x40 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x40 = 0: tone detect int disabled\n");
#endif
	if (((data&0x4000)!=0x4000) || (m_tlc_ringdetect == 0)) // check to be sure we don't disable int if both ints fired at once
		machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_4, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
	if ((data&0x100) == 0x100) // bit 8: answer phone relay enable
	{
#ifdef TLC_LOG
		logerror(" | 0x100: answer phone relay enabled\n");
#endif
	}
	else // data&0x100 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x100 = 0: answer phone relay disabled\n");
#endif
	}
	if ((data&0x4000) == 0x4000) // bit 14: ring int enable
	{
#ifdef TLC_LOG
		logerror(" | 0x4000: ring detect int enabled\n");
#endif
		if (m_tlc_ringdetect == 1)
		{
#ifdef TLC_LOG
			logerror("    TLC int fired!\n");
#endif
			machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR); // set int because tone detect was set
		}
	}
	else // data&0x4000 == 0
	{
#ifdef TLC_LOG
		logerror(" | 0x4000 = 0: ring detect int disabled\n");
#endif
	if (((data&0x40)!=0x40) || (m_tlc_tonedetect == 0)) // check to be sure we don't disable int if both ints fired at once
		machine().device("maincpu")->execute().set_input_line_and_vector(M68K_IRQ_4, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR); // clear int because int is now disabled
	}
}

READ16_MEMBER(dectalk_state::m68k_tlc_dtmf_r)// dtmf chip read
{
#ifdef TLC_LOG
	UINT16 data = 0xFFFF;
	data = m_tlc_dtmf&0xF;
	logerror("m68k: TLC dtmf detector read, returning data = %02X", data);
#endif
	return 0;
}
/* End 68k i/o handlers */

/* Begin tms32010 i/o handlers */
WRITE16_MEMBER(dectalk_state::spc_latch_outfifo_error_stats)// latch 74ls74 @ E64 upper and lower halves with d0 and 1 respectively
{
#ifdef USE_LOOSE_TIMING
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(25));
#endif
#ifdef SPC_LOG_DSP
	logerror("dsp: set fifo semaphore and set error status = %01X\n",data&1);
#endif
	dectalk_semaphore_w(machine(), (~m_simulate_outfifo_error)&1); // always set to 1 here, unless outfifo error.
	m_spc_error_latch = (data&1);
}

READ16_MEMBER(dectalk_state::spc_infifo_data_r)
{
	UINT16 data = 0xFFFF;
	data = m_infifo[m_infifo_tail_ptr];
#ifdef SPC_LOG_DSP
	logerror("dsp: SPC infifo read with data = %04X, fifo head: %02X; fifo tail was: %02X\n",data, m_infifo_head_ptr, m_infifo_tail_ptr);
#endif
	// if fifo is empty (tail ptr == head ptr), do not increment the tail ptr, otherwise do.
	if (m_infifo_tail_ptr != m_infifo_head_ptr) m_infifo_tail_ptr++; // technically correct but doesn't match sn74ls224 sheet
	//if (((m_infifo_head_ptr-1)&0x1F) != m_infifo_tail_ptr) m_infifo_tail_ptr++; // matches sn74ls224 sheet
	m_infifo_tail_ptr&=0x1F;
	return data;
}

WRITE16_MEMBER(dectalk_state::spc_outfifo_data_w)
{
	// the low 4 data bits are thrown out on the real unit due to use of a 12 bit dac (and to save use of another 16x4 fifo chip), though technically they're probably valid, and with suitable hacking a dtc-01 could probably output full 16 bit samples at 10khz.
#ifdef SPC_LOG_DSP
	logerror("dsp: SPC outfifo write, data = %04X, fifo head was: %02X; fifo tail: %02X\n", data, m_outfifo_head_ptr, m_outfifo_tail_ptr);
#endif
	machine().device("dsp")->execute().set_input_line(0, CLEAR_LINE); //TMS32010 INT (cleared because LDCK inverts the IR line, clearing int on any outfifo write... for a moment at least.)
	// if fifo is full (head ptr = tail ptr-1), do not increment the head ptr and do not store the data
	if (((m_outfifo_tail_ptr-1)&0xF) == m_outfifo_head_ptr)
	{
#ifdef SPC_LOG_DSP
		logerror("outfifo was full, write ignored!\n");
#endif
		return;
	}
	m_outfifo[m_outfifo_head_ptr] = data;
	m_outfifo_head_ptr++;
	m_outfifo_head_ptr&=0xF;
	//dectalk_outfifo_check(machine()); // commented to allow int to clear
}

READ16_MEMBER(dectalk_state::spc_semaphore_r)// Return state of d-latch 74ls74 @ E64 'lower half' in d0 which indicates whether infifo is readable
{
#ifdef SPC_LOG_DSP
	//logerror("dsp: read infifo semaphore, returned %d\n", m_infifo_semaphore); // commented due to extreme annoyance factor
	if (!m_infifo_semaphore) logerror("dsp: read infifo semaphore, returned %d\n", m_infifo_semaphore);
#endif
	return m_infifo_semaphore;
}
/* end tms32010 i/o handlers */


/******************************************************************************
 Address Maps
******************************************************************************/
/*
Address maps (x = ignored; * = selects address within this range)
68k address map:
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
0   x   x   x   0   x   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROM
0   x   x   x   1   x   x   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  RAM (first 4 chip pairs)
0   x   x   x   1   x   x   1   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  RAM (last chip pair)
0   x   x   x   1   x   x   1   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x   0       W   Status LED <d7-d0>
0   x   x   x   1   x   x   1   0   1   x   x   x   x   0   *   *   *   *   *   *   *   *   1       RW  NVRAM (read/write volatile ram, does not store to eeprom)
0   x   x   x   1   x   x   1   0   1   x   x   x   x   1   *   *   *   *   *   *   *   *   1       RW  NVRAM (all reads do /recall from eeprom, all writes do /store to eeprom)
0   x   x   x   1   x   x   1   1   0   x   x   x   x   x   x   x   x   x   *   *   *   *   x       RW  DUART (keep in mind that a0 is not connected)
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   0   0   *       RW  SPC flags: fifo writable (readonly, d7), spc irq suppress (readwrite, d6), fifo error status (readonly, d5), 'fifo release'/clear-tms-fifo-error-status-bits (writeonly, d1), speech initialize/clear (readwrite, d0) [see schematic sheet 4]
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   0   1   0?      W   SPC fifo write (clocks fifo)
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   1   0   *       RW  TLC flags: ring detect (readonly, d15), ring detected irq enable (readwrite, d14), answer phone (readwrite, d8), tone detected (readonly, d7), tone detected irq enable (readwrite, d6) [see schematic sheet 6]
0   x   x   x   1   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   1   1   *       R   TLC tone chip read, reads on bits d0-d7 only, d4-d7 are tied low; d15-d8 are probably open bus
              |               |               |               |               |
*/

static ADDRESS_MAP_START(m68k_mem, AS_PROGRAM, 16, dectalk_state )
    ADDRESS_MAP_UNMAP_HIGH
    AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_MIRROR(0x740000) /* ROM */
    AM_RANGE(0x080000, 0x093fff) AM_RAM AM_MIRROR(0x760000) /* RAM */
    //AM_RANGE(0x094000, 0x0943ff) AM_READWRITE_LEGACY(led_sw_nvr_read, led_sw_nv_write) AM_MIRROR(0x763C00) /* LED array and Xicor X2212 NVRAM */
    AM_RANGE(0x094000, 0x0943ff) AM_WRITE8(led_write, 0x00FF) AM_MIRROR(0x763C00) /* LED array */
    AM_RANGE(0x094000, 0x0943ff) AM_READWRITE8(nvram_read, nvram_write, 0xFF00) AM_MIRROR(0x763C00) /* Xicor X2212 NVRAM */
    AM_RANGE(0x098000, 0x09801f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff) AM_MIRROR(0x763FE0) /* DUART */
    AM_RANGE(0x09C000, 0x09C001) AM_READWRITE(m68k_spcflags_r, m68k_spcflags_w) AM_MIRROR(0x763FF8) /* SPC flags reg */
    AM_RANGE(0x09C002, 0x09C003) AM_WRITE(m68k_infifo_w) AM_MIRROR(0x763FF8) /* SPC fifo reg */
    AM_RANGE(0x09C004, 0x09C005) AM_READWRITE(m68k_tlcflags_r, m68k_tlcflags_w) AM_MIRROR(0x763FF8) /* telephone status flags */
    AM_RANGE(0x09C006, 0x09C007) AM_READ(m68k_tlc_dtmf_r) AM_MIRROR(0x763FF8) /* telephone dtmf read */
ADDRESS_MAP_END

// do we even need this below?
static ADDRESS_MAP_START(m68k_io, AS_IO, 16, dectalk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms32010_mem, AS_PROGRAM, 16, dectalk_state )
    AM_RANGE(0x000, 0x7ff) AM_ROM /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms32010_io, AS_IO, 16, dectalk_state )
    AM_RANGE(0, 0) AM_WRITE(spc_latch_outfifo_error_stats) // *set* the outfifo_status_r semaphore, and also latch the error bit at D0.
    AM_RANGE(1, 1) AM_READWRITE(spc_infifo_data_r, spc_outfifo_data_w) //read from input fifo, write to sound fifo
    AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(spc_semaphore_r) //read output fifo writable status
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( dectalk )
PORT_START("duart_in") // IP4, IP5, IP6 bits on duart are dipswitches (really unsoldered holes in the pcb where jumper wires can be soldered or shorted over)
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // these bits are dealt with elsewhere
	PORT_DIPNAME( 0x10, 0x00, "Skip Self Test (IP4)" )
	PORT_DIPSETTING(    0x10, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown (IP5)" )
	PORT_DIPSETTING(    0x20, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown (IP6)" )
	PORT_DIPSETTING(    0x40, "Open (VCC)" )
	PORT_DIPSETTING(    0x00, "Short to GND" )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // this pin (IP7) doesn't actually exist as a pin at all, reads as 1

PORT_START("hacks")
	PORT_CONFNAME( 0x01, 0x01, "Hack to prevent hang when skip self test is shorted" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
static TIMER_CALLBACK( outfifo_read_cb )
{
	UINT16 data;
	dac_device *speaker = machine.device<dac_device>("dac");
	data = dectalk_outfifo_r(machine);
#ifdef VERBOSE
	if (data!= 0x8000) logerror("sample output: %04X\n", data);
#endif
	machine.scheduler().timer_set(attotime::from_hz(10000), FUNC(outfifo_read_cb));
	speaker->write_signed16(data);
}

/* Driver init: stuff that needs setting up which isn't directly affected by reset */
DRIVER_INIT_MEMBER(dectalk_state,dectalk)
{
	dectalk_clear_all_fifos(machine());
	m_simulate_outfifo_error = 0;
	machine().scheduler().timer_set(attotime::from_hz(10000), FUNC(outfifo_read_cb));
}

static WRITE8_DEVICE_HANDLER( dectalk_kbd_put )
{
	duart68681_rx_data(device->machine().device("duart68681"), 1, data);
}

static GENERIC_TERMINAL_INTERFACE( dectalk_terminal_intf )
{
	DEVCB_HANDLER(dectalk_kbd_put)
};

static MACHINE_CONFIG_START( dectalk, dectalk_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2) /* E74 20MHz OSC (/2) */
    MCFG_CPU_PROGRAM_MAP(m68k_mem)
    MCFG_CPU_IO_MAP(m68k_io)
    MCFG_MACHINE_RESET(dectalk)
    MCFG_DUART68681_ADD( "duart68681", XTAL_3_6864MHz, dectalk_duart68681_config ) /* Y3 3.6864MHz Xtal */


    MCFG_CPU_ADD("dsp", TMS32010, XTAL_20MHz) /* Y1 20MHz xtal */
    MCFG_CPU_PROGRAM_MAP(tms32010_mem)
    MCFG_CPU_IO_MAP(tms32010_io)
#ifdef USE_LOOSE_TIMING
    MCFG_QUANTUM_TIME(attotime::from_hz(100))
#else
    MCFG_QUANTUM_PERFECT_CPU("dsp")
#endif

    //MCFG_NVRAM_ADD_0FILL("nvram")

    /* video hardware */
    //MCFG_DEFAULT_LAYOUT(layout_dectalk) // hack to avoid screenless system crash

    /* sound hardware */
    MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_SOUND_ADD("dac", DAC, 0) /* E88 10KHz OSC, handled by timer */
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.9)

    /* Y2 is a 3.579545 MHz xtal for the dtmf decoder chip */

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG,dectalk_terminal_intf)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( dectalk )
	ROM_REGION16_BE(0x40000,"maincpu", 0)
	// dectalk dtc-01 firmware v2.0 (first half: 23Jul84 tag; second half: 02Jul84 tag), all roms are 27128 eproms
	// technically the correct rom names are probably 23-123e5.e8, etc, but the chips they were dumped from were NOT labeled that way
	ROM_LOAD16_BYTE("sp8510123e5.e8", 0x00000, 0x4000, CRC(03e1eefa) SHA1(e586de03e113683c2534fca1f3f40ba391193044)) // Label: "SP8510123E5" @ E8
	ROM_LOAD16_BYTE("sp8510119e5.e22", 0x00001, 0x4000, CRC(af20411f) SHA1(7954bb56b7591f8954403a22d34de31c7d5441ac)) // Label: "SP8510119E5" @ E22
	ROM_LOAD16_BYTE("sp8510124e5.e7", 0x08000, 0x4000, CRC(9edeafcb) SHA1(7724babf4ae5d77c0b4200f608d599058d04b25c)) // Label: "SP8510124E5" @ E7
	ROM_LOAD16_BYTE("sp8510120e5.e21", 0x08001, 0x4000, CRC(f2a346a6) SHA1(af5e4ea0b3631f7d6f16c22e86a33fa2cb520ee0)) // Label: "SP8510120E5" @ E21
	ROM_LOAD16_BYTE("sp8510125e5.e6", 0x10000, 0x4000, CRC(1c0100d1) SHA1(1b60cd71dfa83408b17e13f683b6bf3198c905cc)) // Label: "SP8510125E5" @ E6
	ROM_LOAD16_BYTE("sp8510121e5.e20", 0x10001, 0x4000, CRC(4cb081bd) SHA1(4ad0b00628a90085cd7c78a354256c39fd14db6c)) // Label: "SP8510121E5" @ E20
	ROM_LOAD16_BYTE("sp8510126e5.e5", 0x18000, 0x4000, CRC(7823dedb) SHA1(e2b2415eec838ddd46094f2fea93fd289dd0caa2)) // Label: "SP8510126E5" @ E5
	ROM_LOAD16_BYTE("sp8510122e5.e19", 0x18001, 0x4000, CRC(b86370e6) SHA1(92ab22a24484ad0d0f5c8a07347105509999f3ee)) // Label: "SP8510122E5" @ E19
	ROM_LOAD16_BYTE("sp8510103e5.e4", 0x20000, 0x4000, CRC(35aac6b9) SHA1(b5aec0bf37a176ff4d66d6a10357715957662ebd)) // Label: "SP8510103E5" @ E4
	ROM_LOAD16_BYTE("sp8510095e5.e18", 0x20001, 0x4000, CRC(2296fe39) SHA1(891f3a3b4ce75ef14001257bc8f1f60463a9a7cb)) // Label: "SP8510095E5" @ E18
	ROM_LOAD16_BYTE("sp8510104e5.e3", 0x28000, 0x4000, CRC(9658b43c) SHA1(4d6808f67cbdd316df23adc8ddf701df57aa854a)) // Label: "SP8510104E5" @ E3
	ROM_LOAD16_BYTE("sp8510096e5.e17", 0x28001, 0x4000, CRC(cf236077) SHA1(496c69e52cfa013173f7b9c500ce544a03ad01f7)) // Label: "SP8510096E5" @ E17
	ROM_LOAD16_BYTE("sp8510105e5.e2", 0x30000, 0x4000, CRC(09cddd28) SHA1(de0c25687bab3ff0c88c98622092e0b58331aa16)) // Label: "SP8510105E5" @ E2
	ROM_LOAD16_BYTE("sp8510097e5.e16", 0x30001, 0x4000, CRC(49434da1) SHA1(c450abae0ccf372d7eb87370b8a8c97a45e164d3)) // Label: "SP8510097E5" @ E16
	ROM_LOAD16_BYTE("sp8510106e5.e1", 0x38000, 0x4000, CRC(a389ab31) SHA1(355348bfc96a04193136cdde3418366e6476c3ca)) // Label: "SP8510106E5" @ E1
	ROM_LOAD16_BYTE("sp8510098e5.e15", 0x38001, 0x4000, CRC(3d8910e7) SHA1(01921e77b46c2d4845023605239c45ffa4a35872)) // Label: "SP8510098E5" @ E15

	ROM_REGION(0x2000,"dsp", 0)
	// dectalk dtc-01 'klsyn' tms32010 firmware v2.0, both proms are 82s191 equivalent
	ROM_LOAD16_BYTE("lm8506205f4.e70", 0x000, 0x800, CRC(ed76a3ad) SHA1(3136bae243ef48721e21c66fde70dab5fc3c21d0)) // Label: "LM8506205F4 // M1-76161-5" @ E70
	ROM_LOAD16_BYTE("lm8504204f4.e69", 0x001, 0x800, CRC(79bb54ff) SHA1(9409f90f7a397b041e4440341f2d7934cb479285)) // Label: "LM8504204F4 // 78S191" @ E69

	ROM_REGION(0x100,"nvram", 0) // default nvram image is at 0x1A7AE in main rom, read lsn first so 0x0005 in rom becomes 05 00 00 00 etc for all words of main rom
	ROM_FILL(0x00, 0xff, 0x00) // blank it first;
	ROM_FILL(0x00, 0x01, 0x05)
	ROM_FILL(0x04, 0x01, 0x00)
	ROM_FILL(0x08, 0x01, 0x06)
	ROM_FILL(0x0C, 0x01, 0x01)
	ROM_FILL(0x10, 0x01, 0x06)
	ROM_FILL(0x14, 0x01, 0x0B)
	ROM_FILL(0x18, 0x01, 0x02)
	ROM_FILL(0x1C, 0x01, 0x02)
	ROM_FILL(0x20, 0x01, 0x01)
	ROM_FILL(0x24, 0x01, 0x01)
	ROM_FILL(0x28, 0x01, 0x00)
	ROM_FILL(0x2c, 0x01, 0x01)
	ROM_FILL(0xFC, 0x01, 0x0D) // checksum, calculated some weird way which I haven't figured out yet
	ROM_FILL(0xFD, 0x01, 0x02) // "
	ROM_FILL(0xFE, 0x01, 0x05) // "
	ROM_FILL(0xFF, 0x01, 0x0B) // "

ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT      COMPANY     FULLNAME            FLAGS */
COMP( 1984, dectalk,	0,		0,		dectalk,	dectalk, dectalk_state,	dectalk,  "Digital Equipment Corporation",		"DECTalk DTC-01",	GAME_NOT_WORKING )
