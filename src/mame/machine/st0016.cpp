// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/* ST0016 - CPU (z80) + Sound + Video */

#include "st0016.h"

const device_type ST0016_CPU = &device_creator<st0016_cpu_device>;

UINT8 macs_cart_slot;


static ADDRESS_MAP_START(st0016_cpu_internal_map, AS_PROGRAM, 8, st0016_cpu_device)
	AM_RANGE(0xc000, 0xcfff) AM_READ(st0016_sprite_ram_r) AM_WRITE(st0016_sprite_ram_w)
	AM_RANGE(0xd000, 0xdfff) AM_READ(st0016_sprite2_ram_r) AM_WRITE(st0016_sprite2_ram_w)
	AM_RANGE(0xea00, 0xebff) AM_READ(st0016_palette_ram_r) AM_WRITE(st0016_palette_ram_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE("stsnd", st0016_device, st0016_snd_r, st0016_snd_w) /* sound regs 8 x $20 bytes, see notes */
ADDRESS_MAP_END


static ADDRESS_MAP_START(st0016_cpu_internal_io_map, AS_IO, 8, st0016_cpu_device)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w) /* video/crt regs ? */
	AM_RANGE(0xe2, 0xe2) AM_WRITE(st0016_sprite_bank_w)
	AM_RANGE(0xe3, 0xe4) AM_WRITE(st0016_character_bank_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(st0016_palette_bank_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END

// note: a lot of bits are left uninitialized by the games, the default values are uncertain

st0016_cpu_device::st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, ST0016_CPU, "ST0016", tag, owner, clock, "st0016_cpu", __FILE__),
		st0016_game(-1),
		st0016_spr_bank(0),
		st0016_spr2_bank(0),
		st0016_pal_bank(0),
		st0016_char_bank(0),
		spr_dx(0),
		spr_dy(0),
		st0016_ramgfx(0),

		m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME(st0016_cpu_internal_io_map)),
		m_space_config("regs", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME(st0016_cpu_internal_map)),


		m_screen(*this, ":screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")

{
	for (auto & elem : st0016_vregs)
		elem = 0;
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0016_cpu_device::device_start()
{
	z80_device::device_start();
	startup();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void st0016_cpu_device::device_reset()
{
	z80_device::device_reset();

	switch(st0016_game&0x3f)
	{
		case 0: //renju kizoku
			m_screen->set_visible_area(0, 40*8-1, 0, 30*8-1);
			spr_dx=0;
			spr_dy=0;
		break;

		case 1: //neratte chu!
			m_screen->set_visible_area(8,41*8-1,0,30*8-1);
			spr_dx=0;
			spr_dy=8;
		break;

		case 4: //mayjinsen 1&2
			m_screen->set_visible_area(0,32*8-1,0,28*8-1);
		break;

		case 10:
			m_screen->set_visible_area(0,383,0,255);
		break;

		case 11:
			m_screen->set_visible_area(0,383,0,383);
		break;

	}
}

READ8_MEMBER(st0016_cpu_device::soundram_read)
{
	return m_charram[offset];
}

static GFXDECODE_START( st0016 )
GFXDECODE_END

/* CPU interface */
static MACHINE_CONFIG_FRAGMENT( st0016_cpu )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", st0016)
	MCFG_PALETTE_ADD("palette", 16*16*4+1)

	MCFG_DEVICE_ADD("stsnd", ST0016, 0)
	MCFG_ST0016_SOUNDRAM_READ_CB(READ8(st0016_cpu_device, soundram_read))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

MACHINE_CONFIG_END

machine_config_constructor st0016_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( st0016_cpu );
}








static const gfx_layout charlayout =
{
	8,8,
	0x10000,
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

WRITE8_MEMBER(st0016_cpu_device::st0016_sprite_bank_w)
{
/*
    76543210
        xxxx - spriteram  bank1
    xxxx     - spriteram  bank2
*/
	st0016_spr_bank=data&ST0016_SPR_BANK_MASK;
	st0016_spr2_bank=(data>>4)&ST0016_SPR_BANK_MASK;
}

WRITE8_MEMBER(st0016_cpu_device::st0016_palette_bank_w)
{
/*
    76543210
          xx - palram  bank
    xxxxxx   - unknown/unused
*/
	st0016_pal_bank=data&ST0016_PAL_BANK_MASK;
}

WRITE8_MEMBER(st0016_cpu_device::st0016_character_bank_w)
{
/*
    fedcba9876543210
    xxxxxxxxxxxxxxxx - character (bank )
*/

	if(offset&1)
		st0016_char_bank=(st0016_char_bank&0xff)|(data<<8);
	else
		st0016_char_bank=(st0016_char_bank&0xff00)|data;

	st0016_char_bank&=ST0016_CHAR_BANK_MASK;
}


READ8_MEMBER(st0016_cpu_device::st0016_sprite_ram_r)
{
	return st0016_spriteram[ST0016_SPR_BANK_SIZE*st0016_spr_bank+offset];
}

WRITE8_MEMBER(st0016_cpu_device::st0016_sprite_ram_w)
{
	st0016_spriteram[ST0016_SPR_BANK_SIZE*st0016_spr_bank+offset]=data;
}

READ8_MEMBER(st0016_cpu_device::st0016_sprite2_ram_r)
{
	return st0016_spriteram[ST0016_SPR_BANK_SIZE*st0016_spr2_bank+offset];
}

WRITE8_MEMBER(st0016_cpu_device::st0016_sprite2_ram_w)
{
	st0016_spriteram[ST0016_SPR_BANK_SIZE*st0016_spr2_bank+offset]=data;
}

READ8_MEMBER(st0016_cpu_device::st0016_palette_ram_r)
{
	return st0016_paletteram[ST0016_PAL_BANK_SIZE*st0016_pal_bank+offset];
}

WRITE8_MEMBER(st0016_cpu_device::st0016_palette_ram_w)
{
	int color=(ST0016_PAL_BANK_SIZE*st0016_pal_bank+offset)/2;
	int val;
	st0016_paletteram[ST0016_PAL_BANK_SIZE*st0016_pal_bank+offset]=data;
	val=st0016_paletteram[color*2]+(st0016_paletteram[color*2+1]<<8);
	if(!color)
		m_palette->set_pen_color(UNUSED_PEN,pal5bit(val >> 0),pal5bit(val >> 5),pal5bit(val >> 10)); /* same as color 0 - bg ? */
	m_palette->set_pen_color(color,pal5bit(val >> 0),pal5bit(val >> 5),pal5bit(val >> 10));
}

READ8_MEMBER(st0016_cpu_device::st0016_character_ram_r)
{
	return m_charram[ST0016_CHAR_BANK_SIZE*st0016_char_bank+offset];
}

WRITE8_MEMBER(st0016_cpu_device::st0016_character_ram_w)
{
	m_charram[ST0016_CHAR_BANK_SIZE*st0016_char_bank+offset]=data;
	m_gfxdecode->gfx(st0016_ramgfx)->mark_dirty(st0016_char_bank);
}

READ8_MEMBER(st0016_cpu_device::st0016_vregs_r)
{
/*
        $0, $1 = max scanline(including vblank)/timer? ($3e7)

        $8-$40 = bg tilemaps  (8 bytes each) :
                   0 - ? = usually 0/20/ba*
                   1 - 0 = disabled , !zero = address of tilemap in spriteram /$1000  (for example: 3 -> tilemap at $3000 )
                   2 - ? = usually ff/1f/af*
                   3 - priority ? = 0 - under sprites , $ff - over sprites \
                   4 - ? = $7f/$ff
                   5 - ? = $29/$20 (29 when tilemap must be drawn over sprites . maybe this is real priority ?)
                   6 - ? = 0
                   7 - ? =$20/$10/$12*


        $40-$60 = scroll registers , X.w, Y.w

*/

	switch (offset)
	{
		case 0:
		case 1:
			return machine().rand();
	}

	return st0016_vregs[offset];
}

READ8_MEMBER(st0016_cpu_device::st0016_dma_r)
{
	/* bits 0 and 1 = 0 -> DMA transfer complete */
	if(ISMACS)
		return 0;
	else
		return 0;
}


WRITE8_MEMBER(st0016_cpu_device::st0016_vregs_w)
{
	/*

	   I/O ports:

	    $a0 \
	    $a1 - source address >> 1
	    $a2 /

	    $a3 \
	    $a4 - destination address >> 1  (inside character ram)
	    $a5 /

	    $a6 \
	    &a7 - (length inbytes - 1 ) >> 1

	    $a8 - 76543210
	          ??faaaaa

	          a - most sign. bits of length
	          f - DMA start latch

	*/

	st0016_vregs[offset]=data;
	if(offset==0xa8 && (data&0x20))
	{
		UINT32 srcadr=(st0016_vregs[0xa0]|(st0016_vregs[0xa1]<<8)|(st0016_vregs[0xa2]<<16))<<1;
		UINT32 dstadr=(st0016_vregs[0xa3]|(st0016_vregs[0xa4]<<8)|(st0016_vregs[0xa5]<<16))<<1;
		UINT32 length=((st0016_vregs[0xa6]|(st0016_vregs[0xa7]<<8)|((st0016_vregs[0xa8]&0x1f)<<16))+1)<<1;

		// todo : dma callback so macs_cart_slot can be local to MACs driver?

		UINT32 srclen = (memregion(":maincpu")->bytes());
		UINT8 *mem = memregion(":maincpu")->base();

		srcadr += macs_cart_slot*0x400000;

		while(length>0)
		{
			if( srcadr < srclen && (dstadr < ST0016_MAX_CHAR_BANK*ST0016_CHAR_BANK_SIZE))
			{
				st0016_char_bank=dstadr>>5;
				st0016_character_ram_w(space,dstadr&0x1f,mem[srcadr]);
				srcadr++;
				dstadr++;
				length--;
			}
			else
			{
				/* samples ? sound dma ? */
				// speaglsht:  unknown DMA copy : src - 2B6740, dst - 4400, len - 1E400
				logerror("unknown DMA copy : src - %X, dst - %X, len - %X, PC - %X\n",srcadr,dstadr,length,space.device().safe_pcbase());
				break;
			}
		}
	}
}

void st0016_cpu_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	object ram :

	each entry is 8 bytes:

	76543210 (bit)
	0 llllllll
	1 ---gSSSl
	2 oooooooo
	3 fooooooo
	4 xxxxxxxx
	5 ----XXxx
	6 yyyyyyyy
	7 ----XYyy

	1 - always(?) set
	S - scroll index ? (ports $40-$60, X(word),Y(word) )
	l - sublist length (8 byte entries -1)
	o - sublist offset (*8 to get real offset)
	f - end of list  flag
	x,y - sprite coords

	(complete guess)
	g - use tile sizes specified in list (global / fixed ones if not set?) (gostop)
	X = global X size?
	Y = global Y size?

	sublist format (8 bytes/entry):

	76543210
	0 cccccccc
	1 cccccccc
	2 --kkkkkk
	3 QW------
	4 xxxxxxxx
	5 -B--XX-x
	6 yyyyyyyy
	7 ----YY-y

	c - character code
	k - palette
	QW - flips
	x,y - coords
	XX,YY - size (1<<size)
	B - merge pixel data with prevoius one (8bpp mode - neratte: seta logo and title screen)

	*/

	gfx_element *gfx = m_gfxdecode->gfx(st0016_ramgfx);
	int i, j, lx, ly, x, y, code, offset, length, sx, sy, color, flipx, flipy, scrollx, scrolly/*,plx,ply*/;


	for (i = 0; i < ST0016_SPR_BANK_SIZE*ST0016_MAX_SPR_BANK; i += 8)
	{
		x = st0016_spriteram[i + 4] + ((st0016_spriteram[i + 5] & 3) << 8);
		y = st0016_spriteram[i + 6] + ((st0016_spriteram[i + 7] & 3) << 8);

		int use_sizes = (st0016_spriteram[i + 1] & 0x10);
		int globalx = (st0016_spriteram[i + 5] & 0x0c)>>2;
		int globaly = (st0016_spriteram[i + 7] & 0x0c)>>2;

		scrollx = (st0016_vregs[(((st0016_spriteram[i + 1] & 0x0f) >> 1) << 2) + 0x40] + 256 * st0016_vregs[(((st0016_spriteram[i + 1] & 0x0f) >> 1) << 2) + 1 + 0x40]) & 0x3ff;
		scrolly = (st0016_vregs[(((st0016_spriteram[i + 1] & 0x0f) >> 1) << 2) + 2 + 0x40] + 256 * st0016_vregs[(((st0016_spriteram[i + 1] & 0x0f) >> 1) << 2) + 3 + 0x40]) & 0x3ff;

		if (!ISMACS)
		{
			if (x & 0x200) x -= 0x400; //sign
			if (y & 0x200) y -= 0x400;

			if (scrollx & 0x200) scrollx -= 0x400; //sign
			if (scrolly & 0x200) scrolly -= 0x400;
		}

		if (ISMACS1)
		{
			if (x & 0x200) x -= 0x400; //sign
			if (y & 0x200) y -= 0x2b0;//0x400;

			if (scrollx & 0x200) scrollx -= 0x400; //sign
			if (scrolly & 0x200) scrolly -= 0x400;
		}

		x += scrollx;
		y += scrolly;

		if (ISMACS)
		{
			y += 0x20;
		}

		if (st0016_spriteram[i + 3] & 0x80) /* end of list */
			break;

		offset = st0016_spriteram[i + 2] + 256 * (st0016_spriteram[i + 3]);
		offset <<= 3;

		length = st0016_spriteram[i + 0] + 1 + 256 * (st0016_spriteram[i + 1] & 1);

		//plx=(st0016_spriteram[i+5]>>2)&0x3;
		//ply=(st0016_spriteram[i+7]>>2)&0x3;

		if (offset < ST0016_SPR_BANK_SIZE*ST0016_MAX_SPR_BANK)
		{
			for (j = 0; j < length; j++)
			{
				code = st0016_spriteram[offset] + 256 * st0016_spriteram[offset + 1];
				sx = st0016_spriteram[offset + 4] + ((st0016_spriteram[offset + 5] & 1) << 8);
				sy = st0016_spriteram[offset + 6] + ((st0016_spriteram[offset + 7] & 1) << 8);

				if (ISMACS && !(ISMACS1))
				{
					if (sy & 0x100) sy -= 0x200; //yuka & yujan
				}

				if (ISMACS)
				{
					sy = 0xe0 - sy;
				}

				sx += x;
				sy += y;
				color = st0016_spriteram[offset + 2] & 0x3f;

				if (use_sizes)
				{
					lx = (st0016_spriteram[offset + 5] >> 2) & 3;
					ly = (st0016_spriteram[offset + 7] >> 2) & 3;
				}
				else
				{
					lx = globalx;
					ly = globaly;

				}

				/*
				    if(plx |ply) //parent
				    {
				    lx=plx;
				    ly=ply;
				    }
				    */

				flipx = st0016_spriteram[offset + 3] & 0x80;
				flipy = st0016_spriteram[offset + 3] & 0x40;

				if (ISMACS)
					sy -= (1 << ly) * 8;

				{
					int x0, y0, i0 = 0;
					for (x0 = (flipx ? ((1 << lx) - 1) : 0); x0 != (flipx ? -1 : (1 << lx)); x0 += (flipx ? -1 : 1))
						for (y0 = (flipy ? ((1 << ly) - 1) : 0); y0 != (flipy ? -1 : (1 << ly)); y0 += (flipy ? -1 : 1))
						{
						/* custom draw */
						UINT16 *destline;
						int yloop, xloop;
						int ypos, xpos;
						int tileno;
						const UINT8 *srcgfx;
						int gfxoffs;
						ypos = sy + y0 * 8 + spr_dy;
						xpos = sx + x0 * 8 + spr_dx;
						tileno = (code + i0++)&ST0016_CHAR_BANK_MASK;

						gfxoffs = 0;
						srcgfx = gfx->get_data(tileno);

						for (yloop = 0; yloop < 8; yloop++)
						{
							UINT16 drawypos;

							if (!flipy) { drawypos = ypos + yloop; }
							else { drawypos = (ypos + 8 - 1) - yloop; }
							destline = &bitmap.pix16(drawypos);

							for (xloop = 0; xloop<8; xloop++)
							{
								UINT16 drawxpos;
								int pixdata;
								pixdata = srcgfx[gfxoffs];

								if (!flipx) { drawxpos = xpos + xloop; }
								else { drawxpos = (xpos + 8 - 1) - xloop; }

								if (drawxpos > cliprect.max_x)
									drawxpos -= 512; // wrap around

								if (cliprect.contains(drawxpos, drawypos))
								{
									if (st0016_spriteram[offset + 5] & 0x40)
									{
										destline[drawxpos] = (destline[drawxpos] | pixdata << 4) & 0x3ff;
									}
									else
									{
										if (ISMACS2)
										{
											if (pixdata)//|| destline[drawxpos]==UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}
										else
										{
											if (pixdata || destline[drawxpos] == UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}
									}
								}

								gfxoffs++;
							}
						}
						}
				}
				offset += 8;
				if (offset >= ST0016_SPR_BANK_SIZE*ST0016_MAX_SPR_BANK)
					break;
			}
		}
	}
}


void st0016_cpu_device::st0016_save_init()
{
	save_item(NAME(st0016_spr_bank));
	save_item(NAME(st0016_spr2_bank));
	save_item(NAME(st0016_pal_bank));
	save_item(NAME(st0016_char_bank));
	//save_item(NAME(st0016_rom_bank));
	save_item(NAME(st0016_vregs));
	save_pointer(NAME(m_charram.get()), ST0016_MAX_CHAR_BANK*ST0016_CHAR_BANK_SIZE);
	save_pointer(NAME(st0016_paletteram.get()), ST0016_MAX_PAL_BANK*ST0016_PAL_BANK_SIZE);
	save_pointer(NAME(st0016_spriteram.get()), ST0016_MAX_SPR_BANK*ST0016_SPR_BANK_SIZE);
}


void st0016_cpu_device::startup()
{
	int gfx_index=0;

	macs_cart_slot = 0;
	m_charram=make_unique_clear<UINT8[]>(ST0016_MAX_CHAR_BANK*ST0016_CHAR_BANK_SIZE);
	st0016_spriteram=make_unique_clear<UINT8[]>(ST0016_MAX_SPR_BANK*ST0016_SPR_BANK_SIZE);
	st0016_paletteram=make_unique_clear<UINT8[]>(ST0016_MAX_PAL_BANK*ST0016_PAL_BANK_SIZE);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, m_charram.get(), 0, 0x40, 0));
	st0016_ramgfx = gfx_index;

	spr_dx=0;
	spr_dy=0;



	st0016_save_init();
}


void st0016_cpu_device::draw_bgmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	gfx_element *gfx = m_gfxdecode->gfx(st0016_ramgfx);
	int j;
	//for(j=0x40-8;j>=0;j-=8)
	for (j = 0; j < 0x40; j += 8)
	{
		if (st0016_vregs[j + 1] && ((priority && (st0016_vregs[j + 3] == 0xff)) || ((!priority) && (st0016_vregs[j + 3] != 0xff))))
		{
			int x, y, code, color, flipx, flipy;
			int i = st0016_vregs[j + 1] * 0x1000;

			for (x = 0; x < 32 * 2; x++)
			{
				for (y = 0; y < 8 * 4; y++)
				{
					code = st0016_spriteram[i] + 256 * st0016_spriteram[i + 1];
					color = st0016_spriteram[i + 2] & 0x3f;

					flipx = st0016_spriteram[i + 3] & 0x80;
					flipy = st0016_spriteram[i + 3] & 0x40;

					if (priority)
					{
						gfx->transpen(bitmap, cliprect,
							code,
							color,
							flipx, flipy,
							x * 8 + spr_dx, y * 8 + spr_dy, 0);
					}
					else
					{
						UINT16 *destline;
						int yloop, xloop;
						int ypos, xpos;
						const UINT8 *srcgfx;
						int gfxoffs;
						ypos = y * 8 + spr_dy;//+((st0016_vregs[j+2]==0xaf)?0x50:0);//hack for mayjinsen title screen
						xpos = x * 8 + spr_dx;
						gfxoffs = 0;
						srcgfx = gfx->get_data(code);

						for (yloop = 0; yloop < 8; yloop++)
						{
							UINT16 drawypos;

							if (!flipy) { drawypos = ypos + yloop; }
							else { drawypos = (ypos + 8 - 1) - yloop; }
							destline = &bitmap.pix16(drawypos);

							for (xloop = 0; xloop<8; xloop++)
							{
								UINT16 drawxpos;
								int pixdata;
								pixdata = srcgfx[gfxoffs];

								if (!flipx) { drawxpos = xpos + xloop; }
								else { drawxpos = (xpos + 8 - 1) - xloop; }

								if (drawxpos > cliprect.max_x)
									drawxpos -= 512; // wrap around

								if (cliprect.contains(drawxpos, drawypos))
								{
									if (st0016_vregs[j + 7] == 0x12)
										destline[drawxpos] = (destline[drawxpos] | (pixdata << 4)) & 0x3ff;
									else
									{
										if (ISMACS2)
										{
											if (pixdata)// || destline[drawxpos]==UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}
										else
										{
											if (pixdata || destline[drawxpos] == UNUSED_PEN)
											{
												destline[drawxpos] = pixdata + (color * 16);
											}
										}

									}
								}

								gfxoffs++;

							}
						}
					}
					i += 4;
				}
			}
		}
	}
}


void st0016_cpu_device::st0016_draw_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_bgmap(bitmap,cliprect,0);
	draw_sprites(bitmap,cliprect);
	draw_bgmap(bitmap,cliprect,1);
}

UINT32 st0016_cpu_device::update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	if(machine().input().code_pressed_once(KEYCODE_Z))
	{
		int h,j;
		FILE *p=fopen("vram.bin","wb");
		fwrite(st0016_spriteram.get(),1,0x1000*ST0016_MAX_SPR_BANK,p);
		fclose(p);

		p=fopen("vram.txt","wt");
		for(h=0;h<0xc0;h++)
			fprintf(p,"VREG %.4x - %.4x\n",h,st0016_vregs[h]);
		for(h=0;h<0x1000*ST0016_MAX_SPR_BANK;h+=8)
		{
			fprintf(p,"%.4x - %.4x - ",h,h>>3);
			for(j=0;j<8;j++)
				fprintf(p,"%.2x ",st0016_spriteram[h+j]);
				fprintf(p,"\n");
		}
		fclose(p);
	}
#endif

	bitmap.fill(UNUSED_PEN, cliprect);
	st0016_draw_screen(screen, bitmap, cliprect);
	return 0;
}
