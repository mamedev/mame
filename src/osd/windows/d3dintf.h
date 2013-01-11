//============================================================
//
//  d3dintf.h - Direct3D 8/9 interface abstractions
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

#ifndef __WIN_D3DINTF__
#define __WIN_D3DINTF__


//============================================================
//  CONSTANTS
//============================================================

#ifndef D3DCAPS2_DYNAMICTEXTURES
#define D3DCAPS2_DYNAMICTEXTURES 0x20000000L
#endif

#ifndef D3DPRESENT_DONOTWAIT
#define D3DPRESENT_DONOTWAIT 0x00000001L
#endif


#if (DIRECT3D_VERSION >= 0x0900)
// the following used to be TEXTURESTAGESTATES but are now SAMPLERSTATES
#define D3DTSS_ADDRESSU       13
#define D3DTSS_ADDRESSV       14
#define D3DTSS_BORDERCOLOR    15
#define D3DTSS_MAGFILTER      16
#define D3DTSS_MINFILTER      17
#define D3DTSS_MIPFILTER      18
#define D3DTSS_MIPMAPLODBIAS  19
#define D3DTSS_MAXMIPLEVEL    20
#define D3DTSS_MAXANISOTROPY  21
#endif


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct d3d_base;
struct d3d_device;
struct d3d_surface;
struct d3d_texture;
struct d3d_vertex_buffer;
struct d3d_effect;
typedef D3DXVECTOR4 d3d_vector;
typedef D3DMATRIX d3d_matrix;


//============================================================
//  Abstracted presentation parameters
//============================================================

struct d3d_present_parameters
{
	UINT BackBufferWidth;
	UINT BackBufferHeight;
	D3DFORMAT BackBufferFormat;
	UINT BackBufferCount;
	D3DMULTISAMPLE_TYPE MultiSampleType;
	DWORD MultiSampleQuality;
	D3DSWAPEFFECT SwapEffect;
	HWND hDeviceWindow;
	BOOL Windowed;
	BOOL EnableAutoDepthStencil;
	D3DFORMAT AutoDepthStencilFormat;
	DWORD Flags;
	UINT FullScreen_RefreshRateInHz;
	UINT PresentationInterval;
};


//============================================================
//  Abstracted device identifier
//============================================================

struct d3d_adapter_identifier
{
	char            Driver[512];
	char            Description[512];
	LARGE_INTEGER   DriverVersion;
	DWORD           VendorId;
	DWORD           DeviceId;
	DWORD           SubSysId;
	DWORD           Revision;
	GUID            DeviceIdentifier;
	DWORD           WHQLLevel;
};


//============================================================
//  Caps enumeration
//============================================================

enum d3d_caps_index
{
	CAPS_PRESENTATION_INTERVALS,
	CAPS_CAPS2,
	CAPS_DEV_CAPS,
	CAPS_SRCBLEND_CAPS,
	CAPS_DSTBLEND_CAPS,
	CAPS_TEXTURE_CAPS,
	CAPS_TEXTURE_FILTER_CAPS,
	CAPS_TEXTURE_ADDRESS_CAPS,
	CAPS_TEXTURE_OP_CAPS,
	CAPS_MAX_TEXTURE_ASPECT,
	CAPS_MAX_TEXTURE_WIDTH,
	CAPS_MAX_TEXTURE_HEIGHT,
	CAPS_STRETCH_RECT_FILTER,
	CAPS_MAX_PS30_INSN_SLOTS
};


//============================================================
//  Direct3D interfaces
//============================================================

struct d3d_interface
{
	HRESULT  (*check_device_format)(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT adapterformat, DWORD usage, D3DRESOURCETYPE restype, D3DFORMAT format);
	HRESULT  (*check_device_type)(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT format, D3DFORMAT backformat, BOOL windowed);
	HRESULT  (*create_device)(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, HWND focus, DWORD behavior, d3d_present_parameters *params, d3d_device **dev);
	HRESULT  (*enum_adapter_modes)(d3d_base *d3dptr, UINT adapter, D3DFORMAT format, UINT index, D3DDISPLAYMODE *mode);
	UINT     (*get_adapter_count)(d3d_base *d3dptr);
	HRESULT  (*get_adapter_display_mode)(d3d_base *d3dptr, UINT adapter, D3DDISPLAYMODE *mode);
	HRESULT  (*get_adapter_identifier)(d3d_base *d3dptr, UINT adapter, DWORD flags, d3d_adapter_identifier *identifier);
	UINT     (*get_adapter_mode_count)(d3d_base *d3dptr, UINT adapter, D3DFORMAT format);
	HMONITOR (*get_adapter_monitor)(d3d_base *d3dptr, UINT adapter);
	HRESULT  (*get_caps_dword)(d3d_base *d3dptr, UINT adapter, D3DDEVTYPE devtype, d3d_caps_index which, DWORD *value);
	ULONG    (*release)(d3d_base *d3dptr);
};


//============================================================
//  Direct3DDevice interfaces
//============================================================

struct d3d_device_interface
{
	HRESULT (*begin_scene)(d3d_device *dev);
	HRESULT (*clear)(d3d_device *dev, DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);
	HRESULT (*create_offscreen_plain_surface)(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, D3DPOOL pool, d3d_surface **surface);
	HRESULT (*create_effect)(d3d_device *dev, const WCHAR *name, d3d_effect **effect);
	HRESULT (*create_texture)(d3d_device *dev, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, d3d_texture **texture);
	HRESULT (*create_vertex_buffer)(d3d_device *dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, d3d_vertex_buffer **buf);
	HRESULT (*create_render_target)(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, d3d_surface **surface);
	HRESULT (*draw_primitive)(d3d_device *dev, D3DPRIMITIVETYPE type, UINT start, UINT count);
	HRESULT (*end_scene)(d3d_device *dev);
	HRESULT (*get_raster_status)(d3d_device *dev, D3DRASTER_STATUS *status);
	HRESULT (*get_render_target)(d3d_device *dev, DWORD index, d3d_surface **surface);
	HRESULT (*get_render_target_data)(d3d_device *dev, d3d_surface *rendertarget, d3d_surface *destsurface);
	HRESULT (*present)(d3d_device *dev, const RECT *source, const RECT *dest, HWND override, RGNDATA *dirty, DWORD flags);
	ULONG   (*release)(d3d_device *dev);
	HRESULT (*reset)(d3d_device *dev, d3d_present_parameters *params);
	void    (*set_gamma_ramp)(d3d_device *dev, DWORD flags, const D3DGAMMARAMP *ramp);
	HRESULT (*set_render_state)(d3d_device *dev, D3DRENDERSTATETYPE state, DWORD value);
	HRESULT (*set_render_target)(d3d_device *dev, DWORD index, d3d_surface *surf);
	HRESULT (*set_stream_source)(d3d_device *dev, UINT number, d3d_vertex_buffer *vbuf, UINT stride);
	HRESULT (*set_texture)(d3d_device *dev, DWORD stage, d3d_texture *tex);
	HRESULT (*set_texture_stage_state)(d3d_device *dev, DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	HRESULT (*set_vertex_format)(d3d_device *dev, D3DFORMAT format);
	HRESULT (*stretch_rect)(d3d_device *dev, d3d_surface *source, const RECT *srcrect, d3d_surface *dest, const RECT *dstrect, D3DTEXTUREFILTERTYPE filter);
	HRESULT (*test_cooperative_level)(d3d_device *dev);
};


//============================================================
//  Direct3DSurface interfaces
//============================================================

struct d3d_surface_interface
{
	HRESULT (*lock_rect)(d3d_surface *surf, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags);
	ULONG   (*release)(d3d_surface *tex);
	HRESULT (*unlock_rect)(d3d_surface *surf);
};


//============================================================
//  Direct3DTexture interfaces
//============================================================

struct d3d_texture_interface
{
	HRESULT (*get_surface_level)(d3d_texture *tex, UINT level, d3d_surface **surface);
	HRESULT (*lock_rect)(d3d_texture *tex, UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags);
	ULONG   (*release)(d3d_texture *tex);
	HRESULT (*unlock_rect)(d3d_texture *tex, UINT level);
};


//============================================================
//  Direct3DVertexBuffer interfaces
//============================================================

struct d3d_vertex_buffer_interface
{
	HRESULT (*lock)(d3d_vertex_buffer *vbuf, UINT offset, UINT size, VOID **data, DWORD flags);
	ULONG   (*release)(d3d_vertex_buffer *vbuf);
	HRESULT (*unlock)(d3d_vertex_buffer *vbuf);
};


//============================================================
//  Direct3DEffect interfaces
//============================================================

struct d3d_effect_interface
{
	void     (*begin)(d3d_effect *effect, UINT *passes, DWORD flags);
	void     (*end)(d3d_effect *effect);
	void     (*begin_pass)(d3d_effect *effect, UINT pass);
	void     (*end_pass)(d3d_effect *effect);
	void     (*set_technique)(d3d_effect *effect, const char *name);
	void     (*set_vector)(d3d_effect *effect, const char *name, int count, float *vector);
	void     (*set_float)(d3d_effect *effect, const char *name, float value);
	void     (*set_int)(d3d_effect *effect, const char *name, int value);
	void     (*set_matrix)(d3d_effect *effect, const char *name, d3d_matrix *matrix);
	void     (*set_texture)(d3d_effect *effect, const char *name, d3d_texture *tex);
	ULONG    (*release)(d3d_effect *effect);
};


//============================================================
//  Core D3D object
//============================================================

struct d3d_base
{
	// internal objects
	int                         version;
	void *                      d3dobj;
	HINSTANCE                   dllhandle;
	bool                        post_fx_available;

	// interface pointers
	d3d_interface               d3d;
	d3d_device_interface        device;
	d3d_surface_interface       surface;
	d3d_texture_interface       texture;
	d3d_vertex_buffer_interface vertexbuf;
	d3d_effect_interface        effect;
};


//============================================================
//  PROTOTYPES
//============================================================

#if DIRECT3D_VERSION < 0x0900
d3d_base *drawd3d8_init(void);
#endif
d3d_base *drawd3d9_init(void);


#endif
