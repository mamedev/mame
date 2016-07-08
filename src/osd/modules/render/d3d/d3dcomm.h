// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  d3dcomm.h - Common Win32 Direct3D structures
//
//============================================================

#ifndef __WIN_D3DCOMM__
#define __WIN_D3DCOMM__

//============================================================
//  CONSTANTS
//============================================================

#define MAX_BLOOM_COUNT 15 // shader model 3.0 support up to 16 samplers, but we need the last for the original texture
#define HALF_BLOOM_COUNT 8

//============================================================
//  FORWARD DECLARATIONS
//============================================================

class texture_info;
class renderer_d3d9;
struct d3d_base;

//============================================================
//  TYPE DEFINITIONS
//============================================================

class vec2f
{
public:
	vec2f()
	{
		memset(&c, 0, sizeof(float) * 2);
	}
	vec2f(float x, float y)
	{
		c.x = x;
		c.y = y;
	}

	vec2f operator+(const vec2f& a) const
	{
		return vec2f(c.x + a.c.x, c.y + a.c.y);
	}

	vec2f operator-(const vec2f& a) const
	{
		return vec2f(c.x - a.c.x, c.y - a.c.y);
	}

	struct
	{
		float x, y;
	} c;
};

class d3d_texture_manager
{
public:
	d3d_texture_manager(): m_renderer(nullptr), m_dynamic_supported(0), m_stretch_supported(0), m_yuv_format(), m_texture_caps(0), m_texture_max_aspect(0), m_texture_max_width(0), m_texture_max_height(0), m_default_texture(nullptr)
	{ }

	d3d_texture_manager(renderer_d3d9 *d3d);

	void                    update_textures();

	void                    create_resources();
	void                    delete_resources();

	texture_info *          find_texinfo(const render_texinfo *texture, UINT32 flags);
	UINT32                  texture_compute_hash(const render_texinfo *texture, UINT32 flags);

	bool                    is_dynamic_supported() const { return (bool)m_dynamic_supported; }
	void                    set_dynamic_supported(bool dynamic_supported) { m_dynamic_supported = dynamic_supported; }
	bool                    is_stretch_supported() const { return (bool)m_stretch_supported; }
	D3DFORMAT               get_yuv_format() const { return m_yuv_format; }

	DWORD                   get_texture_caps() const { return m_texture_caps; }
	DWORD                   get_max_texture_aspect() const { return m_texture_max_aspect; }
	DWORD                   get_max_texture_width() const { return m_texture_max_width; }
	DWORD                   get_max_texture_height() const { return m_texture_max_height; }

	texture_info *          get_default_texture() const { return m_default_texture; }

	renderer_d3d9 *         get_d3d() const { return m_renderer; }

	std::vector<std::unique_ptr<texture_info>> m_texture_list;  // list of active textures

private:
	renderer_d3d9 *         m_renderer;
	int                     m_dynamic_supported;        // are dynamic textures supported?
	int                     m_stretch_supported;        // is StretchRect with point filtering supported?
	D3DFORMAT               m_yuv_format;               // format to use for YUV textures

	DWORD                   m_texture_caps;             // textureCaps field
	DWORD                   m_texture_max_aspect;       // texture maximum aspect ratio
	DWORD                   m_texture_max_width;        // texture maximum width
	DWORD                   m_texture_max_height;       // texture maximum height

	bitmap_rgb32            m_default_bitmap;           // experimental: default bitmap
	texture_info *          m_default_texture;          // experimental: default texture
};


/* texture_info holds information about a texture */
class texture_info
{
public:
	texture_info(d3d_texture_manager *manager, const render_texinfo *texsource, int prescale, UINT32 flags);
	~texture_info();

	render_texinfo &        get_texinfo() { return m_texinfo; }

	int                     get_width() const { return m_rawdims.c.x; }
	int                     get_height() const { return m_rawdims.c.y; }
	int                     get_xscale() const { return m_xprescale; }
	int                     get_yscale() const { return m_yprescale; }

	UINT32                  get_flags() const { return m_flags; }

	void                    set_data(const render_texinfo *texsource, UINT32 flags);

	UINT32                  get_hash() const { return m_hash; }

	void                    increment_frame_count() { m_cur_frame++; }
	void                    mask_frame_count(int mask) { m_cur_frame %= mask; }

	int                     get_cur_frame() const { return m_cur_frame; }

	IDirect3DTexture9 *     get_tex() const { return m_d3dtex; }
	IDirect3DSurface9 *     get_surface() const { return m_d3dsurface; }
	IDirect3DTexture9 *     get_finaltex() const { return m_d3dfinaltex; }

	vec2f &                 get_uvstart() { return m_start; }
	vec2f &                 get_uvstop() { return m_stop; }
	vec2f &                 get_rawdims() { return m_rawdims; }

private:
	void prescale();
	void compute_size(int texwidth, int texheight);
	void compute_size_subroutine(int texwidth, int texheight, int* p_width, int* p_height);

	d3d_texture_manager *   m_texture_manager;          // texture manager pointer

	renderer_d3d9 *         m_renderer;                 // renderer pointer

	UINT32                  m_hash;                     // hash value for the texture
	UINT32                  m_flags;                    // rendering flags
	render_texinfo          m_texinfo;                  // copy of the texture info
	vec2f                   m_start;                    // beggining UV coordinates
	vec2f                   m_stop;                     // ending UV coordinates
	vec2f                   m_rawdims;                  // raw dims of the texture
	int                     m_type;                     // what type of texture are we?
	int                     m_xborderpix, m_yborderpix; // number of border pixels on X/Y
	int                     m_xprescale, m_yprescale;   // X/Y prescale factor
	int                     m_cur_frame;                // what is our current frame?
	IDirect3DTexture9 *     m_d3dtex;                   // Direct3D texture pointer
	IDirect3DSurface9 *     m_d3dsurface;               // Direct3D offscreen plain surface pointer
	IDirect3DTexture9 *     m_d3dfinaltex;              // Direct3D final (post-scaled) texture
};

/* poly_info holds information about a single polygon/d3d primitive */
class poly_info
{
public:
	void init(D3DPRIMITIVETYPE type, UINT32 count, UINT32 numverts,
				UINT32 flags, texture_info *texture, UINT32 modmode,
				float prim_width, float prim_height)
	{
		m_type = type;
		m_count = count;
		m_numverts = numverts;
		m_flags = flags;
		m_texture = texture;
		m_modmode = modmode;
		m_prim_width = prim_width;
		m_prim_height = prim_height;
	}

	D3DPRIMITIVETYPE        type() const { return m_type; }
	UINT32                  count() const { return m_count; }
	UINT32                  numverts() const { return m_numverts; }
	UINT32                  flags() const { return m_flags; }

	texture_info *          texture() const { return m_texture; }
	DWORD                   modmode() const { return m_modmode; }

	float                   prim_width() const { return m_prim_width; }
	float                   prim_height() const { return m_prim_height; }

private:
	D3DPRIMITIVETYPE        m_type;         // type of primitive
	UINT32                  m_count;        // total number of primitives
	UINT32                  m_numverts;     // total number of vertices
	UINT32                  m_flags;        // rendering flags

	texture_info *          m_texture;      // pointer to texture info
	DWORD                   m_modmode;      // texture modulation mode

	float                   m_prim_width;   // used by quads
	float                   m_prim_height;  // used by quads
};

/* vertex describes a single vertex */
struct vertex
{
	float       x, y, z;                    // X,Y,Z coordinates
	float       rhw;                        // RHW when no HLSL, padding when HLSL
	D3DCOLOR    color;                      // diffuse color
	float       u0, v0;                     // texture stage 0 coordinates
	float       u1, v1;                     // additional info for vector data
};


/* cache_target is a simple linked list containing only a renderable target and texture, used for phosphor effects */
class cache_target
{
public:
	// construction/destruction
	cache_target(): target(nullptr), texture(nullptr), target_width(0), target_height(0), width(0), height(0), screen_index(0)
	{ }

	~cache_target();

	bool init(renderer_d3d9 *d3d, int source_width, int source_height, int target_width, int target_height, int screen_index);

	IDirect3DSurface9 *target;
	IDirect3DTexture9 *texture;

	int target_width;
	int target_height;

	int width;
	int height;

	int screen_index;
};

/* d3d_render_target is the information about a Direct3D render target chain */
class d3d_render_target
{
public:
	// construction/destruction
	d3d_render_target(): target_width(0), target_height(0), width(0), height(0), screen_index(0), page_index(0), bloom_count(0)
	{
		for (int index = 0; index < MAX_BLOOM_COUNT; index++)
		{
			bloom_texture[index] = nullptr;
			bloom_surface[index] = nullptr;
		}

		for (int index = 0; index < 2; index++)
		{
			source_texture[index] = nullptr;
			source_surface[index] = nullptr;
			target_texture[index] = nullptr;
			target_surface[index] = nullptr;
		}
	}

	~d3d_render_target();

	bool init(renderer_d3d9 *d3d, int source_width, int source_height, int target_width, int target_height, int screen_index, int page_index);
	int next_index(int index) { return ++index > 1 ? 0 : index; }

	// real target dimension
	int target_width;
	int target_height;

	// only used to identify/find the render target
	int width;
	int height;

	int screen_index;
	int page_index;

	IDirect3DSurface9 *target_surface[2];
	IDirect3DTexture9 *target_texture[2];
	IDirect3DSurface9 *source_surface[2];
	IDirect3DTexture9 *source_texture[2];

	IDirect3DSurface9 *bloom_surface[MAX_BLOOM_COUNT];
	IDirect3DTexture9 *bloom_texture[MAX_BLOOM_COUNT];

	float bloom_dims[MAX_BLOOM_COUNT][2];

	int bloom_count;
};

#endif
