// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  winglcontext.h - Windows-specific GL context
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================
#ifndef MAME_RENDER_WINGLCONTEXT_H
#define MAME_RENDER_WINGLCONTEXT_H

#pragma once

#include "modules/opengl/osd_opengl.h"
#include "modules/lib/osdlib.h"

#include "strconv.h"

#include <cstring>
#include <string>


class win_gl_context : public osd_gl_context
{
public:
	using osd_gl_context::get_proc_address;

	win_gl_context(HWND window) : m_context(nullptr), m_window(window), m_hdc(nullptr)
	{
		// open DLL and bind required functions
		opengl32_dll = osd::dynamic_module::open({ "opengl32.dll" });
		pfn_wglGetProcAddress = opengl32_dll->bind<wglGetProcAddress_fn>("wglGetProcAddress");
		pfn_wglCreateContext = opengl32_dll->bind<wglCreateContext_fn>("wglCreateContext");
		pfn_wglDeleteContext = opengl32_dll->bind<wglDeleteContext_fn>("wglDeleteContext");
		pfn_wglMakeCurrent = opengl32_dll->bind<wglMakeCurrent_fn>("wglMakeCurrent");
		if (!pfn_wglGetProcAddress || !pfn_wglCreateContext || !pfn_wglDeleteContext || !pfn_wglMakeCurrent)
			return;

		m_hdc = GetDC(window);
		if (!m_hdc)
		{
			get_last_error_string();
			return;
		}

		if (setupPixelFormat())
		{
			m_context = (*pfn_wglCreateContext)(m_hdc);
			if (!m_context)
				get_last_error_string();
		}
		if (!m_context)
			return;

		(*pfn_wglMakeCurrent)(m_hdc, m_context);

		get_proc_address(pfn_wglGetExtensionsStringEXT, "wglGetExtensionsStringEXT");
		if (WGLExtensionSupported("WGL_EXT_swap_control"))
		{
			get_proc_address(pfn_wglSwapIntervalEXT, "wglSwapIntervalEXT");
			get_proc_address(pfn_wglGetSwapIntervalEXT, "wglGetSwapIntervalEXT");
		}
		else
		{
			pfn_wglSwapIntervalEXT = nullptr;
			pfn_wglGetSwapIntervalEXT = nullptr;
		}
	}

	virtual ~win_gl_context()
	{
		if (m_context)
			(*pfn_wglDeleteContext)(m_context);

		if (m_hdc)
			ReleaseDC(m_window, m_hdc);
	}

	virtual explicit operator bool() const override
	{
		return bool(m_context);
	}

	virtual void make_current() override
	{
		(*pfn_wglMakeCurrent)(m_hdc, m_context);
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
		return reinterpret_cast<void *>(uintptr_t((*pfn_wglGetProcAddress)(proc)));
	}

	virtual bool set_swap_interval(const int swap) override
	{
		if (!pfn_wglSwapIntervalEXT)
			return false;

		pfn_wglSwapIntervalEXT(swap ? 1 : 0);
		return true;
	}

	virtual void swap_buffer() override
	{
		SwapBuffers(m_hdc);
		//wglSwapLayerBuffers(GetDC(window().m_hwnd), WGL_SWAP_MAIN_PLANE);
	}

private:
	// Typedefs for dynamically loaded functions
	typedef PROC (WINAPI *wglGetProcAddress_fn)(LPCSTR);
	typedef HGLRC (WINAPI *wglCreateContext_fn)(HDC);
	typedef BOOL (WINAPI *wglDeleteContext_fn)(HGLRC);
	typedef BOOL (WINAPI *wglMakeCurrent_fn)(HDC, HGLRC);

	typedef const char * (WINAPI *wglGetExtensionsStringEXT_fn)();
	typedef BOOL (WINAPI *wglSwapIntervalEXT_fn)(int);
	typedef int (WINAPI *wglGetSwapIntervalEXT_fn)();

	bool setupPixelFormat()
	{
		PIXELFORMATDESCRIPTOR pfd = {
				sizeof(PIXELFORMATDESCRIPTOR),  // size
				1,                              // version
				PFD_SUPPORT_OPENGL |
				PFD_DRAW_TO_WINDOW |
				PFD_DOUBLEBUFFER,               // support double-buffering
				PFD_TYPE_RGBA,                  // color type
				32,                             // prefered color depth
				0, 0, 0, 0, 0, 0,               // color bits (ignored)
				0,                              // no alpha buffer
				0,                              // alpha bits (ignored)
				0,                              // no accumulation buffer
				0, 0, 0, 0,                     // accum bits (ignored)
				16,                             // depth buffer
				0,                              // no stencil buffer
				0,                              // no auxiliary buffers
				PFD_MAIN_PLANE,                 // main layer
				0,                              // reserved
				0, 0, 0,                        // no layer, visible, damage masks
			};

		const int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
		if (pixelFormat == 0)
		{
			get_last_error_string();
			return false;
		}

		if (SetPixelFormat(m_hdc, pixelFormat, &pfd) != TRUE)
		{
			get_last_error_string();
			return false;
		}

		return true;
	}

	bool WGLExtensionSupported(const char *extension_name)
	{
		if (!pfn_wglGetExtensionsStringEXT)
			return false;

		return strstr(pfn_wglGetExtensionsStringEXT(), extension_name) != nullptr;
	}

	void get_last_error_string()
	{
		LPTSTR buffer = nullptr;
		const auto result = FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
				nullptr,
				GetLastError(),
				0,
				LPTSTR(&buffer),
				0,
				nullptr);
		if (!result || !buffer)
		{
			m_error.clear();
			return;
		}
		try { m_error = osd::text::from_tstring(buffer); }
		catch (...) { m_error.clear(); }
		LocalFree(buffer);
	}

	HGLRC m_context;
	HWND const m_window;
	HDC m_hdc;
	std::string m_error;

	osd::dynamic_module::ptr     opengl32_dll;
	wglGetProcAddress_fn         pfn_wglGetProcAddress;
	wglCreateContext_fn          pfn_wglCreateContext;
	wglDeleteContext_fn          pfn_wglDeleteContext;
	wglMakeCurrent_fn            pfn_wglMakeCurrent;

	wglGetExtensionsStringEXT_fn pfn_wglGetExtensionsStringEXT;
	wglSwapIntervalEXT_fn        pfn_wglSwapIntervalEXT;
	wglGetSwapIntervalEXT_fn     pfn_wglGetSwapIntervalEXT;
};

#endif // MAME_RENDER_WINGLCONTEXT_H
