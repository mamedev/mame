#ifndef AICADSP_H
#define AICADSP_H

//the DSP Context
struct _AICADSP
{
//Config
	UINT16 *AICARAM;
	UINT32 AICARAM_LENGTH;
	UINT32 RBP;	//Ring buf pointer
	UINT32 RBL;	//Delay ram (Ring buffer) size in words

//context

	INT16 COEF[128*2];		//16 bit signed
	UINT16 MADRS[64*2];	//offsets (in words), 16 bit
	UINT16 MPRO[128*4*2*2];	//128 steps 64 bit
	INT32 TEMP[128];	//TEMP regs,24 bit signed
	INT32 MEMS[32];	//MEMS regs,24 bit signed
	UINT32 DEC;

//input
	INT32 MIXS[16];	//MIXS, 24 bit signed
	INT16 EXTS[2];	//External inputs (CDDA)    16 bit signed

//output
	INT16 EFREG[16];	//EFREG, 16 bit signed

	int Stopped;
	int LastStep;
};

void AICADSP_Init(struct _AICADSP *DSP);
void AICADSP_SetSample(struct _AICADSP *DSP, INT32 sample, INT32 SEL, INT32 MXL);
void AICADSP_Step(struct _AICADSP *DSP);
void AICADSP_Start(struct _AICADSP *DSP);
#endif
