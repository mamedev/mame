// #included multiple times from cavesh3.c



INLINE void FUNCNAME(bitmap_t *bitmap,
					 const rectangle *clip,
					 UINT32 *gfx, int gfx_size,
					 int src_p,
					 int src_x,
					 int src_y,
					 int dst_x_start,
					 int dst_y_start,
					 int dimx,
					 int dimy,
					 int flipx,
					 int flipy,
					 clr_t *s_alpha_clr,
					 clr_t *d_alpha_clr,
					 clr_t *tint_clr )
{

	//logerror("draw sprite %04x %04x - %04x %04x\n", dst_x_start, dst_y_start, dimx, dimy);

	int x,y, xf,yf;
	clr_t s_clr;
#ifdef BLENDED	
	clr_t d_clr, clr0, clr1;
#endif
	UINT32 pen;
	UINT32 *bmp;

	if (flipx)	{	xf = -1;	src_x += (dimx-1);	}
	else		{	xf = +1;						}

	if (flipy)	{	yf = -1;	src_y += (dimy-1);	}
	else		{	yf = +1;						}

	int starty = 0;
	int dst_y_end = dst_y_start+dimy;

	if (dst_y_start < clip->min_y)
		starty = clip->min_y - dst_y_start;

	if (dst_y_end > clip->max_y)
		dimy -= (dst_y_end-1) - clip->max_y;


	for (y = starty; y < dimy; y++)
	{

		int startx = 0;
		int dst_x_end = dst_x_start+dimx;

		if (dst_x_start < clip->min_x)
			startx = clip->min_x - dst_x_start;

		if (dst_x_end > clip->max_x)
			dimx -= (dst_x_end-1) - clip->max_x;

		bmp = BITMAP_ADDR32(bitmap, dst_y_start + y, dst_x_start+startx);


		for (x = startx; x < dimx; x++, bmp++)
		{
			pen = gfx[GFX_OFFSET(src_x,src_y, xf * x, yf * y)/* & (gfx_size-1)*/]; // no need to mask, already masked in function!

#if TRANSPARENT == 1
			if ((pen & 0x20000000) == 0)
			{
				continue;
			}
#endif
			// convert source to clr
			pen_to_clr(pen, &s_clr);
			// source * intesity and clamp
			clr_mul(&s_clr, tint_clr, &s_clr);
			clamp_clr(&s_clr);


			#ifdef BLENDED

				// convert destination to clr
				pen_to_clr(*bmp, &d_clr);
                
				#if _SMODE == 0
				clr0.r = s_alpha_clr->r;
				clr0.g = s_alpha_clr->g;
				clr0.b = s_alpha_clr->b;
				#elif _SMODE == 1
				clr0.r = s_clr.r;
				clr0.g = s_clr.g;
				clr0.b = s_clr.b;
				#elif _SMODE == 2
				clr0.r = d_clr.r;
				clr0.g = d_clr.g;
				clr0.b = d_clr.b;
				#elif _SMODE == 3
				clr0.r = 0x1f;
				clr0.g = 0x1f;
				clr0.b = 0x1f;
				#elif _SMODE == 4
				clr0.r = s_alpha_clr->r^0x1f;
				clr0.g = s_alpha_clr->g^0x1f;
				clr0.b = s_alpha_clr->b^0x1f;
				#elif _SMODE == 5
				clr0.r = s_clr.r^0x1f;
				clr0.g = s_clr.g^0x1f;
				clr0.b = s_clr.b^0x1f;
				#elif _SMODE == 6
				clr0.r = d_clr.r^0x1f;
				clr0.g = d_clr.g^0x1f;
				clr0.b = d_clr.b^0x1f;
				#elif _SMODE == 7
				clr0.r = 0x1f;
				clr0.g = 0x1f;
				clr0.b = 0x1f;
				#endif

				clr_mul(&clr0, &s_clr, &clr0); 
				
				#if _DMODE == 0
				clr1.r = d_alpha_clr->r;
				clr1.g = d_alpha_clr->g;
				clr1.b = d_alpha_clr->b;
				#elif _DMODE == 1
				clr1.r = s_clr.r;
				clr1.g = s_clr.g;
				clr1.b = s_clr.b;
				#elif _DMODE == 2
				clr1.r = d_clr.r;
				clr1.g = d_clr.g;
				clr1.b = d_clr.b;
				#elif _DMODE == 3
				clr1.r = 0x1f;
				clr1.g = 0x1f;
				clr1.b = 0x1f;
				#elif _DMODE == 4
				clr1.r = d_alpha_clr->r^0x1f;
				clr1.g = d_alpha_clr->g^0x1f;
				clr1.b = d_alpha_clr->b^0x1f;
				#elif _DMODE == 5
				clr1.r = s_clr.r^0x1f;
				clr1.g = s_clr.g^0x1f;
				clr1.b = s_clr.b^0x1f;
				#elif _DMODE == 6
				clr1.r = d_clr.r^0x1f;
				clr1.g = d_clr.g^0x1f;
				clr1.b = d_clr.b^0x1f;
				#elif _DMODE == 7
				clr1.r = 0x1f;
				clr1.g = 0x1f;
				clr1.b = 0x1f;
				#endif

				clr_mul(&clr1, &d_clr, &clr1);
				
				// blend (add) into source
				clr_add(&clr0, &clr1, &s_clr);
				clamp_clr(&s_clr);
			#endif

			// write result
			*bmp = clr_to_pen(&s_clr)|(pen&0x20000000);


		}
	}
}
