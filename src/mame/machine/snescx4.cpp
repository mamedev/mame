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

static uint16_t CX4_readw(uint16_t addr);
static uint32_t CX4_readl(uint16_t addr);

static void CX4_writew(address_space &space, uint16_t addr, uint16_t data);
//static void CX4_writel(address_space &space, uint16_t addr, uint32_t data);

static void CX4_C4DrawLine(int32_t X1, int32_t Y1, int16_t Z1, int32_t X2, int32_t Y2, int16_t Z2, uint8_t Color);

#include "machine/cx4data.hxx"
#include "machine/cx4fn.hxx"

static uint32_t CX4_ldr(uint8_t r)
{
	uint16_t addr = 0x0080 + (r * 3);
	return (cx4.reg[addr + 0] <<  0)
			| (cx4.reg[addr + 1] <<  8)
			| (cx4.reg[addr + 2] << 16);
}

static void CX4_str(uint8_t r, uint32_t data)
{
	uint16_t addr = 0x0080 + (r * 3);
	cx4.reg[addr + 0] = (data >>  0);
	cx4.reg[addr + 1] = (data >>  8);
	cx4.reg[addr + 2] = (data >> 16);
}

static void CX4_mul(uint32_t x, uint32_t y, uint32_t *rl, uint32_t *rh)
{
	int64_t rx = x & 0xffffff;
	int64_t ry = y & 0xffffff;
	if(rx & 0x800000)rx |= ~0x7fffff;
	if(ry & 0x800000)ry |= ~0x7fffff;

	rx *= ry;

	*rl = (rx)       & 0xffffff;
	*rh = (rx >> 24) & 0xffffff;
}

static uint32_t CX4_sin(uint32_t rx)
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

static uint32_t CX4_cos(uint32_t rx)
{
	return CX4_sin(rx + 0x080);
}

static void CX4_immediate_reg(uint32_t start)
{
	uint32_t i = 0;
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

static void CX4_transfer_data(address_space &space)
{
	uint32_t src;
	uint16_t dest, count;
	uint32_t i;

	src   = (cx4.reg[0x40]) | (cx4.reg[0x41] << 8) | (cx4.reg[0x42] << 16);
	count = (cx4.reg[0x43]) | (cx4.reg[0x44] << 8);
	dest  = (cx4.reg[0x45]) | (cx4.reg[0x46] << 8);

	for(i=0;i<count;i++)
	{
		CX4_write(space, dest++, space.read_byte(src++));
	}
}

#include "machine/cx4oam.hxx"
#include "machine/cx4ops.hxx"

void CX4_write(address_space &space, uint32_t addr, uint8_t data)
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
		CX4_transfer_data(space);
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
			case 0x00: CX4_op00(space); break;
			case 0x01: CX4_op01(space); break;
			case 0x05: CX4_op05(space); break;
			case 0x0d: CX4_op0d(space); break;
			case 0x10: CX4_op10(); break;
			case 0x13: CX4_op13(); break;
			case 0x15: CX4_op15(space); break;
			case 0x1f: CX4_op1f(space); break;
			case 0x22: CX4_op22(); break;
			case 0x25: CX4_op25(); break;
			case 0x2d: CX4_op2d(space); break;
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
void CX4_writeb(address_space &space, uint16_t addr, uint8_t data)
{
	CX4_write(space, addr,     data);
}
#endif

static void CX4_writew(address_space &space, uint16_t addr, uint16_t data)
{
	CX4_write(space, addr + 0, data >> 0);
	CX4_write(space, addr + 1, data >> 8);
}

#ifdef UNUSED_FUNCTION
void CX4_writel(address_space &space, uint16_t addr, uint32_t data)
{
	CX4_write(space, addr + 0, data >>  0);
	CX4_write(space, addr + 1, data >>  8);
	CX4_write(space, addr + 2, data >> 16);
}
#endif

uint8_t CX4_read(uint32_t addr)
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
uint8_t CX4_readb(uint16_t addr)
{
	return CX4_read(addr);
}
#endif

static uint16_t CX4_readw(uint16_t addr)
{
	return CX4_read(addr) | (CX4_read(addr + 1) << 8);
}

static uint32_t CX4_readl(uint16_t addr)
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
