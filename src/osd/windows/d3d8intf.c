//============================================================
//
//  d3d8intf.c - Direct3D 8.1 abstraction layer
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
#include <d3d8.h>
#define D3DXVECTOR4 void
#undef interface

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "d3dintf.h"
#include "winmain.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef IDirect3D8 *(WINAPI *direct3dcreate8_ptr)(UINT SDKVersion);



//============================================================
//  PROTOTYPES
//============================================================

static void set_interfaces(d3d_base *d3dptr);



//============================================================
//  INLINES
//============================================================

INLINE void convert_present_params(const d3d_present_parameters *params, D3DPRESENT_PARAMETERS *d3d8params)
{
	memset(d3d8params, 0, sizeof(*d3d8params));
	d3d8params->BackBufferWidth = params->BackBufferWidth;
	d3d8params->BackBufferHeight = params->BackBufferHeight;
	d3d8params->BackBufferFormat = params->BackBufferFormat;
	d3d8params->BackBufferCount = params->BackBufferCount;
	d3d8params->MultiSampleType = params->MultiSampleType;
//  d3d8params->MultiSampleQuality = params->MultiSampleQuality;
	d3d8params->SwapEffect = params->SwapEffect;
	d3d8params->hDeviceWindow = params->hDeviceWindow;
	d3d8params->Windowed = params->Windowed;
	d3d8params->EnableAutoDepthStencil = params->EnableAutoDepthStencil;
	d3d8params->AutoDepthStencilFormat = params->AutoDepthStencilFormat;
	d3d8params->Flags = params->Flags;
	d3d8params->FullScreen_RefreshRateInHz = params->FullScreen_RefreshRateInHz;
	d3d8params->FullScreen_PresentationInterval = params->PresentationInterval;
	if (d3d8params->Windowed)
		d3d8params->FullScreen_PresentationInterval = 0;
}



//============================================================
//  drawd3d8_init
//============================================================

d3d_base *drawd3d8_init(void)
{
	direct3dcreate8_ptr direct3dcreate8;
	HINSTANCE dllhandle;
	IDirect3D8 *d3d8;
	d3d_base *d3dptr;

	// dynamically grab the create function from d3d8.dll
	dllhandle = LoadLibrary(TEXT("d3d8.dll"));
	if (dllhandle == NULL)
	{
		mame_printf_verbose("Direct3D: Unable to access d3d8.dll\n");
		return NULL;
	}

	// import the create function
	direct3dcreate8 = (direct3dcreate8_ptr)GetProcAddress(dllhandle, "Direct3DCreate8");
	if (direct3dcreate8 == NULL)
	{
		mame_printf_verbose("Direct3D: Unable to find Direct3DCreate8\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return NULL;
	}

	// create our core direct 3d object
	d3d8 = (*direct3dcreate8)(D3D_SDK_VERSION);
	if (d3d8 == NULL)
	{
		mame_printf_verbose("Direct3D: Error attempting to initialize Direct3D8\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return NULL;
	}

	// allocate an object to hold our data
	d3dptr = global_alloc(d3d_base);
	d3dptr->version = 8;
	d3dptr->d3dobj = d3d8;
	d3dptr->dllhandle = dllhandle;
	d3dptr->post_fx_available = false;
	set_interfaces(d3dptr);

	mame_printf_verbose("Direct3D: Using Direct3D 8\n");
	return d3dptr;
}



//============================================================
//  Direct3D interfaces
//============================================================

static HRESULT d3d_check_device_format(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT adapterformat, DWORD usage, D3DRESOURCETYPE restype, D3DFORMAT format)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_CheckDeviceFormat(d3d8, adapter, devtype, adapterformat, usage, restype, format);
}


static HRESULT d3d_check_device_type(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT format, D3DFORMAT backformat, BOOL windowed)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_CheckDeviceType(d3d8, adapter, devtype, format, backformat, windowed);
}


static HRESULT d3d_create_device(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, HWND focus, DWORD behavior, d3d_present_parameters *params, d3d_device **dev)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	D3DPRESENT_PARAMETERS d3d8params;
	convert_present_params(params, &d3d8params);
	return IDirect3D8_CreateDevice(d3d8, adapter, devtype, focus, behavior, &d3d8params, (IDirect3DDevice8 **)dev);
}


static HRESULT d3d_enum_adapter_modes(d3d_base *d3dptr, UINT adapter, D3DFORMAT format, UINT index, D3DDISPLAYMODE *mode)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_EnumAdapterModes(d3d8, adapter, index, mode);
}


static UINT d3d_get_adapter_count(d3d_base *d3dptr)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_GetAdapterCount(d3d8);
}


static HRESULT d3d_get_adapter_display_mode(d3d_base *d3dptr, UINT adapter, D3DDISPLAYMODE *mode)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_GetAdapterDisplayMode(d3d8, adapter, mode);
}


static HRESULT d3d_get_adapter_identifier(d3d_base *d3dptr, UINT adapter, DWORD flags, d3d_adapter_identifier *identifier)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	D3DADAPTER_IDENTIFIER8 id;
	HRESULT result = IDirect3D8_GetAdapterIdentifier(d3d8, adapter, flags, &id);
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
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_GetAdapterModeCount(d3d8, adapter);
}


static HMONITOR d3d_get_adapter_monitor(d3d_base *d3dptr, UINT adapter)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	return IDirect3D8_GetAdapterMonitor(d3d8, adapter);
}


static HRESULT d3d_get_caps_dword(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, d3d_caps_index which, DWORD *value)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	D3DCAPS8 caps;
	HRESULT result = IDirect3D8_GetDeviceCaps(d3d8, adapter, devtype, &caps);
	switch (which)
	{
		case CAPS_PRESENTATION_INTERVALS:   *value = caps.PresentationIntervals;    break;
		case CAPS_CAPS2:                    *value = caps.DevCaps;                  break;
		case CAPS_DEV_CAPS:                 *value = caps.DevCaps;                  break;
		case CAPS_SRCBLEND_CAPS:            *value = caps.SrcBlendCaps;             break;
		case CAPS_DSTBLEND_CAPS:            *value = caps.DestBlendCaps;            break;
		case CAPS_TEXTURE_CAPS:             *value = caps.TextureCaps;              break;
		case CAPS_TEXTURE_FILTER_CAPS:      *value = caps.TextureFilterCaps;        break;
		case CAPS_TEXTURE_ADDRESS_CAPS:     *value = caps.TextureAddressCaps;       break;
		case CAPS_TEXTURE_OP_CAPS:          *value = caps.TextureOpCaps;            break;
		case CAPS_MAX_TEXTURE_ASPECT:       *value = caps.MaxTextureAspectRatio;    break;
		case CAPS_MAX_TEXTURE_WIDTH:        *value = caps.MaxTextureWidth;          break;
		case CAPS_MAX_TEXTURE_HEIGHT:       *value = caps.MaxTextureHeight;         break;
		case CAPS_STRETCH_RECT_FILTER:      *value = 0;                             break;
		case CAPS_MAX_PS30_INSN_SLOTS:      *value = 0;                             break;
	}
	return result;
}


static ULONG d3d_release(d3d_base *d3dptr)
{
	IDirect3D8 *d3d8 = (IDirect3D8 *)d3dptr->d3dobj;
	ULONG result = IDirect3D8_Release(d3d8);
	FreeLibrary(d3dptr->dllhandle);
	global_free(d3dptr);
	return result;
}


static const d3d_interface d3d8_interface =
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
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_BeginScene(device);
}


static HRESULT d3d_device_clear(d3d_device *dev, DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_Clear(device, count, rects, flags, color, z, stencil);
}


static HRESULT d3d_device_create_offscreen_plain_surface(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, D3DPOOL pool, d3d_surface **surface)
{
	assert(FALSE);
	return D3D_OK;
}


static HRESULT d3d_device_create_effect(d3d_device *dev, const WCHAR *name, d3d_effect **effect)
{
	assert(FALSE);
	return D3D_OK;
}



static HRESULT d3d_device_create_texture(d3d_device *dev, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, d3d_texture **texture)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_CreateTexture(device, width, height, levels, usage, format, pool, (IDirect3DTexture8 **)texture);
}


static HRESULT d3d_device_create_vertex_buffer(d3d_device *dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, d3d_vertex_buffer **buf)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_CreateVertexBuffer(device, length, usage, fvf, pool, (IDirect3DVertexBuffer8 **)buf);
}


static HRESULT d3d_device_draw_primitive(d3d_device *dev, D3DPRIMITIVETYPE type, UINT start, UINT count)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_DrawPrimitive(device, type, start, count);
}


static HRESULT d3d_device_end_scene(d3d_device *dev)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_EndScene(device);
}


static HRESULT d3d_device_get_raster_status(d3d_device *dev, D3DRASTER_STATUS *status)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_GetRasterStatus(device, status);
}


static HRESULT d3d_device_get_render_target(d3d_device *dev, DWORD index, d3d_surface **surface)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	assert(index == 0);
	return IDirect3DDevice8_GetRenderTarget(device, (IDirect3DSurface8 **)surface);
}


static HRESULT d3d_device_get_render_target_data(d3d_device *dev, d3d_surface *rendertarget, d3d_surface *destsurface)
{
	assert(false);
	return D3D_OK;
}


static HRESULT d3d_device_present(d3d_device *dev, const RECT *source, const RECT *dest, HWND override, RGNDATA *dirty, DWORD flags)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_Present(device, source, dest, override, dirty);
}


static ULONG d3d_device_release(d3d_device *dev)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_Release(device);
}


static HRESULT d3d_device_reset(d3d_device *dev, d3d_present_parameters *params)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	D3DPRESENT_PARAMETERS d3d8params;
	convert_present_params(params, &d3d8params);
	return IDirect3DDevice8_Reset(device, &d3d8params);
}


static void d3d_device_set_gamma_ramp(d3d_device *dev, DWORD flags, const D3DGAMMARAMP *ramp)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	IDirect3DDevice8_SetGammaRamp(device, flags, ramp);
}


static HRESULT d3d_device_set_render_state(d3d_device *dev, D3DRENDERSTATETYPE state, DWORD value)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_SetRenderState(device, state, value);
}


static HRESULT d3d_device_set_render_target(d3d_device *dev, DWORD index, d3d_surface *surf)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	IDirect3DSurface8 *surface = (IDirect3DSurface8 *)surf;
	assert(index == 0);
	return IDirect3DDevice8_SetRenderTarget(device, surface, NULL);
}


static HRESULT d3d_device_create_render_target(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, d3d_surface **surface)
{
	assert(false);
	return D3D_OK;
}


static HRESULT d3d_device_set_stream_source(d3d_device *dev, UINT number, d3d_vertex_buffer *vbuf, UINT stride)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	IDirect3DVertexBuffer8 *vertexbuf = (IDirect3DVertexBuffer8 *)vbuf;
	return IDirect3DDevice8_SetStreamSource(device, number, vertexbuf, stride);
}


static HRESULT d3d_device_set_texture(d3d_device *dev, DWORD stage, d3d_texture *tex)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	IDirect3DBaseTexture8 *texture = (IDirect3DBaseTexture8 *)tex;
	return IDirect3DDevice8_SetTexture(device, stage, texture);
}


static HRESULT d3d_device_set_texture_stage_state(d3d_device *dev, DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_SetTextureStageState(device, stage, state, value);
}


static HRESULT d3d_device_set_vertex_format(d3d_device *dev, D3DFORMAT format)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_SetVertexShader(device, format);
}


static HRESULT d3d_device_stretch_rect(d3d_device *dev, d3d_surface *source, const RECT *srcrect, d3d_surface *dest, const RECT *dstrect, D3DTEXTUREFILTERTYPE filter)
{
	assert(FALSE);
	return D3D_OK;
}


static HRESULT d3d_device_test_cooperative_level(d3d_device *dev)
{
	IDirect3DDevice8 *device = (IDirect3DDevice8 *)dev;
	return IDirect3DDevice8_TestCooperativeLevel(device);
}


static const d3d_device_interface d3d8_device_interface =
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
	IDirect3DSurface8 *surface = (IDirect3DSurface8 *)surf;
	return IDirect3DSurface8_LockRect(surface, locked, rect, flags);
}


static ULONG d3d_surface_release(d3d_surface *surf)
{
	IDirect3DSurface8 *surface = (IDirect3DSurface8 *)surf;
	return IDirect3DSurface8_Release(surface);
}


static HRESULT d3d_surface_unlock_rect(d3d_surface *surf)
{
	IDirect3DSurface8 *surface = (IDirect3DSurface8 *)surf;
	return IDirect3DSurface8_UnlockRect(surface);
}


static const d3d_surface_interface d3d8_surface_interface =
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
	IDirect3DTexture8 *texture = (IDirect3DTexture8 *)tex;
	return IDirect3DTexture8_GetSurfaceLevel(texture, level, (IDirect3DSurface8 **)surface);
}


static HRESULT d3d_texture_lock_rect(d3d_texture *tex, UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
{
	IDirect3DTexture8 *texture = (IDirect3DTexture8 *)tex;
	return IDirect3DTexture8_LockRect(texture, level, locked, rect, flags);
}


static ULONG d3d_texture_release(d3d_texture *tex)
{
	IDirect3DTexture8 *texture = (IDirect3DTexture8 *)tex;
	return IDirect3DTexture8_Release(texture);
}


static HRESULT d3d_texture_unlock_rect(d3d_texture *tex, UINT level)
{
	IDirect3DTexture8 *texture = (IDirect3DTexture8 *)tex;
	return IDirect3DTexture8_UnlockRect(texture, level);
}


static const d3d_texture_interface d3d8_texture_interface =
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
	IDirect3DVertexBuffer8 *vertexbuf = (IDirect3DVertexBuffer8 *)vbuf;
	return IDirect3DVertexBuffer8_Lock(vertexbuf, offset, size, (BYTE **)data, flags);
}


static ULONG d3d_vertex_buffer_release(d3d_vertex_buffer *vbuf)
{
	IDirect3DVertexBuffer8 *vertexbuf = (IDirect3DVertexBuffer8 *)vbuf;
	return IDirect3DVertexBuffer8_Release(vertexbuf);
}


static HRESULT d3d_vertex_buffer_unlock(d3d_vertex_buffer *vbuf)
{
	IDirect3DVertexBuffer8 *vertexbuf = (IDirect3DVertexBuffer8 *)vbuf;
	return IDirect3DVertexBuffer8_Unlock(vertexbuf);
}


static const d3d_vertex_buffer_interface d3d8_vertex_buffer_interface =
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
	assert(FALSE);
}


static void d3d_effect_end(d3d_effect *effect)
{
	assert(FALSE);
}


static void d3d_effect_begin_pass(d3d_effect *effect, UINT pass)
{
	assert(FALSE);
}


static void d3d_effect_end_pass(d3d_effect *effect)
{
	assert(FALSE);
}


static void d3d_effect_set_technique(d3d_effect *effect, const char *name)
{
	assert(FALSE);
}


static void d3d_effect_set_vector(d3d_effect *effect, const char *name, int count, float *vector)
{
	assert(FALSE);
}


static void d3d_effect_set_float(d3d_effect *effect, const char *name, float value)
{
	assert(FALSE);
}


static void d3d_effect_set_int(d3d_effect *effect, const char *name, int value)
{
	assert(FALSE);
}


static void d3d_effect_set_matrix(d3d_effect *effect, const char *name, d3d_matrix *matrix)
{
	assert(FALSE);
}


static void d3d_effect_set_texture(d3d_effect *effect, const char *name, d3d_texture *tex)
{
	assert(FALSE);
}


static ULONG d3d_effect_release(d3d_effect *effect)
{
	assert(FALSE);
	return 0;
}


static const d3d_effect_interface d3d8_effect_interface =
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
	d3dptr->d3d = d3d8_interface;
	d3dptr->device = d3d8_device_interface;
	d3dptr->surface = d3d8_surface_interface;
	d3dptr->texture = d3d8_texture_interface;
	d3dptr->vertexbuf = d3d8_vertex_buffer_interface;
	d3dptr->effect = d3d8_effect_interface;
}
