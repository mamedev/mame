/*****************************************************************************
 *
 *   upd7810.h
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.3
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
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
 * - fixed INRW_wa(cpustate)
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
 * 2008-02-24 (Wilbert Pol):
 * - Added preliminary support for uPD7801
 *   For the uPD7801 only the basic instruction set was added. The current timer
 *   and serial i/o implementations are most likely incorrect.
 * - Added basic support for uPD78C05 and uPD78C06
 *   Documentation of the actual instruction set layout is missing, so we took
 *   the uPD7801 instruction set and only kept the instructions mentioned in
 *   the little documentation available on the uPD78c05A/06A. The serial i/o
 *   implementation has not been tested and is probably incorrect.
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

#include "emu.h"
#include "debugger.h"
#include "upd7810.h"

struct upd7810_state
{
	PAIR    ppc;    /* previous program counter */
	PAIR    pc;     /* program counter */
	PAIR    sp;     /* stack pointer */
	UINT8   op;     /* opcode */
	UINT8   op2;    /* opcode part 2 */
	UINT8   iff;    /* interrupt enable flip flop */
	UINT8   psw;    /* processor status word */
	PAIR    ea;     /* extended accumulator */
	PAIR    va;     /* accumulator + vector register */
	PAIR    bc;     /* 8bit B and C registers / 16bit BC register */
	PAIR    de;     /* 8bit D and E registers / 16bit DE register */
	PAIR    hl;     /* 8bit H and L registers / 16bit HL register */
	PAIR    ea2;    /* alternate register set */
	PAIR    va2;
	PAIR    bc2;
	PAIR    de2;
	PAIR    hl2;
	PAIR    cnt;    /* 8 bit timer counter */
	PAIR    tm;     /* 8 bit timer 0/1 comparator inputs */
	PAIR    ecnt;   /* timer counter register / capture register */
	PAIR    etm;    /* timer 0/1 comparator inputs */
	UINT8   ma;     /* port A input or output mask */
	UINT8   mb;     /* port B input or output mask */
	UINT8   mcc;    /* port C control/port select */
	UINT8   mc;     /* port C input or output mask */
	UINT8   mm;     /* memory mapping */
	UINT8   mf;     /* port F input or output mask */
	UINT8   tmm;    /* timer 0 and timer 1 operating parameters */
	UINT8   etmm;   /* 16-bit multifunction timer/event counter */
	UINT8   eom;    /* 16-bit timer/event counter output control */
	UINT8   sml;    /* serial interface parameters low */
	UINT8   smh;    /* -"- high */
	UINT8   anm;    /* analog to digital converter operating parameters */
	UINT8   mkl;    /* interrupt mask low */
	UINT8   mkh;    /* -"- high */
	UINT8   zcm;    /* bias circuitry for ac zero-cross detection */
	UINT8   pa_in;  /* port A,B,C,D,F inputs */
	UINT8   pb_in;
	UINT8   pc_in;
	UINT8   pd_in;
	UINT8   pf_in;
	UINT8   pa_out; /* port A,B,C,D,F outputs */
	UINT8   pb_out;
	UINT8   pc_out;
	UINT8   pd_out;
	UINT8   pf_out;
	UINT8   cr0;    /* analog digital conversion register 0 */
	UINT8   cr1;    /* analog digital conversion register 1 */
	UINT8   cr2;    /* analog digital conversion register 2 */
	UINT8   cr3;    /* analog digital conversion register 3 */
	UINT8   txb;    /* transmitter buffer */
	UINT8   rxb;    /* receiver buffer */
	UINT8   txd;    /* port C control line states */
	UINT8   rxd;
	UINT8   sck;
	UINT8   ti;
	UINT8   to;
	UINT8   ci;
	UINT8   co0;
	UINT8   co1;
	UINT16  irr;    /* interrupt request register */
	UINT16  itf;    /* interrupt test flag register */
	int     int1;   /* keep track of current int1 state. Needed for 7801 irq checking. */
	int     int2;   /* keep track to current int2 state. Needed for 7801 irq checking. */

/* internal helper variables */
	UINT16  txs;    /* transmitter shift register */
	UINT16  rxs;    /* receiver shift register */
	UINT8   txcnt;  /* transmitter shift register bit count */
	UINT8   rxcnt;  /* receiver shift register bit count */
	UINT8   txbuf;  /* transmitter buffer was written */
	INT32   ovc0;   /* overflow counter for timer 0 (for clock div 12/384) */
	INT32   ovc1;   /* overflow counter for timer 0 (for clock div 12/384) */
	INT32   ovce;   /* overflow counter for ecnt */
	INT32   ovcf;   /* overflow counter for fixed clock div 3 mode */
	INT32   ovcs;   /* overflow counter for serial I/O */
	UINT8   edges;  /* rising/falling edge flag for serial I/O */
	const struct opcode_s *opXX;    /* opcode table */
	const struct opcode_s *op48;
	const struct opcode_s *op4C;
	const struct opcode_s *op4D;
	const struct opcode_s *op60;
	const struct opcode_s *op64;
	const struct opcode_s *op70;
	const struct opcode_s *op74;
	void (*handle_timers)(upd7810_state *cpustate, int cycles);
	UPD7810_CONFIG config;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int icount;
};

INLINE upd7810_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == UPD7810 ||
			device->type() == UPD7807 ||
			device->type() == UPD7801 ||
			device->type() == UPD78C05 ||
			device->type() == UPD78C06);
	return (upd7810_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define CY  0x01
#define F1  0x02
#define L0  0x04
#define L1  0x08
#define HC  0x10
#define SK  0x20
#define Z   0x40
#define F7  0x80

/* IRR flags */
#define INTNMI  0x0001
#define INTFT0  0x0002
#define INTFT1  0x0004
#define INTF1   0x0008
#define INTF2   0x0010
#define INTFE0  0x0020
#define INTFE1  0x0040
#define INTFEIN 0x0080
#define INTFAD  0x0100
#define INTFSR  0x0200
#define INTFST  0x0400
#define INTER   0x0800
#define INTOV   0x1000
#define INTF0   0x2000

/* ITF flags */
#define INTAN4  0x0001
#define INTAN5  0x0002
#define INTAN6  0x0004
#define INTAN7  0x0008
#define INTSB   0x0010

#define PPC     cpustate->ppc.w.l
#define PC      cpustate->pc.w.l
#define PCL     cpustate->pc.b.l
#define PCH     cpustate->pc.b.h
#define PCD     cpustate->pc.d
#define SP      cpustate->sp.w.l
#define SPL     cpustate->sp.b.l
#define SPH     cpustate->sp.b.h
#define SPD     cpustate->sp.d
#define PSW     cpustate->psw
#define OP      cpustate->op
#define OP2     cpustate->op2
#define IFF     cpustate->iff
#define EA      cpustate->ea.w.l
#define EAL     cpustate->ea.b.l
#define EAH     cpustate->ea.b.h
#define VA      cpustate->va.w.l
#define V       cpustate->va.b.h
#define A       cpustate->va.b.l
#define VAD     cpustate->va.d
#define BC      cpustate->bc.w.l
#define B       cpustate->bc.b.h
#define C       cpustate->bc.b.l
#define DE      cpustate->de.w.l
#define D       cpustate->de.b.h
#define E       cpustate->de.b.l
#define HL      cpustate->hl.w.l
#define H       cpustate->hl.b.h
#define L       cpustate->hl.b.l
#define EA2     cpustate->ea2.w.l
#define VA2     cpustate->va2.w.l
#define BC2     cpustate->bc2.w.l
#define DE2     cpustate->de2.w.l
#define HL2     cpustate->hl2.w.l

#define OVC0    cpustate->ovc0
#define OVC1    cpustate->ovc1
#define OVCE    cpustate->ovce
#define OVCF    cpustate->ovcf
#define OVCS    cpustate->ovcs
#define EDGES   cpustate->edges

#define CNT0    cpustate->cnt.b.l
#define CNT1    cpustate->cnt.b.h
#define TM0     cpustate->tm.b.l
#define TM1     cpustate->tm.b.h
#define ECNT    cpustate->ecnt.w.l
#define ECPT    cpustate->ecnt.w.h
#define ETM0    cpustate->etm.w.l
#define ETM1    cpustate->etm.w.h

#define MA      cpustate->ma
#define MB      cpustate->mb
#define MCC     cpustate->mcc
#define MC      cpustate->mc
#define MM      cpustate->mm
#define MF      cpustate->mf
#define TMM     cpustate->tmm
#define ETMM    cpustate->etmm
#define EOM     cpustate->eom
#define SML     cpustate->sml
#define SMH     cpustate->smh
#define ANM     cpustate->anm
#define MKL     cpustate->mkl
#define MKH     cpustate->mkh
#define ZCM     cpustate->zcm

#define CR0     cpustate->cr0
#define CR1     cpustate->cr1
#define CR2     cpustate->cr2
#define CR3     cpustate->cr3
#define RXB     cpustate->rxb
#define TXB     cpustate->txb

#define RXD     cpustate->rxd
#define TXD     cpustate->txd
#define SCK     cpustate->sck
#define TI      cpustate->ti
#define TO      cpustate->to
#define CI      cpustate->ci
#define CO0     cpustate->co0
#define CO1     cpustate->co1

#define IRR     cpustate->irr
#define ITF     cpustate->itf

struct opcode_s {
	void (*opfunc)(upd7810_state *cpustate);
	UINT8 oplen;
	UINT8 cycles;
	UINT8 cycles_skip;
	UINT8 mask_l0_l1;
};

#define RDOP(O)     O = cpustate->direct->read_decrypted_byte(PCD); PC++
#define RDOPARG(A)  A = cpustate->direct->read_raw_byte(PCD); PC++
#define RM(A)       cpustate->program->read_byte(A)
#define WM(A,V)     cpustate->program->write_byte(A,V)

#define ZHC_ADD(after,before,carry)     \
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (after == before) \
		PSW = (PSW&~CY) | (carry); \
	else if (after < before)            \
		PSW |= CY;          \
	else                                \
		PSW &= ~CY;             \
	if ((after & 15) < (before & 15))   \
		PSW |= HC;                      \
	else                                \
		PSW &= ~HC;
#define ZHC_SUB(after,before,carry)     \
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (before == after)                    \
		PSW = (PSW & ~CY) | (carry);    \
	else if (after > before)            \
		PSW |= CY;          \
	else                                \
		PSW &= ~CY;             \
	if ((after & 15) > (before & 15))   \
		PSW |= HC;                      \
	else                                \
		PSW &= ~HC;
#define SKIP_CY     if (CY == (PSW & CY)) PSW |= SK
#define SKIP_NC     if (0 == (PSW & CY)) PSW |= SK
#define SKIP_Z      if (Z == (PSW & Z)) PSW |= SK
#define SKIP_NZ     if (0 == (PSW & Z)) PSW |= SK
#define SET_Z(n)    if (n) PSW &= ~Z; else PSW |= Z

static UINT8 RP(upd7810_state *cpustate, offs_t port)
{
	UINT8 data = 0xff;
	switch (port)
	{
	case UPD7810_PORTA:
		if (cpustate->ma)   // NS20031301 no need to read if the port is set as output
			cpustate->pa_in = cpustate->io->read_byte(port);
		data = (cpustate->pa_in & cpustate->ma) | (cpustate->pa_out & ~cpustate->ma);
		break;
	case UPD7810_PORTB:
		if (cpustate->mb)   // NS20031301 no need to read if the port is set as output
			cpustate->pb_in = cpustate->io->read_byte(port);
		data = (cpustate->pb_in & cpustate->mb) | (cpustate->pb_out & ~cpustate->mb);
		break;
	case UPD7810_PORTC:
		if (cpustate->mc)   // NS20031301 no need to read if the port is set as output
			cpustate->pc_in = cpustate->io->read_byte(port);
		data = (cpustate->pc_in & cpustate->mc) | (cpustate->pc_out & ~cpustate->mc);
		if (cpustate->mcc & 0x01)   /* PC0 = TxD output */
			data = (data & ~0x01) | (cpustate->txd & 1 ? 0x01 : 0x00);
		if (cpustate->mcc & 0x02)   /* PC1 = RxD input */
			data = (data & ~0x02) | (cpustate->rxd & 1 ? 0x02 : 0x00);
		if (cpustate->mcc & 0x04)   /* PC2 = SCK input/output */
			data = (data & ~0x04) | (cpustate->sck & 1 ? 0x04 : 0x00);
		if (cpustate->mcc & 0x08)   /* PC3 = TI input */
			data = (data & ~0x08) | (cpustate->ti & 1 ? 0x08 : 0x00);
		if (cpustate->mcc & 0x10)   /* PC4 = TO output */
			data = (data & ~0x10) | (cpustate->to & 1 ? 0x10 : 0x00);
		if (cpustate->mcc & 0x20)   /* PC5 = CI input */
			data = (data & ~0x20) | (cpustate->ci & 1 ? 0x20 : 0x00);
		if (cpustate->mcc & 0x40)   /* PC6 = CO0 output */
			data = (data & ~0x40) | (cpustate->co0 & 1 ? 0x40 : 0x00);
		if (cpustate->mcc & 0x80)   /* PC7 = CO1 output */
			data = (data & ~0x80) | (cpustate->co1 & 1 ? 0x80 : 0x00);
		break;
	case UPD7810_PORTD:
		cpustate->pd_in = cpustate->io->read_byte(port);
		switch (cpustate->mm & 0x07)
		{
		case 0x00:          /* PD input mode, PF port mode */
			data = cpustate->pd_in;
			break;
		case 0x01:          /* PD output mode, PF port mode */
			data = cpustate->pd_out;
			break;
		default:            /* PD extension mode, PF port/extension mode */
			data = 0xff;    /* what do we see on the port here? */
			break;
		}
		break;
	case UPD7810_PORTF:
		cpustate->pf_in = cpustate->io->read_byte(port);
		switch (cpustate->mm & 0x06)
		{
		case 0x00:          /* PD input/output mode, PF port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			break;
		case 0x02:          /* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			data |= 0x0f;   /* what would we see on the lower bits here? */
			break;
		case 0x04:          /* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			data |= 0x3f;   /* what would we see on the lower bits here? */
			break;
		case 0x06:
			data = 0xff;    /* what would we see on the lower bits here? */
			break;
		}
		break;
	case UPD7807_PORTT: // NS20031301 partial implementation
		data = cpustate->io->read_byte(port);
		break;
	default:
		logerror("uPD7810 internal error: RP(cpustate) called with invalid port number\n");
	}
	return data;
}

static void WP(upd7810_state *cpustate, offs_t port, UINT8 data)
{
	switch (port)
	{
	case UPD7810_PORTA:
		cpustate->pa_out = data;
//      data = (data & ~cpustate->ma) | (cpustate->pa_in & cpustate->ma);
		data = (data & ~cpustate->ma) | (cpustate->ma); // NS20031401
		cpustate->io->write_byte(port, data);
		break;
	case UPD7810_PORTB:
		cpustate->pb_out = data;
//      data = (data & ~cpustate->mb) | (cpustate->pb_in & cpustate->mb);
		data = (data & ~cpustate->mb) | (cpustate->mb); // NS20031401
		cpustate->io->write_byte(port, data);
		break;
	case UPD7810_PORTC:
		cpustate->pc_out = data;
//      data = (data & ~cpustate->mc) | (cpustate->pc_in & cpustate->mc);
		data = (data & ~cpustate->mc) | (cpustate->mc); // NS20031401
		if (cpustate->mcc & 0x01)   /* PC0 = TxD output */
			data = (data & ~0x01) | (cpustate->txd & 1 ? 0x01 : 0x00);
		if (cpustate->mcc & 0x02)   /* PC1 = RxD input */
			data = (data & ~0x02) | (cpustate->rxd & 1 ? 0x02 : 0x00);
		if (cpustate->mcc & 0x04)   /* PC2 = SCK input/output */
			data = (data & ~0x04) | (cpustate->sck & 1 ? 0x04 : 0x00);
		if (cpustate->mcc & 0x08)   /* PC3 = TI input */
			data = (data & ~0x08) | (cpustate->ti & 1 ? 0x08 : 0x00);
		if (cpustate->mcc & 0x10)   /* PC4 = TO output */
			data = (data & ~0x10) | (cpustate->to & 1 ? 0x10 : 0x00);
		if (cpustate->mcc & 0x20)   /* PC5 = CI input */
			data = (data & ~0x20) | (cpustate->ci & 1 ? 0x20 : 0x00);
		if (cpustate->mcc & 0x40)   /* PC6 = CO0 output */
			data = (data & ~0x40) | (cpustate->co0 & 1 ? 0x40 : 0x00);
		if (cpustate->mcc & 0x80)   /* PC7 = CO1 output */
			data = (data & ~0x80) | (cpustate->co1 & 1 ? 0x80 : 0x00);
		cpustate->io->write_byte(port, data);
		break;
	case UPD7810_PORTD:
		cpustate->pd_out = data;
		switch (cpustate->mm & 0x07)
		{
		case 0x00:          /* PD input mode, PF port mode */
			data = cpustate->pd_in;
			break;
		case 0x01:          /* PD output mode, PF port mode */
			data = cpustate->pd_out;
			break;
		default:            /* PD extension mode, PF port/extension mode */
			return;
		}
		cpustate->io->write_byte(port, data);
		break;
	case UPD7810_PORTF:
		cpustate->pf_out = data;
		data = (data & ~cpustate->mf) | (cpustate->pf_in & cpustate->mf);
		switch (cpustate->mm & 0x06)
		{
		case 0x00:          /* PD input/output mode, PF port mode */
			break;
		case 0x02:          /* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data |= 0x0f;   /* what would come out for the lower bits here? */
			break;
		case 0x04:          /* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data |= 0x3f;   /* what would come out for the lower bits here? */
			break;
		case 0x06:
			data |= 0xff;   /* what would come out for the lower bits here? */
			break;
		}
		cpustate->io->write_byte(port, data);
		break;
	default:
		logerror("uPD7810 internal error: RP(cpustate) called with invalid port number\n");
	}
}

static void upd7810_take_irq(upd7810_state *cpustate)
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

	switch ( cpustate->config.type )
	{
	case TYPE_7801:
		/* 1 - SOFTI - vector at 0x0060 */
		/* 2 - INT0 - Masked by MK0 bit */
		if ( IRR & INTF0 && 0 == (MKL & 0x01 ) )
		{
			irqline = UPD7810_INTF0;
			vector = 0x0004;
			IRR &= ~INTF0;
		}
		/* 3 - INTT - Masked by MKT bit */
		if ( IRR & INTFT0 && 0 == ( MKL & 0x02 ) )
		{
			vector = 0x0008;
			IRR &= ~INTFT0;
		}
		/* 4 - INT1 - Masked by MK1 bit */
		if ( IRR & INTF1 && 0 == ( MKL & 0x04 ) )
		{
			irqline = UPD7810_INTF1;
			vector = 0x0010;
			IRR &= ~INTF1;
		}
		/* 5 - INT2 - Masked by MK2 bit */
		if ( IRR & INTF2 && 0 == ( MKL & 0x08 ) )
		{
			irqline = UPD7810_INTF2;
			vector = 0x0020;
			IRR &= ~INTF2;
		}
		/* 6 - INTS - Masked by MKS bit */
		if ( IRR & INTFST && 0 == ( MKL & 0x10 ) )
		{
			vector = 0x0040;
			IRR &= ~INTFST;
		}
		break;

	default:
		/* check the interrupts in priority sequence */
		if ((IRR & INTFT0)  && 0 == (MKL & 0x02))
		{
			switch (cpustate->config.type)
			{
				case TYPE_7810_GAMEMASTER:
					vector = 0xff2a;
					break;
				default:
					vector = 0x0008;
			}
			if (!((IRR & INTFT1)    && 0 == (MKL & 0x04)))
			IRR&=~INTFT0;
		}
		else
		if ((IRR & INTFT1)  && 0 == (MKL & 0x04))
		{
			switch (cpustate->config.type)
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
		if ((IRR & INTF1)   && 0 == (MKL & 0x08))
		{
			irqline = UPD7810_INTF1;
			vector = 0x0010;
			if (!((IRR & INTF2) && 0 == (MKL & 0x10)))
				IRR&=~INTF1;
		}
		else
		if ((IRR & INTF2)   && 0 == (MKL & 0x10))
		{
			irqline = UPD7810_INTF2;
			vector = 0x0010;
			IRR&=~INTF2;
		}
		else
		if ((IRR & INTFE0)  && 0 == (MKL & 0x20))
		{
			switch (cpustate->config.type)
			{
				case TYPE_7810_GAMEMASTER:
					vector = 0xff2d;
					break;
				default:
					vector = 0x0018;
			}
			if (!((IRR & INTFE1)    && 0 == (MKL & 0x40)))
			IRR&=~INTFE0;
		}
		else
		if ((IRR & INTFE1)  && 0 == (MKL & 0x40))
		{
			switch (cpustate->config.type)
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
		if ((IRR & INTFAD)  && 0 == (MKH & 0x01))
		{
			vector = 0x0020;
		}
		else
		if ((IRR & INTFSR)  && 0 == (MKH & 0x02))
		{
			vector = 0x0028;
			IRR&=~INTFSR;
		}
		else
		if ((IRR & INTFST)  && 0 == (MKH & 0x04))
		{
			vector = 0x0028;
			IRR&=~INTFST;
		}
		break;
	}

	if (vector)
	{
		/* acknowledge external IRQ */
		if (irqline)
			(*cpustate->irq_callback)(cpustate->device, irqline);
		SP--;
		WM( SP, PSW );
		SP--;
		WM( SP, PCH );
		SP--;
		WM( SP, PCL );
		IFF = 0;
		PSW &= ~(SK|L0|L1);
		PC = vector;
	}
}

static void upd7810_write_EOM(upd7810_state *cpustate)
{
	if (EOM & 0x01) /* output LV0 content ? */
	{
		switch (EOM & 0x0e)
		{
		case 0x02:  /* toggle CO0 */
			CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
			break;
		case 0x04:  /* reset CO0 */
			CO0 = 0;
			break;
		case 0x08:  /* set CO0 */
			CO0 = 1;
			break;
		}
	}
	if (EOM & 0x10) /* output LV0 content ? */
	{
		switch (EOM & 0xe0)
		{
		case 0x20:  /* toggle CO1 */
			CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
			break;
		case 0x40:  /* reset CO1 */
			CO1 = 0;
			break;
		case 0x80:  /* set CO1 */
			CO1 = 1;
			break;
		}
	}
}

static void upd7810_write_TXB(upd7810_state *cpustate)
{
	cpustate->txbuf = 1;
}

#define PAR7(n) ((((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)
#define PAR8(n) ((((n)>>7)^((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)

static void upd7810_sio_output(upd7810_state *cpustate)
{
	/* shift out more bits? */
	if (cpustate->txcnt > 0)
	{
		TXD = cpustate->txs & 1;
		if (cpustate->config.io_callback)
			(*cpustate->config.io_callback)(cpustate->device,UPD7810_TXD,TXD);
		cpustate->txs >>= 1;
		cpustate->txcnt--;
		if (0 == cpustate->txcnt)
			IRR |= INTFST;      /* serial transfer completed */
	}
	else
	if (SMH & 0x04) /* send enable ? */
	{
		/* nothing written into the transmitter buffer ? */
		if (0 == cpustate->txbuf)
			return;
		cpustate->txbuf = 0;

		if (SML & 0x03)         /* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:  /* 7bits, no parity, 1 stop bit */
			case 0x68:  /* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit8 */
				cpustate->txs = (TXB << 1) | (1 << 8);
				cpustate->txcnt = 9;
				break;
			case 0x4c:  /* 8bits, no parity, 1 stop bit */
			case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit9 */
				cpustate->txs = (TXB << 1) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x58:  /* 7bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				cpustate->txs = (TXB << 1) | (PAR7(TXB) << 8) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x5c:  /* 8bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				cpustate->txs = (TXB << 1) | (PAR8(TXB) << 9) | (1 << 10);
				cpustate->txcnt = 11;
				break;
			case 0x78:  /* 7bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				cpustate->txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x7c:  /* 8bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				cpustate->txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				cpustate->txcnt = 11;
				break;
			case 0xc8:  /* 7bits, no parity, 2 stop bits */
			case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits int bit8+9 */
				cpustate->txs = (TXB << 1) | (3 << 8);
				cpustate->txcnt = 10;
				break;
			case 0xcc:  /* 8bits, no parity, 2 stop bits */
			case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits in bits9+10 */
				cpustate->txs = (TXB << 1) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xd8:  /* 7bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bits9+10 */
				cpustate->txs = (TXB << 1) | (PAR7(TXB) << 8) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xdc:  /* 8bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bit10+11 */
				cpustate->txs = (TXB << 1) | (PAR8(TXB) << 9) | (3 << 10);
				cpustate->txcnt = 12;
				break;
			case 0xf8:  /* 7bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bit9+10 */
				cpustate->txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xfc:  /* 8bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bits10+10 */
				cpustate->txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				cpustate->txcnt = 12;
				break;
			}
		}
		else
		{
			/* synchronous mode */
			cpustate->txs = TXB;
			cpustate->txcnt = 8;
		}
	}
}

static void upd7810_sio_input(upd7810_state *cpustate)
{
	/* sample next bit? */
	if (cpustate->rxcnt > 0)
	{
		if (cpustate->config.io_callback)
			RXD = (*cpustate->config.io_callback)(cpustate->device,UPD7810_RXD,RXD);
		cpustate->rxs = (cpustate->rxs >> 1) | ((UINT16)RXD << 15);
		cpustate->rxcnt--;
		if (0 == cpustate->rxcnt)
		{
			/* reset the TSK bit */
			SMH &= ~0x40;
			/* serial receive completed interrupt */
			IRR |= INTFSR;
			/* now extract the data from the shift register */
			if (SML & 0x03)     /* asynchronous mode ? */
			{
				switch (SML & 0xfc)
				{
				case 0x48:  /* 7bits, no parity, 1 stop bit */
				case 0x68:  /* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 9;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 8) != (cpustate->rxs & (1 | (1 << 8))))
						IRR |= INTER;   /* framing error */
					break;
				case 0x4c:  /* 8bits, no parity, 1 stop bit */
				case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					break;
				case 0x58:  /* 7bits, odd parity, 1 stop bit */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x5c:  /* 8bits, odd parity, 1 stop bit */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 10) != (cpustate->rxs & (1 | (1 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x78:  /* 7bits, even parity, 1 stop bit */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x7c:  /* 8bits, even parity, 1 stop bit */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 10) != (cpustate->rxs & (1 | (1 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xc8:  /* 7bits, no parity, 2 stop bits */
				case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 9) != (cpustate->rxs & (1 | (3 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xcc:  /* 8bits, no parity, 2 stop bits */
				case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xd8:  /* 7bits, odd parity, 2 stop bits */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xdc:  /* 8bits, odd parity, 2 stop bits */
					cpustate->rxs >>= 16 - 12;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 11) != (cpustate->rxs & (1 | (3 << 11))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xf8:  /* 7bits, even parity, 2 stop bits */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xfc:  /* 8bits, even parity, 2 stop bits */
					cpustate->rxs >>= 16 - 12;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 11) != (cpustate->rxs & (1 | (3 << 11))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				}
			}
			else
			{
				cpustate->rxs >>= 16 - 8;
				RXB = cpustate->rxs;
//              cpustate->rxcnt = 8;
			}
		}
	}
	else
	if (SMH & 0x08) /* receive enable ? */
	{
		if (SML & 0x03)     /* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:  /* 7bits, no parity, 1 stop bit */
			case 0x68:  /* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				cpustate->rxcnt = 9;
				break;
			case 0x4c:  /* 8bits, no parity, 1 stop bit */
			case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				cpustate->rxcnt = 10;
				break;
			case 0x58:  /* 7bits, odd parity, 1 stop bit */
				cpustate->rxcnt = 10;
				break;
			case 0x5c:  /* 8bits, odd parity, 1 stop bit */
				cpustate->rxcnt = 11;
				break;
			case 0x78:  /* 7bits, even parity, 1 stop bit */
				cpustate->rxcnt = 10;
				break;
			case 0x7c:  /* 8bits, even parity, 1 stop bit */
				cpustate->rxcnt = 11;
				break;
			case 0xc8:  /* 7bits, no parity, 2 stop bits */
			case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				cpustate->rxcnt = 10;
				break;
			case 0xcc:  /* 8bits, no parity, 2 stop bits */
			case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				cpustate->rxcnt = 11;
				break;
			case 0xd8:  /* 7bits, odd parity, 2 stop bits */
				cpustate->rxcnt = 11;
				break;
			case 0xdc:  /* 8bits, odd parity, 2 stop bits */
				cpustate->rxcnt = 12;
				break;
			case 0xf8:  /* 7bits, even parity, 2 stop bits */
				cpustate->rxcnt = 11;
				break;
			case 0xfc:  /* 8bits, even parity, 2 stop bits */
				cpustate->rxcnt = 12;
				break;
			}
		}
		else
		/* TSK bit set ? */
		if (SMH & 0x40)
		{
			cpustate->rxcnt = 8;
		}
	}
}

static void upd7810_timers(upd7810_state *cpustate, int cycles)
{
	/**** TIMER 0 ****/
	if (TMM & 0x10)         /* timer 0 upcounter reset ? */
		CNT0 = 0;
	else
	{
		switch (TMM & 0x0c) /* timer 0 clock source */
		{
		case 0x00:  /* clock divided by 12 */
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
						if (cpustate->config.io_callback)
							(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
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
								if (cpustate->config.io_callback)
									(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x04:  /* clock divided by 384 */
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
						if (cpustate->config.io_callback)
							(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
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
								if (cpustate->config.io_callback)
									(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x08:  /* external signal at TI */
			break;
		case 0x0c:  /* disabled */
			break;
		}
	}

	/**** TIMER 1 ****/
	if (TMM & 0x80)         /* timer 1 upcounter reset ? */
		CNT1 = 0;
	else
	{
		switch (TMM & 0x60) /* timer 1 clock source */
		{
		case 0x00:  /* clock divided by 12 */
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
						if (cpustate->config.io_callback)
							(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
					}
				}
			}
			break;
		case 0x20:  /* clock divided by 384 */
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
						if (cpustate->config.io_callback)
							(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
					}
				}
			}
			break;
		case 0x40:  /* external signal at TI */
			break;
		case 0x60:  /* clocked with timer 0 */
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
			if (cpustate->config.io_callback)
				(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
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
			case 0x00:              /* clear ECNT */
				break;
			case 0x04:              /* free running */
				if (0 == ECNT)
					ITF |= INTOV;   /* set overflow flag if counter wrapped */
				break;
			case 0x08:              /* reset at falling edge of CI or TO */
				break;
			case 0x0c:              /* reset if ECNT == ETM1 */
				if (ETM1 == ECNT)
					ECNT = 0;
				break;
			}
			switch (ETMM & 0x30)
			{
			case 0x00:  /* set CO0 if ECNT == ETM0 */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:  /* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:  /* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:  /* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x10:  /* prohibited */
				break;
			case 0x20:  /* set CO0 if ECNT == ETM0 or at falling CI input */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:  /* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:  /* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:  /* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x30:  /* latch CO0 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:  /* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:  /* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:  /* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			}
			switch (ETMM & 0xc0)
			{
			case 0x00:  /* lacth CO1 if ECNT == ETM1 */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:  /* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:  /* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:  /* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0x40:  /* prohibited */
				break;
			case 0x80:  /* latch CO1 if ECNT == ETM1 or falling edge of CI input */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:  /* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:  /* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:  /* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0xc0:  /* latch CO1 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:  /* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:  /* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:  /* set CO1 */
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
	case 0x00:      /* interval timer F/F */
		break;
	case 0x01:      /* internal clock divided by 384 */
		OVCS += cycles;
		while (OVCS >= 384)
		{
			OVCS -= 384;
			if (0 == (EDGES ^= 1))
				upd7810_sio_input(cpustate);
			else
				upd7810_sio_output(cpustate);
		}
		break;
	case 0x02:      /* internal clock divided by 24 */
		OVCS += cycles;
		while (OVCS >= 24)
		{
			OVCS -= 24;
			if (0 == (EDGES ^= 1))
				upd7810_sio_input(cpustate);
			else
				upd7810_sio_output(cpustate);
		}
		break;
	}
}

static void upd7801_timers(upd7810_state *cpustate, int cycles)
{
	if ( cpustate->ovc0 )
	{
		cpustate->ovc0 -= cycles;

		/* Check if timer expired */
		if ( cpustate->ovc0 <= 0 )
		{
			IRR |= INTFT0;

			/* Reset the timer flip/fliop */
			TO = 0;
			if ( cpustate->config.io_callback)
				(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);

			/* Reload the timer */
			cpustate->ovc0 = 16 * ( TM0 + ( ( TM1 & 0x0f ) << 8 ) );
		}
	}
}

static void upd78c05_timers(upd7810_state *cpustate, int cycles)
{
	if ( cpustate->ovc0 ) {
		cpustate->ovc0 -= cycles;

		if ( cpustate->ovc0 <= 0 ) {
			IRR |= INTFT0;
			if (0x00 == (TMM & 0x03)) {
				TO ^= 1;
				if (cpustate->config.io_callback)
					(*cpustate->config.io_callback)(cpustate->device,UPD7810_TO,TO);
			}

			while ( cpustate->ovc0 <= 0 ) {
				cpustate->ovc0 += ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
			}
		}
	}
}

static CPU_INIT( upd7810 )
{
	upd7810_state *cpustate = get_safe_token(device);

	cpustate->config = *(const UPD7810_CONFIG*) device->static_config();
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);

	device->save_item(NAME(cpustate->ppc.w.l));
	device->save_item(NAME(cpustate->pc.w.l));
	device->save_item(NAME(cpustate->sp.w.l));
	device->save_item(NAME(cpustate->psw));
	device->save_item(NAME(cpustate->op));
	device->save_item(NAME(cpustate->op2));
	device->save_item(NAME(cpustate->iff));
	device->save_item(NAME(cpustate->ea.w.l));
	device->save_item(NAME(cpustate->va.w.l));
	device->save_item(NAME(cpustate->bc.w.l));
	device->save_item(NAME(cpustate->de.w.l));
	device->save_item(NAME(cpustate->hl.w.l));
	device->save_item(NAME(cpustate->ea2.w.l));
	device->save_item(NAME(cpustate->va2.w.l));
	device->save_item(NAME(cpustate->bc2.w.l));
	device->save_item(NAME(cpustate->de2.w.l));
	device->save_item(NAME(cpustate->hl2.w.l));
	device->save_item(NAME(cpustate->cnt.d));
	device->save_item(NAME(cpustate->tm.d));
	device->save_item(NAME(cpustate->ecnt.d));
	device->save_item(NAME(cpustate->etm.d));
	device->save_item(NAME(cpustate->ma));
	device->save_item(NAME(cpustate->mb));
	device->save_item(NAME(cpustate->mcc));
	device->save_item(NAME(cpustate->mc));
	device->save_item(NAME(cpustate->mm));
	device->save_item(NAME(cpustate->mf));
	device->save_item(NAME(cpustate->tmm));
	device->save_item(NAME(cpustate->etmm));
	device->save_item(NAME(cpustate->eom));
	device->save_item(NAME(cpustate->sml));
	device->save_item(NAME(cpustate->smh));
	device->save_item(NAME(cpustate->anm));
	device->save_item(NAME(cpustate->mkl));
	device->save_item(NAME(cpustate->mkh));
	device->save_item(NAME(cpustate->zcm));
	device->save_item(NAME(cpustate->pa_out));
	device->save_item(NAME(cpustate->pb_out));
	device->save_item(NAME(cpustate->pc_out));
	device->save_item(NAME(cpustate->pd_out));
	device->save_item(NAME(cpustate->pf_out));
	device->save_item(NAME(cpustate->cr0));
	device->save_item(NAME(cpustate->cr1));
	device->save_item(NAME(cpustate->cr2));
	device->save_item(NAME(cpustate->cr3));
	device->save_item(NAME(cpustate->txb));
	device->save_item(NAME(cpustate->rxb));
	device->save_item(NAME(cpustate->txd));
	device->save_item(NAME(cpustate->rxd));
	device->save_item(NAME(cpustate->sck));
	device->save_item(NAME(cpustate->ti));
	device->save_item(NAME(cpustate->to));
	device->save_item(NAME(cpustate->ci));
	device->save_item(NAME(cpustate->co0));
	device->save_item(NAME(cpustate->co1));
	device->save_item(NAME(cpustate->irr));
	device->save_item(NAME(cpustate->itf));
	device->save_item(NAME(cpustate->ovc0));
	device->save_item(NAME(cpustate->ovc1));
	device->save_item(NAME(cpustate->ovcf));
	device->save_item(NAME(cpustate->ovcs));
	device->save_item(NAME(cpustate->edges));
	device->save_item(NAME(cpustate->int1));
	device->save_item(NAME(cpustate->int2));
}

#include "7810tbl.c"
#include "7810ops.c"

static CPU_RESET( upd7810 )
{
	upd7810_state *cpustate = get_safe_token(device);
	UPD7810_CONFIG save_config;
	device_irq_acknowledge_callback save_irqcallback;

	save_config = cpustate->config;
	save_irqcallback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->config = save_config;
	cpustate->irq_callback = save_irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);

	cpustate->opXX = opXX_7810;
	cpustate->op48 = op48;
	cpustate->op4C = op4C;
	cpustate->op4D = op4D;
	cpustate->op60 = op60;
	cpustate->op64 = op64;
	cpustate->op70 = op70;
	cpustate->op74 = op74;
	ETMM = 0xff;
	TMM = 0xff;
	MA = 0xff;
	MB = 0xff;
	switch (cpustate->config.type)
	{
		case TYPE_7810_GAMEMASTER:
			// needed for lcd screen/ram selection; might be internal in cpu and therefor not needed; 0x10 written in some games
			MC = 0xff&~0x7;
			WP( cpustate, UPD7810_PORTC, 1 ); //hyper space
			PCD=0x8000;
			break;
		default:
			MC = 0xff;
	}
	MF = 0xff;
	// gamemaster falling block "and"s to enable interrupts
	MKL = 0xff;
	MKH = 0xff; //?
	cpustate->handle_timers = upd7810_timers;
}

static CPU_RESET( upd7807 )
{
	upd7810_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->opXX = opXX_7807;
}

static CPU_RESET( upd7801 )
{
	upd7810_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->op48 = op48_7801;
	cpustate->op4C = op4C_7801;
	cpustate->op4D = op4D_7801;
	cpustate->op60 = op60_7801;
	cpustate->op64 = op64_7801;
	cpustate->op70 = op70_7801;
	cpustate->op74 = op74_7801;
	cpustate->opXX = opXX_7801;
	cpustate->handle_timers = upd7801_timers;
	MA = 0;     /* Port A is output port on the uPD7801 */
	cpustate->ovc0 = 0;
}

static CPU_RESET( upd78c05 )
{
	upd7810_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->op48 = op48_78c05;
	cpustate->op4C = op4C_78c05;
	cpustate->op4D = op4D_78c05;
	cpustate->op60 = op60_78c05;
	cpustate->op64 = op64_78c05;
	cpustate->op70 = op70_78c05;
	cpustate->op74 = op74_78c05;
	cpustate->opXX = opXX_78c05;
	MA = 0;     /* All outputs */
	MC = 0xFF;  /* All inputs */
	V = 0xFF;   /* The vector register is always pointing to FF00 */
	cpustate->handle_timers = upd78c05_timers;
	TM0 = 0xFF; /* Timer seems to be running from boot */
	cpustate->ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
}

static CPU_RESET( upd78c06 )
{
	upd7810_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd78c05);
	cpustate->op48 = op48_78c06;
	cpustate->op4C = op4C_78c06;
	cpustate->op4D = op4D_78c06;
	cpustate->op60 = op60_78c06;
	cpustate->op64 = op64_78c06;
	cpustate->op70 = op70_78c06;
	cpustate->op74 = op74_78c06;
	cpustate->opXX = opXX_78c06;
}

static CPU_EXIT( upd7810 )
{
}

static CPU_EXECUTE( upd7810 )
{
	upd7810_state *cpustate = get_safe_token(device);

	do
	{
		int cc = 0;

		debugger_instruction_hook(device, PC);

		PPC = PC;
		RDOP(OP);

		/*
		 * clear L0 and/or L1 flags for all opcodes except
		 * L0   for "MVI L,xx" or "LXI H,xxxx"
		 * L1   for "MVI A,xx"
		 */
		PSW &= ~cpustate->opXX[OP].mask_l0_l1;

		/* skip flag set and not SOFTI opcode? */
		if ((PSW & SK) && (OP != 0x72))
		{
			if (cpustate->opXX[OP].cycles)
			{
				cc = cpustate->opXX[OP].cycles_skip;
				PC += cpustate->opXX[OP].oplen - 1;
			}
			else
			{
				RDOP(OP2);
				switch (OP)
				{
				case 0x48:
					cc = cpustate->op48[OP2].cycles_skip;
					PC += cpustate->op48[OP2].oplen - 2;
					break;
				case 0x4c:
					cc = cpustate->op4C[OP2].cycles_skip;
					PC += cpustate->op4C[OP2].oplen - 2;
					break;
				case 0x4d:
					cc = cpustate->op4D[OP2].cycles_skip;
					PC += cpustate->op4D[OP2].oplen - 2;
					break;
				case 0x60:
					cc = cpustate->op60[OP2].cycles_skip;
					PC += cpustate->op60[OP2].oplen - 2;
					break;
				case 0x64:
					cc = cpustate->op64[OP2].cycles_skip;
					PC += cpustate->op64[OP2].oplen - 2;
					break;
				case 0x70:
					cc = cpustate->op70[OP2].cycles_skip;
					PC += cpustate->op70[OP2].oplen - 2;
					break;
				case 0x74:
					cc = cpustate->op74[OP2].cycles_skip;
					PC += cpustate->op74[OP2].oplen - 2;
					break;
				default:
					fatalerror("uPD7810 internal error: check cycle counts for main\n");
				}
			}
			PSW &= ~SK;
			cpustate->handle_timers( cpustate, cc );
		}
		else
		{
			cc = cpustate->opXX[OP].cycles;
			cpustate->handle_timers( cpustate, cc );
			(*cpustate->opXX[OP].opfunc)(cpustate);
		}
		cpustate->icount -= cc;
		upd7810_take_irq(cpustate);

	} while (cpustate->icount > 0);
}

static void set_irq_line(upd7810_state *cpustate, int irqline, int state)
{
	/* The uPD7801 can check for falling and rising edges changes on the INT2 input */
	switch ( cpustate->config.type )
	{
	case TYPE_7801:
		switch ( irqline )
		{
		case UPD7810_INTF0:
			/* INT0 is level sensitive */
			if ( state == ASSERT_LINE )
				IRR |= INTF0;
			else
				IRR &= INTF0;
			break;

		case UPD7810_INTF1:
			/* INT1 is rising edge sensitive */
			if ( cpustate->int1 == CLEAR_LINE && state == ASSERT_LINE )
				IRR |= INTF1;

			cpustate->int1 = state;
			break;

		case UPD7810_INTF2:
			/* INT2 is rising or falling edge sensitive */
			/* Check if the ES bit is set then check for rising edge, otherwise falling edge */
			if ( MKL & 0x20 )
			{
				if ( cpustate->int2 == CLEAR_LINE && state == ASSERT_LINE )
				{
					IRR |= INTF2;
				}
			}
			else
			{
				if ( cpustate->int2 == ASSERT_LINE && state == CLEAR_LINE )
				{
					IRR |= INTF2;
				}
			}
			cpustate->int2 = state;
			break;
		}
		break;

	default:
		if (state != CLEAR_LINE)
		{
			if (irqline == INPUT_LINE_NMI)
			{
				/* no nested NMIs ? */
//              if (0 == (IRR & INTNMI))
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
				}
			}
			else
			if (irqline == UPD7810_INTF1)
				IRR |= INTF1;
			else
			if ( irqline == UPD7810_INTF2 && ( MKL & 0x20 ) )
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
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( upd7810 )
{
	upd7810_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:  set_irq_line(cpustate, INPUT_LINE_NMI, info->i);    break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF1:   set_irq_line(cpustate, UPD7810_INTF1, info->i); break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF2:   set_irq_line(cpustate, UPD7810_INTF2, info->i); break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTFE1:  set_irq_line(cpustate, UPD7810_INTFE1, info->i);    break;

		case CPUINFO_INT_PC:                            PC = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_PC:         PC = info->i;                           break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + UPD7810_SP:         SP = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_PSW:        PSW = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_A:          A = info->i;                            break;
		case CPUINFO_INT_REGISTER + UPD7810_V:          V = info->i;                            break;
		case CPUINFO_INT_REGISTER + UPD7810_EA:         EA = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_VA:         VA = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_BC:         BC = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_DE:         DE = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_HL:         HL = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_EA2:        EA2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_VA2:        VA2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_BC2:        BC2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_DE2:        DE2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_HL2:        HL2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT0:       CNT0 = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT1:       CNT1 = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_TM0:        TM0 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TM1:        TM1 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ECNT:       ECNT = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ECPT:       ECPT = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM0:       ETM0 = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM1:       ETM1 = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_MA:         MA = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MB:         MB = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MCC:        MCC = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MC:         MC = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MM:         MM = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MF:         MF = info->i;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_TMM:        TMM = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ETMM:       ETMM = info->i;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_EOM:        EOM = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SML:        SML = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SMH:        SMH = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ANM:        ANM = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MKL:        MKL = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MKH:        MKH = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ZCM:        ZCM = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TXB:        TXB = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_RXB:        RXB = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR0:        CR0 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR1:        CR1 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR2:        CR2 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR3:        CR3 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TXD:        TXD = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_RXD:        RXD = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SCK:        SCK = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TI:         TI  = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TO:         TO  = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CI:         CI  = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CO0:        CO0 = info->i;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CO1:        CO1 = info->i;                          break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( upd7810 )
{
	upd7810_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(upd7810_state);                break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = 2;                            break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_LITTLE;                    break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 1;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 4;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 40;                           break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;                  break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                   break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:  info->i = (IRR & INTNMI) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF1:   info->i = (IRR & INTF1) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTF2:   info->i = (IRR & INTF2) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + UPD7810_INTFE1:  info->i = (IRR & INTFE1) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:                    info->i = PPC;                          break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + UPD7810_PC:         info->i = PC;                           break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + UPD7810_SP:         info->i = SP;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_PSW:        info->i = PSW;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_EA:         info->i = EA;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_VA:         info->i = VA;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_BC:         info->i = BC;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_DE:         info->i = DE;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_HL:         info->i = HL;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_EA2:        info->i = EA2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_VA2:        info->i = VA2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_BC2:        info->i = BC2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_DE2:        info->i = DE2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_HL2:        info->i = HL2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT0:       info->i = CNT0;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_CNT1:       info->i = CNT1;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_TM0:        info->i = TM0;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TM1:        info->i = TM1;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ECNT:       info->i = ECNT;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ECPT:       info->i = ECPT;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM0:       info->i = ETM0;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_ETM1:       info->i = ETM1;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_MA:         info->i = MA;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MB:         info->i = MB;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MCC:        info->i = MCC;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MC:         info->i = MC;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MM:         info->i = MM;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_MF:         info->i = MF;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_TMM:        info->i = TMM;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ETMM:       info->i = ETMM;                         break;
		case CPUINFO_INT_REGISTER + UPD7810_EOM:        info->i = EOM;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SML:        info->i = SML;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SMH:        info->i = SMH;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ANM:        info->i = ANM;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MKL:        info->i = MKL;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_MKH:        info->i = MKH;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_ZCM:        info->i = ZCM;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TXB:        info->i = TXB;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_RXB:        info->i = RXB;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR0:        info->i = CR0;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR1:        info->i = CR1;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR2:        info->i = CR2;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CR3:        info->i = CR3;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TXD:        info->i = TXD;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_RXD:        info->i = RXD;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_SCK:        info->i = SCK;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_TI:         info->i = TI;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_TO:         info->i = TO;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_CI:         info->i = CI;                           break;
		case CPUINFO_INT_REGISTER + UPD7810_CO0:        info->i = CO0;                          break;
		case CPUINFO_INT_REGISTER + UPD7810_CO1:        info->i = CO1;                          break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(upd7810);     break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(upd7810);                break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(upd7810);          break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(upd7810);                break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(upd7810);      break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(upd7810);      break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;           break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "uPD7810");             break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "NEC uPD7810");         break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "0.3");                 break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s:%s:%s:%s:%s:%s",
				cpustate->psw & 0x40 ? "ZF":"--",
				cpustate->psw & 0x20 ? "SK":"--",
				cpustate->psw & 0x10 ? "HC":"--",
				cpustate->psw & 0x08 ? "L1":"--",
				cpustate->psw & 0x04 ? "L0":"--",
				cpustate->psw & 0x01 ? "CY":"--");
			break;

		case CPUINFO_STR_REGISTER + UPD7810_PC:         sprintf(info->s, "PC  :%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_SP:         sprintf(info->s, "SP  :%04X", cpustate->sp.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_PSW:        sprintf(info->s, "PSW :%02X", cpustate->psw); break;
		case CPUINFO_STR_REGISTER + UPD7810_A:          sprintf(info->s, "A   :%02X", cpustate->va.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_V:          sprintf(info->s, "V   :%02X", cpustate->va.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_EA:         sprintf(info->s, "EA  :%04X", cpustate->ea.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_BC:         sprintf(info->s, "BC  :%04X", cpustate->bc.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_DE:         sprintf(info->s, "DE  :%04X", cpustate->de.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_HL:         sprintf(info->s, "HL  :%04X", cpustate->hl.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_A2:         sprintf(info->s, "A'  :%02X", cpustate->va2.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_V2:         sprintf(info->s, "V'  :%02X", cpustate->va2.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_EA2:        sprintf(info->s, "EA' :%04X", cpustate->ea2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_BC2:        sprintf(info->s, "BC' :%04X", cpustate->bc2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_DE2:        sprintf(info->s, "DE' :%04X", cpustate->de2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_HL2:        sprintf(info->s, "HL' :%04X", cpustate->hl2.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_CNT0:       sprintf(info->s, "CNT0:%02X", cpustate->cnt.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_CNT1:       sprintf(info->s, "CNT1:%02X", cpustate->cnt.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_TM0:        sprintf(info->s, "TM0 :%02X", cpustate->tm.b.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_TM1:        sprintf(info->s, "TM1 :%02X", cpustate->tm.b.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_ECNT:       sprintf(info->s, "ECNT:%04X", cpustate->ecnt.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_ECPT:       sprintf(info->s, "ECPT:%04X", cpustate->ecnt.w.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETM0:       sprintf(info->s, "ETM0:%04X", cpustate->etm.w.l); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETM1:       sprintf(info->s, "ETM1:%04X", cpustate->etm.w.h); break;
		case CPUINFO_STR_REGISTER + UPD7810_MA:         sprintf(info->s, "MA  :%02X", cpustate->ma); break;
		case CPUINFO_STR_REGISTER + UPD7810_MB:         sprintf(info->s, "MB  :%02X", cpustate->mb); break;
		case CPUINFO_STR_REGISTER + UPD7810_MCC:        sprintf(info->s, "MCC :%02X", cpustate->mcc); break;
		case CPUINFO_STR_REGISTER + UPD7810_MC:         sprintf(info->s, "MC  :%02X", cpustate->mc); break;
		case CPUINFO_STR_REGISTER + UPD7810_MM:         sprintf(info->s, "MM  :%02X", cpustate->mm); break;
		case CPUINFO_STR_REGISTER + UPD7810_MF:         sprintf(info->s, "MF  :%02X", cpustate->mf); break;
		case CPUINFO_STR_REGISTER + UPD7810_TMM:        sprintf(info->s, "TMM :%02X", cpustate->tmm); break;
		case CPUINFO_STR_REGISTER + UPD7810_ETMM:       sprintf(info->s, "ETMM:%02X", cpustate->etmm); break;
		case CPUINFO_STR_REGISTER + UPD7810_EOM:        sprintf(info->s, "EOM :%02X", cpustate->eom); break;
		case CPUINFO_STR_REGISTER + UPD7810_SML:        sprintf(info->s, "SML :%02X", cpustate->sml); break;
		case CPUINFO_STR_REGISTER + UPD7810_SMH:        sprintf(info->s, "SMH :%02X", cpustate->smh); break;
		case CPUINFO_STR_REGISTER + UPD7810_ANM:        sprintf(info->s, "ANM :%02X", cpustate->anm); break;
		case CPUINFO_STR_REGISTER + UPD7810_MKL:        sprintf(info->s, "MKL :%02X", cpustate->mkl); break;
		case CPUINFO_STR_REGISTER + UPD7810_MKH:        sprintf(info->s, "MKH :%02X", cpustate->mkh); break;
		case CPUINFO_STR_REGISTER + UPD7810_ZCM:        sprintf(info->s, "ZCM :%02X", cpustate->zcm); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR0:        sprintf(info->s, "CR0 :%02X", cpustate->cr0); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR1:        sprintf(info->s, "CR1 :%02X", cpustate->cr1); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR2:        sprintf(info->s, "CR2 :%02X", cpustate->cr2); break;
		case CPUINFO_STR_REGISTER + UPD7810_CR3:        sprintf(info->s, "CR3 :%02X", cpustate->cr3); break;
		case CPUINFO_STR_REGISTER + UPD7810_RXB:        sprintf(info->s, "RXB :%02X", cpustate->rxb); break;
		case CPUINFO_STR_REGISTER + UPD7810_TXB:        sprintf(info->s, "TXB :%02X", cpustate->txb); break;
		case CPUINFO_STR_REGISTER + UPD7810_TXD:        sprintf(info->s, "TXD :%d", cpustate->txd); break;
		case CPUINFO_STR_REGISTER + UPD7810_RXD:        sprintf(info->s, "RXD :%d", cpustate->rxd); break;
		case CPUINFO_STR_REGISTER + UPD7810_SCK:        sprintf(info->s, "SCK :%d", cpustate->sck); break;
		case CPUINFO_STR_REGISTER + UPD7810_TI:         sprintf(info->s, "TI  :%d", cpustate->ti); break;
		case CPUINFO_STR_REGISTER + UPD7810_TO:         sprintf(info->s, "TO  :%d", cpustate->to); break;
		case CPUINFO_STR_REGISTER + UPD7810_CI:         sprintf(info->s, "CI  :%d", cpustate->ci); break;
		case CPUINFO_STR_REGISTER + UPD7810_CO0:        sprintf(info->s, "CO0 :%d", cpustate->co0 & 1); break;
		case CPUINFO_STR_REGISTER + UPD7810_CO1:        sprintf(info->s, "CO1 :%d", cpustate->co1 & 1); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( upd7807 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(upd7807);          break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(upd7807);      break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "uPD7807");             break;

		default:                                        CPU_GET_INFO_CALL(upd7810);             break;
	}
}

CPU_GET_INFO( upd7801 ) {
	switch( state ) {
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(upd7801);          break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(upd7801);      break;

		case CPUINFO_STR_NAME:                          strcpy(info->s, "uPD7801");             break;

		default:                                        CPU_GET_INFO_CALL(upd7810);             break;
	}
}

CPU_GET_INFO( upd78c05 ) {
	switch ( state ) {
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 4;                            break;

		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(upd78c05);         break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(upd78c05);     break;

		case CPUINFO_STR_NAME:                          strcpy(info->s, "uPD78C05");            break;

		/* These registers are not present in the uPD78C05 cpu */
		case CPUINFO_STR_REGISTER + UPD7810_A2:
		case CPUINFO_STR_REGISTER + UPD7810_V2:
		case CPUINFO_STR_REGISTER + UPD7810_EA2:
		case CPUINFO_STR_REGISTER + UPD7810_BC2:
		case CPUINFO_STR_REGISTER + UPD7810_DE2:
		case CPUINFO_STR_REGISTER + UPD7810_HL2:
		case CPUINFO_STR_REGISTER + UPD7810_MA:
		case CPUINFO_STR_REGISTER + UPD7810_MCC:
		case CPUINFO_STR_REGISTER + UPD7810_MC:
		case CPUINFO_STR_REGISTER + UPD7810_MM:
		case CPUINFO_STR_REGISTER + UPD7810_MF:
		case CPUINFO_STR_REGISTER + UPD7810_ETMM:
		case CPUINFO_STR_REGISTER + UPD7810_EOM:
		case CPUINFO_STR_REGISTER + UPD7810_SML:
		case CPUINFO_STR_REGISTER + UPD7810_SMH:
		case CPUINFO_STR_REGISTER + UPD7810_ANM:
		case CPUINFO_STR_REGISTER + UPD7810_MKH:
		case CPUINFO_STR_REGISTER + UPD7810_ZCM:
		case CPUINFO_STR_REGISTER + UPD7810_CR0:
		case CPUINFO_STR_REGISTER + UPD7810_CR1:
		case CPUINFO_STR_REGISTER + UPD7810_CR2:
		case CPUINFO_STR_REGISTER + UPD7810_CR3:
		case CPUINFO_STR_REGISTER + UPD7810_RXB:
		case CPUINFO_STR_REGISTER + UPD7810_TXB:
		case CPUINFO_STR_REGISTER + UPD7810_TXD:
		case CPUINFO_STR_REGISTER + UPD7810_RXD:
		case CPUINFO_STR_REGISTER + UPD7810_SCK:
		case CPUINFO_STR_REGISTER + UPD7810_TI:
		case CPUINFO_STR_REGISTER + UPD7810_TO:
		case CPUINFO_STR_REGISTER + UPD7810_CI:
		case CPUINFO_STR_REGISTER + UPD7810_CO0:
		case CPUINFO_STR_REGISTER + UPD7810_CO1:        break;

		default:                                        CPU_GET_INFO_CALL(upd7801);             break;
	}
}

CPU_GET_INFO( upd78c06 ) {
	switch ( state ) {
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(upd78c06);         break;

		case CPUINFO_STR_NAME:                          strcpy(info->s, "uPD78C06");            break;

		default:                                        CPU_GET_INFO_CALL(upd78c05);                break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(UPD7810, upd7810);
DEFINE_LEGACY_CPU_DEVICE(UPD7807, upd7807);
DEFINE_LEGACY_CPU_DEVICE(UPD7801, upd7801);
DEFINE_LEGACY_CPU_DEVICE(UPD78C05, upd78c05);
DEFINE_LEGACY_CPU_DEVICE(UPD78C06, upd78c06);
