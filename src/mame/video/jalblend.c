/***************************************************************************

    Jaleco color blend emulation

****************************************************************************

    This implements the behaviour of color blending/alpha hardware
    found in a small set of machines from the late 80's.

    Thus far, Psychic 5, Argus, and Valtric are presumed to use it.

****************************************************************************/


/* each palette entry contains a fourth 'alpha' value */
static UINT8 *jal_blend_table;

/*
 * 'Alpha' Format
 * ------------------
 *
 * Bytes     | Use
 * -76543210-+----------------
 *  xxxx---- | unknown - maybe special background?
 *  ----x--- | unknown - same as above
 *  -----x-- | red add/subtract
 *  ------x- | green add/subtract
 *  -------x | blue add/subtract
 */

/* basically an add/subtract function with clamping */
static rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, UINT8 alpha)
{
	int r  = (float)RGB_RED  (addMe) ;
	int g  = (float)RGB_GREEN(addMe) ;
	int b  = (float)RGB_BLUE (addMe) ;

	int rd = (float)RGB_RED  (dest) ;
	int gd = (float)RGB_GREEN(dest) ;
	int bd = (float)RGB_BLUE (dest) ;

	int finalR, finalG, finalB ;

	int subR = (alpha & 0x04) >> 2 ;
	int subG = (alpha & 0x02) >> 1 ;
	int subB = (alpha & 0x01) ;

	if (subR) finalR = rd - r ;
	else      finalR = rd + r ;
	if (finalR < 0) finalR = 0 ;
	else if (finalR > 255) finalR = 255 ;


	if (subG) finalG = gd - g ;
	else      finalG = gd + g ;
	if (finalG < 0) finalG = 0 ;
	else if (finalG > 255) finalG = 255 ;


	if (subB) finalB = bd - b ;
	else      finalB = bd + b ;
	if (finalB < 0) finalB = 0 ;
	else if (finalB > 255) finalB = 255 ;

	return MAKE_RGB((UINT8)finalR,(UINT8)finalG,(UINT8)finalB) ;
}

static void jal_blend_drawgfx( mame_bitmap *dest_bmp,const gfx_element *gfx,
							   UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
							   const rectangle *clip,int transparency,int transparent_color)
{
	/* drawgfx(dest_bmp, gfx, code, color, flipx, flipy, offsx, offsy, clip, TRANSPARENCY_PEN, 15); */

	int xstart, ystart, xend, yend, xinc, yinc;
	int xtile, ytile ;
	int code_offset = 0;

	int wide = 1 ;
	int high = 1 ;

	if (flipx)	{ xstart = wide-1; xend = -1;   xinc = -1; }
	else		{ xstart = 0;      xend = wide; xinc = +1; }

	if (flipy)	{ ystart = high-1; yend = -1;   yinc = -1; }
	else		{ ystart = 0;      yend = high; yinc = +1; }

	/* Start drawing */
	if( gfx )
	{
		for (ytile = ystart; ytile != yend; ytile += yinc )
		{
			for (xtile = xstart; xtile != xend; xtile += xinc )
			{
				const pen_t *pal = &Machine->remapped_colortable[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
				const UINT8 *alpha = &jal_blend_table[gfx->color_granularity * (color % gfx->total_colors)];
				int source_base = ((code + code_offset++) % gfx->total_elements) * gfx->height;

				int x_index_base, y_index, sx, sy, ex, ey;

				if (flipx)	{ x_index_base = gfx->width-1; }
				else		{ x_index_base = 0; }

				if (flipy)	{ y_index = gfx->height-1; }
				else		{ y_index = 0; }

				/* start coordinates */
				sx = offsx + xtile*gfx->width;
				sy = offsy + ytile*gfx->height;

				/* end coordinates */
				ex = sx + gfx->width;
				ey = sy + gfx->height;

				if( clip )
				{
					if( sx < clip->min_x)
					{ /* clip left */
						int pixels = clip->min_x-sx;
						sx += pixels;
						x_index_base += xinc*pixels;
					}
					if( sy < clip->min_y )
					{ /* clip top */
						int pixels = clip->min_y-sy;
						sy += pixels;
						y_index += yinc*pixels;
					}
					/* NS 980211 - fixed incorrect clipping */
					if( ex > clip->max_x+1 )
					{ /* clip right */
						int pixels = ex-clip->max_x-1;
						ex -= pixels;
					}
					if( ey > clip->max_y+1 )
					{ /* clip bottom */
						int pixels = ey-clip->max_y-1;
						ey -= pixels;
					}
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* taken from case 7: TRANSPARENCY_ALPHARANGE */
					UINT8 *source = gfx->gfxdata + (source_base + y_index)*gfx->line_modulo + x_index_base;
					UINT32 *dest = (UINT32 *)dest_bmp->base + sy*dest_bmp->rowpixels + sx;
					int src_modulo = yinc*gfx->line_modulo - xinc*(ex-sx);
					int dst_modulo = dest_bmp->rowpixels - (ex-sx);

					for( y=sy; y<ey; y++ )
					{
						int x;
						for( x=sx; x<ex; x++ )
						{
							int c = *source;
							if( c != transparent_color )
							{
								if( alpha[c] == 0x00 )
								{
									/* Skip the costly alpha step altogether */
									*dest = pal[c];
								}
								else
								{
									/* Comp with clamp */
									*dest = jal_blend_func(*dest, pal[c], alpha[c]) ;
								}
							}
							dest++;
							source += xinc;
						}
						dest += dst_modulo;
						source += src_modulo;
					}
				}
			}
		}
	}
}
