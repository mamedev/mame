/*******************************************************************************

    Battle Rangers - Bryan McPhail, mish@tendril.co.uk

    This file only implements necessary features - not all PC-Engine video
    features are used in this game (no DMA for one).

*******************************************************************************/

#include "emu.h"
#include "cpu/h6280/h6280.h"
#include "includes/battlera.h"


/******************************************************************************/

VIDEO_START( battlera )
{
	battlera_state *state = machine.driver_data<battlera_state>();
	state->m_HuC6270_vram=auto_alloc_array(machine, UINT8, 0x20000);
	state->m_vram_dirty=auto_alloc_array(machine, UINT8, 0x1000);

	memset(state->m_HuC6270_vram,0,0x20000);
	memset(state->m_vram_dirty,1,0x1000);

	state->m_tile_bitmap=auto_bitmap_alloc(machine,512,512,machine.primary_screen->format());
	state->m_front_bitmap=auto_bitmap_alloc(machine,512,512,machine.primary_screen->format());

	state->m_vram_ptr=0;
	state->m_inc_value=1;
	state->m_current_scanline=0;
	state->m_irq_enable=state->m_rcr_enable=state->m_sb_enable=state->m_bb_enable=0;

	gfx_element_set_source(machine.gfx[0], state->m_HuC6270_vram);
	gfx_element_set_source(machine.gfx[1], state->m_HuC6270_vram);
	gfx_element_set_source(machine.gfx[2], state->m_blank_tile);
}

/******************************************************************************/

WRITE8_HANDLER( battlera_palette_w )
{
	int pal_word;

	space->machine().generic.paletteram.u8[offset]=data;
	if (offset%2) offset-=1;

	pal_word=space->machine().generic.paletteram.u8[offset] | (space->machine().generic.paletteram.u8[offset+1]<<8);
	palette_set_color_rgb(space->machine(), offset/2, pal3bit(pal_word >> 3), pal3bit(pal_word >> 6), pal3bit(pal_word >> 0));
}

/******************************************************************************/

READ8_HANDLER( HuC6270_debug_r )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	return state->m_HuC6270_vram[offset];
}

WRITE8_HANDLER( HuC6270_debug_w )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	state->m_HuC6270_vram[offset]=data;
}
READ8_HANDLER( HuC6270_register_r )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	int rr;

	if ((state->m_current_scanline+56)==state->m_HuC6270_registers[6]) rr=1; else rr=0;

	return 0		/* CR flag */
		| (0 << 1)	/* OR flag */
		| (rr << 2)	/* RR flag */
		| (0 << 3)	/* DS flag */
		| (0 << 4)	/* DV flag */
		| (state->m_bldwolf_vblank << 5)	/* VD flag (1 when vblank else 0) */
		| (0 << 6)	/* BSY flag (1 when dma active, else 0) */
		| (0 << 7);	/* Always zero */
}

WRITE8_HANDLER( HuC6270_register_w )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	switch (offset) {
	case 0: /* Select data region */
		state->m_VDC_register=data;
		break;
	case 1: /* Unused */
		break;
	}
}

/******************************************************************************/

#ifdef UNUSED_FUNCTION
READ8_HANDLER( HuC6270_data_r )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	int result;

	switch (offset) {
		case 0: /* LSB */
			return state->m_HuC6270_vram[(state->m_HuC6270_registers[1]<<1)|1];

		case 1:/* MSB */
			result=state->m_HuC6270_vram[(state->m_HuC6270_registers[1]<<1)|0];
			state->m_HuC6270_registers[1]=(state->m_HuC6270_registers[1]+state->m_inc_value)&0xffff;
			return result;
	}

	return 0;
}
#endif

WRITE8_HANDLER( HuC6270_data_w )
{
	battlera_state *state = space->machine().driver_data<battlera_state>();
	switch (offset) {
		case 0: /* LSB */
			switch (state->m_VDC_register) {

			case 0: /* MAWR */
				state->m_HuC6270_registers[0]=(state->m_HuC6270_registers[0]&0xff00) | (data);
				return;

			case 1: /* MARR */
				state->m_HuC6270_registers[0]=(state->m_HuC6270_registers[1]&0xff00) | (data);
				return;

			case 2: /* VRAM */
				if (state->m_HuC6270_vram[(state->m_HuC6270_registers[0]<<1)|1]!=data) {
					state->m_HuC6270_vram[(state->m_HuC6270_registers[0]<<1)|1]=data;
					gfx_element_mark_dirty(space->machine().gfx[0], state->m_HuC6270_registers[0]>>4);
					gfx_element_mark_dirty(space->machine().gfx[1], state->m_HuC6270_registers[0]>>6);
				}
				if (state->m_HuC6270_registers[0]<0x1000) state->m_vram_dirty[state->m_HuC6270_registers[0]]=1;
				return;

			case 3: break; /* Unused */
			case 4: break; /* Unused */

			case 5: /* CR - Control register */
				/* Bits 0,1 unknown */
				state->m_rcr_enable=data&0x4; /* Raster interrupt enable */
				state->m_irq_enable=data&0x8; /* VBL interrupt enable */
				/* Bits 4,5 unknown (EX) */
				state->m_sb_enable=data&0x40; /* Sprites enable */
				state->m_bb_enable=data&0x80; /* Background enable */
				return;

			case 6: /* RCR - Raster counter register */
				state->m_HuC6270_registers[6]=(state->m_HuC6270_registers[6]&0xff00) | (data);
				return;

			case 7: /* BXR - X scroll */
				state->m_HuC6270_registers[7]=(state->m_HuC6270_registers[7]&0xff00) | (data);
				return;

			case 8: /* BYR - Y scroll */
				state->m_HuC6270_registers[8]=(state->m_HuC6270_registers[8]&0xff00) | (data);
				return;

			case 15: /* DMA */
			case 16:
			case 17:
			case 18:
				logerror("%04x: dma 2 %02x\n",cpu_get_pc(&space->device()),data);
				break;

			case 19: /* SATB */
				state->m_HuC6270_registers[19]=(state->m_HuC6270_registers[19]&0xff00) | (data);
				return;

			}
			break;

		/*********************************************/

		case 1: /* MSB (Autoincrement on this write) */
			switch (state->m_VDC_register) {

			case 0: /* MAWR - Memory Address Write Register */
				state->m_HuC6270_registers[0]=(state->m_HuC6270_registers[0]&0xff) | (data<<8);
				return;

			case 1: /* MARR */
				state->m_HuC6270_registers[1]=(state->m_HuC6270_registers[1]&0xff) | (data<<8);
				return;

			case 2: /* VWR - VRAM */
				if (state->m_HuC6270_vram[(state->m_HuC6270_registers[0]<<1)|0]!=data) {
					state->m_HuC6270_vram[(state->m_HuC6270_registers[0]<<1)|0]=data;
					gfx_element_mark_dirty(space->machine().gfx[0], state->m_HuC6270_registers[0]>>4);
					gfx_element_mark_dirty(space->machine().gfx[1], state->m_HuC6270_registers[0]>>6);
					if (state->m_HuC6270_registers[0]<0x1000) state->m_vram_dirty[state->m_HuC6270_registers[0]]=1;
				}
				state->m_HuC6270_registers[0]+=state->m_inc_value;
				state->m_HuC6270_registers[0]=state->m_HuC6270_registers[0]&0xffff;
				return;

			case 5: /* CR */
				/* IW - Auto-increment values */
				switch ((data>>3)&3) {
					case 0: state->m_inc_value=1; break;
					case 1: state->m_inc_value=32;break;
					case 2: state->m_inc_value=64; break;
					case 3: state->m_inc_value=128; break;
				}

				/* DR, TE unknown */
				return;

			case 6: /* RCR - Raster counter register */
				state->m_HuC6270_registers[6]=(state->m_HuC6270_registers[6]&0xff) | (data<<8);
				return;

			case 7: /* BXR - X scroll */
				state->m_HuC6270_registers[7]=(state->m_HuC6270_registers[7]&0xff) | (data<<8);
						return;

			case 8: /* BYR - Y scroll */
				state->m_HuC6270_registers[8]=(state->m_HuC6270_registers[8]&0xff) | (data<<8);
				return;

			case 15: /* DMA */
			case 16:
			case 17:
			case 18:
				logerror("%04x: dma 2 %02x\n",cpu_get_pc(&space->device()),data);
				break;

			case 19: /* SATB - Sprites */
				state->m_HuC6270_registers[19]=(state->m_HuC6270_registers[19]&0xff) | (data<<8);
				return;
			}
			break;
	}
	logerror("%04x: unknown write to  VDC_register %02x (%02x) at %02x\n",cpu_get_pc(&space->device()),state->m_VDC_register,data,offset);
}

/******************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *clip,int pri)
{
	battlera_state *state = machine.driver_data<battlera_state>();
	int offs,my,mx,code,code2,fx,fy,cgy=0,cgx,colour,i,yinc;

	/* Draw sprites, starting at SATB, draw in _reverse_ order */
	for (offs=(state->m_HuC6270_registers[19]<<1)+0x200-8; offs>=(state->m_HuC6270_registers[19]<<1); offs-=8)
	{
		if ((state->m_HuC6270_vram[offs+7]&0x80) && !pri) continue;
		if (!(state->m_HuC6270_vram[offs+7]&0x80) && pri) continue;

		code=state->m_HuC6270_vram[offs+5] + (state->m_HuC6270_vram[offs+4]<<8);
		code=code>>1;

		my=state->m_HuC6270_vram[offs+1] + (state->m_HuC6270_vram[offs+0]<<8);
		mx=state->m_HuC6270_vram[offs+3] + (state->m_HuC6270_vram[offs+2]<<8);

		mx-=32;
		my-=57;

		fx=state->m_HuC6270_vram[offs+6]&0x8;
		fy=state->m_HuC6270_vram[offs+6]&0x80;
		cgx=state->m_HuC6270_vram[offs+6]&1;
		colour=state->m_HuC6270_vram[offs+7]&0xf;

		switch ((state->m_HuC6270_vram[offs+6]>>4)&3) {
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
			drawgfx_transpen(bitmap,clip,machine.gfx[1],
				code,
				colour,
				fx,fy,
				mx,my,0);

			if (cgx)
				drawgfx_transpen(bitmap,clip,machine.gfx[1],
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

SCREEN_UPDATE( battlera )
{
	battlera_state *state = screen->machine().driver_data<battlera_state>();
	int offs,code,scrollx,scrolly,mx,my;

	/* if any tiles changed, redraw the VRAM */
	if (screen->machine().gfx[0]->dirtyseq != state->m_tile_dirtyseq)
	{
		state->m_tile_dirtyseq = screen->machine().gfx[0]->dirtyseq;
		memset(state->m_vram_dirty, 1, 0x1000);
	}

	mx=-1;
	my=0;
	for (offs = 0x0000;offs < 0x2000;offs += 2)
	{
		mx++;
		if (mx==64) {mx=0; my++;}
		code=state->m_HuC6270_vram[offs+1] + ((state->m_HuC6270_vram[offs] & 0x0f) << 8);

		/* If this tile was changed OR tilemap was changed, redraw */
		if (state->m_vram_dirty[offs/2]) {
			state->m_vram_dirty[offs/2]=0;
			drawgfx_opaque(state->m_tile_bitmap,0,screen->machine().gfx[0],
					code,
					state->m_HuC6270_vram[offs] >> 4,
					0,0,
					8*mx,8*my);
			drawgfx_opaque(state->m_front_bitmap,0,screen->machine().gfx[2],
					0,
					0,	/* fill the spot with pen 256 */
					0,0,
					8*mx,8*my);
			drawgfx_transmask(state->m_front_bitmap,0,screen->machine().gfx[0],
					code,
					state->m_HuC6270_vram[offs] >> 4,
					0,0,
					8*mx,8*my,0x1);
		}
	}

	/* Render bitmap */
	scrollx=-state->m_HuC6270_registers[7];
	scrolly=-state->m_HuC6270_registers[8]+cliprect->min_y-1;

	copyscrollbitmap(bitmap,state->m_tile_bitmap,1,&scrollx,1,&scrolly,cliprect);

	/* Todo:  Background enable (not used anyway) */

	/* Render low priority sprites, if enabled */
	if (state->m_sb_enable) draw_sprites(screen->machine(),bitmap,cliprect,0);

	/* Render background over sprites */
	copyscrollbitmap_trans(bitmap,state->m_front_bitmap,1,&scrollx,1,&scrolly,cliprect,256);

	/* Render high priority sprites, if enabled */
	if (state->m_sb_enable) draw_sprites(screen->machine(),bitmap,cliprect,1);

	return 0;
}

/******************************************************************************/

TIMER_DEVICE_CALLBACK( battlera_irq )
{
	battlera_state *state = timer.machine().driver_data<battlera_state>();
	state->m_current_scanline = param; /* 8 lines clipped at top */

	/* If raster interrupt occurs, refresh screen _up_ to this point */
	if (state->m_rcr_enable && (state->m_current_scanline+56)==state->m_HuC6270_registers[6]) {
		timer.machine().primary_screen->update_partial(state->m_current_scanline);
		device_set_input_line(state->m_maincpu, 0, HOLD_LINE); /* RCR interrupt */
	}

	/* Start of vblank */
	else if (state->m_current_scanline==240) {
		state->m_bldwolf_vblank=1;
		timer.machine().primary_screen->update_partial(240);
		if (state->m_irq_enable)
			device_set_input_line(state->m_maincpu, 0, HOLD_LINE); /* VBL */
	}

	/* End of vblank */
	if (state->m_current_scanline==254) {
		state->m_bldwolf_vblank=0;
	}
}
