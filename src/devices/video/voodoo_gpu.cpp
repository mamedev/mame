// license:BSD-3-Clause
// copyright-holders: Ted Green

//
// 3dfx Voodoo Graphics acceleration using DirectX 11
//

#include "voodoo_gpu.h"

voodoo_gpu::voodoo_gpu()
{
	m_gpu = NULL;
	m_context = NULL;

	m_renderMode = TRIANGLE;
	m_colorCtrl.pixel_mode = 0;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
	m_pixelBuffer = nullptr;

	m_pixelPoints.reserve(DEPTH_PIXEL_BUFFER);
	m_pixels_ready = false;

	m_lfbWritePoints.reserve(DEPTH_LFB_WRITE_BUFFER);
	m_lfb_write_ready = false;

	m_pVS = nullptr;
	m_pixelVS = nullptr;
	m_pPS = nullptr;
	m_fastFillPS = nullptr;

	m_pixelVertexLayout = nullptr;
	m_compVS = nullptr;
	m_compPS = nullptr;

	m_dither4x4Texture = NULL;
	m_dither2x2Texture = NULL;
	m_dither4x4ResourceView = NULL;
	m_dither2x2ResourceView = NULL;
	m_ditherState = NULL;

	m_renderTexture = NULL;
	m_renderTargetView = NULL;
	m_renderResourceView = NULL;
	m_renderState = NULL;

	m_compTexture = NULL;
	m_compTargetView = NULL;
	m_compResourceView = NULL;

	m_depthBuffer = NULL;
	m_depthState = NULL;
	m_depthView = NULL;
	m_rasterState = NULL;
	m_blendState = NULL;

	m_frameBufferCtrlBuf = NULL;
	m_colorCtrlBuf = NULL;
	m_texCtrlBuf = NULL;
	m_fogCtrlBuf = NULL;
	m_fogTableBuf = NULL;

	m_frameBufferCtrl.xSize = 0.0f;
	m_frameBufferCtrl.ySize = 0.0f;
	m_frameBufferCtrl.flipY = 0;
	m_frameBufferCtrl.enablePerspective0 = 0;
	m_frameBufferCtrl.enablePerspective1 = 0;

	m_updateFrameBufferCtrl = true;
	m_updateTexCtrl = false;

	m_regFbzMode = 0;

	m_updateColorCtrl = false;
	m_regFbzColorPath = 0;
	m_regColor0 = 0;
	m_regColor1 = 0;
	for (size_t i = 0; i < NUM_TEX; i++) {
		m_regTexMode[i] = 0;
	}

	m_updateBlendState = false;
	m_updateAlphaTest = false;
	m_regAlphaMode = 0;
	ZeroMemory(&m_blendDesc, sizeof(D3D11_BLEND_DESC));
	m_updateDepth = false;
	m_regZAColor = 0;

	m_updateFogTable = false;
	m_updateFogCtrl = false;
	m_regFogMode = 0;
	m_regFogColor = 0;

	m_need_copy = false;
}

voodoo_gpu::~voodoo_gpu()
{
	for (std::map<UINT32, Tex_Map_List_Struct>::iterator ii = m_texMap.begin(); ii != m_texMap.end(); ++ii)
	{
		SAFE_RELEASE((*ii).second.texTexture);
		SAFE_RELEASE((*ii).second.texRV);
		SAFE_RELEASE((*ii).second.texSampler);
	}

	SAFE_RELEASE(m_frameBufferCtrlBuf);
	SAFE_RELEASE(m_colorCtrlBuf);
	SAFE_RELEASE(m_texCtrlBuf);
	SAFE_RELEASE(m_fogCtrlBuf);
	SAFE_RELEASE(m_fogTableBuf);

	SAFE_RELEASE(m_dither4x4Texture);
	SAFE_RELEASE(m_dither2x2Texture);
	SAFE_RELEASE(m_dither4x4ResourceView);
	SAFE_RELEASE(m_dither2x2ResourceView);
	SAFE_RELEASE(m_ditherState);
	SAFE_RELEASE(m_renderResourceView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_renderTexture);
	SAFE_RELEASE(m_renderState);
	SAFE_RELEASE(m_compResourceView);
	SAFE_RELEASE(m_compTargetView);
	SAFE_RELEASE(m_compTexture);
	SAFE_RELEASE(m_depthBuffer);
	SAFE_RELEASE(m_depthState);
	SAFE_RELEASE(m_depthView);
	SAFE_RELEASE(m_rasterState);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_pVS);
	SAFE_RELEASE(m_pixelVS);
	SAFE_RELEASE(m_pPS);
	SAFE_RELEASE(m_fastFillPS);
	SAFE_RELEASE(m_pixelVertexLayout);
	SAFE_RELEASE(m_compVS);
	SAFE_RELEASE(m_compPS);
	SAFE_RELEASE(m_pixelBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_context);
	SAFE_RELEASE(m_gpu);
}

bool voodoo_gpu::InitDevice()
{
	m_gpu = nullptr;
	m_context = nullptr;

	HRESULT hr = S_OK;

	UINT uCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
	uCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL flOut;
	static const D3D_FEATURE_LEVEL flvl[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

	bool bNeedRefDevice = false;
	bool bForceRef = false;
	if (!bForceRef)
	{
		hr = D3D11CreateDevice(nullptr,                        // Use default graphics card
			D3D_DRIVER_TYPE_HARDWARE,    // Try to create a hardware accelerated device
			nullptr,                        // Do not use external software rasterizer module
			uCreationFlags,              // Device creation flags
			flvl,
			sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,           // SDK version
			&m_gpu,                 // Device out
			&flOut,                      // Actual feature level created
			&m_context);              // Context out

		if (SUCCEEDED(hr))
		{
			// A hardware accelerated device has been created, so check for Compute Shader support

			// If we have a device >= D3D_FEATURE_LEVEL_11_0 created, full CS5.0 support is guaranteed, no need for further checks
			if (flOut < D3D_FEATURE_LEVEL_11_0)
			{
				// Otherwise, we need further check whether this device support CS4.x (Compute on 10)
				D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
				(m_gpu)->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
				if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
				{
					bNeedRefDevice = true;
					printf("No hardware Compute Shader capable device found, trying to create ref device.\n");
				}
			}

		}
	}

	if (bForceRef || FAILED(hr) || bNeedRefDevice)
	{
		// Either because of failure on creating a hardware device or hardware lacking CS capability, we create a ref device here

		SAFE_RELEASE(m_gpu);
		SAFE_RELEASE(m_context);

		hr = D3D11CreateDevice(nullptr,                        // Use default graphics card
			D3D_DRIVER_TYPE_REFERENCE,   // Try to create a hardware accelerated device
			nullptr,                        // Do not use external software rasterizer module
			uCreationFlags,              // Device creation flags
			flvl,
			sizeof(flvl) / sizeof(D3D_FEATURE_LEVEL),
			D3D11_SDK_VERSION,           // SDK version
			&m_gpu,                 // Device out
			&flOut,                      // Actual feature level created
			&m_context);              // Context out
		if (FAILED(hr))
		{
			printf("Reference rasterizer device create failure\n");
			return false;
		}
	}

	return true;
}

bool voodoo_gpu::CompileShaders()
{

	// Compile vertex and pixel shader routines
	SAFE_RELEASE(m_pVS);
	if (FAILED(CreateVertexShader(L"src/devices/video/voodoo_gpu_vs.hlsl", "VS", &m_pVS, 1))) {
		printf("voodoo_gpu:: Error could not compile VS\n");
		return false;
	}

	SAFE_RELEASE(m_pixelVS);
	if (FAILED(CreateVertexShader(L"src/devices/video/voodoo_gpu_vs.hlsl", "PIXEL_VS", &m_pixelVS, 2))) {
		printf("voodoo_gpu:: Error could not compile PIXEL_VS\n");
		return false;
	}

	SAFE_RELEASE(m_pPS);
	if (FAILED(CreatePixelShader(L"src/devices/video/voodoo_gpu_ps.hlsl", "PS", &m_pPS))) {
		printf("voodoo_gpu:: Error could not compile PS\n");
		return false;
	}

	SAFE_RELEASE(m_fastFillPS);
	if (FAILED(CreatePixelShader(L"src/devices/video/voodoo_gpu_ps.hlsl", "FASTFILL_PS", &m_fastFillPS))) {
		printf("voodoo_gpu:: Error could not compile FASTFILL_PS\n");
		return false;
	}

	// Compile vertex and pixel shader routines for compressed buffer
	SAFE_RELEASE(m_compVS);
	if (FAILED(CreateVertexShader(L"src/devices/video/voodoo_gpu_vs.hlsl", "COMP_VS", &m_compVS))) {
		printf("voodoo_gpu:: Error could not compile COMP_VS\n");
		return false;
	}

	SAFE_RELEASE(m_compPS);
	if (FAILED(CreatePixelShader(L"src/devices/video/voodoo_gpu_ps.hlsl", "COMP_PS", &m_compPS))) {
		printf("voodoo_gpu:: Error could not compile COMP_PS\n");
		return false;
	}

	return true;
}

bool voodoo_gpu::InitRenderBuffers(int sizeX, int sizeY, int fbiWidth)
{
	m_fbiWidth = fbiWidth;

	if (m_frameBufferCtrl.xSize == sizeX && m_frameBufferCtrl.ySize == sizeY)
		return true;

	// Clear all textures
	if (!m_texHist.empty()) {
		for (std::map<UINT32, Tex_Map_List_Struct>::iterator ii = m_texMap.begin(); ii != m_texMap.end(); ++ii)
		{
			SAFE_RELEASE((*ii).second.texTexture);
			SAFE_RELEASE((*ii).second.texRV);
			SAFE_RELEASE((*ii).second.texSampler);
		}
		m_texMap.clear();
		while (!m_texHist.empty())
			m_texHist.pop();
	}

	// ViewPort
	D3D11_VIEWPORT vp;
	vp.Width = float(sizeX);
	vp.Height = float(sizeY);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	// TODO: Add clip window
	m_context->RSSetViewports(1, &vp);

	// Set the constant buffer parameters
	m_frameBufferCtrl.xSize = float(sizeX);
	m_frameBufferCtrl.ySize = float(sizeY);
	m_updateFrameBufferCtrl = true;

	// PASS0 Buffers
	// Create the render target texture
	D3D11_TEXTURE2D_DESC textureDesc = { 0 };
	textureDesc.Width = sizeX;
	textureDesc.Height = sizeY;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	SAFE_RELEASE(m_renderTexture);
	if (FAILED(m_gpu->CreateTexture2D(&textureDesc, NULL, &m_renderTexture)))
		return false;

	// Create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	SAFE_RELEASE(m_renderTargetView);
	if (FAILED(m_gpu->CreateRenderTargetView(m_renderTexture, &renderTargetViewDesc, &m_renderTargetView)))
		return false;

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	SAFE_RELEASE(m_renderResourceView);
	if (FAILED(m_gpu->CreateShaderResourceView(m_renderTexture, &shaderResourceViewDesc, &m_renderResourceView)))
		return false;

	// PASS1 Buffers
	// Create the comp render target texture.
	textureDesc = { 0 };
	textureDesc.Width = sizeX;
	textureDesc.Height = sizeY;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	SAFE_RELEASE(m_compTexture);
	if (FAILED(m_gpu->CreateTexture2D(&textureDesc, NULL, &m_compTexture)))
		return false;

	// Create the comp render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	SAFE_RELEASE(m_compTargetView);
	if (FAILED(m_gpu->CreateRenderTargetView(m_compTexture, &renderTargetViewDesc, &m_compTargetView)))
		return false;

	// Create the comp shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	SAFE_RELEASE(m_compResourceView);
	if (FAILED(m_gpu->CreateShaderResourceView(m_compTexture, &shaderResourceViewDesc, &m_compResourceView)))
		return false;

	// Create depth buffer
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = sizeX;
	descDepth.Height = sizeY;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D16_UNORM;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	SAFE_RELEASE(m_depthBuffer);
	if (FAILED(m_gpu->CreateTexture2D(&descDepth, nullptr, &m_depthBuffer)))
		return false;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	SAFE_RELEASE(m_depthView);
	if (FAILED(m_gpu->CreateDepthStencilView(m_depthBuffer, &descDSV, &m_depthView)))
		return false;

	return true;
}

bool voodoo_gpu::InitBuffers(int sizeX, int sizeY)
{
	// Compile the shaders
	if(!CompileShaders())
		return false;

	// Initialize the rendering buffers
	if (!InitRenderBuffers(sizeX, sizeY, sizeX))
		return false;

	// Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	bd.ByteWidth = sizeof(frameBufferCtrl);
	if (FAILED(m_gpu->CreateBuffer(&bd, nullptr, &m_frameBufferCtrlBuf)))
		return false;

	bd.ByteWidth = sizeof(Color_Ctrl_Struct);
	if (FAILED(m_gpu->CreateBuffer(&bd, nullptr, &m_colorCtrlBuf)))
		return false;

	bd.ByteWidth = sizeof(Tex_Interface_Struct);
	if (FAILED(m_gpu->CreateBuffer(&bd, nullptr, &m_texCtrlBuf)))
		return false;

	bd.ByteWidth = sizeof(Fog_Ctrl_Struct);
	if (FAILED(m_gpu->CreateBuffer(&bd, nullptr, &m_fogCtrlBuf)))
		return false;

	bd.ByteWidth = sizeof(Fog_Table_Struct);
	if (FAILED(m_gpu->CreateBuffer(&bd, nullptr, &m_fogTableBuf)))
		return false;

	// Vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShaderVertex) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	if (FAILED(m_gpu->CreateBuffer(&bd, NULL, &m_pVertexBuffer)))
		return false;
	
	// Pixel buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShaderPoint) * DEPTH_PIXEL_BUFFER;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	if (FAILED(m_gpu->CreateBuffer(&bd, NULL, &m_pixelBuffer)))
		return false;

	// Render Sampler
	D3D11_SAMPLER_DESC rendDesc;
	ZeroMemory(&rendDesc, sizeof(rendDesc));
	rendDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	rendDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	rendDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	rendDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	rendDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	rendDesc.MinLOD = 0;
	rendDesc.MaxLOD = 0;
	if (FAILED(m_gpu->CreateSamplerState(&rendDesc, &m_renderState)))
		return false;

	// Create the depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Depth test parameters
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	if (FAILED(m_gpu->CreateDepthStencilState(&depthStencilDesc, &m_depthState)))
		return false;

	// Dithering Textures
	uint8_t dither_maxtrix_4x4[4][4] = { { 0, 8, 2, 10 }, { 12, 4, 14, 6 }, { 3, 11, 1, 9 }, { 15, 7, 13, 5 } };
	//uint8_t dither_maxtrix_2x2[4][4] = { { 2, 10, 2, 10 },{ 14, 6, 14, 6 },{ 2, 10, 2, 10 },{ 14, 6, 14, 6 } };
	// TODO: Check, textureMode tlodither description says dithering should average 3/8 (0.375)
	uint8_t dither_maxtrix_2x2[4][4] = { { 0, 8, 0, 8 }, { 12, 4, 12, 4 }, { 0, 8, 0, 8 }, { 12, 4, 12, 4 } };
	D3D11_SUBRESOURCE_DATA ditherData;
	ditherData.SysMemPitch = 4;
	ditherData.SysMemSlicePitch = 0;
	
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 4;
	desc.Height = 4;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ditherData.pSysMem = dither_maxtrix_4x4;
	if (FAILED(m_gpu->CreateTexture2D(&desc, &ditherData, &m_dither4x4Texture)))
		return false;

	ditherData.pSysMem = dither_maxtrix_2x2;
	if (FAILED(m_gpu->CreateTexture2D(&desc, &ditherData, &m_dither2x2Texture)))
		return false;

	// Dither Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC pixelResourceViewDesc;
	pixelResourceViewDesc.Format = desc.Format;
	pixelResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	pixelResourceViewDesc.Texture2D.MostDetailedMip = 0;
	pixelResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(m_gpu->CreateShaderResourceView(m_dither4x4Texture, &pixelResourceViewDesc, &m_dither4x4ResourceView)))
		return false;

	if (FAILED(m_gpu->CreateShaderResourceView(m_dither2x2Texture, &pixelResourceViewDesc, &m_dither2x2ResourceView)))
		return false;

	// Dither Sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_gpu->CreateSamplerState(&sampDesc, &m_ditherState)))
		return false;

	// Rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerState.AntialiasedLineEnable = FALSE;
	rasterizerState.CullMode = D3D11_CULL_NONE; // D3D11_CULL_FRONT or D3D11_CULL_NONE D3D11_CULL_BACK or D3D11_CULL_NONE
	rasterizerState.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_SOLID  D3D11_FILL_WIREFRAME
	rasterizerState.DepthBias = 0;
	rasterizerState.DepthBiasClamp = 0.0f;
	rasterizerState.DepthClipEnable = FALSE;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.ScissorEnable = FALSE;
	rasterizerState.SlopeScaledDepthBias = 0.0f;

	if (FAILED(m_gpu->CreateRasterizerState(&rasterizerState, &m_rasterState)))
		return false;

	// Blending
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_blendDesc = blendDesc;
	if (FAILED(m_gpu->CreateBlendState(&blendDesc, &m_blendState)))
		return false;

	// Set the input layout
	m_context->IASetInputLayout(m_pVertexLayout);

	// Set vertex buffer
	UINT stride = sizeof(ShaderVertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set primitive topology
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Setup gpu chain
	m_context->RSSetState(m_rasterState);

	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	m_context->OMSetDepthStencilState(m_depthState, 0);

	m_context->VSSetShader(m_pVS, nullptr, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_frameBufferCtrlBuf);
	m_context->VSSetConstantBuffers(1, 1, &m_colorCtrlBuf);
	m_context->VSSetConstantBuffers(2, 1, &m_texCtrlBuf);
	//m_context->VSSetConstantBuffers(3, 1, &m_fogTableBuf);
	//m_context->VSSetConstantBuffers(4, 1, &m_fogCtrlBuf);

	m_context->PSSetShader(m_pPS, nullptr, 0);
	m_context->PSSetConstantBuffers(0, 1, &m_frameBufferCtrlBuf);
	m_context->PSSetConstantBuffers(1, 1, &m_colorCtrlBuf);
	m_context->PSSetConstantBuffers(2, 1, &m_texCtrlBuf);
	m_context->PSSetConstantBuffers(3, 1, &m_fogTableBuf);
	m_context->PSSetConstantBuffers(4, 1, &m_fogCtrlBuf);
	m_context->PSSetShaderResources(DITH_TEX_OFFSET, 1, &m_dither4x4ResourceView);
	m_context->PSSetShaderResources(DITH_TEX_OFFSET+1, 1, &m_dither2x2ResourceView);
	m_context->PSSetSamplers(DITH_TEX_OFFSET, 1, &m_ditherState);

	return true;
}

ID3D11Device* voodoo_gpu::GetGPU()
{
	return m_gpu;
}

ID3D11DeviceContext* voodoo_gpu::GetContext()
{
	return m_context;
}

HRESULT voodoo_gpu::CreateVertexShader(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11VertexShader** ppShaderOut, int mode)
{
	HRESULT result;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	result = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, "vs_4_1", dwShaderFlags, 0, &pVSBlob, &pErrorBlob);
	if (FAILED(result))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return result;
	}
	if (pErrorBlob) pErrorBlob->Release();

	if (FAILED(result))
	{
		printf("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.\n");
		return result;
	}

	// Create the vertex shader
	result = m_gpu->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, ppShaderOut);
	if (FAILED(result))
	{
		pVSBlob->Release();
		return result;
	}

	// Define the input layout
	if (mode==1) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the input layout
		result = m_gpu->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &m_pVertexLayout);
		pVSBlob->Release();
		if (FAILED(result))
			return result;
	}
	else  if (mode==2) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the input layout
		result = m_gpu->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &m_pixelVertexLayout);
		pVSBlob->Release();
		if (FAILED(result))
			return result;

	}
	return result;
}
HRESULT voodoo_gpu::CreatePixelShader(LPCWSTR pSrcFile, LPCSTR pFunctionName, ID3D11PixelShader** ppShaderOut)
{
	HRESULT result;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	result = D3DCompileFromFile(pSrcFile, nullptr, nullptr, pFunctionName, "ps_4_1", dwShaderFlags, 0, &pVSBlob, &pErrorBlob);
	if (FAILED(result))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return result;
	}
	if (pErrorBlob) pErrorBlob->Release();

	if (FAILED(result))
	{
		printf("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.\n");
		return result;
	}

	// Create the vertex shader
	result = m_gpu->CreatePixelShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, ppShaderOut);
	if (FAILED(result))
	{
		pVSBlob->Release();
		return result;
	}

	return result;
}

void voodoo_gpu::SetFbzMode(UINT32 fbzMode)
{
	// Make sure pixel pipe is clear
	DrawPixels();

	// Y origin for render operations (fastfill, triangle, pixel pipeline lfb)
	if ((m_regFbzMode ^ fbzMode) & 0x20000) {
		m_frameBufferCtrl.flipY = (fbzMode >> 17) & 1;
		m_updateFrameBufferCtrl = true;
	}

	// Dithering
	if ((m_regFbzMode ^ fbzMode) & 0x900)
		m_updateColorCtrl = true;

	// Only update when enable depth is switched on/off or if compare op changes while enabled
	if (((m_regFbzMode ^ fbzMode) & 0x10) || (((m_regFbzMode ^ fbzMode ) & 0x4f0) && (fbzMode & 0x10)))
		m_updateDepth = true;

	// Depth testing
	if ((m_regFbzMode ^ fbzMode) & 0x00310018)
		m_updateColorCtrl = true;

	m_regFbzMode = fbzMode;
}

void voodoo_gpu::SetAlphaMode(UINT32 &alphaMode)
{
	// Make sure pixel pipe is clear
	DrawPixels();

	if ((m_regAlphaMode ^ alphaMode) & 0xf000000f)
	{
		m_updateAlphaTest = true;
	}
	if ((m_regAlphaMode ^ alphaMode) & 0x0ffffff0)
	{
		m_updateBlendState = true;
	}
	m_regAlphaMode = alphaMode;
}

void voodoo_gpu::SetZAColor(UINT32 zaColor)
{
	// Make sure pixel pipe is clear
	DrawPixels();

	if (m_regZAColor != zaColor)
		m_updateColorCtrl = true;

	m_regZAColor = zaColor;
}

void voodoo_gpu::SetFogCtrl(UINT32 &fogMode, UINT32 &fogColor)
{
	// Make sure pixel pipe is clear
	DrawPixels();

	if (m_regFogMode != fogMode)
		m_updateFogCtrl = true;
	
	if (m_regFogColor != fogColor)
		m_updateFogCtrl = true;

	m_regFogMode = fogMode;
	m_regFogColor = fogColor;
}

void voodoo_gpu::SetColorCtrl(UINT32 fbzColorPath, UINT32 color0, UINT32 color1)
{
	// Make sure pixel pipe is clear
	DrawPixels();

	if (m_regFbzColorPath != fbzColorPath) {
		m_updateColorCtrl = true;
		m_regFbzColorPath = fbzColorPath;
	}
	if (m_regColor0 != color0) {
		m_updateColorCtrl = true;
		m_regColor0 = color0;
	}
	if (m_regColor1 != color1) {
		m_updateColorCtrl = true;
		m_regColor1 = color1;
	}
}

void voodoo_gpu::UpdateDepth()
{
	HRESULT result;

	// Create the depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Depth test parameters
	depthStencilDesc.DepthEnable = (m_regFbzMode >> 4) & 1;
	depthStencilDesc.DepthWriteMask = ((m_regFbzMode >> 10) & 1) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	UINT32 depthOp = (m_regFbzMode >> 5) & 7;
	switch (depthOp) {
	case 0:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
		break;
	case 1:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		break;
	case 2:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		break;
	case 3:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case 4:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		break;
	case 5:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case 6:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	case 7:
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		break;
	}
	// Release old state
	SAFE_RELEASE(m_depthState);
	// Create New
	result = m_gpu->CreateDepthStencilState(&depthStencilDesc, &m_depthState);

	/*
	// Release old state
	SAFE_RELEASE(m_depthNoWriteState);
	// Create the depth stencil state with no writes
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // Do not write to depth buffer
	result = m_gpu->CreateDepthStencilState(&depthStencilDesc, &m_depthNoWriteState);
	*/
}

D3D11_BLEND voodoo_gpu::ConvAlphaBlendOp(uint32_t alphaBlend, bool dest)
{
	switch (alphaBlend) {
	case 0:
		return D3D11_BLEND_ZERO;
	case 1:
		return D3D11_BLEND_SRC_ALPHA;
	case 2:
		if (dest)
			return D3D11_BLEND_SRC_COLOR;
		else
			return D3D11_BLEND_DEST_COLOR;
	case 3:
		return D3D11_BLEND_DEST_ALPHA;
	case 4:
		return D3D11_BLEND_ONE;
	case 5:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case 6:
		if (dest)
			return D3D11_BLEND_INV_SRC_COLOR;
		else
			return D3D11_BLEND_INV_DEST_COLOR;
	case 7:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case 15:
		if (dest)
			return D3D11_BLEND_SRC1_COLOR; // Color before fog
		else
			return D3D11_BLEND_SRC_ALPHA_SAT;  // Source
	}
	return D3D11_BLEND_ZERO; // Error
}

void voodoo_gpu::UpdateAlphaBlend()
{
	if ((m_regAlphaMode >> 4) & 1) {
		m_blendDesc.RenderTarget[0].BlendEnable = TRUE;
		m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		m_blendDesc.RenderTarget[0].SrcBlend = ConvAlphaBlendOp((m_regAlphaMode >> 8) & 0xf, false);
		m_blendDesc.RenderTarget[0].DestBlend = ConvAlphaBlendOp((m_regAlphaMode >> 12) & 0xf, true);
		m_blendDesc.RenderTarget[0].SrcBlendAlpha = ConvAlphaBlendOp((m_regAlphaMode >> 16) & 0xf, false);
		m_blendDesc.RenderTarget[0].DestBlendAlpha = ConvAlphaBlendOp((m_regAlphaMode >> 20) & 0xf, true);

		m_blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	} else {
		m_blendDesc.RenderTarget[0].BlendEnable = FALSE;
	}
	SAFE_RELEASE(m_blendState);
	m_gpu->CreateBlendState(&m_blendDesc, &m_blendState);
}

void voodoo_gpu::UpdateAlphaTest()
{
	// Done in color control
}

void voodoo_gpu::UpdateColorCtrl()
{
	Color_Ctrl_Struct newCtrl;
	float r, g, b, a;

	a = float((m_regColor0 >> 24) & 0xff) / 255.0f;
	r = float((m_regColor0 >> 16) & 0xff) / 255.0f;
	g = float((m_regColor0 >> 8) & 0xff) / 255.0f;
	b = float((m_regColor0 >> 0) & 0xff) / 255.0f;
	m_colorCtrl.color0 = XMFLOAT4(r, g, b, a);

	a = float((m_regColor1 >> 24) & 0xff) / 255.0f;
	r = float((m_regColor1 >> 16) & 0xff) / 255.0f;
	g = float((m_regColor1 >> 8) & 0xff) / 255.0f;
	b = float((m_regColor1 >> 0) & 0xff) / 255.0f;
	m_colorCtrl.color1 = XMFLOAT4(r, g, b, a);

	m_colorCtrl.rgbselect				= (m_regFbzColorPath >> 0) & 3;
	m_colorCtrl.aselect					= (m_regFbzColorPath >> 2) & 3;
	m_colorCtrl.c_localselect			= (m_regFbzColorPath >> 4) & 1;
	m_colorCtrl.a_localselect			= (m_regFbzColorPath >> 5) & 3;
	m_colorCtrl.c_localselect_override	= (m_regFbzColorPath >> 7) & 1;

	m_colorCtrl.colCtrl.c_zero_cother			= (m_regFbzColorPath >> 8) & 1;
	m_colorCtrl.colCtrl.c_sub_clocal			= (m_regFbzColorPath >> 9) & 1;
	m_colorCtrl.colCtrl.c_mselect				= (m_regFbzColorPath >> 10) & 7;
	m_colorCtrl.colCtrl.c_reverse_blend			= (m_regFbzColorPath >> 13) & 1;
	m_colorCtrl.colCtrl.c_add_aclocal			= (m_regFbzColorPath >> 14) & 3;
	m_colorCtrl.colCtrl.c_invert_output			= (m_regFbzColorPath >> 16) & 1;
	m_colorCtrl.colCtrl.a_zero_aother			= (m_regFbzColorPath >> 17) & 1;
	m_colorCtrl.colCtrl.a_sub_alocal			= (m_regFbzColorPath >> 18) & 1;
	m_colorCtrl.colCtrl.a_mselect				= (m_regFbzColorPath >> 19) & 7;
	m_colorCtrl.colCtrl.a_reverse_blend			= (m_regFbzColorPath >> 22) & 1;
	m_colorCtrl.colCtrl.a_add_aclocal			= (m_regFbzColorPath >> 23) & 3;
	m_colorCtrl.colCtrl.a_invert_output			= (m_regFbzColorPath >> 25) & 1;

	m_colorCtrl.enable_texture			= (m_regFbzColorPath >> 27) & 1;
	m_colorCtrl.enable_param_clamp		= (m_regFbzColorPath >> 28) & 1;

	// Depth Control
	m_colorCtrl.wfloat_select = (m_regFbzMode >> 3) & 1;
	m_colorCtrl.depth_enable = (m_regFbzMode >> 4) & 1;
	m_colorCtrl.zbias_enable = (m_regFbzMode >> 16) & 1;
	m_colorCtrl.depth_src_select = (m_regFbzMode >> 20) & 1;
	m_colorCtrl.depthfloat_select = (m_regFbzMode >> 21) & 1;
	m_colorCtrl.zSrc = float(m_regZAColor & 0xffff) / 65535.0f;
	m_colorCtrl.zBias = float(INT32(m_regZAColor << 16) >> 16) / 65535.0f; // Sign extend and normalize

	// Alpha Testing
	m_colorCtrl.alpha_test_enable = (m_regAlphaMode >> 0) & 1;
	m_colorCtrl.alpha_op = (m_regAlphaMode >> 1) & 7;
	m_colorCtrl.alpha_ref = float(m_regAlphaMode >> 24) / 255.0f;

	// Dithering 0=no dither, 1=4x4, 2=2x2
	m_colorCtrl.dither_sel = (m_regFbzMode & 0x100) ? ((m_regFbzMode >> 11) & 1) + 1 : 0;
}

void voodoo_gpu::SetFogTable(UINT32 &data, int &index)
{	
	XMFLOAT4 entry;
	entry.w = 0.0f;

	entry.x = float((data >> 8) & 0xff); // 8 integer bits of fog alpha (unnormalized)
	entry.y = float((data >> 2) & 0x3f); // 6 integer bits of fog delta
	entry.z = float((data >> 1) & 1); // 2nd lsb of fog delta
	m_fogTable.fog_table[index] = entry;

	entry.x = float((data >> (8 + 16)) & 0xff); // 8 integer bits of fog alpha (unnormalized)
	entry.y = float((data >> (2 + 16)) & 0x3f); // 6 integer bits of fog delta
	entry.z = float((data >> (1 + 16)) & 1); // 2nd lsb of fog delta
	m_fogTable.fog_table[index + 1] = entry;
	m_updateFogTable = true;
}

void voodoo_gpu::UpdateFogCtrl()
{
	m_fogCtrl.fogColor = XMFLOAT3(((m_regFogColor >> 16) & 0xff) / 255.0f, ((m_regFogColor >> 8) & 0xff) / 255.0f, ((m_regFogColor >> 0) & 0xff) / 255.0f);
	m_fogCtrl.fog_enable = (m_regFogMode >> 0) & 1;
	m_fogCtrl.fogadd = (m_regFogMode >> 1) & 1;
	m_fogCtrl.fogmult = (m_regFogMode >> 2) & 1;
	m_fogCtrl.fogza = (m_regFogMode >> 3) & 3;
	m_fogCtrl.fogconstant = (m_regFogMode >> 5) & 1;
	m_fogCtrl.fogdither = (m_regFogMode >> 6) & 1;
	m_fogCtrl.fogzones = (m_regFogMode >> 7) & 1;
}

void voodoo_gpu::UpdateTexCtrl(int enableTex0, int enableTex1)
{
	if (m_texCtrl.texCtrl[0].enable != enableTex0 || m_texCtrl.texCtrl[1].enable != enableTex1) {
		m_texCtrl.texCtrl[0].enable = enableTex0;
		m_texCtrl.texCtrl[1].enable = enableTex1;
		m_updateTexCtrl = true;
	}
}

void voodoo_gpu::UpdateConstants()
{
	if (m_updateFrameBufferCtrl) {
		m_context->UpdateSubresource(m_frameBufferCtrlBuf, 0, NULL, &m_frameBufferCtrl, 0, 0);
		m_updateFrameBufferCtrl = false;
	}
	if (m_updateColorCtrl || m_updateAlphaTest) {
		UpdateColorCtrl();
		m_context->UpdateSubresource(m_colorCtrlBuf, 0, NULL, &m_colorCtrl, 0, 0);
		m_updateColorCtrl = false;
		m_updateAlphaTest = false;
	}
	if (m_updateTexCtrl) {
		m_context->UpdateSubresource(m_texCtrlBuf, 0, NULL, &m_texCtrl, 0, 0);
		m_updateTexCtrl = false;
	}
	if (m_updateBlendState) {
		UpdateAlphaBlend();
		m_updateBlendState = false;
	}
	if (m_updateDepth) {
		UpdateDepth();
		m_updateDepth = false;
	}
	if (m_updateFogCtrl) {
		UpdateFogCtrl();
		m_context->UpdateSubresource(m_fogCtrlBuf, 0, NULL, &m_fogCtrl, 0, 0);
		m_updateFogCtrl = false;
	}
	if (m_updateFogTable) {
		m_context->UpdateSubresource(m_fogTableBuf, 0, NULL, &m_fogTable, 0, 0);
		m_updateFogTable = false;
	}
}

void voodoo_gpu::FastFill(ShaderVertex *triangleVertices)
{
	// Check for pixels that haven't been drawn
	if (m_pixels_ready)
		DrawPixels();

	// Check to see if we are in the right mode
	if (m_renderMode != FASTFILL)
	{
		// Set input layout
		m_context->IASetInputLayout(m_pVertexLayout);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT stride = sizeof(ShaderVertex);
		UINT offset = 0;
		m_context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		m_renderMode = FASTFILL;
		// Set pixel mode in constant buffer
		m_colorCtrl.pixel_mode = 1;
		m_context->UpdateSubresource(m_colorCtrlBuf, 0, NULL, &m_colorCtrl, 0, 0);
		m_updateColorCtrl = true;

		// Create the depth stencil state
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		// Depth test parameters
		depthStencilDesc.DepthEnable = (m_regFbzMode >> 4) & 1;
		depthStencilDesc.DepthWriteMask = ((m_regFbzMode >> 10) & 1) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		// Release old state
		SAFE_RELEASE(m_depthState);
		// Create New
		m_gpu->CreateDepthStencilState(&depthStencilDesc, &m_depthState);
		m_updateDepth = true;
	}

	// See if parameters have changed
	//UpdateConstants();

	// Copy vertex buffer over
	m_context->UpdateSubresource(m_pVertexBuffer, 0, nullptr, triangleVertices, 0, 0);

	//////// Pass 0 /////////
	// Setup to draw to render buffer
	// Set the render resouce to be NULL to avoid output binding warnings
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, pSRV);

	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	// Blending is disabled on the second pass
	m_context->OMSetBlendState(NULL, 0, 0xffffffff);
	// Write to depth buffer first pass
	m_context->OMSetDepthStencilState(m_depthState, 0);
	m_context->VSSetShader(m_pVS, nullptr, 0);
	m_context->PSSetShader(m_fastFillPS, nullptr, 0);

	m_context->Draw(3, 0);

	//////// Pass 1 /////////
	// Setup to draw from render buffer to compressed (COMP) buffer, depth testing is disabled
	m_context->OMSetRenderTargets(1, &m_compTargetView, nullptr);
	// Blending is disabled on the second pass
	m_context->OMSetBlendState(NULL, 0, 0xffffffff);
	m_context->VSSetShader(m_compVS, nullptr, 0);
	m_context->PSSetShader(m_compPS, nullptr, 0);
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, &m_renderResourceView);
	m_context->PSSetSamplers(COMP_TEX_OFFSET, 1, &m_renderState);

	m_context->Draw(3, 0);

	m_need_copy = true;
}

void voodoo_gpu::DrawTriangle(ShaderVertex *triangleVertices)
{
	// Check for pixels that haven't been drawn
	if (m_pixels_ready)
		DrawPixels();

	/*
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//	Disable GPU access to the vertex buffer data.
	m_context->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, vertices, sizeof(vertices));
	//	Reenable GPU access to the vertex buffer data.
	m_context->Unmap(m_pVertexBuffer, 0);
	*/

	// Check to see if we are in the right mode
	if (m_renderMode != TRIANGLE)
	{
		// Set input layout
		m_context->IASetInputLayout(m_pVertexLayout);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT stride = sizeof(ShaderVertex);
		UINT offset = 0;
		m_context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		m_renderMode = TRIANGLE;
		// Clear pixel mode in constant buffer
		m_colorCtrl.pixel_mode = 0;
		m_updateColorCtrl = true;
		// Set input layout
		m_context->IASetInputLayout(m_pVertexLayout);
	}

	// See if parameters have changed
	UpdateConstants();

	// Copy vertex buffer over
	m_context->UpdateSubresource(m_pVertexBuffer, 0, nullptr, triangleVertices, 0, 0);

	//////// Pass 0 /////////
	// Setup to draw to render buffer
	// Set the render resouce to be NULL to avoid output binding warnings
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, pSRV);

	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	// Blending could be enabled on the first pass
	m_context->OMSetBlendState(m_blendState, 0, 0xffffffff);
	// Write to depth buffer first pass
	m_context->OMSetDepthStencilState(m_depthState, 0);
	m_context->VSSetShader(m_pVS, nullptr, 0);
	m_context->PSSetShader(m_pPS, nullptr, 0);

	m_context->Draw(3, 0);

	//////// Pass 1 /////////
	// Setup to draw from render buffer to compressed (COMP) buffer, depth testing is disabled
	m_context->OMSetRenderTargets(1, &m_compTargetView, nullptr);
	// Blending is disabled on the second pass
	m_context->OMSetBlendState(NULL, 0, 0xffffffff);
	m_context->VSSetShader(m_compVS, nullptr, 0);
	m_context->PSSetShader(m_compPS, nullptr, 0);
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, &m_renderResourceView);
	m_context->PSSetSamplers(COMP_TEX_OFFSET, 1, &m_renderState);

	m_context->Draw(3, 0);

	m_need_copy = true;
}

void voodoo_gpu::DrawPixels()
{
	if (!m_pixels_ready)
		return;

	// Check to see if we are in the right mode
	if (m_renderMode != PIXEL)
	{
		// Turn off tmus
		m_texCtrl.texCtrl[0].enable = 0;
		m_texCtrl.texCtrl[1].enable = 0;
		m_updateTexCtrl = true;

		// Set input layout
		m_context->IASetInputLayout(m_pixelVertexLayout);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT stride = sizeof(ShaderPoint);
		UINT offset = 0;
		m_context->IASetVertexBuffers(0, 1, &m_pixelBuffer, &stride, &offset);
		m_renderMode = PIXEL;
		// Set pixel mode in gpu constant buffer
		m_colorCtrl.pixel_mode = 1;
		m_updateColorCtrl = true;

	}

	UpdateConstants();

	// Copy pixel point buffer over
	D3D11_BOX pointBox;
	pointBox.left = 0;
	pointBox.right = m_pixelPoints.size() * sizeof(ShaderPoint);
	pointBox.top = 0;
	pointBox.bottom = 1;
	pointBox.front = 0;
	pointBox.back = 1;
	m_context->UpdateSubresource(m_pixelBuffer, 0, &pointBox, m_pixelPoints.data(), 0, 0);

	//////// Pass 0 /////////
	// Setup to draw to render buffer
	// Set the render resouce to be NULL to avoid output binding warnings
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, pSRV);

	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	// Blending could be enabled on the first pass
	m_context->OMSetBlendState(m_blendState, 0, 0xffffffff);
	// Write to depth buffer first pass
	m_context->OMSetDepthStencilState(m_depthState, 0);
	m_context->VSSetShader(m_pixelVS, nullptr, 0);
	m_context->PSSetShader(m_pPS, nullptr, 0);

	m_context->Draw(m_pixelPoints.size(), 0);

	//////// Pass 1 /////////
	// Setup to draw from render buffer to compressed (COMP) buffer, depth testing is disabled
	m_context->OMSetRenderTargets(1, &m_compTargetView, nullptr);
	// Blending is disabled on the second pass
	m_context->OMSetBlendState(NULL, 0, 0xffffffff);
	m_context->VSSetShader(m_compVS, nullptr, 0);
	m_context->PSSetShader(m_compPS, nullptr, 0);
	m_context->PSSetShaderResources(COMP_TEX_OFFSET, 1, &m_renderResourceView);
	m_context->PSSetSamplers(COMP_TEX_OFFSET, 1, &m_renderState);

	m_context->Draw(m_pixelPoints.size(), 0);

	// Clear cached pixels
	m_pixelPoints.clear();
	m_pixels_ready = false;

	m_need_copy = true;

}

void voodoo_gpu::ClearBuffer(int sx, int ex, int sy, int ey)
{
	FLOAT depth = float(m_regZAColor & 0xffff) / 65535.0f;
	
	UpdateColorCtrl();

	if (sx == 0 && sy == 0 && ex >= m_fbiWidth && ey == m_frameBufferCtrl.ySize) {
		// Check for pixels that haven't been drawn
		if (m_pixels_ready)
			DrawPixels();
		m_context->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const FLOAT*>(&m_colorCtrl.color1));
		m_context->ClearDepthStencilView(m_depthView, D3D11_CLEAR_DEPTH, depth, 0);
		// TODO: Should be dithering
		m_context->ClearRenderTargetView(m_compTargetView, reinterpret_cast<const FLOAT*>(&m_colorCtrl.color1));
		m_need_copy = true;
	} else {
		std::vector<ShaderVertex> inVec;
		inVec.push_back({ XMFLOAT4(float(sx), float(sy), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
		inVec.push_back({ XMFLOAT4(float(ex), float(sy), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
		inVec.push_back({ XMFLOAT4(float(sx), float(ey), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
		
		FastFill(inVec.data());

		inVec.clear();
		inVec.push_back({ XMFLOAT4(float(ex), float(sy), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
		inVec.push_back({ XMFLOAT4(float(ex), float(ey), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
		inVec.push_back({ XMFLOAT4(float(sx), float(ey), 0.0f, depth), m_colorCtrl.color1, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });

		FastFill(inVec.data());
	}
}

HRESULT voodoo_gpu::RenderToTex()
{
	
	Combine_Struct testComb;
	UINT32 texMode;
	texMode = 0x0C261ACD;
	testComb = ConvertTexmode(texMode);  // Mode 1
	texMode = 0x0C224A0D;
	testComb = ConvertTexmode(texMode);  // Mode 0

	// Clear the back buffer 
	XMFLOAT4 Zero = { 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f };
	ClearBuffer(0, 511, 0, 384);

	// Setup constants
	//m_frameBufferCtrl.xSize = 512.0f;
	//m_frameBufferCtrl.ySize = 384.0f;
	//m_updateFrameBufferCtrl = true;
	InitRenderBuffers(512, 384, 512);
	m_context->UpdateSubresource(m_frameBufferCtrlBuf, 0, NULL, &m_frameBufferCtrl, 0, 0);

	SetZAColor(0xffff);
	SetFbzMode(0x300);
	// Sets colorpath so that color1 is output
	SetColorCtrl(0x2, 0x0, 0x010101);

	// Test pixels
	if (0) {
		int x = 0x0;
		int y = 0x0;
		int mask = 0xff;
		int sr[2] = { 0xff, 0x0f };
		int sg[2] = { 0xff, 0x0f };
		int sb[2] = { 0xff, 0x0f };
		int sa[2] = { 0xff, 0x0f };
		int sw[2] = { 0xff, 0x0f };
		int sz[2] = { 0xf, 0x8f };
		
		UINT32 wSel = 0;
		UINT32 wVal = 0x0;
		PushPixel(x, y, mask, sr, sg, sb, sa, sz, wSel, wVal);
		x = 0x2;
		PushPixel(x, y, mask, sr, sg, sb, sa, sz, wSel, wVal);
		x = 0x0;
		y = 0x1;
		PushPixel(x, y, mask, sr, sg, sb, sa, sz, wSel, wVal);
		DrawPixels();
		return S_OK;
	}

	// Create vertex buffer
	std::vector<ShaderVertex> inVec;
	inVec.push_back({ XMFLOAT4(0.0f, 0.0f, 0.0f, 0.75f), XMFLOAT4(255.0f, 2.0f, 3.0f, 255.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	inVec.push_back({ XMFLOAT4(36.0f, 0.0f, 0.0f, 0.75f), XMFLOAT4(255.0f, 2.0f, 3.0f, 255.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	inVec.push_back({ XMFLOAT4(0.0f, 36.0f, 0.0f, 0.75f), XMFLOAT4(255.0f, 2.0f, 3.0f, 255.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	//inVec.push_back({ XMFLOAT4(10.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(128.0f, 254.0f, 253.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });
	//inVec.push_back({ XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f), XMFLOAT4(128.0f, 254.0f, 253.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) });

	DrawTriangle(inVec.data());
	//DrawTriangle(inVec.data());

	return S_OK;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//	Disable GPU access to the vertex buffer data.
	m_context->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, inVec.data(), sizeof(inVec));
	//	Reenable GPU access to the vertex buffer data.
	m_context->Unmap(m_pVertexBuffer, 0);

	m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	//m_context->ClearRenderTargetView(m_renderTargetView, &Zero[0]);
	// Render a triangle

	m_context->Draw(3, 0);

	return S_OK;
}
void voodoo_gpu::CopyBuffer(uint16_t *dst, int rowPixels)
{
	if (!m_need_copy) return;
	m_need_copy = false;

	ID3D11Texture2D* pNewTexture = NULL;
	D3D11_TEXTURE2D_DESC desc;
	m_compTexture->GetDesc(&desc);

	desc.BindFlags = 0;

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;

	HRESULT hr = m_gpu->CreateTexture2D(&desc, NULL, &pNewTexture);

	m_context->CopyResource(pNewTexture, m_compTexture);

	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	m_context->Map(pNewTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	// COPY from texture to bitmap buffer
	uint32_t *tmpBuf = new uint32_t[desc.Width];

	uint32_t* dptr = tmpBuf;
	uint32_t* sptr = reinterpret_cast<uint32_t*>(resource.pData);

	for (size_t h = 0; h < desc.Height; ++h)
	{
		//size_t msize = std::min<size_t>(desc.Width * 4, resource.RowPitch);
		//size_t msize = resource.RowPitch;
		size_t msize = desc.Width * 4;
		//memcpy_s(dptr, desc.Width * 4, sptr, msize);
		memcpy(dptr, sptr, desc.Width*4);
		sptr += desc.Width;
		//dptr += desc.Width * 4;
		//for (size_t elem = 0; elem < desc.Width; elem++)
		for (int elem = 0; elem < m_fbiWidth; elem++)
		{
			//uint16_t rgb16 = uint16_t((dptr[elem] & 0xf8) << 8) | uint16_t((dptr[elem+1] & 0xfc) << 3) | uint16_t((dptr[elem+2] & 0xf8) >> 2);
			// RGB565 conversion masking already done in Pixel Shader
			//uint16_t rgb16 = (uint16_t(dptr[elem]) << 8) | (uint16_t(dptr[elem + 1]) << 3) | (uint16_t(dptr[elem + 2]) >> 3);
			//uint16_t rgb16 = ((dptr[elem] >> (24 - 11 + 3)) & 0xf800) | ((dptr[elem] >> (16 - 5 + 2)) & 0x07e0) | ((dptr[elem] >> (8 - 0 + 3)) & 0x001f);
			// Copied using 32 bit access so it is abgr ??
			uint16_t rgb16 = ((dptr[elem] << (0 + 11 - 3)) & 0xf800) | ((dptr[elem] >> (8 - 5 + 2)) & 0x07e0) | ((dptr[elem] >> (16 - 0 + 3)) & 0x001f);
			//dst[h * m_hVis + elem] = rgb16;
			dst[elem] = rgb16;
		}
		//dst += desc.Width;
		dst += rowPixels;
	}

	m_context->Unmap(pNewTexture, 0);
	delete[] tmpBuf;
	SAFE_RELEASE(pNewTexture);
}

void voodoo_gpu::CopyBufferRGB(uint8_t *dst)
{
	if (!m_need_copy) return;
	m_need_copy = false;

	ID3D11Texture2D* pNewTexture = NULL;
	D3D11_TEXTURE2D_DESC desc;
	m_renderTexture->GetDesc(&desc);

	desc.BindFlags = 0;

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;

	HRESULT hr = m_gpu->CreateTexture2D(&desc, NULL, &pNewTexture);

	m_context->CopyResource(pNewTexture, m_renderTexture);

	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	m_context->Map(pNewTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	// COPY from texture to bitmap buffer
	uint32_t* dptr = reinterpret_cast<uint32_t*>(dst);
	uint32_t* sptr = reinterpret_cast<uint32_t*>(resource.pData);

	for (size_t h = 0; h < desc.Height; ++h)
	{
		//size_t msize = std::min<size_t>(desc.Width * 4, resource.RowPitch);
		//size_t msize = resource.RowPitch;
		size_t msize = desc.Width;
		//memcpy_s(dptr, desc.Width, sptr, msize);
		//sptr += resource.RowPitch;
		memcpy(dptr, sptr, desc.Width * 4);
		sptr += desc.Width;
		dptr += desc.Width;
	}

	m_context->Unmap(pNewTexture, 0);
	SAFE_RELEASE(pNewTexture);
}

void voodoo_gpu::CopyBufferComp(uint16_t *dst)
{
	if (!m_need_copy) return;
	m_need_copy = false;

	ID3D11Texture2D* pNewTexture = NULL;
	D3D11_TEXTURE2D_DESC desc;
	m_compTexture->GetDesc(&desc);

	desc.BindFlags = 0;

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;

	HRESULT hr = m_gpu->CreateTexture2D(&desc, NULL, &pNewTexture);

	m_context->CopyResource(pNewTexture, m_compTexture);

	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	m_context->Map(pNewTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

	//uint32_t* dptr = dst;
	uint16_t* sptr = reinterpret_cast<uint16_t*>(resource.pData);

	memcpy(dst, sptr, desc.Width * desc.Height);

	m_context->Unmap(pNewTexture, 0);

	SAFE_RELEASE(pNewTexture);
}

void voodoo_gpu::FlagTexture(int index, UINT32 *texBase, UINT32 &texLod)
{
	if (!m_texHist.empty()) {
		UINT32 minLod = ((texLod >> 0) & 0x3f) >> 2;
		UINT32 mapIndex = texBase[minLod] + index;
		if (m_texMap.find(mapIndex) != m_texMap.end())
		{
			//m_texMap.erase(mapIndex);
			//m_texHist
			// Nuke everything
			for (std::map<UINT32, Tex_Map_List_Struct>::iterator ii = m_texMap.begin(); ii != m_texMap.end(); ++ii)
			{
				SAFE_RELEASE((*ii).second.texTexture);
				SAFE_RELEASE((*ii).second.texRV);
				SAFE_RELEASE((*ii).second.texSampler);
			}
			m_texMap.clear();
			while (!m_texHist.empty())
				m_texHist.pop();
		}
	}
}

voodoo_gpu::Combine_Struct voodoo_gpu::ConvertTexmode(UINT32 &texMode)
{
	Combine_Struct newCombine;
	newCombine.c_zero_cother = (texMode >> 12) & 1;
	newCombine.c_sub_clocal = (texMode >> 13) & 1;
	newCombine.c_mselect = (texMode >> 14) & 0x7;
	newCombine.c_reverse_blend = (texMode >> 17) & 1;
	newCombine.c_add_aclocal = (texMode >> 18) & 0x3;
	newCombine.c_invert_output = (texMode >> 20) & 1;
	newCombine.a_zero_aother = (texMode >> 21) & 1;
	newCombine.a_sub_alocal = (texMode >> 22) & 1;
	newCombine.a_mselect = (texMode >> 23) & 0x7;
	newCombine.a_reverse_blend = (texMode >> 26) & 1;
	newCombine.a_add_aclocal = (texMode >> 27) & 0x3;
	newCombine.a_invert_output = (texMode >> 29) & 1;
	return newCombine;
}

void voodoo_gpu::CreateTexture(texDescription &desc, int index, UINT32 &texMode, UINT32 &texLod, UINT32 &texDetail)
{
	Tex_Map_List_Struct new_map;
	UINT32 minLod = ((texLod >> 0) & 0x3f) >> 2;
	UINT32 mapIndex = desc.texBase[minLod] + index;
	if (m_texMap.find(mapIndex) == m_texMap.end())
	{
		if (m_texMap.size() == MAX_TEX)
		{
			// Erase the oldest entry
			Tex_Map_List_Struct old_map = m_texMap[m_texHist.front()];
			SAFE_RELEASE(old_map.texTexture);
			SAFE_RELEASE(old_map.texRV);
			SAFE_RELEASE(old_map.texSampler);
			m_texMap.erase(m_texHist.front());
			m_texHist.pop();
		}
		// Create the new entries
		// Create gpu texture
		HRESULT hr;
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = desc.sSize;
		texDesc.Height = desc.tSize;
		texDesc.MipLevels = 9;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		hr = m_gpu->CreateTexture2D(&texDesc, NULL, &new_map.texTexture);

		// Create the Resource View
		D3D11_SHADER_RESOURCE_VIEW_DESC texResourceViewDesc;
		texResourceViewDesc.Format = texDesc.Format;
		texResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		texResourceViewDesc.Texture2D.MostDetailedMip = 0;
		texResourceViewDesc.Texture2D.MipLevels = 9;
		hr = m_gpu->CreateShaderResourceView(new_map.texTexture, &texResourceViewDesc, &new_map.texRV);

		// Create the Sampler State
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		switch ((texMode >> 1) & 0x3)  // tmag : tmin
		{
		case 0:
			sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case 1:
			sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case 2:
			sampDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		default:
			sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		}
		// Not sure if mirror_once is a valid 3dfx mode
		sampDesc.AddressU = (texMode & 0x40) ?
			((texLod & (1 << 28)) ? D3D11_TEXTURE_ADDRESS_MIRROR_ONCE : D3D11_TEXTURE_ADDRESS_CLAMP) :
			((texLod & (1 << 28)) ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP);
		sampDesc.AddressV = (texMode & 0x80) ?
			((texLod & (1 << 29)) ? D3D11_TEXTURE_ADDRESS_MIRROR_ONCE : D3D11_TEXTURE_ADDRESS_CLAMP) :
			((texLod & (1 << 29)) ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP);
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = float((texLod >> 0) & 0x3f) / 4.0f; // Should actually be signed (4.2s)
		sampDesc.MaxLOD = float((texLod >> 6) & 0x3f) / 4.0f; // Should actually be signed (4.2s)
		sampDesc.MipLODBias = float(int32_t(((texLod >> 12) & 0x3f) << 26)) / 268435456.0f; // val / 2^28 (4.2s)
		hr = m_gpu->CreateSamplerState(&sampDesc, &new_map.texSampler);

		// Copy texture data to buffer
		UINT32 *srcBuff;
		UINT32 texel0;
		UINT32 result;
		for (size_t mipLevel = minLod; mipLevel <= 8; mipLevel++)
		{
			UINT32 texbase = desc.texBase[mipLevel];
			// Create temporary buffer space for texture
			size_t width = (desc.sSize >> mipLevel);
			if (width == 0) width = 1;
			size_t height = (desc.tSize >> mipLevel);
			if (height == 0) height = 1;
			srcBuff = new UINT32[width * height];
			// Copy data if lod is defined
			if (desc.lodMask & (1 << mipLevel)) {
				for (size_t loc = 0; loc < width * height; loc++)
				{
					/* fetch texel data */
					if (desc.texFormat < 8)
					{
						texel0 = *(UINT8 *)&desc.ram[(texbase + loc) & desc.texMask];
						result = (desc.texLookup)[texel0];
					}
					else
					{
						texel0 = *(UINT16 *)&desc.ram[(texbase + 2 * loc) & desc.texMask];
						if (desc.texFormat >= 10 && desc.texFormat <= 12)
							result = (desc.texLookup)[texel0];
						else
							result = ((desc.texLookup)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16);
					}
					// Convert argb to rgba
					//srcBuff[loc] = (result>>24) | (result<<8);
					// Do swizzle in pixel shader
					srcBuff[loc] = result;
				}
			}
			m_context->UpdateSubresource(new_map.texTexture, mipLevel, NULL, srcBuff, width * 4, 0);
			delete[] srcBuff;
		}

		// Add the new data to the map
		m_texMap[mapIndex] = new_map;
		m_texHist.push(mapIndex);

	}

	// Update cb tex control
	m_texCtrl.texCtrl[index].enable = 1;
	m_texCtrl.texCtrl[index].tloddither = (texMode >> 4) & 1;
	m_texCtrl.texCtrl[index].texSize = XMFLOAT2(float(desc.sSize), float(desc.tSize));
	m_texCtrl.texCombine[index] = ConvertTexmode(texMode);

	m_texCtrl.texCtrl[index].detailMax = float((texDetail >> 0) & 0xff);
	m_texCtrl.texCtrl[index].detailBias = float((int32_t(texDetail << (31 - 13)) >> (31 - 13)) >> 8);
	m_texCtrl.texCtrl[index].detailScale = float(1 << ((texDetail >> 14) & 0x7));

	m_texCtrl.texCtrl[index].send_config = desc.sendConfig ? desc.tmuConfig : 0;

	m_updateTexCtrl = true;

	// Check perspective
	if (index == 0) {
		if ((texMode & 1) ^ m_frameBufferCtrl.enablePerspective0)
		{
			m_frameBufferCtrl.enablePerspective0 = texMode & 1;
			m_updateFrameBufferCtrl = true;
		}
	} else {
		if ((texMode & 1) ^ m_frameBufferCtrl.enablePerspective1)
		{
			m_frameBufferCtrl.enablePerspective1 = texMode & 1;
			m_updateFrameBufferCtrl = true;
		}
	}
	// Set the GPU Chain texture and sampler
	m_context->PSSetShaderResources(index + TMU_TEX_OFFSET, 1, &m_texMap[mapIndex].texRV);
	m_context->PSSetSamplers(index + TMU_TEX_OFFSET, 1, &m_texMap[mapIndex].texSampler);
}

void voodoo_gpu::PushPixel(int &x, int &y, int &mask, int *sr, int *sg, int *sb, int *sa, int *sz, UINT32 wSel, UINT32 &wVal)
{
	ShaderPoint pixel;
	for (size_t pix = 0; pix < 2; ++pix)
	{
		if (mask & (0xf << (pix*4))) {
			pixel.intX = x + pix;
			pixel.intY = y;
			pixel.intZ = sz[pix];
			if (wSel) {
				pixel.intW = wVal;
			} else {
				pixel.intW = pixel.intZ;
			}
			pixel.intR = sr[pix];
			pixel.intG = sg[pix];
			pixel.intB = sb[pix];
			pixel.intA = sa[pix];
			m_pixelPoints.push_back(pixel);
			m_pixels_ready = true;
		}
	}
	// Need to start drawing one early in case of single pixel write followed by a 2 pixel write
	if (m_pixelPoints.size() >= DEPTH_PIXEL_BUFFER-1) {
		DrawPixels();
	}
}

void voodoo_gpu::ReadTex()
{

	// COPY from texture to bitmap buffer

	uint8_t dither_maxtrix_4x4[4][4] = { { 0, 8, 2, 10 },{ 12, 4, 14, 6 },{ 3, 11, 1, 9 },{ 15, 7, 13, 5 } };
#define DITHER_RB(val,dith) ((((val) << 1) - ((val) >> 4) + ((val) >> 7) + (dith)) >> 1);
#define DITHER_G(val,dith)  ((((val) << 2) - ((val) >> 4) + ((val) >> 6) + (dith)) >> 2);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			for (int val = 10; val <= 10; val++) {
				int dith = dither_maxtrix_4x4[y][x];
				int rb = DITHER_RB(val, dith);
				rb >>= 3;
				printf("x: %i y: %i val: %i dith: %i rb: %i\n", x, y, val, dith, rb);
			}
		}
	}

	struct My_RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};


#if (0)
		My_RGBA dest_array[384][512];
		CopyBufferRGB(reinterpret_cast<uint8_t*>(&dest_array[0][0]));

		for (int row = 0; row < 11; row++) {
			for (int col = 0; col < 12; col++)
			{
				if (col == 0) printf("\n");
				if (col <= 10) {
					printf("xy(%i,%i) %02X %02X %02X ", col, row, dest_array[row][col].r, dest_array[row][col].g, dest_array[row][col].b);
					//printf("xy(%i,%i) %04X ", col, row, dest_array[row][col]);
				}
			}
		}
		printf("\n");
#else
		// RGB565 buffer reading

		uint16_t comp_arrray[384][512];
		m_need_copy = true;
		CopyBuffer(&comp_arrray[0][0], 512);
		for (int row = 0; row < 11; row++) {
			for (int col = 0; col < 9; col++)
			{
				if (col == 0) printf("\n");
				if (col <= 10) {
					printf("xy(%i,%i) %04X ", col, row, comp_arrray[row][col]);
				}
			}
		}
		printf("\n");
#endif
}
