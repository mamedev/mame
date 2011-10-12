// #included multiple times from cavesh3.c



const void FUNCNAME(bitmap_t *bitmap,
					 const rectangle *clip,
					 UINT32 *gfx,
					 int src_x,
					 int src_y,
					 int dst_x_start,
					 int dst_y_start,
					 int dimx,
					 int dimy,
					 int flipy,
					 const UINT8 s_alpha,
					 const UINT8 d_alpha,
					// int tint,
					 clr_t *tint_clr )
{


	UINT32* gfx2;
	int x,y, yf;
	clr_t s_clr;
#ifdef BLENDED	
	clr_t d_clr, clr0, clr1;
#endif
	UINT32 pen;
	UINT32 *bmp;

#ifdef FLIPX
	src_x += (dimx-1);
#endif

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
		int ysrc_index =  ((src_y + yf * y) & 0x0fff) * 0x2000;
		gfx2 = gfx + ysrc_index;

		for (x = startx; x < dimx; x++, bmp++)
		{

#ifdef FLIPX
			pen = gfx2[((src_x - x) & 0x1fff)];
#else
			pen = gfx2[((src_x + x) & 0x1fff)];
#endif

#if TRANSPARENT == 1
			if ((pen & 0x20000000) == 0)
			{
				continue;
			}
#endif
			// convert source to clr
			pen_to_clr(pen, &s_clr);
			// source * intesity and clamp
			
			//if (tint)
			//{
				clr_mul(&s_clr, tint_clr);
				//clamp_clr(&s_clr); // table used by clr_mul is pre-clamped
			//}

			#ifdef BLENDED

				// convert destination to clr
				pen_to_clr(*bmp, &d_clr);
                
				#if _SMODE == 0
				clr_mul_fixed(&clr0, s_alpha, &s_clr); 
				#elif _SMODE == 1
				clr0.r = s_clr.r;
				clr0.g = s_clr.g;
				clr0.b = s_clr.b;
				clr_mul(&clr0, &s_clr); 
				#elif _SMODE == 2
				clr0.r = d_clr.r;
				clr0.g = d_clr.g;
				clr0.b = d_clr.b;
				clr_mul(&clr0, &s_clr); 
				#elif _SMODE == 3
				clr_copy(&clr0, &s_clr);
				
				#elif _SMODE == 4
				clr_mul_fixed_rev(&clr0, s_alpha, &s_clr); 
				#elif _SMODE == 5
				clr0.r = s_clr.r;
				clr0.g = s_clr.g;
				clr0.b = s_clr.b;
				clr_mul_rev(&clr0, &s_clr); 
				#elif _SMODE == 6
				clr0.r = d_clr.r;
				clr0.g = d_clr.g;
				clr0.b = d_clr.b;
				clr_mul_rev(&clr0, &s_clr); 
				#elif _SMODE == 7
				clr_copy(&clr0, &s_clr);
				#endif

				
				
				#if _DMODE == 0
				clr_mul_fixed(&clr1, d_alpha, &d_clr);
				#elif _DMODE == 1
				clr1.r = s_clr.r;
				clr1.g = s_clr.g;
				clr1.b = s_clr.b;
				clr_mul(&clr1, &d_clr);
				#elif _DMODE == 2
				clr1.r = d_clr.r;
				clr1.g = d_clr.g;
				clr1.b = d_clr.b;
				clr_mul(&clr1, &d_clr);
				#elif _DMODE == 3
				clr_copy(&clr1, &d_clr);
				
				#elif _DMODE == 4
				clr_mul_fixed_rev(&clr1, d_alpha, &d_clr);
				#elif _DMODE == 5
				clr1.r = s_clr.r;
				clr1.g = s_clr.g;
				clr1.b = s_clr.b;
				clr_mul_rev(&clr1, &d_clr);
				#elif _DMODE == 6
				clr1.r = d_clr.r;
				clr1.g = d_clr.g;
				clr1.b = d_clr.b;
				clr_mul_rev(&clr1, &d_clr);
				#elif _DMODE == 7
				clr_copy(&clr1, &d_clr);
				#endif

				
				
				// blend (add) into source
				clr_add(&s_clr, &clr0, &clr1);
				//clamp_clr(&s_clr); // our add is pre-clamped
			#endif

			// write result
			*bmp = clr_to_pen(&s_clr)|(pen&0x20000000);


		}
	}
}
