// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "dsp16.h"

#define DSP_LINE(__DSP_DOCLINE__) printf("0x%04x - %d (%s)\n", m_pc, __LINE__, __DSP_DOCLINE__);

// TODO:
//   * AUC has a CLR field for writing to A0 & A1 + sign extension + psw + zero lower bits
//     implement as a clean function (page 2-7)
//   * Implement saturation overflow (SAT on AUC) (page 2-8)
//   * Implement p alignment (ALIGN on AUC) (page 2-9)
//   * When a register is used as a memory pointer. its value is compared with re. If its value is
//     equal to the contents of re and the postincrement is +1, then the value in rb is copied into
//     the register after the memory access is complete. See Section 4.2.3.
//   * CPU flags go to the PSW & conditionTest() works on that (Page 3-4)
//   * Some instructions are not interruptible.
//


// NOTES:
// When y is used in an assembly-language instruction, the DSPI6/DSPI6A device will read
// or write the high half (bits 16-31) of the y register  (page 2-7)

// The YL register is the lower half of the 32 bit Y register
void* dsp16_device::addressYL()
{
	return (void*)(((UINT8*)&m_y) + 2);
}


// Flag getters
bool dsp16_device::lmi()
{
	return m_psw & 0x8000;
}

bool dsp16_device::leq()
{
	return m_psw & 0x4000;
}

bool dsp16_device::llv()
{
	return m_psw & 0x2000;
}

bool dsp16_device::lmv()
{
	return m_psw & 0x1000;
}


void dsp16_device::writeRegister(void* reg, const UINT16 &value)
{
	// Make sure you're not attempting to write somewhere this function doesn't support.
	if (reg == &m_p || reg == &m_a0 || reg == &m_a1)
	{
		logerror("dsp16::writeRegister called on invalid register at PC 0x%04x.\n", m_pc);
		return;
	}

	if (reg == &m_auc || reg == &m_c0 || reg == &m_c1 || reg == &m_c2)
	{
		// 8 bit registers
		*(UINT8*)reg = value & 0x00ff;
	}
	else if (reg == &m_psw)
	{
		// Writes to the a0 & a1 guard bits too
		m_a0 &= U64(0x0ffffffff);
		m_a0 |= U64(m_psw & 0x000f) << 32;
		m_a1 &= U64(0x0ffffffff);
		m_a1 |= U64(m_psw & 0x01e0) << 27;
		m_psw = value;
	}
	else if (reg == &m_i)
	{
		// 12 bit register
		m_i = value & 0x0fff;
	}
	else if (reg == &m_y)
	{
		// Y register
		// TODO - Automatic clearing of yl may be selected (according to the CLR field of the auc register)  (page 2-7)
		m_y = (value << 16) | (m_y & 0x0000ffff);
	}
	else if (reg == addressYL())
	{
		// Yl register (Writes to yl do not change the data in the high half of y)
		m_y = value | (m_y & 0xffff0000);
	}
	else
	{
		// Everything else
		*(UINT16*)reg = value;
	}
}


bool dsp16_device::conditionTest(const UINT8& CON)
{
	switch (CON)
	{
		case 0x00: return lmi();   // mi (negative result)
		case 0x01: return !lmi();  // pl (positive result)
		case 0x02: return leq();   // eq (result == 0)
		case 0x03: return !leq();  // ne (result != 0)
		case 0x04: return llv();   // lvs (logical overflow set)
		case 0x05: return !llv();  // lvc (logical overflow clear)
		case 0x06: return lmv();   // mvs (math. overflow set)
		case 0x07: return !lmv();  // mvc (math. overflow clear)
		case 0x08: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // heads (random bit set)
		case 0x09: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // tails (random bit clear)
		case 0x0a: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // c0ge (counter0 >= 0)*
		case 0x0b: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // c0lt (counter0 < 0)*
		case 0x0c: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // c1ge (counter1 >= 0)*
		case 0x0d: printf("UNIMPLEMENTED condition check @ PC 0x%04x\n", m_pc); return false;   // c1lt (counter1 < 0)*
		case 0x0e: return true;    // true (always)
		case 0x0f: return false;   // false (never)
		case 0x10: return (!lmi() && !leq());   // gt (result > 0)
		case 0x11: return (lmi()  ||  leq());   // le (result <= 0)
		default: logerror("Unrecognized condition at PC=0x%04x\n", m_pc); break;
	}

	// Testing each of these conditions (*) increments the respective counter being tested  (page 3-5)

	return false;
}


void* dsp16_device::registerFromRImmediateField(const UINT8& R)
{
	switch (R)
	{
		case 0x00: return (void*)&m_j;
		case 0x01: return (void*)&m_k;
		case 0x02: return (void*)&m_rb;
		case 0x03: return (void*)&m_re;
		case 0x04: return (void*)&m_r0;
		case 0x05: return (void*)&m_r1;
		case 0x06: return (void*)&m_r2;
		case 0x07: return (void*)&m_r3;

		default: return NULL;
	}
	return NULL;
}


void* dsp16_device::registerFromRTable(const UINT8 &R)
{
	switch (R)
	{
		case 0x00: return (void*)&m_r0;
		case 0x01: return (void*)&m_r1;
		case 0x02: return (void*)&m_r2;
		case 0x03: return (void*)&m_r3;
		case 0x04: return (void*)&m_j;
		case 0x05: return (void*)&m_k;
		case 0x06: return (void*)&m_rb;
		case 0x07: return (void*)&m_re;
		case 0x08: return (void*)&m_pt;
		case 0x09: return (void*)&m_pr;
		case 0x0a: return (void*)&m_pi;
		case 0x0b: return (void*)&m_i;

		case 0x10: return (void*)&m_x;
		case 0x11: return (void*)&m_y;
		case 0x12: return (void*)addressYL();
		case 0x13: return (void*)&m_auc;    // zero extended
		case 0x14: return (void*)&m_psw;
		case 0x15: return (void*)&m_c0;     // sign extended
		case 0x16: return (void*)&m_c1;     // sign extended
		case 0x17: return (void*)&m_c2;     // sign extended
		case 0x18: return (void*)&m_sioc;
		case 0x19: return (void*)&m_srta;
		case 0x1a: return (void*)&m_sdx;
		case 0x1b: logerror("dsp16::registerFromRTable tdms requested 0x%04x.\n", m_pc); break;
		case 0x1c: return (void*)&m_pioc;
		case 0x1d: return (void*)&m_pdx0;
		case 0x1e: return (void*)&m_pdx1;

		default: return NULL;
	}
	return NULL;
}


void dsp16_device::executeF1Field(const UINT8& F1, const UINT8& D, const UINT8& S)
{
	// TODO: I'm pretty sure we need to feed X into these as well - Double check

	// Note these instructions read right-to-left, so act accordingly  (page 3-6)
	// y & p are sign extended  (page 3-9)
	// implementation details  (page 3-9)

	// Where is are the results going?
	UINT64* destinationReg = NULL;
	switch (D)
	{
		case 0x00: destinationReg = &m_a0; break;
		case 0x01: destinationReg = &m_a1; break;
		default: break;
	}

	// Which source is being used?
	UINT64* sourceReg = NULL;
	switch (S)
	{
		case 0x00: sourceReg = &m_a0; break;
		case 0x01: sourceReg = &m_a1; break;
		default: break;
	}


	// We must compute into an intermediate variable to compute flags on
	UINT64 result = 0;
	bool justATest = false;

	switch (F1)
	{
		case 0x00:
		{
			// Ad = p   p = x*y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x01:
		{
			// Ad = aS+p   p = x*y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x02:
		{
			// p = x*y
			// TODO: What happens to the flags in this operation?
			const INT16 y = (m_y & 0xffff0000) >> 16;
			m_p = (INT32)((INT16)m_x * y);
			justATest = true;
			break;
		}
		case 0x03:
		{
			// Ad = aS-p   p = x*y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x04:
		{
			// Ad = p
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x05:
		{
			// Ad = aS+p
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x06:
		{
			// nop
			justATest = true;
			break;
		}
		case 0x07:
		{
			// Ad = aS-p
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x08:
		{
			// Ad = aS|y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x09:
		{
			// Ad = aS^y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x0a:
		{
			// aS&y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			justATest = true;
			break;
		}
		case 0x0b:
		{
			// aS-y
			INT64 aS = *sourceReg;
			if (aS & U64(0x800000000))
				aS |= U64(0xfffffff000000000);

			INT64 y  = (m_y & 0xffff0000) >> 16;
			if (y & 0x8000)
				y |= U64(0xffffffffffff0000);

			result = aS-y;
			justATest = true;
			break;
		}
		case 0x0c:
		{
			// Ad = y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x0d:
		{
			// Ad = aS+y
			INT64 aS = *sourceReg;
			if (aS & U64(0x800000000))
				aS |= U64(0xfffffff000000000);

			INT64 y  = (m_y & 0xffff0000) >> 16;
			if (y & 0x8000)
				y |= U64(0xffffffffffff0000);

			result = aS+y;
			break;
		}
		case 0x0e:
		{
			// Ad = aS&y
			printf("UNIMPLEMENTED F1 operation @ PC 0x%04x (%d)\n", m_pc, __LINE__);
			break;
		}
		case 0x0f:
		{
			// Ad = aS-y
			INT64 aS = *sourceReg;
			if (aS & U64(0x800000000))
				aS |= U64(0xfffffff000000000);

			INT64 y  = (m_y & 0xffff0000) >> 16;
			if (y & 0x8000)
				y |= U64(0xffffffffffff0000);

			result = aS-y;
			break;
		}
	}

	// CPU Flags  (page 3-4)
	// LMI (logical minus)
	if (result & U64(0x800000000))
		m_psw |= 0x8000;
	else
		m_psw &= (~0x8000);

	// LEQ (logical equal)
	if (result == U64(0x000000000))
		m_psw |= 0x4000;
	else
		m_psw &= (~0x4000);

	// LLV (logical overflow)
	// TODO

	// LMV (mathematical overflow)
	if ((result & U64(0xf00000000)) != U64(0xf00000000) &&
		(result & U64(0xf00000000)) != U64(0x000000000))
		m_psw |= 0x1000;
	else
		m_psw &= (~0x1000);

	// If it was a real operation, make sure the data goes where it should
	if (!justATest)
		*destinationReg = (UINT64)result & U64(0x0000000fffffffff);
}


UINT16* dsp16_device::registerFromYFieldUpper(const UINT8& Y)
{
	UINT16* destinationReg = NULL;
	const UINT8 N = (Y & 0x0c) >> 2;
	switch (N)
	{
		case 0x00: destinationReg = &m_r0; break;
		case 0x01: destinationReg = &m_r1; break;
		case 0x02: destinationReg = &m_r2; break;
		case 0x03: destinationReg = &m_r3; break;
		default: break;
	}
	return destinationReg;
}


void dsp16_device::executeYFieldPost(const UINT8& Y)
{
	UINT16* opReg = registerFromYFieldUpper(Y);

	const UINT8 lower = Y & 0x03;
	switch (lower)
	{
		case 0x00: /* nop */ break;
		case 0x01: (*opReg)++; break;
		case 0x02: (*opReg)--; break;
		case 0x03: (*opReg) += m_j; break;  // TODO: J is signed
	}
}


void dsp16_device::executeZFieldPartOne(const UINT8& Z, UINT16* rN)
{
	const UINT8 lower = Z & 0x03;
	switch (lower)
	{
		case 0x00: /* nop */ break;
		case 0x01: (*rN)++; break;
		case 0x02: (*rN)--; break;
		case 0x03: (*rN) += m_j; break;  // TODO: J is signed
	}
}


void dsp16_device::executeZFieldPartTwo(const UINT8& Z, UINT16* rN)
{
	const UINT8 lower = Z & 0x03;
	switch (lower)
	{
		case 0x00: (*rN)++; break;
		case 0x01: /* nop */   break;
		case 0x02: (*rN) += 2; break;
		case 0x03: (*rN) += m_k; break;  // TODO: K is signed
	}
}


void dsp16_device::execute_one(const UINT16& op, UINT8& cycles, UINT8& pcAdvance)
{
	cycles = 1;
	pcAdvance = 0;

// NOTE: pages 3-5 through 3-19 are good english descriptions of what's up

	const UINT8 opcode = (op >> 11) & 0x1f;
	switch(opcode)
	{
		// Format 1: Multiply/ALU Read/Write Group
		case 0x06:
		{
			DSP_LINE("3-38")
			// F1, Y  :  (page 3-38)
			const UINT8 Y = (op & 0x000f);
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			executeYFieldPost(Y);
			cycles = 1;
			pcAdvance = 1;
			break;
		}
		case 0x04: case 0x1c:
		{
			DSP_LINE("3-40")
			// F1 Y=a0[1] | F1 Y=a1[1]  :  (page 3-40)
			const UINT8 Y = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			UINT16* destinationReg = registerFromYFieldUpper(Y);
			// (page 3-18)
			UINT16 aRegValue = 0x0000;
			if (op & 0xc000)
			{
				aRegValue = (m_a0 & U64(0x0ffff0000)) >> 16;
			}
			else
			{
				aRegValue = (m_a1 & U64(0x0ffff0000)) >> 16;
			}
			data_write(*destinationReg, aRegValue);
			executeYFieldPost(Y);
			executeF1Field(F1, D, S);
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x16:
		{
			DSP_LINE("3-42")
			// F1, x = Y  :  (page 3-42)
			const UINT8 Y = (op & 0x000f);
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			UINT16* sourceReg = registerFromYFieldUpper(Y);
			writeRegister(&m_x, data_read(*sourceReg));
			executeYFieldPost(Y);
			cycles = 1;
			pcAdvance = 1;
			break;
		}
		case 0x17:
		{
			DSP_LINE("3-44")
			// F1, y[l] = Y  :  (page 3-44)
			const UINT8 Y = (op & 0x000f);
			const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			UINT16* sourceReg = registerFromYFieldUpper(Y);
			UINT16 sourceValue = data_read(*sourceReg);
			switch (X)
			{
				case 0x00: writeRegister(addressYL(), sourceValue); break;
				case 0x01: writeRegister(&m_y, sourceValue); break;
				default: break;
			}
			executeYFieldPost(Y);
			cycles = 1;
			pcAdvance = 1;
			break;
		}
		case 0x1f:
		{
			DSP_LINE("3-46")
			// F1, y = Y, x = *pt++[i]  :  (page 3-46)
			const UINT8 Y = (op & 0x000f);
			const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			UINT16* sourceRegR = registerFromYFieldUpper(Y);
			writeRegister(&m_y, data_read(*sourceRegR));
			executeYFieldPost(Y);
			writeRegister(&m_x, data_read(m_pt));
			switch (X)
			{
				case 0x00: m_pt++;      break;
				case 0x01: m_pt += m_i; break;
			}
			cycles = 2;     // TODO: 1 if cached
			pcAdvance = 1;
			break;
		}
		case 0x19: case 0x1b:
		{
			DSP_LINE("3-48")
			// F1, y = a0|1, x = *pt++[i]  :  (page 3-48)
			const UINT8 Y = (op & 0x000f);
			const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			bool useA1 = (opcode == 0x1b);
			if (Y != 0x00) printf("Unknown opcode @ PC=0x%04x", m_pc);
			m_y = (useA1) ? (m_a1 & 0xffffffff) : (m_a0 & 0xffffffff);      // TODO: What happens to Ax when it goes 32 bit (pc=3f & pc=47)?
			executeF1Field(F1, D, S);
			writeRegister(&m_x, data_read(m_pt));                           // TODO: EXM Pin & internal/external ROM?  Research.
			switch (X)
			{
				case 0x00: m_pt++;      break;
				case 0x01: m_pt += m_i; break;
			}
			cycles = 2;     // TODO: 1 if cached
			pcAdvance = 1;
			break;
		}
		case 0x14:
		{
			DSP_LINE("3-53")
			// F1, Y = y[l]  :  (page 3-53)
			const UINT8 Y = (op & 0x000f);
			const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			UINT16* destinationReg = registerFromYFieldUpper(Y);
			UINT16 yRegValue = 0x0000;
			switch (X)
			{
				case 0x00: yRegValue = (m_y & 0x0000ffff); break;
				case 0x01: yRegValue = (m_y & 0xffff0000) >> 16; break;
				default: break;
			}
			data_write(*destinationReg, yRegValue);
			executeYFieldPost(Y);
			cycles = 2;
			pcAdvance = 1;
			break;
		}

		// Format 1a: Multiply/ALU Read/Write Group (TODO: Figure out major typo in docs on p3-51)
		case 0x07:
		{
			DSP_LINE("3-50")
			// F1, At[1] = Y  :  (page 3-50)
			// TODO: What does the X field do here, exactly?
			const UINT8 Y = (op & 0x000f);
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 aT = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, !aT, S);
			UINT64* destinationReg = NULL;
			switch(aT)
			{
				case 0: destinationReg = &m_a1; break;
				case 1: destinationReg = &m_a0; break;
				default: break;
			}
			UINT16 sourceAddress = *(registerFromYFieldUpper(Y));
			INT64 sourceValueSigned = (INT16)data_read(sourceAddress);
			*destinationReg = sourceValueSigned & U64(0xffffffffff);
			executeYFieldPost(Y);
			cycles = 1;
			pcAdvance = 1;
			break;
		}

		// Format 2: Multiply/ALU Read/Write Group
		case 0x15:
		{
			DSP_LINE("3-54")
			// F1, Z : y[l]  :  (page 3-54)
			const UINT8 Z = (op & 0x000f);
			const UINT8 X = (op & 0x0010) >> 4;
			const UINT8 S = (op & 0x0200) >> 9;
			const UINT8 D = (op & 0x0400) >> 10;
			const UINT8 F1 = (op & 0x01e0) >> 5;
			executeF1Field(F1, D, S);
			UINT16 temp = 0x0000;
			UINT16* rN = registerFromYFieldUpper(Z);
			switch (X)
			{
				case 0x00:
					temp = m_y & 0x0000ffff;
					m_y &= 0xffff0000;
					m_y |= data_read(*rN);
					executeZFieldPartOne(Z, rN);
					data_write(*rN, temp);
					executeZFieldPartTwo(Z, rN);
					break;
				case 0x01:
					temp = (m_y & 0xffff0000) >> 16;
					m_y &= 0x0000ffff;
					m_y |= (data_read(*rN) << 16);
					executeZFieldPartOne(Z, rN);
					data_write(*rN, temp);
					executeZFieldPartTwo(Z, rN);
					break;
			}
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x1d:
		{
			DSP_LINE("?")
			// F1, Z : y, x=*pt++[i]
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 2a: Multiply/ALU Read/Write Group
		case 0x05:
		{
			DSP_LINE("?")
			// F1, Z : aT[1]
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 X = (op & 0x0010) >> 4;
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 aT = (op & 0x0400) >> 10;
			//const UINT8 F1 = (op & 0x01e0) >> 5;
			break;
		}

		// Format 3: Special Functions
		case 0x12:
		case 0x13:
		{
			DSP_LINE("3-36")
			// if|ifc CON F2  (page 3-36)
			const UINT8 CON = (op & 0x001f);
			//const UINT8 S = (op & 0x0200) >> 9;
			//const UINT8 D = (op & 0x0400) >> 10;
			//const UINT8 F2 = (op & 0x01e0) >> 5;
			bool conditionFulfilled = conditionTest(CON);
			if (conditionFulfilled)
			{
				printf("Fulfilled condition not yet implemented @ PC=0x%04x\n", m_pc);
			}
			cycles = 1;
			pcAdvance = 1;
			break;
		}

		// Format 4: Branch Direct Group
		case 0x00: case 0x01:
		{
			DSP_LINE("3-20")
			// goto JA  :  (page 3-20) (DONE)
			const UINT16 JA = (op & 0x0fff) | (m_pc & 0xf000);
			m_pc = JA;
			cycles = 2;
			pcAdvance = 0;
			break;
		}

		case 0x10: case 0x11:
		{
			DSP_LINE("3-23")
			// call JA  :  (page 3-23)
			const UINT16 JA = (op & 0x0fff) | (m_pc & 0xf000);
			m_pr = m_pc + 1;
			m_pc = JA;
			cycles = 2;
			pcAdvance = 0;
			break;
		}

		// Format 5: Branch Indirect Group
		case 0x18:
		{
			DSP_LINE("3-21")
			// goto B  :  (page 3-21)
			const UINT8 B = (op & 0x0700) >> 8;
			switch (B)
			{
				case 0x00: m_pc = m_pr; break;
				case 0x01: printf("UNIMPLEMENTED branch instruction @ PC 0x%04x\n", m_pc); break;
				case 0x02: printf("UNIMPLEMENTED branch instruction @ PC 0x%04x\n", m_pc); break;
				case 0x03: printf("UNIMPLEMENTED branch instruction @ PC 0x%04x\n", m_pc); break;
				default: logerror("DSP16: Invalid branch indirect instruction executed at PC=0x%04x\n.", m_pc); break;
			}
			cycles = 2;
			pcAdvance = 0;
			break;
		}

		// Format 6: Contitional Branch Qualifier/Software Interrupt (icall)
		case 0x1a:
		{
			DSP_LINE("3-22")
			// if CON [goto/call/return]  :  (page 3-22)
			const UINT8 CON = (op & 0x001f);
			bool conditionFulfilled = conditionTest(CON);
			cycles = 3;                 // TODO: This may need to interact with the next opcode to make sure it doesn't exceed 3?
			pcAdvance = 1;
			if (!conditionFulfilled)
			{
				pcAdvance = 2;
			}
			break;
		}

		// Format 7: Data Move Group
		case 0x09: case 0x0b:
		{
			DSP_LINE("3-29")
			// R = aS  :  (page 3-29)
			// TODO: Fix register pdxX (pc=338)
			const UINT8 R = (op & 0x03f0) >> 4;
			const UINT8 S = (op & 0x1000) >> 12;
			void* destinationReg = registerFromRTable(R);
			UINT64* sourceReg = (S) ? &m_a1 : &m_a0;
			UINT16 sourceValue = (*sourceReg & U64(0x0ffff0000)) >> 16;
			writeRegister(destinationReg, sourceValue);
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x08:
		{
			DSP_LINE("3-30")
			// aT = R  :  (page 3-30)
			const UINT8 R  = (op & 0x03f0) >> 4;
			const UINT8 aT = (op & 0x0400) >> 10;
			UINT64* destinationReg = NULL;
			switch(aT)
			{
				case 0: destinationReg = &m_a1; break;
				case 1: destinationReg = &m_a0; break;
				default: break;
			}
			void* sourceReg = registerFromRTable(R);
			*destinationReg &= U64(0x00000ffff);
			*destinationReg |= (*(UINT16*)sourceReg) << 16;     // TODO: Fix for all registers
			if (*(UINT16*)sourceReg & 0x8000)
				*destinationReg |= U64(0xf00000000);
			// TODO: Special function encoding
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x0f:
		{
			DSP_LINE("3-32")
			// R = Y  :  (page 3-32)
			const UINT8 Y = (op & 0x000f);
			const UINT8 R = (op & 0x03f0) >> 4;
			UINT16* sourceReg = registerFromYFieldUpper(Y);
			void* destinationReg = registerFromRTable(R);
			writeRegister(destinationReg, data_read(*sourceReg));
			executeYFieldPost(Y);
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x0c:
		{
			DSP_LINE("3-33")
			// Y = R  :  (page 3-33)
			// TODO: Zero & Sign extend i, c0, c1, c2, and auc
			const UINT8 Y = (op & 0x000f);
			const UINT8 R = (op & 0x03f0) >> 4;
			UINT16* destinationReg = registerFromYFieldUpper(Y);
			UINT16* sourceReg = (UINT16*)registerFromRTable(R);     // TODO: This won't work for certain registers!
			data_write(*destinationReg, *sourceReg);                //       Fix in data_write() maybe?
			executeYFieldPost(Y);
			cycles = 2;
			pcAdvance = 1;
			break;
		}
		case 0x0d:
		{
			DSP_LINE("?")
			// Z : R
			//const UINT8 Z = (op & 0x000f);
			//const UINT8 R = (op & 0x03f0) >> 4;
			break;
		}

		// Format 8: Data Move (immediate operand - 2 words)
		case 0x0a:
		{
			DSP_LINE("3-28")
			// R = N  :  (page 3-28) (DONE)
			// NOTE: The docs speak of register sources & sign extension, but this is a register
			// destination, so, typo?  If so, what does one do with the overflow bits?
			const UINT8 R = (op & 0x03f0) >> 4;
			const UINT16 iVal = opcode_read(1);
			void* destinationReg = registerFromRTable(R);
			writeRegister(destinationReg, iVal);
			cycles = 2;
			pcAdvance = 2;
			break;
		}

		// Format 9: Short Immediate Group
		case 0x02: case 0x03:
		{
			DSP_LINE("3-27")
			// R = M  :  (page 3-27)
			// TODO: Figure out notes about the DSP16A vs the DSP16.  9 bit is very DSP16...
			const UINT16 M = (op & 0x01ff);
			const UINT8  R = (op & 0x0e00) >> 9;
			void* destinationReg = registerFromRImmediateField(R);
			// Sign extend if the destination is j or k
			UINT16 mValue = M;
			if (destinationReg == &m_j || destinationReg == &m_k)
			{
				if (mValue & 0x0100) mValue |= 0xfe00;
			}
			writeRegister(destinationReg, mValue);
			cycles = 1;
			pcAdvance = 1;
			break;
		}

		// Format 10: do - redo
		case 0x0e:
		{
			DSP_LINE("3-25/3-26")
			// do|redo K  :  (pages 3-25 & 3-26)
			// TODO: The timings are intricate to say the least...
			const UINT8 K = (op & 0x007f);
			const UINT8 NI = (op & 0x0780) >> 7;
			if (NI != 0)
			{
				// Do
				m_cacheStart = m_pc + 1;
				m_cacheEnd   = m_pc + 1 + NI;
				m_cacheIterations = K-1;    // -1 because we check the counter @ the end
				cycles = 1;
				pcAdvance = 1;
			}
			else
			{
				// Redo
				m_cacheIterations = K-1;    // -1 because we check the counter @ the end
				m_cacheRedoNextPC = m_pc + 1;
				m_pc = m_cacheStart;
				cycles = 2;
				pcAdvance = 0;
			}
			break;
		}

		// RESERVED
		case 0x1e:
		{
			DSP_LINE("XXX")
			break;
		}

		// UNKNOWN
		default:
		{
			DSP_LINE("XXX")
			break;
		}
	}

	// Handle end-of-cache conditions for do|redos
	if (m_cacheIterations == 0 && m_cacheRedoNextPC != CACHE_INVALID)
	{
		// You've reached the end of a cache loop after a redo opcode.
		m_pc = m_cacheRedoNextPC;
		m_cacheRedoNextPC = CACHE_INVALID;
		pcAdvance = 0;
	}
	if (m_cacheIterations > 0 && (m_pc+pcAdvance == m_cacheEnd))
	{
		// A regular iteration on a cached loop.
		m_cacheIterations--;
		m_pc = m_cacheStart;
		pcAdvance = 0;
	}
}
