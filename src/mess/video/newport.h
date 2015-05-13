// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s

*/

#ifndef __VIDHRDW_NEWPORT__
#define __VIDHRDW_NEWPORT__

struct VC2_t
{
	UINT16 nRegister[0x21];
	UINT16 nRAM[0x8000];
	UINT8 nRegIdx;
	UINT16 nRegData;
};


struct XMAP_t
{
	UINT32 nRegister[0x08];
	UINT32 nModeTable[0x20];
};

struct REX3_t
{
	UINT32 nDrawMode1;
	UINT32 nDrawMode0;
	UINT32 nLSMode;
	UINT32 nLSPattern;
	UINT32 nLSPatSave;
	UINT32 nZPattern;
	UINT32 nColorBack;
	UINT32 nColorVRAM;
	UINT32 nAlphaRef;
	//UINT32 nStall0;
	UINT32 nSMask0X;
	UINT32 nSMask0Y;
	UINT32 nSetup;
	UINT32 nStepZ;
	UINT32 nXStart;
	UINT32 nYStart;
	UINT32 nXEnd;
	UINT32 nYEnd;
	UINT32 nXSave;
	UINT32 nXYMove;
	UINT32 nBresD;
	UINT32 nBresS1;
	UINT32 nBresOctInc1;
	UINT32 nBresRndInc2;
	UINT32 nBresE1;
	UINT32 nBresS2;
	UINT32 nAWeight0;
	UINT32 nAWeight1;
	UINT32 nXStartF;
	UINT32 nYStartF;
	UINT32 nXEndF;
	UINT32 nYEndF;
	UINT32 nXStartI;
	//UINT32 nYEndF1;
	UINT32 nXYStartI;
	UINT32 nXYEndI;
	UINT32 nXStartEndI;
	UINT32 nColorRed;
	UINT32 nColorAlpha;
	UINT32 nColorGreen;
	UINT32 nColorBlue;
	UINT32 nSlopeRed;
	UINT32 nSlopeAlpha;
	UINT32 nSlopeGreen;
	UINT32 nSlopeBlue;
	UINT32 nWriteMask;
	UINT32 nZeroFract;
	UINT32 nZeroOverflow;
	//UINT32 nColorIndex;
	UINT32 nHostDataPortMSW;
	UINT32 nHostDataPortLSW;
	UINT32 nDCBMode;
	UINT32 nDCBRegSelect;
	UINT32 nDCBSlvSelect;
	UINT32 nDCBDataMSW;
	UINT32 nDCBDataLSW;
	UINT32 nSMask1X;
	UINT32 nSMask1Y;
	UINT32 nSMask2X;
	UINT32 nSMask2Y;
	UINT32 nSMask3X;
	UINT32 nSMask3Y;
	UINT32 nSMask4X;
	UINT32 nSMask4Y;
	UINT32 nTopScanline;
	UINT32 nXYWin;
	UINT32 nClipMode;
	UINT32 nConfig;
	UINT32 nStatus;
	UINT8 nXFerWidth;
#if 0
	UINT32 nCurrentX;
	UINT32 nCurrentY;
#endif
	UINT32 nKludge_SkipLine;
};


struct CMAP_t
{
	UINT16 nPaletteIndex;
	UINT32 nPalette[0x10000];
};


class newport_video_device : public device_t
{
public:
	newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~newport_video_device() {}


	DECLARE_READ32_MEMBER( rex3_r );
	DECLARE_WRITE32_MEMBER( rex3_w );

	UINT32 screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
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
	UINT32 *m_base;
	UINT8  m_nDrawGreen;
	CMAP_t m_CMAP0;
};



#define MCFG_NEWPORT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NEWPORT_VIDEO, 0)


extern const device_type NEWPORT_VIDEO;


#endif
