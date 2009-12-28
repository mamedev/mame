#if defined(DOS)
	#if defined(DOT)
		INLINE void CLAMP_NC_DOS_DOT(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#else
		INLINE void CLAMP_NC_DOS_NDOT(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#endif
#else
	#if defined(DOT)
		INLINE void CLAMP_NC_NDOS_DOT(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#else
		INLINE void CLAMP_NC_NDOS_NDOT(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#endif
#endif
{
	//int dosfrac = (tex_tile->cs || !tex_tile->mask_s);
	//int dotfrac = (tex_tile->ct || !tex_tile->mask_t);
#if defined(DOS)
	if (*S & 0x10000)
	{
		*S = 0;
		*SFRAC = 0;
	}
	else if (maxs)
	{
		*S = clamp_s_diff[tex_tile->num];
		*SFRAC = 0;
	}
	else
#endif
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

#if defined(DOT)
	if (*T & 0x10000)
	{
		*T = 0;
		*TFRAC = 0;
	}
	else if (maxt)
	{
		*T = clamp_t_diff[tex_tile->num];
		*TFRAC = 0;
	}
	else
#endif
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

#if defined(DOS)
	#if defined(DOT)
		INLINE void CLAMP_LIGHT_NC_DOS_DOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#else
		INLINE void CLAMP_LIGHT_NC_DOS_NDOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#endif
#else
	#if defined(DOT)
		INLINE void CLAMP_LIGHT_NC_NDOS_DOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#else
		INLINE void CLAMP_LIGHT_NC_NDOS_NDOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, TILE* tex_tile)
	#endif
#endif
{
	//int dos = (tex_tile->cs || !tex_tile->mask_s);
	//int dot = (tex_tile->ct || !tex_tile->mask_t);
#if defined(DOS)
	if (*S & 0x10000)
	{
		*S = 0;
	}
	else if (maxs)
	{
		*S = clamp_s_diff[tex_tile->num];
	}
	else
#endif
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

#if defined(DOT)
	if (*T & 0x10000)
	{
		*T = 0;
	}
	else if (maxt)
	{
		*T = clamp_t_diff[tex_tile->num];
	}
	else
#endif
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

#if defined(DOS)
	#if defined(DOT)
		INLINE void CLAMP_QUICK_NC_DOS_DOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, int num)
	#else
		INLINE void CLAMP_QUICK_NC_DOS_NDOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, int num)
	#endif
#else
	#if defined(DOT)
		INLINE void CLAMP_QUICK_NC_NDOS_DOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, int num)
	#else
		INLINE void CLAMP_QUICK_NC_NDOS_NDOT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, int num)
	#endif
#endif
{
#if defined(DOS)
	if (*S & 0x10000)
	{
		*S = 0;
	}
	else if (maxs)
	{
		*S = clamp_s_diff[num];
	}
	else
#endif
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

#if defined(DOT)
	if (*T & 0x10000)
	{
		*T = 0;
	}
	else if (maxt)
	{
		*T = clamp_t_diff[num];
	}
	else
#endif
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

#if defined(DOS) && defined(DOT)
INLINE void CLAMP_C(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, TILE* tex_tile)
{
	*S = (SIGN17(*S) >> 5) & 0x1fff;
	*T = (SIGN17(*T) >> 5) & 0x1fff;
}

INLINE void CLAMP_LIGHT_C(INT32* S, INT32* T, INT32 maxs, INT32 maxt, TILE* tex_tile)
{
	*S = (SIGN17(*S) >> 5) & 0x1fff;
	*T = (SIGN17(*T) >> 5) & 0x1fff;
}

INLINE void CLAMP_QUICK_C(INT32* S, INT32* T, INT32 maxs, INT32 maxt, int num)
{
	*S = (SIGN17(*S) >> 5) & 0x1fff;
	*T = (SIGN17(*T) >> 5) & 0x1fff;
}
#endif
