#if defined(SHIFT_S)
	#if defined(SHIFT_T)
		static void TEXSHIFT_S_T(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile)
	#else
		static void TEXSHIFT_S_NT(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile)
	#endif
#else
	#if defined(SHIFT_T)
		static void TEXSHIFT_NS_T(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile)
	#else
		static void TEXSHIFT_NS_NT(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile)
	#endif
#endif
{
#if defined(SHIFT_S)
	*S = SIGN16(*S);
	if (tex_tile->shift_s < 11)
	{
		*S >>= tex_tile->shift_s;
	}
	else
	{
		*S <<= (16 - tex_tile->shift_s);
	}
	*S = SIGN16(*S);
#endif
#if defined(SHIFT_T)
	*T = SIGN16(*T);
	if (tex_tile->shift_t < 11)
	{
		*T >>= tex_tile->shift_t;
	}
	else
	{
		*T <<= (16 - tex_tile->shift_t);
	}
	*T = SIGN16(*T);
#endif
	*maxs = ((*S >> 3) >= tex_tile->sh);
	*maxt = ((*T >> 3) >= tex_tile->th);
}
