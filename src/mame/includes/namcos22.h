#include "video/poly.h"

#define USE_NAMCOS22_SPEED_HACK

#define MAX_LIT_SURFACES 32
#define MAX_RENDER_CMD_SEQ 0x1c


#define GFX_CHAR               0
#define GFX_TEXTURE_TILE       1
#define GFX_SPRITE             2

enum
{
	NAMCOS22_AIR_COMBAT22,
	NAMCOS22_ALPINE_RACER,
	NAMCOS22_CYBER_COMMANDO,
	NAMCOS22_CYBER_CYCLES,
	NAMCOS22_PROP_CYCLE,
	NAMCOS22_RAVE_RACER,
	NAMCOS22_RIDGE_RACER,
	NAMCOS22_RIDGE_RACER2,
	NAMCOS22_TIME_CRISIS,
	NAMCOS22_VICTORY_LAP,
	NAMCOS22_ACE_DRIVER,
	NAMCOS22_ALPINE_RACER_2,
	NAMCOS22_ALPINE_SURFER,
	NAMCOS22_TOKYO_WARS,
	NAMCOS22_AQUA_JET,
	NAMCOS22_DIRT_DASH
};

#define NAMCOS22_NUM_ROWS 30
#define NAMCOS22_NUM_COLS 40

#define NAMCOS22_PALETTE_SIZE 0x8000


class namcos22_state : public driver_device
{
public:
	namcos22_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *m_spriteram;
	int m_mbEnableDspIrqs;
	UINT32 *m_shareram;
	UINT32 *m_system_controller;
	UINT32 *m_nvmem;
	size_t m_nvmem_size;
	UINT16 m_mMasterBIOZ;
	UINT32 *m_mpPointRAM;
	UINT32 m_old_coin_state;
	UINT32 m_credits1;
	UINT32 m_credits2;
	UINT16 *m_mpSlaveExternalRAM;
	UINT32 m_mPointAddr;
	UINT32 m_mPointData;
	UINT16 *m_mpMasterExternalRAM;
	UINT16 m_mSerialDataSlaveToMasterNext;
	UINT16 m_mSerialDataSlaveToMasterCurrent;
	int m_mRenderBufSize;
	UINT16 m_mRenderBufData[MAX_RENDER_CMD_SEQ];
	UINT32 m_mSys22PortBits;
	int m_mFrameCount;
	UINT8 m_stick_input;
	UINT8 m_prev_stick_state;
	int m_mDspUploadState;
	int m_mUploadDestIdx;
	UINT32 m_mAlpineSurferProtData;
	int m_p4;
	UINT16 m_su_82;
	int m_gametype;
	int m_mbSuperSystem22;
	UINT32 *m_cgram;
	UINT32 *m_textram;
	UINT32 *m_polygonram;
	UINT32 *m_gamma;
	UINT32 *m_vics_data;
	UINT32 *m_vics_control;
	UINT32 *m_czattr;
	UINT32 *m_tilemapattr;
	int m_mbSpotlightEnable;
	UINT16 *m_czram[4];
	poly_manager *m_poly;
	UINT16 *m_mpTextureTileMap16;
	UINT8 *m_mpTextureTileMapAttr;
	UINT8 *m_mpTextureTileData;
	UINT8 m_mXYAttrToPixel[16][16][16];
	UINT16 m_dspram_bank;
	UINT16 m_mUpperWordLatch;
	int m_mbDSPisActive;
	INT32 m_mAbsolutePriority;
	INT32 m_mObjectShiftValue22;
	UINT16 m_mPrimitiveID;
	float m_mViewMatrix[4][4];
	UINT8 m_mLitSurfaceInfo[MAX_LIT_SURFACES];
	INT32 m_mSurfaceNormalFormat;
	unsigned m_mLitSurfaceCount;
	unsigned m_mLitSurfaceIndex;
	int m_mPtRomSize;
	const UINT8 *m_mpPolyH;
	const UINT8 *m_mpPolyM;
	const UINT8 *m_mpPolyL;
	UINT8 *m_dirtypal;
	tilemap_t *m_bgtilemap;
};


/*----------- defined in video/namcos22.c -----------*/

WRITE16_HANDLER( namcos22_dspram16_bank_w );
READ16_HANDLER( namcos22_dspram16_r );
WRITE16_HANDLER( namcos22_dspram16_w );

READ32_HANDLER( namcos22_cgram_r );
WRITE32_HANDLER( namcos22_cgram_w );

READ32_HANDLER( namcos22_czram_r );
WRITE32_HANDLER( namcos22_czram_w );

WRITE32_HANDLER(namcos22_port800000_w);

READ32_HANDLER( namcos22_paletteram_r );
WRITE32_HANDLER( namcos22_paletteram_w );

READ32_HANDLER( namcos22_textram_r );
WRITE32_HANDLER( namcos22_textram_w );

READ32_HANDLER( namcos22_gamma_r );
WRITE32_HANDLER( namcos22_gamma_w );

READ32_HANDLER( namcos22_dspram_r );
WRITE32_HANDLER( namcos22_dspram_w );


VIDEO_START( namcos22 );
SCREEN_UPDATE( namcos22 );

VIDEO_START( namcos22s );
SCREEN_UPDATE( namcos22s );

void namcos22_draw_direct_poly( running_machine &machine, const UINT16 *pSource );
UINT32 namcos22_point_rom_r( running_machine &machine, offs_t offs );
void namcos22_enable_slave_simulation( running_machine &machine );
