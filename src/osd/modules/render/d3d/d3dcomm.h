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
//  FORWARD DECLARATIONS
//============================================================

namespace d3d
{
class texture_info;
class renderer;

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

	vec2f operator+(const vec2f& a)
	{
		return vec2f(c.x + a.c.x, c.y + a.c.y);
	}

	vec2f operator-(const vec2f& a)
	{
		return vec2f(c.x - a.c.x, c.y - a.c.y);
	}

	struct
	{
		float x, y;
	} c;
};

class texture_manager
{
public:
	texture_manager() { }
	texture_manager(renderer *d3d);
	~texture_manager();

	void                    update_textures();

	void                    create_resources();
	void                    delete_resources();

	texture_info *          find_texinfo(const render_texinfo *texture, UINT32 flags);
	UINT32                  texture_compute_hash(const render_texinfo *texture, UINT32 flags);

	texture_info *          get_texlist() { return m_texlist; }
	void                    set_texlist(texture_info *texlist) { m_texlist = texlist; }
	bool                    is_dynamic_supported() { return (bool)m_dynamic_supported; }
	void                    set_dynamic_supported(bool dynamic_supported) { m_dynamic_supported = dynamic_supported; }
	bool                    is_stretch_supported() { return (bool)m_stretch_supported; }
	D3DFORMAT               get_yuv_format() { return m_yuv_format; }

	DWORD                   get_texture_caps() { return m_texture_caps; }
	DWORD                   get_max_texture_aspect() { return m_texture_max_aspect; }
	DWORD                   get_max_texture_width() { return m_texture_max_width; }
	DWORD                   get_max_texture_height() { return m_texture_max_height; }

	texture_info *          get_default_texture() { return m_default_texture; }
	texture_info *          get_vector_texture() { return m_vector_texture; }

	renderer *              get_d3d() { return m_renderer; }

private:
	renderer *              m_renderer;

	texture_info *          m_texlist;                  // list of active textures
	int                     m_dynamic_supported;        // are dynamic textures supported?
	int                     m_stretch_supported;        // is StretchRect with point filtering supported?
	D3DFORMAT               m_yuv_format;               // format to use for YUV textures

	DWORD                   m_texture_caps;             // textureCaps field
	DWORD                   m_texture_max_aspect;       // texture maximum aspect ratio
	DWORD                   m_texture_max_width;        // texture maximum width
	DWORD                   m_texture_max_height;       // texture maximum height

	bitmap_argb32           m_vector_bitmap;            // experimental: bitmap for vectors
	texture_info *          m_vector_texture;           // experimental: texture for vectors

	bitmap_rgb32            m_default_bitmap;           // experimental: default bitmap
	texture_info *          m_default_texture;          // experimental: default texture
};


/* texture_info holds information about a texture */
class texture_info
{
public:
	texture_info(texture_manager *manager, const render_texinfo *texsource, int prescale, UINT32 flags);
	~texture_info();

	render_texinfo &        get_texinfo() { return m_texinfo; }

	int                     get_width() { return m_rawdims.c.x; }
	int                     get_height() { return m_rawdims.c.y; }
	int                     get_xscale() { return m_xprescale; }
	int                     get_yscale() { return m_yprescale; }

	UINT32                  get_flags() { return m_flags; }

	void                    set_data(const render_texinfo *texsource, UINT32 flags);

	texture_info *          get_next() { return m_next; }
	texture_info *          get_prev() { return m_prev; }

	UINT32                  get_hash() { return m_hash; }

	void                    set_next(texture_info *next) { m_next = next; }
	void                    set_prev(texture_info *prev) { m_prev = prev; }

	bool                    paused() { return m_cur_frame == m_prev_frame; }
	void                    advance_frame() { m_prev_frame = m_cur_frame; }
	void                    increment_frame_count() { m_cur_frame++; }
	void                    mask_frame_count(int mask) { m_cur_frame %= mask; }

	int                     get_cur_frame() { return m_cur_frame; }
	int                     get_prev_frame() { return m_prev_frame; }

	texture *               get_tex() { return m_d3dtex; }
	surface *               get_surface() { return m_d3dsurface; }
	texture *               get_finaltex() { return m_d3dfinaltex; }

	vec2f &                 get_uvstart() { return m_start; }
	vec2f &                 get_uvstop() { return m_stop; }
	vec2f &                 get_rawdims() { return m_rawdims; }

private:
	void prescale();
	void compute_size(int texwidth, int texheight);
	void compute_size_subroutine(int texwidth, int texheight, int* p_width, int* p_height);

	texture_manager *       m_texture_manager;          // texture manager pointer

	renderer *              m_renderer;                 // renderer pointer

	texture_info *          m_next;                     // next texture in the list
	texture_info *          m_prev;                     // prev texture in the list

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
	int                     m_prev_frame;               // what was our last frame? (used to determine pause state)
	texture *               m_d3dtex;                   // Direct3D texture pointer
	surface *               m_d3dsurface;               // Direct3D offscreen plain surface pointer
	texture *               m_d3dfinaltex;              // Direct3D final (post-scaled) texture
};

/* d3d::poly_info holds information about a single polygon/d3d primitive */
class poly_info
{
public:
	poly_info() { }

	void init(D3DPRIMITIVETYPE type, UINT32 count, UINT32 numverts,
			UINT32 flags, d3d::texture_info *texture, UINT32 modmode,
			float prim_width, float prim_height);
	void init(D3DPRIMITIVETYPE type, UINT32 count, UINT32 numverts,
			UINT32 flags, d3d::texture_info *texture, UINT32 modmode,
			float line_time, float line_length,
			float prim_width, float prim_height);

	D3DPRIMITIVETYPE        get_type() { return m_type; }
	UINT32                  get_count() { return m_count; }
	UINT32                  get_vertcount() { return m_numverts; }
	UINT32                  get_flags() { return m_flags; }

	d3d::texture_info *     get_texture() { return m_texture; }
	DWORD                   get_modmode() { return m_modmode; }

	float                   get_line_time() { return m_line_time; }
	float                   get_line_length() { return m_line_length; }

	float                   get_prim_width() { return m_prim_width; }
	float                   get_prim_height() { return m_prim_height; }

private:

	D3DPRIMITIVETYPE        m_type;                       // type of primitive
	UINT32                  m_count;                      // total number of primitives
	UINT32                  m_numverts;                   // total number of vertices
	UINT32                  m_flags;                      // rendering flags

	texture_info *          m_texture;                    // pointer to texture info
	DWORD                   m_modmode;                    // texture modulation mode

	float                   m_line_time;                  // used by vectors
	float                   m_line_length;                // used by vectors

	float                   m_prim_width;                 // used by quads
	float                   m_prim_height;                // used by quads
};

} // d3d

/* vertex describes a single vertex */
struct vertex
{
	float                   x, y, z;                    // X,Y,Z coordinates
	float                   rhw;                        // RHW when no HLSL, padding when HLSL
	D3DCOLOR                color;                      // diffuse color
	float                   u0, v0;                     // texture stage 0 coordinates
	float                   u1, v1;                     // additional info for vector data
};


/* line_aa_step is used for drawing antialiased lines */
struct line_aa_step
{
	float                   xoffs, yoffs;               // X/Y deltas
	float                   weight;                     // weight contribution
};


#endif
