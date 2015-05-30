// license:BSD-3-Clause
// copyright-holders:Sven Gothel
/***
 *
 * GL Shader Tool - Convinient Basic Shader functionality
 *
 * Copyright (c) 2007, Sven Gothel
 * Copyright (c) 2007, MAME Team
 *
 * GPL version 2
 *
 * OpenGL GLSL ARB extentions:
 *
 *  GL_ARB_shader_objects
 *  GL_ARB_shading_language_100
 *  GL_ARB_vertex_shader
 *  GL_ARB_fragment_shader
 *
 */


#ifndef _GL_SHADER_TOOL_
#define _GL_SHADER_TOOL_

#ifdef __cplusplus
extern "C" {
#endif

/***
 *
 * OpenGL GLSL extentions:
         GL_ARB_shader_objects
         GL_ARB_shading_language_100
         GL_ARB_vertex_shader
         GL_ARB_fragment_shader
 *
 */

#include <stdint.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "osd_opengl.h"

#if defined(SDLMAME_MACOSX)

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#endif

typedef void * (APIENTRYP PFNGLGETPROCADDRESSOS)(const char *procName);

/**
 * YOU HAVE TO CALL THIS FUNCTION ONCE !
 * @return 0 - ok .. all shader ARB functions loaded
 *         otherwise !=0
 */
int gl_shader_loadExtention(osd_gl_context *gl_ctx);

enum GLSLCheckMode {
		CHECK_QUIET,         /* just return 0, if no error, otherwise the GL error code, no stderr output */
		CHECK_VERBOSE,       /* same as CHECK_QUIET, but in the case of an error, use stderr to be verbose */
		CHECK_ALWAYS_VERBOSE /* always print out all information available */
};

#define GL_CHECK_ERROR_VERBOSE() gl_check_error(CHECK_ALWAYS_VERBOSE,__FILE__,__LINE__)
#define GL_CHECK_ERROR_NORMAL() gl_check_error(CHECK_VERBOSE,__FILE__,__LINE__)
#define GL_CHECK_ERROR_QUIET() gl_check_error(CHECK_QUIET,__FILE__,__LINE__)
#define GL_CHECK_ERROR(m) gl_check_error(m,__FILE__,__LINE__)

#define GL_SHADER_CHECK_ERROR_VERBOSE(o,q) gl_shader_check_error(o,q,CHECK_ALWAYS_VERBOSE,__FILE__,__LINE__)
#define GL_SHADER_CHECK_ERROR_NORMAL(o,q) gl_shader_check_error(o,q,CHECK_VERBOSE,__FILE__,__LINE__)
#define GL_SHADER_CHECK_ERROR_QUIET(o,q) gl_shader_check_error(o,q,CHECK_QUIET,__FILE__,__LINE__)
#define GL_SHADER_CHECK_ERROR(o,q,m) gl_shader_check_error(o,q,m,__FILE__,__LINE__)

#ifdef GL_SHADER_TOOL_DEBUG
	#define GL_SHADER_CHECK(o,q) GL_SHADER_CHECK_ERROR_VERBOSE(o,q)
	#define GL_CHECK() GL_CHECK_ERROR_VERBOSE()
#else
	#define GL_SHADER_CHECK(o,q) GL_SHADER_CHECK_ERROR_NORMAL(o,q)
	#define GL_CHECK() GL_CHECK_ERROR_NORMAL()
#endif

int gl_check_error(GLSLCheckMode m, const char *file, const int line);

int gl_texture_check_size(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
							GLint border, GLenum format, GLenum type,
				GLsizei *avail_width, GLsizei *avail_height,
				int verbose);

int gl_round_to_pow2(int v);

/**
 * @param obj_query Can be either GL_OBJECT_TYPE_ARB, GL_OBJECT_DELETE_STATUS_ARB, GL_OBJECT_COMPILE_STATUS_ARB,
 *                                GL_OBJECT_LINK_STATUS_ARB, GL_OBJECT_VALIDATE_STATUS_ARB
 *                  Should be used after the refering action, i.e. GL_OBJECT_DELETE_STATUS_ARB after a
 *              glDeleteObjectARB call, etc.
 */
int gl_shader_check_error(GLhandleARB obj, GLenum obj_query, GLSLCheckMode m, const char *file, const int line);

int gl_compile_shader_file  ( GLhandleARB *shader, GLenum type, const char * shader_file, int verbose );
int gl_compile_shader_source( GLhandleARB *shader, GLenum type, const char * shader_source, int verbose );

/**
 * you can pass either a valid shader_file, or a precompiled vertex_shader,
 * this is true for both, vertex and fragment shaders.
 */
int gl_compile_shader_files( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader,
								const char * vertex_shader_file,
								const char * fragment_shader_file,
					int verbose
							);

/**
 * you can pass either a valid shader_file, or a precompiled vertex_shader,
 * this is true for both, vertex and fragment shaders.
 */
int gl_compile_shader_sources( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader,
								const GLcharARB * vertex_shader_source,
								const GLcharARB * fragment_shader_source
								);

int gl_delete_shader( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader );

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

extern PFNGLGETOBJECTPARAMETERIVARBPROC pfn_glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC pfn_glGetInfoLogARB;
extern PFNGLDELETEOBJECTARBPROC pfn_glDeleteObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC pfn_glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC pfn_glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC pfn_glCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC pfn_glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC pfn_glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC pfn_glLinkProgramARB;
extern PFNGLVALIDATEPROGRAMARBPROC pfn_glValidateProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC pfn_glUseProgramObjectARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC pfn_glGetUniformLocationARB;
extern PFNGLUNIFORM1FARBPROC pfn_glUniform1fARB;
extern PFNGLUNIFORM1IARBPROC pfn_glUniform1iARB;
extern PFNGLUNIFORM1FVARBPROC pfn_glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC pfn_glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC pfn_glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC pfn_glUniform4fvARB;
extern PFNGLUNIFORM1IVARBPROC pfn_glUniform1ivARB;
extern PFNGLUNIFORM2IVARBPROC pfn_glUniform2ivARB;
extern PFNGLUNIFORM3IVARBPROC pfn_glUniform3ivARB;
extern PFNGLUNIFORM4IVARBPROC pfn_glUniform4ivARB;


#ifdef __cplusplus
}
#endif

#endif
