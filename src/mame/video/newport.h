// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s
*/

#ifndef MAME_VIDEO_NEWPORT_H
#define MAME_VIDEO_NEWPORT_H

#pragma once

class newport_video_device : public device_t
{
public:
	newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: newport_video_device(mconfig, tag, owner, (uint32_t)0) // TODO: Use actual pixel clock
	{
	}

	newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER( rex3_r );
	DECLARE_WRITE32_MEMBER( rex3_w );

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		DCR_CURSOR_FUNC_ENABLE_BIT = 4,
		DCR_CURSOR_ENABLE_BIT = 7,

		DCR_CURSOR_MODE_BIT = 8,
		DCR_CURSOR_MODE_GLYPH = 0,
		DCR_CURSOR_MODE_CROSSHAIR = 1,

		DCR_CURSOR_SIZE_BIT = 9,
		DCR_CURSOR_SIZE_32 = 0,
		DCR_CURSOR_SIZE_64 = 1
	};

	struct VC2_t
	{
		uint16_t nRegister[0x21];
		uint16_t nRAM[0x8000];
		uint8_t nRegIdx;
		uint16_t nRegData;
	};


	struct XMAP_t
	{
		uint32_t nRegister[0x08];
		uint32_t nModeTable[0x20];
	};

	struct REX3_t
	{
		uint32_t nDrawMode1;
		uint32_t nDrawMode0;
		uint32_t nLSMode;
		uint32_t nLSPattern;
		uint32_t nLSPatSave;
		uint32_t nZPattern;
		uint32_t nColorBack;
		uint32_t nColorVRAM;
		uint32_t nAlphaRef;
		//uint32_t nStall0;
		uint32_t nSMask0X;
		uint32_t nSMask0Y;
		uint32_t nSetup;
		uint32_t nStepZ;
		uint32_t nXStart;
		uint32_t nYStart;
		uint32_t nXEnd;
		uint32_t nYEnd;
		uint32_t nXSave;
		uint32_t nXYMove;
		uint32_t nBresD;
		uint32_t nBresS1;
		uint32_t nBresOctInc1;
		uint32_t nBresRndInc2;
		uint32_t nBresE1;
		uint32_t nBresS2;
		uint32_t nAWeight0;
		uint32_t nAWeight1;
		uint32_t nXStartF;
		uint32_t nYStartF;
		uint32_t nXEndF;
		uint32_t nYEndF;
		uint32_t nXStartI;
		//uint32_t nYEndF1;
		uint32_t nXYStartI;
		uint32_t nXYEndI;
		uint32_t nXStartEndI;
		uint32_t nColorRed;
		uint32_t nColorAlpha;
		uint32_t nColorGreen;
		uint32_t nColorBlue;
		uint32_t nSlopeRed;
		uint32_t nSlopeAlpha;
		uint32_t nSlopeGreen;
		uint32_t nSlopeBlue;
		uint32_t nWriteMask;
		uint32_t nZeroFract;
		uint32_t nZeroOverflow;
		//uint32_t nColorIndex;
		uint32_t nHostDataPortMSW;
		uint32_t nHostDataPortLSW;
		uint32_t nDCBMode;
		uint32_t nDCBRegSelect;
		uint32_t nDCBSlvSelect;
		uint32_t nDCBDataMSW;
		uint32_t nDCBDataLSW;
		uint32_t nSMask1X;
		uint32_t nSMask1Y;
		uint32_t nSMask2X;
		uint32_t nSMask2Y;
		uint32_t nSMask3X;
		uint32_t nSMask3Y;
		uint32_t nSMask4X;
		uint32_t nSMask4Y;
		uint32_t nTopScanline;
		uint32_t nXYWin;
		uint32_t nClipMode;
		uint32_t nConfig;
		uint32_t nStatus;
		uint8_t nXFerWidth;
#if 0
		uint32_t nCurrentX;
		uint32_t nCurrentY;
#endif
		uint32_t nKludge_SkipLine;
	};


	struct CMAP_t
	{
		uint16_t nPaletteIndex;
		uint32_t nPalette[0x10000];
	};

	uint32_t get_cursor_pixel(int x, int y);

	// internal state

	DECLARE_READ32_MEMBER( cmap0_r );
	DECLARE_WRITE32_MEMBER( cmap0_w );
	DECLARE_READ32_MEMBER( cmap1_r );
	DECLARE_READ32_MEMBER( xmap0_r );
	DECLARE_WRITE32_MEMBER( xmap0_w );
	DECLARE_READ32_MEMBER( xmap1_r );
	DECLARE_WRITE32_MEMBER( xmap1_w );
	DECLARE_READ32_MEMBER( vc2_r );
	DECLARE_WRITE32_MEMBER( vc2_w );
	void DoREX3Command();

	VC2_t  m_VC2;
	XMAP_t m_XMAP0;
	XMAP_t m_XMAP1;
	REX3_t m_REX3;
	std::unique_ptr<uint32_t[]> m_base;
	uint8_t  m_nDrawGreen;
	CMAP_t m_CMAP0;
};

DECLARE_DEVICE_TYPE(NEWPORT_VIDEO, newport_video_device)


#endif // MAME_VIDEO_NEWPORT_H
