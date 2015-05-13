// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s

    Newport is modular, consisting of the following custom chips:
    - REX3: Raster Engine, which is basically a blitter which can also draw antialiased lines.
            REX also acts as the interface to the rest of the system - all the other chips on
        a Newport board are accessed through it.
    - RB2: Frame buffer input controller
    - RO1: Frame buffer output controller
    - XMAP9: Final display generator
    - CMAP: Palette mapper
    - VC2: Video timing controller / CRTC

    Taken from the Linux Newport driver, slave addresses for Newport devices are:
            VC2         0
            Both CMAPs  1
            CMAP 0      2
            CMAP 1      3
            Both XMAPs  4
            XMAP 0      5
            XMAP 1      6
            RAMDAC      7
            VIDEO (CC1) 8
            VIDEO (AB1) 9
*/

#include "emu.h"
#include "video/newport.h"

#define VERBOSE_LEVEL ( 0 )

#if 0
INLINE void ATTR_PRINTF(3,4) verboselog(running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}
#endif

#define VC2_VIDENTRY        m_VC2.nRegister[0x00]
#define VC2_CURENTRY        m_VC2.nRegister[0x01]
#define VC2_CURSORX         m_VC2.nRegister[0x02]
#define VC2_CURSORY         m_VC2.nRegister[0x03]
#define VC2_CURCURSORX      m_VC2.nRegister[0x04]
#define VC2_DIDENTRY        m_VC2.nRegister[0x05]
#define VC2_SCANLINELEN     m_VC2.nRegister[0x06]
#define VC2_RAMADDR         m_VC2.nRegister[0x07]
#define VC2_VTFRAMEPTR      m_VC2.nRegister[0x08]
#define VC2_VTLINEPTR       m_VC2.nRegister[0x09]
#define VC2_VTLINERUN       m_VC2.nRegister[0x0a]
#define VC2_VLINECNT        m_VC2.nRegister[0x0b]
#define VC2_CURTABLEPTR     m_VC2.nRegister[0x0c]
#define VC2_WORKCURSORY     m_VC2.nRegister[0x0d]
#define VC2_DIDFRAMEPTR     m_VC2.nRegister[0x0e]
#define VC2_DIDLINEPTR      m_VC2.nRegister[0x0f]
#define VC2_DISPLAYCTRL     m_VC2.nRegister[0x10]
#define VC2_CONFIG          m_VC2.nRegister[0x1f]

#define XMAP0_CONFIG        m_XMAP0.nRegister[0x00]
#define XMAP0_REVISION      m_XMAP0.nRegister[0x01]
#define XMAP0_ENTRIES       m_XMAP0.nRegister[0x02]
#define XMAP0_CURCMAP       m_XMAP0.nRegister[0x03]
#define XMAP0_POPUPCMAP     m_XMAP0.nRegister[0x04]
#define XMAP0_MODETBLIDX    m_XMAP0.nRegister[0x07]

#define XMAP1_CONFIG        m_XMAP1.nRegister[0x00]
#define XMAP1_REVISION      m_XMAP1.nRegister[0x01]
#define XMAP1_ENTRIES       m_XMAP1.nRegister[0x02]
#define XMAP1_CURCMAP       m_XMAP1.nRegister[0x03]
#define XMAP1_POPUPCMAP     m_XMAP1.nRegister[0x04]
#define XMAP1_MODETBLIDX    m_XMAP1.nRegister[0x07]


const device_type NEWPORT_VIDEO = &device_creator<newport_video_device>;


newport_video_device::newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, NEWPORT_VIDEO, "SGI Newport graphics board", tag, owner, clock, "newport_video", __FILE__)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void newport_video_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newport_video_device::device_start()
{
	m_base = auto_alloc_array_clear(machine(), UINT32, (1280+64) * (1024+64));

	save_pointer(NAME(m_base), (1280+64) * (1024+64));
	save_item(NAME(m_VC2.nRegister));
	save_item(NAME(m_VC2.nRAM));
	save_item(NAME(m_VC2.nRegIdx));
	save_item(NAME(m_VC2.nRegData));

	save_item(NAME(m_XMAP0.nRegister));
	save_item(NAME(m_XMAP0.nModeTable));
	save_item(NAME(m_XMAP1.nRegister));
	save_item(NAME(m_XMAP1.nModeTable));

	save_item(NAME(m_REX3.nDrawMode1));
	save_item(NAME(m_REX3.nDrawMode0));
	save_item(NAME(m_REX3.nLSMode));
	save_item(NAME(m_REX3.nLSPattern));
	save_item(NAME(m_REX3.nLSPatSave));
	save_item(NAME(m_REX3.nZPattern));
	save_item(NAME(m_REX3.nColorBack));
	save_item(NAME(m_REX3.nColorVRAM));
	save_item(NAME(m_REX3.nAlphaRef));
	save_item(NAME(m_REX3.nSMask0X));
	save_item(NAME(m_REX3.nSMask0Y));
	save_item(NAME(m_REX3.nSetup));
	save_item(NAME(m_REX3.nStepZ));
	save_item(NAME(m_REX3.nXStart));
	save_item(NAME(m_REX3.nYStart));
	save_item(NAME(m_REX3.nXEnd));
	save_item(NAME(m_REX3.nYEnd));

	save_item(NAME(m_REX3.nXSave));
	save_item(NAME(m_REX3.nXYMove));
	save_item(NAME(m_REX3.nBresD));
	save_item(NAME(m_REX3.nBresS1));
	save_item(NAME(m_REX3.nBresOctInc1));
	save_item(NAME(m_REX3.nBresRndInc2));
	save_item(NAME(m_REX3.nBresE1));
	save_item(NAME(m_REX3.nBresS2));
	save_item(NAME(m_REX3.nAWeight0));
	save_item(NAME(m_REX3.nAWeight1));
	save_item(NAME(m_REX3.nXStartF));
	save_item(NAME(m_REX3.nYStartF));
	save_item(NAME(m_REX3.nXEndF));
	save_item(NAME(m_REX3.nYEndF));
	save_item(NAME(m_REX3.nXStartI));
	save_item(NAME(m_REX3.nXYStartI));
	save_item(NAME(m_REX3.nXYEndI));
	save_item(NAME(m_REX3.nXStartEndI));
	save_item(NAME(m_REX3.nColorRed));
	save_item(NAME(m_REX3.nColorAlpha));
	save_item(NAME(m_REX3.nColorGreen));
	save_item(NAME(m_REX3.nColorBlue));
	save_item(NAME(m_REX3.nSlopeRed));
	save_item(NAME(m_REX3.nSlopeAlpha));
	save_item(NAME(m_REX3.nSlopeGreen));
	save_item(NAME(m_REX3.nSlopeBlue));
	save_item(NAME(m_REX3.nWriteMask));
	save_item(NAME(m_REX3.nZeroFract));
	save_item(NAME(m_REX3.nZeroOverflow));
	save_item(NAME(m_REX3.nHostDataPortMSW));
	save_item(NAME(m_REX3.nHostDataPortLSW));
	save_item(NAME(m_REX3.nDCBMode));
	save_item(NAME(m_REX3.nDCBRegSelect));
	save_item(NAME(m_REX3.nDCBSlvSelect));
	save_item(NAME(m_REX3.nDCBDataMSW));
	save_item(NAME(m_REX3.nDCBDataLSW));
	save_item(NAME(m_REX3.nSMask1X));
	save_item(NAME(m_REX3.nSMask1Y));
	save_item(NAME(m_REX3.nSMask2X));
	save_item(NAME(m_REX3.nSMask2Y));
	save_item(NAME(m_REX3.nSMask3X));
	save_item(NAME(m_REX3.nSMask3Y));
	save_item(NAME(m_REX3.nSMask4X));
	save_item(NAME(m_REX3.nSMask4Y));
	save_item(NAME(m_REX3.nTopScanline));
	save_item(NAME(m_REX3.nXYWin));
	save_item(NAME(m_REX3.nClipMode));
	save_item(NAME(m_REX3.nConfig));
	save_item(NAME(m_REX3.nStatus));
	save_item(NAME(m_REX3.nXFerWidth));
#if 0
	save_item(NAME(m_REX3.nCurrentX));
	save_item(NAME(m_REX3.nCurrentY));
#endif
	save_item(NAME(m_REX3.nKludge_SkipLine));

	save_item(NAME(m_nDrawGreen));
	save_item(NAME(m_CMAP0.nPaletteIndex));
	save_item(NAME(m_CMAP0.nPalette));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newport_video_device::device_reset()
{
	m_nDrawGreen = 0;
	m_REX3.nDrawMode0 = 0x00000000;
	m_REX3.nDrawMode1 = 0x3002f001;
	m_REX3.nDCBMode = 0x00000780;
	m_REX3.nKludge_SkipLine = 0;
}


UINT32 newport_video_device::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* loop over rows and copy to the destination */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *src = &m_base[1344 * y];
		UINT32 *dest = &bitmap.pix32(y, cliprect.min_x);

		/* loop over columns */
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			*dest++ = (*src++) & 0x00f8f8f8;
		}
	}
	return 0;
}


WRITE32_MEMBER( newport_video_device::cmap0_w )
{
	switch (m_REX3.nDCBRegSelect)
	{
	case 0x00:
		//verboselog(machine(), 2, "CMAP0 Palette Index Write: %04x\n", data & 0x0000ffff );
		m_CMAP0.nPaletteIndex = data & 0x0000ffff;
		break;
	case 0x02:
		//verboselog(machine(), 2, "CMAP0 Palette Entry %04x Write: %08x\n", m_CMAP0.nPaletteIndex, ( data >> 8 ) & 0x00ffffff );
		m_CMAP0.nPalette[m_CMAP0.nPaletteIndex] = ( data >> 8 ) & 0x00ffffff;
		break;
	default:
		//verboselog(machine(), 2, "Unknown CMAP0 Register %d Write: %08x\n", m_REX3.nDCBRegSelect, data );
		break;
	}
}

READ32_MEMBER( newport_video_device::cmap0_r )
{
	switch (m_REX3.nDCBRegSelect)
	{
	case 0x04:
		//verboselog(machine(), 2, "CMAP0 Status Read: %08x\n", 0x00000008 );
		return 0x00000008;
	case 0x06: /* Revision */
		//verboselog(machine(), 2, "CMAP0 Revision Read: CMAP Rev 1, Board Rev 2, 8bpp\n" );
		return 0x000000a1;
	default:
		//verboselog(machine(), 2, "Unknown CMAP0 Register %d Read\n", m_REX3.nDCBRegSelect );
		return 0x00000000;
	}
}

READ32_MEMBER( newport_video_device::cmap1_r )
{
	switch (m_REX3.nDCBRegSelect)
	{
	case 0x04:
		//verboselog(machine(), 2, "CMAP1 Status Read: %08x\n", 0x00000008 );
		return 0x00000008;
	case 0x06: /* Revision */
		//verboselog(machine(), 2, "CMAP1 Revision Read: CMAP Rev 1, Board Rev 2, 8bpp\n" );
		return 0x000000a1;
	default:
		//verboselog(machine(), 2, "Unknown CMAP0 Register %d Read\n", m_REX3.nDCBRegSelect );
		return 0x00000000;
	}
}

READ32_MEMBER( newport_video_device::xmap0_r )
{
	UINT8 nModeIdx;

	switch (m_REX3.nDCBRegSelect)
	{
	case 0:
		//verboselog(machine(), 2, "XMAP0 Config Read: %08x\n", XMAP0_CONFIG );
		return XMAP0_CONFIG;
	case 1:
		//verboselog(machine(), 2, "XMAP0 Revision Read: %08x\n", 0x00 );
		return 0x00000000;
	case 2:
		//verboselog(machine(), 2, "XMAP0 FIFO Availability Read: %08x\n", 0x02 );
		return 0x00000002;
	case 3:
		//verboselog(machine(), 2, "XMAP0 Cursor CMAP MSB Read: %08x\n", XMAP0_CURCMAP );
		return XMAP0_CURCMAP;
	case 4:
		//verboselog(machine(), 2, "XMAP0 Pop Up CMAP MSB Read: %08x\n", XMAP0_POPUPCMAP );
		return XMAP0_POPUPCMAP;
	case 5:
		nModeIdx = ( XMAP0_MODETBLIDX & 0x0000007c ) >> 2;
		switch (XMAP0_MODETBLIDX & 0x00000003)
		{
		case 0:
			//verboselog(machine(), 2, "XMAP0 Mode Register Read: %02x (Byte 0): %08x\n", nModeIdx, ( m_XMAP0.nModeTable[ nModeIdx ] & 0x00ff0000 ) >> 16 );
			return ( m_XMAP0.nModeTable[ nModeIdx ] & 0x00ff0000 ) >> 16;
		case 1:
			//verboselog(machine(), 2, "XMAP0 Mode Register Read: %02x (Byte 1): %08x\n", nModeIdx, ( m_XMAP0.nModeTable[ nModeIdx ] & 0x0000ff00 ) >>  8 );
			return ( m_XMAP0.nModeTable[ nModeIdx ] & 0x0000ff00 ) >>  8;
		case 2:
			//verboselog(machine(), 2, "XMAP0 Mode Register Read: %02x (Byte 2): %08x\n", nModeIdx, ( m_XMAP0.nModeTable[ nModeIdx ] & 0x000000ff ) );
			return ( m_XMAP0.nModeTable[ nModeIdx ] & 0x000000ff );
		}
		break;
	case 6:
		//verboselog(machine(), 2, "XMAP0 Unused Read: %08x\n", 0x00000000 );
		return 0x00000000;
	case 7:
		//verboselog(machine(), 2, "XMAP0 Mode Table Address Read: %08x\n", XMAP0_MODETBLIDX );
		return XMAP0_MODETBLIDX;
	}

	//verboselog(machine(), 2, "XMAP0 Unknown m_REX3.nDCBRegSelect Value: %02x, returning 0\n", m_REX3.nDCBRegSelect );
	return 0x00000000;
}

WRITE32_MEMBER( newport_video_device::xmap0_w )
{
	UINT8 n8BitVal = data & 0x000000ff;

	switch (m_REX3.nDCBRegSelect)
	{
	case 0:
		//verboselog(machine(), 2, "XMAP0 Config Write: %02x\n", n8BitVal );
		XMAP0_CONFIG = n8BitVal;
		break;
	case 1:
		//verboselog(machine(), 2, "XMAP0 Revision Write (Ignored): %02x\n", n8BitVal );
		break;
	case 2:
		//verboselog(machine(), 2, "XMAP0 FIFO Availability Write (Ignored): %02x\n", n8BitVal );
		break;
	case 3:
		//verboselog(machine(), 2, "XMAP0 Cursor CMAP MSB Write: %02x\n", n8BitVal );
		XMAP0_CURCMAP = n8BitVal;
		break;
	case 4:
		//verboselog(machine(), 2, "XMAP0 Pop Up CMAP MSB Write: %02x\n", n8BitVal );
		XMAP0_POPUPCMAP = n8BitVal;
		break;
	case 5:
		//verboselog(machine(), 2, "XMAP0 Mode Register Write: %02x = %06x\n", ( data & 0xff000000 ) >> 24, data & 0x00ffffff );
		m_XMAP0.nModeTable[ ( data & 0xff000000 ) >> 24 ] = data & 0x00ffffff;
		break;
	case 6:
		//verboselog(machine(), 2, "XMAP0 Unused Write (Ignored): %08x\n", data );
		break;
	case 7:
		//verboselog(machine(), 2, "XMAP0 Mode Table Address Write: %02x\n", n8BitVal );
		XMAP0_MODETBLIDX = n8BitVal;
		break;
	}
}

READ32_MEMBER( newport_video_device::xmap1_r )
{
	UINT8 nModeIdx;

	switch (m_REX3.nDCBRegSelect)
	{
	case 0:
		//verboselog(machine(), 2, "XMAP1 Config Read: %08x\n", XMAP1_CONFIG );
		return XMAP1_CONFIG;
	case 1:
		//verboselog(machine(), 2, "XMAP1 Revision Read: %08x\n", 0x00 );
		return 0x00000000;
	case 2:
		//verboselog(machine(), 2, "XMAP1 FIFO Availability Read: %08x\n", 0x02 );
		return 0x00000002;
	case 3:
		//verboselog(machine(), 2, "XMAP1 Cursor CMAP MSB Read: %08x\n", XMAP1_CURCMAP );
		return XMAP1_CURCMAP;
	case 4:
		//verboselog(machine(), 2, "XMAP1 Pop Up CMAP MSB Read: %08x\n", XMAP1_POPUPCMAP );
		return XMAP1_POPUPCMAP;
	case 5:
		nModeIdx = ( XMAP1_MODETBLIDX & 0x0000007c ) >> 2;
		switch (XMAP1_MODETBLIDX & 0x00000003)
		{
		case 0:
			//verboselog(machine(), 2, "XMAP1 Mode Register Read: %02x (Byte 0): %08x\n", nModeIdx, ( m_XMAP1.nModeTable[ nModeIdx ] & 0x00ff0000 ) >> 16 );
			return ( m_XMAP1.nModeTable[ nModeIdx ] & 0x00ff0000 ) >> 16;
		case 1:
			//verboselog(machine(), 2, "XMAP1 Mode Register Read: %02x (Byte 1): %08x\n", nModeIdx, ( m_XMAP1.nModeTable[ nModeIdx ] & 0x0000ff00 ) >>  8 );
			return ( m_XMAP1.nModeTable[ nModeIdx ] & 0x0000ff00 ) >>  8;
		case 2:
			//verboselog(machine(), 2, "XMAP1 Mode Register Read: %02x (Byte 2): %08x\n", nModeIdx, ( m_XMAP1.nModeTable[ nModeIdx ] & 0x000000ff ) );
			return ( m_XMAP1.nModeTable[ nModeIdx ] & 0x000000ff );
		}
		break;
	case 6:
		//verboselog(machine(), 2, "XMAP1 Unused Read: %08x\n", 0x00000000 );
		return 0x00000000;
	case 7:
		//verboselog(machine(), 2, "XMAP1 Mode Table Address Read: %08x\n", XMAP0_MODETBLIDX );
		return XMAP1_MODETBLIDX;
	}

	//verboselog(machine(), 2, "XMAP1 Unknown m_REX3.nDCBRegSelect Value: %02x, returning 0\n", m_REX3.nDCBRegSelect );
	return 0x00000000;
}

WRITE32_MEMBER( newport_video_device::xmap1_w )
{
	UINT8 n8BitVal = data & 0x000000ff;

	switch (m_REX3.nDCBRegSelect)
	{
	case 0:
		//verboselog(machine(), 2, "XMAP1 Config Write: %02x\n", n8BitVal );
		XMAP1_CONFIG = n8BitVal;
		break;
	case 1:
		//verboselog(machine(), 2, "XMAP1 Revision Write (Ignored): %02x\n", n8BitVal );
		break;
	case 2:
		//verboselog(machine(), 2, "XMAP1 FIFO Availability Write (Ignored): %02x\n", n8BitVal );
		break;
	case 3:
		//verboselog(machine(), 2, "XMAP1 Cursor CMAP MSB Write: %02x\n", n8BitVal );
		XMAP1_CURCMAP = n8BitVal;
		break;
	case 4:
		//verboselog(machine(), 2, "XMAP1 Pop Up CMAP MSB Write: %02x\n", n8BitVal );
		XMAP1_POPUPCMAP = n8BitVal;
		break;
	case 5:
		//verboselog(machine(), 2, "XMAP1 Mode Register Write: %02x = %06x\n", ( data & 0xff000000 ) >> 24, data & 0x00ffffff );
		m_XMAP1.nModeTable[ ( data & 0xff000000 ) >> 24 ] = data & 0x00ffffff;
		break;
	case 6:
		//verboselog(machine(), 2, "XMAP1 Unused Write (Ignored): %08x\n", data );
		break;
	case 7:
		//verboselog(machine(), 2, "XMAP1 Mode Table Address Write: %02x\n", n8BitVal );
		XMAP1_MODETBLIDX = n8BitVal;
		break;
	}
}

READ32_MEMBER( newport_video_device::vc2_r )
{
	UINT16 ret16;

	switch (m_REX3.nDCBRegSelect)
	{
	case 0x01: /* Register Read */
		//verboselog(machine(), 2, "VC2 Register Read: %02x, %08x\n", m_VC2.nRegIdx, m_VC2.nRegister[m_VC2.nRegIdx] );
		return m_VC2.nRegister[m_VC2.nRegIdx];
	case 0x03: /* RAM Read */
		//verboselog(machine(), 2, "VC2 RAM Read: %04x = %08x\n", VC2_RAMADDR, m_VC2.nRAM[VC2_RAMADDR] );
		ret16 = m_VC2.nRAM[VC2_RAMADDR];
		VC2_RAMADDR++;
		if( VC2_RAMADDR == 0x8000 )
		{
			VC2_RAMADDR = 0x0000;
		}
		return ret16;
	default:
		//verboselog(machine(), 2, "Unknown VC2 Register Read: %02x\n", m_REX3.nDCBRegSelect );
		return 0;
	}
}

WRITE32_MEMBER( newport_video_device::vc2_w )
{
	switch (m_REX3.nXFerWidth)
	{
	case 0x01: /* Register Select */
		switch (m_REX3.nDCBRegSelect)
		{
		case 0x00:
			m_VC2.nRegIdx = ( data & 0x000000ff ) >> 0;
			//verboselog(machine(), 2, "VC2 Register Select: %02x\n", m_VC2.nRegIdx );
			break;
		default:
			//verboselog(machine(), 2, "Unknown VC2 Register Select: DCB Register %02x, data = 0x%08x\n", m_REX3.nDCBRegSelect, data );
			break;
		}
		break;
	case 0x02: /* RAM Write */
		switch (m_REX3.nDCBRegSelect)
		{
		case 0x03:
			//verboselog(machine(), 2, "VC2 RAM Write: %04x = %08x\n", VC2_RAMADDR, data & 0x0000ffff );
			m_VC2.nRAM[VC2_RAMADDR] = data & 0x0000ffff;
			VC2_RAMADDR++;
			if( VC2_RAMADDR == 0x8000 )
			{
				VC2_RAMADDR = 0x0000;
			}
			break;
		default:
			//verboselog(machine(), 2, "Unknown 2-byte Write: DCB Register %02x, data = 0x%08x\n", m_REX3.nDCBRegSelect, data );
			break;
		}
		break;
	case 0x03: /* Register Write */
		switch (m_REX3.nDCBRegSelect)
		{
		case 0x00:
			//verboselog(machine(), 2, "VC2 Register Setup:\n" );
			m_VC2.nRegIdx = ( data & 0xff000000 ) >> 24;
			m_VC2.nRegData = ( data & 0x00ffff00 ) >> 8;
		switch( m_VC2.nRegIdx )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Video Entry Pointer:  %04x\n", m_VC2.nRegData );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Cursor Entry Pointer: %04x\n", m_VC2.nRegData );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Cursor X Location:    %04x\n", m_VC2.nRegData );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Cursor Y Location:    %04x\n", m_VC2.nRegData );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Current Cursor X:     %04x\n", m_VC2.nRegData );
			break;
		case 0x05:
			//verboselog(machine(), 2, "    DID Entry Pointer:    %04x\n", m_VC2.nRegData );
			break;
		case 0x06:
			//verboselog(machine(), 2, "    Scanline Length:      %04x\n", m_VC2.nRegData );
			break;
		case 0x07:
			//verboselog(machine(), 2, "    RAM Address:          %04x\n", m_VC2.nRegData );
			break;
		case 0x08:
			//verboselog(machine(), 2, "    VT Frame Table Ptr:   %04x\n", m_VC2.nRegData );
			break;
		case 0x09:
			//verboselog(machine(), 2, "    VT Line Sequence Ptr: %04x\n", m_VC2.nRegData );
			break;
		case 0x0a:
			//verboselog(machine(), 2, "    VT Lines in Run:      %04x\n", m_VC2.nRegData );
			break;
		case 0x0b:
			//verboselog(machine(), 2, "    Vertical Line Count:  %04x\n", m_VC2.nRegData );
			break;
		case 0x0c:
			//verboselog(machine(), 2, "    Cursor Table Ptr:     %04x\n", m_VC2.nRegData );
			break;
		case 0x0d:
			//verboselog(machine(), 2, "    Working Cursor Y:     %04x\n", m_VC2.nRegData );
			break;
		case 0x0e:
			//verboselog(machine(), 2, "    DID Frame Table Ptr:  %04x\n", m_VC2.nRegData );
			break;
		case 0x0f:
			//verboselog(machine(), 2, "    DID Line Table Ptr:   %04x\n", m_VC2.nRegData );
			break;
		case 0x10:
			//verboselog(machine(), 2, "    Display Control:      %04x\n", m_VC2.nRegData );
			break;
		case 0x1f:
			//verboselog(machine(), 2, "    Configuration:        %04x\n", m_VC2.nRegData );
			m_VC2.nRegister[0x20] = m_VC2.nRegData;
			break;
		default:
			//verboselog(machine(), 2, "    Unknown VC2 Register: %04x\n", m_VC2.nRegData );
			break;
		}
			m_VC2.nRegister[m_VC2.nRegIdx] = m_VC2.nRegData;
			break;
		default:
			//verboselog(machine(), 2, "Unknown VC2 Register Write: %02x = %08x\n", m_REX3.nDCBRegSelect, data );
			break;
		}
		break;
	default:
		//verboselog(machine(), 2, "Unknown VC2 XFer Width: Width %02x, DCB Register %02x, Value 0x%08x\n", m_REX3.nXFerWidth, m_REX3.nDCBRegSelect, data );
		break;
	}
}

READ32_MEMBER( newport_video_device::rex3_r )
{
//  UINT32 nTemp;

//  if( offset >= ( 0x0800 / 4 ) )
//  {
//      //verboselog(machine(), 2, "%08x:\n", 0xbf0f0000 + ( offset << 2 ) );
//  }
	switch (offset)
	{
	case 0x0000/4:
	case 0x0800/4:
		//verboselog(machine(), 2, "REX3 Draw Mode 1 Read: %08x\n", m_REX3.nDrawMode1 );
		return m_REX3.nDrawMode1;
	case 0x0004/4:
	case 0x0804/4:
		//verboselog(machine(), 2, "REX3 Draw Mode 0 Read: %08x\n", m_REX3.nDrawMode0 );
		return m_REX3.nDrawMode0;
	case 0x0008/4:
	case 0x0808/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Mode Read: %08x\n", m_REX3.nLSMode );
		return m_REX3.nLSMode;
	case 0x000c/4:
	case 0x080c/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Pattern Read: %08x\n", m_REX3.nLSPattern );
		return m_REX3.nLSPattern;
	case 0x0010/4:
	case 0x0810/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Pattern (Save) Read: %08x\n", m_REX3.nLSPatSave );
		return m_REX3.nLSPatSave;
	case 0x0014/4:
	case 0x0814/4:
		//verboselog(machine(), 2, "REX3 Pattern Register Read: %08x\n", m_REX3.nZPattern );
		return m_REX3.nZPattern;
	case 0x0018/4:
	case 0x0818/4:
		//verboselog(machine(), 2, "REX3 Opaque Pattern / Blendfunc Dest Color Read: %08x\n", m_REX3.nColorBack );
		return m_REX3.nColorBack;
	case 0x001c/4:
	case 0x081c/4:
		//verboselog(machine(), 2, "REX3 VRAM Fastclear Color Read: %08x\n", m_REX3.nColorVRAM );
		return m_REX3.nColorVRAM;
	case 0x0020/4:
	case 0x0820/4:
		//verboselog(machine(), 2, "REX3 AFUNCTION Reference Alpha Read: %08x\n", m_REX3.nAlphaRef );
		return m_REX3.nAlphaRef;
	case 0x0028/4:
	case 0x0828/4:
		//verboselog(machine(), 2, "REX3 Screenmask 0 X Min/Max Read: %08x\n", m_REX3.nSMask0X );
		return m_REX3.nSMask0X;
	case 0x002c/4:
	case 0x082c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 0 Y Min/Max Read: %08x\n", m_REX3.nSMask0Y );
		return m_REX3.nSMask0Y;
	case 0x0030/4:
	case 0x0830/4:
		//verboselog(machine(), 2, "REX3 Line/Span Setup Read: %08x\n", m_REX3.nSetup );
		return m_REX3.nSetup;
	case 0x0034/4:
	case 0x0834/4:
		//verboselog(machine(), 2, "REX3 ZPattern Enable Read: %08x\n", m_REX3.nStepZ );
		return m_REX3.nStepZ;
	case 0x0100/4:
	case 0x0900/4:
		//verboselog(machine(), 2, "REX3 X Start Read: %08x\n", m_REX3.nXStart );
		return m_REX3.nXStart;
	case 0x0104/4:
	case 0x0904/4:
		//verboselog(machine(), 2, "REX3 YStart Read: %08x\n", m_REX3.nYStart );
		return m_REX3.nYStart;
	case 0x0108/4:
	case 0x0908/4:
		//verboselog(machine(), 2, "REX3 XEnd Read: %08x\n", m_REX3.nXEnd );
		return m_REX3.nXEnd;
	case 0x010c/4:
	case 0x090c/4:
		//verboselog(machine(), 2, "REX3 YEnd Read: %08x\n", m_REX3.nYEnd );
		return m_REX3.nYEnd;
	case 0x0110/4:
	case 0x0910/4:
		//verboselog(machine(), 2, "REX3 XSave Read: %08x\n", m_REX3.nXSave );
		return m_REX3.nXSave;
	case 0x0114/4:
	case 0x0914/4:
		//verboselog(machine(), 2, "REX3 XYMove Read: %08x\n", m_REX3.nXYMove );
		return m_REX3.nXYMove;
	case 0x0118/4:
	case 0x0918/4:
		//verboselog(machine(), 2, "REX3 Bresenham D Read: %08x\n", m_REX3.nBresD );
		return m_REX3.nBresD;
	case 0x011c/4:
	case 0x091c/4:
		//verboselog(machine(), 2, "REX3 Bresenham S1 Read: %08x\n", m_REX3.nBresS1 );
		return m_REX3.nBresS1;
	case 0x0120/4:
	case 0x0920/4:
		//verboselog(machine(), 2, "REX3 Bresenham Octant & Incr1 Read: %08x\n", m_REX3.nBresOctInc1 );
		return m_REX3.nBresOctInc1;
	case 0x0124/4:
	case 0x0924/4:
		//verboselog(machine(), 2, "REX3 Bresenham Octant Rounding Mode & Incr2 Read: %08x\n", m_REX3.nBresRndInc2 );
		return m_REX3.nBresRndInc2;
	case 0x0128/4:
	case 0x0928/4:
		//verboselog(machine(), 2, "REX3 Bresenham E1 Read: %08x\n", m_REX3.nBresE1 );
		return m_REX3.nBresE1;
	case 0x012c/4:
	case 0x092c/4:
		//verboselog(machine(), 2, "REX3 Bresenham S2 Read: %08x\n", m_REX3.nBresS2 );
		return m_REX3.nBresS2;
	case 0x0130/4:
	case 0x0930/4:
		//verboselog(machine(), 2, "REX3 AA Line Weight Table 1/2 Read: %08x\n", m_REX3.nAWeight0 );
		return m_REX3.nAWeight0;
	case 0x0134/4:
	case 0x0934/4:
		//verboselog(machine(), 2, "REX3 AA Line Weight Table 2/2 Read: %08x\n", m_REX3.nAWeight1 );
		return m_REX3.nAWeight1;
	case 0x0138/4:
	case 0x0938/4:
		//verboselog(machine(), 2, "REX3 GL XStart Read: %08x\n", m_REX3.nXStartF );
		return m_REX3.nXStartF;
	case 0x013c/4:
	case 0x093c/4:
		//verboselog(machine(), 2, "REX3 GL YStart Read: %08x\n", m_REX3.nYStartF );
		return m_REX3.nYStartF;
	case 0x0140/4:
	case 0x0940/4:
		//verboselog(machine(), 2, "REX3 GL XEnd Read: %08x\n", m_REX3.nXEndF );
		return m_REX3.nXEndF;
	case 0x0144/4:
	case 0x0944/4:
		//verboselog(machine(), 2, "REX3 GL YEnd Read: %08x\n", m_REX3.nYEndF );
		return m_REX3.nYEndF;
	case 0x0148/4:
	case 0x0948/4:
		//verboselog(machine(), 2, "REX3 XStart (integer) Read: %08x\n", m_REX3.nXStartI );
		return m_REX3.nXStartI;
	case 0x014c/4:
	case 0x094c/4:
		//verboselog(machine(), 2, "REX3 GL XEnd (copy) Read: %08x\n", m_REX3.nXEndF );
		return m_REX3.nXEndF;
	case 0x0150/4:
	case 0x0950/4:
		//verboselog(machine(), 2, "REX3 XYStart (integer) Read: %08x\n", m_REX3.nXYStartI );
		return m_REX3.nXYStartI;
	case 0x0154/4:
	case 0x0954/4:
		//verboselog(machine(), 2, "REX3 XYEnd (integer) Read: %08x\n", m_REX3.nXYEndI );
		return m_REX3.nXYEndI;
	case 0x0158/4:
	case 0x0958/4:
		//verboselog(machine(), 2, "REX3 XStartEnd (integer) Read: %08x\n", m_REX3.nXStartEndI );
		return m_REX3.nXStartEndI;
	case 0x0200/4:
	case 0x0a00/4:
		//verboselog(machine(), 2, "REX3 Red/CI Full State Read: %08x\n", m_REX3.nColorRed );
		return m_REX3.nColorRed;
	case 0x0204/4:
	case 0x0a04/4:
		//verboselog(machine(), 2, "REX3 Alpha Full State Read: %08x\n", m_REX3.nColorAlpha );
		return m_REX3.nColorAlpha;
	case 0x0208/4:
	case 0x0a08/4:
		//verboselog(machine(), 2, "REX3 Green Full State Read: %08x\n", m_REX3.nColorGreen );
		return m_REX3.nColorGreen;
	case 0x020c/4:
	case 0x0a0c/4:
		//verboselog(machine(), 2, "REX3 Blue Full State Read: %08x\n", m_REX3.nColorBlue );
		return m_REX3.nColorBlue;
	case 0x0210/4:
	case 0x0a10/4:
		//verboselog(machine(), 2, "REX3 Red/CI Slope Read: %08x\n", m_REX3.nSlopeRed );
		return m_REX3.nSlopeRed;
	case 0x0214/4:
	case 0x0a14/4:
		//verboselog(machine(), 2, "REX3 Alpha Slope Read: %08x\n", m_REX3.nSlopeAlpha );
		return m_REX3.nSlopeAlpha;
	case 0x0218/4:
	case 0x0a18/4:
		//verboselog(machine(), 2, "REX3 Green Slope Read: %08x\n", m_REX3.nSlopeGreen );
		return m_REX3.nSlopeGreen;
	case 0x021c/4:
	case 0x0a1c/4:
		//verboselog(machine(), 2, "REX3 Blue Slope Read: %08x\n", m_REX3.nSlopeBlue );
		return m_REX3.nSlopeBlue;
	case 0x0220/4:
	case 0x0a20/4:
		//verboselog(machine(), 2, "REX3 Write Mask Read: %08x\n", m_REX3.nWriteMask );
		return m_REX3.nWriteMask;
	case 0x0224/4:
	case 0x0a24/4:
		//verboselog(machine(), 2, "REX3 Packed Color Fractions Read: %08x\n", m_REX3.nZeroFract );
		return m_REX3.nZeroFract;
	case 0x0228/4:
	case 0x0a28/4:
		//verboselog(machine(), 2, "REX3 Color Index Zeros Overflow Read: %08x\n", m_REX3.nZeroOverflow );
		return m_REX3.nZeroOverflow;
	case 0x022c/4:
	case 0x0a2c/4:
		//verboselog(machine(), 2, "REX3 Red/CI Slope (copy) Read: %08x\n", m_REX3.nSlopeRed );
		return m_REX3.nSlopeRed;
	case 0x0230/4:
	case 0x0a30/4:
		//verboselog(machine(), 2, "REX3 Host Data Port MSW Read: %08x\n", m_REX3.nHostDataPortMSW );
		return m_REX3.nHostDataPortMSW;
	case 0x0234/4:
	case 0x0a34/4:
		//verboselog(machine(), 2, "REX3 Host Data Port LSW Read: %08x\n", m_REX3.nHostDataPortLSW );
		return m_REX3.nHostDataPortLSW;
	case 0x0238/4:
	case 0x0a38/4:
		//verboselog(machine(), 2, "REX3 Display Control Bus Mode Read: %08x\n", m_REX3.nDCBMode );
		return m_REX3.nDCBMode;
	case 0x0240/4:
	case 0x0a40/4:
		switch (m_REX3.nDCBSlvSelect)
		{
		case 0x00:
			return vc2_r(space, 0, mem_mask);
		case 0x02:
			return cmap0_r(space, 0, mem_mask);
		case 0x03:
			return cmap1_r(space, 0, mem_mask);
		case 0x05:
			return xmap0_r(space, 0, mem_mask);
		case 0x06:
			return xmap1_r(space, 0, mem_mask);
		default:
			//verboselog(machine(), 2, "REX3 Display Control Bus Data MSW Read: %08x\n", m_REX3.nDCBDataMSW );
			break;
		}
		return m_REX3.nDCBDataMSW;
	case 0x0244/4:
	case 0x0a44/4:
		//verboselog(machine(), 2, "REX3 Display Control Bus Data LSW Read: %08x\n", m_REX3.nDCBDataLSW );
		return m_REX3.nDCBDataLSW;
	case 0x1300/4:
		//verboselog(machine(), 2, "REX3 Screenmask 1 X Min/Max Read: %08x\n", m_REX3.nSMask1X );
		return m_REX3.nSMask1X;
	case 0x1304/4:
		//verboselog(machine(), 2, "REX3 Screenmask 1 Y Min/Max Read: %08x\n", m_REX3.nSMask1Y );
		return m_REX3.nSMask1Y;
	case 0x1308/4:
		//verboselog(machine(), 2, "REX3 Screenmask 2 X Min/Max Read: %08x\n", m_REX3.nSMask2X );
		return m_REX3.nSMask2X;
	case 0x130c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 2 Y Min/Max Read: %08x\n", m_REX3.nSMask2Y );
		return m_REX3.nSMask2Y;
	case 0x1310/4:
		//verboselog(machine(), 2, "REX3 Screenmask 3 X Min/Max Read: %08x\n", m_REX3.nSMask3X );
		return m_REX3.nSMask3X;
	case 0x1314/4:
		//verboselog(machine(), 2, "REX3 Screenmask 3 Y Min/Max Read: %08x\n", m_REX3.nSMask3Y );
		return m_REX3.nSMask3Y;
	case 0x1318/4:
		//verboselog(machine(), 2, "REX3 Screenmask 4 X Min/Max Read: %08x\n", m_REX3.nSMask4X );
		return m_REX3.nSMask4X;
	case 0x131c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 4 Y Min/Max Read: %08x\n", m_REX3.nSMask4Y );
		return m_REX3.nSMask4Y;
	case 0x1320/4:
		//verboselog(machine(), 2, "REX3 Top of Screen Scanline Read: %08x\n", m_REX3.nTopScanline );
		return m_REX3.nTopScanline;
	case 0x1324/4:
		//verboselog(machine(), 2, "REX3 Clipping Mode Read: %08x\n", m_REX3.nXYWin );
		return m_REX3.nXYWin;
	case 0x1328/4:
		//verboselog(machine(), 2, "REX3 Clipping Mode Read: %08x\n", m_REX3.nClipMode );
		return m_REX3.nClipMode;
	case 0x1330/4:
		//verboselog(machine(), 2, "REX3 Config Read: %08x\n", m_REX3.nConfig );
		return m_REX3.nConfig;
	case 0x1338/4:
		//verboselog(machine(), 2, "REX3 Status Read: %08x\n", 0x00000001 );
//      nTemp = m_REX3.nStatus;
		m_REX3.nStatus = 0;
		return 0x00000001;
	case 0x133c/4:
		//verboselog(machine(), 2, "REX3 User Status Read: %08x\n", 0x00000001 );
		return 0x00000001;
	default:
		//verboselog(machine(), 2, "Unknown REX3 Read: %08x (%08x)\n", 0x1f0f0000 + ( offset << 2 ), mem_mask );
		return 0;
	}
}

void newport_video_device::DoREX3Command()
{
	UINT32 nCommand = ( ( m_REX3.nDrawMode0 & ( 1 << 15 ) ) >> 15 ) |
						( ( m_REX3.nDrawMode0 & ( 1 <<  5 ) ) >>  4 ) |
						( ( m_REX3.nDrawMode0 & ( 1 <<  9 ) ) >>  7 ) |
						( ( m_REX3.nDrawMode0 & ( 1 <<  8 ) ) >>  5 ) |
						( ( m_REX3.nDrawMode0 & 0x0000001c  ) <<  2 ) |
						( ( m_REX3.nDrawMode0 & 0x00000003  ) <<  7 );
	UINT16 nX, nY;
	UINT16 nStartX = ( m_REX3.nXYStartI >> 16 ) & 0x0000ffff;
	UINT16 nStartY = ( m_REX3.nXYStartI >>  0 ) & 0x0000ffff;
	UINT16 nEndX = ( m_REX3.nXYEndI >> 16 ) & 0x0000ffff;
	UINT16 nEndY = ( m_REX3.nXYEndI >>  0 ) & 0x0000ffff;
	INT16 nMoveX, nMoveY;

	switch (nCommand)
	{
	case 0x00000110:
		nX = nStartX;
		nY = nStartY;
		//verboselog(machine(), 3, "Tux Logo Draw: %04x, %04x = %08x\n", nX, nY, m_CMAP0.nPalette[ ( m_REX3.nHostDataPortMSW & 0xff000000 ) >> 24 ] );
//      m_REX3.nKludge_SkipLine = 1;
		m_REX3.nBresOctInc1 = 0;
		m_base[ nY*(1280+64) + nX ] = m_CMAP0.nPalette[ ( m_REX3.nHostDataPortMSW & 0xff000000 ) >> 24 ];
		nX++;
		if( nX > ( ( m_REX3.nXYEndI & 0xffff0000 ) >> 16 ) )
		{
			nY++;
			nX = m_REX3.nXSave;
		}
		m_REX3.nXYStartI = ( nX << 16 ) | nY;
		m_REX3.nXStartI = nX;
		m_REX3.nXStart = 0 | ( ( m_REX3.nXYStartI & 0xffff0000 ) >>  5 );
		m_REX3.nYStart = 0 | ( ( m_REX3.nXYStartI & 0x0000ffff ) << 11 );
		break;
	case 0x0000011e:
		//verboselog(machine(), 3, "Block draw: %04x, %04x to %04x, %04x = %08x\n", nStartX, nStartY, nEndX, nEndY, m_CMAP0.nPalette[ m_REX3.nZeroFract ] );
		for( nY = nStartY; nY <= nEndY; nY++ )
		{
			//verboselog(machine(), 3, "Pixel: %04x, %04x = %08x\n", nStartX, nY, m_CMAP0.nPalette[ m_REX3.nZeroFract ] );
			for( nX = nStartX; nX <= nEndX; nX++ )
			{
				m_base[ nY*(1280+64) + nX ] = m_CMAP0.nPalette[ m_REX3.nZeroFract ];
			}
		}
		break;
	case 0x00000119:
		if( !m_REX3.nKludge_SkipLine )
		{
			//verboselog(machine(), 3, "Pattern Line Draw: %08x at %04x, %04x color %08x\n", m_REX3.nZPattern, m_REX3.nXYStartI >> 16, m_REX3.nXYStartI & 0x0000ffff, m_CMAP0.nPalette[ m_REX3.nZeroFract ] );
			for( nX = nStartX; nX <= nEndX && nX < ( nStartX + 32 ); nX++ )
			{
				if( m_REX3.nZPattern & ( 1 << ( 31 - ( nX - nStartX ) ) ) )
				{
					m_base[ nStartY*(1280+64) + nX ] = m_CMAP0.nPalette[ m_REX3.nZeroFract ];
				}
			}
			if( m_REX3.nBresOctInc1 & 0x01000000 )
			{
				nStartY--;
			}
			else
			{
				nStartY++;
			}
			m_REX3.nXYStartI = ( nStartX << 16 ) | nStartY;
			m_REX3.nYStart = 0 | ( ( m_REX3.nXYStartI & 0x0000ffff ) << 11 );
		}
		break;
	case 0x0000019e:
		nMoveX = (INT16)( ( m_REX3.nXYMove >> 16 ) & 0x0000ffff );
		nMoveY = (INT16)( m_REX3.nXYMove & 0x0000ffff );
		//verboselog(machine(), 1, "FB to FB Copy: %04x, %04x - %04x, %04x to %04x, %04x\n", nStartX, nStartY, nEndX, nEndY, nStartX + nMoveX, nStartY + nMoveY );
		for( nY = nStartY; nY <= nEndY; nY++ )
		{
			for( nX = nStartX; nX <= nEndX; nX++ )
			{
				m_base[ (nY + nMoveY)*(1280+64) + (nX + nMoveX) ] = m_base[ nY*(1280+64) + nX ];
			}
		}
		break;
	default:
		//verboselog(machine(), 1, "Unknown draw command: %08x\n", nCommand );
		break;
	}
}

WRITE32_MEMBER( newport_video_device::rex3_w )
{
	UINT32 nTemp = 0;

	if( offset & 0x00000200 )
	{
		//verboselog(machine(), 2, "Start Cmd\n" );
	}
	switch( offset )
	{
	case 0x0000/4:
	case 0x0800/4:
		//verboselog(machine(), 2, "REX3 Draw Mode 1 Write: %08x\n", data );
		switch( data & 0x00000007 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Planes Enabled:     None\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Planes Enabled:     R/W RGB/CI\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Planes Enabled:     R/W RGBA\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Planes Enabled:     R/W OLAY\n" );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Planes Enabled:     R/W PUP\n" );
			break;
		case 0x05:
			//verboselog(machine(), 2, "    Planes Enabled:     R/W CID\n" );
			break;
		default:
			//verboselog(machine(), 2, "    Unknown Plane Enable Value\n" );
			break;
		}
		switch( ( data & 0x00000018 ) >> 3 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Plane Draw Depth:    4 bits\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Plane Draw Depth:    8 bits\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Plane Draw Depth:   12 bits\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Plane Draw Depth:   32 bits\n" );
			break;
		}
		//verboselog(machine(), 2, "    DBuf Source Buffer: %d\n", ( data & 0x00000020 ) >>  5 );
		//verboselog(machine(), 2, "    GL Y Coordinates:   %d\n", ( data & 0x00000040 ) >>  6 );
		//verboselog(machine(), 2, "    Enable Pxl Packing: %d\n", ( data & 0x00000080 ) >>  7 );
		switch( ( data & 0x00000300 ) >> 8 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    HOSTRW Depth:        4 bits\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    HOSTRW Depth:        8 bits\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    HOSTRW Depth:       12 bits\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    HOSTRW Depth:       32 bits\n" );
			break;
		}
		//verboselog(machine(), 2, "    DWord Transfers:    %d\n", ( data & 0x00000400 ) >> 10 );
		//verboselog(machine(), 2, "    Swap Endianness:    %d\n", ( data & 0x00000800 ) >> 11 );
		//verboselog(machine(), 2, "    Compare Src > Dest: %d\n", ( data & 0x00001000 ) >> 12 );
		//verboselog(machine(), 2, "    Compare Src = Dest: %d\n", ( data & 0x00002000 ) >> 13 );
		//verboselog(machine(), 2, "    Compare Src < Dest: %d\n", ( data & 0x00004000 ) >> 14 );
		//verboselog(machine(), 2, "    RGB Mode Select:    %d\n", ( data & 0x00008000 ) >> 15 );
		//verboselog(machine(), 2, "    Enable Dithering:   %d\n", ( data & 0x00010000 ) >> 16 );
		//verboselog(machine(), 2, "    Enable Fast Clear:  %d\n", ( data & 0x00020000 ) >> 17 );
		//verboselog(machine(), 2, "    Enable Blending:    %d\n", ( data & 0x00040000 ) >> 18 );
		switch( ( data & 0x00380000 ) >> 19 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Src Blend Factor:   0\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Src Blend Factor:   1\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Src Blend Factor:   Normalized Dest (or COLORBACK)\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Src Blend Factor:   1 - Normalized Dest (or COLORBACK)\n" );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Src Blend Factor:   Normalized Src\n" );
			break;
		case 0x05:
			//verboselog(machine(), 2, "    Src Blend Factor:   1 - Normalized Src\n" );
			break;
		default:
			//verboselog(machine(), 2, "    Unknown Src Blend Factor: %02x\n", ( data & 0x00380000 ) >> 19 );
			break;
		}
		switch( ( data & 0x01c00000 ) >> 22 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Dest Blend Factor:  0\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Dest Blend Factor:  1\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Dest Blend Factor:  Normalized Dest (or COLORBACK)\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Dest Blend Factor:  1 - Normalized Dest (or COLORBACK)\n" );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Dest Blend Factor:  Normalized Src\n" );
			break;
		case 0x05:
			//verboselog(machine(), 2, "    Dest Blend Factor:  1 - Normalized Src\n" );
			break;
		default:
			//verboselog(machine(), 2, "    Unknown Src Blend Factor: %02x\n", ( data & 0x00380000 ) >> 19 );
			break;
		}
		//verboselog(machine(), 2, "  COLORBACK Dest Blend: %d\n", ( data & 0x02000000 ) >> 25 );
		//verboselog(machine(), 2, "   Enable Pxl Prefetch: %d\n", ( data & 0x04000000 ) >> 26 );
		//verboselog(machine(), 2, "    SFACTOR Src Alpha:  %d\n", ( data & 0x08000000 ) >> 27 );
		switch( ( data & 0xf0000000 ) >> 28 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Logical Op. Type:   0\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src & Dst\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src & ~Dst\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src\n" );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~Src & Dst\n" );
			break;
		case 0x05:
			//verboselog(machine(), 2, "    Logical Op. Type:   Dst\n" );
			break;
		case 0x06:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src ^ Dst\n" );
			break;
		case 0x07:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src | Dst\n" );
			break;
		case 0x08:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~(Src | Dst)\n" );
			break;
		case 0x09:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~(Src ^ Dst)\n" );
			break;
		case 0x0a:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~Dst\n" );
			break;
		case 0x0b:
			//verboselog(machine(), 2, "    Logical Op. Type:   Src | ~Dst\n" );
			break;
		case 0x0c:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~Src\n" );
			break;
		case 0x0d:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~Src | Dst\n" );
			break;
		case 0x0e:
			//verboselog(machine(), 2, "    Logical Op. Type:   ~(Src & Dst)\n" );
			break;
		case 0x0f:
			//verboselog(machine(), 2, "    Logical Op. Type:   1\n" );
			break;
		}
		m_REX3.nDrawMode1 = data;
//      if( offset >= ( 0x800 / 4 ) )
//      {
//          DoREX3Command();
//      }
		break;
	case 0x0004/4:
		//verboselog(machine(), 2, "REX3 Draw Mode 0 Write: %08x\n", data );
		switch( data & 0x00000003 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Primitive Function: No Op\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Primitive Function: Read From FB\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Primitive Function: Draw To FB\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Primitive Function: Copy FB To FB\n" );
			break;
		}
		switch( ( data & 0x0000001c ) >> 2 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Addressing Mode: Span/Point\n" );
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Addressing Mode: Block\n" );
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Addressing Mode: Bresenham Line, Integer Endpoints\n" );
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Addressing Mode: Bresenham Line, Fractional Endpoints\n" );
			break;
		case 0x04:
			//verboselog(machine(), 2, "    Addressing Mode: AA Bresenham Line\n" );
			break;
		default:
			//verboselog(machine(), 2, "    Unknown Addressing Mode: %02x\n", ( data & 0x0000001c ) >> 2 );
			break;
		}
		//verboselog(machine(), 2, "    Iterator Setup:     %d\n", ( data & 0x00000020 ) >>  5 );
		//verboselog(machine(), 2, "    RGB/CI Draw Source: %d\n", ( data & 0x00000040 ) >>  6 );
		//verboselog(machine(), 2, "     Alpha Draw Source: %d\n", ( data & 0x00000080 ) >>  7 );
		//verboselog(machine(), 2, "    Stop On X:          %d\n", ( data & 0x00000100 ) >>  8 );
		//verboselog(machine(), 2, "    Stop On Y:          %d\n", ( data & 0x00000200 ) >>  9 );
		//verboselog(machine(), 2, "    Skip Start Point:   %d\n", ( data & 0x00000400 ) >> 10 );
		//verboselog(machine(), 2, "    Skip End Point:     %d\n", ( data & 0x00000800 ) >> 11 );
		//verboselog(machine(), 2, "    Enable Patterning:  %d\n", ( data & 0x00001000 ) >> 12 );
		//verboselog(machine(), 2, "    Enable Stippling:   %d\n", ( data & 0x00002000 ) >> 13 );
		//verboselog(machine(), 2, "    Stipple Advance:    %d\n", ( data & 0x00004000 ) >> 14 );
		//verboselog(machine(), 2, "    Limit Draw To 32px: %d\n", ( data & 0x00008000 ) >> 15 );
		//verboselog(machine(), 2, "     Z Opaque Stipple   %d\n", ( data & 0x00010000 ) >> 16 );
		//verboselog(machine(), 2, "    LS Opaque Stipple:  %d\n", ( data & 0x00020000 ) >> 17 );
		//verboselog(machine(), 2, "    Enable Lin. Shade:  %d\n", ( data & 0x00040000 ) >> 18 );
		//verboselog(machine(), 2, "    Left-Right Only:    %d\n", ( data & 0x00080000 ) >> 19 );
		//verboselog(machine(), 2, "    Offset by XYMove:   %d\n", ( data & 0x00100000 ) >> 20 );
		//verboselog(machine(), 2, "    Enable CI Clamping: %d\n", ( data & 0x00200000 ) >> 21 );
		//verboselog(machine(), 2, "    Enable End Filter:  %d\n", ( data & 0x00400000 ) >> 22 );
		//verboselog(machine(), 2, "    Enable Y+2 Stride:  %d\n", ( data & 0x00800000 ) >> 23 );
		m_REX3.nDrawMode0 = data;
		break;
	case 0x0804/4:
		//verboselog(machine(), 2, "REX3 Draw Mode 0 Write: %08x\n", data );
		m_REX3.nDrawMode0 = data;
		break;
	case 0x0008/4:
	case 0x0808/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Mode Write: %08x\n", data );
		m_REX3.nLSMode = data & 0x0fffffff;
		break;
	case 0x000C/4:
	case 0x080c/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Pattern Write: %08x\n", data );
		m_REX3.nLSPattern = data;
		break;
	case 0x0010/4:
	case 0x0810/4:
		//verboselog(machine(), 2, "REX3 Line Stipple Pattern (Save) Write: %08x\n", data );
		m_REX3.nLSPatSave = data;
		break;
	case 0x0014/4:
	case 0x0814/4:
		//verboselog(machine(), 2, "REX3 Pattern Register Write: %08x\n", data );
		m_REX3.nZPattern = data;
		if (offset & 0x00000200)
		{
			DoREX3Command();
		}
		break;
	case 0x0018/4:
	case 0x0818/4:
		//verboselog(machine(), 2, "REX3 Opaque Pattern / Blendfunc Dest Color Write: %08x\n", data );
		m_REX3.nColorBack = data;
		break;
	case 0x001c/4:
	case 0x081c/4:
		//verboselog(machine(), 2, "REX3 VRAM Fastclear Color Write: %08x\n", data );
		m_REX3.nColorVRAM = data;
		break;
	case 0x0020/4:
	case 0x0820/4:
		//verboselog(machine(), 2, "REX3 AFUNCTION Reference Alpha Write: %08x\n", data );
		m_REX3.nAlphaRef = data & 0x000000ff;
		break;
	case 0x0024/4:
	case 0x0824/4:
		//verboselog(machine(), 2, "REX3 Stall GFIFO Write: %08x\n", data );
		break;
	case 0x0028/4:
	case 0x0828/4:
		//verboselog(machine(), 2, "REX3 Screenmask 0 X Min/Max Write: %08x\n", data );
		m_REX3.nSMask0X = data;
		break;
	case 0x002c/4:
	case 0x082c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 0 Y Min/Max Write: %08x\n", data );
		m_REX3.nSMask0Y = data;
		break;
	case 0x0030/4:
	case 0x0830/4:
		//verboselog(machine(), 2, "REX3 Line/Span Setup Write: %08x\n", data );
		m_REX3.nSetup = data;
		break;
	case 0x0034/4:
	case 0x0834/4:
		//verboselog(machine(), 2, "REX3 ZPattern Enable Write: %08x\n", data );
		m_REX3.nStepZ = data;
		break;
	case 0x0038/4:
	case 0x0838/4:
		//verboselog(machine(), 2, "REX3 Update LSPATTERN/LSRCOUNT\n" );
		m_REX3.nLSPattern = m_REX3.nLSPatSave;
		break;
	case 0x003c/4:
	case 0x083c/4:
		//verboselog(machine(), 2, "REX3 Update LSPATSAVE/LSRCNTSAVE\n" );
		m_REX3.nLSPatSave = m_REX3.nLSPattern;
		break;
	case 0x0100/4:
	case 0x0900/4:
		//verboselog(machine(), 2, "REX3 XStart Write: %08x\n", data );
		m_REX3.nXStart = data & ( 0x0000fffff << 7 );
		break;
	case 0x0104/4:
	case 0x0904/4:
		//verboselog(machine(), 2, "REX3 YStart Write: %08x\n", data );
		m_REX3.nYStart = data & ( 0x0000fffff << 7 );
		break;
	case 0x0108/4:
	case 0x0908/4:
		//verboselog(machine(), 2, "REX3 XEnd Write: %08x\n", data );
		m_REX3.nXEnd = data & ( 0x0000fffff << 7 );
		break;
	case 0x010c/4:
	case 0x090c/4:
		//verboselog(machine(), 2, "REX3 YEnd Write: %08x\n", data );
		m_REX3.nYEnd = data & ( 0x0000fffff << 7 );
		break;
	case 0x0110/4:
	case 0x0910/4:
		//verboselog(machine(), 2, "REX3 XSave Write: %08x\n", data );
		m_REX3.nXSave = data & 0x0000ffff;
		m_REX3.nXStartI = m_REX3.nXSave & 0x0000ffff;
		break;
	case 0x0114/4:
	case 0x0914/4:
		//verboselog(machine(), 2, "REX3 XYMove Write: %08x\n", data );
		m_REX3.nXYMove = data;
		if (offset & 0x00000200)
		{
			DoREX3Command();
		}
		break;
	case 0x0118/4:
	case 0x0918/4:
		//verboselog(machine(), 2, "REX3 Bresenham D Write: %08x\n", data );
		m_REX3.nBresD = data & 0x07ffffff;
		break;
	case 0x011c/4:
	case 0x091c/4:
		//verboselog(machine(), 2, "REX3 Bresenham S1 Write: %08x\n", data );
		m_REX3.nBresS1 = data & 0x0001ffff;
		break;
	case 0x0120/4:
	case 0x0920/4:
		//verboselog(machine(), 2, "REX3 Bresenham Octant & Incr1 Write: %08x\n", data );
		m_REX3.nBresOctInc1 = data & 0x070fffff;
		break;
	case 0x0124/4:
	case 0x0924/4:
		//verboselog(machine(), 2, "REX3 Bresenham Octant Rounding Mode & Incr2 Write: %08x\n", data );
		m_REX3.nBresRndInc2 = data & 0xff1fffff;
		break;
	case 0x0128/4:
	case 0x0928/4:
		//verboselog(machine(), 2, "REX3 Bresenham E1 Write: %08x\n", data );
		m_REX3.nBresE1 = data & 0x0000ffff;
		break;
	case 0x012c/4:
	case 0x092c/4:
		//verboselog(machine(), 2, "REX3 Bresenham S2 Write: %08x\n", data );
		m_REX3.nBresS2 = data & 0x03ffffff;
		break;
	case 0x0130/4:
	case 0x0930/4:
		//verboselog(machine(), 2, "REX3 AA Line Weight Table 1/2 Write: %08x\n", data );
		m_REX3.nAWeight0 = data;
		break;
	case 0x0134/4:
	case 0x0934/4:
		//verboselog(machine(), 2, "REX3 AA Line Weight Table 2/2 Write: %08x\n", data );
		m_REX3.nAWeight1 = data;
		break;
	case 0x0138/4:
	case 0x0938/4:
		//verboselog(machine(), 2, "REX3 GL XStart Write: %08x\n", data );
		m_REX3.nXStartF = data & ( 0x0000ffff << 7 );
		break;
	case 0x013c/4:
	case 0x093c/4:
		//verboselog(machine(), 2, "REX3 GL YStart Write: %08x\n", data );
		m_REX3.nYStartF = data & ( 0x0000ffff << 7 );
		break;
	case 0x0140/4:
	case 0x0940/4:
		//verboselog(machine(), 2, "REX3 GL XEnd Write: %08x\n", data );
		m_REX3.nXEndF = data & ( 0x0000ffff << 7 );
		break;
	case 0x0144/4:
	case 0x0944/4:
		//verboselog(machine(), 2, "REX3 GL YEnd Write: %08x\n", data );
		m_REX3.nYEndF = data & ( 0x0000ffff << 7 );
		break;
	case 0x0148/4:
	case 0x0948/4:
		//verboselog(machine(), 2, "REX3 XStart (integer) Write: %08x\n", data );
		m_REX3.nXStartI = data & 0x0000ffff;
		m_REX3.nXSave = m_REX3.nXStartI;
		m_REX3.nXStart = 0 | ( ( m_REX3.nXStartI & 0x0000ffff ) << 11 );
		break;
	case 0x014c/4:
	case 0x094c/4:
		//verboselog(machine(), 2, "REX3 GL XEnd (copy) Write: %08x\n", data );
		m_REX3.nXEndF = data & ( 0x0000ffff << 7 );
		break;
	case 0x0150/4:
	case 0x0950/4:
		//verboselog(machine(), 2, "REX3 XYStart (integer) Write: %08x\n", data );
		m_REX3.nXYStartI = data;
		m_REX3.nXStartI = ( data & 0xffff0000 ) >> 16;
		m_REX3.nXSave = m_REX3.nXStartI;
		m_REX3.nXStart = 0 | ( ( m_REX3.nXYStartI & 0xffff0000 ) >>  5 );
		m_REX3.nYStart = 0 | ( ( m_REX3.nXYStartI & 0x0000ffff ) << 11 );
		break;
	case 0x0154/4:
	case 0x0954/4:
		//verboselog(machine(), 2, "REX3 XYEnd (integer) Write: %08x\n", data );
		m_REX3.nXYEndI = data;
		m_REX3.nXEnd = 0 | ( ( m_REX3.nXYEndI & 0xffff0000 ) >>  5 );
		m_REX3.nYEnd = 0 | ( ( m_REX3.nXYEndI & 0x0000ffff ) << 11 );
		if (offset & 0x00000200)
		{
			DoREX3Command();
		}
		break;
	case 0x0158/4:
	case 0x0958/4:
		//verboselog(machine(), 2, "REX3 XStartEnd (integer) Write: %08x\n", data );
		m_REX3.nXStartEndI = data;
		m_REX3.nXYEndI   = ( m_REX3.nXYEndI   & 0x0000ffff ) | ( ( m_REX3.nXStartEndI & 0x0000ffff ) << 16 );
		m_REX3.nXYStartI = ( m_REX3.nXYStartI & 0x0000ffff ) | ( m_REX3.nXStartEndI & 0xffff0000 );
		m_REX3.nXSave = m_REX3.nXStartI;
		m_REX3.nXStart = 0 | ( ( m_REX3.nXStartEndI & 0xffff0000 ) >>  5 );
		m_REX3.nXEnd   = 0 | ( ( m_REX3.nXStartEndI & 0x0000ffff ) << 11 );
		break;
	case 0x0200/4:
	case 0x0a00/4:
		//verboselog(machine(), 2, "REX3 Red/CI Full State Write: %08x\n", data );
		m_REX3.nColorRed = data & 0x00ffffff;
		break;
	case 0x0204/4:
	case 0x0a04/4:
		//verboselog(machine(), 2, "REX3 Alpha Full State Write: %08x\n", data );
		m_REX3.nColorAlpha = data & 0x000fffff;
		break;
	case 0x0208/4:
	case 0x0a08/4:
		//verboselog(machine(), 2, "REX3 Green Full State Write: %08x\n", data );
		m_REX3.nColorGreen = data & 0x000fffff;
		break;
	case 0x020c/4:
	case 0x0a0c/4:
		//verboselog(machine(), 2, "REX3 Blue Full State Write: %08x\n", data );
		m_REX3.nColorBlue = data & 0x000fffff;
		break;
	case 0x0210/4:
	case 0x0a10/4:
		//verboselog(machine(), 2, "REX3 Red/CI Slope Write: %08x\n", data );
		data &= 0x807fffff;
		switch( data & 0x80000000 )
		{
		case 0x00000000:
			nTemp = data & 0x007fffff;
			break;
		case 0x80000000:
			nTemp  = 0x00800000 - ( data & 0x007fffff );
			nTemp |= 0x00800000;
			break;
		}
		m_REX3.nSlopeRed = nTemp;
		break;
	case 0x0214/4:
	case 0x0a14/4:
		//verboselog(machine(), 2, "REX3 Alpha Slope Write: %08x\n", data );
		data &= 0x8007ffff;
		switch( data & 0x80000000 )
		{
		case 0x00000000:
			nTemp = data & 0x0007ffff;
			break;
		case 0x80000000:
			nTemp  = 0x00080000 - ( data & 0x0007ffff );
			nTemp |= 0x00080000;
			break;
		}
		m_REX3.nSlopeAlpha = nTemp;
		break;
	case 0x0218/4:
	case 0x0a18/4:
		//verboselog(machine(), 2, "REX3 Green Slope Write: %08x\n", data );
		data &= 0x8007ffff;
		switch( data & 0x80000000 )
		{
		case 0x00000000:
			nTemp = data & 0x0007ffff;
			break;
		case 0x80000000:
			nTemp  = 0x00080000 - ( data & 0x0007ffff );
			nTemp |= 0x00080000;
			break;
		}
		m_REX3.nSlopeGreen = nTemp;
		break;
	case 0x021c/4:
	case 0x0a1c/4:
		//verboselog(machine(), 2, "REX3 Blue Slope Write: %08x\n", data );
		data &= 0x8007ffff;
		switch( data & 0x80000000 )
		{
		case 0x00000000:
			nTemp = data & 0x0007ffff;
			break;
		case 0x80000000:
			nTemp  = 0x00080000 - ( data & 0x0007ffff );
			nTemp |= 0x00080000;
			break;
		}
		m_REX3.nSlopeBlue = nTemp;
		break;
	case 0x0220/4:
	case 0x0a20/4:
		//verboselog(machine(), 2, "REX3 Write Mask Write: %08x\n", data );
		m_REX3.nWriteMask = data & 0x00ffffff;
		break;
	case 0x0224/4:
	case 0x0a24/4:
		//verboselog(machine(), 2, "REX3 Packed Color Fractions Write: %08x\n", data );
		m_REX3.nZeroFract = data;
		break;
	case 0x0228/4:
	case 0x0a28/4:
		//verboselog(machine(), 2, "REX3 Color Index Zeros Overflow Write: %08x\n", data );
		m_REX3.nZeroOverflow = data;
		break;
	case 0x022c/4:
	case 0x0a2c/4:
		//verboselog(machine(), 2, "REX3 Red/CI Slope (copy) Write: %08x\n", data );
		m_REX3.nSlopeRed = data;
		break;
	case 0x0230/4:
	case 0x0a30/4:
		//verboselog(machine(), 3, "REX3 Host Data Port MSW Write: %08x\n", data );
		m_REX3.nHostDataPortMSW = data;
		if (offset & 0x00000200)
		{
			DoREX3Command();
		}
		break;
	case 0x0234/4:
	case 0x0a34/4:
		//verboselog(machine(), 2, "REX3 Host Data Port LSW Write: %08x\n", data );
		m_REX3.nHostDataPortLSW = data;
		break;
	case 0x0238/4:
	case 0x0a38/4:
		//verboselog(machine(), 2, "REX3 Display Control Bus Mode Write: %08x\n", data );
		switch( data & 0x00000003 )
		{
		case 0x00:
			//verboselog(machine(), 2, "    Transfer Width:     4 bytes\n" );
			m_REX3.nXFerWidth = 4;
			break;
		case 0x01:
			//verboselog(machine(), 2, "    Transfer Width:     1 bytes\n" );
			m_REX3.nXFerWidth = 1;
			break;
		case 0x02:
			//verboselog(machine(), 2, "    Transfer Width:     2 bytes\n" );
			m_REX3.nXFerWidth = 2;
			break;
		case 0x03:
			//verboselog(machine(), 2, "    Transfer Width:     3 bytes\n" );
			m_REX3.nXFerWidth = 3;
			break;
		}
		//verboselog(machine(), 2, "    DCB Reg Select Adr: %d\n", ( data & 0x00000070 ) >> 4 );
		//verboselog(machine(), 2, "     DCB Slave Address: %d\n", ( data & 0x00000780 ) >> 7 );
//      //verboselog(machine(), 2, "    Use Sync XFer ACK:  %d\n", ( data & 0x00000800 ) >> 11 );
//      //verboselog(machine(), 2, "    Use Async XFer ACK: %d\n", ( data & 0x00001000 ) >> 12 );
//      //verboselog(machine(), 2, "   GIO CLK Cycle Width: %d\n", ( data & 0x0003e000 ) >> 13 );
//      //verboselog(machine(), 2, "    GIO CLK Cycle Hold: %d\n", ( data & 0x007c0000 ) >> 18 );
//      //verboselog(machine(), 2, "   GIO CLK Cycle Setup: %d\n", ( data & 0x0f800000 ) >> 23 );
//      //verboselog(machine(), 2, "    Swap Byte Ordering: %d\n", ( data & 0x10000000 ) >> 28 );
		m_REX3.nDCBRegSelect = ( data & 0x00000070 ) >> 4;
		m_REX3.nDCBSlvSelect = ( data & 0x00000780 ) >> 7;
		m_REX3.nDCBMode = data & 0x1fffffff;
		break;
	case 0x0240/4:
	case 0x0a40/4:
		m_REX3.nDCBDataMSW = data;
		switch( m_REX3.nDCBSlvSelect )
		{
		case 0x00:
			vc2_w(space, 0, data, mem_mask);
			break;
		case 0x01:
			cmap0_w(space, 0, data, mem_mask);
			break;
		case 0x04:
			xmap0_w(space, 0, data, mem_mask);
			xmap1_w(space, 0, data, mem_mask);
			break;
		case 0x05:
			xmap0_w(space, 0, data, mem_mask);
			break;
		case 0x06:
			xmap1_w(space, 0, data, mem_mask);
			break;
		default:
			//verboselog(machine(), 2, "REX3 Display Control Bus Data MSW Write: %08x\n", data );
			break;
		}
		break;
	case 0x0244/4:
	case 0x0a44/4:
		//verboselog(machine(), 2, "REX3 Display Control Bus Data LSW Write: %08x\n", data );
		m_REX3.nDCBDataLSW = data;
		break;
	case 0x1300/4:
		//verboselog(machine(), 2, "REX3 Screenmask 1 X Min/Max Write: %08x\n", data );
		m_REX3.nSMask1X = data;
		break;
	case 0x1304/4:
		//verboselog(machine(), 2, "REX3 Screenmask 1 Y Min/Max Write: %08x\n", data );
		m_REX3.nSMask1Y = data;
		break;
	case 0x1308/4:
		//verboselog(machine(), 2, "REX3 Screenmask 2 X Min/Max Write: %08x\n", data );
		m_REX3.nSMask2X = data;
		break;
	case 0x130c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 2 Y Min/Max Write: %08x\n", data );
		m_REX3.nSMask2Y = data;
		break;
	case 0x1310/4:
		//verboselog(machine(), 2, "REX3 Screenmask 3 X Min/Max Write: %08x\n", data );
		m_REX3.nSMask3X = data;
		break;
	case 0x1314/4:
		//verboselog(machine(), 2, "REX3 Screenmask 3 Y Min/Max Write: %08x\n", data );
		m_REX3.nSMask3Y = data;
		break;
	case 0x1318/4:
		//verboselog(machine(), 2, "REX3 Screenmask 4 X Min/Max Write: %08x\n", data );
		m_REX3.nSMask4X = data;
		break;
	case 0x131c/4:
		//verboselog(machine(), 2, "REX3 Screenmask 4 Y Min/Max Write: %08x\n", data );
		m_REX3.nSMask4Y = data;
		break;
	case 0x1320/4:
		//verboselog(machine(), 2, "REX3 Top of Screen Scanline Write: %08x\n", data );
		m_REX3.nTopScanline = data & 0x000003ff;
		break;
	case 0x1324/4:
		//verboselog(machine(), 2, "REX3 Clipping Mode Write: %08x\n", data );
		m_REX3.nXYWin = data;
		break;
	case 0x1328/4:
		//verboselog(machine(), 2, "REX3 Clipping Mode Write: %08x\n", data );
		m_REX3.nClipMode = data & 0x00001fff;
		break;
	case 0x132c/4:
		//verboselog(machine(), 2, "Request GFIFO Stall\n" );
		break;
	case 0x1330/4:
		//verboselog(machine(), 2, "REX3 Config Write: %08x\n", data );
		m_REX3.nConfig = data & 0x001fffff;
		break;
	case 0x1340/4:
		//verboselog(machine(), 2, "Reset DCB Bus and Flush BFIFO\n" );
		break;
	default:
		//verboselog(machine(), 2, "Unknown REX3 Write: %08x (%08x): %08x\n", 0xbf0f0000 + ( offset << 2 ), mem_mask, data );
		break;
	}
}
