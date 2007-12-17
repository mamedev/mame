#ifndef TLCS90_INCLUDE
#define TLCS90_INCLUDE

#include "cpuintrf.h"

void tmp90840_get_info(UINT32, cpuinfo*);
void tmp90841_get_info(UINT32, cpuinfo*);
void tmp91640_get_info(UINT32, cpuinfo*);
void tmp91641_get_info(UINT32, cpuinfo*);

#ifdef MAME_DEBUG
unsigned t90_dasm(char *buffer, UINT32 oldpc, const UINT8 *oprom, const UINT8 *opram);
#endif

#define T90_IOBASE	0xffc0

typedef enum	{
	T90_P0=T90_IOBASE,	T90_P1,		T90_P01CR_IRFL,	T90_IRFH,	T90_P2,		T90_P2CR,	T90_P3,		T90_P3CR,
	T90_P4,				T90_P4CR,	T90_P5,			T90_SMMOD,	T90_P6,		T90_P7,		T90_P67CR,	T90_SMCR,
	T90_P8,				T90_P8CR,	T90_WDMOD,		T90_WDCR,	T90_TREG0,	T90_TREG1,	T90_TREG2,	T90_TREG3,
	T90_TCLK,			T90_TFFCR,	T90_TMOD,		T90_TRUN,	T90_CAP1L,	T90_CAP1H,	T90_CAP2L,	T90_CAL2H,
	T90_TREG4L,			T90_TREG4H,	T90_TREG5L,		T90_TREG5H,	T90_T4MOD,	T90_T4FFCR,	T90_INTEL,	T90_INTEH,
	T90_DMAEH,			T90_SCMOD,	T90_SCCR,		T90_SCBUF,	T90_BX,		T90_BY,		T90_ADREG,	T90_ADMOD
}	e_ir;

#endif
