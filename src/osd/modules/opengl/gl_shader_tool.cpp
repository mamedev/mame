// license:BSD-3-Clause
// copyright-holders:Sven Gothel
/***
 *
 * GL Shader Tool - Convenient Basic Shader functionality
 *
 * Copyright (c) 2007, Sven Gothel
 *
 * OpenGL GLSL ARB extensions:
 *
 *  GL_ARB_shader_objects
 *  GL_ARB_shading_language_100
 *  GL_ARB_vertex_shader
 *  GL_ARB_fragment_shader
 *
 */

#include "gl_shader_tool.h"

#include "osdcore.h"


#define GL_CHECK_ERROR_VERBOSE() check_error(CHECK_ALWAYS_VERBOSE, __FILE__, __LINE__)
#define GL_CHECK_ERROR_NORMAL()  check_error(CHECK_VERBOSE,        __FILE__, __LINE__)
#define GL_CHECK_ERROR_QUIET()   check_error(CHECK_QUIET,          __FILE__, __LINE__)
#define GL_CHECK_ERROR(m)        check_error(m,                    __FILE__, __LINE__)

#define GL_SHADER_CHECK_ERROR_VERBOSE(o,q) check_error(o, q, CHECK_ALWAYS_VERBOSE, __FILE__, __LINE__)
#define GL_SHADER_CHECK_ERROR_NORMAL(o,q)  check_error(o, q, CHECK_VERBOSE,        __FILE__, __LINE__)
#define GL_SHADER_CHECK_ERROR_QUIET(o,q)   check_error(o, q, CHECK_QUIET,          __FILE__, __LINE__)
#define GL_SHADER_CHECK_ERROR(o,q,m)       check_error(o, q, m,                    __FILE__, __LINE__)

#ifdef GL_SHADER_TOOL_DEBUG
	#define GL_SHADER_CHECK(o, q) GL_SHADER_CHECK_ERROR_VERBOSE(o, q)
	#define GL_CHECK() GL_CHECK_ERROR_VERBOSE()
#else
	#define GL_SHADER_CHECK(o, q) GL_SHADER_CHECK_ERROR_NORMAL(o, q)
	#define GL_CHECK() GL_CHECK_ERROR_NORMAL()
#endif


gl_shader_tool::gl_shader_tool(
#if defined(USE_DISPATCH_GL)
		osd_gl_dispatch *gld
#endif
		)
	: pfn_glGetObjectParameterivARB(nullptr)
	, pfn_glGetInfoLogARB(nullptr)
	, pfn_glDeleteObjectARB(nullptr)
	, pfn_glCreateShaderObjectARB(nullptr)
	, pfn_glShaderSourceARB(nullptr)
	, pfn_glCompileShaderARB(nullptr)
	, pfn_glCreateProgramObjectARB(nullptr)
	, pfn_glAttachObjectARB(nullptr)
	, pfn_glLinkProgramARB(nullptr)
	, pfn_glValidateProgramARB(nullptr)
	, pfn_glUseProgramObjectARB(nullptr)
	, pfn_glGetUniformLocationARB(nullptr)
	, pfn_glUniform1fARB(nullptr)
	, pfn_glUniform1iARB(nullptr)
	, pfn_glUniform1fvARB(nullptr)
	, pfn_glUniform2fvARB(nullptr)
	, pfn_glUniform3fvARB(nullptr)
	, pfn_glUniform4fvARB(nullptr)
	, pfn_glUniform1ivARB(nullptr)
	, pfn_glUniform2ivARB(nullptr)
	, pfn_glUniform3ivARB(nullptr)
	, pfn_glUniform4ivARB(nullptr)
#if defined(USE_DISPATCH_GL)
	, gl_dispatch(gld)
#endif
{
}


bool gl_shader_tool::load_extension(osd_gl_context &gl_ctx)
{
	gl_ctx.get_proc_address(pfn_glGetObjectParameterivARB, "glGetObjectParameterivARB");
	gl_ctx.get_proc_address(pfn_glGetInfoLogARB,           "glGetInfoLogARB");
	gl_ctx.get_proc_address(pfn_glDeleteObjectARB,         "glDeleteObjectARB");
	gl_ctx.get_proc_address(pfn_glCreateShaderObjectARB,   "glCreateShaderObjectARB");
	gl_ctx.get_proc_address(pfn_glShaderSourceARB,         "glShaderSourceARB");
	gl_ctx.get_proc_address(pfn_glCompileShaderARB,        "glCompileShaderARB");
	gl_ctx.get_proc_address(pfn_glCreateProgramObjectARB,  "glCreateProgramObjectARB");
	gl_ctx.get_proc_address(pfn_glAttachObjectARB,         "glAttachObjectARB");
	gl_ctx.get_proc_address(pfn_glLinkProgramARB,          "glLinkProgramARB");
	gl_ctx.get_proc_address(pfn_glValidateProgramARB,      "glValidateProgramARB");
	gl_ctx.get_proc_address(pfn_glUseProgramObjectARB,     "glUseProgramObjectARB");
	gl_ctx.get_proc_address(pfn_glGetUniformLocationARB,   "glGetUniformLocationARB");
	gl_ctx.get_proc_address(pfn_glUniform1fARB,            "glUniform1fARB");
	gl_ctx.get_proc_address(pfn_glUniform1iARB,            "glUniform1iARB");
	gl_ctx.get_proc_address(pfn_glUniform1fvARB,           "glUniform1fvARB");
	gl_ctx.get_proc_address(pfn_glUniform2fvARB,           "glUniform2fvARB");
	gl_ctx.get_proc_address(pfn_glUniform3fvARB,           "glUniform3fvARB");
	gl_ctx.get_proc_address(pfn_glUniform4fvARB,           "glUniform4fvARB");
	gl_ctx.get_proc_address(pfn_glUniform1ivARB,           "glUniform1ivARB");
	gl_ctx.get_proc_address(pfn_glUniform2ivARB,           "glUniform2ivARB");
	gl_ctx.get_proc_address(pfn_glUniform3ivARB,           "glUniform3ivARB");
	gl_ctx.get_proc_address(pfn_glUniform4ivARB,           "glUniform4ivARB");

	if (pfn_glGetObjectParameterivARB && pfn_glGetInfoLogARB && pfn_glDeleteObjectARB && pfn_glCreateShaderObjectARB &&
			pfn_glShaderSourceARB && pfn_glCompileShaderARB && pfn_glCreateProgramObjectARB && pfn_glAttachObjectARB &&
			pfn_glLinkProgramARB && pfn_glValidateProgramARB && pfn_glUseProgramObjectARB &&
			pfn_glGetUniformLocationARB && pfn_glUniform1fARB && pfn_glUniform1iARB &&
			pfn_glUniform1fvARB && pfn_glUniform2fvARB && pfn_glUniform3fvARB && pfn_glUniform4fvARB &&
			pfn_glUniform1ivARB && pfn_glUniform2ivARB && pfn_glUniform3ivARB && pfn_glUniform4ivARB
		)
	{
		return true;
	}

	osd_printf_error("OpenGL: missing ARB shader function: ");
	if (!pfn_glGetObjectParameterivARB) osd_printf_error(" glGetObjectParameterivARB");
	if (!pfn_glGetInfoLogARB)           osd_printf_error(" glGetInfoLogARB");
	if (!pfn_glDeleteObjectARB)         osd_printf_error(" glDeleteObjectARB");
	if (!pfn_glCreateShaderObjectARB)   osd_printf_error(" glCreateShaderObjectARB");
	if (!pfn_glShaderSourceARB)         osd_printf_error(" glShaderSourceARB");
	if (!pfn_glCompileShaderARB)        osd_printf_error(" glCompileShaderARB");
	if (!pfn_glCreateProgramObjectARB)  osd_printf_error(" glCreateProgramObjectARB");
	if (!pfn_glAttachObjectARB)         osd_printf_error(" glAttachObjectARB");
	if (!pfn_glLinkProgramARB)          osd_printf_error(" glLinkProgramARB");
	if (!pfn_glValidateProgramARB)      osd_printf_error(" glValidateProgramARB");
	if (!pfn_glUseProgramObjectARB)     osd_printf_error(" glUseProgramObjectARB");
	if (!pfn_glGetUniformLocationARB)   osd_printf_error(" glGetUniformLocationARB");
	if (!pfn_glUniform1fARB)            osd_printf_error(" glUniform1fARB");
	if (!pfn_glUniform1iARB)            osd_printf_error(" glUniform1iARB");
	if (!pfn_glUniform1fvARB)           osd_printf_error(" glUniform1fvARB");
	if (!pfn_glUniform2fvARB)           osd_printf_error(" glUniform2fvARB");
	if (!pfn_glUniform3fvARB)           osd_printf_error(" glUniform3fvARB");
	if (!pfn_glUniform4fvARB)           osd_printf_error(" glUniform4fvARB");
	if (!pfn_glUniform1ivARB)           osd_printf_error(" glUniform1ivARB");
	if (!pfn_glUniform2ivARB)           osd_printf_error(" glUniform2ivARB");
	if (!pfn_glUniform3ivARB)           osd_printf_error(" glUniform3ivARB");
	if (!pfn_glUniform4ivARB)           osd_printf_error(" glUniform4ivARB");
	osd_printf_error("\n");

	return false;
}

int gl_shader_tool::check_error(GLSLCheckMode m, const char *file, const int line)
{
	GLenum const glerr = glGetError();
	if (GL_NO_ERROR != glerr)
	{
		if (CHECK_VERBOSE <= m)
			osd_printf_warning("%s:%d: GL Error: %d 0x%X\n", file, line, int(glerr), unsigned(glerr));
	}
	return (GL_NO_ERROR != glerr) ? glerr : 0;
}

int gl_shader_tool::check_error(GLhandleARB obj, GLenum obj_query, GLSLCheckMode m, const char *file, const int line)
{
	GLenum const glerr = check_error(m, file, line);
	if (!obj)
		return glerr;

	if(obj_query != GL_OBJECT_TYPE_ARB            &&
		obj_query != GL_OBJECT_DELETE_STATUS_ARB   &&
		obj_query != GL_OBJECT_COMPILE_STATUS_ARB  &&
		obj_query != GL_OBJECT_LINK_STATUS_ARB     &&
		obj_query != GL_OBJECT_VALIDATE_STATUS_ARB
		)
	{
		osd_printf_warning("%s:%d: GL Error: gl_shader_check_error unsupported object query 0x%X\n", file, line, (unsigned int)obj_query);
		return -1;
	}

	GLint param;
	pfn_glGetObjectParameterivARB(obj, obj_query, &param);

	int res = 0;
	switch(obj_query)
	{
	case GL_OBJECT_TYPE_ARB:
		if (param != GL_PROGRAM_OBJECT_ARB && param != GL_SHADER_OBJECT_ARB)
		{
			if ( CHECK_VERBOSE <= m )
				osd_printf_warning("%s:%d: GL Error: object type 0x%X generation failed\n", file, line, (unsigned int)(uintptr_t)obj);
			res=-1;
		}
		else if (CHECK_ALWAYS_VERBOSE <= m)
		{
			if (param==GL_PROGRAM_OBJECT_ARB)
				osd_printf_warning("%s:%d: GL Error: object type 0x%X is PROGRAM, successful\n", file, line, (unsigned int)(uintptr_t)obj);
			else
				osd_printf_warning("%s:%d: GL Info: object type 0x%X is SHADER, successful\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		break;
	case GL_OBJECT_DELETE_STATUS_ARB:
		if (param != 1)
		{
			if (CHECK_ALWAYS_VERBOSE <= m)
				osd_printf_warning("%s:%d: GL Info: object 0x%X not yet marked for deletion\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		else if (CHECK_ALWAYS_VERBOSE <= m)
		{
			osd_printf_warning("%s:%d: GL Info: object 0x%X deletion successful\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		break;
	case GL_OBJECT_COMPILE_STATUS_ARB:
		if (param != 1)
		{
			if (CHECK_VERBOSE <= m)
				osd_printf_warning("%s:%d: GL Error: object 0x%X compilation failed\n", file, line, (unsigned int)(uintptr_t)obj);
			res = -1;
		}
		else if (CHECK_ALWAYS_VERBOSE <= m)
		{
			osd_printf_warning("%s:%d: GL Info: object 0x%X compiled successful\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		break;
	case GL_OBJECT_LINK_STATUS_ARB:
		if (param != 1)
		{
			if (CHECK_VERBOSE <= m)
				osd_printf_warning("%s:%d: GL Error: object 0x%X linking failed\n", file, line, (unsigned int)(uintptr_t)obj);
			res = -1;
		}
		else if (CHECK_ALWAYS_VERBOSE <= m)
		{
			osd_printf_warning("%s:%d: GL Info: object 0x%X linked successful\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		break;
	case GL_OBJECT_VALIDATE_STATUS_ARB:
		if (param != 1)
		{
			if (CHECK_VERBOSE <= m)
				osd_printf_warning("%s:%d: GL Error: object 0x%X validation failed\n", file, line, (unsigned int)(uintptr_t)obj);
			res = -1;
		}
		else if (CHECK_ALWAYS_VERBOSE <= m
				)
		{
			osd_printf_warning("%s:%d: GL Info: object 0x%X validation successful\n", file, line, (unsigned int)(uintptr_t)obj);
		}
		break;
	}

	if (res < 0 || CHECK_ALWAYS_VERBOSE <= m)
	{
		GLsizei length;
		GLcharARB buffer[255];
		length=0;
		pfn_glGetInfoLogARB(obj, sizeof(buffer), &length, buffer);
		if (length>0)
			osd_printf_warning("%s:%d glInfoLog: %s\n", file, line, buffer);
	}

	(void)glGetError(); // ;-)

	return res;
}

int gl_round_to_pow2(int v)
{
	if (v & (v - 1))
	{
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v++;
	}
	return v;
}

int gl_shader_tool::texture_check_size(
		GLenum target,
		GLint level,
		GLint internalFormat,
		GLsizei width,
		GLsizei height,
		GLint border,
		GLenum format,
		GLenum type,
		GLint *avail_width,
		GLint *avail_height,
		bool verbose)
{
	if (!avail_width || !avail_height)
		return -1;

	*avail_width  = 0;
	*avail_height = 0;

	GL_CHECK_ERROR_QUIET();

	// Test the max texture size
	int err = 1;
	GLenum const texTargetProxy = (target == GL_TEXTURE_RECTANGLE_ARB) ? GL_PROXY_TEXTURE_RECTANGLE_ARB : GL_PROXY_TEXTURE_2D;
	while (err && width>=1 && height>=1 /* && width>=64 && height>=64 */)
	{
		glTexImage2D(texTargetProxy, level,
				internalFormat,
				width, height,
				border, format, type, nullptr);
		if (0 != (err=GL_CHECK_ERROR_NORMAL()))
			return err;

		glGetTexLevelParameteriv(texTargetProxy, level, GL_TEXTURE_WIDTH,  avail_width);
		glGetTexLevelParameteriv(texTargetProxy, level, GL_TEXTURE_HEIGHT, avail_height);
		if (0 != (err=GL_CHECK_ERROR_NORMAL()))
			return err;

		if ((*avail_width != width) || (*avail_height != height))
		{
			err = 1;

			if (verbose)
			{
				osd_printf_warning("gl_texture_size_check: "
						"TexImage2D(0x%X, %d, 0x%X, %d, %d, %d, 0x%X, 0x%X): returned size does not match: %dx%d\n",
						(unsigned int)target, (int)level, (int)internalFormat,
						width, height, border, (unsigned int)format, (unsigned int)type,
						*avail_width, *avail_height);
			}

			if (*avail_width == width)
				height /= 2;
			else if (*avail_height == height)
				width /= 2;
			else if (width > height)
				width /= 2;
			else
				height /= 2;
			if (verbose)
			{
				osd_printf_verbose("gl_texture_size_check: trying [%dx%d] !\n", height, width);
			}
		}
		else
		{
			err = 0;
		}
	}
	if (!err)
	{
		*avail_width = width;
		*avail_height = height;
	}
	return err;
}

int gl_shader_tool::delete_shader_tool(
		GLhandleARB *program,
		GLhandleARB *vertex_shader,
		GLhandleARB *fragment_shader,
		bool externalcall)
{
	int res = 0;

	GL_CHECK_ERROR_QUIET();
	if (program && *program!=0)
	{
		pfn_glDeleteObjectARB(*program);
		if (externalcall)
			res |= GL_SHADER_CHECK(*program, GL_OBJECT_DELETE_STATUS_ARB);
		*program = 0;
	}
	if (vertex_shader && *vertex_shader!=0)
	{
		pfn_glDeleteObjectARB(*vertex_shader);
		if (externalcall)
			res |= GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_DELETE_STATUS_ARB);
		*vertex_shader=0;
	}
	if (fragment_shader && *fragment_shader!=0)
	{
		pfn_glDeleteObjectARB(*fragment_shader);
		if (externalcall)
			res |= GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_DELETE_STATUS_ARB);
		*fragment_shader=0;
	}

	return res;
}

int gl_shader_tool::compile_source(
		GLhandleARB *shader,
		GLenum type,
		const char *shader_source,
		bool verbose)
{
	int err = 0;

	if (!shader || !shader_source)
	{
		if (!shader)
			osd_printf_warning("error: gl_compile_shader_source: nullptr shader passed\n");
		if (!shader_source)
			osd_printf_warning("error: gl_compile_shader_source: nullptr shader source passed\n");
		return -1;
	}
	*shader = pfn_glCreateShaderObjectARB(type);
	err = GL_SHADER_CHECK(*shader, GL_OBJECT_TYPE_ARB);
	if (err)
		goto errout;

	pfn_glShaderSourceARB(*shader, 1, (const GLcharARB **)&shader_source, nullptr);

	pfn_glCompileShaderARB(*shader);
	err = GL_SHADER_CHECK(*shader, GL_OBJECT_COMPILE_STATUS_ARB);
	if (err)
		goto errout;

	if (verbose)
		osd_printf_warning( "<%s>\n", shader_source);

	return 0;

errout:
	if (*shader != 0)
		pfn_glDeleteObjectARB(*shader);

	osd_printf_warning("failed to process shader: <%s>\n", shader_source);
	return err;
}

int gl_shader_tool::compile_file(
		GLhandleARB *shader,
		GLenum type,
		const char *shader_file,
		bool verbose)
{
	if (!shader || !shader_file)
	{
		if (!shader)
			osd_printf_warning("error: gl_compile_shader_source: nullptr shader passed\n");
		if (!shader_file)
			osd_printf_warning("error: gl_compile_shader_source: nullptr shader file passed\n");
		return -1;
	}

	FILE *const file = fopen(shader_file, "r");
	if (!file)
	{
		osd_printf_warning("cannot open shader_file: %s\n", shader_file);
		return -1;
	}

	// get the real file size
	fseek(file, 0, SEEK_END);
	int const buffer_len = (int)ftell(file);
	fseek(file, 0, SEEK_SET);

	auto *const buffer = (GLcharARB *)malloc(buffer_len + 1);
	memset(buffer, 0, buffer_len + 1);

	/* Load Shader Sources */
	for (int i = 0, c = 0; i<buffer_len && EOF!=(c=fgetc(file)); i++)
		buffer[i] = char(c);
	fclose(file);

	int const err = compile_source(shader, type, buffer, verbose);
	free(buffer);
	if (err)
		goto errout;

	if (verbose)
		osd_printf_warning("shader file: %s\n", shader_file);

	return 0;

errout:
	osd_printf_warning("failed to process shader_file: %s\n", shader_file);
	return err;
}


int gl_shader_tool::compile_files(
		GLhandleARB *program,
		GLhandleARB *vertex_shader,
		GLhandleARB *fragment_shader,
		const char * vertex_shader_file,
		const char * fragment_shader_file,
		bool verbose)
{
	int err;

	if (!program)
	{
		err = -1;
		osd_printf_warning("no program ptr passed\n");
		goto errout;
	}
	if (!vertex_shader)
	{
		err = -1;
		osd_printf_warning("no vertex_shader ptr passed\n");
		goto errout;
	}
	if (!fragment_shader)
	{
		err = -1;
		osd_printf_warning("no fragment_shader ptr passed\n");
		goto errout;
	}

	*program = pfn_glCreateProgramObjectARB();
	err=GL_SHADER_CHECK(*program, GL_OBJECT_TYPE_ARB);
	if (err)
		goto errout;

	if (!vertex_shader_file)
	{
		if (!(*vertex_shader))
		{
			err = -1;
			osd_printf_warning("no vertex_shader_file, nor vertex_shader id passed\n");
			goto errout;
		}
		err = GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_TYPE_ARB);
		if (err)
			goto errout;
	}
	else
	{
		err = compile_file(vertex_shader, GL_VERTEX_SHADER_ARB, vertex_shader_file, verbose);
		if (err)
			return err;
	}
	pfn_glAttachObjectARB(*program, *vertex_shader);

	if (!fragment_shader_file)
	{
		if (!(*fragment_shader))
		{
			err = -1;
			osd_printf_warning("no fragment_shader_file, nor fragment_shader id passed\n");
			goto errout;
		}
		err = GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_TYPE_ARB);
		if (err)
			goto errout;
	}
	else
	{
		err = compile_file(fragment_shader, GL_FRAGMENT_SHADER_ARB, fragment_shader_file, verbose);
		if (err)
			return err;
	}
	pfn_glAttachObjectARB(*program, *fragment_shader);

	pfn_glLinkProgramARB(*program);
	err = GL_SHADER_CHECK(*program, GL_OBJECT_LINK_STATUS_ARB);
	if (err)
		goto errout;

	pfn_glValidateProgramARB(*program);
	err = GL_SHADER_CHECK(*program, GL_OBJECT_VALIDATE_STATUS_ARB);
	if (err)
		goto errout;

	return 0;

errout:
	delete_shader_tool(program, vertex_shader, fragment_shader, false);
	return err;

}

int gl_shader_tool::compile_sources(
		GLhandleARB *program,
		GLhandleARB *vertex_shader,
		GLhandleARB *fragment_shader,
		const GLcharARB * vertex_shader_source,
		const GLcharARB * fragment_shader_source)
{
	int err = 0;

	if (!program)
	{
		err = -1;
		osd_printf_warning("no program ptr passed\n");
		goto errout;
	}
	if (!vertex_shader)
	{
		err = -1;
		osd_printf_warning("no vertex_shader ptr passed\n");
		goto errout;
	}
	if (!fragment_shader)
	{
		err = -1;
		osd_printf_warning("no fragment_shader ptr passed\n");
		goto errout;
	}

	*program = pfn_glCreateProgramObjectARB();
	err = GL_SHADER_CHECK(*program, GL_OBJECT_TYPE_ARB);
	if (err)
		goto errout;

	if (!vertex_shader_source)
	{
		if (!(*vertex_shader))
		{
			err = -1;
			osd_printf_warning("no vertex_shader_source nor a vertex_shader id passed\n");
			goto errout;
		}
		err = GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_TYPE_ARB);
		if (err)
			goto errout;
	}
	if (!fragment_shader_source)
	{
		if (!(*fragment_shader))
		{
			err = -1;
			osd_printf_warning("no fragment_shader_source nor a fragment_shader id passed\n");
			goto errout;
		}
		err = GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_TYPE_ARB);
		if (err)
			goto errout;
	}

	if (vertex_shader_source)
	{
		err = compile_source(vertex_shader, GL_VERTEX_SHADER_ARB, vertex_shader_source, false);
		if (err)
			goto errout;
	}
	pfn_glAttachObjectARB(*program, *vertex_shader);

	if (fragment_shader_source)
	{
		err = compile_source(fragment_shader, GL_FRAGMENT_SHADER_ARB, fragment_shader_source, false);
		if (err)
			goto errout;
	}
	pfn_glAttachObjectARB(*program, *fragment_shader);

	pfn_glLinkProgramARB(*program);
	err = GL_SHADER_CHECK(*program, GL_OBJECT_LINK_STATUS_ARB);
	if (err)
	{
		osd_printf_warning("failed to link program\n");
		osd_printf_warning("vertex shader: <%s>\n", vertex_shader_source);
		osd_printf_warning("fragment shader: <%s>\n", fragment_shader_source);
		goto errout;
	}

	pfn_glValidateProgramARB(*program);
	err = GL_SHADER_CHECK(*program, GL_OBJECT_VALIDATE_STATUS_ARB);
	if (err)
	{
		osd_printf_warning("failed to validate program\n");
		osd_printf_warning("vertex shader: <%s>\n", vertex_shader_source);
		osd_printf_warning("fragment shader: <%s>\n", fragment_shader_source);
		goto errout;
	}

	return 0;

errout:
	delete_shader_tool(program, vertex_shader, fragment_shader, false);
	return err;
}


int gl_shader_tool::delete_shader(
		GLhandleARB *program,
		GLhandleARB *vertex_shader,
		GLhandleARB *fragment_shader)
{
	return delete_shader_tool(program, vertex_shader, fragment_shader, true);
}
