// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    PDP-8 family disassembler

    PDP-8 peripherals were traditionally classified by their device codes,
    which correspond to the second and third octal digits (bits 3:8) of
    the IOT instruction word. Older PDP-8 buses had the CPU decode the
    least significant three bits (9:11) into separate strobes issued one
    after another, which allows some IOTs with codes 6XX1, 6XX2 and 6XX4
    to be usefully combined with each other. Though the notion of device
    codes persisted in the Omnibus specification and CMOS-based systems,
    the I/O transfer protocols used on the PDP-8/E and later CPUs assert
    the lowest three bits in parallel with the device code, leaving
    decoding of all nine bits up to the peripherals. This allowed later
    options to define eight distinct functions for each of their assigned
    device codes. (The peripherals are also responsible for determining,
    in real time, whether their IOTs perform reads, writes and/or
    conditional skips; for the PDP-8/E, PDP-8/A and IM6100 CPUs, they may
    also request relative or absolute jumps.)

    Most IOTs were traditionally implemented on peripheral options outside
    the CPU, which originally supported only ION and IOF by itself. (The
    PDP-8/E CPU added several more which became standard.) It would have
    been convenient if DEC had assigned every common device type a
    distinct and unchanging set of IOTs. Unfortunately, not only do
    several types of options conflict with each other (a few of these
    could also have their standard device codes modified by jumpers,
    particularly storage and communications options which could have
    several used in one system), many Omnibus options use IOTs largely if
    not entirely different from their functional predecessors built for
    the older positive and negative buses. DEC reshuffled IOTs less often
    after the PDP-8/E, but real time clock and parallel I/O facilities
    were incompatibly reimplemented on the PDP-8/A and CMOS-8s.

    Due to this inconsistency, the "pdp8" disassembler implemented here
    only specifically identifies IOTs for a few of the most commonly
    used I/O devices with long and consistent implementation histories
    (some of which date back all the way to the PDP-5, though the Omnibus
    interfaces, being functional supersets, are the reference versions):
    * TTY interface: keyboard (03) + teleprinter (04)
    * High-speed paper tape reader (01) and punch (02)
    * Memory extension and time-sharing modes (20–27)
    * Power fail and automatic restart (10)

    The Group 3 OPR instructions belonging to the Extended Arithmetic
    Element are also recognized here, though several are unique to the
    KE8-E version provided for the PDP-8/E, and almost no EAE support was
    provided for later PDP-8 CPUs except for transfers between AC and
    MQ. (GT was reintroduced as a general-purpose flag on the HD-6120.)
    PDP-8/L omits this group entirely, not even recognizing CLA.

    Group 1 OPR combinations are decoded using the standard three-letter
    shorthand mnemonics. Even shorter macros for these were commonly
    defined to identify those useful for loading certain constants into
    AC; the resulting constant values are presented here as comments.

    Certain combinations of IAC, CMA, CML and rotate OPRs are illegal on
    the PDP-5, the original PDP-8 and the PDP-8/S; of these models, the
    "Classic-8", which disallows IAC with any rotation, is the only one
    many programs even tried to remain compatible with. For more obvious
    reasons, RAL RAR and RTL RTR were officially undefined before HD-6120
    reassigned the former as R3L, though they had some predictable and
    even useful results which were highly model-dependent.

    The "hd6120" disassembler, besides adding the HD-6120 exclusive R3L
    operation, specifically recognizes only its internal IOTs, which now
    include the memory extension IOTs plus a large number of completely
    original operations using device codes 20 through 27 (which are
    strictly internal here and unavailable to external peripherals). The
    different IOTs for panel mode are used when the disassembly address
    points above the normal 32K words. HD-6120 based systems tended to
    reimplement external IOTs using the HD-6121 Input/Output Controller,
    sometimes partially emulating them with panel-mode programs, and the
    resulting interfaces offered notoriously poor compatibility with
    preexisting code.

***************************************************************************/

#include "emu.h"
#include "pdp8dasm.h"

pdp8_disassembler::pdp8_disassembler(bool has_r3l)
	: util::disasm_interface()
	, m_has_r3l(has_r3l)
{
}

hd6120_disassembler::hd6120_disassembler()
	: pdp8_disassembler(true)
{
}

u32 pdp8_disassembler::opcode_alignment() const
{
	return 1;
}

u32 hd6120_disassembler::interface_flags() const
{
	return PAGED2LEVEL;
}

u32 hd6120_disassembler::page_address_bits() const
{
	// 5 bits for page + 7 bits for relative address
	return 12;
}

u32 hd6120_disassembler::page2_address_bits() const
{
	// 3 bits for current field
	return 3;
}

void pdp8_disassembler::dasm_memory_reference(std::ostream &stream, u16 inst, offs_t pc)
{
	// Bit 4 (DEC numbering) determines whether references are paged
	const u16 addr = (BIT(inst, 7) ? (pc & 07600) : 0) | (inst & 0177);

	// Bit 3 (DEC numbering) determines indirect addressing
	if (BIT(inst, 8))
	{
		util::stream_format(stream, " I %04o", addr);
		if ((addr & 07770) == 0010)
		{
			stream << "      /AUTO-INDEX";
			if (BIT(inst, 7))
				stream << "?"; // IM6100 does no auto-indexing in this case
		}
	}
	else
		util::stream_format(stream, " %04o", addr);
}

offs_t pdp8_disassembler::dasm_iot(std::ostream &stream, u16 dev, offs_t pc)
{
	switch (dev)
	{
	case 000:
		stream << "SKON"; // PDP-8/E and up
		return 1 | SUPPORTED | STEP_COND;

	case 001:
		stream << "ION";
		break;

	case 002:
		stream << "IOF";
		break;

	case 003:
		stream << "SRQ"; // PDP-8/E and up
		return 1 | SUPPORTED | STEP_COND;

	case 004:
		stream << "GTF"; // PDP-8/E and up (used earlier by a PDP-5/Classic-8 ADC option)
		break;

	case 005:
		stream << "RTF"; // PDP-8/E and up
		break;

	case 006:
		stream << "SGT"; // KE8-E
		return 1 | SUPPORTED | STEP_COND;

	case 007:
		stream << "CAF"; // PDP-8/E and up
		break;

	case 010:
		stream << "RPE"; // Paper tape reader (PC8-E)
		break;

	case 011:
		stream << "RSF"; // Paper tape reader
		return 1 | SUPPORTED | STEP_COND;

	case 012: case 016:
		stream << "RRB"; // Paper tape reader
		if (BIT(dev, 2))
			stream << " RFC";
		break;

	case 014:
		stream << "RFC"; // Paper tape reader
		break;

	case 020:
		stream << "PCE"; // Paper tape punch (PC8-E)
		break;

	case 021:
		stream << "PSF"; // Paper tape punch
		return 1 | SUPPORTED | STEP_COND;

	case 022:
		stream << "PCF"; // Paper tape punch
		break;

	case 024:
		stream << "PPC"; // Paper tape punch
		break;

	case 026:
		stream << "PLS"; // Paper tape punch
		break;

	case 030:
		stream << "KCF"; // TTY/keyboard (KL8-E and up)
		break;

	case 031:
		stream << "KSF"; // TTY/keyboard
		return 1 | SUPPORTED | STEP_COND;

	case 032:
		stream << "KCC"; // TTY/keyboard
		break;

	case 034:
		stream << "KRS"; // TTY/keyboard
		break;

	case 035:
		stream << "KIE"; // TTY/keyboard (KL8-E and up)
		break;

	case 036:
		stream << "KRB"; // TTY/keyboard
		break;

	case 040:
		stream << "TFL"; // Teleprinter (KL8-E and up)
		break;

	case 041:
		stream << "TSF"; // Teleprinter
		return 1 | SUPPORTED | STEP_COND;

	case 042:
		stream << "TCF"; // Teleprinter
		break;

	case 044:
		stream << "TPC"; // Teleprinter
		break;

	case 045:
		stream << "TSK"; // Teleprinter (KL8-E and up; SPI is a later name for this IOT)
		return 1 | SUPPORTED | STEP_COND;

	case 046:
		stream << "TLS"; // Teleprinter
		break;

	case 0102:
		stream << "SPL"; // Automatic restart
		return 1 | SUPPORTED | STEP_COND;

	case 0201: case 0202: case 0203:
	case 0211: case 0212: case 0213:
	case 0221: case 0222: case 0223:
	case 0231: case 0232: case 0233:
	case 0241: case 0242: case 0243:
	case 0251: case 0252: case 0253:
	case 0261: case 0262: case 0263:
	case 0271: case 0272: case 0273: // Memory extension
		if (BIT(dev, 0))
			stream << "CDF ";
		if (BIT(dev, 1))
			stream << "CIF ";
		util::stream_format(stream, "%o", dev & 070);
		break;

	case 0204:
		stream << "CINT"; // Time-sharing extension
		break;

	case 0214:
		stream << "RDF"; // Memory extension
		break;

	case 0224:
		stream << "RIF"; // Memory extension
		break;

	case 0234:
		stream << "RIB"; // Memory extension
		break;

	case 0244:
		stream << "RMF"; // Memory extension
		break;

	case 0254:
		stream << "SINT"; // Time-sharing extension (LIF on some Intersil peripherals)
		return 1 | SUPPORTED | STEP_COND;

	case 0264:
		stream << "CUF"; // Time-sharing extension
		break;

	case 0274:
		stream << "SUF"; // Time-sharing extension
		break;

	default:
		util::stream_format(stream, "IOT %03o", dev);
		return 1 | SUPPORTED | STEP_COND; // any IOT can theoretically cause a skip
	}

	return 1 | SUPPORTED;
}


offs_t hd6120_disassembler::dasm_iot(std::ostream &stream, u16 dev, offs_t pc)
{
	switch (dev)
	{
	case 000:
		if (BIT(pc, 15))
			stream << "PRS";
		else
		{
			stream << "SKON";
			return 1 | SUPPORTED | STEP_COND;
		}
		break;

	case 001:
		stream << "ION";
		break;

	case 002:
		stream << "IOF";
		break;

	case 003:
		if (BIT(pc, 15))
			stream << "PGO";
		else
		{
			stream << "SRQ";
			return 1 | SUPPORTED | STEP_COND;
		}
		break;

	case 004:
		if (BIT(pc, 15))
			stream << "PEX";
		else
			stream << "GTF";
		break;

	case 005:
		stream << "RTF";
		break;

	case 006:
		stream << "SGT";
		return 1 | SUPPORTED | STEP_COND;

	case 007:
		stream << "CAF";
		break;

	case 0201: case 0202: case 0203:
	case 0211: case 0212: case 0213:
	case 0221: case 0222: case 0223:
	case 0231: case 0232: case 0233:
	case 0241: case 0242: case 0243:
	case 0251: case 0252: case 0253:
	case 0261: case 0262: case 0263:
	case 0271: case 0272: case 0273:
		if (BIT(dev, 0))
			stream << "CDF ";
		if (BIT(dev, 1))
			stream << "CIF ";
		util::stream_format(stream, "%o", dev & 070);
		break;

	case 0205: case 0215: case 0245: case 0255:
		util::stream_format(stream, "P%cC%d", BIT(dev, 3) ? 'A' : 'P', BIT(dev, 5) ? 2 : 1);
		break;

	case 0206: case 0216: case 0226: case 0236:
		if (BIT(pc, 15))
			util::stream_format(stream, "IOT %03o", dev); // NOPs if already in panel mode
		else
			util::stream_format(stream, "PR%d", (dev & 030) >> 3);
		return 1 | SUPPORTED | STEP_OVER;

	case 0207: case 0217: case 0227: case 0237:
		util::stream_format(stream, "%cSP%d", BIT(dev, 3) ? 'L' : 'R', BIT(dev, 4) ? 2 : 1);
		break;

	case 0214:
		stream << "RDF";
		break;

	case 0224:
		stream << "RIF";
		break;

	case 0225: case 0265:
		util::stream_format(stream, "RTN%d", BIT(dev, 5) ? 2 : 1);
		return 1 | SUPPORTED | STEP_OUT;

	case 0235: case 0275:
		util::stream_format(stream, "POP%d", BIT(dev, 5) ? 2 : 1);
		break;

	case 0234:
		stream << "RIB";
		break;

	case 0244:
		stream << "RMF";
		break;

	case 0246:
		stream << "WSR";
		break;

	case 0256:
		stream << "GCF";
		break;

	case 0266: case 0276:
		if (BIT(pc, 15))
			util::stream_format(stream, "%cPD", BIT(dev, 3) ? 'S' : 'C');
		else
			util::stream_format(stream, "IOT %03o", dev); // Panel mode only
		break;

	default:
		util::stream_format(stream, "IOT %03o", dev);
		return 1 | SUPPORTED | STEP_COND;
	}

	return 1 | SUPPORTED;
}

// Sequence 1: CLA, CLL
// Sequence 2: CMA, CML
// Sequence 3: IAC
// Sequence 4: RAL, RTL, RAR, RTR, BSW, R3L
void pdp8_disassembler::dasm_opr_group1(std::ostream &stream, u16 opr)
{
	if (opr == 000)
		stream << "NOP";
	else if ((opr & 0357) == 0204)
	{
		if (BIT(opr, 4))
			stream << "CML ";
		stream << "GLK"; // Get link (CLA RAL)
	}
	else
	{
		int count = 0;
		switch (opr & 0240)
		{
		case 000:
			break;

		case 040:
			if (BIT(opr, 0))
				stream << "CIA"; // Complement and increment (CMA IAC); may not be combined with rotate operations on Classic-8
			else
				stream << "CMA";
			++count;
			break;

		case 0200:
			stream << "CLA";
			++count;
			break;

		case 0240:
			stream << "STA"; // Set accumulator (CLA CMA)
			++count;
			break;
		}
		switch (opr & 0120)
		{
		case 000:
			break;

		case 020:
			if (count != 0)
				stream << " ";
			stream << "CML";
			++count;
			break;

		case 0100:
			if (count != 0)
				stream << " ";
			stream << "CLL";
			++count;
			break;

		case 0120:
			if (count != 0)
				stream << " ";
			stream << "STL"; // Set link (CLL CML)
			++count;
			break;
		}
		if (BIT(opr, 0) && (opr & 0240) != 040)
		{
			if (count != 0)
				stream << " ";
			stream << "IAC"; // May not be combined with rotate operations on Classic-8
			++count;
		}

		// Rotate operations
		if (m_has_r3l && (opr & 016) == 014)
		{
			if (count != 0)
				stream << " ";
			stream << "R3L";
			++count;
		}
		else if ((opr & 016) == 002)
		{
			if (count != 0)
				stream << " ";
			stream << "BSW"; // PDP-8/E and up
			++count;
		}
		else
		{
			if (BIT(opr, 2))
			{
				if (count != 0)
					stream << " ";
				util::stream_format(stream, "R%cL", BIT(opr, 1) ? 'T' : 'A');
				++count;
			}
			if (BIT(opr, 3))
			{
				if (count != 0)
					stream << " ";
				util::stream_format(stream, "R%cR", BIT(opr, 1) ? 'T' : 'A');
				++count;
			}
		}

		u16 nl;
		switch (opr & 0314)
		{
		case 0200: case 0300:
			nl = (opr & 041) == 040 ? 07777 : (opr & 041) == 001 ? 1 : 0;
			if ((opr & 042) == 002)
				nl <<= 6;
			util::stream_format(stream, "%*s/NL%04o", 17 - std::min(count, 4) * 4, "", nl);
			break;

		case 0214: case 0314:
			if (m_has_r3l && !BIT(opr, 1))
			{
				nl = (opr & 041) == 040 ? 07777 : (opr & 041) == 001 ? 010 : 0;
				util::stream_format(stream, "%*s/NL%04o", 17 - std::min(count, 4) * 4, "", nl);
			}
			break;

		case 0304:
			nl = ((opr & 041) == 001 ? 2 : 0) | (BIT(opr, 4) != BIT(opr, 5) ? 1 : 0);
			if (BIT(opr, 1))
				nl <<= 1;
			if ((opr & 041) == 040)
				nl ^= 07777;
			util::stream_format(stream, "%*s/NL%04o", 17 - std::min(count, 4) * 4, "", nl);
			break;

		case 0310:
			nl = BIT(opr, 4) != BIT(opr, 5) ? 04000 : 0;
			if (BIT(opr, 1))
			{
				nl >>= 1;
				if ((opr & 041) == 001)
					nl |= 04000;
			}
			if ((opr & 041) == 040)
				nl ^= 07777;
			util::stream_format(stream, "%*s/NL%04o", 17 - std::min(count, 4) * 4, "", nl);
			break;
		}
	}
}

// Sequence 1a: SKP, SPA, SNA, SZL (skip if all conditions hold)
// Sequence 1b: SMA, SZA, SZL (skip if any condition holds)
// Sequence 2: CLA
// Sequence 3: OSR, HLT
offs_t pdp8_disassembler::dasm_opr_group2(std::ostream &stream, u16 opr)
{
	if ((opr & 0170) != 0)
	{
		// Skip instructions
		if ((opr & 0170) == 010)
			stream << "SKP";
		else
		{
			if (BIT(opr, 6))
			{
				stream << (BIT(opr, 3) ? "SPA" : "SMA");
				if ((opr & 060) != 0)
					stream << " ";
			}
			if (BIT(opr, 5))
			{
				stream << (BIT(opr, 3) ? "SNA" : "SZA");
				if (BIT(opr, 4))
					stream << " ";
			}
			if (BIT(opr, 4))
				stream << (BIT(opr, 3) ? "SZL" : "SNL");
		}
		if ((opr & 0206) != 0)
			stream << " ";
	}
	if (BIT(opr, 2))
	{
		if (BIT(opr, 7))
			stream << "LSR"; // Load switch register (CLA OSR)
		else
			stream << "OSR";
		if (BIT(opr, 1))
			stream << " HLT";
	}
	else if (BIT(opr, 1))
	{
		if (BIT(opr, 7))
			stream << "CLA ";
		stream << "HLT";
	}
	else
	{
		if (BIT(opr, 7))
			stream << "CLA";
		else if (opr == 0400)
			stream << "NOP";
		if ((opr & 0170) == 0)
			stream << "!400";
	}

	// HACK: HLT is a software trap on HD-6120
	if (m_has_r3l && BIT(opr, 1))
		return 1 | SUPPORTED | STEP_OVER;
	else if ((opr & 0160) != 0)
		return 1 | SUPPORTED | STEP_COND;
	else
		return 1 | SUPPORTED;
}

// Sequence 1: CLA
// Sequence 2: MQL, MQA (also SCA on older EAEs)
// Sequence 3: MUY, DIV, NMI, SHL, ASR, LSR, etc.
offs_t pdp8_disassembler::dasm_opr_group3(std::ostream &stream, u16 opr, offs_t pc, const pdp8_disassembler::data_buffer &opcodes)
{
	switch (opr & 0320)
	{
	case 000:
		if (opr == 0401)
			stream << "NOP";
		break;

	case 020:
		if (opr == 0431)
		{
			stream << "SWAB"; // KE8-E Mode A (change to Mode B)
			return 1 | SUPPORTED;
		}
		else
			stream << "MQL"; // Extended Arithmetic Element or internal for PDP-8/E and up
		break;

	case 0100:
		stream << "MQA"; // Extended Arithmetic Element or internal for PDP-8/E and up
		break;

	case 0120: case 0320:
		if (BIT(opr, 7))
			stream << "CLA ";
		if (opr == 0573)
		{
			stream << "DPIC"; // KE8-E Mode B
			return 1 | SUPPORTED;
		}
		else if (opr == 0575)
		{
			stream << "DCM"; // KE8-E Mode B
			return 1 | SUPPORTED;
		}
		else
			stream << "SWP"; // PDP-8/E and up
		break;

	case 0200:
		stream << "CLA"; // NOP on PDP-8/L
		break;

	case 0220:
		if (opr == 0663)
		{
			stream << "DLD"; // KE8-E Mode B (CAM DAD)
			return 1 | SUPPORTED;
		}
		else
			stream << "CAM"; // Extended Arithmetic Element or internal for PDP-8/E and up (CLA MQA)
		break;

	case 0300:
		stream << "ACL"; // Extended Arithmetic Element or internal for PDP-8/E and up (CLA MQL)
		break;
	}
	switch (opr & 056)
	{
	case 000:
		if ((opr & 0120) == 0)
			stream << "!201";
		break;

	case 002:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "ACS"; // KE8-E Mode B
		break;

	case 004:
		if ((opr & 0320) != 0)
			stream << " ";
		util::stream_format(stream, "MUY; %04o", opcodes.r16(pc + 1) & 07777); // Extended Arithmetic Element
		return 2 | SUPPORTED;

	case 006:
		if ((opr & 0320) != 0)
			stream << " ";
		util::stream_format(stream, "DVI; %04o", opcodes.r16(pc + 1) & 07777); // Extended Arithmetic Element
		return 2 | SUPPORTED;

	case 010:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "NMI"; // Extended Arithmetic Element
		break;

	case 012:
		if ((opr & 0320) != 0)
			stream << " ";
		util::stream_format(stream, "SHL; %04o", opcodes.r16(pc + 1) & 07777); // Extended Arithmetic Element
		return 2 | SUPPORTED;

	case 014: case 016:
		if ((opr & 0320) != 0)
			stream << " ";
		util::stream_format(stream, "%cSR; %04o", BIT(opr, 1) ? 'L' : 'A', opcodes.r16(pc + 1) & 07777); // Extended Arithmetic Element
		return 2 | SUPPORTED;

	case 040:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "SCA"; // Extended Arithmetic Element
		break;

	case 042:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "DAD"; // KE8-E Mode B
		break;

	case 044:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "DST"; // KE8-E Mode B
		break;

	case 046:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "SWBA"; // KE8-E Mode B (change to mode A)
		break;

	case 050:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "DPSZ"; // KE8-E Mode B
		return 1 | SUPPORTED | STEP_COND;

	case 056:
		if ((opr & 0320) != 0)
			stream << " ";
		stream << "SAM"; // KE8-E Mode B
		break;

	default:
		if ((opr & 0320) != 0)
			util::stream_format(stream, "!%02o", opr & 056);
		else
			util::stream_format(stream, "OPR %03o", opr & 0777);
		break;
	}
	return 1 | SUPPORTED;
}

offs_t hd6120_disassembler::dasm_opr_group3(std::ostream &stream, u16 opr, offs_t pc, const hd6120_disassembler::data_buffer &opcodes)
{
	switch (opr & 0320)
	{
	case 000:
		stream << "NOP";
		break;

	case 020:
		stream << "MQL";
		break;

	case 0100:
		stream << "MQA";
		break;

	case 0120: case 0320:
		if (BIT(opr, 7))
			stream << "CLA ";
		stream << "SWP";
		break;

	case 0200:
		stream << "CLA";
		break;

	case 0220:
		stream << "CAM";
		break;

	case 0300:
		stream << "ACL";
		break;
	}
	if ((opr & 0120) == 0)
		util::stream_format(stream, "!%03o", opr & 0457);
	else if ((opr & 056) != 0)
		util::stream_format(stream, "!%02o", opr & 056); // Inhibit interrupts for next instruction
	return 1 | SUPPORTED;
}

offs_t pdp8_disassembler::disassemble(std::ostream &stream, offs_t pc, const pdp8_disassembler::data_buffer &opcodes, const pdp8_disassembler::data_buffer &params)
{
	const u16 inst = opcodes.r16(pc);

	// Bits 0:2 (DEC numbering) specify instruction code
	switch (BIT(inst, 9, 3))
	{
	case 0:
		stream << "AND";
		dasm_memory_reference(stream, inst, pc);
		break;

	case 1:
		stream << "TAD";
		dasm_memory_reference(stream, inst, pc);
		break;

	case 2:
		stream << "ISZ";
		dasm_memory_reference(stream, inst, pc);
		return 1 | SUPPORTED | STEP_COND;

	case 3:
		stream << "DCA";
		dasm_memory_reference(stream, inst, pc);
		break;

	case 4:
		stream << "JMS";
		dasm_memory_reference(stream, inst, pc);
		return 1 | SUPPORTED | STEP_OVER;

	case 5:
		stream << "JMP";
		dasm_memory_reference(stream, inst, pc);
		if (BIT(inst, 8))
			return 1 | SUPPORTED | STEP_OUT;
		break;

	case 6:
		return dasm_iot(stream, inst & 0777, pc);

	case 7:
		if (!BIT(inst, 8))
			dasm_opr_group1(stream, inst & 0777);
		else if (!BIT(inst, 0))
			return dasm_opr_group2(stream, inst & 0777);
		else
			return dasm_opr_group3(stream, inst & 0777, pc, opcodes);
		break;
	}

	return 1 | SUPPORTED;
}
