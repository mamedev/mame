static void TEXSHIFT_S_T(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile);
static void TEXSHIFT_S_NT(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile);
static void TEXSHIFT_NS_T(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile);
static void TEXSHIFT_NS_NT(INT32* S, INT32* T, INT32* maxs, INT32* maxt, TILE* tex_tile);

static void (*rdp_shift_func[4])(INT32*, INT32*, INT32*, INT32*, TILE*) =
{
	TEXSHIFT_NS_NT, TEXSHIFT_NS_T, TEXSHIFT_S_NT, TEXSHIFT_S_T
};
