// license:BSD-3-Clause
// copyright-holders:ElSemi, Deunan Knute, R. Belmont
// thanks-to: kingshriek
#include "emu.h"
#include "aicadsp.h"

static uint16_t PACK(int32_t val)
{
	uint32_t temp;
	int sign,exponent,k;

	sign = (val >> 23) & 0x1;
	temp = (val ^ (val << 1)) & 0xFFFFFF;
	exponent = 0;
	for (k=0; k<12; k++)
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

	return (uint16_t)val;
}

static int32_t UNPACK(uint16_t val)
{
	int sign,exponent,mantissa;
	int32_t uval;

	sign = (val >> 15) & 0x1;
	exponent = (val >> 11) & 0xF;
	mantissa = val & 0x7FF;
	uval = mantissa << 11;
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
	RBL=0x8000;
	Stopped=1;
}

void AICADSP::step()
{
	int32_t ACC=0;    //26 bit
	int32_t SHIFTED=0;    //24 bit
	int32_t X;  //24 bit
	int32_t Y=0;  //13 bit
	int32_t B;  //26 bit
	int32_t INPUTS=0; //24 bit
	int32_t MEMVAL=0;
	int32_t FRC_REG=0;    //13 bit
	int32_t Y_REG=0;      //24 bit
	uint32_t ADDR;
	uint32_t ADRS_REG=0;  //13 bit
	int step;

	if(Stopped)
		return;

	memset(EFREG,0,2*16);
#if 0
	int dump=0;
	FILE *f=nullptr;
	if(dump)
		f=fopen("dsp.txt","wt");
#endif
	for(step=0;step</*128*/LastStep;++step)
	{
		uint16_t *IPtr=MPRO+step*8;

//      if(IPtr[0]==0 && IPtr[1]==0 && IPtr[2]==0 && IPtr[3]==0)
//          break;

		uint32_t TRA=(IPtr[0]>>9)&0x7F;
		uint32_t TWT=(IPtr[0]>>8)&0x01;
		uint32_t TWA=(IPtr[0]>>1)&0x7F;

		uint32_t XSEL=(IPtr[2]>>15)&0x01;
		uint32_t YSEL=(IPtr[2]>>13)&0x03;
		uint32_t IRA=(IPtr[2]>>7)&0x3F;
		uint32_t IWT=(IPtr[2]>>6)&0x01;
		uint32_t IWA=(IPtr[2]>>1)&0x1F;

		uint32_t TABLE=(IPtr[4]>>15)&0x01;
		uint32_t MWT=(IPtr[4]>>14)&0x01;
		uint32_t MRD=(IPtr[4]>>13)&0x01;
		uint32_t EWT=(IPtr[4]>>12)&0x01;
		uint32_t EWA=(IPtr[4]>>8)&0x0F;
		uint32_t ADRL=(IPtr[4]>>7)&0x01;
		uint32_t FRCL=(IPtr[4]>>6)&0x01;
		uint32_t SHIFT=(IPtr[4]>>4)&0x03;
		uint32_t YRL=(IPtr[4]>>3)&0x01;
		uint32_t NEGB=(IPtr[4]>>2)&0x01;
		uint32_t ZERO=(IPtr[4]>>1)&0x01;
		uint32_t BSEL=(IPtr[4]>>0)&0x01;

		uint32_t NOFL=(IPtr[6]>>15)&1;        //????
		uint32_t COEF=step;

		uint32_t MASA=(IPtr[6]>>9)&0x1f;  //???
		uint32_t ADREB=(IPtr[6]>>8)&0x1;
		uint32_t NXADR=(IPtr[6]>>7)&0x1;

		int64_t v;

		//operations are done at 24 bit precision
#if 0
		if(MASA)
			int a=1;
		if(NOFL)
			int a=1;

//      int dump=0;

		if(f)
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
		if(IRA<=0x1f)
			INPUTS=MEMS[IRA];
		else if(IRA<=0x2F)
			INPUTS=MIXS[IRA-0x20]<<4;  //MIXS is 20 bit
		else if(IRA<=0x31)
			INPUTS=0;

		INPUTS<<=8;
		INPUTS>>=8;
		//if(INPUTS&0x00800000)
		//  INPUTS|=0xFF000000;

		if(IWT)
		{
			MEMS[IWA]=MEMVAL;  //MEMVAL was selected in previous MRD
			if(IRA==IWA)
				INPUTS=MEMVAL;
		}

		//Operand sel
		//B
		if(!ZERO)
		{
			if(BSEL)
				B=ACC;
			else
			{
				B=TEMP[(TRA+DEC)&0x7F];
				B<<=8;
				B>>=8;
				//if(B&0x00800000)
				//  B|=0xFF000000;  //Sign extend
			}
			if(NEGB)
				B=0-B;
		}
		else
			B=0;

		//X
		if(XSEL)
			X=INPUTS;
		else
		{
			X=TEMP[(TRA+DEC)&0x7F];
			X<<=8;
			X>>=8;
			//if(X&0x00800000)
			//  X|=0xFF000000;
		}

		//Y
		if(YSEL==0)
			Y=FRC_REG;
		else if(YSEL==1)
			Y=this->COEF[COEF<<1]>>3;    //COEF is 16 bits
		else if(YSEL==2)
			Y=(Y_REG>>11)&0x1FFF;
		else if(YSEL==3)
			Y=(Y_REG>>4)&0x0FFF;

		if(YRL)
			Y_REG=INPUTS;

		//Shifter
		if(SHIFT==0)
		{
			SHIFTED=ACC;
			if(SHIFTED>0x007FFFFF)
				SHIFTED=0x007FFFFF;
			if(SHIFTED<(-0x00800000))
				SHIFTED=-0x00800000;
		}
		else if(SHIFT==1)
		{
			SHIFTED=ACC*2;
			if(SHIFTED>0x007FFFFF)
				SHIFTED=0x007FFFFF;
			if(SHIFTED<(-0x00800000))
				SHIFTED=-0x00800000;
		}
		else if(SHIFT==2)
		{
			SHIFTED=ACC*2;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}
		else if(SHIFT==3)
		{
			SHIFTED=ACC;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}

		//ACCUM
		Y<<=19;
		Y>>=19;
		//if(Y&0x1000)
		//  Y|=0xFFFFF000;

		v=(((int64_t) X*(int64_t) Y)>>12);
		ACC=(int) v+B;

		if(TWT)
			TEMP[(TWA+DEC)&0x7F]=SHIFTED;

		if(FRCL)
		{
			if(SHIFT==3)
				FRC_REG=SHIFTED&0x0FFF;
			else
				FRC_REG=(SHIFTED>>11)&0x1FFF;
		}

		if(MRD || MWT)
		//if(0)
		{
			ADDR=MADRS[MASA<<1];
			if(!TABLE)
				ADDR+=DEC;
			if(ADREB)
				ADDR+=ADRS_REG&0x0FFF;
			if(NXADR)
				ADDR++;
			if(!TABLE)
				ADDR&=RBL-1;
			else
				ADDR&=0xFFFF;
			//ADDR<<=1;
			//ADDR+=RBP<<13;
			//MEMVAL=AICARAM[ADDR>>1];
			ADDR+=RBP<<10;
			if(MRD && (step&1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if(NOFL)
					MEMVAL=AICARAM[ADDR]<<8;
				else
					MEMVAL=UNPACK(AICARAM[ADDR]);
			}
			if(MWT && (step&1))
			{
				if(NOFL)
					AICARAM[ADDR]=SHIFTED>>8;
				else
					AICARAM[ADDR]=PACK(SHIFTED);
			}
		}

		if(ADRL)
		{
			if(SHIFT==3)
				ADRS_REG=(SHIFTED>>12)&0xFFF;
			else
				ADRS_REG=(INPUTS>>16);
		}

		if(EWT)
			EFREG[EWA]+=SHIFTED>>8;

	}
	--DEC;
	memset(MIXS,0,4*16);
//  if(f)
//      fclose(f);
}

void AICADSP::setsample(int32_t sample,int SEL,int MXL)
{
	//MIXS[SEL]+=sample<<(MXL+1)/*7*/;
	MIXS[SEL]+=sample;
//  if(MXL)
//      int a=1;
}

void AICADSP::start()
{
	int i;
	Stopped=0;
	for(i=127;i>=0;--i)
	{
		uint16_t *IPtr=MPRO+i*8;

		if(IPtr[0]!=0 || IPtr[2]!=0 || IPtr[4]!=0 || IPtr[6]!=0)
			break;
	}
	LastStep=i+1;
}
