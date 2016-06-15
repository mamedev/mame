// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  winglcontext.h - Windows-specific GL context
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#pragma once

#ifndef __WIN_GL_CONTEXT__
#define __WIN_GL_CONTEXT__

#include "modules/opengl/osd_opengl.h"
#include "modules/lib/osdlib.h"

// Typedefs for dynamically loaded functions
typedef PROC WINAPI (*wglGetProcAddress_fn)(LPCSTR);
typedef HGLRC WINAPI (*wglCreateContext_fn)(HDC);
typedef BOOL WINAPI (*wglDeleteContext_fn)(HGLRC);
typedef BOOL WINAPI (*wglMakeCurrent_fn)(HDC, HGLRC);

typedef const char * WINAPI (*wglGetExtensionsStringEXT_fn)(void);
typedef BOOL WINAPI (*wglSwapIntervalEXT_fn)(int);
typedef int WINAPI (*wglGetSwapIntervalEXT_fn)(void);

class win_gl_context : public osd_gl_context
{
public:
	win_gl_context(HWND window) : osd_gl_context(), m_context(0), m_window(nullptr), m_hdc(0)
	{
		m_error[0] = 0;

		opengl32_dll = osd::dynamic_module::open({ "opengl32.dll" });

		pfn_wglGetProcAddress = opengl32_dll->bind<wglGetProcAddress_fn>("wglGetProcAddress");
		pfn_wglCreateContext = opengl32_dll->bind<wglCreateContext_fn>("wglCreateContext");
		pfn_wglDeleteContext = opengl32_dll->bind<wglDeleteContext_fn>("wglDeleteContext");
		pfn_wglMakeCurrent = opengl32_dll->bind<wglMakeCurrent_fn>("wglMakeCurrent");

		if (pfn_wglGetProcAddress == nullptr || pfn_wglCreateContext == nullptr ||
			pfn_wglDeleteContext == nullptr || pfn_wglMakeCurrent == nullptr)
		{
			return;
		}

		pfn_wglGetExtensionsStringEXT = (wglGetExtensionsStringEXT_fn)(*pfn_wglGetProcAddress)("wglGetExtensionsStringEXT");

		if (WGLExtensionSupported("WGL_EXT_swap_control"))
		{
			pfn_wglSwapIntervalEXT = (BOOL (WINAPI *) (int)) getProcAddress("wglSwapIntervalEXT");
			pfn_wglGetSwapIntervalEXT = (int (WINAPI *) (void)) getProcAddress("wglGetSwapIntervalEXT");
		}
		else
		{
			pfn_wglSwapIntervalEXT = nullptr;
			pfn_wglGetSwapIntervalEXT = nullptr;
		}

		m_hdc = GetDC(window);
		if (!setupPixelFormat(m_hdc))
		{
			m_context = (*pfn_wglCreateContext)(m_hdc);
			if (!m_context)
			{
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), 0, m_error, 255, nullptr);
				return;
			}
			(*pfn_wglMakeCurrent)(m_hdc, m_context);
		}
	}

	virtual ~win_gl_context()
	{
		(*pfn_wglDeleteContext)(m_context);
		ReleaseDC(m_window, m_hdc);
	}

	virtual void MakeCurrent() override
	{
		(*pfn_wglMakeCurrent)(m_hdc, m_context);
	}

	virtual const char *LastErrorMsg() override
	{
		if (m_error[0] == 0)
			return nullptr;
		else
			return m_error;
	}

	virtual void *getProcAddress(const char *proc) override
	{
		return (void *)(*pfn_wglGetProcAddress)(proc);
	}

	virtual int SetSwapInterval(const int swap) override
	{
		if (pfn_wglSwapIntervalEXT != nullptr)
		{
			pfn_wglSwapIntervalEXT(swap ? 1 : 0);
		}
		return 0;
	}

	virtual void SwapBuffer() override
	{
		SwapBuffers(m_hdc);
		//wglSwapLayerBuffers(GetDC(window().m_hwnd), WGL_SWAP_MAIN_PLANE);
	}

private:

	int setupPixelFormat(HDC hDC)
	{
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),  /* size */
			1,                              /* version */
			PFD_SUPPORT_OPENGL |
			PFD_DRAW_TO_WINDOW |
			PFD_DOUBLEBUFFER,               /* support double-buffering */
			PFD_TYPE_RGBA,                  /* color type */
			32,                             /* prefered color depth */
			0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
			0,                              /* no alpha buffer */
			0,                              /* alpha bits (ignored) */
			0,                              /* no accumulation buffer */
			0, 0, 0, 0,                     /* accum bits (ignored) */
			16,                             /* depth buffer */
			0,                              /* no stencil buffer */
			0,                              /* no auxiliary buffers */
			PFD_MAIN_PLANE,                 /* main layer */
			0,                              /* reserved */
			0, 0, 0,                        /* no layer, visible, damage masks */
		};

		int pixelFormat = ChoosePixelFormat(hDC, &pfd);

		if (pixelFormat == 0)
		{
			strcpy(m_error, "ChoosePixelFormat failed.");
			return 1;
		}

		if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
		{
			strcpy(m_error, "SetPixelFormat failed.");
			return 1;
		}
		return 0;
	}

	bool WGLExtensionSupported(const char *extension_name)
	{
		if (pfn_wglGetExtensionsStringEXT == nullptr)
			return false;
		
		//  printf("%s\n", pfn_wglGetExtensionsStringEXT());

		if (strstr(pfn_wglGetExtensionsStringEXT(), extension_name) != nullptr)
			return true;
		else
			return false;
	}

	HGLRC m_context;
	HWND m_window;
	HDC m_hdc;
	char m_error[256];

	osd::dynamic_module::ptr     opengl32_dll;
	wglGetProcAddress_fn         pfn_wglGetProcAddress;
	wglCreateContext_fn          pfn_wglCreateContext;
	wglDeleteContext_fn          pfn_wglDeleteContext;
	wglMakeCurrent_fn            pfn_wglMakeCurrent;

	wglGetExtensionsStringEXT_fn pfn_wglGetExtensionsStringEXT;
	wglSwapIntervalEXT_fn        pfn_wglSwapIntervalEXT;
	wglGetSwapIntervalEXT_fn     pfn_wglGetSwapIntervalEXT;
};

#endif // __WIN_GL_CONTEXT__
