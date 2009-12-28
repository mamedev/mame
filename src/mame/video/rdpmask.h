INLINE void MASK_NS_NT(INT32* S, INT32* T, TILE* tex_tile);
INLINE void MASK_NS_T(INT32* S, INT32* T, TILE* tex_tile);
INLINE void MASK_S_NT(INT32* S, INT32* T, TILE* tex_tile);
INLINE void MASK_S_T(INT32* S, INT32* T, TILE* tex_tile);

static void (*rdp_mask_func[8])(INT32*, INT32*, TILE*) =
{
	MASK_NS_NT,	MASK_NS_T,	MASK_S_NT,	MASK_S_T,
};
