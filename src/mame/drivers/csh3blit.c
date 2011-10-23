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
	colour_t s_clr;
#endif

#ifdef BLENDED
	colour_t d_clr;

#if _SMODE == 2
#if _DMODE != 0
	colour_t clr0;
#endif
#elif _SMODE == 0
#if _DMODE != 0
#if _DMODE != 5
#if _DMODE != 1
	colour_t clr0;
#endif
#endif
#endif
#else
	colour_t clr0;
#endif


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


#ifdef BLENDED
#if _SMODE == 0
#if _DMODE == 0
	UINT8* salpha_table = cavesh3_colrtable[s_alpha];
	UINT8* dalpha_table = cavesh3_colrtable[d_alpha];
#endif

#if _DMODE == 5
	UINT8* salpha_table = cavesh3_colrtable[s_alpha];
#endif
#if _DMODE == 1
	UINT8* salpha_table = cavesh3_colrtable[s_alpha];
#endif

#endif

#if _SMODE == 2
#if _DMODE == 0

	UINT8* dalpha_table = cavesh3_colrtable[d_alpha];
#endif
#endif
#endif



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
			if (pen & 0x20000000)
			{
			*bmp = pen;
#else
			*bmp = *gfx2;
#endif

/*************** REGULAR INNER LOOPS ****************/
#else // NOT REALLY_SIMPLE

			pen = *gfx2;

#if TRANSPARENT == 1
			if (pen & 0x20000000)
			{
#endif

			// convert source to clr
			pen_to_clr(pen, &s_clr.trgb);
			//s_clr.u32 = (pen >> 3); // using the union is actually significantly slower than our pen_to_clr to function!
			// source * intesity and clamp

#ifdef TINT
			clr_mul(&s_clr.trgb, tint_clr);
#endif

			#ifdef BLENDED

				// convert destination to clr
				pen_to_clr(*bmp, &d_clr.trgb);
                //d_clr.u32 = *bmp >> 3; // using the union is actually significantly slower than our pen_to_clr to function!
				#if _SMODE == 0
				//g_profiler.start(PROFILER_USER7);


					#if _DMODE == 0
					//g_profiler.start(PROFILER_USER1);
					// this is used extensively in the games (ingame, futari title screens etc.)

					s_clr.trgb.r = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.r)]][dalpha_table[(d_clr.trgb.r)]];
					s_clr.trgb.g = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.g)]][dalpha_table[(d_clr.trgb.g)]];
					s_clr.trgb.b = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.b)]][dalpha_table[(d_clr.trgb.b)]];
					#elif _DMODE == 1
					//g_profiler.start(PROFILER_USER2);
					// futari ~7%
					s_clr.trgb.r = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.r)]][cavesh3_colrtable[(s_clr.trgb.r)][(d_clr.trgb.r)]];
					s_clr.trgb.g = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.g)]][cavesh3_colrtable[(s_clr.trgb.g)][(d_clr.trgb.g)]];
					s_clr.trgb.b = cavesh3_colrtable_add[salpha_table[(s_clr.trgb.b)]][cavesh3_colrtable[(s_clr.trgb.b)][(d_clr.trgb.b)]];
					#elif _DMODE == 2
					//g_profiler.start(PROFILER_USER3);
					clr_mul_fixed(&clr0.trgb, s_alpha, &s_clr.trgb);
					clr_add_with_clr_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
					#elif _DMODE == 3
					//g_profiler.start(PROFILER_USER4);
					clr_mul_fixed(&clr0.trgb, s_alpha, &s_clr.trgb);
					clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);

					#elif _DMODE == 4
					//g_profiler.start(PROFILER_USER5);
					clr_mul_fixed(&clr0.trgb, s_alpha, &s_clr.trgb);
					clr_add_with_clr_mul_fixed_rev(&s_clr.trgb, &clr0.trgb, d_alpha, &d_clr.trgb);
					#elif _DMODE == 5
					// futari black character select ~13%
					//g_profiler.start(PROFILER_USER6);
					s_clr.trgb.r =  cavesh3_colrtable_add[salpha_table[(s_clr.trgb.r)]][cavesh3_colrtable_rev[(s_clr.trgb.r)][(d_clr.trgb.r)]];
					s_clr.trgb.g =  cavesh3_colrtable_add[salpha_table[(s_clr.trgb.g)]][cavesh3_colrtable_rev[(s_clr.trgb.g)][(d_clr.trgb.g)]];
					s_clr.trgb.b =  cavesh3_colrtable_add[salpha_table[(s_clr.trgb.b)]][cavesh3_colrtable_rev[(s_clr.trgb.b)][(d_clr.trgb.b)]];

					#elif _DMODE == 6
					//g_profiler.start(PROFILER_USER7);
					clr_mul_fixed(&clr0.trgb, s_alpha, &s_clr.trgb);
					clr_add_with_clr_mul_rev_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
					#elif _DMODE == 7
					//g_profiler.start(PROFILER_USER8);
					clr_mul_fixed(&clr0.trgb, s_alpha, &s_clr.trgb);
					clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);
					#endif

				//g_profiler.stop();
				#elif _SMODE == 1
				//g_profiler.start(PROFILER_USER6);
				clr_square(&clr0.trgb, &s_clr.trgb);

				#elif _SMODE == 2
			//  g_profiler.start(PROFILER_USER4);
					#if _DMODE == 0
					// this is used heavily on espgal2 highscore screen (~28%) optimized to avoid use of temp clr0 variable
					s_clr.trgb.r = cavesh3_colrtable_add[cavesh3_colrtable[(d_clr.trgb.r)][(s_clr.trgb.r)]][dalpha_table[(d_clr.trgb.r)]];
					s_clr.trgb.g = cavesh3_colrtable_add[cavesh3_colrtable[(d_clr.trgb.g)][(s_clr.trgb.g)]][dalpha_table[(d_clr.trgb.g)]];
					s_clr.trgb.b = cavesh3_colrtable_add[cavesh3_colrtable[(d_clr.trgb.b)][(s_clr.trgb.b)]][dalpha_table[(d_clr.trgb.b)]];
					#elif _DMODE == 1
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add_with_clr_mul_3param(&s_clr.trgb, &clr0.trgb, &d_clr.trgb, &s_clr.trgb);
					#elif _DMODE == 2
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add_with_clr_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
					#elif _DMODE == 3
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);

					#elif _DMODE == 4
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add_with_clr_mul_fixed_rev(&s_clr.trgb, &clr0.trgb, d_alpha, &d_clr.trgb);
					#elif _DMODE == 5
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add_with_clr_mul_rev_3param(&s_clr.trgb, &clr0.trgb, &d_clr.trgb, &s_clr.trgb);
					#elif _DMODE == 6
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add_with_clr_mul_rev_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
					#elif _DMODE == 7
					clr_mul_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
					clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);
					#endif
				//g_profiler.stop();

				#elif _SMODE == 3
				//g_profiler.start(PROFILER_USER1);
				clr_copy(&clr0.trgb, &s_clr.trgb);

				#elif _SMODE == 4
				//g_profiler.start(PROFILER_USER2);
				clr_mul_fixed_rev(&clr0.trgb, s_alpha, &s_clr.trgb);
				#elif _SMODE == 5
				//g_profiler.start(PROFILER_USER3);
				clr_mul_rev_square(&clr0.trgb, &s_clr.trgb);
				#elif _SMODE == 6
				//g_profiler.start(PROFILER_USER4);
				clr_mul_rev_3param(&clr0.trgb, &s_clr.trgb, &d_clr.trgb);
				#elif _SMODE == 7
				//g_profiler.start(PROFILER_USER5);
				clr_copy(&clr0.trgb, &s_clr.trgb);
				#endif


// smode 0/2 cases are already split up and handled above.
#if _SMODE != 2
#if _SMODE != 0

				#if _DMODE == 0
				clr_add_with_clr_mul_fixed(&s_clr.trgb, &clr0.trgb, d_alpha, &d_clr.trgb);
				#elif _DMODE == 1
				clr_add_with_clr_mul_3param(&s_clr.trgb, &clr0.trgb, &d_clr.trgb, &s_clr.trgb);
				#elif _DMODE == 2
				clr_add_with_clr_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
				#elif _DMODE == 3
				clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);

				#elif _DMODE == 4
				clr_add_with_clr_mul_fixed_rev(&s_clr.trgb, &clr0.trgb, d_alpha, &d_clr.trgb);
				#elif _DMODE == 5
				clr_add_with_clr_mul_rev_3param(&s_clr.trgb, &clr0.trgb, &d_clr.trgb, &s_clr.trgb);
				#elif _DMODE == 6
				clr_add_with_clr_mul_rev_square(&s_clr.trgb, &clr0.trgb, &d_clr.trgb);
				#elif _DMODE == 7
				clr_add(&s_clr.trgb, &s_clr.trgb, &d_clr.trgb);
				#endif

				//g_profiler.stop();
#endif
#endif


			#endif

			// write result
			*bmp = clr_to_pen(&s_clr.trgb)|(pen&0x20000000);
			//*bmp = (s_clr.u32<<3)|(pen&0x20000000); // using the union is actually significantly slower than our clr_to_pen function!

#endif // END NOT REALLY SIMPLE

#if TRANSPARENT == 1
			}
#endif
			LOOP_INCREMENTS
		}

	}

//  g_profiler.stop();
}

#undef LOOP_INCREMENTS
