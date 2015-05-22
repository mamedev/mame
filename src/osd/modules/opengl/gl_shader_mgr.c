// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#include <stdio.h>   /* snprintf */
#include <stdlib.h>  /* malloc */

#include "osdcomm.h"
#include "osd_opengl.h"

#include "gl_shader_mgr.h"
#include "gl_shader_tool.h"

#define GLSL_VERTEX_SHADER_INT_NUMBER 1 // general
#define GLSL_VERTEX_SHADER_MAX_NUMBER 2 // general + custom
#define GLSL_VERTEX_SHADER_CUSTOM     1 // custom idx

#ifdef GLSL_SOURCE_ON_DISK

static const char * glsl_mamebm_vsh_files [GLSL_VERTEX_SHADER_INT_NUMBER] =
{
	"/tmp/glsl_general.vsh"                             // general
};

static const char * glsl_mamebm_fsh_files [GLSL_SHADER_FEAT_INT_NUMBER] =
{
	"/tmp/glsl_plain_rgb32_dir.fsh",                           // rgb32 dir plain
	"/tmp/glsl_bilinear_rgb32_dir.fsh"                          // rgb32 dir bilinear
};

#else // GLSL_SOURCE_ON_DISK

#include "shader/glsl_general.vsh.c"

#include "shader/glsl_plain_rgb32_dir.fsh.c"
#include "shader/glsl_bilinear_rgb32_dir.fsh.c"

static const char * glsl_mamebm_vsh_sources [GLSL_VERTEX_SHADER_INT_NUMBER] =
{
	glsl_general_vsh_src                                    // general
};

static const char * glsl_mamebm_fsh_sources [GLSL_SHADER_FEAT_INT_NUMBER] =
{
	glsl_plain_rgb32_dir_fsh_src,                              // rgb32 dir plain
	glsl_bilinear_rgb32_dir_fsh_src                         // rgb32 dir bilinear
};

#endif // GLSL_SOURCE_ON_DISK

static const char * glsl_mamebm_filter_names [GLSL_SHADER_FEAT_MAX_NUMBER] =
{
	"plain",
	"bilinear",
	"custom"
};

static GLhandleARB glsl_mamebm_programs [GLSL_SHADER_FEAT_MAX_NUMBER+9] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* rgb32 dir: plain, bilinear, custom0-9, .. */
};

/**
 * fragment shader -> vertex shader mapping
 */
static int glsl_mamebm_fsh2vsh[GLSL_SHADER_FEAT_MAX_NUMBER] =
{
	0,  // plain    -> general
	0,  // bilinear -> general
	1,      // custom       -> custom
};

static GLhandleARB glsl_mamebm_vsh_shader[GLSL_VERTEX_SHADER_MAX_NUMBER+9] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; /* general, custom0-9 */

static GLhandleARB glsl_mamebm_fsh_shader [GLSL_SHADER_FEAT_MAX_NUMBER+9] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* rgb32 dir: plain, bilinear, custom0-9 */
};

static GLhandleARB glsl_scrn_programs [10] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* rgb32: custom0-9, .. */
};

static GLhandleARB glsl_scrn_vsh_shader[10] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; /* custom0-9 */

static GLhandleARB glsl_scrn_fsh_shader [10] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; /* rgb32: custom0-9 */

const char * glsl_shader_get_filter_name_mamebm(int glslShaderFeature)
{
	if ( !(0 <= glslShaderFeature && glslShaderFeature < GLSL_SHADER_FEAT_MAX_NUMBER) )
		return "illegal shader feature";

	return glsl_mamebm_filter_names[glslShaderFeature];
}

GLhandleARB glsl_shader_get_program_mamebm(int glslShaderFeature, int idx)
{
	if ( !(0 <= glslShaderFeature && glslShaderFeature < GLSL_SHADER_FEAT_MAX_NUMBER) )
		return 0;

	return glsl_mamebm_programs[glslShaderFeature+idx];
}

GLhandleARB glsl_shader_get_program_scrn(int idx)
{
	if ( !(0 <= idx && idx < 10) )
		return 0;

	return glsl_scrn_programs[idx];
}

glsl_shader_info *glsl_shader_init(osd_gl_context *gl_ctx)
{
	int i,j, err;

	err = gl_shader_loadExtention(gl_ctx);
	if(err) return NULL;

	for (i=0; !err && i<GLSL_VERTEX_SHADER_INT_NUMBER; i++)
	{
	#ifdef GLSL_SOURCE_ON_DISK
		if(glsl_mamebm_vsh_files[i])
			err = gl_compile_shader_file  ( &glsl_mamebm_vsh_shader[i], GL_VERTEX_SHADER_ARB,
							glsl_mamebm_vsh_files[i], 0);
	#else
		if(glsl_mamebm_vsh_sources[i])
			err = gl_compile_shader_source( &glsl_mamebm_vsh_shader[i], GL_VERTEX_SHADER_ARB,
							glsl_mamebm_vsh_sources[i], 0);
	#endif
	}

	if(err) return NULL;

	for (j=0; !err && j<GLSL_SHADER_FEAT_INT_NUMBER; j++)
	{
	#ifdef GLSL_SOURCE_ON_DISK
		if(glsl_mamebm_fsh_files[j])
			err = gl_compile_shader_files  (&glsl_mamebm_programs[j],
							&glsl_mamebm_vsh_shader[glsl_mamebm_fsh2vsh[j]],
							&glsl_mamebmfsh_shader[j],
							NULL /*precompiled*/, glsl_mamebm_fsh_files[j], 0);
	#else
		if(glsl_mamebm_fsh_sources[j])
			err = gl_compile_shader_sources(&glsl_mamebm_programs[j],
							&glsl_mamebm_vsh_shader[glsl_mamebm_fsh2vsh[j]],
							&glsl_mamebm_fsh_shader[j],
							NULL /*precompiled*/, glsl_mamebm_fsh_sources[j]);
	#endif
	}
	if (err) return NULL;
	return (glsl_shader_info *) malloc(sizeof(glsl_shader_info *));
}

int glsl_shader_free(glsl_shader_info *shinfo)
{
	int i,j;

	pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
		glFinish();

	for (i=0; i<GLSL_VERTEX_SHADER_MAX_NUMBER+9; i++)
	{
		if ( glsl_mamebm_vsh_shader[i] )
			(void) gl_delete_shader( NULL,  &glsl_mamebm_vsh_shader[i], NULL);
	}

	for (j=0; j<GLSL_SHADER_FEAT_MAX_NUMBER+9; j++)
	{
		if ( glsl_mamebm_fsh_shader[j] )
			(void) gl_delete_shader( NULL, NULL, &glsl_mamebm_fsh_shader[j]);
	}

	for (j=0; j<GLSL_SHADER_FEAT_MAX_NUMBER+9; j++)
	{
		if ( glsl_mamebm_programs[j] )
			(void) gl_delete_shader( &glsl_mamebm_programs[j], NULL, NULL);
	}

	for (i=0; i<10; i++)
	{
		if ( glsl_scrn_vsh_shader[i] )
			(void) gl_delete_shader( NULL,  &glsl_scrn_vsh_shader[i], NULL);
		if ( glsl_scrn_fsh_shader[i] )
			(void) gl_delete_shader( NULL, NULL, &glsl_scrn_fsh_shader[i]);
		if ( glsl_scrn_programs[i] )
			(void) gl_delete_shader( &glsl_scrn_programs[i], NULL, NULL);
	}

	free(shinfo);
	return 0;
}

int glsl_shader_add_mamebm(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx)
{
	int err;
	static char fname[8192];

	snprintf(fname, 8192, "%s.vsh", custShaderPrefix); fname[8191]=0;

	err = gl_compile_shader_file  ( &glsl_mamebm_vsh_shader[GLSL_VERTEX_SHADER_CUSTOM+idx], GL_VERTEX_SHADER_ARB, fname, 0);
	if(err) return err;

	snprintf(fname, 8192, "%s_rgb32_dir.fsh", custShaderPrefix); fname[8191]=0;

	err = gl_compile_shader_files  (&glsl_mamebm_programs[GLSL_SHADER_FEAT_CUSTOM+idx],
					&glsl_mamebm_vsh_shader[GLSL_VERTEX_SHADER_CUSTOM+idx],
					&glsl_mamebm_fsh_shader[GLSL_SHADER_FEAT_CUSTOM+idx],
					NULL /*precompiled*/, fname, 0);

	return err;
}

int glsl_shader_add_scrn(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx)
{
	int err;
	static char fname[8192];

	snprintf(fname, 8192, "%s.vsh", custShaderPrefix); fname[8191]=0;

	err = gl_compile_shader_file  ( &glsl_scrn_vsh_shader[idx], GL_VERTEX_SHADER_ARB, fname, 0);
	if(err) return err;

	snprintf(fname, 8192, "%s.fsh", custShaderPrefix); fname[8191]=0;

	err = gl_compile_shader_files  (&glsl_scrn_programs[idx],
					&glsl_scrn_vsh_shader[idx],
					&glsl_scrn_fsh_shader[idx],
					NULL /*precompiled*/, fname, 0);
	return err;
}
