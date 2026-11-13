// license:BSD-3-Clause
// copyright-holders:R. Belmont
//============================================================
//
//  macglcontext.h - Mac-specific GL context
//
//  Mac OSD by R. Belmont
//
//============================================================

#ifndef MAME_OSD_MAC_MACGLCONTEXT_H
#define MAME_OSD_MAC_MACGLCONTEXT_H

#pragma once

#include "modules/opengl/osd_opengl.h"

// implemented in macglcontext.mm
extern "C" {
void *MacGLContextCreate(void *wincontroller);
void MacGLContextDestroy(void *context);
void MacGLContextMakeCurrent(void *context);
void MacGLContextSwapBuffers(void *context);
bool MacGLContextSetSwapInterval(void *context, int interval);
void *MacGLGetProcAddress(const char *proc);
}

class mac_gl_context : public osd_gl_context
{
public:
	mac_gl_context(void *wincontroller) : m_context(MacGLContextCreate(wincontroller))
	{
	}

	virtual ~mac_gl_context()
	{
		if (m_context)
		{
			MacGLContextDestroy(m_context);
		}
	}

	virtual explicit operator bool() const override
	{
		return m_context != nullptr;
	}

	virtual void make_current() override
	{
		MacGLContextMakeCurrent(m_context);
	}

	virtual bool set_swap_interval(const int swap) override
	{
		return MacGLContextSetSwapInterval(m_context, swap);
	}

	virtual const char *last_error_message() override
	{
		if (!m_context)
		{
			return "Could not create an NSOpenGLContext for the window";
		}
		return nullptr;
	}

	virtual void *get_proc_address(const char *proc) override
	{
		return MacGLGetProcAddress(proc);
	}

	virtual void swap_buffer() override
	{
		MacGLContextSwapBuffers(m_context);
	}

private:
	void *m_context;
};

#endif // MAME_OSD_MAC_MACGLCONTEXT_H
