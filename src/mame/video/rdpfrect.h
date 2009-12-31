static void fill_rectangle_16bit_c1_dm(RECTANGLE *rect);
static void fill_rectangle_16bit_c1_db(RECTANGLE *rect);
static void fill_rectangle_16bit_c1_dn(RECTANGLE *rect);
static void fill_rectangle_16bit_c2_dm(RECTANGLE *rect);
static void fill_rectangle_16bit_c2_db(RECTANGLE *rect);
static void fill_rectangle_16bit_c2_dn(RECTANGLE *rect);
static void fill_rectangle_16bit_cc(RECTANGLE *rect);
static void fill_rectangle_16bit_cf(RECTANGLE *rect);

static void (*rdp_fill_rectangle_16bit_func[16])(RECTANGLE *) =
{
	fill_rectangle_16bit_c1_dm, fill_rectangle_16bit_c1_db, fill_rectangle_16bit_c1_dn, fill_rectangle_16bit_c1_dn,
	fill_rectangle_16bit_c2_dm, fill_rectangle_16bit_c2_db, fill_rectangle_16bit_c2_dn, fill_rectangle_16bit_c2_dn,
	fill_rectangle_16bit_cc,	fill_rectangle_16bit_cc,	fill_rectangle_16bit_cc,	fill_rectangle_16bit_cc,
	fill_rectangle_16bit_cf,	fill_rectangle_16bit_cf,	fill_rectangle_16bit_cf,	fill_rectangle_16bit_cf,
};
