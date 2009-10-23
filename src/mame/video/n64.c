/*
    Nintendo 64 Video Hardware
*/

#include "driver.h"
#include "includes/n64.h"

#define LOG_RDP_EXECUTION 		0

static FILE *rdp_exec;

static UINT32 rdp_cmd_data[0x1000];
static int rdp_cmd_ptr = 0;
static int rdp_cmd_cur = 0;

#define RDP_CVG_SPAN_MAX    1024

typedef struct
{
	int lx, rx;
	int s, ds;
	int t, dt;
	int w, dw;
	int r, dr;
	int g, dg;
	int b, db;
	int a, da;
	int z, dz;
    UINT8 cvg[RDP_CVG_SPAN_MAX];
} SPAN;

UINT8 cvg_mem[8*1048576];

static SPAN span[4096];

/*****************************************************************************/

#define BYTE_ADDR_XOR		BYTE4_XOR_BE(0)
#define WORD_ADDR_XOR		(WORD_XOR_BE(0) >> 1)

typedef struct
{
	UINT8 r, g, b, a;
} COLOR;

typedef struct
{
	UINT16 xl, yl, xh, yh;		// 10.2 fixed-point
} RECTANGLE;

typedef struct
{
	int tilenum;
	UINT16 xl, yl, xh, yh;		// 10.2 fixed-point
	INT16 s, t;					// 10.5 fixed-point
	INT16 dsdx, dtdy;			// 5.10 fixed-point
} TEX_RECTANGLE;

typedef struct
{
	int format, size;
	UINT32 line;
	UINT32 tmem;
	int palette;
	// TODO: clamp & mirror parameters
	int ct, mt, cs, ms;
	int mask_t, shift_t, mask_s, shift_s;

	UINT16 sl, tl, sh, th;		// 10.2 fixed-point
} TILE;

typedef struct
{
	int sub_a_rgb0;
	int sub_b_rgb0;
	int mul_rgb0;
	int add_rgb0;
	int sub_a_a0;
	int sub_b_a0;
	int mul_a0;
	int add_a0;

	int sub_a_rgb1;
	int sub_b_rgb1;
	int mul_rgb1;
	int add_rgb1;
	int sub_a_a1;
	int sub_b_a1;
	int mul_a1;
	int add_a1;
} COMBINE_MODES;

typedef struct
{
	int cycle_type;
	int persp_tex_en;
	int detail_tex_en;
	int sharpen_tex_en;
	int tex_lod_en;
	int en_tlut;
	int tlut_type;
	int sample_type;
	int mid_texel;
	int bi_lerp0;
	int bi_lerp1;
	int convert_one;
	int key_en;
	int rgb_dither_sel;
	int alpha_dither_sel;
	int blend_m1a_0;
	int blend_m1a_1;
	int blend_m1b_0;
	int blend_m1b_1;
	int blend_m2a_0;
	int blend_m2a_1;
	int blend_m2b_0;
	int blend_m2b_1;
    int tex_edge;
	int force_blend;
	int alpha_cvg_select;
	int cvg_times_alpha;
	int z_mode;
	int cvg_dest;
	int color_on_cvg;
	int image_read_en;
	int z_update_en;
	int z_compare_en;
	int antialias_en;
	int z_source_sel;
	int dither_alpha_en;
	int alpha_compare_en;
} OTHER_MODES;

/*****************************************************************************/

#define PIXEL_SIZE_4BIT			0
#define PIXEL_SIZE_8BIT			1
#define PIXEL_SIZE_16BIT		2
#define PIXEL_SIZE_32BIT		3

#define CYCLE_TYPE_1			0
#define CYCLE_TYPE_2			1
#define CYCLE_TYPE_COPY			2
#define CYCLE_TYPE_FILL			3

#define SAMPLE_TYPE_1x1         0
#define SAMPLE_TYPE_2x2         1

static COMBINE_MODES combine;
static OTHER_MODES other_modes;

static COLOR blend_color;
static COLOR prim_color;
static COLOR env_color;
static COLOR fog_color;
static COLOR combined_color;
static COLOR texel0_color;
static COLOR texel1_color;
static COLOR shade_color;

static COLOR one_color		= { 0xff, 0xff, 0xff, 0xff };
static COLOR zero_color		= { 0x00, 0x00, 0x00, 0x00 };

// combiner inputs
static UINT8 *combiner_rgbsub_a_r[2];
static UINT8 *combiner_rgbsub_a_g[2];
static UINT8 *combiner_rgbsub_a_b[2];
static UINT8 *combiner_rgbsub_b_r[2];
static UINT8 *combiner_rgbsub_b_g[2];
static UINT8 *combiner_rgbsub_b_b[2];
static UINT8 *combiner_rgbmul_r[2];
static UINT8 *combiner_rgbmul_g[2];
static UINT8 *combiner_rgbmul_b[2];
static UINT8 *combiner_rgbadd_r[2];
static UINT8 *combiner_rgbadd_g[2];
static UINT8 *combiner_rgbadd_b[2];

static UINT8 *combiner_alphasub_a[2];
static UINT8 *combiner_alphasub_b[2];
static UINT8 *combiner_alphamul[2];
static UINT8 *combiner_alphaadd[2];

// blender input
static UINT8 *blender1a_r[2];
static UINT8 *blender1a_g[2];
static UINT8 *blender1a_b[2];
static UINT8 *blender1b_a[2];
static UINT8 *blender2a_r[2];
static UINT8 *blender2a_g[2];
static UINT8 *blender2a_b[2];
static UINT8 *blender2b_a[2];

static COLOR pixel_color;
static COLOR inv_pixel_color;
static COLOR blended_pixel_color;
static COLOR memory_color;

static UINT32 fill_color;		// packed 16-bit or 32-bit, depending on framebuffer format

static UINT16 primitive_z;
static UINT16 primitive_delta_z;

static int fb_format;
static int fb_size;
int fb_width;
static UINT32 fb_address;

static UINT32 zb_address;

static int ti_format;
static int ti_size;
static int ti_width;
static UINT32 ti_address;

static TILE tile[8];

static RECTANGLE clip;

static UINT8 *texture_cache;
static UINT32 tlut[256];

#define MASK(S, T, CYCLE)                                               \
do                                                                      \
{                                                                       \
    if (mask_s[CYCLE] != 0)                                             \
    {                                                                   \
        S &= (1 << mask_s[CYCLE]) - 1;                                  \
    }                                                                   \
    if (mask_t[CYCLE] != 0)                                             \
    {                                                                   \
        T &= (1 << mask_t[CYCLE]) - 1;                                  \
    }                                                                   \
}                                                                       \
while (0)

#define CLAMP(S, T, CYCLE)                                              \
do                                                                      \
{                                                                       \
    if (mask_s[CYCLE] != 0 && clamp_s[CYCLE])                           \
    {                                                                   \
        if (S < (tsl[CYCLE] >> 2)) S = (tsl[CYCLE] >> 2);               \
        if (S > (tsh[CYCLE] >> 2)) S = (tsh[CYCLE] >> 2);               \
    }                                                                   \
    if (mask_s[CYCLE] != 0 && mirror_s[CYCLE])                          \
    {                                                                   \
        if (S & (1 << mask_s[CYCLE])) S ^= (1 << mask_s[CYCLE]) - 1;    \
    }                                                                   \
    if (mask_t[CYCLE] != 0 && clamp_t[CYCLE])                           \
    {                                                                   \
        if (T < (ttl[CYCLE] >> 2)) T = (ttl[CYCLE] >> 2);               \
        if (T > (tth[CYCLE] >> 2)) T = (tth[CYCLE] >> 2);               \
    }                                                                   \
    if (mask_t[CYCLE] != 0 && mirror_t[CYCLE])                          \
    {                                                                   \
        if (T & (1 << mask_t[CYCLE])) T ^= (1 << mask_t[CYCLE]) - 1;    \
    }                                                                   \
}                                                                       \
while (0)

/*****************************************************************************/



VIDEO_START(n64)
{
	if (LOG_RDP_EXECUTION)
		rdp_exec = fopen("rdp_execute.txt", "wt");

	texture_cache = auto_alloc_array(machine, UINT8, 0x100000);

	combiner_rgbsub_a_r[0] = combiner_rgbsub_a_r[1] = &one_color.r;
	combiner_rgbsub_a_g[0] = combiner_rgbsub_a_g[1] = &one_color.g;
	combiner_rgbsub_a_b[0] = combiner_rgbsub_a_b[1] = &one_color.b;
	combiner_rgbsub_b_r[0] = combiner_rgbsub_b_r[1] = &one_color.r;
	combiner_rgbsub_b_g[0] = combiner_rgbsub_b_g[1] = &one_color.g;
	combiner_rgbsub_b_b[0] = combiner_rgbsub_b_b[1] = &one_color.b;
	combiner_rgbmul_r[0] = combiner_rgbmul_r[1] = &one_color.r;
	combiner_rgbmul_g[0] = combiner_rgbmul_g[1] = &one_color.g;
	combiner_rgbmul_b[0] = combiner_rgbmul_b[1] = &one_color.b;
	combiner_rgbadd_r[0] = combiner_rgbadd_r[1] = &one_color.r;
	combiner_rgbadd_g[0] = combiner_rgbadd_g[1] = &one_color.g;
	combiner_rgbadd_b[0] = combiner_rgbadd_b[1] = &one_color.b;

	combiner_alphasub_a[0] = combiner_alphasub_a[1] = &one_color.a;
	combiner_alphasub_b[0] = combiner_alphasub_b[1] = &one_color.a;
	combiner_alphamul[0] = combiner_alphamul[1] = &one_color.a;
	combiner_alphaadd[0] = combiner_alphaadd[1] = &one_color.a;

	blender1a_r[0] = blender1a_r[1] = &pixel_color.r;
	blender1a_g[0] = blender1a_g[1] = &pixel_color.r;
	blender1a_b[0] = blender1a_b[1] = &pixel_color.r;
	blender1b_a[0] = blender1b_a[1] = &pixel_color.r;
	blender2a_r[0] = blender2a_r[1] = &pixel_color.r;
	blender2a_g[0] = blender2a_g[1] = &pixel_color.r;
	blender2a_b[0] = blender2a_b[1] = &pixel_color.r;
	blender2b_a[0] = blender2b_a[1] = &pixel_color.r;
}

static void vi_filter_16( INT32 x, INT32 y, UINT16 *out )
{
    UINT8 *cvg_buffer = &cvg_mem[n64_vi_origin / 4];
    UINT16 *frame_buffer = (UINT16*)&rdram[n64_vi_origin / 4];
    INT32 i_start = x - 2;
    INT32 i_end = x + 2;
    INT32 j_start = y - 1;
    INT32 j_end = y + 1;
    INT32 r_sum = 0;
    INT32 g_sum = 0;
    INT32 b_sum = 0;
    INT32 covered_count = 0;
    INT32 i = 0;
    INT32 j = 0;

    if (cvg_buffer[y*fb_width + x] == 8)
    {
        *out = frame_buffer[y*fb_width + x];
        return;
    }

    if (i_start < 0)
    {
        i_start = 0;
    }
    if (j_start < 0)
    {
        j_start = 0;
    }
    if (i_end >= fb_width)
    {
        i_end = fb_width - 1;
    }
    if (j_end >= ((n64_vi_control & 0x40) ? 480 : 240))
    {
        j_end = (n64_vi_control & 0x40) ? 479 : 239;
    }

    for (j = j_start; j <= j_end; j++)
    {
        for (i = i_start; i <= i_end; i++)
        {
            if (cvg_buffer[j*fb_width + x] == 8)
            {
                UINT16 color = frame_buffer[j*fb_width + x];
                covered_count++;
                r_sum += (color >> 11) & 0x1f;
                g_sum += (color >>  6) & 0x1f;
                b_sum += (color >>  1) & 0x1f;
            }
        }
    }

    if (covered_count)
    {
        UINT16 fb = frame_buffer[y*fb_width + x];
        r_sum /= covered_count;
        g_sum /= covered_count;
        b_sum /= covered_count;
        if (r_sum > 0x1f) r_sum = 0x1f;
        if (g_sum > 0x1f) g_sum = 0x1f;
        if (b_sum > 0x1f) b_sum = 0x1f;
        r_sum = (UINT32)(((float)r_sum / 8.0f) * (float)(8 - cvg_buffer[y*fb_width + x]) +
                         ((float)((fb >> 11) & 0x1f) / 8.0f) * (float)cvg_buffer[y*fb_width + x]);
        g_sum = (UINT32)(((float)g_sum / 8.0f) * (float)(8 - cvg_buffer[y*fb_width + x]) +
                         ((float)((fb >>  6) & 0x1f) / 8.0f) * (float)cvg_buffer[y*fb_width + x]);
        b_sum = (UINT32)(((float)b_sum / 8.0f) * (float)(8 - cvg_buffer[y*fb_width + x]) +
                         ((float)((fb >>  1) & 0x1f) / 8.0f) * (float)cvg_buffer[y*fb_width + x]);
        *out = (r_sum << 11) | (g_sum << 6) | (b_sum << 1);
    }
    else
    {
        *out = frame_buffer[y*fb_width + x];
    }
}

static void vi_filter_32( INT32 x, INT32 y, UINT32 *out )
{
    UINT8 *cvg_buffer = &cvg_mem[n64_vi_origin / 4];
    UINT32 *frame_buffer = (UINT32*)&rdram[n64_vi_origin / 4];
    INT32 i_start = x - 2;
    INT32 i_end = x + 2;
    INT32 j_start = y - 1;
    INT32 j_end = y + 1;
    UINT32 r_sum = 0;
    UINT32 g_sum = 0;
    UINT32 b_sum = 0;
    INT32 covered_count = 0;
    INT32 i = 0;
    INT32 j = 0;

    if (cvg_buffer[y*fb_width + x] == 8)
    {
        *out = frame_buffer[y*fb_width + x];
        return;
    }

    if (i_start < 0)
    {
        i_start = 0;
    }
    if (j_start < 0)
    {
        j_start = 0;
    }
    if (i_end >= fb_width)
    {
        i_end = fb_width - 1;
    }
    if (j_end >= ((n64_vi_control & 0x40) ? 480 : 240))
    {
        j_end = (n64_vi_control & 0x40) ? 479 : 239;
    }

    for (j = j_start; j <= j_end; j++)
    {
        for (i = i_start; i <= i_end; i++)
        {
            if (cvg_buffer[j*fb_width + x] == 8)
            {
                UINT32 color = frame_buffer[j*fb_width + x];
                covered_count++;
                r_sum += (color >> 16) & 0xff;
                g_sum += (color >>  8) & 0xff;
                b_sum += (color >>  0) & 0xff;
            }
        }
    }

    if (covered_count)
    {
        r_sum /= covered_count;
        g_sum /= covered_count;
        b_sum /= covered_count;
        if (r_sum > 0xff) r_sum = 0xff;
        if (g_sum > 0xff) g_sum = 0xff;
        if (b_sum > 0xff) b_sum = 0xff;
        *out = (r_sum << 24) | (g_sum << 16) | (b_sum << 8) | 0xe0;
    }
    else
    {
        *out = frame_buffer[y*fb_width + x];
    }
}

VIDEO_UPDATE(n64)
{
	int i, j;
    int height = (n64_vi_control & 0x40) ? 479 : 239;

    if (n64_vi_blank)
    {
        for (j=0; j <height; j++)
        {
            UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);
            for (i=0; i < fb_width; i++)
            {
                d[BYTE_XOR_BE(i)] = 0;
            }
        }
        return 0;
    }

    switch (n64_vi_control & 0x3)
	{
		case 0:		// blank/no signal
		{
			break;
		}

		case 2:		// RGBA5551
		{
            UINT16 *frame_buffer = (UINT16*)&rdram[n64_vi_origin / 4];
			if (frame_buffer)
			{
				for (j=0; j <= height; j++)
				{
					UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);
					for (i=0; i < fb_width; i++)
					{
						//UINT16 pix = *frame_buffer++;
                        UINT16 pix = 0;
                        int r, g, b;
                        vi_filter_16( i, j, &pix );
						r = ((pix >> 11) & 0x1f) << 3;
						g = ((pix >> 6) & 0x1f) << 3;
						b = ((pix >> 1) & 0x1f) << 3;
						d[BYTE_XOR_BE(i)] = (r << 16) | (g << 8) | b;
					}
				}
			}
			break;
		}

		case 3:		// RGBA8888
		{
            UINT32 *frame_buffer = (UINT32*)&rdram[n64_vi_origin / 4];
			if (frame_buffer)
			{
				for (j=0; j <= height; j++)
				{
					UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);
					for (i=0; i < fb_width; i++)
					{
                        //UINT32 pix = *frame_buffer++;
                        UINT32 pix = 0;
                        vi_filter_32( i, j, &pix );
                        *d++ = (pix >> 8) & 0x00ffffff;
					}
				}
			}
			break;
		}

        default:    fatalerror("Unknown framebuffer format %d\n", n64_vi_control & 0x3);
	}
	return 0;
}

/*****************************************************************************/

INLINE void SET_SUBA_RGB_INPUT(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &combined_color.r;	*input_g = &combined_color.g;	*input_b = &combined_color.b;	break;
		case 1:		*input_r = &texel0_color.r;		*input_g = &texel0_color.g;		*input_b = &texel0_color.b;		break;
		case 2:		*input_r = &texel1_color.r;		*input_g = &texel1_color.g;		*input_b = &texel1_color.b;		break;
		case 3:		*input_r = &prim_color.r;		*input_g = &prim_color.g;		*input_b = &prim_color.b;		break;
		case 4:		*input_r = &shade_color.r;		*input_g = &shade_color.g;		*input_b = &shade_color.b;		break;
		case 5:		*input_r = &env_color.r;		*input_g = &env_color.g;		*input_b = &env_color.b;		break;
		case 6:		*input_r = &one_color.r;		*input_g = &one_color.g;		*input_b = &one_color.b;		break;
		case 7:		break; //TODO fatalerror("SET_SUBA_RGB_INPUT: noise\n"); break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
			*input_r = &zero_color.r;		*input_g = &zero_color.g;		*input_b = &zero_color.b;		break;
		}
	}
}

INLINE void SET_SUBB_RGB_INPUT(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &combined_color.r;	*input_g = &combined_color.g;	*input_b = &combined_color.b;	break;
		case 1:		*input_r = &texel0_color.r;		*input_g = &texel0_color.g;		*input_b = &texel0_color.b;		break;
		case 2:		*input_r = &texel1_color.r;		*input_g = &texel1_color.g;		*input_b = &texel1_color.b;		break;
		case 3:		*input_r = &prim_color.r;		*input_g = &prim_color.g;		*input_b = &prim_color.b;		break;
		case 4:		*input_r = &shade_color.r;		*input_g = &shade_color.g;		*input_b = &shade_color.b;		break;
		case 5:		*input_r = &env_color.r;		*input_g = &env_color.g;		*input_b = &env_color.b;		break;
		case 6:		fatalerror("SET_SUBB_RGB_INPUT: key_center\n"); break;
		case 7:		fatalerror("SET_SUBB_RGB_INPUT: convert_k4\n"); break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
			*input_r = &zero_color.r;		*input_g = &zero_color.g;		*input_b = &zero_color.b;		break;
		}
	}
}

INLINE void SET_MUL_RGB_INPUT(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code)
{
	switch (code & 0x1f)
	{
		case 0:		*input_r = &combined_color.r;	*input_g = &combined_color.g;	*input_b = &combined_color.b;	break;
		case 1:		*input_r = &texel0_color.r;		*input_g = &texel0_color.g;		*input_b = &texel0_color.b;		break;
		case 2:		*input_r = &texel1_color.r;		*input_g = &texel1_color.g;		*input_b = &texel1_color.b;		break;
		case 3:		*input_r = &prim_color.r;		*input_g = &prim_color.g;		*input_b = &prim_color.b;		break;
		case 4:		*input_r = &shade_color.r;		*input_g = &shade_color.g;		*input_b = &shade_color.b;		break;
		case 5:		*input_r = &env_color.r;		*input_g = &env_color.g;		*input_b = &env_color.b;		break;
		case 6:		fatalerror("SET_MUL_RGB_INPUT: key scale\n"); break;
		case 7:		*input_r = &combined_color.a;	*input_g = &combined_color.a;	*input_b = &combined_color.a;	break;
		case 8:		*input_r = &texel0_color.a;		*input_g = &texel0_color.a;		*input_b = &texel0_color.a;		break;
		case 9:		*input_r = &texel1_color.a;		*input_g = &texel1_color.a;		*input_b = &texel1_color.a;		break;
		case 10:	*input_r = &prim_color.a;		*input_g = &prim_color.a;		*input_b = &prim_color.a;		break;
		case 11:	*input_r = &shade_color.a;		*input_g = &shade_color.a;		*input_b = &shade_color.a;		break;
		case 12:	*input_r = &env_color.a;		*input_g = &env_color.a;		*input_b = &env_color.a;		break;
		case 13:	break;//TODO fatalerror("SET_MUL_RGB_INPUT: lod fraction\n"); break;
		case 14:	break;//TODO fatalerror("SET_MUL_RGB_INPUT: primitive lod fraction\n"); break;
		case 15:	break;//TODO fatalerror("SET_MUL_RGB_INPUT: convert k5\n"); break;
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
		{
			*input_r = &zero_color.r;		*input_g = &zero_color.g;		*input_b = &zero_color.b;		break;
		}
	}
}

INLINE void SET_ADD_RGB_INPUT(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code)
{
	switch (code & 0x7)
	{
		case 0:		*input_r = &combined_color.r;	*input_g = &combined_color.g;	*input_b = &combined_color.b;	break;
		case 1:		*input_r = &texel0_color.r;		*input_g = &texel0_color.g;		*input_b = &texel0_color.b;		break;
		case 2:		*input_r = &texel1_color.r;		*input_g = &texel1_color.g;		*input_b = &texel1_color.b;		break;
		case 3:		*input_r = &prim_color.r;		*input_g = &prim_color.g;		*input_b = &prim_color.b;		break;
		case 4:		*input_r = &shade_color.r;		*input_g = &shade_color.g;		*input_b = &shade_color.b;		break;
		case 5:		*input_r = &env_color.r;		*input_g = &env_color.g;		*input_b = &env_color.b;		break;
		case 6:		*input_r = &one_color.r;		*input_g = &one_color.g;		*input_b = &one_color.b;		break;
		case 7:		*input_r = &zero_color.r;		*input_g = &zero_color.g;		*input_b = &zero_color.b;		break;
	}
}

INLINE void SET_SUB_ALPHA_INPUT(UINT8 **input, int code)
{
	switch (code & 0x7)
	{
		case 0:		*input = &combined_color.a; break;
		case 1:		*input = &texel0_color.a; break;
		case 2:		*input = &texel1_color.a; break;
		case 3:		*input = &prim_color.a; break;
		case 4:		*input = &shade_color.a; break;
		case 5:		*input = &env_color.a; break;
		case 6:		*input = &one_color.a; break;
		case 7:		*input = &zero_color.a; break;
	}
}

INLINE void SET_MUL_ALPHA_INPUT(UINT8 **input, int code)
{
	switch (code & 0x7)
	{
		case 0:		break;//TODO fatalerror("SET_MUL_ALPHA_INPUT: lod fraction\n"); break;
		case 1:		*input = &texel0_color.a; break;
		case 2:		*input = &texel1_color.a; break;
		case 3:		*input = &prim_color.a; break;
		case 4:		*input = &shade_color.a; break;
		case 5:		*input = &env_color.a; break;
		case 6:		break;//TODO fatalerror("SET_MUL_ALPHA_INPUT: primitive lod fraction\n"); break;
		case 7:		*input = &zero_color.a; break;
	}
}



INLINE COLOR COLOR_COMBINER(int cycle)
{
	COLOR c;
	UINT32 r, g, b, a;

    r = (((UINT32)((UINT32)(*combiner_rgbsub_a_r[cycle]) - (UINT32)(*combiner_rgbsub_b_r[cycle])) *
        *combiner_rgbmul_r[cycle]) >> 8);

    g = (((UINT32)((UINT32)(*combiner_rgbsub_a_g[cycle]) - (UINT32)(*combiner_rgbsub_b_g[cycle])) *
        *combiner_rgbmul_g[cycle]) >> 8);

    b = (((UINT32)((UINT32)(*combiner_rgbsub_a_b[cycle]) - (UINT32)(*combiner_rgbsub_b_b[cycle])) *
        *combiner_rgbmul_b[cycle]) >> 8);

    a = (((UINT32)((UINT32)(*combiner_alphasub_a[cycle]) - (UINT32)(*combiner_alphasub_b[cycle])) *
        *combiner_alphamul[cycle]) >> 8);

    if (r & 0xffffff00)
    {
        if(r & 0x00000080)
        {
            r = 0;
        }
        else
        {
            r = 255;
        }
    }
    if (g & 0xffffff00)
    {
        if(g & 0x00000080)
        {
            g = 0;
        }
        else
        {
            g = 255;
        }
    }
    if (b & 0xffffff00)
    {
        if(b & 0x00000080)
        {
            b = 0;
        }
        else
        {
            b = 255;
        }
    }
    if (a & 0xffffff00)
    {
        if(a & 0x00000080)
        {
            a = 0;
        }
        else
        {
            a = 255;
        }
    }

    c.r = r; c.g = g; c.b = b; c.a = a;

    r += *combiner_rgbadd_r[cycle];
    g += *combiner_rgbadd_g[cycle];
    b += *combiner_rgbadd_b[cycle];
    a += *combiner_alphaadd[cycle];

    r &= 0x000000ff;
    g &= 0x000000ff;
    b &= 0x000000ff;
    a &= 0x000000ff;

    /*
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (a > 255) a = 255;
    */

    c.r = r; c.g = g; c.b = b; c.a = a;

    combined_color.r = r;
    combined_color.g = g;
    combined_color.b = b;
    combined_color.a = a;

    return c;
}

INLINE void SET_BLENDER_INPUT(int cycle, int which, UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, UINT8 **input_a, int a, int b)
{
	switch (a & 0x3)
	{
		case 0:
		{
			if (cycle == 0)
			{
				*input_r = &pixel_color.r;
				*input_g = &pixel_color.g;
				*input_b = &pixel_color.b;
			}
			else
			{
				*input_r = &blended_pixel_color.r;
				*input_g = &blended_pixel_color.g;
				*input_b = &blended_pixel_color.b;
			}
			break;
		}

		case 1:
		{
			//fatalerror("SET_BLENDER_INPUT: cycle %d, input A: memory color\n", cycle);
			// TODO
			*input_r = &memory_color.r;
			*input_g = &memory_color.g;
			*input_b = &memory_color.b;
			break;
		}

		case 2:
		{
			*input_r = &blend_color.r;		*input_g = &blend_color.g;		*input_b = &blend_color.b;
			break;
		}

		case 3:
		{
			*input_r = &fog_color.r;		*input_g = &fog_color.g;		*input_b = &fog_color.b;
			break;
		}
	}

	if (which == 0)
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &pixel_color.a; break;
			case 1:		*input_a = &one_color.a; break;
			case 2:		*input_a = &shade_color.a; break;
			case 3:		*input_a = &zero_color.a; break;
		}
	}
	else
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &inv_pixel_color.a; break;
			case 1:		*input_a = &memory_color.a; break;
			case 2:		*input_a = &one_color.a; break;
			case 3:		*input_a = &zero_color.a; break;
		}
	}
}

static const UINT8 dither_matrix_4x4[16] =
{
	 0,  8,  2, 10,
	12,  4, 14,  6,
	 3, 11,  1,  9,
	15,  7, 13,  5
};

#define DITHER_RB(val,dith)	((((val) << 1) - ((val) >> 4) + ((val) >> 7) + (dith)) >> 1)
#define DITHER_G(val,dith)	((((val) << 2) - ((val) >> 4) + ((val) >> 6) + (dith)) >> 2)

INLINE void BLENDER1_16(UINT16 *fb, COLOR c, int dith, UINT8 *new_cvg, UINT8 *old_cvg, int *alpha_reject)
{
    int r, g, b;

    if (other_modes.force_blend )
    {
        blend_color.r = c.r;
        blend_color.g = c.g;
        blend_color.b = c.b;
    }

    pixel_color.r = c.r;
    pixel_color.g = c.g;
    pixel_color.b = c.b;
    pixel_color.a = c.a;
    inv_pixel_color.r = 0xff - *blender1a_r[0];
    inv_pixel_color.g = 0xff - *blender1a_g[0];
    inv_pixel_color.b = 0xff - *blender1a_b[0];
    inv_pixel_color.a = 0xff - *blender1b_a[0];

    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }
        inv_pixel_color.a = 0xff - pixel_color.a;

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }

    if (other_modes.image_read_en)
    {
        UINT16 mem = *fb;
        memory_color.r = (((mem >> 11) & 0x1f) << 3) | ((mem >> 13) & 0x07);
        memory_color.g = (((mem >>  6) & 0x1f) << 3) | ((mem >>  8) & 0x07);
        memory_color.b = (((mem >>  1) & 0x1f) << 3) | ((mem >>  3) & 0x07);
        memory_color.a = /*(UINT8)((INT32)(*old_cvg) * 0x20 - 1)*/0;
    }

    if (((int)(*blender1b_a[0]) + (int)(*blender2b_a[0])) > 0)
    {
        r = ((int)(*blender1a_r[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_r[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        g = ((int)(*blender1a_g[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_g[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        b = ((int)(*blender1a_b[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_b[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));
    }
    else
    {
        r = g = b = 0;
    }

    //r = g = b = 0xff;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    if (other_modes.rgb_dither_sel != 3)
    {
        r = DITHER_RB(r, dith);
        g = DITHER_G (g, dith);
        b = DITHER_RB(b, dith);
    }

    *fb = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1;
}

INLINE void BLENDER2_16(UINT16 *fb, COLOR c1, COLOR c2, int dith, UINT8 *new_cvg, UINT8 *old_cvg, int *alpha_reject)
{
    int r, g, b;

    if( other_modes.force_blend )
    {
        blend_color.r = c2.r;
        blend_color.g = c2.g;
        blend_color.b = c2.b;
    }

    pixel_color.r = c2.r;
    pixel_color.g = c2.g;
    pixel_color.b = c2.b;
    pixel_color.a = c2.a;
    inv_pixel_color.r = 0xff - *blender1a_r[0];
    inv_pixel_color.g = 0xff - *blender1a_g[0];
    inv_pixel_color.b = 0xff - *blender1a_b[0];
    inv_pixel_color.a = 0xff - *blender1b_a[0];
    *alpha_reject = 0;

    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c2.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }
        //inv_pixel_color.a = 0xff - pixel_color.a;

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }

    if (other_modes.image_read_en)
    {
        UINT16 mem = *fb;
        memory_color.r = (((mem >> 11) & 0x1f) << 3) | ((mem >> 13) & 0x07);
        memory_color.g = (((mem >>  6) & 0x1f) << 3) | ((mem >>  8) & 0x07);
        memory_color.b = (((mem >>  1) & 0x1f) << 3) | ((mem >>  3) & 0x07);
        memory_color.a = /*(UINT8)((INT32)(*old_cvg) * 0x20 - 1)*/0;
    }

    if (((int)(*blender1b_a[0]) + (int)(*blender2b_a[0])) > 0)
    {
        r = ((int)(*blender1a_r[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_r[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        g = ((int)(*blender1a_g[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_g[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        b = ((int)(*blender1a_b[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_b[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));
    }
    else
    {
        r = g = b = 0;
    }

    blended_pixel_color.r = r;
    blended_pixel_color.g = g;
    blended_pixel_color.b = b;

    pixel_color.r = c2.r;
    pixel_color.g = c2.g;
    pixel_color.b = c2.b;
    pixel_color.a = c2.a;
    inv_pixel_color.r = 0xff - *blender1a_r[1];
    inv_pixel_color.g = 0xff - *blender1a_g[1];
    inv_pixel_color.b = 0xff - *blender1a_b[1];
    inv_pixel_color.a = 0xff - *blender1b_a[1];

    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c2.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }

    if (((int)(*blender1b_a[1]) + (int)(*blender2b_a[1])) > 0)
    {
        r = ((int)(*blender1a_r[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_r[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));

        g = ((int)(*blender1a_g[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_g[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));

        b = ((int)(*blender1a_b[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_b[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));
    }

    //if (r > 255) r = 255;
    //if (g > 255) g = 255;
    //if (b > 255) b = 255;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0)   r = 0;
    if (g < 0)   g = 0;
    if (b < 0)   b = 0;
    //r &= 0x000000ff;
    //g &= 0x000000ff;
    //b &= 0x000000ff;

    if (other_modes.rgb_dither_sel != 3)
    {
        r = DITHER_RB(r, dith);
        g = DITHER_G (g, dith);
        b = DITHER_RB(b, dith);
    }

    *fb = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1;
}

/*****************************************************************************/

static void fill_rectangle_16bit(RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
    int alpha_reject = 0;
    COLOR c1, c2;

	UINT16 fill_color1, fill_color2;
	fill_color1 = (fill_color >> 16) & 0xffff;
	fill_color2 = (fill_color >>  0) & 0xffff;

    c1.r = c1.g = c1.b = c1.a = 0;
    c2.r = c2.g = c2.b = c2.a = 0;

	// TODO: clip
	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	// clip
	if (x1 < clipx1)
	{
		x1 = clipx1;
	}
	if (y1 < clipy1)
	{
		y1 = clipy1;
	}
	if (x2 >= clipx2)
	{
		x2 = clipx2-1;
	}
	if (y2 >= clipy2)
	{
		y2 = clipy2-1;
	}

	if (other_modes.cycle_type == CYCLE_TYPE_FILL)
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1; i <= x2; i++)
			{
                cvg_buf[(index + i) ^ 1] = (i & 1) ? ( ( fill_color1 & 1 ) ? 8 : 1 ) : ( ( fill_color2 & 1 ) ? 8 : 1 );
				fb[(index + i) ^ 1] = (i & 1) ? fill_color1 : fill_color2;
			}
		}
	}
    else if (other_modes.cycle_type == CYCLE_TYPE_1)
    {
        shade_color.r = prim_color.r;
        shade_color.g = prim_color.g;
        shade_color.b = prim_color.b;
        shade_color.a = prim_color.a;

        for (j=y1; j <= y2; j++)
        {
            index = j * fb_width;
            for (i=x1; i <= x2; i++)
            {
                int dith = dither_matrix_4x4[(((j) & 3) << 2) + ((i^WORD_ADDR_XOR) & 3)];

                c1 = COLOR_COMBINER(0);

                cvg_buf[(index + i) ^ WORD_ADDR_XOR] = 8; // Wrong
                BLENDER1_16(&fb[(index + i) ^ WORD_ADDR_XOR], c1, dith, &cvg_buf[(index + i) ^ WORD_ADDR_XOR], &cvg_buf[(index + i) ^ WORD_ADDR_XOR], &alpha_reject);
            }
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_2)
    {
        shade_color.r = prim_color.r;
        shade_color.g = prim_color.g;
        shade_color.b = prim_color.b;
        shade_color.a = prim_color.a;

        for (j=y1; j <= y2; j++)
        {
            index = j * fb_width;
            for (i=x1; i <= x2; i++)
            {
                int dith = dither_matrix_4x4[(((j) & 3) << 2) + ((i^WORD_ADDR_XOR) & 3)];

                c1 = COLOR_COMBINER(0);
                c2 = COLOR_COMBINER(1);

                cvg_buf[(index + i) ^ WORD_ADDR_XOR] = 8; // Wrong
                BLENDER2_16(&fb[(index + i) ^ WORD_ADDR_XOR], c1, c2, dith, &cvg_buf[(index + i) ^ WORD_ADDR_XOR], &cvg_buf[(index + i) ^ WORD_ADDR_XOR], &alpha_reject);
            }
        }
    }
    else
    {
        fatalerror("fill_rectangle_16bit: cycle type copy");
    }
}

#define XOR_SWAP_BYTE	4
#define XOR_SWAP_WORD	2
#define XOR_SWAP_DWORD	1

INLINE int taddr_clamp(int addr)
{
    return (addr > 0x100000) ? 0 : addr;
}

INLINE void FETCH_TEXEL(COLOR *color, int s, int t, UINT32 twidth, UINT32 tformat, UINT32 tsize, UINT32 tbase, UINT8 tpal)
{
	if (t < 0) t = 0;
	if (s < 0) s = 0;

	switch (tformat)
	{
		case 0:		// RGBA
		{
			switch (tsize)
			{
                case PIXEL_SIZE_4BIT:
                {
                    // 4-bit RGBA is a "valid" mode; some games use it, it behaves the same as 4-bit CI
                    UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
                    UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
                    UINT16 c = tlut[(p + (tpal << 4))^ WORD_ADDR_XOR];

                    if (other_modes.tlut_type == 0)
                    {
                        color->r = ((c >> 11) & 0x1f) << 3;
                        color->g = ((c >>  6) & 0x1f) << 3;
                        color->b = ((c >>  1) & 0x1f) << 3;
                        color->a = (c & 1) ? 0xff : 0;
                    }
                    else
                    {
                        color->r = color->g = color->b = (c >> 8) & 0xff;
                        color->a = c & 0xff;
                    }
                    break;
                }
                case PIXEL_SIZE_8BIT:
                {
                    // 8-bit RGBA is a "valid" mode; some games use it, it behaves the same as 8-bit CI
                    UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
                    UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
                    UINT16 c = tlut[p ^ WORD_ADDR_XOR];

                    if (other_modes.tlut_type == 0)
                    {
                        color->r = ((c >> 11) & 0x1f) << 3;
                        color->g = ((c >>  6) & 0x1f) << 3;
                        color->b = ((c >>  1) & 0x1f) << 3;
                        color->a = (c & 1) ? 0xff : 0;
                    }
                    else
                    {
                        color->r = color->g = color->b = (c >> 8) & 0xff;
                        color->a = c & 0xff;
                    }
                    break;
                }
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = (UINT16*)texture_cache;
                    int taddr = taddr_clamp(((tbase/2) + ((t) * (twidth/2)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0));
					UINT16 c = tc[taddr ^ WORD_ADDR_XOR];

					color->r = ((c >> 11) & 0x1f) << 3;
					color->g = ((c >>  6) & 0x1f) << 3;
					color->b = ((c >>  1) & 0x1f) << 3;
					color->a = (c & 1) ? 0xff : 0;
					break;
				}
				case PIXEL_SIZE_32BIT:
				{
					UINT32 *tc = (UINT32*)texture_cache;
                    int taddr = taddr_clamp(((tbase/4) + ((t) * (twidth/2)) + (s)) ^ ((t & 1) ? XOR_SWAP_DWORD : 0));
					UINT32 c = tc[taddr];

					color->r = ((c >> 24) & 0xff);
					color->g = ((c >> 16) & 0xff);
					color->b = ((c >>  8) & 0xff);
					color->a = ((c >>  0) & 0xff);
					break;
				}
				default:
					color->r = color->g = color->b = color->a = 0xff;
					fatalerror("FETCH_TEXEL: unknown RGBA texture size %d\n", tsize);
					break;
			}
			break;
		}
		case 2:		// Color Index
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					UINT16 c = tlut[p ^ WORD_ADDR_XOR];

					if (other_modes.tlut_type == 0)
					{
						color->r = ((c >> 11) & 0x1f) << 3;
						color->g = ((c >>  6) & 0x1f) << 3;
						color->b = ((c >>  1) & 0x1f) << 3;
						color->a = (c & 1) ? 0xff : 0;
					}
					else
					{
						color->r = color->g = color->b = (c >> 8) & 0xff;
						color->a = c & 0xff;
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
					UINT16 c = tlut[p ^ WORD_ADDR_XOR];

					if (other_modes.tlut_type == 0)
					{
						color->r = ((c >> 11) & 0x1f) << 3;
						color->g = ((c >>  6) & 0x1f) << 3;
						color->b = ((c >>  1) & 0x1f) << 3;
						color->a = (c & 1) ? 0xff : 0;
					}
					else
					{
						color->r = color->g = color->b = (c >> 8) & 0xff;
						color->a = c & 0xff;
					}
					break;
				}
                case PIXEL_SIZE_16BIT:
                {
                    // 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
                    UINT16 *tc = (UINT16*)texture_cache;
                    int taddr = taddr_clamp(((tbase/2) + ((t) * (twidth/2)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0));
                    UINT16 c = tc[taddr ^ WORD_ADDR_XOR];

                    color->r = ((c >> 11) & 0x1f) << 3;
                    color->g = ((c >>  6) & 0x1f) << 3;
                    color->b = ((c >>  1) & 0x1f) << 3;
                    color->a = (c & 1) ? 0xff : 0;
                    break;
                }
				default:
					color->r = color->g = color->b = color->a = 0xff;
					fatalerror("FETCH_TEXEL: unknown CI texture size %d\n", tsize);
					break;
			}
			break;
		}
		case 3:		// Intensity + Alpha
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					UINT8 i = ((p & 0xe) << 4) | ((p & 0xe) << 1) | (p & 0xe >> 2);

					color->r = i;
					color->g = i;
					color->b = i;
					color->a = (p & 0x1) ? 0xff : 0;
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
					UINT8 i = (p >> 4) | (p & 0xf0);

					color->r = i;
					color->g = i;
					color->b = i;
					color->a = (p & 0xf) | ((p << 4) & 0xf0);
					break;
				}
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = (UINT16*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * (twidth/2)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0));
					UINT16 c = tc[taddr ^ WORD_ADDR_XOR];
					UINT8 i = (c >> 8);

					color->r = i;
					color->g = i;
					color->b = i;
					color->a = c & 0xff;
					break;
				}
				default:
					color->r = color->g = color->b = color->a = 0xff;
					fatalerror("FETCH_TEXEL: unknown IA texture size %d\n", tsize);
					break;
			}
			break;
		}
		case 4:		// Intensity
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 c = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					c |= (c << 4);

					color->r = c;
					color->g = c;
					color->b = c;
					color->a = c;
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)texture_cache;
                    int taddr = taddr_clamp((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0));
					UINT8 c = tc[taddr ^ BYTE_ADDR_XOR];

					color->r = c;
					color->g = c;
					color->b = c;
					color->a = c;
					break;
				}
				default:
					color->r = color->g = color->b = color->a = 0xff;
					fatalerror("FETCH_TEXEL: unknown I texture size %d\n", tsize);
					break;
			}
			break;
		}
		default:
		{
			color->r = color->g = color->b = color->a = 0xff;
			fatalerror("FETCH_TEXEL: unknown texture format %d\n", tformat);
			break;
		}
	}
}

#if 1
#define TEXTURE_PIPELINE(TEX, SSS, SST, NOBILINEAR, CYCLE)                                                                          \
do                                                                                                                                  \
{                                                                                                                                   \
    if (other_modes.sample_type && !NOBILINEAR)                                                                                     \
    {                                                                                                                               \
        COLOR t0, t1, t2, t3;                                                                                                       \
        int sss1, sst1, sss2, sst2;                                                                                                 \
                                                                                                                                    \
        /* bilinear */                                                                                                              \
                                                                                                                                    \
        sss1 = SSS >> 10;                                                                                                           \
        sss2 = sss1 + 1;                                                                                                            \
                                                                                                                                    \
        sst1 = SST >> 10;                                                                                                           \
        sst2 = sst1 + 1;                                                                                                            \
                                                                                                                                    \
        sss1 -= tsl[CYCLE] >> 2;                                                                                                    \
        sss2 -= tsl[CYCLE] >> 2;                                                                                                    \
                                                                                                                                    \
        sst1 -= ttl[CYCLE] >> 2;                                                                                                    \
        sst2 -= ttl[CYCLE] >> 2;                                                                                                    \
                                                                                                                                    \
        CLAMP(sss1, sst1, CYCLE);                                                                                                   \
        CLAMP(sss2, sst2, CYCLE);                                                                                                   \
                                                                                                                                    \
        MASK(sss1, sst1, CYCLE);                                                                                                    \
        MASK(sss2, sst2, CYCLE);                                                                                                    \
                                                                                                                                    \
        FETCH_TEXEL(&t0, sss1, sst1, twidth[CYCLE], tformat[CYCLE], tsize[CYCLE], tbase[CYCLE], tpal[CYCLE]);                       \
        FETCH_TEXEL(&t1, sss2, sst1, twidth[CYCLE], tformat[CYCLE], tsize[CYCLE], tbase[CYCLE], tpal[CYCLE]);                       \
        FETCH_TEXEL(&t2, sss1, sst2, twidth[CYCLE], tformat[CYCLE], tsize[CYCLE], tbase[CYCLE], tpal[CYCLE]);                       \
        FETCH_TEXEL(&t3, sss2, sst2, twidth[CYCLE], tformat[CYCLE], tsize[CYCLE], tbase[CYCLE], tpal[CYCLE]);                       \
                                                                                                                                    \
        TEX.r = (((( (t0.r * (0x3ff - (SSS & 0x3ff))) + (t1.r * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                         (((( (t2.r * (0x3ff - (SSS & 0x3ff))) + (t3.r * ((SSS & 0x3ff))) ) >> 10) * ((SST & 0x3ff))) >> 10);       \
                                                                                                                                    \
        TEX.g = (((( (t0.g * (0x3ff - (SSS & 0x3ff))) + (t1.g * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                         (((( (t2.g * (0x3ff - (SSS & 0x3ff))) + (t3.g * ((SSS & 0x3ff))) ) >> 10) * ((SST & 0x3ff))) >> 10);       \
                                                                                                                                    \
        TEX.b = (((( (t0.b * (0x3ff - (SSS & 0x3ff))) + (t1.b * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                         (((( (t2.b * (0x3ff - (SSS & 0x3ff))) + (t3.b * ((SSS & 0x3ff))) ) >> 10) * ((SST & 0x3ff))) >> 10);       \
                                                                                                                                    \
        TEX.a = (((( (t0.a * (0x3ff - (SSS & 0x3ff))) + (t1.a * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                         (((( (t2.a * (0x3ff - (SSS & 0x3ff))) + (t3.a * ((SSS & 0x3ff))) ) >> 10) * ((SST & 0x3ff))) >> 10);       \
    }                                                                                                                               \
    else                                                                                                                            \
    {                                                                                                                               \
        int sss1, sst1;                                                                                                             \
        sss1 = SSS >> 10;                                                                                                           \
        if ((SSS & 0x3ff) >= 0x200) sss1++;                                                                                         \
                                                                                                                                    \
        sst1 = SST >> 10;                                                                                                           \
        if ((SST & 0x3ff) >= 0x200) sst1++;                                                                                         \
                                                                                                                                    \
        sss1 -= tsl[CYCLE] >> 2;                                                                                                    \
        sst1 -= ttl[CYCLE] >> 2;                                                                                                    \
                                                                                                                                    \
        CLAMP(sss1, sst1, CYCLE);                                                                                                   \
                                                                                                                                    \
        MASK(sss1, sst1, CYCLE);                                                                                                    \
                                                                                                                                    \
        /* point sample */                                                                                                          \
        FETCH_TEXEL(&TEX, sss1, sst1, twidth[CYCLE], tformat[CYCLE], tsize[CYCLE], tbase[CYCLE], tpal[CYCLE]);                      \
    }                                                                                                                               \
}                                                                                                                                   \
while(0)
#endif

// This is the implementation with 3-sample bilinear filtering, which should be more correct, but not yet working properly
// TODO: check the correct texel samples and weighting values

#if 0
#define TEXTURE_PIPELINE(TEX, SSS, SST, NOBILINEAR)                                                                                 \
do                                                                                                                                  \
{                                                                                                                                   \
    if (other_modes.sample_type && !NOBILINEAR)                                                                                     \
    {                                                                                                                               \
        COLOR t0, t1, t2;                                                                                                           \
        int sss1, sst1, sss2, sst2;                                                                                                 \
                                                                                                                                    \
        /* bilinear */                                                                                                              \
                                                                                                                                    \
        sss1 = (SSS >> 10) - (tsl >> 2);                                                                                            \
        sss2 = sss1 + 1;                                                                                                            \
                                                                                                                                    \
        sst1 = (SST >> 10) - (ttl >> 2);                                                                                            \
        sst2 = sst1 + 1;                                                                                                            \
                                                                                                                                    \
        CLAMP(sss1, sst1);                                                                                                          \
        CLAMP(sss2, sst2);                                                                                                          \
                                                                                                                                    \
        FETCH_TEXEL(&t0, sss1, sst1, twidth, tformat, tsize, tbase);                                                                \
        FETCH_TEXEL(&t1, sss2, sst1, twidth, tformat, tsize, tbase);                                                                \
        FETCH_TEXEL(&t2, sss2, sst2, twidth, tformat, tsize, tbase);                                                                \
                                                                                                                                    \
        TEX.r = (((( (t0.r * (0x3ff - (SSS & 0x3ff))) + (t1.r * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                (( (t2.r) * ((SST & 0x3ff))) >> 10);                                                                                \
                                                                                                                                    \
        TEX.g = (((( (t0.g * (0x3ff - (SSS & 0x3ff))) + (t1.g * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                (( (t2.g) * ((SST & 0x3ff))) >> 10);                                                                                \
                                                                                                                                    \
        TEX.b = (((( (t0.b * (0x3ff - (SSS & 0x3ff))) + (t1.b * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                (( (t2.b) * ((SST & 0x3ff))) >> 10);                                                                                \
                                                                                                                                    \
        TEX.a = (((( (t0.a * (0x3ff - (SSS & 0x3ff))) + (t1.a * ((SSS & 0x3ff))) ) >> 10) * (0x3ff - (SST & 0x3ff))) >> 10) +       \
                (( (t2.a) * ((SST & 0x3ff))) >> 10);                                                                                \
    }                                                                                                                               \
    else                                                                                                                            \
    {                                                                                                                               \
        int sss1, sst1;                                                                                                             \
        sss1 = (SSS >> 10) - (tsl >> 2);                                                                                            \
        if ((SSS & 0x3ff) >= 0x200) sss1++;                                                                                         \
                                                                                                                                    \
        sst1 = (SST >> 10) - (ttl >> 2);                                                                                            \
        if ((SST & 0x3ff) >= 0x200) sst1++;                                                                                         \
                                                                                                                                    \
        CLAMP(sss1, sst1);                                                                                                          \
                                                                                                                                    \
        /* point sample */                                                                                                          \
        FETCH_TEXEL(&TEX, sss1, sst1, twidth, tformat, tsize, tbase);                                                               \
    }                                                                                                                               \
}                                                                                                                                   \
while(0)
#endif

#define TEXTURE_PIPELINE1(SSS, SST, NOBILINEAR)             \
do                                                          \
{                                                           \
    TEXTURE_PIPELINE(texel0_color, SSS, SST, NOBILINEAR, 0);\
}                                                           \
while(0)

#define TEXTURE_PIPELINE2(SSS, SST, NOBILINEAR)             \
do                                                          \
{                                                           \
    TEXTURE_PIPELINE(texel0_color, SSS, SST, NOBILINEAR, 0);\
    TEXTURE_PIPELINE(texel1_color, SSS, SST, NOBILINEAR, 1);\
}                                                           \
while(0)

static int alpha_compare_success(COLOR c1, COLOR c2)
{
    if( other_modes.alpha_compare_en )
    {
        if( other_modes.cycle_type == CYCLE_TYPE_1 )
        {
            if( c1.a > blend_color.a )
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        if( other_modes.cycle_type == CYCLE_TYPE_2 )
        {
            if( c2.a > blend_color.a )
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

/* Does not yet work properly */
static void compress_z(UINT32 in, UINT16 *out_z, UINT8 *out_dz)
{
    UINT32 rounded = (in >> 13) & 0x0003ffff;
    UINT16 full_delta = in & 0x0000ffff;
    UINT8 exponent = 0;
    UINT8 high_delta_bit = 15;
    UINT16 mantissa = 0;
    UINT32 final_value = 0;

    // Determine exponent and mantissa
    for (exponent = 0; exponent < 7; exponent++)
    {
        if (!(rounded & (0x00020000 >> exponent)))
        {
            break;
        }
    }
    mantissa = (rounded >> ((exponent != 7) ? (6 - exponent) : 0)) & 0x000007ff;

    // Determine the high bit of Z delta
    for (high_delta_bit = 15; high_delta_bit > 0; high_delta_bit--)
    {
        if (full_delta & (1 << high_delta_bit))
        {
            break;
        }
    }

    // Composite the final value
    final_value = (mantissa << 7) | (exponent << 4) | (high_delta_bit & 0x0f);
    *out_z = (final_value >> 2) & 0xfffc;
    *out_dz = final_value & 0x000f;
}

/* Does not yet work properly */
static void decompress_z(UINT16 in, UINT32 *out)
{
    UINT16 mantissa = (in >> 5) & 0x07ff;
    UINT8 exponent = (in >> 2) & 0x0007;
    switch (exponent)
    {
        case 0:
            (*out) = (mantissa << 6);
            break;
        case 1:
            (*out) = 0x20000 | (mantissa << 5);
            break;
        case 2:
            (*out) = 0x30000 | (mantissa << 4);
            break;
        case 3:
            (*out) = 0x38000 | (mantissa << 3);
            break;
        case 4:
            (*out) = 0x3c000 | (mantissa << 2);
            break;
        case 5:
            (*out) = 0x3e000 | (mantissa << 1);
            break;
        case 6:
            (*out) = 0x3f000 | mantissa;
            break;
        case 7:
            (*out) = 0x3f800 | mantissa;
            break;
    }
}

static void calculate_coverage( UINT8 *out, UINT8 in, int* ovf, UINT8 *old )
{
    (*old) = (*out);
    if (((*out) + in) > 8)
    {
        (*ovf) = 1;
    }
    switch (other_modes.cvg_dest)
    {
        case 0: // Clamp
            if (other_modes.image_read_en)
            {
                (*out) = (*out) + in;
            }
            else
            {
                (*out) = in;
            }
            if (*out > 8)
            {
                *out = 8;
            }
            break;
        case 1: // Wrap
            if (other_modes.image_read_en)
            {
                (*out) = ( (*out) - 1 ) + in;
            }
            else
            {
                (*out) = in;
            }
            (*out) &= 0x07;
            (*out)++;
            break;
        case 2: // Zap
            (*out) = 8;
            break;
        case 3: // Save
            (*out) = (*out);
            break;
    }
}

static void texture_rectangle_16bit(TEX_RECTANGLE *rect)
{
    UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
    UINT16 *zb = (UINT16*)&rdram[(zb_address / 4)];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
    int i, j;
    int x1, x2, y1, y2;
    int tsl[2], tsh[2], ttl[2], tth[2];
    int s, t;
    int twidth[2], tformat[2], tsize[2], tbase[2], tpal[2];
    int clamp_s[2], clamp_t[2], mask_s[2], mask_t[2], mirror_s[2], mirror_t[2];
    int clipx1, clipx2, clipy1, clipy2;
    int cycle;
    UINT8 cvg_span[RDP_CVG_SPAN_MAX];
    int span_index = 0;
    int alpha_reject = 0;
    COLOR c1, c2;
    c1.r = c1.g = c1.b = c1.a = 0;
    c2.r = c2.g = c2.b = c2.a = 0;

    x1 = rect->xh;
    x2 = rect->xl;
    y1 = rect->yh;
    y2 = rect->yl;

    if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
    {
        rect->dsdx /= 4;
        x2 += 1;
        y2 += 1;
    }

    clipx1 = clip.xh;
    clipx2 = clip.xl;
    clipy1 = clip.yh;
    clipy2 = clip.yl;

    // clip
    if (x1 < clipx1)
    {
        x1 = clipx1;
    }
    if (y1 < clipy1)
    {
        y1 = clipy1;
    }
    if (x2 >= clipx2)
    {
        x2 = clipx2-1;
    }
    if (y2 >= clipy2)
    {
        y2 = clipy2-1;
    }

    for( cycle = 0; cycle < 2; cycle++ )
    {
        tsl[cycle] = tile[rect->tilenum + cycle].sl;
        tsh[cycle] = tile[rect->tilenum + cycle].sh;
        ttl[cycle] = tile[rect->tilenum + cycle].tl;
        tth[cycle] = tile[rect->tilenum + cycle].th;

        twidth[cycle] = tile[rect->tilenum + cycle].line;
        tformat[cycle] = tile[rect->tilenum + cycle].format;
        tsize[cycle] = tile[rect->tilenum + cycle].size;
        tbase[cycle] = tile[rect->tilenum + cycle].tmem;
        tpal[cycle] = tile[rect->tilenum + cycle].palette;

        clamp_t[cycle] = tile[rect->tilenum + cycle].ct;
        clamp_s[cycle] = tile[rect->tilenum + cycle].cs;

        mirror_t[cycle] = tile[rect->tilenum + cycle].mt;
        mirror_s[cycle] = tile[rect->tilenum + cycle].ms;
        mask_t[cycle] = tile[rect->tilenum + cycle].mask_t;
        mask_s[cycle] = tile[rect->tilenum + cycle].mask_s;
    }

    t = (int)(rect->t) << 5;

    if (other_modes.cycle_type == CYCLE_TYPE_1)
    {
        for (j = y1; j < y2; j += ((j == y1) ? (4 - (y1 & 0x0003)) : 4) )
        {
            int fb_index = (j >> 2) * fb_width;

            s = (int)(rect->s) << 5;

            memset( cvg_span, 0, RDP_CVG_SPAN_MAX );

            for (span_index = j; span_index < ((j &~ 0x0003) + 4) && span_index < y2; span_index++)
            {
                int span_start = x1;
                int span_end = x2;
                int span_iter = 0;
                if (x1 < 0)
                {
                    span_start = 0;
                }
                if (x2 >= (RDP_CVG_SPAN_MAX << 2))
                {
                    span_end = (RDP_CVG_SPAN_MAX << 2) - 1;
                }
                //printf( "%d: Span from %d to %d\n", j, span_start, span_end );
                for (span_iter=span_start; span_iter < span_end; span_iter++)
                {
                    if ((span_iter & 1) == (span_index & 1))
                    {
                        cvg_span[span_iter >> 2]++;
                    }
                }
            }

            for (i = x1; i < x2; i += 4)
            {
                if (i >= 0 && i < (RDP_CVG_SPAN_MAX << 2) && cvg_span[i >> 2] > 0)
                {
                    TEXTURE_PIPELINE1(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                    c1 = COLOR_COMBINER(0);

                    if( alpha_compare_success(c1, c2) )
                    {
                        int dith = dither_matrix_4x4[(((j >> 2) & 3) << 2) + (((i >> 2)^WORD_ADDR_XOR) & 3)];
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], cvg_span[i >> 2], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            BLENDER1_16(&fb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], c1, dith, &cvg_span[i >> 2], &cvg_old, &alpha_reject);

                            if( other_modes.z_update_en && other_modes.z_source_sel && !alpha_reject )
                            {
                                zb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = primitive_z;
                            }
                        }
                    }
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_2)
    {
        for (j = y1; j < y2; j += (j == y1) ? (4 - (y1 & 0x0003)): 4 )
        {
            int fb_index = (j >> 2) * fb_width;

            s = (int)(rect->s) << 5;

            memset( cvg_span, 0, RDP_CVG_SPAN_MAX );

            for (span_index = j; span_index < ((j &~ 0x0003) + 4) && span_index < y2; span_index++)
            {
                int span_start = x1;
                int span_end = x2;
                int span_iter = 0;
                if (x1 < 0)
                {
                    span_start = 0;
                }
                if (x2 >= (RDP_CVG_SPAN_MAX << 2))
                {
                    span_end = (RDP_CVG_SPAN_MAX << 2) - 1;
                }
                for (span_iter=span_start; span_iter < span_end; span_iter++)
                {
                    if ((span_iter & 1) == (span_index & 1))
                    {
                        cvg_span[span_iter >> 2]++;
                    }
                }
            }

            for (i = x1; i < x2; i += 4)
            {
                if (i >= 0 && i < (RDP_CVG_SPAN_MAX << 2) && cvg_span[i >> 2] > 0)
                {
                    TEXTURE_PIPELINE2(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                    c1 = COLOR_COMBINER(0);
                    c2 = COLOR_COMBINER(1);

                    if( alpha_compare_success(c1, c2) )
                    {
                        int dith = dither_matrix_4x4[((((j >> 2)) & 3) << 2) + (((i >> 2)^WORD_ADDR_XOR) & 3)];
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], cvg_span[i >> 2], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            BLENDER2_16(&fb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], c1, c2, dith, &cvg_span[i >> 2], &cvg_old, &alpha_reject);

                            if( other_modes.z_update_en && other_modes.z_source_sel && !alpha_reject )
                            {
                                zb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = primitive_z;
                            }
                        }
                    }
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_COPY)
    {
        for (j = y1; j < y2; j += (j == y1) ? (4 - (y1 & 0x0003)): 4 )
        {
            int fb_index = (j >> 2) * fb_width;

            s = (int)(rect->s) << 5;

            memset( cvg_span, 0, RDP_CVG_SPAN_MAX );

            for (span_index = j; span_index < ((j &~ 0x0003) + 4) && span_index < y2; span_index++)
            {
                int span_start = x1;
                int span_end = x2;
                int span_iter = 0;
                if (x1 < 0)
                {
                    span_start = 0;
                }
                if (x2 >= (RDP_CVG_SPAN_MAX << 2))
                {
                    span_end = (RDP_CVG_SPAN_MAX << 2) - 1;
                }
                for (span_iter=span_start; span_iter < span_end; span_iter++)
                {
                    if ((span_iter & 1) == (span_index & 1))
                    {
                        cvg_span[span_iter >> 2]++;
                    }
                }
            }

            // MooglyGuy note: Fix this, this is probably completely wrong

            for (i = x1; i < x2; i += 4)
            {
                if (i >= 0 && i < (RDP_CVG_SPAN_MAX << 2) && cvg_span[i >> 2] > 0)
                {
                    TEXTURE_PIPELINE1(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                    if( texel0_color.a != 0 )
                    {
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], cvg_span[i >> 2], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            fb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = ((texel0_color.r >> 3) << 11) | ((texel0_color.g >> 3) << 6) | ((texel0_color.b >> 3) << 1) | ((cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] - 1) >> 2);
                            if( other_modes.z_update_en && other_modes.z_source_sel )
                            {
                                zb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = primitive_z;
                            }
                        }
                    }
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else
    {
        fatalerror("texture_rectangle_16bit: unknown cycle type %d\n", other_modes.cycle_type);
    }
}

/*****************************************************************************/

INLINE void BLENDER1_32(UINT32 *fb, COLOR c, UINT8 *new_cvg, UINT8 *old_cvg, int *alpha_reject)
{
    int r, g, b;

    if( other_modes.force_blend )
    {
        blend_color.r = c.r;
        blend_color.g = c.g;
        blend_color.b = c.b;
    }

    pixel_color.r = c.r;
    pixel_color.g = c.g;
    pixel_color.b = c.b;
    pixel_color.a = c.a;
    inv_pixel_color.r = 0xff - *blender1a_r[0];
    inv_pixel_color.g = 0xff - *blender1a_g[0];
    inv_pixel_color.b = 0xff - *blender1a_b[0];
    inv_pixel_color.a = 0xff - *blender1b_a[0];

    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }

    if (other_modes.image_read_en)
    {
        UINT32 mem = *fb;
        memory_color.r = ((mem >> 24) & 0xff);
        memory_color.g = ((mem >> 16) & 0xff);
        memory_color.b = ((mem >>  8) & 0xff);
        memory_color.a = /*(UINT8)((INT32)(*old_cvg) * 0x20 - 1)*/0;
    }

    if (((int)(*blender1b_a[0]) + (int)(*blender2b_a[0])) > 0)
    {
        r = ((int)(*blender1a_r[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_r[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        g = ((int)(*blender1a_g[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_g[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        b = ((int)(*blender1a_b[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_b[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));
    }
    else
    {
        r = g = b = 0;
    }

    //r = g = b = 0xff;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    *fb = (r << 24) | (g << 16) | (b << 8) | 0xe0;
}

INLINE void BLENDER2_32(UINT32 *fb, COLOR c1, COLOR c2, UINT8 *new_cvg, UINT8 *old_cvg, int *alpha_reject)
{
    int r, g, b;

    if( other_modes.force_blend )
    {
        blend_color.r = c2.r;
        blend_color.g = c2.g;
        blend_color.b = c2.b;
    }

    pixel_color.r = c2.r;
    pixel_color.g = c2.g;
    pixel_color.b = c2.b;
    pixel_color.a = c2.a;
    inv_pixel_color.r = 0xff - *blender1a_r[0];
    inv_pixel_color.g = 0xff - *blender1a_g[0];
    inv_pixel_color.b = 0xff - *blender1a_b[0];
    inv_pixel_color.a = 0xff - *blender1b_a[0];

    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c2.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }

    if (other_modes.image_read_en)
    {
        UINT32 mem = *fb;
        memory_color.r = ((mem >> 24) & 0xff);
        memory_color.g = ((mem >> 16) & 0xff);
        memory_color.b = ((mem >>  8) & 0xff);
        memory_color.a = /*(UINT8)((INT32)(*old_cvg) * 0x20 - 1)*/0;
    }

    if (((int)(*blender1b_a[0]) + (int)(*blender2b_a[0])) > 0)
    {
        r = ((int)(*blender1a_r[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_r[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        g = ((int)(*blender1a_g[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_g[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));

        b = ((int)(*blender1a_b[0]) * (int)(*blender1b_a[0]) +
             (int)(*blender2a_b[0]) * (int)(*blender2b_a[0])) /
            ((int)(*blender1b_a[0]) + (int)(*blender2b_a[0]));
    }
    else
    {
        r = g = b = 0;
    }

    blended_pixel_color.r = r;
    blended_pixel_color.g = g;
    blended_pixel_color.b = b;

    pixel_color.r = c2.r;
    pixel_color.g = c2.g;
    pixel_color.b = c2.b;
    pixel_color.a = c2.a;
    if( other_modes.alpha_cvg_select )
    {
        if( other_modes.cvg_times_alpha )
        {
            pixel_color.a = (UINT8)( ( (INT32)(c2.a) * (INT32)( (*new_cvg) * 0x20 - 1 ) ) >> 8 );
        }
        else
        {
            pixel_color.a = (UINT8)( (INT32)(*new_cvg) * 0x20 - 1 );
        }

        if( pixel_color.a == 0 )
        {
            *alpha_reject = 1;
            return;
        }
    }
    inv_pixel_color.r = 0xff - *blender1a_r[0];
    inv_pixel_color.g = 0xff - *blender1a_g[0];
    inv_pixel_color.b = 0xff - *blender1a_b[0];
    inv_pixel_color.a = 0xff - *blender1b_a[0];

    if (((int)(*blender1b_a[1]) + (int)(*blender2b_a[1])) > 0)
    {
        r = ((int)(*blender1a_r[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_r[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));

        g = ((int)(*blender1a_g[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_g[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));

        b = ((int)(*blender1a_b[1]) * (int)(*blender1b_a[1]) +
             (int)(*blender2a_b[1]) * (int)(*blender2b_a[1])) /
            ((int)(*blender1b_a[1]) + (int)(*blender2b_a[1]));
    }
    else
    {
        r = g = b = 0;
    }

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    *fb = (r << 24) | (g << 16) | (b << 8) | 0xe0;
}

static void fill_rectangle_32bit(RECTANGLE *rect)
{
    UINT32 *fb = (UINT32*)&rdram[(fb_address / 4)];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
    int index, i, j;
    int x1 = rect->xh / 4;
    int x2 = rect->xl / 4;
    int y1 = rect->yh / 4;
    int y2 = rect->yl / 4;
    int clipx1, clipx2, clipy1, clipy2;
    int alpha_reject = 0;
    COLOR c1, c2;
    c1.r = c1.g = c1.b = c1.a = 0;
    c2.r = c2.g = c2.b = c2.a = 0;

    // TODO: clip
    clipx1 = clip.xh / 4;
    clipx2 = clip.xl / 4;
    clipy1 = clip.yh / 4;
    clipy2 = clip.yl / 4;

    // clip
    if (x1 < clipx1)
    {
        x1 = clipx1;
    }
    if (y1 < clipy1)
    {
        y1 = clipy1;
    }
    if (x2 >= clipx2)
    {
        x2 = clipx2-1;
    }
    if (y2 >= clipy2)
    {
        y2 = clipy2-1;
    }

    if (other_modes.cycle_type == CYCLE_TYPE_FILL)
    {
        for (j=y1; j <= y2; j++)
        {
            index = j * fb_width;
            for (i=x1; i <= x2; i++)
            {
                cvg_buf[(index + i)] = (fill_color & 0x000000ff) ? 8 : 1; // Wrong
                fb[(index + i) ^ 1] = fill_color;
            }
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_1)
    {
        shade_color.r = prim_color.r;
        shade_color.g = prim_color.g;
        shade_color.b = prim_color.b;
        shade_color.a = prim_color.a;

        for (j=y1; j <= y2; j++)
        {
            index = j * fb_width;
            for (i=x1; i <= x2; i++)
            {
                c1 = COLOR_COMBINER(0);

                cvg_buf[(index + i)] = 8; // Wrong
                BLENDER1_32(&fb[(index + i)], c1, &cvg_buf[(index + i)], &cvg_buf[(index + i)], &alpha_reject);
            }
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_2)
    {
        shade_color.r = prim_color.r;
        shade_color.g = prim_color.g;
        shade_color.b = prim_color.b;
        shade_color.a = prim_color.a;

        for (j=y1; j <= y2; j++)
        {
            index = j * fb_width;
            for (i=x1; i <= x2; i++)
            {
                c1 = COLOR_COMBINER(0);
                c2 = COLOR_COMBINER(1);

                cvg_buf[(index + i)] = 8; // Wrong
                BLENDER2_32(&fb[(index + i)], c1, c2, &cvg_buf[(index + i)], &cvg_buf[(index + i)], &alpha_reject);
            }
        }
    }
    else
    {
        fatalerror("fill_rectangle_32bit: cycle type copy");
    }
}

static void texture_rectangle_32bit(TEX_RECTANGLE *rect)
{
    UINT32 *fb = (UINT32*)&rdram[(fb_address / 4)];
    UINT16 *zb = (UINT16*)&rdram[(zb_address / 4)];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
    int i, j;
    int x1, x2, y1, y2;
    int tsl[2], tsh[2], ttl[2], tth[2];
    int s, t;
    int twidth[2], tformat[2], tsize[2], tbase[2], tpal[2];
    int clamp_s[2], clamp_t[2], mask_s[2], mask_t[2], mirror_s[2], mirror_t[2];
    int clipx1, clipx2, clipy1, clipy2;
    int cycle;
    UINT8 cvg_span[RDP_CVG_SPAN_MAX];
    int span_index = 0;
    int alpha_reject = 0;
    COLOR c1, c2;
    c1.r = c1.g = c1.b = c1.a = 0;
    c2.r = c2.g = c2.b = c2.a = 0;

    x1 = rect->xh;
    x2 = rect->xl;
    y1 = rect->yh;
    y2 = rect->yl;

    if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
    {
        rect->dsdx /= 4;
        x2 += 1;
        y2 += 1;
    }

    clipx1 = clip.xh;
    clipx2 = clip.xl;
    clipy1 = clip.yh;
    clipy2 = clip.yl;

    // clip
    if (x1 < clipx1)
    {
        x1 = clipx1;
    }
    if (y1 < clipy1)
    {
        y1 = clipy1;
    }
    if (x2 >= clipx2)
    {
        x2 = clipx2-1;
    }
    if (y2 >= clipy2)
    {
        y2 = clipy2-1;
    }

    for( cycle = 0; cycle < 2; cycle++ )
    {
        tsl[cycle] = tile[rect->tilenum + cycle].sl;
        tsh[cycle] = tile[rect->tilenum + cycle].sh;
        ttl[cycle] = tile[rect->tilenum + cycle].tl;
        tth[cycle] = tile[rect->tilenum + cycle].th;

        twidth[cycle] = tile[rect->tilenum + cycle].line;
        tformat[cycle] = tile[rect->tilenum + cycle].format;
        tsize[cycle] = tile[rect->tilenum + cycle].size;
        tbase[cycle] = tile[rect->tilenum + cycle].tmem;
        tpal[cycle] = tile[rect->tilenum + cycle].palette;

        clamp_t[cycle] = tile[rect->tilenum + cycle].ct;
        clamp_s[cycle] = tile[rect->tilenum + cycle].cs;

        mirror_t[cycle] = tile[rect->tilenum + cycle].mt;
        mirror_s[cycle] = tile[rect->tilenum + cycle].ms;
        mask_t[cycle] = tile[rect->tilenum + cycle].mask_t;
        mask_s[cycle] = tile[rect->tilenum + cycle].mask_s;
    }

    t = (int)(rect->t) << 5;

    if (other_modes.cycle_type == CYCLE_TYPE_1)
    {
        for (j = y1; j < y2; j += ((j == y1) ? (4 - (y1 & 0x0003)) : 4) )
        {
            int fb_index = (j >> 2) * fb_width;

            s = (int)(rect->s) << 5;

            memset( cvg_span, 0, RDP_CVG_SPAN_MAX );

            for (span_index = j; span_index < ((j &~ 0x0003) + 4) && span_index < y2; span_index++)
            {
                int span_start = x1;
                int span_end = x2;
                int span_iter = 0;
                if (x1 < 0)
                {
                    span_start = 0;
                }
                if (x2 >= (RDP_CVG_SPAN_MAX << 2))
                {
                    span_end = (RDP_CVG_SPAN_MAX << 2) - 1;
                }
                //printf( "%d: Span from %d to %d\n", j, span_start, span_end );
                for (span_iter=span_start; span_iter < span_end; span_iter++)
                {
                    if ((span_iter & 1) == (span_index & 1))
                    {
                        cvg_span[span_iter >> 2]++;
                    }
                }
            }

            for (i = x1; i < x2; i += 4)
            {
                if (i >= 0 && i < (RDP_CVG_SPAN_MAX << 2) && cvg_span[i >> 2] > 0)
                {
                    TEXTURE_PIPELINE1(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                    c1 = COLOR_COMBINER(0);

                    if( alpha_compare_success(c1, c2) )
                    {
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], cvg_span[i >> 2], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            BLENDER1_32(&fb[(fb_index + (i >> 2))], c1, &cvg_span[i >> 2], &cvg_old, &alpha_reject);

                            if( other_modes.z_update_en && other_modes.z_source_sel && !alpha_reject )
                            {
                                zb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = primitive_z;
                            }
                        }
                    }
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_2)
    {
        for (j = y1; j < y2; j += (j == y1) ? (4 - (y1 & 0x0003)): 4 )
        {
            int fb_index = (j >> 2) * fb_width;

            s = (int)(rect->s) << 5;

            memset( cvg_span, 0, RDP_CVG_SPAN_MAX );

            for (span_index = j; span_index < ((j &~ 0x0003) + 4) && span_index < y2; span_index++)
            {
                int span_start = x1;
                int span_end = x2;
                int span_iter = 0;
                if (x1 < 0)
                {
                    span_start = 0;
                }
                if (x2 >= (RDP_CVG_SPAN_MAX << 2))
                {
                    span_end = (RDP_CVG_SPAN_MAX << 2) - 1;
                }
                for (span_iter=span_start; span_iter < span_end; span_iter++)
                {
                    if ((span_iter & 1) == (span_index & 1))
                    {
                        cvg_span[span_iter >> 2]++;
                    }
                }
            }

            for (i = x1; i < x2; i += 4)
            {
                if (i >= 0 && i < (RDP_CVG_SPAN_MAX << 2) && cvg_span[i >> 2] > 0)
                {
                    TEXTURE_PIPELINE2(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                    c1 = COLOR_COMBINER(0);
                    c2 = COLOR_COMBINER(1);

                    if( alpha_compare_success(c1, c2) )
                    {
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR], cvg_span[i >> 2], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            BLENDER2_32(&fb[fb_index + (i >> 2)], c1, c2, &cvg_span[i >> 2], &cvg_old, &alpha_reject);

                            if( other_modes.z_update_en && other_modes.z_source_sel && !alpha_reject )
                            {
                                zb[(fb_index + (i >> 2)) ^ WORD_ADDR_XOR] = primitive_z;
                            }
                        }
                    }
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else if (other_modes.cycle_type == CYCLE_TYPE_COPY)
    {
        for (j = y1; j < y2; j++)
        {
            int fb_index = j * fb_width;

            s = (int)(rect->s) << 5;

            // MooglyGuy note: This is probably completely wrong, fix it

            for (i = x1; i < x2; i++)
            {
            /*
                if (clamp_s)
                {
                    if (s < (tsl << 8)) s = (tsl << 8);
                    if (s > (tsh << 8)) s = (tsh << 8);
                }
                if (clamp_t)
                {
                    if (t < (ttl << 8)) t = (ttl << 8);
                    if (t > (tth << 8)) t = (tth << 8);
                }
                if (mask_s)
                {
                    s &= (((UINT32)(0xffffffff) >> (32-mask_s)) << 10) | 0x3ff;
                }
                if (mask_t)
                {
                    t &= (((UINT32)(0xffffffff) >> (32-mask_t)) << 10) | 0x3ff;


                FETCH_TEXEL(&c, (s-tile[rect->tilenum].sl) >> 10, (t-tile[rect->tilenum].tl) >> 10, twidth, tformat, tsize, tbase);
            */
                //ss = (s >> 10) - (tsl >> 2);
                //if ((s & 0x3ff) >= 0x200) ss++;
                //st = (t >> 10) - (ttl >> 2);
                //if ((t & 0x3ff) >= 0x200) st++;

                TEXTURE_PIPELINE1(s, t, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)));

                //CLAMP(ss, st);
                //FETCH_TEXEL(&c, ss, st, twidth, tformat, tsize, tbase);

                if (texel0_color.a != 0)
                {
                    fb[(fb_index + i) ^ WORD_ADDR_XOR] = (texel0_color.r << 24) | (texel0_color.g << 16) | (texel0_color.b << 8) | 0xe0;
                }

                s += rect->dsdx;
            }

            t += rect->dtdy;
        }
    }
    else
    {
        fatalerror("texture_rectangle_32bit: unknown cycle type %d\n", other_modes.cycle_type);
    }
}


static void render_spans_32(int start, int end, int tilenum, int shade, int texture, int zbuffer, int flip)
{
    UINT32 *fb = (UINT32*)&rdram[fb_address / 4];
    UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
    int i, j;
    int tsl[2], tsh[2], ttl[2], tth[2];
    int twidth[2], tformat[2], tsize[2], tbase[2], tpal[2];
    int clamp_s[2], clamp_t[2], mask_s[2], mask_t[2], mirror_s[2], mirror_t[2];
    int cycle;
    int alpha_reject = 0;

    int clipx1, clipx2, clipy1, clipy2;

    clipx1 = clip.xh / 4;
    clipx2 = clip.xl / 4;
    clipy1 = clip.yh / 4;
    clipy2 = clip.yl / 4;

    for( cycle = 0; cycle < 2; cycle++ )
    {
        tsl[cycle] = tile[tilenum + cycle].sl;
        tsh[cycle] = tile[tilenum + cycle].sh;
        ttl[cycle] = tile[tilenum + cycle].tl;
        tth[cycle] = tile[tilenum + cycle].th;

        twidth[cycle] = tile[tilenum + cycle].line;
        tformat[cycle] = tile[tilenum + cycle].format;
        tsize[cycle] = tile[tilenum + cycle].size;
        tbase[cycle] = tile[tilenum + cycle].tmem;
        tpal[cycle] = tile[tilenum + cycle].palette;

        clamp_t[cycle] = tile[tilenum + cycle].ct;
        clamp_s[cycle] = tile[tilenum + cycle].cs;
        mirror_t[cycle] = tile[tilenum + cycle].mt;
        mirror_s[cycle] = tile[tilenum + cycle].ms;
        mask_t[cycle] = tile[tilenum + cycle].mask_t;
        mask_s[cycle] = tile[tilenum + cycle].mask_s;
    }

    if (start < clipy1)
    {
        start = clipy1;
    }
    if (start >= clipy2)
    {
        start = clipy2-1;
    }
    if (end < clipy1)
    {
        end = clipy1;
    }
    if (end >= clipy2)
    {
        end = clipy2-1;
    }

    for (i=start; i <= end; i++)
    {
        int xstart = span[i].lx;
        int xend = span[i].rx;
        int r = span[i].r;
        int g = span[i].g;
        int b = span[i].b;
        int a = span[i].a;
        int z = span[i].z;
        int s = span[i].s;
        int t = span[i].t;
        int w = span[i].w;
        int dr = span[i].dr;
        int dg = span[i].dg;
        int db = span[i].db;
        int da = span[i].da;
        int dz = span[i].dz;
        int ds = span[i].ds;
        int dt = span[i].dt;
        int dw = span[i].dw;
        int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;

        int x;
        int xinc;

        int fb_index = fb_width * i;
        int length;

        drinc = flip ? (dr) : -dr;
        dginc = flip ? (dg) : -dg;
        dbinc = flip ? (db) : -db;
        dainc = flip ? (da) : -da;
        dzinc = flip ? (dz) : -dz;
        dsinc = flip ? (ds) : -ds;
        dtinc = flip ? (dt) : -dt;
        dwinc = flip ? (dw) : -dw;

        xinc = flip ? 1 : -1;
        x = xend;

        length = flip ? (xstart - xend) : (xend - xstart);

        for (j=0; j <= length; j++)
        {
            int sr = r >> 16;
            int sg = g >> 16;
            int sb = b >> 16;
            int sa = a >> 16;
            int ss = s >> 16;
            int st = t >> 16;
            int sw = w >> 16;
            UINT16 sz = (other_modes.z_source_sel) ? primitive_z : (z >> 16);
            int oz;
            int sss = 0, sst = 0;

            if (x >= clipx1 && x < clipx2)
            {
                COLOR c1, c2;
                c1.r = c1.g = c1.b = c1.a = 0;
                c2.r = c2.g = c2.b = c2.a = 0;

                if ((z & 0xffff) >= 0x8000) sz++;
                if ((w & 0xffff) >= 0x8000) sw++;

                if (other_modes.persp_tex_en)
                {
                    if (sw != 0)
                    {
                        sss = (((INT64)(ss) << 20) / sw);
                        sst = (((INT64)(st) << 20) / sw);
                    }
                }
                else
                {
                    sss = ss;
                    sst = st;
                }

                if (sr > 0xff) sr = 0xff;   if (sg > 0xff) sg = 0xff;   if (sb > 0xff) sb = 0xff;

                shade_color.r = sr; shade_color.g = sg; shade_color.b = sb; shade_color.a = sa;

                if (texture)
                {
                    if (other_modes.cycle_type == CYCLE_TYPE_1)
                    {
                        TEXTURE_PIPELINE1(sss, sst, other_modes.sample_type == 0);
                    }
                    else
                    {
                        TEXTURE_PIPELINE2(sss, sst, other_modes.sample_type == 0);
                    }
                }

                if (other_modes.cycle_type == CYCLE_TYPE_1)
                {
                    c1 = COLOR_COMBINER(0);
                }
                else if (other_modes.cycle_type == CYCLE_TYPE_2)
                {
                    c1 = COLOR_COMBINER(0);
                    c2 = COLOR_COMBINER(1);
                }

                if( alpha_compare_success(c1, c2) )
                {
                    oz = (other_modes.z_source_sel) ? primitive_z : (UINT16)zb[(fb_index + x) ^ WORD_ADDR_XOR];
                    if (other_modes.z_compare_en && zbuffer)
                    {
                        if (sz < oz)
                        {
                            int cvg_ovf = 0;
                            UINT8 cvg_old = 0;
                            calculate_coverage( &cvg_buf[(fb_index + x) ^ WORD_ADDR_XOR], span[i].cvg[x], &cvg_ovf, &cvg_old );
                            if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                            {
                                if (other_modes.cycle_type == CYCLE_TYPE_1)
                                {
                                    BLENDER1_32(&fb[(fb_index + x)], c1, &span[i].cvg[x], &cvg_old, &alpha_reject);
                                }
                                else
                                {
                                    BLENDER2_32(&fb[(fb_index + x)], c1, c2, &span[i].cvg[x], &cvg_old, &alpha_reject);
                                }

                                if (other_modes.z_update_en && !alpha_reject)
                                {
                                    zb[(fb_index + x) ^ WORD_ADDR_XOR] = sz;
                                }
                            }
                        }
                    }
                    else
                    {
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + x) ^ WORD_ADDR_XOR], span[i].cvg[x], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            if (other_modes.cycle_type == CYCLE_TYPE_1)
                            {
                                BLENDER1_32(&fb[(fb_index + x)], c1, &span[i].cvg[x], &cvg_old, &alpha_reject);
                            }
                            else
                            {
                                BLENDER2_32(&fb[(fb_index + x)], c1, c2, &span[i].cvg[x], &cvg_old, &alpha_reject);
                            }
                        }
                    }
                }
            }

            r += drinc;
            g += dginc;
            b += dbinc;
            a += dainc;
            z += dzinc;
            s += dsinc;
            t += dtinc;
            w += dwinc;

            x += xinc;
        }
    }
}

/*****************************************************************************/

static void render_spans_16(int start, int end, int tilenum, int shade, int texture, int zbuffer, int flip)
{
    UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
    UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
    UINT8* cvg_buf = (UINT8*)&cvg_mem[fb_address];
    UINT8* zd_buf = (UINT8*)&cvg_mem[zb_address];
    int i, j;
    int tsl[2], tsh[2], ttl[2], tth[2];
    int twidth[2], tformat[2], tsize[2], tbase[2], tpal[2];
    int clamp_s[2], clamp_t[2], mask_s[2], mask_t[2], mirror_s[2], mirror_t[2];
    int cycle;
    int alpha_reject = 0;

    int clipx1, clipx2, clipy1, clipy2;

    clipx1 = clip.xh / 4;
    clipx2 = clip.xl / 4;
    clipy1 = clip.yh / 4;
    clipy2 = clip.yl / 4;

    for( cycle = 0; cycle < 2; cycle++ )
    {
        tsl[cycle] = tile[tilenum + cycle].sl;
        tsh[cycle] = tile[tilenum + cycle].sh;
        ttl[cycle] = tile[tilenum + cycle].tl;
        tth[cycle] = tile[tilenum + cycle].th;

        twidth[cycle] = tile[tilenum + cycle].line;
        tformat[cycle] = tile[tilenum + cycle].format;
        tsize[cycle] = tile[tilenum + cycle].size;
        tbase[cycle] = tile[tilenum + cycle].tmem;
        tpal[cycle] = tile[tilenum + cycle].palette;

        clamp_t[cycle] = tile[tilenum + cycle].ct;
        clamp_s[cycle] = tile[tilenum + cycle].cs;
        mirror_t[cycle] = tile[tilenum + cycle].mt;
        mirror_s[cycle] = tile[tilenum + cycle].ms;
        mask_t[cycle] = tile[tilenum + cycle].mask_t;
        mask_s[cycle] = tile[tilenum + cycle].mask_s;
    }

    if (start < clipy1)
    {
        start = clipy1;
    }
    if (start >= clipy2)
    {
        start = clipy2-1;
    }
    if (end < clipy1)
    {
        end = clipy1;
    }
    if (end >= clipy2)
    {
        end = clipy2-1;
    }

    for (i=start; i <= end; i++)
    {
        int xstart = span[i].lx;
        int xend = span[i].rx;
        int r = span[i].r;
        int g = span[i].g;
        int b = span[i].b;
        int a = span[i].a;
        int z = span[i].z;
        int s = span[i].s;
        int t = span[i].t;
        int w = span[i].w;
        int dr = span[i].dr;
        int dg = span[i].dg;
        int db = span[i].db;
        int da = span[i].da;
        int dz = span[i].dz;
        int ds = span[i].ds;
        int dt = span[i].dt;
        int dw = span[i].dw;
        int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;

        int x;
        int xinc;

        int fb_index = fb_width * i;
        int length;

        drinc = flip ? (dr) : -dr;
        dginc = flip ? (dg) : -dg;
        dbinc = flip ? (db) : -db;
        dainc = flip ? (da) : -da;
        dzinc = flip ? (dz) : -dz;
        dsinc = flip ? (ds) : -ds;
        dtinc = flip ? (dt) : -dt;
        dwinc = flip ? (dw) : -dw;

        xinc = flip ? 1 : -1;
        x = xend;

        length = flip ? (xstart - xend) : (xend - xstart);

        for (j=0; j <= length; j++)
        {
            int sr = r >> 16;
            int sg = g >> 16;
            int sb = b >> 16;
            int sa = a >> 16;
            int ss = s >> 16;
            int st = t >> 16;
            int sw = w >> 16;
            //UINT16 sz;
            int sz = (other_modes.z_source_sel) ? (int)((primitive_z << 16) | primitive_delta_z) : (int)z;
            UINT32 oz = 0;
            int sss = 0, sst = 0;
            COLOR c1, c2;
            //UINT8 old_delta_z;
            c1.r = c1.g = c1.b = c1.a = 0;
            c2.r = c2.g = c2.b = c2.a = 0;

            /*
            if (other_modes.z_source_sel)
            {
                compress_z((primitive_z << 16) | primitive_delta_z, &sz, &old_delta_z);
            }
            else
            {
                compress_z(z, &sz, &old_delta_z);
            }
            */

            if (x >= clipx1 && x < clipx2)
            {
                //if ((z & 0xffff) >= 0x8000) sz++;
                if ((w & 0xffff) >= 0x8000) sw++;

                if (other_modes.persp_tex_en)
                {
                    if (sw != 0)
                    {
                        sss = (((INT64)(ss) << 20) / sw);
                        sst = (((INT64)(st) << 20) / sw);
                    }
                }
                else
                {
                    sss = ss;
                    sst = st;
                }

                if (sr > 0xff) sr = 0xff;   if (sg > 0xff) sg = 0xff;   if (sb > 0xff) sb = 0xff;

                shade_color.r = sr; shade_color.g = sg; shade_color.b = sb; shade_color.a = sa;

                if (texture)
                {
                    if (other_modes.cycle_type == CYCLE_TYPE_1)
                    {
                        TEXTURE_PIPELINE1(sss, sst, other_modes.sample_type == 0);
                    }
                    else
                    {
                        TEXTURE_PIPELINE2(sss, sst, other_modes.sample_type == 0);
                    }
                }

                if (other_modes.cycle_type == CYCLE_TYPE_1)
                {
                    c1 = COLOR_COMBINER(0);
                }
                else if (other_modes.cycle_type == CYCLE_TYPE_2)
                {
                    c1 = COLOR_COMBINER(0);
                    c2 = COLOR_COMBINER(1);
                }

                if( alpha_compare_success(c1, c2) )
                {
                    decompress_z(zb[(fb_index + x) ^ WORD_ADDR_XOR], &oz);
                    //oz = (UINT16)zb[(fb_index + x) ^ WORD_ADDR_XOR];

                    if (other_modes.z_compare_en && zbuffer)
                    {
                        if (((sz >> 13) < oz) || ((other_modes.z_mode == 3) && ((sz >> 13) == oz)))
                        {
                            int dith = dither_matrix_4x4[(((j) & 3) << 2) + ((i^WORD_ADDR_XOR) & 3)];
                            int cvg_ovf = 0;
                            UINT8 cvg_old = 0;
                            calculate_coverage( &cvg_buf[(fb_index + x) ^ WORD_ADDR_XOR], span[i].cvg[x], &cvg_ovf, &cvg_old );
                            if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                            {
                                if (other_modes.cycle_type == CYCLE_TYPE_1)
                                {
                                    BLENDER1_16(&fb[(fb_index + x) ^ WORD_ADDR_XOR], c1, dith, &span[i].cvg[x], &cvg_old, &alpha_reject);
                                }
                                else
                                {
                                    if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                                    {
                                        BLENDER2_16(&fb[(fb_index + x) ^ WORD_ADDR_XOR], c1, c2, dith, &span[i].cvg[x], &cvg_old, &alpha_reject);
                                    }
                                }

                                if (other_modes.z_update_en && !alpha_reject)
                                {
                                    //zb[(fb_index + x) ^ WORD_ADDR_XOR] = sz;
                                    compress_z(sz, &zb[(fb_index + x) ^ WORD_ADDR_XOR], &zd_buf[(fb_index + x) ^ WORD_ADDR_XOR] );
                                }
                            }
                        }
                    }
                    else
                    {
                        int dith = dither_matrix_4x4[(((j) & 3) << 2) + ((i^WORD_ADDR_XOR) & 3)];
                        int cvg_ovf = 0;
                        UINT8 cvg_old = 0;
                        calculate_coverage( &cvg_buf[(fb_index + x) ^ WORD_ADDR_XOR], span[i].cvg[x], &cvg_ovf, &cvg_old );
                        if ((other_modes.color_on_cvg && cvg_ovf) || !other_modes.color_on_cvg)
                        {
                            if (other_modes.cycle_type == CYCLE_TYPE_1)
                            {
                                BLENDER1_16(&fb[(fb_index + x) ^ WORD_ADDR_XOR], c1, dith, &span[i].cvg[x], &cvg_old, &alpha_reject);
                            }
                            else
                            {
                                BLENDER2_16(&fb[(fb_index + x) ^ WORD_ADDR_XOR], c1, c2, dith, &span[i].cvg[x], &cvg_old, &alpha_reject);
                            }
                        }
                    }
                }
            }

            r += drinc;
            g += dginc;
            b += dbinc;
            a += dainc;
            z += dzinc;
            s += dsinc;
            t += dtinc;
            w += dwinc;

            x += xinc;
        }
    }
}

typedef struct
{
    INT32 xl, xm, xh;
    INT32 yl, ym, yh;
    INT64 dxldy, dxhdy, dxmdy;
    int r, g, b, a, s, t, w, z;
    int drdx, dgdx, dbdx, dadx, dsdx, dtdx, dwdx, dzdx;
    int drdy, dgdy, dbdy, dady, dsdy, dtdy, dwdy, dzdy;
    int drde, dgde, dbde, dade, dsde, dtde, dwde, dzde;
    int tile;
    int flip;

} triangle_data;

static void fill_span_buffer_1x1( triangle_data* tri, int* span_start, int* span_end )
{
    int j;
    int cvg_idx;
    int xleft, xright, xleft_inc, xright_inc;
    int xstart, xend;

    tri->yl >>= 2;
    tri->ym >>= 2;
    tri->yh >>= 2;

    xleft = tri->xm;
    xright = tri->xh;
    xleft_inc = tri->dxmdy;
    xright_inc = tri->dxhdy;

    for(j = tri->yh; j <= tri->yl; j++)
    {
        if(j == tri->ym)
        {
            xleft = tri->xl;
            xleft_inc = tri->dxldy;
            xright_inc = tri->dxhdy;
        }

        xstart = xleft >> 16;
        xend = xright >> 16;

        if(j >= 0)
        {
            span[j].lx = xstart;
            span[j].rx = xend;
            span[j].s = tri->s;     span[j].ds = tri->dsdx;
            span[j].t = tri->t;     span[j].dt = tri->dtdx;
            span[j].w = tri->w;     span[j].dw = tri->dwdx;
            span[j].r = tri->r;     span[j].dr = tri->drdx;
            span[j].g = tri->g;     span[j].dg = tri->dgdx;
            span[j].b = tri->b;     span[j].db = tri->dbdx;
            span[j].a = tri->a;     span[j].da = tri->dadx;
            span[j].z = tri->z;     span[j].dz = tri->dzdx;
            if(tri->flip)
            {
                for(cvg_idx = xend; cvg_idx <= xstart; cvg_idx++)
                {
                    span[j].cvg[cvg_idx] = 8;
                }
            }
            else
            {
                for(cvg_idx = xstart; cvg_idx <= xend; cvg_idx++)
                {
                    span[j].cvg[cvg_idx] = 8;
                }
            }
        }

        xleft += xleft_inc;
        xright += xright_inc;
        tri->s += tri->dsde;
        tri->t += tri->dtde;
        tri->w += tri->dwde;
        tri->r += tri->drde;
        tri->g += tri->dgde;
        tri->b += tri->dbde;
        tri->a += tri->dade;
        tri->z += tri->dzde;
    }

    *span_start = tri->yh;
    *span_end = tri->yl;
}

static void fill_span_buffer_2x2( triangle_data* tri, int* span_start, int* span_end )
{
    int j;
    int xleft, xright, xleft_inc, xright_inc;
    int xstart, xend;
    int covered_pixels = 0;

    tri->xm += (tri->dxmdy >> 2) * (tri->yh & 0x0003);
    tri->xh += (tri->dxhdy >> 2) * (tri->yh & 0x0003);

    tri->r += (tri->drde >> 2) * (tri->yh & 0x0003);
    tri->g += (tri->dgde >> 2) * (tri->yh & 0x0003);
    tri->b += (tri->dbde >> 2) * (tri->yh & 0x0003);
    tri->a += (tri->dade >> 2) * (tri->yh & 0x0003);
    tri->s += (tri->dsde >> 2) * (tri->yh & 0x0003);
    tri->t += (tri->dtde >> 2) * (tri->yh & 0x0003);
    tri->w += (tri->dwde >> 2) * (tri->yh & 0x0003);
    tri->z += (tri->dzde >> 2) * (tri->yh & 0x0003);

    xleft = tri->xm;
    xright = tri->xh;
    xleft_inc = tri->dxmdy;
    xright_inc = tri->dxhdy;

    for( j = tri->yh; j <= tri->yl; j++ )
    {
        if (j == tri->ym)
        {
            xleft = tri->xl;
            xleft_inc = tri->dxldy;
            xright_inc = tri->dxhdy;
        }

        xstart = xleft >> 14;
        xend = xright >> 14;

        if( j >= 0 && ((j == tri->yh) || ((j & 3) == 0)) )
        {
            memset( span[j >> 2].cvg, 0, RDP_CVG_SPAN_MAX );
            covered_pixels = 0;
        }

        if( j >= 0 )
        {
            if( tri->flip )
            {
                int cvg_iter = 0;

                if( xend & 0x0003 )
                {
                    // Round up to the next pixel
                    xend &= ~0x0003;
                    xend += 4;
                }
                if( xleft & ((1 << 14) - 1) )
                {
                    xstart++;
                }
                for( cvg_iter=xend; cvg_iter < xstart; cvg_iter++ )
                {
                    if( (cvg_iter < 0) || (cvg_iter >= (RDP_CVG_SPAN_MAX << 2)) )
                    {
                        continue;
                    }
                    if( (cvg_iter & 1) == (j & 1) )
                    {
                        span[j >> 2].cvg[cvg_iter >> 2]++;
                        covered_pixels++;
                    }
                }
            }
            else
            {
                int cvg_iter = 0;

                if( xstart & 0x0003 )
                {
                    // Round up to the next pixel
                    xstart &= ~0x0003;
                    xstart += 4;
                }
                if( xright & ((1 << 14) - 1) )
                {
                    xend++;
                }
                for( cvg_iter = xstart; cvg_iter < xend; cvg_iter++ )
                {
                    if( cvg_iter < 0 || cvg_iter >= (RDP_CVG_SPAN_MAX << 2) )
                    {
                        continue;
                    }
                    if( (cvg_iter & 1) == (j & 1) )
                    {
                        span[j >> 2].cvg[cvg_iter >> 2]++;
                        covered_pixels++;
                    }
                }
            }

            if( (j & 3) == 3 )
            {
                span[j >> 2].s = tri->s;    span[j >> 2].ds = tri->dsdx;
                span[j >> 2].t = tri->t;    span[j >> 2].dt = tri->dtdx;
                span[j >> 2].w = tri->w;    span[j >> 2].dw = tri->dwdx;
                span[j >> 2].r = tri->r;    span[j >> 2].dr = tri->drdx;
                span[j >> 2].g = tri->g;    span[j >> 2].dg = tri->dgdx;
                span[j >> 2].b = tri->b;    span[j >> 2].db = tri->dbdx;
                span[j >> 2].a = tri->a;    span[j >> 2].da = tri->dadx;
                span[j >> 2].z = tri->z;    span[j >> 2].dz = tri->dzdx;
            }

            if( (j & 3) == 3 )
            {
                int cvg_start = 0;
                int cvg_end = 0;
                int cvg_iter = 0;

                for( cvg_iter = 0; cvg_iter < (RDP_CVG_SPAN_MAX - 1); cvg_iter++ )
                {
                    if( span[j >> 2].cvg[cvg_iter] == 0 && span[j >> 2].cvg[cvg_iter + 1] > 0 )
                    {
                        cvg_start = cvg_iter + 1;
                    }
                    if( span[j >> 2].cvg[cvg_iter] > 0 && span[j >> 2].cvg[cvg_iter + 1] == 0 )
                    {
                        break;
                    }
                }
                cvg_end = cvg_iter;

                span[j >> 2].lx = xstart >> 2;
                span[j >> 2].rx = xend >> 2;

                if( covered_pixels == 0 )
                {
                    span[j >> 2].lx = -1;
                    span[j >> 2].rx = -1;
                }
            }
        }

        xleft += xleft_inc >> 2;
        xright += xright_inc >> 2;
        tri->s += tri->dsde >> 2;
        tri->t += tri->dtde >> 2;
        tri->w += tri->dwde >> 2;
        tri->r += tri->drde >> 2;
        tri->g += tri->dgde >> 2;
        tri->b += tri->dbde >> 2;
        tri->a += tri->dade >> 2;
        tri->z += tri->dzde >> 2;
    }

    *span_start = tri->yh >> 2;
    *span_end = ((tri->yl & 0x0003) == 3) ? (tri->yl >> 2) : ((tri->yl >> 2) - 1);
}

static void triangle(UINT32 w1, UINT32 w2, int shade, int texture, int zbuffer)
{
    UINT32 w3, w4, w5, w6, w7, w8;

    int shade_base = rdp_cmd_cur + 8;
    int texture_base = rdp_cmd_cur + 8;
    int zbuffer_base = rdp_cmd_cur + 8;

    int span_start = 0;
    int span_end = 0;

    triangle_data tri;

    tri.yl = tri.ym = tri.yh = 0;
    tri.xl = tri.xm = tri.xh = 0;
    tri.dxldy = tri.dxhdy = tri.dxmdy = 0;
    tri.r = tri.g = tri.b = tri.a = tri.s = tri.t = tri.w = tri.z = 0;
    tri.drdx = tri.dgdx = tri.dbdx = tri.dadx = tri.dsdx = tri.dtdx = tri.dwdx = tri.dzdx = 0;
    tri.drdy = tri.dgdy = tri.dbdy = tri.dady = tri.dsdy = tri.dtdy = tri.dwdy = tri.dzdy = 0;
    tri.drde = tri.dgde = tri.dbde = tri.dade = tri.dsde = tri.dtde = tri.dwde = tri.dzde = 0;
    tri.tile = (w1 >> 16) & 0x7;;
    tri.flip = (w1 & 0x800000) ? 1 : 0;

    if (shade)
    {
        texture_base += 16;
        zbuffer_base += 16;
    }
    if (texture)
    {
        zbuffer_base += 16;
    }

    w3 = rdp_cmd_data[rdp_cmd_cur+2];
    w4 = rdp_cmd_data[rdp_cmd_cur+3];
    w5 = rdp_cmd_data[rdp_cmd_cur+4];
    w6 = rdp_cmd_data[rdp_cmd_cur+5];
    w7 = rdp_cmd_data[rdp_cmd_cur+6];
    w8 = rdp_cmd_data[rdp_cmd_cur+7];

    tri.yl = (w1 & 0x3fff);
    tri.ym = ((w2 >> 16) & 0x3fff);
    tri.yh = ((w2 >>  0) & 0x3fff);
    tri.xl = (INT32)(w3);
    tri.xh = (INT32)(w5);
    tri.xm = (INT32)(w7);
    tri.dxldy = (INT32)(w4);
    tri.dxhdy = (INT32)(w6);
    tri.dxmdy = (INT32)(w8);

    if (tri.yl & 0x2000) tri.yl |= 0xffffc000;
    if (tri.ym & 0x2000) tri.ym |= 0xffffc000;
    if (tri.yh & 0x2000) tri.yh |= 0xffffc000;

    tri.r = tri.g = tri.b = tri.a = 0xff;
    tri.s = tri.t = tri.w = 0;
    tri.z = 0;

    if (shade)
    {
        tri.r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
        tri.g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
        tri.b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
        tri.a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
        tri.drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
        tri.dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
        tri.dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
        tri.dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
        tri.drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
        tri.dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
        tri.dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
        tri.dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
        tri.drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
        tri.dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
        tri.dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
        tri.dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);
    }
    if (texture)
    {
        tri.s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
        tri.t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
        tri.w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
        tri.dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
        tri.dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
        tri.dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
        tri.dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
        tri.dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[texture_base+12] & 0x0000ffff);
        tri.dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
        tri.dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
        tri.dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[texture_base+14] & 0x0000ffff);
        tri.dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);
    }
    if (zbuffer)
    {
        tri.z    = rdp_cmd_data[zbuffer_base+0];
        tri.dzdx = rdp_cmd_data[zbuffer_base+1];
        tri.dzde = rdp_cmd_data[zbuffer_base+2];
        tri.dzdy = rdp_cmd_data[zbuffer_base+3];
    }

    if( other_modes.sample_type == SAMPLE_TYPE_1x1 )
    {
        fill_span_buffer_1x1( &tri, &span_start, &span_end );
    }
    else
    {
        fill_span_buffer_2x2( &tri, &span_start, &span_end );
    }

    switch (fb_size)
    {
        case PIXEL_SIZE_16BIT:  render_spans_16(span_start, span_end, tri.tile, shade, texture, zbuffer, tri.flip); break;
        case PIXEL_SIZE_32BIT:  render_spans_32(span_start, span_end, tri.tile, shade, texture, zbuffer, tri.flip); break;
    }
}

/*****************************************************************************/

INLINE UINT32 READ_RDP_DATA(UINT32 address)
{
	if (dp_status & 0x1)		// XBUS_DMEM_DMA enabled
	{
		return rsp_dmem[(address & 0xfff) / 4];
	}
	else
	{
		return rdram[(address / 4)];
	}
}

static const char *const image_format[] = { "RGBA", "YUV", "CI", "IA", "I", "???", "???", "???" };
static const char *const image_size[] = { "4-bit", "8-bit", "16-bit", "32-bit" };

static const int rdp_command_length[64] =
{
	8,			// 0x00, No Op
	8,			// 0x01, ???
	8,			// 0x02, ???
	8,			// 0x03, ???
	8,			// 0x04, ???
	8,			// 0x05, ???
	8,			// 0x06, ???
	8,			// 0x07, ???
	32,			// 0x08, Non-Shaded Triangle
	32+16,		// 0x09, Non-Shaded, Z-Buffered Triangle
	32+64,		// 0x0a, Textured Triangle
	32+64+16,	// 0x0b, Textured, Z-Buffered Triangle
	32+64,		// 0x0c, Shaded Triangle
	32+64+16,	// 0x0d, Shaded, Z-Buffered Triangle
	32+64+64,	// 0x0e, Shaded+Textured Triangle
	32+64+64+16,// 0x0f, Shaded+Textured, Z-Buffered Triangle
	8,			// 0x10, ???
	8,			// 0x11, ???
	8,			// 0x12, ???
	8,			// 0x13, ???
	8,			// 0x14, ???
	8,			// 0x15, ???
	8,			// 0x16, ???
	8,			// 0x17, ???
	8,			// 0x18, ???
	8,			// 0x19, ???
	8,			// 0x1a, ???
	8,			// 0x1b, ???
	8,			// 0x1c, ???
	8,			// 0x1d, ???
	8,			// 0x1e, ???
	8,			// 0x1f, ???
	8,			// 0x20, ???
	8,			// 0x21, ???
	8,			// 0x22, ???
	8,			// 0x23, ???
	16,			// 0x24, Texture_Rectangle
	16,			// 0x25, Texture_Rectangle_Flip
	8,			// 0x26, Sync_Load
	8,			// 0x27, Sync_Pipe
	8,			// 0x28, Sync_Tile
	8,			// 0x29, Sync_Full
	8,			// 0x2a, Set_Key_GB
	8,			// 0x2b, Set_Key_R
	8,			// 0x2c, Set_Convert
	8,			// 0x2d, Set_Scissor
	8,			// 0x2e, Set_Prim_Depth
	8,			// 0x2f, Set_Other_Modes
	8,			// 0x30, Load_TLUT
	8,			// 0x31, ???
	8,			// 0x32, Set_Tile_Size
	8,			// 0x33, Load_Block
	8,			// 0x34, Load_Tile
	8,			// 0x35, Set_Tile
	8,			// 0x36, Fill_Rectangle
	8,			// 0x37, Set_Fill_Color
	8,			// 0x38, Set_Fog_Color
	8,			// 0x39, Set_Blend_Color
	8,			// 0x3a, Set_Prim_Color
	8,			// 0x3b, Set_Env_Color
	8,			// 0x3c, Set_Combine
	8,			// 0x3d, Set_Texture_Image
	8,			// 0x3e, Set_Mask_Image
	8			// 0x3f, Set_Color_Image
};

static int rdp_dasm(char *buffer)
{
	int i;
	int tile;
	const char *format, *size;
	char sl[32], tl[32], sh[32], th[32];
	char s[32], t[32], w[32];
	char dsdx[32], dtdx[32], dwdx[32];
	char dsdy[32], dtdy[32], dwdy[32];
	char dsde[32], dtde[32], dwde[32];
	char yl[32], yh[32], ym[32], xl[32], xh[32], xm[32];
	char dxldy[32], dxhdy[32], dxmdy[32];
	char rt[32], gt[32], bt[32], at[32];
	char drdx[32], dgdx[32], dbdx[32], dadx[32];
	char drdy[32], dgdy[32], dbdy[32], dady[32];
	char drde[32], dgde[32], dbde[32], dade[32];
	UINT32 r,g,b,a;

	UINT32 cmd[64];
	UINT32 length;
	UINT32 command;

	length = rdp_cmd_ptr * 4;
	if (length < 8)
	{
		sprintf(buffer, "ERROR: length = %d\n", length);
		return 0;
	}

	cmd[0] = rdp_cmd_data[rdp_cmd_cur+0];
	cmd[1] = rdp_cmd_data[rdp_cmd_cur+1];

	tile = (cmd[1] >> 24) & 0x7;
	sprintf(sl, "%4.2f", (float)((cmd[0] >> 12) & 0xfff) / 4.0f);
	sprintf(tl, "%4.2f", (float)((cmd[0] >>  0) & 0xfff) / 4.0f);
	sprintf(sh, "%4.2f", (float)((cmd[1] >> 12) & 0xfff) / 4.0f);
	sprintf(th, "%4.2f", (float)((cmd[1] >>  0) & 0xfff) / 4.0f);

	format = image_format[(cmd[0] >> 21) & 0x7];
	size = image_size[(cmd[0] >> 19) & 0x3];

	r = (cmd[1] >> 24) & 0xff;
	g = (cmd[1] >> 16) & 0xff;
	b = (cmd[1] >>  8) & 0xff;
	a = (cmd[1] >>  0) & 0xff;

	command = (cmd[0] >> 24) & 0x3f;
	switch (command)
	{
		case 0x00:	sprintf(buffer, "No Op"); break;
		case 0x08:		// Tri_NoShade
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_NoShade length = %d\n", length);
				return 0;
			}

			cmd[2] = rdp_cmd_data[rdp_cmd_cur+2];
			cmd[3] = rdp_cmd_data[rdp_cmd_cur+3];
			cmd[4] = rdp_cmd_data[rdp_cmd_cur+4];
			cmd[5] = rdp_cmd_data[rdp_cmd_cur+5];
			cmd[6] = rdp_cmd_data[rdp_cmd_cur+6];
			cmd[7] = rdp_cmd_data[rdp_cmd_cur+7];

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)(cmd[7] / 65536.0f));

					sprintf(buffer, "Tri_NoShade            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x0a:		// Tri_Tex
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Tex length = %d\n", length);
				return 0;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = rdp_cmd_data[rdp_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


			buffer+=sprintf(buffer, "Tri_Tex               %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x0c:		// Tri_Shade
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Shade length = %d\n", length);
				return 0;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = rdp_cmd_data[i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_Shade              %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);
			break;
		}
		case 0x0e:		// Tri_TexShade
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShade length = %d\n", length);
				return 0;
			}

			for (i=2; i < 40; i++)
			{
				cmd[i] = rdp_cmd_data[rdp_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


			buffer+=sprintf(buffer, "Tri_TexShade           %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);

			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x24:
		case 0x25:
		{
			if (length < 16)
			{
				sprintf(buffer, "ERROR: Texture_Rectangle length = %d\n", length);
				return 0;
			}
			cmd[2] = rdp_cmd_data[rdp_cmd_cur+2];
			cmd[3] = rdp_cmd_data[rdp_cmd_cur+3];
			sprintf(s,    "%4.4f", (float)(INT16)((cmd[2] >> 16) & 0xffff) / 32.0f);
			sprintf(t,    "%4.4f", (float)(INT16)((cmd[2] >>  0) & 0xffff) / 32.0f);
			sprintf(dsdx, "%4.4f", (float)(INT16)((cmd[3] >> 16) & 0xffff) / 1024.0f);
			sprintf(dtdy, "%4.4f", (float)(INT16)((cmd[3] >> 16) & 0xffff) / 1024.0f);

			if (command == 0x24)
					sprintf(buffer, "Texture_Rectangle      %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);
			else
					sprintf(buffer, "Texture_Rectangle_Flip %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);

			break;
		}
		case 0x26:	sprintf(buffer, "Sync_Load"); break;
		case 0x27:	sprintf(buffer, "Sync_Pipe"); break;
		case 0x28:	sprintf(buffer, "Sync_Tile"); break;
		case 0x29:	sprintf(buffer, "Sync_Full"); break;
		case 0x2d:	sprintf(buffer, "Set_Scissor            %s, %s, %s, %s", sl, tl, sh, th); break;
		case 0x2e:	sprintf(buffer, "Set_Prim_Depth         %04X, %04X", (cmd[1] >> 16) & 0xffff, cmd[1] & 0xffff); break;
		case 0x2f:	sprintf(buffer, "Set_Other_Modes        %08X %08X", cmd[0], cmd[1]); break;
		case 0x30:	sprintf(buffer, "Load_TLUT              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x32:	sprintf(buffer, "Set_Tile_Size          %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x33:	sprintf(buffer, "Load_Block             %d, %03X, %03X, %03X, %03X", tile, (cmd[0] >> 12) & 0xfff, cmd[0] & 0xfff, (cmd[1] >> 12) & 0xfff, cmd[1] & 0xfff); break;
		case 0x34:	sprintf(buffer, "Load_Tile              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x35:	sprintf(buffer, "Set_Tile               %d, %s, %s, %d, %04X", tile, format, size, ((cmd[0] >> 9) & 0x1ff) * 8, (cmd[0] & 0x1ff) * 8); break;
		case 0x36:	sprintf(buffer, "Fill_Rectangle         %s, %s, %s, %s", sh, th, sl, tl); break;
		case 0x37:	sprintf(buffer, "Set_Fill_Color         R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x38:	sprintf(buffer, "Set_Fog_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x39:	sprintf(buffer, "Set_Blend_Color        R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3a:	sprintf(buffer, "Set_Prim_Color         %d, %d, R: %d, G: %d, B: %d, A: %d", (cmd[0] >> 8) & 0x1f, cmd[0] & 0xff, r, g, b, a); break;
		case 0x3b:	sprintf(buffer, "Set_Env_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3c:	sprintf(buffer, "Set_Combine            %08X %08X", cmd[0], cmd[1]); break;
		case 0x3d:	sprintf(buffer, "Set_Texture_Image      %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		case 0x3e:	sprintf(buffer, "Set_Mask_Image         %08X", cmd[1]); break;
		case 0x3f:	sprintf(buffer, "Set_Color_Image        %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		default:	sprintf(buffer, "??? (%08X %08X)", cmd[0], cmd[1]); break;
	}

	return rdp_command_length[command];
}

/*****************************************************************************/

////////////////////////
// RDP COMMANDS
////////////////////////

typedef void (*rdp_command_func)(running_machine *machine, UINT32 w1, UINT32 w2);
#define RDP_COMMAND(name) void name(running_machine *machine, UINT32 w1, UINT32 w2)

static RDP_COMMAND( rdp_invalid )
{
	fatalerror("RDP: invalid command  %d, %08X %08X\n", (w1 >> 24) & 0x3f, w1, w2);
}

static RDP_COMMAND( rdp_noop )
{

}

static RDP_COMMAND( rdp_tri_noshade )
{
	//fatalerror("RDP: unhandled command tri_noshade, %08X %08X\n", w1, w2);
	triangle(w1, w2, 0, 0, 0);
}

static RDP_COMMAND( rdp_tri_noshade_z )
{
	//fatalerror("RDP: unhandled command tri_noshade_z, %08X %08X\n", w1, w2);
	triangle(w1, w2, 0, 0, 1);
}

static RDP_COMMAND( rdp_tri_tex )
{
	//osd_die("RDP: unhandled command tri_tex, %08X %08X\n", w1, w2);

	triangle(w1, w2, 0, 1, 0);
}

static RDP_COMMAND( rdp_tri_tex_z )
{
	//osd_die("RDP: unhandled command tri_tex_z, %08X %08X\n", w1, w2);

	triangle(w1, w2, 0, 1, 1);
}

static RDP_COMMAND( rdp_tri_shade )
{
	//osd_die("RDP: unhandled command tri_shade, %08X %08X\n", w1, w2);

	triangle(w1, w2, 1, 0, 0);
}

static RDP_COMMAND( rdp_tri_shade_z )
{
	//osd_die("RDP: unhandled command tri_shade_z, %08X %08X\n", w1, w2);

	triangle(w1, w2, 1, 0, 1);
}

static RDP_COMMAND( rdp_tri_texshade )
{
	//osd_die("RDP: unhandled command tri_texshade, %08X %08X\n", w1, w2);

	triangle(w1, w2, 1, 1, 0);
}

static RDP_COMMAND( rdp_tri_texshade_z )
{
	//osd_die("RDP: unhandled command tri_texshade_z, %08X %08X\n", w1, w2);

	triangle(w1, w2, 1, 1, 1);
}

static RDP_COMMAND( rdp_tex_rect )
{
	UINT32 w3, w4;
	TEX_RECTANGLE rect;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];

	rect.tilenum	= (w2 >> 24) & 0x7;
	rect.xl			= (w1 >> 12) & 0xfff;
	rect.yl			= (w1 >>  0) & 0xfff;
	rect.xh			= (w2 >> 12) & 0xfff;
	rect.yh			= (w2 >>  0) & 0xfff;
	rect.s			= (w3 >> 16) & 0xffff;
	rect.t			= (w3 >>  0) & 0xffff;
	rect.dsdx		= (w4 >> 16) & 0xffff;
	rect.dtdy		= (w4 >>  0) & 0xffff;

	switch (fb_size)
	{
		case PIXEL_SIZE_16BIT:		texture_rectangle_16bit(&rect); break;
		case PIXEL_SIZE_32BIT:		texture_rectangle_32bit(&rect); break;
	}
}

static RDP_COMMAND( rdp_tex_rect_flip )
{
	UINT32 w3, w4;
	TEX_RECTANGLE rect;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];

	rect.tilenum	= (w2 >> 24) & 0x7;
	rect.xl			= (w1 >> 12) & 0xfff;
	rect.yl			= (w1 >>  0) & 0xfff;
	rect.xh			= (w2 >> 12) & 0xfff;
	rect.yh			= (w2 >>  0) & 0xfff;
	rect.t			= (w3 >> 16) & 0xffff;
	rect.s			= (w3 >>  0) & 0xffff;
	rect.dtdy		= (w4 >> 16) & 0xffff;
	rect.dsdx		= (w4 >>  0) & 0xffff;

	switch (fb_size)
	{
		case PIXEL_SIZE_16BIT:		texture_rectangle_16bit(&rect); break;
		case PIXEL_SIZE_32BIT:		texture_rectangle_32bit(&rect); break;
	}
}

static RDP_COMMAND( rdp_sync_load )
{
	// Nothing to do?
}

static RDP_COMMAND( rdp_sync_pipe )
{
	// Nothing to do?
}

static RDP_COMMAND( rdp_sync_tile )
{
	// Nothing to do?
}

static RDP_COMMAND( rdp_sync_full )
{
	dp_full_sync(machine);
}

static RDP_COMMAND( rdp_set_key_gb )
{
	//osd_die("RDP: unhandled command set_key_gb, %08X %08X\n", w1, w2);
}

static RDP_COMMAND( rdp_set_key_r )
{
	//osd_die("RDP: unhandled command set_key_r, %08X %08X\n", w1, w2);
}

static RDP_COMMAND( rdp_set_convert )
{
	//osd_die("RDP: unhandled command set_convert, %08X %08X\n", w1, w2);
}

static RDP_COMMAND( rdp_set_scissor )
{
	clip.xh = (w1 >> 12) & 0xfff;
	clip.yh = (w1 >>  0) & 0xfff;
	clip.xl = (w2 >> 12) & 0xfff;
	clip.yl = (w2 >>  0) & 0xfff;

	// TODO: handle f & o?
}

static RDP_COMMAND( rdp_set_prim_depth )
{
	primitive_z = (UINT16)(w2 >> 16);
	primitive_delta_z = (UINT16)(w1);
}

static RDP_COMMAND( rdp_set_other_modes )
{
	other_modes.cycle_type			= (w1 >> 20) & 0x3;
	other_modes.persp_tex_en 		= (w1 & 0x80000) ? 1 : 0;
	other_modes.detail_tex_en		= (w1 & 0x40000) ? 1 : 0;
	other_modes.sharpen_tex_en		= (w1 & 0x20000) ? 1 : 0;
	other_modes.tex_lod_en			= (w1 & 0x10000) ? 1 : 0;
	other_modes.en_tlut				= (w1 & 0x08000) ? 1 : 0;
	other_modes.tlut_type			= (w1 & 0x04000) ? 1 : 0;
	other_modes.sample_type			= (w1 & 0x02000) ? 1 : 0;
	other_modes.mid_texel			= (w1 & 0x01000) ? 1 : 0;
	other_modes.bi_lerp0			= (w1 & 0x00800) ? 1 : 0;
	other_modes.bi_lerp1			= (w1 & 0x00400) ? 1 : 0;
	other_modes.convert_one			= (w1 & 0x00200) ? 1 : 0;
	other_modes.key_en				= (w1 & 0x00100) ? 1 : 0;
	other_modes.rgb_dither_sel		= (w1 >> 6) & 0x3;
	other_modes.alpha_dither_sel	= (w1 >> 4) & 0x3;
	other_modes.blend_m1a_0			= (w2 >> 30) & 0x3;
	other_modes.blend_m1a_1			= (w2 >> 28) & 0x3;
	other_modes.blend_m1b_0			= (w2 >> 26) & 0x3;
	other_modes.blend_m1b_1			= (w2 >> 24) & 0x3;
	other_modes.blend_m2a_0			= (w2 >> 22) & 0x3;
	other_modes.blend_m2a_1			= (w2 >> 20) & 0x3;
	other_modes.blend_m2b_0			= (w2 >> 18) & 0x3;
	other_modes.blend_m2b_1			= (w2 >> 16) & 0x3;
	other_modes.force_blend			= (w2 & 0x4000) ? 1 : 0;
	other_modes.alpha_cvg_select	= (w2 & 0x2000) ? 1 : 0;
	other_modes.cvg_times_alpha		= (w2 & 0x1000) ? 1 : 0;
	other_modes.z_mode				= (w2 >> 10) & 0x3;
	other_modes.cvg_dest			= (w2 >> 8) & 0x3;
	other_modes.color_on_cvg		= (w2 & 0x80) ? 1 : 0;
	other_modes.image_read_en		= (w2 & 0x40) ? 1 : 0;
	other_modes.z_update_en			= (w2 & 0x20) ? 1 : 0;
	other_modes.z_compare_en		= (w2 & 0x10) ? 1 : 0;
	other_modes.antialias_en		= (w2 & 0x08) ? 1 : 0;
	other_modes.z_source_sel		= (w2 & 0x04) ? 1 : 0;
	other_modes.dither_alpha_en		= (w2 & 0x02) ? 1 : 0;
	other_modes.alpha_compare_en	= (w2 & 0x01) ? 1 : 0;

	SET_BLENDER_INPUT(0, 0, &blender1a_r[0], &blender1a_g[0], &blender1a_b[0], &blender1b_a[0],
					  other_modes.blend_m1a_0, other_modes.blend_m1b_0);
	SET_BLENDER_INPUT(0, 1, &blender2a_r[0], &blender2a_g[0], &blender2a_b[0], &blender2b_a[0],
					  other_modes.blend_m2a_0, other_modes.blend_m2b_0);
	SET_BLENDER_INPUT(1, 0, &blender1a_r[1], &blender1a_g[1], &blender1a_b[1], &blender1b_a[1],
					  other_modes.blend_m1a_1, other_modes.blend_m1b_1);
	SET_BLENDER_INPUT(1, 1, &blender2a_r[1], &blender2a_g[1], &blender2a_b[1], &blender2b_a[1],
					  other_modes.blend_m2a_1, other_modes.blend_m2b_1);

//  mame_printf_debug("blend: m1a = %d, m1b = %d, m2a = %d, m2b = %d\n", other_modes.blend_m1a_0, other_modes.blend_m1b_0, other_modes.blend_m2a_0, other_modes.blend_m2b_0);

	switch (other_modes.rgb_dither_sel)
	{
		case 0: break;
		case 1: break; //fatalerror("bayer matrix dither\n"); break;
		case 2: break; /////fatalerror("noise dither\n"); break;
		case 3: break;
	}
}

static RDP_COMMAND( rdp_load_tlut )
{
	int i;
	UINT16 sl, sh;
//  int tilenum = (w2 >> 24) & 0x7;

	sl	= ((w1 >> 12) & 0xfff) / 4;
	sh	= ((w2 >> 12) & 0xfff) / 4;

	switch (ti_format)
	{
		case 0:		// RGBA
		{
			switch (ti_size)
			{
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *src = (UINT16*)&rdram[ti_address / 4];
	//              UINT16 *tlut = (UINT16*)&texture_cache[tile[tilenum].tmem/2];

					for (i=sl; i <= sh; i++)
					{
						tlut[i] = src[i];
			//          mame_printf_debug("TLUT %d = %04X\n", i, tlut[i]);
					}
					break;
				}
				default:	fatalerror("RDP: load_tlut: size = %d\n", ti_size);
			}
			break;
		}

		default:	fatalerror("RDP: load_tlut: format = %d\n", ti_format);
	}

}

static RDP_COMMAND( rdp_set_tile_size )
{
	int tilenum = (w2 >> 24) & 0x7;

	tile[tilenum].sl = (w1 >> 12) & 0xfff;
	tile[tilenum].tl = (w1 >>  0) & 0xfff;
	tile[tilenum].sh = (w2 >> 12) & 0xfff;
	tile[tilenum].th = (w2 >>  0) & 0xfff;
}

static RDP_COMMAND( rdp_load_block )
{
	int i, width;
	UINT16 sl, sh, tl, dxt;
	int tilenum = (w2 >> 24) & 0x7;
	UINT32 *src, *tc;
	int tb;

	if (ti_format != tile[tilenum].format || ti_size != tile[tilenum].size)
		fatalerror("RDP: load_block: format conversion required!\n");

	sl	= ((w1 >> 12) & 0xfff);
	tl	= ((w1 >>  0) & 0xfff) << 11;
	sh	= ((w2 >> 12) & 0xfff);
	dxt	= ((w2 >>  0) & 0xfff);

	width = (sh - sl) + 1;

	switch (ti_size)
	{
		case PIXEL_SIZE_4BIT:	width >>= 1; break;
		case PIXEL_SIZE_8BIT:	break;
		case PIXEL_SIZE_16BIT:	width <<= 1; break;
		case PIXEL_SIZE_32BIT:	width <<= 2; break;
	}


	src = (UINT32*)&rdram[ti_address / 4];
	tc = (UINT32*)texture_cache;
	tb = tile[tilenum].tmem;

	if (dxt != 0)
	{
		int j=0;
		for (i=0; i < width; i+=2)
		{
			int t = j >> 11;

			tc[((tb+i) + 0)] = src[((((tl * ti_width) / 4) + sl + i + 0) ^ ((t & 1) ? 1 : 0))];
			tc[((tb+i) + 1)] = src[((((tl * ti_width) / 4) + sl + i + 1) ^ ((t & 1) ? 1 : 0))];

			j += dxt;
		}
	}
	else
	{
		for (i=0; i < width / 4; i++)
		{
			tc[(tb+i)] = src[((tl * ti_width) / 4) + sl + i];
		}
	}
}

static RDP_COMMAND( rdp_load_tile )
{
	int i, j;
	UINT16 sl, sh, tl, th;
	int width, height;
	int tilenum = (w2 >> 24) & 0x7;

	if (ti_format != tile[tilenum].format || ti_size != tile[tilenum].size)
		fatalerror("RDP: load_block: format conversion required!\n");

	sl	= ((w1 >> 12) & 0xfff) / 4;
	tl	= ((w1 >>  0) & 0xfff) / 4;
	sh	= ((w2 >> 12) & 0xfff) / 4;
	th	= ((w2 >>  0) & 0xfff) / 4;

	width = (sh - sl) + 1;
	height = (th - tl) + 1;

	switch (ti_size)
	{
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *src = (UINT8*)&rdram[ti_address / 4];
			UINT8 *tc = (UINT8*)texture_cache;
			int tb = tile[tilenum].tmem;

			if (tb + (width * height) > 4096)
			{
				fatalerror("rdp_load_tile 8-bit: tmem %04X, width %d, height %d = %d\n", tile[tilenum].tmem, width, height, width*height);
			}

			for (j=0; j < height; j++)
			{
				int tline = tb + (tile[tilenum].line * j);
				int s = ((j + tl) * ti_width) + sl;

				for (i=0; i < width; i++)
				{
					tc[((tline+i) ^ BYTE_ADDR_XOR) ^ ((j & 1) ? 4 : 0)] = src[(s++) ^ BYTE_ADDR_XOR];
				}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			UINT16 *src = (UINT16*)&rdram[ti_address / 4];
			UINT16 *tc = (UINT16*)texture_cache;
			int tb = (tile[tilenum].tmem / 2);

			if (tb + (width * height) > 2048)
			{
				//fatalerror("rdp_load_tile 16-bit: tmem %04X, width %d, height %d = %d\n", tile[tilenum].tmem, width, height, width*height);
			}

			for (j=0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line / 2) * j);
				int s = ((j + tl) * ti_width) + sl;

				for (i=0; i < width; i++)
				{
					tc[((tline+i) ^ WORD_ADDR_XOR) ^ ((j & 1) ? 2 : 0)] = src[(s++) ^ WORD_ADDR_XOR];
				}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			UINT32 *src = (UINT32*)&rdram[ti_address / 4];
			UINT32 *tc = (UINT32*)texture_cache;
			int tb = (tile[tilenum].tmem / 4);

			if (tb + (width * height) > 1024)
			{
				fatalerror("rdp_load_tile 32-bit: tmem %04X, width %d, height %d = %d\n", tile[tilenum].tmem, width, height, width*height);
			}

			for (j=0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line / 2) * j);
				int s = ((j + tl) * ti_width) + sl;

				for (i=0; i < width; i++)
				{
					tc[(tline+i) ^ ((j & 1) ? 1 : 0)] = src[(s++)];
				}
			}
			break;
		}

		default:	fatalerror("RDP: load_tile: size = %d\n", ti_size);
	}

}

static RDP_COMMAND( rdp_set_tile )
{
	int tilenum = (w2 >> 24) & 0x7;

	tile[tilenum].format	= (w1 >> 21) & 0x7;
	tile[tilenum].size		= (w1 >> 19) & 0x3;
	tile[tilenum].line		= ((w1 >>  9) & 0x1ff) * 8;
	tile[tilenum].tmem		= ((w1 >>  0) & 0x1ff) * 8;
	tile[tilenum].ct		= (w2 >> 19) & 0x1;
	tile[tilenum].mt		= (w2 >> 18) & 0x1;
	tile[tilenum].mask_t	= (w2 >> 14) & 0xf;
	tile[tilenum].shift_t	= (w2 >> 10) & 0xf;
	tile[tilenum].cs		= (w2 >>  9) & 0x1;
	tile[tilenum].ms		= (w2 >>  8) & 0x1;
	tile[tilenum].mask_s	= (w2 >>  4) & 0xf;
	tile[tilenum].shift_s	= (w2 >>  0) & 0xf;

	// TODO: clamp & mirror parameters
}

static RDP_COMMAND( rdp_fill_rect )
{
	RECTANGLE rect;
	rect.xl = (w1 >> 12) & 0xfff;
	rect.yl = (w1 >>  0) & 0xfff;
	rect.xh = (w2 >> 12) & 0xfff;
	rect.yh = (w2 >>  0) & 0xfff;

	switch (fb_size)
	{
		case PIXEL_SIZE_16BIT:		fill_rectangle_16bit(&rect); break;
		case PIXEL_SIZE_32BIT:		fill_rectangle_32bit(&rect); break;
	}
}

static RDP_COMMAND( rdp_set_fill_color )
{
	fill_color = w2;
}

static RDP_COMMAND( rdp_set_fog_color )
{
	fog_color.r = (w2 >> 24) & 0xff;
	fog_color.g = (w2 >> 16) & 0xff;
	fog_color.b = (w2 >>  8) & 0xff;
	fog_color.a = (w2 >>  0) & 0xff;
}

static RDP_COMMAND( rdp_set_blend_color )
{
	blend_color.r = (w2 >> 24) & 0xff;
	blend_color.g = (w2 >> 16) & 0xff;
	blend_color.b = (w2 >>  8) & 0xff;
	blend_color.a = (w2 >>  0) & 0xff;
}

static RDP_COMMAND( rdp_set_prim_color )
{
	// TODO: prim min level, prim_level
	prim_color.r = (w2 >> 24) & 0xff;
	prim_color.g = (w2 >> 16) & 0xff;
	prim_color.b = (w2 >>  8) & 0xff;
	prim_color.a = (w2 >>  0) & 0xff;
}

static RDP_COMMAND( rdp_set_env_color )
{
	env_color.r = (w2 >> 24) & 0xff;
	env_color.g = (w2 >> 16) & 0xff;
	env_color.b = (w2 >>  8) & 0xff;
	env_color.a = (w2 >>  0) & 0xff;
}

static RDP_COMMAND( rdp_set_combine )
{
	combine.sub_a_rgb0	= (w1 >> 20) & 0xf;
	combine.mul_rgb0	= (w1 >> 15) & 0x1f;
	combine.sub_a_a0	= (w1 >> 12) & 0x7;
	combine.mul_a0		= (w1 >>  9) & 0x7;
	combine.sub_a_rgb1	= (w1 >>  5) & 0xf;
	combine.mul_rgb1	= (w1 >>  0) & 0x1f;

	combine.sub_b_rgb0	= (w2 >> 28) & 0xf;
	combine.sub_b_rgb1	= (w2 >> 24) & 0xf;
	combine.sub_a_a1	= (w2 >> 21) & 0x7;
	combine.mul_a1		= (w2 >> 18) & 0x7;
	combine.add_rgb0	= (w2 >> 15) & 0x7;
	combine.sub_b_a0	= (w2 >> 12) & 0x7;
	combine.add_a0		= (w2 >>  9) & 0x7;
	combine.add_rgb1	= (w2 >>  6) & 0x7;
	combine.sub_b_a1	= (w2 >>  3) & 0x7;
	combine.add_a1		= (w2 >>  0) & 0x7;

	SET_SUBA_RGB_INPUT(&combiner_rgbsub_a_r[0], &combiner_rgbsub_a_g[0], &combiner_rgbsub_a_b[0], combine.sub_a_rgb0);
	SET_SUBB_RGB_INPUT(&combiner_rgbsub_b_r[0], &combiner_rgbsub_b_g[0], &combiner_rgbsub_b_b[0], combine.sub_b_rgb0);
	SET_MUL_RGB_INPUT(&combiner_rgbmul_r[0], &combiner_rgbmul_g[0], &combiner_rgbmul_b[0], combine.mul_rgb0);
	SET_ADD_RGB_INPUT(&combiner_rgbadd_r[0], &combiner_rgbadd_g[0], &combiner_rgbadd_b[0], combine.add_rgb0);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_a[0], combine.sub_a_a0);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_b[0], combine.sub_b_a0);
	SET_MUL_ALPHA_INPUT(&combiner_alphamul[0], combine.mul_a0);
	SET_SUB_ALPHA_INPUT(&combiner_alphaadd[0], combine.add_a0);

	SET_SUBA_RGB_INPUT(&combiner_rgbsub_a_r[1], &combiner_rgbsub_a_g[1], &combiner_rgbsub_a_b[1], combine.sub_a_rgb1);
	SET_SUBB_RGB_INPUT(&combiner_rgbsub_b_r[1], &combiner_rgbsub_b_g[1], &combiner_rgbsub_b_b[1], combine.sub_b_rgb1);
	SET_MUL_RGB_INPUT(&combiner_rgbmul_r[1], &combiner_rgbmul_g[1], &combiner_rgbmul_b[1], combine.mul_rgb1);
	SET_ADD_RGB_INPUT(&combiner_rgbadd_r[1], &combiner_rgbadd_g[1], &combiner_rgbadd_b[1], combine.add_rgb1);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_a[1], combine.sub_a_a1);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_b[1], combine.sub_b_a1);
	SET_MUL_ALPHA_INPUT(&combiner_alphamul[1], combine.mul_a1);
	SET_SUB_ALPHA_INPUT(&combiner_alphaadd[1], combine.add_a1);
}

static RDP_COMMAND( rdp_set_texture_image )
{
	ti_format	= (w1 >> 21) & 0x7;
	ti_size		= (w1 >> 19) & 0x3;
	ti_width	= (w1 & 0x3ff) + 1;
	ti_address	= w2 & 0x01ffffff;
}

static RDP_COMMAND( rdp_set_mask_image )
{
	zb_address	= w2 & 0x01ffffff;
}

static RDP_COMMAND( rdp_set_color_image )
{
	fb_format 	= (w1 >> 21) & 0x7;
	fb_size		= (w1 >> 19) & 0x3;
	fb_width	= (w1 & 0x3ff) + 1;
	fb_address	= w2 & 0x01ffffff;

	//if (fb_format != 0)   fatalerror("rdp_set_color_image: format = %d\n", fb_format);
	if (fb_format != 0) fb_format = 0;
}

/*****************************************************************************/

static const rdp_command_func rdp_command_table[64] =
{
	/* 0x00 */
	rdp_noop,			rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_tri_noshade,	rdp_tri_noshade_z,		rdp_tri_tex,			rdp_tri_tex_z,
	rdp_tri_shade,		rdp_tri_shade_z,		rdp_tri_texshade,		rdp_tri_texshade_z,
	/* 0x10 */
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	/* 0x20 */
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_tex_rect,		rdp_tex_rect_flip,		rdp_sync_load,			rdp_sync_pipe,
	rdp_sync_tile,		rdp_sync_full,			rdp_set_key_gb,			rdp_set_key_r,
	rdp_set_convert,	rdp_set_scissor,		rdp_set_prim_depth,		rdp_set_other_modes,
	/* 0x30 */
	rdp_load_tlut,		rdp_invalid,			rdp_set_tile_size,		rdp_load_block,
	rdp_load_tile,		rdp_set_tile,			rdp_fill_rect,			rdp_set_fill_color,
	rdp_set_fog_color,	rdp_set_blend_color,	rdp_set_prim_color,		rdp_set_env_color,
	rdp_set_combine,	rdp_set_texture_image,	rdp_set_mask_image,		rdp_set_color_image
};

void rdp_process_list(running_machine *machine)
{
	int i;
	UINT32 cmd, length, cmd_length;

	length = dp_end - dp_current;

	// load command data
	for (i=0; i < length; i += 4)
	{
		rdp_cmd_data[rdp_cmd_ptr++] = READ_RDP_DATA(dp_current + i);
		if (rdp_cmd_ptr >= 0x1000)
		{
			fatalerror("rdp_process_list: rdp_cmd_ptr overflow\n");
		}
	}

	dp_current = dp_end;

	cmd = (rdp_cmd_data[0] >> 24) & 0x3f;
	cmd_length = (rdp_cmd_ptr + 1) * 4;

	// check if more data is needed
	if (cmd_length < rdp_command_length[cmd])
	{
		return;
	}

	while (rdp_cmd_cur < rdp_cmd_ptr)
	{
		cmd = (rdp_cmd_data[rdp_cmd_cur] >> 24) & 0x3f;
	//  if (((rdp_cmd_data[rdp_cmd_cur] >> 24) & 0xc0) != 0xc0)
	//  {
	//      fatalerror("rdp_process_list: invalid rdp command %08X at %08X\n", rdp_cmd_data[rdp_cmd_cur], dp_start+(rdp_cmd_cur * 4));
	//  }

		if (((rdp_cmd_ptr-rdp_cmd_cur) * 4) < rdp_command_length[cmd])
		{
			return;
			//fatalerror("rdp_process_list: not enough rdp command data: cur = %d, ptr = %d, expected = %d\n", rdp_cmd_cur, rdp_cmd_ptr, rdp_command_length[cmd]);
		}

		if (LOG_RDP_EXECUTION)
		{
			char string[4000];
			rdp_dasm(string);

			fprintf(rdp_exec, "%08X: %08X %08X   %s\n", dp_start+(rdp_cmd_cur * 4), rdp_cmd_data[rdp_cmd_cur+0], rdp_cmd_data[rdp_cmd_cur+1], string);
		}

		// execute the command
		rdp_command_table[cmd](machine, rdp_cmd_data[rdp_cmd_cur+0], rdp_cmd_data[rdp_cmd_cur+1]);

		rdp_cmd_cur += rdp_command_length[cmd] / 4;
	};
	rdp_cmd_ptr = 0;
	rdp_cmd_cur = 0;

	dp_start = dp_end;
}
