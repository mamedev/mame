// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   8008dasm.c
 *
 *   Intel 8008 CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

static const char reg[] = { 'a', 'b', 'c', 'd', 'e', 'h', 'l', 'm' };
static const char flag_names[] = { 'c', 'z', 's', 'p' };

CPU_DISASSEMBLE( i8008 )
{
	UINT32 flags = 0;
	unsigned PC = pc;
	UINT8 op = OP(pc++);
	switch (op >> 6)
	{
		case 0x03:  // starting with 11
					if (op==0xff) {
						sprintf (buffer,"hlt");
					} else {
						sprintf (buffer,"l%c%c",reg[(op >> 3) & 7],reg[op & 7]);
					}
					break;
		case 0x00:  // starting with 00
					switch(op & 7) {
						case 0 :    if(((op >> 3) & 7)==0) {
										sprintf (buffer,"hlt");
									} else {
										if(((op >> 3) & 7)==7) {
											sprintf (buffer,"illegal");
										} else {
											sprintf (buffer,"in%c",reg[(op >> 3) & 7]);
										}
									}
									break;
						case 1 :    if(((op >> 3) & 7)==0) {
										sprintf (buffer,"hlt");
									} else {
										if(((op >> 3) & 7)==7) {
											sprintf (buffer,"illegal");
										} else {
											sprintf (buffer,"dc%c",reg[(op >> 3) & 7]);
										}
									}
									break;
						case 2 :    {
										switch((op >> 3) & 7) {
											case 0 :    sprintf (buffer,"rlc"); break;
											case 1 :    sprintf (buffer,"rrc"); break;
											case 2 :    sprintf (buffer,"ral"); break;
											case 3 :    sprintf (buffer,"rar"); break;
											default :   sprintf (buffer,"illegal"); break;
										}
									}
									break;
						case 3 :    sprintf (buffer,"r%c%c",(BIT(op,5) ? 't' : 'f'),flag_names[(op>>3)&3]); break;
						case 4 :    {
										switch((op >> 3) & 7) {
											case 0 :    sprintf (buffer,"adi %02x",ARG(pc)); pc++; break;
											case 1 :    sprintf (buffer,"aci %02x",ARG(pc)); pc++; break;
											case 2 :    sprintf (buffer,"sui %02x",ARG(pc)); pc++; break;
											case 3 :    sprintf (buffer,"sbi %02x",ARG(pc)); pc++; break;
											case 4 :    sprintf (buffer,"ndi %02x",ARG(pc)); pc++; break;
											case 5 :    sprintf (buffer,"xri %02x",ARG(pc)); pc++; break;
											case 6 :    sprintf (buffer,"ori %02x",ARG(pc)); pc++; break;
											case 7 :    sprintf (buffer,"cpi %02x",ARG(pc)); pc++; break;
										}
									}
									break;
						case 5 :    sprintf (buffer,"rst %02x",(op>>3) & 7); break;
						case 6 :    sprintf (buffer,"l%ci %02x",reg[(op >> 3) & 7],ARG(pc)); pc++; break;
						case 7 :    sprintf (buffer,"ret"); break;
					}
					break;
		case 0x01:  // starting with 01
					switch(op & 7) {
						case 0 :    sprintf (buffer,"j%c%c %02x%02x",(BIT(op,5)? 't' : 'f'),flag_names[(op>>3)&3], ARG(pc+1) & 0x3f,ARG(pc)); pc+=2; break;
						case 2 :    sprintf (buffer,"c%c%c %02x%02x",(BIT(op,5)? 't' : 'f'),flag_names[(op>>3)&3], ARG(pc+1) & 0x3f,ARG(pc)); pc+=2; break;
						case 4 :    sprintf (buffer,"jmp %02x%02x",ARG(pc+1) & 0x3f,ARG(pc)); pc+=2; break;
						case 6 :    sprintf (buffer,"cal %02x%02x",ARG(pc+1) & 0x3f,ARG(pc)); pc+=2; break;
						case 1 :
						case 3 :
						case 5 :
						case 7 :    if (((op>>4)&3)==0) {
										sprintf (buffer,"inp %02x",(op >> 1) & 0x07);
									} else {
										sprintf (buffer,"out %02x",(op >> 1) & 0x1f);
									}
									break;
					}
					break;
		case 0x02:  // starting with 10
					switch((op >> 3) & 7) {
						case 0 :    sprintf (buffer,"ad%c",reg[op & 7]); break;
						case 1 :    sprintf (buffer,"ac%c",reg[op & 7]); break;
						case 2 :    sprintf (buffer,"su%c",reg[op & 7]); break;
						case 3 :    sprintf (buffer,"sb%c",reg[op & 7]); break;
						case 4 :    sprintf (buffer,"nd%c",reg[op & 7]); break;
						case 5 :    sprintf (buffer,"xr%c",reg[op & 7]); break;
						case 6 :    sprintf (buffer,"or%c",reg[op & 7]); break;
						case 7 :    sprintf (buffer,"cp%c",reg[op & 7]); break;
					}
					break;
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}
