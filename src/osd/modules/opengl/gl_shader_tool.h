// license:BSD-3-Clause
// copyright-holders:Sven Gothel
/***
 *
 * GL Shader Tool - Convenient Basic Shader functionality
 *
 * Copyright (c) 2007, Sven Gothel
 * Copyright (c) 2007, MAME Team
 *
 * May be distributed under the terms of the GNU General Public
 * License, version 2.0, or the 3-Clause BSD License.
 *
 * OpenGL GLSL ARB extensions:
 *
 *  GL_ARB_shader_objects
 *  GL_ARB_shading_language_100
 *  GL_ARB_vertex_shader
 *  GL_ARB_fragment_shader
 *
 */
#ifndef MAME_OSD_OPENGL_GL_SHADER_TOOL_H
#define MAME_OSD_OPENGL_GL_SHADER_TOOL_H

#pragma once

#include "osd_opengl.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * OpenGL GLSL extensions:
         GL_ARB_shader_objects
         GL_ARB_shading_language_100
         GL_ARB_vertex_shader
         GL_ARB_fragment_shader
 *
 */

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#if defined(SDLMAME_MACOSX)

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#endif

#if defined(SDLMAME_MACOSX)
#ifndef GL_ARB_shader_objects
	typedef char GLcharARB;
	typedef unsigned int GLhandleARB;
#endif
typedef void (APIENTRYP PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (APIENTRYP PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef GLhandleARB (APIENTRYP PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRYP PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRYP PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (APIENTRYP PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRYP PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRYP PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count, const GLint *value);
#endif

typedef void * (APIENTRYP PFNGLGETPROCADDRESSOS)(const char *procName);

#ifdef __cplusplus
}
#endif

class gl_shader_tool
{
public:
	gl_shader_tool(
#if defined(USE_DISPATCH_GL)
			osd_gl_dispatch *gld
#endif
			);

	PFNGLGETOBJECTPARAMETERIVARBPROC pfn_glGetObjectParameterivARB;
	PFNGLGETINFOLOGARBPROC pfn_glGetInfoLogARB;
	PFNGLDELETEOBJECTARBPROC pfn_glDeleteObjectARB;
	PFNGLCREATESHADEROBJECTARBPROC pfn_glCreateShaderObjectARB;
	PFNGLSHADERSOURCEARBPROC pfn_glShaderSourceARB;
	PFNGLCOMPILESHADERARBPROC pfn_glCompileShaderARB;
	PFNGLCREATEPROGRAMOBJECTARBPROC pfn_glCreateProgramObjectARB;
	PFNGLATTACHOBJECTARBPROC pfn_glAttachObjectARB;
	PFNGLLINKPROGRAMARBPROC pfn_glLinkProgramARB;
	PFNGLVALIDATEPROGRAMARBPROC pfn_glValidateProgramARB;
	PFNGLUSEPROGRAMOBJECTARBPROC pfn_glUseProgramObjectARB;
	PFNGLGETUNIFORMLOCATIONARBPROC pfn_glGetUniformLocationARB;
	PFNGLUNIFORM1FARBPROC pfn_glUniform1fARB;
	PFNGLUNIFORM1IARBPROC pfn_glUniform1iARB;
	PFNGLUNIFORM1FVARBPROC pfn_glUniform1fvARB;
	PFNGLUNIFORM2FVARBPROC pfn_glUniform2fvARB;
	PFNGLUNIFORM3FVARBPROC pfn_glUniform3fvARB;
	PFNGLUNIFORM4FVARBPROC pfn_glUniform4fvARB;
	PFNGLUNIFORM1IVARBPROC pfn_glUniform1ivARB;
	PFNGLUNIFORM2IVARBPROC pfn_glUniform2ivARB;
	PFNGLUNIFORM3IVARBPROC pfn_glUniform3ivARB;
	PFNGLUNIFORM4IVARBPROC pfn_glUniform4ivARB;

	/**
	 * YOU HAVE TO CALL THIS FUNCTION ONCE !
	 * @return true - ok .. all shader ARB functions loaded
	 *         otherwise false
	 */
	bool load_extension(osd_gl_context &gl_ctx);

	int texture_check_size(
			GLenum target,
			GLint level,
			GLint internalFormat,
			GLsizei width,
			GLsizei height,
			GLint border,
			GLenum format,
			GLenum type,
			GLsizei *avail_width,
			GLsizei *avail_height,
			bool verbose);

protected:
	int compile_file(GLhandleARB *shader, GLenum type, const char *shader_file, bool verbose);
	int compile_source(GLhandleARB *shader, GLenum type, const char *shader_source, bool verbose);

	/**
	 * you can pass either a valid shader_file, or a precompiled vertex_shader,
	 * this is true for both, vertex and fragment shaders.
	 */
	int compile_files(
			GLhandleARB *program,
			GLhandleARB *vertex_shader,
			GLhandleARB *fragment_shader,
			const char *vertex_shader_file,
			const char *fragment_shader_file,
			bool verbose);

	/**
	 * you can pass either a valid shader_file, or a precompiled vertex_shader,
	 * this is true for both, vertex and fragment shaders.
	 */
	int compile_sources(
			GLhandleARB *program,
			GLhandleARB *vertex_shader,
			GLhandleARB *fragment_shader,
			const GLcharARB *vertex_shader_source,
			const GLcharARB *fragment_shader_source);

	int delete_shader(GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader);

#if defined(USE_DISPATCH_GL)
	osd_gl_dispatch *const gl_dispatch; // name is magic, can't be changed
#endif

private:
	enum GLSLCheckMode
	{
		CHECK_QUIET,         // just return 0, if no error, otherwise the GL error code, no stderr output
		CHECK_VERBOSE,       // same as CHECK_QUIET, but in the case of an error, use stderr to be verbose
		CHECK_ALWAYS_VERBOSE // always print out all information available
	};

	int check_error(GLSLCheckMode m, const char *file, const int line);

	/**
	 * @param obj_query Can be either GL_OBJECT_TYPE_ARB, GL_OBJECT_DELETE_STATUS_ARB, GL_OBJECT_COMPILE_STATUS_ARB,
	 *                                GL_OBJECT_LINK_STATUS_ARB, GL_OBJECT_VALIDATE_STATUS_ARB
	 *                  Should be used after the referring action, i.e. GL_OBJECT_DELETE_STATUS_ARB after a
	 *              glDeleteObjectARB call, etc.
	 */
	int check_error(GLhandleARB obj, GLenum obj_query, GLSLCheckMode m, const char *file, const int line);

	int delete_shader_tool(GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader, bool externalcall);
};

int gl_round_to_pow2(int v);

#endif // MAME_OSD_OPENGL_GL_SHADER_TOOL_H
