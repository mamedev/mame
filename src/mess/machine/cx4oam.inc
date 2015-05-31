// license:GPL-2.0+
// copyright-holders:byuu, Nach
/***************************************************************************

    cx4oam.c

    Code based on original work by zsKnight, anomie and Nach.
    This implementation is based on C++ "cx4*.cpp" by byuu
    (up to date with source v 0.49).

***************************************************************************/

//Build OAM
static void CX4_op00_00(running_machine &machine)
{
	INT32 i;

	UINT32 oamptr = cx4.ram[0x626] << 2;
	UINT16 globalx, globaly;
	UINT32 oamptr2;
	INT16 sprx, spry;
	UINT8 sprname, sprattr;
	UINT8 sprcount;
	UINT8 offset;
	UINT32 srcptr;

	for(i = 0x1fd; i > oamptr && i >= 0; i -= 4)
	{
		//clear oam-to-be
		if(i >= 0)
		{
			cx4.ram[i] = 0xe0;
		}
	}

	globalx = CX4_readw(0x621);
	globaly = CX4_readw(0x623);
	oamptr2 = 0x200 + (cx4.ram[0x626] >> 2);

	if(!cx4.ram[0x620])
	{
		return;
	}

	sprcount = 128 - cx4.ram[0x626];
	offset = (cx4.ram[0x626] & 3) * 2;
	srcptr = 0x220;

	address_space &space = machine.device<cpu_device>("maincpu")->space(AS_PROGRAM);
	for(i = cx4.ram[0x620]; i > 0 && sprcount > 0; i--, srcptr += 16)
	{
		UINT32 spraddr = CX4_readl(srcptr + 7);

		sprx = CX4_readw(srcptr)     - globalx;
		spry = CX4_readw(srcptr + 2) - globaly;
		sprname = cx4.ram[srcptr + 5];
		sprattr = cx4.ram[srcptr + 4] | cx4.ram[srcptr + 6];

		if(space.read_byte(spraddr))
		{
			INT16 x, y;
			INT32 sprcnt;
			for(sprcnt = space.read_byte(spraddr++); sprcnt > 0 && sprcount > 0; sprcnt--, spraddr += 4)
			{
				x = (INT8)space.read_byte(spraddr + 1);
				if(sprattr & 0x40)
				{
					x = -x - ((space.read_byte(spraddr) & 0x20) ? 16 : 8);
				}
				x += sprx;
				if(x >= -16 && x <= 272)
				{
					y = (INT8)space.read_byte(spraddr + 2);
					if(sprattr & 0x80)
					{
						y = -y - ((space.read_byte(spraddr) & 0x20) ? 16 : 8);
					}
					y += spry;
					if(y >= -16 && y <= 224)
					{
						cx4.ram[oamptr    ] = (UINT8)x;
						cx4.ram[oamptr + 1] = (UINT8)y;
						cx4.ram[oamptr + 2] = sprname + space.read_byte(spraddr + 3);
						cx4.ram[oamptr + 3] = sprattr ^ (space.read_byte(spraddr) & 0xc0);
						cx4.ram[oamptr2] &= ~(3 << offset);
						if(x & 0x100)
						{
							cx4.ram[oamptr2] |= 1 << offset;
						}
						if(space.read_byte(spraddr) & 0x20)
						{
							cx4.ram[oamptr2] |= 2 << offset;
						}
						oamptr += 4;
						sprcount--;
						offset = (offset + 2) & 6;
						if(!offset)
						{
							oamptr2++;
						}
					}
				}
			}
		}
		else if(sprcount > 0)
		{
			cx4.ram[oamptr    ] = (UINT8)sprx;
			cx4.ram[oamptr + 1] = (UINT8)spry;
			cx4.ram[oamptr + 2] = sprname;
			cx4.ram[oamptr + 3] = sprattr;
			cx4.ram[oamptr2] &= ~(3 << offset);
			if(sprx & 0x100)
			{
				cx4.ram[oamptr2] |= 3 << offset;
			}
			else
			{
				cx4.ram[oamptr2] |= 2 << offset;
			}
			oamptr += 4;
			sprcount--;
			offset = (offset + 2) & 6;
			if(!offset)
			{
				oamptr2++;
			}
		}
	}
}

//Scale and Rotate
static void CX4_op00_03(void)
{
	CX4_C4DoScaleRotate(0);
}

//Transform Lines
static void CX4_op00_05(running_machine &machine)
{
	INT32 i;
	UINT32 ptr = 0, ptr2 = 0;

	cx4.C4WFX2Val = CX4_read(0x1f83);
	cx4.C4WFY2Val = CX4_read(0x1f86);
	cx4.C4WFDist  = CX4_read(0x1f89);
	cx4.C4WFScale = CX4_read(0x1f8c);

	//Transform Vertices
	for(i = CX4_readw(0x1f80); i > 0; i--, ptr += 0x10)
	{
		cx4.C4WFXVal = CX4_readw(ptr + 1);
		cx4.C4WFYVal = CX4_readw(ptr + 5);
		cx4.C4WFZVal = CX4_readw(ptr + 9);
		CX4_C4TransfWireFrame();

		//Displace
		CX4_writew(machine, ptr + 1, cx4.C4WFXVal + 0x80);
		CX4_writew(machine, ptr + 5, cx4.C4WFYVal + 0x50);
	}

	CX4_writew(machine, 0x600,     23);
	CX4_writew(machine, 0x602,     0x60);
	CX4_writew(machine, 0x605,     0x40);
	CX4_writew(machine, 0x600 + 8, 23);
	CX4_writew(machine, 0x602 + 8, 0x60);
	CX4_writew(machine, 0x605 + 8, 0x40);

	ptr = 0xb02;

	for(i = CX4_readw(0xb00); i > 0; i--, ptr += 2, ptr2 += 8)
	{
		cx4.C4WFXVal  = CX4_readw((CX4_read(ptr + 0) << 4) + 1);
		cx4.C4WFYVal  = CX4_readw((CX4_read(ptr + 0) << 4) + 5);
		cx4.C4WFX2Val = CX4_readw((CX4_read(ptr + 1) << 4) + 1);
		cx4.C4WFY2Val = CX4_readw((CX4_read(ptr + 1) << 4) + 5);
		CX4_C4CalcWireFrame();
		CX4_writew(machine, ptr2 + 0x600, cx4.C4WFDist ? cx4.C4WFDist : 1);
		CX4_writew(machine, ptr2 + 0x602, cx4.C4WFXVal);
		CX4_writew(machine, ptr2 + 0x605, cx4.C4WFYVal);
	}
}

//Scale and Rotate
static void CX4_op00_07(void)
{
	CX4_C4DoScaleRotate(64);
}

//Draw Wireframe
static void CX4_op00_08(running_machine &machine)
{
	CX4_C4DrawWireFrame(machine);
}

//Disintegrate
static void CX4_op00_0b(running_machine &machine)
{
	UINT8  width, height;
	UINT32 startx, starty;
	UINT32 srcptr;
	UINT32 x, y;
	INT32  scalex, scaley;
	INT32  cx, cy;
	INT32  i, j;

	width  = CX4_read(0x1f89);
	height = CX4_read(0x1f8c);
	cx     = CX4_readw(0x1f80);
	cy     = CX4_readw(0x1f83);

	scalex = (INT16)CX4_readw(0x1f86);
	scaley = (INT16)CX4_readw(0x1f8f);
	startx = -cx * scalex + (cx << 8);
	starty = -cy * scaley + (cy << 8);
	srcptr = 0x600;

	for(i = 0; i < (width * height) >> 1; i++)
	{
		CX4_write(machine, i, 0);
	}

	for(y = starty, i = 0;i < height; i++, y += scaley)
	{
		for(x = startx, j = 0;j < width; j++, x += scalex)
		{
			if((x >> 8) < width && (y >> 8) < height && (y >> 8) * width + (x >> 8) < 0x2000)
			{
				UINT8 pixel = (j & 1) ? (cx4.ram[srcptr] >> 4) : (cx4.ram[srcptr]);
				INT32 index = (y >> 11) * width * 4 + (x >> 11) * 32 + ((y >> 8) & 7) * 2;
				UINT8 mask = 0x80 >> ((x >> 8) & 7);

				if(pixel & 1) cx4.ram[index     ] |= mask;
				if(pixel & 2) cx4.ram[index +  1] |= mask;
				if(pixel & 4) cx4.ram[index + 16] |= mask;
				if(pixel & 8) cx4.ram[index + 17] |= mask;
			}
			if(j & 1)
			{
				srcptr++;
			}
		}
	}
}

//Bitplane Wave
static void CX4_op00_0c(running_machine &machine)
{
	int i, j;
	UINT32 destptr = 0;
	UINT32 waveptr = CX4_read(0x1f83);
	UINT16 mask1   = 0xc0c0;
	UINT16 mask2   = 0x3f3f;

	for(j = 0; j < 0x10; j++)
	{
		do
		{
			INT16 height = -((INT8)CX4_read(waveptr + 0xb00)) - 16;
			for(i = 0; i < 40; i++)
			{
				UINT16 temp = CX4_readw(destptr + CX4_wave_data[i]) & mask2;
				if(height >= 0)
				{
					if(height < 8)
					{
						temp |= mask1 & CX4_readw(0xa00 + height * 2);
					}
					else
					{
						temp |= mask1 & 0xff00;
					}
				}
				CX4_writew(machine, destptr + CX4_wave_data[i], temp);
				height++;
			}
			waveptr = (waveptr + 1) & 0x7f;
			mask1   = (mask1 >> 2) | (mask1 << 6);
			mask2   = (mask2 >> 2) | (mask2 << 6);
		} while(mask1 != 0xc0c0);
		destptr += 16;

		do
		{
			INT16 height = -((INT8)CX4_read(waveptr + 0xb00)) - 16;
			for(i = 0; i < 40; i++)
			{
				UINT16 temp = CX4_readw(destptr + CX4_wave_data[i]) & mask2;
				if(height >= 0)
				{
					if(height < 8)
					{
						temp |= mask1 & CX4_readw(0xa10 + height * 2);
					}
					else
					{
						temp |= mask1 & 0xff00;
					}
				}
				CX4_writew(machine, destptr + CX4_wave_data[i], temp);
				height++;
			}
			waveptr = (waveptr + 1) & 0x7f;
			mask1   = (mask1 >> 2) | (mask1 << 6);
			mask2   = (mask2 >> 2) | (mask2 << 6);
		} while(mask1 != 0xc0c0);
		destptr += 16;
	}
}
