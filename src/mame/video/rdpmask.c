#if defined(MASK_S)
	#if defined(MASK_T)
		INLINE void MASK_S_T(INT32* S, INT32* T, TILE* tex_tile)
	#else
		INLINE void MASK_S_NT(INT32* S, INT32* T, TILE* tex_tile)
	#endif
#else
	#if defined(MASK_T)
		INLINE void MASK_NS_T(INT32* S, INT32* T, TILE* tex_tile)
	#else
		INLINE void MASK_NS_NT(INT32* S, INT32* T, TILE* tex_tile)
	#endif
#endif
{
#if defined(MASK_S)
	INT32 swrap;
#endif
#if defined(MASK_T)
	INT32 twrap;
#endif

#if defined(MASK_S)
	swrap = *S >> tex_tile->mask_s;
	swrap &= 1;
	if (tex_tile->ms && swrap)
	{
		*S = (~(*S)) & maskbits_table[tex_tile->mask_s]; // Mirroring and masking
	}
	else
	{
		*S &= maskbits_table[tex_tile->mask_s]; // Masking
	}
#endif

#if defined(MASK_T)
	twrap = *T >> tex_tile->mask_t;
	twrap &= 1;
	if (tex_tile->mt && twrap)
	{
		*T = (~(*T)) & maskbits_table[tex_tile->mask_t]; // Mirroring and masking
	}
	else
	{
		*T &= maskbits_table[tex_tile->mask_t];
	}
#endif
}

