// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
// thanks-to: kingshriek
#include "emu.h"
#include "aicadsp.h"

#include <algorithm>

static u16 PACK(s32 val)
{
	const int sign = (val >> 23) & 0x1;
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

	return (u16)val;
}

static s32 UNPACK(u16 val)
{
	const int sign = (val >> 15) & 0x1;
	int exponent = (val >> 11) & 0xF;
	const int mantissa = val & 0x7FF;
	u32 uval = mantissa << 11;
	if (exponent > 11)
	{
		exponent = 11;
		uval |= sign << 22;
	}
	else
		uval |= (sign ^ 1) << 22;
	uval |= sign << 23;
	uval <<= 8;
	uval >>= 8;
	uval >>= exponent;

	return uval;
}

void AICADSP::init()
{
	memset(this,0,sizeof(*this));
	RBL = (8 * 1024); // Initial RBL is 0
	Stopped = true;
}

void AICADSP::step()
{
	s32 ACC=0;    //26 bit
	s32 MEMVAL=0;
	s32 FRC_REG=0;    //13 bit
	s32 Y_REG=0;      //24 bit
	u32 ADRS_REG=0;  //13 bit

	if (Stopped)
		return;

	std::fill(std::begin(EFREG), std::end(EFREG), 0);
#if 0
	int dump=0;
	FILE *f=nullptr;
	if (dump)
		f=fopen("dsp.txt","wt");
#endif
	for (int step = 0; step < /*128*/LastStep; ++step)
	{
		u16 *IPtr=MPRO+step*8;

//      if (IPtr[0]==0 && IPtr[1]==0 && IPtr[2]==0 && IPtr[3]==0)
//          break;

		const u32 TRA   = (IPtr[0] >>  9) & 0x7F;
		const u32 TWT   = (IPtr[0] >>  8) & 0x01;
		const u32 TWA   = (IPtr[0] >>  1) & 0x7F;

		const u32 XSEL  = (IPtr[2] >> 15) & 0x01;
		const u32 YSEL  = (IPtr[2] >> 13) & 0x03;
		const u32 IRA   = (IPtr[2] >>  7) & 0x3F;
		const u32 IWT   = (IPtr[2] >>  6) & 0x01;
		const u32 IWA   = (IPtr[2] >>  1) & 0x1F;

		const u32 TABLE = (IPtr[4] >> 15) & 0x01;
		const u32 MWT   = (IPtr[4] >> 14) & 0x01;
		const u32 MRD   = (IPtr[4] >> 13) & 0x01;
		const u32 EWT   = (IPtr[4] >> 12) & 0x01;
		const u32 EWA   = (IPtr[4] >>  8) & 0x0F;
		const u32 ADRL  = (IPtr[4] >>  7) & 0x01;
		const u32 FRCL  = (IPtr[4] >>  6) & 0x01;
		const u32 SHIFT = (IPtr[4] >>  4) & 0x03;
		const u32 YRL   = (IPtr[4] >>  3) & 0x01;
		const u32 NEGB  = (IPtr[4] >>  2) & 0x01;
		const u32 ZERO  = (IPtr[4] >>  1) & 0x01;
		const u32 BSEL  = (IPtr[4] >>  0) & 0x01;

		const u32 NOFL  = (IPtr[6] >> 15) & 1;        //????
		const u32 COEF  = step;

		const u32 MASA  = (IPtr[6] >>  9) & 0x1f;  //???
		const u32 ADREB = (IPtr[6] >>  8) & 0x1;
		const u32 NXADR = (IPtr[6] >>  7) & 0x1;

		//operations are done at 24 bit precision
#if 0
		if (MASA)
			int a=1;
		if (NOFL)
			int a=1;

//      int dump=0;

		if (f)
		{
#define DUMP(v) fprintf(f," " #v ": %04X",v);

			fprintf(f,"%d: ",step);
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
			fprintf(f,"\n");
		}
#endif
		//INPUTS RW
		assert(IRA<0x32);
		s32 INPUTS=0; //24 bit
		if (IRA <= 0x1f)
			INPUTS = MEMS[IRA];
		else if (IRA <= 0x2F)
			INPUTS = MIXS[IRA - 0x20] << 4;  //MIXS is 20 bit
		else if (IRA <= 0x31)
			INPUTS = EXTS[IRA - 0x30] << 8;  //EXTS is 16 bit

		INPUTS <<= 8;
		INPUTS >>= 8;
		//if (INPUTS & 0x00800000)
		//  INPUTS |= 0xFF000000;

		if (IWT)
		{
			MEMS[IWA] = MEMVAL;  //MEMVAL was selected in previous MRD
			if (IRA == IWA)
				INPUTS = MEMVAL;
		}

		//Operand sel
		//B
		s32 B;  //26 bit
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
				//  B |= 0xFF000000;  //Sign extend
			}
			if (NEGB)
				B = 0 - B;
		}
		else
			B = 0;

		//X
		s32 X;  //24 bit
		if (XSEL)
			X = INPUTS;
		else
		{
			X = TEMP[(TRA + DEC) & 0x7F];
			X <<= 8;
			X >>= 8;
			//if (X & 0x00800000)
			//  X |= 0xFF000000;
		}

		//Y
		s32 Y = 0;  //13 bit
		if (YSEL == 0)
			Y = FRC_REG;
		else if (YSEL == 1)
			Y = this->COEF[COEF << 1] >> 3;    //COEF is 16 bits
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
			//  SHIFTED |= 0xFF000000;
		}
		else if (SHIFT == 3)
		{
			SHIFTED = ACC;
			SHIFTED <<= 8;
			SHIFTED >>= 8;
			//SHIFTED &= 0x00FFFFFF;
			//if (SHIFTED & 0x00800000)
			//  SHIFTED |= 0xFF000000;
		}

		//ACCUM
		Y <<= 19;
		Y >>= 19;
		//if (Y & 0x1000)
		//  Y |= 0xFFFFF000;

		const s64 v = (((s64)X * (s64)Y) >> 12);
		ACC = (int)v + B;

		if (TWT)
			TEMP[(TWA + DEC) & 0x7F] = SHIFTED;

		if (FRCL)
		{
			if (SHIFT == 3)
				FRC_REG = SHIFTED & 0x0FFF;
			else
				FRC_REG = (SHIFTED >> 11) & 0x1FFF;
		}

		u32 ADDR;
		if (MRD || MWT)
		//if (0)
		{
			ADDR = MADRS[MASA << 1];
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
			//ADDR <<= 1;
			//ADDR += RBP << 13;
			//MEMVAL = space->read_word(ADDR >> 1);
			ADDR += RBP << 10;
			if (MRD && (step & 1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if (NOFL)
					MEMVAL = cache->read_word(ADDR) << 8;
				else
					MEMVAL = UNPACK(cache->read_word(ADDR));
			}
			if (MWT && (step&1))
			{
				if (NOFL)
					space->write_word(ADDR, SHIFTED>>8);
				else
					space->write_word(ADDR, PACK(SHIFTED));
			}
		}

		if (ADRL)
		{
			if (SHIFT == 3)
				ADRS_REG = (SHIFTED >> 12) & 0xFFF;
			else
				ADRS_REG = (INPUTS >> 16);
		}

		if (EWT)
			EFREG[EWA] += SHIFTED >> 8;

	}
	--DEC;
	std::fill(std::begin(MIXS), std::end(MIXS), 0);
//  if (f)
//      fclose(f);
}

void AICADSP::setsample(s32 sample, u8 SEL, s32 MXL)
{
	//MIXS[SEL] += sample << (MXL + 1)/*7*/;
	MIXS[SEL] += sample;
//  if (MXL)
//      int a=1;
}

void AICADSP::start()
{
	int i;
	Stopped = false;
	for (i = 127; i >= 0; --i)
	{
		u16 *IPtr = MPRO + i * 8;

		if (IPtr[0] != 0 || IPtr[2] != 0 || IPtr[4] != 0 || IPtr[6] != 0)
			break;
	}
	LastStep = i + 1;
}
