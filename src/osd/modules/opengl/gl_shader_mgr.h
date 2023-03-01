// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#ifndef MAME_OSD_OPENGL_GL_SHADER_MGR_H
#define MAME_OSD_OPENGL_GL_SHADER_MGR_H

#pragma once

#include "gl_shader_tool.h"
#include "osd_opengl.h"

#include <memory>


// #define GLSL_SOURCE_ON_DISK 1


class glsl_shader_info : public gl_shader_tool
{
public:
	enum FEATURE
	{
		FEAT_PLAIN,
		FEAT_BILINEAR,
		FEAT_BICUBIC,
		FEAT_INT_NUMBER,
		FEAT_CUSTOM = FEAT_INT_NUMBER,
		FEAT_MAX_NUMBER
	};

	using gl_shader_tool::gl_shader_tool;

	virtual ~glsl_shader_info() = default;

	/**
	 * returns the GLSL program if ok/available, otherwise 0
	 */
	virtual GLhandleARB get_program_mamebm(int glslShaderFeature, int idx) = 0;

	virtual const char *get_filter_name_mamebm(int glslShaderFeature) = 0;

	virtual int add_mamebm(const char *custShaderPrefix, int idx) = 0;

	virtual GLhandleARB get_program_scrn(int idx) = 0;
	virtual int add_scrn(const char *custShaderPrefix, int idx) = 0;

	/**
	 * returns pointer if OK, otherwise nullptr
	 */
	static std::unique_ptr<glsl_shader_info> init(
			osd_gl_context &gl_ctx
#if defined(USE_DISPATCH_GL)
			, osd_gl_dispatch *gld
#endif
			);
};

#endif // MAME_OSD_OPENGL_GL_SHADER_MGR_H
