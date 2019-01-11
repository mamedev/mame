// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC

 *****************************************************************************/

#include "emu.h"
#include "axc51-core_dasm.h"

const axc51core_disassembler::mem_info axc51core_disassembler::axc51core_names[] = {

	// SFR Registers
	//0x80  -
	{ 0x81, "SP" }, // Stack Pointer
	//0x82  -
	//0x83  -
	//0x84  -
	//0x85  -
	{ 0x86, "DPCON" }, // Data Pointer Configure Register
	{ 0x87, "PCON0" }, // Power Control 0
	//0x88  -
	//0x89  -
	//0x8A  -
	//0x8B  -
	//0x8C  -
	//0x8D  -
	//0x8E  -
	//0x8F  -
	//0x90  -
	//0x91  -
	//0x92  -
	//0x93  -
	//0x94  -
	{ 0x95, "IE2" }, // Interrupt Enable 2
	{ 0x96, "UARTBAUDH" }, // UART Baud (high)
	{ 0x97, "PWKEN" }, // Port Wakeup Enable
	{ 0x98, "PWKPND" }, //Port Wakeup Flag
	{ 0x99, "PWKEDGE" }, // Port Wakeup Edge
	{ 0x9A, "PIE0" }, // Port Digital Input Enable Control 0
	{ 0x9B, "DBASE" }, // DRAM Base Address Register
	{ 0x9C, "PCON1" }, // Power Control 1
	{ 0x9D, "PIE1" }, // Port Digital Input Enable Control 1
	{ 0x9E, "IRTDATA" }, // IRTCC Communication Data
	{ 0x9F, "IRTCON" }, // IRTCC Control
	//0xA0  -
	{ 0xA1, "GP0" }, // (General Purpose Register 0)
	{ 0xA2, "GP1" }, // (General Purpose Register 1)
	{ 0xA3, "GP2" }, // (General Purpose Register 2)
	{ 0xA4, "GP3" }, // (General Purpose Register 3)
	{ 0xA5, "DACCON" }, // DAC Control Register
	{ 0xA6, "DACLCH" }, // DAC Left Channel
	{ 0xA7, "DACRCH" }, // DAC Right Channel
	{ 0xA8, "IE0" }, // Interrupt Enable 0
	{ 0xA9, "IE1" }, // Interrupt Enable 1
	//0xAA  -
	//0xAB  -
	{ 0xAC, "TMR3CON" }, // Timer3 Control
	{ 0xAD, "TMR3CNT" }, // Timer3 Counter
	{ 0xAE, "TMR3PR" }, // Timer3 Period
	{ 0xAF, "TMR3PSR" }, // Timer3 Pre-scalar
	//0xB0  -
	{ 0xB1, "GP4" }, // (General Purpose Register 4)
	{ 0xB2, "GP5" }, // (General Purpose Register 5)
	{ 0xB3, "GP6" }, // (General Purpose Register 6)
	// 0xB4  -
	{ 0xB5, "GP7" }, // (General Purpose Register 7)
	{ 0xB6, "LCDCON" }, // LCD Control Register (or C6?)
	{ 0xB7, "PLLCON" }, // PLL Configuration
	{ 0xB8, "IP0" }, // Interrupt Priority 0
	{ 0xB9, "IP1" }, // Interrupt Priority 1
	//0xBA  -
	//0xBB  -
	//0xBC  -
	//0xBD  -
	//0xBE  -
	{ 0xBF, "LVDCON" }, // LVD Control Register
	//0xC0  -
	{ 0xC1, "TMR2CON" }, // Timer2 Control
	//0xC2  -
	//0xC3  -
	//0xC4  -
	//0xC5  -
	//0xC6  -
	{ 0xC7, "LCDPR" }, // LCD CS Pulse Width Register
	{ 0xC8, "LCDTCON" }, // LCD WR Pulse Timing Control Register
	//0xC9  -
	//0xCA  -
	//0xCB  -
	//0xCC  -
	//0xCD  -
	//0xCE  -
	{ 0xCF, "MICCON" }, // MIC Control
	{ 0xD0, "PSW" }, // Processor Status Word
	{ 0xD1, "PGCON" }, // Power Gate Control Register
	{ 0xD2, "ADCCON" }, // SARADC Control
	{ 0xD3, "PCON2" }, // Power Control 2
	{ 0xD4, "ADCDATAL" }, // SARADC Buffer Low Byte Control
	{ 0xD5, "ADCDATAH" }, // SARADC Buffer High Byte Control
	{ 0xD6, "SPIDMAADDR" }, // SPI DMA Start Address
	{ 0xD7, "SPIDMACNT" }, // SPI DMA counter
	{ 0xD8, "SPICON" }, // SPI Control
	{ 0xD9, "SPIBUF" }, // SPI Data Buffer
	{ 0xDA, "SPIBAUD" }, // SPI Baud Rate
	{ 0xDB, "CLKCON" }, // Clock Control
	// 0xDC  -
	// 0xDD  -
	// 0xDE  -
	// 0xDF  -
	// 0xE0  -
	{ 0xE1, "TMR1CON" }, // Timer1 Control
	// 0xE2  -
	// 0xE3  -
	// 0xE4  -
	// 0xE5  -
	{ 0xE6, "ER00" }, // ER00 \- ER0 (16-bit)  Extended Registers (used by 16-bit opcodes)
	{ 0xE7, "ER01" }, // ER01 /
	{ 0xE8, "ER10" }, // ER10 \- ER1 (16-bit)
	{ 0xE9, "ER11" }, // ER11 /
	{ 0xEA, "ER20" }, // ER20 \- ER2 (16-bit)
	{ 0xEB, "ER21" }, // ER21 /
	{ 0xEC, "ER30" }, // ER30 \- ER3 (16-bit)
	{ 0xED, "ER31" }, // ER31 /
	{ 0xEE, "ER8" }, // ER8
	//0xEF  -
	//0xF0  -
	//0xF1  -
	//0xF2  -
	//0xF3  -
	//0xF4  -
	//0xF5  -
	//0xF6  -
	{ 0xF7, "WDTCON" }, // Watchdog Control
	{ 0xF8, "TMR0CON" }, // Timer0 Control
	{ 0xF9, "TMR0CNT" }, // Timer0 Counter
	{ 0xFA, "TMR0PR" }, // Timer0 Period
	{ 0xFB, "TMR0PSR" }, // Timer0 Pre-scalar
	{ 0xFC, "UARTSTA" }, // UART Status
	{ 0xFD, "UARTCON" }, // UART Control
	{ 0xFE, "UARTBAUD" }, // UART Baud (low)
	{ 0xFF, "UARTDATA" }, // UART Communication Data

	// Upper Registers

	{ 0x3020, "TMR1CNTL" }, // Timer 1 Counter (low)
	{ 0x3021, "TMR1CNTH" }, // Timer 1 Counter (high)
	{ 0x3022, "TMR1PRL" }, // Timer 1 Period (low)
	{ 0x3023, "TMR1PRH" }, // Timer 1 Period (high)
	{ 0x3024, "TMR1PWML" }, // Timer 1 Duty (low)
	{ 0x3025, "TMR1PWMH" }, // Timer 1 Duty (high)

	{ 0x3030, "TMR2CNTL" }, // Timer 2 Counter (low)
	{ 0x3031, "TMR2CNTH" }, // Timer 2 Counter (high)
	{ 0x3032, "TMR2PRL" }, // Timer 2 Period (low)
	{ 0x3033, "TMR2PRH" }, // Timer 2 Period (high)
	{ 0x3034, "TMR2PWML" }, // Timer 2 Duty (low)
	{ 0x3035, "TMR2PWMH" }, // Timer 2 Duty (high)

	{ 0x3040, "ADCBAUD" }, //S ARADC Baud

	{ 0x3070, "DACPTR" }, // DAC DMA Pointer
	{ 0x3071, "DACCNT" }, // DAC DMA Counter

	{ -1 }
};

axc51core_disassembler::axc51core_disassembler() : mcs51_disassembler(axc51core_names)
{
}
/* Extended 16-bit Opcodes

Opcode/params        |    Operation                                        | Flags touched
----------------------------------------------------------------------------------------
INCDPi               |                                                     |
                     |    DPTRi = DPTRi + 1                                |
----------------------------------------------------------------------------------------
DECDPi               |                                                     |
                     |    DPTRi = DPTRi - 1                                |
----------------------------------------------------------------------------------------
ADDDPi               |                                                     |
                     |    DPTRi = DPTRi + {R8, B}                          |
----------------------------------------------------------------------------------------
SUBDPi               |                                                     |
                     |    DPTRi = DPTRi - {R8, B}                          |
----------------------------------------------------------------------------------------
INC2DPi              |                                                     |
                     |    DPTRi = DPTRi + 2                                |
----------------------------------------------------------------------------------------
DEC2DPi              |                                                     |
                     |    DPTRi = DPTRi - 2                                |
----------------------------------------------------------------------------------------
ROTR8                |                                                     |
EACC, ER8            |    Rotate Right ACC by R8 &0x3 bits                 |
----------------------------------------------------------------------------------------
ROTL8                |                                                     |
EACC, ER8            |    Rotate Left ACC by R8 &0x3 bits                  |
----------------------------------------------------------------------------------------
ADD16                |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM + ERn + EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn + ERp + EC                            |
ERp, ERn, ERm        |    ERp = ERn + ERm + EC                             |
----------------------------------------------------------------------------------------
SUB16                |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM - ERn - EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn - ERp - EC                            |
ERp, ERn, ERm        |    ERp = ERn - ERm - EC                             |
----------------------------------------------------------------------------------------
NOT16                |                                                     |
ERn                  |    ERn = ~ERn                                       |
----------------------------------------------------------------------------------------
CLR16                |                                                     |
ERn                  |    ERn = 0                                          |
----------------------------------------------------------------------------------------
INC16                |                                                     |
ERn                  |    ERn = ERn + 1                                    |    EZ
----------------------------------------------------------------------------------------
DEC16                |                                                     |
ERn                  |    ERn = ERn - 1                                    |    EZ
----------------------------------------------------------------------------------------
ANL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM & ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM & ERn                                |
ERn, ERm             |    ERn = ERn & ERm                                  |
----------------------------------------------------------------------------------------
ORL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM | ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM | ERn                                |
ERn, ERm             |    ERn = ERn | ERm                                  |
----------------------------------------------------------------------------------------
XRL16                |                                                     |
ERn, DPTRi           |    ERn = XRAM ^ ERn                                 |    EZ
DPTRi, ERn           |    XRAM = XRAM ^ ERn                                |
ERn, ERm             |    ERn = ERn ^ ERm                                  |
----------------------------------------------------------------------------------------
MOV16                |                                                     |
ERn, DPTRi           |    ERn = XRAM                                       |    EZ
DPTRi, ERn           |    XRAM = ERn                                       |
ERn, ERm             |    ERn = ERm                                        |
----------------------------------------------------------------------------------------
MUL16 (signed)       |                                                     |
ERn, ERm             |    {ERn, ERm} = ERn * ERm                           |
----------------------------------------------------------------------------------------
MULS16 (sign, satur) |                                                     |
ERn, ERm             |    {ERn, ERm} = ERn * ERm                           |
----------------------------------------------------------------------------------------
ROTR16               |                                                     |
ERn, ER8             |    Rotate Right ERn by ER8 & 0xf bits               |
----------------------------------------------------------------------------------------
ROTL16               |                                                     |
ERn, ER8             |    Rotate Left ERn by ER8 & 0xf bits                |
----------------------------------------------------------------------------------------
SHIFTL   (lsl)       |                                                     |
ERn, ER8             |    ERn = ERn >> (ER8 & 0xf)                         |
----------------------------------------------------------------------------------------
SHIFTR   (asr)       |                                                     |
ERn, ER8             |    ERn = ERn >> (ER8 & 0xf)                         |
----------------------------------------------------------------------------------------
ADD16 (saturate)     |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM + ERn + EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn + ERp + EC                            |
ERp, ERn, ERm        |    ERp = ERn + ERm + EC                             |
----------------------------------------------------------------------------------------
SUB16 (saturate)     |                                                     |
ERp, DPTRi, ERn      |    ERp = XRAM - ERn - EC                            |    EZ, EC
DPTRi, ERn, ERp      |    XRAM = ERn - ERp - EC                            |
ERp, ERn, ERm        |    ERp = ERn - ERm - EC                             |
----------------------------------------------------------------------------------------
SWAP16               |                                                     |
ERn                  |    Swap upper and lower 8-bits of ERn               |
----------------------------------------------------------------------------------------

access to 16-bit registers is mapped in SFR space, from 0xE6 (note, changing SFR bank does NOT update the actual registers)

ERn - 16-bit register ER0-ER3 (as data?)
ERm - 16-bit register ER0-ER3 (as data?)
ERp - 16-bit register ER0-ER3 (as pointer?)
EACC = 8-bit accumulator (same as regular opcodes?)
EZ = Zero flag (same as regular opcodes?)
EC = Carry flag  (same as regular opcodes?)

*/

offs_t axc51core_disassembler::disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op)
{
	uint32_t flags = 0;
	uint8_t prm = 0;

	switch( op )
	{
		// unknown, probably the 16-bit extended ocpodes, see page 14 of AX208-SP-101-EN manual, or table above, encodings not specified!
		// note, the other AXC51-CORE based manuals don't seem to mention these, are they really AX208 specific?
		case 0xa5:
			prm = params.r8(PC++);
			util::stream_format(stream, "unknown ax208 a5 $%02X", prm);
			break;

		default:
			return mcs51_disassembler::disassemble_op(stream, PC, pc, opcodes, params, op);
	}

	return (PC - pc) | flags | SUPPORTED;
}


