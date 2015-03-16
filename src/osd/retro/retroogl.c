#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static struct retro_hw_render_callback hw_render;
#warning ogl experimental
#include "glsym/glsym.h"

int vloc,cloc,tloc,utloc;

//static GLuint tex;
static GLuint prog;
static GLuint vbo;

GLfloat glverts[2*4],glcols[4*4],glvtexs[2*4];
GLfloat UseTexture=0.0f;

static const GLfloat vertex_data[] = {
//VERTEX
	-1.0, -1.0,
	1.0, -1.0,
	-1.0,  1.0,
	1.0,  1.0,
//COLOR
	1.0, 0.0, 0.0, 1.0,
	1.0, 0.0, 1.0, 1.0,
	0.0, 1.0, 1.0, 1.0,
	1.0, 1.0, 0.0, 1.0,
//TEXCOORD
	0, 0,
	1, 0,
	0, 1,
	1, 1,
};

static const char *vertex_shader[] = {
	"attribute vec2 aVertex;\n",
	"attribute vec2 aTexCoord;\n",
	"attribute vec4 aColor;\n",
	"varying vec4 color;\n",
	"varying vec2 vTex;\n",
	"void main() {\n",
	"  gl_Position = vec4(aVertex, 0.0, 1.0);\n",
	"  vTex =  aTexCoord ; color = aColor;\n",
	"}",
};

static const char *fragment_shader[] = {
	"#ifdef GL_ES\n",
	"precision mediump float;\n",
	"#endif\n",
	"varying vec2 vTex;\n",
	"varying vec4 color;\n",
	"uniform float uUseTexture;\n",
	"uniform sampler2D sTex0;\n",
	"void main() {",
	"   vec4 texColor = texture2D(sTex0, vTex) * color * uUseTexture ;\n",
	"   vec4 vertColor = color * (1.0 - uUseTexture);\n",
	"   gl_FragColor =  texColor + vertColor;\n",
	"}",
};

#ifdef PTR64
typedef UINT64 HashT;
#else
typedef UINT32 HashT;
#endif

#define HASH_SIZE       ((1<<10)+1)
#define OVERFLOW_SIZE   (1<<10)

struct texture_info;

/* texture_info holds information about a texture */
struct texture_info
{
	HashT               hash;               // hash value for the texture (must be >= pointer size)
	UINT32              flags;              // rendering flags
	render_texinfo      texinfo;            // copy of the texture info
	UINT32              texture;            // OpenGL texture "name"/ID
	GLenum              texTarget;          // OpenGL texture target
	UINT32              *data;                  // pixels for the texture
	bool 				data_own;
};

struct retro_info
{
	// 3D info (GL mode only)
	texture_info *  texhash[HASH_SIZE + OVERFLOW_SIZE];
	int             last_blendmode;     // previous blendmode
};

retro_info *retro;

INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	HashT h = (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
	//printf("hash %d\n", (int) h % HASH_SIZE);
	return (h >> 8) % HASH_SIZE;
}

INLINE void set_blendmode(int blendmode)
{
	// try to minimize texture state changes
	if (blendmode != retro->last_blendmode)
	{
		switch (blendmode)
		{
			case BLENDMODE_NONE:
				glDisable(GL_BLEND);
				break;
			case BLENDMODE_ALPHA:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BLENDMODE_RGB_MULTIPLY:
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			case BLENDMODE_ADD:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
		}

		retro->last_blendmode = blendmode;
	}
}

//============================================================
//  copyline_palette16
//============================================================

INLINE void copyline_palette16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*src];
	for (x = 0; x < width; x++)
	{
		int srcpix = *src++;
		for (int x2 = 0; x2 < xprescale; x2++)
			*dst++ = 0xff000000 | palette[srcpix];
	}
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*--src];
}

//============================================================
//  copyline_palettea16
//============================================================

INLINE void copyline_palettea16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = palette[*src];
	for (x = 0; x < width; x++)
	{
		int srcpix = *src++;
		for (int x2 = 0; x2 < xprescale; x2++)
			*dst++ = palette[srcpix];
	}
	if (xborderpix)
		*dst++ = palette[*--src];
}

//============================================================
//  copyline_rgb32
//============================================================

INLINE void copyline_rgb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			rgb_t srcpix = *src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			for (int x2 = 0; x2 < xprescale; x2++)
			{
				*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
			}
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = 0xff000000 | *src;
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;

			for (int x2 = 0; x2 < xprescale; x2++)
			{
				*dst++ = 0xff000000 | srcpix;
			}
		}
		if (xborderpix)
			*dst++ = 0xff000000 | *--src;
	}
}

//============================================================
//  copyline_argb32
//============================================================

INLINE void copyline_argb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			rgb_t srcpix = *src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = *src;
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = srcpix;
		}
		if (xborderpix)
			*dst++ = *--src;
	}
}

INLINE UINT32 ycc_to_rgb(UINT8 y, UINT8 cb, UINT8 cr)
{
	/* original equations:

	    C = Y - 16
	    D = Cb - 128
	    E = Cr - 128

	    R = clip(( 298 * C           + 409 * E + 128) >> 8)
	    G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
	    B = clip(( 298 * C + 516 * D           + 128) >> 8)

	    R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
	    G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
	    B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
	*/
	int r, g, b, common;

	common = 298 * y - 298 * 16;
	r = (common +                        409 * cr - 409 * 128 + 128) >> 8;
	g = (common - 100 * cb + 100 * 128 - 208 * cr + 208 * 128 + 128) >> 8;
	b = (common + 516 * cb - 516 * 128                        + 128) >> 8;

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	return rgb_t(0xff, r, g, b);
}

//============================================================
//  copyline_yuy16_to_argb
//============================================================

INLINE void copyline_yuy16_to_argb(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 2);
	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = src[0];
			UINT16 srcpix1 = src[1];
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
		}
		for (x = 0; x < width / 2; x++)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			for (int x2 = 0; x2 < xprescale/2; x2++)
			{
				*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
				*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			}
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
		}
	}

	// direct case
	else
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = src[0];
			UINT16 srcpix1 = src[1];
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
		}
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			for (int x2 = 0; x2 < xprescale/2; x2++)
			{
				*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
				*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			}
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
		}
	}
}

static texture_info *texture_create(const render_texinfo *texsource, UINT32 flags)
{
	texture_info *texture;

	// allocate a new texture
	texture = (texture_info *) malloc(sizeof(*texture));
	memset(texture, 0, sizeof(*texture));

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	texture->texTarget = GL_TEXTURE_2D;

		// get a name for this texture
		glGenTextures(1, (GLuint *)&texture->texture);

		glEnable(texture->texTarget);

		// make sure we're operating on *this* texture
		glBindTexture(texture->texTarget, texture->texture);

		// this doesn't actually upload, it just sets up the PBO's parameters
		glTexImage2D(texture->texTarget, 0, GL_RGBA8,
				texsource->width,texsource->height,
				//texture->rawwidth_create, texture->rawheight_create,
				 0,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		// TODO: video_config.filter
		if ((PRIMFLAG_GET_SCREENTEX(flags)) /*&& video_config.filter*/)
		{
			// screen textures get the user's choice of filtering
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			// non-screen textures will never be filtered
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

			// set wrapping mode appropriately
			if (texture->flags & PRIMFLAG_TEXWRAP_MASK)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			}

	texture->data = (UINT32 *) malloc(texsource->width*texsource->height* sizeof(UINT32));
	texture->data_own=TRUE;

	// add us to the texture list
	if (retro->texhash[texture->hash] == NULL)
		retro->texhash[texture->hash] = texture;
	else
	{
		int i;
		for (i = HASH_SIZE; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			if (retro->texhash[i] == NULL)
			{
				retro->texhash[i] = texture;
				break;
			}
		assert(i < HASH_SIZE + OVERFLOW_SIZE);
	}

	return texture;
}

static void texture_set_data(texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
		int y;
		UINT8 *dst;

		for (y = 0; y < texsource->height; y++)
		{

				dst = (UINT8 *)(texture->data + y * texsource->width);

				switch (PRIMFLAG_GET_TEXFORMAT(flags))
				{
					case TEXFORMAT_PALETTE16:
						copyline_palette16((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, 0, 1);
						break;

					case TEXFORMAT_PALETTEA16:
						copyline_palettea16((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, 0, 1);
						break;

					case TEXFORMAT_RGB32:
						copyline_rgb32((UINT32 *)dst, (UINT32 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, 0, 1);
						break;

					case TEXFORMAT_ARGB32:
						copyline_argb32((UINT32 *)dst, (UINT32 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, 0, 1);
						break;

					case TEXFORMAT_YUY16:
						copyline_yuy16_to_argb((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, 0, 1);
						break;

					default:
						osd_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
						break;
				}
		}

		glBindTexture(texture->texTarget, texture->texture);

		// and upload the image
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texsource->width, texsource->height,
						GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->data);

}


static int compare_texture_primitive(const texture_info *texture, const render_primitive *prim)
{
	if (texture->texinfo.base == prim->texture.base &&
		texture->texinfo.width == prim->texture.width &&
		texture->texinfo.height == prim->texture.height &&
		texture->texinfo.rowpixels == prim->texture.rowpixels &&
		texture->texinfo.palette == prim->texture.palette &&
		((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		return 1;
	else
		return 0;
}

static texture_info *texture_find(retro_info *retro, const render_primitive *prim)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;

	texture = retro->texhash[texhash];
	if (texture != NULL)
	{
		int i;
		if (compare_texture_primitive(texture, prim))
			return texture;
		for (i=HASH_SIZE; i<HASH_SIZE + OVERFLOW_SIZE; i++)
		{
			texture = retro->texhash[i];
			if (texture != NULL && compare_texture_primitive(texture, prim))
				return texture;
		}
	}
	return NULL;
}

static texture_info * texture_update(const render_primitive *prim, int shaderIdx)
{
	texture_info *texture = texture_find(retro, prim);
	int texBound = 0;

	// if we didn't find one, create a new texture
	if (texture == NULL && prim->texture.base != NULL)
	{
		texture = texture_create(&prim->texture, prim->flags);
	}
	else if (texture != NULL)
	{
		glEnable(texture->texTarget);
	}

	if (texture != NULL)
	{
		if (prim->texture.base != NULL && texture->texinfo.seqid != prim->texture.seqid)
		{
			texture->texinfo.seqid = prim->texture.seqid;

			// if we found it, but with a different seqid, copy the data
			texture_set_data(texture, &prim->texture, prim->flags);
			texBound=1;
		}

		if (!texBound) {
			glBindTexture(texture->texTarget, texture->texture);
		}

	}

	return texture;
}

static void destroy_all_textures(){

	int i;
	texture_info *texture = NULL;
	if (retro == NULL)
			return;

	glDisable(GL_TEXTURE_2D);

	i=0;
	while (i<HASH_SIZE+OVERFLOW_SIZE)
	{
		texture = retro->texhash[i];
		retro->texhash[i] = NULL;
		if (texture != NULL)
		{

			glDeleteTextures(1, (GLuint *)&texture->texture);
			if ( texture->data_own )
			{
				free(texture->data);
				texture->data=NULL;
				texture->data_own=FALSE;
			}
			free(texture);

		}
		i++;
	}
}

static void compile_program(void)
{
	prog = glCreateProgram();
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert, ARRAY_SIZE(vertex_shader), vertex_shader, 0);
	glShaderSource(frag, ARRAY_SIZE(fragment_shader), fragment_shader, 0);
	glCompileShader(vert);
	glCompileShader(frag);

	glAttachShader(prog, vert);
	glAttachShader(prog, frag);

	glLinkProgram(prog);

	glDeleteShader(vert);
	glDeleteShader(frag);
}

static void setup_vao(void)
{
	glUseProgram(prog);

	//setup_loc
	glUniform1i(glGetUniformLocation(prog, "sTex0"), 0);
	vloc = glGetAttribLocation(prog, "aVertex");
	tloc = glGetAttribLocation(prog, "aTexCoord");
	cloc = glGetAttribLocation(prog, "aColor");
	utloc= glGetUniformLocation(prog, "uUseTexture");

	//setup_vbo
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
}

static void context_reset(void)
{
	fprintf(stderr, "Context reset!\n");
	rglgen_resolve_symbols(hw_render.get_proc_address);
	compile_program();
	setup_vao();

	glDisable(GL_DEPTH_TEST);

	destroy_all_textures();
}

static void context_destroy(void)
{
	fprintf(stderr, "Context destroy!\n");

	glDeleteBuffers(1,&vbo);
	vbo = 0;
	glDeleteProgram(prog);
	prog = 0;
}

INLINE float round_nearest(float f)
{
	return floor(f + 0.5f);
}

static void do_glflush(){

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	video_cb(RETRO_HW_FRAME_BUFFER_VALID, rtwi, rthe, 0);
}

#define prep_vertex_attrib()\
		glBindBuffer(GL_ARRAY_BUFFER, vbo);\
		glVertexAttribPointer(vloc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);\
		glEnableVertexAttribArray(vloc);\
		glVertexAttribPointer(cloc, 4, GL_FLOAT, GL_FALSE, 0, (void*)(8 * sizeof(GLfloat)));\
		glEnableVertexAttribArray(cloc);\
		glVertexAttribPointer(tloc, 2, GL_FLOAT, GL_FALSE, 0,  (void*)(24 * sizeof(GLfloat)) );\
		glEnableVertexAttribArray(tloc);\
		glBindBuffer(GL_ARRAY_BUFFER, 0);\
		glUniform1f(utloc,UseTexture );

static void gl_draw_primitives(const render_primitive_list &primlst,int minwidth,int minheight){

	texture_info *texture=NULL;

	if(init3d==1){

		printf("initGL\n");
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		retro->last_blendmode = BLENDMODE_ALPHA;

		init3d=0;

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glDisable(GL_DEPTH_TEST);

		//TODO: only if machine->options().antialias()
		{
			// enable antialiasing for lines
			glEnable(GL_LINE_SMOOTH);
			// enable antialiasing for points
			glEnable(GL_POINT_SMOOTH);
			// prefer quality to speed
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		}
		//TODO: only to match machine->options().beam()
		glLineWidth(2.0);
		glPointSize(2.0);

		//TODO:texture destroy
		destroy_all_textures();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, hw_render.get_current_framebuffer());
	glViewport(0, 0, minwidth, minheight);

	for (const render_primitive *prim = primlst.first(); prim != NULL; prim = prim->next())
	{
		UseTexture=0.0;

		switch (prim->type)
		{
			case render_primitive::LINE:
				glUseProgram(prog);

				set_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags));

				if (((prim->bounds.x1 - prim->bounds.x0) == 0) && ((prim->bounds.y1 - prim->bounds.y0) == 0))
				{
					//GLPOINT

					glverts[0]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[1]=2*(prim->bounds.y0/minheight-0.5);
					glcols[0]=prim->color.r;
					glcols[1]=prim->color.g;
					glcols[2]=prim->color.b;
					glcols[3]=prim->color.a;

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glverts)/4, &glverts[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), sizeof(glcols)/4, &glcols[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					prep_vertex_attrib();

					glDrawArrays(GL_POINTS, 0, 1);

					glDisableVertexAttribArray(vloc);
					glDisableVertexAttribArray(cloc);

				}
				else
				{
					glverts[0]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[1]=2*(prim->bounds.y0/minheight-0.5);
					glverts[2]=2*(prim->bounds.x1/minwidth-0.5);
					glverts[3]=2*(prim->bounds.y1/minheight-0.5);

					glcols[0]=prim->color.r;
					glcols[1]=prim->color.g;
					glcols[2]=prim->color.b;
					glcols[3]=prim->color.a;
					glcols[4]=prim->color.r;
					glcols[5]=prim->color.g;
					glcols[6]=prim->color.b;
					glcols[7]=prim->color.a;

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glverts)/2, &glverts[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), sizeof(glcols)/2, &glcols[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					prep_vertex_attrib();

					glDrawArrays(GL_LINES, 0, 2);

					glDisableVertexAttribArray(vloc);
					glDisableVertexAttribArray(cloc);
				}
				break;

			case render_primitive::QUAD:

				glColor4f(prim->color.r, prim->color.g, prim->color.b, prim->color.a);
				set_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags));

				if (!prim->texture.base) {
					glUseProgram(prog);

					glverts[0]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[1]=2*(prim->bounds.y0/minheight-0.5);
					glverts[2]=2*(prim->bounds.x1/minwidth-0.5);
					glverts[3]=2*(prim->bounds.y0/minheight-0.5);
					glverts[6]=2*(prim->bounds.x1/minwidth-0.5);
					glverts[7]=2*(prim->bounds.y1/minheight-0.5);
					glverts[4]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[5]=2*(prim->bounds.y1/minheight-0.5);

					glcols[0]=prim->color.r;
					glcols[1]=prim->color.g;
					glcols[2]=prim->color.b;
					glcols[3]=prim->color.a;
					glcols[4]=prim->color.r;
					glcols[5]=prim->color.g;
					glcols[6]=prim->color.b;
					glcols[7]=prim->color.a;
					glcols[8]=prim->color.r;
					glcols[9]=prim->color.g;
					glcols[10]=prim->color.b;
					glcols[11]=prim->color.a;
					glcols[12]=prim->color.r;
					glcols[13]=prim->color.g;
					glcols[14]=prim->color.b;
					glcols[15]=prim->color.a;

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glverts), &glverts[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), sizeof(glcols), &glcols[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					prep_vertex_attrib();

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					glDisableVertexAttribArray(vloc);
					glDisableVertexAttribArray(cloc);
				} else {
					glUseProgram(prog);

					texture = texture_update( prim, 0);

					glverts[0]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[1]=2*(prim->bounds.y0/minheight-0.5);
					glverts[2]=2*(prim->bounds.x1/minwidth-0.5);
					glverts[3]=2*(prim->bounds.y0/minheight-0.5);
					glverts[4]=2*(prim->bounds.x0/minwidth-0.5);
					glverts[5]=2*(prim->bounds.y1/minheight-0.5);
					glverts[6]=2*(prim->bounds.x1/minwidth-0.5);
					glverts[7]=2*(prim->bounds.y1/minheight-0.5);

					glcols[0]=prim->color.r;
					glcols[1]=prim->color.g;
					glcols[2]=prim->color.b;
					glcols[3]=prim->color.a;
					glcols[4]=prim->color.r;
					glcols[5]=prim->color.g;
					glcols[6]=prim->color.b;
					glcols[7]=prim->color.a;
					glcols[8]=prim->color.r;
					glcols[9]=prim->color.g;
					glcols[10]=prim->color.b;
					glcols[11]=prim->color.a;
					glcols[12]=prim->color.r;
					glcols[13]=prim->color.g;
					glcols[14]=prim->color.b;
					glcols[15]=prim->color.a;

					glvtexs[0] = prim->texcoords.tl.u;
					glvtexs[1] = prim->texcoords.tl.v;
					glvtexs[2] = prim->texcoords.tr.u;
					glvtexs[3] = prim->texcoords.tr.v;
					glvtexs[4] = prim->texcoords.bl.u;
					glvtexs[5] = prim->texcoords.bl.v;
					glvtexs[6] = prim->texcoords.br.u;
					glvtexs[7] = prim->texcoords.br.v;

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glverts), &glverts[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), sizeof(glcols), &glcols[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), sizeof(glvtexs), &glvtexs[0]);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					UseTexture=1.0;
					prep_vertex_attrib();

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					glDisableVertexAttribArray(vloc);
					glDisableVertexAttribArray(cloc);
					glDisableVertexAttribArray(tloc);

					if ( texture )
						glDisable(texture->texTarget);
				}
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
	}
}
