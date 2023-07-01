// license:BSD-3-Clause
// copyright-holders:Sven Gothel

#include "gl_shader_mgr.h"

#include "osdcomm.h"

#include "util/strformat.h"

#include <algorithm>
#include <iterator>


namespace {

#define GLSL_VERTEX_SHADER_INT_NUMBER 1 // general
#define GLSL_VERTEX_SHADER_MAX_NUMBER 2 // general + custom
#define GLSL_VERTEX_SHADER_CUSTOM     1 // custom idx

#ifdef GLSL_SOURCE_ON_DISK

static char const *const glsl_mamebm_vsh_files[GLSL_VERTEX_SHADER_INT_NUMBER] =
{
	"/tmp/glsl_general.vsh"                // general
};

static char const *const glsl_mamebm_fsh_files[glsl_shader_info::FEAT_INT_NUMBER] =
{
	"/tmp/glsl_plain_rgb32_dir.fsh",       // rgb32 dir plain
	"/tmp/glsl_bilinear_rgb32_dir.fsh",    // rgb32 dir bilinear
	"/tmp/glsl_bicubic_rgb32_dir.fsh",     // rgb32 dir bicubic
};

#else // GLSL_SOURCE_ON_DISK

#include "shader/glsl_general.vsh.c"

#include "shader/glsl_plain_rgb32_dir.fsh.c"
#include "shader/glsl_bilinear_rgb32_dir.fsh.c"
#include "shader/glsl_bicubic_rgb32_dir.fsh.c"

static char const *const glsl_mamebm_vsh_sources[GLSL_VERTEX_SHADER_INT_NUMBER] =
{
	glsl_general_vsh_src                   // general
};

static char const *const glsl_mamebm_fsh_sources[glsl_shader_info::FEAT_INT_NUMBER] =
{
	glsl_plain_rgb32_dir_fsh_src,          // rgb32 dir plain
	glsl_bilinear_rgb32_dir_fsh_src,       // rgb32 dir bilinear
	glsl_bicubic_rgb32_dir_fsh_src,        // rgb32 dir bicubic
};

#endif // GLSL_SOURCE_ON_DISK

static char const *const glsl_mamebm_filter_names[glsl_shader_info::FEAT_MAX_NUMBER]{
		"plain",
		"bilinear",
		"bicubic",
		"custom" };

/**
 * fragment shader -> vertex shader mapping
 */
static const int glsl_mamebm_fsh2vsh[glsl_shader_info::FEAT_MAX_NUMBER] =
{
	0,  // plain    -> general
	0,  // bilinear -> general
	0,  // bicubic  -> general
	1,  // custom   -> custom
};

class glsl_shader_impl : public glsl_shader_info
{
public:
	glsl_shader_impl(
#if defined(USE_DISPATCH_GL)
			osd_gl_dispatch *gld
#endif
			)
#if defined(USE_DISPATCH_GL)
		: glsl_shader_info(gld)
#endif
	{
		std::fill(std::begin(mamebm_programs), std::end(mamebm_programs), GLhandleARB(0));
		std::fill(std::begin(mamebm_vsh_shader), std::end(mamebm_vsh_shader), GLhandleARB(0));
		std::fill(std::begin(mamebm_fsh_shader), std::end(mamebm_fsh_shader), GLhandleARB(0));
		std::fill(std::begin(scrn_programs), std::end(scrn_programs), GLhandleARB(0));
		std::fill(std::begin(scrn_vsh_shader), std::end(scrn_vsh_shader), GLhandleARB(0));
		std::fill(std::begin(scrn_fsh_shader), std::end(scrn_fsh_shader), GLhandleARB(0));
	}

	bool initalize(osd_gl_context &gl_ctx)
	{
		if (!load_extension(gl_ctx))
			return false;

		int err = 0;

		for (int i = 0; !err && (i < GLSL_VERTEX_SHADER_INT_NUMBER); i++)
		{
#ifdef GLSL_SOURCE_ON_DISK
			if (glsl_mamebm_vsh_files[i])
				err = compile_file(
						&mamebm_vsh_shader[i],
						GL_VERTEX_SHADER_ARB,
						glsl_mamebm_vsh_files[i],
						false);
#else
			if (glsl_mamebm_vsh_sources[i])
				err = compile_source(
						&mamebm_vsh_shader[i],
						GL_VERTEX_SHADER_ARB,
						glsl_mamebm_vsh_sources[i],
						false);
#endif
		}
		if (err)
			return false;

		for (int j = 0; !err && (j < FEAT_INT_NUMBER); j++)
		{
#ifdef GLSL_SOURCE_ON_DISK
			if (glsl_mamebm_fsh_files[j])
				err = compile_files(
						&mamebm_programs[j],
						&mamebm_vsh_shader[glsl_mamebm_fsh2vsh[j]],
						&mamebm_fsh_shader[j],
						nullptr, // precompiled
						glsl_mamebm_fsh_files[j],
						false);
#else
			if (glsl_mamebm_fsh_sources[j])
				err = compile_sources(
						&mamebm_programs[j],
						&mamebm_vsh_shader[glsl_mamebm_fsh2vsh[j]],
						&mamebm_fsh_shader[j],
						nullptr, // precompiled
						glsl_mamebm_fsh_sources[j]);
#endif
		}
		if (err)
			return false;

		return true;
	}

	virtual ~glsl_shader_impl()
	{
		pfn_glUseProgramObjectARB(0);
		glFinish();

		for (int i = 0; i < GLSL_VERTEX_SHADER_MAX_NUMBER + 9; i++)
		{
			if (mamebm_vsh_shader[i])
				delete_shader(nullptr, &mamebm_vsh_shader[i], nullptr);
		}

		for (int j = 0; j < FEAT_MAX_NUMBER + 9; j++)
		{
			if (mamebm_fsh_shader[j])
				delete_shader(nullptr, nullptr, &mamebm_fsh_shader[j]);
		}

		for (int j = 0; j < FEAT_MAX_NUMBER + 9; j++)
		{
			if (mamebm_programs[j])
				delete_shader(&mamebm_programs[j], nullptr, nullptr);
		}

		for (int i = 0; i < 10; i++)
		{
			if (scrn_vsh_shader[i])
				delete_shader(nullptr, &scrn_vsh_shader[i], nullptr);
			if (scrn_fsh_shader[i])
				delete_shader(nullptr, nullptr, &scrn_fsh_shader[i]);
			if (scrn_programs[i])
				delete_shader(&scrn_programs[i], nullptr, nullptr);
		}
	}

	virtual const char *get_filter_name_mamebm(int glslShaderFeature) override
	{
		if ((0 > glslShaderFeature) || (glslShaderFeature >= FEAT_MAX_NUMBER))
			return "illegal shader feature";

		return glsl_mamebm_filter_names[glslShaderFeature];
	}

	virtual GLhandleARB get_program_mamebm(int glslShaderFeature, int idx) override
	{
		if ((0 > glslShaderFeature) || (glslShaderFeature >= FEAT_MAX_NUMBER))
			return 0;

		return mamebm_programs[glslShaderFeature + idx];
	}

	virtual GLhandleARB get_program_scrn(int idx) override
	{
		if ((0 > idx) || (idx >= 10))
			return 0;

		return scrn_programs[idx];
	}

	virtual int add_mamebm(const char *custShaderPrefix, int idx) override
	{
		int err;

		err = compile_file(
				&mamebm_vsh_shader[GLSL_VERTEX_SHADER_CUSTOM + idx],
				GL_VERTEX_SHADER_ARB,
				util::string_format("%s.vsh", custShaderPrefix).c_str(),
				false);
		if (err)
			return err;

		err = compile_files(
				&mamebm_programs[FEAT_CUSTOM + idx],
				&mamebm_vsh_shader[GLSL_VERTEX_SHADER_CUSTOM + idx],
				&mamebm_fsh_shader[FEAT_CUSTOM + idx],
				nullptr, // precompiled
				util::string_format("%s_rgb32_dir.fsh", custShaderPrefix).c_str(),
				false);
		return err;
	}

	virtual int add_scrn(const char *custShaderPrefix, int idx) override
	{
		int err;

		err = compile_file(
				&scrn_vsh_shader[idx],
				GL_VERTEX_SHADER_ARB,
				util::string_format("%s.vsh", custShaderPrefix).c_str(),
				false);
		if (err)
			return err;

		err = compile_files(
				&scrn_programs[idx],
				&scrn_vsh_shader[idx],
				&scrn_fsh_shader[idx],
				nullptr, // precompiled
				util::string_format("%s.fsh", custShaderPrefix).c_str(),
				false);
		return err;
	}

private:
	GLhandleARB mamebm_programs[FEAT_MAX_NUMBER + 9];                 // rgb32 dir: plain, bilinear, bicubic, custom0-9, ..
	GLhandleARB mamebm_vsh_shader[GLSL_VERTEX_SHADER_MAX_NUMBER + 9]; // general, custom0-9
	GLhandleARB mamebm_fsh_shader[FEAT_MAX_NUMBER+9];                 // rgb32 dir: plain, bilinear, bicubic, custom0-9
	GLhandleARB scrn_programs[10];                                    // rgb32: custom0-9, ..
	GLhandleARB scrn_vsh_shader[10];                                  // custom0-9
	GLhandleARB scrn_fsh_shader[10];                                  // rgb32: custom0-9
};

} // anonymous namespace


std::unique_ptr<glsl_shader_info> glsl_shader_info::init(
		osd_gl_context &gl_ctx
#if defined(USE_DISPATCH_GL)
		, osd_gl_dispatch *gld
#endif
		)
{
	auto result = std::make_unique<glsl_shader_impl>(
#if defined(USE_DISPATCH_GL)
			gld
#endif
			);
	if (!result->initalize(gl_ctx))
		result.reset();
	return result;
}
