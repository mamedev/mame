//============================================================
//
//  drawd3di.h - Direct3D 8/9 interface abstractions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WIN_DRAWD3DI__
#define __WIN_DRAWD3DI__


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
enum
{
    D3DTSS_ADDRESSU       = 13,
    D3DTSS_ADDRESSV       = 14,
    D3DTSS_BORDERCOLOR    = 15,
    D3DTSS_MAGFILTER      = 16,
    D3DTSS_MINFILTER      = 17,
    D3DTSS_MIPFILTER      = 18,
    D3DTSS_MIPMAPLODBIAS  = 19,
    D3DTSS_MAXMIPLEVEL    = 20,
    D3DTSS_MAXANISOTROPY  = 21
};
#endif


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _d3d d3d;
typedef struct _d3d_device d3d_device;
typedef struct _d3d_surface d3d_surface;
typedef struct _d3d_texture d3d_texture;
typedef struct _d3d_vertex_buffer d3d_vertex_buffer;


//============================================================
//  Abstracted presentation parameters
//============================================================

typedef struct _d3d_present_parameters d3d_present_parameters;
struct _d3d_present_parameters
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

typedef struct _d3d_adapter_identifier d3d_adapter_identifier;
struct _d3d_adapter_identifier
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

enum _d3d_caps_index
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
	CAPS_STRETCH_RECT_FILTER
};
typedef enum _d3d_caps_index d3d_caps_index;


//============================================================
//  Direct3D interfaces
//============================================================

typedef struct _d3d_interface d3d_interface;
struct _d3d_interface
{
	HRESULT  (*check_device_format)(d3d *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT adapterformat, DWORD usage, D3DRESOURCETYPE restype, D3DFORMAT format);
	HRESULT  (*check_device_type)(d3d *d3dptr, UINT adapter, D3DDEVTYPE devtype, D3DFORMAT format, D3DFORMAT backformat, BOOL windowed);
	HRESULT  (*create_device)(d3d *d3dptr, UINT adapter, D3DDEVTYPE devtype, HWND focus, DWORD behavior, d3d_present_parameters *params, d3d_device **dev);
	HRESULT  (*enum_adapter_modes)(d3d *d3dptr, UINT adapter, D3DFORMAT format, UINT index, D3DDISPLAYMODE *mode);
	UINT     (*get_adapter_count)(d3d *d3dptr);
	HRESULT  (*get_adapter_display_mode)(d3d *d3dptr, UINT adapter, D3DDISPLAYMODE *mode);
	HRESULT  (*get_adapter_identifier)(d3d *d3dptr, UINT adapter, DWORD flags, d3d_adapter_identifier *identifier);
	UINT     (*get_adapter_mode_count)(d3d *d3dptr, UINT adapter, D3DFORMAT format);
	HMONITOR (*get_adapter_monitor)(d3d *d3dptr, UINT adapter);
	HRESULT  (*get_caps_dword)(d3d *d3dptr, UINT adapter, D3DDEVTYPE devtype, d3d_caps_index which, DWORD *value);
	ULONG    (*release)(d3d *d3dptr);
};


//============================================================
//  Direct3DDevice interfaces
//============================================================

typedef struct _d3d_device_interface d3d_device_interface;
struct _d3d_device_interface
{
	HRESULT (*begin_scene)(d3d_device *dev);
	HRESULT (*clear)(d3d_device *dev, DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);
	HRESULT (*create_offscreen_plain_surface)(d3d_device *dev, UINT width, UINT height, D3DFORMAT format, D3DPOOL pool, d3d_surface **surface);
	HRESULT (*create_texture)(d3d_device *dev, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, d3d_texture **texture);
	HRESULT (*create_vertex_buffer)(d3d_device *dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, d3d_vertex_buffer **buf);
	HRESULT (*draw_primitive)(d3d_device *dev, D3DPRIMITIVETYPE type, UINT start, UINT count);
	HRESULT (*end_scene)(d3d_device *dev);
	HRESULT (*get_raster_status)(d3d_device *dev, D3DRASTER_STATUS *status);
	HRESULT (*get_render_target)(d3d_device *dev, DWORD index, d3d_surface **surface);
	HRESULT (*present)(d3d_device *dev, const RECT *source, const RECT *dest, HWND override, RGNDATA *dirty, DWORD flags);
	ULONG   (*release)(d3d_device *dev);
	HRESULT (*reset)(d3d_device *dev, d3d_present_parameters *params);
	void    (*set_gamma_ramp)(d3d_device *dev, DWORD flags, const D3DGAMMARAMP *ramp);
	HRESULT (*set_render_state)(d3d_device *dev, D3DRENDERSTATETYPE state, DWORD value);
	HRESULT (*set_render_target)(d3d_device *dev, DWORD index, d3d_surface *surf);
	HRESULT (*set_stream_source)(d3d_device *dev, UINT number, d3d_vertex_buffer *vbuf, UINT stride);
	HRESULT (*set_texture)(d3d_device *dev, DWORD stage, d3d_texture *tex);
	HRESULT (*set_texture_stage_state)(d3d_device *dev, DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	HRESULT (*set_vertex_shader)(d3d_device *dev, D3DFORMAT format);
	HRESULT (*stretch_rect)(d3d_device *dev, d3d_surface *source, const RECT *srcrect, d3d_surface *dest, const RECT *dstrect, D3DTEXTUREFILTERTYPE filter);
	HRESULT (*test_cooperative_level)(d3d_device *dev);
};


//============================================================
//  Direct3DSurface interfaces
//============================================================

typedef struct _d3d_surface_interface d3d_surface_interface;
struct _d3d_surface_interface
{
	HRESULT (*lock_rect)(d3d_surface *surf, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags);
	ULONG   (*release)(d3d_surface *tex);
	HRESULT (*unlock_rect)(d3d_surface *surf);
};


//============================================================
//  Direct3DTexture interfaces
//============================================================

typedef struct _d3d_texture_interface d3d_texture_interface;
struct _d3d_texture_interface
{
	HRESULT (*get_surface_level)(d3d_texture *tex, UINT level, d3d_surface **surface);
	HRESULT (*lock_rect)(d3d_texture *tex, UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags);
	ULONG   (*release)(d3d_texture *tex);
	HRESULT (*unlock_rect)(d3d_texture *tex, UINT level);
};


//============================================================
//  Direct3DVertexBuffer interfaces
//============================================================

typedef struct _d3d_vertex_buffer_interface d3d_vertex_buffer_interface;
struct _d3d_vertex_buffer_interface
{
	HRESULT (*lock)(d3d_vertex_buffer *vbuf, UINT offset, UINT size, VOID **data, DWORD flags);
	ULONG   (*release)(d3d_vertex_buffer *vbuf);
	HRESULT (*unlock)(d3d_vertex_buffer *vbuf);
};


//============================================================
//  Core D3D object
//============================================================

struct _d3d
{
	// internal objects
	int							version;
	void *						d3dobj;
	HINSTANCE					dllhandle;

	// interface pointers
	d3d_interface				d3d;
	d3d_device_interface		device;
	d3d_surface_interface		surface;
	d3d_texture_interface		texture;
	d3d_vertex_buffer_interface vertexbuf;
};


//============================================================
//  PROTOTYPES
//============================================================

d3d *drawd3d8_init(void);
d3d *drawd3d9_init(void);


#endif
