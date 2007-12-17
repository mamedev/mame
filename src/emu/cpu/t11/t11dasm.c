/*
 *   A T11 disassembler
 *
 *   Note: this is probably not the most efficient disassembler in the world :-)
 *
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include "debugger.h"
#include "t11.h"

static const char *regs[8] = { "R0", "R1", "R2", "R3", "R4", "R5", "SP", "PC" };

static const UINT8 *rombase;
static offs_t pcbase;

#define PARAM_WORD(v)	((v) = rombase[pc - pcbase] | (rombase[pc + 1 - pcbase] << 8), pc += 2)

unsigned MakeEA (char **ea, int lo, unsigned pc, int width)
{
	char *buffer = cpuintrf_temp_str();
	int reg, pm;

	assert (width == 2 || width == 4);

    reg = lo & 7;

	switch ((lo >> 3) & 7)
	{
		case 0:
			sprintf (buffer, "%s", regs[reg]);
			break;
		case 1:
			sprintf (buffer, "(%s)", regs[reg]);
			break;
		case 2:
			if (reg == 7)
            {
				PARAM_WORD (pm);
				sprintf (buffer, "#$%0*X", width, pm & ((width == 2) ? 0xff : 0xffff));
            }
			else
            {
                sprintf (buffer, "(%s)+", regs[reg]);
            }
            break;
		case 3:
			if (reg == 7)
			{
                PARAM_WORD (pm);
                sprintf (buffer,  "$%04X", pm &= 0xffff);
			}
			else
			{
                sprintf (buffer, "@(%s)+", regs[reg]);
			}
            break;
		case 4:
            sprintf (buffer, "-(%s)", regs[reg]);
			break;
		case 5:
			sprintf (buffer, "@-(%s)", regs[reg]);
			break;
		case 6:
			PARAM_WORD (pm);
			sprintf(buffer, "%s$%X(%s)",
				(pm&0x8000)?"-":"",
				(pm&0x8000)?-(signed short)pm:pm,
				regs[reg]);
			break;
		case 7:
			PARAM_WORD (pm);
			sprintf(buffer, "@%s$%X(%s)",
				(pm&0x8000)?"-":"",
				(pm&0x8000)?-(signed short)pm:pm,
				regs[reg]);
			break;
	}

	*ea = buffer;
    return pc;
}


offs_t t11_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	char *ea1, *ea2;
	unsigned PC = pc;
	UINT16 op, lo, hi, addr;
    INT16 offset;
    UINT32 flags = 0;

	rombase = oprom;
	pcbase = pc;

	PARAM_WORD(op);
    lo = op & 077;
	hi = (op >> 6) & 077;

	switch (op & 0xffc0)
	{
		case 0x0000:
			switch (lo)
			{
				case 0x00:	sprintf (buffer, "HALT"); break;
				case 0x01:	sprintf (buffer, "WAIT"); break;
				case 0x02:	sprintf (buffer, "RTI"); flags = DASMFLAG_STEP_OUT; break;
				case 0x03:	sprintf (buffer, "BPT"); break;
				case 0x04:	sprintf (buffer, "IOT"); break;
				case 0x05:	sprintf (buffer, "RESET"); break;
				case 0x06:	sprintf (buffer, "RTT"); break;
				default:	sprintf (buffer, "???? (%04X)", op); break;
			}
			break;
		case 0x0040:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "JMP   %s", ea1);
			break;
		case 0x0080:
			switch (lo & 070)
			{
				case 000:
					if( (lo & 7) == 7 )
						sprintf (buffer, "RTS");
					else
						sprintf (buffer, "RTS   %s", regs[lo & 7]);
					flags = DASMFLAG_STEP_OUT;
					break;
				case 040:
				case 050:
					switch( lo & 15 )
					{
						case 0x00:	sprintf (buffer, "NOP"); break;
						case 0x0f:	sprintf (buffer, "CCC"); break;
						case 0x01:	sprintf (buffer, "CEC"); break;
						case 0x02:	sprintf (buffer, "CEV"); break;
						case 0x04:	sprintf (buffer, "CEZ"); break;
						case 0x08:	sprintf (buffer, "CEN"); break;
						default:	sprintf (buffer, "Ccc   #$%X", lo & 15); break;
					}
					break;
				case 060:
				case 070:
					switch( lo & 15 )
					{
						case 0x00:	sprintf (buffer, "NOP"); break;
						case 0x0f:	sprintf (buffer, "SCC"); break;
						case 0x01:	sprintf (buffer, "SEC"); break;
						case 0x02:	sprintf (buffer, "SEV"); break;
						case 0x04:	sprintf (buffer, "SEZ"); break;
						case 0x08:	sprintf (buffer, "SEN"); break;
						default:	sprintf (buffer, "Scc   #$%X", lo & 15); break;
					}
					break;
			}
			break;
		case 0x00c0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "SWAB  %s", ea1);
			break;
		case 0x0100: case 0x0140: case 0x0180: case 0x01c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BR    $%04X", pc + offset);
			break;
		case 0x0200: case 0x0240: case 0x0280: case 0x02c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BNE   $%04X", pc + offset);
			break;
		case 0x0300: case 0x0340: case 0x0380: case 0x03c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BEQ   $%04X", pc + offset);
			break;
		case 0x0400: case 0x0440: case 0x0480: case 0x04c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BGE   $%04X", pc + offset);
			break;
		case 0x0500: case 0x0540: case 0x0580: case 0x05c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BLT   $%04X", pc + offset);
			break;
		case 0x0600: case 0x0640: case 0x0680: case 0x06c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BGT   $%04X", pc + offset);
			break;
		case 0x0700: case 0x0740: case 0x0780: case 0x07c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BLE   $%04X", pc + offset);
			break;
		case 0x0800: case 0x0840: case 0x0880: case 0x08c0:
		case 0x0900: case 0x0940: case 0x0980: case 0x09c0:
			pc = MakeEA (&ea1, lo, pc, 4);
			if ( (hi & 7) == 7 )
				sprintf (buffer, "JSR   %s", ea1);
			else
				sprintf (buffer, "JSR   %s,%s", regs[hi & 7], ea1);
			flags = DASMFLAG_STEP_OVER;
			break;
		case 0x0a00:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "CLR   %s", ea1);
			break;
		case 0x0a40:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "COM   %s", ea1);
			break;
		case 0x0a80:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "INC   %s", ea1);
			break;
		case 0x0ac0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "DEC   %s", ea1);
			break;
		case 0x0b00:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "NEG   %s", ea1);
			break;
		case 0x0b40:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "ADC   %s", ea1);
			break;
		case 0x0b80:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "SBC   %s", ea1);
			break;
		case 0x0bc0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "TST   %s", ea1);
			break;
		case 0x0c00:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "ROR   %s", ea1);
			break;
		case 0x0c40:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "ROL   %s", ea1);
			break;
		case 0x0c80:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "ASR   %s", ea1);
			break;
		case 0x0cc0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "ASL   %s", ea1);
			break;
/*      case 0x0d00:
            sprintf (buffer, "MARK  #$%X", lo);
            break;*/
		case 0x0dc0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "SXT   %s", ea1);
			break;
		case 0x1000: case 0x1040: case 0x1080: case 0x10c0: case 0x1100: case 0x1140: case 0x1180: case 0x11c0:
		case 0x1200: case 0x1240: case 0x1280: case 0x12c0: case 0x1300: case 0x1340: case 0x1380: case 0x13c0:
		case 0x1400: case 0x1440: case 0x1480: case 0x14c0: case 0x1500: case 0x1540: case 0x1580: case 0x15c0:
		case 0x1600: case 0x1640: case 0x1680: case 0x16c0: case 0x1700: case 0x1740: case 0x1780: case 0x17c0:
		case 0x1800: case 0x1840: case 0x1880: case 0x18c0: case 0x1900: case 0x1940: case 0x1980: case 0x19c0:
		case 0x1a00: case 0x1a40: case 0x1a80: case 0x1ac0: case 0x1b00: case 0x1b40: case 0x1b80: case 0x1bc0:
		case 0x1c00: case 0x1c40: case 0x1c80: case 0x1cc0: case 0x1d00: case 0x1d40: case 0x1d80: case 0x1dc0:
		case 0x1e00: case 0x1e40: case 0x1e80: case 0x1ec0: case 0x1f00: case 0x1f40: case 0x1f80: case 0x1fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			if (lo == 046)		/* MOV src,-(SP) */
				sprintf (buffer, "PUSH  %s", ea1);
			else
			if (hi == 026)		/* MOV (SP)+,buffer */
				sprintf (buffer, "POP   %s", ea2);
			else				/* all other */
				sprintf (buffer, "MOV   %s,%s", ea1, ea2);
			break;
		case 0x2000: case 0x2040: case 0x2080: case 0x20c0: case 0x2100: case 0x2140: case 0x2180: case 0x21c0:
		case 0x2200: case 0x2240: case 0x2280: case 0x22c0: case 0x2300: case 0x2340: case 0x2380: case 0x23c0:
		case 0x2400: case 0x2440: case 0x2480: case 0x24c0: case 0x2500: case 0x2540: case 0x2580: case 0x25c0:
		case 0x2600: case 0x2640: case 0x2680: case 0x26c0: case 0x2700: case 0x2740: case 0x2780: case 0x27c0:
		case 0x2800: case 0x2840: case 0x2880: case 0x28c0: case 0x2900: case 0x2940: case 0x2980: case 0x29c0:
		case 0x2a00: case 0x2a40: case 0x2a80: case 0x2ac0: case 0x2b00: case 0x2b40: case 0x2b80: case 0x2bc0:
		case 0x2c00: case 0x2c40: case 0x2c80: case 0x2cc0: case 0x2d00: case 0x2d40: case 0x2d80: case 0x2dc0:
		case 0x2e00: case 0x2e40: case 0x2e80: case 0x2ec0: case 0x2f00: case 0x2f40: case 0x2f80: case 0x2fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "CMP   %s,%s", ea1, ea2);
			break;
		case 0x3000: case 0x3040: case 0x3080: case 0x30c0: case 0x3100: case 0x3140: case 0x3180: case 0x31c0:
		case 0x3200: case 0x3240: case 0x3280: case 0x32c0: case 0x3300: case 0x3340: case 0x3380: case 0x33c0:
		case 0x3400: case 0x3440: case 0x3480: case 0x34c0: case 0x3500: case 0x3540: case 0x3580: case 0x35c0:
		case 0x3600: case 0x3640: case 0x3680: case 0x36c0: case 0x3700: case 0x3740: case 0x3780: case 0x37c0:
		case 0x3800: case 0x3840: case 0x3880: case 0x38c0: case 0x3900: case 0x3940: case 0x3980: case 0x39c0:
		case 0x3a00: case 0x3a40: case 0x3a80: case 0x3ac0: case 0x3b00: case 0x3b40: case 0x3b80: case 0x3bc0:
		case 0x3c00: case 0x3c40: case 0x3c80: case 0x3cc0: case 0x3d00: case 0x3d40: case 0x3d80: case 0x3dc0:
		case 0x3e00: case 0x3e40: case 0x3e80: case 0x3ec0: case 0x3f00: case 0x3f40: case 0x3f80: case 0x3fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "BIT   %s,%s", ea1, ea2);
			break;
		case 0x4000: case 0x4040: case 0x4080: case 0x40c0: case 0x4100: case 0x4140: case 0x4180: case 0x41c0:
		case 0x4200: case 0x4240: case 0x4280: case 0x42c0: case 0x4300: case 0x4340: case 0x4380: case 0x43c0:
		case 0x4400: case 0x4440: case 0x4480: case 0x44c0: case 0x4500: case 0x4540: case 0x4580: case 0x45c0:
		case 0x4600: case 0x4640: case 0x4680: case 0x46c0: case 0x4700: case 0x4740: case 0x4780: case 0x47c0:
		case 0x4800: case 0x4840: case 0x4880: case 0x48c0: case 0x4900: case 0x4940: case 0x4980: case 0x49c0:
		case 0x4a00: case 0x4a40: case 0x4a80: case 0x4ac0: case 0x4b00: case 0x4b40: case 0x4b80: case 0x4bc0:
		case 0x4c00: case 0x4c40: case 0x4c80: case 0x4cc0: case 0x4d00: case 0x4d40: case 0x4d80: case 0x4dc0:
		case 0x4e00: case 0x4e40: case 0x4e80: case 0x4ec0: case 0x4f00: case 0x4f40: case 0x4f80: case 0x4fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "BIC   %s,%s", ea1, ea2);
			break;
		case 0x5000: case 0x5040: case 0x5080: case 0x50c0: case 0x5100: case 0x5140: case 0x5180: case 0x51c0:
		case 0x5200: case 0x5240: case 0x5280: case 0x52c0: case 0x5300: case 0x5340: case 0x5380: case 0x53c0:
		case 0x5400: case 0x5440: case 0x5480: case 0x54c0: case 0x5500: case 0x5540: case 0x5580: case 0x55c0:
		case 0x5600: case 0x5640: case 0x5680: case 0x56c0: case 0x5700: case 0x5740: case 0x5780: case 0x57c0:
		case 0x5800: case 0x5840: case 0x5880: case 0x58c0: case 0x5900: case 0x5940: case 0x5980: case 0x59c0:
		case 0x5a00: case 0x5a40: case 0x5a80: case 0x5ac0: case 0x5b00: case 0x5b40: case 0x5b80: case 0x5bc0:
		case 0x5c00: case 0x5c40: case 0x5c80: case 0x5cc0: case 0x5d00: case 0x5d40: case 0x5d80: case 0x5dc0:
		case 0x5e00: case 0x5e40: case 0x5e80: case 0x5ec0: case 0x5f00: case 0x5f40: case 0x5f80: case 0x5fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "BIS   %s,%s", ea1, ea2);
			break;
		case 0x6000: case 0x6040: case 0x6080: case 0x60c0: case 0x6100: case 0x6140: case 0x6180: case 0x61c0:
		case 0x6200: case 0x6240: case 0x6280: case 0x62c0: case 0x6300: case 0x6340: case 0x6380: case 0x63c0:
		case 0x6400: case 0x6440: case 0x6480: case 0x64c0: case 0x6500: case 0x6540: case 0x6580: case 0x65c0:
		case 0x6600: case 0x6640: case 0x6680: case 0x66c0: case 0x6700: case 0x6740: case 0x6780: case 0x67c0:
		case 0x6800: case 0x6840: case 0x6880: case 0x68c0: case 0x6900: case 0x6940: case 0x6980: case 0x69c0:
		case 0x6a00: case 0x6a40: case 0x6a80: case 0x6ac0: case 0x6b00: case 0x6b40: case 0x6b80: case 0x6bc0:
		case 0x6c00: case 0x6c40: case 0x6c80: case 0x6cc0: case 0x6d00: case 0x6d40: case 0x6d80: case 0x6dc0:
		case 0x6e00: case 0x6e40: case 0x6e80: case 0x6ec0: case 0x6f00: case 0x6f40: case 0x6f80: case 0x6fc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "ADD   %s,%s", ea1, ea2);
			break;

		case 0x7800: case 0x7840: case 0x7880: case 0x78c0: case 0x7900: case 0x7940: case 0x7980: case 0x79c0:
			pc = MakeEA (&ea1, lo, pc, 4);
			sprintf (buffer, "XOR   %s,%s", regs[hi & 7], ea1);
			break;

		case 0x7e00: case 0x7e40: case 0x7e80: case 0x7ec0: case 0x7f00: case 0x7f40: case 0x7f80: case 0x7fc0:
			addr = (pc + 2 - 2 * lo) & 0xffff;
			sprintf (buffer, "SOB   %s,$%X", regs[hi & 7], addr);
			break;

		case 0x8000: case 0x8040: case 0x8080: case 0x80c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BPL   $%04X", pc + offset);
			break;
		case 0x8100: case 0x8140: case 0x8180: case 0x81c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BMI   $%04X", pc + offset);
			break;
		case 0x8200: case 0x8240: case 0x8280: case 0x82c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BHI   $%04X", pc + offset);
			break;
		case 0x8300: case 0x8340: case 0x8380: case 0x83c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BLOS  $%04X", pc + offset);
			break;
		case 0x8400: case 0x8440: case 0x8480: case 0x84c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BVC   $%04X", pc + offset);
			break;
		case 0x8500: case 0x8540: case 0x8580: case 0x85c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BVS   $%04X", pc + offset);
			break;
		case 0x8600: case 0x8640: case 0x8680: case 0x86c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BCC   $%04X", pc + offset);
            break;
		case 0x8700: case 0x8740: case 0x8780: case 0x87c0:
			offset = 2 * (INT8)(op & 0xff);
            sprintf (buffer, "BCS   $%04X", pc + offset);
			break;
		case 0x8800: case 0x8840: case 0x8880: case 0x88c0:
			sprintf (buffer, "EMT   #$%02X", op & 0xff);
			break;
		case 0x8900: case 0x8940: case 0x8980: case 0x89c0:
			sprintf (buffer, "TRAP  #$%02X", op & 0xff);
			break;

		case 0x8a00:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "CLRB  %s", ea1);
			break;
		case 0x8a40:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "COMB  %s", ea1);
			break;
		case 0x8a80:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "INCB  %s", ea1);
			break;
		case 0x8ac0:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "DECB  %s", ea1);
			break;
		case 0x8b00:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "NEGB  %s", ea1);
			break;
		case 0x8b40:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "ADCB  %s", ea1);
			break;
		case 0x8b80:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "SBCB  %s", ea1);
			break;
		case 0x8bc0:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "TSTB  %s", ea1);
			break;
		case 0x8c00:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "RORB  %s", ea1);
			break;
		case 0x8c40:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "ROLB  %s", ea1);
			break;
		case 0x8c80:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "ASRB  %s", ea1);
			break;
		case 0x8cc0:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "ASLB  %s", ea1);
			break;
		case 0x8d00:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "MTPS  %s", ea1);
			break;
		case 0x8dc0:
			pc = MakeEA (&ea1, lo, pc, 2);
			sprintf (buffer, "MFPS  %s", ea1);
			break;
		case 0x9000: case 0x9040: case 0x9080: case 0x90c0: case 0x9100: case 0x9140: case 0x9180: case 0x91c0:
		case 0x9200: case 0x9240: case 0x9280: case 0x92c0: case 0x9300: case 0x9340: case 0x9380: case 0x93c0:
		case 0x9400: case 0x9440: case 0x9480: case 0x94c0: case 0x9500: case 0x9540: case 0x9580: case 0x95c0:
		case 0x9600: case 0x9640: case 0x9680: case 0x96c0: case 0x9700: case 0x9740: case 0x9780: case 0x97c0:
		case 0x9800: case 0x9840: case 0x9880: case 0x98c0: case 0x9900: case 0x9940: case 0x9980: case 0x99c0:
		case 0x9a00: case 0x9a40: case 0x9a80: case 0x9ac0: case 0x9b00: case 0x9b40: case 0x9b80: case 0x9bc0:
		case 0x9c00: case 0x9c40: case 0x9c80: case 0x9cc0: case 0x9d00: case 0x9d40: case 0x9d80: case 0x9dc0:
		case 0x9e00: case 0x9e40: case 0x9e80: case 0x9ec0: case 0x9f00: case 0x9f40: case 0x9f80: case 0x9fc0:
			pc = MakeEA (&ea1, hi, pc, 2);
			pc = MakeEA (&ea2, lo, pc, 2);
			sprintf (buffer, "MOVB  %s,%s", ea1, ea2);
			break;
		case 0xa000: case 0xa040: case 0xa080: case 0xa0c0: case 0xa100: case 0xa140: case 0xa180: case 0xa1c0:
		case 0xa200: case 0xa240: case 0xa280: case 0xa2c0: case 0xa300: case 0xa340: case 0xa380: case 0xa3c0:
		case 0xa400: case 0xa440: case 0xa480: case 0xa4c0: case 0xa500: case 0xa540: case 0xa580: case 0xa5c0:
		case 0xa600: case 0xa640: case 0xa680: case 0xa6c0: case 0xa700: case 0xa740: case 0xa780: case 0xa7c0:
		case 0xa800: case 0xa840: case 0xa880: case 0xa8c0: case 0xa900: case 0xa940: case 0xa980: case 0xa9c0:
		case 0xaa00: case 0xaa40: case 0xaa80: case 0xaac0: case 0xab00: case 0xab40: case 0xab80: case 0xabc0:
		case 0xac00: case 0xac40: case 0xac80: case 0xacc0: case 0xad00: case 0xad40: case 0xad80: case 0xadc0:
		case 0xae00: case 0xae40: case 0xae80: case 0xaec0: case 0xaf00: case 0xaf40: case 0xaf80: case 0xafc0:
			pc = MakeEA (&ea1, hi, pc, 2);
			pc = MakeEA (&ea2, lo, pc, 2);
			sprintf (buffer, "CMPB  %s,%s", ea1, ea2);
			break;
		case 0xb000: case 0xb040: case 0xb080: case 0xb0c0: case 0xb100: case 0xb140: case 0xb180: case 0xb1c0:
		case 0xb200: case 0xb240: case 0xb280: case 0xb2c0: case 0xb300: case 0xb340: case 0xb380: case 0xb3c0:
		case 0xb400: case 0xb440: case 0xb480: case 0xb4c0: case 0xb500: case 0xb540: case 0xb580: case 0xb5c0:
		case 0xb600: case 0xb640: case 0xb680: case 0xb6c0: case 0xb700: case 0xb740: case 0xb780: case 0xb7c0:
		case 0xb800: case 0xb840: case 0xb880: case 0xb8c0: case 0xb900: case 0xb940: case 0xb980: case 0xb9c0:
		case 0xba00: case 0xba40: case 0xba80: case 0xbac0: case 0xbb00: case 0xbb40: case 0xbb80: case 0xbbc0:
		case 0xbc00: case 0xbc40: case 0xbc80: case 0xbcc0: case 0xbd00: case 0xbd40: case 0xbd80: case 0xbdc0:
		case 0xbe00: case 0xbe40: case 0xbe80: case 0xbec0: case 0xbf00: case 0xbf40: case 0xbf80: case 0xbfc0:
			pc = MakeEA (&ea1, hi, pc, 2);
			pc = MakeEA (&ea2, lo, pc, 2);
			sprintf (buffer, "BITB  %s,%s", ea1, ea2);
			break;
		case 0xc000: case 0xc040: case 0xc080: case 0xc0c0: case 0xc100: case 0xc140: case 0xc180: case 0xc1c0:
		case 0xc200: case 0xc240: case 0xc280: case 0xc2c0: case 0xc300: case 0xc340: case 0xc380: case 0xc3c0:
		case 0xc400: case 0xc440: case 0xc480: case 0xc4c0: case 0xc500: case 0xc540: case 0xc580: case 0xc5c0:
		case 0xc600: case 0xc640: case 0xc680: case 0xc6c0: case 0xc700: case 0xc740: case 0xc780: case 0xc7c0:
		case 0xc800: case 0xc840: case 0xc880: case 0xc8c0: case 0xc900: case 0xc940: case 0xc980: case 0xc9c0:
		case 0xca00: case 0xca40: case 0xca80: case 0xcac0: case 0xcb00: case 0xcb40: case 0xcb80: case 0xcbc0:
		case 0xcc00: case 0xcc40: case 0xcc80: case 0xccc0: case 0xcd00: case 0xcd40: case 0xcd80: case 0xcdc0:
		case 0xce00: case 0xce40: case 0xce80: case 0xcec0: case 0xcf00: case 0xcf40: case 0xcf80: case 0xcfc0:
			pc = MakeEA (&ea1, hi, pc, 2);
			pc = MakeEA (&ea2, lo, pc, 2);
			sprintf (buffer, "BICB  %s,%s", ea1, ea2);
			break;
		case 0xd000: case 0xd040: case 0xd080: case 0xd0c0: case 0xd100: case 0xd140: case 0xd180: case 0xd1c0:
		case 0xd200: case 0xd240: case 0xd280: case 0xd2c0: case 0xd300: case 0xd340: case 0xd380: case 0xd3c0:
		case 0xd400: case 0xd440: case 0xd480: case 0xd4c0: case 0xd500: case 0xd540: case 0xd580: case 0xd5c0:
		case 0xd600: case 0xd640: case 0xd680: case 0xd6c0: case 0xd700: case 0xd740: case 0xd780: case 0xd7c0:
		case 0xd800: case 0xd840: case 0xd880: case 0xd8c0: case 0xd900: case 0xd940: case 0xd980: case 0xd9c0:
		case 0xda00: case 0xda40: case 0xda80: case 0xdac0: case 0xdb00: case 0xdb40: case 0xdb80: case 0xdbc0:
		case 0xdc00: case 0xdc40: case 0xdc80: case 0xdcc0: case 0xdd00: case 0xdd40: case 0xdd80: case 0xddc0:
		case 0xde00: case 0xde40: case 0xde80: case 0xdec0: case 0xdf00: case 0xdf40: case 0xdf80: case 0xdfc0:
			pc = MakeEA (&ea1, hi, pc, 2);
			pc = MakeEA (&ea2, lo, pc, 2);
			sprintf (buffer, "BISB  %s,%s", ea1, ea2);
			break;
		case 0xe000: case 0xe040: case 0xe080: case 0xe0c0: case 0xe100: case 0xe140: case 0xe180: case 0xe1c0:
		case 0xe200: case 0xe240: case 0xe280: case 0xe2c0: case 0xe300: case 0xe340: case 0xe380: case 0xe3c0:
		case 0xe400: case 0xe440: case 0xe480: case 0xe4c0: case 0xe500: case 0xe540: case 0xe580: case 0xe5c0:
		case 0xe600: case 0xe640: case 0xe680: case 0xe6c0: case 0xe700: case 0xe740: case 0xe780: case 0xe7c0:
		case 0xe800: case 0xe840: case 0xe880: case 0xe8c0: case 0xe900: case 0xe940: case 0xe980: case 0xe9c0:
		case 0xea00: case 0xea40: case 0xea80: case 0xeac0: case 0xeb00: case 0xeb40: case 0xeb80: case 0xebc0:
		case 0xec00: case 0xec40: case 0xec80: case 0xecc0: case 0xed00: case 0xed40: case 0xed80: case 0xedc0:
		case 0xee00: case 0xee40: case 0xee80: case 0xeec0: case 0xef00: case 0xef40: case 0xef80: case 0xefc0:
			pc = MakeEA (&ea1, hi, pc, 4);
			pc = MakeEA (&ea2, lo, pc, 4);
			sprintf (buffer, "SUB   %s,%s", ea1, ea2);
			break;

        default:
			sprintf (buffer, "???? (%06o)", op);
			break;
	}

    return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}
