// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC

 *****************************************************************************/

#include "emu.h"
#include "axc51-core_dasm.h"

// SOME of these might be AX208 specific, we do not currently hvae enough information to split it into AXC51 / AX208 however
const axc51core_disassembler::mem_info axc51core_disassembler::axc51core_names[] = {

	// SFR Registers
	{ 0x80, "P0" },
	{ 0x81, "SP" }, // Stack Pointer
	{ 0x82, "DPL0" },
	{ 0x83, "DPH0" },
	{ 0x84, "DPL1" },
	{ 0x85, "DPH1" },
	{ 0x86, "DPCON" }, // Data Pointer Configure Register
	{ 0x87, "PCON0" }, // Power Control 0
	{ 0x88, "SDCON0" },
	{ 0x89, "SDCON1" },
	{ 0x8A, "SDCON2" },
	{ 0x8B, "JPGCON4" },
	{ 0x8C, "JPGCON3" },
	{ 0x8D, "JPGCON2" },
	{ 0x8E, "JPGCON1" },
	{ 0x8F, "TRAP" },
	{ 0x90, "P1" },
	{ 0x91, "SDBAUD" },
	{ 0x92, "SDCPTR" },
	{ 0x93, "SDDCNT" },
	{ 0x94, "SDDPTR" },
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
	{ 0xA0, "P2" },
	{ 0xA1, "GP0" }, // (General Purpose Register 0)
	{ 0xA2, "GP1" }, // (General Purpose Register 1)
	{ 0xA3, "GP2" }, // (General Purpose Register 2)
	{ 0xA4, "GP3" }, // (General Purpose Register 3)
	{ 0xA5, "DACCON" }, // DAC Control Register
	{ 0xA6, "DACLCH" }, // DAC Left Channel
	{ 0xA7, "DACRCH" }, // DAC Right Channel
	{ 0xA8, "IE0" }, // Interrupt Enable 0
	{ 0xA9, "IE1" }, // Interrupt Enable 1
	{ 0xAA, "KEY0" },
	{ 0xAB, "KEY1" },
	{ 0xAC, "TMR3CON" }, // Timer3 Control
	{ 0xAD, "TMR3CNT" }, // Timer3 Counter
	{ 0xAE, "TMR3PR" }, // Timer3 Period
	{ 0xAF, "TMR3PSR" }, // Timer3 Pre-scalar
	{ 0xB0, "P3" },
	{ 0xB1, "GP4" }, // (General Purpose Register 4)
	{ 0xB2, "GP5" }, // (General Purpose Register 5)
	{ 0xB3, "GP6" }, // (General Purpose Register 6)
	{ 0xB4, "P4" },
	{ 0xB5, "GP7" }, // (General Purpose Register 7)
	{ 0xB6, "LCDCON" }, // LCD Control Register (or C6?)
	{ 0xB7, "PLLCON" }, // PLL Configuration
	{ 0xB8, "IP0" }, // Interrupt Priority 0
	{ 0xB9, "IP1" }, // Interrupt Priority 1
	{ 0xBA, "P0DIR" },
	{ 0xBB, "P1DIR" },
	{ 0xBC, "P2DIR" },
	{ 0xBD, "P3DIR" },
	{ 0xBE, "P4DIR" },
	{ 0xBF, "LVDCON" }, // LVD Control Register
	{ 0xC0, "JPGCON0" },
	{ 0xC1, "TMR2CON" }, // Timer2 Control
	{ 0xC2, "JPGCON9" },
	{ 0xC3, "JPGCON5" },
	{ 0xC4, "JPGCON6" },
	{ 0xC5, "JPGCON7" },
	{ 0xC6, "JPGCON8" },
	{ 0xC7, "LCDPR" }, // LCD CS Pulse Width Register
	{ 0xC8, "LCDTCON" }, // LCD WR Pulse Timing Control Register
	{ 0xC9, "USBCON0" },
	{ 0xCA, "USBCON1" },
	{ 0xCB, "USBCON2" },
	{ 0xCC, "USBDATA" },
	{ 0xCD, "USBADR" },
	{ 0xCE, "illegal" },
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
	{ 0xDC, "CLKCON1" },
	{ 0xDD, "USBDPDM" },
	{ 0xDE, "LFSRPOLY0" },
	{ 0xDF, "LFSRPOLY1" },
	{ 0xE0, "ACC" },
	{ 0xE1, "TMR1CON" }, // Timer1 Control
	{ 0xE2, "UID0" },
	{ 0xE3, "UID1" },
	{ 0xE4, "UID2" },
	{ 0xE5, "UID3" },
	{ 0xE6, "ER00" }, // ER00 \- ER0 (16-bit)  Extended Registers (used by 16-bit opcodes)
	{ 0xE7, "ER01" }, // ER01 /
	{ 0xE8, "ER10" }, // ER10 \- ER1 (16-bit)
	{ 0xE9, "ER11" }, // ER11 /
	{ 0xEA, "ER20" }, // ER20 \- ER2 (16-bit)
	{ 0xEB, "ER21" }, // ER21 /
	{ 0xEC, "ER30" }, // ER30 \- ER3 (16-bit)
	{ 0xED, "ER31" }, // ER31 /
	{ 0xEE, "ER8" }, // ER8
	{ 0xEF, "illegal" },
	{ 0xF0, "B" },
	{ 0xF1, "HUFFBUF" },
	{ 0xF2, "HUFFSFT" },
	{ 0xF3, "HUFFDCL" },
	{ 0xF4, "HUFFDCH" },
	{ 0xF5, "CRC" },
	{ 0xF6, "LFSRFIFO" },
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

	{ 0x3010, "PUP0" },
	{ 0x3011, "PUP1" },
	{ 0x3012, "PUP2" },
	{ 0x3013, "PUP3" },
	{ 0x3014, "PUP4" },
	{ 0x3015, "PDN0" },
	{ 0x3016, "PDN1" },
	{ 0x3017, "PDN2" },
	{ 0x3018, "PDN3" },
	{ 0x3019, "PDN4" },

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

	{ 0x3050, "USBEP0ADL" },
	{ 0x3051, "USBEP0ADH" },
	{ 0x3052, "USBEP1RXADL" },
	{ 0x3053, "USBEP1RXADH" },
	{ 0x3054, "USBEP1TXADL" },
	{ 0x3055, "USBEP1TXADH" },
	{ 0x3056, "USBEP2RXADL" },
	{ 0x3057, "USBEP2RXADH" },
	{ 0x3058, "USBEP2TXADL" },
	{ 0x3059, "USBEP2TXADH" },

	{ 0x3060, "SFSCON" },
	{ 0x3061, "SFSPID" },
	{ 0x3062, "SFSCNTH" },
	{ 0x3063, "SFSCNTL" },

	{ 0x3070, "DACPTR" }, // DAC DMA Pointer
	{ 0x3071, "DACCNT" }, // DAC DMA Counter

	{ -1 }
};


// based on extracted symbol table, note 0x8000 - 0x8ca3 is likely boot code, interrupt code, kernel etc.
// this should be the same for all ax208 CPUs as they are thought to all use the same internal ROM
const ax208_disassembler::ax208_bios_info ax208_disassembler::bios_call_names[] = {
	{ 0x8000, "entry point" },

	{ 0x8006, "unknown, used" },
	{ 0x8009, "unknown, used" },

	{ 0x8ca4, "_STRCHR" },
	{ 0x8dd6, "_STRLEN" },
	{ 0x8eb7, "_tolower" },
	{ 0x8ec8, "_toupper" },
	{ 0x900f, "_isalpha" },
	{ 0x902a, "_iscntrl" },
	{ 0x9038, "_isdigit" },
	{ 0x9047, "_isalnum" },
	{ 0x906d, "_isgraph" },
	{ 0x907c, "_isprint" },
	{ 0x908b, "_ispunct" },
	{ 0x90bb, "_islower" },
	{ 0x90ca, "_isupper" },
	{ 0x90d9, "_isspace" },
	{ 0x90ec, "_isxdigit" },
	{ 0x91e2, "COPY" },
	{ 0x9208, "SCDIV" },
	{ 0x922a, "CLDPTR" },
	{ 0x9243, "CLDIPTR" },
	{ 0x9267, "CLDOPTR" },
	{ 0x9294, "CLDIOPTR" },
	{ 0x92cb, "CILDPTR" },
	{ 0x92ed, "CILDOPTR" },
	{ 0x9320, "CSTPTR" },
	{ 0x9332, "CSTOPTR" },
	{ 0x9354, "UIDIV" },
	{ 0x93a9, "SIDIV" },
	{ 0x93df, "IILDX" },
	{ 0x93f5, "ILDIX" },
	{ 0x940b, "ILDPTR" },
	{ 0x9436, "ILDIPTR" },
	{ 0x946b, "ILDOPTR" },
	{ 0x94a3, "ILDIOPTR" },
	{ 0x94ef, "IILDPTR" },
	{ 0x9527, "IILDOPTR" },
	{ 0x9574, "ISTPTR" },
	{ 0x9593, "ISTOPTR" },
	{ 0x95c0, "LADD" },
	{ 0x95cd, "LSUB" },
	{ 0x95db, "LMUL" },
	{ 0x9666, "ULDIV" },
	{ 0x96f8, "LAND" },
	{ 0x9705, "LOR" },
	{ 0x9712, "LXOR" },
	{ 0x971f, "LNOT" },
	{ 0x972c, "LNEG" },
	{ 0x973a, "SLCMP" },
	{ 0x9750, "ULCMP" },
	{ 0x9761, "ULSHR" },
	{ 0x9774, "SLSHR" },
	{ 0x9788, "LSHL" },
	{ 0x979b, "LLDPTR" },
	{ 0x97bb, "LLDOPTR" },
	{ 0x97eb, "LSTPTR" },
	{ 0x9805, "LSTOPTR" },
	{ 0x9829, "LILDPTR" },
	{ 0x9849, "LILDOPTR" },
	{ 0x9879, "LLDIPTR" },
	{ 0x9899, "LLDIOPTR" },
	{ 0x98c9, "LLDIDATA" },
	{ 0x98d5, "LLDXDATA" },
	{ 0x98e1, "LLDPDATA" },
	{ 0x98ed, "LLDCODE" },
	{ 0x98fd, "LLDIDATA0" },
	{ 0x990a, "LLDXDATA0" },
	{ 0x9916, "LLDPDATA0" },
	{ 0x9923, "LLDCODE0" },
	{ 0x9933, "LLDPTR0" },
	{ 0x9953, "LLDOPTR0" },
	{ 0x9983, "LLDIIDATA1" },
	{ 0x9985, "LLDIIDATA8" },
	{ 0x998c, "LLDIIDATA" },
	{ 0x99a3, "LLDIXDATA1" },
	{ 0x99a5, "LLDIXDATA8" },
	{ 0x99ac, "LLDIXDATA" },
	{ 0x99d8, "LLDIPDATA1" },
	{ 0x99da, "LLDIPDATA8" },
	{ 0x99e1, "LLDIPDATA" },
	{ 0x99f8, "LILDIDATA1" },
	{ 0x99fa, "LILDIDATA8" },
	{ 0x9a01, "LILDIDATA" },
	{ 0x9a18, "LILDXDATA1" },
	{ 0x9a1a, "LILDXDATA8" },
	{ 0x9a21, "LILDXDATA" },
	{ 0x9a4d, "LILDPDATA1" },
	{ 0x9a4f, "LILDPDATA8" },
	{ 0x9a56, "LILDPDATA" },
	{ 0x9a6d, "LSTIDATA" },
	{ 0x9a79, "LSTXDATA" },
	{ 0x9a85, "LSTPDATA" },
	{ 0x9a91, "LSTKIDATA" },
	{ 0x9aaa, "LSTKXDATA" },
	{ 0x9adb, "LSTKPDATA" },
	{ 0x9af4, "LSTKPTR" },
	{ 0x9b0e, "LSTKOPTR" },
	{ 0x9b32, "BCAST_L" },
	{ 0x9b3b, "OFFX256" },
	{ 0x9b4c, "OFFXADD" },
	{ 0x9b58, "OFFXADD1" },
	{ 0x9b61, "PLDIDATA" },
	{ 0x9b6a, "PLDIIDATA" },
	{ 0x9b7a, "PILDIDATA" },
	{ 0x9b8a, "PSTIDATA" },
	{ 0x9b93, "PLDXDATA" },
	{ 0x9b9c, "PLDIXDATA" },
	{ 0x9bb3, "PILDXDATA" },
	{ 0x9bca, "PSTXDATA" },
	{ 0x9bd3, "PLDPDATA" },
	{ 0x9bdc, "PLDIPDATA" },
	{ 0x9bec, "PILDPDATA" },
	{ 0x9bfc, "PSTPDATA" },
	{ 0x9c05, "PLDCODE" },
	{ 0x9c11, "PLDPTR" },
	{ 0x9c31, "PLDIPTR" },
	{ 0x9c53, "PILDPTR" },
	{ 0x9c75, "PSTPTR" },
	{ 0x9cc4, "PSTPTRR" },
	{ 0x9cf4, "PLDOPTR" },
	{ 0x9d24, "PLDIOPTR" },
	{ 0x9d56, "PILDOPTR" },
	{ 0x9d88, "PSTOPTR" },
	{ 0x9de1, "CCASE" },
	{ 0x9e07, "ICASE" },
	{ 0x9e34, "LCASE" },
	{ 0x9e6e, "ICALL" },
	{ 0x9e72, "ICALL2" },
	{ 0x9e74, "MEMSET" },
	{ 0x9ea0, "LROL" },
	{ 0x9eb4, "LROR" },
	{ 0x9ec8, "SLDIV" },
	{ 0x9f47, "SPI_ENCRYPT_ON3" },
	{ 0x9f5c, "_lshift9" },
	{ 0x9fdc, "SPI_ENCRYPT_CLOSE" },
	{ -1, "unknown" }
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

offs_t axc51core_disassembler::disassemble_extended_a5_0e(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADD16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADD16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "ADD16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 0e $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_0f(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUB16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUB16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "SUB16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}


	default:
		util::stream_format(stream, "illegal ax208 a5 0f $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_d0(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADDS16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "ADDS16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "ADDS16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 d0 $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}

offs_t axc51core_disassembler::disassemble_extended_a5_d1(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm2 = params.r8(PC++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUBS16 ER%01x, EDP%01x, ER%01x", p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		util::stream_format(stream, "SUBS16 EDP%01x, ER%01x, ER%01x", i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		util::stream_format(stream, "SUBS16 ER%01x, ER%01x, ER%01x", p, n, m);
		break;
	}

	default:
		util::stream_format(stream, "illegal ax208 a5 d1 $%02X", prm2);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}


offs_t axc51core_disassembler::disassemble_extended_a5(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	uint32_t flags = 0;
	uint8_t prm = params.r8(PC++);

	switch (prm)
	{
	case 0x00:
		util::stream_format(stream, "INCDP0");
		break;

	case 0x01:
		util::stream_format(stream, "INCDP1");
		break;

	case 0x02:
		util::stream_format(stream, "DECDP0");
		break;

	case 0x03:
		util::stream_format(stream, "DECDP1");
		break;

	case 0x04:
		util::stream_format(stream, "ADDDP0");
		break;

	case 0x05:
		util::stream_format(stream, "ADDDP1");
		break;

	case 0x06:
		util::stream_format(stream, "SUBDP0");
		break;

	case 0x07:
		util::stream_format(stream, "SUBDP1");
		break;

	case 0x08:
		util::stream_format(stream, "INC2DP0");
		break;

	case 0x09:
		util::stream_format(stream, "INC2DP1");
		break;

	case 0x0a:
		util::stream_format(stream, "DEC2DP0");
		break;

	case 0x0b:
		util::stream_format(stream, "DEC2DP1");
		break;

	case 0x0c:
		util::stream_format(stream, "ROTR8 EACC, ER8");
		break;

	case 0x0d:
		util::stream_format(stream, "ROTL8 EACC, ER8");
		break;

	case 0x0e: // ADD16
		return disassemble_extended_a5_0e(stream, PC, pc, opcodes, params);

	case 0x0f: // SUB16
		return disassemble_extended_a5_0f(stream, PC, pc, opcodes, params);

	case 0x10: case 0x14: case 0x18: case 0x1c:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "NOT16 ER%01x", n);
		break;
	}

	case 0x11: case 0x15: case 0x19: case 0x1d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "CLR16 ER%01x", n);
		break;
	}

	case 0x12: case 0x16: case 0x1a: case 0x1e:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "INC16 ER%01x", n);
		break;
	}

	case 0x13: case 0x17: case 0x1b: case 0x1f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "DEC16 ER%01x", n);
		break;
	}

	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ANL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ANL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "ANL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x40: case 0x41: case 0x44: case 0x45: case 0x48: case 0x49: case 0x4c: case 0x4d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ORL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x42: case 0x43: case 0x46: case 0x47: case 0x4a: case 0x4b: case 0x4e: case 0x4f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "ORL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "ORL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x60: case 0x61: case 0x64: case 0x65: case 0x68: case 0x69: case 0x6c: case 0x6d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "XRL16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x62: case 0x63: case 0x66: case 0x67: case 0x6a: case 0x6b: case 0x6e: case 0x6f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "XRL16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "XRL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0x80: case 0x81: case 0x84: case 0x85: case 0x88: case 0x89: case 0x8c: case 0x8d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "MOV16 ER%01x, EDP%01x", n, i);
		break;
	}

	case 0x82: case 0x83: case 0x86: case 0x87: case 0x8a: case 0x8b: case 0x8e: case 0x8f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		util::stream_format(stream, "MOV16 EDP%01x, ER%01x", i, n);
		break;
	}

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MOV16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MUL16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		util::stream_format(stream, "MULS16 ER%01x, ER%01x", n, m);
		break;
	}

	case 0xc0: case 0xc4: case 0xc8: case 0xcc:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "ROTR16 ER%01x, ER8", n);
		break;
	}

	case 0xc1: case 0xc5: case 0xc9: case 0xcd:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "ROTL16 ER%01x, ER8", n);
		break;
	}

	case 0xc2: case 0xc6: case 0xca: case 0xce:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SHIFTL ER%01x, ER8", n);
		break;
	}

	case 0xc3: case 0xc7: case 0xcb: case 0xcf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SHIFTA ER%01x, ER8", n);
		break;
	}

	case 0xd0: // ADDS16
		return disassemble_extended_a5_d0(stream, PC, pc, opcodes, params);

	case 0xd1: // SUBS16
		return disassemble_extended_a5_d1(stream, PC, pc, opcodes, params);

	case 0xd2: case 0xd6: case 0xda: case 0xde:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		util::stream_format(stream, "SWAP16 ER%01x", n);
		break;
	}

	case 0xd3: case 0xd4: case 0xd5: case 0xd7: case 0xd8: case 0xd9: case 0xdb: case 0xdc: case 0xdd: case 0xdf:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		util::stream_format(stream, "invalid ax208 a5 $%02X", prm);
		break;

	default:
		util::stream_format(stream, "unknown ax208 a5 $%02X", prm);
		break;
	}

	return (PC - pc) | flags | SUPPORTED;
}


offs_t axc51core_disassembler::disassemble_op(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params, uint8_t op)
{
	uint32_t flags = 0;

	switch (op)
	{
	case 0xa5:
		return disassemble_extended_a5(stream, PC, pc, opcodes, params);

	default:
		return mcs51_disassembler::disassemble_op(stream, PC, pc, opcodes, params, op);
	}

	return (PC - pc) | flags | SUPPORTED;
}


ax208_disassembler::ax208_disassembler() : axc51core_disassembler(axc51core_names)
{
}

void ax208_disassembler::disassemble_op_ljmp(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++) << 8) & 0xff00;
	addr |= params.r8(PC++);
	if ((addr >= 0x8000) && (addr < 0xa000))
	{
		int i = 0;
		int lookaddr = -1;
		const char* lookname;
		do
		{
			lookaddr = bios_call_names[i].addr;
			lookname = bios_call_names[i].name;

			if (lookaddr == addr)
				break;

			i++;
		} while (lookaddr != -1);

		util::stream_format(stream, "ljmp  $%04X (%s)", addr, lookname);
	}
	else
	{
		util::stream_format(stream, "ljmp  $%04X", addr);
	}
}

void ax208_disassembler::disassemble_op_lcall(std::ostream& stream, unsigned &PC, const data_buffer& params)
{
	uint16_t addr = (params.r8(PC++)<<8) & 0xff00;
	addr|= params.r8(PC++);
	if ((addr >= 0x8000) && (addr < 0xa000))
	{
		int i = 0;
		int lookaddr = -1;
		const char* lookname;
		do
		{
			lookaddr = bios_call_names[i].addr;
			lookname = bios_call_names[i].name;

			if (lookaddr == addr)
				break;

			i++;
		} while (lookaddr != -1);

		util::stream_format(stream, "lcall $%04X (%s)", addr, lookname);
	}
	else
	{
		util::stream_format(stream, "lcall $%04X", addr);
	}
}
