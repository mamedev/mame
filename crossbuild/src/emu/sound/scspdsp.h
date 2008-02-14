#ifndef SCSPDSP_H
#define SCSPDSP_H

//the DSP Context
struct _SCSPDSP
{
//Config
	UINT16 *SCSPRAM;
	UINT32 SCSPRAM_LENGTH;
	UINT32 RBP;	//Ring buf pointer
	UINT32 RBL;	//Delay ram (Ring buffer) size in words

//context

	INT16 COEF[64];		//16 bit signed
	UINT16 MADRS[32];	//offsets (in words), 16 bit
	UINT16 MPRO[128*4];	//128 steps 64 bit
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

void SCSPDSP_Init(struct _SCSPDSP *DSP);
void SCSPDSP_SetSample(struct _SCSPDSP *DSP, INT32 sample, INT32 SEL, INT32 MXL);
void SCSPDSP_Step(struct _SCSPDSP *DSP);
void SCSPDSP_Start(struct _SCSPDSP *DSP);
#endif
