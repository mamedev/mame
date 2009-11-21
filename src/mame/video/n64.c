/*
    Nintendo 64 Video Hardware

    Initial version by Ville Linde
    Many improvements by Harmony
    Many improvements by angrylion, Ziggy, Gonetz, Orkin
*/

#include <math.h>
#include "driver.h"
#include "includes/n64.h"

#define LOG_RDP_EXECUTION 		0

#define STRICT_ERROR (0)

#if STRICT_ERROR
#define stricterror fatalerror
#else
#define stricterror(...)
#endif

static FILE *rdp_exec;

static UINT32 rdp_cmd_data[0x1000];
static int rdp_cmd_ptr = 0;
static int rdp_cmd_cur = 0;

static UINT32 curpixel_cvg=0;
static UINT32 curpixel_overlap = 0;

//sign-extension macros
#define SIGN17(x)	(((x) & 0x10000) ? ((x) | ~0x1ffff) : ((x) & 0x1ffff))
#define SIGN16(x)	(((x) & 0x8000) ? ((x) | ~0xffff) : ((x) & 0xffff))
#define SIGN11(x)	(((x) & 0x400) ? ((x) | ~0x7ff) : ((x) & 0x7ff))

#define RDP_CVG_SPAN_MAX    1024

typedef struct
{
	int lx, rx;
	int dymax;
	int s, ds;
	int t, dt;
	int w, dw;
	int r, dr;
	int g, dg;
	int b, db;
	int a, da;
	int z, dz;
    UINT8 cvg[RDP_CVG_SPAN_MAX];
    int dzpix;
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
	int flip;
} TEX_RECTANGLE;

typedef struct
{
	int format;	// Image data format: RGBA, YUV, CI, IA, I
	int size; // Size of texel element: 4b, 8b, 16b, 32b
	UINT32 line; // Size of tile line in bytes
	UINT32 tmem; // Starting tmem address for this tile in bytes
	int palette; // Palette number for 4b CI texels
	int ct, mt, cs, ms; // Clamp / mirror enable bits for S / T direction
	int mask_t, shift_t, mask_s, shift_s; // Mask values / LOD shifts
	UINT16 sl, tl, sh, th;		// 10.2 fixed-point, starting and ending texel row / column
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
static COLOR key_scale; // Used in NASCAR 2000
static COLOR noise_color; // Used in Super Smash Bros.

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
int fb_height;
static UINT32 fb_address;

static UINT32 zb_address;

static int ti_format;
static int ti_size;
static int ti_width;
static UINT32 ti_address;

static TILE tile[8];

static RECTANGLE clip;

static UINT8 *TMEM;
#define tlut ((UINT16*)(TMEM + 0x800))

static INT32 k0, k1, k2, k3, k4, k5;
static UINT8 primitive_lod_frac = 0;
static UINT8 lod_frac = 0;
static UINT8 hidden_bits[0x800000];

static INT32 NormPointRom[64];
static INT32 NormSlopeRom[64];

#define zmode other_modes.z_mode
struct
{
	UINT32 shift;
	UINT32 add;
} z_dec_table[8] =
{
	{ 6, 0x00000 },
	{ 5, 0x20000 },
	{ 4, 0x30000 },
	{ 3, 0x38000 },
	{ 2, 0x3c000 },
	{ 1, 0x3e000 },
	{ 0, 0x3f000 },
	{ 0, 0x3f800 },
};

static INT32 gamma_table[256];
static INT32 gamma_dither_table[0x4000];
static UINT16 z_com_table[0x40000]; // Pre-calculated table of compressed z values
static UINT32 max_level = 0;
static UINT32 min_level = 0;
static COLOR ViBuffer[640][480]; // Used by divot filter
static INT32 maskbits_table[16]; // Pre-calculated
static INT32 clamp_t_diff[8];
static INT32 clamp_s_diff[8];

INLINE int alpha_compare(running_machine *machine, UINT8 comb_alpha);
INLINE UINT8 alpha_cvg_get(UINT8 comb_alpha);
INLINE UINT8 COMBINER_EQUATION(UINT8 A, UINT8 B, UINT8 C, UINT8 D);
INLINE void BLENDER_EQUATION(INT32* r, INT32* g, INT32* b, int cycle, int bsel_special);
INLINE UINT32 addrightcvg(UINT32 x, UINT32 k);
INLINE UINT32 addleftcvg(UINT32 x, UINT32 k);
INLINE UINT32 FBWRITE_16(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b);
INLINE UINT32 FBWRITE_32(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b);
//INLINE UINT32 FBWRITE_8(UINT8* fb, UINT8 c);
INLINE UINT32 z_decompress(UINT16* zb);
INLINE UINT16 dz_decompress(UINT16* zb, UINT8* zhb);
INLINE void z_build_com_table(void);
INLINE void z_store(UINT16* zb, UINT8* zhb, UINT32 z, UINT32 deltaz);
INLINE UINT32 z_compare(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);
INLINE INT32 normalize_dzpix(INT32 sum);
INLINE INT32 CLIP(INT32 value,INT32 min,INT32 max);
INLINE COLOR video_filter16(UINT16* vbuff, UINT8* hbuff, UINT32 hres);
INLINE void divot_filter16(INT32* r, INT32* g, INT32* b, UINT16* fbuff, UINT32 fbuff_index);
INLINE void restore_filter16(INT32* r, INT32* g, INT32* b, UINT16* fbuff, UINT32 fbuff_index, UINT32 hres);
INLINE UINT32 getlog2(UINT32 lod_clamp);
INLINE void set_shade_for_rects(void);
INLINE void set_shade_for_tris(UINT32 shade);
INLINE void copy_colors(COLOR* dst, COLOR* src);
INLINE void BILERP_AND_WRITE(UINT32* src0, UINT32* src1, UINT32* dest);
INLINE void tcdiv(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst);
INLINE void divot_filter16_buffer(INT32* r, INT32* g, INT32* b, COLOR* vibuffer);
INLINE void restore_filter16_buffer(INT32* r, INT32* g, INT32* b, COLOR* vibuff, UINT32 hres);
INLINE void restore_two(COLOR* filtered, COLOR* neighbour);
INLINE void video_max(UINT32* Pixels, UINT8* max, UINT32* enb);
INLINE UINT32 ge_two(UINT32 enb);
INLINE void calculate_clamp_diffs(UINT32 prim_tile);
INLINE void rgb_dither(INT32* r, INT32* g, INT32* b, int dith);

INLINE void MASK(INT32* S, INT32* T, INT32 cycle)
{
	INT32 swrap, twrap;

	if (tile[cycle].mask_s) // Select clamp if mask == 0
	{
		swrap = *S >> (tile[cycle].mask_s > 10 ? 10 : tile[cycle].mask_s);
		swrap &= 1;
		if (tile[cycle].ms && swrap)
		{
			*S = (~(*S)) & maskbits_table[tile[cycle].mask_s]; // Mirroring and masking
		}
		else if (tile[cycle].mask_s)
		{
			*S &= maskbits_table[tile[cycle].mask_s]; // Masking
		}
	}

	if (tile[cycle].mask_t)
	{
		twrap = *T >> (tile[cycle].mask_t > 10 ? 10 : tile[cycle].mask_t);
		twrap &= 1;
		if (tile[cycle].mt && twrap)
		{
			*T = (~(*T)) & maskbits_table[tile[cycle].mask_t]; // Mirroring and masking
		}
		else if (tile[cycle].mask_t)
		{
			*T &= maskbits_table[tile[cycle].mask_t];
		}
	}
}

INLINE void SHIFT(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num)
{
	*S = SIGN16(*S);
	*T = SIGN16(*T);
	if (tile[num].shift_s)
	{
		if (tile[num].shift_s < 11)
		{
			*S >>= tile[num].shift_s;
		}
		else
		{
			*S <<= (16 - tile[num].shift_s);
		}
		*S = SIGN16(*S);
	}
	if (tile[num].shift_s)
	{
		if (tile[num].shift_t < 11)
		{
			*T >>= tile[num].shift_t;
		}
    	else
    	{
			*T <<= (16 - tile[num].shift_t);
		}
		*T = SIGN16(*T);
	}
	*maxs = ((*S >> 3) >= tile[num].sh);
	*maxt = ((*T >> 3) >= tile[num].th);
}

INLINE void CLAMP(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num)
{
	int notcopy = (other_modes.cycle_type != CYCLE_TYPE_COPY);
	int dosfrac = (tile[num].cs || !tile[num].mask_s);
	int dos = dosfrac && notcopy;
	int dotfrac = (tile[num].ct || !tile[num].mask_t);
	int dot = dotfrac && notcopy;
	int overunders = 0;
	int overundert = 0;
	if (*S & 0x10000 && dos)
	{
		*S = 0;
		overunders = 1;
	}
	else if (maxs && dos)
	{
		*S = clamp_s_diff[num];
		overunders = 1;

	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}
	if (overunders && dosfrac)
	{
		*SFRAC = 0;
	}

	if (*T & 0x10000 && dot)
	{
		*T = 0;
		overundert = 1;
	}
	else if (maxt && dot)
	{
		*T = clamp_t_diff[num];
		overundert = 1;
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
	if (overundert && dotfrac)
	{
		*TFRAC = 0;
	}
}

INLINE void CLAMP_LIGHT(INT32* S, INT32* T, INT32 maxs, INT32 maxt, INT32 num)
{
	int notcopy = (other_modes.cycle_type != CYCLE_TYPE_COPY);
	int dos = (tile[num].cs || !tile[num].mask_s) && notcopy;
	int dot = (tile[num].ct || !tile[num].mask_t) && notcopy;

	if (*S & 0x10000 && dos)
	{
		*S = 0;
	}
	else if (maxs && dos)
	{
		*S = clamp_s_diff[num];
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (*T & 0x10000 && dot)
	{
		*T = 0;
	}
	else if (maxt && dot)
	{
		*T = clamp_t_diff[num];
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

/*****************************************************************************/



VIDEO_START(n64)
{
	int i = 0;
	UINT8 *normpoint = memory_region(machine, "normpoint");
	UINT8 *normslope = memory_region(machine, "normslope");

	if (LOG_RDP_EXECUTION)
		rdp_exec = fopen("rdp_execute.txt", "wt");

	TMEM = auto_alloc_array(machine, UINT8, 0x1004); // 4 guard bytes
	memset(TMEM, 0, 0x1000);
	TMEM[0x1000] = TMEM[0x1001] = 25;

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

	memset(hidden_bits, 3, 4194304); // Hack / fix for letters in Rayman 2

	for (i = 0; i < 256; i++)
	{
		gamma_table[i] = sqrt((float)(i << 6));
		gamma_table[i] <<= 1;
	}

	for (i = 0; i < 0x4000; i++)
	{
		gamma_dither_table[i] = sqrt(i);
		gamma_dither_table[i] <<= 1;
	}

	z_build_com_table();

	maskbits_table[0] = 0x3ff;
	for(i = 1; i < 16; i++)
	{
		maskbits_table[i] = ((UINT16)(0xffff) >> (16 - i)) & 0x3ff;
	}

	for(i = 0; i < 64; i++)
	{
		NormPointRom[i] = (normpoint[(i << 1) + 1] << 8) | normpoint[i << 1];
		NormSlopeRom[i] = (normslope[(i << 1) + 1] << 8) | normslope[i << 1];
	}
}

VIDEO_UPDATE(n64)
{
	int i, j;
	UINT32 final = 0;
	UINT32 prev_cvg = 0, next_cvg = 0;
    int height = fb_height;
    int dither_filter = (n64_vi_control >> 16) & 1;
    int fsaa = (((n64_vi_control >> 8) & 3) < 2);
    int divot = (n64_vi_control >> 4) & 1;
    int gamma = (n64_vi_control >> 3) & 1;
    int gamma_dither = (n64_vi_control >> 2) & 1;
    int vibuffering = ((n64_vi_control & 2) && fsaa && divot);

    UINT32 *frame_buffer32;
	UINT16 *frame_buffer;
	UINT32 hb;
	UINT8* hidden_buffer;
	COLOR newc;

	UINT32 pixels = 0;
	UINT16 pix = 0;
	int r, g, b;
	int dith = 0;

	INT32 hdiff = (n64_vi_hstart & 0x3ff) - ((n64_vi_hstart >> 16) & 0x3ff);
	float hcoeff = ((float)(n64_vi_xscale & 0xfff) / (1 << 10));
	UINT32 hres = ((float)hdiff * hcoeff);
	INT32 invisiblewidth = n64_vi_width - hres;

	INT32 vdiff = ((n64_vi_vstart & 0x3ff) - ((n64_vi_vstart >> 16) & 0x3ff)) >> 1;
	float vcoeff = ((float)(n64_vi_yscale & 0xfff) / (1 << 10));
	UINT32 vres = ((float)vdiff * vcoeff);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return 0;
	}

	frame_buffer = (UINT16*)&rdram[(n64_vi_origin & 0xffffff) >> 2];
	hb = ((n64_vi_origin & 0xffffff) >> 2) >> 1;
	hidden_buffer = &hidden_bits[hb];

	vibuffering = 0; // Disabled for now


	if (hres > 640) // Needed by Top Gear Overdrive (E)
	{
		invisiblewidth += (hres - 640);
		hres = 640;
	}

	if (vibuffering && ((n64_vi_control & 3) == 2))
	{
		if (frame_buffer)
		{
			for (j=0; j < vres; j++)
			{
				for (i=0; i < hres; i++)
				{
					UINT16 pix;
					pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
					curpixel_cvg = ((pix & 1) << 2) | (hidden_buffer[pixels ^ BYTE_ADDR_XOR] & 3); // Reuse of this variable
					if (curpixel_cvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2) && fsaa)
					{
						newc = video_filter16(&frame_buffer[pixels ^ WORD_ADDR_XOR], &hidden_buffer[pixels ^ BYTE_ADDR_XOR], n64_vi_width);
						ViBuffer[i][j] = newc;
					}
					else
					{
						newc.r = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
						newc.g = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
						newc.b = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
						ViBuffer[i][j] = newc;
					}
					pixels++;
				}
				pixels += invisiblewidth;
			}
		}
	}

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
			pixels = 0;

			if (frame_buffer)
			{
				for (j=0; j < vres; j++)
				{
					UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);

					for (i=0; i < hres; i++)
					{
						int r, g, b;

						pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
						curpixel_cvg = ((pix & 1) << 2) | (hidden_buffer[pixels ^ BYTE_ADDR_XOR] & 3);

						if (i > 0 && i < (hres - 1) && divot)
						{
							prev_cvg = ((frame_buffer[(pixels - 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels - 1)^BYTE_ADDR_XOR] & 3);
							next_cvg = ((frame_buffer[(pixels + 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels + 1)^BYTE_ADDR_XOR] & 3);
						}
						r = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
						g = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
						b = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);

						if (!vibuffering && curpixel_cvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2) && fsaa)
						{
							newc = video_filter16(&frame_buffer[pixels ^ WORD_ADDR_XOR],&hidden_buffer[pixels ^ BYTE_ADDR_XOR], n64_vi_width);
							r = newc.r; g = newc.g; b = newc.b;
						}
						else if (dither_filter && curpixel_cvg == 7 && i > 0 && j > 0 && i < (hres - 1) && j < (vres - 1))
						{
							if (vibuffering)
							{
								restore_filter16_buffer(&r, &g, &b, &ViBuffer[i][j], n64_vi_width);
							}
							else
							{
								restore_filter16(&r, &g, &b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR, n64_vi_width);
							}
						}
						if (i > 0 && i < (hres - 1) && divot && (curpixel_cvg != 7 || prev_cvg != 7 || next_cvg != 7))
						{
							if (vibuffering)
							{
								divot_filter16_buffer(&r, &g, &b, &ViBuffer[i][j]);
							}
							else
							{
								divot_filter16(&r, &g, &b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR);
							}
						}

						if (gamma_dither)
						{
							dith = mame_rand(screen->machine) & 0x3f;
						}
						if (gamma)
						{
							if (gamma_dither)
							{
								r = gamma_dither_table[(r << 6)|dith];
								g = gamma_dither_table[(g << 6)|dith];
								b = gamma_dither_table[(b << 6)|dith];
							}
							else
							{
								r = gamma_table[r];
								g = gamma_table[g];
								b = gamma_table[b];
							}
						}
						else if (gamma_dither)
						{
							if (r < 255)
								r += (dith & 1);
							if (g < 255)
								g += (dith & 1);
							if (b < 255)
								b += (dith & 1);
						}
						pixels++;

						final = (r << 16) | (g << 8) | b;
						d[i] = final; // Fix me for endianness
					}
					pixels +=invisiblewidth;
				}
			}
			break;
		}

		case 3:		// RGBA8888
		{
            frame_buffer32 = (UINT32*)&rdram[(n64_vi_origin & 0xffffff) >> 2];
			if (frame_buffer32)
			{
				for (j=0; j < vres; j++)
				{
					UINT32 *d = BITMAP_ADDR32(bitmap, j, 0);
					for (i=0; i < hres; i++)
					{
						UINT32 pix = *frame_buffer32++;
						if (gamma || gamma_dither)
						{
							r = (pix >> 24) & 0xff;
							g = (pix >> 16) & 0xff;
							b = (pix >> 8) & 0xff;
							if (gamma_dither)
							{
								dith = mame_rand(screen->machine) & 0x3f;
							}
							if (gamma)
							{
								if (gamma_dither)
								{
									r = gamma_dither_table[(r << 6)| dith];
									g = gamma_dither_table[(g << 6)| dith];
									b = gamma_dither_table[(b << 6)| dith];
								}
								else
								{
									r = gamma_table[r];
									g = gamma_table[g];
									b = gamma_table[b];
								}
							}
							else if (gamma_dither)
							{
								if (r < 255)
									r += (dith & 1);
								if (g < 255)
									g += (dith & 1);
								if (b < 255)
									b += (dith & 1);
							}
							pix = (r << 24) | (g << 16) | (b << 8);
						}


						d[i] = (pix >> 8);
					}
					frame_buffer32 += invisiblewidth;
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
		case 7:		*input_r = &noise_color.r;		*input_g = &noise_color.g;		*input_b = &noise_color.b;		break;
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
		case 7:		*input_r = (UINT8*)&k4;			*input_g = (UINT8*)&k4;			*input_b = (UINT8*)&k4;			break;
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
		case 6:		*input_r = &key_scale.r;		*input_g = &key_scale.g;		*input_b = &key_scale.b;		break;
		case 7:		*input_r = &combined_color.a;	*input_g = &combined_color.a;	*input_b = &combined_color.a;	break;
		case 8:		*input_r = &texel0_color.a;		*input_g = &texel0_color.a;		*input_b = &texel0_color.a;		break;
		case 9:		*input_r = &texel1_color.a;		*input_g = &texel1_color.a;		*input_b = &texel1_color.a;		break;
		case 10:	*input_r = &prim_color.a;		*input_g = &prim_color.a;		*input_b = &prim_color.a;		break;
		case 11:	*input_r = &shade_color.a;		*input_g = &shade_color.a;		*input_b = &shade_color.a;		break;
		case 12:	*input_r = &env_color.a;		*input_g = &env_color.a;		*input_b = &env_color.a;		break;
		case 13:	*input_r = &lod_frac;			*input_g = &lod_frac;			*input_b = &lod_frac;			break;
		case 14:	*input_r = &primitive_lod_frac;	*input_g = &primitive_lod_frac;	*input_b = &primitive_lod_frac; break;
		case 15:	*input_r = (UINT8*)&k5;			*input_g = (UINT8*)&k5;			*input_b = (UINT8*)&k5;			break;
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
		case 0:		*input = &lod_frac; break;//HACK
		case 1:		*input = &texel0_color.a; break;
		case 2:		*input = &texel1_color.a; break;
		case 3:		*input = &prim_color.a; break;
		case 4:		*input = &shade_color.a; break;
		case 5:		*input = &env_color.a; break;
		case 6:		*input = &primitive_lod_frac; break;//HACK
		case 7:		*input = &zero_color.a; break;
	}
}



INLINE COLOR COLOR_COMBINER(running_machine *machine, int cycle)
{
	COLOR c;
	COLOR temp;
	UINT32 r, g, b, a;

	if (other_modes.cycle_type == CYCLE_TYPE_1) //http://moogle-tech.com/blog/?p=84
	{
		cycle=1; // Needed by F-1 Pole Position 64 menus
	}

	if (other_modes.cycle_type == CYCLE_TYPE_2 && cycle == 1)
	{
		copy_colors(&temp, &texel0_color);
		copy_colors(&texel0_color, &texel1_color);
		copy_colors(&texel1_color, &temp);
	}

	if (combiner_rgbsub_a_r[cycle] == &noise_color.r)
	{
		noise_color.r = mame_rand(machine) & 0xff;
		noise_color.g = mame_rand(machine) & 0xff;
		noise_color.b = mame_rand(machine) & 0xff;
	}

	r = COMBINER_EQUATION(*combiner_rgbsub_a_r[cycle],*combiner_rgbsub_b_r[cycle],*combiner_rgbmul_r[cycle],*combiner_rgbadd_r[cycle]);
	g = COMBINER_EQUATION(*combiner_rgbsub_a_g[cycle],*combiner_rgbsub_b_g[cycle],*combiner_rgbmul_g[cycle],*combiner_rgbadd_g[cycle]);
	b = COMBINER_EQUATION(*combiner_rgbsub_a_b[cycle],*combiner_rgbsub_b_b[cycle],*combiner_rgbmul_b[cycle],*combiner_rgbadd_b[cycle]);
	a = COMBINER_EQUATION(*combiner_alphasub_a[cycle],*combiner_alphasub_b[cycle],*combiner_alphamul[cycle],*combiner_alphaadd[cycle]);

	if (other_modes.cycle_type == CYCLE_TYPE_2 && cycle == 0)
	{
		combined_color.r=r;
		combined_color.g=g;
		combined_color.b=b;
		combined_color.a=a;
	}

	//Alpha coverage combiner
	if (other_modes.cycle_type == CYCLE_TYPE_1 || (other_modes.cycle_type == CYCLE_TYPE_2 && cycle == 1))
	{
		a = alpha_cvg_get(a);
	}

	c.r = r; c.g = g; c.b = b; c.a = a;
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
			case 1:		*input_a = &fog_color.a; break;
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

static const UINT8 bayer_matrix[16] =
{ /* Bayer matrix */
	 0,  4,  1, 5,
	 6,  2,  7, 3,
	 1,	 5,  0, 4,
	 7,  3,  6, 2
};

static const UINT8 magic_matrix[16] =
{ /* Magic square matrix */
	 0,  6,  1, 7,
	 4,  2,  5, 3,
	 3,	 5,  2, 4,
	 7,  1,  6, 0
};

INLINE int BLENDER1_16(running_machine *machine, UINT16 *fb, UINT8* hb, COLOR c, int dith)
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);

	// Alpha compare
	if (!alpha_compare(machine, c.a))
	{
		return 0;
	}
	if (!curpixel_cvg) // New coverage is zero, so abort
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.a)
	{
		special_bsel = 1;
	}

	copy_colors(&pixel_color,&c);

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	memory_color.r = (((mem >> 11) & 0x1f) << 3) | (((mem >> 11) & 0x1f) >> 2);
	memory_color.g = (((mem >>  6) & 0x1f) << 3) | (((mem >>  6) & 0x1f) >> 2);
	memory_color.b = (((mem >>  1) & 0x1f) << 3) | (((mem >>  1) & 0x1f) >> 2);

	if (other_modes.image_read_en)
	{
		memory_color.a = (memory_cvg << 5) & 0xe0;
	}
	else
	{
		memory_color.a = 0xe0;
	}

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		inv_pixel_color.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION(&r, &g, &b, 0, special_bsel);
	}

	if (other_modes.rgb_dither_sel < 2)
	{
		// Hack to prevent "double-dithering" artifacts
		int dithhack = ((r & 0xf8)==(memory_color.r&0xf8) && (g & 0xf8) == (memory_color.g & 0xf8) &&(b&0xf8)==(memory_color.b&0xf8));
		if (!dithhack)
		{
			rgb_dither(&r, &g, &b, dith);
		}
	}

    return (FBWRITE_16(fb, hb, r, g, b));
}

INLINE int BLENDER2_16(running_machine *machine, UINT16 *fb, UINT8* hb, COLOR c1, COLOR c2, int dith)
{
	int r, g, b;
	int special_bsel = 0;
	UINT16 mem = *fb;
	UINT32 memory_cvg = ((mem & 1) << 2) + (*hb & 3);

	// Alpha compare
	if (!alpha_compare(machine, c2.a))
	{
		return 0;
	}
	if (!curpixel_cvg)
	{
		return 0;
	}

	if (blender2b_a[0] == &memory_color.a)
	{
		special_bsel = 1;
	}

	copy_colors(&pixel_color, &c2);
	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	memory_color.r = (((mem >> 11) & 0x1f) << 3) | (((mem >> 11) & 0x1f) >> 2);
	memory_color.g = (((mem >>  6) & 0x1f) << 3) | (((mem >>  6) & 0x1f) >> 2);
	memory_color.b = (((mem >>  1) & 0x1f) << 3) | (((mem >>  1) & 0x1f) >> 2);

	if (other_modes.image_read_en)
	{
		memory_color.a = (memory_cvg << 5) & 0xe0;
	}
	else
	{
		memory_color.a = 0xe0;
	}

	inv_pixel_color.a = 0xff - *blender1b_a[0];

	BLENDER_EQUATION(&r, &g, &b, 0, special_bsel);

	blended_pixel_color.r = r;
	blended_pixel_color.g = g;
	blended_pixel_color.b = b;
	blended_pixel_color.a = pixel_color.a;

	pixel_color.r = r;
	pixel_color.g = g;
	pixel_color.b = b;

	inv_pixel_color.a = 0xff - *blender1b_a[1];

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[1];
		g = *blender1a_g[1];
		b = *blender1a_b[1];
	}
	else
	{
		if (blender2b_a[1] == &memory_color.a)
		{
			special_bsel = 1;
		}
		else
		{
			special_bsel = 0;
		}

		BLENDER_EQUATION(&r, &g, &b, 1, special_bsel);
	}

	if (other_modes.rgb_dither_sel < 2)
	{
		// Hack to prevent "double-dithering" artifacts
		int dithhack = ((r & 0xf8)==(memory_color.r&0xf8) && (g & 0xf8) == (memory_color.g & 0xf8) &&(b&0xf8)==(memory_color.b&0xf8));
		if (!dithhack)
		{
			rgb_dither(&r, &g, &b, dith);
		}
	}

	return (FBWRITE_16(fb, hb, r, g, b));
}

/*****************************************************************************/


static void fill_rectangle_16bit(running_machine *machine, RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT8* hb = &hidden_bits[fb_address >> 1];

	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
	UINT16 fill_color1, fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (fill_color >> 16) & 0xffff;
	fill_color2 = (fill_color >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

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

	set_shade_for_rects(); // Needed by Command & Conquer menus

	if (other_modes.cycle_type == CYCLE_TYPE_FILL)
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;
			for (i=x1; i <= x2; i++)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = (i & 1) ? fill_color1 : fill_color2;
				hb[curpixel ^ BYTE_ADDR_XOR] = (i & 1) ? ((fill_color1 & 1)? 3:0) : ((fill_color2 & 1) ? 3:0);
			}
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_1)
	{
		for (j = y1; j <= y2; j++)
		{
			COLOR c;
			int dith = 0;
			index = j * fb_width;
			for (i = x1; i <= x2; i++)
			{
				curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
				c = COLOR_COMBINER(machine, 0);
				if (!other_modes.rgb_dither_sel)
				{
					dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				else if (other_modes.rgb_dither_sel == 1)
				{
					dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				BLENDER1_16(machine, &fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c, dith);
			}
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		for (j=y1; j <= y2; j++)
		{
			COLOR c1, c2;
			int dith = 0;
			index = j * fb_width;
			for (i=x1; i <= x2; i++)
			{
				curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;
				c1 = COLOR_COMBINER(machine, 0);
				c2 = COLOR_COMBINER(machine, 1);
				if (!other_modes.rgb_dither_sel)
				{
					dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				else if (other_modes.rgb_dither_sel == 1)
				{
					dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				BLENDER2_16(machine, &fb[(index + i) ^ WORD_ADDR_XOR],  &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, dith);
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

INLINE void FETCH_TEXEL(COLOR *color, int s, int t, UINT32 tilenum)
{
	UINT32 twidth	= tile[tilenum].line;
	UINT32	tformat = tile[tilenum].format;
	UINT32 tsize =	tile[tilenum].size;
	UINT32 tbase =	tile[tilenum].tmem;
	UINT32 tpal	= tile[tilenum].palette & 0xf;

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
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
					UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					UINT16 c = tlut[(((tpal << 4) | p) ^ WORD_ADDR_XOR) << 2];

					if (other_modes.en_tlut)
					{
						if (other_modes.tlut_type == 0)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					else
					{
						color->r = color->g = color->b = color->a = (tpal << 4) | p;
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
					UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
					UINT16 c = tlut[(p ^ WORD_ADDR_XOR) << 2];

					if (other_modes.en_tlut)
					{
						if (other_modes.tlut_type == 0)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					else
					{
						color->r = color->g = color->b = color->a = p;
					}
					break;
				}
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = (UINT16*)TMEM;
                    int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
					UINT16 c = tc[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

					if (!other_modes.en_tlut)
					{
						color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
						color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
						color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
						color->a = (c & 1) ? 0xff : 0;
					}
					else
					{
						c = tlut[(c >> 8) << 2];
						if (other_modes.tlut_type == 0) //Golden Eye 007, sea, "frigate" level
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else // Beetle Adventure Racing, Mount Mayhem
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_32BIT:
				{
					UINT32 *tc = (UINT32*)TMEM;
					int xorval = (fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
                    int taddr = (((tbase >> 2) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;
					UINT32 c = tc[taddr];

					if (!other_modes.en_tlut)
					{
						color->r = ((c >> 24) & 0xff);
						color->g = ((c >> 16) & 0xff);
						color->b = ((c >>  8) & 0xff);
						color->a = ((c >>  0) & 0xff);
					}
					else
					{
						c = tlut[(c >> 24) << 2];
						if (!other_modes.tlut_type)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					break;
				}
				default:
					color->r = color->g = color->b = color->a = 0xff;
					fatalerror("FETCH_TEXEL: unknown RGBA texture size %d\n", tsize);
					break;
			}
			break;
		}
		case 1:		// YUV: Bottom of the 9th, Pokemon Stadium, Ogre Battle 64
		{
			switch (tsize)
			{
				case PIXEL_SIZE_16BIT:
				{
					INT32 newr = 0;
					INT32 newg = 0;
					INT32 newb = 0;
					UINT16 *tc = (UINT16*)TMEM;
					int taddr = ((tbase >> 1) + ((t) * (twidth)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
					UINT16 c1, c2;
					INT32 y;
					INT32 u, v;
					c1 = tc[taddr ^ WORD_ADDR_XOR];
					c2 = tc[taddr]; // other word

					if (!(taddr & 1))
					{
						v = c2 >> 8;
						u = c1 >> 8;
						y = c1 & 0xff;
					}
					else
					{
						v = c1 >> 8;
						u = c2 >> 8;
						y = c1 & 0xff;
					}
					v -= 128;
					u -= 128;

					if (!other_modes.bi_lerp0)
					{
						newr = y + ((k0 * v) >> 8);
						newg = y + ((k1 * u) >> 8) + ((k2 * v) >> 8);
						newb = y + ((k3 * u) >> 8);
					}
					color->r = (newr < 0) ? 0 : ((newr > 0xff) ? 0xff : newr);
					color->g = (newg < 0) ? 0 : ((newg > 0xff) ? 0xff : newg);
					color->b = (newb < 0) ? 0 : ((newb > 0xff) ? 0xff : newb);
					color->a = 0xff;
					break;
				}
			}
			break;
		}
		case 2:		// Color Index
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
					UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					UINT16 c = tlut[((tpal << 4) | p) << 2];

					if (other_modes.en_tlut)
					{
						if (other_modes.tlut_type == 0)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					else
					{
						color->r = color->g = color->b = color->a = (tpal << 4) | p;
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
					UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
					UINT16 c = tlut[p << 2];

					if (other_modes.en_tlut)
					{
						if (other_modes.tlut_type == 0)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					else
					{
						color->r = color->g = color->b = color->a = p;
					}
					break;
				}
				case PIXEL_SIZE_16BIT:
				{
                    // 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
					UINT16 *tc = (UINT16*)TMEM;
                    int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
					UINT16 c = tc[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

					if (!other_modes.en_tlut)
					{
						color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
						color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
						color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
						color->a = (c & 1) ? 0xff : 0;
					}
					else
					{
						c = tlut[(c >> 8) << 2];
						if (!other_modes.tlut_type) // GoldenEye 007, sea, "frigate" level
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
							color->a = (c & 1) ? 0xff : 0;
						}
						else // Beetle Adventure Racing, Mount Mayhem
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
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
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = (tbase + ((t) * twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
					UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					UINT8 i = ((p & 0xe) << 4) | ((p & 0xe) << 1) | (p & 0xe >> 2);

					if (!other_modes.en_tlut)
					{
						color->r = i;
						color->g = i;
						color->b = i;
						color->a = (p & 0x1) ? 0xff : 0;
					}
					else
					{
						UINT16 c = tlut[((tpal << 4) | p) << 2];
						if (!other_modes.tlut_type)
						{
                    		color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
                    		color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
                    		color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
                    		color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
					UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
					UINT8 i = 0;

					if (!other_modes.en_tlut)
					{
						i = (p >> 4) | (p & 0xf0);
						color->r = i;
						color->g = i;
						color->b = i;
						color->a = (p & 0xf) | ((p << 4) & 0xf0);
					}
					else
					{
						UINT16 c = tlut[p << 2];
						if (!other_modes.tlut_type)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
                    		color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (c >> 8) & 0xff;
							color->a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = (UINT16*)TMEM;
                    int taddr = ((tbase >> 1) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
					UINT16 c = tc[taddr ^ WORD_ADDR_XOR];
					UINT8 i = (c >> 8);

					if (!other_modes.en_tlut)
					{
						color->r = i;
						color->g = i;
						color->b = i;
						color->a = c & 0xff;
					}
					else
					{
						c = tlut[(c >> 8) << 2];
						if (!other_modes.tlut_type)
						{
							color->r = (((c >> 11) & 0x1f) << 3) | (((c >> 11) & 0x1f) >> 2);
							color->g = (((c >>  6) & 0x1f) << 3) | (((c >>  6) & 0x1f) >> 2);
							color->b = (((c >>  1) & 0x1f) << 3) | (((c >>  1) & 0x1f) >> 2);
                    		color->a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color->r = c >> 8;
							color->g = c >> 8;
							color->b = c >> 8;
                    		color->a = c & 0xff;
						}
					}
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
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
					UINT8 c = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
					c |= (c << 4);

					if (!other_modes.en_tlut)
					{
						color->r = c;
						color->g = c;
						color->b = c;
						color->a = c;
					}
					else
					{
						UINT16 k = tlut[((tpal << 4) | c) << 2];
						if (!other_modes.tlut_type)
						{
							color->r = (((k >> 11) & 0x1f) << 3) | (((k >> 11) & 0x1f) >> 2);
							color->g = (((k >>  6) & 0x1f) << 3) | (((k >>  6) & 0x1f) >> 2);
							color->b = (((k >>  1) & 0x1f) << 3) | (((k >>  1) & 0x1f) >> 2);
							color->a = (k & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (k >> 8) & 0xff;
							color->a = k & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = (UINT8*)TMEM;
                    int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
					UINT8 c = tc[taddr ^ BYTE_ADDR_XOR];

					if (!other_modes.en_tlut)
					{
						color->r = c;
						color->g = c;
						color->b = c;
						color->a = c;
					}
					else
					{
						UINT16 k = tlut[ c << 2];
						if (!other_modes.tlut_type)
						{
							color->r = (((k >> 11) & 0x1f) << 3) | (((k >> 11) & 0x1f) >> 2);
							color->g = (((k >>  6) & 0x1f) << 3) | (((k >>  6) & 0x1f) >> 2);
							color->b = (((k >>  1) & 0x1f) << 3) | (((k >>  1) & 0x1f) >> 2);
							color->a = (k & 1) ? 0xff : 0;
						}
						else
						{
							color->r = color->g = color->b = (k >> 8) & 0xff;
							color->a = k & 0xff;
						}
					}
					break;
				}
				default:
					color->r = color->g = color->b = color->a = 0xff;
					//fatalerror("FETCH_TEXEL: unknown I texture size %d\n", tsize);
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

INLINE void TEXTURE_PIPELINE(COLOR* TEX, INT32 SSS, INT32 SST, UINT32 NOBILINEAR, UINT32 tilenum)
{
#define RELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 maxs, maxt;
	if (other_modes.sample_type)
	{
		COLOR t0, t1, t2, t3;
		int sss1, sst1, sss2, sst2;
		INT32 rt0 = 0, rt1 = 0, rt2 = 0, rt3 = 0;
		INT32 gt0 = 0, gt1 = 0, gt2 = 0, gt3 = 0;
		INT32 bt0 = 0, bt1 = 0, bt2 = 0, bt3 = 0;
		INT32 at0 = 0, at1 = 0, at2 = 0, at3 = 0;

		INT32 SFRAC = 0, TFRAC = 0, INVSF = 0, INVTF = 0;
		INT32 R32, G32, B32, A32;
		INT32 maxs2, maxt2;
		int upper = 0;

		t0.r = t0.g = t0.b = t0.a = 0;
		t1.r = t1.g = t1.b = t1.a = 0;
		t2.r = t2.g = t2.b = t2.a = 0;
		t3.r = t3.g = t3.b = t3.a = 0;

		sss1 = SSS;
		sst1 = SST;

		SHIFT(&sss1, &sst1, &maxs, &maxt, tilenum);

		sss2 = sss1 + 32; sst2 = sst1 + 32;
		maxs2 = ((sss2 >> 3) >= tile[tilenum].sh);
		maxt2 = ((sst2 >> 3) >= tile[tilenum].th);

		sss1 = RELATIVE(sss1, tile[tilenum].sl);
		sst1 = RELATIVE(sst1, tile[tilenum].tl);
		sss2 = RELATIVE(sss2, tile[tilenum].sl);
		sst2 = RELATIVE(sst2, tile[tilenum].tl);

		SFRAC = sss1 & 0x1f;
		TFRAC = sst1 & 0x1f;

		CLAMP(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tilenum);
		CLAMP_LIGHT(&sss2, &sst2, maxs2, maxt2, tilenum);

		MASK(&sss1, &sst1, tilenum);
        MASK(&sss2, &sst2, tilenum);

		upper = ((SFRAC + TFRAC) >= 0x20);
		if (upper)
		{
			INVSF = 0x20 - SFRAC;
			INVTF = 0x20 - TFRAC;
		}

		if (other_modes.mid_texel || !upper)
		{
			FETCH_TEXEL(&t0, sss1, sst1, tilenum);
		}
		FETCH_TEXEL(&t1, sss2, sst1, tilenum);
		FETCH_TEXEL(&t2, sss1, sst2, tilenum);
		if (other_modes.mid_texel || upper)
		{
			FETCH_TEXEL(&t3, sss2, sst2, tilenum);
		}
		if (other_modes.mid_texel || !upper)
		{
			rt0 = t0.r; gt0 = t0.g; bt0 = t0.b; at0 = t0.a;
		}
		rt1 = t1.r; rt2 = t2.r; gt1 = t1.g; gt2 = t2.g; bt1 = t1.b; bt2 = t2.b; at1 = t1.a; at2 = t2.a;
		if (other_modes.mid_texel || upper)
		{
			rt3 = t3.r; gt3 = t3.g; bt3 = t3.b; at3 = t3.a;
		}

		if (!other_modes.mid_texel || SFRAC!= 0x10 || TFRAC != 0x10)
		{
			if (upper)
			{
				R32 = rt3 + ((INVSF*(rt2 - rt3))>>5) + ((INVTF*(rt1 - rt3))>>5);
				TEX->r = (R32 < 0) ? 0 : R32;
				G32 = gt3 + ((INVSF*(gt2 - gt3))>>5) + ((INVTF*(gt1 - gt3))>>5);
				TEX->g = (G32 < 0) ? 0 : G32;
				B32 = bt3 + ((INVSF*(bt2 - bt3))>>5) + ((INVTF*(bt1 - bt3))>>5);
				TEX->b = (B32 < 0) ? 0 : B32;
				A32 = at3 + ((INVSF*(at2 - at3))>>5) + ((INVTF*(at1 - at3))>>5);
				TEX->a = (A32 < 0) ? 0 : A32;
			}
			else
			{
				R32 = rt0 + ((SFRAC*(rt1 - rt0))>>5) + ((TFRAC*(rt2 - rt0))>>5);
				TEX->r = (R32 < 0) ? 0 : R32;
				G32 = gt0 + ((SFRAC*(gt1 - gt0))>>5) + ((TFRAC*(gt2 - gt0))>>5);
				TEX->g = (G32 < 0) ? 0 : G32;
				B32 = bt0 + ((SFRAC*(bt1 - bt0))>>5) + ((TFRAC*(bt2 - bt0))>>5);
				TEX->b = (B32 < 0) ? 0 : B32;
				A32 = at0 + ((SFRAC*(at1 - at0))>>5) + ((TFRAC*(at2 - at0))>>5);
				TEX->a = (A32 < 0) ? 0 : A32;
			}
		}
		else // Is this accurate?
		{
			TEX->r = (rt0 + rt1 + rt2 + rt3) >> 2;
			TEX->g = (gt0 + gt1 + gt2 + gt3) >> 2;
			TEX->b = (bt0 + bt1 + bt2 + bt3) >> 2;
			TEX->a = (at0 + at1 + at2 + at3) >> 2;
		}
	}
	else
	{
		int sss1, sst1;
		INT32 SFRAC, TFRAC;

		sss1 = SSS;
		sst1 = SST;

		SHIFT(&sss1, &sst1, &maxs, &maxt, tilenum);
		sss1 = RELATIVE(sss1, tile[tilenum].sl);
		sst1 = RELATIVE(sst1, tile[tilenum].tl);

		sss1 += 0x10;
		sst1 += 0x10;

		SFRAC = sss1 & 0x1f;
		TFRAC = sst1 & 0x1f;

		CLAMP(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tilenum);

        MASK(&sss1, &sst1, tilenum);

		/* point sample */
		FETCH_TEXEL(TEX, sss1, sst1, tilenum);
	}
}

INLINE UINT32 z_decompress(UINT16 *zb)
{
	UINT32 exponent = (*zb >> 13) & 7;
	UINT32 mantissa = (*zb >> 2) & 0x7ff;
	return ((mantissa << z_dec_table[exponent].shift) + z_dec_table[exponent].add);
}

INLINE void z_build_com_table(void) // Thanks to angrylion, Gonetz and Orkin
{
	int j = 0;
	for(j = 0; j < 0x40000; j++)
	{
		UINT32 exponent = 0;
		UINT32 testbit = 0x20000;
		UINT32 mantissa = 0;
		while( (j & testbit) && (exponent < 7) )
		{
			exponent++;
			testbit = 1 << (17 - exponent);
		}

		mantissa = (j >> (6 - (6 < exponent ? 6 : exponent))) & 0x7ff;
		z_com_table[j] = (UINT16)(((exponent << 11) | mantissa) << 2);
	}
}

INLINE void z_store(UINT16* zb, UINT8* zhb, UINT32 z, UINT32 deltaz)
{
	UINT8 deltazmem = 15;
	int j = 0;
	z &= 0x3ffff;
	deltaz &= 0xffff;
	for(j = 15; j >= 0; j--)
	{
		if( (deltaz >> j) == 1 )
		{
			break;
		}
		else
		{
			deltazmem--;
		}
	}
	if (deltazmem>15)
	{
		deltazmem=0;
	}
	*zb = z_com_table[z]|(deltazmem>>2);
	*zhb = (deltazmem & 3);
}

INLINE UINT16 dz_decompress(UINT16* zb, UINT8* zhb)
{
	UINT32 dz_compressed = (((*zb & 3) << 2)|(*zhb & 3));
	return (1 << dz_compressed);
}

INLINE UINT32 z_compare(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix)
{
	int force_coplanar = 0;
	UINT32 oz = z_decompress(zb);
	UINT32 dzmem = dz_decompress(zb, zhb);
	UINT32 dznew = 0;
	UINT32 farther = 0;
	UINT32 diff = 0;
	UINT32 nearer = 0;
	UINT32 infront = 0;
	UINT32 max = 0;
	int precision_factor = (oz >> 15) & 0xf;
	int overflow = 0;
	int cvgcoeff = 0;
	UINT32 mempixel, memory_cvg;

	sz &= 0x3ffff;
	if (dzmem == 0x8000 && precision_factor < 3)
	{
		force_coplanar = 1;
	}
	if (!precision_factor)
	{
		dzmem = ((dzmem << 1) > 16) ? (dzmem << 1) : 16;
	}
	else if (precision_factor == 1)
	{
		dzmem = ((dzmem << 1) > 8) ? (dzmem << 1) : 8;
	}
	else if (precision_factor == 2)
	{
		dzmem = ((dzmem << 1) > 4) ? (dzmem << 1) : 4;
	}
	if (dzmem == 0 && precision_factor < 3)
	{
		dzmem = 0xffff;
	}
	if (dzmem > 0x8000)
	{
		dzmem = 0xffff;
	}
	dznew =((dzmem > dzpix) ? dzmem : (UINT32)dzpix) << 3;
	dznew &= 0x3ffff;

	farther = ((sz + dznew) >= oz) ? 1 : 0;
	diff = (sz >= dznew) ? (sz - dznew) : 0;
	nearer = (diff <= oz) ? 1: 0;
	infront = (sz < oz) ? 1 : 0;
	max = (dzmem == 0x3ffff);

	if (force_coplanar)
	{
		farther = nearer = 1;
	}

	curpixel_overlap = 0;

	switch(fb_size)
	{
		case 1: /* Banjo Tooie */
			memory_cvg = 0; //??
			break;
		case 2:
			mempixel = *(UINT16*)fb;
			memory_cvg = ((mempixel & 1) << 2) + (*hb & 3);
			break;
		case 3:
			mempixel = *(UINT32*)fb;
			memory_cvg = (mempixel >> 5) & 7;
			break;
		default:
			fatalerror("z_compare: fb_size = %d",fb_size);
			break;
	}

	if (!other_modes.image_read_en)
	{
		memory_cvg = 7;
	}

	overflow = ((memory_cvg + curpixel_cvg - 1) > 7) ? 1 : 0;
	curpixel_overlap = (other_modes.force_blend || (!overflow && other_modes.antialias_en && farther));

	if (other_modes.z_mode == 1 && infront && farther && overflow)
	{
		cvgcoeff = ((dzmem >> dznew) - (sz >> dznew)) & 0xf;
		curpixel_cvg = ((cvgcoeff * (curpixel_cvg - 1)) >> 3) & 0xf;
	}
	if (curpixel_cvg > 8)
		curpixel_cvg = 8;

	switch(other_modes.z_mode)
	{
		case 0: // Opaque
			return (max || (overflow ? infront : nearer))? 1 : 0;
			break;
		case 1: // Interpenetrating
			return (max || (overflow ? infront : nearer))? 1 : 0;
			break;
		case 2: // Transparent
			return (infront || max)? 1: 0;
			break;
		case 3: // Decal
			return (farther && nearer && !max) ? 1 : 0;
			break;
		default:
			fatalerror( "z_mode = %d", other_modes.z_mode);
			break;
	}
}

INLINE INT32 normalize_dzpix(INT32 sum)
{
	int count = 0;
	if (sum & 0xc000)
	{
		return 0x8000;
	}
	if (!(sum & 0xffff))
	{
		return 1;
	}
	for(count = 0x2000; count > 0; count >>= 1)
    {
		if (sum & count)
		{
        	return(count << 1);
		}
    }
    return 0;
}

INLINE INT32 CLIP(INT32 value,INT32 min,INT32 max)
{
	if (value < min)
	{
		return min;
	}
	else if(value > max)
	{
		return max;
	}
	else
	{
		return value;
	}
}

INLINE COLOR video_filter16(UINT16* vbuff, UINT8* hbuff, UINT32 hres)
{
	COLOR filtered, penumax, penumin, max, min;
	UINT16 pix = *vbuff;
	UINT32 centercvg = (*hbuff & 3) + ((pix & 1) << 2) + 1;
	UINT32 numoffull = 1;
	UINT32 cvg;
	UINT32 r = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
	UINT32 g = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
	UINT32 b = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
	UINT32 backr[7], backg[7], backb[7];
	UINT32 invr[7], invg[7], invb[7];
	INT32 coeff;
	INT32 leftup = -hres - 2;
	INT32 leftdown = hres - 2;
	INT32 toleft = -2;
	UINT32 colr, colg, colb;
	UINT32 enb;
	int i = 0;

	filtered.r = filtered.g = filtered.b = filtered.a = 0;

	backr[0] = r;
	backg[0] = g;
	backb[0] = b;
	invr[0] = 255 - r;
	invg[0] = 255 - g;
	invb[0] = 255 - b;

	if (centercvg == 8)
	{
		filtered.r = r; filtered.g = g; filtered.b = b;
		return filtered;
	}

	for(i = 0; i < 5; i++)
	{
		pix = vbuff[leftup ^ WORD_ADDR_XOR];
		cvg = hbuff[leftup ^ BYTE_ADDR_XOR] & 3;
		if(i & 1)
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
				backg[numoffull] = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
				backb[numoffull] = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
				invr[numoffull] = 255 - backr[numoffull];
				invg[numoffull] = 255 - backg[numoffull];
				invb[numoffull] = 255 - backb[numoffull];
			}
			else
			{
                backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		leftup++;
	}

	for(i = 0; i < 5; i++)
	{
		pix = vbuff[leftdown ^ WORD_ADDR_XOR];
		cvg = hbuff[leftdown ^ BYTE_ADDR_XOR] & 3;
		if (i&1)
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
				backg[numoffull] = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
				backb[numoffull] = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
				invr[numoffull] = 255 - backr[numoffull];
				invg[numoffull] = 255 - backg[numoffull];
				invb[numoffull] = 255 - backb[numoffull];
			}
			else
			{
                backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		leftdown++;
	}

	for(i = 0; i < 5; i++)
	{
		pix = vbuff[toleft ^ WORD_ADDR_XOR];
		cvg = hbuff[toleft ^ BYTE_ADDR_XOR] & 3;
		if (!(i&3))
		{
			if (cvg == 3 && (pix & 1))
			{
				backr[numoffull] = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
				backg[numoffull] = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
				backb[numoffull] = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
				invr[numoffull] = 255 - backr[numoffull];
				invg[numoffull] = 255 - backg[numoffull];
				invb[numoffull] = 255 - backb[numoffull];
			}
			else
			{
                backr[numoffull] = invr[numoffull] = 0;
				backg[numoffull] = invg[numoffull] = 0;
				backb[numoffull] = invb[numoffull] = 0;
			}
			numoffull++;
		}
		toleft++;
	}

	if (numoffull != 7)
	{
		fatalerror("Something went wrong in vi_filter16");
	}

	video_max(&backr[0], &max.r, &enb);
	for(i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backr[i] = 0;
		}
	}
	video_max(&backg[0], &max.g, &enb);
	for (i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backg[i] = 0;
		}
	}
	video_max(&backb[0], &max.b, &enb);
	for (i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backb[i] = 0;
		}
	}
	video_max(&invr[0], &min.r, &enb);
	for (i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backr[i] = 0;
		}
	}
	video_max(&invg[0], &min.g, &enb);
	for (i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backg[i] = 0;
		}
	}
	video_max(&invb[0], &min.b, &enb);
	for (i = 1; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			backb[i] = 0;
		}
	}

	video_max(&backr[0], &penumax.r, &enb);
	i = ge_two(enb);
	penumax.r = i ? max.r : penumax.r;

	video_max(&backg[0], &penumax.g, &enb);
	i = ge_two(enb);
	penumax.g = i ? max.g : penumax.g;

	video_max(&backb[0], &penumax.b, &enb);
	i = ge_two(enb);
	penumax.b = i ? max.b : penumax.b;

	video_max(&invr[0], &penumin.r, &enb);
	i = ge_two(enb);
	penumin.r = i ? min.r : penumin.r;

	video_max(&invg[0], &penumin.g, &enb);
	i = ge_two(enb);
	penumin.g = i ? min.g : penumin.g;

	video_max(&invb[0], &penumin.b, &enb);
	i = ge_two(enb);
	penumin.b = i ? min.b : penumin.b;

	penumin.r = 255 - penumin.r;
	penumin.g = 255 - penumin.g;
	penumin.b = 255 - penumin.b;

	colr = (UINT32)penumin.r + (UINT32)penumax.r - (r << 1);
	colg = (UINT32)penumin.g + (UINT32)penumax.g - (g << 1);
	colb = (UINT32)penumin.b + (UINT32)penumax.b - (b << 1);
	coeff = 8 - (INT32)centercvg;
	colr = (((colr * coeff) + 4) >> 3) + r;
	colg = (((colg * coeff) + 4) >> 3) + g;
	colb = (((colb * coeff) + 4) >> 3) + b;
	colr &= 0xff;
	colg &= 0xff;
	colb &= 0xff;

	filtered.r = colr;
	filtered.g = colg;
	filtered.b = colb;
	return filtered;
}

// This needs to be fixed for endianness.
INLINE void divot_filter16(INT32* r, INT32* g, INT32* b, UINT16* fbuff, UINT32 fbuff_index)
{
	UINT32 leftr, leftg, leftb, rightr, rightg, rightb;
	UINT16 leftpix, rightpix;
	UINT16* next, *prev;
	UINT32 Lsw = fbuff_index & 1;
	next = (Lsw) ? (UINT16*)(fbuff - 1) : (UINT16*)(fbuff + 3);
	prev = (Lsw) ? (UINT16*)(fbuff - 3) : (UINT16*)(fbuff + 1);
	leftpix = *prev;
	rightpix = *next;

	//leftpix = *(fbuff - 1); //for BE targets
	//rightpix = *(fbuff + 1);

	leftr = (((leftpix >> 11) & 0x1f) << 3) | (((leftpix >> 11) & 0x1f) >> 2);
	leftg = (((leftpix >> 6) & 0x1f) << 3) | (((leftpix >> 6) & 0x1f) >> 2);
	leftb = (((leftpix >> 1) & 0x1f) << 3) | (((leftpix >> 1) & 0x1f) >> 2);
	rightr = (((rightpix >> 11) & 0x1f) << 3) | (((rightpix >> 11) & 0x1f) >> 2);
	rightg = (((rightpix >> 6) & 0x1f) << 3) | (((rightpix >> 6) & 0x1f) >> 2);
	rightb = (((rightpix >> 1) & 0x1f) << 3) |(((rightpix >> 1) & 0x1f) >> 2);
	if ((leftr >= *r && rightr >= leftr) || (leftr >= rightr && *r >= leftr))
	{
		*r = leftr; //left = median value
	}
	if ((rightr >= *r && leftr >= rightr) || (rightr >= leftr && *r >= rightr))
	{
		*r = rightr; //right = median, else *r itself is median
	}
	if ((leftg >= *g && rightg >= leftg) || (leftg >= rightg && *g >= leftg))
	{
		*g = leftg;
	}
	if ((rightg >= *g && leftg >= rightg) || (rightg >= leftg && *g >= rightg))
	{
		*g = rightg;
	}
	if ((leftb >= *b && rightb >= leftb) || (leftb >= rightb && *b >= leftb))
	{
		*b = leftb;
	}
	if ((rightb >= *b && leftb >= rightb) || (rightb >= leftb && *b >= rightb))
	{
		*b = rightb;
	}
}

INLINE void divot_filter16_buffer(int* r, int* g, int* b, COLOR* vibuffer)
{
	UINT32 leftr, leftg, leftb, rightr, rightg, rightb;
	COLOR leftpix, rightpix, filtered;
	leftpix = vibuffer[-1];
	rightpix = vibuffer[1];
	filtered = *vibuffer;

	*r = filtered.r;
	*g = filtered.g;
	*b = filtered.b;
	leftr = leftpix.r;
	leftg = leftpix.g;
	leftb = leftpix.b;
	rightr = rightpix.r;
	rightg = rightpix.g;
	rightb = rightpix.b;
	if ((leftr >= *r && rightr >= leftr) || (leftr >= rightr && *r >= leftr))
	{
		*r = leftr; //left = median value
	}
	if ((rightr >= *r && leftr >= rightr) || (rightr >= leftr && *r >= rightr))
	{
		*r = rightr; //right = median, else *r itself is median
	}
	if ((leftg >= *g && rightg >= leftg) || (leftg >= rightg && *g >= leftg))
	{
		*g = leftg;
	}
	if ((rightg >= *g && leftg >= rightg) || (rightg >= leftg && *g >= rightg))
	{
		*g = rightg;
	}
	if ((leftb >= *b && rightb >= leftb) || (leftb >= rightb && *b >= leftb))
	{
		*b = leftb;
	}
	if ((rightb >= *b && leftb >= rightb) || (rightb >= leftb && *b >= rightb))
	{
		*b = rightb;
	}

	filtered.r = *r;
	filtered.g = *g;
	filtered.b = *b;
}

// Fix me.
INLINE void restore_filter16(int* r, int* g, int* b, UINT16* fbuff, UINT32 fbuff_index, UINT32 hres)
{
	INT32 leftuppix = -hres - 1;
	INT32 leftdownpix = hres - 1;
	INT32 toleftpix = -1;
	UINT8 tempr, tempg, tempb;
	UINT16 pix;
	int i;

	UINT8 r5 = *r;
	UINT8 g5 = *g;
	UINT8 b5 = *b;
	r5 &= ~7;
	g5 &= ~7;
	b5 &= ~7;

	for (i = 0; i < 3; i++)
	{
		pix = fbuff[leftuppix ^ 1];
		tempr = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
		tempg = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
		tempb = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
		tempr &= ~7;
		tempg &= ~7;
		tempb &= ~7;
		if (tempr > r5)
		{
			*r += 1;
		}
		if (tempr < r5)
		{
			*r -= 1;
		}
		if (tempg > g5)
		{
			*g += 1;
		}
		if (tempg < g5)
		{
			*g -= 1;
		}
		if (tempb > b5)
		{
			*b += 1;
		}
		if (tempb < b5)
		{
			*b -= 1;
		}
		leftuppix++;
	}

	for (i = 0; i < 3; i++)
	{
		pix = fbuff[leftdownpix ^ 1];
		tempr = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
		tempg = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
		tempb = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
		tempr &= ~7;
		tempg &= ~7;
		tempb &= ~7;
		if (tempr > r5)
		{
			*r += 1;
		}
		if (tempr < r5)
		{
			*r -= 1;
		}
		if (tempg > g5)
		{
			*g += 1;
		}
		if (tempg < g5)
		{
			*g -= 1;
		}
		if (tempb > b5)
		{
			*b += 1;
		}
		if (tempb < b5)
		{
			*b -= 1;
		}
		leftdownpix++;
	}
	for(i = 0; i < 3; i++)
	{
		if (!(i & 1))
		{
			pix = fbuff[toleftpix ^ 1];
			tempr = (((pix >> 11) & 0x1f) << 3) | (((pix >> 11) & 0x1f) >> 2);
			tempg = (((pix >> 6) & 0x1f) << 3) | (((pix >> 6) & 0x1f) >> 2);
			tempb = (((pix >> 1) & 0x1f) << 3) | (((pix >> 1) & 0x1f) >> 2);
			tempr &= ~7;
			tempg &= ~7;
			tempb &= ~7;
			if (tempr > r5)
			{
				*r += 1;
			}
			if (tempr < r5)
			{
				*r -= 1;
			}
			if (tempg > g5)
			{
				*g += 1;
			}
			if (tempg < g5)
			{
				*g -= 1;
			}
			if (tempb > b5)
			{
				*b += 1;
			}
			if (tempb < b5)
			{
				*b -= 1;
			}
		}
		toleftpix++;
	}
}

INLINE void restore_filter16_buffer(INT32* r, INT32* g, INT32* b, COLOR* vibuff, UINT32 hres)
{
	COLOR filtered;
	COLOR leftuppix, leftdownpix, leftpix;
	COLOR rightuppix, rightdownpix, rightpix;
	COLOR uppix, downpix;
	INT32 ihres = (INT32)hres; //can't apply unary minus to unsigned

	leftuppix = vibuff[-ihres - 1];
	leftdownpix = vibuff[ihres - 1];
	leftpix = vibuff[-1];

	rightuppix = vibuff[-ihres + 1];
	rightdownpix = vibuff[ihres + 1];
	rightpix = vibuff[1];

	uppix = vibuff[-ihres];
	downpix = vibuff[ihres];
	filtered = *vibuff;

	restore_two(&filtered, &leftuppix);
	restore_two(&filtered, &uppix);
	restore_two(&filtered, &rightuppix);

	restore_two(&filtered, &leftpix);
	restore_two(&filtered, &rightpix);

	restore_two(&filtered, &leftdownpix);
	restore_two(&filtered, &downpix);
	restore_two(&filtered, &rightdownpix);

	*r = filtered.r;
	*g = filtered.g;
	*b = filtered.b;

	*r = CLIP(*r, 0, 0xff);
	*g = CLIP(*g, 0, 0xff);
	*b = CLIP(*b, 0, 0xff);
}

// This is wrong, only the 5 upper bits are compared.
INLINE void restore_two(COLOR* filtered, COLOR* neighbour)
{
	if (neighbour->r > filtered->r)
	{
		filtered->r += 1;
	}
	if (neighbour->r < filtered->r)
	{
		filtered->r -= 1;
	}
	if (neighbour->g > filtered->g)
	{
		filtered->g += 1;
	}
	if (neighbour->g < filtered->g)
	{
		filtered->g -= 1;
	}
	if (neighbour->b > filtered->b)
	{
		filtered->b += 1;
	}
	if (neighbour->b < filtered->b)
	{
		filtered->b -= 1;
	}
}

INLINE void video_max(UINT32* Pixels, UINT8* max, UINT32* enb)
{
	int i;
	int pos = 0;
	*enb = 0;
	for(i = 0; i < 7; i++)
	{
	    if (Pixels[i] >= Pixels[pos])
			pos = i;
	}
	for(i = 0; i < 7; i++)
	{
	    if (Pixels[i] != Pixels[pos])
		    *enb += (1 << i);
	}
	*max = Pixels[pos];
}

INLINE UINT32 ge_two(UINT32 enb)
{
	int i;
	int j = 0;
	for(i = 0; i < 7; i++)
	{
		if (!((enb >> i) & 1))
		{
			j++;
		}
	}
	return (j > 1);
}

INLINE void calculate_clamp_diffs(UINT32 prim_tile)
{
	int start, end;
	if (other_modes.tex_lod_en || prim_tile == 7)
	{
		start = 0;
		end = 7;
	}
	else
	{
		start = prim_tile;
		end = prim_tile + 1;
	}
	for (; start <= end; start++)
	{
		clamp_s_diff[start] = (tile[start].sh >> 2) - (tile[start].sl >> 2);
		clamp_t_diff[start] = (tile[start].th >> 2) - (tile[start].tl >> 2);
	}
}

INLINE void rgb_dither(INT32* r, INT32* g, INT32* b, int dith)
{
	if ((*r & 7) > dith)
	{
		*r = (*r & 0xf8) + 8;
		if (*r > 247)
			*r = 255;
	}
	if ((*g & 7) > dith)
	{
		*g = (*g & 0xf8) + 8;
		if (*g > 247)
			*g = 255;
	}
	if ((*b & 7) > dith)
	{
		*b = (*b & 0xf8) + 8;
		if (*b > 247)
			*b = 255;
	}
}

INLINE void set_shade_for_rects(void)
{
	shade_color.r = 0;
	shade_color.g = 0;
	shade_color.b = 0;
	shade_color.a = 0;
}

INLINE void set_shade_for_tris(UINT32 shade)
{
	if (!shade) // Hack, needed for Top Gear Rally 2 sky
	{
		shade_color.r = prim_color.r;
		shade_color.g = prim_color.g;
		shade_color.b = prim_color.b;;
		shade_color.a = prim_color.a;
	}
}

INLINE UINT32 getlog2(UINT32 lod_clamp)
{
	int i;
	if (lod_clamp < 2)
	{
		return 0;
	}
	else
	{
		for (i = 7; i > 0; i--)
		{
			if ((lod_clamp >> i) & 1)
			{
				return i;
			}
		}
	}
	return 0;
}

INLINE void copy_colors(COLOR* dst, COLOR* src)
{
	dst->r = src->r;
	dst->g = src->g;
	dst->b = src->b;
	dst->a = src->a;
}

INLINE void BILERP_AND_WRITE(UINT32* src0, UINT32* src1, UINT32* dest)
{
	UINT32 r1, g1, b1, col0, col1;
	col0 = *src0;
	col1 = *src1;
	r1 = (((col0 >> 16)&0xff) + ((col1 >> 16)&0xff))>>1;
	g1 = (((col0 >> 8)&0xff) + ((col1 >> 8)&0xff))>>1;
	b1 = (((col0 >> 0)&0xff) + ((col1 >> 0)&0xff))>>1;
	*dest = (r1 << 16) | (g1 << 8) | b1;
}

INLINE void tcdiv(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst)
{
	int w_carry;
	int shift;
	int normout;
	int wnorm;
	int temppoint, tempslope;
	int tlu_rcp;
	int tempmask;
	int shift_value;

	if ((sw & 0x8000) || !(sw & 0x7fff))
	{
		w_carry = 1;
	}
	else
	{
		w_carry = 0;
	}

	sw &= 0x7fff;
	for(shift = 1; shift <= 14 && !((sw << shift) & 0x8000); shift++);
	shift -= 1;

	normout = ((sw << shift) & 0x3fff) >> 8;
	wnorm = ((sw << shift) & 0xff) << 2;
	temppoint = NormPointRom[normout];
	tempslope = NormSlopeRom[normout];

	tlu_rcp = ((-(tempslope * wnorm)) >> 10);
	tlu_rcp = (tlu_rcp + temppoint);

	*sss = SIGN16(ss) * tlu_rcp;
	*sst = SIGN16(st) * tlu_rcp;
	tempmask = ((1 << (shift+1)) - 1) << (29 - shift);
	shift_value = 13 - shift;
	if (shift_value < 0)
	{
		shift_value = 0;
	}
	*sss >>= shift_value;
	*sst >>= shift_value;
	if (shift == 0xe)
	{
		*sss <<= 1;
		*sst <<= 1;
	}
}

void col_decode16(UINT16* addr, COLOR* col)
{
	col->r = (((*addr >> 11) & 0x1f) << 3) | (((*addr >> 11) & 0x1f) >> 2);
	col->g = (((*addr >> 6) & 0x1f) << 3) | (((*addr >> 6) & 0x1f) >> 2);
	col->b = (((*addr >> 1) & 0x1f) << 3) | (((*addr >> 6) & 0x1f) >> 2);
	col->a = (*addr & 1) ? 0xff : 0;
}

static void texture_rectangle_16bit(running_machine *machine, TEX_RECTANGLE *rect)
{
	UINT16 *fb = (UINT16*)&rdram[(fb_address / 4)];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

	int i, j;
	int x1, x2, y1, y2;
	int s, t;
	int ss, st;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum = rect->tilenum;
	UINT32 tilenum2 = 0;

	x1 = (rect->xh / 4);
	x2 = (rect->xl / 4);
	y1 = (rect->yh / 4);
	y2 = (rect->yl / 4);

	if (x2<=x1)
	{
		x2 = x1 + 1;
	}
	if (y1==y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}
	if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		rect->dsdx /= 4;
		x2 += 1;
		y2 += 1;
	}
	else
	{
		if ((rect->xl & 3) == 3) // Needed by Mega Man 64
		{
			x2++;
		}
		if ((rect->yl & 3) == 3)
		{
			y2++;
		}
	}

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	calculate_clamp_diffs(tilenum);

	if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		if (!other_modes.tex_lod_en)
		{
			tilenum2 = (tilenum + 1) & 7;
		}
		else
		{
			tilenum2 = (tilenum + 1) & 7;
		}
	}


	set_shade_for_rects(); // Needed by Pilotwings 64

	t = ((int)(rect->t)) << 5;

	if (other_modes.cycle_type == CYCLE_TYPE_1)
	{
		for (j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
			if (j >= clipy1 && j < clipy2)
			{
            	s = ((int)(rect->s)) << 5;

				for (i = x1; i < x2; i++)
				{
					if (i >= clipx1 && i < clipx2)
					{
						COLOR c;
						int rendered = 0;
						int dith = 0;
						int curpixel = fb_index + i;
						UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
						UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
						UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
						UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

						curpixel_cvg = 8;

						ss = s >> 5;
						st = t >> 5;
						if (rect->flip)
						{
							TEXTURE_PIPELINE(&texel0_color, st, ss, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
						}
						else
						{
							TEXTURE_PIPELINE(&texel0_color, ss, st, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
						}

						c = COLOR_COMBINER(machine, 0);

						if (!other_modes.rgb_dither_sel)
						{
							dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
						}
						else if (other_modes.rgb_dither_sel == 1)
						{
							dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
						}


						if (other_modes.z_compare_en && other_modes.z_source_sel)
						{
							if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z)<<3,primitive_delta_z))
							{
								rendered = BLENDER1_16(machine, fbcur, hbcur, c, dith);
							}
						}
						else
						{
							rendered = BLENDER1_16(machine, fbcur, hbcur, c, dith);
						}
						if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
						{
							z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
						}
					}

					s += (int)(rect->dsdx);
				}
			}
			t += (int)(rect->dtdy);
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		for (j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;

			if (j >= clipy1 && j < clipy2)
			{
				s = (int)(rect->s) << 5;

				for (i = x1; i < x2; i++)
				{
					if (i >= clipx1 && i < clipx2)
					{
						COLOR c1, c2;
						int rendered=0;
						int curpixel = fb_index + i;
						UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
						UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
						UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
						UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
						int dith = 0;

						curpixel_cvg = 8;

						ss = s >> 5;
						st = t >> 5;

						if (rect->flip)
						{
							TEXTURE_PIPELINE(&texel0_color, st, ss, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum);
							TEXTURE_PIPELINE(&texel1_color, st, ss, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum2);
						}
						else
						{
							TEXTURE_PIPELINE(&texel0_color, ss, st, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum);
							TEXTURE_PIPELINE(&texel1_color, ss, st, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum2);
						}
						c1 = COLOR_COMBINER(machine, 0);
						c2 = COLOR_COMBINER(machine, 1);

						if (!other_modes.rgb_dither_sel)
						{
							dith = magic_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
						}
						else if (other_modes.rgb_dither_sel == 1)
						{
							dith = bayer_matrix[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
						}

						if (other_modes.z_compare_en && other_modes.z_source_sel)
						{
							if (z_compare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z))
							{
								rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);
							}
						}
						else
						{
							rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);
						}
						if (other_modes.z_update_en && other_modes.z_source_sel && rendered)
						{
							z_store(zbcur, zhbcur, ((UINT32)primitive_z) << 3,primitive_delta_z);
						}
					}

					s += (rect->dsdx);
				}
			}
			t += (rect->dtdy);
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		for (j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;
			if (j >= clipy1 && j < clipy2)
			{
				s = (int)(rect->s) << 5;

				for (i = x1; i < x2; i++)
				{
                	if (i >= clipx1 && i < clipx2)
					{
						ss = s >> 5;
						st = t >> 5;

						if (rect->flip)
						{
							TEXTURE_PIPELINE(&texel0_color, st, ss, 1, tilenum);
						}
						else
						{
							TEXTURE_PIPELINE(&texel0_color, ss, st, 1, tilenum);
						}

						curpixel_cvg = 8;

						if ((texel0_color.a != 0)||(!other_modes.alpha_compare_en))
						{
							fb[(fb_index + i) ^ WORD_ADDR_XOR] = ((texel0_color.r >> 3) << 11) | ((texel0_color.g >> 3) << 6) | ((texel0_color.b >> 3) << 1)|1;
						}
					}
					s += rect->dsdx;
				}
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

INLINE int BLENDER1_32(running_machine *machine, UINT32 *fb, COLOR c)
{
	UINT32 mem = *fb;
	int r, g, b;

	// Alpha compare
	if (!alpha_compare(machine, c.a))
	{
		return 0;
	}
	if (!curpixel_cvg)
	{
		return 0;
	}

	copy_colors(&pixel_color, &c);
	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	memory_color.r = (mem >>24) & 0xff;
	memory_color.g = (mem >> 16) & 0xff;
	memory_color.b = (mem >> 8) & 0xff;

	if (other_modes.image_read_en)
	{
		memory_color.a = mem & 0xe0;
	}
	else
	{
		memory_color.a = 0xe0;
	}

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[0];
		g = *blender1a_g[0];
		b = *blender1a_b[0];
	}
	else
	{
		int special_bsel = 0;
		if (blender2b_a[0] == &memory_color.a)
		{
			special_bsel = 1;
		}

		inv_pixel_color.a = 0xff - *blender1b_a[0];

		BLENDER_EQUATION(&r, &g, &b, 0, special_bsel);
	}

	return  (FBWRITE_32(fb,r,g,b));
}

INLINE int BLENDER2_32(running_machine *machine, UINT32 *fb, COLOR c1, COLOR c2)
{
	UINT32 mem = *fb;
	int r, g, b;
	int special_bsel = 0;

	// Alpha compare
	if (!alpha_compare(machine, c2.a))
	{
		return 0;
	}
	if (!curpixel_cvg)
	{
		return 0;
	}

	copy_colors(&pixel_color, &c2);
	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}

	memory_color.r = (mem >>24) & 0xff;
	memory_color.g = (mem >> 16) & 0xff;
	memory_color.b = (mem >> 8) & 0xff;

	if (other_modes.image_read_en)
	{
		memory_color.a = (mem & 0xe0);
	}
	else
	{
		memory_color.a = 0xe0;
	}

	if (blender2b_a[0] == &memory_color.a)
	{
		special_bsel = 1;
	}

	inv_pixel_color.a = 0xff - *blender1b_a[0];

	BLENDER_EQUATION(&r, &g, &b, 0, special_bsel);

	blended_pixel_color.r = r;
	blended_pixel_color.g = g;
	blended_pixel_color.b = b;

	pixel_color.r = r;
	pixel_color.g = g;
	pixel_color.b = b;

	if (!curpixel_overlap && !other_modes.force_blend)
	{
		r = *blender1a_r[1];
		g = *blender1a_g[1];
		b = *blender1a_b[1];
	}
	else
	{
		if (blender2b_a[1] == &memory_color.a)
		{
			special_bsel = 1;
		}
		else
		{
			special_bsel = 0;
		}

		inv_pixel_color.a = 0xff - *blender1b_a[1];

		BLENDER_EQUATION(&r, &g, &b, 1, special_bsel);
	}

	return  (FBWRITE_32(fb,r,g,b));
}

static void fill_rectangle_32bit(running_machine *machine, RECTANGLE *rect)
{
	UINT32 *fb = (UINT32*)&rdram[(fb_address / 4)];
	int index, i, j;
	int x1 = rect->xh / 4;
	int x2 = rect->xl / 4;
	int y1 = rect->yh / 4;
	int y2 = rect->yl / 4;
	int clipx1, clipx2, clipy1, clipy2;
	int fill_cvg = 0;

	if (x2 < x1)
	{
		stricterror ("SCARS (E) case: fill_rectangle_32bit");
	}

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

	set_shade_for_rects();

	fill_cvg = ((fill_color >> 5) & 7) + 1;

	if (other_modes.cycle_type == CYCLE_TYPE_FILL)
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * fb_width;

			for (i=x1; i <= x2; i++)
			{
				fb[(index + i)^1] = fill_color;
			}
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_1)
	{
		for (j=y1; j <= y2; j++)
		{
			COLOR c;
			index = j * fb_width;
			for (i=x1; i <= x2; i++)
			{
				curpixel_cvg = fill_cvg;
				c = COLOR_COMBINER(machine, 0);
				BLENDER1_32(machine, &fb[(index + i)], c);
			}
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		for (j=y1; j <= y2; j++)
		{
			COLOR c1, c2;
			index = j * fb_width;
			for (i=x1; i <= x2; i++)
			{
				curpixel_cvg = fill_cvg;
				c1 = COLOR_COMBINER(machine, 0);
				c2 = COLOR_COMBINER(machine, 1);
				BLENDER2_32(machine, &fb[(index + i)], c1, c2);
			}
		}
	}
	else
	{
		fatalerror("fill_rectangle_32bit: cycle type copy");
	}
}

INLINE void texture_rectangle_32bit(running_machine *machine, TEX_RECTANGLE *rect)
{	// TODO: Z-compare and Z-update
	UINT32 *fb = (UINT32*)&rdram[(fb_address / 4)];
	int i, j;
	int x1, x2, y1, y2;
	int s, t;
	int ss, st;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum = rect ->tilenum;
	UINT32 tilenum2 = 0;

	x1 = (rect->xh / 4);
	x2 = (rect->xl / 4);
	y1 = (rect->yh / 4);
	y2 = (rect->yl / 4);

	if (x2 < x1)
	{
		stricterror( "x2 < x1: texture_rectangle_32bit" );
	}

	if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		rect->dsdx /= 4;
		x2 += 1;
		y2 += 1;
	}

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	calculate_clamp_diffs(tilenum);

	if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		if (!other_modes.tex_lod_en)
		{
			tilenum2 = (tilenum + 1) & 7;
		}
		else
		{
			tilenum2 = (tilenum + 1) & 7;
		}
	}

	set_shade_for_rects(); // Needed by Pilotwings 64

	t = (int)(rect->t) << 5;

	if (other_modes.cycle_type == CYCLE_TYPE_1)
	{
		for (j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;

			s = (int)(rect->s) << 5;

			for (i = x1; i < x2; i++)
			{
				COLOR c;
				curpixel_cvg=8;
				ss = s >> 5;
				st = t >> 5;
				if (rect->flip)
				{
					TEXTURE_PIPELINE(&texel0_color, st, ss,(rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
				}
				else
				{
					TEXTURE_PIPELINE(&texel0_color, ss, st,(rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
				}

				c = COLOR_COMBINER(machine, 0);

				BLENDER1_32(machine, &fb[(fb_index + i)], c);

				s += rect->dsdx;
			}

			t += rect->dtdy;
		}
	}
	else if (other_modes.cycle_type == CYCLE_TYPE_2)
	{
		for (j = y1; j < y2; j++)
		{
			int fb_index = j * fb_width;

			s = (int)(rect->s) << 5;

			for (i = x1; i < x2; i++)
			{
				COLOR c1, c2;
				curpixel_cvg=8;
				ss = s >> 5;
				st = t >> 5;
				if (rect->flip)
				{
					TEXTURE_PIPELINE(&texel0_color, st, ss, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum);
					TEXTURE_PIPELINE(&texel1_color, st, ss, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum2);
				}
				else
				{
					TEXTURE_PIPELINE(&texel0_color, ss, st, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum);
					TEXTURE_PIPELINE(&texel1_color, ss, st, (rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)),tilenum2);
				}

				c1 = COLOR_COMBINER(machine, 0);
				c2 = COLOR_COMBINER(machine, 1);

				BLENDER2_32(machine, &fb[(fb_index + i)], c1, c2);

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

			for (i = x1; i < x2; i++)
			{
				ss = s >> 5;
				st = t >> 5;
				if (rect->flip)
				{
					TEXTURE_PIPELINE(&texel0_color, st, ss,(rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
				}
				else
				{
					TEXTURE_PIPELINE(&texel0_color, ss, st,(rect->dsdx == (1 << 10)) && (rect->dtdy == (1 << 10)), tilenum);
				}

				fb[fb_index + i] = (texel0_color.r << 24) | (texel0_color.g << 16) | (texel0_color.b << 8)|1;

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


static void render_spans_32(running_machine *machine, int start, int end, int tilenum, int shade, int texture, int zbuffer, int flip)
{
	UINT32 *fb = (UINT32*)&rdram[fb_address / 4];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];
	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 tilenum2 = 0;

	int dr = span[0].dr;
	int dg = span[0].dg;
	int db = span[0].db;
	int da = span[0].da;
	int dz = span[0].dz;
	int ds = span[0].ds;
	int dt = span[0].dt;
	int dw = span[0].dw;
	int dzpix = span[0].dzpix;
	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc = flip ? 1 : -1;

	calculate_clamp_diffs(tilenum);

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;


	if (other_modes.key_en)
	{
		stricterror( "render_spans_32: key_en" );
	}

	if (other_modes.cycle_type == CYCLE_TYPE_2 && texture)
	{
		if (!other_modes.tex_lod_en)
		{
			tilenum2 = (tilenum + 1) & 7;
		}
		else
		{
			tilenum2 = (tilenum + 1) & 7;
		}
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

	drinc = flip ? (dr) : -dr;
	dginc = flip ? (dg) : -dg;
	dbinc = flip ? (db) : -db;
	dainc = flip ? (da) : -da;
	dzinc = flip ? (dz) : -dz;
	dsinc = flip ? (ds) : -ds;
	dtinc = flip ? (dt) : -dt;
	dwinc = flip ? (dw) : -dw;

	set_shade_for_tris(shade);

	for (i = start; i <= end; i++)
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

		int x;
		int fb_index = fb_width * i;
		int length;

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
			UINT32 sz = z >> 13;
			int sss = 0, sst = 0;
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}

			if (x >= clipx1 && x < clipx2)
			{
				curpixel_cvg = span[i].cvg[x];
				if (curpixel_cvg > 8)
				{
					stricterror("render_spans_32: curpixel_cvg = %d", curpixel_cvg);
				}
				if (curpixel_cvg)
				{
					COLOR c1, c2;
					int curpixel = fb_index + x;
					UINT32* fbcur = &fb[curpixel];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* hbcur = 0;
					int z_compare_result = 1;

					c1.r = c1.g = c1.b = c1.a = 0;
					c2.r = c2.g = c2.b = c2.a = 0;

					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else
					{
						sss = ss;
						sst = st;
					}

					if (shade)
					{
						// Needed by Superman
						if (sr > 0xff) sr = 0xff;
						if (sg > 0xff) sg = 0xff;
						if (sb > 0xff) sb = 0xff;
						if (sa > 0xff) sa = 0xff;
						if (sr < 0) sr = 0;
						if (sg < 0) sg = 0;
						if (sb < 0) sb = 0;
						if (sa < 0) sa = 0;
						shade_color.r = sr;
						shade_color.g = sg;
						shade_color.b = sb;
						shade_color.a = sa;
					}

					if (texture)
					{
						if (other_modes.cycle_type == CYCLE_TYPE_1)
						{
							TEXTURE_PIPELINE(&texel0_color, sss, sst, 0, tilenum);
						}
						else
						{
							TEXTURE_PIPELINE(&texel0_color, sss, sst, 0, tilenum);
							TEXTURE_PIPELINE(&texel1_color, sss, sst, 0, tilenum2);
						}
					}

					if (other_modes.cycle_type == CYCLE_TYPE_1)
					{
						c1 = COLOR_COMBINER(machine, 0);
					}
					else if (other_modes.cycle_type == CYCLE_TYPE_2)
					{
						c1 = COLOR_COMBINER(machine, 0);
						c2 = COLOR_COMBINER(machine, 1);
  					}

					if ((zbuffer || other_modes.z_source_sel) && other_modes.z_compare_en)
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
					{
						int rendered = 0;

						if (other_modes.cycle_type == CYCLE_TYPE_1)
						{
							rendered = BLENDER1_32(machine, fbcur, c1);
						}
                		else
                		{
							rendered = BLENDER2_32(machine, fbcur, c1, c2);
						}

						if (other_modes.z_update_en && rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
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

static void render_spans_16(running_machine *machine, int start, int end, int tilenum, int shade, int texture, int zbuffer, int flip)
{
	UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];

	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

	UINT32 prim_tile = tilenum;
	UINT32 tilenum2 = 0;
	UINT32 disable_lod = 0;

	int LOD = 0;
	INT32 horstep, vertstep;
	INT32 l_tile;
	UINT32 magnify = 0;
	UINT32 distant = 0;

	int dr = span[0].dr;
	int dg = span[0].dg;
	int db = span[0].db;
	int da = span[0].da;
	int dz = span[0].dz;
	int ds = span[0].ds;
	int dt = span[0].dt;
	int dw = span[0].dw;
	int dzpix = span[0].dzpix;
	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc = flip ? 1 : -1;

	int nexts, nextt, nextsw;
	int lodclamp = 0;

	calculate_clamp_diffs(tilenum);

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	if (other_modes.cycle_type == CYCLE_TYPE_2 && texture && !other_modes.tex_lod_en)
	{
		tilenum2 = (prim_tile + 1) & 7;
	}
	if (texture && !other_modes.tex_lod_en)
	{
		tilenum = prim_tile;
	}


	if (other_modes.tex_lod_en && other_modes.cycle_type != CYCLE_TYPE_2) // Used by World Driver Championship
	{
		disable_lod = 1;
	}

	drinc = flip ? (dr) : -dr;
	dginc = flip ? (dg) : -dg;
	dbinc = flip ? (db) : -db;
	dainc = flip ? (da) : -da;
	dzinc = flip ? (dz) : -dz;
	dsinc = flip ? (ds) : -ds;
	dtinc = flip ? (dt) : -dt;
	dwinc = flip ? (dw) : -dw;

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

	set_shade_for_tris(shade); // Needed by backgrounds in Top Gear Rally 1

	for (i = start; i <= end; i++)
	{
		int xstart = span[i].lx;
		int xend = span[i].rx;
		int r = 0, g = 0, b = 0, a = 0;
		int z = span[i].z;
		int s = span[i].s;
		int t = span[i].t;
		int w = span[i].w;

		int x;

		int fb_index = fb_width * i;
		int length;

		if (shade)
		{
			r = span[i].r;
			g = span[i].g;
			b = span[i].b;
			a = span[i].a;
		}

		x = xend;

		length = flip ? (xstart - xend) : (xend - xstart); //Moogly

		for (j = 0; j <= length; j++)
		{
			int sr = 0, sg = 0, sb = 0, sa = 0;
			int ss = s >> 16;
			int st = t >> 16;
			int sw = w >> 16;
			int sz = z >> 13;
			int sss = 0, sst = 0;
			COLOR c1, c2;
			if (shade)
			{
				sr = r >> 16;
				sg = g >> 16;
				sb = b >> 16;
				sa = a >> 16;
			}
			c1.r = c1.g = c1.b = c1.a = 0;
			c2.r = c2.g = c2.b = c2.a = 0;
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}


			if (x >= clipx1 && x < clipx2)
			{
				int z_compare_result = 1;

				curpixel_cvg=span[i].cvg[x];

				if (curpixel_cvg > 8)
				{
					stricterror("render_spans_16: cvg of current pixel is %d", curpixel_cvg);
				}

				if (curpixel_cvg)
				{
					int curpixel = fb_index + x;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}

					if (other_modes.tex_lod_en && !disable_lod)
					{
						if (other_modes.persp_tex_en)
						{
							nextsw = (w + dwinc) >> 16;
							nexts = (s + dsinc) >> 16;
							nextt = (t + dtinc) >> 16;
							tcdiv(nexts, nextt, nextsw, &nexts, &nextt);
						}
						else
						{
							nexts = (s + dsinc)>>16;
							nextt = (t + dtinc)>>16;
						}

						lodclamp = 0;

						horstep = SIGN17(nexts & 0x1ffff) - SIGN17(sss & 0x1ffff);
						vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(sst & 0x1ffff);
						if (horstep & 0x20000)
						{
							horstep = ~horstep & 0x1ffff;
						}
						if (vertstep & 0x20000)
						{
							vertstep = ~vertstep & 0x1ffff;
						}
						LOD = ((horstep >= vertstep) ? horstep : vertstep);
						LOD = (LOD >= span[0].dymax) ? LOD : span[0].dymax;

						if ((LOD & 0x1c000) || lodclamp)
						{
							LOD = 0x7fff;
						}
						if (LOD < min_level)
						{
							LOD = min_level;
						}

						magnify = (LOD < 32) ? 1: 0;
						l_tile = getlog2((LOD >> 5) & 0xff);
						distant = ((LOD & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

						lod_frac = ((LOD << 3) >> l_tile) & 0xff;

						if (distant)
						{
							l_tile = max_level;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && magnify)
						{
							lod_frac = 0;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && distant)
						{
							lod_frac = 0xff;
						}
						if(other_modes.sharpen_tex_en && magnify)
						{
							lod_frac |= 0x100;
						}

						if (!other_modes.detail_tex_en)
						{
							tilenum = (prim_tile + l_tile);
							tilenum &= 7;
							if (other_modes.sharpen_tex_en)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else if (!distant)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else
							{
								tilenum2 = tilenum;
							}
						}
						else
						{
							if (!magnify)
							{
								tilenum = (prim_tile + l_tile + 1);
							}
							else
							{
								tilenum = (prim_tile + l_tile);
							}
							tilenum &= 7;

							if (!distant && !magnify)
							{
								tilenum2 = (prim_tile + l_tile + 2) & 7;
							}
							else
							{
								tilenum2 = (prim_tile + l_tile + 1) & 7;
							}
						}
					}

					if (shade)
					{
						if (sr > 0xff) sr = 0xff;
						if (sg > 0xff) sg = 0xff;
						if (sb > 0xff) sb = 0xff;
						if (sa > 0xff) sa = 0xff;
						if (sr < 0) sr = 0;
						if (sg < 0) sg = 0;
						if (sb < 0) sb = 0;
						if (sa < 0) sa = 0;
						shade_color.r = sr;
						shade_color.g = sg;
						shade_color.b = sb;
						shade_color.a = sa;
					}

					if (texture)
					{
						if (other_modes.cycle_type == CYCLE_TYPE_1)
						{
							TEXTURE_PIPELINE(&texel0_color, sss, sst, 0, tilenum);
						}
						else
						{
							TEXTURE_PIPELINE(&texel0_color, sss, sst, 0, tilenum);
							TEXTURE_PIPELINE(&texel1_color, sss, sst, 0, tilenum2);
						}
					}

					if (other_modes.cycle_type == CYCLE_TYPE_1)
					{
						c1 = COLOR_COMBINER(machine, 0);
					}
					else if (other_modes.cycle_type == CYCLE_TYPE_2)
					{
						c1 = COLOR_COMBINER(machine, 0);
						c2 = COLOR_COMBINER(machine, 1);
					}

					if ((zbuffer || other_modes.z_source_sel) && other_modes.z_compare_en)
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
					{
						int rendered = 0;
						int dith = 0;
						if (!other_modes.rgb_dither_sel)
						{
							dith = magic_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}
						else if (other_modes.rgb_dither_sel == 1)
						{
							dith = bayer_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
						}

                    	if (other_modes.cycle_type == CYCLE_TYPE_1)
                    	{
							rendered = BLENDER1_16(machine, fbcur, hbcur, c1, dith);
						}
						else
						{
							rendered = BLENDER2_16(machine, fbcur, hbcur, c1, c2, dith);
						}

						if (other_modes.z_update_en && rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
						}
					}
				}
			}

			if (shade)
			{
				r += drinc;
				g += dginc;
				b += dbinc;
				a += dainc;
			}
			z += dzinc;
			s += dsinc;
			t += dtinc;
			w += dwinc;

			x += xinc;
		}
	}
}

static void triangle(running_machine *machine, UINT32 w1, UINT32 w2, int shade, int texture, int zbuffer)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int r = 0, g = 0, b = 0, a = 0, z = 0, s = 0, t = 0, w = 0;
	int dr, dg, db, da;
	int drdx = 0, dgdx = 0, dbdx = 0, dadx = 0, dzdx = 0, dsdx = 0, dtdx = 0, dwdx = 0;
	int drdy = 0, dgdy = 0, dbdy = 0, dady = 0, dzdy = 0, dsdy = 0, dtdy = 0, dwdy = 0;
	int drde = 0, dgde = 0, dbde = 0, dade = 0, dzde = 0, dsde = 0, dtde = 0, dwde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dzdy_dz, dzdx_dz;
	int dsdylod, dtdylod;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	int sign_dxhdy = 0;

	int dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0, dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dseoff = 0, dteoff = 0, dweoff = 0, dreoff = 0, dgeoff = 0, dbeoff = 0, daeoff = 0, dzeoff = 0;

	int dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int shade_base = rdp_cmd_cur + 8;
	int texture_base = rdp_cmd_cur + 8;
	int zbuffer_base = rdp_cmd_cur + 8;

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

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if ((xl >> 16) & 0x2000)  xl |= 0xc0000000;
	if ((xm >> 16) & 0x2000)  xm |= 0xc0000000;
	if ((xh >> 16) & 0x2000)  xh |= 0xc0000000;

	z = 0;
	s = 0;	t = 0;	w = 0;
	dr = 0;		dg = 0;		db = 0;		da = 0;

	if (shade)
	{
		r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
		g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
		b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
		a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
		drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
		dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
		dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
		dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
		drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
		dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
		dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
		dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
		drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
		dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
		dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
		dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);
	}
	if (texture)
	{
		s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
		t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
		w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
		dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
		dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
		dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
		dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
		dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+12] & 0x0000ffff);
		dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
		dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
		dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+14] & 0x0000ffff);
		dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);
	}
	if (zbuffer)
	{
		z    = rdp_cmd_data[zbuffer_base+0];
		dzdx = rdp_cmd_data[zbuffer_base+1];
		dzde = rdp_cmd_data[zbuffer_base+2];
		dzdy = rdp_cmd_data[zbuffer_base+3];
	}

	span[0].ds = dsdx;
	span[0].dt = dtdx;
	span[0].dw = dwdx;
	span[0].dr = drdx & ~0x1f;
	span[0].dg = dgdx & ~0x1f;
	span[0].db = dbdx & ~0x1f;
	span[0].da = dadx & ~0x1f;
	span[0].dz = dzdx;
	dzdy_dz = (dzdy >> 16) & 0xffff;
	dzdx_dz = (dzdx >> 16) & 0xffff;
	span[0].dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	span[0].dzpix = normalize_dzpix(span[0].dzpix);
	dsdylod = dsdy >> 16;
	dtdylod = dtdy >> 16;
	if (dsdylod & 0x20000)
	{
		dsdylod = ~dsdylod & 0x1ffff;
	}
	if (dtdylod & 0x20000)
	{
		dtdylod = ~dtdylod & 0x1ffff;
	}
	span[0].dymax = (dsdylod > dtdylod)? dsdylod : dtdylod;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

		dsdiff = (dsdeh*3 - dsdyh*3) << 7;
		dtdiff = (dtdeh*3 - dtdyh*3) << 7;
		dwdiff = (dwdeh*3 - dwdyh*3) << 7;
		drdiff = (drdeh*3 - drdyh*3) << 7;
		dgdiff = (dgdeh*3 - dgdyh*3) << 7;
		dbdiff = (dbdeh*3 - dbdyh*3) << 7;
		dadiff = (dadeh*3 - dadyh*3) << 7;
		dzdiff = (dzdeh*3 - dzdyh*3) << 7;
	}
	else
	{
		dsdiff = dtdiff = dwdiff = drdiff = dgdiff = dbdiff = dadiff = dzdiff = 0;
	}

	if (do_offset)
	{
		dseoff = (dsdeh*3) << 7;
		dteoff = (dtdeh*3) << 7;
		dweoff = (dwdeh*3) << 7;
		dreoff = (drdeh*3) << 7;
		dgeoff = (dgdeh*3) << 7;
		dbeoff = (dbdeh*3) << 7;
		daeoff = (dadeh*3) << 7;
		dzeoff = (dzdeh*3) << 7;
	}
	else
	{
		dseoff = dteoff = dweoff = dreoff = dgeoff = dbeoff = daeoff = dzeoff = 0;
	}
#define adjust_deoff_only()		\
{							\
			span[j].s = s + dseoff;				\
			span[j].t = t + dteoff;				\
			span[j].w = w + dweoff;				\
			span[j].r = r + dreoff;				\
			span[j].g = g + dgeoff;				\
			span[j].b = b + dbeoff;				\
			span[j].a = a + daeoff;				\
			span[j].z = z + dzeoff;				\
}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;
	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;
	dzdxh = dzdx >> 8;

#define adjust_attr()		\
{							\
			span[j].s = (s + dsdiff - (xfrac * dsdxh)) & ~0x1f;				\
			span[j].t = (t + dtdiff - (xfrac * dtdxh)) & ~0x1f;				\
			span[j].w = (w + dwdiff - (xfrac * dwdxh)) & ~0x1f;				\
			span[j].r = r + drdiff - (xfrac * drdxh);				\
			span[j].g = g + dgdiff - (xfrac * dgdxh);				\
			span[j].b = b + dbdiff - (xfrac * dbdxh);				\
			span[j].a = a + dadiff - (xfrac * dadxh);				\
			span[j].z = z + dzdiff - (xfrac * dzdxh);				\
}

#define adjust_diffonly()		\
{							\
			span[j].s = s + dsdiff;				\
			span[j].t = t + dtdiff;				\
			span[j].w = w + dwdiff;				\
			span[j].r = r + drdiff;				\
			span[j].g = g + dgdiff;				\
			span[j].b = b + dbdiff;				\
			span[j].a = a + dadiff;				\
			span[j].z = z + dzdiff;				\
}

#define addvalues() {	\
			s += dsde;	\
			t += dtde;	\
			w += dwde; \
			r += drde; \
			g += dgde; \
			b += dbde; \
			a += dade; \
			z += dzde; \
}

#define justassign()		\
{							\
			span[j].s = s & ~0x1f;				\
			span[j].t = t & ~0x1f;				\
			span[j].w = w & ~0x1f;				\
			span[j].r = r;				\
			span[j].g = g;				\
			span[j].b = b;				\
			span[j].a = a;				\
			span[j].z = z;				\
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:	render_spans_16(machine, yh>>2, yl>>2, tilenum, shade, texture, zbuffer, flip); break;
		case PIXEL_SIZE_32BIT:	render_spans_32(machine, yh>>2, yl>>2, tilenum, shade, texture, zbuffer, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
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
		return rdram[((address & 0xffffff) / 4)];
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
	triangle(machine, w1, w2, 0, 0, 0);
}

static RDP_COMMAND( rdp_tri_noshade_z )
{
	triangle(machine, w1, w2, 0, 0, 1);
}

static RDP_COMMAND( rdp_tri_tex )
{
	triangle(machine, w1, w2, 0, 1, 0);
}

static RDP_COMMAND( rdp_tri_tex_z )
{
	triangle(machine, w1, w2, 0, 1, 1);
}

static RDP_COMMAND( rdp_tri_shade )
{
	triangle(machine, w1, w2, 1, 0, 0);
}

static RDP_COMMAND( rdp_tri_shade_z )
{
	triangle(machine, w1, w2, 1, 0, 1);
}

static RDP_COMMAND( rdp_tri_texshade )
{
	triangle(machine, w1, w2, 1, 1, 0);
}

static RDP_COMMAND( rdp_tri_texshade_z )
{
	triangle(machine, w1, w2, 1, 1, 1);
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
	rect.flip		= 0;

	switch (fb_size)
	{
		case PIXEL_SIZE_16BIT:		texture_rectangle_16bit(machine, &rect); break;
		case PIXEL_SIZE_32BIT:		texture_rectangle_32bit(machine, &rect); break;
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
	rect.flip		= 1;

	switch (fb_size)
	{
		case PIXEL_SIZE_16BIT:		texture_rectangle_16bit(machine, &rect); break;
		case PIXEL_SIZE_32BIT:		texture_rectangle_32bit(machine, &rect); break;
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
	key_scale.b = w2 & 0xff;
	key_scale.g = (w2 >> 16) & 0xff;
}

static RDP_COMMAND( rdp_set_key_r )
{
	key_scale.r = w2 & 0xff;
}

static RDP_COMMAND( rdp_set_convert )
{
	k0 = (w1 >> 13) & 0xff;
	k0 = ((w1 >> 21) & 1) ? (-(0x100 - k0)) : k0;
	k1 = (w1 >> 4) & 0xff;
	k1 = ((w1 >> 12) & 1) ?	(-(0x100 - k1)) : k1;
	k2 = ((w1 & 7) << 5) | ((w2 >> 27) & 0x1f);
	k2 = (w1 & 0xf) ? (-(0x100 - k2)) : k2;
	k3 = (w2 >> 18) & 0xff;
	k3 = ((w2 >> 26) & 1) ? (-(0x100 - k3)) : k3;
	k4 = (w2 >> 9) & 0xff;
	k4 = ((w2 >> 17) & 1) ? (-(0x100 - k4)) : k4;
	k5 = w2 & 0xff;
	k5 = ((w2 >> 8) & 1) ? (-(0x100 - k5)) : k5;
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
	primitive_z = (UINT16)(w2 >> 16) & 0x7fff;
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
}

static RDP_COMMAND( rdp_load_tlut )
{
	int i;
	int tl, th, sl, sh;
	int tilenum = (w2 >> 24) & 7;

	sl = tile[tilenum].sl = ((w1 >> 12) & 0xfff);
	tl = tile[tilenum].tl =  w1 & 0xfff;
	sh = tile[tilenum].sh = ((w2 >> 12) & 0xfff);
	th = tile[tilenum].th = w2 & 0xfff;

	switch (ti_size)
	{
		case PIXEL_SIZE_16BIT:
		{
			//UINT16 *src = (UINT16*)&rdram[(ti_address + (tl >> 2) * (ti_width << 1) + (sl >> 1)) >> 2];
			UINT16 *src = (UINT16*)rdram;
			UINT32 srcstart = (ti_address + (tl >> 2) * (ti_width << 1) + (sl >> 1)) >> 1;
			UINT16 *dst = (UINT16*)&TMEM[tile[tilenum].tmem];
			int count = ((sh >> 2) - (sl >> 2)) + 1;

			for (i = 0; i < count; i++)
			{
				if((i*4) < 0x400)
				{
					dst[i*4] = src[(srcstart + i) ^ WORD_ADDR_XOR];
					dst[i*4+1] = 0;
					dst[i*4+2] = 0;
					dst[i*4+3] = 0;
				}
				//tlut[i] = src[i];
			}
			break;
		}
		/*
		case PIXEL_SIZE_16BIT:
		{
			UINT16 *src = (UINT16*)&rdram[(ti_address + (tl >> 2) * (ti_width << 1) + (sl >> 1)) >> 2];

			for (i = (sl >> 2); i <= (sh >> 2); i++)
			{
				tlut[i] = src[i];
			}
			break;
		}*/
		default:	stricterror("RDP: load_tlut: size = %d\n", ti_size);
	}

	if (TMEM[0x1000] != 25 || TMEM[0x1001] != 25)
	{
		stricterror("rdp_load_tlut: TMEM has been written out-of-bounds");
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
	UINT32 tb;
	UINT16 *ram16 = (UINT16*)rdram;
	UINT32 ti_address2 = ti_address - ((ti_address & 3) ? 4 : 0);
	int ti_width2 = ti_width;
	int slindwords = 0;

	tile[tilenum].sl = sl = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl = tl = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh = sh = ((w2 >> 12) & 0xfff);
	dxt	= ((w2 >>  0) & 0xfff);

	if (sh < sl)
	{
		//fatalerror( "load_block: sh < sl" );
	}

	width = (sh - sl) + 1;

	if (width > 2048) // Hack for Magical Tetris Challenge
	{
		width = 2048;
	}

	switch (ti_size)
	{
		case PIXEL_SIZE_4BIT:	width >>= 1; break;
		case PIXEL_SIZE_8BIT:	break;
		case PIXEL_SIZE_16BIT:	width <<= 1; break;
		case PIXEL_SIZE_32BIT:	width <<= 2; break;
	}

	if ((ti_address & 3) && (ti_address & 0xffffff00) != 0xf8a00)
	{
		stricterror( "load block: unaligned ti_address 0x%x",ti_address ); // Rat Attack, Frogger 2 prototype
	}

	src = (UINT32*)&ram16[ti_address2 >> 1];
	tc = (UINT32*)TMEM;
	tb = tile[tilenum].tmem >> 2;

	if ((tb + (width >> 2)) > 0x400)
	{
		width = 0x1000 - tb*4; // Hack for Magical Tetris Challenge
	}

	if (ti_width2 < 0)
	{
		fatalerror( "load_block: negative ti_width2" );
	}

	switch (ti_size)
	{
		case PIXEL_SIZE_4BIT:	ti_width2 >>= 1; break;
		case PIXEL_SIZE_8BIT:	break;
		case PIXEL_SIZE_16BIT:	ti_width2 <<= 1; break;
		case PIXEL_SIZE_32BIT:	ti_width2 <<= 2; break;
	}

	slindwords = sl;
	switch (ti_size) // Needed by Vigilante 8
	{
		case PIXEL_SIZE_4BIT:	slindwords >>= 3; break;
		case PIXEL_SIZE_8BIT:	slindwords >>= 2;break;
		case PIXEL_SIZE_16BIT:	slindwords >>= 1; break;
		case PIXEL_SIZE_32BIT:	break;
	}

	if (width & 7)	// Sigh... another Rat Attack-specific thing.
	{
		width = (width & (~7)) + 8;
	}

	if (dxt != 0)
	{
		int j= 0;
		int t = 0;
		int oldt = 0;
		int ptr;
		int xorval = (fb_size == PIXEL_SIZE_16BIT && ti_size == PIXEL_SIZE_32BIT ) ? 2 : 1; // Wave Race-specific
		UINT32 srcstart = ((tl * ti_width2) >> 2) + slindwords;
		src = &src[srcstart];
		for (i = 0; i < (width >> 2); i += 2)
		{
			oldt = t;
			t = ((j >> 11) & 1) ? xorval : 0;
			if (t > oldt)
			{
				i += ((tile[tilenum].line >> 3) << 1);
			}
			ptr = tb + i;
			tc[ptr & 0x3ff] = src[i ^ t];
			tc[(ptr + 1) & 0x3ff] = src[(i + 1) ^ t];
			j += dxt;
		}
		tile[tilenum].th = tl + (j >> 11);
	}
	else // Needed by Pilotwings 64 intro, Top Gear Rally intro
	{
		UINT32 srcstart = ((tl * ti_width2) >> 2) + slindwords;
		memcpy(&tc[tb],&src[srcstart],width);
		tile[tilenum].th = tl;
	}
}

static RDP_COMMAND( rdp_load_tile )
{
	int i, j;
	UINT16 sl, sh, tl, th;
	int width, height;
	int tilenum = (w2 >> 24) & 0x7;
	int line = tile[tilenum].line; // Per Ziggy
	int toppad;

	if (!line)
	{
		return; // Needed by Wipeout 64
	}

	if ((ti_format != tile[tilenum].format || ti_size != tile[tilenum].size))
	{
		fatalerror("load_tile: format conversion required!\n %d %d %d %d", ti_format, tile[tilenum].format, ti_size, tile[tilenum].size);
	}

	tile[tilenum].sl = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh = ((w2 >> 12) & 0xfff);
	tile[tilenum].th = ((w2 >>  0) & 0xfff);
	sl = tile[tilenum].sl / 4;
	tl = tile[tilenum].tl / 4;
	sh = tile[tilenum].sh / 4;
	th = tile[tilenum].th / 4;

	width = (sh - sl) + 1;
	height = (th - tl) + 1;

	if (ti_size < 3)
	{
		toppad = (width * ti_size) & 0x7;
	}
	else
	{
		toppad = (width << 2) & 0x7;
	}
	toppad = 0; // Currently disabled

	switch (ti_size)
	{
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *src = (UINT8*)rdram;
			UINT8 *tc = (UINT8*)TMEM;
			int tb = tile[tilenum].tmem;

			if (tb + (width * height) > 4096)
			{
				height = (4096 - tb) / line; // Per Ziggy
			}

			for (j=0; j < height; j++)
			{
				int tline = tb + (tile[tilenum].line * j);
				int s = ((j + tl) * ti_width) + sl;
#define BYTE_XOR_DWORD_SWAP 7
				int xorval8 = ((j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR); // Per Ziggy

				for (i=0; i < width; i++)
				{
					tc[(tline + i) ^ xorval8] = src[(ti_address + s + i) ^ BYTE_ADDR_XOR];
				}
				//for (int k=0; k < (topad); k++) // Padding is possibly necessary, not yet known.
				//{
				//    tc[((tline+i+k) ^ xorval8] = src[(ti_address + s + i) ^ BYTE_ADDR_XOR];
				//}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			UINT16 *src = (UINT16*)rdram;
			UINT32 ti_addr16 = ti_address >> 1;
			UINT16 *tc = (UINT16*)TMEM;
			int tb = (tile[tilenum].tmem / 2);
			int taddr;

			if ((tb + (width * height)) > 2048)
			{
				height = (2048 - tb) / (line / 2); // Per Ziggy
			}

			for (j = 0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line / 2) * j);
				int s = 0;
				int xorval16 = 0;
				if (tile[tilenum].format == 1) // Needed by Ogre Battle 64
				{
					tline = tb + (tile[tilenum].line * j);
				}
				s = ((j + tl) * ti_width) + sl;
#define WORD_XOR_DWORD_SWAP 3
				xorval16 = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;

				for (i = 0; i < width; i++)
				{
					taddr = (tline+i) ^ xorval16;
					if (taddr < 2048) // Needed by World Driver Championship
					{
						tc[taddr] = src[(ti_addr16 + s + i) ^ WORD_ADDR_XOR];
					}
				}
				//for (int k=0; k < (topad>>1); k++) // Padding is possibly necessary, not yet known.
				//{
				//    tc[((tline+i+k) ^ xorval16] = src[(ti_addr16+s+i) ^ WORD_ADDR_XOR];
				//}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			UINT32 *src = (UINT32*)&rdram[ti_address / 4];
			UINT32 *tc = (UINT32*)TMEM;
			int tb = (tile[tilenum].tmem / 4);
			int xorval32 = ((fb_size == PIXEL_SIZE_16BIT) ? 2 : 1);

			if (tb + (width * height) > 1024)
			{
				height = (1024 - tb) / (line/4); // Per Ziggy
			}

			for (j=0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line / 2) * j);
				int s = ((j + tl) * ti_width) + sl;
				int xorval32cur = (j & 1) ? xorval32 : 0;
				for (i=0; i < width; i++)
				{
					tc[(tline + i) ^ xorval32cur] = src[s + i];
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
	tile[tilenum].palette	= (w2 >> 20) & 0xf;
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
		case PIXEL_SIZE_16BIT:		fill_rectangle_16bit(machine, &rect); break;
		case PIXEL_SIZE_32BIT:		fill_rectangle_32bit(machine, &rect); break;
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
	min_level = (w1 >> 8) & 0x1f;
	primitive_lod_frac = (w1 & 0xff);
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
	if (fb_format && fb_format != 2) // Jet Force Gemini sets the format to 4, Intensity.  Protection?
	{
		if (fb_size == 1)
		{
			fb_format = 2;
		}
		else
		{
			fb_format = 0;
		}
	}

	if (fb_format != 0)
	{
		fb_format = 0;
	}
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
		rdp_cmd_data[rdp_cmd_ptr++] = READ_RDP_DATA((dp_current & 0x1fffffff) + i);
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

	dp_start = dp_current = dp_end;
}

INLINE int alpha_compare(running_machine *machine, UINT8 comb_alpha)
{
	if (other_modes.alpha_compare_en)
	{
		if (other_modes.dither_alpha_en)
		{
			if (comb_alpha < (mame_rand(machine) & 0xff))
			{
				return 0;
			}
		}
		else
		{
			if (comb_alpha < blend_color.a)
			{
				return 0;
			}
		}
		return 1;
	}
	return 1;
}

INLINE UINT8 alpha_cvg_get(UINT8 comb_alpha)
{
	UINT32 temp = comb_alpha;
	UINT32 temp2 = curpixel_cvg;
	UINT32 temp3 = 0;

	if (other_modes.cvg_times_alpha > 1 || other_modes.alpha_cvg_select > 1)
	{
		fatalerror( "alpha_cvg_get failed" );
	}

	if (other_modes.cvg_times_alpha)
	{
		temp3 = (temp * temp2) + 4;
		curpixel_cvg = temp3 >> 8;
	}

	if (other_modes.alpha_cvg_select)
	{
		temp = (other_modes.cvg_times_alpha) ? (temp3 >> 3) : (temp2 << 5);
	}

	if (temp > 0xff)
	{
		temp = 0xff;
	}

	return (UINT8)temp;
}

INLINE UINT8 COMBINER_EQUATION(UINT8 A, UINT8 B, UINT8 C, UINT8 D)
{
	INT32 color = (((A-B)* C) + (D << 8) + 0x80);
	color >>= 8;
	if (color > 255)
	{
		color = 255;
	}
	if (color < 0)
	{
		color = 0;
	}
	return (UINT8)color;
}

INLINE void BLENDER_EQUATION(INT32* r, INT32* g, INT32* b, int cycle, int bsel_special)
{
	UINT8 blend1a, blend2a;
	UINT32 sum = 0;
	blend1a = *blender1b_a[cycle];
	blend2a = *blender2b_a[cycle];
	if (bsel_special)
	{
		blend1a &= 0xe0;
	}

	sum = (((blend1a >> 5) + (blend2a >> 5) + 1) & 0xf) << 5;

	*r = (((int)(*blender1a_r[cycle]) * (int)(blend1a))) +
		(((int)(*blender2a_r[cycle]) * (int)(blend2a)));
	*r += (bsel_special) ? (((int)(*blender2a_r[cycle])) << 5) : (((int)(*blender2a_r[cycle])) << 3);

	*g = (((int)(*blender1a_g[cycle]) * (int)(blend1a))) +
		(((int)(*blender2a_g[cycle]) * (int)(blend2a)));
	*g += (bsel_special) ? ((int)((*blender2a_g[cycle])) << 5) : (((int)(*blender2a_g[cycle])) << 3);

	*b = (((int)(*blender1a_b[cycle]) * (int)(blend1a))) +
		(((int)(*blender2a_b[cycle]) * (int)(blend2a)));
	*b += (bsel_special) ? (((int)(*blender2a_b[cycle])) << 5) : (((int)(*blender2a_b[cycle])) << 3);

	if (other_modes.force_blend)
	{
		*r >>= 8;
		*g >>= 8;
		*b >>= 8;
	}
	else
	{
		if (sum)
		{
			*r /= sum;
			*g /= sum;
			*b /= sum;
		}
		else
		{
			*r = *g = *b = 0xff;
		}
	}


	if (*r > 255) *r = 255;
	if (*g > 255) *g = 255;
	if (*b > 255) *b = 255;
}

INLINE UINT32 addrightcvg(UINT32 x, UINT32 k)
{
//#undef FULL_SUBPIXELS
#define FULL_SUBPIXELS
	UINT32 coveredsubpixels=((x >> 14) & 3);
	if (!(x & 0xffff))
	{
		return 0;
	}
#ifdef FULL_SUBPIXELS
	if (!coveredsubpixels)
	{
		return 0;
	}
	if (!(k & 1))
	{
		return (coveredsubpixels<3) ? 1 : 2;
	}
	else
	{
		return (coveredsubpixels<2) ? 0 : 1;
	}
#endif
	if (!(k & 1))
	{
		return (coveredsubpixels < 2) ? 1 : 2;
	}
	else
	{
		if (coveredsubpixels<1)
		{
			return 0;
		}
		else if (coveredsubpixels<3)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

INLINE UINT32 addleftcvg(UINT32 x, UINT32 k)
{
	UINT32 coveredsubpixels = 3 - ((x >> 14) & 3);
	if (!(x & 0xffff))
	{
		return 2;
	}
#ifdef FULL_SUBPIXELS
	if (!coveredsubpixels)
	{
		return 0;
	}
	if (!(k & 1))
	{
		return (coveredsubpixels<2) ? 0 : 1;
	}
	else
	{
		return (coveredsubpixels<3) ? 1 : 2;
	}
#endif
	if (k & 1)
	{
		return (coveredsubpixels<2) ? 1 : 2;
	}
	else
	{
		if (coveredsubpixels < 1)
		{
			return 0;
		}
		else if (coveredsubpixels < 3)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

INLINE UINT32 FBWRITE_16(UINT16 *fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b)
{
#undef CVG_DRAW
	UINT16 finalcolor;
	UINT32 memory_cvg;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
#ifdef CVG_DRAW
	int covdraw;
	if (curpixel_cvg == 8)
	{
		covdraw=255;
	}
	else
	{
		covdraw = curpixel_cvg << 5;
	}
	r=covdraw; g=covdraw; b=covdraw;
#endif

	if (!other_modes.z_compare_en)
	{
		curpixel_overlap = 0;
	}
	if (other_modes.image_read_en)
	{
		memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	}
	else
	{
		memory_cvg = 8;
	}

	if (curpixel_cvg > 8)
	{
		fatalerror("FBWRITE_16: curpixel_cvg %d", curpixel_cvg);
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;

	finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	if (!curpixel_cvg)
	{
		fatalerror("cvg underflow");
	}

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

	if (other_modes.color_on_cvg && !wrapflag)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return 0;
	}

	if (!other_modes.cvg_dest)
	{
		if (!other_modes.force_blend && !curpixel_overlap)
		{
			*fb = finalcolor|((curpixel_cvg >>2)&1);
			*hb = (curpixel_cvg & 3);
		}
		else
		{
			*fb = finalcolor|((clampcvg>>2)&1);
			*hb = (clampcvg&3);
		}
	}
	else if (other_modes.cvg_dest == 2)
	{
		*fb = finalcolor|1;
		*hb = 3;
	}
	else if (other_modes.cvg_dest == 3)
	{
		*fb = finalcolor|((memory_cvg >> 2) & 1);
		*hb = (memory_cvg & 3);
	}
	else if (other_modes.cvg_dest == 1)
	{
		*fb = finalcolor|((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
	}
	return 1;
}

INLINE UINT32 FBWRITE_32(UINT32 *fb, UINT32 r, UINT32 g, UINT32 b)
{
	UINT32 finalcolor=(r << 24) | (g << 16) | (b << 8);
	UINT32 memory_alphachannel = *fb & 0xff;
	UINT32 memory_cvg;
	UINT32 newcvg;
	UINT32 wrapflag;
	UINT32 clampcvg;
	if (other_modes.image_read_en)
	{
		memory_cvg = ((*fb >>5) & 7) + 1;
	}
	else
	{
		memory_cvg = 8;
	}

	newcvg = curpixel_cvg + memory_cvg;
	wrapflag = (newcvg > 8) ? 1 : 0;
	clampcvg = (newcvg > 8) ? 8 : newcvg;
	newcvg = (wrapflag)? (newcvg - 8) : newcvg;

	curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clampcvg--;

	if (other_modes.color_on_cvg && !wrapflag)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}

	if (!other_modes.cvg_dest)
	{
		if (!other_modes.force_blend && !curpixel_overlap)
		{
			*fb = finalcolor|(curpixel_cvg << 5);
		}
		else
		{
			*fb = finalcolor|(clampcvg << 5);
		}
	}
	else if (other_modes.cvg_dest==2)
	{
		*fb = finalcolor | 0xE0;
	}
	else if (other_modes.cvg_dest == 3)
	{
		*fb = finalcolor | memory_alphachannel;
	}
	else if (other_modes.cvg_dest == 1)
	{
		*fb = finalcolor | (newcvg << 5);
	}
	return 1;
}

