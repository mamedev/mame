// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  drawogl.h - SDL software and OpenGL implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#pragma once

#ifndef __DRAWOGL__
#define __DRAWOGL__

// OSD headers
#ifndef OSD_WINDOWS
#include "osdsdl.h"
#include "window.h"
#else
#include "../windows/window.h"
typedef UINT64 HashT;
#endif

#if defined(OSD_WINDOWS)
#include "winglcontext.h"
#else
#include "sdlglcontext.h"
#endif

#include "modules/opengl/gl_shader_mgr.h"

//============================================================
//  Textures
//============================================================

/* ogl_texture_info holds information about a texture */
class ogl_texture_info
{
public:
	ogl_texture_info()
	:   hash(0), flags(0), rawwidth(0), rawheight(0),
		rawwidth_create(0), rawheight_create(0),
		type(0), format(0), borderpix(0), xprescale(0), yprescale(0), nocopy(0),
		texture(0), texTarget(0), texpow2(0), mpass_dest_idx(0), pbo(0), data(NULL),
		data_own(0), texCoordBufferName(0)
	{
		for (int i=0; i<2; i++)
		{
			mpass_textureunit[i] = 0;
			mpass_texture_mamebm[i] = 0;
			mpass_fbo_mamebm[i] = 0;
			mpass_texture_scrn[i] = 0;
			mpass_fbo_scrn[i] = 0;
		}
		for (int i=0; i<8; i++)
			texCoord[i] = 0.0f;
	}

	HashT               hash;               // hash value for the texture (must be >= pointer size)
	UINT32              flags;              // rendering flags
	render_texinfo      texinfo;            // copy of the texture info
	int                 rawwidth, rawheight;    // raw width/height of the texture
	int                 rawwidth_create;    // raw width/height, pow2 compatible, if needed
	int                 rawheight_create;   // (create and initial set the texture, not for copy!)
	int                 type;               // what type of texture are we?
	int                 format;             // texture format
	int                 borderpix;          // do we have a 1 pixel border?
	int                 xprescale;          // what is our X prescale factor?
	int                 yprescale;          // what is our Y prescale factor?
	int                 nocopy;             // must the texture date be copied?

	UINT32              texture;            // OpenGL texture "name"/ID

	GLenum              texTarget;          // OpenGL texture target
	int                 texpow2;            // Is this texture pow2

	UINT32              mpass_dest_idx;         // Multipass dest idx [0..1]
	UINT32              mpass_textureunit[2];   // texture unit names for GLSL

	UINT32              mpass_texture_mamebm[2];// Multipass OpenGL texture "name"/ID for the shader
	UINT32              mpass_fbo_mamebm[2];    // framebuffer object for this texture, multipass
	UINT32              mpass_texture_scrn[2];  // Multipass OpenGL texture "name"/ID for the shader
	UINT32              mpass_fbo_scrn[2];      // framebuffer object for this texture, multipass

	UINT32              pbo;                    // pixel buffer object for this texture (DYNAMIC only!)
	UINT32              *data;                  // pixels for the texture
	int                 data_own;               // do we own / allocated it ?
	GLfloat             texCoord[8];
	GLuint              texCoordBufferName;

};

/* sdl_info is the information about SDL for the current screen */
class renderer_ogl : public osd_renderer
{
public:
	renderer_ogl(osd_window *window)
		: osd_renderer(window, FLAG_NEEDS_OPENGL)
		, m_blittimer(0)
		, m_width(0)
		, m_height(0)
		, m_blit_dim(0, 0)
		, m_gl_context(NULL)
		, m_initialized(0)
		, m_last_blendmode(0)
		, m_texture_max_width(0)
		, m_texture_max_height(0)
		, m_texpoweroftwo(0)
		, m_usevbo(0)
		, m_usepbo(0)
		, m_usefbo(0)
		, m_useglsl(0)
		, m_glsl(nullptr)
		, m_glsl_program_num(0)
		, m_glsl_program_mb2sc(0)
		, m_usetexturerect(0)
		, m_init_context(0)
		, m_last_hofs(0.0f)
		, m_last_vofs(0.0f)
		, m_surf_w(0)
		, m_surf_h(0)
	{
		for (int i=0; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			m_texhash[i] = NULL;
		for (int i=0; i < 2*GLSL_SHADER_MAX; i++)
			m_glsl_program[i] = 0;
		for (int i=0; i < 8; i++)
			m_texVerticex[i] = 0.0f;
	}
	virtual ~renderer_ogl();

	static bool init(running_machine &machine);
	static void exit();

	virtual int create() override;
	virtual int draw(const int update) override;

#ifndef OSD_WINDOWS
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#endif
	virtual render_primitive_list *get_primitives() override
	{
		osd_dim nd = window().get_size();
		if (nd != m_blit_dim)
		{
			m_blit_dim = nd;
			notify_changed();
		}
		window().target()->set_bounds(m_blit_dim.width(), m_blit_dim.height(), window().pixel_aspect());
		return &window().target()->get_primitives();
	}

#ifdef OSD_WINDOWS
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
#endif

private:
	static const UINT32 HASH_SIZE = ((1 << 10) + 1);
	static const UINT32 OVERFLOW_SIZE = (1 << 10);

	void destroy_all_textures();

	static void load_gl_lib(running_machine &machine);
	void loadGLExtensions();
	void initialize_gl();
	void set_blendmode(int blendmode);
	HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags);
	void texture_compute_type_subroutine(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	void texture_compute_size_subroutine(ogl_texture_info *texture, UINT32 flags,
				UINT32 width, UINT32 height,
				int* p_width, int* p_height, int* p_width_create, int* p_height_create);
	void texture_compute_size_type(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	ogl_texture_info *texture_create(const render_texinfo *texsource, UINT32 flags);
	int texture_shader_create(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	ogl_texture_info *texture_find(const render_primitive *prim);
	void texture_coord_update(ogl_texture_info *texture, const render_primitive *prim, int shaderIdx);
	void texture_mpass_flip(ogl_texture_info *texture, int shaderIdx);
	void texture_shader_update(ogl_texture_info *texture, render_container *container,  int shaderIdx);
	ogl_texture_info * texture_update(const render_primitive *prim, int shaderIdx);
	void texture_disable(ogl_texture_info * texture);
	void texture_all_disable();

	INT32           m_blittimer;
	int             m_width;
	int             m_height;
	osd_dim         m_blit_dim;

	osd_gl_context  *m_gl_context;

	int             m_initialized;        // is everything well initialized, i.e. all GL stuff etc.
	// 3D info (GL mode only)
	ogl_texture_info *  m_texhash[HASH_SIZE + OVERFLOW_SIZE];
	int             m_last_blendmode;     // previous blendmode
	INT32           m_texture_max_width;      // texture maximum width
	INT32           m_texture_max_height;     // texture maximum height
	int             m_texpoweroftwo;          // must textures be power-of-2 sized?
	int             m_usevbo;         // runtime check if VBO is available
	int             m_usepbo;         // runtime check if PBO is available
	int             m_usefbo;         // runtime check if FBO is available
	int             m_useglsl;        // runtime check if GLSL is available

	glsl_shader_info *m_glsl;             // glsl_shader_info

	GLhandleARB     m_glsl_program[2*GLSL_SHADER_MAX];  // GLSL programs, or 0
	int             m_glsl_program_num;   // number of GLSL programs
	int             m_glsl_program_mb2sc; // GLSL program idx, which transforms
										// the mame-bitmap. screen-bitmap (size/rotation/..)
										// All progs <= glsl_program_mb2sc using the mame bitmap
										// as input, otherwise the screen bitmap.
										// All progs >= glsl_program_mb2sc using the screen bitmap
										// as output, otherwise the mame bitmap.
	int             m_usetexturerect;     // use ARB_texture_rectangle for non-power-of-2, general use

	int             m_init_context;       // initialize context before next draw

	float           m_last_hofs;
	float           m_last_vofs;

	// Static vars from draogl_window_dra
	INT32           m_surf_w;
	INT32           m_surf_h;
	GLfloat         m_texVerticex[8];

	static bool     s_shown_video_info;
	static bool     s_dll_loaded;
};

#endif // __DRAWOGL__
