//============================================================
//
//  d3d9intf.c - Direct3D 9 abstraction layer
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#undef interface

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "d3dintf.h"
#include "winmain.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef IDirect3D9 *(WINAPI *direct3dcreate9_ptr)(UINT SDKVersion);

typedef HRESULT (WINAPI *direct3dx9_loadeffect_ptr)(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors);
static direct3dx9_loadeffect_ptr g_load_effect = NULL;


//============================================================
//  PROTOTYPES
//============================================================

static void set_interfaces(d3d_base *d3dptr);

//============================================================
//  INLINES
//============================================================

INLINE void convert_present_params(const d3d_present_parameters *params, D3DPRESENT_PARAMETERS *d3d9params)
{
	memset(d3d9params, 0, sizeof(*d3d9params));
	d3d9params->BackBufferWidth = params->BackBufferWidth;
	d3d9params->BackBufferHeight = params->BackBufferHeight;
	d3d9params->BackBufferFormat = params->BackBufferFormat;
	d3d9params->BackBufferCount = params->BackBufferCount;
	d3d9params->MultiSampleType = params->MultiSampleType;
	d3d9params->MultiSampleQuality = params->MultiSampleQuality;
	d3d9params->SwapEffect = params->SwapEffect;
	d3d9params->hDeviceWindow = params->hDeviceWindow;
	d3d9params->Windowed = params->Windowed;
	d3d9params->EnableAutoDepthStencil = params->EnableAutoDepthStencil;
	d3d9params->AutoDepthStencilFormat = params->AutoDepthStencilFormat;
	d3d9params->Flags = params->Flags;
	d3d9params->FullScreen_RefreshRateInHz = params->FullScreen_RefreshRateInHz;
	d3d9params->PresentationInterval = params->PresentationInterval;
}



//============================================================
//  drawd3d9_init
//============================================================

d3d_base *drawd3d9_init(void)
{
	direct3dcreate9_ptr direct3dcreate9;
	HINSTANCE dllhandle;
	IDirect3D9 *d3d9;
	d3d_base *d3dptr;
	bool post_available = true;

	// dynamically grab the create function from d3d9.dll
	dllhandle = LoadLibrary(TEXT("d3d9.dll"));
	if (dllhandle == NULL)
	{
		mame_printf_verbose("Direct3D: Unable to access d3d9.dll\n");
		return NULL;
	}

	// import the create function
	direct3dcreate9 = (direct3dcreate9_ptr)GetProcAddress(dllhandle, "Direct3DCreate9");
	if (direct3dcreate9 == NULL)
	{
		mame_printf_verbose("Direct3D: Unable to find Direct3DCreate9\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return NULL;
	}

	// create our core direct 3d object
	d3d9 = (*direct3dcreate9)(D3D_SDK_VERSION);
	if (d3d9 == NULL)
	{
		mame_printf_verbose("Direct3D: Error attempting to initialize Direct3D9\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return NULL;
	}

	// dynamically grab the shader load function from d3dx9.dll
	HINSTANCE fxhandle = LoadLibrary(TEXT("d3dx9_43.dll"));
	if (fxhandle == NULL)
	{
		post_available = false;
		mame_printf_verbose("Direct3D: Warning - Unable to access d3dx9_43.dll; disabling post-effect rendering\n");
	}

	// import the create function
	if(post_available)
	{
		g_load_effect = (direct3dx9_loadeffect_ptr)GetProcAddress(fxhandle, "D3DXCreateEffectFromFileW");
		if (g_load_effect == NULL)
		{
			printf("Direct3D: Unable to find D3DXCreateEffectFromFileW\n");
			FreeLibrary(dllhandle);
			fxhandle = NULL;
			dllhandle = NULL;
			return NULL;
		}
	}
	else
	{
		g_load_effect = NULL;
		post_available = false;
		mame_printf_verbose("Direct3D: Warning - Unable to get a handle to D3DXCreateEffectFromFileW; disabling post-effect rendering\n");
	}

	// allocate an object to hold our data
	d3dptr = global_alloc(d3d_base);
	d3dptr->version = 9;
	d3dptr->d3dobj = d3d9;
	d3dptr->dllhandle = dllhandle;
	d3dptr->post_fx_available = post_available;
	set_interfaces(d3dptr);

	mame_printf_verbose("Direct3D: Using Direct3D 9\n");
	return d3dptr;
}



//============================================================
//  Direct3D interfaces
//============================================================

static HRESULT d3d_check_device_format(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT adapterformat, DWORD usage, D3DRESOURCETYPE restype, D3DFORMAT format)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_CheckDeviceFormat(d3d9, adapter, devtype, adapterformat, usage, restype, format);
}


static HRESULT d3d_check_device_type(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT format, D3DFORMAT backformat, BOOL windowed)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_CheckDeviceType(d3d9, adapter, devtype, format, backformat, windowed);
}

static HRESULT d3d_create_device(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, HWND focus, DWORD behavior, d3d_present_parameters *params, d3d_device **dev)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	D3DPRESENT_PARAMETERS d3d9params;
	convert_present_params(params, &d3d9params);
	return IDirect3D9_CreateDevice(d3d9, adapter, devtype, focus, behavior, &d3d9params, (IDirect3DDevice9 **)dev);
}

static HRESULT d3d_enum_adapter_modes(d3d_base *d3dptr, UINT adapter, D3DFORMAT format, UINT index, D3DDISPLAYMODE *mode)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_EnumAdapterModes(d3d9, adapter, format, index, mode);
}


static UINT d3d_get_adapter_count(d3d_base *d3dptr)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_GetAdapterCount(d3d9);
}


static HRESULT d3d_get_adapter_display_mode(d3d_base *d3dptr, UINT adapter, D3DDISPLAYMODE *mode)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_GetAdapterDisplayMode(d3d9, adapter, mode);
}


static HRESULT d3d_get_adapter_identifier(d3d_base *d3dptr, UINT adapter, DWORD flags, d3d_adapter_identifier *identifier)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	D3DADAPTER_IDENTIFIER9 id;
	HRESULT result = IDirect3D9_GetAdapterIdentifier(d3d9, adapter, flags, &id);
	memcpy(identifier->Driver, id.Driver, sizeof(identifier->Driver));
	memcpy(identifier->Description, id.Description, sizeof(identifier->Description));
	identifier->DriverVersion = id.DriverVersion;
	identifier->VendorId = id.VendorId;
	identifier->DeviceId = id.DeviceId;
	identifier->SubSysId = id.SubSysId;
	identifier->Revision = id.Revision;
	identifier->DeviceIdentifier = id.DeviceIdentifier;
	identifier->WHQLLevel = id.WHQLLevel;
	return result;
}


static UINT d3d_get_adapter_mode_count(d3d_base *d3dptr, UINT adapter, D3DFORMAT format)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_GetAdapterModeCount(d3d9, adapter, format);
}


static HMONITOR d3d_get_adapter_monitor(d3d_base *d3dptr, UINT adapter)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	return IDirect3D9_GetAdapterMonitor(d3d9, adapter);
}


static HRESULT d3d_get_caps_dword(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, d3d_caps_index which, DWORD *value)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	D3DCAPS9 caps;
	HRESULT result = IDirect3D9_GetDeviceCaps(d3d9, adapter, devtype, &caps);
	switch (which)
	{
		case CAPS_PRESENTATION_INTERVALS:	*value = caps.PresentationIntervals;	break;
		case CAPS_CAPS2:					*value = caps.Caps2;					break;
		case CAPS_DEV_CAPS:					*value = caps.DevCaps;					break;
		case CAPS_SRCBLEND_CAPS:			*value = caps.SrcBlendCaps;				break;
		case CAPS_DSTBLEND_CAPS:			*value = caps.DestBlendCaps;			break;
		case CAPS_TEXTURE_CAPS:				*value = caps.TextureCaps;				break;
		case CAPS_TEXTURE_FILTER_CAPS:		*value = caps.TextureFilterCaps;		break;
		case CAPS_TEXTURE_ADDRESS_CAPS:		*value = caps.TextureAddressCaps;		break;
		case CAPS_TEXTURE_OP_CAPS:			*value = caps.TextureOpCaps;			break;
		case CAPS_MAX_TEXTURE_ASPECT:		*value = caps.MaxTextureAspectRatio;	break;
		case CAPS_MAX_TEXTURE_WIDTH:		*value = caps.MaxTextureWidth;			break;
		case CAPS_MAX_TEXTURE_HEIGHT:		*value = caps.MaxTextureHeight;			break;
		case CAPS_STRETCH_RECT_FILTER:		*value = caps.StretchRectFilterCaps;	break;
		case CAPS_MAX_PS30_INSN_SLOTS:		*value = caps.MaxPixelShader30InstructionSlots; break;
	}
	return result;
}


static ULONG d3d_release(d3d_base *d3dptr)
{
	IDirect3D9 *d3d9 = (IDirect3D9 *)d3dptr->d3dobj;
	ULONG result = IDirect3D9_Release(d3d9);
	FreeLibrary(d3dptr->dllhandle);
	global_free(d3dptr);
	return result;
}


static const d3d_interface d3d9_interface =
{
	d3d_check_device_format,
	d3d_check_device_type,
	d3d_create_device,
	d3d_enum_adapter_modes,
	d3d_get_adapter_count,
	d3d_get_adapter_display_mode,
	d3d_get_adapter_identifier,
	d3d_get_adapter_mode_count,
	d3d_get_adapter_monitor,
	d3d_get_caps_dword,
	d3d_release
};



//============================================================
//  Direct3DDevice interfaces
//============================================================

static HRESULT d3d_device_begin_scene(d3d_device *dev)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_BeginScene(device);
}

static HRESULT d3d_device_clear(d3d_device *dev, DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_Clear(device, count, rects, flags, color, z, stencil);
}


static HRESULT d3d_device_create_offscreen_plain_surface(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, D3DPOOL pool, d3d_surface **surface)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_CreateOffscreenPlainSurface(device, width, height, format, pool, (IDirect3DSurface9 **)surface, NULL);
}


static HRESULT d3d_device_create_effect(d3d_device *dev, const WCHAR *name, d3d_effect **effect)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;

	LPD3DXBUFFER buffer_errors = NULL;
	HRESULT hr = (*g_load_effect)(device, name, NULL, NULL, 0, NULL, (ID3DXEffect**)effect, &buffer_errors);
	if(FAILED(hr))
	{
		if(buffer_errors != NULL)
		{
			LPVOID compile_errors = buffer_errors->GetBufferPointer();
			printf("Unable to compile shader: %s\n", (const char*)compile_errors);
		}
		else
		{
			printf("Unable to compile shader (unspecified reason)\n");
		}
	}

	return hr;
}


static HRESULT d3d_device_create_texture(d3d_device *dev, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, d3d_texture **texture)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_CreateTexture(device, width, height, levels, usage, format, pool, (IDirect3DTexture9 **)texture, NULL);
}


static HRESULT d3d_device_create_vertex_buffer(d3d_device *dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, d3d_vertex_buffer **buf)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_CreateVertexBuffer(device, length, usage, fvf, pool, (IDirect3DVertexBuffer9 **)buf, NULL);
}


static HRESULT d3d_device_draw_primitive(d3d_device *dev, D3DPRIMITIVETYPE type, UINT start, UINT count)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_DrawPrimitive(device, type, start, count);
}


static HRESULT d3d_device_end_scene(d3d_device *dev)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_EndScene(device);
}


static HRESULT d3d_device_get_raster_status(d3d_device *dev, D3DRASTER_STATUS *status)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_GetRasterStatus(device, 0, status);
}


static HRESULT d3d_device_get_render_target(d3d_device *dev, DWORD index, d3d_surface **surface)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_GetRenderTarget(device, index, (IDirect3DSurface9 **)surface);
}


static HRESULT d3d_device_get_render_target_data(d3d_device *dev, d3d_surface *rendertarget, d3d_surface *destsurface)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_GetRenderTargetData(device, (IDirect3DSurface9 *)rendertarget, (IDirect3DSurface9 *)destsurface);
}


static HRESULT d3d_device_present(d3d_device *dev, const RECT *source, const RECT *dest, HWND override, RGNDATA *dirty, DWORD flags)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	if (flags != 0)
	{
		IDirect3DSwapChain9 *chain;
		HRESULT result = IDirect3DDevice9_GetSwapChain(device, 0, &chain);
		if (result == D3D_OK)
		{
			result = IDirect3DSwapChain9_Present(chain, source, dest, override, dirty, flags);
			IDirect3DSwapChain9_Release(chain);
			return result;
		}
	}
	return IDirect3DDevice9_Present(device, source, dest, override, dirty);
}


static ULONG d3d_device_release(d3d_device *dev)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_Release(device);
}


static HRESULT d3d_device_reset(d3d_device *dev, d3d_present_parameters *params)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	D3DPRESENT_PARAMETERS d3d9params;
	convert_present_params(params, &d3d9params);
	return IDirect3DDevice9_Reset(device, &d3d9params);
}


static void d3d_device_set_gamma_ramp(d3d_device *dev, DWORD flags, const D3DGAMMARAMP *ramp)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	IDirect3DDevice9_SetGammaRamp(device, 0, flags, ramp);
}


static HRESULT d3d_device_set_render_state(d3d_device *dev, D3DRENDERSTATETYPE state, DWORD value)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_SetRenderState(device, state, value);
}


static HRESULT d3d_device_set_render_target(d3d_device *dev, DWORD index, d3d_surface *surf)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	IDirect3DSurface9 *surface = (IDirect3DSurface9 *)surf;
	return IDirect3DDevice9_SetRenderTarget(device, index, surface);
}


static HRESULT d3d_device_create_render_target(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, d3d_surface **surface)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_CreateRenderTarget(device, width, height, format, D3DMULTISAMPLE_NONE, 0, false, (IDirect3DSurface9 **)surface, NULL);
}


static HRESULT d3d_device_set_stream_source(d3d_device *dev, UINT number, d3d_vertex_buffer *vbuf, UINT stride)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	IDirect3DVertexBuffer9 *vertexbuf = (IDirect3DVertexBuffer9 *)vbuf;
	return IDirect3DDevice9_SetStreamSource(device, number, vertexbuf, 0, stride);
}


static HRESULT d3d_device_set_texture(d3d_device *dev, DWORD stage, d3d_texture *tex)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	IDirect3DBaseTexture9 *texture = (IDirect3DBaseTexture9 *)tex;
	return IDirect3DDevice9_SetTexture(device, stage, texture);
}


static HRESULT d3d_device_set_texture_stage_state(d3d_device *dev, DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;

	// some state which was here got pushed into sampler state in D3D9
	switch ((DWORD)state)
	{
		case D3DTSS_ADDRESSU:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_ADDRESSU, value);
		case D3DTSS_ADDRESSV:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_ADDRESSV, value);
		case D3DTSS_BORDERCOLOR:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_BORDERCOLOR, value);
		case D3DTSS_MAGFILTER:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MAGFILTER, value);
		case D3DTSS_MINFILTER:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MINFILTER, value);
		case D3DTSS_MIPFILTER:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MIPFILTER, value);
		case D3DTSS_MIPMAPLODBIAS:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MIPMAPLODBIAS, value);
		case D3DTSS_MAXMIPLEVEL:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MAXMIPLEVEL, value);
		case D3DTSS_MAXANISOTROPY:
			return IDirect3DDevice9_SetSamplerState(device, stage, D3DSAMP_MAXANISOTROPY, value);
		default:
			return IDirect3DDevice9_SetTextureStageState(device, stage, state, value);
	}
}


static HRESULT d3d_device_set_vertex_format(d3d_device *dev, D3DFORMAT format)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_SetFVF(device, format);
}


static HRESULT d3d_device_stretch_rect(d3d_device *dev, d3d_surface *source, const RECT *srcrect, d3d_surface *dest, const RECT *dstrect, D3DTEXTUREFILTERTYPE filter)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	IDirect3DSurface9 *ssurface = (IDirect3DSurface9 *)source;
	IDirect3DSurface9 *dsurface = (IDirect3DSurface9 *)dest;
	return IDirect3DDevice9_StretchRect(device, ssurface, srcrect, dsurface, dstrect, filter);
}


static HRESULT d3d_device_test_cooperative_level(d3d_device *dev)
{
	IDirect3DDevice9 *device = (IDirect3DDevice9 *)dev;
	return IDirect3DDevice9_TestCooperativeLevel(device);
}


static const d3d_device_interface d3d9_device_interface =
{
	d3d_device_begin_scene,
	d3d_device_clear,
	d3d_device_create_offscreen_plain_surface,
	d3d_device_create_effect,
	d3d_device_create_texture,
	d3d_device_create_vertex_buffer,
	d3d_device_create_render_target,
	d3d_device_draw_primitive,
	d3d_device_end_scene,
	d3d_device_get_raster_status,
	d3d_device_get_render_target,
	d3d_device_get_render_target_data,
	d3d_device_present,
	d3d_device_release,
	d3d_device_reset,
	d3d_device_set_gamma_ramp,
	d3d_device_set_render_state,
	d3d_device_set_render_target,
	d3d_device_set_stream_source,
	d3d_device_set_texture,
	d3d_device_set_texture_stage_state,
	d3d_device_set_vertex_format,
	d3d_device_stretch_rect,
	d3d_device_test_cooperative_level
};



//============================================================
//  Direct3DSurface interfaces
//============================================================

static HRESULT d3d_surface_lock_rect(d3d_surface *surf, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
{
	IDirect3DSurface9 *surface = (IDirect3DSurface9 *)surf;
	return IDirect3DSurface9_LockRect(surface, locked, rect, flags);
}


static ULONG d3d_surface_release(d3d_surface *surf)
{
	IDirect3DSurface9 *surface = (IDirect3DSurface9 *)surf;
	return IDirect3DSurface9_Release(surface);
}


static HRESULT d3d_surface_unlock_rect(d3d_surface *surf)
{
	IDirect3DSurface9 *surface = (IDirect3DSurface9 *)surf;
	return IDirect3DSurface9_UnlockRect(surface);
}


static const d3d_surface_interface d3d9_surface_interface =
{
	d3d_surface_lock_rect,
	d3d_surface_release,
	d3d_surface_unlock_rect
};



//============================================================
//  Direct3DTexture interfaces
//============================================================

static HRESULT d3d_texture_get_surface_level(d3d_texture *tex, UINT level, d3d_surface **surface)
{
	IDirect3DTexture9 *texture = (IDirect3DTexture9 *)tex;
	return IDirect3DTexture9_GetSurfaceLevel(texture, level, (IDirect3DSurface9 **)surface);
}


static HRESULT d3d_texture_lock_rect(d3d_texture *tex, UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
{
	IDirect3DTexture9 *texture = (IDirect3DTexture9 *)tex;
	return IDirect3DTexture9_LockRect(texture, level, locked, rect, flags);
}


static ULONG d3d_texture_release(d3d_texture *tex)
{
	IDirect3DTexture9 *texture = (IDirect3DTexture9 *)tex;
	return IDirect3DTexture9_Release(texture);
}


static HRESULT d3d_texture_unlock_rect(d3d_texture *tex, UINT level)
{
	IDirect3DTexture9 *texture = (IDirect3DTexture9 *)tex;
	return IDirect3DTexture9_UnlockRect(texture, level);
}


static const d3d_texture_interface d3d9_texture_interface =
{
	d3d_texture_get_surface_level,
	d3d_texture_lock_rect,
	d3d_texture_release,
	d3d_texture_unlock_rect
};



//============================================================
//  Direct3DVertexBuffer interfaces
//============================================================

static HRESULT d3d_vertex_buffer_lock(d3d_vertex_buffer *vbuf, UINT offset, UINT size, VOID **data, DWORD flags)
{
	IDirect3DVertexBuffer9 *vertexbuf = (IDirect3DVertexBuffer9 *)vbuf;
	return IDirect3DVertexBuffer9_Lock(vertexbuf, offset, size, data, flags);
}


static ULONG d3d_vertex_buffer_release(d3d_vertex_buffer *vbuf)
{
	IDirect3DVertexBuffer9 *vertexbuf = (IDirect3DVertexBuffer9 *)vbuf;
	return IDirect3DVertexBuffer9_Release(vertexbuf);
}


static HRESULT d3d_vertex_buffer_unlock(d3d_vertex_buffer *vbuf)
{
	IDirect3DVertexBuffer9 *vertexbuf = (IDirect3DVertexBuffer9 *)vbuf;
	return IDirect3DVertexBuffer9_Unlock(vertexbuf);
}


static const d3d_vertex_buffer_interface d3d9_vertex_buffer_interface =
{
	d3d_vertex_buffer_lock,
	d3d_vertex_buffer_release,
	d3d_vertex_buffer_unlock
};



//============================================================
//  Direct3DEffect interfaces
//============================================================

static void d3d_effect_begin(d3d_effect *effect, UINT *passes, DWORD flags)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->Begin(passes, flags);
}


static void d3d_effect_end(d3d_effect *effect)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->End();
}


static void d3d_effect_begin_pass(d3d_effect *effect, UINT pass)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->BeginPass(pass);
}


static void d3d_effect_end_pass(d3d_effect *effect)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->EndPass();
}


static void d3d_effect_set_technique(d3d_effect *effect, const char *name)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->SetTechnique(name);
}


static void d3d_effect_set_vector(d3d_effect *effect, const char *name, int count, float *vector)
{
	static D3DXVECTOR4 out_vector;
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	if (count > 0)
		out_vector.x = vector[0];
	if (count > 1)
		out_vector.y = vector[1];
	if (count > 2)
		out_vector.z = vector[2];
	if (count > 3)
		out_vector.w = vector[3];
	d3dfx->SetVector(name, &out_vector);
}


static void d3d_effect_set_float(d3d_effect *effect, const char *name, float value)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->SetFloat(name, value);
}


static void d3d_effect_set_int(d3d_effect *effect, const char *name, int value)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->SetInt(name, value);
}


static void d3d_effect_set_matrix(d3d_effect *effect, const char *name, d3d_matrix *matrix)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->SetMatrix(name, (D3DXMATRIX*)matrix);
}


static void d3d_effect_set_texture(d3d_effect *effect, const char *name, d3d_texture *tex)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	d3dfx->SetTexture(name, (IDirect3DTexture9*)tex);
}


static ULONG d3d_effect_release(d3d_effect *effect)
{
	ID3DXEffect *d3dfx = (ID3DXEffect*)effect;
	return d3dfx->Release();
}


static const d3d_effect_interface d3d9_effect_interface =
{
	d3d_effect_begin,
	d3d_effect_end,
	d3d_effect_begin_pass,
	d3d_effect_end_pass,
	d3d_effect_set_technique,
	d3d_effect_set_vector,
	d3d_effect_set_float,
	d3d_effect_set_int,
	d3d_effect_set_matrix,
	d3d_effect_set_texture,
	d3d_effect_release
};



//============================================================
//  set_interfaces
//============================================================

static void set_interfaces(d3d_base *d3dptr)
{
	d3dptr->d3d = d3d9_interface;
	d3dptr->device = d3d9_device_interface;
	d3dptr->surface = d3d9_surface_interface;
	d3dptr->texture = d3d9_texture_interface;
	d3dptr->vertexbuf = d3d9_vertex_buffer_interface;
	d3dptr->effect = d3d9_effect_interface;
}
