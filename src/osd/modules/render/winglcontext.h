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

class win_gl_context : public osd_gl_context
{
public:
	win_gl_context(HWND window) : osd_gl_context(), m_context(0), m_window(NULL), m_hdc(0)
	{
		m_error[0] = 0;

		this->pfn_wglGetProcAddress = (PROC (WINAPI *)(LPCSTR lpszProc)) GetProcAddress(m_module, "wglGetProcAddress");
		this->pfn_wglCreateContext = (HGLRC (WINAPI *)(HDC hdc)) GetProcAddress(m_module, "wglCreateContext");
		this->pfn_wglDeleteContext = (BOOL (WINAPI *)(HGLRC hglrc)) GetProcAddress(m_module, "wglDeleteContext");
		this->pfn_wglMakeCurrent = (BOOL (WINAPI *)(HDC hdc, HGLRC hglrc)) GetProcAddress(m_module, "wglMakeCurrent");

		this->pfn_wglGetExtensionsStringEXT = (const char *(WINAPI *) (void)) pfn_wglGetProcAddress("wglGetExtensionsStringEXT");

		if (WGLExtensionSupported("WGL_EXT_swap_control"))
		{
			this->pfn_wglSwapIntervalEXT = (BOOL (WINAPI *) (int)) getProcAddress("wglSwapIntervalEXT");
			this->pfn_wglGetSwapIntervalEXT = (int (WINAPI *) (void)) getProcAddress("wglGetSwapIntervalEXT");
		}
		else
		{
			pfn_wglSwapIntervalEXT = NULL;
			pfn_wglGetSwapIntervalEXT = NULL;
		}

		m_hdc = GetDC(window);
		if (!setupPixelFormat(m_hdc))
		{
			m_context = this->pfn_wglCreateContext(m_hdc);
			if  (!m_context)
			{
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, m_error, 255, NULL);
				return;
			}
			this->pfn_wglMakeCurrent(m_hdc, m_context);
		}
	}

	virtual ~win_gl_context()
	{
		this->pfn_wglDeleteContext(m_context);
		ReleaseDC(m_window, m_hdc);
	}

	virtual void MakeCurrent() override
	{
		this->pfn_wglMakeCurrent(m_hdc, m_context);
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
		void *ret = (void *) GetProcAddress(m_module, proc);
		if (ret == NULL)
			ret = (void *) this->pfn_wglGetProcAddress(proc);
		return ret;
	}

	virtual int SetSwapInterval(const int swap) override
	{
		if (this->pfn_wglSwapIntervalEXT != NULL)
		{
			this->pfn_wglSwapIntervalEXT(swap ? 1 : 0);
		}
		return 0;
	}

	virtual void SwapBuffer() override
	{
		SwapBuffers(m_hdc);
		//wglSwapLayerBuffers(GetDC(window().m_hwnd), WGL_SWAP_MAIN_PLANE);
	}

	static void load_library()
	{
		m_module = LoadLibraryA("opengl32.dll");
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
		int pixelFormat;

		pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0) {
			strcpy(m_error, "ChoosePixelFormat failed");
			return 1;
		}

		if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
			strcpy(m_error, "SetPixelFormat failed.");
			return 1;
		}
		return 0;
	}

	bool WGLExtensionSupported(const char *extension_name)
	{
		//if (pfn_wglGetExtensionsStringEXT != NULL)
		//  printf("%s\n", this->pfn_wglGetExtensionsStringEXT());

		if (pfn_wglGetExtensionsStringEXT != NULL && strstr(pfn_wglGetExtensionsStringEXT(), extension_name) != NULL)
			return true;
		else
			return false;
	}

	HGLRC m_context;
	HWND m_window;
	HDC m_hdc;
	char m_error[256];

	PROC (WINAPI *pfn_wglGetProcAddress)(LPCSTR lpszProc);
	HGLRC (WINAPI *pfn_wglCreateContext)(HDC hdc);
	BOOL (WINAPI *pfn_wglDeleteContext)(HGLRC hglrc);
	BOOL (WINAPI *pfn_wglMakeCurrent)(HDC hdc, HGLRC hglrc);

	const char *(WINAPI *pfn_wglGetExtensionsStringEXT) (void);
	BOOL (WINAPI *pfn_wglSwapIntervalEXT) (int interval);
	int (WINAPI * pfn_wglGetSwapIntervalEXT) (void);

	static HMODULE m_module;
};

#endif // __WIN_GL_CONTEXT__
