#if defined(ZMODE0)
	#if defined(ANTIALIAS)
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_AA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_AA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_NAA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_NAA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#endif
#elif defined(ZMODE1)
	#if defined(ANTIALIAS)
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_AA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_AA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_NAA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_NAA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#endif
#elif defined(ZMODE2)
	#if defined(ANTIALIAS)
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_AA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_AA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_NAA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_NAA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#endif
#elif defined(ZMODE3)
	#if defined(ANTIALIAS)
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_AA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_AA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#else
		#if defined(IMGREAD)
			INLINE UINT32 z_compare_IMR_NAA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#else
			INLINE UINT32 z_compare_NIMR_NAA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
		#endif
	#endif
#endif
{
	int force_coplanar = 0;
	UINT32 oz = z_decompress(zb);
	UINT32 dzmem = dz_decompress(zb, zhb);
	UINT32 dznew = 0;
	UINT32 farther = 0;
	UINT32 diff = 0;
	UINT32 nearer = 0;
	UINT32 infront = 0;
	UINT32 max = 0;
	int precision_factor = (oz >> 15) & 0xf;
	int overflow = 0;
#if defined(ZMODE1)
	int cvgcoeff = 0;
#endif
	UINT32 mempixel, memory_cvg;

	sz &= 0x3ffff;
	if (dzmem == 0x8000 && precision_factor < 3)
	{
		force_coplanar = 1;
	}
	if (!precision_factor)
	{
		dzmem = ((dzmem << 1) > 16) ? (dzmem << 1) : 16;
	}
	else if (precision_factor == 1)
	{
		dzmem = ((dzmem << 1) > 8) ? (dzmem << 1) : 8;
	}
	else if (precision_factor == 2)
	{
		dzmem = ((dzmem << 1) > 4) ? (dzmem << 1) : 4;
	}
	if (dzmem == 0 && precision_factor < 3)
	{
		dzmem = 0xffff;
	}
	if (dzmem > 0x8000)
	{
		dzmem = 0xffff;
	}
	dznew =((dzmem > dzpix) ? dzmem : (UINT32)dzpix) << 3;
	dznew &= 0x3ffff;

	farther = ((sz + dznew) >= oz) ? 1 : 0;
	diff = (sz >= dznew) ? (sz - dznew) : 0;
	nearer = (diff <= oz) ? 1: 0;
	infront = (sz < oz) ? 1 : 0;
	max = (dzmem == 0x3ffff);

	if (force_coplanar)
	{
		farther = nearer = 1;
	}

	curpixel_overlap = 0;

	switch(fb_size)
	{
		case 1: /* Banjo Tooie */
			memory_cvg = 0; //??
			break;
		case 2:
			mempixel = *(UINT16*)fb;
			memory_cvg = ((mempixel & 1) << 2) + (*hb & 3);
			break;
		case 3:
			mempixel = *(UINT32*)fb;
			memory_cvg = (mempixel >> 5) & 7;
			break;
		default:
			fatalerror("z_compare: fb_size = %d",fb_size);
			break;
	}

#if !defined(IMGREAD)
	memory_cvg = 7;
#endif

	overflow = ((memory_cvg + curpixel_cvg - 1) > 7) ? 1 : 0;

#if defined(ANTIALIAS)
	curpixel_overlap = (other_modes.force_blend || (!overflow && farther));
#else
	curpixel_overlap = other_modes.force_blend;
#endif

#if defined(ZMODE1)
	if (infront && farther && overflow)
	{
		cvgcoeff = ((dzmem >> dznew) - (sz >> dznew)) & 0xf;
		curpixel_cvg = ((cvgcoeff * (curpixel_cvg - 1)) >> 3) & 0xf;
	}
#endif
	if (curpixel_cvg > 8)
		curpixel_cvg = 8;

#if defined(ZMODE0)
	return (max || (overflow ? infront : nearer))? 1 : 0;
#elif defined(ZMODE1)
	return (max || (overflow ? infront : nearer))? 1 : 0;
#elif defined(ZMODE2)
	return infront | max;
#elif defined(ZMODE3)
	return farther & nearer & !max;
#else
	return 0;
#endif
}

