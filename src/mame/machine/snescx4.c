// license:GPL-2.0+
// copyright-holders:Fabio Priuli, byuu, Nach
/***************************************************************************

    snescx4.c

    File to handle emulation of the SNES "CX4" add-on chip.

    Code based on original work by zsKnight, anomie and Nach.
    This implementation is based on C++ "cx4*.cpp" by byuu
    (up to date with source v 0.49).

***************************************************************************/

#include "emu.h"
#include "machine/snescx4.h"

static CX4 cx4;

static UINT16 CX4_readw(UINT16 addr);
static UINT32 CX4_readl(UINT16 addr);

static void CX4_writew(running_machine &machine, UINT16 addr, UINT16 data);
//static void CX4_writel(running_machine &machine, UINT16 addr, UINT32 data);

static void CX4_C4DrawLine(INT32 X1, INT32 Y1, INT16 Z1, INT32 X2, INT32 Y2, INT16 Z2, UINT8 Color);

#include "machine/cx4data.inc"
#include "machine/cx4fn.inc"

static UINT32 CX4_ldr(UINT8 r)
{
	UINT16 addr = 0x0080 + (r * 3);
	return (cx4.reg[addr + 0] <<  0)
			| (cx4.reg[addr + 1] <<  8)
			| (cx4.reg[addr + 2] << 16);
}

static void CX4_str(UINT8 r, UINT32 data)
{
	UINT16 addr = 0x0080 + (r * 3);
	cx4.reg[addr + 0] = (data >>  0);
	cx4.reg[addr + 1] = (data >>  8);
	cx4.reg[addr + 2] = (data >> 16);
}

static void CX4_mul(UINT32 x, UINT32 y, UINT32 *rl, UINT32 *rh)
{
	INT64 rx = x & 0xffffff;
	INT64 ry = y & 0xffffff;
	if(rx & 0x800000)rx |= ~0x7fffff;
	if(ry & 0x800000)ry |= ~0x7fffff;

	rx *= ry;

	*rl = (rx)       & 0xffffff;
	*rh = (rx >> 24) & 0xffffff;
}

static UINT32 CX4_sin(UINT32 rx)
{
	cx4.r0 = rx & 0x1ff;
	if(cx4.r0 & 0x100)
	{
		cx4.r0 ^= 0x1ff;
	}
	if(cx4.r0 & 0x080)
	{
		cx4.r0 ^= 0x0ff;
	}
	if(rx & 0x100)
	{
		return CX4_sin_table[cx4.r0 + 0x80];
	}
	else
	{
		return CX4_sin_table[cx4.r0];
	}
}

static UINT32 CX4_cos(UINT32 rx)
{
	return CX4_sin(rx + 0x080);
}

static void CX4_immediate_reg(UINT32 start)
{
	UINT32 i = 0;
	cx4.r0 = CX4_ldr(0);
	for(i = start; i < 48; i++)
	{
		if((cx4.r0 & 0x0fff) < 0x0c00)
		{
			cx4.ram[cx4.r0 & 0x0fff] = CX4_immediate_data[i];
		}
		cx4.r0++;
	}
	CX4_str(0, cx4.r0);
}

static void CX4_transfer_data(running_machine &machine)
{
	UINT32 src;
	UINT16 dest, count;
	UINT32 i;

	src   = (cx4.reg[0x40]) | (cx4.reg[0x41] << 8) | (cx4.reg[0x42] << 16);
	count = (cx4.reg[0x43]) | (cx4.reg[0x44] << 8);
	dest  = (cx4.reg[0x45]) | (cx4.reg[0x46] << 8);

	address_space &space = machine.device<cpu_device>("maincpu")->space(AS_PROGRAM);
	for(i=0;i<count;i++)
	{
		CX4_write(machine, dest++, space.read_byte(src++));
	}
}

#include "machine/cx4oam.inc"
#include "machine/cx4ops.inc"

void CX4_write(running_machine &machine, UINT32 addr, UINT8 data)
{
	addr &= 0x1fff;

	if(addr < 0x0c00)
	{
		//ram
		cx4.ram[addr] = data;
		return;
	}

	if(addr < 0x1f00)
	{
		//unmapped
		return;
	}

	//command register
	cx4.reg[addr & 0xff] = data;

	if(addr == 0x1f47)
	{
		//memory transfer
		CX4_transfer_data(machine);
		return;
	}

	if(addr == 0x1f4f)
	{
		//c4 command
		if(cx4.reg[0x4d] == 0x0e && !(data & 0xc3))
		{
			//c4 test command
			cx4.reg[0x80] = data >> 2;
			return;
		}

		switch(data)
		{
			case 0x00: CX4_op00(machine); break;
			case 0x01: CX4_op01(machine); break;
			case 0x05: CX4_op05(machine); break;
			case 0x0d: CX4_op0d(machine); break;
			case 0x10: CX4_op10(); break;
			case 0x13: CX4_op13(); break;
			case 0x15: CX4_op15(machine); break;
			case 0x1f: CX4_op1f(machine); break;
			case 0x22: CX4_op22(); break;
			case 0x25: CX4_op25(); break;
			case 0x2d: CX4_op2d(machine); break;
			case 0x40: CX4_op40(); break;
			case 0x54: CX4_op54(); break;
			case 0x5c: CX4_op5c(); break;
			case 0x5e: CX4_op5e(); break;
			case 0x60: CX4_op60(); break;
			case 0x62: CX4_op62(); break;
			case 0x64: CX4_op64(); break;
			case 0x66: CX4_op66(); break;
			case 0x68: CX4_op68(); break;
			case 0x6a: CX4_op6a(); break;
			case 0x6c: CX4_op6c(); break;
			case 0x6e: CX4_op6e(); break;
			case 0x70: CX4_op70(); break;
			case 0x72: CX4_op72(); break;
			case 0x74: CX4_op74(); break;
			case 0x76: CX4_op76(); break;
			case 0x78: CX4_op78(); break;
			case 0x7a: CX4_op7a(); break;
			case 0x7c: CX4_op7c(); break;
			case 0x89: CX4_op89(); break;
		}
	}
}

#ifdef UNUSED_FUNCTION
void CX4_writeb(running_machine &machine, UINT16 addr, UINT8 data)
{
	CX4_write(machine, addr,     data);
}
#endif

static void CX4_writew(running_machine &machine, UINT16 addr, UINT16 data)
{
	CX4_write(machine, addr + 0, data >> 0);
	CX4_write(machine, addr + 1, data >> 8);
}

#ifdef UNUSED_FUNCTION
void CX4_writel(running_machine &machine, UINT16 addr, UINT32 data)
{
	CX4_write(machine, addr + 0, data >>  0);
	CX4_write(machine, addr + 1, data >>  8);
	CX4_write(machine, addr + 2, data >> 16);
}
#endif

UINT8 CX4_read(UINT32 addr)
{
	addr &= 0x1fff;

	if(addr < 0x0c00)
	{
		return cx4.ram[addr];
	}

	if(addr >= 0x1f00)
	{
		return cx4.reg[addr & 0xff];
	}

	return 0xff;
}

#ifdef UNUSED_FUNCTION
UINT8 CX4_readb(UINT16 addr)
{
	return CX4_read(addr);
}
#endif

static UINT16 CX4_readw(UINT16 addr)
{
	return CX4_read(addr) | (CX4_read(addr + 1) << 8);
}

static UINT32 CX4_readl(UINT16 addr)
{
	return CX4_read(addr) | (CX4_read(addr + 1) << 8) | (CX4_read(addr + 2) << 16);
}

#ifdef UNUSED_FUNCTION
void CX4_reset()
{
	memset(cx4.ram, 0, 0x0c00);
	memset(cx4.reg, 0, 0x0100);
}
#endif
