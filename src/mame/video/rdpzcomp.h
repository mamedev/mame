INLINE UINT32 z_compare_IMR_AA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_AA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_NAA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_NAA_Z0(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_AA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_AA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_NAA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_NAA_Z1(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_AA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_AA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_NAA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_NAA_Z2(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_AA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_AA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_IMR_NAA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE UINT32 z_compare_NIMR_NAA_Z3(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);

static UINT32 (*rdp_z_compare_func[16])(void*, UINT8*, UINT16*, UINT8*, UINT32, UINT16) =
{
	z_compare_IMR_AA_Z0,	z_compare_NIMR_AA_Z0, 	z_compare_IMR_NAA_Z0, 	z_compare_NIMR_NAA_Z0,
	z_compare_IMR_AA_Z1,	z_compare_NIMR_AA_Z1, 	z_compare_IMR_NAA_Z1, 	z_compare_NIMR_NAA_Z1,
	z_compare_IMR_AA_Z2,	z_compare_NIMR_AA_Z2, 	z_compare_IMR_NAA_Z2, 	z_compare_NIMR_NAA_Z2,
	z_compare_IMR_AA_Z3,	z_compare_NIMR_AA_Z3, 	z_compare_IMR_NAA_Z3, 	z_compare_NIMR_NAA_Z3
};
