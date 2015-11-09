// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Sandro Ronco
/***************************************************************************

    HD63484 ACRTC (rewrite in progress)

    TODO:
    - 8-bit support for FIFO, parameters and command values
    - execution cycles;

***************************************************************************/

#include "emu.h"
#include "h63484.h"

#define LOG 0
#define FIFO_LOG 0
#define CMD_LOG 0

// default address map
static ADDRESS_MAP_START( h63484_vram, AS_0, 16, h63484_device )
//  AM_RANGE(0x00000, 0x7ffff) AM_RAM
//  AM_RANGE(0x80000, 0xfffff) AM_NOP
ADDRESS_MAP_END


//-------------------------------------------------
//  h63484_device - constructor
//-------------------------------------------------

h63484_device::h63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, H63484, "HD63484 CRTC (rewrite)", tag, owner, clock, "h63484", __FILE__),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_ar(0),
	m_sr(0),
	m_fifo_ptr(-1),
	m_fifo_r_ptr(-1),
	m_cr(0),
	m_param_ptr(0),
	m_rwp_dn(0),
	m_org_dpa(0),
	m_org_dn(0),
	m_org_dpd(0),
	m_cl0(0),
	m_cl1(0),
	m_dcr(0),
	m_space_config("videoram", ENDIANNESS_BIG, 16, 20, -1, NULL, *ADDRESS_MAP_NAME(h63484_vram))
{
}

enum
{
	FIFO_READ = 0,
	FIFO_WRITE
};

enum
{
	FIFO_EMPTY = -1,
	FIFO_PARAMETER,
	FIFO_COMMAND
};

#define H63484_SR_CER 0x80 // Command Error
#define H63484_SR_ARD 0x40 // Area Detect
#define H63484_SR_CED 0x20 // Command End
#define H63484_SR_LPD 0x10 // Light Pen Strobe Detect
#define H63484_SR_RFF 0x08 // Read FIFO Full
#define H63484_SR_RFR 0x04 // Read FIFO Ready
#define H63484_SR_WFR 0x02 // Write FIFO Ready
#define H63484_SR_WFE 0x01 // Write FIFO Empty


static const char *const acrtc_regnames[0x100/2] =
{
	"FIFO Entry", // 0x00
	"Command Control (CCR)", // 0x02
	"Operation Mode (OMR)", // 0x04
	"Display Control (DCR)", // 0x06
	"(Undefined)", // 0x08
	"(Undefined)", // 0x0a
	"(Undefined)", // 0x0c
	"(Undefined)", // 0x0e
	"(Undefined)", // 0x10
	"(Undefined)", // 0x12
	"(Undefined)", // 0x14
	"(Undefined)", // 0x16
	"(Undefined)", // 0x18
	"(Undefined)", // 0x1a
	"(Undefined)", // 0x1c
	"(Undefined)", // 0x1e
	"(Undefined)", // 0x20
	"(Undefined)", // 0x22
	"(Undefined)", // 0x24
	"(Undefined)", // 0x26
	"(Undefined)", // 0x28
	"(Undefined)", // 0x2a
	"(Undefined)", // 0x2c
	"(Undefined)", // 0x2e
	"(Undefined)", // 0x30
	"(Undefined)", // 0x32
	"(Undefined)", // 0x34
	"(Undefined)", // 0x36
	"(Undefined)", // 0x38
	"(Undefined)", // 0x3a
	"(Undefined)", // 0x3c
	"(Undefined)", // 0x3e
	"(Undefined)", // 0x40
	"(Undefined)", // 0x42
	"(Undefined)", // 0x44
	"(Undefined)", // 0x46
	"(Undefined)", // 0x48
	"(Undefined)", // 0x4a
	"(Undefined)", // 0x4c
	"(Undefined)", // 0x4e
	"(Undefined)", // 0x50
	"(Undefined)", // 0x52
	"(Undefined)", // 0x54
	"(Undefined)", // 0x56
	"(Undefined)", // 0x58
	"(Undefined)", // 0x5a
	"(Undefined)", // 0x5c
	"(Undefined)", // 0x5e
	"(Undefined)", // 0x60
	"(Undefined)", // 0x62
	"(Undefined)", // 0x64
	"(Undefined)", // 0x66
	"(Undefined)", // 0x68
	"(Undefined)", // 0x6a
	"(Undefined)", // 0x6c
	"(Undefined)", // 0x6e
	"(Undefined)", // 0x70
	"(Undefined)", // 0x72
	"(Undefined)", // 0x74
	"(Undefined)", // 0x76
	"(Undefined)", // 0x78
	"(Undefined)", // 0x7a
	"(Undefined)", // 0x7c
	"(Undefined)", // 0x7e
	"Raster Count (RCR)", // 0x80
	"Horizontal Sync (HSR)", // 0x82
	"Horizontal Display (HDR)", // 0x84
	"Vertical Sync (VSR)", // 0x86
	"Vertical Display (VDR)", // 0x88
	"Split Screen Width (SSW) 0x8a", // 0x8a
	"Split Screen Width (SSW) 0x8c", // 0x8c
	"Split Screen Width (SSW) 0x8e", // 0x8e
	"Blink Control (BCR)", // 0x90
	"Horizontal Window Display (HWR)", // 0x92
	"Vertical Window Display (VWR) 0x94", // 0x94
	"Vertical Window Display (VWR) 0x96", // 0x96
	"Graphic Cursor (GCR0)", // 0x98
	"Graphic Cursor (GCR1)", // 0x9a
	"Graphic Cursor (GCR2)", // 0x9c
	"(Undefined)", // 0x9e
	"(Undefined)", // 0xa0
	"(Undefined)", // 0xa2
	"(Undefined)", // 0xa4
	"(Undefined)", // 0xa6
	"(Undefined)", // 0xa8
	"(Undefined)", // 0xaa
	"(Undefined)", // 0xac
	"(Undefined)", // 0xae
	"(Undefined)", // 0xb0
	"(Undefined)", // 0xb2
	"(Undefined)", // 0xb4
	"(Undefined)", // 0xb6
	"(Undefined)", // 0xb8
	"(Undefined)", // 0xba
	"(Undefined)", // 0xbc
	"(Undefined)", // 0xbe
	// upper screen
	"Raster Address 0 (RAR0)", // 0xc0
	"Memory Width 0 (MWR0)", // 0xc2
	"Start Address 0 (SAR0) 0xc4", // 0xc4
	"Start Address 0 (SAR0) 0xc6", // 0xc6
	// base screen
	"Raster Address 1 (RAR1)", // 0xc8
	"Memory Width 1 (MWR1)", // 0xca
	"Start Address 1 (SAR1) 0xcc", // 0xcc
	"Start Address 1 (SAR1) 0xce", // 0xce
	// lower screen
	"Raster Address 2 (RAR2)", // 0xd0
	"Memory Width 2 (MWR2)", // 0xd2
	"Start Address 2 (SAR2) 0xd4", // 0xd4
	"Start Address 2 (SAR2) 0xd6", // 0xd6
	// window screen
	"Raster Address 3 (RAR3)", // 0xd8
	"Memory Width 3 (MWR3)", // 0xda
	"Start Address 3 (SAR3) 0xdc", // 0xdc
	"Start Address 3 (SAR3) 0xde", // 0xde
	// block cursor 1
	"Block Cursor 1 (BCUR1) 0xe0",
	"Block Cursor 1 (BCUR1) 0xe2",
	// block cursor 2
	"Block Cursor 2 (BCUR2) 0xe4",
	"Block Cursor 2 (BCUR2) 0xe6",
	"Cursor Definition (CDR)",
	"Zoom Factor (ZFR)",
	"Lightpen Address (LPAR) 0xec",
	"Lightpen Address (LPAR) 0xee",
	"(Undefined)", // 0xf0
	"(Undefined)", // 0xf2
	"(Undefined)", // 0xf4
	"(Undefined)", // 0xf6
	"(Undefined)", // 0xf8
	"(Undefined)", // 0xfa
	"(Undefined)", // 0xfc
	"(Undefined)"  // 0xfe
};

static const char *const wpr_regnames[0x20] =
{
	"Color 0 (CL0)",
	"Color 1 (CL1)",
	"Color Comparison (COMP)",
	"Edge Color (EDG)",
	"Mask (MASK)",
	"Pattern RAM Control (PRC) 0x05",
	"Pattern RAM Control (PRC) 0x06",
	"Pattern RAM Control (PRC) 0x07",
	"Area Definition (ADR) 0x08",
	"Area Definition (ADR) 0x09",
	"Area Definition (ADR) 0x0a",
	"Area Definition (ADR) 0x0b",
	"Read Write Pointer (RWP) H",
	"Read Write Pointer (RWP) L",
	"(Undefined)",
	"(Undefined)",
	"Drawing Pointer (DP) H",
	"Drawing Pointer (DP) L",
	"Current Pointer (CP) H",
	"Current Pointer (CP) L",
	"(Undefined)", // 0x14
	"(Undefined)",
	"(Undefined)", // 0x16
	"(Undefined)",
	"(Undefined)", // 0x18
	"(Undefined)",
	"(Undefined)", // 0x1a
	"(Undefined)",
	"(Undefined)", // 0x1c
	"(Undefined)",
	"(Undefined)", // 0x1e
	"(Undefined)"
};

enum
{
	COMMAND_INVALID = -1,
	COMMAND_ORG,
	COMMAND_WPR,
	COMMAND_RPR,
	COMMAND_WPTN,
	COMMAND_RPTN,
	COMMAND_DRD,
	COMMAND_DWT,
	COMMAND_DMOD,
	COMMAND_RD,
	COMMAND_WT,
	COMMAND_MOD,
	COMMAND_CLR,
	COMMAND_SCLR,
	COMMAND_CPY,
	COMMAND_SCPY,
	COMMAND_AMOVE,
	COMMAND_RMOVE,
	COMMAND_ALINE,
	COMMAND_RLINE,
	COMMAND_ARCT,
	COMMAND_RRCT,
	COMMAND_APLL,
	COMMAND_RPLL,
	COMMAND_APLG,
	COMMAND_RPLG,
	COMMAND_CRCL,
	COMMAND_ELPS,
	COMMAND_AARC,
	COMMAND_RARC,
	COMMAND_AEARC,
	COMMAND_REARC,
	COMMAND_AFRCT,
	COMMAND_RFRCT,
	COMMAND_PAINT,
	COMMAND_DOT,
	COMMAND_PTN,
	COMMAND_AGCPY,
	COMMAND_RGCPY
};

#define H63484_COMMAND_ORG      0x0400  //              p: 2
#define H63484_COMMAND_WPR      0x0800  // & ~0x1f      p: 1
#define H63484_COMMAND_RPR      0x0c00  // & ~0x1f      p: 0
#define H63484_COMMAND_WPTN     0x1800  // & ~0xf       p: 1 + n
#define H63484_COMMAND_RPTN     0x1c00  // & ~0xf       p: 1
#define H63484_COMMAND_DRD      0x2400  //              p: 2
#define H63484_COMMAND_DWT      0x2800  //              p: 2
#define H63484_COMMAND_DMOD     0x2c00  // & ~3         p: 2
#define H63484_COMMAND_RD       0x4400  //              p: 0
#define H63484_COMMAND_WT       0x4800  //              p: 1
#define H63484_COMMAND_MOD      0x4c00  // & ~3         p: 1
#define H63484_COMMAND_CLR      0x5800  //              p: 3
#define H63484_COMMAND_SCLR     0x5c00  // & ~3         p: 3
#define H63484_COMMAND_CPY      0x6000  // & ~0x0f03    p: 4
#define H63484_COMMAND_SCPY     0x7000  // & ~0x0f03    p: 4
#define H63484_COMMAND_AMOVE    0x8000  //              p: 2
#define H63484_COMMAND_RMOVE    0x8400  //              p: 2
#define H63484_COMMAND_ALINE    0x8800  // & ~0x00ff    p: 2
#define H63484_COMMAND_RLINE    0x8c00  // & ~0x00ff    p: 2
#define H63484_COMMAND_ARCT     0x9000  // & ~0x00ff    p: 2
#define H63484_COMMAND_RRCT     0x9400  // & ~0x00ff    p: 2
#define H63484_COMMAND_APLL     0x9800  // & ~0x00ff    p: 1 + n
#define H63484_COMMAND_RPLL     0x9c00  // & ~0x00ff    p: 1 + n
#define H63484_COMMAND_APLG     0xa000  // & ~0x00ff    p: 1 + n
#define H63484_COMMAND_RPLG     0xa400  // & ~0x00ff    p: 1 + n
#define H63484_COMMAND_CRCL     0xa800  // & ~0x01ff    p: 1
#define H63484_COMMAND_ELPS     0xac00  // & ~0x01ff    p: 3
#define H63484_COMMAND_AARC     0xb000  // & ~0x01ff    p: 4
#define H63484_COMMAND_RARC     0xb400  // & ~0x01ff    p: 4
#define H63484_COMMAND_AEARC    0xb800  // & ~0x01ff    p: 6
#define H63484_COMMAND_REARC    0xbc00  // & ~0x01ff    p: 6
#define H63484_COMMAND_AFRCT    0xc000  // & ~0x00ff    p: 2
#define H63484_COMMAND_RFRCT    0xc400  // & ~0x00ff    p: 2
#define H63484_COMMAND_PAINT    0xc800  // & ~0x01ff    p: 0
#define H63484_COMMAND_DOT      0xcc00  // & ~0x00ff    p: 0
#define H63484_COMMAND_PTN      0xd000  // & ~0x0fff    p: 1
#define H63484_COMMAND_AGCPY    0xe000  // & ~0x0fff    p: 4
#define H63484_COMMAND_RGCPY    0xf000  // & ~0x0fff    p: 4


/*-------------------------------------------------
    ROM( h63484 )
-------------------------------------------------*/

// devices
const device_type H63484 = &device_creator<h63484_device>;


ROM_START( h63484 )
	ROM_REGION( 0x100, "h63484", 0 )
	ROM_LOAD( "h63484.bin", 0x000, 0x100, NO_DUMP ) /* internal control ROM */
ROM_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *h63484_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *h63484_device::device_rom_region() const
{
	return ROM_NAME( h63484 );
}

//-------------------------------------------------
//  readword - read a word at the given address
//-------------------------------------------------

inline UINT16 h63484_device::readword(offs_t address)
{
	return space().read_word(address << 1);
}


//-------------------------------------------------
//  writeword - write a word at the given address
//-------------------------------------------------

inline void h63484_device::writeword(offs_t address, UINT16 data)
{
	space().write_word(address << 1, data);
}


inline void h63484_device::inc_ar(int value)
{
	if(m_ar & 0x80)
	{
		if (m_ar + value > 0xff)    // TODO: what happens if it overflows?
			logerror("HD63484 '%s': Address Register overflows 0x%02x\n", tag(), m_ar + value);

		m_ar = (m_ar + value) & 0xff;
	}
}

inline void h63484_device::fifo_w_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo[i] = 0;

	m_fifo_ptr = -1;

	m_sr |= H63484_SR_WFR;
	m_sr |= H63484_SR_WFE;
}

inline void h63484_device::queue_w(UINT8 data)
{
	if (m_fifo_ptr < 15)
	{
		m_fifo_ptr++;

		m_fifo[m_fifo_ptr] = data;

		if (m_fifo_ptr == 16)
			m_sr &= ~H63484_SR_WFR;

		m_sr &= ~H63484_SR_WFE;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

inline void h63484_device::dequeue_w(UINT8 *data)
{
	int i;

	*data = m_fifo[0];

	if (m_fifo_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo[i] = m_fifo[i + 1];

		m_fifo[15] = 0;

		m_fifo_ptr--;

		m_sr |= H63484_SR_WFR;

		if (m_fifo_ptr == -1)
			m_sr |= H63484_SR_WFE;

	}
}

inline void h63484_device::fifo_r_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo_r[i] = 0;

	m_fifo_r_ptr = -1;

	m_sr &= ~H63484_SR_RFR;
	m_sr &= ~H63484_SR_RFF;
}

inline void h63484_device::queue_r(UINT8 data)
{
	if (m_fifo_r_ptr < 15)
	{
		m_fifo_r_ptr++;

		m_fifo_r[m_fifo_r_ptr] = data;

		if (m_fifo_r_ptr == 16)
			m_sr |= H63484_SR_RFF;

		m_sr |= H63484_SR_RFR;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		printf("FIFO?\n");
	}
}

inline void h63484_device::dequeue_r(UINT8 *data)
{
	int i;

	*data = m_fifo_r[0];

	if (m_fifo_r_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo_r[i] = m_fifo_r[i + 1];

		m_fifo_r[15] = 0;

		m_fifo_r_ptr--;

		m_sr &= ~H63484_SR_RFF;

		if (m_fifo_r_ptr == -1)
			m_sr &= ~H63484_SR_RFR;
	}
}

//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

inline void h63484_device::recompute_parameters()
{
	if(m_hdw < 3 || m_hc == 0 || m_vc == 0) //bail out if screen params aren't valid
		return;

	if (LOG)
	{
		printf("HC %d HSW %d HDS %d HDW %d HWS %d HWW %d\n",m_hc,m_hsw,m_hds,m_hdw,m_hws,m_hww);
		printf("VC %d VDS %d VSW %d VWS %d VWW %d\n",m_vc,m_vds,m_vsw,m_vws,m_vww);
		printf("SP0 %d SP1 %d SP2 %d\n",m_sp[0],m_sp[1],m_sp[2]);
	}

	int gai = (m_omr>>4) & 0x07;
	if (gai > 3)    printf("unsupported GAI=%d\n", gai);
	int acm = (m_omr & 0x08) ? 2 : 1;
	int ppw = 16 / get_bpp();
	int ppmc = ppw * (1 << gai) / acm;  // TODO: GAI > 3
	int vbstart = m_vds + m_sp[1];
	if (BIT(m_dcr, 13)) vbstart += m_sp[0];
	if (BIT(m_dcr, 11)) vbstart += m_sp[2];

	rectangle visarea = m_screen->visible_area();
	visarea.set((m_hsw + m_hds) * ppmc, (m_hsw + m_hds + m_hdw) * ppmc - 1, m_vds, vbstart - 1);
	attoseconds_t frame_period = m_screen->frame_period().attoseconds(); // TODO: use clock() to calculate the frame_period
	m_screen->configure(m_hc * ppmc, m_vc, visarea, frame_period);
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

int h63484_device::translate_command(UINT16 data)
{
	/* annoying switch-case sequence, but it's the only way to get invalid commands ... */
	switch (data)
	{
		case H63484_COMMAND_ORG:    return COMMAND_ORG;
		case H63484_COMMAND_DRD:    return COMMAND_DRD;
		case H63484_COMMAND_DWT:    return COMMAND_DWT;
		case H63484_COMMAND_RD:     return COMMAND_RD;
		case H63484_COMMAND_WT:     return COMMAND_WT;
		case H63484_COMMAND_CLR:    return COMMAND_CLR;
		case H63484_COMMAND_AMOVE:  return COMMAND_AMOVE;
		case H63484_COMMAND_RMOVE:  return COMMAND_RMOVE;
	}

	switch(data & ~0x3)
	{
		case H63484_COMMAND_DMOD:   return COMMAND_DMOD;
		case H63484_COMMAND_MOD:    return COMMAND_MOD;
		case H63484_COMMAND_SCLR:   return COMMAND_SCLR;
	}

	switch(data & ~0xf)
	{
		case H63484_COMMAND_WPTN:   return COMMAND_WPTN;
		case H63484_COMMAND_RPTN:   return COMMAND_RPTN;
	}

	switch(data & ~0x1f)
	{
		case H63484_COMMAND_WPR:    return COMMAND_WPR;
		case H63484_COMMAND_RPR:    return COMMAND_RPR;
	}

	switch(data & ~0x0f03)
	{
		case H63484_COMMAND_CPY:    return COMMAND_CPY;
		case H63484_COMMAND_SCPY:   return COMMAND_SCPY;
	}

	switch(data & ~0x00ff)
	{
		case H63484_COMMAND_ALINE:  return COMMAND_ALINE;
		case H63484_COMMAND_RLINE:  return COMMAND_RLINE;
		case H63484_COMMAND_ARCT:   return COMMAND_ARCT;
		case H63484_COMMAND_RRCT:   return COMMAND_RRCT;
		case H63484_COMMAND_APLL:   return COMMAND_APLL;
		case H63484_COMMAND_RPLL:   return COMMAND_RPLL;
		case H63484_COMMAND_APLG:   return COMMAND_APLG;
		case H63484_COMMAND_RPLG:   return COMMAND_RPLG;
		case H63484_COMMAND_AFRCT:  return COMMAND_AFRCT;
		case H63484_COMMAND_RFRCT:  return COMMAND_RFRCT;
		case H63484_COMMAND_DOT:    return COMMAND_DOT;
	}

	switch(data & ~0x01ff)
	{
		case H63484_COMMAND_CRCL:   return COMMAND_CRCL;
		case H63484_COMMAND_ELPS:   return COMMAND_ELPS;
		case H63484_COMMAND_AARC:   return COMMAND_AARC;
		case H63484_COMMAND_RARC:   return COMMAND_RARC;
		case H63484_COMMAND_AEARC:  return COMMAND_AEARC;
		case H63484_COMMAND_REARC:  return COMMAND_REARC;
		case H63484_COMMAND_PAINT:  return COMMAND_PAINT;
	}

	switch(data & ~0x0fff)
	{
		case H63484_COMMAND_PTN:    return COMMAND_PTN;
		case H63484_COMMAND_AGCPY:  return COMMAND_AGCPY;
		case H63484_COMMAND_RGCPY:  return COMMAND_RGCPY;
	}

	return COMMAND_INVALID;
}

inline void h63484_device::command_end_seq()
{
	//h63484->param_ptr = 0;
	m_sr |= H63484_SR_CED;

	/* TODO: we might need to be more aggressive and clear the params in there */
}

int h63484_device::get_bpp()
{
	int gbm = (m_ccr >> 8) & 0x07;

	if (gbm <= 4)
		return 1 << gbm;

	//logerror ("Invalid Graphic Bit Mode (%d)\n", gbm);
	return 1;
}

void h63484_device::calc_offset(INT16 x, INT16 y, UINT32 &offset, UINT8 &bit_pos)
{
	int bpp = get_bpp();
	int ppw = 16 / bpp;
	int gbm = (m_ccr >> 8) & 0x07;
	x += (m_org_dpd >> gbm);
	if (x >= 0)
	{
		offset = x / ppw;
		bit_pos = x % ppw;
	}
	else
	{
		offset = x / ppw;
		bit_pos= (-x) % ppw;
		if (bit_pos != 0)
		{
			offset--;
			bit_pos = ppw - bit_pos;
		}
	}

	offset += m_org_dpa - y * m_mwr[m_org_dn];
	bit_pos *= bpp;
}

UINT16 h63484_device::get_dot(INT16 x, INT16 y)
{
	UINT8 bpp = get_bpp();
	UINT32 offset = 0;
	UINT8 bit_pos = 0;

	calc_offset(x, y, offset, bit_pos);

	return (readword(offset) >> bit_pos) & ((1 << bpp) - 1);
}

bool h63484_device::set_dot(INT16 x, INT16 y, INT16 px, INT16 py)
{
	int xs = m_pex - m_psx + 1;
	int ys = m_pey - m_psy + 1;
	int zx = m_pzx + 1;
	int zy = m_pzy + 1;
	int xp = m_psx + ((px % (xs * zx)) / zx);
	int yp = m_psy + ((py % (ys * zy)) / zy);

	if (xp < m_psx)
		xp = (m_pex + 1) - (m_psx - xp);

	if (yp < m_psy)
		yp = (m_pey + 1) - (m_psy - yp);

	int pix = (m_pram[yp & 0x0f] >> (xp & 0x0f)) & 0x01;

	UINT8 col = (m_cr >> 3) & 0x03;
	UINT8 bpp = get_bpp();
	UINT16 mask = (1 << bpp) - 1;
	UINT16 xmask = (16 / bpp) - 1;
	UINT16 cl0 = (m_cl0 >> ((x & xmask) * bpp)) & mask;
	UINT16 cl1 = (m_cl1 >> ((x & xmask) * bpp)) & mask;

	switch (col)
	{
		case 0x00:
			return set_dot(x, y, pix ? cl1 : cl0);
		case 0x01:
			if (pix)
				return set_dot(x, y, cl1);
			break;
		case 0x02:
			if (!pix)
				return set_dot(x, y, cl0);
			break;
		case 0x03:
			fatalerror("HD63484 color modes (Pattern RAM indirect)\n");
			// TODO
	}

	return false;
}

bool h63484_device::set_dot(INT16 x, INT16 y, UINT16 color)
{
	UINT8 bpp = get_bpp();
	UINT32 offset = 0;
	UINT8 bit_pos = 0;
	UINT8 opm = m_cr & 0x07;
	UINT8 area = (m_cr >> 5) & 0x07;

	calc_offset(x, y, offset, bit_pos);

	UINT16 mask = ((1 << bpp) - 1) << bit_pos;
	UINT16 color_shifted = (color << bit_pos) & mask;

	UINT16 data = readword(offset);
	UINT16 res = data;

	switch (opm)
	{
		case 0:
			res = (data & ~mask) | color_shifted;
			break;
		case 1:
			res = (data & ~mask) | ((data & mask) | color_shifted);
			break;
		case 2:
			res = (data & ~mask) | ((data & mask) & color_shifted);
			break;
		case 3:
			res = (data & ~mask) | ((data & mask) ^ color_shifted);
			break;
		case 4:
			if (get_dot(x, y) == ((m_ccmp & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 5:
			if (get_dot(x, y) != ((m_ccmp & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 6:
			if (get_dot(x, y) < ((m_cl0 & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 7:
			if (get_dot(x, y) > ((m_cl1 & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
	}

	writeword(offset, res);

	if (area)
		logerror("HD63484 '%s': unsupported area detection %x (%d %d)\n", tag(), area, x, y);

	return false;   // TODO: return area detection status
}

void h63484_device::draw_line(INT16 sx, INT16 sy, INT16 ex, INT16 ey)
{
	UINT16 delta_x = abs(ex - sx) * 2;
	UINT16 delta_y = abs(ey - sy) * 2;
	int dir_x = (ex < sx) ? -1 : ((ex > sx) ? +1 : 0);
	int dir_y = (ey < sy) ? -1 : ((ey > sy) ? +1 : 0);
	int pram_pos = 0;

	if(delta_x > delta_y)
	{
		int delta = delta_y - delta_x / 2;
		while(sx != ex)
		{
			set_dot(sx, sy, pram_pos, 0);

			if(delta >= 0)
			{
				sy += dir_y;
				delta -= delta_x;
			}
			pram_pos++;
			sx += dir_x;
			delta += delta_y;
		}
	}
	else
	{
		int delta = delta_x - delta_y / 2;
		while(sy != ey)
		{
			set_dot(sx, sy, pram_pos, 0);

			if(delta >= 0)
			{
				sx += dir_x;
				delta -= delta_y;
			}
			pram_pos++;
			sy += dir_y;
			delta += delta_x;
		}
	}
}

void h63484_device::draw_ellipse(INT16 cx, INT16 cy, double dx, double dy, double s_angol, double e_angol, bool c)
{
	double inc = 1.0 / (MAX(dx, dy) * 100);
	for (double angol = s_angol; fabs(angol - e_angol) >= inc*2; angol += inc * (c ? -1 : +1))
	{
		if (angol > DEGREE_TO_RADIAN(360))    angol -= DEGREE_TO_RADIAN(360);
		if (angol < DEGREE_TO_RADIAN(0))      angol += DEGREE_TO_RADIAN(360);

		double px = cos(angol) * dx;
		double py = sin(angol) * dy;
		set_dot(cx + round(px), cy + round(py), 0, 0);
	}
}

void h63484_device::paint(INT16 sx, INT16 sy)
{
/*
    This is not accurate since real hardware can only paint 4 'unpaintable' areas,
    the other 'unpaintable' points are pushed into the read FIFO to be processed
    later by the program, but currently is impossible suspend/resume the command
    in case the read FIFO is full, so all 'unpaintable' areas are painted.
    Also CP is not in the correct position after this command.
*/
	UINT8 e = BIT(m_cr, 8);
	UINT8 bpp = get_bpp();
	UINT16 mask = (1 << bpp) - 1;
	UINT16 xmask = (16 / bpp) - 1;

	for (int ydir=0; ydir<2; ydir++)
		for(UINT16 y=0;y<0x7fff; y++)
		{
			bool limit = true;
			bool unpaintable_up = false;
			bool unpaintable_dn = false;

			for (int xdir=0; xdir<2; xdir++)
				for(UINT16 x=0; x<0x7fff; x++)
				{
					INT16 px = sx + (xdir ? -x : x);
					INT16 py = sy + (ydir ? -y : y);

					UINT16 dot = get_dot(px, py);
					UINT16 edg = (m_edg >> (px & xmask) * bpp) & mask;
					UINT16 cl0 = (m_cl0 >> (px & xmask) * bpp) & mask;
					UINT16 cl1 = (m_cl1 >> (px & xmask) * bpp) & mask;

					if ((e && dot != edg) || (!e && dot == edg) || dot == cl0 || dot == cl1)
						break;

					if ((!ydir && !xdir && x && y) || (xdir && y) || (ydir && x) || (ydir && xdir))
						set_dot(px, py, px - m_cpx, py - m_cpy);

					dot = get_dot(px, py + 1);
					if (unpaintable_up && !((e && dot == edg) && (!e && dot != edg) && dot != cl0 && dot != cl1))
						paint(px, py + 1);
					else if (!unpaintable_up && ((e && dot != edg) || (!e && dot == edg) || dot == cl0 || dot == cl1))
						unpaintable_up = true;

					dot = get_dot(px, py - 1);
					if (unpaintable_dn && !((e && dot == edg) && (!e && dot != edg) && dot != cl0 && dot != cl1))
						paint(px, py - 1);
					else if (!unpaintable_dn && ((e && dot != edg) || (!e && dot == edg) || dot == cl0 || dot == cl1))
						unpaintable_dn = true;

					limit = false;
				}
				if (limit)     break;
			}
}

UINT16 h63484_device::command_rpr_exec()
{
	switch(m_cr & 0x1f)
	{
		case 0x00: // color 0
			return m_cl0;
		case 0x01: // color 1
			return m_cl1;
		case 0x02: // color comparison
			return m_ccmp;
		case 0x03: // edge color
			return m_edg;
		case 0x04: // mask
			return m_mask;
		case 0x05: // Pattern RAM Control 1
			return (m_ppy << 12) | (m_pzcy << 8) | (m_ppx << 4) | m_pzcx;
		case 0x06: // Pattern RAM Control 2
			return (m_psy << 12) | (m_psx << 4);
		case 0x07: // Pattern RAM Control 3
			return (m_pey << 12) | (m_pzy << 8) | (m_pex << 4) | m_pzx;
		case 0x08: // Area Definition XMIN
			return m_xmin;
		case 0x09: // Area Definition YMIN
			return m_ymin;
		case 0x0a: // Area Definition XMAX
			return m_xmax;
		case 0x0b: // Area Definition YMAX
			return m_ymax;
		case 0x0c: // Read Write Pointer H
			return (m_rwp_dn << 14) | ((m_rwp[m_rwp_dn] >> 12) & 0xff);
		case 0x0d: // Read Write Pointer L
			return (m_rwp[m_rwp_dn] & 0x0fff) << 4;
		default:
			if(LOG) printf("Read %sx\n", wpr_regnames[m_cr & 0x1f]);
			return 0;
	}
}

void h63484_device::command_wpr_exec()
{
	switch(m_cr & 0x1f)
	{
		case 0x00: // color 0
			m_cl0 = m_pr[0];
			break;
		case 0x01: // color 1
			m_cl1 = m_pr[0];
			break;
		case 0x02: // color comparison
			m_ccmp = m_pr[0];
			break;
		case 0x03: // edge color
			m_edg = m_pr[0];
			break;
		case 0x04: // mask
			m_mask = m_pr[0];
			break;
		case 0x05: // Pattern RAM Control 1
			m_pzcx = (m_pr[0] >> 0) & 0x0f;
			m_ppx = (m_pr[0] >> 4) & 0x0f;
			m_pzcy = (m_pr[0] >> 8) & 0x0f;
			m_ppy = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x06: // Pattern RAM Control 2
			m_psx = (m_pr[0] >> 4) & 0x0f;
			m_psy = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x07: // Pattern RAM Control 3
			m_pzx = (m_pr[0] >> 0) & 0x0f;
			m_pex = (m_pr[0] >> 4) & 0x0f;
			m_pzy = (m_pr[0] >> 8) & 0x0f;
			m_pey = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x08: // Area Definition XMIN
			m_xmin = m_pr[0];
			break;
		case 0x09: // Area Definition YMIN
			m_ymin = m_pr[0];
			break;
		case 0x0a: // Area Definition XMAX
			m_xmax = m_pr[0];
			break;
		case 0x0b: // Area Definition YMAX
			m_ymax = m_pr[0];
			break;
		case 0x0c: // Read Write Pointer H
			m_rwp_dn = (m_pr[0] & 0xc000) >> 14;
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0x00fff) | ((m_pr[0] & 0x00ff) << 12);
			break;
		case 0x0d: // Read Write Pointer L
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0xff000) | ((m_pr[0] & 0xfff0) >> 4);
			break;
		default:
			if(LOG) printf("%s -> %02x\n",wpr_regnames[m_cr & 0x1f],m_pr[0]);
			break;
	}
}

void h63484_device::command_clr_exec()
{
	UINT8 mm = m_cr & 0x03;
	UINT16 d = m_pr[0];
	INT16 ax = (INT16)m_pr[1];
	INT16 ay = (INT16)m_pr[2];

	int d0_inc = (ax < 0) ? -1 : 1;
	int d1_inc = (ay < 0) ? -1 : 1;

	for(INT16 d1=0; d1!=ay+d1_inc; d1+=d1_inc)
	{
		for(INT16 d0=0; d0!=ax+d0_inc; d0+=d0_inc)
		{
			UINT32 offset = m_rwp[m_rwp_dn] - d1 * m_mwr[m_rwp_dn] + d0;
			UINT16 data = readword(offset);
			UINT16 res = 0;

			if (BIT(m_cr, 10))
			{
				switch(mm)
				{
					case 0: // replace
						res = (data & ~m_mask) | (d & m_mask);
						break;
					case 1: // OR
						res = (data & ~m_mask) | ((data | d) & m_mask);
						break;
					case 2: // AND
						res = (data & ~m_mask) | ((data & d) & m_mask);
						break;
					case 3: // EOR
						res = (data & ~m_mask) | ((data ^ d) & m_mask);
						break;
				}
			}
			else
				res = d;

			writeword(offset, res);
		}
	}

	m_rwp[m_rwp_dn] -= (ay + d1_inc) * m_mwr[m_rwp_dn];
	m_rwp[m_rwp_dn] &= 0xfffff;
}

void h63484_device::command_cpy_exec()
{
	UINT8 mm = m_cr & 0x03;
	UINT8 dsd = (m_cr >> 8) & 0x07;
	UINT8 s = BIT(m_cr, 11);
	UINT32 SA = ((m_pr[0] & 0xff) << 12) | ((m_pr[1]&0xfff0) >> 4);
	INT16 DX = (INT16)m_pr[s ? 3 : 2];
	INT16 DY = (INT16)m_pr[s ? 2 : 3];

	int sd0_inc = (DX < 0) ? -1 : 1;
	int sd1_inc = (DY < 0) ? -1 : 1;
	int dd0_inc, dd1_inc;
	if (dsd & 0x04)
	{
		dd0_inc = dsd & 0x01 ? -1 : +1;
		dd1_inc = dsd & 0x02 ? -1 : +1;
	}
	else
	{
		dd0_inc = dsd & 0x02 ? -1 : +1;
		dd1_inc = dsd & 0x01 ? -1 : +1;
	}

	for(INT16 sd1=0, dd1=0; sd1!=DY+sd1_inc; sd1+=sd1_inc, dd1+=dd1_inc)
	{
		for(INT16 sd0=0, dd0=0; sd0!=DX+sd0_inc; sd0+=sd0_inc, dd0+=dd0_inc)
		{
			UINT32 src_offset, dst_offset;

			if (s)
				src_offset = SA + sd1 - sd0 * m_mwr[m_rwp_dn];
			else
				src_offset = SA + sd0 - sd1 * m_mwr[m_rwp_dn];

			if (BIT(dsd, 2))
				dst_offset = m_rwp[m_rwp_dn] + dd1 - dd0 * m_mwr[m_rwp_dn];
			else
				dst_offset = m_rwp[m_rwp_dn] + dd0 - dd1 * m_mwr[m_rwp_dn];

			UINT16 src_data = readword(src_offset);
			UINT16 dst_data = readword(dst_offset);

			if (BIT(m_cr, 12))
			{
				switch(mm)
				{
					case 0: // replace
						dst_data = (dst_data & ~m_mask) | (src_data & m_mask);
						break;
					case 1: // OR
						dst_data = (dst_data & ~m_mask) | ((dst_data | src_data) & m_mask);
						break;
					case 2: // AND
						dst_data = (dst_data & ~m_mask) | ((dst_data & src_data) & m_mask);
						break;
					case 3: // EOR
						dst_data = (dst_data & ~m_mask) | ((dst_data ^ src_data) & m_mask);
						break;
				}
			}
			else
				dst_data = src_data;

			writeword(dst_offset, dst_data);
		}
	}

	m_rwp[m_rwp_dn] += dsd & 0x04 ? (DY + dd1_inc) : (-(DY + dd1_inc) * m_mwr[m_rwp_dn]);
	m_rwp[m_rwp_dn] &= 0xfffff;
}

void h63484_device::command_line_exec()
{
	INT16 x = (INT16)m_pr[0];
	INT16 y = (INT16)m_pr[1];

	if (BIT(m_cr, 10))
	{
		x += m_cpx;
		y += m_cpy;
	}

	draw_line(m_cpx, m_cpy, x, y);

	m_cpx = x;
	m_cpy = y;
}

void h63484_device::command_rct_exec()
{
	INT16 dX = m_pr[0];
	INT16 dY = m_pr[1];

	if (BIT(m_cr, 10))  // relative (RRCT)
	{
		dX += m_cpx;
		dY += m_cpy;
	}

	/*
	3<-2
	|  ^
	v  |
	0->1
	*/

	/* 0 -> 1 */
	draw_line(m_cpx, m_cpy, dX, m_cpy);

	/* 1 -> 2 */
	draw_line(dX, m_cpy, dX, dY);

	/* 2 -> 3 */
	draw_line(dX, dY, m_cpx, dY);

	/* 3 -> 4 */
	draw_line(m_cpx, dY, m_cpx, m_cpy);
}

void h63484_device::command_gcpy_exec()
{
	UINT8 dsd = (m_cr >> 8) & 0x07;
	UINT8 s = BIT(m_cr, 11);
	INT16 Xs = (INT16)m_pr[0];
	INT16 Ys = (INT16)m_pr[1];
	INT16 DX = (INT16)m_pr[s ? 3 : 2];
	INT16 DY = (INT16)m_pr[s ? 2 : 3];

	if (BIT(m_cr, 12))  // relative (RGCPY)
	{
		Xs += m_cpx;
		Ys += m_cpy;
	}

	int sd0_inc = (DX < 0) ? -1 : 1;
	int sd1_inc = (DY < 0) ? -1 : 1;
	int dd0_inc, dd1_inc;
	if (dsd & 0x04)
	{
		dd0_inc = dsd & 0x01 ? -1 : +1;
		dd1_inc = dsd & 0x02 ? -1 : +1;
	}
	else
	{
		dd0_inc = dsd & 0x02 ? -1 : +1;
		dd1_inc = dsd & 0x01 ? -1 : +1;
	}

	for(INT16 sd1=0, dd1=0; sd1!=DY+sd1_inc; sd1+=sd1_inc, dd1+=dd1_inc)
	{
		for(INT16 sd0=0, dd0=0; sd0!=DX+sd0_inc; sd0+=sd0_inc, dd0+=dd0_inc)
		{
			UINT16 color;
			if (s)
				color = get_dot(Xs + sd1, Ys + sd0);
			else
				color = get_dot(Xs + sd0, Ys + sd1);

			if (BIT(dsd, 2))
				set_dot(m_cpx + dd1, m_cpy + dd0, color);
			else
				set_dot(m_cpx + dd0, m_cpy + dd1, color);
		}
	}

	if (dsd & 0x04)
		m_cpx += DY + dd1_inc;
	else
		m_cpy += DY + dd1_inc;

}

void h63484_device::command_frct_exec()
{
	INT16 X = (INT16)m_pr[0];
	INT16 Y = (INT16)m_pr[1];

	if (!BIT(m_cr, 10))
	{
		X -= m_cpx;
		Y -= m_cpy;
	}

	int d0_inc = (X < 0) ? -1 : 1;
	int d1_inc = (Y < 0) ? -1 : 1;

	for(INT16 d1=0; d1!=Y+d1_inc; d1+=d1_inc)
	{
		for(INT16 d0=0; d0!=X+d0_inc; d0+=d0_inc)
			set_dot(m_cpx + d0, m_cpy + d1, d0, d1);
	}

	m_cpy += (Y + d1_inc);
}

void h63484_device::command_ptn_exec()
{
	INT16 szx = ((m_pr[0] >> 0) & 0xff);
	INT16 szy = ((m_pr[0] >> 8) & 0xff);
	UINT8 sl_sd = (m_cr >> 8) & 0x0f;
	INT16 px = 0;
	INT16 py = 0;

	for(INT16 d1=0; d1!=szy+1; d1++)
	{
		switch (sl_sd)
		{
			case 0x00: px = 0;    py = d1;   break;
			case 0x01: px = -d1;  py = d1;   break;
			case 0x02: py = 0;    px = -d1;  break;
			case 0x03: px = -d1;  py = -d1;  break;
			case 0x04: px = 0;    py = -d1;  break;
			case 0x05: px = d1;   py = -d1;  break;
			case 0x06: py = 0;    px = d1;   break;
			case 0x07: px = d1;   py = d1;   break;
			case 0x08: px = d1;   py = d1;   break;
			case 0x09: px = 0;    py = d1;   break;
			case 0x0a: px = -d1;  py = d1;   break;
			case 0x0b: px = -d1;  py = 0;    break;
			case 0x0c: px = -d1;  py = -d1;  break;
			case 0x0d: px = 0;    py = -d1;  break;
			case 0x0e: px = +d1;  py = -d1;  break;
			case 0x0f: px = +d1;  py = 0;    break;
		}

		for(INT16 d0=0; d0!=szx+1; d0++)
		{
			set_dot(m_cpx + px, m_cpy + py, d0, d1);

			switch (sl_sd)
			{
				case 0x00: px++;        break;
				case 0x01: px++; py++;  break;
				case 0x02: py++;        break;
				case 0x03: px--; py++;  break;
				case 0x04: px--;        break;
				case 0x05: px--; py--;  break;
				case 0x06: py--;        break;
				case 0x07: px++; py--;  break;
				case 0x08: px++;        break;
				case 0x09: px++; py++;  break;
				case 0x0a: py++;        break;
				case 0x0b: px--; py++;  break;
				case 0x0c: px--;        break;
				case 0x0d: px--; py--;  break;
				case 0x0e: py--;        break;
				case 0x0f: px++; py--;  break;
			}
		}
	}

	switch (sl_sd)
	{
		case 0x00: m_cpy += (szy + 1); break;
		case 0x01: m_cpx -= (szy + 1); m_cpy += (szy + 1); break;
		case 0x02: m_cpx -= (szy + 1); break;
		case 0x03: m_cpx -= (szy + 1); m_cpy -= (szy + 1); break;
		case 0x04: m_cpy -= (szy + 1); break;
		case 0x05: m_cpx += (szy + 1); m_cpy -= (szy + 1); break;
		case 0x06: m_cpx += (szy + 1); break;
		case 0x07: m_cpx += (szy + 1); m_cpy += (szy + 1); break;
		case 0x08: m_cpx += (szy + 1); m_cpy += (szy + 1); break;
		case 0x09: m_cpy += (szy + 1); break;
		case 0x0a: m_cpx -= (szy + 1); m_cpy += (szy + 1); break;
		case 0x0b: m_cpx -= (szy + 1); break;
		case 0x0c: m_cpx -= (szy + 1); m_cpy -= (szy + 1); break;
		case 0x0d: m_cpy -= (szy + 1); break;
		case 0x0e: m_cpx += (szy + 1); m_cpy -= (szy + 1); break;
		case 0x0f: m_cpx += (szy + 1); break;
	}
}

void h63484_device::command_plg_exec()
{
	int sx = m_cpx;
	int sy = m_cpy;
	int ex=0;
	int ey=0;

	for (int i = 0; i < m_dn; i++)
	{
		if (BIT(m_cr, 10))
		{
			ex = sx + (INT16)m_pr[1 + i * 2];
			ey = sy + (INT16)m_pr[1 + i * 2 + 1];
		}
		else
		{
			ex = (INT16)m_pr[1 + i * 2];
			ey = (INT16)m_pr[1 + i * 2 + 1];
		}

		draw_line(sx, sy, ex, ey);

		sx = ex;
		sy = ey;
	}

	if (m_cr & 0x2000)
	{
		// APLG/RPLG
		draw_line(sx, sy, m_cpx, m_cpy);
	}
	else
	{
		// APLL/RPLL
		m_cpx = ex;
		m_cpy = ey;
	}
}

void h63484_device::command_arc_exec()
{
	INT16 xc = (INT16)m_pr[0];
	INT16 yc = (INT16)m_pr[1];
	INT16 xe = (INT16)m_pr[2];
	INT16 ye = (INT16)m_pr[3];

	if (BIT(m_cr, 10))
	{
		xc += m_cpx;
		yc += m_cpy;
		xe += m_cpx;
		ye += m_cpy;
	}

	double r = sqrt(pow((double)(xc - m_cpx), 2) + pow((double)(yc - m_cpy), 2));
	double s_angol = atan2((double)(m_cpy - yc), (double)(m_cpx - xc));
	double e_angol = atan2((double)(ye - yc), (double)(xe - xc));
	if (s_angol < 0)    s_angol += DEGREE_TO_RADIAN(360);
	if (e_angol < 0)    e_angol += DEGREE_TO_RADIAN(360);

	draw_ellipse(xc, yc, r, r, s_angol, e_angol, BIT(m_cr, 8));

	m_cpx = xe;
	m_cpy = ye;
}

void h63484_device::command_earc_exec()
{
	UINT16 a = m_pr[0];
	UINT16 b = m_pr[1];
	INT16 xc = (INT16)m_pr[2];
	INT16 yc = (INT16)m_pr[3];
	INT16 xe = (INT16)m_pr[4];
	INT16 ye = (INT16)m_pr[5];

	if (BIT(m_cr, 10))
	{
		xc += m_cpx;
		yc += m_cpy;
		xe += m_cpx;
		ye += m_cpy;
	}

	double r = sqrt(pow((double)(xc - m_cpx), 2) / a + pow((double)(yc - m_cpy), 2) / b);
	double dx = sqrt((double)a);
	double dy = sqrt((double)b);
	double s_angol = atan2((double)(m_cpy - yc) / dy, (double)(m_cpx - xc) / dx);
	double e_angol = atan2((double)(ye - yc) / dy, (double)(xe - xc) / dx);
	if (s_angol < 0)    s_angol += DEGREE_TO_RADIAN(360);
	if (e_angol < 0)    e_angol += DEGREE_TO_RADIAN(360);

	draw_ellipse(xc, yc, r * dx, r * dy, s_angol, e_angol, BIT(m_cr, 8));

	m_cpx = xe;
	m_cpy = ye;
}

void h63484_device::process_fifo()
{
	UINT8 data;

	dequeue_w(&data);

	if (m_sr & H63484_SR_CED)
	{
		m_cr = (data & 0xff) << 8;
		dequeue_w(&data);
		m_cr |= data & 0xff;
		m_param_ptr = 0;
		m_sr &= ~H63484_SR_CED;
	}
	else
	{
		m_pr[m_param_ptr] = (data & 0xff) << 8;
		dequeue_w(&data);
		m_pr[m_param_ptr] |= (data & 0xff);
		m_param_ptr++;
	}

	switch (translate_command(m_cr))
	{
		case COMMAND_INVALID:
			if (CMD_LOG)    logerror("HD63484 '%s': <invalid %04x>\n", tag(), m_cr);
			printf("H63484 '%s' Invalid Command Byte %02x\n", tag(), m_cr);
			m_sr |= H63484_SR_CER; // command error
			command_end_seq();
			break;

		case COMMAND_ORG:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': ORG 0x%04x, 0x%04x\n", tag(), m_pr[0], m_pr[1]);
				m_org_dn = (m_pr[0] & 0xc000) >> 14;
				m_org_dpa = ((m_pr[0] & 0xff) << 12) | ((m_pr[1] & 0xfff0) >> 4);
				m_org_dpd = (m_pr[1] & 0xf);
				m_cpx = m_cpy = 0;
				command_end_seq();
			}
			break;

		case COMMAND_WPR: // 0x0800 & ~0x1f
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WPR (%d) 0x%04x\n", tag(), m_cr & 0x1f, m_pr[0]);
				command_wpr_exec();
				command_end_seq();
			}
			break;

		case COMMAND_RPR:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RPR (%d)\n", tag(), m_cr & 0x1f);
				UINT16 data = command_rpr_exec();
				queue_r((data >> 8) & 0xff);
				queue_r((data >> 0) & 0xff);
				command_end_seq();
			}
			break;

		case COMMAND_WPTN:
			if(m_param_ptr == 1)
			{
				m_dn = m_pr[0]; // number of param words

				//if(m_dn > 0x10 || m_dn == 0)
				//  fatalerror("stop!\n");
			}

			if(m_param_ptr == (1 + m_dn))
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WPTN (%d) %d", tag(), m_cr & 0x0f, m_pr[0]);

				int pra = m_cr & 0xf;
				for(int i=0; i<m_dn; i++)
				{
					if (CMD_LOG)    logerror(", 0x%04x", m_pr[1 + i]);
					m_pram[(i + pra) & 0xf] = m_pr[1 + i];
				}

				if (CMD_LOG)    logerror("\n");
				command_end_seq();
			}
			break;

		case COMMAND_RPTN:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RPTN (%d) %d\n", tag(), m_cr & 0x0f, m_pr[0]);
				command_end_seq();
				fatalerror("HD63484 COMMAND_RPTN!\n");
			}
			break;

		case COMMAND_DRD:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DRD %d, %d\n", tag(), m_pr[0], m_pr[1]);
				command_end_seq();
				fatalerror("HD63484 COMMAND_DRD!\n");
			}
			break;

		case COMMAND_DWT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DWT %d, %d\n", tag(), m_pr[0], m_pr[1]);
				command_end_seq();
				fatalerror("HD63484 COMMAND_DWT!\n");
			}
			break;

		case COMMAND_DMOD:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DMOD (%d) %d, %d\n", tag(), m_cr & 0x03, m_pr[0], m_pr[1]);
				command_end_seq();
				fatalerror("HD63484 COMMAND_DMOD!\n");
			}
			break;

		case COMMAND_RD:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RD\n", tag());
				UINT16 data = readword(m_rwp[m_rwp_dn]);
				queue_r((data >> 8) & 0xff);
				queue_r((data >> 0) & 0xff);
				m_rwp[m_rwp_dn]+=1;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq();
			}
			break;

		case COMMAND_WT:
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WT 0x%04x\n", tag(), m_pr[0]);
				writeword(m_rwp[m_rwp_dn], m_pr[0]);
				m_rwp[m_rwp_dn]+=1;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq();
			}
			break;

		case COMMAND_MOD:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': MOD (%d) 0x%04x\n", tag(), m_cr & 0x03, m_pr[0]);
				UINT16 d = m_pr[0];
				UINT16 data = readword(m_rwp[m_rwp_dn]);
				UINT16 res = 0;

				switch(m_cr & 0x03)
				{
					case 0: // replace
						res = (data & ~m_mask) | (d & m_mask);
						break;
					case 1: // OR
						res = (data & ~m_mask) | ((data | d) & m_mask);
						break;
					case 2: // AND
						res = (data & ~m_mask) | ((data & d) & m_mask);
						break;
					case 3: // EOR
						res = (data & ~m_mask) | ((data ^ d) & m_mask);
						break;
				}

				writeword(m_rwp[m_rwp_dn], res);
				command_end_seq();
			}
			break;

		case COMMAND_CLR:
		case COMMAND_SCLR:
			if (m_param_ptr == 3)
			{
				if (CMD_LOG)
				{
					if (BIT(m_cr, 10))
						logerror("HD63484 '%s': SCLR (%d) 0x%04x,  %d, %d\n", tag(), m_cr & 0x03, m_pr[0], (INT16)m_pr[1], (INT16)m_pr[2]);
					else
						logerror("HD63484 '%s': CLR 0x%04x, %d, %d\n", tag(), m_pr[0], (INT16)m_pr[1], (INT16)m_pr[2]);
				}

				command_clr_exec();
				command_end_seq();
			}
			break;

		case COMMAND_CPY:
		case COMMAND_SCPY:
			if (m_param_ptr == 4)
			{
				if (CMD_LOG)
				{
					if (BIT(m_cr, 12))
						logerror("HD63484 '%s': SCPY (%d, %d, %d) 0x%x, 0x%x, %d, %d\n", tag(), BIT(m_cr, 11), (m_cr >> 8) & 0x07, m_cr & 0x07, m_pr[0] & 0xff, (m_pr[1]&0xfff0) >> 4, (INT16)m_pr[2], (INT16)m_pr[3]);
					else
						logerror("HD63484 '%s': CPY (%d, %d) 0x%x, 0x%x, %d, %d\n", tag(), BIT(m_cr, 11), (m_cr >> 8) & 0x07, m_pr[0] & 0xff, (m_pr[1]&0xfff0) >> 4, (INT16)m_pr[2], (INT16)m_pr[3]);
				}

				command_cpy_exec();
				command_end_seq();
			}
			break;

		case COMMAND_AMOVE:
		case COMMAND_RMOVE:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cMOVE %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (INT16)m_pr[0], (INT16)m_pr[1]);
				if (BIT(m_cr, 10))
				{
					m_cpx += (INT16)m_pr[0];
					m_cpy += (INT16)m_pr[1];
				}
				else
				{
					m_cpx = (INT16)m_pr[0];
					m_cpy = (INT16)m_pr[1];
				}
				command_end_seq();
			}
			break;

		case COMMAND_RRCT:
		case COMMAND_ARCT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cRTC (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (INT16)m_pr[0], (INT16)m_pr[1]);
				command_rct_exec();
				command_end_seq();
			}
			break;

		case COMMAND_RLINE:
		case COMMAND_ALINE:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cLINE (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (INT16)m_pr[0], (INT16)m_pr[1]);
				command_line_exec();
				command_end_seq();
			}
			break;

		case COMMAND_APLG:
		case COMMAND_RPLG:
		case COMMAND_APLL:
		case COMMAND_RPLL:
			if(m_param_ptr == 1)
				m_dn = m_pr[0]; // number of param words

			if(m_param_ptr == (1 + m_dn*2))
			{
				if (CMD_LOG)
				{
					logerror("HD63484 '%s': %cPL%c (%d, %d, %d) %d", tag(), BIT(m_cr, 10) ? 'R' : 'A', m_cr & 0x2000 ? 'G' : 'L', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
					for (int i=0; i<m_dn; i++)
						logerror(", %d, %d", (INT16)m_pr[1 + i * 2], (INT16)m_pr[1 + i * 2 + 1]);
					logerror("\n");
				}

				command_plg_exec();
				command_end_seq();
			}
			break;

		case COMMAND_CRCL:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': CRCL (%d, %d, %d, %d) %d\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
				UINT16 r = m_pr[0] & 0x1fff;
				draw_ellipse(m_cpx, m_cpy, r, r, DEGREE_TO_RADIAN(0), DEGREE_TO_RADIAN(360), BIT(m_cr, 8));
				command_end_seq();
			}
			break;

		case COMMAND_ELPS:
			if(m_param_ptr == 3)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': ELPS (%d, %d, %d, %d) %d, %d, %d\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0], m_pr[1], m_pr[2]);
				double dx = (double)m_pr[3];
				double dy = sqrt(pow(dx, 2) / ((double)m_pr[0] / m_pr[1]));
				draw_ellipse(m_cpx, m_cpy, dx, dy, DEGREE_TO_RADIAN(0), DEGREE_TO_RADIAN(360), BIT(m_cr, 8));
				command_end_seq();
			}
			break;

		case COMMAND_AARC:
		case COMMAND_RARC:
			if(m_param_ptr == 4)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cARC (%d, %d, %d, %d) %d, %d, %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (INT16)m_pr[0], (INT16)m_pr[1], (INT16)m_pr[2], (INT16)m_pr[3]);
				command_arc_exec();
				command_end_seq();
			}
			break;

		case COMMAND_AEARC:
		case COMMAND_REARC:
			if(m_param_ptr == 6)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cEARC (%d, %d, %d, %d) %d, %d, %d, %d, %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0], m_pr[1], m_pr[2], m_pr[3], m_pr[4], m_pr[5]);
				command_earc_exec();
				command_end_seq();
			}
			break;

		case COMMAND_AFRCT:
		case COMMAND_RFRCT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cFRCT (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (INT16)m_pr[0], (INT16)m_pr[1]);

				command_frct_exec();
				command_end_seq();
			}
			break;

		case COMMAND_PAINT:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': PAINT (%d, %d, %d, %d)\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07);
				paint(m_cpx, m_cpy);
				command_end_seq();
			}
			break;

		case COMMAND_DOT:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DOT (%d, %d, %d)\n", tag(), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07);
				set_dot(m_cpx, m_cpy, 0, 0);
				command_end_seq();
			}
			break;

		case COMMAND_PTN:
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': PTN (%d, %d, %d, %d, %d) 0x%04x\n", tag(), (m_cr >> 11) & 0x01, (m_cr >> 8) & 0x07, (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
				command_ptn_exec();
				command_end_seq();
			}
			break;

		case COMMAND_RGCPY:
		case COMMAND_AGCPY:
			if (m_param_ptr == 4)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cGCPY (%d, %d, %d, %d, %d) %d, %d, %d, %d\n", tag(), BIT(m_cr, 12) ? 'R' : 'A', (m_cr >> 11) & 0x01, (m_cr >> 8) & 0x07, (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (INT16)m_pr[0], (INT16)m_pr[1], (INT16)m_pr[2], (INT16)m_pr[3]);

				command_gcpy_exec();
				command_end_seq();
			}
			break;

		default:
			printf("%04x\n",m_cr);
			fatalerror("stop!\n");
	}
}

void h63484_device::exec_abort_sequence()
{
	fifo_w_clear();
	fifo_r_clear();
	m_sr = H63484_SR_WFR | H63484_SR_WFE | H63484_SR_CED; // hard-set to 0x23
}

UINT16 h63484_device::video_registers_r(int offset)
{
	UINT16 res = (m_vreg[offset] << 8) | (m_vreg[offset+1] & 0xff);

	switch(offset)
	{
		case 0x06:
			res = m_dcr;
			break;

		case 0x80:
			res = m_screen->vpos() & 0xfff; // Raster Count
			break;

		default:
			if(LOG) printf("%s R\n",acrtc_regnames[m_ar/2]);
			break;
	}

	return res;
}

void h63484_device::video_registers_w(int offset)
{
	UINT16 vreg_data;

	vreg_data = (m_vreg[offset]<<8)|(m_vreg[offset+1]&0xff);

	switch(offset)
	{
		case 0x00: // FIFO entry
			queue_w((vreg_data & 0xff00) >> 8);
			queue_w((vreg_data & 0x00ff) >> 0);
			if(FIFO_LOG) printf("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			process_fifo();
			break;

		case 0x02: // Command Entry

			if(vreg_data & 0x8000) // abort sequence (ABT)
				exec_abort_sequence();

			/*
			x--- ---- ---- ---- ABorT
			-x-- ---- ---- ---- PauSE
			...
			---- -xxx ---- ---- Graphic Bit Mode (bpp)
			---- ---- xxxx xxxx irq mask, directly correlated to sr
			*/
			m_ccr = vreg_data;
			break;

		case 0x04:
			m_omr = vreg_data;
			break;

		case 0x06:
			m_dcr = vreg_data;
			recompute_parameters();
			break;

		case 0x82: // Horizontal Sync Register
			m_hc = ((vreg_data & 0xff00) >> 8) + 1;
			m_hsw = vreg_data & 0x1f;
			recompute_parameters();
			break;
		case 0x84: // Horizontal Display Register
			m_hds = ((vreg_data & 0xff00) >> 8) + 1;
			m_hdw = ((vreg_data & 0x00ff) >> 0) + 1;
			recompute_parameters();
			break;
		case 0x92: // Horizontal Window Register
			m_hws = ((vreg_data & 0xff00) >> 8) + 1;
			m_hww = ((vreg_data & 0x00ff) >> 0) + 1;
			recompute_parameters();
			break;

		case 0x86: // Vertical Sync Register
			m_vc = (vreg_data & 0xfff);
			recompute_parameters();
			break;
		case 0x88: // Vertical Display Register
			m_vds = ((vreg_data & 0xff00) >> 8) + 1;
			m_vsw = (vreg_data & 0x1f);
			recompute_parameters();
			break;
		case 0x8a:  // Split Screen Width 1
			m_sp[1] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0x8c:  // Split Screen Width 0
			m_sp[0] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0x8e:  // Split Screen Width 2
			m_sp[2] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0x94: // Vertical Window Register A
			m_vws = (vreg_data & 0xfff) + 1;
			recompute_parameters();
			break;
		case 0x96: // Vertical Window Register B
			m_vww = (vreg_data & 0xfff);
			recompute_parameters();
			break;

		case 0xc2: // Memory Width Register
		case 0xca:
		case 0xd2:
		case 0xda:
			m_mwr[(offset & 0x18) >> 3] = vreg_data & 0xfff; // pitch
			m_mwr_chr[(offset & 0x18) >> 3] = (vreg_data & 0x8000) >> 15;
			break;

		case 0xc4: // Start Address Register
		case 0xcc:
		case 0xd4:
		case 0xdc:
			m_sar[(offset & 0x18) >> 3] = ((vreg_data & 0xf) << 16) | (m_sar[(offset & 0x18) >> 3] & 0xffff);
			m_sda[(offset & 0x18) >> 3] = (vreg_data & 0x0f00) >> 8;
			break;

		case 0xc6: // Start Address Register
		case 0xce:
		case 0xd6:
		case 0xde:
			m_sar[(offset & 0x18) >> 3] = (vreg_data & 0xffff) | (m_sar[(offset & 0x18) >> 3] & 0xf0000);
			break;

		default:
			if(LOG) printf("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			break;
	}
}

READ16_MEMBER( h63484_device::status_r )
{
	return m_sr;
}

READ16_MEMBER( h63484_device::data_r )
{
	UINT16 res = 0xffff;

	if(m_ar == 0) // FIFO read
	{
		UINT8 data;

		dequeue_r(&data);
		res = (data & 0xff) << 8;
		dequeue_r(&data);
		res |= data & 0xff;
	}
	else
		res = video_registers_r(m_ar);

	inc_ar(2);

	return res;
}

WRITE16_MEMBER( h63484_device::address_w )
{
	if(ACCESSING_BITS_0_7)
		m_ar = data & 0xfe;
}

WRITE16_MEMBER( h63484_device::data_w )
{
	if(ACCESSING_BITS_8_15)
		m_vreg[m_ar] = (data & 0xff00) >> 8;

	if(ACCESSING_BITS_0_7)
		m_vreg[m_ar+1] = (data & 0xff);

	video_registers_w(m_ar);

	inc_ar(2);
}

READ8_MEMBER( h63484_device::status_r )
{
	return m_sr;
}

WRITE8_MEMBER( h63484_device::address_w )
{
	m_ar = data;
}

READ8_MEMBER( h63484_device::data_r )
{
	UINT8 res = 0xff;

	if(m_ar < 2) // FIFO read
		dequeue_r(&res);
	else
		res = video_registers_r(m_ar & 0xfe) >> (m_ar & 1 ? 0 : 8);

	inc_ar(1);

	return res;
}

WRITE8_MEMBER( h63484_device::data_w )
{
	m_vreg[m_ar] = data;

	if(m_ar < 2) // FIFO write
	{
		queue_w(data);
		if (m_ar & 1)
			process_fifo();

		m_ar ^= 1;
	}
	else
		video_registers_w(m_ar & 0xfe);

	inc_ar(1);
}

void h63484_device::device_start()
{
	m_display_cb.bind_relative_to(*owner());

	register_save_state();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void h63484_device::device_reset()
{
	m_sr = H63484_SR_CED | H63484_SR_WFR | H63484_SR_WFE;
	m_ccr = m_omr = m_edg = m_dcr = m_hsw = 0;
	m_hc = m_hds = m_hdw = m_hws = m_hww = 0;
	m_vc = m_vws = m_vww = m_vds = m_vsw = 0;
	m_sp[0] = m_sp[1] = m_sp[2] = 0;
	m_ppx = m_ppy = 0;
	m_cl0 = m_cl1 = 0;
	m_xmin = m_ymin = m_xmax = m_ymax = 0;
	m_ppx = m_pzcx = m_psx = m_pzx = m_pex = 0;
	m_ppy = m_pzcy=  m_psy = m_pzy = m_pey = 0;
	m_ar = m_cr = 0;
	m_param_ptr = 0;
	m_rwp_dn = 0;
	m_org_dpa = 0;
	m_org_dn = 0;
	m_org_dpd = 0;
	m_ccmp = 0;
	m_mask = -1;
	m_cpx = m_cpy = 0;
	m_dn = 0;

	memset(m_vreg, 0, sizeof(m_vreg));
	memset(m_fifo, 0, sizeof(m_fifo));
	memset(m_fifo_r, 0, sizeof(m_fifo_r));
	memset(m_pr, 0, sizeof(m_pr));
	memset(m_rwp, 0, sizeof(m_rwp));
	memset(m_mwr, 0, sizeof(m_mwr));
	memset(m_mwr_chr, 0, sizeof(m_mwr_chr));
	memset(m_sar, 0, sizeof(m_sar));
	memset(m_sda, 0, sizeof(m_sda));
	memset(m_pram, 0, sizeof(m_pram));
}

//-------------------------------------------------
//  draw_graphics_line -
//-------------------------------------------------

void h63484_device::draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int vs, int y, int layer_n, bool active, bool ins_window)
{
	int bpp = get_bpp();
	int ppw = 16 / bpp;
	UINT32 mask = (1 << bpp) - 1;
	UINT32 base_offs = m_sar[layer_n] + (y - vs) * m_mwr[layer_n];
	UINT32 wind_offs = m_sar[3] + (y - m_vws) * m_mwr[3];
	int step = (m_omr & 0x08) ? 2 : 1;
	int gai = (m_omr>>4) & 0x07;
	int ppmc = ppw * (1 << gai) / step;  // TODO: GAI > 3
	int ws = m_hsw + m_hws;

	if (m_omr & 0x08)
	{
		/*
		    According to the datasheet, in interleaved and superimposed modes:
		    - HDW and HWW must be even
		    - the relation between HDS and HWS must be even/even or odd/odd
		*/

		if (m_hww & 1)
			ws += step;

		if ((m_hws & 1) ^ (m_hds & 1))
			wind_offs++;
	}

	for(int x=cliprect.min_x; x<=cliprect.max_x; x+=ppw)
	{
		UINT16 data = 0;
		if (ins_window && x >= ws * ppmc && x < (ws + m_hww) * ppmc)
		{
			data = readword(wind_offs);
			wind_offs++;
		}
		else if (active)
			data = readword(base_offs);

		for (int b=0; b<ppw; b++)
		{
			int px = x + b;
			if (!m_display_cb.isnull())
				m_display_cb(bitmap, cliprect, y, px, data & mask);
			else if (cliprect.contains(px, y))
				bitmap.pix16(y, px) = data & mask;

			data >>= bpp;
		}

		base_offs++;
	}
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 h63484_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int l0 = cliprect.min_y + (BIT(m_dcr, 13) ? m_sp[0] : 0);
	int l1 = l0 + m_sp[1];
	int l2 = l1 + (BIT(m_dcr, 11) ? m_sp[2] : 0);

	if(m_omr & 0x4000)
	{
		for(int y=cliprect.min_y; y<=cliprect.max_y; y++)
		{
			bool ins_window = BIT(m_dcr, 9) && y >= m_vws && y < m_vws+m_vww;

			if (BIT(m_dcr, 13) && y >= cliprect.min_y && y < l0)
				draw_graphics_line(bitmap, cliprect, cliprect.min_y, y, 0, BIT(m_dcr, 12), ins_window);
			else if (y >= l0 && y < l1)
				draw_graphics_line(bitmap, cliprect, l0, y, 1, BIT(m_dcr, 14), ins_window);
			else if (BIT(m_dcr, 11) && y >= l1 && y < l2)
				draw_graphics_line(bitmap, cliprect, l1, y, 2, BIT(m_dcr, 10), ins_window);
		}
	}
	return 0;
}

void h63484_device::register_save_state()
{
	save_item(NAME(m_ar));
	save_item(NAME(m_vreg));
	save_item(NAME(m_sr));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_ptr));
	save_item(NAME(m_fifo_r));
	save_item(NAME(m_fifo_r_ptr));
	save_item(NAME(m_cr));
	save_item(NAME(m_pr));
	save_item(NAME(m_param_ptr));
	save_item(NAME(m_rwp));
	save_item(NAME(m_rwp_dn));
	save_item(NAME(m_org_dpa));
	save_item(NAME(m_org_dn));
	save_item(NAME(m_org_dpd));
	save_item(NAME(m_cl0));
	save_item(NAME(m_cl1));
	save_item(NAME(m_ccmp));
	save_item(NAME(m_mask));
	save_item(NAME(m_cpx));
	save_item(NAME(m_cpy));
	save_item(NAME(m_mwr));
	save_item(NAME(m_mwr_chr));
	save_item(NAME(m_sar));
	save_item(NAME(m_sda));
	save_item(NAME(m_pram));
	save_item(NAME(m_dn));
	save_item(NAME(m_ccr));
	save_item(NAME(m_omr));
	save_item(NAME(m_edg));
	save_item(NAME(m_dcr));
	save_item(NAME(m_hc));
	save_item(NAME(m_hds));
	save_item(NAME(m_hdw));
	save_item(NAME(m_hws));
	save_item(NAME(m_hww));
	save_item(NAME(m_sp));
	save_item(NAME(m_hsw));
	save_item(NAME(m_vc));
	save_item(NAME(m_vws));
	save_item(NAME(m_vww));
	save_item(NAME(m_vds));
	save_item(NAME(m_vsw));
	save_item(NAME(m_ppy));
	save_item(NAME(m_pzcy));
	save_item(NAME(m_ppx));
	save_item(NAME(m_pzcx));
	save_item(NAME(m_psx));
	save_item(NAME(m_pex));
	save_item(NAME(m_pzx));
	save_item(NAME(m_psy));
	save_item(NAME(m_pzy));
	save_item(NAME(m_pey));
	save_item(NAME(m_xmin));
	save_item(NAME(m_ymin));
	save_item(NAME(m_xmax));
	save_item(NAME(m_ymax));
}
