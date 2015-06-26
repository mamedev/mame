// license:BSD-3-Clause
// copyright-holders:Sven Gothel

#ifndef GL_SHADER_MGR_H
#define GL_SHADER_MGR_H

// #define GLSL_SOURCE_ON_DISK 1

enum GLSL_SHADER_FEATURE {
	GLSL_SHADER_FEAT_PLAIN,
	GLSL_SHADER_FEAT_BILINEAR,
	GLSL_SHADER_FEAT_INT_NUMBER,
	GLSL_SHADER_FEAT_CUSTOM = GLSL_SHADER_FEAT_INT_NUMBER,
	GLSL_SHADER_FEAT_MAX_NUMBER
};

// old code passed sdl_info * to functions here
// however the parameter was not used
// changed interface to more generic one.
struct glsl_shader_info
{
	int dummy; // avoid compiler breakage
};


/**
 * returns pointer if ok, otherwise NULL
 */
glsl_shader_info *glsl_shader_init(osd_gl_context *gl_ctx);

/**
 * returns 0 if ok, otherwise an error value
 */
int glsl_shader_free(glsl_shader_info *shinfo);

/**
 * returns the GLSL program if ok/available, otherwise 0
 */
GLhandleARB glsl_shader_get_program_mamebm(int glslShaderFeature, int idx);

const char * glsl_shader_get_filter_name_mamebm(int glslShaderFeature);

int glsl_shader_add_mamebm(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx);

GLhandleARB glsl_shader_get_program_scrn(int idx);
int glsl_shader_add_scrn(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx);

#endif // GL_SHADER_MGR_H
