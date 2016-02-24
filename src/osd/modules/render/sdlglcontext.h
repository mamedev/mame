// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlglcontext.h - SDL-specific GL context
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#pragma once

#ifndef __SDL_GL_CONTEXT__
#define __SDL_GL_CONTEXT__

#include "modules/opengl/osd_opengl.h"

class sdl_gl_context : public osd_gl_context
{
public:
	sdl_gl_context(SDL_Window *window) : osd_gl_context(), m_context(0), m_window(window)
	{
		m_error[0] = 0;
		m_context = SDL_GL_CreateContext(window);
		if  (!m_context)
		{
			snprintf(m_error,255, "OpenGL not supported on this driver: %s", SDL_GetError());
		}
	}
	virtual ~sdl_gl_context()
	{
		SDL_GL_DeleteContext(m_context);
	}
	virtual void MakeCurrent() override
	{
		SDL_GL_MakeCurrent(m_window, m_context);
	}

	virtual int SetSwapInterval(const int swap) override
	{
		return SDL_GL_SetSwapInterval(swap);
	}

	virtual const char *LastErrorMsg() override
	{
		if (m_error[0] == 0)
			return NULL;
		else
			return m_error;
	}
	virtual void *getProcAddress(const char *proc) override
	{
		return SDL_GL_GetProcAddress(proc);
	}

	virtual void SwapBuffer() override
	{
		SDL_GL_SwapWindow(m_window);
	}

private:
	SDL_GLContext m_context;
	SDL_Window *m_window;
	char m_error[256];
};

#endif // __SDL_GL_CONTEXT__
