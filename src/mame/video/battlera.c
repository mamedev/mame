/*******************************************************************************

    Battle Rangers - Bryan McPhail, mish@tendril.co.uk

    This file only implements necessary features - not all PC-Engine video
    features are used in this game (no DMA for one).

*******************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/h6280/h6280.h"
#include "includes/battlera.h"

static int HuC6270_registers[20];
static int VDC_register,vram_ptr;
static UINT8 *HuC6270_vram,*vram_dirty;
static bitmap_t *tile_bitmap,*front_bitmap;
static UINT32 tile_dirtyseq;

static int current_scanline,inc_value;
static int irq_enable,rcr_enable,sb_enable,bb_enable,bldwolf_vblank;

static UINT8 blank_tile[32];


/******************************************************************************/

VIDEO_START( battlera )
{
	HuC6270_vram=auto_alloc_array(machine, UINT8, 0x20000);
	vram_dirty=auto_alloc_array(machine, UINT8, 0x1000);

	memset(HuC6270_vram,0,0x20000);
	memset(vram_dirty,1,0x1000);

	tile_bitmap=auto_bitmap_alloc(machine,512,512,machine->primary_screen->format());
	front_bitmap=auto_bitmap_alloc(machine,512,512,machine->primary_screen->format());

	vram_ptr=0;
	inc_value=1;
	current_scanline=0;
	irq_enable=rcr_enable=sb_enable=bb_enable=0;

	gfx_element_set_source(machine->gfx[0], HuC6270_vram);
	gfx_element_set_source(machine->gfx[1], HuC6270_vram);
	gfx_element_set_source(machine->gfx[2], blank_tile);
}

/******************************************************************************/

WRITE8_HANDLER( battlera_palette_w )
{
	int pal_word;

	space->machine->generic.paletteram.u8[offset]=data;
	if (offset%2) offset-=1;

	pal_word=space->machine->generic.paletteram.u8[offset] | (space->machine->generic.paletteram.u8[offset+1]<<8);
	palette_set_color_rgb(space->machine, offset/2, pal3bit(pal_word >> 3), pal3bit(pal_word >> 6), pal3bit(pal_word >> 0));
}

/******************************************************************************/

READ8_HANDLER( HuC6270_debug_r )
{
	return HuC6270_vram[offset];
}

WRITE8_HANDLER( HuC6270_debug_w )
{
	HuC6270_vram[offset]=data;
}
READ8_HANDLER( HuC6270_register_r )
{
	int rr;

	if ((current_scanline+56)==HuC6270_registers[6]) rr=1; else rr=0;

	return 0		/* CR flag */
		| (0 << 1)	/* OR flag */
		| (rr << 2)	/* RR flag */
		| (0 << 3)	/* DS flag */
		| (0 << 4)	/* DV flag */
		| (bldwolf_vblank << 5)	/* VD flag (1 when vblank else 0) */
		| (0 << 6)	/* BSY flag (1 when dma active, else 0) */
		| (0 << 7);	/* Always zero */
}

WRITE8_HANDLER( HuC6270_register_w )
{
	switch (offset) {
	case 0: /* Select data region */
		VDC_register=data;
		break;
	case 1: /* Unused */
		break;
	}
}

/******************************************************************************/

#ifdef UNUSED_FUNCTION
READ8_HANDLER( HuC6270_data_r )
{
	int result;

	switch (offset) {
		case 0: /* LSB */
			return HuC6270_vram[(HuC6270_registers[1]<<1)|1];

		case 1:/* MSB */
			result=HuC6270_vram[(HuC6270_registers[1]<<1)|0];
			HuC6270_registers[1]=(HuC6270_registers[1]+inc_value)&0xffff;
			return result;
	}

	return 0;
}
#endif

WRITE8_HANDLER( HuC6270_data_w )
{
	switch (offset) {
		case 0: /* LSB */
			switch (VDC_register) {

			case 0: /* MAWR */
				HuC6270_registers[0]=(HuC6270_registers[0]&0xff00) | (data);
				return;

			case 1: /* MARR */
				HuC6270_registers[0]=(HuC6270_registers[1]&0xff00) | (data);
				return;

			case 2: /* VRAM */
				if (HuC6270_vram[(HuC6270_registers[0]<<1)|1]!=data) {
					HuC6270_vram[(HuC6270_registers[0]<<1)|1]=data;
					gfx_element_mark_dirty(space->machine->gfx[0], HuC6270_registers[0]>>4);
					gfx_element_mark_dirty(space->machine->gfx[1], HuC6270_registers[0]>>6);
				}
				if (HuC6270_registers[0]<0x1000) vram_dirty[HuC6270_registers[0]]=1;
				return;

			case 3: break; /* Unused */
			case 4: break; /* Unused */

			case 5: /* CR - Control register */
				/* Bits 0,1 unknown */
				rcr_enable=data&0x4; /* Raster interrupt enable */
				irq_enable=data&0x8; /* VBL interrupt enable */
				/* Bits 4,5 unknown (EX) */
				sb_enable=data&0x40; /* Sprites enable */
				bb_enable=data&0x80; /* Background enable */
				return;

			case 6: /* RCR - Raster counter register */
				HuC6270_registers[6]=(HuC6270_registers[6]&0xff00) | (data);
				return;

			case 7: /* BXR - X scroll */
				HuC6270_registers[7]=(HuC6270_registers[7]&0xff00) | (data);
				return;

			case 8: /* BYR - Y scroll */
				HuC6270_registers[8]=(HuC6270_registers[8]&0xff00) | (data);
				return;

			case 15: /* DMA */
			case 16:
			case 17:
			case 18:
				logerror("%04x: dma 2 %02x\n",cpu_get_pc(space->cpu),data);
				break;

			case 19: /* SATB */
				HuC6270_registers[19]=(HuC6270_registers[19]&0xff00) | (data);
				return;

			}
			break;

		/*********************************************/

		case 1: /* MSB (Autoincrement on this write) */
			switch (VDC_register) {

			case 0: /* MAWR - Memory Address Write Register */
				HuC6270_registers[0]=(HuC6270_registers[0]&0xff) | (data<<8);
				return;

			case 1: /* MARR */
				HuC6270_registers[1]=(HuC6270_registers[1]&0xff) | (data<<8);
				return;

			case 2: /* VWR - VRAM */
				if (HuC6270_vram[(HuC6270_registers[0]<<1)|0]!=data) {
					HuC6270_vram[(HuC6270_registers[0]<<1)|0]=data;
					gfx_element_mark_dirty(space->machine->gfx[0], HuC6270_registers[0]>>4);
					gfx_element_mark_dirty(space->machine->gfx[1], HuC6270_registers[0]>>6);
					if (HuC6270_registers[0]<0x1000) vram_dirty[HuC6270_registers[0]]=1;
				}
				HuC6270_registers[0]+=inc_value;
				HuC6270_registers[0]=HuC6270_registers[0]&0xffff;
				return;

			case 5: /* CR */
				/* IW - Auto-increment values */
				switch ((data>>3)&3) {
					case 0: inc_value=1; break;
					case 1: inc_value=32;break;
					case 2: inc_value=64; break;
					case 3: inc_value=128; break;
				}

				/* DR, TE unknown */
				return;

			case 6: /* RCR - Raster counter register */
				HuC6270_registers[6]=(HuC6270_registers[6]&0xff) | (data<<8);
				return;

			case 7: /* BXR - X scroll */
				HuC6270_registers[7]=(HuC6270_registers[7]&0xff) | (data<<8);
						return;

			case 8: /* BYR - Y scroll */
				HuC6270_registers[8]=(HuC6270_registers[8]&0xff) | (data<<8);
				return;

			case 15: /* DMA */
			case 16:
			case 17:
			case 18:
				logerror("%04x: dma 2 %02x\n",cpu_get_pc(space->cpu),data);
				break;

			case 19: /* SATB - Sprites */
				HuC6270_registers[19]=(HuC6270_registers[19]&0xff) | (data<<8);
				return;
			}
			break;
	}
	logerror("%04x: unknown write to  VDC_register %02x (%02x) at %02x\n",cpu_get_pc(space->cpu),VDC_register,data,offset);
}

/******************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *clip,int pri)
{
	int offs,my,mx,code,code2,fx,fy,cgy=0,cgx,colour,i,yinc;

	/* Draw sprites, starting at SATB, draw in _reverse_ order */
	for (offs=(HuC6270_registers[19]<<1)+0x200-8; offs>=(HuC6270_registers[19]<<1); offs-=8)
	{
		if ((HuC6270_vram[offs+7]&0x80) && !pri) continue;
		if (!(HuC6270_vram[offs+7]&0x80) && pri) continue;

		code=HuC6270_vram[offs+5] + (HuC6270_vram[offs+4]<<8);
		code=code>>1;

		my=HuC6270_vram[offs+1] + (HuC6270_vram[offs+0]<<8);
		mx=HuC6270_vram[offs+3] + (HuC6270_vram[offs+2]<<8);

		mx-=32;
		my-=57;

		fx=HuC6270_vram[offs+6]&0x8;
		fy=HuC6270_vram[offs+6]&0x80;
		cgx=HuC6270_vram[offs+6]&1;
		colour=HuC6270_vram[offs+7]&0xf;

		switch ((HuC6270_vram[offs+6]>>4)&3) {
		case 0: cgy=1; break;
		case 1: cgy=2; break;
		case 2: cgy=0; break; /* Illegal */
		case 3: cgy=4; break;
		}

		if (cgx && cgy==2) code=code&0x3fc; /* Title screen */

		if (fx && cgx) {code2=code; code++;} /* Swap tile order on X flips */
		else code2=code+1;

		yinc = 16;
		if (fy) { my += 16*(cgy-1); yinc = -16; } /* Swap tile order on Y flips */

		for (i=0; i<cgy; i++) {
			drawgfx_transpen(bitmap,clip,machine->gfx[1],
				code,
				colour,
				fx,fy,
				mx,my,0);

			if (cgx)
				drawgfx_transpen(bitmap,clip,machine->gfx[1],
						code2,
						colour,
						fx,fy,
						mx+16,my,0);
			my += yinc;
			/* if (cgx) */ /* Different from console? */
			code += 2;
			code2 += 2;
			/*else code += 1; */ /* Different from console? */
		}
	}

}

/******************************************************************************/

VIDEO_UPDATE( battlera )
{
	int offs,code,scrollx,scrolly,mx,my;

	/* if any tiles changed, redraw the VRAM */
	if (screen->machine->gfx[0]->dirtyseq != tile_dirtyseq)
	{
		tile_dirtyseq = screen->machine->gfx[0]->dirtyseq;
		memset(vram_dirty, 1, 0x1000);
	}

	mx=-1;
	my=0;
	for (offs = 0x0000;offs < 0x2000;offs += 2)
	{
		mx++;
		if (mx==64) {mx=0; my++;}
		code=HuC6270_vram[offs+1] + ((HuC6270_vram[offs] & 0x0f) << 8);

		/* If this tile was changed OR tilemap was changed, redraw */
		if (vram_dirty[offs/2]) {
			vram_dirty[offs/2]=0;
			drawgfx_opaque(tile_bitmap,0,screen->machine->gfx[0],
					code,
					HuC6270_vram[offs] >> 4,
					0,0,
					8*mx,8*my);
			drawgfx_opaque(front_bitmap,0,screen->machine->gfx[2],
					0,
					0,	/* fill the spot with pen 256 */
					0,0,
					8*mx,8*my);
			drawgfx_transmask(front_bitmap,0,screen->machine->gfx[0],
					code,
					HuC6270_vram[offs] >> 4,
					0,0,
					8*mx,8*my,0x1);
		}
	}

	/* Render bitmap */
	scrollx=-HuC6270_registers[7];
	scrolly=-HuC6270_registers[8]+cliprect->min_y-1;

	copyscrollbitmap(bitmap,tile_bitmap,1,&scrollx,1,&scrolly,cliprect);

	/* Todo:  Background enable (not used anyway) */

	/* Render low priority sprites, if enabled */
	if (sb_enable) draw_sprites(screen->machine,bitmap,cliprect,0);

	/* Render background over sprites */
	copyscrollbitmap_trans(bitmap,front_bitmap,1,&scrollx,1,&scrolly,cliprect,256);

	/* Render high priority sprites, if enabled */
	if (sb_enable) draw_sprites(screen->machine,bitmap,cliprect,1);

	return 0;
}

/******************************************************************************/

INTERRUPT_GEN( battlera_interrupt )
{
	current_scanline=255-cpu_getiloops(device); /* 8 lines clipped at top */

	/* If raster interrupt occurs, refresh screen _up_ to this point */
	if (rcr_enable && (current_scanline+56)==HuC6270_registers[6]) {
		device->machine->primary_screen->update_partial(current_scanline);
		cpu_set_input_line(device, 0, HOLD_LINE); /* RCR interrupt */
	}

	/* Start of vblank */
	else if (current_scanline==240) {
		bldwolf_vblank=1;
		device->machine->primary_screen->update_partial(240);
		if (irq_enable)
			cpu_set_input_line(device, 0, HOLD_LINE); /* VBL */
	}

	/* End of vblank */
	if (current_scanline==254) {
		bldwolf_vblank=0;
	}
}
