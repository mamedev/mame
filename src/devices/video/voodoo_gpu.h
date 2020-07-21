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
#include <sstream>
#include <string>

#include <d3d11.h>
#include <directxcolors.h>
#include <d3dcompiler.h>
#include <directxmath.h>
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

////////////////////////////////////
// Defines for 3dfx configuration //
////////////////////////////////////
// Use dynamic buffers
//#define USE_MAPPED

// Number of texture mapping units (TMUs)
#define NUM_TEX 2

// Number of textures for use by TMUs
#define MAX_TEX 2048

// Dither Texture t and s register offset
#define DITH_TEX_OFFSET 0

// Rendering Texture t and s register offset
#define COMP_TEX_OFFSET 2

// Starting texture and sampling register offset in gpu
#define TMU_TEX_OFFSET 3

// Depth of the pixel buffer for lfb writes through pixel pipeline and direct
#define DEPTH_PIXEL_BUFFER 1024

// Depth of triangle vertex buffer for triangle drawing
#define DEPTH_TRIANGLE_BUFFER 1024

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

	struct frameBufferCtrl // register(b0)
	{
		float xSize;
		float ySize;
		int fbzFlipY;
		int lfbFlipY;
		int enablePerspective0;
		int enablePerspective1;
		int pad0[2];
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
		uint32_t lodMask;
		uint32_t sSize;
		uint32_t tSize;
		uint32_t texMask;
		uint8_t  *ram;
		uint32_t *texBase;
		uint32_t *texLookup;
		uint32_t texFormat;
		bool sendConfig;
		uint32_t tmuConfig;
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
	enum RenderMode { TRIANGLE, PIXEL, DIRECTWRITE, FASTFILL };
	bool InitDevice();
	bool CompileShaders(std::wstring pSrcDir);
	bool InitBuffers(int sizeX, int sizeY);
	bool InitRenderBuffers(int sizeX, int sizeY, int fbiWidth);
	HRESULT CreateVertexShaderFile(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11VertexShader** ppShaderOut, int mode = 0);
	HRESULT CreatePixelShaderFile(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11PixelShader** ppShaderOut);
	HRESULT RenderToTex();
	void ReadTex();
	// Flushes pending pixels and copies frame buffer to local memory
	void FlushBuffer(void);
	void DrawFastFill(ShaderPoint *triangleVertices);
	void DrawTriangle();
	// Sends any pixels in the pixel proccessing pipeline queue to the gpu to be processed
	void DrawPixels();
	// Sends any lfb writes in the lfb write queue to the gpu to be processed
	void DrawLfbWrites();
	void FastFill(int sx, int ex, int sy, int ey, uint16_t *dst, int drawIndex);
	// Copies gpu frame buffer to local frame buffer if there has been changes
	void CopyBuffer(uint16_t *dst);
	void CopyBufferComp(uint16_t *dst);
	void CopyBufferRGB(uint8_t *dst);
	std::string SVInfo(const ShaderVertex sv);
	std::string GpuInfo(void);

	void SetFbzMode(uint32_t fbzMode);
	void SetLfbMode(uint32_t lfbMode);
	void SetAlphaMode(uint32_t &alphaMode);
	void SetZAColor(uint32_t zaColor);
	void SetFogCtrl(uint32_t &fogMode, uint32_t &fogColor);
	void SetFogTable(uint32_t &data, int &index);

	void UpdateDepth();
	void UpdateAlphaBlend();
	D3D11_BLEND ConvAlphaBlendOp(uint32_t alphaBlend, bool dest);
	void UpdateAlphaTest();
	void SetColorCtrl(uint32_t fbzColorPath, uint32_t color0, uint32_t color1);

	void UpdateColorCtrl();
	void UpdateFogCtrl();

	void UpdateTexCtrl(int enalbeTex0, int enableTex1);
	// Transfer any control structure updates from local to gpu registers
	void UpdateConstants();
	void FlagTexture(uint32_t &offset);
	void FlagTexture(int index, uint32_t *texBase, uint32_t &texLod);
	Combine_Struct ConvertTexmode(uint32_t &texMode);
	void CreateTexture(texDescription &desc, int index, uint32_t &texMode, uint32_t &texLod, uint32_t &texDetail);
	// Pushes a triangle onto the triangle processing queue
	void PushTriangle(ShaderVertex *triangleVertices, uint16_t *dst, int drawIndex);
	int NumTrianglePending() { return m_trianglePoints.size() / 3; };
	// Pushes a pixel onto LFB 3d processing queue
	void PushPixel(int &x, int &y, int &mask, uint8_t *sr, uint8_t *sg, uint8_t *sb, int *sa, int *sz, uint32_t wSel, uint32_t &wVal, uint16_t *dst, int drawIndex);
	// Pushes a RGB write onto the lfb write queue
	void PushLfbWrite(int &x, int &y, int &mask, uint8_t *sr, uint8_t *sg, uint8_t *sb, int *depth, uint16_t *dst, int drawIndex);
	void SetDrawIndex(int index) { m_drawIndex = index; };
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

	ID3D11Texture2D* m_stageTexture;

	ID3D11Texture2D* m_dither4x4Texture;
	ID3D11ShaderResourceView* m_dither4x4ResourceView;
	ID3D11Texture2D* m_dither2x2Texture;
	ID3D11ShaderResourceView* m_dither2x2ResourceView;
	ID3D11SamplerState* m_ditherState;

	ID3D11Texture2D*				m_texTexture[MAX_TEX];
	ID3D11ShaderResourceView*       m_texRV[MAX_TEX];
	ID3D11SamplerState*             m_texSampler[MAX_TEX];
	std::map<uint32_t, Tex_Map_List_Struct> m_texMap;
	std::deque<uint32_t> m_texHist;

	ID3D11Texture2D* m_depthBuffer;
	ID3D11DepthStencilState* m_depthState;
	// Always write depth
	ID3D11DepthStencilState* m_depthAlwaysState;
	// Never write depth
	ID3D11DepthStencilState* m_depthNeverState;
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
	// Queue for triangle drawing
	std::vector<ShaderVertex> m_trianglePoints;
	// Queue for LFB 3D pixel pipeline writes
	std::vector<ShaderPoint> m_pixelPoints;
	// Queue for LFB direct writes
	std::vector<ShaderPoint> m_lfbWritePoints;

	bool m_updateBlendState;		// Blending
	bool m_updateAlphaTest;			// Alpha Testing
	D3D11_BLEND_DESC m_blendDesc;
	bool m_updateDepth;				// Depth Testing

	bool m_updateFogTable; // Fog table
	bool m_updateFogCtrl; // Fog control
	Fog_Ctrl_Struct m_fogCtrl;
	Fog_Table_Struct m_fogTable;

	bool m_need_copy;
	bool m_triangles_ready;
	bool m_pixels_ready;
	bool m_lfb_write_ready;

	int m_fbiWidth;
	int m_drawIndex;
	uint16_t *m_drawBuffer;

	uint32_t m_fastFbzMode;
	uint32_t m_regFbzMode, m_regLfbMode, m_regFbzColorPath, m_regColor0, m_regColor1, m_regAlphaMode, m_regTexMode[NUM_TEX];
	uint32_t m_regZAColor, m_regFogMode, m_regFogColor;
};
