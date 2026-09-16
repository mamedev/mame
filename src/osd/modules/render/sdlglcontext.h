// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlglcontext.h - SDL-specific GL context
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef MAME_RENDER_SDLGLCONTEXT_H
#define MAME_RENDER_SDLGLCONTEXT_H

#pragma once

#include "modules/opengl/osd_opengl.h"

#include "strformat.h"

#ifdef SDLMAME_SDL3
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <string>


class sdl_gl_context : public osd_gl_context
{
public:
	sdl_gl_context(SDL_Window *window) : m_context(0), m_window(window)
	{
		m_context = SDL_GL_CreateContext(window);
		if (!m_context)
		{
			try { m_error = util::string_format("OpenGL not supported on this driver: %s", SDL_GetError()); }
			catch (...) { m_error.clear(); }
		}
	}

	virtual ~sdl_gl_context()
	{
		if (m_context)
		{
#ifdef SDLMAME_SDL3
			SDL_GL_DestroyContext(m_context);
#else
			SDL_GL_DeleteContext(m_context);
#endif
		}
	}

	virtual explicit operator bool() const override
	{
		return bool(m_context);
	}

	virtual void make_current() override
	{
		SDL_GL_MakeCurrent(m_window, m_context);
	}

	virtual bool set_swap_interval(const int swap) override
	{
		return 0 == SDL_GL_SetSwapInterval(swap);
	}

	virtual const char *last_error_message() override
	{
		if (!m_error.empty())
			return m_error.c_str();
		else
			return nullptr;
	}

	virtual void *get_proc_address(const char *proc) override
	{
		return (void *)SDL_GL_GetProcAddress(proc);
	}

	virtual void swap_buffer() override
	{
		SDL_GL_SwapWindow(m_window);
	}

private:
	SDL_GLContext m_context;
	SDL_Window *const m_window;
	std::string m_error;
};

#endif // MAME_RENDER_SDLGLCONTEXT_H
