#if defined(FSAA)
	#if defined(DIVOT)
		static void video_update_n64_16_fsaa_divot(bitmap_t *bitmap)
	#else
		static void video_update_n64_16_fsaa_nodivot(bitmap_t *bitmap)
	#endif
#else
	#if defined(DIVOT)
		static void video_update_n64_16_nofsaa_divot(bitmap_t *bitmap)
	#else
		static void video_update_n64_16_nofsaa_nodivot(bitmap_t *bitmap)
	#endif
#endif
{
	int i, j;
	UINT32 final = 0;
#if defined(DIVOT)
	UINT32 prev_cvg = 0, next_cvg = 0;
#endif
    //int dither_filter = (n64_vi_control >> 16) & 1;
    //int vibuffering = ((n64_vi_control & 2) && fsaa && divot);

	UINT16 *frame_buffer;
	UINT32 hb;
	UINT8* hidden_buffer;

	UINT32 pixels = 0;
	UINT16 pix = 0;

	INT32 hdiff = (n64_vi_hstart & 0x3ff) - ((n64_vi_hstart >> 16) & 0x3ff);
	float hcoeff = ((float)(n64_vi_xscale & 0xfff) / (1 << 10));
	UINT32 hres = ((float)hdiff * hcoeff);
	INT32 invisiblewidth = n64_vi_width - hres;

	INT32 vdiff = ((n64_vi_vstart & 0x3ff) - ((n64_vi_vstart >> 16) & 0x3ff)) >> 1;
	float vcoeff = ((float)(n64_vi_yscale & 0xfff) / (1 << 10));
	UINT32 vres = ((float)vdiff * vcoeff);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return;
	}

	frame_buffer = (UINT16*)&rdram[(n64_vi_origin & 0xffffff) >> 2];
	hb = ((n64_vi_origin & 0xffffff) >> 2) >> 1;
	hidden_buffer = &hidden_bits[hb];

	if (hres > 640) // Needed by Top Gear Overdrive (E)
	{
		invisiblewidth += (hres - 640);
		hres = 640;
	}

	pixels = 0;

	if (frame_buffer)
	{
		for (j=0; j < vres; j++)
		{
			UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);

			for (i=0; i < hres; i++)
			{
				int r, g, b;

				pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
				curpixel_cvg = ((pix & 1) << 2) | (hidden_buffer[pixels ^ BYTE_ADDR_XOR] & 3);

#if defined(DIVOT)
				if (i > 0 && i < (hres - 1))
				{
					prev_cvg = ((frame_buffer[(pixels - 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels - 1)^BYTE_ADDR_XOR] & 3);
					next_cvg = ((frame_buffer[(pixels + 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels + 1)^BYTE_ADDR_XOR] & 3);
				}
#endif
				r = ((pix >> 8) & 0xf8) | (pix >> 13);
				g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);

#if defined(FSAA)
				if (/*!vibuffering &&*/ curpixel_cvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2))
				{
					video_filter16(&r, &g, &b, &frame_buffer[pixels ^ WORD_ADDR_XOR],&hidden_buffer[pixels ^ BYTE_ADDR_XOR], n64_vi_width);
				}
#endif
				//else if (dither_filter && curpixel_cvg == 7 && i > 0 && j > 0 && i < (hres - 1) && j < (vres - 1))
				//{
					//if (vibuffering)
					//{
					//	restore_filter16_buffer(&r, &g, &b, &ViBuffer[i][j], n64_vi_width);
					//}
					//else
					//{
						//restore_filter16(&r, &g, &b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR, n64_vi_width);
					//}
				//}
#if defined(DIVOT)
				if (i > 0 && i < (hres - 1) && (curpixel_cvg != 7 || prev_cvg != 7 || next_cvg != 7))
				{
					//if (vibuffering)
					//{
					//	divot_filter16_buffer(&r, &g, &b, &ViBuffer[i][j]);
					//}
					//else
					//{
						divot_filter16(&r, &g, &b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR);
					//}
				}
#endif

				/*
				if (gamma_dither)
				{
					dith = mame_rand(screen->machine) & 0x3f;
				}
				if (gamma)
				{
					if (gamma_dither)
					{
						r = gamma_dither_table[(r << 6)|dith];
						g = gamma_dither_table[(g << 6)|dith];
						b = gamma_dither_table[(b << 6)|dith];
					}
					else
					{
						r = gamma_table[r];
						g = gamma_table[g];
						b = gamma_table[b];
					}
				}
				else if (gamma_dither)
				{
					if (r < 255)
						r += (dith & 1);
					if (g < 255)
						g += (dith & 1);
					if (b < 255)
						b += (dith & 1);
				}
				*/
				pixels++;

				final = (r << 16) | (g << 8) | b;
				d[i] = final; // Fix me for endianness
			}
			pixels +=invisiblewidth;
		}
	}
}
