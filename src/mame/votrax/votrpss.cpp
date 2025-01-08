// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Robbbert
/******************************************************************************
*
*  Votrax Personal Speech System Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with help from Kevin 'kevtris' Horton
*  Special thanks to Professor Nicholas Gessler for loaning several PSS units
*
*  The votrax PSS was sold from before the 35th week of 1982 until october, 1990

Main xtal is 8MHz
AY-3-8910 and i8253 clock is running at 2 MHz (xtal/4)
Z80A is running at 4MHz (xtal/2)
clock dividers also generate the system reset signals for the 8251 and and a periodic IRQ at 122Hz (xtal/65536)

I8253:
Timer0 = Baud Clock, not gated (constantly on)
Timer1 = output to transistor chopper on clock input to sc-01-a to control pitch; gated by xtal/256
Timer2 = output to transistor chopper on output of sc-01-a to control volume; gated by xtal/4096

I8255 ports:
PortA 0:7 = pins 16 thru 23 of parallel port
PortB 0:7 = pins 6 thru 13 of parallel port
PortC =
    0 = NC
    1 = GND
    2 = pin 5 of parallel port
    3 = /RXINTEN
    4 = pin 15 of parallel port
    5 = pin 14 of parallel port through inverter
    6 = ay-3-8910 enable (which pin? BC1?)
    7 = input from parallel port pin 4 through inverter

AY-3-8910 I/O ports:
    IOA is in output mode
        IOA0-A5 = phoneme #
        IOA6 = strobe (SC-01)
        IOA7 = vochord control, 0 = off, 1 = on
    IOB is in input mode
        IOB0-IOB7 = dip switches

I8251 UART:
    RESET is taken from the same inverter that resets the counters

Things to be looked at:
- volume and pitch should be controlled by ppi outputs
- pit to be hooked up
- bit 0 of portc is not connected according to text above, but it
  completely changes the irq operation.

Notes:
- When Serial dip is chosen, you type the commands in, but you cannot see anything.
  If you enter text via parallel, it is echoed but otherwise ignored.
- When Parallel dip is chosen, you type the commands in, but again you cannot
  see anything.
- These operations ARE BY DESIGN. Everything is working correctly.
- Commands are case-sensitive.
- Some tests...
  - Say the time: EscT. (include the period)
  - Play some notes: !T08:1234:125:129:125:130. (then press enter)
- It is known from testing that the PSS will not work with a CMOS z80.
  - It is believed this has to do with the ram test code on startup expecting
    to read open bus as FF instead of whatever was last written by the z80,
    but this needs further code tracing/verification.
    (If this is actually the case, it can probably be worked around by pulling
    the data bus lines high with resistors)

******************************************************************************/

/*****************************************************************************
Notes about main PCB revisions:

PCB 1871G: (1982 or earlier?)
  has extensive rework around the 74ls02 (see below for before and after rework)
  manually added 3 resistors (brown black yellow gold) between gnd and pins 3,5, and vcc and pin 1 of resistor array
  manually added one resistor (yellow purple red gold) between pin 13 (connected to parallel port pin 16) and 26(gnd) of 8255
  Heatsink rivets are separated from pcb by two plastic washers, trace runs underneath and would short w/o washers
  Power plug has shield connected to board GND
  Chip behind serial port is an hcf4049 hex buffer
  74LS04 near xtal has pin 3 unconnected?
  hcf4049 behind serial port has pin 9 grounded
  diode piggybacking on resistor above 8255

PCB 1871H: (1982)
  74ls02 fixes from 1871G integrated on PCB (see below)
  extra array resistor integrated to replace manually added 3 resistors on other resistor array. hookup is basically the same
  resistor between pins 13 and 26 of 8255 integrated on pcb
  Heatsink rivets directly on pcb, no washers. trace has been rerouted so as not to short.
  power plug has shield connected to board gnd BUT was manually milled off so is connected to nothing.
  chip behind serial port is an 74C14/CD40106 hex schmitt trigger
  hookup of 74ls04 near xtal has pin 3 connected to pin 8 of schmitt trigger
  hookup of mc14106/hcf4049 and the 74ls04 has significantly changed? (see below)
  diode above 8255 is now only half piggybacking on resistor, now has a true attachment point on pcb for other end

PCB 1871J: (1987?)
  power plug shield no longer connects to gnd plane on board
  chip behind serial port is an mc14106 hex schmitt trigger (trivial change)

***REWORK on pcb 1871G:
*Before rework:
Ay-3-8910 connections:
pin 29 (BC1)  <->  pin 12 (C/D) of 8251

"74ls02" (clearly a ?74ls00? originally) connections:
pin 1 (Y1)->  pin 14 (PC0) of 8255
pin 2 (A1)<-  GND
pin 3 (B1)<-  GND
pin 4 (Y2)->  pin 7 of left edge of potted block, and to pin 24 (/A9) of ay-3-8910
pin 5 (A2)<-  pin 19 of left edge of potted block and pin 23 (/WR) of 8253
pin 6 (B2)<-  pin 27 (BDIR) of ay-3-8910
pin 7   GND
pin 8 (Y3)-> pin 1 (A1) of 74ls04 near xtal
pin 9 (A3)<-  8251 pin 14 (RxRDY)
pin 10 (B3)<- GND
pin 11 (Y4)-> pin 17 (PC3) of 8255
pin 12 (A4)<- pin 11 of left edge of potted block
pin 13 (B4)<- pin 2 (/Y1) of 74ls04 near xtal
pin 14  VCC

resistor array (08-2-393 // 8042) connections:
pin 1: serial port pin 11
pin 2: hcf4049 pin 5 (A2)
pin 3: serial port pin 9(and 6?)
pin 4: hcf4049 pin 3 (A1)
pin 5: serial port pin 6(and 9?)
pin 6: hcf4049 pin 7 (A3)
pin 7: N/C

Parallel port pin 16 is connected to pin 13 on 8255

*After rework:
Ay-3-8910 connections:
pin 29 (BC1)  <-> 74ls02 pin 13

74ls02 connections:
pin 1 (Y1)->  pin 11 of left edge of potted block
pin 2 (A1)<-  pin 14 (PC0) of 8255
pin 3 (B1)<-  pin 2 (/Y1) of 74ls04 near xtal
pin 4 (Y2)->  pin 27 (BDIR) of ay-3-8910
pin 5 (A2)<-  pin 19 of left edge of potted block and pin 23 (/WR) of 8253
pin 6 (B2)<-  74ls02 pin 11 and pin 7 of left edge of potted block, and to pin 24 (/A9) of ay-3-8910
pin 7   GND
pin 8 (A3)<-  pin 17 (PC3) of 8255
pin 9 (B3)<-  8251 pin 14 (RxRDY)
pin 10 (Y3)-> pin 1 (A1) of 74ls04 near xtal
pin 11 (A4)<- 74ls02 pin 7 and pin 7 of left edge of potted block, and to pin 24 (/A9) of ay-3-8910
pin 12 (B4)<- 8251 pin 12 (C/D)
pin 13 (Y4)-> pin 29 (BC1) of ay-3-8910
pin 14  VCC

resistor array (08-2-393 // 8042) connections:
pin 1: serial port pin 11 and 100KOhm resistor to GND
pin 2: hcf4049 pin 5 (A2)
pin 3: serial port pin 9(and 6?) and 100KOhm resistor to VCC
pin 4: hcf4049 pin 3 (A1)
pin 5: serial port pin 6(and 9?) and 100KOhm resistor to VCC
pin 6: hcf4049 pin 7 (A3)
pin 7: N/C

Parallel port pin 16 is connected to pin 13 on 8255 and 4.7KOhm resistor to 8255 pin 26 (GND)
*****************************************************************************/

/*****************************************************************************
Notes about the (sometimes) potted 1872/1872C CPU board:

top/solder side:

^       ^       1        ^                      40
|       |       2        |                      39
74LS74  74LS139 3        |      ^       ^       38
|       |       4        |      |       |       37
|       |       5        |      |       |       36
|       |       6        |      |       |       35
V       V       7        |      |       |       34
                8        |      |       |       33
                9       z80     |       |       32
                10       |      2764    2764    31
                11       |      rom1    rom2    30
                12       |      |       |       29
^       ^       13       |      |       |       28
|       |       14       |      |       |       27
74LS02  74LS367 15       |      V       v       26
|       |       16       |                      25
|       |       17       |                      24
|       |       18       |                      23
|       |       19       |                      22
V       V       20       v                      21


<- = input to device in question or to cpu module
-> = output
<-> = both input and output

Pinout:
pin     purpose         connects to, on cpu board
1       A13<->          from z80 pin 3->(A13), to 74ls02 pin 9<-(B3) (and out pin 10->(Y3) to eprom 1 pin 20(/CE)), to eprom 2 pin 20<-(/CE)
2       8255 /EN->      from 74ls139 pin 4->(/O0a)
3       USER ROM /EN->  from 74ls139 pin 9->(/O3b)
4       8251 /EN->      from 74ls139 pin 5->(/O1a)
5       RAM /EN ->      from 74ls139 pin 10->(/O2b)
6       8253 /EN->      from 74ls139 pin 6->(/O2a)
7       ay-3-8910 /EN-> from 74ls139 pin 7->(/O3a)
8       A2<->           from z80 pin 32->(A2), eproms pin 8<-(A2)
9       A1<->           from z80 pin 31->(A1), eproms pin 9<-(A1)
10      A0<->           from z80 pin 30->(A0), eproms pin 10<-(A0)
11      /INT<-          to z80 pin 16<-(/INT)
12      EXP_POLL<-      to 74ls74 pin 3<-(CLK1)
13      EXP_/EN->       from 74ls74 pin 5->(Q1)
14      /M1<->          from/to z80 pin 27->(/M1), 74ls02 pin 12<-(B4)
15      CLK<-           to 74ls367 pin 2<-(A1a)
16      /RESET<-        to 74ls367 pin 14<-(A2b)
17      /MREQ<->        from/to 74ls139 pin 15<-(/Eb), 74ls02 pin 11<-(A4), 74ls367 pin 5->(Y2a),
18      /IORQ<->        from/to 74ls139 pin 1<-(/Ea), 74ls74 pin 10<-(/PR2), 74ls367 pin 11->(Y1b)
19      Buffered /WR->  from 74ls367 pin 7->(Y3a)
20      Buffered /RD->  from 74ls367 pin 9->(Y4a)
21      D3<->           from/to z80 pin 8<->(D3), eproms pin 15->(D3)
22      D4<->           from/to z80 pin 7<->(D4), eproms pin 16->(D4)
23      D5<->           from/to z80 pin 9<->(D5), eproms pin 17->(D5)
24      D6<->           from/to z80 pin 10<->(D6), eproms pin 18->(D6)
25      D7<->           from/to z80 pin 13<->(D7), eproms pin 19->(D7)
26      GND             everything
27      A10->           from z80 pin 40->(A10), eproms pin 21<-(A10)
28      D2<->           from/to z80 pin 12<->(D2), eproms pin 13->(D2)
29      D1<->           from/to z80 pin 15<->(D1), eproms pin 12->(D1)
30      D0<->           from/to z80 pin 14<->(D0), eproms pin 11->(D0)
31      A6<->           from z80 pin 36->(A6), to eproms pin 4<-(A6), to 74ls139 pin 2<-(A0a)
32      A5<->           from z80 pin 35->(A5), eproms pin 5<-(A5)
33      A3<->           from z80 pin 33->(A3), eproms pin 7<-(A3)
34      A4<->           from z80 pin 34->(A4), eproms pin 6<-(A4)
35      A11->           from z80 pin 1->(A11), eproms pin 23<-(A11)
36      A9<->           from z80 pin 39->(A9), eproms pin 24<-(A9)
37      A8<->           from z80 pin 38->(A8), eproms pin 25<-(A8)
38      A7<->           from z80 pin 37->(A7), to eproms pin 3<-(A7), to 74ls139 pin 3<-(A1a)
39      A12<->          from z80 pin 2->(A12), eproms pin 2<-(A12)
40      VCC             everything

EXP_POLL clocks the flipflop 1 of the 74ls74
if 74ls139 pin 11 is LOW(0x4000-7fff address space is being accessed) , EXP_/EN will return 0
if 74ls139 pin 11 is HIGH(any other memory space is being accessed other than 0x4000-0x7fff), EXP_/EN will return 1


74ls02 quad 2-input NOR pinout:
1       Y1->    to 74ls02 pin 5<-(A2)
2       A1<-    from 74ls139 pin 12->(/O0b)
3       B1<-    from 74ls74 pin 9->(Q2)
4       Y2->    to eproms pin 22<-(/OE)
5       A2<-    from 74ls02 pin 1->(Y1)
6       B2<-    tied to GND
7       GND     GND
8       A3<-    tied to GND
9       B3<-    from z80 pin 3->(A13) AND to eprom 2 pin 20<-(/CE)
10      Y3->    to eprom 1 pin 20<-(/CE)
11      A4<-    to/from main board(module pin 17) AND to 74ls139 pin 15<-(/Eb) AND from 74ls367 pin 5->(Y2a)
12      B4<-    from z80 pin 27->(/M1) AND to main board (module pin 14)
13      Y4->    to 74ls74 pin 11<-(/CLK2)
14      VCC     VCC


74ls139 dual 1-of-4 decoder/demultiplexer pinout:
1       /Ea<-   to/from main board (module pin 18)<-> AND to 74ls74 pin 10<-(/PR2) AND from 74ls367 pin 11->(Y1b)
2       A0a<-   to/from main board (module pin 31)<-> AND from z80 pin 36->(A6) AND to eproms pin 4<-
3       A1a<-   to/from main board (module pin 38)<-> AND from z80 pin 37->(A7) AND to eproms pin 3<-
4       /O0a->  to main board (module pin 2, /0x**00 8255 /EN)
5       /O1a->  to main board (module pin 4, /0x**40 8251 /EN)
6       /O2a->  to main board (module pin 6, /0x**80 8253 /EN)
7       /O3a->  to main board (module pin 7, /0x**C0 ay-3-8910 /EN)
8       GND     GND
9       /O3b->  to main board (module pin 3, USER eprom /EN)
10      /O2b->  to main board (module pin 5, RAM /EN)
11      /O1b->  to 74ls74 pin 1<-(/CLR1) (Expansion /EN)
12      /O0b->  to 74ls02 pin 2<-(A1) (MODULE EPROMS /OE, but ONLY if 74ls02 pin 3(B1), which comes from 74ls74 pin 9(Q2), is LOW)
13      A1b<-   from z80 pin 5->(A15) and to 74ls74 pin 12<-(D2)
14      A0b<-   from z80 pin 4->(A14)
15      /Eb<-   to/from main board (module pin 17) AND from 74ls367 pin 5->(Y2a) AND to 74ls02 pin 11<-(A4)
16      VCC     VCC


74ls367 hex 3-state buffer with separate 2-bit and 4 bit sections pinout:
1       /Ea<-   tied to GND
2       A1a<-   from main board (module pin 15)
3       Y1a->   to z80 pin 6<-(CLK)
4       A2a<-   from z80 pin 19->(/MREQ)
5       Y2a->   to/from main board (module pin 17) AND to 74ls139 pin 15<-(/Eb) AND to 74ls02 pin 11<-(A4)
6       A3a<-   from z80 pin 22->(/WR)
7       Y3a->   to main board (module pin 19)
8       GND     GND
9       Y4a->   to main board (module pin 20)
10      A4a<-   from z80 pin 21->(/RD)
11      Y1b->   to/from main board (module pin 18)<-> AND to 74ls139 pin 1<-(/Ea) AND to 74ls74 pin 10<-(/PR2)
12      A1b<-   from z80 pin 20->(/IORQ)
13      Y2b->   to z80 pin 26<-(/RESET)
14      A2b<-   from main board (module pin 16)
15      /Eb<-   tied to GND
16      VCC     VCC


74ls74 dual positive-edge-triggered flip flops with preset, clear and complementary outputs pinout:
1       /CLR1<- from 74ls139 pin 11->(/O1b)
2       D1<-    tied to VCC (and 74ls74 pin 4<-(/PR1))
3       CLK1<-  from main board (module pin 12)
4       /PR1<-  tied to VCC (and 74ls74 pin 2<-(D1))
5       Q1->    to main board (module pin 13)
6       /Q1->   N/C
7       GND     GND
8       /Q2->   N/C
9       Q2->    to 74ls02 pin 3<-(B1)
10      /PR2<-  to/from main board (module pin 18)<-> AND to 74ls139 pin 1<-(/Ea) AND from 74ls367 pin 11->(Y1b)
11      CLK2<-  from 74ls02 pin 13->(Y4)
12      D2<-    from z80 pin 5->(A15) AND to 74ls139 pin 13<-(A1b)
13      /CLR2<- tied to VCC
14      VCC     VCC


z80 pinout:
1       A11->    to eproms and module bus
2       A12->    to eproms and module bus
3       A13->    to /CE on eprom 2 AND through 74ls02 pins 9 and 10 (acting as an inverter) to /CE on eprom 1 AND to module bus
4       A14->    to 74ls139 pin 14<-(A0b)
5       A15->    to 74ls74 pin 12<-(D2) AND to 74ls139 pin 13<-(A1b)
6       CLK<-    from 74ls367 pin 3->(Y1a) AND pulled to VCC by (orange orange red gold)Ohm resistor
7       D4<->    to eproms and module bus
8       D3<->    to eproms and module bus
9       D5<->    to eproms and module bus
10      D6<->    to eproms and module bus
11      VCC      VCC
12      D2<->    to eproms and module bus
13      D7<->    to eproms and module bus
14      D0<->    to eproms and module bus
15      D1<->    to eproms and module bus
16      /INT<-   from main board (module pin 11) AND pulled to VCC by (yellow purple red gold)Ohm resistor
17      /NMI<-   tied to VCC*
18      /HALT->  tied to VCC*
19      /MREQ->  to 74ls367 pin 4<-(A2a)
20      /IORQ->  to 74ls367 pin 12<-(A1b)
21      /RD->    to 74ls367 pin 10<-(A4a)
22      /WR->    to 74ls367 pin 6<-(A3a)
23      /BUSAK-> N/C
24      /WAIT<-  tied to /BUSRQ and to VCC
25      /BUSRQ<- tied to /WAIT and to VCC
26      /RESET<- from pin 13 of 74ls367->(Y2b)
27      /M1->    from 74ls02 pin 12<-(B4) AND to main board (module pin 14)
28      /RFSH->  N/C
29      GND      GND
30      A0->     to eproms and module bus
31      A1->     to eproms and module bus
32      A2->     to eproms and module bus
33      A3->     to eproms and module bus
34      A4->     to eproms and module bus
35      A5->     to eproms and module bus
36      A6->     to eproms and module bus AND to 74ls139 pin 2<-(A0a)
37      A7->     to eproms and module bus AND to 74ls139 pin 3<-(A1a)
38      A8->     to eproms and module bus
39      A9->     to eproms and module bus
40      A10->    to eproms and module bus

Note: It seems originally that /NMI and /HALT may have been tied together
  (and perhaps to VCC through a resistor) and NOT directly connected to VCC,
  so when the HALT opcode was executed it would immediately trigger an NMI
  and the CPU could handle this specifically.
  However, at least on the revision C 1872 CPU pcb, they are both tied
  directly to VCC, disabling this behavior.
*****************************************************************************/

/* Core includes */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
//#include "votrpss.lh"

/* Components */
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/votrax.h"
#include "machine/terminal.h"
#include "speaker.h"


namespace {

class votrpss_state : public driver_device
{
public:
	votrpss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_ppi(*this, "ppi")
	{ }

	void votrpss(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void kbd_put(u8 data);
	uint8_t ppi_pa_r();
	uint8_t ppi_pb_r();
	uint8_t ppi_pc_r();
	void ppi_pa_w(uint8_t data);
	void ppi_pb_w(uint8_t data);
	void ppi_pc_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	void write_uart_clock(int state);
	IRQ_CALLBACK_MEMBER(irq_ack);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_term_data = 0U;
	uint8_t m_porta = 0U;
	uint8_t m_portb = 0U;
	uint8_t m_portc = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8255_device> m_ppi;
};


/******************************************************************************
 Address Maps
******************************************************************************/

void votrpss_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom(); /* main roms (in potted module) */
	map(0x4000, 0x7fff).noprw(); /* open bus/space for expansion rom (reads as 0xFF) */
	map(0x8000, 0x8fff).ram(); /* onboard memory (2x 6116) */
	map(0x9000, 0xbfff).noprw(); /* open bus (space for memory expansion, checked by main roms, will be used if found)*/
	map(0xc000, 0xdfff).rom(); /* 'personality rom', containing self-test code and optional user code */
	map(0xe000, 0xffff).noprw(); /* open bus (space for more personality rom, not normally used) */
}

void votrpss_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x3c).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).mirror(0x3e).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x80, 0x83).mirror(0x3c).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xc0, 0xc0).mirror(0x3e).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0xc1, 0xc1).mirror(0x3e).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(votrpss)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Baud Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x01, "4800" )
	PORT_DIPSETTING(    0x02, "2400" )
	PORT_DIPSETTING(    0x03, "1200" )
	PORT_DIPSETTING(    0x04, "600" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "150" )
	PORT_DIPSETTING(    0x07, "75" )
	PORT_DIPNAME( 0x08, 0x00, "Serial Handshaking" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "RTS/CTS" )
	PORT_DIPSETTING(    0x08, "XON/XOFF" )
	PORT_DIPNAME( 0x10, 0x00, "Parity bit behavior" )   PORT_DIPLOCATION("SW1:5") /* note: only firmware 3.C (1984?) and up handle this bit; on earlier firmwares, its function is 'unused' */
	PORT_DIPSETTING(    0x00, "Bit 8 ignored/zeroed" )
	PORT_DIPSETTING(    0x10, "Bit 8 treated as data" )
	PORT_DIPNAME( 0x20, 0x20, "Startup Message" )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Default Input Port" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Serial/RS-232" )
	PORT_DIPSETTING(    0x40, "Parallel" )
	PORT_DIPNAME( 0x80, 0x00, "Self Test Mode" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR ( On )  )
INPUT_PORTS_END

void votrpss_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));

	m_term_data = 0;
	m_porta = 0;
	m_portb = 0;
	m_portc = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( votrpss_state::irq_timer )
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER( votrpss_state::irq_ack )
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x38;
}

uint8_t votrpss_state::ppi_pa_r()
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

uint8_t votrpss_state::ppi_pb_r()
{
	return m_portb;
}

// Bit 0 controls what happens at interrupt time. See code around 518.
uint8_t votrpss_state::ppi_pc_r()
{
	uint8_t data = 0;

	if (m_term_data)
	{
		m_ppi->pc4_w(0); // send a strobe pulse
		data |= 0x20;
	}

	return (m_portc & 0xdb) | data;
}

void votrpss_state::ppi_pa_w(uint8_t data)
{
	m_porta = data;
}

void votrpss_state::ppi_pb_w(uint8_t data)
{
	m_portb = data;
	m_terminal->write(data&0x7f);
}

void votrpss_state::ppi_pc_w(uint8_t data)
{
	m_portc = data;
}

void votrpss_state::kbd_put(u8 data)
{
	m_term_data = data;
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

void votrpss_state::votrpss(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000)/2);  /* 4.000 MHz, verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &votrpss_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &votrpss_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(votrpss_state::irq_ack));

	/* video hardware */
	//config.set_default_layout(layout_votrpss);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay", XTAL(8'000'000)/4)); /* 2.000 MHz, verified */
	ay.port_b_read_callback().set_ioport("DSW1");
	ay.port_a_write_callback().set("votrax", FUNC(votrax_sc01_device::write));
	ay.add_route(ALL_OUTPUTS, "mono", 0.25);
	VOTRAX_SC01A(config, "votrax", 720000).add_route(ALL_OUTPUTS, "mono", 1.00); /* the actual SC-01-A clock is generated by an R/C circuit PWMed by the timer1 output of the 8253 PIT to adjust the resistance, to allow for fine pitch control. 8253 Timer2 is used to PWM an analog switch on the output of the SC-01 to adjust the volume.*/

	/* Devices */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(votrpss_state::kbd_put));

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	// when serial is chosen, and you select terminal, nothing shows (by design). You can only type commands in.
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(8_MHz_XTAL); // Timer 0: baud rate gen for 8251
	pit.out_handler<0>().set("uart", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("uart", FUNC(i8251_device::write_rxc));
	pit.set_clk<1>(8_MHz_XTAL / 256); // Timer 1: Pitch
	pit.set_clk<2>(8_MHz_XTAL / 4096); // Timer 2: Volume

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(votrpss_state::ppi_pa_r));
	m_ppi->in_pb_callback().set(FUNC(votrpss_state::ppi_pb_r));
	m_ppi->in_pc_callback().set(FUNC(votrpss_state::ppi_pc_r));
	m_ppi->out_pa_callback().set(FUNC(votrpss_state::ppi_pa_w));
	m_ppi->out_pb_callback().set(FUNC(votrpss_state::ppi_pb_w));
	m_ppi->out_pc_callback().set(FUNC(votrpss_state::ppi_pc_w));

	TIMER(config, "irq_timer").configure_periodic(FUNC(votrpss_state::irq_timer), attotime::from_msec(10));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrpss)
	ROM_REGION(0x10000, "maincpu", 0)
	/* old logo PSS, version 3.A  (was this even released, or just a prototype?) */

	/* old logo PSS, version 3.B (1982?), selftest 3.1 (1982 or earlier); 3.B may have used the 1871G PCB which probably had a somewhat different memory map, see the main PCB notes at the top of the file */
	//ROM_LOAD("u-2.3.b.bin",   0x0000, 0x2000, NO_DUMP ))
	//ROM_LOAD("u-3.3.b.bin",   0x2000, 0x2000, NO_DUMP ))

	/* old or new logo PSS, Version 3.C (1982), selftest 3.1 (1982 or earlier) */
	// these two roms are on an 1872 daughterboard PCB inside the potted brick daughterboard, or, on unpotted 1987/1988 PSS units, just on an unpotted 1872C daughterboard.
	ROM_LOAD("u-2.v3.c.bin",   0x0000, 0x2000, CRC(410c58cf) SHA1(6e181e61ab9c268e3772fbeba101302fd40b09a2)) /* The 1987/1988 version's rom is marked "U-2 // 090788" but the actual rom data is from 1982 */
	ROM_LOAD("u-3.v3.c.bin",   0x2000, 0x2000, CRC(1439492e) SHA1(46af8ccac6fdb93cbeb8a6d57dce5898e0e0d623)) /* The 1987/1988 version's rom is marked "U-3" */

	// this rom is on the 1871G/H/J mainboard, underneath the cpu module daughterboard, in a socket. Technically it is the 'user rom', but it contains the self test code. A user dictionary could in theory be put in this rom, and larger roms than a 2764 could be used.
	ROM_LOAD("u-4.v3.1.bin", 0xc000, 0x2000, CRC(0b7c4260) SHA1(56f0b6b1cd7b1104e09a9962583121c112337984)) /* the 1987/1988 version's rom is marked "3.1 10/09/85" but the actual rom data is the same from at least as far back as 1982; the 1982 version is marked "U4" in handwriting on a sticker, or "U-4" dot-matrix printed on a sticker */
ROM_END

} // Anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE    INPUT    CLASS          INIT        COMPANY   FULLNAME                  FLAGS
COMP( 1982, votrpss, 0,      0,      votrpss,   votrpss, votrpss_state, empty_init, "Votrax", "Personal Speech System", MACHINE_SUPPORTS_SAVE )
