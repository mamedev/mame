static void render_spans_16_c1(running_machine *machine, int start, int end, TILE* tex_tile, int shade, int texture, int zbuffer, int flip);
static void render_spans_16_c2(running_machine *machine, int start, int end, TILE* tex_tile, int shade, int texture, int zbuffer, int flip);

static void (*rdp_render_spans_16_func[4])(running_machine*, int, int, TILE*, int, int, int, int) =
{
	render_spans_16_c1, render_spans_16_c2, render_spans_16_c1, render_spans_16_c1
};
