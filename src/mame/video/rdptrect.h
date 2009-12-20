static void texture_rectangle_16bit_c1_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_nzc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_zc_nzu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_nzc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c1_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_c2_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cc_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);
static void texture_rectangle_16bit_cf_zc_zu(running_machine *machine, TEX_RECTANGLE *rect);

static void (*rdp_texture_rectangle_16bit_func[16])(running_machine *, TEX_RECTANGLE *) =
{
	texture_rectangle_16bit_c1_nzc_nzu,	texture_rectangle_16bit_c2_nzc_nzu,	texture_rectangle_16bit_cc_nzc_nzu,	texture_rectangle_16bit_cf_nzc_nzu,
	texture_rectangle_16bit_c1_zc_nzu,	texture_rectangle_16bit_c2_zc_nzu,	texture_rectangle_16bit_cc_zc_nzu,	texture_rectangle_16bit_cf_zc_nzu,
	texture_rectangle_16bit_c1_nzc_zu,	texture_rectangle_16bit_c2_nzc_zu,	texture_rectangle_16bit_cc_nzc_zu,	texture_rectangle_16bit_cf_nzc_zu,
	texture_rectangle_16bit_c1_zc_zu,	texture_rectangle_16bit_c2_zc_zu,	texture_rectangle_16bit_cc_zc_zu,	texture_rectangle_16bit_cf_zc_zu,
};

