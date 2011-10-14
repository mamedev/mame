// #included multiple times from cavesh3.c

#ifdef FLIPX
#define LOOP_INCREMENTS \
			bmp++;  \
			gfx2--; \

#else     

#define LOOP_INCREMENTS \
			bmp++;  \
			gfx2++; \

#endif               

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
	int y, yf;

#ifndef REALLY_SIMPLE
	clr_t s_clr;
#endif

#ifdef BLENDED	
	clr_t d_clr, clr0;
#endif

#ifdef REALLY_SIMPLE
#if TRANSPARENT == 1
	UINT32 pen;
#endif
#else
	UINT32 pen;
#endif
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
	
	// check things are safe to draw (note, if the source would wrap round an edge of the 0x2000*0x1000 vram we don't draw.. not sure what the hw does anyway)
#ifdef FLIPX
	if ((src_x &0x1fff) < ((src_x-(dimx-1))&0x1fff))
	{
		printf("sprite gets clipped off src_x %04x dimx %04x\n", src_x, dimx);
		return;
	}
#else
	if ((src_x &0x1fff) > ((src_x+(dimx-1))&0x1fff))
	{
		printf("sprite gets clipped off src_x %04x dimx %04x\n", src_x, dimx);
		return;
	}
#endif

	int startx = 0;
	int dst_x_end = dst_x_start+dimx;

	if (dst_x_start < clip->min_x)
		startx = clip->min_x - dst_x_start;

	if (dst_x_end > clip->max_x)
		dimx -= (dst_x_end-1) - clip->max_x;
	

//	g_profiler.start(PROFILER_USER2);

	for (y = starty; y < dimy; y++)
	{
		bmp = BITMAP_ADDR32(bitmap, dst_y_start + y, dst_x_start+startx);
		int ysrc_index =  ((src_y + yf * y) & 0x0fff) * 0x2000;
		gfx2 = gfx + ysrc_index;

		#ifdef FLIPX
			gfx2 += (src_x-startx);
		#else
			gfx2 += (src_x+startx);
		#endif

		const UINT32* end = bmp+(dimx-startx);

		while (bmp<end)
		{

			


/*************** REALLY SIMPLE INNER LOOP, NON-BLENDED, NON-TINTED, SIMPLEST CASE ****************/
#ifdef REALLY_SIMPLE

#if TRANSPARENT == 1
			pen = *gfx2;
			if (!(pen & 0x20000000))
			{
				LOOP_INCREMENTS
				continue;
			}
			*bmp = pen;
#else
			*bmp = *gfx2;
#endif

/*************** REGULAR INNER LOOPS ****************/		
#else // NOT REALLY_SIMPLE

			pen = *gfx2;

#if TRANSPARENT == 1
			if (!(pen & 0x20000000))
			{
				LOOP_INCREMENTS
				continue;
			}
#endif

			// convert source to clr
			pen_to_clr(pen, &s_clr);
			// source * intesity and clamp
			
			clr_mul(&s_clr, tint_clr);

			#ifdef BLENDED

				// convert destination to clr
				pen_to_clr(*bmp, &d_clr);
                
				#if _SMODE == 0
				clr_mul_fixed(&clr0, s_alpha, &s_clr); 
				#elif _SMODE == 1
				clr_square(&clr0, &s_clr); 
				#elif _SMODE == 2
				clr_mul_3param(&clr0, &s_clr, &d_clr); 
				#elif _SMODE == 3
				clr_copy(&clr0, &s_clr);
				
				#elif _SMODE == 4
				clr_mul_fixed_rev(&clr0, s_alpha, &s_clr); 
				#elif _SMODE == 5
				clr_mul_rev_square(&clr0, &s_clr); 
				#elif _SMODE == 6
				clr_mul_rev_3param(&clr0, &s_clr, &d_clr); 
				#elif _SMODE == 7
				clr_copy(&clr0, &s_clr);
				#endif

				
				
				#if _DMODE == 0
				clr_add_with_clr_mul_fixed(&s_clr, &clr0, d_alpha, &d_clr);
				#elif _DMODE == 1
				clr_add_with_clr_mul_3param(&s_clr, &clr0, &d_clr, &s_clr);
				#elif _DMODE == 2
				clr_add_with_clr_square(&s_clr, &clr0, &d_clr); 
				#elif _DMODE == 3
				clr_add(&s_clr, &s_clr, &d_clr);
				
				#elif _DMODE == 4
				clr_add_with_clr_mul_fixed_rev(&s_clr, &clr0, d_alpha, &d_clr);
				#elif _DMODE == 5
				clr_add_with_clr_mul_rev_3param(&s_clr, &clr0, &d_clr, &s_clr);
				#elif _DMODE == 6
				clr_add_with_clr_mul_rev_square(&s_clr, &clr0, &d_clr);
				#elif _DMODE == 7
				clr_add(&s_clr, &s_clr, &d_clr);
				#endif

			#endif

			// write result
			*bmp = clr_to_pen(&s_clr)|(pen&0x20000000);
			

#endif // END NOT REALLY SIMPLE
			LOOP_INCREMENTS
		}
		
	}

//	g_profiler.stop();
}

#undef LOOP_INCREMENTS
