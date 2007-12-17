/*****************************************************************************
 *
 *   upd7810.c
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.3
 *
 *   Copyright (c) 2001 Juergen Buchmueller, all rights reserved.
 *
 *   Copyright (C) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *  This work is based on the
 *  "NEC Electronics User's Manual, April 1987"
 *
 * NS20030115:
 * - fixed INRW_wa()
 * - TODO: add 7807, differences are listed below.
 *       I only added support for these opcodes needed by homedata.c (yes, I am
 *       lazy):
 *       4C CE (MOV A,PT)
 *       48 AC (EXA)
 *       48 AD (EXR)
 *       48 AE (EXH)
 *       48 AF (EXX)
 *       50 xx (SKN bit)
 *       58 xx (SETB)
 *       5B xx (CLR)
 *       5D xx (SK bit)
 *
 *****************************************************************************/
/* Hau around 23 May 2004
  gta, gti, dgt fixed
  working reg opcodes fixed
  sio input fixed
--
  PeT around 19 February 2002
  type selection/gamemaster support added
  gamemaster init hack? added
  ORAX added
  jre negativ fixed
  prefixed opcodes skipping fixed
  interrupts fixed and improved
  sub(and related)/add/daa flags fixed
  mvi ports,... fixed
  rll, rlr, drll, drlr fixed
  rets fixed
  l0, l1 skipping fixed
  calt fixed
*/

/*

7807 DESCRIPTION



   PA0  1     64 Vcc
   PA1  2     63 Vdd
   PA2  3     62 PD7/AD7
   PA3  4     61 PD6/AD6
   PA4  5     60 PD5/AD5
   PA5  6     59 PD4/AD4
   PA6  7     58 PD3/AD3
   PA7  8     57 PD2/AD2
   PB0  9     56 PD1/AD1
   PB1 10     55 PD0/AD0
   PB2 11     54 PF7/AB15
   PB3 12     53 PF6/AB14
   PB4 13     52 PF5/AB13
   PB5 14     51 PF4/AB12
   PB6 15     50 PF3/AB11
   PB7 16     49 PF2/AB10
   PC0 17     48 PF1/AB9
   PC1 18     47 PF0/AB8
   PC2 19     46 ALE
   PC3 20     45 WR*
   PC4 21     44 RD*
   PC5 22     43 HLDA
   PC6 23     42 HOLD
   PC7 24     41 PT7
  NMI* 25     40 PT6
  INT1 26     39 PT5
 MODE1 27     38 PT4
RESET* 28     37 PT3
 MODE0 29     36 PT2
    X2 30     35 PT1
    X1 31     34 PT0
   Vss 32     33 Vth

PA, PB, PC, PD, and PF is bidirectional I/O port
and PT is comparator input port in uPD7808.
uPD7807 uses PD port as data I/O and bottom address output,
and uses PF port as top address output.

NMI* is non maskable interrupt input signal (negative edge trigger).

INT1 is interrupt input (positive edge trigger). It can be used as
AC zero-cross input or trigger source of 16bit timer counter.

MODE0 and MODE1 is input terminal which decides total amount of
external memory of uPD7807 (4KByte, 16KBYte, and 64KByte).
It also decides number of PF ports used as top address output.
 4KByte mode: PF0~PF3=address output, PF4~PF7=data I/O port
16KByte mode: PF0~PF5=address output, PF6~PF7=data I/O port
64KByte mode: PF0~PF7=address output

RESET* is system rest terminal.

X1 and X2 does clock signal generation (connect OSC and condenser).

Vth is used to determine threshold voltage for PT port.
PT0~PT7 is connected to + input of each comparator,
and Vth deterimnes voltage connected to - input of PT0~PT7.
But the voltage of Vth is not directly connected to comapators.
It is connected via 16-level programmable voltage separate circuit.

HOLD and HLDA is terminal for DMA. RD*, WR*, and ALE is bus
interface signal (they are same type of Intel 8085).
Unlike 8085, I/O address space is not available, so IO /M* signal
does not exist. Read/write of external memory can be done
by RD*, WR*, and ALE only.

Vcc and Vss is main power source. Vdd is backup power source
for internal RWM (32 Byte).


PA and PB is I/O port. They have control register MA and MB.
If control register is set to 1, the port is input.
If control register is set to 0, the port is output.
They are set to 1 by reset.

PT is input-only port. It is consisted of input terminal PT0~PT7
and Vth (set threshold voltage). Each PT input has analog comparator
and latch, and + input of analog comparator is connected to
PT terminal. Every - input of analog comparator is connected
to devided voltage of Vth. Voltage dividing level can be set by
bottom 4bits of MT (mode T) register. The range is 1/16~16/16 of Vth.

Other internal I/Os are
8bit timer (x2): Upcounter. If the counter matches to specified value,
the timer is reset and counts again from 0.
You can also set it to generate interrupt, or invert output flip-flop
when the counter matches to specified value.
Furthermore, you can output that flip-flop output to PC4/TO output,
connect it to clock input of timer/event counter or watchdog timer.
Or you can use it as bitrate clock of serial interface.
Note: There is only 1 output flip-flop for 2 timers.
If you use it for timer output of 1 timer, another timer cannot be used
for other than interrupt generator.
Clock input for timer can be switched between internal clock (2 type)
or PC3/TI input. You can set 1 timer's match-output as another timer's
clock input, so that you can use them as 1 16bit timer.

16bit timer/event counter (x1): It can be used as
- Interval timer
- External event counter
- Frequency measurement
- Pulse width measurement
- Programmable rectangle wave output
- One pulse output
Related terminals are PC5/CI input, PC6/CO0 output, and PC7/CO1.
You can measure CI input's H duration, or you can output timing signal
(with phase difference) to CO0 and CO1.

serial I/F (x1): has 3 modes.
- Asynchronous mode
- Synchronous mode
- I/O interface mode
In all 3 modes, bitrate can be internal fixed clock, or timer output,
or external clock.
In asynchronous mode, you can
- switch 7bit/8bit data
- set parity ON/OFF and EVEN/ODD
- set 1/2 stop bit




DIFFERENCES BETWEEN 7810 and 7807

--------------------------
8bit transfer instructions
--------------------------

7810
inst.     1st byte 2nd byte state   action
EXX       00001001            4     Swap BC DE HL
EXA       00001000            4     Swap VA EA
EXH       01010000            4     Swap HL
BLOCK     00110001          13(C+1)  (DE)+ <- (HL)+, C <- C - 1, until CY

7807
inst.     1st byte  2nd byte state   action
EXR       01001000  10101101   8     Swap VA BC DE HL EA
EXX       01001000  10101111   8     Swap BC DE HL
EXA       01001000  10101100   8     Swap VA EA
EXH       01001000  10101110   8     Swap HL
BLOCK  D+ 00010000           13(C+1) (DE)+ <- (HL)+, C <- C - 1, until CY
BLOCK  D- 00010001           13(C+1) (DE)- <- (HL)-, C <- C - 1, until CY


---------------------------
16bit transfer instructions
---------------------------
All instructions are same except operand sr4 of DMOV instruction.
7810
V0-sr4 -function
 0-ECNT-timer/event counter upcounter
 1-ECPT-timer/event counter capture

7807
V1-V0- sr4 -function
 0- 0-ECNT -timer/event counter upcounter
 0- 1-ECPT0-timer/event counter capture 0
 1- 0-ECPT1-timer/event counter capture 1


-----------------------------------------
8bit operation instructions for registers
-----------------------------------------
All instructions are same.


--------------------------------------
8bit operation instructions for memory
--------------------------------------
All instructions are same.


-----------------------------------------
Operation instructions for immediate data
-----------------------------------------
uPD7807 has read-only PT port and special register group sr5 for it.
ins.               1st byte  2nd byte 3rd 4th state func
GTI    sr5, byte   01100100  s0101sss  dd      14   !CY  sr5 - byte - 1
LTI    sr5, byte   01100100  s0111sss  dd      14    CY  sr5 - byte
NEI    sr5, byte   01100100  s1101sss  dd      14   !Z   sr5 - byte
EQI    sr5, byte   01100100  s1111sss  dd      14    Z   sr5 - byte
ONI    sr5, byte   01100100  s1001sss  dd      14   !Z   sr5 & byte
OFFI   sr5, byte   01100100  s1011sss  dd      14    Z   sr5 & byte

S5-S4-S3-S2-S1-S0-sr -sr1-sr2-sr5-register function
 0  0  1  1  1  0 --- PT  --- PT  comparator input port T data
 1  0  0  1  0  0 WDM WDM --- --- watchdog timer mode register
 1  0  0  1  0  1 MT  --- --- --- port T mode

7807 doesn't have registers below
 0  0  1  0  0  0 ANM ANM ANM     A/D channel mode
 1  0  0  0  0  0 --- CR0 ---     A/D conversion result 0
 1  0  0  0  0  1 --- CR1 ---     A/D conversion result 1
 1  0  0  0  1  0 --- CR2 ---     A/D conversion result 2
 1  0  0  0  1  1 --- CR3 ---     A/D conversion result 3
 1  0  1  0  0  0 ZCM --- ---     zero cross mode

Special register operand (includes registers for I/O ports) has
6 groups - sr, sr1, sr2, sr3, sr4, and sr5. Among these groups,
sr, sr1, sr2, and sr5 includes registers described in the table
below, and expressed as bit pattern S5-S0.

S5S4S3S2S1S0 sr  sr1 sr2 sr5 register function
0 0 0 0 0 0  PA  PA  PA  PA  port A
0 0 0 0 0 1  PB  PB  PB  PB  port B
0 0 0 0 1 0  PC  PC  PC  PC  port C
0 0 0 0 1 1  PD  PD  PD  PD  port D
0 0 0 1 0 1  PF  PF  PF  PF  port F
0 0 0 1 1 0  MKH MKH MKH MKH mask high
0 0 0 1 1 1  MKL MKL MKL MKL mask low
0 0 1 0 0 1  SMH SMH SMH SMH serial mode high
0 0 1 0 1 0  SML --- --- --- serial mode low
0 0 1 0 1 1  EOM EOM EOM EOM timer/event counter output mode
0 0 1 1 0 0 ETMM --- --- --- timer/event counter mode
0 0 1 1 0 1  TMM TMM TMM TMM timer mode
0 0 1 1 1 0  --- PT  --- PT  port T
0 1 0 0 0 0  MM  --- --- --- memory mapping
0 1 0 0 0 1  MCC --- --- --- mode control C
0 1 0 0 1 0  MA  --- --- --- mode A
0 1 0 0 1 1  MB  --- --- --- mode B
0 1 0 1 0 0  MC  --- --- --- mode C
0 1 0 1 1 1  MF  --- --- --- mode F
0 1 1 0 0 0  TXB --- --- --- Tx buffer
0 1 1 0 0 1  --- RXB --- --- Rx buffer
0 1 1 0 1 0  TM0 --- --- --- timer register 0
0 1 1 0 1 1  TM1 --- --- --- timer register 1
1 0 0 1 0 0  WDM WDM --- --- watchdog timer mode
1 0 0 1 0 1  MT  --- --- --- mode T

For sr and sr1, all 6bits (S5, S4, S3, S2, S1, and S0) are used.
For sr2 and sr5, only 4bits (S3, S2, S1, AND S0) are used.
They are expressed as 'ssssss' and 's sss' in operation code.
Note that 's sss' (of sr2 and sr5) is located separately.
S0 is rightmost bit (LSB).


--------------------------------------------
Operation instructions for working registers
--------------------------------------------
All instructions are same.


--------------------------------------------------------------------------
16bit operation instructions and divider/multiplier operation instructions
--------------------------------------------------------------------------
All instructions are same.


------------------------------------------
Increment/decrement operation instructions
------------------------------------------
All instructions are same.


----------------------------
Other operation instructions
----------------------------
7807 has CMC instruction (inverts CY flag).
ins. 1st byte 2nd byte 3rd 4th state func
CMC  01001000 10101010           8   CY <- !CY


---------------------------
Rotation/shift instructions
---------------------------
All instructions are same.


-----------------------------
Jump/call/return instructions
-----------------------------
All instructions are same.


-----------------
Skip instructions
-----------------
7807 doesn't have this
ins.            1st byte 2nd byte 3rd 4th state func
BIT bit, wa     01011bbb  wwwwwwww          10*  bit skip if (V.wa).bit = 1

Instead, 7807 has these bit manipulation instructions.
ins.            1st byte 2nd byte 3rd 4th state func
MOV    CY, bit  01011111  bbbbbbbb          10* CY <- (bit)
MOV    bit, CY  01011010  bbbbbbbb          13* (bit) <- CY
AND    CY, bit  00110001  bbbbbbbb          10* CY <- CY & (bit)
OR     CY, bit  01011100  bbbbbbbb          10* CY <- CY | (bit)
XOR    CY, bit  01011110  bbbbbbbb          10* CY <- CY ^ (bit)
SETB   bit      01011000  bbbbbbbb          13* (bit) <- 1
CLR    bit      01011011  bbbbbbbb          13* (bit) <- 0
NOT    bit      01011001  bbbbbbbb          13* (bit) <- !(bit)
SK     bit      01011101  bbbbbbbb          10*  (b) skip if (bit) = 1
SKN    bit      01010000  bbbbbbbb          10* !(b) skip if (bit) = 0


------------------------
CPU control instructions
------------------------
ins.            1st byte 2nd byte 3rd 4th state func
HLT             01001000  00111011        11/12 halt
11 state in uPD7807 and uPD7810, 12 state in uPD78C10.

STOP            01001000  10111011          12  stop
7807 doesn't have STOP instruction.

*/


#include "debugger.h"
#include "upd7810.h"

static UPD7810 upd7810;
static int upd7810_icount;

#define CY	0x01
#define F1	0x02
#define L0	0x04
#define L1	0x08
#define HC	0x10
#define SK	0x20
#define Z	0x40
#define F7	0x80

/* IRR flags */
#define INTNMI	0x0001
#define INTFT0	0x0002
#define INTFT1	0x0004
#define INTF1	0x0008
#define INTF2	0x0010
#define INTFE0	0x0020
#define INTFE1	0x0040
#define INTFEIN 0x0080
#define INTFAD	0x0100
#define INTFSR	0x0200
#define INTFST	0x0400
#define INTER	0x0800
#define INTOV	0x1000

/* ITF flags */
#define INTAN4	0x0001
#define INTAN5	0x0002
#define INTAN6	0x0004
#define INTAN7	0x0008
#define INTSB	0x0010

#define PPC 	upd7810.ppc.w.l
#define PC		upd7810.pc.w.l
#define PCL 	upd7810.pc.b.l
#define PCH 	upd7810.pc.b.h
#define PCD 	upd7810.pc.d
#define SP		upd7810.sp.w.l
#define SPL 	upd7810.sp.b.l
#define SPH 	upd7810.sp.b.h
#define SPD 	upd7810.sp.d
#define PSW 	upd7810.psw
#define OP		upd7810.op
#define OP2 	upd7810.op2
#define IFF 	upd7810.iff
#define EA		upd7810.ea.w.l
#define EAL 	upd7810.ea.b.l
#define EAH 	upd7810.ea.b.h
#define VA		upd7810.va.w.l
#define V		upd7810.va.b.h
#define A		upd7810.va.b.l
#define VAD 	upd7810.va.d
#define BC		upd7810.bc.w.l
#define B		upd7810.bc.b.h
#define C		upd7810.bc.b.l
#define DE		upd7810.de.w.l
#define D		upd7810.de.b.h
#define E		upd7810.de.b.l
#define HL		upd7810.hl.w.l
#define H		upd7810.hl.b.h
#define L		upd7810.hl.b.l
#define EA2 	upd7810.ea2.w.l
#define VA2 	upd7810.va2.w.l
#define BC2 	upd7810.bc2.w.l
#define DE2 	upd7810.de2.w.l
#define HL2 	upd7810.hl2.w.l

#define OVC0	upd7810.ovc0
#define OVC1	upd7810.ovc1
#define OVCE	upd7810.ovce
#define OVCF	upd7810.ovcf
#define OVCS	upd7810.ovcs
#define EDGES	upd7810.edges

#define CNT0	upd7810.cnt.b.l
#define CNT1	upd7810.cnt.b.h
#define TM0 	upd7810.tm.b.l
#define TM1 	upd7810.tm.b.h
#define ECNT	upd7810.ecnt.w.l
#define ECPT	upd7810.ecnt.w.h
#define ETM0	upd7810.etm.w.l
#define ETM1	upd7810.etm.w.h

#define MA		upd7810.ma
#define MB		upd7810.mb
#define MCC 	upd7810.mcc
#define MC		upd7810.mc
#define MM		upd7810.mm
#define MF		upd7810.mf
#define TMM 	upd7810.tmm
#define ETMM	upd7810.etmm
#define EOM 	upd7810.eom
#define SML 	upd7810.sml
#define SMH 	upd7810.smh
#define ANM 	upd7810.anm
#define MKL 	upd7810.mkl
#define MKH 	upd7810.mkh
#define ZCM 	upd7810.zcm

#define CR0 	upd7810.cr0
#define CR1 	upd7810.cr1
#define CR2 	upd7810.cr2
#define CR3 	upd7810.cr3
#define RXB 	upd7810.rxb
#define TXB 	upd7810.txb

#define RXD 	upd7810.rxd
#define TXD 	upd7810.txd
#define SCK 	upd7810.sck
#define TI		upd7810.ti
#define TO		upd7810.to
#define CI		upd7810.ci
#define CO0 	upd7810.co0
#define CO1 	upd7810.co1

#define IRR 	upd7810.irr
#define ITF 	upd7810.itf

struct opcode_s {
	void (*opfunc)(void);
	UINT8 oplen;
	UINT8 cycles;
	UINT8 cycles_skip;
	UINT8 mask_l0_l1;
};

#define RDOP(O) 	O = cpu_readop(PCD); PC++
#define RDOPARG(A)	A = cpu_readop_arg(PCD); PC++
#define RM(A)		program_read_byte_8(A)
#define WM(A,V) 	program_write_byte_8(A,V)

#define ZHC_ADD(after,before,carry) 	\
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (after == before) \
		PSW = (PSW&~CY) | (carry); \
	else if (after < before)			\
		PSW |= CY;			\
	else								\
		PSW &= ~CY;				\
	if ((after & 15) < (before & 15))	\
		PSW |= HC;						\
	else								\
		PSW &= ~HC; 					\

#define ZHC_SUB(after,before,carry) 	\
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (before == after) 					\
		PSW = (PSW & ~CY) | (carry);	\
	else if (after > before)			\
		PSW |= CY;			\
	else								\
		PSW &= ~CY;				\
	if ((after & 15) > (before & 15))	\
		PSW |= HC;						\
	else								\
		PSW &= ~HC; 					\

#define SKIP_CY 	if (CY == (PSW & CY)) PSW |= SK
#define SKIP_NC 	if (0 == (PSW & CY)) PSW |= SK
#define SKIP_Z		if (Z == (PSW & Z)) PSW |= SK
#define SKIP_NZ 	if (0 == (PSW & Z)) PSW |= SK
#define SET_Z(n)	if (n) PSW &= ~Z; else PSW |= Z

static UINT8 RP(offs_t port)
{
	UINT8 data = 0xff;
	switch (port)
	{
	case UPD7810_PORTA:
		if (upd7810.ma)	// NS20031301 no need to read if the port is set as output
			upd7810.pa_in = io_read_byte_8(port);
		data = (upd7810.pa_in & upd7810.ma) | (upd7810.pa_out & ~upd7810.ma);
		break;
	case UPD7810_PORTB:
		if (upd7810.mb)	// NS20031301 no need to read if the port is set as output
			upd7810.pb_in = io_read_byte_8(port);
		data = (upd7810.pb_in & upd7810.mb) | (upd7810.pb_out & ~upd7810.mb);
		break;
	case UPD7810_PORTC:
		if (upd7810.mc)	// NS20031301 no need to read if the port is set as output
			upd7810.pc_in = io_read_byte_8(port);
		data = (upd7810.pc_in & upd7810.mc) | (upd7810.pc_out & ~upd7810.mc);
		if (upd7810.mcc & 0x01) 	/* PC0 = TxD output */
			data = (data & ~0x01) | (upd7810.txd & 1 ? 0x01 : 0x00);
		if (upd7810.mcc & 0x02) 	/* PC1 = RxD input */
			data = (data & ~0x02) | (upd7810.rxd & 1 ? 0x02 : 0x00);
		if (upd7810.mcc & 0x04) 	/* PC2 = SCK input/output */
			data = (data & ~0x04) | (upd7810.sck & 1 ? 0x04 : 0x00);
		if (upd7810.mcc & 0x08) 	/* PC3 = TI input */
			data = (data & ~0x08) | (upd7810.ti & 1 ? 0x08 : 0x00);
		if (upd7810.mcc & 0x10) 	/* PC4 = TO output */
			data = (data & ~0x10) | (upd7810.to & 1 ? 0x10 : 0x00);
		if (upd7810.mcc & 0x20) 	/* PC5 = CI input */
			data = (data & ~0x20) | (upd7810.ci & 1 ? 0x20 : 0x00);
		if (upd7810.mcc & 0x40) 	/* PC6 = CO0 output */
			data = (data & ~0x40) | (upd7810.co0 & 1 ? 0x40 : 0x00);
		if (upd7810.mcc & 0x80) 	/* PC7 = CO1 output */
			data = (data & ~0x80) | (upd7810.co1 & 1 ? 0x80 : 0x00);
		break;
	case UPD7810_PORTD:
		upd7810.pd_in = io_read_byte_8(port);
		switch (upd7810.mm & 0x07)
		{
		case 0x00:			/* PD input mode, PF port mode */
			data = upd7810.pd_in;
			break;
		case 0x01:			/* PD output mode, PF port mode */
			data = upd7810.pd_out;
			break;
		default:			/* PD extension mode, PF port/extension mode */
			data = 0xff;	/* what do we see on the port here? */
			break;
		}
		break;
	case UPD7810_PORTF:
		upd7810.pf_in = io_read_byte_8(port);
		switch (upd7810.mm & 0x06)
		{
		case 0x00:			/* PD input/output mode, PF port mode */
			data = (upd7810.pf_in & upd7810.mf) | (upd7810.pf_out & ~upd7810.mf);
			break;
		case 0x02:			/* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data = (upd7810.pf_in & upd7810.mf) | (upd7810.pf_out & ~upd7810.mf);
			data |= 0x0f;	/* what would we see on the lower bits here? */
			break;
		case 0x04:			/* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data = (upd7810.pf_in & upd7810.mf) | (upd7810.pf_out & ~upd7810.mf);
			data |= 0x3f;	/* what would we see on the lower bits here? */
			break;
		case 0x06:
			data = 0xff;	/* what would we see on the lower bits here? */
			break;
		}
		break;
	case UPD7807_PORTT:	// NS20031301 partial implementation
		data = io_read_byte_8(port);
		break;
	default:
		logerror("uPD7810 internal error: RP() called with invalid port number\n");
	}
	return data;
}

static void WP(offs_t port, UINT8 data)
{
	switch (port)
	{
	case UPD7810_PORTA:
		upd7810.pa_out = data;
//      data = (data & ~upd7810.ma) | (upd7810.pa_in & upd7810.ma);
		data = (data & ~upd7810.ma) | (upd7810.ma);	// NS20031401
		io_write_byte_8(port, data);
		break;
	case UPD7810_PORTB:
		upd7810.pb_out = data;
//      data = (data & ~upd7810.mb) | (upd7810.pb_in & upd7810.mb);
		data = (data & ~upd7810.mb) | (upd7810.mb);	// NS20031401
		io_write_byte_8(port, data);
		break;
	case UPD7810_PORTC:
		upd7810.pc_out = data;
//      data = (data & ~upd7810.mc) | (upd7810.pc_in & upd7810.mc);
		data = (data & ~upd7810.mc) | (upd7810.mc);	// NS20031401
		if (upd7810.mcc & 0x01) 	/* PC0 = TxD output */
			data = (data & ~0x01) | (upd7810.txd & 1 ? 0x01 : 0x00);
		if (upd7810.mcc & 0x02) 	/* PC1 = RxD input */
			data = (data & ~0x02) | (upd7810.rxd & 1 ? 0x02 : 0x00);
		if (upd7810.mcc & 0x04) 	/* PC2 = SCK input/output */
			data = (data & ~0x04) | (upd7810.sck & 1 ? 0x04 : 0x00);
		if (upd7810.mcc & 0x08) 	/* PC3 = TI input */
			data = (data & ~0x08) | (upd7810.ti & 1 ? 0x08 : 0x00);
		if (upd7810.mcc & 0x10) 	/* PC4 = TO output */
			data = (data & ~0x10) | (upd7810.to & 1 ? 0x10 : 0x00);
		if (upd7810.mcc & 0x20) 	/* PC5 = CI input */
			data = (data & ~0x20) | (upd7810.ci & 1 ? 0x20 : 0x00);
		if (upd7810.mcc & 0x40) 	/* PC6 = CO0 output */
			data = (data & ~0x40) | (upd7810.co0 & 1 ? 0x40 : 0x00);
		if (upd7810.mcc & 0x80) 	/* PC7 = CO1 output */
			data = (data & ~0x80) | (upd7810.co1 & 1 ? 0x80 : 0x00);
		io_write_byte_8(port, data);
		break;
	case UPD7810_PORTD:
		upd7810.pd_out = data;
		switch (upd7810.mm & 0x07)
		{
		case 0x00:			/* PD input mode, PF port mode */
			data = upd7810.pd_in;
			break;
		case 0x01:			/* PD output mode, PF port mode */
			data = upd7810.pd_out;
			break;
		default:			/* PD extension mode, PF port/extension mode */
			return;
		}
		io_write_byte_8(port, data);
		break;
	case UPD7810_PORTF:
		upd7810.pf_out = data;
		data = (data & ~upd7810.mf) | (upd7810.pf_in & upd7810.mf);
		switch (upd7810.mm & 0x06)
		{
		case 0x00:			/* PD input/output mode, PF port mode */
			break;
		case 0x02:			/* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data |= 0x0f;	/* what would come out for the lower bits here? */
			break;
		case 0x04:			/* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data |= 0x3f;	/* what would come out for the lower bits here? */
			break;
		case 0x06:
			data |= 0xff;	/* what would come out for the lower bits here? */
			break;
		}
		io_write_byte_8(port, data);
		break;
	default:
		logerror("uPD7810 internal error: RP() called with invalid port number\n");
	}
}

static void upd7810_take_irq(void)
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

	/* check the interrupts in priority sequence */
	if ((IRR & INTFT0)	&& 0 == (MKL & 0x02))
	{
	    switch (upd7810.config.type)
		{
			case TYPE_7810_GAMEMASTER:
				vector = 0xff2a;
				break;
			default:
				vector = 0x0008;
		}
	    if (!((IRR & INTFT1)	&& 0 == (MKL & 0x04)))
		IRR&=~INTFT0;
	}
	else
	if ((IRR & INTFT1)	&& 0 == (MKL & 0x04))
	{
	    switch (upd7810.config.type)
		{
			case TYPE_7810_GAMEMASTER:
				vector = 0xff2a;
				break;
			default:
				vector = 0x0008;
		}
	    IRR&=~INTFT1;
	}
	else
	if ((IRR & INTF1)	&& 0 == (MKL & 0x08))
	{
		irqline = UPD7810_INTF1;
		vector = 0x0010;
		if (!((IRR & INTF2)	&& 0 == (MKL & 0x10)))
		    IRR&=~INTF1;
	}
	else
	if ((IRR & INTF2)	&& 0 == (MKL & 0x10))
	{
		irqline = UPD7810_INTF2;
		vector = 0x0010;
		IRR&=~INTF2;
	}
	else
	if ((IRR & INTFE0)	&& 0 == (MKL & 0x20))
	{
	    switch (upd7810.config.type)
		{
			case TYPE_7810_GAMEMASTER:
				vector = 0xff2d;
				break;
			default:
				vector = 0x0018;
		}
	    if (!((IRR & INTFE1)	&& 0 == (MKL & 0x40)))
		IRR&=~INTFE0;
	}
	else
	if ((IRR & INTFE1)	&& 0 == (MKL & 0x40))
	{
	    switch (upd7810.config.type)
		{
		    case TYPE_7810_GAMEMASTER:
				vector = 0xff2d;
				break;
		    default:
				vector = 0x0018;
		}
	    IRR&=~INTFE1;
	}
	else
	if ((IRR & INTFEIN) && 0 == (MKL & 0x80))
	{
		vector = 0x0020;
	}
	else
	if ((IRR & INTFAD)	&& 0 == (MKH & 0x01))
	{
		vector = 0x0020;
	}
	else
	if ((IRR & INTFSR)	&& 0 == (MKH & 0x02))
	{
		vector = 0x0028;
	    IRR&=~INTFSR;
	}
	else
	if ((IRR & INTFST)	&& 0 == (MKH & 0x04))
	{
		vector = 0x0028;
	    IRR&=~INTFST;
	}
	if (vector)
	{
		/* acknowledge external IRQ */
		if (irqline)
			(*upd7810.irq_callback)(irqline);
		SP--;
		WM( SP, PSW );
		SP--;
		WM( SP, PCH );
		SP--;
		WM( SP, PCL );
		IFF = 0;
		PSW &= ~(SK|L0|L1);
		PC = vector;
		change_pc( PCD );
	}
}

static void upd7810_write_EOM(void)
{
	if (EOM & 0x01) /* output LV0 content ? */
	{
		switch (EOM & 0x0e)
		{
		case 0x02:	/* toggle CO0 */
			CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
			break;
		case 0x04:	/* reset CO0 */
			CO0 = 0;
			break;
		case 0x08:	/* set CO0 */
			CO0 = 1;
			break;
		}
	}
	if (EOM & 0x10) /* output LV0 content ? */
	{
		switch (EOM & 0xe0)
		{
		case 0x20:	/* toggle CO1 */
			CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
			break;
		case 0x40:	/* reset CO1 */
			CO1 = 0;
			break;
		case 0x80:	/* set CO1 */
			CO1 = 1;
			break;
		}
	}
}

static void upd7810_write_TXB(void)
{
	upd7810.txbuf = 1;
}

#define PAR7(n) ((((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)
#define PAR8(n) ((((n)>>7)^((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)

static void upd7810_sio_output(void)
{
	/* shift out more bits? */
	if (upd7810.txcnt > 0)
	{
		TXD = upd7810.txs & 1;
		if (upd7810.config.io_callback)
			(*upd7810.config.io_callback)(UPD7810_TXD,TXD);
		upd7810.txs >>= 1;
		upd7810.txcnt--;
		if (0 == upd7810.txcnt)
			IRR |= INTFST;		/* serial transfer completed */
	}
	else
	if (SMH & 0x04) /* send enable ? */
	{
		/* nothing written into the transmitter buffer ? */
        if (0 == upd7810.txbuf)
			return;
        upd7810.txbuf = 0;

        if (SML & 0x03)         /* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:	/* 7bits, no parity, 1 stop bit */
			case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit8 */
				upd7810.txs = (TXB << 1) | (1 << 8);
				upd7810.txcnt = 9;
				break;
			case 0x4c:	/* 8bits, no parity, 1 stop bit */
			case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit9 */
				upd7810.txs = (TXB << 1) | (1 << 9);
				upd7810.txcnt = 10;
				break;
			case 0x58:	/* 7bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				upd7810.txs = (TXB << 1) | (PAR7(TXB) << 8) | (1 << 9);
				upd7810.txcnt = 10;
				break;
			case 0x5c:	/* 8bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				upd7810.txs = (TXB << 1) | (PAR8(TXB) << 9) | (1 << 10);
				upd7810.txcnt = 11;
				break;
			case 0x78:	/* 7bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				upd7810.txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (1 << 9);
				upd7810.txcnt = 10;
				break;
			case 0x7c:	/* 8bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				upd7810.txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				upd7810.txcnt = 11;
				break;
			case 0xc8:	/* 7bits, no parity, 2 stop bits */
			case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits int bit8+9 */
				upd7810.txs = (TXB << 1) | (3 << 8);
				upd7810.txcnt = 10;
				break;
			case 0xcc:	/* 8bits, no parity, 2 stop bits */
			case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits in bits9+10 */
				upd7810.txs = (TXB << 1) | (3 << 9);
				upd7810.txcnt = 11;
				break;
			case 0xd8:	/* 7bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bits9+10 */
				upd7810.txs = (TXB << 1) | (PAR7(TXB) << 8) | (3 << 9);
				upd7810.txcnt = 11;
				break;
			case 0xdc:	/* 8bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bit10+11 */
				upd7810.txs = (TXB << 1) | (PAR8(TXB) << 9) | (3 << 10);
				upd7810.txcnt = 12;
				break;
			case 0xf8:	/* 7bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bit9+10 */
				upd7810.txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (3 << 9);
				upd7810.txcnt = 11;
				break;
			case 0xfc:	/* 8bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bits10+10 */
				upd7810.txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				upd7810.txcnt = 12;
				break;
			}
		}
		else
		{
			/* synchronous mode */
			upd7810.txs = TXB;
			upd7810.txcnt = 8;
		}
	}
}

static void upd7810_sio_input(void)
{
	/* sample next bit? */
	if (upd7810.rxcnt > 0)
	{
		if (upd7810.config.io_callback)
			RXD = (*upd7810.config.io_callback)(UPD7810_RXD,RXD);
		upd7810.rxs = (upd7810.rxs >> 1) | ((UINT16)RXD << 15);
		upd7810.rxcnt--;
		if (0 == upd7810.rxcnt)
		{
			/* reset the TSK bit */
			SMH &= ~0x40;
			/* serial receive completed interrupt */
			IRR |= INTFSR;
			/* now extract the data from the shift register */
			if (SML & 0x03) 	/* asynchronous mode ? */
			{
				switch (SML & 0xfc)
				{
				case 0x48:	/* 7bits, no parity, 1 stop bit */
				case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					upd7810.rxs >>= 16 - 9;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((1 << 8) != (upd7810.rxs & (1 | (1 << 8))))
						IRR |= INTER;	/* framing error */
					break;
				case 0x4c:	/* 8bits, no parity, 1 stop bit */
				case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					upd7810.rxs >>= 16 - 10;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((1 << 9) != (upd7810.rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					break;
				case 0x58:	/* 7bits, odd parity, 1 stop bit */
					upd7810.rxs >>= 16 - 10;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((1 << 9) != (upd7810.rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((upd7810.rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x5c:	/* 8bits, odd parity, 1 stop bit */
					upd7810.rxs >>= 16 - 11;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((1 << 10) != (upd7810.rxs & (1 | (1 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((upd7810.rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x78:	/* 7bits, even parity, 1 stop bit */
					upd7810.rxs >>= 16 - 10;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((1 << 9) != (upd7810.rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((upd7810.rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x7c:	/* 8bits, even parity, 1 stop bit */
					upd7810.rxs >>= 16 - 11;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((1 << 10) != (upd7810.rxs & (1 | (1 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((upd7810.rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xc8:	/* 7bits, no parity, 2 stop bits */
				case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					upd7810.rxs >>= 16 - 10;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((3 << 9) != (upd7810.rxs & (1 | (3 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((upd7810.rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xcc:	/* 8bits, no parity, 2 stop bits */
				case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					upd7810.rxs >>= 16 - 11;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((3 << 10) != (upd7810.rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((upd7810.rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xd8:	/* 7bits, odd parity, 2 stop bits */
					upd7810.rxs >>= 16 - 11;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((3 << 10) != (upd7810.rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((upd7810.rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xdc:	/* 8bits, odd parity, 2 stop bits */
					upd7810.rxs >>= 16 - 12;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((3 << 11) != (upd7810.rxs & (1 | (3 << 11))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((upd7810.rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xf8:	/* 7bits, even parity, 2 stop bits */
					upd7810.rxs >>= 16 - 11;
					RXB = (upd7810.rxs >> 1) & 0x7f;
					if ((3 << 10) != (upd7810.rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((upd7810.rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xfc:	/* 8bits, even parity, 2 stop bits */
					upd7810.rxs >>= 16 - 12;
					RXB = (upd7810.rxs >> 1) & 0xff;
					if ((3 << 11) != (upd7810.rxs & (1 | (3 << 11))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((upd7810.rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				}
			}
			else
			{
				upd7810.rxs >>= 16 - 8;
				RXB = upd7810.rxs;
//              upd7810.rxcnt = 8;
			}
		}
	}
	else
	if (SMH & 0x08) /* receive enable ? */
	{
		if (SML & 0x03) 	/* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:	/* 7bits, no parity, 1 stop bit */
			case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				upd7810.rxcnt = 9;
				break;
			case 0x4c:	/* 8bits, no parity, 1 stop bit */
			case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				upd7810.rxcnt = 10;
				break;
			case 0x58:	/* 7bits, odd parity, 1 stop bit */
				upd7810.rxcnt = 10;
				break;
			case 0x5c:	/* 8bits, odd parity, 1 stop bit */
				upd7810.rxcnt = 11;
				break;
			case 0x78:	/* 7bits, even parity, 1 stop bit */
				upd7810.rxcnt = 10;
				break;
			case 0x7c:	/* 8bits, even parity, 1 stop bit */
				upd7810.rxcnt = 11;
				break;
			case 0xc8:	/* 7bits, no parity, 2 stop bits */
			case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				upd7810.rxcnt = 10;
				break;
			case 0xcc:	/* 8bits, no parity, 2 stop bits */
			case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				upd7810.rxcnt = 11;
				break;
			case 0xd8:	/* 7bits, odd parity, 2 stop bits */
				upd7810.rxcnt = 11;
				break;
			case 0xdc:	/* 8bits, odd parity, 2 stop bits */
				upd7810.rxcnt = 12;
				break;
			case 0xf8:	/* 7bits, even parity, 2 stop bits */
				upd7810.rxcnt = 11;
				break;
			case 0xfc:	/* 8bits, even parity, 2 stop bits */
				upd7810.rxcnt = 12;
				break;
			}
		}
		else
		/* TSK bit set ? */
		if (SMH & 0x40)
		{
			upd7810.rxcnt = 8;
		}
	}
}

static void upd7810_timers(int cycles)
{
	/**** TIMER 0 ****/
	if (TMM & 0x10) 		/* timer 0 upcounter reset ? */
		CNT0 = 0;
	else
	{
		switch (TMM & 0x0c) /* timer 0 clock source */
		{
		case 0x00:	/* clock divided by 12 */
			OVC0 += cycles;
			while (OVC0 >= 12)
			{
				OVC0 -= 12;
				CNT0++;
				if (CNT0 == TM0)
				{
					CNT0 = 0;
					IRR |= INTFT0;
					/* timer F/F source is timer 0 ? */
					if (0x00 == (TMM & 0x03))
					{
						TO ^= 1;
						if (upd7810.config.io_callback)
							(*upd7810.config.io_callback)(UPD7810_TO,TO);
					}
					/* timer 1 chained with timer 0 ? */
					if ((TMM & 0xe0) == 0x60)
					{
						CNT1++;
						if (CNT1 == TM1)
						{
							IRR |= INTFT1;
							CNT1 = 0;
							/* timer F/F source is timer 1 ? */
							if (0x01 == (TMM & 0x03))
							{
								TO ^= 1;
								if (upd7810.config.io_callback)
									(*upd7810.config.io_callback)(UPD7810_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x04:	/* clock divided by 384 */
			OVC0 += cycles;
			while (OVC0 >= 384)
			{
				OVC0 -= 384;
				CNT0++;
				if (CNT0 == TM0)
				{
					CNT0 = 0;
					IRR |= INTFT0;
					/* timer F/F source is timer 0 ? */
					if (0x00 == (TMM & 0x03))
					{
						TO ^= 1;
						if (upd7810.config.io_callback)
							(*upd7810.config.io_callback)(UPD7810_TO,TO);
					}
					/* timer 1 chained with timer 0 ? */
					if ((TMM & 0xe0) == 0x60)
					{
						CNT1++;
						if (CNT1 == TM1)
						{
							CNT1 = 0;
							IRR |= INTFT1;
							/* timer F/F source is timer 1 ? */
							if (0x01 == (TMM & 0x03))
							{
								TO ^= 1;
								if (upd7810.config.io_callback)
									(*upd7810.config.io_callback)(UPD7810_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x08:	/* external signal at TI */
			break;
		case 0x0c:	/* disabled */
			break;
		}
	}

	/**** TIMER 1 ****/
	if (TMM & 0x80) 		/* timer 1 upcounter reset ? */
		CNT1 = 0;
	else
	{
		switch (TMM & 0x60) /* timer 1 clock source */
		{
		case 0x00:	/* clock divided by 12 */
			OVC1 += cycles;
			while (OVC1 >= 12)
			{
				OVC1 -= 12;
				CNT1++;
				if (CNT1 == TM1)
				{
					CNT1 = 0;
					IRR |= INTFT1;
					/* timer F/F source is timer 1 ? */
					if (0x01 == (TMM & 0x03))
					{
						TO ^= 1;
						if (upd7810.config.io_callback)
							(*upd7810.config.io_callback)(UPD7810_TO,TO);
					}
				}
			}
			break;
		case 0x20:	/* clock divided by 384 */
			OVC1 += cycles;
			while (OVC1 >= 384)
			{
				OVC1 -= 384;
				CNT1++;
				if (CNT1 == TM1)
				{
					CNT1 = 0;
					IRR |= INTFT1;
					/* timer F/F source is timer 1 ? */
					if (0x01 == (TMM & 0x03))
					{
						TO ^= 1;
						if (upd7810.config.io_callback)
							(*upd7810.config.io_callback)(UPD7810_TO,TO);
					}
				}
			}
			break;
		case 0x40:	/* external signal at TI */
			break;
		case 0x60:	/* clocked with timer 0 */
			break;
		}
	}

	/**** TIMER F/F ****/
	/* timer F/F source is clock divided by 3 ? */
	if (0x02 == (TMM & 0x03))
	{
		OVCF += cycles;
		while (OVCF >= 3)
		{
			TO ^= 1;
			if (upd7810.config.io_callback)
				(*upd7810.config.io_callback)(UPD7810_TO,TO);
			OVCF -= 3;
		}
	}

	/**** ETIMER ****/
	/* ECNT clear */
	if (0x00 == (ETMM & 0x0c))
		ECNT = 0;
	else
	if (0x00 == (ETMM & 0x03) || (0x01 == (ETMM & 0x03) && CI))
	{
		OVCE += cycles;
		/* clock divided by 12 */
		while (OVCE >= 12)
		{
			OVCE -= 12;
			ECNT++;
			switch (ETMM & 0x0c)
			{
			case 0x00:				/* clear ECNT */
				break;
			case 0x04:				/* free running */
				if (0 == ECNT)
					ITF |= INTOV;	/* set overflow flag if counter wrapped */
				break;
			case 0x08:				/* reset at falling edge of CI or TO */
				break;
			case 0x0c:				/* reset if ECNT == ETM1 */
				if (ETM1 == ECNT)
					ECNT = 0;
				break;
			}
			switch (ETMM & 0x30)
			{
			case 0x00:	/* set CO0 if ECNT == ETM0 */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x10:	/* prohibited */
				break;
			case 0x20:	/* set CO0 if ECNT == ETM0 or at falling CI input */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x30:	/* latch CO0 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			}
			switch (ETMM & 0xc0)
			{
			case 0x00:	/* lacth CO1 if ECNT == ETM1 */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0x40:	/* prohibited */
				break;
			case 0x80:	/* latch CO1 if ECNT == ETM1 or falling edge of CI input */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0xc0:	/* latch CO1 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			}
		}
	}

	/**** SIO ****/
	switch (SMH & 0x03)
	{
	case 0x00:		/* interval timer F/F */
		break;
	case 0x01:		/* internal clock divided by 384 */
		OVCS += cycles;
		while (OVCS >= 384)
		{
			OVCS -= 384;
			if (0 == (EDGES ^= 1))
				upd7810_sio_input();
			else
				upd7810_sio_output();
		}
		break;
	case 0x02:		/* internal clock divided by 24 */
		OVCS += cycles;
		while (OVCS >= 24)
		{
			OVCS -= 24;
			if (0 == (EDGES ^= 1))
				upd7810_sio_input();
			else
				upd7810_sio_output();
		}
		break;
	}
}

void upd7810_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	upd7810.config = *(const UPD7810_CONFIG*) config;
	upd7810.irq_callback = irqcallback;

	state_save_register_item("upd7810", index, upd7810.ppc.w.l);
	state_save_register_item("upd7810", index, upd7810.pc.w.l);
	state_save_register_item("upd7810", index, upd7810.sp.w.l);
	state_save_register_item("upd7810", index, upd7810.psw);
	state_save_register_item("upd7810", index, upd7810.op);
	state_save_register_item("upd7810", index, upd7810.op2);
	state_save_register_item("upd7810", index, upd7810.iff);
	state_save_register_item("upd7810", index, upd7810.ea.w.l);
	state_save_register_item("upd7810", index, upd7810.va.w.l);
	state_save_register_item("upd7810", index, upd7810.bc.w.l);
	state_save_register_item("upd7810", index, upd7810.de.w.l);
	state_save_register_item("upd7810", index, upd7810.hl.w.l);
	state_save_register_item("upd7810", index, upd7810.ea2.w.l);
	state_save_register_item("upd7810", index, upd7810.va2.w.l);
	state_save_register_item("upd7810", index, upd7810.bc2.w.l);
	state_save_register_item("upd7810", index, upd7810.de2.w.l);
	state_save_register_item("upd7810", index, upd7810.hl2.w.l);
	state_save_register_item("upd7810", index, upd7810.cnt.d);
	state_save_register_item("upd7810", index, upd7810.tm.d);
	state_save_register_item("upd7810", index, upd7810.ecnt.d);
	state_save_register_item("upd7810", index, upd7810.etm.d);
	state_save_register_item("upd7810", index, upd7810.ma);
	state_save_register_item("upd7810", index, upd7810.mb);
	state_save_register_item("upd7810", index, upd7810.mcc);
	state_save_register_item("upd7810", index, upd7810.mc);
	state_save_register_item("upd7810", index, upd7810.mm);
	state_save_register_item("upd7810", index, upd7810.mf);
	state_save_register_item("upd7810", index, upd7810.tmm);
	state_save_register_item("upd7810", index, upd7810.etmm);
	state_save_register_item("upd7810", index, upd7810.eom);
	state_save_register_item("upd7810", index, upd7810.sml);
	state_save_register_item("upd7810", index, upd7810.smh);
	state_save_register_item("upd7810", index, upd7810.anm);
	state_save_register_item("upd7810", index, upd7810.mkl);
	state_save_register_item("upd7810", index, upd7810.mkh);
	state_save_register_item("upd7810", index, upd7810.zcm);
	state_save_register_item("upd7810", index, upd7810.pa_out);
	state_save_register_item("upd7810", index, upd7810.pb_out);
	state_save_register_item("upd7810", index, upd7810.pc_out);
	state_save_register_item("upd7810", index, upd7810.pd_out);
	state_save_register_item("upd7810", index, upd7810.pf_out);
	state_save_register_item("upd7810", index, upd7810.cr0);
	state_save_register_item("upd7810", index, upd7810.cr1);
	state_save_register_item("upd7810", index, upd7810.cr2);
	state_save_register_item("upd7810", index, upd7810.cr3);
	state_save_register_item("upd7810", index, upd7810.txb);
	state_save_register_item("upd7810", index, upd7810.rxb);
	state_save_register_item("upd7810", index, upd7810.txd);
	state_save_register_item("upd7810", index, upd7810.rxd);
	state_save_register_item("upd7810", index, upd7810.sck);
	state_save_register_item("upd7810", index, upd7810.ti);
	state_save_register_item("upd7810", index, upd7810.to);
	state_save_register_item("upd7810", index, upd7810.ci);
	state_save_register_item("upd7810", index, upd7810.co0);
	state_save_register_item("upd7810", index, upd7810.co1);
	state_save_register_item("upd7810", index, upd7810.irr);
	state_save_register_item("upd7810", index, upd7810.itf);
	state_save_register_item("upd7810", index, upd7810.ovc0);
	state_save_register_item("upd7810", index, upd7810.ovc1);
	state_save_register_item("upd7810", index, upd7810.ovcf);
	state_save_register_item("upd7810", index, upd7810.ovcs);
	state_save_register_item("upd7810", index, upd7810.edges);
}

#include "7810tbl.c"
#include "7810ops.c"

static void upd7810_reset (void)
{
	UPD7810_CONFIG save_config;
	int (*save_irqcallback)(int);

	save_config = upd7810.config;
	save_irqcallback = upd7810.irq_callback;
	memset(&upd7810, 0, sizeof(upd7810));
	upd7810.config = save_config;
	upd7810.irq_callback = save_irqcallback;

	upd7810.opXX = opXX_7810;
	ETMM = 0xff;
	TMM = 0xff;
	MA = 0xff;
	MB = 0xff;
	switch (upd7810.config.type)
	{
		case TYPE_7810_GAMEMASTER:
		    // needed for lcd screen/ram selection; might be internal in cpu and therefor not needed; 0x10 written in some games
			MC = 0xff&~0x7;
			WP( UPD7810_PORTC, 1 ); //hyper space
			PCD=0x8000;
			break;
		default:
			MC = 0xff;
	}
	MF = 0xff;
	// gamemaster falling block "and"s to enable interrupts
	MKL = 0xff;
	MKH = 0xff; //?
}

static void upd7807_reset (void)
{
	upd7810_reset();
	upd7810.opXX = opXX_7807;
}

static void upd7810_exit (void)
{
}

static int upd7810_execute (int cycles)
{
	const struct opcode_s *opXX = upd7810.opXX;

	upd7810_icount = cycles;

	do
	{
		int cc = 0;

		CALL_MAME_DEBUG;

		PPC = PC;
		RDOP(OP);

		/*
         * clear L0 and/or L1 flags for all opcodes except
         * L0   for "MVI L,xx" or "LXI H,xxxx"
         * L1   for "MVI A,xx"
         */
		PSW &= ~opXX[OP].mask_l0_l1;

		/* skip flag set and not SOFTI opcode? */
		if ((PSW & SK) && (OP != 0x72))
		{
			if (opXX[OP].cycles)
			{
				cc = opXX[OP].cycles_skip;
				PC += opXX[OP].oplen - 1;
			}
			else
			{
				RDOP(OP2);
				switch (OP)
				{
				case 0x48:
					cc = op48[OP2].cycles_skip;
					PC += op48[OP2].oplen - 2;
					break;
				case 0x4c:
					cc = op4C[OP2].cycles_skip;
					PC += op4C[OP2].oplen - 2;
					break;
				case 0x4d:
					cc = op4D[OP2].cycles_skip;
					PC += op4D[OP2].oplen - 2;
					break;
				case 0x60:
					cc = op60[OP2].cycles_skip;
					PC += op60[OP2].oplen - 2;
					break;
				case 0x64:
					cc = op64[OP2].cycles_skip;
					PC += op64[OP2].oplen - 2;
					break;
				case 0x70:
					cc = op70[OP2].cycles_skip;
					PC += op70[OP2].oplen - 2;
					break;
				case 0x74:
					cc = op74[OP2].cycles_skip;
					PC += op74[OP2].oplen - 2;
					break;
				default:
					fatalerror("uPD7810 internal error: check cycle counts for main");
				}
			}
			PSW &= ~SK;
			upd7810_timers( cc );
			change_pc( PCD );
		}
		else
		{
			cc = opXX[OP].cycles;
			upd7810_timers( cc );
			(*opXX[OP].opfunc)();
		}
		upd7810_icount -= cc;
		upd7810_take_irq();

	} while (upd7810_icount > 0);

	return cycles - upd7810_icount;
}

static void upd7810_get_context (void *dst)
{
	if (dst)
		memcpy(dst, &upd7810, sizeof(upd7810));
}

static void upd7810_set_context (void *src)
{
	if (src)
		memcpy(&upd7810, src, sizeof(upd7810));
}

static void set_irq_line(int irqline, int state)
{
	if (state != CLEAR_LINE)
	{
		if (irqline == INPUT_LINE_NMI)
		{
			/* no nested NMIs ? */
//          if (0 == (IRR & INTNMI))
			{
	            IRR |= INTNMI;
				SP--;
				WM( SP, PSW );
				SP--;
				WM( SP, PCH );
				SP--;
				WM( SP, PCL );
				IFF = 0;
				PSW &= ~(SK|L0|L1);
				PC = 0x0004;
				change_pc( PCD );
			}
		}
		else
		if (irqline == UPD7810_INTF1)
			IRR |= INTF1;
		else
		if (irqline == UPD7810_INTF2)
			IRR |= INTF2;
		// gamemaster hack
		else
		if (irqline == UPD7810_INTFE1)
			IRR |= INTFE1;
		else
			logerror("upd7810_set_irq_line invalid irq line #%d\n", irqline);
	}
	/* resetting interrupt requests is done with the SKIT/SKNIT opcodes only! */
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void upd7810_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF1:	set_irq_line(UPD7810_INTF1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF2:	set_irq_line(UPD7810_INTF2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTFE1:	set_irq_line(UPD7810_INTFE1, info->i);	break;

		case CPUINFO_INT_PC:							PC = info->i; change_pc(PCD);			break;
		case CPUINFO_INT_REGISTER + UPD7810_PC:			PC = info->i; 							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + UPD7810_SP:			SP = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_PSW:		PSW = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_A:			A = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_V:			V = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_EA:			EA = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_VA:			VA = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_BC:			BC = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_DE:			DE = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_HL:			HL = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_EA2:		EA2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_VA2:		VA2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_BC2:		BC2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_DE2:		DE2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_HL2:		HL2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT0:		CNT0 = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT1:		CNT1 = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_TM0:		TM0 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TM1:		TM1 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_ECNT:		ECNT = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_ECPT:		ECPT = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM0:		ETM0 = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM1:		ETM1 = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_MA:			MA = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MB:			MB = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MCC:		MCC = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MC:			MC = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MM:			MM = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MF:			MF = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TMM:		TMM = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_ETMM:		ETMM = info->i; 						break;
		case CPUINFO_INT_REGISTER + UPD7810_EOM:		EOM = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_SML:		SML = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_SMH:		SMH = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_ANM:		ANM = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MKL:		MKL = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_MKH:		MKH = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_ZCM:		ZCM = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TXB:		TXB = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_RXB:		RXB = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR0:		CR0 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR1:		CR1 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR2:		CR2 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR3:		CR3 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TXD:		TXD = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_RXD:		RXD = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_SCK:		SCK = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TI:			TI	= info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_TO:			TO	= info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CI:			CI	= info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CO0:		CO0 = info->i; 							break;
		case CPUINFO_INT_REGISTER + UPD7810_CO1:		CO1 = info->i; 							break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void upd7810_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(upd7810);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = (IRR & INTNMI) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF1:	info->i = (IRR & INTF1) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF2:	info->i = (IRR & INTF2) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTFE1:	info->i = (IRR & INTFE1) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + UPD7810_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + UPD7810_SP:			info->i = SP;							break;
		case CPUINFO_INT_REGISTER + UPD7810_PSW:		info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + UPD7810_EA:			info->i = EA;							break;
		case CPUINFO_INT_REGISTER + UPD7810_VA:			info->i = VA;							break;
		case CPUINFO_INT_REGISTER + UPD7810_BC:			info->i = BC;							break;
		case CPUINFO_INT_REGISTER + UPD7810_DE:			info->i = DE;							break;
		case CPUINFO_INT_REGISTER + UPD7810_HL:			info->i = HL;							break;
		case CPUINFO_INT_REGISTER + UPD7810_EA2:		info->i = EA2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_VA2:		info->i = VA2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_BC2:		info->i = BC2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_DE2:		info->i = DE2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_HL2:		info->i = HL2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT0:		info->i = CNT0;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT1:		info->i = CNT1;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TM0:		info->i = TM0;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TM1:		info->i = TM1;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ECNT:		info->i = ECNT;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ECPT:		info->i = ECPT;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM0:		info->i = ETM0;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM1:		info->i = ETM1;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MA:			info->i = MA;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MB:			info->i = MB;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MCC:		info->i = MCC;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MC:			info->i = MC;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MM:			info->i = MM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MF:			info->i = MF;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TMM:		info->i = TMM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ETMM:		info->i = ETMM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_EOM:		info->i = EOM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_SML:		info->i = SML;							break;
		case CPUINFO_INT_REGISTER + UPD7810_SMH:		info->i = SMH;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ANM:		info->i = ANM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MKL:		info->i = MKL;							break;
		case CPUINFO_INT_REGISTER + UPD7810_MKH:		info->i = MKH;							break;
		case CPUINFO_INT_REGISTER + UPD7810_ZCM:		info->i = ZCM;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TXB:		info->i = TXB;							break;
		case CPUINFO_INT_REGISTER + UPD7810_RXB:		info->i = RXB;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR0:		info->i = CR0;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR1:		info->i = CR1;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR2:		info->i = CR2;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CR3:		info->i = CR3;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TXD:		info->i = TXD;							break;
		case CPUINFO_INT_REGISTER + UPD7810_RXD:		info->i = RXD;							break;
		case CPUINFO_INT_REGISTER + UPD7810_SCK:		info->i = SCK;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TI:			info->i = TI;							break;
		case CPUINFO_INT_REGISTER + UPD7810_TO:			info->i = TO;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CI:			info->i = CI;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CO0:		info->i = CO0;							break;
		case CPUINFO_INT_REGISTER + UPD7810_CO1:		info->i = CO1;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = upd7810_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = upd7810_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = upd7810_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = upd7810_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = upd7810_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = upd7810_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = upd7810_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = upd7810_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &upd7810_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "uPD7810");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "NEC uPD7810");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.3");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2001 Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s:%s:%s:%s:%s:%s",
				upd7810.psw & 0x40 ? "ZF":"--",
				upd7810.psw & 0x20 ? "SK":"--",
				upd7810.psw & 0x10 ? "HC":"--",
				upd7810.psw & 0x08 ? "L1":"--",
				upd7810.psw & 0x04 ? "L0":"--",
				upd7810.psw & 0x01 ? "CY":"--");
			break;

		case CPUINFO_STR_REGISTER + UPD7810_PC:			sprintf(info->s, "PC  :%04X", upd7810.pc.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_SP:			sprintf(info->s, "SP  :%04X", upd7810.sp.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_PSW:		sprintf(info->s, "PSW :%02X", upd7810.psw); break;
		case CPUINFO_STR_REGISTER + UPD7810_A:			sprintf(info->s, "A   :%02X", upd7810.va.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_V:			sprintf(info->s, "V   :%02X", upd7810.va.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_EA:			sprintf(info->s, "EA  :%04X", upd7810.ea.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_BC:			sprintf(info->s, "BC  :%04X", upd7810.bc.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_DE:			sprintf(info->s, "DE  :%04X", upd7810.de.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_HL:			sprintf(info->s, "HL  :%04X", upd7810.hl.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_A2:			sprintf(info->s, "A'  :%02X", upd7810.va2.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_V2:			sprintf(info->s, "V'  :%02X", upd7810.va2.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_EA2:		sprintf(info->s, "EA' :%04X", upd7810.ea2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_BC2:		sprintf(info->s, "BC' :%04X", upd7810.bc2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_DE2:		sprintf(info->s, "DE' :%04X", upd7810.de2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_HL2:		sprintf(info->s, "HL' :%04X", upd7810.hl2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_CNT0:		sprintf(info->s, "CNT0:%02X", upd7810.cnt.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_CNT1:		sprintf(info->s, "CNT1:%02X", upd7810.cnt.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_TM0:		sprintf(info->s, "TM0 :%02X", upd7810.tm.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_TM1:		sprintf(info->s, "TM1 :%02X", upd7810.tm.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_ECNT:		sprintf(info->s, "ECNT:%04X", upd7810.ecnt.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_ECPT:		sprintf(info->s, "ECPT:%04X", upd7810.ecnt.w.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETM0:		sprintf(info->s, "ETM0:%04X", upd7810.etm.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETM1:		sprintf(info->s, "ETM1:%04X", upd7810.etm.w.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_MA:			sprintf(info->s, "MA  :%02X", upd7810.ma); break;
		case CPUINFO_STR_REGISTER + UPD7810_MB:			sprintf(info->s, "MB  :%02X", upd7810.mb); break;
		case CPUINFO_STR_REGISTER + UPD7810_MCC:		sprintf(info->s, "MCC :%02X", upd7810.mcc); break;
		case CPUINFO_STR_REGISTER + UPD7810_MC:			sprintf(info->s, "MC  :%02X", upd7810.mc); break;
		case CPUINFO_STR_REGISTER + UPD7810_MM:			sprintf(info->s, "MM  :%02X", upd7810.mm); break;
		case CPUINFO_STR_REGISTER + UPD7810_MF:			sprintf(info->s, "MF  :%02X", upd7810.mf); break;
		case CPUINFO_STR_REGISTER + UPD7810_TMM:		sprintf(info->s, "TMM :%02X", upd7810.tmm); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETMM:		sprintf(info->s, "ETMM:%02X", upd7810.etmm); break;
		case CPUINFO_STR_REGISTER + UPD7810_EOM:		sprintf(info->s, "EOM :%02X", upd7810.eom); break;
		case CPUINFO_STR_REGISTER + UPD7810_SML:		sprintf(info->s, "SML :%02X", upd7810.sml); break;
		case CPUINFO_STR_REGISTER + UPD7810_SMH:		sprintf(info->s, "SMH :%02X", upd7810.smh); break;
		case CPUINFO_STR_REGISTER + UPD7810_ANM:		sprintf(info->s, "ANM :%02X", upd7810.anm); break;
		case CPUINFO_STR_REGISTER + UPD7810_MKL:		sprintf(info->s, "MKL :%02X", upd7810.mkl); break;
		case CPUINFO_STR_REGISTER + UPD7810_MKH:		sprintf(info->s, "MKH :%02X", upd7810.mkh); break;
		case CPUINFO_STR_REGISTER + UPD7810_ZCM:		sprintf(info->s, "ZCM :%02X", upd7810.zcm); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR0:		sprintf(info->s, "CR0 :%02X", upd7810.cr0); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR1:		sprintf(info->s, "CR1 :%02X", upd7810.cr1); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR2:		sprintf(info->s, "CR2 :%02X", upd7810.cr2); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR3:		sprintf(info->s, "CR3 :%02X", upd7810.cr3); break;
		case CPUINFO_STR_REGISTER + UPD7810_RXB:		sprintf(info->s, "RXB :%02X", upd7810.rxb); break;
		case CPUINFO_STR_REGISTER + UPD7810_TXB:		sprintf(info->s, "TXB :%02X", upd7810.txb); break;
		case CPUINFO_STR_REGISTER + UPD7810_TXD:		sprintf(info->s, "TXD :%d", upd7810.txd); break;
		case CPUINFO_STR_REGISTER + UPD7810_RXD:		sprintf(info->s, "RXD :%d", upd7810.rxd); break;
		case CPUINFO_STR_REGISTER + UPD7810_SCK:		sprintf(info->s, "SCK :%d", upd7810.sck); break;
		case CPUINFO_STR_REGISTER + UPD7810_TI:			sprintf(info->s, "TI  :%d", upd7810.ti); break;
		case CPUINFO_STR_REGISTER + UPD7810_TO:			sprintf(info->s, "TO  :%d", upd7810.to); break;
		case CPUINFO_STR_REGISTER + UPD7810_CI:			sprintf(info->s, "CI  :%d", upd7810.ci); break;
		case CPUINFO_STR_REGISTER + UPD7810_CO0:		sprintf(info->s, "CO0 :%d", upd7810.co0 & 1); break;
		case CPUINFO_STR_REGISTER + UPD7810_CO1:		sprintf(info->s, "CO1 :%d", upd7810.co1 & 1); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void upd7807_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = upd7807_reset;			break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = upd7807_dasm;		break;
#endif /* MAME_DEBUG */

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "uPD7807");				break;

		default:										upd7810_get_info(state, info);			break;
	}
}
