/*

Sega System H1
preliminary

22 Aug 2004 - Basic skeleton driver, loads some roms doesn't run the right code yet
2 Dec 2008 - Added an hack for the SH-2,fixed some irqs and some memory maps/ram sharing.
             Got to the point that area 0x03e00000 on the SH-2 loads some DMA-style tables.

Known Games on this Platform
Cool Riders

-- readme --

Cool Riders by SEGA 1995
SYSTEM H1 CPU Board
-------------------
Processors :
Hitachi SH2 HD6417095
Toshiba TMP68HC000N-16
Hitachi SH7032 HD6417032F20

Eprom :
Ep17662.12

SEGA CUSTOM IC :
315-5687 (x2)
315-5757
315-5758
315-5849
315-5800 GAL16V8B
315-5801 GAL16V8B
315-5802 GAL16V8B

SYSTEM H1 VIDEO BOARD
---------------------
SEGA CUSTOM IC :
315-5648 (x4)
315-5691
315-5692
315-5693 (x2)
315-5694
315-5695 (x2)
315-5696 (x2)
315-5697
315-5698
315-5803 GAL16V8B
315-5864 GAL16V8B

*/

#include "driver.h"
#include "debugger.h"
#include "cpu/sh2/sh2.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/scsp.h"

static UINT32* sysh1_workram_h,*framebuffer_vram, *h1_unk, *h1_charram;
static UINT32* sysh1_txt_blit;

/* video */

static VIDEO_START(coolridr)
{
}

static VIDEO_UPDATE(coolridr)
{
	/*
	0x3e09c00-0x3e0bbff and 0x3e0bc00-0x3e0dbff looks like tilemap planes.
	0x3e00000 onward seems to contain video registers, I've seen MAP registers that clearly points to the aforementioned planes.
	*/
	const gfx_element *gfx = screen->machine->gfx[2];
	int count = 0x09c00/4;
	int y,x;
	static int color;


	if(input_code_pressed(screen->machine,KEYCODE_Z))
		count = 0x0bc00/4;

	if(input_code_pressed_once(screen->machine,KEYCODE_A))
		color++;

	if(input_code_pressed_once(screen->machine,KEYCODE_S))
		color--;

	for (y=0;y<64;y++)
	{
		for (x=0;x<128;x+=2)
		{
			int tile;

			tile = (framebuffer_vram[count] & 0x0fff0000) >> 16;
			//int colour = tile>>12;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,0,0,(x+0)*16,y*16);

			tile = (framebuffer_vram[count] & 0x00000fff) >> 0;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,0,0,(x+1)*16,y*16);

			count++;
		}
	}

	return 0;
}

/* end video */

/* unknown purpose */
static READ32_HANDLER(sysh1_unk_r)
{
	switch(offset)
	{
		case 0x08/4:
		{
			static UINT8 vblank = 0;

			vblank^=1;

			return (h1_unk[offset] & 0xfdffffff) | (vblank<<25);
		}
		case 0x14/4:
			return h1_unk[offset];
		//case 0x20/4:
	}

	return 0xffffffff;//h1_unk[offset];
}

static WRITE32_HANDLER(sysh1_unk_w)
{
	COMBINE_DATA(&h1_unk[offset]);
}

/* According to Guru, this is actually the same I/O chip of Sega Model 2 HW */
#if 0
static READ32_HANDLER(sysh1_ioga_r)
{
	//return mame_rand(space->machine);//h1_ioga[offset];
	return h1_ioga[offset];
}

static WRITE32_HANDLER(sysh1_ioga_w)
{
	COMBINE_DATA(&h1_ioga[offset]);
}
#endif

/* this looks like an exotic I/O-based tilemap blitter, very unusual from Sega... */
static WRITE32_HANDLER( sysh1_txt_blit_w )
{
	static UINT16 cmd,param;
	static UINT32 dst_addr;

	COMBINE_DATA(&sysh1_txt_blit[offset]);

	switch(offset)
	{
		case 0x10/4: //cmd + param?
			cmd = (sysh1_txt_blit[offset] & 0xffff0000) >> 16;
			param = (sysh1_txt_blit[offset] & 0x0000ffff) >> 0;
			dst_addr = 0x3f40000;
			break;
		case 0x14/4: //data
			/*  "THIS MACHINE IS STAND-ALONE." / disclaimer written with this CMD */
			if((cmd & 0xff) == 0xf4)
			{
				/* FIXME: color offset and proper offset calculation */
				memory_write_dword(space,(dst_addr + param),data);
				dst_addr+=4;

				//printf("PARAM = %04x | %c%c%c%c\n",param,(data >> 24) & 0xff,(data >> 16) & 0xff,(data >> 8) & 0xff,(data >> 0) & 0xff);
			}
			//else
			//	printf("CMD = %04x PARAM = %04x DATA = %08x\n",cmd,param,data);
			break;
	}
}


//UINT16* sysh1_soundram;

static WRITE32_HANDLER( sysh1_pal_w )
{
	int r,g,b;
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	b = ((space->machine->generic.paletteram.u32[offset] & 0x00007c00) >> 10);
	g = ((space->machine->generic.paletteram.u32[offset] & 0x000003e0) >> 5);
	r = ((space->machine->generic.paletteram.u32[offset] & 0x0000001f) >> 0);
	palette_set_color_rgb(space->machine,(offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
	b = ((space->machine->generic.paletteram.u32[offset] & 0x7c000000) >> 26);
	g = ((space->machine->generic.paletteram.u32[offset] & 0x03e00000) >> 21);
	r = ((space->machine->generic.paletteram.u32[offset] & 0x001f0000) >> 16);
	palette_set_color_rgb(space->machine,offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
}


/* FIXME: this seems to do a hell lot of stuff, it's not ST-V SCU but still somewhat complex :/ */
static void sysh1_dma_transfer( const address_space *space, UINT16 dma_index )
{
	static UINT32 src,dst,size,type,s_i;
	static UINT8 end_dma_mark;

	end_dma_mark = 0;

	do{
		src = (framebuffer_vram[(0+dma_index)/4] & 0x0fffffff);
		dst = (framebuffer_vram[(4+dma_index)/4] & 0xfffff);
		size = framebuffer_vram[(8+dma_index)/4]*2;
		type = (framebuffer_vram[(0+dma_index)/4] & 0xf0000000) >> 28;

		if(type & 8)
		printf("%08x %08x %08x %08x\n",src,dst,size,type);

		if(type == 0xd)
			dst |= 0x3d00000; //to charram, FIXME: unknown offset

		if(type == 0xe)
		{
			dst |= 0x3c00000; //to paletteram FIXME: unknown offset
			//printf("%08x %08x %08x %08x\n",src,dst,size,type);
			if((src & 0xff00000) == 0x3e00000)
				return; //FIXME: kludge to avoid palette corruption
			//debugger_break(space->machine);
		}

		if(type == 0xd || type == 0xe)
		{
			for(s_i=0;s_i<size;s_i+=4)
			{
				memory_write_dword(space,dst,memory_read_dword(space,src));
				dst+=4;
				src+=4;
			}
		}
		else
		{
			//printf("%08x %08x %08x %08x\n",src,dst,size,type);
		}

		if(type == 0x0)
			end_dma_mark = 1; //end of DMA list

		dma_index+=0xc;

	}while(!end_dma_mark );
}

static WRITE32_HANDLER( sysh1_dma_w )
{
	COMBINE_DATA(&framebuffer_vram[offset]);

	if(offset*4 == 0x000)
	{
		if((framebuffer_vram[offset] & 0xff00000) == 0xfe00000)
			sysh1_dma_transfer(space, framebuffer_vram[offset] & 0xffff);
	}
}

static WRITE32_HANDLER( sysh1_char_w )
{
	COMBINE_DATA(&h1_charram[offset]);

	{
		UINT8 *gfx = memory_region(space->machine, "ram_gfx");

		gfx[offset*4+0] = (h1_charram[offset] & 0xff000000) >> 24;
		gfx[offset*4+1] = (h1_charram[offset] & 0x00ff0000) >> 16;
		gfx[offset*4+2] = (h1_charram[offset] & 0x0000ff00) >> 8;
		gfx[offset*4+3] = (h1_charram[offset] & 0x000000ff) >> 0;

		gfx_element_mark_dirty(space->machine->gfx[2], offset/64); //*4/256
	}
}

static ADDRESS_MAP_START( system_h1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_SHARE("share1") AM_WRITENOP
	AM_RANGE(0x01000000, 0x01ffffff) AM_ROM AM_REGION("gfx_data",0x0000000)

	AM_RANGE(0x03c00000, 0x03c0ffff) AM_RAM_WRITE(sysh1_pal_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x03d00000, 0x03dfffff) AM_RAM_WRITE(sysh1_char_w) AM_BASE(&h1_charram) //FIXME: half size
	AM_RANGE(0x03e00000, 0x03efffff) AM_RAM_WRITE(sysh1_dma_w) AM_BASE(&framebuffer_vram) //FIXME: not all of it

	AM_RANGE(0x03f00000, 0x03f0ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x03f40000, 0x03f4ffff) AM_RAM //text tilemap + "lineram"
	AM_RANGE(0x04000000, 0x0400003f) AM_RAM_WRITE(sysh1_txt_blit_w) AM_BASE(&sysh1_txt_blit)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_BASE(&sysh1_workram_h)
	AM_RANGE(0x20000000, 0x201fffff) AM_ROM AM_SHARE("share1")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( coolridr_submap, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM

	AM_RANGE(0x03008800, 0x03008803) AM_RAM /*???*/
	AM_RANGE(0x03008900, 0x03008903) AM_RAM /*???*/
	AM_RANGE(0x03100400, 0x03100403) AM_RAM /*irq enable?*/
	AM_RANGE(0x03208800, 0x03208803) AM_RAM /*???*/
	AM_RANGE(0x03208900, 0x03208903) AM_RAM /*???*/
	AM_RANGE(0x03300400, 0x03300403) AM_RAM /*irq enable?*/

	AM_RANGE(0x04000000, 0x0400003f) AM_READWRITE(sysh1_unk_r,sysh1_unk_w) AM_BASE(&h1_unk)
	AM_RANGE(0x04200000, 0x0420003f) AM_RAM /*???*/

	AM_RANGE(0x05000000, 0x05000fff) AM_RAM
	AM_RANGE(0x05200000, 0x052001ff) AM_RAM
	AM_RANGE(0x05300000, 0x0530ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x05ff0000, 0x05ffffff) AM_RAM /*???*/
	AM_RANGE(0x06000000, 0x06000fff) AM_RAM
	AM_RANGE(0x06100000, 0x06100003) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x06100004, 0x06100007) AM_READ_PORT("IN1")
	AM_RANGE(0x06100010, 0x06100013) AM_READ_PORT("IN2")
	AM_RANGE(0x06100014, 0x06100017) AM_READ_PORT("IN3")
	AM_RANGE(0x0610001c, 0x0610001f) AM_READ_PORT("IN4") AM_WRITENOP
	AM_RANGE(0x06200000, 0x06200fff) AM_RAM
	AM_RANGE(0x07fff000, 0x07ffffff) AM_RAM
	AM_RANGE(0x20000000, 0x2001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

// SH-1 or SH-2 almost certainly copies the program down to here: the ROM containing the program is 32-bit wide and the 68000 is 16-bit
// the SCSP is believed to be hardcoded to decode the first 4 MB like this for a master/slave config
// (see also Model 3):
static ADDRESS_MAP_START( system_h1_sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM
//  AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("scsp1", scsp_r, scsp_w)
	AM_RANGE(0x200000, 0x27ffff) AM_RAM
//  AM_RANGE(0x300000, 0x300fff) AM_DEVREADWRITE("scsp2", scsp_r, scsp_w)
ADDRESS_MAP_END



static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*128
};

#if 0
static const gfx_layout test =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4, 7*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
};
#endif

static GFXDECODE_START( coolridr )
//	GFXDECODE_ENTRY( "maincpu_data", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx_data", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "ram_gfx", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static INPUT_PORTS_START( coolridr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN0-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN0-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN1-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	/* Service port*/
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00040000, IP_ACTIVE_LOW )
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN2-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN2-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN3-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN3-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN4-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN4-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN5-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN5-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// IRQs 4 & 6 are valid on SH-2
static INTERRUPT_GEN( system_h1 )
{
	cpu_set_input_line(device, 4, HOLD_LINE);
/*	switch(cpu_getiloops(device))
	{
      	case 0: break;
        case 1:cpu_set_input_line(device, 6, HOLD_LINE); break;
//      case 2:cpu_set_input_line(device, 8, HOLD_LINE); break;
	}*/
}

//IRQs 10,12 and 14 are valid on SH-1 instead
static INTERRUPT_GEN( system_h1_sub )
{
	switch(cpu_getiloops(device))
	{
      	case 0:cpu_set_input_line(device, 0xa, HOLD_LINE); break;
        case 1:cpu_set_input_line(device, 0xc, HOLD_LINE); break;
        case 2:cpu_set_input_line(device, 0xe, HOLD_LINE); break;
	}
}

static MACHINE_RESET ( coolridr )
{

//  cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_HALT, ASSERT_LINE);

}

static MACHINE_DRIVER_START( coolridr )
	MDRV_CPU_ADD("maincpu", SH2, 28000000)	// 28 mhz
	MDRV_CPU_PROGRAM_MAP(system_h1_map)
	MDRV_CPU_VBLANK_INT("screen",system_h1)

	MDRV_CPU_ADD("soundcpu", M68000, 16000000)	// 16 mhz
	MDRV_CPU_PROGRAM_MAP(system_h1_sound_map)

	MDRV_CPU_ADD("sub", SH1, 16000000)	// SH7032 HD6417032F20!! 16 mhz
	MDRV_CPU_PROGRAM_MAP(coolridr_submap)
	MDRV_CPU_VBLANK_INT_HACK(system_h1_sub, 3)

	MDRV_GFXDECODE(coolridr)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(128*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 128*8-1, 0*8, 64*8-1) //TODO: these are just two different screens

	MDRV_PALETTE_LENGTH(0x10000)
	MDRV_MACHINE_RESET(coolridr)

	MDRV_VIDEO_START(coolridr)
	MDRV_VIDEO_UPDATE(coolridr)
MACHINE_DRIVER_END

ROM_START( coolridr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "ep17659.30", 0x0000000, 0x080000, CRC(473027b0) SHA1(acaa212869dd79550235171b9f054e82750f74c3) )
	ROM_LOAD32_WORD_SWAP( "ep17658.29", 0x0000002, 0x080000, CRC(7ecfdfcc) SHA1(97cb3e6cf9764c8db06de12e4e958148818ef737) )
	ROM_LOAD32_WORD_SWAP( "ep17661.32", 0x0100000, 0x080000, CRC(81a7d90b) SHA1(99f8c3e75b94dd1b60455c26dc38ce08db82fe32) )
	ROM_LOAD32_WORD_SWAP( "ep17660.31", 0x0100002, 0x080000, CRC(27b7a507) SHA1(4c28b1d18d75630a73194b5d4fd166f3b647c595) )

	/* Page 12 of the service manual states that these 4 regions are tested, so I believe that they are read by the SH-2 */
	ROM_REGION32_BE( 0x1000000, "gfx_data", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "mp17650.11", 0x0000002, 0x0200000, CRC(0ccc84a1) SHA1(65951685b0c8073f6bd1cf9959e1b4d0fc6031d8) )
	ROM_LOAD32_WORD_SWAP( "mp17651.12", 0x0000000, 0x0200000, CRC(25fd7dde) SHA1(a1c3f3d947ce20fbf61ea7ab235259be9b7d35a8) )
	ROM_LOAD32_WORD_SWAP( "mp17652.13", 0x0400002, 0x0200000, CRC(be9b4d05) SHA1(0252ba647434f69d6eacb4efc6f55e6af534c7c5) )
	ROM_LOAD32_WORD_SWAP( "mp17653.14", 0x0400000, 0x0200000, CRC(64d1406d) SHA1(779dbbf42a14a6be1de9afbae5bbb18f8f36ceb3) )
	ROM_LOAD32_WORD_SWAP( "mp17654.15", 0x0800002, 0x0200000, CRC(5dee5cba) SHA1(6e6ec8574bdd35cc27903fc45f0d4a36ce9df103) )
	ROM_LOAD32_WORD_SWAP( "mp17655.16", 0x0800000, 0x0200000, CRC(02903cf2) SHA1(16d555fda144e0f1b62b428e9158a0e8ebf7084e) )
	ROM_LOAD32_WORD_SWAP( "mp17656.17", 0x0c00002, 0x0200000, CRC(945c89e3) SHA1(8776d74f73898d948aae3c446d7c710ad0407603) )
	ROM_LOAD32_WORD_SWAP( "mp17657.18", 0x0c00000, 0x0200000, CRC(74676b1f) SHA1(b4a9003a052bde93bebfa4bef9e8dff65003c3b2) )

	ROM_REGION32_BE( 0x100000, "ram_gfx", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_REGION( 0x100000, "soundcpu", ROMREGION_ERASE00 )	/* 68000 */
	/* uploaded by the main CPU */

	ROM_REGION( 0x100000, "sub", 0 ) /* SH1 */
	ROM_LOAD16_WORD_SWAP( "ep17662.12", 0x000000, 0x020000,  CRC(50d66b1f) SHA1(f7b7f2f5b403a13b162f941c338a3e1207762a0b) )

	/* these 10 interleave somehow?? */
	ROM_REGION( 0x1600000, "gfx5", ROMREGION_ERASEFF ) /* Other Roms */
	ROMX_LOAD( "mp17640.1", 0x0000000, 0x0200000, CRC(5ecd98c7) SHA1(22027c1e9e6195d27f29a5779695d8597f68809e), ROM_SKIP(9) )
	ROMX_LOAD( "mp17641.2", 0x0000001, 0x0200000, CRC(a59b0605) SHA1(c93f84fd58f1942b40b7a55058e02a18a3dec3af), ROM_SKIP(9) )
	ROMX_LOAD( "mp17642.3", 0x0000002, 0x0200000, CRC(5f8a1827) SHA1(23179d751777436f2a4f652132001d5e425d8cd5), ROM_SKIP(9) )
	ROMX_LOAD( "mp17643.4", 0x0000003, 0x0200000, CRC(44a05dd0) SHA1(32aa86f8761ec6ffceb63979c44828603c244e7d), ROM_SKIP(9) )
	ROMX_LOAD( "mp17644.5", 0x0000004, 0x0200000, CRC(be2763c5) SHA1(1044b0a73e334337b0b9ac958df59480aedfb942), ROM_SKIP(9) )
	ROMX_LOAD( "mp17645.6", 0x0000005, 0x0200000, CRC(00954173) SHA1(863f32565296448ef10992dc3c0480411eb2b193), ROM_SKIP(9) )
	ROMX_LOAD( "mp17646.7", 0x0000006, 0x0200000, CRC(7ae4d92e) SHA1(8a0eaa5dce112289ac5d16ad5dc7f5895e71e87b), ROM_SKIP(9) )
	ROMX_LOAD( "mp17647.8", 0x0000007, 0x0200000, CRC(082faee8) SHA1(c047b8475517f96f481c09471a77aa0d103631d6), ROM_SKIP(9) )
	ROMX_LOAD( "mp17648.9", 0x0000008, 0x0200000, CRC(0791802f) SHA1(acad55bbd22c7e955a729c8abed9509fc6f10927), ROM_SKIP(9) )
	ROMX_LOAD( "mp17649.10",0x0000009, 0x0200000, CRC(567fbc0a) SHA1(3999c99b26f13d97ac1c58de00a44049ee7775fd), ROM_SKIP(9) )
ROM_END

/*TODO: there must be an irq line with custom vector located somewhere that writes to here...*/
#if 0
static READ32_HANDLER( coolridr_hack1_r )
{
	return sysh1_workram_h[0xd88a4/4];
}
#endif

static READ32_HANDLER( coolridr_hack2_r )
{
	if(cpu_get_pc(space->cpu) == 0x6002cba || cpu_get_pc(space->cpu) == 0x6002d42)
		return 0;

	return sysh1_workram_h[0xd8894/4];
}

static DRIVER_INIT( coolridr )
{
//	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60d88a4, 0x060d88a7, 0, 0, coolridr_hack1_r );
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60d8894, 0x060d8897, 0, 0, coolridr_hack2_r );
}

GAME( 1995, coolridr,    0, coolridr,    coolridr,    coolridr, ROT0,  "Sega", "Cool Riders (US)",GAME_NOT_WORKING|GAME_NO_SOUND )
