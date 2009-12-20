#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDEN_CVGD0_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDEN_CVGD0_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	if (!other_modes.force_blend && !curpixel_overlap)
	{
		*fb = finalcolor|((curpixel_cvg >>2)&1);
		*hb = (curpixel_cvg & 3);
	}
	else
	{
		*fb = finalcolor|((clampcvg>>2)&1);
		*hb = (clampcvg&3);
	}
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDEN_CVGD1_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDEN_CVGD1_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|((newcvg >> 2) & 1);
	*hb = (newcvg & 3);
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDEN_CVGD2_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDEN_CVGD2_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|1;
	*hb = 3;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDEN_CVGD3_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDEN_CVGD3_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|((memory_cvg >> 2) & 1);
	*hb = (memory_cvg & 3);
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD0_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD0_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	if (!other_modes.force_blend && !curpixel_overlap)
	{
		*fb = finalcolor|((curpixel_cvg >>2)&1);
		*hb = (curpixel_cvg & 3);
	}
	else
	{
		*fb = finalcolor|((clampcvg>>2)&1);
		*hb = (clampcvg&3);
	}
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD1_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD1_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|1;
	*hb = 3;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD2_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD2_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|1;
	*hb = 3;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD3_COC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_16_RDNEN_CVGD3_NCOC(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}
#endif

	*fb = finalcolor|1;
	*hb = 4;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDEN_CVGD0_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDEN_CVGD0_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_cvg = ((*fb >>5) & 7) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	if (!other_modes.force_blend && !curpixel_overlap)
	{
		*fb = finalcolor|(curpixel_cvg << 5);
	}
	else
	{
		*fb = finalcolor|(clampcvg << 5);
	}
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDEN_CVGD1_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDEN_CVGD1_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_cvg = ((*fb >>5) & 7) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | (newcvg << 5);
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDEN_CVGD2_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDEN_CVGD2_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_cvg = ((*fb >>5) & 7) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | 0xE0;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDEN_CVGD3_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDEN_CVGD3_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_alphachannel = *fb & 0xff;
	UINT32 memory_cvg = ((*fb >>5) & 7) + 1;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | memory_alphachannel;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD0_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD0_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	if (!other_modes.force_blend && !curpixel_overlap)
	{
		*fb = finalcolor|(curpixel_cvg << 5);
	}
	else
	{
		*fb = finalcolor|(clampcvg << 5);
	}
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD1_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD1_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | (newcvg << 5);
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD2_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD2_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | 0xE0;
	return 1;
}

#if defined(COLOR_ON_CVG)
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD3_COC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#else
	INLINE UINT32 FBWRITE_32_RDNEN_CVGD3_NCOC(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
#endif
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_alphachannel = *fb & 0xff;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;

	newcvg = curpixel_cvg + 8;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	clampcvg--;

#if defined(COLOR_ON_CVG)
	if (!wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}
#endif

	*fb = finalcolor | memory_alphachannel;
	return 1;
}
