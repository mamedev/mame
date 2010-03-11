#define RELATIVE(x, y)	(((((x) >> 3) - (y)) << 3) | (x & 7))

	#if defined(COPY)
		#define CLAMP(SSS, SST, maxs, maxt) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#else

	#if defined(DOS) && defined(DOT)
		#define CLAMP(SSS, SST, maxs, maxt) \
			if (SSS & 0x10000) \
			{ \
				SSS = 0; \
				SFRAC = 0; \
			} \
			else if (maxs) \
			{ \
				SSS = clamp_s_diff[tex_tile->num]; \
				SFRAC = 0; \
			} \
			else \
			{ \
				SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			} \
 \
			if (SST & 0x10000) \
			{ \
				SST = 0; \
				TFRAC = 0; \
			} \
			else if (maxt) \
			{ \
				SST = clamp_t_diff[tex_tile->num]; \
				TFRAC = 0; \
			} \
			else \
			{ \
				SST = (SIGN17(SST) >> 5) & 0x1fff; \
			}
	#elif defined(DOS) && !defined(DOT)
		#define CLAMP(SSS, SST, maxs, maxt) \
			if (SSS & 0x10000) \
			{ \
				SSS = 0; \
				SFRAC = 0; \
			} \
			else if (maxs) \
			{ \
				SSS = clamp_s_diff[tex_tile->num]; \
				SFRAC = 0; \
			} \
			else \
			{ \
				SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			} \
 \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#elif !defined(DOS) && defined(DOT)
		#define CLAMP(SSS, SST, maxs, maxt) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
 \
			if (SST & 0x10000) \
			{ \
				SST = 0; \
				TFRAC = 0; \
			} \
			else if (maxt) \
			{ \
				SST = clamp_t_diff[tex_tile->num]; \
				TFRAC = 0; \
			} \
			else \
			{ \
				SST = (SIGN17(SST) >> 5) & 0x1fff; \
			}
	#else
		#define CLAMP(SSS, SST, maxs, maxt) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#endif
	#endif

	#if defined(COPY)
		#define CLAMP_LIGHT(SSS, SST) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#else
	#if defined(DOS) && defined(DOT)
		#define CLAMP_LIGHT(SSS, SST) \
			if (SSS & 0x10000) \
			{ \
				SSS = 0; \
			} \
			else if (maxs) \
			{ \
				SSS = clamp_s_diff[tex_tile->num]; \
			} \
			else \
			{ \
				SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			} \
 \
			if (SST & 0x10000) \
			{ \
				SST = 0; \
			} \
			else if (maxt) \
			{ \
				SST = clamp_t_diff[tex_tile->num]; \
			} \
			else \
			{ \
				SST = (SIGN17(SST) >> 5) & 0x1fff; \
			}
	#elif defined(DOS) && !defined(DOT)
		#define CLAMP_LIGHT(SSS, SST) \
			if (SSS & 0x10000) \
			{ \
				SSS = 0; \
			} \
			else if (maxs) \
			{ \
				SSS = clamp_s_diff[tex_tile->num]; \
			} \
			else \
			{ \
				SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			} \
 \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#elif !defined(DOS) && defined(DOT)
		#define CLAMP_LIGHT(SSS, SST) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
 \
			if (SST & 0x10000) \
			{ \
				SST = 0; \
			} \
			else if (maxt) \
			{ \
				SST = clamp_t_diff[tex_tile->num]; \
			} \
			else \
			{ \
				SST = (SIGN17(SST) >> 5) & 0x1fff; \
			}
	#else
		#define CLAMP_LIGHT(SSS, SST) \
			SSS = (SIGN17(SSS) >> 5) & 0x1fff; \
			SST = (SIGN17(SST) >> 5) & 0x1fff;
	#endif
	#endif

	#if defined(SHIFT_S) && defined(SHIFT_T)
		#define TEXSHIFT(SSS, SST, maxs, maxt) \
			SSS = SIGN16(SSS); \
			if (tex_tile->shift_s < 11) \
			{ \
				SSS >>= tex_tile->shift_s; \
			} \
			else \
			{ \
				SSS <<= (16 - tex_tile->shift_s); \
			} \
			SSS = SIGN16(SSS); \
			SST = SIGN16(SST); \
			if (tex_tile->shift_t < 11) \
			{ \
				SST >>= tex_tile->shift_t; \
			} \
			else \
			{ \
				SST <<= (16 - tex_tile->shift_t); \
			} \
			SST = SIGN16(SST); \
			maxs = ((SSS >> 3) >= tex_tile->sh); \
			maxt = ((SST >> 3) >= tex_tile->th);
	#elif defined(SHIFT_S) && !defined(SHIFT_T)
		#define TEXSHIFT(SSS, SST, maxs, maxt) \
			SSS = SIGN16(SSS); \
			if (tex_tile->shift_s < 11) \
			{ \
				SSS >>= tex_tile->shift_s; \
			} \
			else \
			{ \
				SSS <<= (16 - tex_tile->shift_s); \
			} \
			SSS = SIGN16(SSS); \
			maxs = ((SSS >> 3) >= tex_tile->sh); \
			maxt = ((SST >> 3) >= tex_tile->th);
	#elif !defined(SHIFT_S) && defined(SHIFT_T)
		#define TEXSHIFT(SSS, SST, maxs, maxt) \
			SST = SIGN16(SST); \
			if (tex_tile->shift_t < 11) \
			{ \
				SST >>= tex_tile->shift_t; \
			} \
			else \
			{ \
				SST <<= (16 - tex_tile->shift_t); \
			} \
			SST = SIGN16(SST); \
			maxs = ((SSS >> 3) >= tex_tile->sh); \
			maxt = ((SST >> 3) >= tex_tile->th);
	#else
		#define TEXSHIFT(SSS, SST, maxs, maxt) \
			maxs = ((SSS >> 3) >= tex_tile->sh); \
			maxt = ((SST >> 3) >= tex_tile->th);
	#endif

#if defined(SHIFT_T)
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#else
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_C_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_NMID_NC_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#endif
{
	INT32 maxs, maxt;
	int sss2 = SSS + 32;
	int sst2 = SST + 32;

	TEXSHIFT(SSS, SST, maxs, maxt)

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	INT32 SFRAC = SSS & 0x1f;
	INT32 TFRAC = SST & 0x1f;

	CLAMP(SSS, SST, maxs, maxt)
	CLAMP_LIGHT(sss2, sst2)

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	COLOR t1 = { { FETCH_TEXEL(sss2, SST) } };
	COLOR t2 = { { FETCH_TEXEL(SSS, sst2) } };

	COLOR TEX;
	if ((SFRAC + TFRAC) < 0x20)
	{
		COLOR t0 = { { FETCH_TEXEL(SSS, SST) } };
		INT32 R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
		INT32 G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
		INT32 B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
		INT32 A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		TEX.i.g = (G32 < 0) ? 0 : G32;
		TEX.i.b = (B32 < 0) ? 0 : B32;
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}
	else
	{
		INT32 INVSF = 0x20 - SFRAC;
		INT32 INVTF = 0x20 - TFRAC;
		COLOR t3 = { { FETCH_TEXEL(sss2, sst2) } };
		INT32 R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
		INT32 G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
		INT32 B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
		INT32 A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
		TEX.i.r = (R32 < 0) ? 0 : R32;
		TEX.i.g = (G32 < 0) ? 0 : G32;
		TEX.i.b = (B32 < 0) ? 0 : B32;
		TEX.i.a = (A32 < 0) ? 0 : A32;
	}

	return TEX.c;
}

#if defined(SHIFT_T)
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#else
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_C_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_BILINEAR_MID_NC_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#endif
{
	INT32 maxs, maxt;
	int sss2, sst2;

	TEXSHIFT(SSS, SST, maxs, maxt)

	sss2 = SSS + 32; sst2 = SST + 32;

	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);
	sss2 = RELATIVE(sss2, tex_tile->sl);
	sst2 = RELATIVE(sst2, tex_tile->tl);

	INT32 SFRAC = SSS & 0x1f;
	INT32 TFRAC = SST & 0x1f;

	CLAMP(SSS, SST, maxs, maxt)
	CLAMP_LIGHT(sss2, sst2)

	MASK(&SSS, &SST, tex_tile);
	MASK(&sss2, &sst2, tex_tile);

	COLOR t0 = { { FETCH_TEXEL(SSS, SST)   } };
	COLOR t1 = { { FETCH_TEXEL(sss2, SST)  } };
	COLOR t2 = { { FETCH_TEXEL(SSS, sst2)  } };
	COLOR t3 = { { FETCH_TEXEL(sss2, sst2) } };

	COLOR TEX;
	if (SFRAC!= 0x10 || TFRAC != 0x10)
	{
		if ((SFRAC + TFRAC) >= 0x20)
		{
			INT32 INVSF = 0x20 - SFRAC;
			INT32 INVTF = 0x20 - TFRAC;
			INT32 R32 = t3.i.r + ((INVSF*(t2.i.r - t3.i.r))>>5) + ((INVTF*(t1.i.r - t3.i.r))>>5);
			INT32 G32 = t3.i.g + ((INVSF*(t2.i.g - t3.i.g))>>5) + ((INVTF*(t1.i.g - t3.i.g))>>5);
			INT32 B32 = t3.i.b + ((INVSF*(t2.i.b - t3.i.b))>>5) + ((INVTF*(t1.i.b - t3.i.b))>>5);
			INT32 A32 = t3.i.a + ((INVSF*(t2.i.a - t3.i.a))>>5) + ((INVTF*(t1.i.a - t3.i.a))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			TEX.i.g = (G32 < 0) ? 0 : G32;
			TEX.i.b = (B32 < 0) ? 0 : B32;
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
		else
		{
			INT32 R32 = t0.i.r + ((SFRAC*(t1.i.r - t0.i.r))>>5) + ((TFRAC*(t2.i.r - t0.i.r))>>5);
			INT32 G32 = t0.i.g + ((SFRAC*(t1.i.g - t0.i.g))>>5) + ((TFRAC*(t2.i.g - t0.i.g))>>5);
			INT32 B32 = t0.i.b + ((SFRAC*(t1.i.b - t0.i.b))>>5) + ((TFRAC*(t2.i.b - t0.i.b))>>5);
			INT32 A32 = t0.i.a + ((SFRAC*(t1.i.a - t0.i.a))>>5) + ((TFRAC*(t2.i.a - t0.i.a))>>5);
			TEX.i.r = (R32 < 0) ? 0 : R32;
			TEX.i.g = (G32 < 0) ? 0 : G32;
			TEX.i.b = (B32 < 0) ? 0 : B32;
			TEX.i.a = (A32 < 0) ? 0 : A32;
		}
	}
	else // Is this accurate?
	{
		TEX.i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
		TEX.i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
		TEX.i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
		TEX.i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
	}

	return TEX.c;
}

#if defined(SHIFT_T)
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_DOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_NDOT_SS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_DOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_NDOT_NSS_ST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#else
	#if defined(SHIFT_S)
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_DOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_NDOT_SS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#else
		#if defined(DOT)
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_DOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#else
			#if defined(DOS)
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_DOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#else
				#if defined(COPY)
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_C_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#else
					INLINE UINT32 TEXTURE_PIPELINE_NEAREST_NMID_NC_NDOS_NDOT_NSS_NST(INT32 SSS, INT32 SST, TILE* tex_tile)
				#endif
			#endif
		#endif
	#endif
#endif
{
	INT32 maxs, maxt;
	INT32 SFRAC, TFRAC;

	TEXSHIFT(SSS, SST, maxs, maxt)
	SSS = RELATIVE(SSS, tex_tile->sl);
	SST = RELATIVE(SST, tex_tile->tl);

	SSS += 0x10;
	SST += 0x10;

	SFRAC = SSS & 0x1f;
	TFRAC = SST & 0x1f;

	CLAMP(SSS, SST, maxs, maxt)

	MASK(&SSS, &SST, tex_tile);

	/* point sample */
	return FETCH_TEXEL(SSS, SST);
}

#undef CLAMP
#undef CLAMP_LIGHT
#undef TEXSHIFT
