// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

----------------------------------------------------------------------------

The Alpha-8201/830x isn't a real CPU. It is a Hitachi HD44801 4-bit MCU,
programmed to interpret an external program using a custom instruction set.
Alpha-8301 has an expanded instruction set, backwards compatible with Alpha-8201.


Game                      Year   MCU
------------------------  ----   ---
Shougi                    1982?  8201 (pcb)
Shougi 2                  1982?  8201 (pcb)
Talbot                    1982   8201?
Champion Base Ball        1983   8201 (schematics)
Exciting Soccer           1983   8302 (pcb)
Champion Base Ball II     1983   8302 (pcb, unofficial schematics)
Exciting Soccer II        1984   8303 (uses 8303+ opcodes)
Equites                   1984   8303 (post)
Bull Fighter              1984   8303 (post)
Splendor Blast            1985   8303 (post)
Gekisou                   1985   8304 (post)
The Koukouyakyuh          1985   8304 (post)
High Voltage              1985   8404?(post says 8404, but readme says 8304)

ALPHA-8201: "44801A75" -> HD44801, ROM code = A75
ALPHA-8302: "44801B35" -> HD44801, ROM code = B35
ALPHA-8303: "44801B42" -> HD44801, ROM code = B42
ALPHA-8304: ?


package / pin assign
ALPHA 8201 DIP 42

pin    HD44801  Alpha
---    -------  -----
1    : D3       WR
2-4  : D4-D6    n.c.
5-7  : D7-D9    GND in shougi , n.c. in champbas
8-13 : D10-D15  n.c.
14   : n.c.     n.c.
15   : RESET    RESET
16   : GND      GND
17   : OSC1     (champbas=384KHz)
18   : OSC2     n.c.
19   : !HLT     Vcc
20   : !TEST    Vcc
21   : Vcc      Vcc
22-25: R00-R03  DB4-DB7
26-29: R10-R13  DB0-DB3
30   : INT0     GO (input)
31   : INT1     n.c.
32-35: R20-R23  A4-A7
36-39: R30-R33  A0-A3
40-41: D0-D1    A8-A9
42   : D2       /RD


TODO:
- bus conflicts?
- support larger RAM size, if any game uses it


----------------------------------------------------------------------------

Interpreted CPU reverse engineered info by Tatsuyuki Satoh


-----------------------
Register Set
-----------------------

PC      : 10bit Program Pointer
          A lower 8bits are loaded from the immediate.
          A higher 2bits are loaded from the MB register.

MB      : 2bit memory bank register, load PC[9:8] after branch
          load high to higher 2bit of PC after branch.

RB      : 3bit register bank select register

R0-R7   : internal? RAM register 8bitx8 (x8 bank)

A       : 8bit

IX0/1   : memory indirect 'read' access pointer

IX2     : memory indirect 'write' access pointer

RXB     : unknown , looks index register

LP0/1/2 : loop count register used by DJNZ operation

cf      : carry flag
zf      : zero flag


-----------------------
Memory Space
-----------------------

000     : unknown ...
001     : bit4..0 = pointer of current entry , bit7..6 = unknown
002-003 : entrypoint1  vector
004-005 : entrypoint2  vector
006-007 : entrypoint3  vector
008-009 : entrypoint4  vector
00A-00B : entrypoint5  vector
00C-00D : entrypoint6  vector
00E-00F : entrypoint7  vector
010-011 : entrypoint8  vector
012-013 : entrypoint9  vector
014-015 : entrypoint10 vector
016-017 : entrypoint11 vector
018-019 : entrypoint12 vector
01A-01B : entrypoint13 vector
01C-01D : entrypoint14 vector
01E-01F : entrypoint15 vector
020-0FF : bank 0, program / data memory
100-1FF : bank 1, program / data memory
200-2FF : bank 2, program / data memory
300-3FF : bank 3, program / data memory

The even address is the lower byte of the entry address.
The odd-address of entry point is a MB and status.
  Bit 0 and 1 are a memory bank.
  Bit 2 is HALT.At the time of set, it doesn't execute entry address.
  After EXIT operation, Bit2 is set.


8201 CONFIRMED OPCODES:
----------------------
opcode       mnemonic     function      flags
--------     ------------ ------------- -----
00000000     NOP          -             --
00000001     RORA         ror A         -C
00000010     ROLA         rol A         -C
00000011     INC RXB      RXB+=2        ZC
00000100     DEC RXB      RXB-=2        ZC (C=1 means No Borrow: RXB>=2)
00000101     INC A        A++           ZC
00000110     DEC A        A--           ZC (C=1 means No Borrow: A>=1)
00000110     CPL A        A^=$FF        --
00001aaa     LD A,(IX0+i) A=[IX0+i]     --
00010aaa     LD A,(IX1+i) A=[IX1+i]     --
00011aaa     LD (IX2+i),A [IX2+i]=A     --
00111aaa     BIT R0.n     ZF=R0 bit n   Z-
0100aaa0     LD A,Rn      A=Rn          Z- [1]
0100aaa1     LD Rn,A      Rn=A          --
0101aaa0     ADD A,Rn     A+=Rn         ZC
0101aaa1     SUB A,Rn     A-=Rn         ZC (C=1 means No Borrow: A>=Rn)
0110aaa0     AND A,Rn     A&=Rn         Z-
0110aaa1     OR A,Rn      A|=Rn         Z-
0111aaaa     ADD IX0,i    IX0+=i        --
1000aaaa     ADD IX1,i    IX1+=i        --
1001aaaa     ADD IX2,i    IX2+=i        --
1010aaaa     LD RB,i      RB=i          -- Note: no bounds checking. Can set bank up to F.
1011-0aa     LD MB,i      set after-jump page
1011-1--     STOP
11000000 imm LD IX0,imm   IX0=imm       --
11000001 imm LD IX1,imm   IX1=imm       --
11000010 imm LD IX2,imm   IX2=imm       --
11000011 imm LD A,imm     A=imm         --
11000100 imm LD LP0,imm   LP0=imm       --
11000101 imm LD LP1,imm   LP1=imm       --
11000110 imm LD LP2,imm   LP2=imm       --
11000111 imm LD RXB,imm   RXB=imm       --
11001000 imm ADD A,imm    A+=imm        ZC
11001001 imm SUB A,imm    A-=imm        ZC (C=1 means No Borrow: A>=imm)
11001010 imm AND A,imm    A&=imm        Z-
11001011 imm OR  A,imm    A|=imm        Z-
11001100 imm DJNZ LP0,imm LP0--,branch  --
11001101 imm DJNZ LP1,imm LP1--,branch  --
11001110 imm DJNZ LP2,imm LP2--,branch  --
11001111 imm JNZ imm      branch if !Z  --
1101--00 imm JNC imm      branch if !C  --
1101--01 imm JZ  imm      branch if Z   --
1101--1- imm J   imm      branch        --
1110--xx mirror for the above
1111--xx mirror for the above

Notes:
[1] bug: the Z flag is not updated correctly after a LD A,Rn instruction. Fixed in 8302 (possibly 8301).


8302 CONFIRMED OPCODES:
----------------------
all of the 8201 ones, with stricter decoding for the following:

11010-00 imm JNC imm      branch if !C  --
11010-01 imm JZ  imm      branch if Z   --
11010-1- imm J   imm      branch        --

and these new opcodes:

opcode       mnemonic     function         flags
--------     ------------ ---------------  -----
11011000 imm LD A,(imm)   A=MB:[imm]       --
11011001 imm LD (imm),A   MB:[imm]=A       --
11011010 imm CMP A,imm    temp=A-imm       ZC
11011011 imm XOR A,imm    A^=imm           Z0
11011100 imm LD A,R(imm)  A=reg(imm)       --
11011101 imm LD R(imm),A  reg(imm)=A       --
11011110 imm JC imm       branch if C      --
11011111 imm CALL $xx     save PC, branch  --

11100000     EXG A,IX0    A<->IX0          --
11100001     EXG A,IX1    A<->IX1          --
11100010     EXG A,IX2    A<->IX2          --
11100011     EXG A,LP1    A<->LP1          --
11100100     EXG A,LP2    A<->LP2          --
11100101     EXG A,RXB    A<->RXB          --
11100110     EXG A,LP0    A<->LP0          --
11100111     EXG A,RB     A<->RB           --
11101000     LD IX0,A     IX0=A            --
11101001     LD IX1,A     IX1=A            --
11101010     LD IX2,A     IX2=A            --
11101011     LD LP1,A     LP1=A            --
11101100     LD LP2,A     LP2=A            --
11101101     LD RXB,A     RXB=A            --
11101110     LD LP0,A     LP0=A            --
11101111     LD RB,A      RB=A             --
11110000     EXG IX0,IX1  IX0<->IX1        --
11110001     EXG IX0,IX2  IX0<->IX2        --
11110010     REP LD (IX2),(RXB)  equivalent to LD (IX2),(RXB); INC RXB; DJNZ LP0
11110011     REP LD (RXB),(IX0)  equivalent to LD (RXB),(IX0); INC RXB; DJNZ LP0
11110100     SAVE ZC      save ZC          --
11110101     REST ZC      restore ZC       ZC
11110110     LD (RXB),A   reg(RXB)=A       --
11110111     LD A,(RXB)   A=reg(RXB)       --
11111000     CMP A,(RXB)  temp=A-reg(RXB)  ZC
11111001     XOR A,(RXB)  A^=reg(RXB)      Z0
11111010     ADD A,CF     if (C) A++       ZC
11111011     SUB A,!CF    if (!C) A--      ZC
11111100     TST A        A==0?            Z-
11111101     CLR A        A=0              --
11111110     LD A,(IX0+A) A=[IX0+A]        --
11111111     RET          restore PC       --


8303 CONFIRMED OPCODES:
----------------------
all of the 8302 ones, with stricter decoding for the following:

11010000 imm JNC imm      branch if !C  --
11010001 imm JZ  imm      branch if Z   --
1101001- imm J   imm      branch        --

additionally, this opcode is modified to support 11-bit instead of 10-bit
external addressing, this wasn't used in games however.

1011-0aa     LD MB,i      modified so that bit 3 is shifted to bit 2 before loading MB.

and these new opcodes are added:

110101--
11010100 imm LD A,(R77:$%02X)
11010101 imm LD (R77:$%02X),A
11010110 imm LD PC,(R77:$%02X)  [1]
11010111 imm LD (R77:$%02X),PC  [2]

Notes:
[1] appears to be LD PC,x in the disassembly, however it's LD LP0,x for kouyakyu
    which uses a 8304, so the opcode was probably changed again.
[2] appears to be LD x,PC in the disassembly, however it's LD x,LP0 for hvoltage
    which uses a 8304 (or 8404?), so the opcode was probably changed again.


***************************************************************************/

#include "cpu/hmcs40/hmcs40.h"
#include "alpha8201.h"

/**************************************************************************/

const device_type ALPHA_8201 = &device_creator<alpha_8201_device>;

//-------------------------------------------------
//  alpha_8201_device - constructor
//-------------------------------------------------

alpha_8201_device::alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ALPHA_8201, "ALPHA-8201", tag, owner, clock, "alpha8201", __FILE__),
	m_mcu(*this, "mcu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alpha_8201_device::device_start()
{
	m_shared_ram = auto_alloc_array_clear(machine(), UINT8, 0x400);
	
	// zerofill
	m_bus = 0;
	m_mcu_address = 0;
	m_mcu_d = 0;
	memset(m_mcu_r, 0, sizeof(m_mcu_r));

	// register for savestates
	save_pointer(NAME(m_shared_ram), 0x400);
	save_item(NAME(m_bus));
	save_item(NAME(m_mcu_address));
	save_item(NAME(m_mcu_d));
	save_item(NAME(m_mcu_r));
}

// machine config additions
static MACHINE_CONFIG_FRAGMENT(alpha8201)

	MCFG_CPU_ADD("mcu", HD44801, DERIVED_CLOCK(1,1)) // 8H
	MCFG_HMCS40_READ_R_CB(0, READ8(alpha_8201_device, mcu_data_r))
	MCFG_HMCS40_READ_R_CB(1, READ8(alpha_8201_device, mcu_data_r))
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(alpha_8201_device, mcu_d_w))
MACHINE_CONFIG_END

machine_config_constructor alpha_8201_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(alpha8201);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alpha_8201_device::device_reset()
{
	m_bus = 0;
	m_mcu->set_input_line(0, CLEAR_LINE);
}



/***************************************************************************

  Internal I/O

***************************************************************************/

void alpha_8201_device::mcu_writeram()
{
	// RAM WR is level-triggered
	if (m_bus && (m_mcu_d & 0xc) == 0xc)
		m_shared_ram[m_mcu_address] = m_mcu_r[0] << 4 | m_mcu_r[1];
}

void alpha_8201_device::mcu_update_address()
{
	m_mcu_address = (m_mcu_d << 8 & 0x300) | m_mcu_r[2] << 4 | m_mcu_r[3];
	mcu_writeram();
}


READ8_MEMBER(alpha_8201_device::mcu_data_r)
{
	UINT8 ret = 0;
	
	if (m_bus && ~m_mcu_d & 4)
		ret = m_shared_ram[m_mcu_address];
	else
		logerror("%s: MCU side invalid read\n", tag());
	
	if (offset == HMCS40_PORT_R0X)
		ret >>= 4;
	return ret & 0xf;
}

WRITE8_MEMBER(alpha_8201_device::mcu_data_w)
{
	// R0,R1: RAM data
	// R2,R3: RAM A0-A7
	m_mcu_r[offset] = data & 0xf;
	mcu_update_address();
}

WRITE16_MEMBER(alpha_8201_device::mcu_d_w)
{
	// D0,D1: RAM A8,A9
	// D2: _RD
	// D3: WR
	m_mcu_d = data;
	mcu_update_address();
}



/***************************************************************************

  I/O for External Interface

***************************************************************************/

WRITE_LINE_MEMBER(alpha_8201_device::bus_dir_w)
{
	// set RAM bus direction to 0: external, 1: MCU side
	// selects one of two 74LS245 (octal bus transceiver) for databus, addressbus via
	// a couple of 74LS157 (2-input multiplexer)
	m_bus = (state) ? 1 : 0;
	mcu_writeram();
}

WRITE_LINE_MEMBER(alpha_8201_device::mcu_start_w)
{
	// connected to MCU INT0
	m_mcu->set_input_line(0, (state) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(alpha_8201_device::ext_ram_r)
{
	// going by exctsccr, m_bus has no effect here
	return m_shared_ram[offset & 0x3ff];
}

WRITE8_MEMBER(alpha_8201_device::ext_ram_w)
{
	// going by exctsccr, m_bus has no effect here
	m_shared_ram[offset & 0x3ff] = data;
}
