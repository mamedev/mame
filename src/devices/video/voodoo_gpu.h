// license:BSD-3-Clause
// copyright-holders: Ted Green

#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include <stdio.h>
#include <vector>
#include <map>
#include <queue>

#include <d3d11.h>
#include <directxcolors.h>
#include <d3dcompiler.h>
#include <directxmath.h>
using namespace DirectX;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

////////////////////////////////////
// Defines for 3dfx configuration //
////////////////////////////////////
// Number of texture mapping units (TMUs)
#define NUM_TEX 2

// Number of textures for use by TMUs
#define MAX_TEX 128

// Dither Texture t and s register offset
#define DITH_TEX_OFFSET 0

// Rendering Texture t and s register offset
#define COMP_TEX_OFFSET 2

// Starting texture and sampling register offset in gpu
#define TMU_TEX_OFFSET 3

// Depth of the pixel buffer for lfb writes through pixel pipeline
#define DEPTH_PIXEL_BUFFER 512

// Depth of the pixel buffer for lfb direct writes
#define DEPTH_LFB_WRITE_BUFFER 256

class voodoo_gpu
{
public:
	struct ShaderPoint
	{
		int intX;
		int intY;
		int intZ;
		int intW;
		int intR;
		int intG;
		int intB;
		int intA;
	};

	struct ShaderVertex
	{
		XMFLOAT4 Pos;
		XMFLOAT4 Col;
		XMFLOAT3 Tex0;
		XMFLOAT3 Tex1;
	};

	struct LFBWriteStruct
	{
		UINT32 x;
		UINT32 y;

	};

	struct frameBufferCtrl // register(b0)
	{
		float xSize;
		float ySize;
		int flipY;
		int enablePerspective0;
		int enablePerspective1;
		int pad0[3];
	};

	struct Combine_Struct {
		int c_zero_cother;
		int c_sub_clocal;
		int c_mselect;
		int c_reverse_blend;
		int c_add_aclocal;
		int c_invert_output;
		int a_zero_aother;
		int a_sub_alocal;
		int a_mselect;
		int a_reverse_blend;
		int a_add_aclocal;
		int a_invert_output;
	};

	struct Color_Ctrl_Struct // register(b1)
	{
		XMFLOAT4 color0;
		XMFLOAT4 color1;
		Combine_Struct colCtrl;
		int rgbselect;
		int aselect;
		int c_localselect;
		int a_localselect;
		int c_localselect_override;
		int enable_param_clamp;
		int enable_texture;
		// Depth Ctrl
		int depth_enable;
		int depthfloat_select;
		int wfloat_select;
		int zbias_enable;
		int depth_src_select;
		float zBias;  // From zaColor
		float zSrc;  // From zaColor
		// Alpha Testing
		int alpha_test_enable;
		int alpha_op;
		float alpha_ref;
		// Select pixel mode
		int pixel_mode;
		// Dithering
		int dither_sel;
		int pad1[1];
	};

	struct Tex_Ctrl_Struct
	{
		XMFLOAT2 texSize;
		int enable;
		int tloddither;
		float detailScale;
		float detailBias;
		float detailMax;
		int send_config;
		//float pad;
	};

	struct Tex_Interface_Struct  // register(b2)
	{
		Combine_Struct texCombine[NUM_TEX];
		Tex_Ctrl_Struct texCtrl[NUM_TEX];
		//int pad2[2];
	};

	struct Fog_Table_Struct // register(b3)
	{
		XMFLOAT4 fog_table[64]; // 8.0 fog alpha, 6.0 fog delta alpha, 1.0 fog delta alpha invert (2nd lsb of 6.2 fog delta alpha), pad
	};

	struct Fog_Ctrl_Struct // register(b4)
	{
		XMFLOAT3 fogColor;
		int fog_enable;
		int fogadd;
		int fogmult;
		int fogza;
		int fogconstant;
		int fogdither;
		int fogzones;
		float pad30, pad31;
	};

	struct texDescription
	{
		UINT32 lodMask;
		UINT32 sSize;
		UINT32 tSize;
		UINT32 texMask;
		UINT8  *ram;
		UINT32 *texBase;
		UINT32 *texLookup;
		UINT32 texFormat;
		bool sendConfig;
		UINT32 tmuConfig;
	};

	struct Tex_Map_List_Struct
	{
		ID3D11Texture2D*				texTexture;
		ID3D11ShaderResourceView*       texRV;
		ID3D11SamplerState*             texSampler;
	};

public:
	voodoo_gpu();
	~voodoo_gpu();
	enum RenderMode { TRIANGLE, PIXEL, FASTFILL };
	bool InitDevice();
	bool CompileShaders();
	bool InitBuffers(int sizeX, int sizeY);
	bool InitRenderBuffers(int sizeX, int sizeY, int fbiWidth);
	HRESULT CreateVertexShader(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11VertexShader** ppShaderOut, int mode = 0);
	HRESULT CreatePixelShader(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11PixelShader** ppShaderOut);
	HRESULT RenderToTex();
	void ReadTex();
	void FlushBuffer(void);
	void DrawFastFill(ShaderPoint *triangleVertices);
	void DrawTriangle(ShaderVertex *triangleVertices, uint16_t *dst);
	void DrawPixels();
	void FastFill(int sx, int ex, int sy, int ey, uint16_t *dst);
	void CopyBuffer(uint16_t *dst);
	void CopyBufferComp(uint16_t *dst);
	void CopyBufferRGB(uint8_t *dst);
	
	void SetFbzMode(UINT32 fbzMode);
	void SetAlphaMode(UINT32 &alphaMode);
	void SetZAColor(UINT32 zaColor);
	void SetFogCtrl(UINT32 &fogMode, UINT32 &fogColor);
	void SetFogTable(UINT32 &data, int &index);

	void UpdateDepth();
	void UpdateAlphaBlend();
	D3D11_BLEND ConvAlphaBlendOp(uint32_t alphaBlend, bool dest);
	void UpdateAlphaTest();
	void SetColorCtrl(UINT32 fbzColorPath, UINT32 color0, UINT32 color1);

	void UpdateColorCtrl();
	void UpdateFogCtrl();

	void UpdateTexCtrl(int enalbeTex0, int enableTex1);
	void UpdateConstants();
	void FlagTexture(int index, UINT32 *texBase, UINT32 &texLod);
	Combine_Struct ConvertTexmode(UINT32 &texMode);
	void CreateTexture(texDescription &desc, int index, UINT32 &texMode, UINT32 &texLod, UINT32 &texDetail);
	void PushPixel(int &x, int &y, int &mask, int *sr, int *sg, int *sb, int *sa, int *sz, UINT32 wSel, UINT32 &wVal, uint16_t *dst);

private:
	ID3D11Device* GetGPU();
	ID3D11DeviceContext* GetContext();

	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];

	ID3D11Device* m_gpu;
	ID3D11DeviceContext* m_context;

	RenderMode				m_renderMode;
	ID3D11InputLayout*      m_pVertexLayout;
	ID3D11Buffer*           m_pVertexBuffer;
	ID3D11Buffer*           m_pixelBuffer;

	ID3D11VertexShader*     m_pVS;
	ID3D11VertexShader*     m_pixelVS;
	ID3D11PixelShader*      m_pPS;
	ID3D11PixelShader*      m_fastFillPS;

	ID3D11InputLayout*      m_pixelVertexLayout;
	ID3D11VertexShader*     m_compVS;
	ID3D11PixelShader*      m_compPS;

	ID3D11Texture2D* m_renderTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_renderResourceView;
	ID3D11SamplerState* m_renderState;

	ID3D11Texture2D* m_compTexture;
	ID3D11RenderTargetView* m_compTargetView;
	ID3D11ShaderResourceView* m_compResourceView;

	ID3D11Texture2D* m_dither4x4Texture;
	ID3D11ShaderResourceView* m_dither4x4ResourceView;
	ID3D11Texture2D* m_dither2x2Texture;
	ID3D11ShaderResourceView* m_dither2x2ResourceView;
	ID3D11SamplerState* m_ditherState;

	ID3D11Texture2D*				m_texTexture[MAX_TEX];
	ID3D11ShaderResourceView*       m_texRV[MAX_TEX];
	ID3D11SamplerState*             m_texSampler[MAX_TEX];
	std::map<UINT32, Tex_Map_List_Struct> m_texMap;
	std::queue<UINT32> m_texHist;

	ID3D11Texture2D* m_depthBuffer;
	ID3D11DepthStencilState* m_depthState;
	ID3D11DepthStencilState* m_depthFastFillState;
	ID3D11DepthStencilView* m_depthView;
	ID3D11RasterizerState* m_rasterState;
	ID3D11BlendState* m_blendState;

	ID3D11Buffer* m_frameBufferCtrlBuf;
	ID3D11Buffer* m_colorCtrlBuf;
	ID3D11Buffer* m_texCtrlBuf;
	ID3D11Buffer* m_fogCtrlBuf;
	ID3D11Buffer* m_fogTableBuf;

	bool m_updateFrameBufferCtrl, m_updateColorCtrl, m_updateTexCtrl;
	frameBufferCtrl m_frameBufferCtrl;
	Color_Ctrl_Struct m_colorCtrl;
	Tex_Interface_Struct m_texCtrl;
	std::vector<ShaderPoint> m_pixelPoints;
	std::vector<LFBWriteStruct> m_lfbWritePoints;

	bool m_updateBlendState;		// Blending
	bool m_updateAlphaTest;			// Alpha Testing
	D3D11_BLEND_DESC m_blendDesc;
	bool m_updateDepth;				// Depth Testing

	bool m_updateFogTable; // Fog table
	bool m_updateFogCtrl; // Fog control
	Fog_Ctrl_Struct m_fogCtrl;
	Fog_Table_Struct m_fogTable;

	bool m_need_copy;
	bool m_pixels_ready;
	bool m_lfb_write_ready;

	int m_fbiWidth;
	uint16_t *m_destBuffer;

	UINT32 m_fastFbzMode;
	UINT32 m_regFbzMode, m_regFbzColorPath, m_regColor0, m_regColor1, m_regAlphaMode, m_regTexMode[NUM_TEX];
	UINT32 m_regZAColor, m_regFogMode, m_regFogColor;
};
