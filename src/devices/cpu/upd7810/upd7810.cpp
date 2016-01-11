// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   upd7810.c
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.3
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
#include "upd7810_macros.h"


const device_type UPD7810 = &device_creator<upd7810_device>;
const device_type UPD7807 = &device_creator<upd7807_device>;
const device_type UPD7801 = &device_creator<upd7801_device>;
const device_type UPD78C05 = &device_creator<upd78c05_device>;
const device_type UPD78C06 = &device_creator<upd78c06_device>;


upd7810_device::upd7810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, UPD7810, "uPD7810", tag, owner, clock, "upd7810", __FILE__)
	, m_to_func(*this)
	, m_co0_func(*this)
	, m_co1_func(*this)
	, m_txd_func(*this)
	, m_rxd_func(*this)
	, m_an0_func(*this)
	, m_an1_func(*this)
	, m_an2_func(*this)
	, m_an3_func(*this)
	, m_an4_func(*this)
	, m_an5_func(*this)
	, m_an6_func(*this)
	, m_an7_func(*this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
{
	m_opXX = s_opXX_7810;
	m_op48 = s_op48;
	m_op4C = s_op4C;
	m_op4D = s_op4D;
	m_op60 = s_op60;
	m_op64 = s_op64;
	m_op70 = s_op70;
	m_op74 = s_op74;
}

upd7810_device::upd7810_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_to_func(*this)
	, m_co0_func(*this)
	, m_co1_func(*this)
	, m_txd_func(*this)
	, m_rxd_func(*this)
	, m_an0_func(*this)
	, m_an1_func(*this)
	, m_an2_func(*this)
	, m_an3_func(*this)
	, m_an4_func(*this)
	, m_an5_func(*this)
	, m_an6_func(*this)
	, m_an7_func(*this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
{
}

upd7807_device::upd7807_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd7810_device(mconfig, UPD7807, "uPD7807", tag, owner, clock, "upd7807", __FILE__)
{
	m_opXX = s_opXX_7807;
	m_op48 = s_op48;
	m_op4C = s_op4C;
	m_op4D = s_op4D;
	m_op60 = s_op60;
	m_op64 = s_op64;
	m_op70 = s_op70;
	m_op74 = s_op74;
}

upd7801_device::upd7801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd7810_device(mconfig, UPD7801, "uPD7801", tag, owner, clock, "upd7801", __FILE__)
{
	m_op48 = s_op48_7801;
	m_op4C = s_op4C_7801;
	m_op4D = s_op4D_7801;
	m_op60 = s_op60_7801;
	m_op64 = s_op64_7801;
	m_op70 = s_op70_7801;
	m_op74 = s_op74_7801;
	m_opXX = s_opXX_7801;
}

upd78c05_device::upd78c05_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd7810_device(mconfig, UPD78C05, "uPD78C05", tag, owner, clock, "upd78c05", __FILE__)
{
	m_op48 = s_op48_78c05;
	m_op4C = s_op4C_78c05;
	m_op4D = s_op4D_78c05;
	m_op60 = s_op60_78c05;
	m_op64 = s_op64_78c05;
	m_op70 = s_op70_78c05;
	m_op74 = s_op74_78c05;
	m_opXX = s_opXX_78c05;
}

upd78c05_device::upd78c05_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: upd7810_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

upd78c06_device::upd78c06_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd78c05_device(mconfig, UPD78C06, "uPD78C06", tag, owner, clock, "upd78c06", __FILE__)
{
	m_op48 = s_op48_78c06;
	m_op4C = s_op4C_78c06;
	m_op4D = s_op4D_78c06;
	m_op60 = s_op60_78c06;
	m_op64 = s_op64_78c06;
	m_op70 = s_op70_78c06;
	m_op74 = s_op74_78c06;
	m_opXX = s_opXX_78c06;
}

offs_t upd7810_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd7810 );
	return CPU_DISASSEMBLE_NAME(upd7810)(this, buffer, pc, oprom, opram, options);
}

offs_t upd7807_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd7807 );
	return CPU_DISASSEMBLE_NAME(upd7807)(this, buffer, pc, oprom, opram, options);
}

offs_t upd7801_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd7801 );
	return CPU_DISASSEMBLE_NAME(upd7801)(this, buffer, pc, oprom, opram, options);
}

offs_t upd78c05_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( upd78c05 );
	return CPU_DISASSEMBLE_NAME(upd78c05)(this, buffer, pc, oprom, opram, options);
}

UINT8 upd7810_device::RP(offs_t port)
{
	UINT8 data = 0xff;
	switch (port)
	{
	case UPD7810_PORTA:
		if (m_ma)   // NS20031301 no need to read if the port is set as output
			m_pa_in = m_io->read_byte(port);
		data = (m_pa_in & m_ma) | (m_pa_out & ~m_ma);
		break;
	case UPD7810_PORTB:
		if (m_mb)   // NS20031301 no need to read if the port is set as output
			m_pb_in = m_io->read_byte(port);
		data = (m_pb_in & m_mb) | (m_pb_out & ~m_mb);
		break;
	case UPD7810_PORTC:
		if (m_mc)   // NS20031301 no need to read if the port is set as output
			m_pc_in = m_io->read_byte(port);
		data = (m_pc_in & m_mc) | (m_pc_out & ~m_mc);
		if (m_mcc & 0x01)   /* PC0 = TxD output */
			data = (data & ~0x01) | (m_txd & 1 ? 0x01 : 0x00);
		if (m_mcc & 0x02)   /* PC1 = RxD input */
			data = (data & ~0x02) | (m_rxd & 1 ? 0x02 : 0x00);
		if (m_mcc & 0x04)   /* PC2 = SCK input/output */
			data = (data & ~0x04) | (m_sck & 1 ? 0x04 : 0x00);
		if (m_mcc & 0x08)   /* PC3 = TI/INT2 input */
			data = (data & ~0x08) | (m_int2 & 1 ? 0x08 : 0x00);
		if (m_mcc & 0x10)   /* PC4 = TO output */
			data = (data & ~0x10) | (m_to & 1 ? 0x10 : 0x00);
		if (m_mcc & 0x20)   /* PC5 = CI input */
			data = (data & ~0x20) | (m_ci & 1 ? 0x20 : 0x00);
		if (m_mcc & 0x40)   /* PC6 = CO0 output */
			data = (data & ~0x40) | (m_co0 & 1 ? 0x40 : 0x00);
		if (m_mcc & 0x80)   /* PC7 = CO1 output */
			data = (data & ~0x80) | (m_co1 & 1 ? 0x80 : 0x00);
		break;
	case UPD7810_PORTD:
		m_pd_in = m_io->read_byte(port);
		switch (m_mm & 0x07)
		{
		case 0x00:          /* PD input mode, PF port mode */
			data = m_pd_in;
			break;
		case 0x01:          /* PD output mode, PF port mode */
			data = m_pd_out;
			break;
		default:            /* PD extension mode, PF port/extension mode */
			data = 0xff;    /* what do we see on the port here? */
			break;
		}
		break;
	case UPD7810_PORTF:
		m_pf_in = m_io->read_byte(port);
		switch (m_mm & 0x06)
		{
		case 0x00:          /* PD input/output mode, PF port mode */
			data = (m_pf_in & m_mf) | (m_pf_out & ~m_mf);
			break;
		case 0x02:          /* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data = (m_pf_in & m_mf) | (m_pf_out & ~m_mf);
			data |= 0x0f;   /* what would we see on the lower bits here? */
			break;
		case 0x04:          /* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data = (m_pf_in & m_mf) | (m_pf_out & ~m_mf);
			data |= 0x3f;   /* what would we see on the lower bits here? */
			break;
		case 0x06:
			data = 0xff;    /* what would we see on the lower bits here? */
			break;
		}
		break;
	case UPD7807_PORTT: // NS20031301 partial implementation
		data = m_io->read_byte(port);
		break;
	default:
		logerror("uPD7810 internal error: RP() called with invalid port number\n");
	}
	return data;
}

void upd7810_device::WP(offs_t port, UINT8 data)
{
	switch (port)
	{
	case UPD7810_PORTA:
		m_pa_out = data;
//      data = (data & ~m_ma) | (m_pa_in & m_ma);
		data = (data & ~m_ma) | (m_ma); // NS20031401
		m_io->write_byte(port, data);
		break;
	case UPD7810_PORTB:
		m_pb_out = data;
//      data = (data & ~m_mb) | (m_pb_in & m_mb);
		data = (data & ~m_mb) | (m_mb); // NS20031401
		m_io->write_byte(port, data);
		break;
	case UPD7810_PORTC:
		m_pc_out = data;
//      data = (data & ~m_mc) | (m_pc_in & m_mc);
		data = (data & ~m_mc) | (m_mc); // NS20031401
		if (m_mcc & 0x01)   /* PC0 = TxD output */
			data = (data & ~0x01) | (m_txd & 1 ? 0x01 : 0x00);
		if (m_mcc & 0x02)   /* PC1 = RxD input */
			data = (data & ~0x02) | (m_rxd & 1 ? 0x02 : 0x00);
		if (m_mcc & 0x04)   /* PC2 = SCK input/output */
			data = (data & ~0x04) | (m_sck & 1 ? 0x04 : 0x00);
		if (m_mcc & 0x08)   /* PC3 = TI/INT2 input */
			data = (data & ~0x08) | (m_int2 & 1 ? 0x08 : 0x00);
		if (m_mcc & 0x10)   /* PC4 = TO output */
			data = (data & ~0x10) | (m_to & 1 ? 0x10 : 0x00);
		if (m_mcc & 0x20)   /* PC5 = CI input */
			data = (data & ~0x20) | (m_ci & 1 ? 0x20 : 0x00);
		if (m_mcc & 0x40)   /* PC6 = CO0 output */
			data = (data & ~0x40) | (m_co0 & 1 ? 0x40 : 0x00);
		if (m_mcc & 0x80)   /* PC7 = CO1 output */
			data = (data & ~0x80) | (m_co1 & 1 ? 0x80 : 0x00);
		m_io->write_byte(port, data);
		break;
	case UPD7810_PORTD:
		m_pd_out = data;
		switch (m_mm & 0x07)
		{
		case 0x00:          /* PD input mode, PF port mode */
			data = m_pd_in;
			break;
		case 0x01:          /* PD output mode, PF port mode */
			data = m_pd_out;
			break;
		default:            /* PD extension mode, PF port/extension mode */
			return;
		}
		m_io->write_byte(port, data);
		break;
	case UPD7810_PORTF:
		m_pf_out = data;
		data = (data & ~m_mf) | (m_pf_in & m_mf);
		switch (m_mm & 0x06)
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
		m_io->write_byte(port, data);
		break;
	default:
		logerror("uPD7810 internal error: RP() called with invalid port number\n");
	}
}

void upd7810_device::upd7810_take_irq()
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

	/* check the interrupts in priority sequence */
	if (IRR & INTNMI)
	{
		/* Nonmaskable interrupt */
		irqline = INPUT_LINE_NMI;
		vector = 0x0004;
		IRR &= ~INTNMI;
	}
	else
	if ((IRR & INTFT0)  && 0 == (MKL & 0x02))
	{
		vector = 0x0008;
		if (!((IRR & INTFT1)    && 0 == (MKL & 0x04)))
		IRR&=~INTFT0;
	}
	else
	if ((IRR & INTFT1)  && 0 == (MKL & 0x04))
	{
		vector = 0x0008;
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
		vector = 0x0018;
		if (!((IRR & INTFE1)    && 0 == (MKL & 0x40)))
		IRR&=~INTFE0;
	}
	else
	if ((IRR & INTFE1)  && 0 == (MKL & 0x40))
	{
		vector = 0x0018;
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

	if (vector)
	{
		/* acknowledge external IRQ */
		if (irqline)
			standard_irq_callback(irqline);
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

void upd7801_device::upd7810_take_irq()
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

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

	if (vector)
	{
		/* acknowledge external IRQ */
		if (irqline)
			standard_irq_callback(irqline);
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

void upd7810_device::upd7810_co0_output_change()
{
	/* Output LV0 Content to CO0 */
	CO0 = LV0;

	/* LV0 Level Inversion */
	if (EOM & 0x02)
		LV0 ^= 1;

	m_co0_func(CO0);
}
void upd7810_device::upd7810_co1_output_change()
{
	/* Output LV1 Content to CO1 */
	CO1 = LV1;

	/* LV1 Level Inversion */
	if (EOM & 0x20)
		LV1 ^= 1;

	m_co1_func(CO1);
}

void upd7810_device::upd7810_write_EOM()
{
	switch (EOM & 0x0c)
	{
	case 0x04:  /* To Reset LV0 */
		LV0 = 0;
		EOM &= 0xfb; /* LRE0 is reset to 0 */
		break;
	case 0x08:  /* To Set LV0 */
		LV0 = 1;
		EOM &= 0xf7; /* LRE1 is reset to 0 */
		break;
	}
	/* Output LV0 Content */
	if (EOM & 0x01) {
		upd7810_co0_output_change();
		EOM &= 0xfe; /* LO0 is reset to 0 */
	}

	switch (EOM & 0xc0)
	{
	case 0x40:  /* To Reset LV1 */
		LV1 = 0;
		EOM &= 0xbf; /* LRE2 is reset to 0 */
		break;
	case 0x80:  /* To Set LV1 */
		LV1 = 1;
		EOM &= 0x7f; /* LRE3 is reset to 0 */
		break;
	}
	/* Output LV1 Content */
	if (EOM & 0x10) {
		upd7810_co1_output_change();
		EOM &= 0xef; /* LO1 is reset to 0 */
	}
}

void upd7810_device::upd7810_write_TXB()
{
	m_txbuf = 1;
}

#define PAR7(n) ((((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)
#define PAR8(n) ((((n)>>7)^((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)

void upd7810_device::upd7810_sio_output()
{
	/* shift out more bits? */
	if (m_txcnt > 0)
	{
		TXD = m_txs & 1;
		m_txd_func(TXD);
		m_txs >>= 1;
		m_txcnt--;
		if (0 == m_txcnt)
			IRR |= INTFST;      /* serial transfer completed */
	}
	else
	if (SMH & 0x04) /* send enable ? */
	{
		/* nothing written into the transmitter buffer ? */
		if (0 == m_txbuf)
			return;
		m_txbuf = 0;

		if (SML & 0x03)         /* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:  /* 7bits, no parity, 1 stop bit */
			case 0x68:  /* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit8 */
				m_txs = (TXB << 1) | (1 << 8);
				m_txcnt = 9;
				break;
			case 0x4c:  /* 8bits, no parity, 1 stop bit */
			case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit9 */
				m_txs = (TXB << 1) | (1 << 9);
				m_txcnt = 10;
				break;
			case 0x58:  /* 7bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				m_txs = (TXB << 1) | (PAR7(TXB) << 8) | (1 << 9);
				m_txcnt = 10;
				break;
			case 0x5c:  /* 8bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				m_txs = (TXB << 1) | (PAR8(TXB) << 9) | (1 << 10);
				m_txcnt = 11;
				break;
			case 0x78:  /* 7bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				m_txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (1 << 9);
				m_txcnt = 10;
				break;
			case 0x7c:  /* 8bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				m_txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				m_txcnt = 11;
				break;
			case 0xc8:  /* 7bits, no parity, 2 stop bits */
			case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits int bit8+9 */
				m_txs = (TXB << 1) | (3 << 8);
				m_txcnt = 10;
				break;
			case 0xcc:  /* 8bits, no parity, 2 stop bits */
			case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits in bits9+10 */
				m_txs = (TXB << 1) | (3 << 9);
				m_txcnt = 11;
				break;
			case 0xd8:  /* 7bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bits9+10 */
				m_txs = (TXB << 1) | (PAR7(TXB) << 8) | (3 << 9);
				m_txcnt = 11;
				break;
			case 0xdc:  /* 8bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bit10+11 */
				m_txs = (TXB << 1) | (PAR8(TXB) << 9) | (3 << 10);
				m_txcnt = 12;
				break;
			case 0xf8:  /* 7bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bit9+10 */
				m_txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (3 << 9);
				m_txcnt = 11;
				break;
			case 0xfc:  /* 8bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bits10+10 */
				m_txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				m_txcnt = 12;
				break;
			}
		}
		else
		{
			/* synchronous mode */
			m_txs = TXB;
			m_txcnt = 8;
		}
	}
}

void upd7810_device::upd7810_sio_input()
{
	/* sample next bit? */
	if (m_rxcnt > 0)
	{
		RXD = m_rxd_func();
		m_rxs = (m_rxs >> 1) | ((UINT16)RXD << 15);
		m_rxcnt--;
		if (0 == m_rxcnt)
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
					m_rxs >>= 16 - 9;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((1 << 8) != (m_rxs & (1 | (1 << 8))))
						IRR |= INTER;   /* framing error */
					break;
				case 0x4c:  /* 8bits, no parity, 1 stop bit */
				case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					m_rxs >>= 16 - 10;
					RXB = (m_rxs >> 1) & 0xff;
					if ((1 << 9) != (m_rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					break;
				case 0x58:  /* 7bits, odd parity, 1 stop bit */
					m_rxs >>= 16 - 10;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((1 << 9) != (m_rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((m_rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x5c:  /* 8bits, odd parity, 1 stop bit */
					m_rxs >>= 16 - 11;
					RXB = (m_rxs >> 1) & 0xff;
					if ((1 << 10) != (m_rxs & (1 | (1 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((m_rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x78:  /* 7bits, even parity, 1 stop bit */
					m_rxs >>= 16 - 10;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((1 << 9) != (m_rxs & (1 | (1 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((m_rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0x7c:  /* 8bits, even parity, 1 stop bit */
					m_rxs >>= 16 - 11;
					RXB = (m_rxs >> 1) & 0xff;
					if ((1 << 10) != (m_rxs & (1 | (1 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((m_rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xc8:  /* 7bits, no parity, 2 stop bits */
				case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					m_rxs >>= 16 - 10;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((3 << 9) != (m_rxs & (1 | (3 << 9))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((m_rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xcc:  /* 8bits, no parity, 2 stop bits */
				case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					m_rxs >>= 16 - 11;
					RXB = (m_rxs >> 1) & 0xff;
					if ((3 << 10) != (m_rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((m_rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xd8:  /* 7bits, odd parity, 2 stop bits */
					m_rxs >>= 16 - 11;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((3 << 10) != (m_rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((m_rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xdc:  /* 8bits, odd parity, 2 stop bits */
					m_rxs >>= 16 - 12;
					RXB = (m_rxs >> 1) & 0xff;
					if ((3 << 11) != (m_rxs & (1 | (3 << 11))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((m_rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xf8:  /* 7bits, even parity, 2 stop bits */
					m_rxs >>= 16 - 11;
					RXB = (m_rxs >> 1) & 0x7f;
					if ((3 << 10) != (m_rxs & (1 | (3 << 10))))
						IRR |= INTER;   /* framing error */
					if (PAR7(RXB) != ((m_rxs >> 8) & 1))
						IRR |= INTER;   /* parity error */
					break;
				case 0xfc:  /* 8bits, even parity, 2 stop bits */
					m_rxs >>= 16 - 12;
					RXB = (m_rxs >> 1) & 0xff;
					if ((3 << 11) != (m_rxs & (1 | (3 << 11))))
						IRR |= INTER;   /* framing error */
					if (PAR8(RXB) != ((m_rxs >> 9) & 1))
						IRR |= INTER;   /* parity error */
					break;
				}
			}
			else
			{
				m_rxs >>= 16 - 8;
				RXB = m_rxs;
//              m_rxcnt = 8;
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
				m_rxcnt = 9;
				break;
			case 0x4c:  /* 8bits, no parity, 1 stop bit */
			case 0x6c:  /* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				m_rxcnt = 10;
				break;
			case 0x58:  /* 7bits, odd parity, 1 stop bit */
				m_rxcnt = 10;
				break;
			case 0x5c:  /* 8bits, odd parity, 1 stop bit */
				m_rxcnt = 11;
				break;
			case 0x78:  /* 7bits, even parity, 1 stop bit */
				m_rxcnt = 10;
				break;
			case 0x7c:  /* 8bits, even parity, 1 stop bit */
				m_rxcnt = 11;
				break;
			case 0xc8:  /* 7bits, no parity, 2 stop bits */
			case 0xe8:  /* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				m_rxcnt = 10;
				break;
			case 0xcc:  /* 8bits, no parity, 2 stop bits */
			case 0xec:  /* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				m_rxcnt = 11;
				break;
			case 0xd8:  /* 7bits, odd parity, 2 stop bits */
				m_rxcnt = 11;
				break;
			case 0xdc:  /* 8bits, odd parity, 2 stop bits */
				m_rxcnt = 12;
				break;
			case 0xf8:  /* 7bits, even parity, 2 stop bits */
				m_rxcnt = 11;
				break;
			case 0xfc:  /* 8bits, even parity, 2 stop bits */
				m_rxcnt = 12;
				break;
			}
		}
		else
		/* TSK bit set ? */
		if (SMH & 0x40)
		{
			m_rxcnt = 8;
		}
	}
}

void upd7810_device::upd7810_handle_timer0(int cycles, int clkdiv)
{
	OVC0 += cycles;
	while (OVC0 >= clkdiv)
	{
		OVC0 -= clkdiv;
		CNT0++;
		if (CNT0 == TM0)
		{
			CNT0 = 0;
			IRR |= INTFT0;
			/* timer F/F source is timer 0 ? */
			if (0x00 == (TMM & 0x03))
			{
				TO ^= 1;
				m_to_func(TO);
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
						m_to_func(TO);
					}
				}
			}
		}
	}
}

void upd7810_device::upd7810_handle_timer1(int cycles, int clkdiv)
{
	OVC1 += cycles;
	while (OVC1 >= clkdiv)
	{
		OVC1 -= clkdiv;
		CNT1++;
		if (CNT1 == TM1)
		{
			CNT1 = 0;
			IRR |= INTFT1;
			/* timer F/F source is timer 1 ? */
			if (0x01 == (TMM & 0x03))
			{
				TO ^= 1;
				m_to_func(TO);
			}
		}
	}
}

void upd7810_device::handle_timers(int cycles)
{
	/**** TIMER 0 ****/
	if (TMM & 0x10)         /* timer 0 upcounter reset ? */
		CNT0 = 0;
	else
	{
		switch (TMM & 0x0c) /* timer 0 clock source */
		{
		case 0x00:  /* clock divided by 12 */
			upd7810_handle_timer0(cycles, 12);
			break;
		case 0x04:  /* clock divided by 384 */
			upd7810_handle_timer0(cycles, 384);
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
			upd7810_handle_timer1(cycles, 12);
			break;
		case 0x20:  /* clock divided by 384 */
			upd7810_handle_timer1(cycles, 384);
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
			m_to_func(TO);
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
			/* Interrupt Control Circuit */
			if (ETM0 == ECNT)
				IRR |= INTFE0;
			if (ETM1 == ECNT)
				IRR |= INTFE1;
			/* Conditions When ECNT Causes a CO0 Output Change */
			if (((0x00 == (ETMM & 0x30)) && (ETM0 == ECNT)) || /* set CO0 if ECNT == ETM0 */
				/* ((0x10 == (ETMM & 0x30)) prohibited */
				((0x20 == (ETMM & 0x30)) && (ETM0 == ECNT)) || /* set CO0 if ECNT == ETM0 or at falling CI input */
				((0x30 == (ETMM & 0x30)) && (ETM0 == ECNT || ETM1 == ECNT))) /* latch CO0 if ECNT == ETM0 or ECNT == ETM1 */
			{
				upd7810_co0_output_change();
			}
			/* Conditions When ECNT Causes a CO1 Output Change */
			if (((0x00 == (ETMM & 0xc0)) && (ETM1 == ECNT)) || /* set CO1 if ECNT == ETM1 */
				/* ((0x40 == (ETMM & 0xc0)) prohibited */
				((0x80 == (ETMM & 0xc0)) && (ETM1 == ECNT)) || /* set CO1 if ECNT == ETM1 or at falling CI input */
				((0xc0 == (ETMM & 0xc0)) && (ETM0 == ECNT || ETM1 == ECNT))) /* latch CO1 if ECNT == ETM0 or ECNT == ETM1 */
			{
				upd7810_co1_output_change();
			}
			/* How and When ECNT is Cleared */
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
				upd7810_sio_input();
			else
				upd7810_sio_output();
		}
		break;
	case 0x02:      /* internal clock divided by 24 */
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

	/**** ADC ****/
	m_adcnt += cycles;
	if (PANM != ANM)
	{
		/* reset A/D converter */
		m_adcnt = 0;
		if (ANM & 0x10)
			m_adtot = 144;
		else
			m_adtot = 192;
		m_adout = 0;
		m_shdone = 0;
		if (ANM & 0x01)
		{
			/* select mode */
			m_adin = (ANM >> 1) & 0x07;
		}
		else
		{
			/* scan mode */
			m_adin    = 0;
			m_adrange = (ANM >> 1) & 0x04;
		}
	}
	PANM = ANM;
	if (ANM & 0x01)
	{
		/* select mode */
		if (m_shdone == 0)
		{
			switch (m_adin)
			{
				case 0: m_tmpcr = m_an0_func(); break;
				case 1: m_tmpcr = m_an1_func(); break;
				case 2: m_tmpcr = m_an2_func(); break;
				case 3: m_tmpcr = m_an3_func(); break;
				case 4: m_tmpcr = m_an4_func(); break;
				case 5: m_tmpcr = m_an5_func(); break;
				case 6: m_tmpcr = m_an6_func(); break;
				case 7: m_tmpcr = m_an7_func(); break;
			}
			m_shdone = 1;
		}
		if (m_adcnt > m_adtot)
		{
			m_adcnt -= m_adtot;
			switch (m_adout)
			{
				case 0: CR0 = m_tmpcr; break;
				case 1: CR1 = m_tmpcr; break;
				case 2: CR2 = m_tmpcr; break;
				case 3: CR3 = m_tmpcr; break;
			}
			m_adout = (m_adout + 1) & 0x03;
			if (m_adout == 0)
				IRR |= INTFAD;
			m_shdone = 0;
		}
	}
	else
	{
		/* scan mode */
		if (m_shdone == 0)
		{
			switch (m_adin | m_adrange)
			{
				case 0: m_tmpcr = m_an0_func(); break;
				case 1: m_tmpcr = m_an1_func(); break;
				case 2: m_tmpcr = m_an2_func(); break;
				case 3: m_tmpcr = m_an3_func(); break;
				case 4: m_tmpcr = m_an4_func(); break;
				case 5: m_tmpcr = m_an5_func(); break;
				case 6: m_tmpcr = m_an6_func(); break;
				case 7: m_tmpcr = m_an7_func(); break;
			}
			m_shdone = 1;
		}
		if (m_adcnt > m_adtot)
		{
			m_adcnt -= m_adtot;
			switch (m_adout)
			{
				case 0: CR0 = m_tmpcr; break;
				case 1: CR1 = m_tmpcr; break;
				case 2: CR2 = m_tmpcr; break;
				case 3: CR3 = m_tmpcr; break;
			}
			m_adin  = (m_adin  + 1) & 0x07;
			m_adout = (m_adout + 1) & 0x03;
			if (m_adout == 0)
				IRR |= INTFAD;
			m_shdone = 0;
		}
	}

}

void upd7801_device::handle_timers(int cycles)
{
	if ( m_ovc0 )
	{
		m_ovc0 -= cycles;

		/* Check if timer expired */
		if ( m_ovc0 <= 0 )
		{
			IRR |= INTFT0;

			/* Reset the timer flip/fliop */
			TO = 0;
			m_to_func(TO);

			/* Reload the timer */
			m_ovc0 = 16 * ( TM0 + ( ( TM1 & 0x0f ) << 8 ) );
		}
	}
}

void upd78c05_device::handle_timers(int cycles)
{
	if ( m_ovc0 ) {
		m_ovc0 -= cycles;

		if ( m_ovc0 <= 0 ) {
			IRR |= INTFT0;
			if (0x00 == (TMM & 0x03)) {
				TO ^= 1;
				m_to_func(TO);
			}

			while ( m_ovc0 <= 0 ) {
				m_ovc0 += ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
			}
		}
	}
}

void upd7810_device::base_device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	m_to_func.resolve_safe();
	m_co0_func.resolve_safe();
	m_co1_func.resolve_safe();
	m_txd_func.resolve_safe();
	m_rxd_func.resolve_safe(0);
	m_an0_func.resolve_safe(0);
	m_an1_func.resolve_safe(0);
	m_an2_func.resolve_safe(0);
	m_an3_func.resolve_safe(0);
	m_an4_func.resolve_safe(0);
	m_an5_func.resolve_safe(0);
	m_an6_func.resolve_safe(0);
	m_an7_func.resolve_safe(0);

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_sp.w.l));
	save_item(NAME(m_psw));
	save_item(NAME(m_op));
	save_item(NAME(m_op2));
	save_item(NAME(m_iff));
	save_item(NAME(m_ea.w.l));
	save_item(NAME(m_va.w.l));
	save_item(NAME(m_bc.w.l));
	save_item(NAME(m_de.w.l));
	save_item(NAME(m_hl.w.l));
	save_item(NAME(m_ea2.w.l));
	save_item(NAME(m_va2.w.l));
	save_item(NAME(m_bc2.w.l));
	save_item(NAME(m_de2.w.l));
	save_item(NAME(m_hl2.w.l));
	save_item(NAME(m_cnt.d));
	save_item(NAME(m_tm.d));
	save_item(NAME(m_ecnt.d));
	save_item(NAME(m_etm.d));
	save_item(NAME(m_ma));
	save_item(NAME(m_mb));
	save_item(NAME(m_mcc));
	save_item(NAME(m_mc));
	save_item(NAME(m_mm));
	save_item(NAME(m_mf));
	save_item(NAME(m_tmm));
	save_item(NAME(m_etmm));
	save_item(NAME(m_eom));
	save_item(NAME(m_sml));
	save_item(NAME(m_smh));
	save_item(NAME(m_anm));
	save_item(NAME(m_mkl));
	save_item(NAME(m_mkh));
	save_item(NAME(m_zcm));
	save_item(NAME(m_pa_out));
	save_item(NAME(m_pb_out));
	save_item(NAME(m_pc_out));
	save_item(NAME(m_pd_out));
	save_item(NAME(m_pf_out));
	save_item(NAME(m_cr0));
	save_item(NAME(m_cr1));
	save_item(NAME(m_cr2));
	save_item(NAME(m_cr3));
	save_item(NAME(m_txb));
	save_item(NAME(m_rxb));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_sck));
	save_item(NAME(m_ti));
	save_item(NAME(m_to));
	save_item(NAME(m_ci));
	save_item(NAME(m_lv0));
	save_item(NAME(m_lv1));
	save_item(NAME(m_co0));
	save_item(NAME(m_co1));
	save_item(NAME(m_irr));
	save_item(NAME(m_itf));
	save_item(NAME(m_ovc0));
	save_item(NAME(m_ovc1));
	save_item(NAME(m_ovcf));
	save_item(NAME(m_ovcs));
	save_item(NAME(m_edges));
	save_item(NAME(m_nmi));
	save_item(NAME(m_int1));
	save_item(NAME(m_int2));

	m_icountptr = &m_icount;
}

void upd7810_device::device_start()
{
	base_device_start();

	state_add( UPD7810_PC,   "PC",   m_pc.w.l).formatstr("%04X");
	state_add( UPD7810_SP,   "SP",   m_sp.w.l).formatstr("%04X");
	state_add( UPD7810_PSW,  "PSW",  m_psw).formatstr("%02X");
	state_add( UPD7810_A,    "A",    m_va.b.l).formatstr("%02X");
	state_add( UPD7810_V,    "V",    m_va.b.h).formatstr("%02X");
	state_add( UPD7810_EA,   "EA",   m_ea.w.l).formatstr("%04X");
	state_add( UPD7810_BC,   "BC",   m_bc.w.l).formatstr("%04X");
	state_add( UPD7810_DE,   "DE",   m_de.w.l).formatstr("%04X");
	state_add( UPD7810_HL,   "HL",   m_hl.w.l).formatstr("%04X");
	state_add( UPD7810_A2,   "A'",   m_va2.b.l).formatstr("%02X");
	state_add( UPD7810_V2,   "V'",   m_va2.b.h).formatstr("%02X");
	state_add( UPD7810_EA2,  "EA'",  m_ea2.w.l).formatstr("%04X");
	state_add( UPD7810_BC2,  "BC'",  m_bc2.w.l).formatstr("%04X");
	state_add( UPD7810_DE2,  "DE'",  m_de2.w.l).formatstr("%04X");
	state_add( UPD7810_HL2,  "HL'",  m_hl2.w.l).formatstr("%04X");
	state_add( UPD7810_CNT0, "CNT0", m_cnt.b.l).formatstr("%02X");
	state_add( UPD7810_CNT1, "CNT1", m_cnt.b.h).formatstr("%02X");
	state_add( UPD7810_TM0,  "TM0",  m_tm.b.l).formatstr("%02X");
	state_add( UPD7810_TM1,  "TM1",  m_tm.b.h).formatstr("%02X");
	state_add( UPD7810_ECNT, "ECNT", m_ecnt.w.l).formatstr("%04X");
	state_add( UPD7810_ECPT, "ECPT", m_ecnt.w.h).formatstr("%04X");
	state_add( UPD7810_ETM0, "ETM0", m_etm.w.l).formatstr("%04X");
	state_add( UPD7810_ETM1, "ETM1", m_etm.w.h).formatstr("%04X");
	state_add( UPD7810_MA,   "MA",   m_ma).formatstr("%02X");
	state_add( UPD7810_MB,   "MB",   m_mb).formatstr("%02X");
	state_add( UPD7810_MCC,  "MCC",  m_mcc).formatstr("%02X");
	state_add( UPD7810_MC,   "MC",   m_mc).formatstr("%02X");
	state_add( UPD7810_MM,   "MM",   m_mm).formatstr("%02X");
	state_add( UPD7810_MF,   "MF",   m_mf).formatstr("%02X");
	state_add( UPD7810_TMM,  "TMM",  m_tmm).formatstr("%02X");
	state_add( UPD7810_ETMM, "ETMM", m_etmm).formatstr("%02X");
	state_add( UPD7810_EOM,  "EOM",  m_eom).formatstr("%02X");
	state_add( UPD7810_SML,  "SML",  m_sml).formatstr("%02X");
	state_add( UPD7810_SMH,  "SMH",  m_smh).formatstr("%02X");
	state_add( UPD7810_ANM,  "ANM",  m_anm).formatstr("%02X");
	state_add( UPD7810_MKL,  "MKL",  m_mkl).formatstr("%02X");
	state_add( UPD7810_MKH,  "MKH",  m_mkh).formatstr("%02X");
	state_add( UPD7810_ZCM,  "ZCM",  m_zcm).formatstr("%02X");
	state_add( UPD7810_CR0,  "CR0",  m_cr0).formatstr("%02X");
	state_add( UPD7810_CR1,  "CR1",  m_cr1).formatstr("%02X");
	state_add( UPD7810_CR2,  "CR2",  m_cr2).formatstr("%02X");
	state_add( UPD7810_CR3,  "CR3",  m_cr3).formatstr("%02X");
	state_add( UPD7810_RXB,  "RXB",  m_rxb).formatstr("%02X");
	state_add( UPD7810_TXB,  "TXB",  m_txb).formatstr("%02X");
	state_add( UPD7810_TXD,  "TXD",  m_txd).formatstr("%3u");
	state_add( UPD7810_RXD,  "RXD",  m_rxd).formatstr("%3u");
	state_add( UPD7810_SCK,  "SCK",  m_sck).formatstr("%3u");
	state_add( UPD7810_TI,   "TI",   m_ti).formatstr("%3u");
	state_add( UPD7810_TO,   "TO",   m_to).formatstr("%3u");
	state_add( UPD7810_CI,   "CI",   m_ci).formatstr("%3u");
	state_add( UPD7810_LV0,  "LV0",  m_lv0).formatstr("%3u");
	state_add( UPD7810_LV1,  "LV1",  m_lv1).formatstr("%3u");
	state_add( UPD7810_CO0,  "CO0",  m_co0).formatstr("%3u");
	state_add( UPD7810_CO1,  "CO1",  m_co1).formatstr("%3u");

	state_add( STATE_GENPC, "GENPC", m_pc.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_ppc.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENSP, "GENSP", m_sp.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_psw ).formatstr("%17s").noshow();
}

void upd78c05_device::device_start()
{
	base_device_start();

	state_add( UPD7810_PC,   "PC",   m_pc.w.l).formatstr("%04X");
	state_add( UPD7810_SP,   "SP",   m_sp.w.l).formatstr("%04X");
	state_add( UPD7810_PSW,  "PSW",  m_psw).formatstr("%02X");
	state_add( UPD7810_A,    "A",    m_va.b.l).formatstr("%02X");
	state_add( UPD7810_V,    "V",    m_va.b.h).formatstr("%02X");
	state_add( UPD7810_EA,   "EA",   m_ea.w.l).formatstr("%04X");
	state_add( UPD7810_BC,   "BC",   m_bc.w.l).formatstr("%04X");
	state_add( UPD7810_DE,   "DE",   m_de.w.l).formatstr("%04X");
	state_add( UPD7810_HL,   "HL",   m_hl.w.l).formatstr("%04X");
	state_add( UPD7810_CNT0, "CNT0", m_cnt.b.l).formatstr("%02X");
	state_add( UPD7810_CNT1, "CNT1", m_cnt.b.h).formatstr("%02X");
	state_add( UPD7810_TM0,  "TM0",  m_tm.b.l).formatstr("%02X");
	state_add( UPD7810_TM1,  "TM1",  m_tm.b.h).formatstr("%02X");
	state_add( UPD7810_ECNT, "ECNT", m_ecnt.w.l).formatstr("%04X");
	state_add( UPD7810_ECPT, "ECPT", m_ecnt.w.h).formatstr("%04X");
	state_add( UPD7810_ETM0, "ETM0", m_etm.w.l).formatstr("%04X");
	state_add( UPD7810_ETM1, "ETM1", m_etm.w.h).formatstr("%04X");
	state_add( UPD7810_MB,   "MB",   m_mb).formatstr("%02X");
	state_add( UPD7810_TMM,  "TMM",  m_tmm).formatstr("%02X");
	state_add( UPD7810_MKL,  "MKL",  m_mkl).formatstr("%02X");

	state_add( STATE_GENPC, "GENPC", m_pc.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_ppc.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENSP, "GENSP", m_sp.w.l ).formatstr("%04X").noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_psw ).formatstr("%17s").noshow();

}

void upd7810_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s:%s:%s:%s:%s:%s",
				m_psw & 0x40 ? "ZF":"--",
				m_psw & 0x20 ? "SK":"--",
				m_psw & 0x10 ? "HC":"--",
				m_psw & 0x08 ? "L1":"--",
				m_psw & 0x04 ? "L0":"--",
				m_psw & 0x01 ? "CY":"--");
			break;
	}
}

void upd7810_device::device_reset()
{
	m_ppc.d = 0;
	m_pc.d = 0;
	m_sp.d = 0;
	m_op = 0;
	m_op2 = 0;
	m_iff = 0;
	m_psw = 0;
	m_ea.d = 0;
	m_va.d = 0;
	m_bc.d = 0;
	m_de.d = 0;
	m_hl.d = 0;
	m_ea2.d = 0;
	m_va2.d = 0;
	m_bc2.d = 0;
	m_de2.d = 0;
	m_hl2.d = 0;
	m_cnt.d = 0;
	m_tm.d = 0;
	m_ecnt.d = 0;
	m_etm.d = 0;
	MA = 0xff;
	MB = 0xff;
	m_mcc = 0;
	MC = 0xff;
	m_mm = 0;
	MF = 0xff;
	TMM = 0xff;
	ETMM = 0xff;
	m_eom = 0;
	m_sml = 0;
	m_smh = 0;
	m_anm = 0;
	MKL = 0xFF;
	MKH = 0xFF; // ??
	m_zcm = 0;
	m_pa_in = 0;
	m_pb_in = 0;
	m_pc_in = 0;
	m_pd_in = 0;
	m_pf_in = 0;
	m_pa_out = 0;
	m_pb_out = 0;
	m_pc_out = 0;
	m_pd_out = 0;
	m_pf_out = 0;
	m_cr0 = 0;
	m_cr1 = 0;
	m_cr2 = 0;
	m_cr3 = 0;
	m_txb = 0;
	m_rxb = 0;
	m_txd = 0;
	m_rxd = 0;
	m_sck = 0;
	m_ti = 0;
	m_to = 0;
	m_ci = 0;
	m_lv0 = 0;
	m_lv1 = 0;
	m_co0 = 0;
	m_co1 = 0;
	m_irr = 0;
	m_itf = 0;
	m_nmi = 0;
	m_int1 = 0;
	m_int2 = 1; /* physical (inverted) INT2 line state */

	m_txs = 0;
	m_rxs = 0;
	m_txcnt = 0;
	m_rxcnt = 0;
	m_txbuf = 0;
	m_ovc0 = 0;
	m_ovc1 = 0;
	m_ovce = 0;
	m_ovcf = 0;
	m_ovcs = 0;
	m_edges = 0;
	m_adcnt = 0;
	m_adtot = 0;
	m_tmpcr = 0;
	m_shdone = 0;
	m_adout = 0;
	m_adin = 0;
	m_adrange = 0;

	PANM = 0xff;
}

void upd7801_device::device_reset()
{
	upd7810_device::device_reset();
	MA = 0;     /* Port A is output port on the uPD7801 */
	m_ovc0 = 0;
}

void upd78c05_device::device_reset()
{
	upd7810_device::device_reset();
	MA = 0;     /* All outputs */
	MC = 0xFF;  /* All inputs */
	V = 0xFF;   /* The vector register is always pointing to FF00 */
	TM0 = 0xFF; /* Timer seems to be running from boot */
	m_ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
}

void upd7810_device::execute_run()
{
	do
	{
		int cc;

		debugger_instruction_hook(this, PC);

		PPC = PC;
		RDOP(OP);

		/*
		 * clear L0 and/or L1 flags for all opcodes except
		 * L0   for "MVI L,xx" or "LXI H,xxxx"
		 * L1   for "MVI A,xx"
		 */
		PSW &= ~m_opXX[OP].mask_l0_l1;

		/* skip flag set and not SOFTI opcode? */
		if ((PSW & SK) && (OP != 0x72))
		{
			if (m_opXX[OP].cycles)
			{
				cc = m_opXX[OP].cycles_skip;
				PC += m_opXX[OP].oplen - 1;
			}
			else
			{
				RDOP(OP2);
				switch (OP)
				{
				case 0x48:
					cc = m_op48[OP2].cycles_skip;
					PC += m_op48[OP2].oplen - 2;
					break;
				case 0x4c:
					cc = m_op4C[OP2].cycles_skip;
					PC += m_op4C[OP2].oplen - 2;
					break;
				case 0x4d:
					cc = m_op4D[OP2].cycles_skip;
					PC += m_op4D[OP2].oplen - 2;
					break;
				case 0x60:
					cc = m_op60[OP2].cycles_skip;
					PC += m_op60[OP2].oplen - 2;
					break;
				case 0x64:
					cc = m_op64[OP2].cycles_skip;
					PC += m_op64[OP2].oplen - 2;
					break;
				case 0x70:
					cc = m_op70[OP2].cycles_skip;
					PC += m_op70[OP2].oplen - 2;
					break;
				case 0x74:
					cc = m_op74[OP2].cycles_skip;
					PC += m_op74[OP2].oplen - 2;
					break;
				default:
					fatalerror("uPD7810 internal error: check cycle counts for main\n");
				}
			}
			PSW &= ~SK;
			handle_timers( cc );
		}
		else
		{
			cc = m_opXX[OP].cycles;
			handle_timers( cc );
			(this->*m_opXX[OP].opfunc)();
		}
		m_icount -= cc;
		upd7810_take_irq();

	} while (m_icount > 0);
}

void upd7801_device::execute_set_input(int irqline, int state)
{
	/* The uPD7801 can check for falling and rising edges changes on the INT2 input */
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
		if ( m_int1 == CLEAR_LINE && state == ASSERT_LINE )
			IRR |= INTF1;

		m_int1 = state;
		break;

	case UPD7810_INTF2:
		/* INT2 is rising or falling edge sensitive */
		/* Check if the ES bit is set then check for rising edge, otherwise falling edge */
		if ( MKL & 0x20 )
		{
			if ( m_int2 == CLEAR_LINE && state == ASSERT_LINE )
			{
				IRR |= INTF2;
			}
		}
		else
		{
			if ( m_int2 == ASSERT_LINE && state == CLEAR_LINE )
			{
				IRR |= INTF2;
			}
		}
		m_int2 = state;
		break;
	}
}

void upd7810_device::execute_set_input(int irqline, int state)
{
	switch (irqline) {
	case INPUT_LINE_NMI:
		/* NMI is falling edge sensitive */
		if ( m_nmi == CLEAR_LINE && state == ASSERT_LINE )
			IRR |= INTNMI;

		m_nmi = state;
		break;
	case UPD7810_INTF1:
		/* INT1 is rising edge sensitive */
		if ( m_int1 == CLEAR_LINE && state == ASSERT_LINE )
			IRR |= INTF1;

		m_int1 = state;
		break;
	case UPD7810_INTF2:
		/* INT2 is falling edge sensitive */
		/* we store the physical state (inverse of the logical state) */
		/* to keep the handling of port C consistent with the upd7801 */
		if ( (!m_int2) == CLEAR_LINE && state == ASSERT_LINE )
			IRR |= INTF2;

		m_int2 = !state;
		break;
	default:
		logerror("upd7810_set_irq_line invalid irq line #%d\n", irqline);
		break;
	}
	/* resetting interrupt requests is done with the SKIT/SKNIT opcodes only! */
}
