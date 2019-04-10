// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
#include "emu.h"
#include "scspdsp.h"

#include <cstring>


namespace {

u16 PACK(s32 val)
{
	int const sign = BIT(val, 23);
	u32 temp = (val ^ (val << 1)) & 0xFFFFFF;
	int exponent = 0;
	for (int k = 0; k < 12; k++)
	{
		if (temp & 0x800000)
			break;
		temp <<= 1;
		exponent += 1;
	}
	if (exponent < 12)
		val = (val << exponent) & 0x3FFFFF;
	else
		val <<= 11;
	val >>= 11;
	val &= 0x7FF;
	val |= sign << 15;
	val |= exponent << 11;

	return u16(val);
}

static s32 UNPACK(u16 val)
{
	int const sign = BIT(val, 15);
	int exponent = (val >> 11) & 0xF;
	int const mantissa = val & 0x7FF;
	s32 uval = mantissa << 11;
	if (exponent > 11)
	{
		exponent = 11;
		uval |= sign << 22;
	}
	else
	{
		uval |= (sign ^ 1) << 22;
	}
	uval |= sign << 23;
	uval <<= 8;
	uval >>= 8;
	uval >>= exponent;

	return uval;
}

} // anonymous namespace


void SCSPDSP::Init()
{
	std::memset(this, 0, sizeof(*this));
	RBL = (8*1024); // Initial RBL is 0
	Stopped = true;
}

void SCSPDSP::Step()
{
	if (Stopped)
		return;

	std::fill(std::begin(EFREG), std::end(EFREG), 0);

#if 0
	int dump=0;
	FILE *f=nullptr;
	if(dump)
		f=fopen("dsp.txt","wt");
#endif

	s32 ACC = 0;    //26 bit
	s32 MEMVAL = 0;
	s32 FRC_REG = 0;    //13 bit
	s32 Y_REG = 0;      //24 bit
	u32 ADRS_REG = 0;  //13 bit

	for (int step = 0; step < /*128*/LastStep; ++step)
	{
		u16 *const IPtr = MPRO + (step * 4);

		//if (!IPtr[0] && !IPtr[1] && !IPtr[2] && !IPtr[3])
			//break;

		u32 const TRA   = (IPtr[0] >>  8) & 0x7F;
		u32 const TWT   = (IPtr[0] >>  7) & 0x01;
		u32 const TWA   = (IPtr[0] >>  0) & 0x7F;

		u32 const XSEL  = (IPtr[1] >> 15) & 0x01;
		u32 const YSEL  = (IPtr[1] >> 13) & 0x03;
		u32 const IRA   = (IPtr[1] >>  6) & 0x3F;
		u32 const IWT   = (IPtr[1] >>  5) & 0x01;
		u32 const IWA   = (IPtr[1] >>  0) & 0x1F;

		u32 const TABLE = (IPtr[2] >> 15) & 0x01;
		u32 const MWT   = (IPtr[2] >> 14) & 0x01;
		u32 const MRD   = (IPtr[2] >> 13) & 0x01;
		u32 const EWT   = (IPtr[2] >> 12) & 0x01;
		u32 const EWA   = (IPtr[2] >>  8) & 0x0F;
		u32 const ADRL  = (IPtr[2] >>  7) & 0x01;
		u32 const FRCL  = (IPtr[2] >>  6) & 0x01;
		u32 const SHIFT = (IPtr[2] >>  4) & 0x03;
		u32 const YRL   = (IPtr[2] >>  3) & 0x01;
		u32 const NEGB  = (IPtr[2] >>  2) & 0x01;
		u32 const ZERO  = (IPtr[2] >>  1) & 0x01;
		u32 const BSEL  = (IPtr[2] >>  0) & 0x01;

		u32 const NOFL  = (IPtr[3] >> 15) & 0x01;  //????
		u32 const COEF  = (IPtr[3] >>  9) & 0x3f;

		u32 const MASA  = (IPtr[3] >>  2) & 0x1f;  //???
		u32 const ADREB = (IPtr[3] >>  1) & 0x01;
		u32 const NXADR = (IPtr[3] >>  0) & 0x01;

		//operations are done at 24 bit precision
#if 0
		if (MASA)
			int a=1;
		if (NOFL)
			int a=1;

		//int dump=0;

		if (f)
		{
#define DUMP(v) fprintf(f, " " #v ": %04X", v);
			fprintf(f, "%d: ", step);
			DUMP(ACC);
			DUMP(SHIFTED);
			DUMP(X);
			DUMP(Y);
			DUMP(B);
			DUMP(INPUTS);
			DUMP(MEMVAL);
			DUMP(FRC_REG);
			DUMP(Y_REG);
			DUMP(ADDR);
			DUMP(ADRS_REG);
			fprintf(f, "\n");
#undef DUMP
		}
#endif

		//INPUTS RW
		// colmns97 hits this
		//assert(IRA < 0x32);
		s32 INPUTS; // 24-bit
		if (IRA <= 0x1f)
			INPUTS = MEMS[IRA];
		else if (IRA <= 0x2F)
			INPUTS = MIXS[IRA - 0x20] << 4;  //MIXS is 20 bit
		else if (IRA <= 0x31)
			INPUTS = EXTS[IRA - 0x30] << 8;  //EXTS is 16 bit
		else
			return;

		INPUTS <<= 8;
		INPUTS >>= 8;
		//if(INPUTS & 0x00800000)
			//INPUTS |= 0xFF000000;

		if (IWT)
		{
			MEMS[IWA] = MEMVAL;  // MEMVAL was selected in previous MRD
			if (IRA == IWA)
				INPUTS = MEMVAL;
		}

		//Operand sel
		s32 B; // 26-bit
		if (!ZERO)
		{
			if (BSEL)
				B = ACC;
			else
			{
				B = TEMP[(TRA + DEC) & 0x7F];
				B <<= 8;
				B >>= 8;
				//if (B & 0x00800000)
					//B |= 0xFF000000;  //Sign extend
			}
			if (NEGB)
				B = 0 - B;
		}
		else
			B = 0;

		s32 X; // 24-bit
		if (XSEL)
			X = INPUTS;
		else
		{
			X = TEMP[(TRA + DEC) & 0x7F];
			X <<= 8;
			X >>= 8;
			//if (X & 0x00800000)
				//X |= 0xFF000000;
		}

		s32 Y = 0;  //13 bit
		if (YSEL == 0)
			Y = FRC_REG;
		else if (YSEL == 1)
			Y = this->COEF[COEF] >> 3;   //COEF is 16 bits
		else if (YSEL == 2)
			Y = (Y_REG >> 11) & 0x1FFF;
		else if (YSEL == 3)
			Y = (Y_REG >> 4) & 0x0FFF;

		if (YRL)
			Y_REG = INPUTS;

		//Shifter
		s32 SHIFTED = 0;    //24 bit
		if (SHIFT == 0)
			SHIFTED = std::max<s32>(std::min<s32>(ACC, 0x007FFFFF), -0x00800000);
		else if (SHIFT == 1)
			SHIFTED = std::max<s32>(std::min<s32>(ACC * 2, 0x007FFFFF), -0x00800000);
		else if (SHIFT == 2)
		{
			SHIFTED = ACC * 2;
			SHIFTED <<= 8;
			SHIFTED >>= 8;
			//SHIFTED &= 0x00FFFFFF;
			//if (SHIFTED & 0x00800000)
				//SHIFTED |= 0xFF000000;
		}
		else if (SHIFT == 3)
		{
			SHIFTED = ACC;
			SHIFTED <<= 8;
			SHIFTED >>= 8;
			//SHIFTED &= 0x00FFFFFF;
			//if (SHIFTED & 0x00800000)
				//SHIFTED |= 0xFF000000;
		}

		//ACCUM
		Y <<= 19;
		Y >>= 19;
		//if (Y & 0x1000)
			//Y |= 0xFFFFF000;

		int64_t const v = (int64_t(X) * int64_t(Y)) >> 12;
		ACC = int(v + B);

		if (TWT)
			TEMP[(TWA + DEC) & 0x7F] = SHIFTED;

		if (FRCL)
		{
			if (SHIFT == 3)
				FRC_REG = SHIFTED & 0x0FFF;
			else
				FRC_REG = (SHIFTED >> 11) & 0x1FFF;
		}

		if (MRD || MWT)
		//if (0)
		{
			u32 ADDR = MADRS[MASA];
			if (!TABLE)
				ADDR += DEC;
			if (ADREB)
				ADDR += ADRS_REG & 0x0FFF;
			if (NXADR)
				ADDR++;
			if (!TABLE)
				ADDR &= RBL - 1;
			else
				ADDR &= 0xFFFF;
			ADDR += RBP << 12;
			ADDR <<= 1;
			if (MRD && (step & 1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if (NOFL)
					MEMVAL = space->read_word(ADDR) << 8;
				else
					MEMVAL = UNPACK(space->read_word(ADDR));
			}
			if (MWT && (step & 1))
			{
				if (NOFL)
					space->write_word(ADDR, SHIFTED >> 8);
				else
					space->write_word(ADDR, PACK(SHIFTED));
			}
		}

		if (ADRL)
		{
			if (SHIFT == 3)
				ADRS_REG = (SHIFTED >> 12) & 0xFFF;
			else
				ADRS_REG = INPUTS >> 16;
		}

		if (EWT)
			EFREG[EWA] += SHIFTED >> 8;
	}
	--DEC;
	std::fill(std::begin(MIXS), std::end(MIXS), 0);
	//if (f)
		//fclose(f);
}

void SCSPDSP::SetSample(s32 sample, int SEL, int MXL)
{
	//MIXS[SEL] += sample << (MXL + 1)/*7*/;
	MIXS[SEL] += sample;
	//if (MXL)
		//int a = 1;
}

void SCSPDSP::Start()
{
	Stopped = false;
	int i;
	for (i = 127; i >= 0; --i)
	{
		u16 const *const IPtr = MPRO + (i * 4);
		if (IPtr[0] || IPtr[1] || IPtr[2] || IPtr[3])
			break;
	}
	LastStep = i + 1;
}
