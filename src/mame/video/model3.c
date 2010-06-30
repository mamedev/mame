#include "emu.h"
#include "video/poly.h"
#include "video/rgbutil.h"
#include "includes/model3.h"


#define pz  p[0]
#define pu	p[1]
#define pv	p[2]



typedef float MATRIX[4][4];
typedef float VECTOR[4];
typedef float VECTOR3[3];

typedef struct {
	float x,y,z,d;
} PLANE;

typedef struct
{
	poly_vertex v[3];
	UINT8 texture_x, texture_y;
	UINT8 texture_width, texture_height;
	UINT8 transparency;
	UINT8 texture_format, param;
	int intensity;
	UINT32 color;
} TRIANGLE;

typedef struct _cached_texture cached_texture;
struct _cached_texture
{
	cached_texture *next;
	UINT8		width;
	UINT8		height;
	UINT8		format;
	UINT8		alpha;
	rgb_t		data[1];
};

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	cached_texture *texture;
	UINT32 color;
	UINT8 texture_param;
	int polygon_transparency;
	int polygon_intensity;
};

#define TRI_PARAM_TEXTURE_PAGE			0x1
#define TRI_PARAM_TEXTURE_MIRROR_U		0x2
#define TRI_PARAM_TEXTURE_MIRROR_V		0x4
#define TRI_PARAM_TEXTURE_ENABLE		0x8
#define TRI_PARAM_ALPHA_TEST			0x10

#define MAX_TRIANGLES		131072


/* forward declarations */
static void real3d_traverse_display_list(running_machine *machine);
static void draw_model(running_machine *machine, UINT32 addr);
static void init_matrix_stack(void);
static void get_top_matrix(MATRIX *out);
static void push_matrix_stack(void);
static void pop_matrix_stack(void);
static void multiply_matrix_stack(MATRIX matrix);
static void translate_matrix_stack(float x, float y, float z);
static void traverse_list(running_machine *machine, UINT32 address);
static void draw_block(running_machine *machine, UINT32 address);
static void draw_viewport(running_machine *machine, int pri, UINT32 address);
static void invalidate_texture(running_machine *machine, int page, int texx, int texy, int texwidth, int texheight);

/*****************************************************************************/

UINT64 *paletteram64;

static UINT8 model3_layer_enable = 0;
static UINT32 layer_modulate_r;
static UINT32 layer_modulate_g;
static UINT32 layer_modulate_b;
static UINT32 model3_layer_modulate1;
static UINT32 model3_layer_modulate2;
static UINT64 layer_scroll[2];

static UINT64 *m3_char_ram;
static UINT64 *m3_tile_ram;

static UINT32 *texture_fifo;
static int texture_fifo_pos = 0;

static UINT16 *texture_ram[2];
static UINT32 *display_list_ram;
static UINT32 *culling_ram;
static UINT32 *polygon_ram;

static UINT16 *pal_lookup;

static int real3d_display_list = 0;

static bitmap_t *bitmap3d;
static bitmap_t *zbuffer;
static rectangle clip3d;
static rectangle *screen_clip;


static VECTOR3 parallel_light;
static float parallel_light_intensity;
static float ambient_light_intensity;

static poly_manager *poly;
static cached_texture *texcache[2][1024/32][2048/32];


static int list_depth = 0;


static int tick = 0;
static int debug_layer_disable = 0;


static UINT64 vid_reg0;

/* matrix stack */
#define MATRIX_STACK_SIZE	256

static int matrix_stack_ptr = 0;
static MATRIX matrix_stack[MATRIX_STACK_SIZE];

#ifdef UNUSED_DEFINITION
static const int num_bits[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
#endif

static MATRIX coordinate_system;
static float viewport_focal_length;
static int viewport_region_x;
static int viewport_region_y;
static int viewport_region_width;
static int viewport_region_height;

static PLANE clip_plane[5];

static UINT32 matrix_base_address;


#define BYTE_REVERSE32(x)		(((x >> 24) & 0xff) | \
								((x >> 8) & 0xff00) | \
								((x << 8) & 0xff0000) | \
								((x << 24) & 0xff000000))

#define BYTE_REVERSE16(x)		(((x >> 8) & 0xff) | ((x << 8) & 0xff00))


static void model3_exit(running_machine &machine)
{
	invalidate_texture(&machine, 0, 0, 0, 6, 5);
	invalidate_texture(&machine, 1, 0, 0, 6, 5);
	poly_free(poly);
}

VIDEO_START( model3 )
{
	int width, height;

	poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), 0);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, model3_exit);

	width = machine->primary_screen->width();
	height = machine->primary_screen->height();
	bitmap3d = machine->primary_screen->alloc_compatible_bitmap();
	zbuffer = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);

	m3_char_ram = auto_alloc_array_clear(machine, UINT64, 0x100000/8);
	m3_tile_ram = auto_alloc_array_clear(machine, UINT64, 0x8000/8);

	pal_lookup = auto_alloc_array_clear(machine, UINT16, 65536);

	texture_fifo = auto_alloc_array_clear(machine, UINT32, 0x100000/4);

	/* 2x 4MB texture sheets */
	texture_ram[0] = auto_alloc_array(machine, UINT16, 0x400000/2);
	texture_ram[1] = auto_alloc_array(machine, UINT16, 0x400000/2);

	/* 1MB Display List RAM */
	display_list_ram = auto_alloc_array_clear(machine, UINT32, 0x100000/4);
	/* 4MB for nodes (< Step 2.0 have only 2MB) */
	culling_ram = auto_alloc_array_clear(machine, UINT32, 0x400000/4);
	/* 4MB Polygon RAM */
	polygon_ram = auto_alloc_array_clear(machine, UINT32, 0x400000/4);

	tick = 0;
	debug_layer_disable = 0;
	vid_reg0 = 0;

	viewport_focal_length = 300.;
	viewport_region_x = 0;
	viewport_region_y = 0;
	viewport_region_width = 496;
	viewport_region_height = 384;

	init_matrix_stack();
}

static void draw_tile_4bit(bitmap_t *bitmap, int tx, int ty, int tilenum)
{
	int x, y;
	UINT8 *tile_base = (UINT8*)m3_char_ram;
	UINT8 *tile;

	int data = (BYTE_REVERSE16(tilenum));
	int c = data & 0x7ff0;
	int tile_index = ((data << 1) & 0x7ffe) | ((data >> 15) & 0x1);
	tile_index *= 32;

	tile = &tile_base[tile_index];

	for(y = ty; y < ty+8; y++) {
		UINT16 *d = BITMAP_ADDR16(bitmap, y^1, 0);
		for(x = tx; x < tx+8; x+=2) {
			UINT8 tile0, tile1;
			UINT16 pix0, pix1;
			tile0 = *tile >> 4;
			tile1 = *tile & 0xf;
			pix0 = pal_lookup[c + tile0];
			pix1 = pal_lookup[c + tile1];
			if((pix0 & 0x8000) == 0)
			{
				d[x+0] = pix0;
			}
			if((pix1 & 0x8000) == 0)
			{
				d[x+1] = pix1;
			}
			++tile;
		}
	}
}

static void draw_tile_8bit(bitmap_t *bitmap, int tx, int ty, int tilenum)
{
	int x, y;
	UINT8 *tile_base = (UINT8*)m3_char_ram;
	UINT8 *tile;

	int data = (BYTE_REVERSE16(tilenum));
	int c = data & 0x7f00;
	int tile_index = ((data << 1) & 0x7ffe) | ((data >> 15) & 0x1);
	tile_index *= 32;

	tile = &tile_base[tile_index];

	for(y = ty; y < ty+8; y++) {
		UINT16 *d = BITMAP_ADDR16(bitmap, y, 0);
		int xx = 0;
		for(x = tx; x < tx+8; x++) {
			UINT8 tile0;
			UINT16 pix;
			tile0 = tile[xx^4];
			pix = pal_lookup[c + tile0];
			if((pix & 0x8000) == 0)
			{
				d[x] = pix;
			}
			++xx;
		}
		tile += 8;
	}
}
#ifdef UNUSED_FUNCTION
static void draw_texture_sheet(bitmap_t *bitmap, const rectangle *cliprect)
{
	int x,y;
	for(y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *d = BITMAP_ADDR16(bitmap, y, 0);
		int index = (y*2)*2048;
		for(x = cliprect->min_x; x <= cliprect->max_x; x++) {
			UINT16 pix = texture_ram[0][index];
			index+=4;
			if(pix != 0) {
				d[x] = pix;
			}
		}
	}
}
#endif

static void draw_layer(bitmap_t *bitmap, const rectangle *cliprect, int layer, int bitdepth)
{
	int x, y;
	int tile_index = 0;
	UINT16 *tiles = (UINT16*)&m3_tile_ram[layer * 0x400];

	//logerror("Layer %d: X: %d, Y: %d\n", layer, x1, y1);

	if(layer > 1) {
		int modr = (model3_layer_modulate2 >> 8) & 0xff;
		int modg = (model3_layer_modulate2 >> 16) & 0xff;
		int modb = (model3_layer_modulate2 >> 24) & 0xff;
		if(modr & 0x80) {
			layer_modulate_r = -(0x7f - (modr & 0x7f)) << 10;
		} else {
			layer_modulate_r = (modr & 0x7f) << 10;
		}
		if(modg & 0x80) {
			layer_modulate_g = -(0x7f - (modr & 0x7f)) << 5;
		} else {
			layer_modulate_g = (modr & 0x7f) << 5;
		}
		if(modb & 0x80) {
			layer_modulate_b = -(0x7f - (modr & 0x7f));
		} else {
			layer_modulate_b = (modr & 0x7f);
		}
	} else {
		int modr = (model3_layer_modulate1 >> 8) & 0xff;
		int modg = (model3_layer_modulate1 >> 16) & 0xff;
		int modb = (model3_layer_modulate1 >> 24) & 0xff;
		if(modr & 0x80) {
			layer_modulate_r = -(0x7f - (modr & 0x7f)) << 10;
		} else {
			layer_modulate_r = (modr & 0x7f) << 10;
		}
		if(modg & 0x80) {
			layer_modulate_g = -(0x7f - (modr & 0x7f)) << 5;
		} else {
			layer_modulate_g = (modr & 0x7f) << 5;
		}
		if(modb & 0x80) {
			layer_modulate_b = -(0x7f - (modr & 0x7f));
		} else {
			layer_modulate_b = (modr & 0x7f);
		}
	}

	if(bitdepth)		/* 4-bit */
	{
		for(y = cliprect->min_y; y <= cliprect->max_y; y+=8)
		{
			tile_index = ((y/8) * 64);
			for (x = cliprect->min_x; x <= cliprect->max_x; x+=8) {
				UINT16 tile = tiles[tile_index ^ 0x2];
				draw_tile_4bit(bitmap, x, y, tile);
				++tile_index;
			}
		}
	}
	else				/* 8-bit */
	{
		for(y = cliprect->min_y; y <= cliprect->max_y; y+=8)
		{
			tile_index = ((y/8) * 64);
			for (x = cliprect->min_x; x <= cliprect->max_x; x+=8) {
				UINT16 tile = tiles[tile_index ^ 0x2];
				draw_tile_8bit(bitmap, x, y, tile);
				++tile_index;
			}
		}
	}
}

#ifdef UNUSED_FUNCTION
static void copy_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
	int x,y;
	for(y=cliprect->min_y; y <= cliprect->max_y; y++) {
		UINT16 *d = BITMAP_ADDR16(bitmap, y, 0);
		UINT16 *s = BITMAP_ADDR16(bitmap3d, y, 0);
		for(x=cliprect->min_x; x <= cliprect->max_x; x++) {
			UINT16 pix = s[x];
			if(!(pix & 0x8000)) {
				d[x] = pix;
			}
		}
	}
}
#endif

VIDEO_UPDATE( model3 )
{
/*
    int layer_scroll_x[4], layer_scroll_y[4];
    UINT32 layer_data[4];

    layer_data[0] = BYTE_REVERSE32((UINT32)(layer_scroll[0] >> 32));
    layer_data[1] = BYTE_REVERSE32((UINT32)(layer_scroll[0] >> 0));
    layer_data[2] = BYTE_REVERSE32((UINT32)(layer_scroll[1] >> 32));
    layer_data[3] = BYTE_REVERSE32((UINT32)(layer_scroll[1] >> 0));
    layer_scroll_x[0] = (layer_data[0] & 0x8000) ? (layer_data[0] & 0x1ff) : -(layer_data[0] & 0x1ff);
    layer_scroll_y[0] = (layer_data[0] & 0x8000) ? (layer_data[0] & 0x1ff) : -(layer_data[0] & 0x1ff);
    layer_scroll_x[1] = (layer_data[1] & 0x8000) ? (layer_data[1] & 0x1ff) : -(layer_data[1] & 0x1ff);
    layer_scroll_y[1] = (layer_data[1] & 0x8000) ? (layer_data[1] & 0x1ff) : -(layer_data[1] & 0x1ff);
    layer_scroll_x[2] = (layer_data[2] & 0x8000) ? (layer_data[2] & 0x1ff) : -(layer_data[2] & 0x1ff);
    layer_scroll_y[2] = (layer_data[2] & 0x8000) ? (layer_data[2] & 0x1ff) : -(layer_data[2] & 0x1ff);
    layer_scroll_x[3] = (layer_data[3] & 0x8000) ? (layer_data[3] & 0x1ff) : -(layer_data[3] & 0x1ff);
    layer_scroll_y[3] = (layer_data[3] & 0x8000) ? (layer_data[3] & 0x1ff) : -(layer_data[3] & 0x1ff);
*/
	screen_clip = (rectangle*)cliprect;

	clip3d.min_x = cliprect->min_x;
	clip3d.max_x = cliprect->max_x;
	clip3d.min_y = cliprect->min_y;
	clip3d.max_y = cliprect->max_y;

	/* layer disable debug keys */
	tick++;
	if( tick >= 5 ) {
		tick = 0;

		if( input_code_pressed(screen->machine, KEYCODE_Y) )
			debug_layer_disable ^= 0x1;
		if( input_code_pressed(screen->machine, KEYCODE_U) )
			debug_layer_disable ^= 0x2;
		if( input_code_pressed(screen->machine, KEYCODE_I) )
			debug_layer_disable ^= 0x4;
		if( input_code_pressed(screen->machine, KEYCODE_O) )
			debug_layer_disable ^= 0x8;
		if( input_code_pressed(screen->machine, KEYCODE_T) )
			debug_layer_disable ^= 0x10;
	}

	bitmap_fill(bitmap, cliprect, 0);

	if (!(debug_layer_disable & 0x8))
		draw_layer(bitmap, cliprect, 3, (model3_layer_enable >> 3) & 0x1);

	if (!(debug_layer_disable & 0x4))
		draw_layer(bitmap, cliprect, 2, (model3_layer_enable >> 2) & 0x1);

	if( !(debug_layer_disable & 0x10) )
	{
//      if(real3d_display_list) {
//          bitmap_fill(zbuffer, cliprect, 0);
//          bitmap_fill(bitmap3d, cliprect, 0x8000);
//          real3d_traverse_display_list(screen->machine);
//      }
		copybitmap_trans(bitmap, bitmap3d, 0, 0, 0, 0, cliprect, 0x8000);
	}

	if (!(debug_layer_disable & 0x2))
		draw_layer(bitmap, cliprect, 1, (model3_layer_enable >> 1) & 0x1);

	if (!(debug_layer_disable & 0x1))
		draw_layer(bitmap, cliprect, 0, (model3_layer_enable >> 0) & 0x1);

	//copy_screen(bitmap, cliprect);

	//draw_texture_sheet(bitmap, cliprect);

	real3d_display_list = 0;
	return 0;
}



READ64_HANDLER(model3_char_r)
{
	return m3_char_ram[offset];
}

WRITE64_HANDLER(model3_char_w)
{
	COMBINE_DATA(&m3_char_ram[offset]);
}

READ64_HANDLER(model3_tile_r)
{
	return m3_tile_ram[offset];
}

WRITE64_HANDLER(model3_tile_w)
{
	COMBINE_DATA(&m3_tile_ram[offset]);
}

READ64_HANDLER(model3_vid_reg_r)
{
	switch(offset)
	{
		case 0x00/8:	return vid_reg0;
		case 0x08/8:	return U64(0xffffffffffffffff);		/* ??? */
		case 0x20/8:	return (UINT64)model3_layer_enable << 52;
		case 0x40/8:	return ((UINT64)model3_layer_modulate1 << 32) | (UINT64)model3_layer_modulate2;
		default:		logerror("read reg %02X\n", offset);break;
	}
	return 0;
}

WRITE64_HANDLER(model3_vid_reg_w)
{
	switch(offset)
	{
		case 0x00/8:	logerror("vid_reg0: %08X%08X\n", (UINT32)(data>>32),(UINT32)(data)); vid_reg0 = data; break;
		case 0x08/8:	break;		/* ??? */
		case 0x10/8:	model3_set_irq_line(space->machine, (data >> 56) & 0x0f, CLEAR_LINE); break;		/* VBL IRQ Ack */

		case 0x20/8:	model3_layer_enable = (data >> 52);	break;

		case 0x40/8:	model3_layer_modulate1 = (UINT32)(data >> 32);
						model3_layer_modulate2 = (UINT32)(data);
						break;
		case 0x60/8:	COMBINE_DATA(&layer_scroll[0]); break;
		case 0x68/8:	COMBINE_DATA(&layer_scroll[1]); break;
		default:		logerror("model3_vid_reg_w: %02X, %08X%08X\n", offset, (UINT32)(data >> 32), (UINT32)(data)); break;
	}
}

WRITE64_HANDLER( model3_palette_w )
{
	int r1,g1,b1,r2,g2,b2;
	UINT32 data1,data2;

	COMBINE_DATA(&paletteram64[offset]);
	data1 = BYTE_REVERSE32((UINT32)(paletteram64[offset] >> 32));
	data2 = BYTE_REVERSE32((UINT32)(paletteram64[offset] >> 0));

	r1 = ((data1 >> 0) & 0x1f);
	g1 = ((data1 >> 5) & 0x1f);
	b1 = ((data1 >> 10) & 0x1f);
	r2 = ((data2 >> 0) & 0x1f);
	g2 = ((data2 >> 5) & 0x1f);
	b2 = ((data2 >> 10) & 0x1f);

	pal_lookup[(offset*2)+0] = (data1 & 0x8000) | (r1 << 10) | (g1 << 5) | b1;
	pal_lookup[(offset*2)+1] = (data2 & 0x8000) | (r2 << 10) | (g2 << 5) | b2;
}

READ64_HANDLER( model3_palette_r )
{
	return paletteram64[offset];
}


/*****************************************************************************/
/* texture caching */

/*
    array of cached textures:
        4 potential textures for 4-bit grayscale
        2 pages
        1024 pixels / 32 pixel resolution vertically
        2048 pixels / 32 pixel resolution horizontally
*/
static void invalidate_texture(running_machine *machine, int page, int texx, int texy, int texwidth, int texheight)
{
	int wtiles = 1 << texwidth;
	int htiles = 1 << texheight;
	int x, y;

	for (y = 0; y < htiles; y++)
		for (x = 0; x < wtiles; x++)
			while (texcache[page][texy + y][texx + x] != NULL)
			{
				cached_texture *freeme = texcache[page][texy + y][texx + x];
				texcache[page][texy + y][texx + x] = freeme->next;
				auto_free(machine, freeme);
			}
}

static cached_texture *get_texture(running_machine *machine, int page, int texx, int texy, int texwidth, int texheight, int format)
{
	cached_texture *tex = texcache[page][texy][texx];
	int pixheight = 32 << texheight;
	int pixwidth = 32 << texwidth;
	UINT32 alpha = ~0;
	int x, y;

	/* if we have one already, validate it */
	for (tex = texcache[page][texy][texx]; tex != NULL; tex = tex->next)
		if (tex->width == texwidth && tex->height == texheight && tex->format == format)
			return tex;

	/* create a new texture */
	tex = (cached_texture *)auto_alloc_array(machine, UINT8, sizeof(cached_texture) + (2 * pixwidth * 2 * pixheight) * sizeof(rgb_t));
	tex->width = texwidth;
	tex->height = texheight;
	tex->format = format;

	/* set the new texture */
	tex->next = texcache[page][texy][texx];
	texcache[page][texy][texx] = tex;

	/* decode it */
	for (y = 0; y < pixheight; y++)
	{
		const UINT16 *texsrc = &texture_ram[page][(texy * 32 + y) * 2048 + texx * 32];
		rgb_t *dest = tex->data + 2 * pixwidth * y;

		switch (format)
		{
			case 0:		/* 1-5-5-5 ARGB */
				for (x = 0; x < pixwidth; x++)
				{
					UINT16 pixdata = texsrc[x];
					dest[x] = MAKE_ARGB(pal1bit(pixdata >> 15), pal5bit(pixdata >> 10), pal5bit(pixdata >> 5), pal5bit(pixdata >> 0));
					alpha = ~0;
				}
				break;

			case 1:		/* 4-bit grayscale in low nibble */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit(texsrc[x] >> 0);
					alpha &= dest[x] = MAKE_ARGB(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 2:		/* 4-bit grayscale in 2nd nibble */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit(texsrc[x] >> 4);
					alpha &= dest[x] = MAKE_ARGB(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 3:		/* 4-bit grayscale in 3rd nibble */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit(texsrc[x] >> 8);
					alpha &= dest[x] = MAKE_ARGB(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 4:		/* 8-bit A4L4 */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 pixdata = texsrc[x / 2] >> ((~x & 1) * 8);
					alpha &= dest[x] = MAKE_ARGB(pal4bit(pixdata >> 4), pal4bit(~pixdata), pal4bit(~pixdata), pal4bit(~pixdata));
				}
				break;

			case 5:		/* 8-bit grayscale */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = texsrc[x / 2] >> ((~x & 1) * 8);
					alpha &= dest[x] = MAKE_ARGB(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 6:		/* 4-bit grayscale in high nibble */
				for (x = 0; x < pixwidth; x++)
				{
					UINT8 grayvalue = pal4bit(texsrc[x] >> 12);
					alpha &= dest[x] = MAKE_ARGB(0xff, grayvalue, grayvalue, grayvalue);
				}
				break;

			case 7:		/* 4-4-4-4 ARGB */
				for (x = 0; x < pixwidth; x++)
				{
					UINT16 pixdata = texsrc[x];
					alpha &= dest[x] = MAKE_ARGB(pal4bit(pixdata >> 0), pal4bit(pixdata >> 12), pal4bit(pixdata >> 8), pal4bit(pixdata >> 4));
				}
				break;
		}

		/* create the horizontal mirror of this line */
		for (x = 0; x < pixwidth; x++)
			dest[pixwidth * 2 - 1 - x] = dest[x];
	}

	/* create the vertical mirror of the texture */
	for (y = 0; y < pixheight; y++)
		memcpy(tex->data + 2 * pixwidth * (pixheight * 2 - 1 - y), tex->data + 2 * pixwidth * y, sizeof(rgb_t) * pixwidth * 2);

	/* remember the overall alpha */
	tex->alpha = alpha >> 24;

	/* return a pointer to the texture */
	return tex;
}

/*****************************************************************************/
/* Real3D Graphics stuff */

WRITE64_HANDLER( real3d_display_list_w )
{
	if(ACCESSING_BITS_32_63) {
		display_list_ram[offset*2] = BYTE_REVERSE32((UINT32)(data >> 32));
	}
	if(ACCESSING_BITS_0_31) {
		display_list_ram[(offset*2)+1] = BYTE_REVERSE32((UINT32)(data));
	}
}

WRITE64_HANDLER( real3d_polygon_ram_w )
{
	if(ACCESSING_BITS_32_63) {
		polygon_ram[offset*2] = BYTE_REVERSE32((UINT32)(data >> 32));
	}
	if(ACCESSING_BITS_0_31) {
		polygon_ram[(offset*2)+1] = BYTE_REVERSE32((UINT32)(data));
	}
}

static const UINT8 texture_decode[64] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,
	16, 17, 20, 21, 24, 25, 28, 29,
	18, 19, 22, 23, 26, 27, 30, 31,
	32, 33, 36, 37, 40, 41, 44, 45,
	34, 35, 38, 39, 42, 43, 46, 47,
	48, 49, 52, 53, 56, 57, 60, 61,
	50, 51, 54, 55, 58, 59, 62, 63
};

INLINE void write_texture16(int xpos, int ypos, int width, int height, int page, UINT16 *data)
{
	int x,y,i,j;

	for(y=ypos; y < ypos+height; y+=8)
	{
		for(x=xpos; x < xpos+width; x+=8)
		{
			UINT16 *texture = &texture_ram[page][y*2048+x];
			int b = 0;
			for(j=y; j < y+8; j++) {
				for(i=x; i < x+8; i++) {
					*texture++ = data[texture_decode[b^1]];
					++b;
				}
				texture += 2048-8;
			}
			data += 64;
		}
	}
}

#ifdef UNUSED_FUNCTION
INLINE void write_texture8(int xpos, int ypos, int width, int height, int page, UINT16 *data)
{
	int x,y,i,j;
	UINT16 color = 0x7c00;

	for(y=ypos; y < ypos+(height/2); y+=4)
	{
		for(x=xpos; x < xpos+width; x+=8)
		{
			UINT16 *texture = &texture_ram[page][y*2048+x];
			for(j=y; j < y+4; j++) {
				for(i=x; i < x+8; i++) {
					*texture = color;
					texture++;
				}
				texture += 2048-8;
			}
		}
	}
}
#endif

static void real3d_upload_texture(running_machine *machine, UINT32 header, UINT32 *data)
{
	int width	= 32 << ((header >> 14) & 0x7);
	int height	= 32 << ((header >> 17) & 0x7);
	int xpos	= (header & 0x3f) * 32;
	int ypos	= ((header >> 7) & 0x1f) * 32;
	int page	= (header >> 20) & 0x1;
//  int bitdepth = (header >> 23) & 0x1;

	switch(header >> 24)
	{
		case 0x00:		/* Texture with mipmaps */
//          if(bitdepth) {
				write_texture16(xpos, ypos, width, height, page, (UINT16*)data);
				invalidate_texture(machine, page, header & 0x3f, (header >> 7) & 0x1f, (header >> 14) & 0x7, (header >> 17) & 0x7);
//          } else {
				/* TODO: 8-bit textures are weird. need to figure out some additional bits */
				//logerror("W: %d, H: %d, X: %d, Y: %d, P: %d, Bit: %d, : %08X, %08X\n", width, height, xpos, ypos, page, bitdepth, header & 0x00681040, header);
				//write_texture8(xpos, ypos, width, height, page, (UINT16*)data);
//          }
			break;
		case 0x01:		/* Texture without mipmaps */
//          if(bitdepth) {
				write_texture16(xpos, ypos, width, height, page, (UINT16*)data);
				invalidate_texture(machine, page, header & 0x3f, (header >> 7) & 0x1f, (header >> 14) & 0x7, (header >> 17) & 0x7);
//          } else {
				/* TODO: 8-bit textures are weird. need to figure out some additional bits */
				//logerror("W: %d, H: %d, X: %d, Y: %d, P: %d, Bit: %d, : %08X, %08X\n", width, height, xpos, ypos, page, bitdepth, header & 0x00681040, header);
				//write_texture8(xpos, ypos, width, height, page, (UINT16*)data);
//          }
			break;
		case 0x02:		/* Only mipmaps */
			break;
		case 0x80:		/* Gamma-table ? */
			break;
		default:
			fatalerror("Unknown texture type: %02X: ", header >> 24);
			break;
	}
}

void real3d_display_list_end(running_machine *machine)
{
	/* upload textures if there are any in the FIFO */
	if (texture_fifo_pos > 0)
	{
		int i = 0;
		while(i < texture_fifo_pos)
		{
			int length = (texture_fifo[i] / 2) + 2;
			UINT32 header = texture_fifo[i+1];
			real3d_upload_texture(machine, header, &texture_fifo[i+2]);
			i += length;
		};
	}
	texture_fifo_pos = 0;
	bitmap_fill(zbuffer, NULL, 0);
	bitmap_fill(bitmap3d, NULL, 0x8000);
	real3d_traverse_display_list(machine);
//  real3d_display_list = 1;
}

void real3d_display_list1_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap)
{
	int i;
	int d = (dst & 0xffffff) / 4;
	for(i=0; i < length; i+=4) {
		UINT32 w;
		if (byteswap) {
			w = BYTE_REVERSE32(memory_read_dword(space, src));
		} else {
			w = memory_read_dword(space, src);
		}
		display_list_ram[d++] = w;
		src += 4;
	}
}

void real3d_display_list2_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap)
{
	int i;
	int d = (dst & 0xffffff) / 4;
	for(i=0; i < length; i+=4) {
		UINT32 w;
		if (byteswap) {
			w = BYTE_REVERSE32(memory_read_dword(space, src));
		} else {
			w = memory_read_dword(space, src);
		}
		culling_ram[d++] = w;
		src += 4;
	}
}

void real3d_vrom_texture_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap)
{
	if((dst & 0xff) == 0) {

		UINT32 address, header;

		if (byteswap) {
			address = BYTE_REVERSE32(memory_read_dword(space, (src+0)));
			header = BYTE_REVERSE32(memory_read_dword(space, (src+4)));
		} else {
			address = memory_read_dword(space, (src+0));
			header = memory_read_dword(space, (src+4));
		}
		real3d_upload_texture(space->machine, header, (UINT32*)&model3_vrom[address]);
	}
}

void real3d_texture_fifo_dma(const address_space *space, UINT32 src, int length, int byteswap)
{
	int i;
	for(i=0; i < length; i+=4) {
		UINT32 w;
		if (byteswap) {
			w = BYTE_REVERSE32(memory_read_dword(space, src));
		} else {
			w = memory_read_dword(space, src);
		}
		texture_fifo[texture_fifo_pos] = w;
		texture_fifo_pos++;
		src += 4;
	}
}

void real3d_polygon_ram_dma(const address_space *space, UINT32 src, UINT32 dst, int length, int byteswap)
{
	int i;
	int d = (dst & 0xffffff) / 4;
	for(i=0; i < length; i+=4) {
		UINT32 w;
		if (byteswap) {
			w = BYTE_REVERSE32(memory_read_dword(space, src));
		} else {
			w = memory_read_dword(space, src);
		}
		polygon_ram[d++] = w;
		src += 4;
	}
}

WRITE64_HANDLER( real3d_cmd_w )
{
	real3d_display_list_end(space->machine);
}


/*****************************************************************************/
/* matrix and vector operations */

#ifdef UNUSED_FUNCTION
INLINE float dot_product(VECTOR a, VECTOR b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}
#endif

INLINE float dot_product3(VECTOR3 a, VECTOR3 b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

/* multiplies a 4-element vector by a 4x4 matrix */
static void matrix_multiply_vector(MATRIX matrix, const VECTOR v, VECTOR *p)
{
	(*p)[0] = (v[0] * matrix[0][0]) + (v[1] * matrix[1][0]) + (v[2] * matrix[2][0]) + (v[3] * matrix[3][0]);
	(*p)[1] = (v[0] * matrix[0][1]) + (v[1] * matrix[1][1]) + (v[2] * matrix[2][1]) + (v[3] * matrix[3][1]);
	(*p)[2] = (v[0] * matrix[0][2]) + (v[1] * matrix[1][2]) + (v[2] * matrix[2][2]) + (v[3] * matrix[3][2]);
	(*p)[3] = (v[0] * matrix[0][3]) + (v[1] * matrix[1][3]) + (v[2] * matrix[2][3]) + (v[3] * matrix[3][3]);
}

/* multiplies a 4x4 matrix with another 4x4 matrix */
static void matrix_multiply(MATRIX a, MATRIX b, MATRIX *out)
{
	int i,j;
	MATRIX tmp;

	for( i=0; i < 4; i++ ) {
		for( j=0; j < 4; j++ ) {
			tmp[i][j] = (a[i][0] * b[0][j]) + (a[i][1] * b[1][j]) + (a[i][2] * b[2][j]) + (a[i][3] * b[3][j]);
		}
	}
	memcpy(out, &tmp, sizeof(MATRIX));
}

static void init_matrix_stack(void)
{
	/* initialize the first matrix as identity */
	matrix_stack[0][0][0] = 1.0f;
	matrix_stack[0][0][1] = 0.0f;
	matrix_stack[0][0][2] = 0.0f;
	matrix_stack[0][0][3] = 0.0f;
	matrix_stack[0][1][0] = 0.0f;
	matrix_stack[0][1][1] = 1.0f;
	matrix_stack[0][1][2] = 0.0f;
	matrix_stack[0][1][3] = 0.0f;
	matrix_stack[0][2][0] = 0.0f;
	matrix_stack[0][2][1] = 0.0f;
	matrix_stack[0][2][2] = 1.0f;
	matrix_stack[0][2][3] = 0.0f;
	matrix_stack[0][3][0] = 0.0f;
	matrix_stack[0][3][1] = 0.0f;
	matrix_stack[0][3][2] = 0.0f;
	matrix_stack[0][3][3] = 1.0f;

	matrix_stack_ptr = 0;
}

static void get_top_matrix(MATRIX *out)
{
	memcpy( out, &matrix_stack[matrix_stack_ptr], sizeof(MATRIX));
}

static void push_matrix_stack(void)
{
	matrix_stack_ptr++;
	if (matrix_stack_ptr >= MATRIX_STACK_SIZE)
		fatalerror("push_matrix_stack: matrix stack overflow");

	memcpy( &matrix_stack[matrix_stack_ptr], &matrix_stack[matrix_stack_ptr-1], sizeof(MATRIX));
}

static void pop_matrix_stack(void)
{
	matrix_stack_ptr--;
	if (matrix_stack_ptr < 0)
		fatalerror("pop_matrix_stack: matrix stack underflow");
}

static void multiply_matrix_stack(MATRIX matrix)
{
	matrix_multiply(matrix, matrix_stack[matrix_stack_ptr], &matrix_stack[matrix_stack_ptr]);
}

static void translate_matrix_stack(float x, float y, float z)
{
	MATRIX tm;

	tm[0][0] = 1.0f;	tm[0][1] = 0.0f;	tm[0][2] = 0.0f;	tm[0][3] = 0.0f;
	tm[1][0] = 0.0f;	tm[1][1] = 1.0f;	tm[1][2] = 0.0f;	tm[1][3] = 0.0f;
	tm[2][0] = 0.0f;	tm[2][1] = 0.0f;	tm[2][2] = 1.0f;	tm[2][3] = 0.0f;
	tm[3][0] = x;		tm[3][1] = y;		tm[3][2] = z;		tm[3][3] = 1.0f;

	matrix_multiply(tm, matrix_stack[matrix_stack_ptr], &matrix_stack[matrix_stack_ptr]);
}

/*****************************************************************************/
/* transformation and rasterizing */

#include "m3raster.c"

INLINE int is_point_inside(float x, float y, float z, PLANE cp)
{
	float s = (x * cp.x) + (y * cp.y) + (z * cp.z) + cp.d;
	if (s >= 0.0f)
		return 1;
	else
		return 0;
}

INLINE float line_plane_intersection(const poly_vertex *v1, const poly_vertex *v2, PLANE cp)
{
	float x = v1->x - v2->x;
	float y = v1->y - v2->y;
	float z = v1->pz - v2->pz;
	float t = ((cp.x * v1->x) + (cp.y * v1->y) + (cp.z * v1->pz)) / ((cp.x * x) + (cp.y * y) + (cp.z * z));
	return t;
}

static int clip_polygon(const poly_vertex *v, int num_vertices, PLANE cp, poly_vertex *vout)
{
	poly_vertex clipv[10];
	int clip_verts = 0;
	float t;
	int i;

	int previ = num_vertices - 1;

	for (i=0; i < num_vertices; i++)
	{
		int v1_in = is_point_inside(v[i].x, v[i].y, v[i].pz, cp);
		int v2_in = is_point_inside(v[previ].x, v[previ].y, v[previ].pz, cp);

		if (v1_in && v2_in)			/* edge is completely inside the volume */
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}
		else if (!v1_in && v2_in)	/* edge is entering the volume */
		{
			/* insert vertex at intersection point */
			t = line_plane_intersection(&v[i], &v[previ], cp);
			clipv[clip_verts].x = v[i].x + ((v[previ].x - v[i].x) * t);
			clipv[clip_verts].y = v[i].y + ((v[previ].y - v[i].y) * t);
			clipv[clip_verts].pz = v[i].pz + ((v[previ].pz - v[i].pz) * t);
			clipv[clip_verts].pu = v[i].pu + ((v[previ].pu - v[i].pu) * t);
			clipv[clip_verts].pv = v[i].pv + ((v[previ].pv - v[i].pv) * t);
			++clip_verts;
		}
		else if (v1_in && !v2_in)	/* edge is leaving the volume */
		{
			/* insert vertex at intersection point */
			t = line_plane_intersection(&v[i], &v[previ], cp);
			clipv[clip_verts].x = v[i].x + ((v[previ].x - v[i].x) * t);
			clipv[clip_verts].y = v[i].y + ((v[previ].y - v[i].y) * t);
			clipv[clip_verts].pz = v[i].pz + ((v[previ].pz - v[i].pz) * t);
			clipv[clip_verts].pu = v[i].pu + ((v[previ].pu - v[i].pu) * t);
			clipv[clip_verts].pv = v[i].pv + ((v[previ].pv - v[i].pv) * t);
			++clip_verts;

			/* insert the existing vertex */
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}
	memcpy(&vout[0], &clipv[0], sizeof(vout[0]) * clip_verts);
	return clip_verts;
}

static void render_one(running_machine *machine, TRIANGLE *tri)
{
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
	poly_draw_scanline_func callback = NULL;

	tri->v[0].pz = 1.0f / tri->v[0].pz;
	tri->v[1].pz = 1.0f / tri->v[1].pz;
	tri->v[2].pz = 1.0f / tri->v[2].pz;

	if (tri->param & TRI_PARAM_TEXTURE_ENABLE)
	{
		tri->v[0].pu = tri->v[0].pu * tri->v[0].pz * 256.0f;
		tri->v[0].pv = tri->v[0].pv * tri->v[0].pz * 256.0f;
		tri->v[1].pu = tri->v[1].pu * tri->v[1].pz * 256.0f;
		tri->v[1].pv = tri->v[1].pv * tri->v[1].pz * 256.0f;
		tri->v[2].pu = tri->v[2].pu * tri->v[2].pz * 256.0f;
		tri->v[2].pv = tri->v[2].pv * tri->v[2].pz * 256.0f;

		extra->texture = get_texture(machine, (tri->param & TRI_PARAM_TEXTURE_PAGE) ? 1 : 0, tri->texture_x, tri->texture_y, tri->texture_width, tri->texture_height, tri->texture_format);
		extra->texture_param		= tri->param;
		extra->polygon_transparency = tri->transparency;
		extra->polygon_intensity	= tri->intensity;

		if (tri->param & TRI_PARAM_ALPHA_TEST)
			callback = draw_scanline_alpha_test;
		else if (extra->texture->alpha == 0xff)
			callback = (tri->transparency >= 32) ? draw_scanline_normal : draw_scanline_trans;
		else
			callback = draw_scanline_alpha;
		poly_render_triangle(poly, bitmap3d, &clip3d, callback, 3, &tri->v[0], &tri->v[1], &tri->v[2]);
	}
	else
	{
		extra->polygon_transparency = tri->transparency;
		extra->polygon_intensity	= tri->intensity;
		extra->color                = tri->color;

		poly_render_triangle(poly, bitmap3d, &clip3d, draw_scanline_color, 1, &tri->v[0], &tri->v[1], &tri->v[2]);
	}
}

static void draw_model(running_machine *machine, UINT32 addr)
{
	UINT32 *model = (addr >= 0x100000) ? &model3_vrom[addr] :  &polygon_ram[addr];
	UINT32 header[7];
	int index = 0;
	int last_polygon = FALSE, back_face = FALSE;
	int num_vertices;
	int i, v, vi;
	float fixed_point_fraction;
	poly_vertex vertex[4];
	poly_vertex prev_vertex[4];
	poly_vertex clip_vert[10];

	MATRIX transform_matrix;
	float center_x, center_y;

	if(model3_step < 0x15) {	/* position coordinates are 17.15 fixed-point in Step 1.0 */
		fixed_point_fraction = 1.0f / 32768.0f;
	} else {					/* 13.19 fixed-point in other Steps */
		fixed_point_fraction = 1.0f / 524288.0f;
	}

	get_top_matrix(&transform_matrix);

	/* current viewport center coordinates on screen */
	center_x = (float)(viewport_region_x + (viewport_region_width / 2));
	center_y = (float)(viewport_region_y + (viewport_region_height / 2));

	memset(prev_vertex, 0, sizeof(prev_vertex));

	while (!last_polygon)
	{
		float texture_coord_scale;
		UINT16 color;
		VECTOR3 normal;
		VECTOR3 sn;
		VECTOR p[4];
		TRIANGLE tri;
		float dot;
		int intensity;
		int polygon_transparency;

		for (i = 0; i < 7; i++)
			header[i] = model[index++];

		if (header[6] == 0)
			return;

		if (header[1] & 0x4)
			last_polygon = TRUE;

		num_vertices = (header[0] & 0x40) ? 4 : 3;

		/* texture coordinates are 16.0 or 13.3 fixed-point */
		texture_coord_scale = (header[1] & 0x40) ? 1.0f : (1.0f / 8.0f);

		/* polygon normal (sign + 1.22 fixed-point) */
		normal[0] = (float)((INT32)header[1] >> 8) * (1.0f / 4194304.0f);
		normal[1] = (float)((INT32)header[2] >> 8) * (1.0f / 4194304.0f);
		normal[2] = (float)((INT32)header[3] >> 8) * (1.0f / 4194304.0f);

		/* load reused vertices */
		vi = 0;
		for (v = 0; v < 4; v++)
			if (header[0] & (1 << v))
				vertex[vi++] = prev_vertex[v];

		/* load new vertices */
		for ( ; vi < num_vertices; vi++)
		{
			vertex[vi].x = (float)((INT32)model[index++]) * fixed_point_fraction;
			vertex[vi].y = (float)((INT32)model[index++]) * fixed_point_fraction;
			vertex[vi].pz = (float)((INT32)model[index++]) * fixed_point_fraction;
			vertex[vi].pu = (UINT16)(model[index] >> 16);
			vertex[vi].pv = (UINT16)(model[index++]);
		}

		/* Copy current vertices as previous vertices */
		memcpy(prev_vertex, vertex, sizeof(poly_vertex) * 4);

		color = (((header[4] >> 27) & 0x1f) << 10) | (((header[4] >> 19) & 0x1f) << 5) | ((header[4] >> 11) & 0x1f);
		polygon_transparency =  (header[6] & 0x800000) ? 32 : ((header[6] >> 18) & 0x1f);

		/* transform polygon normal to view-space */
		sn[0] = (normal[0] * transform_matrix[0][0]) +
				(normal[1] * transform_matrix[1][0]) +
				(normal[2] * transform_matrix[2][0]);
		sn[1] = (normal[0] * transform_matrix[0][1]) +
				(normal[1] * transform_matrix[1][1]) +
				(normal[2] * transform_matrix[2][1]);
		sn[2] = (normal[0] * transform_matrix[0][2]) +
				(normal[1] * transform_matrix[1][2]) +
				(normal[2] * transform_matrix[2][2]);

		sn[0] *= coordinate_system[0][1];
		sn[1] *= coordinate_system[1][2];
		sn[2] *= coordinate_system[2][0];

		/* TODO: depth bias */
		/* transform vertices */
		for (i = 0; i < num_vertices; i++)
		{
			VECTOR vect;

			vect[0] = vertex[i].x;
			vect[1] = vertex[i].y;
			vect[2] = vertex[i].pz;
			vect[3] = 1.0f;

			/* transform to world-space */
			matrix_multiply_vector(transform_matrix, vect, &p[i]);

			/* apply coordinate system */
			clip_vert[i].x = p[i][0] * coordinate_system[0][1];
			clip_vert[i].y = p[i][1] * coordinate_system[1][2];
			clip_vert[i].pz = p[i][2] * coordinate_system[2][0];
			clip_vert[i].pu = vertex[i].pu * texture_coord_scale;
			clip_vert[i].pv = vertex[i].pv * texture_coord_scale;
		}

		/* clip against view frustum */
		num_vertices = clip_polygon(clip_vert, num_vertices, clip_plane[0], clip_vert);
		num_vertices = clip_polygon(clip_vert, num_vertices, clip_plane[1], clip_vert);
		num_vertices = clip_polygon(clip_vert, num_vertices, clip_plane[2], clip_vert);
		num_vertices = clip_polygon(clip_vert, num_vertices, clip_plane[3], clip_vert);
		num_vertices = clip_polygon(clip_vert, num_vertices, clip_plane[4], clip_vert);

		/* backface culling */
		if( (header[6] & 0x800000) && (!(header[1] & 0x0010)) )	{
			if(sn[0]*clip_vert[0].x + sn[1]*clip_vert[0].y + sn[2]*clip_vert[0].pz >0)
				back_face = 1;
			else
				back_face = 0;
		}
		else
			back_face = 0;	//no culling for transparent or two-sided polygons

		if(!back_face)	{
			/* homogeneous Z-divide, screen-space transformation */
			for(i=0; i < num_vertices; i++) {
				float ooz = 1.0f / clip_vert[i].pz;
				clip_vert[i].x = ((clip_vert[i].x * ooz) * viewport_focal_length) + center_x;
				clip_vert[i].y = ((clip_vert[i].y * ooz) * viewport_focal_length) + center_y;
			}

			// lighting
			if ((header[6] & 0x10000) == 0)
			{
				dot = dot_product3(sn, parallel_light);
				intensity = ((dot * parallel_light_intensity) + ambient_light_intensity) * 256.0f;
				if (intensity > 256)
				{
					intensity = 256;
				}
				if (intensity < 0)
				{
					intensity = 0;
				}
			}
			else
			{
				// apply luminosity
				intensity = 256;
			}

			for (i=2; i < num_vertices; i++)
			{
				memcpy(&tri.v[0], &clip_vert[0], sizeof(poly_vertex));
				memcpy(&tri.v[1], &clip_vert[i-1], sizeof(poly_vertex));
				memcpy(&tri.v[2], &clip_vert[i], sizeof(poly_vertex));
				tri.texture_x				= ((header[4] & 0x1f) << 1) | ((header[5] >> 7) & 0x1);
				tri.texture_y				= (header[5] & 0x1f);
				tri.texture_width			= ((header[3] >> 3) & 0x7);
				tri.texture_height			= (header[3] & 0x7);
				tri.texture_format			= (header[6] >> 7) & 0x7;
				tri.transparency			= polygon_transparency;
				tri.intensity				= intensity;
				tri.color					= color;

				tri.param	= 0;
				tri.param	|= (header[4] & 0x40) ? TRI_PARAM_TEXTURE_PAGE : 0;
				tri.param	|= (header[6] & 0x4000000) ? TRI_PARAM_TEXTURE_ENABLE : 0;
				tri.param	|= (header[2] & 0x2) ? TRI_PARAM_TEXTURE_MIRROR_U : 0;
				tri.param	|= (header[2] & 0x1) ? TRI_PARAM_TEXTURE_MIRROR_V : 0;
				tri.param   |= (header[6] & 0x80000000) ? TRI_PARAM_ALPHA_TEST : 0;

				render_one(machine, &tri);
			}
		}
	};
}


/*****************************************************************************/
/* display list parser */

static UINT32 *get_memory_pointer(UINT32 address)
{
	if (address & 0x800000)
	{
		if (address >= 0x840000) {
			fatalerror("get_memory_pointer: invalid display list memory address %08X", address);
		}
		return &display_list_ram[address & 0x7fffff];
	}
	else
	{
		if (address >= 0x100000) {
			fatalerror("get_memory_pointer: invalid node ram address %08X", address);
		}
		return &culling_ram[address];
	}
}

static void load_matrix(int matrix_num, MATRIX *out)
{
	float *matrix = (float *)get_memory_pointer(matrix_base_address + matrix_num * 12);

	(*out)[0][0] = matrix[3];	(*out)[0][1] = matrix[6];	(*out)[0][2] = matrix[9];	(*out)[0][3] = 0.0f;
	(*out)[1][0] = matrix[4];	(*out)[1][1] = matrix[7];	(*out)[1][2] = matrix[10];	(*out)[1][3] = 0.0f;
	(*out)[2][0] = matrix[5];	(*out)[2][1] = matrix[8];	(*out)[2][2] = matrix[11];	(*out)[2][3] = 0.0f;
	(*out)[3][0] = matrix[0];	(*out)[3][1] = matrix[1];	(*out)[3][2] = matrix[2];	(*out)[3][3] = 1.0f;
}

static void traverse_list4(running_machine *machine, int lod_num, UINT32 address)
{
	/* does something with the LOD selection */
	UINT32 *list = get_memory_pointer(address);
	UINT32 link = list[0];

	draw_model(machine, link & 0xffffff);
}

static void traverse_list(running_machine *machine, UINT32 address)
{
	UINT32 *list = get_memory_pointer(address);
	int list_ptr = 0;

	if (list_depth > 2)
		return;

	list_depth++;

	/* find the end of the list */
	while (1)
	{
		address = list[list_ptr++];
		if (address & 0x02000000)
			break;
		if (address == 0 || (address >> 24) != 0)
		{
			list_ptr--;
			break;
		}
	}

	/* walk it backwards */
	while (list_ptr > 0)
	{
		address = list[--list_ptr] & 0xffffff;
		if (address != 0 && address != 0x800800)
//      if (address != 0)
			draw_block(machine, address);
	}

	list_depth--;
}

INLINE void process_link(running_machine *machine, UINT32 address, UINT32 link)
{
	if (link != 0 && link != 0x0fffffff && link != 0x00800800 && link != 0x01000000)
	{
		switch (link >> 24)
		{
			case 0x00:		/* link to another node */
				draw_block(machine, link & 0xffffff);
				break;

			case 0x01:
			case 0x03:		/* both of these link to models, is there any difference ? */
				draw_model(machine, link & 0xffffff);
				break;

			case 0x04:		/* list of links */
				traverse_list(machine, link & 0xffffff);
				break;

			default:
				logerror("process_link %08X: link = %08X\n", address, link);
				break;
		}
	}
}

static void draw_block(running_machine *machine, UINT32 address)
{
	const UINT32 *node = get_memory_pointer(address);
	UINT32 link;
	int node_matrix;
	float x, y, z;
	MATRIX matrix;
	int offset;

	offset = (model3_step < 0x15) ? 2 : 0;
	link = node[7 - offset];

	/* apply matrix and translation */
	node_matrix = node[3 - offset] & 0xfff;
	load_matrix(node_matrix, &matrix);

	push_matrix_stack();

	if (node[0] & 0x10)
	{
		x = *(float *)&node[4 - offset];
		y = *(float *)&node[5 - offset];
		z = *(float *)&node[6 - offset];
		translate_matrix_stack(x, y, z);
	}
	else if (node_matrix != 0)
		multiply_matrix_stack(matrix);

	/* bit 0x08 of word 0 indicates a pointer list */
	if (node[0] & 0x08)
		traverse_list4(machine, (node[3 - offset] >> 12) & 0x7f, link & 0xffffff);
	else
		process_link(machine, address, link);

	pop_matrix_stack();

	/* handle the second link */
	link = node[8 - offset];
	process_link(machine, address, link);
}

static void draw_viewport(running_machine *machine, int pri, UINT32 address)
{
	const UINT32 *node = get_memory_pointer(address);
	UINT32 link_address;
	float /*viewport_left, viewport_right, */viewport_top, viewport_bottom;
	float /*fov_x,*/ fov_y;

	link_address = node[1];
	if (link_address == 0)
		return;

	/* traverse to the link node before drawing this viewport */
	/* check this is correct as this affects the rendering order */
	if (link_address != 0x01000000)
		draw_viewport(machine, pri, link_address);

	/* skip if this isn't the right priority */
	if (pri != ((node[0] >> 3) & 3))
		return;

	/* set viewport parameters */
	viewport_region_x		= (node[26] & 0xffff) >> 4;			/* 12.4 fixed point */
	viewport_region_y		= ((node[26] >> 16) & 0xffff) >> 4;
	viewport_region_width	= (node[20] & 0xffff) >> 2;			/* 14.2 fixed point */
	viewport_region_height	= ((node[20] >> 16) & 0xffff) >> 2;

	/* frustum plane angles */
	//viewport_left         = RADIAN_TO_DEGREE(asin(*(float *)&node[12]));
	//viewport_right            = RADIAN_TO_DEGREE(asin(*(float *)&node[16]));
	viewport_top			= RADIAN_TO_DEGREE(asin(*(float *)&node[14]));
	viewport_bottom			= RADIAN_TO_DEGREE(asin(*(float *)&node[18]));

	/* build clipping planes */
	clip_plane[0].x = *(float *)&node[13];	clip_plane[0].y = 0.0f;		clip_plane[0].z = *(float *)&node[12];	clip_plane[0].d = 0.0f;
	clip_plane[1].x = *(float *)&node[17];	clip_plane[1].y = 0.0f;		clip_plane[1].z = *(float *)&node[16];	clip_plane[1].d = 0.0f;
	clip_plane[2].x = 0.0f;		clip_plane[2].y = *(float *)&node[15];	clip_plane[2].z = *(float *)&node[14];	clip_plane[2].d = 0.0f;
	clip_plane[3].x = 0.0f;		clip_plane[3].y = *(float *)&node[19];	clip_plane[3].z = *(float *)&node[18];	clip_plane[3].d = 0.0f;
	clip_plane[4].x = 0.0f;		clip_plane[4].y = 0.0f;		clip_plane[4].z = 1.0f;		clip_plane[4].d = 1.0f;

	/* compute field of view */
	//fov_x = viewport_left + viewport_right;
	fov_y = viewport_top + viewport_bottom;
	viewport_focal_length = (viewport_region_height / 2) / tan( (fov_y * M_PI / 180.0f) / 2.0f );

	matrix_base_address = node[22];
	/* TODO: where does node[23] point to ? LOD table ? */

	/* set lighting parameters */
	parallel_light[0] = -*(float *)&node[5];
	parallel_light[1] = *(float *)&node[6];
	parallel_light[2] = *(float *)&node[4];
	parallel_light_intensity = *(float *)&node[7];
	ambient_light_intensity = (UINT8)(node[36] >> 8) / 256.0f;

	/* set coordinate system matrix */
	load_matrix(0, &coordinate_system);

	/* process a link */
	process_link(machine, link_address, node[2]);
}


static void real3d_traverse_display_list(running_machine *machine)
{
	int pri;

	init_matrix_stack();

	for (pri = 0; pri < 4; pri++)
		draw_viewport(machine, pri, 0x800000);

	poly_wait(poly, "real3d_traverse_display_list");
}

